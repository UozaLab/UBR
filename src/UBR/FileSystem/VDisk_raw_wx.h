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

#ifndef __RAW_WX_H__
#define __RAW_WX_H__

#include <windows.h>
#include "tstring.h"
#include "smart_ptr.h"
#include "VDisk_raw.h"
#include "VDisk_vss.h"
#include <wx/stream.h>

class RAW_VSS : public RAW
{
  protected:
    wxOutputStream* out_stream;
    wxInputStream* in_stream;

  public:
    RAW_VSS(wxOutputStream* _out_stream, shared_ptr<PhysicalVSS> _vss);
    RAW_VSS(wxInputStream* _in_stream);
    ~RAW_VSS();
    virtual BOOL SetBlockData(const UINT8* blockdata, DWORD blockindex, DWORD* ByteWrite);
    virtual BOOL GetBlockData(UINT8* blockdata, DWORD blockindex, DWORD* ByteRead, bool* can_skip);
};

#endif
