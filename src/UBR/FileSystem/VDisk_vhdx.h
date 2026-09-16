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

#ifndef __VHDX__H__
#define __VHDX__H__

#include <windows.h>
#include "tstring.h"
#include "VDisk_vhd_common.h"
#include "Crc32.h"

#define VHDX_PAYLOAD_BLOCK_NOT_PRESENT       0
#define VHDX_PAYLOAD_BLOCK_UNDEFINED         1
#define VHDX_PAYLOAD_BLOCK_ZERO              2
#define VHDX_PAYLOAD_BLOCK_UNMAPPED          3
#define VHDX_PAYLOAD_BLOCK_FULLY_PRESENT     6
#define VHDX_PAYLOAD_BLOCK_PARTIALLY_PRESENT 7


#pragma pack(1)
struct VHDX_FILE_TYPE_IDENTIFIER
{
    UINT64 Signature;
    UINT16 Creator[256];
};

struct VHDX_HEADER
{
    UINT32 Signature;
    UINT32 Checksum;
    UINT64 SequenceNumber;
    GUID FileWriteGuid;
    GUID DataWriteGuid;
    GUID LogGuid;
    UINT16 LogVersion;
    UINT16 Version;
    UINT32 LogLength;
    UINT64 LogOffset;
    UINT8 Reserved[4016];
};

struct VHDX_REGION_TABLE_ENTRY
{
    GUID Guid;
    UINT64 FileOffset;
    UINT32 Length;
    UINT32 Required;
};

struct VHDX_REGION_TABLE_HEADER
{
    UINT32 Signature;
    UINT32 Checksum;
    UINT32 EntryCount;
    UINT8 Reserved[4];
    VHDX_REGION_TABLE_ENTRY RegionTableEntries[2047];// (64KB-16B)/32B=2047
    UINT8 Pad[16];//64KB-2047*32B-16B
};

struct VHDX_BAT_ENTRY
{
    UINT64 State : 3;
    UINT64 Reserved : 17;
    UINT64 FileOffsetMB : 44;
};

struct VHDX_METADATA_TABLE_HEADER
{
    UINT64 Signature;
    UINT16 Reserved;
    UINT16 EntryCount;
    UINT8 Reserved2[20];
};

struct VHDX_METADATA_TABLE_ENTRY
{
    GUID ItemID;
    UINT32 Offset;
    UINT32 Length;
    UINT32 IsUser : 1;
    UINT32 IsVirtualDisk : 1;
    UINT32 IsRequired : 1;
    UINT32 Reserved : 29;
    UINT8 Reserved2[4];
};

struct VHDX_FILE_PARAMETERS
{
    UINT32 BlockSize;
    UINT32 LeaveBlocksAllocated : 1;
    UINT32 HasParent : 1;
    UINT32 Reserved : 30;
};

struct VHDX_VIRTUAL_DISK_SIZE
{
    UINT64 VirtualDiskSize;
};

struct VHDX_VIRTUAL_DISK_ID
{
    GUID VirtualDiskId;
};

struct VHDX_LOGICAL_SECTOR_SIZE
{
    UINT32 LogicalSectorSize;
};

struct VHDX_PHYSICAL_SECTOR_SIZE
{
    UINT32 PhysicalSectorSize;
};
#pragma pack()



class VHDX : public VHDCommon
{
protected:
    CRC32 crc32;
    UINT64 bat_size;

protected:
    VHDX_FILE_TYPE_IDENTIFIER file_type_identifier;
    VHDX_HEADER header1;
    VHDX_HEADER header2;
    VHDX_REGION_TABLE_HEADER region_table_header1;
    VHDX_REGION_TABLE_HEADER region_table_header2;
    VHDX_BAT_ENTRY* bat;
    VHDX_METADATA_TABLE_HEADER* metadata_table_header;
    VHDX_METADATA_TABLE_ENTRY* metadata_table_entry;
    VHDX_FILE_PARAMETERS metadata_file_parameters;
    VHDX_VIRTUAL_DISK_SIZE metadata_virtual_disk_size;
    VHDX_VIRTUAL_DISK_ID metadata_virtual_disk_id;
    VHDX_LOGICAL_SECTOR_SIZE metadata_logical_sector_size;
    VHDX_PHYSICAL_SECTOR_SIZE metadata_physical_sector_size;

    virtual bool read_footer_header();
    bool write_header();
    bool write_footer(){ return true; }
    void create_new_disk(UINT32 blocksize, UINT64 virtual_disksize, UINT32 logical_sectorsize, UINT32 physical_sectorsize);
    bool checksum(UINT8 *buf, int len);

    virtual ULONGLONG GetDiskSizeImpl();
    virtual DWORD GetSectorSizeImpl();
    virtual DWORD GetBlockSizeImpl();
    virtual DWORD GetTableEntriesCountImpl();
    virtual UINT32 GetDiskTypeImpl();
    virtual BOOL GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);
    virtual BOOL SetBlockDataImpl(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite);

public:
    VHDX(const TCHAR* _filename, UINT32 blocksize, UINT64 virtual_disksize, UINT32 logical_sectorsize, UINT32 physical_sectorsize);
    VHDX(const TCHAR* _filename);
    ~VHDX();
    virtual tstring GetFileFormat()
    {
        return _T("VHDX");
    }
};

#endif