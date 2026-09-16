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
#include <time.h>


VHD::VHD(const TCHAR* _filename, UINT32 blocksize, UINT64 virtual_disksize)
     : VHDCommon(_filename, true), bat(NULL), bat_count(0)
{
    create_new_disk(blocksize, virtual_disksize);
}

VHD::VHD(const TCHAR* _filename)
     : VHDCommon(_filename), bat(NULL), bat_count(0)
{
    ZeroMemory(&footer, sizeof(VHD_FOOTER));
	ZeroMemory(&header, sizeof(VHD_DYNAMIC_DISK_HEADER));
}

VHD::~VHD()
{
    if(create_new && !flushed)
        Flush();

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
    if(create_new) return true;
    if(invalid) return false;

	DWORD dwByteRead = 0;
    if(!read(0, (UINT8*)&footer, sizeof(VHD_FOOTER), &dwByteRead))
      return false;
    if(sizeof(VHD_FOOTER) != dwByteRead) return false;

    const char* cookie="conectix";
    if(memcmp(footer.Cookie, cookie, strlen(cookie)) != 0)
    {
        if(!GetSlow())
        {
            if(!read(-512, (UINT8*)&footer, sizeof(VHD_FOOTER), &dwByteRead, false))
              return false;
            if(sizeof(VHD_FOOTER) != dwByteRead) return false;
            if(memcmp(footer.Cookie, cookie, strlen(cookie)) != 0)
              return false;
        }
    }

    UINT32 disktype = _byteswap_ulong(footer.DiskType);
    if(disktype == 2) return true;// 2:fixed hard disk
    if(disktype != 3) return false;// 3:dynamic hard disk

    //
    // read header
    //
    if(!read(512, (UINT8*)&header, sizeof(VHD_DYNAMIC_DISK_HEADER), &dwByteRead))
      return false;
    if(sizeof(VHD_DYNAMIC_DISK_HEADER) != dwByteRead) return false;
    const char* cookie_header="cxsparse";
    if(memcmp(header.Cookie, cookie_header, strlen(cookie_header)) != 0)
        return false;

    // bat
    bat_count = _byteswap_ulong(header.MaxTableEntries);
    if(bat != NULL) delete [] bat;
    bat = new UINT32[bat_count];
    if(!read(_byteswap_uint64(header.TableOffset), (UINT8*)bat, sizeof(UINT32)*bat_count, &dwByteRead))
      return false;
    if(sizeof(UINT32)*bat_count != dwByteRead) return false;

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
        if(!read(((INT64)blocksize) * blockindex, (UINT8*)blockdata, blocksize, ByteRead))
          return FALSE;
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

    if(!read((bat_indicate_offset_sector + block_bitmap_sector_count) * sector_size, (UINT8*)blockdata, blocksize, ByteRead))
      return FALSE;
    return TRUE;
}

BOOL VHD::SetBlockDataImpl(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite)
{
    if(GetDiskType() != 3) return FALSE;
    if(bat == NULL) return FALSE;
    flushed = false;

    DWORD blockcount = GetTableEntriesCount();
    if(blockindex >= blockcount) return FALSE;
    DWORD blocksize = GetBlockSize();

    UINT64 bat_indicate_offset_sector = 0;
    DWORD sector_size = GetSectorSize();
    const DWORD sector_counts_in_1byte_bitmap = 8;
    UINT32 block_bitmap_sector_count = (blocksize /sector_size) / sector_counts_in_1byte_bitmap;
    block_bitmap_sector_count = (block_bitmap_sector_count + (sector_size - 1)) / sector_size;

    if(_byteswap_ulong(bat[blockindex]) == 0xFFFFFFFF)
    {
        UINT64 batsize = (UINT64)bat_count * sizeof(UINT32);
        unsigned int header_footer_length = sizeof(VHD_FOOTER) + sizeof(VHD_DYNAMIC_DISK_HEADER) + (unsigned int) batsize;
        bat_indicate_offset_sector = (header_footer_length + (UINT64)write_block_count * (blocksize + block_bitmap_sector_count * sector_size)) / sector_size;
        bat[blockindex] = _byteswap_ulong((UINT32) bat_indicate_offset_sector);
        write_block_count++;
    }
    else
    {
        bat_indicate_offset_sector = _byteswap_ulong(bat[blockindex]);
    }

    unsigned int bitmap_size = block_bitmap_sector_count * sector_size;
    UINT8* bitmap = new UINT8[bitmap_size];
    memset(bitmap, 0xFF, bitmap_size);
    DWORD ByteWriteBitmap;
    BOOL result = write(bat_indicate_offset_sector * sector_size, bitmap, bitmap_size, &ByteWriteBitmap);
    delete [] bitmap;
    if(result == 0) return FALSE;

    result = write(bat_indicate_offset_sector * sector_size + bitmap_size, blockdata, blocksize, ByteWrite);
    return result != 0;
}

