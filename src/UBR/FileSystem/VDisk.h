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

#ifndef __VDISK_H__
#define __VDISK_H__

#include <windows.h>
#include <sstream>
#include "tstring.h"

class VirtualDisk
{
  protected:
    HANDLE handle;
    tstring filename;
    int disk_number;
    bool opened;
    bool physical;
    bool large_scale_mode;
    bool open()
    {
        if(opened)
          return true;

        handle = CreateFile(filename.c_str(),
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

        if(handle == INVALID_HANDLE_VALUE)
          return false;

        opened = true;
        return true;
    }
    void close()
    {
        if(!opened) return;
        CloseHandle(handle);
        opened = false;
        handle = NULL;
    }

  public:
    VirtualDisk(const tstring& _filename)
         : handle(NULL), filename(_filename), opened(false), physical(false), large_scale_mode(false)
    {
    }
    VirtualDisk(int _disk_number)
         : handle(NULL), opened(false), physical(true), disk_number(_disk_number), large_scale_mode(false)
    {
        tostringstream oss;
        oss << "\\\\.\\PhysicalDrive" << _disk_number;
        filename = oss.str();
    }
    virtual ~VirtualDisk()
    {
        if(opened)
            close();
    }
    virtual void SetLargeScaleMode(bool tf)
    {
        large_scale_mode = tf;
    }
    virtual tstring GetFileFormat() = 0;
    virtual BOOL IsValid() = 0;

    virtual ULONGLONG GetDiskSize() = 0;
    virtual DWORD GetSectorSize() = 0;
    virtual DWORD GetBlockSize() = 0;
    virtual DWORD GetTableEntriesCount() = 0;
    virtual UINT32 GetDiskType() = 0;
    virtual BOOL GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip) = 0;
};



#endif
