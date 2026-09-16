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

RAW::RAW(const tstring& _filename, UINT32 _block_size, UINT64 _disk_size)
     : VirtualDisk(_filename, true), invalid(false), block_size(_block_size), disk_size(_disk_size)
{
    guard();
    if(invalid) return;
    if(stream_mode) return;
#ifdef ENLARGE_FILE
    if(!privilege.IsValid()) return;
    privilege.SetPrivilege(SE_MANAGE_VOLUME_NAME , true);

    LARGE_INTEGER pointer;
    pointer.QuadPart = disk_size;
    SetFilePointerEx(handle, pointer, NULL, FILE_BEGIN);
    SetEndOfFile(handle);
    SetFileValidData(handle, disk_size);
#endif
}

RAW::~RAW()
{
    if(stream_mode) return;
#ifdef ENLARGE_FILE
    privilege.SetPrivilege(SE_MANAGE_VOLUME_NAME , false);
#endif
}

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
    if(create_new) return disk_size;
    if(stream_mode) return disk_size;

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
    if(create_new) return block_size;
    if(stream_mode)
    {
        if(!large_scale_mode) return 1048576; // 1MB
        return 134217728; // 128MB
    }
    
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

BOOL RAW::SetBlockData(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite)
{
    guard();
    if(invalid) return FALSE;

    DWORD blockcount = GetTableEntriesCount();
    if(blockindex < 0 || blockindex >= blockcount)
        return FALSE;
    DWORD blocksize = GetBlockSize();

    LARGE_INTEGER filepointer;
    filepointer.QuadPart = ((UINT64)blocksize) * blockindex;
    SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);

    ULONGLONG disksize = GetDiskSize();
    if(filepointer.QuadPart + blocksize >  disksize)
    {
        blocksize -= (DWORD) (filepointer.QuadPart + blocksize - disksize);
    }

    if(!WriteFile(handle, blockdata, blocksize, ByteWrite, NULL))
        return FALSE;

    return TRUE;
}