VHD_GEOMETRY sectors2chs(UINT64 total_sectors)
{
    VHD_GEOMETRY result;
    UINT64 cylinder_times_heads = 0;

    if(total_sectors > 65536 * 16 * 255)
        total_sectors = 65536 * 16 * 255;

    if(total_sectors > 65536 * 16 * 63)
    {
        result.SectorsPerTrack = 255;
        result.Heads = 16;
        cylinder_times_heads = total_sectors / 255;
    }
    else
    {
        result.SectorsPerTrack = 17;
        cylinder_times_heads = total_sectors / 17;
        result.Heads = (UINT8) ((cylinder_times_heads + 1023) / 1024);
        if(result.Heads < 4)
        {
            result.Heads = 4;
        }
        if(cylinder_times_heads >= ((UINT64)result.Heads * 1024) || result.Heads > 16)
        {
            result.SectorsPerTrack = 31;
            result.Heads = 16;
            cylinder_times_heads = total_sectors / 31;
        }
        if(cylinder_times_heads >= ((UINT64)result.Heads * 1024))
        {
            result.SectorsPerTrack = 63;
            result.Heads = 16;
            cylinder_times_heads = total_sectors / 63;
        }
    }
    result.Cylinder = (UINT16) (cylinder_times_heads / result.Heads);
    return result;
}

void VHD::create_new_disk(UINT32 blocksize, UINT64 virtual_disksize)
{
    // footer
    ZeroMemory(&footer, sizeof(VHD_FOOTER));
    const char* cookie = "conectix";
    memcpy(&footer.Cookie, cookie, strlen(cookie));
    footer.Features = _byteswap_ulong(0x00000002);
    footer.FileFormatVersion = _byteswap_ulong(0x00010000);
    footer.DataOffset = _byteswap_uint64(sizeof(VHD_FOOTER));
    footer.OriginalSize = _byteswap_uint64(virtual_disksize);
    footer.CurrentSize = _byteswap_uint64(virtual_disksize);
    footer.DiskType = _byteswap_ulong(3);/*dynamic*/
    footer.CreatorHostOS[0] = 0x57;
    footer.CreatorHostOS[1] = 0x69;
    footer.CreatorHostOS[2] = 0x32;
    footer.CreatorHostOS[3] = 0x6B;
    GUID guid;
    CoCreateGuid(&guid);
    guid.Data1 = _byteswap_ulong(guid.Data1);
    guid.Data2 = _byteswap_ushort(guid.Data2);
    guid.Data3 = _byteswap_ushort(guid.Data3);
    UINT64 data4 = *((UINT64*)&guid.Data4);
    memcpy(&guid.Data4, (const void*) &data4, 8);
    memcpy(&footer.UniqueId, &guid, 16);
    VHD_GEOMETRY chs = sectors2chs(virtual_disksize / 512);
    chs.Cylinder = _byteswap_ushort(chs.Cylinder);
    footer.DiskGeometry = chs;
    time_t t = time(NULL);
    footer.TimeStamp = _byteswap_ulong((UINT32) (t - 946684800ULL));

    // header
    ZeroMemory(&header, sizeof(VHD_DYNAMIC_DISK_HEADER));
    const char* cookie_header = "cxsparse";
    memcpy(&header.Cookie, cookie_header, strlen(cookie_header));
    header.DataOffset = ~((UINT64)0);
    header.TableOffset = _byteswap_uint64(sizeof(VHD_FOOTER) + sizeof(VHD_DYNAMIC_DISK_HEADER));
    header.MaxTableEntries = _byteswap_ulong((UINT32) ((virtual_disksize + (blocksize - 1)) / blocksize));
    header.BlockSize = _byteswap_ulong(blocksize);
    header.HeaderVersion = _byteswap_ulong(0x00010000);

    // BAT
    DWORD batcount = _byteswap_ulong(header.MaxTableEntries);
    UINT64 batsize = (UINT64)batcount * sizeof(UINT32);
    if(batsize % 512) batsize += (512 - batsize % 512);
    bat_count = (unsigned int) (batsize / sizeof(UINT32));
    bat = new UINT32[bat_count];
    memset(bat, 0xFF, (size_t) batsize);
}

