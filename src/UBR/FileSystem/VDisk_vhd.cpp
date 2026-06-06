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

#include "VDisk_vhd.h"
#include <algorithm>

VHD::VHD(const TCHAR* _filename)
     : VHDCommon(_filename), bat(NULL)
{
    ZeroMemory(&footer, sizeof(VHD_FOOTER));
	ZeroMemory(&header, sizeof(VHD_DYNAMIC_DISK_HEADER));
}

VHD::~VHD()
{
    if(bat != NULL)
        delete [] bat;
}

ULONGLONG VHD::GetDiskSizeImpl()
{
    if(invalid) return 0ULL;
    return _byteswap_uint64(footer.CurrentSize);
}

DWORD VHD::GetSectorSizeImpl()
{
    return 512;
}

DWORD VHD::GetBlockSizeImpl()
{
    if(invalid) return 0;

    if(GetDiskType() == 2)//fixed
    {
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

    return _byteswap_ulong(header.BlockSize);
}

DWORD VHD::GetTableEntriesCountImpl()
{
    if(invalid) return 0;

    if(GetDiskType() == 2)//fixed
    {
        DWORD blocksize = GetBlockSize();
        return (DWORD) ((GetDiskSize() + blocksize - 1)/blocksize);
    }
    return  _byteswap_ulong(header.MaxTableEntries);
}

UINT32 VHD::GetDiskTypeImpl()
{
    if(invalid) return 0;
    return _byteswap_ulong(footer.DiskType);
}

bool VHD::read_footer_header()
{
    if(invalid) return false;

	DWORD dwByteRead = 0;
	LARGE_INTEGER filepointer;
	filepointer.QuadPart = 0;
	SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
	if(!ReadFile(handle, &footer, sizeof(VHD_FOOTER), &dwByteRead, 0))
      return false;
    if(sizeof(VHD_FOOTER) != dwByteRead) return false;

    const char* cookie="conectix";
    if(memcmp(footer.Cookie, cookie, strlen(cookie)) != 0)
    {
        filepointer.QuadPart = -512;
        SetFilePointerEx(handle, filepointer, NULL, FILE_END);
        if(!ReadFile(handle, &footer, sizeof(VHD_FOOTER), &dwByteRead, 0))
            return false;
        if(sizeof(VHD_FOOTER) != dwByteRead) return false;
        if(memcmp(footer.Cookie, cookie, strlen(cookie)) != 0)
            return false;
    }

    UINT32 disktype = _byteswap_ulong(footer.DiskType);
    if(disktype == 2) return true;// 2:fixed hard disk
    if(disktype != 3) return false;// 3:dynamic hard disk

    //
    // read header
    //
	filepointer.QuadPart = 512;
	SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
	if(!ReadFile(handle, &header, sizeof(VHD_DYNAMIC_DISK_HEADER), &dwByteRead, 0))
      return false;
    if(sizeof(VHD_DYNAMIC_DISK_HEADER) != dwByteRead) return false;
    const char* cookie_header="cxsparse";
    if(memcmp(header.Cookie, cookie_header, strlen(cookie_header)) != 0)
        return false;

    // bat
    DWORD batcount = _byteswap_ulong(header.MaxTableEntries);
    if(bat != NULL) delete [] bat;
    bat = new UINT32[batcount];
	filepointer.QuadPart = _byteswap_uint64(header.TableOffset);
	SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
	if(!ReadFile(handle, bat, sizeof(UINT32)*batcount, &dwByteRead, 0)) return false;
    if(sizeof(UINT32)*batcount != dwByteRead) return false;

    return true;
}

BOOL VHD::GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    if(invalid) return FALSE;
    *ByteRead = 0;
    *can_skip = false;

    DWORD blockcount = GetTableEntriesCount();
    if(blockindex >= blockcount) return FALSE;
    DWORD blocksize = GetBlockSize();

    if(GetDiskType() == 2)
    {
        // fixed disk
        LARGE_INTEGER filepointer;
        filepointer.QuadPart = ((UINT64)blocksize) * blockindex;
        SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
        if(!ReadFile(handle, blockdata, blocksize, ByteRead, 0)) return FALSE;
        return TRUE;
    }

    // dynamic disk
    if(bat == NULL) return FALSE;
    if(_byteswap_ulong(bat[blockindex]) == 0xFFFFFFFF)
    {
        ZeroMemory(blockdata, blocksize);
        *ByteRead = blocksize;
        *can_skip = true;
        return TRUE;
    }

    DWORD sector_size = GetSectorSize(); 
    UINT64 bat_indicate_offset_sector = _byteswap_ulong(bat[blockindex]);
    const DWORD sector_counts_in_1byte_bitmap = 8;
    UINT32 block_bitmap_sector_count = (blocksize /sector_size) / sector_counts_in_1byte_bitmap;
    block_bitmap_sector_count = (block_bitmap_sector_count + (sector_size - 1)) / sector_size;

	LARGE_INTEGER filepointer;
    filepointer.QuadPart = (bat_indicate_offset_sector + block_bitmap_sector_count) * sector_size;
    SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
    if(!ReadFile(handle, blockdata, blocksize, ByteRead, 0))
        return FALSE;
    return TRUE;
}

