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

#include <string>
#include "Fs_fat.h"

FAT::FAT(shared_ptr<VirtualDiskStream> _stream):
stream(_stream)
{
}

void FAT::GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, shared_ptr<MBRGPT> mbrgpt)
{
    UINT64 partition_start_sector = mbrgpt->GetPartitionStartSectorToShrink();
    UINT64 sectors_to_shrink = mbrgpt->GetSectorsToShrink();
    UINT64 fixed_sectors = mbrgpt->GetFixedSectors();

    UINT8* start_sector = stream->ReadRange(partition_start_sector, sizeof(FAT_BS));
    shared_ptr<FATExtractor> extractor = FATExtractorFactory::Create(partition_start_sector, start_sector);
    extractor->GetShrinkedSectorData(data_collection, sectors_to_shrink, fixed_sectors);
    delete [] start_sector;
}

FSInfo FAT::GetFSInfo(UINT64 partition_start_sector)
{
    FSInfo fsinfo;
    UINT8* start_sector = stream->ReadRange(partition_start_sector, sizeof(FAT_BS));
    FAT_BS* bs = (FAT_BS*)start_sector;
    if(bs->Sign[0] != 0x55 || bs->Sign[1] != 0xAA)
    {
        delete [] start_sector;

        fsinfo.TypeInfo.VolumeName = _T("");
        fsinfo.TypeInfo.FileSystemName = _T("Unknown");
        fsinfo.TypeInfo.FileSystemType = FS_TYPE_UNKNOWN;
        fsinfo.TypeInfo.PartitionType = PART_TYPE_Empty;
        fsinfo.SizeInfo.SizeCalculated = false;
        return fsinfo;
    }

    shared_ptr<FATExtractor> extractor = FATExtractorFactory::Create(partition_start_sector, start_sector);

    fsinfo.SizeInfo = extractor->GetVolumeSize(stream);
    fsinfo.TypeInfo.VolumeName = extractor->GetVolumeName(stream);
    FS_TYPE fs_type = extractor->GetFSType();
    fsinfo.TypeInfo.FileSystemType = fs_type;
    fsinfo.TypeInfo.FileSystemName = 
        (fs_type == FS_TYPE_FAT12) ? _T("FAT12"):
        (fs_type == FS_TYPE_FAT16) ? _T("FAT16"):
        (fs_type == FS_TYPE_FAT32) ? _T("FAT32"):
        (fs_type == FS_TYPE_exFAT) ? _T("exFAT"): _T("Unknown");
    fsinfo.TypeInfo.PartitionType = 
        (fs_type == FS_TYPE_FAT12) ? PART_TYPE_FAT12:
        (fs_type == FS_TYPE_FAT16) ? PART_TYPE_FAT16:
        (fs_type == FS_TYPE_FAT32) ? PART_TYPE_FAT32:
        (fs_type == FS_TYPE_exFAT) ? PART_TYPE_NTFS_exFAT: PART_TYPE_Empty;

    delete [] start_sector;
    return fsinfo;
}



tstring FATExtractor::GetVolumeName(shared_ptr<VirtualDiskStream> stream)
{
    tstring result;

    UINT32 first_root_dir_entry_sector_index = FirstRootDirectoryEntrySectorIndex();
    UINT16 bytes_per_sector = BytesPerSector();

    for(int dir_index=0; dir_index<10; dir_index++) // search for volume label entry within 10 sectors
    {
        UINT8* entry = stream->ReadRange(partition_start_sector + first_root_dir_entry_sector_index + dir_index, bytes_per_sector);
        result = FindVolumeLabel(entry);
        delete [] entry;
        if(!result.empty()) break;
    }
    return result;
}

int FATExtractorUtility::pow(int x, int n)
{
    int ret = 1;
    for(int i=0; i<n; i++)
    {
        ret *= x;
    }
    return ret;
}

UINT32 FATExtractorUtility::GetRootDirectoryEntrySector(const FAT_BS* bs)
{
    UINT32 sectors_per_fat = (bs->Bpb.LogicalSectorsPerFAT == 0)
        ? bs->Ebp.Fat32.LogicalSectorsPerFAT
        : bs->Bpb.LogicalSectorsPerFAT;

    UINT32 root_dir_start_sector = bs->Bpb.ReservedLogicalSectors + (bs->Bpb.NumberOfFileAllocationTables * sectors_per_fat);
    return root_dir_start_sector;
}

