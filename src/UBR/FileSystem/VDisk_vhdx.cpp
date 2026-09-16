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

#include "VDisk_vhdx.h"

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


DEFINE_GUID(GUID_BAT, 0x2dc27766, 0xf623, 0x4200, 0x9d, 0x64, 0x11, 0x5e, 0x9b, 0xfd, 0x4a, 0x08);
DEFINE_GUID(GUID_Metadata, 0x8B7CA206, 0x4790, 0x4B9A, 0xB8, 0xFE, 0x57, 0x5F, 0x05, 0x0F, 0x88, 0x6E);
DEFINE_GUID(GUID_FileParameters, 0xCAA16737, 0xFA36, 0x4D43, 0xB3, 0xB6, 0x33, 0xF0, 0xAA, 0x44, 0xE7, 0x6B);
DEFINE_GUID(GUID_VirtualDiskSize, 0x2FA54224, 0xCD1B, 0x4876, 0xB2, 0x11, 0x5D, 0xBE, 0xD8, 0x3B, 0xF4, 0xB8);
DEFINE_GUID(GUID_VirtualDiskID, 0xBECA12AB, 0xB2E6, 0x4523, 0x93, 0xEF, 0xC3, 0x09, 0xE0, 0x00, 0xC7, 0x46);
DEFINE_GUID(GUID_LogicalSectorSize, 0x8141BF1D, 0xA96F, 0x4709, 0xBA, 0x47, 0xF2, 0x33, 0xA8, 0xFA, 0xAB, 0x5F);
DEFINE_GUID(GUID_PhysicalSectorSize, 0xCDA348C7, 0x445D, 0x4471, 0x9C, 0xC9, 0xE9, 0x88, 0x52, 0x51, 0xC5, 0x56);


VHDX::VHDX(const TCHAR* _filename, UINT32 blocksize, UINT64 virtual_disksize, UINT32 logical_sectorsize, UINT32 physical_sectorsize)
: VHDCommon(_filename, true), 
  crc32(0x82f63b78),
  bat(NULL), 
  metadata_table_header(NULL), metadata_table_entry(NULL)
{
    create_new_disk(blocksize, virtual_disksize, logical_sectorsize, physical_sectorsize);
}

VHDX::VHDX(const TCHAR* _filename)
: VHDCommon(_filename), 
  crc32(0x82f63b78),
  bat(NULL), 
  metadata_table_header(NULL), metadata_table_entry(NULL)
{
    ZeroMemory(&metadata_file_parameters, sizeof(VHDX_FILE_PARAMETERS));
    ZeroMemory(&metadata_virtual_disk_size, sizeof(VHDX_VIRTUAL_DISK_SIZE));
    ZeroMemory(&metadata_virtual_disk_id, sizeof(VHDX_VIRTUAL_DISK_ID));
    ZeroMemory(&metadata_logical_sector_size, sizeof(VHDX_LOGICAL_SECTOR_SIZE));
    ZeroMemory(&metadata_physical_sector_size, sizeof(VHDX_PHYSICAL_SECTOR_SIZE));
}

VHDX::~VHDX()
{
    if(create_new && !flushed)
        Flush();

    if(bat != NULL)
        delete [] bat;

    if(metadata_table_header != NULL)
        delete metadata_table_header;

    if(metadata_table_entry != NULL)
        delete [] metadata_table_entry;
}

