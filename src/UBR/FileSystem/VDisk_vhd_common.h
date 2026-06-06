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

#ifndef __VHD_COMMON_H__
#define __VHD_COMMON_H__

#include <windows.h>
#include "tstring.h"
#include "VDisk.h"

class VHDCommon : public VirtualDisk
{
public:
    VHDCommon(const tstring& _filename)
        :VirtualDisk(_filename), invalid(false), header_read(false)
    {
    }

protected:
    bool invalid;
    bool header_read;
    virtual bool read_footer_header() = 0;
    virtual void guard()
    {
        if(!opened)
        {
            if(!open())
            {
                invalid = true;
            }
            else if(!read_footer_header())
            {
                invalid = true;
            }

        }
    }
    virtual BOOL IsValid()
    {
        guard();
        if(invalid) return FALSE;
        return TRUE;
    }

    virtual ULONGLONG GetDiskSizeImpl() = 0;
    virtual DWORD GetSectorSizeImpl() = 0;
    virtual DWORD GetBlockSizeImpl() = 0;
    virtual DWORD GetTableEntriesCountImpl() = 0;
    virtual UINT32 GetDiskTypeImpl() = 0;
    virtual BOOL GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip) = 0;


public:
    virtual ULONGLONG GetDiskSize()
    {
        guard();
        return GetDiskSizeImpl();
    }
    virtual DWORD GetSectorSize()
    {
        guard();
        return GetSectorSizeImpl();
    }
    virtual DWORD GetBlockSize()
    {
        guard();
        return GetBlockSizeImpl();
    }
    virtual DWORD GetTableEntriesCount()
    {
        guard();
        return GetTableEntriesCountImpl();
    }
    virtual UINT32 GetDiskType()
    {
        guard();
        return GetDiskTypeImpl();
    }
    virtual BOOL GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
    {
        guard();
        return GetBlockDataImpl(blockdata, blockindex, ByteRead, can_skip);
    }


};


#endif
