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

#include "VDisk_vss.h"
#include "PhysicalDiskInfo.h"
#include "ForensicAnalysis.h"
#include "Misc.h"
#include <map>
#include <algorithm>


PartitionSequence PartitionSequenceUtility::CreateInvalidSequence(DWORD bytes_per_sector, DWORD disk_id, UINT64 start_sector, UINT64 end_sector)
{
    PartitionSequence seq;

    seq.invalid = true;
    seq.Snapshot = false;
    seq.DeviceNumber = disk_id;
    seq.PartitionNumber = 0;
    seq.StartSector = start_sector;
    seq.EndSector = end_sector;
    seq.PartitionLength = (end_sector - start_sector);
    seq.PartitionLength++;
    seq.PartitionLength *= bytes_per_sector;

    return seq;
}


PartitionSequence PartitionSequenceUtility::GetPartitionSequence(const std::vector<PartitionSequence>& part_seqs, UINT64 address_sector)
{
    for(std::vector<PartitionSequence>::const_iterator itr = part_seqs.begin(), itr_end = part_seqs.end(); itr != itr_end; itr++)
    {
        if(itr->StartSector <= address_sector && itr->EndSector >= address_sector)
        {
            return *itr;
        }
    }
    PartitionSequence seq;
    seq.Index = -1;
    seq.invalid = true;
    return seq;
}


PhysicalVSS::PhysicalVSS(int _disk_number, shared_ptr<DiskInfo> _di)
     : Physical(_disk_number), di(_di), handle_current(NULL), partition_current(-1)
{
    shared_ptr<PhysicalDiskInfo> pdi = di->PhysicalDisks[disk_number];
    std::vector<tstring> volume_names;
    for(std::vector<VolumeInfo>::iterator itr = pdi->Volumes.begin(), itr_end = pdi->Volumes.end(); itr != itr_end; itr++)
    {
        if(itr->VolumeGUIDPath.empty()) continue;
        volume_names.push_back(itr->VolumeGUIDPath);
    }
    
    vss_created = (SystemEnvironment::RunOnPE()) ? false : vss.CreateSnapshot(volume_names);

    std::map<int, PartitionSequence> seq_map;
    std::vector<int> seq_map_keys;
    for(std::vector<PARTITION_INFORMATION_EX>::iterator itr = pdi->Partitions.begin(), itr_end = pdi->Partitions.end(); itr != itr_end; itr++)
    {
        PartitionSequence seq;
        seq.DeviceNumber = disk_number;
        seq.PartitionNumber = itr->PartitionNumber;
        VolumeInfo vi = PhysicalDiskUtil::FindVolumeInfo(pdi, itr->PartitionNumber);
        if(vi.Invalid || !vss_created)
        {
            seq.Snapshot = false;
        }
        else
        {
            seq.OriginalVolumeName = vi.VolumeGUIDPath;
            tstring volume_name = vi.VolumeGUIDPath + _T("\\");
            seq.SnapshotDeviceObject = vss.FindSnapshotVolume(volume_name);
            seq.Snapshot = !seq.SnapshotDeviceObject.empty();
        }
        seq.StartSector = itr->StartingOffset.QuadPart;
        seq.StartSector /= GetSectorSize();
        seq.EndSector = itr->PartitionLength.QuadPart;
        seq.EndSector /= GetSectorSize();
        seq.EndSector += seq.StartSector;
        seq.EndSector--;
        seq.PartitionLength = itr->PartitionLength.QuadPart;
        seq.invalid = false;

        seq_map[seq.PartitionNumber] = seq;
        seq_map_keys.push_back(seq.PartitionNumber);
    }
    std::sort(seq_map_keys.begin(), seq_map_keys.end());

    UINT64 current_sector_address = 0ULL;
    UINT64 partition_length_sum = 0ULL;
    int index = 0;
    for(std::vector<int>::iterator itr = seq_map_keys.begin(), itr_end = seq_map_keys.end(); itr != itr_end; itr++)
    {
        if(current_sector_address < seq_map[*itr].StartSector)
        {
            PartitionSequence ps = PartitionSequenceUtility::CreateInvalidSequence(GetSectorSize(), disk_number, current_sector_address, seq_map[*itr].StartSector - 1);
            ps.Index = index;
            index++;
            ps.PartitionLengthSum = partition_length_sum;
            part_seqs.push_back(ps);
            partition_length_sum += ps.PartitionLength;
        }
        PartitionSequence ps = seq_map[*itr];
        ps.Index = index;
        index++;
        ps.PartitionLengthSum = partition_length_sum;
        part_seqs.push_back(ps);
        partition_length_sum += ps.PartitionLength;
        current_sector_address = ps.EndSector;
        current_sector_address++;
    }

    UINT64 last_sector_address = GetDiskSize();
    last_sector_address /= GetSectorSize();
    last_sector_address--;
    if(current_sector_address < last_sector_address)
    {
        PartitionSequence ps = PartitionSequenceUtility::CreateInvalidSequence(GetSectorSize(), disk_number, current_sector_address, last_sector_address);
        ps.Index = index;
        index++;
        ps.PartitionLengthSum = partition_length_sum;
        part_seqs.push_back(ps);
        partition_length_sum += ps.PartitionLength;
    }

}

