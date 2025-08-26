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

#include "MbrGpt.h"
#include "VdiskStream.h"
#include "PhysicalDiskInfo.h"
#include "ForensicAnalysis.h"
#include "Crc32.h"

MBRGPT::MBRGPT(const MBRGPT& rhs)
: valid(rhs.valid), gpt(rhs.gpt), modified(rhs.modified), 
  target_partition_number(rhs.target_partition_number), shift_sectors(rhs.shift_sectors), 
  partition_start_sector(rhs.partition_start_sector), fixed_sectors(rhs.fixed_sectors), org(rhs.org)
{
    memcpy(&MbrGptInfo, &rhs.MbrGptInfo, sizeof(MBR_GPT_INFO));
    memcpy(&MbrGptSector, &rhs.MbrGptSector, sizeof(MbrGptSector));
}

MBRGPT& MBRGPT::operator=(const MBRGPT& rhs)
{
    memcpy(&MbrGptInfo, &rhs.MbrGptInfo, sizeof(MBR_GPT_INFO));
    memcpy(&MbrGptSector, &rhs.MbrGptSector, sizeof(MbrGptSector));

    valid = rhs.valid;
    gpt = rhs.gpt;
    modified = rhs.modified;
    target_partition_number = rhs.target_partition_number;
    shift_sectors = rhs.shift_sectors;
    partition_start_sector = rhs.partition_start_sector;
    fixed_sectors = rhs.fixed_sectors;
    org = rhs.org;

    return *this;
}

MBRGPT::MBRGPT(shared_ptr<VirtualDisk> _vdisk)
: vdisk(_vdisk), valid(false), modified(false), target_partition_number(0), shift_sectors(0ULL), partition_start_sector(0ULL)
{
    ZeroMemory(&MbrGptInfo, sizeof(MBR_GPT_INFO));
    VirtualDiskStream stream(vdisk);

    UINT8* mbr_gpt_sector = stream.Read(0, 34);// 34 = mbr+GPTheader+GPTtable*32
    memcpy(&MbrGptSector, mbr_gpt_sector, sizeof(MbrGptSector));
    delete [] mbr_gpt_sector;

    valid = (MbrGptSector[510] == 0x55 && MbrGptSector[511] == 0xAA);
    if(!valid)
    {
        return;
    }

    for (int i = 0; i < MBR_PARTITION_ENTRY_COUNT; i++)
    {
        memcpy(&MbrGptInfo.PE[i],
               MbrGptSector + MBR_PARTITION_ENTRY_START_ADDRESS + i * sizeof(MBR_PARTITION_ENTRY),
               sizeof(MBR_PARTITION_ENTRY));
    }
    gpt = (MbrGptInfo.PE[0].PartitionType == 0xEE/*GPT*/);
    if(!gpt)
    {
        return;
    }

    memcpy(&MbrGptInfo.GptHeader, MbrGptSector + GPT_HEADER_START_ADDRESS, sizeof(GPT_HEADER));
    for (int i = 0; i < GPT_PARTITION_ENTRY_COUNT; i++)
    {
        memcpy(&MbrGptInfo.GPE[i],
               MbrGptSector + 512*MbrGptInfo.GptHeader.StartingLBAOfPartitionEntry + i * sizeof(GPT_PARTITION_ENTRY),
               sizeof(GPT_PARTITION_ENTRY));
    }
}

CHS MBRGPT::lba2chs(int lba)
{
    const int HPC = 255;
    const int SPT = 63;

    CHS chs;
    chs.Cylinder = lba / (HPC*SPT);
    chs.Head = (lba / SPT) % HPC;
    chs.Sector = (lba % SPT) + 1;

    return chs;
}