UINT32 FATExtractorUtility::GetDataStartSector(const FAT_BS* bs)
{
    UINT16 bytes_per_sector = bs->Bpb.BytesPerLogicalSector;

    UINT32 root_dir_start_sector = GetRootDirectoryEntrySector(bs);
    UINT32 root_dir_sectors = ((bs->Bpb.MaximumNumberOfRootDirectoryEntries * sizeof(FAT_DIRECTORY_ENTRY)) + (bytes_per_sector - 1)) / bytes_per_sector;
    UINT32 data_start_sector = root_dir_start_sector + root_dir_sectors;
    return data_start_sector;
}

UINT32 FATExtractorUtility::GetDataClusters(const FAT_BS* bs)
{
    UINT32 total_sectors = (bs->Bpb.TotalLogicalSectors == 0) 
            ? bs->Bpb.TotalLogicalSectorsIncludingHiddenSectors
            : bs->Bpb.TotalLogicalSectors;

    UINT32 data_start_sector = GetDataStartSector(bs);
    UINT32 data_sectors = total_sectors - data_start_sector;
    UINT32 clusters = data_sectors / bs->Bpb.LogicalSectorsPerCluster;

    return clusters;
}


UINT64 FATExtractor_12::StartSector()
{
	return partition_start_sector + boot_sector->Bpb.ReservedLogicalSectors;
}

UINT32 FATExtractor_12::SectorsPerFat()
{
    return (boot_sector->Bpb.LogicalSectorsPerFAT == 0) ? boot_sector->Ebp.Fat32.LogicalSectorsPerFAT
                                                        : boot_sector->Bpb.LogicalSectorsPerFAT;
}

UINT16 FATExtractor_12::BytesPerSector()
{
	return boot_sector->Bpb.BytesPerLogicalSector;
}

UINT8 FATExtractor_12::SectorsPerCluster()
{
	return boot_sector->Bpb.LogicalSectorsPerCluster;
}

UINT64 FATExtractor_12::TotalSectors()
{
    return (boot_sector->Bpb.TotalLogicalSectors == 0) 
            ? boot_sector->Bpb.TotalLogicalSectorsIncludingHiddenSectors
            : boot_sector->Bpb.TotalLogicalSectors;
}

UINT32 FATExtractor_12::FirstRootDirectoryEntrySectorIndex()
{
    UINT32 sectors_per_fat = SectorsPerFat();
    UINT32 first_root_dir_entry_sector_index = (boot_sector->Bpb.ReservedLogicalSectors + (sectors_per_fat * boot_sector->Bpb.NumberOfFileAllocationTables));

    return first_root_dir_entry_sector_index;
}

FS_TYPE FATExtractor_12::GetFSType()
{
    return FS_TYPE_FAT12; 
}

tstring FATExtractor_12::FindVolumeLabel(const UINT8* dir_entry_data)
{
	UINT16 bytes_per_sector = BytesPerSector();
    int entry_per_sector = bytes_per_sector / sizeof(FAT_DIRECTORY_ENTRY);
    bool found = false;

    FAT_DIRECTORY_ENTRY* dir_entry = (FAT_DIRECTORY_ENTRY*) dir_entry_data;
    for(int i=0; i<entry_per_sector; i++)
    {
        if(((char*)dir_entry)[0] == 0x00) break;
        if(dir_entry->FileAttributes == DIR_FAT_ATTR_VOLUME_ID)
        {
            found = true;
            break;
        }
        dir_entry++;
    }

    tstring result;
    if(found)
    {
        char tmp[MAX_PATH + 1];
        ZeroMemory(tmp, MAX_PATH + 1);
        memcpy(tmp, (char*) dir_entry->ShortFileName, 11/*max size*/);
        result = string2tstring(std::string(tmp));
    }

    return result;
}

