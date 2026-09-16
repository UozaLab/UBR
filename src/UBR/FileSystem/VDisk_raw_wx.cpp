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

#include "VDisk_raw_wx.h"

RAW_VSS::RAW_VSS(wxOutputStream* _out_stream, shared_ptr<PhysicalVSS> _vss)
     : RAW(_T(""), _vss->GetBlockSize(), _vss->GetDiskSize()),
       out_stream(_out_stream), in_stream(nullptr)
{
}

RAW_VSS::RAW_VSS(wxInputStream* _in_stream)
     : RAW(_T("")),
       out_stream(nullptr), in_stream(_in_stream)
{
    disk_size = in_stream->GetLength();
}

RAW_VSS::~RAW_VSS()
{
    delete out_stream;
    delete in_stream;
}

BOOL RAW_VSS::SetBlockData(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite)
{
    guard();
    if(invalid) return FALSE;

    DWORD blocksize = GetBlockSize();
    ULONGLONG disksize = GetDiskSize();
    UINT64 pos = (UINT64)blocksize * blockindex;

    if(out_stream->IsSeekable())
      out_stream->SeekO(pos);

    if(pos + blocksize >  disksize)
    {
        blocksize -= (DWORD) (pos + blocksize - disksize);
    }
    
    bool result = out_stream->WriteAll(blockdata, blocksize);
    *ByteWrite = out_stream->LastWrite();
    
    return result;
}

BOOL RAW_VSS::GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip)
{
    guard();
    if(invalid) return FALSE;

    *can_skip = false;
    *ByteRead = 0;

    ULONGLONG disksize = GetDiskSize();
    if(disksize != 0)
    {
        DWORD blockcount = GetTableEntriesCount();
        if(blockindex < 0 || blockindex >= blockcount)
          return FALSE;
    }

    DWORD blocksize = GetBlockSize();
    UINT64 pos = (UINT64)blocksize * blockindex;

    if(disksize != 0)
    {
        if(pos + blocksize >  disksize)
        {
            blocksize -= (DWORD) (pos + blocksize - disksize);
        }
    }
    
    if(in_stream->IsSeekable())
      in_stream->SeekI(pos);
    
    bool ret = in_stream->ReadAll(blockdata, blocksize);
    *ByteRead = in_stream->LastRead();

    return ret;
}