void MBRGPT::update_chs()
{
    for(int i=0; i<4; i++)
    {
        if(MbrGptInfo.PE[i].PartitionType == 0x00) continue;
        CHS chs = lba2chs(MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector);
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[0] = chs.Head;
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[1] = chs.Sector & 0x3F;
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[1] |= (UINT8) ((chs.Cylinder >> 2) & 0x00C0);
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[2] = (UINT8) (chs.Cylinder & 0x00FF);

        chs = lba2chs(MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector + MbrGptInfo.PE[i].NumberOfSectors -1);
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[0] = chs.Head;
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[1] = chs.Sector & 0x3F;
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[1] |= (UINT8) ((chs.Cylinder >> 2) & 0x00C0);
        MbrGptInfo.PE[i].CHSAddressOfFirstAbsoluteSector[2] = (UINT8) (chs.Cylinder & 0x00FF);

    }
}

shared_ptr<MBRGPT> MBRGPT::CreateShrinked(shared_ptr<VirtualDisk> dest)
{
    org.reset(new MBRGPT(*this));
    CRC32 crc32(0xedb88320);
    shared_ptr<MBRGPT> mbrgpt_mod(new MBRGPT(*this));
    if(!IsValid()) return mbrgpt_mod;

    ULONGLONG dest_size = dest->GetDiskSize();
    if(dest_size >= vdisk->GetDiskSize()) return mbrgpt_mod;

    shared_ptr<PhysicalDiskInfo> pd = ForensicAnalysis::CreateDiskInfo(vdisk);

    LARGE_INTEGER biggest_size;
    biggest_size.QuadPart = 0;

    target_partition_number = 0;
    ULONGLONG src_size = 0;
    for(std::vector<PARTITION_INFORMATION_EX>::iterator itr = pd->Partitions.begin(),
        itr_end = pd->Partitions.end(); itr != itr_end; itr++)
    {
        PARTITION_INFORMATION_EX pi = *itr;
        if(pi.PartitionLength.QuadPart > biggest_size.QuadPart)
        {
            VolumeInfo vi = PhysicalDiskUtil::FindVolumeInfo(pd, pi.PartitionNumber);
            if(!vi.Invalid && vi.FilesystemInfo.SizeInfo.SizeCalculated)
            {
                biggest_size = pi.PartitionLength;
                target_partition_number = pi.PartitionNumber;
            }
        }
        if(src_size < (ULONGLONG) (pi.StartingOffset.QuadPart + pi.PartitionLength.QuadPart))
        {
            src_size = pi.StartingOffset.QuadPart + pi.PartitionLength.QuadPart;
        }
    }
    if(dest_size >=  (gpt ? src_size + pd->BytesPerSector/*Backup GPT Header*/ : src_size))
    {
        return mbrgpt_mod;
    }
    if(target_partition_number == 0)
    {
        mbrgpt_mod->valid = false;
        mbrgpt_mod->reason = _T("Illegal partition number");
        return mbrgpt_mod;
    }

    VolumeInfo vi = PhysicalDiskUtil::FindVolumeInfo(pd, target_partition_number);
    DWORD sector_size = vi.FilesystemInfo.SizeInfo.BytesPerSector;
    fixed_sectors = vi.FilesystemInfo.SizeInfo.Fixed;
    fixed_sectors /= sector_size;
    mbrgpt_mod->fixed_sectors = fixed_sectors;
    ULONGLONG shrinkable_size = vi.FilesystemInfo.SizeInfo.Total - vi.FilesystemInfo.SizeInfo.Fixed;
    ULONGLONG byte_per_cluster = vi.FilesystemInfo.SizeInfo.SectorsPerCluster;
    byte_per_cluster *= vi.FilesystemInfo.SizeInfo.BytesPerSector;
    if(shrinkable_size <= byte_per_cluster * 10)
    {
        mbrgpt_mod->valid = false;
        mbrgpt_mod->reason = _T("Source disk contains too many data");
        return mbrgpt_mod;
    }
    shrinkable_size -= byte_per_cluster * 10;// margin : 10 clusters
    ULONGLONG shrinkable_block_num = shrinkable_size / byte_per_cluster;
    shrinkable_size = shrinkable_block_num * byte_per_cluster;

    ULONGLONG shift_size = (src_size - dest_size);
    ULONGLONG shift_block_num = shift_size / byte_per_cluster;
    if(shift_size % byte_per_cluster == 0)
        shift_size = shift_block_num * byte_per_cluster;
    else
        shift_size = (shift_block_num + 1) * byte_per_cluster;

    if(shift_size > shrinkable_size)
    {
        mbrgpt_mod->valid = false;
        tostringstream oss;
        ULONGLONG short_size = shift_size - shrinkable_size;
        oss << "Destination disk is too small (" << Unit::HumanReadable(short_size) << " short)";
        mbrgpt_mod->reason = oss.str();
        return mbrgpt_mod;
    }

    mbrgpt_mod->target_partition_number = target_partition_number;
    if(gpt)
    {
        // GPT
        partition_start_sector = MbrGptInfo.GPE[target_partition_number-1].FirstLBA;
        mbrgpt_mod->partition_start_sector = mbrgpt_mod->MbrGptInfo.GPE[target_partition_number-1].FirstLBA;
        shift_sectors = shift_size;
        shift_sectors /= sector_size;
        mbrgpt_mod->shift_sectors = shift_sectors;
        mbrgpt_mod->MbrGptInfo.GPE[target_partition_number-1].LastLBA -= shift_sectors;

        for(DWORD i=target_partition_number; i < mbrgpt_mod->MbrGptInfo.GptHeader.PartitionEntryCount; i++)
        {
            GUID UnusedData = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
            if(IsEqualGUID(mbrgpt_mod->MbrGptInfo.GPE[i].PartitionType, UnusedData)) continue;

            mbrgpt_mod->MbrGptInfo.GPE[i].FirstLBA -= shift_sectors;
            mbrgpt_mod->MbrGptInfo.GPE[i].LastLBA -= shift_sectors;
        }

        mbrgpt_mod->MbrGptInfo.GptHeader.BackupLBA = (dest->GetDiskSize() / sector_size) - 1;
        
        // calc table_len
        DWORD entries_per_sector = sector_size / mbrgpt_mod->MbrGptInfo.GptHeader.PartitionEntrySize;
        DWORD sectors_partition_entries = mbrgpt_mod->MbrGptInfo.GptHeader.PartitionEntryCount / entries_per_sector;
        mbrgpt_mod->MbrGptInfo.GptHeader.LastUsableLBA = (dest->GetDiskSize() / sector_size) - sectors_partition_entries - 2;

        UINT32 crc = crc32.calculate((UINT8*) &mbrgpt_mod->MbrGptInfo.GPE, sizeof(GPT_PARTITION_ENTRY)*mbrgpt_mod->MbrGptInfo.GptHeader.PartitionEntryCount);
        mbrgpt_mod->MbrGptInfo.GptHeader.PartitionEntryChecksum = crc;

        GPT_HEADER header;
        memcpy(&header, &mbrgpt_mod->MbrGptInfo.GptHeader, sizeof(GPT_HEADER));
        header.HeaderChecksum = 0;
        crc = crc32.calculate((UINT8*) &header, sizeof(GPT_HEADER));
        mbrgpt_mod->MbrGptInfo.GptHeader.HeaderChecksum = crc;
    }
    else
    {
        // MBR
        partition_start_sector = MbrGptInfo.PE[target_partition_number-1].LBAOfFirstAbsoluteSector;
        mbrgpt_mod->partition_start_sector = mbrgpt_mod->MbrGptInfo.PE[target_partition_number-1].LBAOfFirstAbsoluteSector;
        shift_sectors = shift_size;
        shift_sectors /= sector_size;
        mbrgpt_mod->shift_sectors = shift_sectors;
        mbrgpt_mod->MbrGptInfo.PE[target_partition_number-1].NumberOfSectors -= (UINT32)shift_sectors;

        for(DWORD i=target_partition_number; i < 4; i++)
        {
            if(mbrgpt_mod->MbrGptInfo.PE[i].PartitionType == 0x00) continue;
            mbrgpt_mod->MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector -= (UINT32)shift_sectors;
        }
        update_chs();
    }

    mbrgpt_mod->modified = true;
    mbrgpt_mod->update_sector();
    return mbrgpt_mod;
}

