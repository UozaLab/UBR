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

#include "Compress.h"
#include "tstring.h"
#include "Misc.h"
#include "MiscWx.h"
#include "LZ4Stream.h"
#include <wx/wfstream.h>
#include <wx/filename.h>

CompressWorker::CompressWorker(wxEvtHandler* event_handler, const CompressData* _compress_data)
: CustomThread(event_handler),
  compress_data(_compress_data)
{
}

void* CompressWorker::Entry()
{
    SetStateStart();

    const int buff_size = 1024*1024;
    UINT8* buff = new UINT8[buff_size];

    wxFileName src_path(compress_data->src_path);
    wxFileName dst_path(compress_data->dst_path);
    unsigned long long src_size = src_path.GetSize().ToULong();
    unsigned long long dst_size = 0;
    unsigned long long sum_bytes = 0;
    int progress = 0;
    
    LZ4OutputStream out_stream(new wxFileOutputStream(dst_path.GetFullPath()), src_size);
    wxFileInputStream in_stream(src_path.GetFullPath());
    if(!in_stream.Ok())
    {
        wxQueueEvent(event_handler, new ErrorEvent(wxString::Format("Cannot open %s", src_path.GetFullName())));
        goto END;
    }

    wxQueueEvent(event_handler, new MsgEvent("Start compressing"));
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Src  : %s", src_path.GetFullPath())));
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Dest : %s", dst_path.GetFullPath())));

    while(true)
    {
        if(TerminateRequired()) goto END;

        in_stream.Read(buff, buff_size);
        size_t byte_to_read = in_stream.LastRead();
        if(byte_to_read == 0) break;

        out_stream.WriteAll(buff, byte_to_read);
        sum_bytes += byte_to_read;

        double progress_double = ((double)sum_bytes / (double) src_size)*100.0;
        if(progress != (int)progress_double)
        {
            progress = (int)progress_double;
            wxQueueEvent(event_handler, new ProgressEvent(progress_double));
        }
    }

    wxQueueEvent(event_handler, new ProgressEvent(100.));
    out_stream.Close();
    dst_size = dst_path.GetSize().ToULong();
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Result : %s ==> %s", Unit::HumanReadable(src_size), Unit::HumanReadable(dst_size))));

    wxQueueEvent(event_handler, new MsgEvent(ttt("FinishMsg"), wxColour(0, 0, 255)));
    wxQueueEvent(event_handler, new MsgEvent(ttt("FinishMsg2")));
    
END:
    if(TerminateRequired())
      wxQueueEvent(event_handler, new MsgEvent(_T("Terminated")));

    delete buff;

    SetStateComplete();

    return NULL;
}
