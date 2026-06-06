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

#include "Fs.h"
#include "Fs_ntfs.h"
#include "Fs_fat.h"

FileSystem::FileSystem(shared_ptr<VirtualDiskStream> _stream):
stream(_stream)
{
}

VOLUME_TYPE_INFO FileSystem::create_volume_type_info(UINT64 partition_start_sector)
{
    // Judge FileSystem
    // https://jdebp.uk/FGA/determining-filesystem-type.html
    UINT8* bpb = stream->ReadRange(partition_start_sector, 0x59);
    UINT8 partition_type = PART_TYPE_Empty;
    tstring partition_name;

    // judge bitlocked
    if(strncmp((char*) &bpb[0x03], "-FVE-FS-", 8) == 0)
    {
        VOLUME_TYPE_INFO type_info;
        type_info.PartitionType = PART_TYPE_BitLocker;
        type_info.FileSystemName = FileSystem::PartitionTypeToString(type_info.PartitionType);
        type_info.FileSystemType = FS_TYPE_UNKNOWN;

        return type_info;
    }

    // judge exFAT
    if(bpb[0] == 0xEB && bpb[1] == 0x76 && bpb[2] == 0x90)
    {
        if(strncmp((char*) &bpb[0x03], "EXFAT", 5) == 0)
        {
            bool damaged = false;
            for(unsigned int i=0x0B; i<=0x3F; i++)
            {
                if(bpb[i] != 0x00) damaged = true;
            }
            if(!damaged)
            {
                partition_type = PART_TYPE_NTFS_exFAT;
                partition_name = _T("exFAT");
            }
        }
    }

    // judge version 7.0 BPB (FAT32)
    unsigned char signature = bpb[0x42];
    if(signature == 0x28 || signature == 0x29)
    {
        // Inspect its filesystem type field at offsets 0x52-0x59
        bool damaged = false;
        for(unsigned int i=0x52; i<=0x59; i++)
        {
            if(!isprint(bpb[i])) damaged = true;
        }
        if(!damaged)
        {
            const char* filesystem_type = (char*) &bpb[0x52];
            partition_type = PART_TYPE_FAT32;
            if(strncmp(filesystem_type, "NTFS", 4) == 0)
            {
                partition_type = PART_TYPE_NTFS_exFAT;
                partition_name = _T("NTFS");
            }
            else if(strncmp(filesystem_type, "FAT32", 5) == 0)
            {
                partition_type = PART_TYPE_FAT32;
            }
            else if(strncmp(filesystem_type, "FAT12", 5) == 0)
            {
                partition_type = PART_TYPE_FAT12;
            }
            else if(strncmp(filesystem_type, "FAT16", 5) == 0)
            {
                partition_type = PART_TYPE_FAT16;
            }
            else if(strncmp(filesystem_type, "FAT", 3) == 0)
            {
                partition_type = PART_TYPE_FAT32;// ??
            }

        }
    }

    // judge version 4.0 BPB (FAT12/16)
    signature = bpb[0x26];
    if(signature == 0x28 || signature == 0x29)
    {
        // Inspect its filesystem type field at offsets 0x36-0x3D
        bool damaged = false;
        for(unsigned int i=0x36; i<=0x3D; i++)
        {
            if(!isprint(bpb[i])) damaged = true;
        }
        if(!damaged)
        {
            const char* filesystem_type = (char*) &bpb[0x36];
            partition_type = PART_TYPE_FAT16;
            if(strncmp(filesystem_type, "NTFS", 4) == 0)
            {
                partition_type = PART_TYPE_NTFS_exFAT;
                partition_name = _T("NTFS");
            }
            else if(strncmp(filesystem_type, "FAT32", 5) == 0)
            {
                partition_type = PART_TYPE_FAT32;
            }
            else if(strncmp(filesystem_type, "FAT12", 5) == 0)
            {
                partition_type = PART_TYPE_FAT12;
            }
            else if(strncmp(filesystem_type, "FAT16", 5) == 0)
            {
                partition_type = PART_TYPE_FAT16;
            }
            else if(strncmp(filesystem_type, "FAT", 3) == 0)
            {
                partition_type = PART_TYPE_FAT16;// ??
            }
        }
    }

    // judge version 8.0 BPB (NTFS)
    signature = bpb[0x26];
    if(signature == 0x80)
    {
        partition_type = PART_TYPE_NTFS_exFAT;
        partition_name = _T("NTFS");
    }

    delete [] bpb;

    FS_TYPE fs_type = FS_TYPE_UNKNOWN;
    if(partition_type == PART_TYPE_NTFS_exFAT)
    {
        if(partition_name == _T("NTFS"))
        {
            fs_type = FS_TYPE_NTFS;
        }
        if(partition_name == _T("exFAT"))
        {
            fs_type = FS_TYPE_exFAT;
        }
    }
    else if(partition_type == PART_TYPE_FAT32)
    {
        fs_type = FS_TYPE_FAT32;
    }
    else if(partition_type == PART_TYPE_FAT16)
    {
        fs_type = FS_TYPE_FAT16;
    }
    else if(partition_type == PART_TYPE_FAT12)
    {
        fs_type = FS_TYPE_FAT12;
    }

    VOLUME_TYPE_INFO type_info;
    type_info.PartitionType = partition_type;
    type_info.FileSystemName = FileSystem::FSTypeToString(fs_type);
    type_info.FileSystemType = fs_type;

    return type_info;

}

