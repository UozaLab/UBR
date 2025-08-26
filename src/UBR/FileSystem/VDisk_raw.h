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

#ifndef __RAW__H__
#define __RAW__H__

#include <windows.h>
#include "tstring.h"
#include "VDisk.h"

class RAW : public VirtualDisk
{
public:
    RAW(const tstring& _filename)
        :VirtualDisk(_filename), invalid(false)
    {
    }

protected:
    virtual void guard();
    bool invalid;

public:
    virtual tstring GetFileFormat()
    {
        return _T("RAW");
    }
    virtual BOOL IsValid();

    virtual ULONGLONG GetDiskSize();
    virtual DWORD GetSectorSize();
    virtual DWORD GetBlockSize();
    virtual DWORD GetTableEntriesCount();
    virtual UINT32 GetDiskType();
    virtual BOOL GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);
};

#endif
