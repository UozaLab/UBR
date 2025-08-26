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

#ifndef __VDISK_PHYSICAL__H__
#define __VDISK_PHYSICAL__H__

#include <windows.h>
#include "tstring.h"
#include "VDisk.h"
#include "PhysicalDiskInfo.h"

class Physical : public VirtualDisk
{
protected:
    virtual void guard();
    PhysicalDiskInfo physical_disk;

public:
    Physical(int disk_number)
        :VirtualDisk(disk_number)
    {
        physical_disk.DeviceNumber = disk_number;
    }

    virtual tstring GetFileFormat()
    {
        return _T("PHYSICAL");
    }
    virtual BOOL IsValid();

    virtual ULONGLONG GetDiskSize();
    virtual DWORD GetSectorSize();
    virtual DWORD GetBlockSize();
    virtual DWORD GetTableEntriesCount();
    virtual UINT32 GetDiskType();
    virtual BOOL GetBlockData(unsigned char* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);
};

#endif
