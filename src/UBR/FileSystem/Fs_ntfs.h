/*
 * Copyright (c) 2024-2026, UozaLab
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

#ifndef __NTFS__H__
#define __NTFS__H__

#include <windows.h>
#include <vector>
#include <algorithm>
#include "smart_ptr.h"
#include "FsInfo.h"
#include "VdiskStream.h"
#include "tstring.h"
#include "MbrGpt.h"

#pragma pack(1)

struct NTFS_BPB
{
    UINT8 JumpInstruction[3];
    UINT8 OEMID[8];
    UINT16 BytesPerSector;
    UINT8 SectorsPerCluster;
    UINT16 ReservedSectors;
    UINT8 Zeros1[3];
    UINT16 UnUsed;
    UINT8 MediaDescriptor;
    UINT8 Zeros2[2];
    UINT16 SectorsPerTrack;
    UINT16 NumberOfHeads;
    UINT32 HiddenSectors;
    UINT8 UnUsed2[4];
    UINT8 UnUsed3[4];
    ULONGLONG TotalSectors;
    ULONGLONG MftStartLcn;
    ULONGLONG MftStartLcnMirr;
    UINT32 ClustersPerFileRecord;
    UINT32 ClustersPerIndexBlock;
    UINT8 VolumeSerialNumber[8];
    UINT8 Checksum[4];
    UINT8 BootStrapCode[426];
    UINT8 Sign[2];
};

struct FILE_RECORD_HEADER
{
    UINT8 Signature[4];
    UINT16 OffsetOfUpdateSequence;
    UINT16 SizeOfUpdateSequence;
    ULONGLONG LSN;
    UINT16 SequenceNumber;
    UINT16 HardlinkCount;
    UINT16 OffsetOfAttribute;
    UINT16 Flags;
    UINT32 RealSize;
    UINT32 AllocatedSize;
    ULONGLONG ReferenceToBaseFileRecord;
    UINT16 NextAttributeId;
    UINT8 UnUsed[2];
    UINT32 RecordId;
};

struct ATTRIBUTE_HEADER
{
    UINT32 Type;
    UINT32 Length;
    UINT8 NonResident;
    UINT8 NameLength;
    UINT16 NameOffset;
    UINT16 Flags;
    UINT16 AttributeId;
};

struct ATTRIBUTE_HEADER_RESIDENT
{
    ATTRIBUTE_HEADER Header;
    UINT32 AttributeLength;
    UINT16 AttributeOffset;
    UINT8 IndexedFlag;
    UINT8 Padding;
};

struct ATTRIBUTE_HEADER_NON_RESIDENT
{
    ATTRIBUTE_HEADER Header;
    ULONGLONG FirstVCN;
    ULONGLONG LastVCN;
    UINT16 DataRunOffset;
    UINT16 CompressionUnitSize;
    UINT32 Padding;
    ULONGLONG AllocatedSize;
    ULONGLONG RealSize;
    ULONGLONG InitializedSize;
};

#pragma pack()

#define MFT_ID_MFT 0
#define MFT_ID_MFT_MIRROR 1
#define MFT_ID_LOG_FILE 2
#define MFT_ID_VOLUME 3
#define MFT_ID_ATTR_DEF 4
#define MFT_ID_ROOT 5
#define MFT_ID_BITMAP 6
#define MFT_ID_BOOT 7
#define MFT_ID_BAD_CLUSTER 8
#define MFT_ID_QUOTA 9
#define MFT_ID_UPCASE 10
#define MFT_ID_CAIRO 11
#define MFT_ID_USER 16

#define ATTRIBUTE_TYPE_STANDARD_INFORMATION 0x10
#define ATTRIBUTE_TYPE_ATTRIBUTE_LIST 0x20
#define ATTRIBUTE_TYPE_FILE_NAME 0x30
#define ATTRIBUTE_TYPE_OBJECT_ID 0x40
#define ATTRIBUTE_TYPE_SECURITY_DESCRIPTOR 0x50
#define ATTRIBUTE_TYPE_VOLUME_NAME 0x60
#define ATTRIBUTE_TYPE_VOLUME_INFORMATION 0x70
#define ATTRIBUTE_TYPE_DATA 0x80
#define ATTRIBUTE_TYPE_INDEX_ROOT 0x90
#define ATTRIBUTE_TYPE_INDEX_ALLOCATION 0xA0
#define ATTRIBUTE_TYPE_BITMAP 0xB0
#define ATTRIBUTE_TYPE_SYMBOLIC_LINK 0xC0
#define ATTRIBUTE_TYPE_EA_INFORMATION 0xD0
#define ATTRIBUTE_TYPE_EA 0xE0
#define ATTRIBUTE_TYPE_USER 0x100


struct ParseResult
{
    std::vector<CLUSTER_RANGE> EmptyClusterRanges;
    ULONGLONG UsedClusterCount;
    ULONGLONG FixedClusterIndex;
    int FixedClusterBitmapIndex;
    bool FoundUsedCluster;
    ParseResult() :
    UsedClusterCount(0ULL), FixedClusterIndex(0ULL), FixedClusterBitmapIndex(0), FoundUsedCluster(false) {}
};

struct SectorRange
{
    UINT64 From;
    UINT64 To;
    bool Mft;
    SectorRange(UINT64 from, UINT64 to, bool mft):
    From(from), To(to), Mft(mft) {}
};

class SectorRangeHolder
{
protected:
    std::vector<SectorRange> sector_ranges;
    bool overlap(const SectorRange& r1, const SectorRange& r2)
    {
        if(r1.From <= r2.From && r1.To >= r2.From) return true;
        if(r1.From <= r2.To && r1.To >= r2.To) return true;

        if(r2.From <= r1.From && r2.To >= r1.From) return true;
        if(r2.From <= r1.To && r2.To >= r1.To) return true;

        return false;
    }
public:
    int Count()
    {
        return static_cast<int>(sector_ranges.size());
    }
    SectorRange Get(int index)
    {
        return sector_ranges[index];
    }

    void Append(UINT64 from, UINT64 to, bool mft)
    {
        SectorRange sr(from, to, mft);

        if(mft)
        {
            sector_ranges.push_back(sr);
        }
        else
        {
            std::vector<SectorRange> sector_ranges_to_append;
            sector_ranges_to_append.push_back(sr);

            std::vector<UINT64> split_address;
            for(std::vector<SectorRange>::iterator itr  = sector_ranges.begin(), itr_end = sector_ranges.end();
                itr != itr_end; itr++)
            {
                if(!itr->Mft) continue;

                if(std::count(split_address.begin(), split_address.end(), itr->From)==0)
                    split_address.push_back(itr->From);
                if(std::count(split_address.begin(), split_address.end(), itr->To + 1)==0)
                    split_address.push_back(itr->To + 1);
            }
            for(std::vector<UINT64>::iterator itr  = split_address.begin(), itr_end = split_address.end();
                itr != itr_end; itr++)
            {
                UINT64 address = *itr;
                for(std::vector<SectorRange>::iterator itr_inner  = sector_ranges_to_append.begin(), itr_end_inner = sector_ranges_to_append.end();
                itr_inner != itr_end_inner; itr_inner++)
                {
                    if(itr_inner->From <= address && itr_inner->To >= address)
                    {
                         SectorRange sr_tmp = *itr_inner;
                         sector_ranges_to_append.erase(itr_inner);
                         sector_ranges_to_append.push_back(SectorRange(sr_tmp.From, address-1, false));
                         sector_ranges_to_append.push_back(SectorRange(address, sr_tmp.To, false));
                         break;
                    }
                }

            }

            std::vector<SectorRange> sector_ranges_tmp;
            for(std::vector<SectorRange>::iterator itr  = sector_ranges_to_append.begin(), itr_end = sector_ranges_to_append.end();
                itr != itr_end; itr++)
            {
                bool skip = false;
                for(std::vector<SectorRange>::iterator itr_inner  = sector_ranges.begin(), itr_end_inner = sector_ranges.end();
                itr_inner != itr_end_inner; itr_inner++)
                {
                    if(overlap(*itr, *itr_inner))
                    {
                        skip = true;
                        break;
                    }
                }
                if(!skip)
                {
                    sector_ranges_tmp.push_back(*itr);
                }
            }

            sector_ranges.insert(sector_ranges.begin(), sector_ranges_tmp.begin(), sector_ranges_tmp.end());
        }
    }

};

class NTFSUtility
{
private:
    static bool bit_table_created;
    static UINT8 bit_table[256];
    static int bit_count(UINT8 target)
    {
        int sum;
        for(sum=0; target != 0; target &= target-1)
        {
            sum++;
        }
        return sum;
    }
    static void create_bit_table()
    {
        for(int i=0; i<=255; i++)
        {
            bit_table[i] = bit_count((UINT8) i);
        }
    }
public:
    static int BitCount(UINT8 target)
    {
        if(!bit_table_created)
        {
            create_bit_table();
            bit_table_created = true;
        }
        return bit_table[target];
    }

    static int NumberOfTrailingZero(UINT8 target)
    {
        //NumberOfTrailingZero(0100b) => 2
        return BitCount(~target & (target - 1));
    }
    static int NumberOfLeadingZero(UINT8 target)
    {
        //NumberOfLeadingZero(0100b) => 1
        target = target | ( target >> 1);
        target = target | ( target >> 2);
        target = target | ( target >> 4);
        return BitCount(~target);
    }

    static int BitmapIndex(UINT8 target)
    {
        return 8 - NumberOfLeadingZero(target);
    }
    static DWORD GetFileRecordSize(const NTFS_BPB* pbs);

};


struct RUN_LIST
{
    bool valid;
    LONGLONG ClusterCount;
    LONGLONG ClusterOffset;
    UINT8 ClusterCountLength;
    UINT8 ClusterOffsetLength;
    UINT8 Step;
    UINT8* Data;
};

enum RUN_LIST_BITMAP_OPERATION
{
    OP_NOP,
    OP_MODIFY,
    OP_REMOVE,
};

struct RUN_LIST_BITMAP
{
    RUN_LIST_BITMAP_OPERATION Operation;
    ULONGLONG ClusterCount;
    ULONGLONG ClusterCountOrg;
    ULONGLONG ClusterAddress;
    CLUSTER_RANGE ClusterRange;
    CLUSTER_RANGE ClusterRangeIntersect;
};

class RunListCollection
{
protected:
    std::vector<RUN_LIST> run_lists;
    std::vector<RUN_LIST_BITMAP> run_list_bitmap;
    RUN_LIST get_data_run(const UINT8* data_run_bytes);
    UINT8 calc_cluster_count_length(ULONGLONG cluster_count);
    UINT8* new_run_list;
    int new_run_list_size;
    DWORD ByteWrite;
    UINT64 new_bitmap_size;
    UINT8* construct_new_runlist(const NTFS_BPB* pbs);
public:
    RunListCollection(const NTFS_BPB* pbs, const ATTRIBUTE_HEADER_NON_RESIDENT* ahnr, UINT64 _new_bitmap_size);
    ~RunListCollection();
    UINT8* GetNewRunList() { return new_run_list; }
    int GetNewRunListSize() { return new_run_list_size; }
    int GetNewRunListByteWrite() { return ByteWrite; }
    std::vector<RUN_LIST_BITMAP> GetRunListBitmap() { return run_list_bitmap; }
    std::vector<RUN_LIST_BITMAP> Intersect(ULONGLONG cluster_address_from, ULONGLONG cluster_count);
};

class FileRecordBitmap
{
protected:
    RunListCollection *rlc;
    FILE_RECORD_HEADER* new_fr_header;
public:
    FileRecordBitmap(const FILE_RECORD_HEADER* fr_header_bitmap, const NTFS_BPB* pbs);
    ~FileRecordBitmap();
    std::vector<RUN_LIST_BITMAP> GetRunListBitmap() { return rlc->GetRunListBitmap(); }
    std::vector<RUN_LIST_BITMAP> GetRunListBitmapIntersect(ULONGLONG cluster_address_from, ULONGLONG cluster_count) { return rlc->Intersect(cluster_address_from, cluster_count); }
    FILE_RECORD_HEADER* GetFileRecord() { return new_fr_header; }
};

class NTFS
{
protected:
    shared_ptr<VirtualDiskStream> stream;

    ATTRIBUTE_HEADER* get_attr_header(NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header, DWORD attr_type);
    FILE_RECORD_HEADER* get_file_record_header(UINT64 partition_start_sector, NTFS_BPB* pbs, int mft_id);
    void patch_file_record_header(FILE_RECORD_HEADER* fr_header, DWORD BytesPerSector);

    UINT64 parse_size(DWORD FileRecordSize, NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header);
    tstring parse_volume_name(DWORD FileRecordSize, NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header);
    VOLUME_SIZE_INFO parse_volume_info(UINT64 partition_start_sector, NTFS_BPB* pbs, FILE_RECORD_HEADER* fr_header);
    ParseResult count_used_clusters(ULONGLONG count_start_sector, ULONGLONG sector_count, ULONGLONG cluster_count, DWORD BytesPerSector);

public:
    NTFS(shared_ptr<VirtualDiskStream> _stream);
    FSInfo GetFSInfo(UINT64 partition_start_sector);
    void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, shared_ptr<MBRGPT> mbrgpt);

};

#endif
