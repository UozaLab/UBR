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

#ifndef __FSINFO__H__
#define __FSINFO__H__

#include <windows.h>
#include <vector>
#include <algorithm>
#include "tstring.h"

enum PARTITION_TYPE
{
    PART_TYPE_Empty = 0x00,
    PART_TYPE_FAT12,
    PART_TYPE_Extended = 0x05,
    PART_TYPE_FAT16,
    PART_TYPE_NTFS_exFAT,
    PART_TYPE_FAT32 = 0x0B,

    PART_TYPE_BitLocker = 0xFF
};

enum FS_TYPE
{
    FS_TYPE_UNKNOWN,
    FS_TYPE_FAT12,
    FS_TYPE_FAT16,
    FS_TYPE_FAT32,
    FS_TYPE_exFAT,
    FS_TYPE_NTFS,
};


struct VOLUME_TYPE_INFO
{
    UINT8 PartitionType;
    tstring FileSystemName;
    FS_TYPE FileSystemType;
    tstring VolumeName;

    VOLUME_TYPE_INFO()
    {
        PartitionType = 0;
        FileSystemType = FS_TYPE_UNKNOWN;
        FileSystemName = _T("");
        VolumeName = _T("");
    }

};

struct CLUSTER_RANGE
{
    UINT64 AddressFrom;
    UINT64 AddressTo;
    CLUSTER_RANGE():
    AddressFrom(0ULL), AddressTo(0ULL) {}
};

struct VOLUME_SIZE_INFO
{
    std::vector<CLUSTER_RANGE> EmptyClusterRanges;
    bool SizeCalculated;
    ULONGLONG Total;//Byte
    ULONGLONG Used;//Byte
    ULONGLONG Fixed;//Byte
    double UsedRatio;

    UINT32 SectorsPerCluster;
    UINT32 BytesPerSector;

    VOLUME_SIZE_INFO()
    {
        SizeCalculated = false;
        UsedRatio = 0.0;
        SectorsPerCluster = 0;
        BytesPerSector = 0;
        Total = 0ULL;
        Used = 0ULL;
        Fixed = 0ULL;
    }
};

struct FSInfo
{
    VOLUME_TYPE_INFO TypeInfo;
    VOLUME_SIZE_INFO SizeInfo;
};


struct DISK_UPDATER_BYTE
{
    UINT8* Data;
    DWORD Size;//byte
    UINT64 Address;//byte
};

struct DISK_UPDATER_SECTOR
{
    UINT64 Count;
    UINT64 AddressFrom;//sector
    UINT64 AddressTo;//sector
    bool can_skip;
};

struct DISK_UPDATER
{
    bool sector;
    bool zerofill;
    bool crc;

    DISK_UPDATER_BYTE ByteUpdater;
    DISK_UPDATER_SECTOR SectorUpdater;
};

enum DISK_BITOP_CMD
{
    BITOP_AND,
    BITOP_OR,
};

struct DISK_BITOP
{
    UINT64 AddressFrom;//sector
    UINT64 AddressTo;//sector
    DWORD IndexFrom;// byte
    DWORD IndexTo;//byte
    DISK_BITOP_CMD Operation;
    UINT8 Value;

    UINT64 IntersectFrom;//sector
    UINT64 IntersectTo;//sector
    DWORD IntersectIndexFrom;//byte
    DWORD IntersectIndexTo;//byte
};

class DiskUpdaterCollection
{
protected:
    std::vector<DISK_UPDATER> updater_collection;
    std::vector<DISK_UPDATER> intercept_collection;
    std::vector<DISK_BITOP> bitop_collection;
public:
    DiskUpdaterCollection(){}
    ~DiskUpdaterCollection()
    {
        Clear();
    }
    void Clear()
    {
        for(std::vector<DISK_UPDATER>::iterator itr = updater_collection.begin(), itr_end = updater_collection.end();
            itr != itr_end; itr++)
        {
            if(!itr->crc &&itr->sector) continue;
            if(itr->ByteUpdater.Data != NULL)
                delete [] itr->ByteUpdater.Data;
        }
        updater_collection.clear();

        for(std::vector<DISK_UPDATER>::iterator itr = intercept_collection.begin(), itr_end = intercept_collection.end();
            itr != itr_end; itr++)
        {
            if(itr->ByteUpdater.Data != NULL)
                delete [] itr->ByteUpdater.Data;
        }
        intercept_collection.clear();
        bitop_collection.clear();
    }