void FATExtractor_12::GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors)
{
    UINT8 sector_data[512];
    memcpy(sector_data, boot_sector, sizeof(sector_data));
    FAT_BS* boot_sector_mod = (FAT_BS*)sector_data;


    UINT64 sectors_after_reserved = TotalSectors() - StartSector();
    sectors_after_reserved -= sectors_to_shrink;
    UINT64 data_clusters = sectors_after_reserved;

    data_clusters *= BytesPerSector();
    data_clusters -= ((UINT32)boot_sector_mod->Bpb.MaximumNumberOfRootDirectoryEntries) * sizeof(FAT_DIRECTORY_ENTRY);
    data_clusters -= BytesPerSector();
    data_clusters += boot_sector_mod->Bpb.NumberOfFileAllocationTables * 4;
    data_clusters += 1;
    data_clusters *= 2;

    UINT64 denom = SectorsPerCluster();
    denom *= BytesPerSector();
    denom *= 2;
    denom += 3 * boot_sector_mod->Bpb.NumberOfFileAllocationTables;
    data_clusters /= denom;

    UINT64 fat_bytes = (data_clusters + 2);
    fat_bytes *= 3;
    fat_bytes /= 2;
    if(data_clusters % 2 != 0)
        fat_bytes += 1;
    boot_sector_mod->Bpb.LogicalSectorsPerFAT = (UINT16) ((fat_bytes + BytesPerSector() - 1)/BytesPerSector());
    if(boot_sector_mod->Bpb.TotalLogicalSectors == 0)
    {
        boot_sector_mod->Bpb.TotalLogicalSectorsIncludingHiddenSectors -= (UINT32)sectors_to_shrink;
    }
    else
    {
        boot_sector_mod->Bpb.TotalLogicalSectors -= (UINT16)sectors_to_shrink;
    }

    data_collection->AppendByteUpdate(sector_data, sizeof(sector_data), partition_start_sector*BytesPerSector());

    for(int i=0; i<boot_sector->Bpb.NumberOfFileAllocationTables; i++)
    {
        data_collection->Append(boot_sector_mod->Bpb.LogicalSectorsPerFAT,
            StartSector() + SectorsPerFat()*i,
            partition_start_sector + boot_sector_mod->Bpb.ReservedLogicalSectors + boot_sector_mod->Bpb.LogicalSectorsPerFAT*i,
            false);
    }

    UINT64 root_dir_entries_sectors = boot_sector_mod->Bpb.MaximumNumberOfRootDirectoryEntries;
    root_dir_entries_sectors *= sizeof(FAT_DIRECTORY_ENTRY);
    root_dir_entries_sectors /= BytesPerSector();

    data_collection->Append(root_dir_entries_sectors + fixed_sectors,
        partition_start_sector + FATExtractorUtility::GetRootDirectoryEntrySector(boot_sector), 
        partition_start_sector + FATExtractorUtility::GetRootDirectoryEntrySector(boot_sector_mod));

}

