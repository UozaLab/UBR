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

#ifndef __VHD_WX_H__
#define __VHD_WX_H__

#include <windows.h>
#include "smart_ptr.h"
#include "VDisk_vhd.h"
#include "VDisk_vss.h"
#include <wx/stream.h>

class VHD_VSS : public VHD
{
protected:
    shared_ptr<PhysicalVSS> vss;
    wxOutputStream* out_stream;
    wxInputStream* in_stream;

    virtual BOOL write(INT64 pos, const UINT8* buf, size_t buf_len, DWORD* dwNumberOfWritten);
    virtual BOOL read(INT64 pos, UINT8* buf, size_t buf_len, DWORD* ByteRead, bool from_begining = true);
    virtual void FlushImpl()
    {
        write_footer();
        flushed = true;
    }
public:
    VHD_VSS(wxOutputStream* _out_stream, shared_ptr<PhysicalVSS> vss);
    VHD_VSS(wxInputStream* _in_stream);
    ~VHD_VSS();
};

#endif