PhysicalVSS::~PhysicalVSS()
{
    if(handle_current != NULL && handle_current != INVALID_HANDLE_VALUE && handle_current != handle)
    {
        CloseHandle(handle_current);
        handle_current = NULL;
    }
}

bool PhysicalVSS::open_partition(const PartitionSequence& seq)
{
    if(!seq.invalid && seq.PartitionNumber == partition_current) return true;

    if(handle_current != NULL && handle_current != INVALID_HANDLE_VALUE && handle_current != handle)
        CloseHandle(handle_current);
    handle_current = NULL;

    if(seq.invalid)
    {
        handle_current = handle;
        partition_current = 0;
        return true;
    }

    tostringstream oss;
    if(seq.Snapshot)
    {
        oss << seq.SnapshotDeviceObject;
    }
    else
    {
        oss << _T("\\\\?\\GLOBALROOT\\Device\\Harddisk") << disk_number << _T("\\Partition") << seq.PartitionNumber;
    }

    handle_current = CreateFile(oss.str().c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if(handle_current != INVALID_HANDLE_VALUE)
    {
        DWORD BytesRead;
        DeviceIoControl(handle_current, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, NULL, 0, &BytesRead, NULL);
    }

    partition_current = (handle_current == INVALID_HANDLE_VALUE) ? -1 : seq.PartitionNumber;
    return (handle_current != INVALID_HANDLE_VALUE);

}

BOOL PhysicalVSS::GetBlockData(unsigned char* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    guard();

    DWORD blockcount = GetTableEntriesCount();
    if(blockindex < 0 || blockindex >= blockcount) return FALSE;

    DWORD blocksize = GetBlockSize();
    UINT64 start_byte = ((UINT64)blocksize) * blockindex;
    UINT64 start_sector = start_byte / ((UINT64) GetSectorSize());

    UINT64 end_byte = ((UINT64)blocksize) * (blockindex + 1);
    UINT64 end_sector = end_byte / ((UINT64) GetSectorSize());
    end_sector--;
    end_byte--;

    PartitionSequence part_seq_start = PartitionSequenceUtility::GetPartitionSequence(part_seqs, start_sector);
    PartitionSequence part_seq_end = PartitionSequenceUtility::GetPartitionSequence(part_seqs, end_sector);

    if(part_seq_start.Index == part_seq_end.Index && part_seq_start.invalid)
        return Physical::GetBlockData(blockdata, blockindex, ByteRead, can_skip);

    if(part_seq_start.Index == part_seq_end.Index)
    {
        *can_skip = skip(part_seq_start, start_sector, end_sector);
        if(*can_skip)
        {
            ZeroMemory(blockdata, blocksize);
            *ByteRead = blocksize;
            return TRUE;
        }

        PartitionSequence seq = part_seqs[part_seq_start.Index];
        if(!open_partition(seq)) return FALSE;

        LARGE_INTEGER filepointer;
        filepointer.QuadPart = (seq.PartitionNumber == 0) ? start_byte : start_byte - seq.PartitionLengthSum;
        SetFilePointerEx(handle_current, filepointer, NULL, FILE_BEGIN);
        DWORD byte_read;
        if(!ReadFile(handle_current, blockdata, blocksize, &byte_read, 0))
            return FALSE;
        if(byte_read != blocksize) return FALSE;
        *ByteRead = byte_read;
        return TRUE;
    }

    // need to get block data across partitions
    *can_skip = false;
    *ByteRead = 0;
    int partition_current_index = part_seq_start.Index;

    do
    {
        PartitionSequence seq = part_seqs[partition_current_index];
        if(!open_partition(seq)) return FALSE;

        UINT64 offset = (partition_current_index == part_seq_start.Index) ? start_byte - seq.PartitionLengthSum 
                                                                          : 0;
        DWORD byte_to_read = (partition_current_index == part_seq_end.Index) ? (DWORD)(end_byte + 1 - seq.PartitionLengthSum) 
                                                                             : (DWORD)(seq.PartitionLength - offset);
        LARGE_INTEGER filepointer;
        filepointer.QuadPart = (seq.PartitionNumber == 0) ? seq.PartitionLengthSum + offset : offset;
        SetFilePointerEx(handle_current, filepointer, NULL, FILE_BEGIN);
        DWORD byte_read;
        if(!ReadFile(handle_current, blockdata, byte_to_read, &byte_read, 0))
            return FALSE;
        if(byte_to_read != byte_read) return FALSE;
        (*ByteRead) += byte_read;
        blockdata += byte_read;

        partition_current_index++;

    } while(partition_current_index <= part_seq_end.Index);

    return TRUE;
}

void PhysicalVSS::CreateEmptyClusterRanges()
{
    if(!empty_cluster_ranges.cluster_ranges.empty()) return;

    shared_ptr<PhysicalDiskInfo> pdi;
    if(vss_created)
    {
        shared_ptr<VirtualDisk> vdisk = GetPtr();
        pdi = ForensicAnalysis::CreateDiskInfo(vdisk);
    }
    else
    {
        pdi = di->PhysicalDisks[disk_number];
    }

    for(unsigned int i=0; i<pdi->PartitionCount; i++)
    {
        int partition_number = pdi->Partitions[i].PartitionNumber;
        VolumeInfo vi = PhysicalDiskUtil::FindVolumeInfo(pdi, partition_number);
        if(vi.Invalid) continue;
        FS_TYPE FileSystemType = vi.FilesystemInfo.TypeInfo.FileSystemType;
        if(FileSystemType == FS_TYPE_NTFS || 
           FileSystemType == FS_TYPE_FAT32 ||
           FileSystemType == FS_TYPE_FAT16 ||
           FileSystemType == FS_TYPE_FAT12)
        {
            empty_cluster_ranges.cluster_ranges[partition_number] = vi.FilesystemInfo.SizeInfo.EmptyClusterRanges;
            empty_cluster_ranges.SectorsPerClusters[partition_number] = vi.FilesystemInfo.SizeInfo.SectorsPerCluster;
        }
    }

}

bool PhysicalVSS::skip(const PartitionSequence& seq, UINT64 start_sector, UINT64 end_sector)
{
    if(seq.invalid) return false;
    if(empty_cluster_ranges.SectorsPerClusters.find(seq.PartitionNumber) == empty_cluster_ranges.SectorsPerClusters.end()) return false;

    UINT8 sectors_per_cluster = empty_cluster_ranges.SectorsPerClusters[seq.PartitionNumber];
    if(sectors_per_cluster == 0) return false;
    UINT64 start_cluster = start_sector - seq.StartSector;
    start_cluster /= sectors_per_cluster;

    UINT64 end_cluster = end_sector - seq.StartSector;
    end_cluster /= sectors_per_cluster;

    std::vector<CLUSTER_RANGE> crs = empty_cluster_ranges.cluster_ranges[seq.PartitionNumber];
    for(std::vector<CLUSTER_RANGE>::iterator itr = crs.begin(), itr_end = crs.end(); itr != itr_end; itr++)
    {
        if(start_cluster < itr->AddressFrom) continue;
        if(start_cluster > itr->AddressTo) continue;
        if(end_cluster < itr->AddressFrom) continue;
        if(end_cluster > itr->AddressTo) continue;
        return true;
    }
    return false;
}
