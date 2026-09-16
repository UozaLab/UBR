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

#ifndef __LZ4_STREAM_H__
#define __LZ4_STREAM_H__

#include <wx/wx.h>
#include "lz4.h"
#include "lz4frame_static.h"

struct LZ4StateRead
{
    bool valid;
    LZ4F_dctx* dctxPtr;
    LZ4_byte* Buf;
    size_t BufNext;
    size_t BufSize;
    size_t BufMaxSize;
};

struct LZ4StateWrite
{
    bool valid;
    LZ4F_cctx* cctxPtr;
    LZ4_byte* Buf;
    size_t WriteMaxSize;
    size_t BufMaxSize;
    LZ4F_errorCode_t errCode;
};

class LZ4InputStream : public wxInputStream
{
  protected:
    wxInputStream* stream;
    LZ4StateRead state_read;
    bool damaged;
    unsigned long long contentSize;
    void init_state(LZ4F_dctx* dctxPtr);

    size_t OnSysRead(void *buf, size_t size) wxOVERRIDE;
  public:
    LZ4InputStream(wxInputStream* _stream);
    virtual ~LZ4InputStream();
    wxFileOffset GetLength() const wxOVERRIDE;
};

class LZ4InputSeekableStream : public LZ4InputStream
{
  protected:
    unsigned long long current_pos;
    bool rewind();

  public:
    LZ4InputSeekableStream(wxInputStream* _stream)
         : LZ4InputStream(_stream), current_pos(0){}
    virtual bool IsSeekable() const wxOVERRIDE { return true; }
    virtual wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode);
    virtual wxFileOffset OnSysTell() const wxOVERRIDE { return (wxFileOffset) current_pos; }
    size_t OnSysRead(void *buf, size_t size) wxOVERRIDE;
};

class LZ4OutputStream : public wxOutputStream
{
  protected:
    wxOutputStream* stream;
    unsigned long long current_pos;
    unsigned long long content_size;
    LZ4StateWrite state_write;
    bool damaged;

    size_t OnSysWrite(const void *buf, size_t nbytes) wxOVERRIDE;
    bool Close() wxOVERRIDE;

  public:
    LZ4OutputStream(wxOutputStream* _stream, unsigned long long _content_size = 0);
    virtual ~LZ4OutputStream();
    virtual wxFileOffset GetLength() const wxOVERRIDE { return (wxFileOffset) current_pos; }
};

#endif

