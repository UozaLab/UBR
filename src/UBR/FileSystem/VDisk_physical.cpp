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

#include <windows.h>
#include "VDisk_physical.h"

BOOL Physical::IsValid()
{
    if(!physical) return FALSE;
    guard();
    if(physical_disk.Invalid) return FALSE;

    return TRUE;
}

void Physical::guard()
{
    if(!opened)
    {
        if(!open())
        {
            physical_disk.Invalid = true;
            return;
        }

        UINT8 out_buffer[sizeof(DISK_GEOMETRY_EX)];
        DWORD bytesReturned;
        if(!DeviceIoControl(handle,
                            IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,
                            NULL,
                            0,
                            &out_buffer,
                            sizeof(DISK_GEOMETRY_EX),
                            &bytesReturned,
                            NULL))
        {
            physical_disk.Invalid = true;
            return;
        }
        PDISK_GEOMETRY_EX geometry = (PDISK_GEOMETRY_EX)&out_buffer[0];
        physical_disk.DiskSize = geometry->DiskSize.QuadPart;
        physical_disk.Cylinders = geometry->Geometry.Cylinders.QuadPart;
        physical_disk.TracksPerCylinder = geometry->Geometry.TracksPerCylinder;
        physical_disk.SectorsPerTrack = geometry->Geometry.SectorsPerTrack;
        physical_disk.BytesPerSector = geometry->Geometry.BytesPerSector;
        physical_disk.Invalid = false;
    }
}

ULONGLONG Physical::GetDiskSize()
{
    guard();
    return physical_disk.DiskSize;
}

DWORD Physical::GetSectorSize()
{
    guard();
    return physical_disk.BytesPerSector;
}

DWORD Physical::GetBlockSize()
{
    guard();
    ULONGLONG disksize = GetDiskSize();

    if(large_scale_mode == false && disksize >= 1048576)
        return 1048576;// 1MB

    unsigned int power = 0;
    while(disksize > 0)
    {
        disksize /= 2;
        power++;
    }
    power--;
    return 1U << ((power < 27) ? power : 27);// 2^27=128MB
}

DWORD Physical::GetTableEntriesCount()
{
    guard();
    DWORD blocksize = GetBlockSize();
    return (DWORD) ((GetDiskSize() + blocksize - 1)/blocksize);
}

UINT32 Physical::GetDiskType()
{
    return 2;// Fixed hard disk
}

BOOL Physical::GetBlockData(unsigned char* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    guard();
    *can_skip = false;
    DWORD blockcount = GetTableEntriesCount();
    if(blockindex < 0 || blockindex >= blockcount)
        return FALSE;
    DWORD blocksize = GetBlockSize();

    LARGE_INTEGER filepointer;
    filepointer.QuadPart = ((UINT64)blocksize) * blockindex;
    SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
    if(!ReadFile(handle, blockdata, blocksize, ByteRead, 0))
        return FALSE;
    return TRUE;
}
