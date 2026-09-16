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

#include "VDiskFactory.h"
#include "VDisk_vhd_wx.h"
#include "VDisk_vhdx_wx.h"
#include "VDisk_raw_wx.h"
#include "LZ4Stream.h"
#include <wx/wfstream.h>

shared_ptr<VirtualDisk> VirtualDiskFactory::Create(const wxString& filename)
{
    FileType filetype = filename.Lower().EndsWith("vhd") ? FILE_TYPE_VHD
                      : filename.Lower().EndsWith("vhd.lz4") ? FILE_TYPE_VHD
                      : filename.Lower().EndsWith("vhdx") ? FILE_TYPE_VHDX
                      : filename.Lower().EndsWith("vhdx.lz4") ? FILE_TYPE_VHDX
                      : filename.Lower().EndsWith("raw") ? FILE_TYPE_RAW
                      : filename.Lower().EndsWith("raw.lz4") ? FILE_TYPE_RAW
                      : FILE_TYPE_RAW;

    bool compressed = filename.Lower().EndsWith("vhd.lz4") ||
                      filename.Lower().EndsWith("vhdx.lz4") ||
                      filename.Lower().EndsWith("raw.lz4");

    wxInputStream* in_stream = compressed ? (wxInputStream*) new LZ4InputSeekableStream(new wxFileInputStream(filename))
                                          : (wxInputStream*) new wxFileInputStream(filename);

    shared_ptr<VirtualDisk> vd(filetype == FILE_TYPE_VHD ? (VirtualDisk*) new VHD_VSS(in_stream)
                             : filetype == FILE_TYPE_VHDX ? (VirtualDisk*) new VHDX_VSS(in_stream)
                             : (VirtualDisk*) new RAW_VSS(in_stream));
    vd->SetSlow(compressed);
    return vd;
}