bool VHDX::read_footer_header()
{
    if(create_new) return true;
	DWORD dwByteRead = 0;

    // FileIdentifier
    if(!read(0, (UINT8*)&file_type_identifier, sizeof(VHDX_FILE_TYPE_IDENTIFIER), &dwByteRead))
      return false;
    if(sizeof(VHDX_FILE_TYPE_IDENTIFIER) != dwByteRead) return false;
    if(file_type_identifier.Signature != 0x656C696678646876/*vhdxfile*/) return false;

    // Header1
    if(!read(64*1024, (UINT8*)&header1, sizeof(VHDX_HEADER), &dwByteRead))
      return false;
    if(sizeof(VHDX_HEADER) != dwByteRead) return false;
    bool header1_valid = (header1.Signature == 0x64616568/*head*/) && checksum((UINT8*)&header1, sizeof(VHDX_HEADER));

    // Header2
    if(!read(64*1024*2, (UINT8*)&header2, sizeof(VHDX_HEADER), &dwByteRead))
      return false;
    if(sizeof(VHDX_HEADER) != dwByteRead) return false;
    bool header2_valid = (header2.Signature == 0x64616568/*head*/) && checksum((UINT8*)&header2, sizeof(VHDX_HEADER));

    VHDX_HEADER* header = &header1;
    if(!header1_valid && !header2_valid) return false;
    if(header1_valid) header = &header1;
    if(header2_valid) header = &header2;
    if(header1_valid && header2_valid)
    {
        if(header1.SequenceNumber > header2.SequenceNumber)
            header = &header1;
        if(header1.SequenceNumber < header2.SequenceNumber)
            header = &header2;
    }

    // region table 1
    if(!read(64*1024*3, (UINT8*)&region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER), &dwByteRead))
      return false;
    if(sizeof(VHDX_REGION_TABLE_HEADER) != dwByteRead) return false;
    bool region_table_header1_valid = (region_table_header1.Signature == 0x69676572/*regi*/) && 
                                      checksum((UINT8*)&region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER));

    // region table 2
    if(!read(64*1024*4, (UINT8*) &region_table_header2, sizeof(VHDX_REGION_TABLE_HEADER), &dwByteRead))
      return false;
    if(sizeof(VHDX_REGION_TABLE_HEADER) != dwByteRead) return false;
    bool region_table_header2_valid = (region_table_header2.Signature == 0x69676572/*regi*/) &&
                                      checksum((UINT8*)&region_table_header2, sizeof(VHDX_REGION_TABLE_HEADER));

    if(!region_table_header1_valid && !region_table_header2_valid) return false;
    VHDX_REGION_TABLE_HEADER* region_table_header = &region_table_header1;
    if(!region_table_header1_valid) region_table_header = &region_table_header2;

    // BAT and Metadata
    for (UINT32 region_index=0; region_index<region_table_header->EntryCount; region_index++)
    {
        UINT64 FileOffset = region_table_header->RegionTableEntries[region_index].FileOffset;
        UINT32 Length = region_table_header->RegionTableEntries[region_index].Length;
        GUID Guid = region_table_header->RegionTableEntries[region_index].Guid;

        if (Guid == GUID_BAT)
        {
            if(bat != NULL) delete [] bat;
            bat = new VHDX_BAT_ENTRY[Length / sizeof(VHDX_BAT_ENTRY)];
            if(!read(FileOffset, (UINT8*)bat, Length, &dwByteRead))
              return false;
            if(dwByteRead != Length) return false;
        }

        if (Guid == GUID_Metadata)
        {
            if(metadata_table_header != NULL) delete metadata_table_header;
            metadata_table_header = new VHDX_METADATA_TABLE_HEADER;

            if(!read(FileOffset, (UINT8*)metadata_table_header, sizeof(VHDX_METADATA_TABLE_HEADER), &dwByteRead))
              return false;
            if(dwByteRead != sizeof(VHDX_METADATA_TABLE_HEADER))
                return false;

            if(metadata_table_entry != NULL) delete [] metadata_table_entry;
            metadata_table_entry = new VHDX_METADATA_TABLE_ENTRY[metadata_table_header->EntryCount];
            if(!read(FileOffset + sizeof(VHDX_METADATA_TABLE_HEADER), (UINT8*)metadata_table_entry, sizeof(VHDX_METADATA_TABLE_ENTRY)*metadata_table_header->EntryCount, &dwByteRead))
                return false;
            if(dwByteRead != sizeof(VHDX_METADATA_TABLE_ENTRY)*metadata_table_header->EntryCount)
                return false;

            for (UINT32 metadata_index=0; metadata_index<metadata_table_header->EntryCount; metadata_index++)
            {
                GUID ItemID = metadata_table_entry[metadata_index].ItemID;
                UINT64 pos = FileOffset + metadata_table_entry[metadata_index].Offset;

                if(ItemID == GUID_FileParameters)
                {
                    if(!read(pos, (UINT8*)&metadata_file_parameters, sizeof(VHDX_FILE_PARAMETERS), &dwByteRead))
                      return false;
                    if(dwByteRead != sizeof(VHDX_FILE_PARAMETERS))
                        return false;
                }
                if(ItemID == GUID_VirtualDiskSize)
                {
                    if(!read(pos, (UINT8*)&metadata_virtual_disk_size, sizeof(VHDX_VIRTUAL_DISK_SIZE), &dwByteRead))
                      return false;
                    if(dwByteRead != sizeof(VHDX_VIRTUAL_DISK_SIZE))
                        return false;
                }
                if(ItemID == GUID_VirtualDiskID)
                {
                    if(!read(pos, (UINT8*)&metadata_virtual_disk_id, sizeof(VHDX_VIRTUAL_DISK_ID), &dwByteRead))
                      return false;
                    if(dwByteRead != sizeof(VHDX_VIRTUAL_DISK_ID))
                        return false;
                }
                if(ItemID == GUID_LogicalSectorSize)
                {
                    if(!read(pos, (UINT8*)&metadata_logical_sector_size, sizeof(VHDX_LOGICAL_SECTOR_SIZE), &dwByteRead))
                      return false;
                    if(dwByteRead != sizeof(VHDX_LOGICAL_SECTOR_SIZE))
                        return false;
                }
                if(ItemID == GUID_PhysicalSectorSize)
                {
                    if(!read(pos, (UINT8*)&metadata_physical_sector_size, sizeof(VHDX_PHYSICAL_SECTOR_SIZE), &dwByteRead))
                      return false;
                    if(dwByteRead != sizeof(VHDX_PHYSICAL_SECTOR_SIZE))
                        return false;
                }
            }
        }
    }
    return true;
}

