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

#ifndef __PHYSICAL_DISK_INFO__H__
#define __PHYSICAL_DISK_INFO__H__

#include <windows.h>
#include <vector>
#include "smart_ptr.h"
#include "VolumeInfo.h"

struct PhysicalDiskInfo
{
    bool Invalid;
    DWORD DeviceNumber;

    LONGLONG DiskSize;
    LONGLONG Cylinders;
    DWORD TracksPerCylinder;
    DWORD SectorsPerTrack;
    DWORD BytesPerSector;

    DWORD PartitionStyle;// PARTITION_STYLE_MBR/PARTITION_STYLE_GPT/PARTITION_STYLE_RAW
    DWORD PartitionCount;
    union {
        DRIVE_LAYOUT_INFORMATION_MBR Mbr;
        DRIVE_LAYOUT_INFORMATION_GPT Gpt;
    } DUMMYUNIONNAME;
    
    std::vector<PARTITION_INFORMATION_EX> Partitions;
    std::vector<VolumeInfo> Volumes;

};

class PhysicalDiskUtil
{
public:
    static VolumeInfo FindVolumeInfo(shared_ptr<PhysicalDiskInfo> pdi, DWORD PartitionNumber);
    static void UpdateVolumeInfo(shared_ptr<PhysicalDiskInfo> pdi, VolumeInfo volume_info);
    static bool LockAndDismount(shared_ptr<PhysicalDiskInfo> pdi, std::vector<HANDLE>& handles_opened);
    static void UnLock(std::vector<HANDLE>& handles_opened);
};

#endif
