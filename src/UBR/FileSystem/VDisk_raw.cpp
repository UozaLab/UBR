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

#include "VDisk_raw.h"


BOOL RAW::IsValid()
{
    guard();
    if(invalid) return FALSE;

    return (GetDiskSize() %  GetSectorSize() == 0);
}

void RAW::guard()
{
    if(!opened)
    {
        if(!open()) invalid = TRUE;
    }
}

ULONGLONG RAW::GetDiskSize()
{
    guard();
    if(invalid) return 0ULL;

    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    LARGE_INTEGER raw_disk_size;

    GetFileAttributesEx( filename.c_str(), GetFileExInfoStandard, &FileInfo );
    raw_disk_size.HighPart = FileInfo.nFileSizeHigh;
    raw_disk_size.LowPart = FileInfo.nFileSizeLow;
    return raw_disk_size.QuadPart;
}

DWORD RAW::GetSectorSize()
{
    return 512;
}

DWORD RAW::GetBlockSize()
{
    guard();
    if(invalid) return 0;

    ULONGLONG disksize = GetDiskSize();

    if(large_scale_mode == false && disksize >= 1048576)
        return 1048576; // 1MB

    unsigned int power = 0;
    while(disksize > 0)
    {
        disksize /= 2;
        power++;
    }
    power--;
    return 1U << ((power < 27) ? power : 27);// 2^27=128MB
}

DWORD RAW::GetTableEntriesCount()
{
    guard();
    if(invalid) return 0;

    DWORD blocksize = GetBlockSize();
    return (DWORD) ((GetDiskSize() + blocksize - 1)/blocksize);
}

UINT32 RAW::GetDiskType()
{
    return 2;// Fixed hard disk
}

BOOL RAW::GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    guard();
    if(invalid) return FALSE;

    *can_skip = false;
    *ByteRead = 0;

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