VOLUME_SIZE_INFO FATExtractor_12::GetVolumeSize(shared_ptr<VirtualDiskStream> stream)
{
    VOLUME_SIZE_INFO info;

    FS_TYPE fs_type = GetFSType();

    UINT64 read_address_sector = StartSector();
    UINT32 FAT_length_sector = SectorsPerFat();
    UINT16 bytes_per_sector = BytesPerSector();
    UINT8 sectors_per_cluster = SectorsPerCluster();

    UINT32 count_zero = 0;
    UINT64 count_zero_index = 0;
    UINT32 count_all = 0;
    bool found_zero = false;
    CLUSTER_RANGE zero_series;

    const UINT32 SECTOR_READ_COUNT = 12;// 12 % 3 = 0 ; for FAT12
    UINT32 fixed_index = 0;
    UINT32 fixed_i = 0;
    UINT32 last_index = (FAT_length_sector + SECTOR_READ_COUNT - 1)/SECTOR_READ_COUNT;
    for(UINT32 index=0; index < last_index; index++)
    {
        UINT32 sectors_to_read = SECTOR_READ_COUNT;
        if((index+1)*SECTOR_READ_COUNT > FAT_length_sector)
            sectors_to_read = FAT_length_sector - index*SECTOR_READ_COUNT;

        unsigned char* fat_sector = stream->Read(read_address_sector, sectors_to_read);
        UINT32 buff_size = sectors_to_read * bytes_per_sector;

        for(UINT32 i=0; i<buff_size; i+=3)
        {
            if(i == buff_size - 1) break;
            UINT16 tmp = (UINT16) (fat_sector[i+1] & 0x0F);
            UINT16 data = fat_sector[i] | (tmp << 8);

            bool skip = false;
            bool stop_cond_sector = (i >= buff_size-2);
            bool stop_cond_cluster = (index == last_index - 1);
            bool stop_cond = stop_cond_sector && stop_cond_cluster;
            if(!found_zero && data != 0x0000) skip = true;
            if(found_zero && data == 0x0000 && !stop_cond) skip = true;

            if(!skip)
            {
                if(found_zero && data == 0x0000 && stop_cond ||
                    found_zero && data != 0x0000 )
                {
                    if(data == 0x0000)
                    {
                        zero_series.AddressTo = count_all;
                        info.EmptyClusterRanges.push_back(zero_series);
                    }
                    else
                    {
                        zero_series.AddressTo = count_all - 1;
                        if(zero_series.AddressTo - zero_series.AddressFrom > 100)
                        {
                            info.EmptyClusterRanges.push_back(zero_series);
                        }
                    }
                }
                if(!found_zero)
                {
                    zero_series.AddressFrom = count_all;
                }
            }

            if(data == 0x0000)
            {
                count_zero++;
                found_zero = true;
            }
            else
            {
                fixed_index = index;
                fixed_i = i;
                found_zero = false;
            }

            count_all += 1;
            if(i == buff_size - 2) break;

            tmp = (UINT16) ((fat_sector[i+1] & 0xF0) >> 4);
            data = ((UINT16) fat_sector[i+2]) << 4 | tmp;

            skip = false;
            stop_cond_sector = (i >= buff_size-4);
            stop_cond_cluster = (index == last_index - 1);
            stop_cond = stop_cond_sector && stop_cond_cluster;
            if(!found_zero && data != 0x0000) skip = true;
            if(found_zero && data == 0x0000 && !stop_cond) skip = true;

            if(!skip)
            {
                if(found_zero && data == 0x0000 && stop_cond ||
                    found_zero && data != 0x0000 )
                {
                    if(data == 0x0000)
                    {
                        zero_series.AddressTo = count_all;
                        info.EmptyClusterRanges.push_back(zero_series);
                    }
                    else
                    {
                        zero_series.AddressTo = count_all - 1;
                        if(zero_series.AddressTo - zero_series.AddressFrom > 100)
                        {
                            info.EmptyClusterRanges.push_back(zero_series);
                        }
                    }
                }
                if(!found_zero)
                {
                    zero_series.AddressFrom = count_all;
                }
            }

            if(data == 0x0000)
            {
                count_zero++;
                found_zero = true;
            }
            else
            {
                fixed_index = index;
                fixed_i = i + 1;
                found_zero = false;
            }

            count_all += 1;
        }

        read_address_sector += sectors_to_read;
        delete [] fat_sector;
    }

    info.Fixed = fixed_index;
    info.Fixed *= SECTOR_READ_COUNT;
    info.Fixed *= bytes_per_sector;
    info.Fixed += fixed_i;
    info.Fixed /= 3;
    info.Fixed *= 2;
    info.Fixed += ((fixed_i % 3 == 0) ? 0 : 1);
    info.Fixed -= 2;//skip 2 clusters

    info.Fixed += 1;// to convert index to #
    info.Fixed *= sectors_per_cluster;
    info.Fixed *= bytes_per_sector;

    count_all -= 2;//skip 2 clusters
    info.Total = count_all;
    info.Total *= sectors_per_cluster;
    info.Total *= bytes_per_sector;
    info.Used = (count_all - count_zero);
    info.Used *= sectors_per_cluster;
    info.Used *= bytes_per_sector;
    info.UsedRatio = (double) info.Used / (double) info.Total;
    info.SizeCalculated = true;

    info.BytesPerSector = bytes_per_sector;
    info.SectorsPerCluster = sectors_per_cluster;

    return info;    
}


