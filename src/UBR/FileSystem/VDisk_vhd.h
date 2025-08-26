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

#ifndef __VHD__H__
#define __VHD__H__

#include <windows.h>
#include "tstring.h"
#include "VDisk_vhd_common.h"

#pragma pack(1)
struct VHD_GEOMETRY
{
    UINT16 Cylinder;
    UINT8 Heads;
    UINT8 SectorsPerTrack;
};

struct VHD_FOOTER
{
    UINT8 Cookie[8];
    UINT32 Features;
    UINT32 FileFormatVersion;
    UINT64 DataOffset;
    UINT32 TimeStamp;
    UINT8 CreatorApplication[4];
    UINT32 CreatorVersion;
    UINT8 CreatorHostOS[4];
    UINT64 OriginalSize;
    UINT64 CurrentSize;
    VHD_GEOMETRY DiskGeometry;
    UINT32 DiskType;
    UINT32 Checksum;
    UINT8 UniqueId[16];
    UINT8 SavedState;
    UINT8 Reserved[427];
};

struct VHD_PARENT_LOCATOR_ENTRY
{
    UINT32 PlatformCode;
    UINT32 PlatformDataSpace;
    UINT32 PlatformDataLength;
    UINT32 Reserved;
    UINT64 PlatformDataOffset;
};

struct VHD_DYNAMIC_DISK_HEADER
{
    UINT8 Cookie[8];
    UINT64 DataOffset;
    UINT64 TableOffset;
    UINT32 HeaderVersion;
    UINT32 MaxTableEntries;
    UINT32 BlockSize;
    UINT32 Checksum;
    UINT8 ParentUniqueId[16];
    UINT32 ParentTimeStamp;
    UINT8 Reserved[4];
    UINT8 ParentUnicodeName[512];
    VHD_PARENT_LOCATOR_ENTRY ParentLocatorEntry[8];
    UINT8 Reserved2[256];
};
#pragma pack()

class VHD : public VHDCommon
{
  protected:
    VHD_FOOTER footer;
    VHD_DYNAMIC_DISK_HEADER header;
    UINT32* bat;
    virtual bool read_footer_header();

    virtual ULONGLONG GetDiskSizeImpl();
    virtual DWORD GetSectorSizeImpl();
    virtual DWORD GetBlockSizeImpl();
    virtual DWORD GetTableEntriesCountImpl();
    virtual UINT32 GetDiskTypeImpl();
    virtual BOOL GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);

  public:
    VHD(const TCHAR* _filename);
    ~VHD();
    virtual tstring GetFileFormat()
    {
        return _T("VHD");
    }

};



#endif
