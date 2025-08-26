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

#ifndef __FAT__H__
#define __FAT__H__

#include <windows.h>
#include "smart_ptr.h"
#include "VdiskStream.h"
#include "FsInfo.h"
#include "tstring.h"
#include "MbrGpt.h"

#pragma pack(1)
struct FAT_BPB {
    UINT16 BytesPerLogicalSector;
    UINT8 LogicalSectorsPerCluster;
    UINT16 ReservedLogicalSectors;
    UINT8 NumberOfFileAllocationTables;
    UINT16 MaximumNumberOfRootDirectoryEntries;
    UINT16 TotalLogicalSectors;
    UINT8 MediaDescriptor;
    UINT16 LogicalSectorsPerFAT;
    UINT16 PhysicalSectorsPerTrack;
    UINT16 NumberOfHeads;
    UINT32 CountOfHiddenSectors;
    UINT32 TotalLogicalSectorsIncludingHiddenSectors;
};

struct FAT_EBPB_FAT12_16 {
    UINT8 PhysicalDriveNumber;
    UINT8 Reserved;
    UINT8 ExtendedBootSignature;
    UINT32 VolumeId;
    UINT8 PartitionVolumeLabel[11];
    UINT8 FileSystemType[8];
    UINT8 BootCode[448];
};

struct FAT_EBPB_FAT32 {
    UINT32 LogicalSectorsPerFAT;
    UINT16 MirroringFlags;
    UINT16 Version;
    UINT32 ClusterNumberOfRootDirectoryStart;
    UINT16 LogicalSectorNumberOfFSInformationSector;
    UINT16 FirstLogicalSectorNumberOfBackupBootSectors;
    UINT8 Reserved[12];
    UINT8 PhysicalDriveNumber;
    UINT8 Reserved2;
    UINT8 ExtendedBootSignature;
    UINT32 VolumeId;
    UINT8 VolumeLabel[11];
    UINT8 FileSystemType[8];
    UINT8 BootCode[420];
};

struct FAT_BS {
    UINT8 JumpInstruction[3];
    UINT8 OemName[8];
    FAT_BPB Bpb;
    union {
        FAT_EBPB_FAT12_16 Fat12_16;
        FAT_EBPB_FAT32 Fat32;
    } Ebp;
    UINT8 Sign[2];
};


#define DIR_FAT_ATTR_READ_ONLY 0x01
#define DIR_FAT_ATTR_HIDDEN 0x02
#define DIR_FAT_ATTR_SYSTEM 0x04
#define DIR_FAT_ATTR_VOLUME_ID 0x08
#define DIR_FAT_ATTR_DIRECTORY 0x10
#define DIR_FAT_ATTR_ARCHIVE 0x20
#define DIR_FAT_ATTR_LONG_FILE_NAME 0x0F

struct FAT_DIRECTORY_ENTRY {
    UINT8 ShortFileName[11];
    UINT8 FileAttributes;
    UINT8 Reserved;
    UINT8 CreationTimeFineResolution;
    UINT16 CreationTime;
    UINT16 CreationDate;
    UINT16 LastAccessDate;
    UINT16 FirstClusterUpper;
    UINT16 LastModifiedTime;
    UINT16 LastModifiedDate;
    UINT16 FirstClusterLower;
    UINT32 FileSize;
};

#pragma pack()


class FATExtractor
{
protected:
    UINT64 partition_start_sector;

public:
    FATExtractor(UINT64 _partition_start_sector):
      partition_start_sector(_partition_start_sector) {}

    virtual UINT64 TotalSectors() = 0;
    virtual UINT32 FirstRootDirectoryEntrySectorIndex() = 0;
    virtual UINT64 StartSector() = 0;
    virtual UINT32 SectorsPerFat() = 0;
    virtual UINT16 BytesPerSector() = 0;
    virtual UINT8 SectorsPerCluster() = 0;
    virtual FS_TYPE GetFSType() = 0;
    virtual tstring FindVolumeLabel(const UINT8* dir_entry_data) = 0;
    virtual void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors) = 0;
    virtual VOLUME_SIZE_INFO GetVolumeSize(shared_ptr<VirtualDiskStream> stream) = 0;

    tstring GetVolumeName(shared_ptr<VirtualDiskStream> stream);
};

class FATExtractorUtility
{
public:
    static int pow(int x, int n);
    static UINT32 GetDataClusters(const FAT_BS* bs);
    static UINT32 GetDataStartSector(const FAT_BS* bs);
    static UINT32 GetRootDirectoryEntrySector(const FAT_BS* bs);
};

class FATExtractor_12 : public FATExtractor
{
protected:
    const FAT_BS* boot_sector;
public:
    FATExtractor_12(UINT64 partition_start_sector, const UINT8* start_sector_data)
        : FATExtractor(partition_start_sector),
          boot_sector((FAT_BS*) start_sector_data) {}
    
    UINT64 TotalSectors();
    UINT64 StartSector();
    UINT32 SectorsPerFat();
    UINT16 BytesPerSector();
    UINT8 SectorsPerCluster();
    tstring FindVolumeLabel(const UINT8* dir_entry_data);

    virtual UINT32 FirstRootDirectoryEntrySectorIndex();
    virtual FS_TYPE GetFSType();
    virtual void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors);
    virtual VOLUME_SIZE_INFO GetVolumeSize(shared_ptr<VirtualDiskStream> stream);
};

class FATExtractor_16 : public FATExtractor_12
{
public:
    FATExtractor_16(UINT64 partition_start_sector, const UINT8* start_sector_data)
        : FATExtractor_12(partition_start_sector, start_sector_data) {}

    virtual FS_TYPE GetFSType();
    virtual void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors);
    virtual VOLUME_SIZE_INFO GetVolumeSize(shared_ptr<VirtualDiskStream> stream);
};

class FATExtractor_32 : public FATExtractor_16
{
public:
    FATExtractor_32(UINT64 partition_start_sector, const UINT8* start_sector_data)
        : FATExtractor_16(partition_start_sector, start_sector_data) {}

    UINT32 FirstRootDirectoryEntrySectorIndex();
    FS_TYPE GetFSType();
    void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, UINT64 sectors_to_shrink, UINT64 fixed_sectors);
    virtual VOLUME_SIZE_INFO GetVolumeSize(shared_ptr<VirtualDiskStream> stream);
};

class FATExtractorFactory
{
public:
    static shared_ptr<FATExtractor> Create(UINT64 partition_start_sector, const UINT8* start_sector_data)
    {
        UINT32 clusters = FATExtractorUtility::GetDataClusters((FAT_BS*)start_sector_data);
        if(clusters <= 4085)
            return shared_ptr<FATExtractor>(new FATExtractor_12(partition_start_sector, start_sector_data));
        if(clusters <= 65525)
            return shared_ptr<FATExtractor>(new FATExtractor_16(partition_start_sector, start_sector_data));
        return shared_ptr<FATExtractor>(new FATExtractor_32(partition_start_sector, start_sector_data));
    }
};

class FAT
{
protected:
    shared_ptr<VirtualDiskStream> stream;

public:
    FAT(shared_ptr<VirtualDiskStream> _stream);
    FSInfo GetFSInfo(UINT64 partition_start_sector);
    void GetShrinkedSectorData(shared_ptr<DiskUpdaterCollection> data_collection, shared_ptr<MBRGPT> mbrgpt);
};

#endif