void FATExtractor_16::GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors)
{
    UINT8 sector_data[512];
    memcpy(sector_data, boot_sector, sizeof(sector_data));
    FAT_BS* boot_sector_mod = (FAT_BS*)sector_data;


    UINT64 sectors_after_reserved = TotalSectors() - StartSector();
    sectors_after_reserved -= sectors_to_shrink;
    UINT64 data_clusters = sectors_after_reserved;

    data_clusters *= BytesPerSector();
    data_clusters -= ((UINT32)boot_sector_mod->Bpb.MaximumNumberOfRootDirectoryEntries) * sizeof(FAT_DIRECTORY_ENTRY);
    data_clusters -= BytesPerSector();
    data_clusters += boot_sector_mod->Bpb.NumberOfFileAllocationTables * 4;
    data_clusters += 1;
    UINT64 denom = SectorsPerCluster();
    denom *= BytesPerSector();
    denom += 2 * boot_sector_mod->Bpb.NumberOfFileAllocationTables;
    data_clusters /= denom;

    boot_sector_mod->Bpb.LogicalSectorsPerFAT = (UINT16) (((data_clusters + 2)* 2 + BytesPerSector() - 1)/BytesPerSector());
    if(boot_sector_mod->Bpb.TotalLogicalSectors == 0)
    {
        boot_sector_mod->Bpb.TotalLogicalSectorsIncludingHiddenSectors -= (UINT32)sectors_to_shrink;
    }
    else
    {
        boot_sector_mod->Bpb.TotalLogicalSectors -= (UINT16)sectors_to_shrink;
    }

    data_collection->AppendByteUpdate(sector_data, sizeof(sector_data), partition_start_sector*BytesPerSector());
    for(int i=0; i<boot_sector->Bpb.NumberOfFileAllocationTables;i++)
    {
        data_collection->Append(boot_sector_mod->Bpb.LogicalSectorsPerFAT,
            StartSector() + SectorsPerFat()*i,
            partition_start_sector + boot_sector_mod->Bpb.ReservedLogicalSectors + boot_sector_mod->Bpb.LogicalSectorsPerFAT*i,
            false);
    }
    UINT64 root_dir_entries_sectors = boot_sector_mod->Bpb.MaximumNumberOfRootDirectoryEntries;
    root_dir_entries_sectors *= sizeof(FAT_DIRECTORY_ENTRY);
    root_dir_entries_sectors /= BytesPerSector();

    data_collection->Append(root_dir_entries_sectors + fixed_sectors,
        partition_start_sector + FATExtractorUtility::GetRootDirectoryEntrySector(boot_sector), 
        partition_start_sector + FATExtractorUtility::GetRootDirectoryEntrySector(boot_sector_mod));
}


FS_TYPE FATExtractor_16::GetFSType()
{
    return FS_TYPE_FAT16; 
}

VOLUME_SIZE_INFO FATExtractor_16::GetVolumeSize(shared_ptr<VirtualDiskStream> stream)
{
    VOLUME_SIZE_INFO info;

    FS_TYPE fs_type = GetFSType();

    UINT64 read_address_sector = StartSector();
    UINT32 FAT_length_sector = SectorsPerFat();
    UINT16 bytes_per_sector = BytesPerSector();
    UINT8 sectors_per_cluster = SectorsPerCluster();

    UINT32 count_zero = 0;
    UINT64 count_zero_index = 0;
    UINT32 count_all = 0;
    bool found_zero = false;
    CLUSTER_RANGE zero_series;

    const UINT32 SECTOR_READ_COUNT = 12;// 12 % 3 = 0 ; for FAT12
    UINT32 fixed_index = 0;
    UINT32 fixed_i = 0;
    UINT32 last_index = (FAT_length_sector + SECTOR_READ_COUNT - 1)/SECTOR_READ_COUNT;
    for(UINT32 index=0; index < last_index; index++)
    {
        UINT32 sectors_to_read = SECTOR_READ_COUNT;
        if((index+1)*SECTOR_READ_COUNT > FAT_length_sector)
            sectors_to_read = FAT_length_sector - index*SECTOR_READ_COUNT;

        unsigned char* fat_sector = stream->Read(read_address_sector, sectors_to_read);
        UINT32 buff_size = sectors_to_read * bytes_per_sector;

        for(UINT32 i=0; i<buff_size; i+=2)
        {
            UINT16* tmp = (UINT16*) &fat_sector[i];

            bool skip = false;
            bool stop_cond_sector = (i >= buff_size-2);
            bool stop_cond_cluster = (index == last_index - 1);
            bool stop_cond = stop_cond_sector && stop_cond_cluster;
            if(!found_zero && *tmp != 0x0000) skip = true;
            if(found_zero && *tmp == 0x0000 && !stop_cond) skip = true;

            if(!skip)
            {
                if(found_zero && *tmp == 0x0000 && stop_cond ||
                    found_zero && *tmp != 0x0000 )
                {
                    if(*tmp == 0x0000)
                    {
                        zero_series.AddressTo = count_all;
                        info.EmptyClusterRanges.push_back(zero_series);
                    }
                    else
                    {
                        zero_series.AddressTo = count_all - 1;
                        if(zero_series.AddressTo - zero_series.AddressFrom > 100)
                        {
                            info.EmptyClusterRanges.push_back(zero_series);
                        }
                    }
                }
                if(!found_zero)
                {
                    zero_series.AddressFrom = count_all;
                }
            }

            if(*tmp == 0x0000)
            {
                count_zero++;
                found_zero = true;
            }
            else
            {
                fixed_index = index;
                fixed_i = i;
                found_zero = false;
            }

            count_all++;
        }

        read_address_sector += sectors_to_read;
        delete [] fat_sector;
    }

    info.Fixed = fixed_index;
    info.Fixed *= SECTOR_READ_COUNT;
    info.Fixed *= bytes_per_sector;
    info.Fixed += fixed_i;
    info.Fixed /= 2;//16bit
    info.Fixed -= 2;//skip 2 cluster
    info.Fixed += 1;// to convert index to #
    info.Fixed *= sectors_per_cluster;
    info.Fixed *= bytes_per_sector;

    count_all -= 2;//skip 2 clusters
    info.Total = count_all;
    info.Total *= sectors_per_cluster;
    info.Total *= bytes_per_sector;
    info.Used = (count_all - count_zero);
    info.Used *= sectors_per_cluster;
    info.Used *= bytes_per_sector;
    info.UsedRatio = (double) info.Used / (double) info.Total;
    info.SizeCalculated = true;

    info.BytesPerSector = bytes_per_sector;
    info.SectorsPerCluster = sectors_per_cluster;

    return info;    
}


