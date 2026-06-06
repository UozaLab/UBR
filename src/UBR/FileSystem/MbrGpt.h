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

#ifndef __MBRGPT_H__
#define __MBRGPT_H__

#include <windows.h>
#include "smart_ptr.h"
#include "Crc32.h"
#include "Vdisk.h"
#include "tstring.h"
#include "Misc.h"

#define MBR_PARTITION_ENTRY_START_ADDRESS 0x01BE
#define MBR_PARTITION_ENTRY_COUNT 4
#define GPT_HEADER_START_ADDRESS 512
#define GPT_PARTITION_ENTRY_COUNT 128

#pragma pack(1)
struct MBR_PARTITION_ENTRY
{
    UINT8 StatusOrPhysicalDrive;
    UINT8 CHSAddressOfFirstAbsoluteSector[3];
    UINT8 PartitionType;
    UINT8 CHSAddressOfLastAbsoluteSector[3];
    UINT32 LBAOfFirstAbsoluteSector;
    UINT32 NumberOfSectors;
};

struct CHS
{
    UINT8 Head;
    UINT8 Sector;
    UINT16 Cylinder;
};

struct GPT_HEADER {
    UINT8 Signature[8];
    UINT8 RevisionNumber[4];
    UINT32 HeaderSize;
    UINT32 HeaderChecksum;
    UINT8 Reserved[4];
    UINT64 CurrentLBA;
    UINT64 BackupLBA;
    UINT64 FirstUsableLBA;
    UINT64 LastUsableLBA;
    GUID Guid;
    UINT64 StartingLBAOfPartitionEntry;
    UINT32 PartitionEntryCount;
    UINT32 PartitionEntrySize;
    UINT32 PartitionEntryChecksum;
};

struct GPT_PARTITION_ENTRY {
    GUID PartitionType;
    GUID UniqueId;
    UINT64 FirstLBA;
    UINT64 LastLBA;
    UINT64 AttributeFlags;
    UINT8 PartitionName[72];
};

struct MBR_GPT_INFO
{
    MBR_PARTITION_ENTRY PE[MBR_PARTITION_ENTRY_COUNT];
    GPT_PARTITION_ENTRY GPE[GPT_PARTITION_ENTRY_COUNT];
    GPT_HEADER GptHeader;
};

#pragma pack()

class MBRGPT
{
protected:
    bool valid;
    bool gpt;
    bool modified;
    tstring reason;
    shared_ptr<VirtualDisk> vdisk;
    DWORD target_partition_number;
    UINT64 shift_sectors;
    UINT64 partition_start_sector;
    UINT64 fixed_sectors;
    shared_ptr<MBRGPT> org;

    void update_chs();
    CHS lba2chs(int lba);
    void update_sector();

public:
    MBR_GPT_INFO MbrGptInfo;
    UINT8 MbrGptSector[34*512];// MBR(1) + GPTHeader(1) + GPTTable(32)

public:
    MBRGPT(shared_ptr<VirtualDisk> _vdisk);
    MBRGPT(const MBRGPT& rhs);
    MBRGPT& operator=(const MBRGPT& rhs);

    shared_ptr<MBRGPT> CreateShrinked(shared_ptr<VirtualDisk> dest);// return invalid mbr if not shrinkable
    shared_ptr<MBRGPT> CreateBackupGPT();

    bool IsValid() {return valid; }
    bool IsGPT() {return gpt; }
    bool IsModified() {return modified; }
    ULONGLONG GetBackupSectorCount();

    DWORD GetTargetPartitionNumberToShrink() { return target_partition_number; }
    UINT64 GetSectorsToShrink() { return shift_sectors; }
    UINT64 GetPartitionStartSectorToShrink() { return partition_start_sector; }
    UINT64 GetFixedSectors(){ return fixed_sectors; }
    shared_ptr<MBRGPT> GetOriginal(){ return org; }
    tstring GetReason(){ return reason; }

};


#endif
