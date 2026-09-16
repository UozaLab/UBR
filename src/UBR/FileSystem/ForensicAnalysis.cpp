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

#include "ForensicAnalysis.h"
#include "VdiskStream.h"
#include "MbrGpt.h"
#include "Fs.h"

shared_ptr<PhysicalDiskInfo> ForensicAnalysis::CreateDiskInfo(shared_ptr<VirtualDisk> vdisk)
{
    shared_ptr<PhysicalDiskInfo> physical_disk(new PhysicalDiskInfo());

    physical_disk->DeviceNumber = 0;
    physical_disk->DiskSize = vdisk->GetDiskSize();
    physical_disk->BytesPerSector = vdisk->GetSectorSize();
    physical_disk->Invalid = false;

    shared_ptr<VirtualDiskStream> stream(new VirtualDiskStream(vdisk));
    MBRGPT mbrgpt(vdisk);
    if(!mbrgpt.IsValid())
    {
        physical_disk->PartitionStyle = PARTITION_STYLE_RAW;
        return physical_disk;
    }


    if(mbrgpt.IsGPT())
    {
        physical_disk->PartitionStyle = PARTITION_STYLE_GPT;
        physical_disk->Gpt.DiskId = mbrgpt.MbrGptInfo.GptHeader.Guid;
        physical_disk->Gpt.MaxPartitionCount = mbrgpt.MbrGptInfo.GptHeader.PartitionEntryCount;
        physical_disk->Gpt.StartingUsableOffset.QuadPart = mbrgpt.MbrGptInfo.GptHeader.FirstUsableLBA;
        physical_disk->Gpt.UsableLength.QuadPart = mbrgpt.MbrGptInfo.GptHeader.LastUsableLBA - mbrgpt.MbrGptInfo.GptHeader.FirstUsableLBA + 1;

        for(unsigned int i=0; i<mbrgpt.MbrGptInfo.GptHeader.PartitionEntryCount; i++)
        {
            GUID UnusedData = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
            if(IsEqualGUID(mbrgpt.MbrGptInfo.GPE[i].PartitionType, UnusedData)) continue;

            PARTITION_INFORMATION_EX pi;
            ZeroMemory(&pi, sizeof(PARTITION_INFORMATION_EX));
            pi.PartitionStyle = PARTITION_STYLE_GPT;
            pi.StartingOffset.QuadPart = mbrgpt.MbrGptInfo.GPE[i].FirstLBA * physical_disk->BytesPerSector;
            pi.PartitionLength.QuadPart = (mbrgpt.MbrGptInfo.GPE[i].LastLBA - mbrgpt.MbrGptInfo.GPE[i].FirstLBA + 1) * physical_disk->BytesPerSector;
            pi.PartitionNumber = i+1;
            pi.Gpt.PartitionType = mbrgpt.MbrGptInfo.GPE[i].PartitionType;
            pi.Gpt.PartitionId = mbrgpt.MbrGptInfo.GPE[i].UniqueId;
            pi.Gpt.Attributes = mbrgpt.MbrGptInfo.GPE[i].AttributeFlags;
            memcpy(&pi.Gpt.Name, &mbrgpt.MbrGptInfo.GPE[i].PartitionName, 72);
            physical_disk->Partitions.push_back(pi);

            //EBD0A0A2-B9E5-4433-87C0-68B6B72699C7 : MicrosoftBasicData
            GUID MicrosoftBasicData = { 0xEBD0A0A2, 0xB9E5, 0x4433, { 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7 } };
            GUID EFISystem = { 0xC12A7328, 0xF81F, 0x11D2, { 0xba, 0x4b, 0x00, 0xa0, 0xc9, 0x3e, 0xc9, 0x3b } };
             
            // volume information
            VolumeInfo vi;
            vi.DriveType = 3;// FIXED
            vi.StorageDeviceNumber.DeviceNumber = 0;
            vi.StorageDeviceNumber.DeviceType = FILE_DEVICE_DISK;
            vi.StorageDeviceNumber.PartitionNumber = i+1;

            if(IsEqualGUID(mbrgpt.MbrGptInfo.GPE[i].PartitionType, MicrosoftBasicData) ||
				IsEqualGUID(mbrgpt.MbrGptInfo.GPE[i].PartitionType, EFISystem))
            {
                if(!vdisk->GetSlow())
                {
                    FileSystem fs(stream);
                    vi.FilesystemInfo = fs.GetFSInfo(mbrgpt.MbrGptInfo.GPE[i].FirstLBA);
                }
                else
                {
                }

            }
            physical_disk->Volumes.push_back(vi);

        }
        physical_disk->PartitionCount = static_cast<DWORD>(physical_disk->Partitions.size());

    }
    else
    {
        physical_disk->PartitionStyle = PARTITION_STYLE_MBR;
        for(unsigned int i=0;i<MBR_PARTITION_ENTRY_COUNT;i++)
        {
            if(mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x00) continue;//empty

            PARTITION_INFORMATION_EX pi;
            ZeroMemory(&pi, sizeof(PARTITION_INFORMATION_EX));
            pi.PartitionStyle = PARTITION_STYLE_MBR;
            pi.StartingOffset.QuadPart = (ULONGLONG)mbrgpt.MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector * physical_disk->BytesPerSector;
            pi.PartitionLength.QuadPart = (ULONGLONG)mbrgpt.MbrGptInfo.PE[i].NumberOfSectors * physical_disk->BytesPerSector;
            pi.PartitionNumber = i+1;
            pi.Mbr.PartitionType = mbrgpt.MbrGptInfo.PE[i].PartitionType;
            pi.Mbr.BootIndicator = (mbrgpt.MbrGptInfo.PE[i].StatusOrPhysicalDrive == 0x80);
            physical_disk->Partitions.push_back(pi);

            // volume information
            VolumeInfo vi;
            vi.DriveType = 3;// FIXED
            vi.StorageDeviceNumber.DeviceNumber = 0;
            vi.StorageDeviceNumber.DeviceType = FILE_DEVICE_DISK;
            vi.StorageDeviceNumber.PartitionNumber = i+1;

            if(mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x07 || //NTFS
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x17 ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x01 || // FAT12
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x11 ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x04 || // FAT16
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x14 ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x06 || // FAT16
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x16 ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x0B || // FAT32
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x1B ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x0C || // FAT32
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x1C ||
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x0E || // FAT16
               mbrgpt.MbrGptInfo.PE[i].PartitionType == 0x1E)
            {
                if(!vdisk->GetSlow())
                {
                    FileSystem fs(stream);
                    vi.FilesystemInfo = fs.GetFSInfo(mbrgpt.MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector);
                }
                else
                {
                    vi.FilesystemInfo.TypeInfo.FileSystemType = FileSystem::PartitionTypeToFSType(mbrgpt.MbrGptInfo.PE[i].PartitionType);
                    vi.FilesystemInfo.TypeInfo.FileSystemName = FileSystem::PartitionTypeToString(mbrgpt.MbrGptInfo.PE[i].PartitionType);
                }
            }
            else
            {
                vi.FilesystemInfo.TypeInfo.FileSystemType = FS_TYPE_UNKNOWN;
                vi.FilesystemInfo.TypeInfo.FileSystemName = FileSystem::PartitionTypeToString(mbrgpt.MbrGptInfo.PE[i].PartitionType);
            }

            physical_disk->Volumes.push_back(vi);
        }
        physical_disk->PartitionCount = static_cast<DWORD>(physical_disk->Partitions.size());
    }


    return physical_disk;
}
