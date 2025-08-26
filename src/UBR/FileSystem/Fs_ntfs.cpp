/*
 * Copyright (c) 2024-2025, UozaLab
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or 
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Fs_ntfs.h"
#include "MbrGpt.h"
#include <string>

bool NTFSUtility::bit_table_created = false;
UINT8 NTFSUtility::bit_table[256];

DWORD NTFSUtility::GetFileRecordSize(const NTFS_BPB* pbs)
{
    int sz = (char)pbs->ClustersPerFileRecord;
    DWORD FileRecordSize;

    if (sz > 0)
    {
        FileRecordSize = pbs->SectorsPerCluster * sz;
    }
    else
    {
        FileRecordSize = 1 << (-sz);
        FileRecordSize /= pbs->BytesPerSector;
    }
    return FileRecordSize;
}


RunListCollection::RunListCollection(const NTFS_BPB* pbs, const ATTRIBUTE_HEADER_NON_RESIDENT* ahnr, UINT64 _new_bitmap_size)
: new_bitmap_size(_new_bitmap_size), ByteWrite(0)
{
    const UINT8* data_run = (UINT8*) ahnr + ahnr->DataRunOffset;

    for(int i=0; i<ahnr->LastVCN - ahnr->FirstVCN + 1; i++)
    {
        RUN_LIST rl = get_data_run(data_run);
        if(!rl.valid) break;
        run_lists.push_back(rl);
        data_run += rl.Step;
    }
    new_run_list = construct_new_runlist(pbs);
}

UINT8* RunListCollection::construct_new_runlist(const NTFS_BPB* pbs)
{
    DWORD BytesPerCluster = pbs->SectorsPerCluster * pbs->BytesPerSector;
    UINT64 cluster_count = (new_bitmap_size + BytesPerCluster - 1) / BytesPerCluster;
    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(pbs);
    new_run_list_size = FileRecordSize * pbs->BytesPerSector;
    UINT8* new_runlist = new UINT8[new_run_list_size];
    ZeroMemory(new_runlist, new_run_list_size);
    UINT8* runlist = new_runlist;
    ULONGLONG address = 0;
    ULONGLONG bitmap_cluster_address = 0;

    ULONGLONG sum_clusters = 0;
    bool first = true;
    bool finished = false;
    for(std::vector<RUN_LIST>::iterator itr = run_lists.begin(), itr_end = run_lists.end(); itr != itr_end; itr++)
    {
        RUN_LIST rl = *itr;

        CLUSTER_RANGE cr;
        cr.AddressFrom = bitmap_cluster_address;
        UINT64 bitmap_cluster_address_tmp = rl.ClusterCount;
        bitmap_cluster_address_tmp *= BytesPerCluster;
        bitmap_cluster_address_tmp *= 8;
        bitmap_cluster_address += bitmap_cluster_address_tmp;
        cr.AddressTo = bitmap_cluster_address - 1;


        if(first)
        {
            address = rl.ClusterOffset;
            first = false;
        }
        else
        {
            address += rl.ClusterOffset;
        }

        if(finished)
        {
            RUN_LIST_BITMAP rlb;
            rlb.Operation = OP_REMOVE;
            rlb.ClusterCount = rl.ClusterCount;
            rlb.ClusterCountOrg = rl.ClusterCount;
            rlb.ClusterAddress = address;
            rlb.ClusterRange = cr;
            run_list_bitmap.push_back(rlb);
            continue;
        }

        if(sum_clusters + rl.ClusterCount >= cluster_count)
        {
            ULONGLONG clusters = cluster_count - sum_clusters;
            UINT8 ClusterCountLength = calc_cluster_count_length(clusters);

            runlist[0] = (rl.ClusterOffsetLength << 4) + ClusterCountLength;
            memcpy(&runlist[1], &clusters, ClusterCountLength);
            memcpy(&runlist[1 + ClusterCountLength], &rl.Data[1 + rl.ClusterCountLength], rl.ClusterOffsetLength);

            runlist += (1 + rl.ClusterOffsetLength + ClusterCountLength);
            ByteWrite += (1 + rl.ClusterOffsetLength + ClusterCountLength);

            RUN_LIST_BITMAP rlb;
            rlb.Operation = OP_MODIFY;
            rlb.ClusterCount = clusters;
            rlb.ClusterCountOrg = rl.ClusterCount;
            rlb.ClusterAddress = address;
            rlb.ClusterRange = cr;
            run_list_bitmap.push_back(rlb);

            finished = true;
            sum_clusters += clusters;
        }
        else
        {
            RUN_LIST_BITMAP rlb;
            rlb.Operation = OP_NOP;
            rlb.ClusterCount = rl.ClusterCount;
            rlb.ClusterCountOrg = rl.ClusterCount;
            rlb.ClusterAddress = address;
            rlb.ClusterRange = cr;
            run_list_bitmap.push_back(rlb);

            memcpy(runlist, rl.Data, rl.Step);
            runlist += rl.Step;
            ByteWrite += rl.Step;
            sum_clusters += rl.ClusterCount;
        }
    }

    if(ByteWrite % 8 != 0)
    {
        ByteWrite = ((ByteWrite + 7) / 8) * 8;
    }

    return new_runlist;
}

RunListCollection::~RunListCollection()
{
    for(std::vector<RUN_LIST>::iterator itr = run_lists.begin(), itr_end = run_lists.end(); itr != itr_end; itr++)
    {
        if(itr->Data != NULL)
            delete [] itr->Data;
    }
    delete [] new_run_list;
}

UINT8 RunListCollection::calc_cluster_count_length(ULONGLONG cluster_count)
{
    if(cluster_count <= 0xFF) return 1;
    if(cluster_count <= 0xFFFF) return 2;
    if(cluster_count <= 0xFFFFFF) return 3;
    if(cluster_count <= 0xFFFFFFFF) return 4;
    if(cluster_count <= 0xFFFFFFFFFF) return 5;
    if(cluster_count <= 0xFFFFFFFFFFFF) return 6;
    if(cluster_count <= 0xFFFFFFFFFFFFFF) return 7;
    return 8;
}


RUN_LIST RunListCollection::get_data_run(const UINT8* data_run_bytes)
{
    RUN_LIST run_list;
    run_list.valid = false;
    run_list.Data = NULL;

    UINT8 size = data_run_bytes[0];
    if(size == 0x00) return run_list;

    UINT8 cluster_count_length = size & 0x0F;
    UINT8 cluster_offset_length = size >> 4;
    if(cluster_count_length > 8) return run_list;
    if(cluster_offset_length > 8) return run_list;

    run_list.ClusterCount = 0;
	memcpy(&run_list.ClusterCount, &data_run_bytes[1], cluster_count_length);
    if(run_list.ClusterCount < 0) return run_list;

    run_list.ClusterOffset = 0;
    if(cluster_offset_length > 0)
    {
        if(data_run_bytes[cluster_count_length+cluster_offset_length] & 0x80)
            run_list.ClusterOffset = -1;
        memcpy(&run_list.ClusterOffset, &data_run_bytes[1+cluster_count_length], cluster_offset_length);
    }

    run_list.ClusterCountLength = cluster_count_length;
    run_list.ClusterOffsetLength = cluster_offset_length;
    run_list.Step = cluster_count_length + cluster_offset_length + 1;
    run_list.Data = new UINT8[run_list.Step];
    memcpy(run_list.Data, data_run_bytes, run_list.Step);
    run_list.valid = true;
    return run_list;
}

std::vector<RUN_LIST_BITMAP> RunListCollection::Intersect(ULONGLONG cluster_address_from, ULONGLONG cluster_count)
{
    std::vector<RUN_LIST_BITMAP> run_list_bitmaps;
    ULONGLONG cluster_address_to = cluster_address_from + cluster_count - 1;

    for(std::vector<RUN_LIST_BITMAP>::iterator itr = run_list_bitmap.begin(), itr_end = run_list_bitmap.end(); itr != itr_end; itr++)
    {
        if(itr->ClusterRange.AddressFrom > cluster_address_to) continue;
        if(itr->ClusterRange.AddressTo < cluster_address_from) continue;

        RUN_LIST_BITMAP rlb;
        memcpy(&rlb, &(*itr), sizeof(RUN_LIST_BITMAP));
        rlb.ClusterRangeIntersect.AddressFrom = itr->ClusterRange.AddressFrom;
        rlb.ClusterRangeIntersect.AddressTo = itr->ClusterRange.AddressTo;
        if(itr->ClusterRange.AddressFrom < cluster_address_from) rlb.ClusterRangeIntersect.AddressFrom = cluster_address_from;
        if(itr->ClusterRange.AddressTo > cluster_address_to) rlb.ClusterRangeIntersect.AddressTo = cluster_address_to;
        run_list_bitmaps.push_back(rlb);
    }

    return run_list_bitmaps;
}

FileRecordBitmap::FileRecordBitmap(const FILE_RECORD_HEADER* fr_header_bitmap, const NTFS_BPB* pbs)
: rlc(NULL), new_fr_header(NULL)
{
    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(pbs);
    new_fr_header = (FILE_RECORD_HEADER*) (new UINT8[FileRecordSize * pbs->BytesPerSector]);
    memcpy((UINT8*)new_fr_header, (UINT8*)fr_header_bitmap, FileRecordSize * pbs->BytesPerSector);
    ZeroMemory((UINT8*)new_fr_header + fr_header_bitmap->OffsetOfAttribute, fr_header_bitmap->RealSize - fr_header_bitmap->OffsetOfAttribute);

    ATTRIBUTE_HEADER* ahc = (ATTRIBUTE_HEADER*) ((UINT8*)fr_header_bitmap + fr_header_bitmap->OffsetOfAttribute);
    DWORD index = fr_header_bitmap->OffsetOfAttribute;
    DWORD new_index = fr_header_bitmap->OffsetOfAttribute;

    UINT64 bitmap_size = pbs->TotalSectors;
    bitmap_size /= pbs->SectorsPerCluster;
    bitmap_size /= 8;
    bitmap_size = ((bitmap_size + 7) / 8) * 8;
    DWORD BytesPerCluster = pbs->BytesPerSector;
    BytesPerCluster *= pbs->SectorsPerCluster;
    UINT64 bitmap_size_cluster = (bitmap_size + BytesPerCluster - 1) / BytesPerCluster;

    while(ahc->Type != 0xFFFFFFFF)
    {
        if(ahc->Length == 0) break;
        if(index + ahc->Length > FileRecordSize*pbs->BytesPerSector) break;
        if(ahc->Type == ATTRIBUTE_TYPE_DATA)
        {
            if(ahc->NonResident)
            {
                if(rlc != NULL) break;

                ATTRIBUTE_HEADER_NON_RESIDENT* ahnr = (ATTRIBUTE_HEADER_NON_RESIDENT*) (new UINT8[ahc->Length]);
                memcpy(ahnr, ahc, ahc->Length);
                rlc = new RunListCollection(pbs, ahnr, bitmap_size);
                UINT8* new_runlist = rlc->GetNewRunList();
                DWORD ByteSize = rlc->GetNewRunListByteWrite();

                ahnr->Header.Length = sizeof(ATTRIBUTE_HEADER_NON_RESIDENT) + ByteSize;
                ahnr->RealSize = bitmap_size;
                ahnr->InitializedSize = bitmap_size;
                ahnr->AllocatedSize = bitmap_size_cluster * BytesPerCluster;
                ahnr->LastVCN = ahnr->FirstVCN + bitmap_size_cluster - 1;

                memcpy((UINT8*)new_fr_header + new_index, (UINT8*)ahnr, ahnr->DataRunOffset);
                new_index += ahnr->DataRunOffset;

                memcpy((UINT8*)new_fr_header + new_index, (UINT8*)new_runlist, ByteSize);
                new_index += ByteSize;
                delete [] ahnr;
            }
            else
            {
                memcpy((UINT8*)new_fr_header + new_index, (UINT8*)fr_header_bitmap + index, ahc->Length);
                new_index += ahc->Length;
            }
        }
        else
        {
            memcpy((UINT8*)new_fr_header + new_index, (UINT8*)fr_header_bitmap + index, ahc->Length);
            new_index += ahc->Length;
        }

        index += ahc->Length;
        ahc = (ATTRIBUTE_HEADER*)((UINT8*)ahc + ahc->Length);
    }
    if(ahc->Type == 0xFFFFFFFF)
    {
        memcpy((UINT8*)new_fr_header + new_index, (UINT8*)fr_header_bitmap + index, 4);// end marker
        index += 4;
        new_index += 4;
    }
    new_fr_header->RealSize = ((new_index + 7) / 8) * 8;

}

FileRecordBitmap::~FileRecordBitmap()
{
    delete rlc;
    delete [] new_fr_header;
}


NTFS::NTFS(shared_ptr<VirtualDiskStream> _stream):
stream(_stream)
{
}


ATTRIBUTE_HEADER* NTFS::get_attr_header(NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header, DWORD attr_type)
{
    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(pbs);
    ATTRIBUTE_HEADER* ahc = (ATTRIBUTE_HEADER*) ((UINT8*)fr_header + fr_header->OffsetOfAttribute);
    DWORD index = fr_header->OffsetOfAttribute;

    while(ahc->Type != 0xFFFFFFFF)
    {
        if(ahc->Length == 0) break;
        if(index + ahc->Length > FileRecordSize*pbs->BytesPerSector) break;
        if(ahc->Type == attr_type) return ahc;

        index += ahc->Length;
        ahc = (ATTRIBUTE_HEADER*)((UINT8*)ahc + ahc->Length);
    }
    return NULL;
}

UINT64 NTFS::parse_size(DWORD FileRecordSize, NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header)
{
    UINT64 result = 0;
    ATTRIBUTE_HEADER* ahc = get_attr_header(pbs, fr_header, ATTRIBUTE_TYPE_DATA);
    if(ahc == NULL) return result;

    if(ahc->NonResident)
    {
        ATTRIBUTE_HEADER_NON_RESIDENT* ahnr = (ATTRIBUTE_HEADER_NON_RESIDENT*) ahc;
        result = ahnr->RealSize;
    }
    else
    {
        ATTRIBUTE_HEADER_RESIDENT* ahr = (ATTRIBUTE_HEADER_RESIDENT*) ahc;
        result = ahr->AttributeLength;
    }
    return result;
}

ParseResult NTFS::count_used_clusters(ULONGLONG count_start_sector, ULONGLONG sector_count, ULONGLONG cluster_count, DWORD BytesPerSector)
{
    ParseResult pr;
    ULONGLONG cluster_index = 0;
    ULONGLONG cluster_range_index = 0;
    CLUSTER_RANGE zero_series;

    bool found_zero = false;
    while(sector_count > 0)
    {
        UINT32 sectors_to_read = (sector_count <= 10) ? (UINT32) sector_count : 10;
        UINT8* sector_buffer = stream->ReadRange(count_start_sector, sectors_to_read*BytesPerSector);
        for(unsigned int i=0; i<sectors_to_read*BytesPerSector; i++)
        {
            if(cluster_count <= 0) break;
            if(cluster_count < 8)
            {
                sector_buffer[i] &= (0xFF >> (8 - (UINT8)cluster_count));
            }
            if(sector_buffer[i] != 0x00)
            {
                pr.FoundUsedCluster = true;
                pr.FixedClusterIndex = cluster_index;
                pr.FixedClusterBitmapIndex = 8 - NTFSUtility::NumberOfLeadingZero(sector_buffer[i]);
                pr.UsedClusterCount += NTFSUtility::BitCount(sector_buffer[i]);
            }

            bool skip = false;
            bool stop_cond_sector = (i == sectors_to_read*BytesPerSector - 1) && (sector_count <= sectors_to_read);
            bool stop_cond_cluster = (cluster_count <= 8);
            bool stop_cond = stop_cond_sector || stop_cond_cluster;
            if(!found_zero && sector_buffer[i] == 0xFF) skip = true;
            if(found_zero && sector_buffer[i] == 0x00 && !stop_cond) skip = true;

            if(!skip)
            {
                if((found_zero && sector_buffer[i] == 0x00 && stop_cond) ||
                   (found_zero && sector_buffer[i] != 0x00))
                {
                    if(sector_buffer[i] == 0x00)
                    {
                        if(stop_cond_cluster)
                            zero_series.AddressTo = cluster_range_index + cluster_count - 1;
                        else
                            zero_series.AddressTo = cluster_range_index + 7;

                        pr.EmptyClusterRanges.push_back(zero_series);
                    }
                    else
                    {
                        zero_series.AddressTo = cluster_range_index + NTFSUtility::NumberOfTrailingZero(sector_buffer[i]) - 1;
                        if(zero_series.AddressTo - zero_series.AddressFrom > 100)
                        {
                            pr.EmptyClusterRanges.push_back(zero_series);
                        }
                    }
                }

                if(!found_zero || sector_buffer[i] != 0x00)
                {
                    zero_series.AddressFrom = cluster_range_index + 8 - NTFSUtility::NumberOfLeadingZero(sector_buffer[i]);
                    found_zero = (NTFSUtility::NumberOfLeadingZero(sector_buffer[i]) != 0);
                }
            }

            cluster_index++;
            cluster_range_index += 8;
            if(cluster_count <= 8)
            {
                cluster_count = 0;
            }
            else
            {
                cluster_count -= 8;
            }
        }
        delete [] sector_buffer;

        if(cluster_count <= 0) break;
        if(sector_count <= sectors_to_read) break;
        sector_count -= sectors_to_read;
        count_start_sector += sectors_to_read;
    }
    return pr;
}

VOLUME_SIZE_INFO NTFS::parse_volume_info(UINT64 partition_start_sector, NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header)
{
    VOLUME_SIZE_INFO result;

    ATTRIBUTE_HEADER* ahc = get_attr_header(pbs, fr_header, ATTRIBUTE_TYPE_DATA);
    if(ahc == NULL) return result;

    if(ahc->NonResident)
    {
        ULONGLONG clusters= pbs->TotalSectors;
        clusters /= pbs->SectorsPerCluster;
        ULONGLONG clusters_org = clusters;

        FILE_RECORD_HEADER* fr_header_bitmap = get_file_record_header(partition_start_sector, pbs, MFT_ID_BITMAP);
        FileRecordBitmap frb(fr_header_bitmap, pbs);
        std::vector<RUN_LIST_BITMAP> rlb = frb.GetRunListBitmap(); 
        delete [] fr_header_bitmap;

        for(std::vector<RUN_LIST_BITMAP>::iterator itr = rlb.begin(), itr_end = rlb.end(); itr != itr_end; itr++)
        {
            ParseResult pr = count_used_clusters(partition_start_sector + itr->ClusterAddress * pbs->SectorsPerCluster,
                itr->ClusterCount * pbs->SectorsPerCluster,
                clusters,
                pbs->BytesPerSector);
            result.Used += pr.UsedClusterCount;
            ULONGLONG cluster_offset = clusters_org - clusters;
            bool first = true;
            for(std::vector<CLUSTER_RANGE>::iterator itr_cr = pr.EmptyClusterRanges.begin(), itr_end_cr = pr.EmptyClusterRanges.end(); itr_cr != itr_end_cr; itr_cr++)
            {
                CLUSTER_RANGE cr;
                cr.AddressFrom = itr_cr->AddressFrom + cluster_offset;
                cr.AddressTo = itr_cr->AddressTo + cluster_offset;

                if(first && !result.EmptyClusterRanges.empty())
                {
                    first = false;
                    CLUSTER_RANGE& cr_pre = result.EmptyClusterRanges.back();
                    if(cr_pre.AddressTo == cr.AddressFrom - 1)
                    {
                        CLUSTER_RANGE cr_rep;
                        cr_rep.AddressFrom = cr_pre.AddressFrom;
                        cr_rep.AddressTo = cr.AddressTo;
                        result.EmptyClusterRanges.pop_back();
                        result.EmptyClusterRanges.push_back(cr_rep);
                        continue;
                    }
                }
                first = false;
                result.EmptyClusterRanges.push_back(cr);
            }

            if(pr.FoundUsedCluster)
            {
                result.Fixed = itr->ClusterRange.AddressFrom + pr.FixedClusterIndex * 8 + (pr.FixedClusterBitmapIndex - 1);
            }
            clusters -= (itr->ClusterCount * pbs->SectorsPerCluster * pbs->BytesPerSector)*8;
        }
        result.Used *= (pbs->SectorsPerCluster * pbs->BytesPerSector);
        result.Fixed *= (pbs->SectorsPerCluster * pbs->BytesPerSector);
    }
    else
    {
        ATTRIBUTE_HEADER_RESIDENT* ahr = (ATTRIBUTE_HEADER_RESIDENT*) ahc;
        UINT8 *AttrBody = (UINT8*)((UINT8*)ahr + ahr->AttributeOffset);
        DWORD AttrBodySize = ahr->AttributeLength;
        ULONGLONG used_clusters = 0;

        UINT8* tmp = AttrBody;
        for(DWORD i=0; i<AttrBodySize; i++)
        {
            if(tmp[i] != 0x00)
            {
                result.Fixed = i;
            }
            used_clusters += NTFSUtility::BitCount(tmp[i]);
        }
        result.Used = used_clusters;
        result.Used *= (pbs->SectorsPerCluster * pbs->BytesPerSector);
        result.Fixed = (result.Fixed * 8 + NTFSUtility::BitmapIndex(tmp[result.Fixed])) * (pbs->SectorsPerCluster * pbs->BytesPerSector);
    }


    result.Total = pbs->TotalSectors;
    result.Total /= pbs->SectorsPerCluster;
    result.Total *= pbs->SectorsPerCluster;
    result.Total *= pbs->BytesPerSector;
    result.UsedRatio = (double) result.Used / (double) result.Total;

    result.SizeCalculated = true;
    result.BytesPerSector = pbs->BytesPerSector;
    result.SectorsPerCluster = pbs->SectorsPerCluster;


    return result;
}


tstring NTFS::parse_volume_name(DWORD FileRecordSize, NTFS_BPB* pbs,FILE_RECORD_HEADER* fr_header)
{
    tstring result;
    ATTRIBUTE_HEADER* ahc = get_attr_header(pbs, fr_header, ATTRIBUTE_TYPE_VOLUME_NAME);
    if(ahc == NULL) return result;

    if(ahc->NonResident)
    {
        // Maximum size of volume name is 32 characters.
        // So ignore NonResident case.
    }
    else
    {
        ATTRIBUTE_HEADER_RESIDENT* ahr = (ATTRIBUTE_HEADER_RESIDENT*) ahc;
        UINT8 *AttrBody = (UINT8*)((UINT8*)ahr + ahr->AttributeOffset);
        char tmp[MAX_PATH + 1];
        ZeroMemory(tmp, MAX_PATH + 1);
        memcpy(tmp, (char*) AttrBody, ahr->AttributeLength);
        std::wstring wstr(reinterpret_cast<wchar_t*>(tmp), ahr->AttributeLength/sizeof(wchar_t));
        result = wstring2tstring(wstr);
    }
    return result;
}


FILE_RECORD_HEADER* NTFS::get_file_record_header(UINT64 partition_start_sector, NTFS_BPB* pbs, int mft_id)
{
    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(pbs);
    UINT64 mft_start_sector = partition_start_sector + pbs->MftStartLcn * pbs->SectorsPerCluster;
    UINT8* mft_sector = stream->ReadRange(mft_start_sector + (FileRecordSize * mft_id), FileRecordSize * pbs->BytesPerSector);
    FILE_RECORD_HEADER* fr_header = (FILE_RECORD_HEADER*) mft_sector;

    // update sequence
    if(fr_header->RealSize <= (UINT32) (pbs->BytesPerSector-2)) return fr_header;
    WORD us_size = fr_header->SizeOfUpdateSequence;
    if(us_size == 0) return fr_header;
    WORD* us = (WORD*) (mft_sector + fr_header->OffsetOfUpdateSequence);
    WORD usn = *us;
    if(usn == 0) return fr_header;
    WORD* us_array = us + 1;

    for(int i = 1; i < us_size; i++)
    {
        WORD* sector_tail = (WORD*) (mft_sector + (pbs->BytesPerSector - 2));
        if(*sector_tail != usn) return fr_header;
        *sector_tail = *us_array;

        mft_sector += pbs->BytesPerSector;
        us_array++;
    }

    return fr_header;
}

void NTFS::patch_file_record_header(FILE_RECORD_HEADER* fr_header, DWORD BytesPerSector)
{
    UINT8* mft_sector = (UINT8*)fr_header;
    WORD us_size = fr_header->SizeOfUpdateSequence;
    if(us_size == 0) return;
    WORD* us = (WORD*) (mft_sector + fr_header->OffsetOfUpdateSequence);
    WORD usn = *us;
    if(usn == 0) return;
    WORD* us_array = us + 1;

    for(int i = 1; i < us_size; i++)
    {
        WORD* sector_tail = (WORD*) (mft_sector + (BytesPerSector - 2));
        *us_array = *sector_tail;
        *sector_tail = usn;

        mft_sector += BytesPerSector;
        us_array++;
    }

}

FSInfo NTFS::GetFSInfo(UINT64 partition_start_sector)
{
    FSInfo result;
    NTFS_BPB* pbs = (NTFS_BPB*) stream->ReadRange(partition_start_sector, sizeof(NTFS_BPB));
    if(pbs->Sign[0] != 0x55 || pbs->Sign[1] != 0xAA)
    {
        delete [] (UINT8*) pbs;

        result.TypeInfo.VolumeName = _T("");
        result.TypeInfo.FileSystemName = _T("Unknown");
        result.TypeInfo.FileSystemType = FS_TYPE_UNKNOWN;
        result.TypeInfo.PartitionType = PART_TYPE_Empty;
        result.SizeInfo.SizeCalculated = false;
        return result;
    }
    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(pbs);

    // Volume Size
    FILE_RECORD_HEADER* fr_header_bitmap = get_file_record_header(partition_start_sector, pbs, MFT_ID_BITMAP);
    result.SizeInfo = parse_volume_info(partition_start_sector, pbs, fr_header_bitmap);

    // Volume Name
    FILE_RECORD_HEADER* fr_header_volume = get_file_record_header(partition_start_sector, pbs, MFT_ID_VOLUME);
    result.TypeInfo.VolumeName = parse_volume_name(FileRecordSize, pbs, fr_header_volume);
    result.TypeInfo.FileSystemName = _T("NTFS");
    result.TypeInfo.FileSystemType = FS_TYPE_NTFS;
    result.TypeInfo.PartitionType = PART_TYPE_NTFS_exFAT;

    delete [] (UINT8*) pbs;
    delete [] (UINT8*) fr_header_bitmap;
    delete [] (UINT8*) fr_header_volume;
    return result;
}


void NTFS::GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, shared_ptr<MBRGPT> mbrgpt)
{
    UINT64 partition_start_sector = mbrgpt->GetPartitionStartSectorToShrink();
    UINT64 sectors_to_shrink = mbrgpt->GetSectorsToShrink();
    UINT64 fixed_sectors = mbrgpt->GetFixedSectors();
    DWORD target_parition = mbrgpt->GetTargetPartitionNumberToShrink();

    UINT8* sector_data = stream->ReadRange(partition_start_sector, sizeof(NTFS_BPB));
    NTFS_BPB* boot_sector_mod = (NTFS_BPB*)sector_data;

    DWORD FileRecordSize = NTFSUtility::GetFileRecordSize(boot_sector_mod);
    FILE_RECORD_HEADER* fr_header_mft = get_file_record_header(partition_start_sector, boot_sector_mod, MFT_ID_MFT);
    UINT64 mft_size = parse_size(FileRecordSize, boot_sector_mod, fr_header_mft);
    UINT64 mft_size_sector = mft_size / boot_sector_mod->BytesPerSector;


    shared_ptr<MBRGPT> mbrgpt_org = mbrgpt->GetOriginal();
    bool exist_backup_bootsector = false;
    UINT64 backup_bootsector = 0;
    if(mbrgpt->IsGPT())
    {
        UINT64 partition_sectors = mbrgpt_org->MbrGptInfo.GPE[target_parition-1].LastLBA;
        partition_sectors -= mbrgpt_org->MbrGptInfo.GPE[target_parition-1].FirstLBA;
        partition_sectors += 1;

        if(boot_sector_mod->TotalSectors < partition_sectors)
        {
            exist_backup_bootsector = true;
            backup_bootsector = mbrgpt->MbrGptInfo.GPE[target_parition-1].LastLBA;
        }
    }
    else
    {
        UINT64 partition_sectors = mbrgpt_org->MbrGptInfo.PE[target_parition-1].NumberOfSectors;

        if(boot_sector_mod->TotalSectors < partition_sectors)
        {
            exist_backup_bootsector = true;
            backup_bootsector = mbrgpt->MbrGptInfo.PE[target_parition-1].LBAOfFirstAbsoluteSector;
            backup_bootsector += mbrgpt->MbrGptInfo.PE[target_parition-1].NumberOfSectors;
            backup_bootsector -= 1;
        }
    }

    boot_sector_mod->TotalSectors -= sectors_to_shrink;
    data_collection->AppendByteUpdate(sector_data, sizeof(NTFS_BPB), partition_start_sector*boot_sector_mod->BytesPerSector);
    if(exist_backup_bootsector)
        data_collection->AppendByteUpdate(sector_data, sizeof(NTFS_BPB), backup_bootsector*boot_sector_mod->BytesPerSector);

    SectorRangeHolder holder;
    holder.Append(partition_start_sector + boot_sector_mod->MftStartLcn * boot_sector_mod->SectorsPerCluster,
        partition_start_sector + boot_sector_mod->MftStartLcn * boot_sector_mod->SectorsPerCluster + mft_size_sector -1,
        true);
    DWORD mft_mirror_sector = (FileRecordSize*4) / boot_sector_mod->BytesPerSector;
    holder.Append(partition_start_sector + boot_sector_mod->MftStartLcnMirr * boot_sector_mod->SectorsPerCluster,
        partition_start_sector + boot_sector_mod->MftStartLcnMirr * boot_sector_mod->SectorsPerCluster + max(mft_mirror_sector, boot_sector_mod->SectorsPerCluster) -1,
        true);
    holder.Append(partition_start_sector + 1, partition_start_sector + fixed_sectors, false);

    for(int i=0; i<holder.Count(); i++)
    {
        SectorRange sr = holder.Get(i);
        data_collection->Append(sr.To - sr.From + 1, sr.From, sr.From, !sr.Mft);
    }

    // update $BITMAP File Record
    UINT64 bitmap_size = boot_sector_mod->TotalSectors;
    bitmap_size /= boot_sector_mod->SectorsPerCluster;
    UINT8 carry = bitmap_size % 8;
    bitmap_size /= 8;
    UINT64 bitmap_size_fit = (carry == 0) ? bitmap_size : bitmap_size + 1;
    bitmap_size = ((bitmap_size + 7) / 8) * 8;

    DWORD BytesPerCluster = boot_sector_mod->BytesPerSector;
    BytesPerCluster *= boot_sector_mod->SectorsPerCluster;

    FILE_RECORD_HEADER* fr_header_bitmap = get_file_record_header(partition_start_sector, boot_sector_mod, MFT_ID_BITMAP);
    FileRecordBitmap frb(fr_header_bitmap, boot_sector_mod);

    FILE_RECORD_HEADER* new_frb = frb.GetFileRecord();
    patch_file_record_header(new_frb, boot_sector_mod->BytesPerSector);
    data_collection->AppendIntercept((UINT8*) new_frb, 
        FileRecordSize * boot_sector_mod->BytesPerSector, boot_sector_mod->BytesPerSector,
        partition_start_sector + boot_sector_mod->MftStartLcn * boot_sector_mod->SectorsPerCluster + FileRecordSize*MFT_ID_BITMAP);

    std::vector<RUN_LIST_BITMAP> bmps_to_remove = frb.GetRunListBitmap();
    ULONGLONG total_clusters = boot_sector_mod->TotalSectors;
    total_clusters /= boot_sector_mod->SectorsPerCluster;
    for(std::vector<RUN_LIST_BITMAP>::iterator itr = bmps_to_remove.begin(), itr_end = bmps_to_remove.end(); itr!=itr_end; itr++)
    {
        if(itr->Operation == OP_MODIFY)
        {
            ULONGLONG cluster_removed_count = itr->ClusterCountOrg - itr->ClusterCount;
            std::vector<RUN_LIST_BITMAP> rlbi = frb.GetRunListBitmapIntersect(itr->ClusterAddress + itr->ClusterCount, cluster_removed_count);
            for(std::vector<RUN_LIST_BITMAP>::iterator iitr = rlbi.begin(), iitr_end = rlbi.end(); iitr != iitr_end; iitr++)
            {
                data_collection->AppendBitmapBitop(partition_start_sector + iitr->ClusterAddress * boot_sector_mod->SectorsPerCluster,
                    iitr->ClusterRange.AddressFrom,
                    iitr->ClusterRangeIntersect.AddressFrom, iitr->ClusterRangeIntersect.AddressTo,
                    boot_sector_mod->SectorsPerCluster, boot_sector_mod->BytesPerSector, true);
            }

            if(carry == 0 && bitmap_size_fit < bitmap_size)
            {
                data_collection->AppendBitmapBitop(partition_start_sector + itr->ClusterAddress * boot_sector_mod->SectorsPerCluster,
                    itr->ClusterRange.AddressFrom,
                    bitmap_size_fit  * 8, (bitmap_size - 1) * 8 + 7,
                    boot_sector_mod->SectorsPerCluster, boot_sector_mod->BytesPerSector, false/*op_set*/);
            }
            else if(carry != 0 && bitmap_size_fit <= bitmap_size)
            {
                data_collection->AppendBitmapBitop(partition_start_sector + itr->ClusterAddress * boot_sector_mod->SectorsPerCluster,
                    itr->ClusterRange.AddressFrom,
                    (bitmap_size_fit - 1) * 8 + carry, (bitmap_size - 1) * 8 + 7,
                    boot_sector_mod->SectorsPerCluster, boot_sector_mod->BytesPerSector, false/*op_set*/);
            }

        }
        if(itr->Operation == OP_REMOVE)
        {
            if(itr->ClusterAddress < total_clusters)
            {
                std::vector<RUN_LIST_BITMAP> rlbi = frb.GetRunListBitmapIntersect(itr->ClusterAddress, itr->ClusterCount);
                for(std::vector<RUN_LIST_BITMAP>::iterator iitr = rlbi.begin(), iitr_end = rlbi.end(); iitr != iitr_end; iitr++)
                {
                    data_collection->AppendBitmapBitop(partition_start_sector + iitr->ClusterAddress * boot_sector_mod->SectorsPerCluster,
                        iitr->ClusterRange.AddressFrom,
                        iitr->ClusterRangeIntersect.AddressFrom, iitr->ClusterRangeIntersect.AddressTo,
                        boot_sector_mod->SectorsPerCluster, boot_sector_mod->BytesPerSector, true);
                }
            }
        }
    }


    delete [] fr_header_mft;
    delete [] fr_header_bitmap;
    delete [] sector_data;
}