ULONGLONG VHDX::GetDiskSizeImpl()
{
    if(invalid) return 0;
    return metadata_virtual_disk_size.VirtualDiskSize;
}

DWORD VHDX::GetSectorSizeImpl()
{
    if(invalid) return 0;
    return metadata_logical_sector_size.LogicalSectorSize;
}

DWORD VHDX::GetBlockSizeImpl()
{
    if(invalid) return 0;
    return metadata_file_parameters.BlockSize;
}

DWORD VHDX::GetTableEntriesCountImpl()
{
    if(invalid) return 0;
    UINT64 ret = (metadata_virtual_disk_size.VirtualDiskSize + metadata_file_parameters.BlockSize - 1) / metadata_file_parameters.BlockSize;
    return (DWORD) ret;
}

UINT32 VHDX::GetDiskTypeImpl()
{
    if(invalid) return 0;
    if(metadata_file_parameters.LeaveBlocksAllocated) return 2;//fixed
    if(metadata_file_parameters.HasParent) return 4;//differencing
    return 3;//dynamic
}

BOOL VHDX::GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    if(invalid) return FALSE;

    *ByteRead = 0;
    *can_skip = false;

    DWORD blocksize = GetBlockSize();
    UINT64 sb_bytes = 0x800000LL * GetSectorSizeImpl();
    UINT32 ChunkRatio = (UINT32) (sb_bytes / GetBlockSizeImpl());
    UINT32 sb_count = blockindex / ChunkRatio;
    UINT32 index = blockindex + sb_count;

    unsigned int state = bat[index].State;
    if(state == VHDX_PAYLOAD_BLOCK_NOT_PRESENT)
	{
        ZeroMemory(blockdata, blocksize);
        *ByteRead = 0;
        *can_skip = true;
	}
    else if(state == VHDX_PAYLOAD_BLOCK_FULLY_PRESENT)
    {
        if(!read(((INT64) bat[index].FileOffsetMB) * 1048576/*1MB*/, (UINT8*)blockdata, blocksize, ByteRead))
          return FALSE;
    }
    else if(state == VHDX_PAYLOAD_BLOCK_ZERO)
    {
        ZeroMemory(blockdata, blocksize);
        *ByteRead = blocksize;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

bool VHDX::checksum(UINT8 *buf, int len)
{
    UINT8* buf_dup = new UINT8[len];
    memcpy(buf_dup, buf, len);
    UINT32 org_sum = *((UINT32*)buf_dup+1);
    ZeroMemory(buf_dup+4, 4);

    UINT32 checksum = crc32.calculate(buf_dup, len);
    delete [] buf_dup;
    return org_sum == checksum;
}

void VHDX::create_new_disk(UINT32 blocksize, UINT64 virtual_disksize, UINT32 logical_sectorsize, UINT32 physical_sectorsize)
{
    UINT64 sb_bytes = logical_sectorsize;
    sb_bytes *= 0x800000LL;
    UINT32 ChunkRatio = (UINT32) (sb_bytes / blocksize);
    UINT32 block_count = (UINT32) ((virtual_disksize + blocksize - 1) / blocksize);
    UINT32 sb_count = block_count / ChunkRatio;
    UINT32 entry_count = block_count + sb_count;
    bat_size = entry_count;
    bat_size *= 8;
    if(bat_size % 1048576 != 0)
        bat_size += (1048576 - (bat_size % 1048576));

    ZeroMemory(&file_type_identifier, sizeof(VHDX_FILE_TYPE_IDENTIFIER));
    file_type_identifier.Signature = 0x656C696678646876;

    GUID guid;
    ZeroMemory(&header1, sizeof(VHDX_HEADER));
    header1.Signature = 0x64616568;
    header1.SequenceNumber = 0;
    CoCreateGuid(&guid);
    header1.FileWriteGuid = guid;
    CoCreateGuid(&guid);
    header1.DataWriteGuid = guid;
    header1.Version = 1;
    header1.LogLength = 1048576;// 1MB
    header1.LogOffset = 1048576;// 1MB
    memcpy(&header2, &header1, sizeof(VHDX_HEADER));
    header2.SequenceNumber = 1;
    header1.Checksum = crc32.calculate((UINT8*) &header1, sizeof(VHDX_HEADER));
    header2.Checksum = crc32.calculate((UINT8*) &header2, sizeof(VHDX_HEADER));

    ZeroMemory(&region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER));
    region_table_header1.Signature = 0x69676572;
    region_table_header1.EntryCount = 2;
    region_table_header1.RegionTableEntries[0].Guid = GUID_BAT;
    region_table_header1.RegionTableEntries[0].FileOffset = 3145728;// 3MB
    region_table_header1.RegionTableEntries[0].Length = (UINT32)bat_size;
    region_table_header1.RegionTableEntries[0].Required = 1;
    region_table_header1.RegionTableEntries[1].Guid = GUID_Metadata;
    region_table_header1.RegionTableEntries[1].FileOffset = 2097152;// 2MB
    region_table_header1.RegionTableEntries[1].Length = 1048576;// 1MB
    region_table_header1.RegionTableEntries[1].Required = 1;
    region_table_header1.Checksum = crc32.calculate((UINT8*)&region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER));
    memcpy(&region_table_header2, &region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER));

    if(metadata_table_header != NULL) delete metadata_table_header;
    metadata_table_header = new VHDX_METADATA_TABLE_HEADER;
    ZeroMemory(metadata_table_header, sizeof(VHDX_METADATA_TABLE_HEADER));
    metadata_table_header->Signature = 0x617461646174656D;
    metadata_table_header->EntryCount = 5;

    if(metadata_table_entry != NULL) delete [] metadata_table_entry;
    metadata_table_entry = new VHDX_METADATA_TABLE_ENTRY[5];
    ZeroMemory(metadata_table_entry, sizeof(VHDX_METADATA_TABLE_ENTRY)*5);
    UINT32 offset = 65536;
    metadata_table_entry[0].ItemID = GUID_FileParameters;
    metadata_table_entry[0].Offset = offset;
    metadata_table_entry[0].Length = sizeof(VHDX_FILE_PARAMETERS);
    offset += metadata_table_entry[0].Length;
    metadata_table_entry[0].IsUser = 0;
    metadata_table_entry[0].IsVirtualDisk = 0;
    metadata_table_entry[0].IsRequired = 1;

    metadata_table_entry[1].ItemID = GUID_VirtualDiskSize;
    metadata_table_entry[1].Offset = offset;
    metadata_table_entry[1].Length = sizeof(VHDX_VIRTUAL_DISK_SIZE);
    offset += metadata_table_entry[1].Length;
    metadata_table_entry[1].IsUser = 0;
    metadata_table_entry[1].IsVirtualDisk = 1;
    metadata_table_entry[1].IsRequired = 1;

    metadata_table_entry[2].ItemID = GUID_VirtualDiskID;
    metadata_table_entry[2].Offset = offset;
    metadata_table_entry[2].Length = sizeof(VHDX_VIRTUAL_DISK_ID);
    offset += metadata_table_entry[2].Length;
    metadata_table_entry[2].IsUser = 0;
    metadata_table_entry[2].IsVirtualDisk = 1;
    metadata_table_entry[2].IsRequired = 1;

    metadata_table_entry[3].ItemID = GUID_LogicalSectorSize;
    metadata_table_entry[3].Offset = offset;
    metadata_table_entry[3].Length = sizeof(VHDX_LOGICAL_SECTOR_SIZE);
    offset += metadata_table_entry[3].Length;
    metadata_table_entry[3].IsUser = 0;
    metadata_table_entry[3].IsVirtualDisk = 1;
    metadata_table_entry[3].IsRequired = 1;

    metadata_table_entry[4].ItemID = GUID_PhysicalSectorSize;
    metadata_table_entry[4].Offset = offset;
    metadata_table_entry[4].Length = sizeof(VHDX_PHYSICAL_SECTOR_SIZE);
    offset += metadata_table_entry[4].Length;
    metadata_table_entry[4].IsUser = 0;
    metadata_table_entry[4].IsVirtualDisk = 1;
    metadata_table_entry[4].IsRequired = 1;

    ZeroMemory(&metadata_file_parameters, sizeof(VHDX_FILE_PARAMETERS));
    ZeroMemory(&metadata_virtual_disk_size, sizeof(VHDX_VIRTUAL_DISK_SIZE));
    ZeroMemory(&metadata_virtual_disk_id, sizeof(VHDX_VIRTUAL_DISK_ID));
    ZeroMemory(&metadata_logical_sector_size, sizeof(VHDX_LOGICAL_SECTOR_SIZE));
    ZeroMemory(&metadata_physical_sector_size, sizeof(VHDX_PHYSICAL_SECTOR_SIZE));
    metadata_file_parameters.BlockSize = blocksize;
    metadata_virtual_disk_size.VirtualDiskSize = virtual_disksize;
    CoCreateGuid(&guid);
    metadata_virtual_disk_id.VirtualDiskId = guid;
    metadata_logical_sector_size.LogicalSectorSize = logical_sectorsize;
    metadata_physical_sector_size.PhysicalSectorSize = physical_sectorsize;

    // BAT
    bat = new VHDX_BAT_ENTRY[(unsigned int) (bat_size/sizeof(VHDX_BAT_ENTRY))];
    ZeroMemory(bat, (size_t) bat_size);
}