ULONGLONG MBRGPT::GetBackupSectorCount()
{
    ULONGLONG backup_target_sector_count;

    if(IsGPT())
    {
        backup_target_sector_count = 34;// MBR(1) + GPTHeader(1) + GPTTable(32)
        for(DWORD i=0; i < MbrGptInfo.GptHeader.PartitionEntryCount; i++)
        {
            GUID UnusedData = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
            if(IsEqualGUID(MbrGptInfo.GPE[i].PartitionType, UnusedData)) continue;

            if(MbrGptInfo.GPE[i].FirstLBA < backup_target_sector_count)
            {
                backup_target_sector_count = MbrGptInfo.GPE[i].FirstLBA;
            }
        }
    }
    else
    {
        backup_target_sector_count = 63;// sector0-62
        for(DWORD i=0; i < 4; i++)
        {
            if(MbrGptInfo.PE[i].PartitionType == 0x00) continue;

            if(MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector < backup_target_sector_count)
            {
                backup_target_sector_count = MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector;
            }
        }
    }
    return backup_target_sector_count;
}

void MBRGPT::update_sector()
{
    if(IsGPT())
    {
        memcpy(MbrGptSector + GPT_HEADER_START_ADDRESS, &MbrGptInfo.GptHeader, sizeof(GPT_HEADER));
        for (int i = 0; i < GPT_PARTITION_ENTRY_COUNT; i++)
        {
            memcpy(MbrGptSector + 512 * 2/*skip mbr and header*/ + i * sizeof(GPT_PARTITION_ENTRY),
                   &MbrGptInfo.GPE[i],
                   sizeof(GPT_PARTITION_ENTRY));
        }
    }
    else
    {
        for (int i = 0; i < MBR_PARTITION_ENTRY_COUNT; i++)
        {
            memcpy(MbrGptSector + MBR_PARTITION_ENTRY_START_ADDRESS + i * sizeof(MBR_PARTITION_ENTRY),
                   &MbrGptInfo.PE[i],
                   sizeof(MBR_PARTITION_ENTRY));
        }
    }
}

shared_ptr<MBRGPT> MBRGPT::CreateBackupGPT()
{
    CRC32 crc32(0xedb88320);
    shared_ptr<MBRGPT> mbrgpt_mod(new MBRGPT(*this));
    if(!IsValid()) return mbrgpt_mod;

    mbrgpt_mod->MbrGptInfo.GptHeader.BackupLBA = MbrGptInfo.GptHeader.CurrentLBA;
    mbrgpt_mod->MbrGptInfo.GptHeader.CurrentLBA = MbrGptInfo.GptHeader.BackupLBA;
    mbrgpt_mod->MbrGptInfo.GptHeader.StartingLBAOfPartitionEntry = mbrgpt_mod->MbrGptInfo.GptHeader.CurrentLBA - 32;

    GPT_HEADER header;
    memcpy(&header, &mbrgpt_mod->MbrGptInfo.GptHeader, sizeof(GPT_HEADER));
    header.HeaderChecksum = 0;
    UINT32 crc = crc32.calculate((UINT8*) &header, sizeof(GPT_HEADER));
    mbrgpt_mod->MbrGptInfo.GptHeader.HeaderChecksum = crc;

    mbrgpt_mod->modified = true;
    mbrgpt_mod->update_sector();
    return mbrgpt_mod;
}

