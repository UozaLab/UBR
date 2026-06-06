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

#ifndef __VDISK_VSS__H__
#define __VDISK_VSS__H__

#include <windows.h>
#include <vector>
#include "tstring.h"
#include "VDisk.h"
#include "VDisk_physical.h"
#include "VSS/Vss.h"
#include "DiskInfo.h"
#include "smart_ptr.h"

struct PartitionSequence
{
    bool invalid;
    bool Snapshot;
    DWORD DeviceNumber;// DiskId
    DWORD PartitionNumber;
    tstring OriginalVolumeName;
    tstring SnapshotDeviceObject;
    UINT64 StartSector;
    UINT64 EndSector;
	UINT64 PartitionLength;
	UINT64 PartitionLengthSum;
    int Index;
};

class PartitionSequenceUtility
{
public:
    static PartitionSequence CreateInvalidSequence(DWORD bytes_per_sector, DWORD disk_id, UINT64 start_sector, UINT64 end_sector);
    static PartitionSequence GetPartitionSequence(const std::vector<PartitionSequence>& part_seqs, UINT64 address_sector);
};

struct EmptyClusterRanges
{
    std::map<int, std::vector<CLUSTER_RANGE>> cluster_ranges;
    std::map<int, UINT8> SectorsPerClusters;
};

class PhysicalVSS : public Physical, public enable_shared_from_this<PhysicalVSS>
{
protected:
    shared_ptr<DiskInfo> di;
    bool vss_created;
    Vss vss;
    std::vector<PartitionSequence> part_seqs;
    HANDLE handle_current;
    int partition_current;
    EmptyClusterRanges empty_cluster_ranges;

    bool open_partition(const PartitionSequence& seq);
    bool skip(const PartitionSequence& seq, UINT64 start_sector, UINT64 end_sector);
public:
    PhysicalVSS(int _disk_number, shared_ptr<DiskInfo> _di);
    ~PhysicalVSS();

    virtual tstring GetFileFormat()
    {
        return _T("VSS");
    }
    virtual BOOL GetBlockData(unsigned char* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);
    bool IsVssSnapshotCreated() { return vss_created; }
    void CreateEmptyClusterRanges();
    shared_ptr<PhysicalVSS> GetPtr() { return shared_from_this(); }
};

#endif