BOOL VHDX::SetBlockDataImpl(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite)
{
    DWORD blocksize = GetBlockSize();
    UINT64 sb_bytes = 0x800000LL * GetSectorSizeImpl();
    UINT32 ChunkRatio = (UINT32) (sb_bytes / GetBlockSizeImpl());
    UINT32 sb_count = blockindex / ChunkRatio;
    UINT32 index = blockindex + sb_count;
    UINT64 file_offset = 0;

    if(bat[index].State == VHDX_PAYLOAD_BLOCK_FULLY_PRESENT)
    {
        file_offset = bat[index].FileOffsetMB;
        file_offset *= 1048576;
    }
    else
    {
        file_offset = bat_size + 1048576*3 + (UINT64)blocksize * write_block_count;
        bat[index].State = VHDX_PAYLOAD_BLOCK_FULLY_PRESENT;
        bat[index].FileOffsetMB = file_offset / 1048576;
        write_block_count++;
    }

    BOOL result = write(file_offset, blockdata, blocksize, ByteWrite);
    flushed = false;
    return result != 0;
}

bool VHDX::write_header()
{
    const unsigned int buf_length = (UINT32)bat_size + 1048576*3;
    UINT8* buf = new UINT8[buf_length];
    ZeroMemory(buf, buf_length);

    memcpy(buf, &file_type_identifier, sizeof(VHDX_FILE_TYPE_IDENTIFIER));
    memcpy(buf + 65536, &header1, sizeof(VHDX_HEADER));
    memcpy(buf + 65536*2, &header2, sizeof(VHDX_HEADER));
    memcpy(buf + 65536*3, &region_table_header1, sizeof(VHDX_REGION_TABLE_HEADER));
    memcpy(buf + 65536*4, &region_table_header2, sizeof(VHDX_REGION_TABLE_HEADER));

    UINT32 offset = 2097152;// 2MB
    memcpy(buf + offset, metadata_table_header, sizeof(VHDX_METADATA_TABLE_HEADER));
    offset += sizeof(VHDX_METADATA_TABLE_HEADER);
    for(int i=0; i<metadata_table_header->EntryCount; i++)
    {
        VHDX_METADATA_TABLE_ENTRY entry = metadata_table_entry[i];
        memcpy(buf + offset, &entry, sizeof(VHDX_METADATA_TABLE_ENTRY));
        if(entry.ItemID == GUID_FileParameters)
            memcpy(buf + 2097152 + entry.Offset, &metadata_file_parameters, sizeof(VHDX_FILE_PARAMETERS));
        else if(entry.ItemID == GUID_VirtualDiskSize)
            memcpy(buf + 2097152 + entry.Offset, &metadata_virtual_disk_size, sizeof(VHDX_VIRTUAL_DISK_SIZE));
        else if(entry.ItemID == GUID_VirtualDiskID)
            memcpy(buf + 2097152 + entry.Offset, &metadata_virtual_disk_id, sizeof(VHDX_VIRTUAL_DISK_ID));
        else if(entry.ItemID == GUID_LogicalSectorSize)
            memcpy(buf + 2097152 + entry.Offset, &metadata_logical_sector_size, sizeof(VHDX_LOGICAL_SECTOR_SIZE));
        else if(entry.ItemID == GUID_PhysicalSectorSize)
            memcpy(buf + 2097152 + entry.Offset, &metadata_physical_sector_size, sizeof(VHDX_PHYSICAL_SECTOR_SIZE));

        offset += sizeof(VHDX_METADATA_TABLE_ENTRY);
    }
    memcpy(buf + 3145728, bat, bat_size);// BAT

    DWORD dwNumberOfWritten = 0;
    BOOL result = write(0, buf, buf_length, &dwNumberOfWritten);
    delete [] buf;
    return result != 0;
}

