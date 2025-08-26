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

#ifndef __FS__H__
#define __FS__H__

#include <windows.h>
#include "smart_ptr.h"
#include "tstring.h"
#include "VDiskStream.h"
#include "FsInfo.h"
#include "MbrGpt.h"

class FileSystem
{
protected:
    shared_ptr<VirtualDiskStream> stream;
    VOLUME_TYPE_INFO create_volume_type_info(UINT64 partition_start_sector);

public:
    static tstring PartitionTypeToString(UINT8 partition_type);
    static tstring FSTypeToString(FS_TYPE fs_type);
    FileSystem(shared_ptr<VirtualDiskStream> _stream);
    FSInfo GetFSInfo(UINT64 partition_start_sector);
    shared_ptr<DiskUpdaterCollection> GetShrinkedSectorData(shared_ptr<MBRGPT> mbrgpt);
};

#endif
