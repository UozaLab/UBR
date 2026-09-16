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

#include "VDisk_vhdx_wx.h"

VHDX_VSS::VHDX_VSS(wxOutputStream* _out_stream, shared_ptr<PhysicalVSS> vss)
     : VHDX(_T(""), vss->GetBlockSize(), vss->GetDiskSize(), vss->GetSectorSize(), vss->GetSectorSize()),
       out_stream(_out_stream), in_stream(nullptr)
{
    DWORD blocksize = GetBlockSize();
    UINT64 sb_bytes = 0x800000LL * GetSectorSize();
    UINT32 ChunkRatio = (UINT32) (sb_bytes / blocksize);

    for(DWORD i=0; i<GetTableEntriesCount(); i++)
    {
        if(vss->CanSkip(i)) continue;

        UINT32 sb_count = i / ChunkRatio;
        UINT32 index = i + sb_count;
        bat[index].State = VHDX_PAYLOAD_BLOCK_FULLY_PRESENT;
        bat[index].FileOffsetMB = bat_size + 1048576*3 + (UINT64)blocksize * write_block_count;
        bat[index].FileOffsetMB /= 1048576;
        write_block_count++;
    }
    write_header();
}

VHDX_VSS::VHDX_VSS(wxInputStream* _in_stream)
     : VHDX(_T("")),
       out_stream(nullptr), in_stream(_in_stream)
{
}

VHDX_VSS::~VHDX_VSS()
{
    delete out_stream;
    delete in_stream;
}

BOOL VHDX_VSS::write(INT64 pos, const UINT8* buf, size_t buf_len, DWORD* dwNumberOfWritten)
{
    if(out_stream->IsSeekable())
      out_stream->SeekO(pos);

    bool result = out_stream->WriteAll(buf, buf_len);
    *dwNumberOfWritten = out_stream->LastWrite();
    
    return result;
}

BOOL VHDX_VSS::read(INT64 pos, UINT8* buf, size_t buf_len, DWORD* ByteRead, bool from_begining)
{
    if(in_stream->IsSeekable())
      in_stream->SeekI(pos, from_begining ? wxFromStart : wxFromEnd);

    bool result = in_stream->ReadAll(buf, buf_len);
    *ByteRead = in_stream->LastRead();
    return result;
}