    void Append(const DiskUpdaterCollection& collection)
    {
        for(std::vector<DISK_UPDATER>::const_iterator itr = collection.updater_collection.begin(), itr_end = collection.updater_collection.end();
            itr != itr_end; itr++)
        {
            DISK_UPDATER du = *itr;
            if(du.crc)
            {
                AppendCrc(du.ByteUpdater.Data, du.ByteUpdater.Size, du.SectorUpdater.Count, du.SectorUpdater.AddressFrom, du.SectorUpdater.AddressTo);
            }
            else if(!du.sector && du.ByteUpdater.Data != NULL)
            {
                AppendByteUpdate(du.ByteUpdater.Data, du.ByteUpdater.Size, du.ByteUpdater.Address);
            }
            else
            {
                updater_collection.push_back(du);
            }
        }

        for(std::vector<DISK_UPDATER>::const_iterator itr = collection.intercept_collection.begin(), itr_end = collection.intercept_collection.end();
            itr != itr_end; itr++)
        {
            DISK_UPDATER du = *itr;
            AppendIntercept(du.ByteUpdater.Data, du.ByteUpdater.Size, du.SectorUpdater.AddressFrom);
        }

        for(std::vector<DISK_BITOP>::const_iterator itr = collection.bitop_collection.begin(), itr_end = collection.bitop_collection.end();
            itr != itr_end; itr++)
        {
            bitop_collection.push_back(*itr);
        }
    }

    void AppendBitop(UINT64 address_from/*sector*/, UINT64 address_to/*sector*/, DWORD index_from, DWORD index_to, DISK_BITOP_CMD operation, UINT8 value)
    {
        DISK_BITOP db;
        db.AddressFrom = address_from;
        db.AddressTo = address_to;
        db.IndexFrom = index_from;
        db.IndexTo = index_to;
        db.Operation = operation;
        db.Value = value;
        bitop_collection.push_back(db);
    }

    //void AppendBitmapShrinkBitop(UINT64 partition_start_sector, UINT64 address/*cluster*/, UINT64 count/*cluster*/, DWORD SectorsPerCluster, DWORD BytePerSector)
    //{
    //    UINT64 address_from_sector = address * SectorsPerCluster;
    //    address_from_sector += partition_start_sector;

    //    UINT64 address_to_sector = address + count;
    //    address_to_sector *= SectorsPerCluster;
    //    address_to_sector += partition_start_sector;
    //    address_to_sector--;

    //    AppendBitop(address_from_sector, address_to_sector, 0, BytePerSector-1, BITOP_AND, 0x00);
    //}

    void AppendBitmapBitop(UINT64 bmp_start_sector, UINT64 bmp_start_cluster, UINT64 address_from/*cluster*/, UINT64 address_to/*cluster*/, DWORD SectorsPerCluster, DWORD BytePerSector, bool op_clear)
    {
        UINT64 address_from_sector = bmp_start_sector;
        UINT64 address_from_byte = (address_from - bmp_start_cluster) / 8;
        UINT8 address_from_bit = (address_from - bmp_start_cluster) % 8;
        while(address_from_byte >= BytePerSector)
        {
            address_from_byte -= BytePerSector;
            address_from_sector++;
        }

        UINT64 address_to_sector = bmp_start_sector;
        UINT64 address_to_byte = (address_to - bmp_start_cluster) / 8;
        UINT8 address_to_bit = (address_to - bmp_start_cluster) % 8;
        while(address_to_byte >= BytePerSector)
        {
            address_to_byte -= BytePerSector;
            address_to_sector++;
        }

        bool head_eq_tail = (address_from_sector == address_to_sector) && (address_from_byte == address_to_byte);
        UINT8 value_from = 0xFF;
        value_from >>= (8 - address_from_bit);
        UINT8 value_to = 0xFF;
        value_to <<= (address_to_bit + 1);
        if(head_eq_tail)
        {
            AppendBitop(address_from_sector, address_from_sector, (DWORD) address_from_byte, (DWORD) address_from_byte,
                op_clear ? BITOP_AND : BITOP_OR, op_clear ? (value_from | value_to) : ~(value_from | value_to));
        }
        else
        {
            AppendBitop(address_from_sector, address_from_sector, (DWORD) address_from_byte, (DWORD) address_from_byte,
                op_clear ? BITOP_AND : BITOP_OR, op_clear ? value_from: ~value_from);

            if(address_from_sector != address_to_sector || address_to_byte - address_from_byte > 1)
                AppendBitop(address_from_sector, address_to_sector, (DWORD) address_from_byte + 1, (DWORD) address_to_byte - 1,
                op_clear ? BITOP_AND : BITOP_OR, op_clear ? 0x00 : 0xFF);

            AppendBitop(address_to_sector, address_to_sector, (DWORD) address_to_byte, (DWORD) address_to_byte, 
                op_clear ? BITOP_AND : BITOP_OR, op_clear ? value_to : ~value_to);
        }
    }

    //void AppendBitmapModTailBitop(UINT64 partition_start_sector, UINT64 address/*cluster*/, UINT64 index, UINT8 usebit_count, DWORD SectorsPerCluster, DWORD BytePerSector)
    //{
    //    if(usebit_count == 0) return;

    //    UINT64 address_sector = address * SectorsPerCluster;
    //    address_sector += partition_start_sector;

    //    while(index > BytePerSector)
    //    {
    //        address_sector++;
    //        index -= BytePerSector;
    //    }
    //    if(index == 0)
    //    {
    //        address_sector--;
    //        index = BytePerSector - 1;
    //    }
    //    else
    //    {
    //        index--;
    //    }
    //    UINT8 value = 0xFF;
    //    value <<= usebit_count;