UINT32 FATExtractor_32::FirstRootDirectoryEntrySectorIndex()
{
    UINT32 sectors_per_fat = SectorsPerFat();
    UINT32 first_root_dir_entry_sector_index = (boot_sector->Bpb.ReservedLogicalSectors + (sectors_per_fat * boot_sector->Bpb.NumberOfFileAllocationTables));

    UINT32 root_cluster_sector = (boot_sector->Ebp.Fat32.ClusterNumberOfRootDirectoryStart - 2);
    root_cluster_sector *= SectorsPerCluster();
    first_root_dir_entry_sector_index += root_cluster_sector;

    return first_root_dir_entry_sector_index;
}

FS_TYPE FATExtractor_32::GetFSType()
{
    return FS_TYPE_FAT32;
}

VOLUME_SIZE_INFO FATExtractor_32::GetVolumeSize(shared_ptr<VirtualDiskStream> stream)
{
    VOLUME_SIZE_INFO info;
    FS_TYPE fs_type = GetFSType();

    UINT64 read_address_sector = StartSector();
    UINT32 FAT_length_sector = SectorsPerFat();
    UINT16 bytes_per_sector = BytesPerSector();
    UINT8 sectors_per_cluster = SectorsPerCluster();

    UINT32 count_zero = 0;
    UINT64 count_zero_index = 0;
    UINT32 count_all = 0;
    bool found_zero = false;
    CLUSTER_RANGE zero_series;

    const UINT32 SECTOR_READ_COUNT = 12;// 12 % 3 = 0 ; for FAT12
    UINT32 fixed_index = 0;
    UINT32 fixed_i = 0;
    UINT32 last_index = (FAT_length_sector + SECTOR_READ_COUNT - 1)/SECTOR_READ_COUNT;
    for(UINT32 index=0; index < last_index; index++)
    {
        UINT32 sectors_to_read = SECTOR_READ_COUNT;
        if((index+1)*SECTOR_READ_COUNT > FAT_length_sector)
            sectors_to_read = FAT_length_sector - index*SECTOR_READ_COUNT;

        unsigned char* fat_sector = stream->Read(read_address_sector, sectors_to_read);
        UINT32 buff_size = sectors_to_read * bytes_per_sector;

        for(UINT32 i=0; i<buff_size; i+=4)
        {
            UINT32* tmp = (UINT32*) &fat_sector[i];
            *tmp &= 0x0FFFFFFF;

            bool skip = false;
            bool stop_cond_sector = (i >= buff_size-4);
            bool stop_cond_cluster = (index == last_index - 1);
            bool stop_cond = stop_cond_sector && stop_cond_cluster;
            if(!found_zero && *tmp != 0x00000000) skip = true;
            if(found_zero && *tmp == 0x00000000 && !stop_cond) skip = true;

            if(!skip)
            {
                if(found_zero && *tmp == 0x00000000 && stop_cond ||
                    found_zero && *tmp != 0x00000000 )
                {
                    if(*tmp == 0x00000000)
                    {
                        zero_series.AddressTo = count_all;
                        info.EmptyClusterRanges.push_back(zero_series);
                    }
                    else
                    {
                        zero_series.AddressTo = count_all - 1;
                        if(zero_series.AddressTo - zero_series.AddressFrom > 100)
                        {
                            info.EmptyClusterRanges.push_back(zero_series);
                        }
                    }
                }
                if(!found_zero)
                {
                    zero_series.AddressFrom = count_all;
                }
            }

            if(*tmp == 0x00000000)
            {
                count_zero++;
                found_zero = true;
            }
            else
            {
                fixed_index = index;
                fixed_i = i;
                found_zero = false;
            }

            count_all++;
        }

        read_address_sector += sectors_to_read;
        delete [] fat_sector;
    }

    info.Fixed = fixed_index;
    info.Fixed *= SECTOR_READ_COUNT;
    info.Fixed *= bytes_per_sector;
    info.Fixed += fixed_i;
    info.Fixed /= 4;//32bit
    info.Fixed -= 2;//skip 2 cluster
    info.Fixed += 1;// to convert index to #
    info.Fixed *= sectors_per_cluster;
    info.Fixed *= bytes_per_sector;

    count_all -= 2;//skip 2 clusters
    info.Total = count_all;
    info.Total *= sectors_per_cluster;
    info.Total *= bytes_per_sector;
    info.Used = (count_all - count_zero);
    info.Used *= sectors_per_cluster;
    info.Used *= bytes_per_sector;
    info.UsedRatio = (double) info.Used / (double) info.Total;
    info.SizeCalculated = true;

    info.BytesPerSector = bytes_per_sector;
    info.SectorsPerCluster = sectors_per_cluster;

    return info;    
}