shared_ptr<DiskUpdaterCollection> FileSystem::GetShrinkedSectorData(shared_ptr<MBRGPT> mbrgpt)
{
    UINT64 partition_start_sector = mbrgpt->GetPartitionStartSectorToShrink();

    shared_ptr<DiskUpdaterCollection> data_collection(new DiskUpdaterCollection());
    VOLUME_TYPE_INFO type_info = create_volume_type_info(partition_start_sector);

    switch(type_info.FileSystemType)
    {
    case FS_TYPE_NTFS:
        {
            NTFS ntfs(stream);
            ntfs.GetShrinkedSectorData(data_collection, mbrgpt);
        }
        break;
    case FS_TYPE_FAT12:
    case FS_TYPE_FAT16:
    case FS_TYPE_FAT32:
        {
            FAT fat(stream);
            fat.GetShrinkedSectorData(data_collection, mbrgpt);
        }
        break;
    default:
        break;
    }

    return data_collection;
}

FSInfo FileSystem::GetFSInfo(UINT64 partition_start_sector)
{
    VOLUME_TYPE_INFO type_info = create_volume_type_info(partition_start_sector);

    switch(type_info.FileSystemType)
    {
    case FS_TYPE_NTFS:
        {
            NTFS ntfs(stream);
            return ntfs.GetFSInfo(partition_start_sector);
        }
    case FS_TYPE_FAT12:
    case FS_TYPE_FAT16:
    case FS_TYPE_FAT32:
        {
            FAT fat(stream);
            return fat.GetFSInfo(partition_start_sector);
        }
    default:
        break;
    }

    FSInfo result;
    result.TypeInfo = type_info;
    result.SizeInfo.SizeCalculated = false;
    return result;
}

tstring FileSystem::FSTypeToString(FS_TYPE fs_type)
{
    switch(fs_type)
    {
    case FS_TYPE_FAT12:
        return _T("FAT12");
    case FS_TYPE_FAT16:
        return _T("FAT16");
    case FS_TYPE_FAT32:
        return _T("FAT32");
    case FS_TYPE_exFAT:
        return _T("exFAT");
    case FS_TYPE_NTFS:
        return _T("NTFS");
    }
    return _T("Unknown");
}

tstring FileSystem::PartitionTypeToString(UINT8 partition_type)
{
    switch(partition_type)
    {
      case 0x00:
        return _T("Empty");
      case 0x01:
        return _T("FAT12");
      case 0x04:
      case 0x06:
      case 0x0E:
        return _T("FAT16");
      case 0x0B:
      case 0x0C:
        return _T("FAT32");
      case 0x05:
      case 0x0F:
        return _T("Extended");
      case 0x07:
        return _T("NTFS");
      case 0x42:
        return _T("Secure");
      case 0x82:
        return _T("Linux Swap");
      case 0x83:
        return _T("Linux Ext");
      case 0xEE:
        return _T("GPT protective");
      case 0xEF:
        return _T("ESP");
      case 0xFF:
        return _T("BitLocker");
    }
    return _T("Unknown");
}

