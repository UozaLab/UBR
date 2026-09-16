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

#include "VDiskStream.h"
#include <algorithm>

VirtualDiskStream::VirtualDiskStream(shared_ptr<VirtualDisk> _vdisk):
vdisk(_vdisk), cache(NULL), cache_block_index(0)
{
}

VirtualDiskStream::~VirtualDiskStream()
{
    if(cache != NULL)
        delete [] cache;
}

UINT8* VirtualDiskStream::ReadRange(UINT64 sector, UINT32 least_byte_size)
{
    DWORD sector_size = vdisk->GetSectorSize();
    UINT32 sector_count = (least_byte_size == 0) ? 1 :
                          ((least_byte_size - 1) / sector_size) + 1;
    return Read(sector, sector_count);
}

UINT8* VirtualDiskStream::Read(UINT64 sector, UINT32 sector_count)
{
    DWORD sector_size = vdisk->GetSectorSize();
    UINT32 ret_size = sector_size * sector_count;
    UINT8* ret = new UINT8[ret_size];

    DWORD block_size = vdisk->GetBlockSize();//byte
    DWORD sector_per_block = block_size / sector_size;
    DWORD block_index = (DWORD) (sector / sector_per_block);
    DWORD block_index_to = (DWORD) ((sector + sector_count - 1) / sector_per_block);

    if(cache != NULL && block_index == cache_block_index && block_index_to == cache_block_index)
    {
        DWORD ofs = sector % sector_per_block;
        ofs *= sector_size;
        UINT8* buff = cache;
        ULONG copy_size = sector_count * sector_size;
        memcpy(ret, buff + ofs, copy_size);
    }
    else
    {
        if(cache != NULL)
        {
            delete []  cache;
            cache = NULL;
        }

        UINT8* ret_point = ret;
        for(DWORD i = 0; i < block_index_to - block_index + 1; i++)
        {
            DWORD ofs = 0;
            if(i == 0)
            {
                ofs = sector % sector_per_block;
                ofs *= sector_size;
            }
            UINT8* buff = new UINT8[block_size];
            DWORD ByteRead;
            bool can_skip;
            BOOL result = vdisk->GetBlockData(buff, block_index + i, &ByteRead, &can_skip);
            ULONG copy_size = (std::min)((DWORD)(ret + ret_size - ret_point), block_size-ofs);
            memcpy(ret_point, buff + ofs, copy_size);
            if(i == 0)
            {
                cache = buff;
                cache_block_index = block_index;
            }
            else
                delete [] buff;

            ret_point += copy_size;
        }

    }

    return ret;
}

bool VirtualDiskStream::Write(UINT8* data, UINT64 sector, UINT32 sector_count)
{
    DWORD sector_size = vdisk->GetSectorSize();
    UINT32 data_size = sector_size * sector_count;

    DWORD block_size = vdisk->GetBlockSize();//byte
    DWORD sector_per_block = block_size / sector_size;
    DWORD block_index = (DWORD) (sector / sector_per_block);
    DWORD block_index_to = (DWORD) ((sector + sector_count - 1) / sector_per_block);

    bool block_index_has_remainder = sector % sector_per_block != 0;
    bool block_index_to_has_remainder = (sector + sector_count) % sector_per_block != 0;

    for(DWORD i = 0; i < block_index_to - block_index + 1; i++)
    {
        UINT8* buff = data + i * block_size;

        bool buff_created = false;
        if(i == 0 && block_index_has_remainder ||
           i == block_index_to - block_index && block_index_to_has_remainder) 
        {
            DWORD ByteRead;
            bool can_skip;
            buff = new UINT8[block_size];
            if(!vdisk->GetBlockData(buff, block_index, &ByteRead, &can_skip)) return false;
            if(ByteRead != block_size && !can_skip) return false;

            buff_created = true;
        }
        bool damaged = false;
        if(i == 0 && block_index_has_remainder)
        {
            UINT32 data_pre_size = sector % sector_per_block;
            data_pre_size *= sector_size;
            memcpy(buff + data_pre_size, data, (std::min)(block_size - data_pre_size, sector_count * sector_size));
            damaged = true;
        }
        if(i == block_index_to - block_index && block_index_to_has_remainder && !damaged)
        {
            UINT32 data_post_size = (sector + sector_count) % sector_per_block;
            data_post_size *= sector_size;
            memcpy(buff, data, data_post_size);
        }

        DWORD ByteWrite;
        BOOL result = vdisk->SetBlockData(buff, block_index + i, &ByteWrite);

        if(buff_created) delete [] buff;
        if(!result) return false;
    }
    return true;
}