void FATExtractor_32::GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors)
{
    UINT8 sector_data[512];
    memcpy(sector_data, boot_sector, sizeof(sector_data));
    FAT_BS* boot_sector_mod = (FAT_BS*)sector_data;

    UINT64 sectors_after_reserved = TotalSectors() - StartSector();
    sectors_after_reserved -= sectors_to_shrink;
    UINT64 data_clusters = sectors_after_reserved;
    data_clusters *= BytesPerSector();
    data_clusters -= 8 * boot_sector_mod->Bpb.NumberOfFileAllocationTables;
    UINT64 denom = SectorsPerCluster();
    denom *= BytesPerSector();
    denom += 4 * boot_sector_mod->Bpb.NumberOfFileAllocationTables;
    data_clusters /= denom;

    boot_sector_mod->Ebp.Fat32.LogicalSectorsPerFAT = (UINT32) (((data_clusters + 2) * 4 + BytesPerSector() - 1)/BytesPerSector());
    boot_sector_mod->Bpb.TotalLogicalSectorsIncludingHiddenSectors -= (UINT32)sectors_to_shrink;

    data_collection->AppendByteUpdate(sector_data, sizeof(sector_data), partition_start_sector*BytesPerSector());
    data_collection->AppendByteUpdate(sector_data, sizeof(sector_data), (partition_start_sector + boot_sector_mod->Ebp.Fat32.FirstLogicalSectorNumberOfBackupBootSectors)*BytesPerSector());
    for(int i=0; i<boot_sector->Bpb.NumberOfFileAllocationTables; i++)
    {
        data_collection->Append(boot_sector_mod->Ebp.Fat32.LogicalSectorsPerFAT,
            StartSector() + SectorsPerFat()*i,
            partition_start_sector + boot_sector_mod->Bpb.ReservedLogicalSectors + boot_sector_mod->Ebp.Fat32.LogicalSectorsPerFAT*i,
            false);
    }

    data_collection->Append(fixed_sectors, 
        partition_start_sector + FATExtractorUtility::GetDataStartSector(boot_sector), 
        partition_start_sector + FATExtractorUtility::GetDataStartSector(boot_sector_mod));

}