    //    AppendBitop(address_sector, address_sector, (DWORD) index, (DWORD) index, BITOP_OR, value);
    //}

    void AppendIntercept(const UINT8* data, DWORD size, UINT64 address_from/*sector*/)
    {
        DISK_UPDATER du;
        du.ByteUpdater.Data = new UINT8[size];
        memcpy(du.ByteUpdater.Data, data, size);
        du.ByteUpdater.Size = size;
        du.SectorUpdater.AddressFrom = address_from;
        intercept_collection.push_back(du);
    }

    void AppendIntercept(const UINT8* data, DWORD size, WORD sector_size, UINT64 address_from/*sector*/)
    {
        int sectors = size / sector_size;
        for(int i=0; i< sectors; i++)
        {
            AppendIntercept(data + sector_size * i, sector_size, address_from + i);
        }
    }

    void AppendByteUpdate(const UINT8* data, DWORD size, UINT64 address/*byte*/)
    {
        DISK_UPDATER du;
        du.sector = false;
        du.zerofill = false;
        du.crc = false;
        du.ByteUpdater.Data = new UINT8[size];
        memcpy(du.ByteUpdater.Data, data, size);
        du.ByteUpdater.Size = size;
        du.ByteUpdater.Address = address;
        updater_collection.push_back(du);
    }

    void Append(UINT64 count/*sector*/, UINT64 address_from/*sector*/, UINT64 address_to/*sector*/, bool can_skip=true)
    {
        DISK_UPDATER du;
        du.sector = true;
        du.zerofill = false;
        du.crc = false;
        du.SectorUpdater.Count = count;
        du.SectorUpdater.AddressFrom = address_from;
        du.SectorUpdater.AddressTo = address_to;
        du.SectorUpdater.can_skip = can_skip;
        updater_collection.push_back(du);
    }

    void AppendZerofill(UINT64 count/*sector*/, UINT64 address_to/*sector*/)
    {
        DISK_UPDATER du;
        du.sector = true;
        du.zerofill = true;
        du.crc = false;
        du.SectorUpdater.Count = count;
        du.SectorUpdater.AddressFrom = 0;
        du.SectorUpdater.AddressTo = address_to;
        du.SectorUpdater.can_skip = false;
        updater_collection.push_back(du);
    }

    void AppendCrc(const UINT8* data, DWORD size, UINT64 count/*sector(11)*/, UINT64 address_from/*sector(0 or 12)*/, UINT64 address_to/*sector(11 or 23)*/)
    {
        DISK_UPDATER du;
        du.sector = true;
        du.zerofill = false;
        du.crc = true;

        du.ByteUpdater.Data = new UINT8[size];
        memcpy(du.ByteUpdater.Data, data, size);
        du.ByteUpdater.Size = size;

        du.SectorUpdater.Count = count;
        du.SectorUpdater.AddressFrom = address_from;
        du.SectorUpdater.AddressTo = address_to;
        du.SectorUpdater.can_skip = false;
        updater_collection.push_back(du);
    }

    int Count()
    {
        return static_cast<int>(updater_collection.size());
    }

    DISK_UPDATER Get(int index)
    {
        return updater_collection[index];
    }

    std::vector<DISK_UPDATER> FindIntercept(UINT64 address_from, UINT64 sector_count)
    {
        std::vector<DISK_UPDATER> result;

        for(std::vector<DISK_UPDATER>::iterator itr = intercept_collection.begin(), itr_end = intercept_collection.end();
            itr != itr_end; itr++)
        {
            DISK_UPDATER du = *itr;
            if(address_from <= du.SectorUpdater.AddressFrom &&
                du.SectorUpdater.AddressFrom <= (address_from + sector_count - 1))
            {
                result.push_back(du);
            }
        }
        return result;
    }

    std::vector<DISK_BITOP> FindBitop(UINT64 address_from, UINT64 sector_count)
    {
        std::vector<DISK_BITOP> result;

        for(std::vector<DISK_BITOP>::iterator itr = bitop_collection.begin(), itr_end = bitop_collection.end();
            itr != itr_end; itr++)
        {
            DISK_BITOP db = *itr;
            if((address_from <= db.AddressFrom && db.AddressFrom <= (address_from + sector_count - 1)) ||
                (address_from <= db.AddressTo && db.AddressTo <= (address_from + sector_count - 1)) ||
                (address_from > db.AddressFrom && db.AddressTo > (address_from + sector_count - 1)))
            {
                db.IntersectFrom = (std::max)(address_from, db.AddressFrom);
                db.IntersectTo = (std::min)(address_from + sector_count -1, db.AddressTo);
                db.IntersectIndexFrom = (db.IntersectFrom == db.AddressFrom) ? db.IndexFrom : 0;
                db.IntersectIndexTo = (db.IntersectTo == db.AddressTo) ? db.IndexTo : 0;

                result.push_back(db);
            }
        }
        return result;
    }

};

#endif
