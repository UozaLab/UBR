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

#ifndef __DISK_INFO_H__
#define __DISK_INFO_H__

#include <windows.h>
#include <vector>
#include <map>
#include "tstring.h"
#include "smart_ptr.h"
#include "PhysicalDiskInfo.h"
#include "VolumeInfo.h"

struct DiskInfo
{
    // map<DiskNumber, PhysicalDisk*>
    std::map<DWORD, shared_ptr<PhysicalDiskInfo>> PhysicalDisks;
};

class DiskInfoFactory
{
protected:
    // map<DiskNumber, map<PartitionNumber, VolumeInfo>>
    static shared_ptr<std::map<DWORD, std::map<DWORD, VolumeInfo>>> get_volume_infos();
    static std::vector<DWORD> get_disk_numbers();
public:
    static shared_ptr<DiskInfo> CreateDiskInfo();
};

#endif