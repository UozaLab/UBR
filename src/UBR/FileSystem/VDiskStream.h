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

#ifndef __VDISK_STREAM__
#define __VDISK_STREAM__

#include "smart_ptr.h"
#include "VDisk.h"

class VirtualDiskStream
{
  protected:
    shared_ptr<VirtualDisk> vdisk;
    UINT8* cache;
    DWORD cache_block_index;

  public:
    VirtualDiskStream(shared_ptr<VirtualDisk> _vdisk);
    ~VirtualDiskStream();
    UINT8* Read(UINT64 sector, UINT32 sector_count);
    UINT8* ReadRange(UINT64 sector, UINT32 least_byte_size);
    bool Write(UINT8* data, UINT64 sector, UINT32 sector_count);
};


#endif