UINT32 VHD::checksum(UINT8*buf, int size)
{
    UINT32 sum = 0;
    for(int i = 0; i<size; i++)
        sum += buf[i];
    return ~sum;
}

bool VHD::write_header()
{
    UINT64 batsize = (UINT64)bat_count * sizeof(UINT32);
    unsigned int buf_length = sizeof(VHD_FOOTER) + sizeof(VHD_DYNAMIC_DISK_HEADER) + (unsigned int) batsize;
    UINT8* buf = new UINT8[buf_length];
    ZeroMemory(buf, buf_length);

    ZeroMemory(((UINT8*) &footer)+64, 4);// clear checksum
    ZeroMemory(((UINT8*) &header)+36, 4);// clear checksum
    footer.Checksum = _byteswap_ulong(checksum((UINT8*)&footer, sizeof(VHD_FOOTER)));
    header.Checksum = _byteswap_ulong(checksum((UINT8*)&header, sizeof(VHD_DYNAMIC_DISK_HEADER)));

    unsigned int index = 0;
    memcpy(buf + index, &footer, sizeof(VHD_FOOTER));
    index += sizeof(VHD_FOOTER);
    memcpy(buf + index, &header, sizeof(VHD_DYNAMIC_DISK_HEADER));
    index += sizeof(VHD_DYNAMIC_DISK_HEADER);
    memcpy(buf + index, bat, (size_t) batsize);
    index += (unsigned int) batsize;

    DWORD dwNumberOfWritten = 0;
    BOOL result = write(0, buf, buf_length, &dwNumberOfWritten);
    delete [] buf;

    return result != 0;
}

bool VHD::write_footer()
{
    DWORD dwNumberOfWritten = 0;
    UINT64 batsize = (UINT64)bat_count * sizeof(UINT32);
    unsigned int buf_length = sizeof(VHD_FOOTER) + sizeof(VHD_DYNAMIC_DISK_HEADER) + (unsigned int) batsize;
    
    DWORD sector_size = GetSectorSize();
    DWORD blocksize = GetBlockSize();
    const DWORD sector_counts_in_1byte_bitmap = 8;
    UINT32 block_bitmap_sector_count = (blocksize /sector_size) / sector_counts_in_1byte_bitmap;
    block_bitmap_sector_count = (block_bitmap_sector_count + (sector_size - 1)) / sector_size;
    
    UINT64 data_block_size = write_block_count;
    data_block_size *= (GetBlockSize() + block_bitmap_sector_count * sector_size);
    UINT8* buf = new UINT8[sizeof(VHD_FOOTER)];
    memcpy(buf, &footer, sizeof(VHD_FOOTER));
    BOOL result = write(data_block_size + buf_length, buf, sizeof(VHD_FOOTER), &dwNumberOfWritten);
    delete [] buf;

    return result != 0;
}

