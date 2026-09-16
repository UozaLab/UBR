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
#include "FSInfo.h"
#include <vector>

class VHDCommon : public VirtualDisk
{
public:
    VHDCommon(const tstring& _filename, bool _create_new = false)
        :VirtualDisk(_filename, _create_new), invalid(false), header_read(false),
         flushed(false), write_block_count(0)
    {
    }

protected:
    bool invalid;
    bool header_read;
    bool flushed;
    DWORD write_block_count;
    std::vector<SECTOR_RANGE> empty_sector_ranges;

    virtual BOOL write(INT64 pos, const UINT8* buf, size_t buf_len, DWORD* dwNumberOfWritten)
    {
        LARGE_INTEGER filepointer;
        filepointer.QuadPart = pos;
        SetFilePointerEx(handle, filepointer, NULL, FILE_BEGIN);
        return WriteFile(handle, buf, buf_len, dwNumberOfWritten, NULL);
    }

    virtual BOOL read(INT64 pos, UINT8* buf, size_t buf_len, DWORD* ByteRead, bool from_begining = true)
    {
        LARGE_INTEGER filepointer;
        filepointer.QuadPart = pos;
        SetFilePointerEx(handle, filepointer, NULL, from_begining ? FILE_BEGIN : FILE_END);
        return ReadFile(handle, buf, buf_len, ByteRead, 0);
    }

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

    virtual bool read_footer_header() = 0;
    virtual bool write_header() = 0;
    virtual bool write_footer() = 0;

    virtual ULONGLONG GetDiskSizeImpl() = 0;
    virtual DWORD GetSectorSizeImpl() = 0;
    virtual DWORD GetBlockSizeImpl() = 0;
    virtual DWORD GetTableEntriesCountImpl() = 0;
    virtual UINT32 GetDiskTypeImpl() = 0;
    virtual BOOL GetBlockDataImpl(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip) = 0;
    virtual BOOL SetBlockDataImpl(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite) = 0;
    virtual void FlushImpl()
    {
        write_header();
        write_footer();
        flushed = true;
    }


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

    virtual BOOL SetBlockData(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite)
    {
        guard();
        if(!create_new) return FALSE;
        return SetBlockDataImpl(blockdata, blockindex, ByteWrite);
    }

    virtual void Flush()
    {
        guard();
        FlushImpl();
    }

    virtual DWORD GetActualBlockCount()
    {
        return write_block_count;
    }

    virtual BOOL IsValid()
    {
        guard();
        if(invalid) return FALSE;
        return TRUE;
    }

};


#endif
