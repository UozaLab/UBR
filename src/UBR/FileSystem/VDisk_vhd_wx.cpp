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

#include "VDisk_vhd_wx.h"

VHD_VSS::VHD_VSS(wxOutputStream* _out_stream, shared_ptr<PhysicalVSS> vss)
     : VHD(_T(""), vss->GetBlockSize(), vss->GetDiskSize()),
       out_stream(_out_stream), in_stream(nullptr)
{
    DWORD blocksize = GetBlockSize();
    DWORD sector_size = GetSectorSize();
    const DWORD sector_counts_in_1byte_bitmap = 8;
    UINT32 block_bitmap_sector_count = (blocksize /sector_size) / sector_counts_in_1byte_bitmap;
    block_bitmap_sector_count = (block_bitmap_sector_count + (sector_size - 1)) / sector_size;

    UINT64 batsize = (UINT64)bat_count * sizeof(UINT32);
    unsigned int header_footer_length = sizeof(VHD_FOOTER) + sizeof(VHD_DYNAMIC_DISK_HEADER) + (unsigned int) batsize;
    for(DWORD i=0; i<GetTableEntriesCount(); i++)
    {
        if(vss->CanSkip(i)) continue;

        UINT64 bat_indicate_offset_sector = (header_footer_length + (UINT64)write_block_count * (blocksize + block_bitmap_sector_count * sector_size)) / sector_size;
        bat[i] = _byteswap_ulong((UINT32) bat_indicate_offset_sector);
        write_block_count++;
    }
    write_header();
}

VHD_VSS::VHD_VSS(wxInputStream* _in_stream)
     : VHD(_T("")),
       out_stream(nullptr), in_stream(_in_stream)
{
}


VHD_VSS::~VHD_VSS()
{
    delete out_stream;
    delete in_stream;
}

BOOL VHD_VSS::write(INT64 pos, const UINT8* buf, size_t buf_len, DWORD* dwNumberOfWritten)
{
    if(out_stream->IsSeekable())
      out_stream->SeekO(pos);

    bool result = out_stream->WriteAll(buf, buf_len);
    *dwNumberOfWritten = out_stream->LastWrite();
    
    return result;
}

BOOL VHD_VSS::read(INT64 pos, UINT8* buf, size_t buf_len, DWORD* ByteRead, bool from_begining)
{
    if(in_stream->IsSeekable())
      in_stream->SeekI(pos, from_begining ? wxFromStart : wxFromEnd);

    bool result = in_stream->ReadAll(buf, buf_len);
    *ByteRead = in_stream->LastRead();
    return result;
}

