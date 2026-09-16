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

#include "Lz4Stream.h"

void LZ4InputStream::init_state(LZ4F_dctx* dctxPtr)
{
    char headerBuf[LZ4F_HEADER_SIZE_MAX];
    size_t bytesRead = sizeof(headerBuf);
    size_t consumedSize = bytesRead;
    LZ4F_errorCode_t result;
    LZ4F_frameInfo_t frameInfo;
    size_t blockSize;
    
    ZeroMemory(&state_read, sizeof(LZ4StateRead));
    state_read.dctxPtr = dctxPtr;

    bool ret = stream->ReadAll(headerBuf, sizeof(headerBuf));
    if(!ret) goto ERROR_END;

    result = LZ4F_getFrameInfo(state_read.dctxPtr, &frameInfo, headerBuf, &consumedSize);
    if(LZ4F_isError(result)) goto ERROR_END;
    contentSize = frameInfo.contentSize;

    blockSize = LZ4F_getBlockSize(frameInfo.blockSizeID);
    if (blockSize == 0) goto ERROR_END;
    state_read.BufMaxSize = blockSize;
    state_read.Buf = (LZ4_byte*)malloc(state_read.BufMaxSize);
    if(state_read.Buf == NULL) goto ERROR_END;
    state_read.BufSize = bytesRead - consumedSize;
    if(state_read.BufSize > 0)
    {
        memcpy(state_read.Buf, headerBuf + consumedSize, state_read.BufSize);
    }
    state_read.BufNext = 0;

  NORMAL_END:
    state_read.valid = true;
    return;

  ERROR_END:
    if(state_read.Buf != NULL) free(state_read.Buf);
    if(state_read.dctxPtr != NULL) LZ4F_freeDecompressionContext(state_read.dctxPtr);
    state_read.valid = false;
}

LZ4InputStream::LZ4InputStream(wxInputStream* _stream)
     : stream(_stream), damaged(false)
{
    LZ4F_dctx* dctxPtr;
    LZ4F_errorCode_t result = LZ4F_createDecompressionContext(&dctxPtr, LZ4F_VERSION);
    if (LZ4F_isError(result))
    {
        ZeroMemory(&state_read, sizeof(LZ4StateRead));
        state_read.valid = false;
        return;
    }
    init_state(dctxPtr);
}

LZ4InputStream::~LZ4InputStream()
{
    if(state_read.valid)
    {
        free(state_read.Buf);
        LZ4F_freeDecompressionContext(state_read.dctxPtr);
    }
    delete stream;
}

wxFileOffset LZ4InputStream::GetLength() const
{
    return contentSize;
}

size_t LZ4InputStream::OnSysRead(void *buf, size_t size)
{
    if(!state_read.valid) return 0;

    LZ4_byte* outPtr = (LZ4_byte*)buf;
    size_t totalBytesRead = 0;

    while (totalBytesRead < size)
    {
        size_t srcBytes = state_read.BufSize - state_read.BufNext;
        size_t dstBytes = size - totalBytesRead;

        if(srcBytes == 0)
        {
            stream->Read(state_read.Buf, state_read.BufMaxSize);
            size_t bytesRead = stream->LastRead();
            if(bytesRead == 0) break;

            state_read.BufSize = bytesRead;
            srcBytes = state_read.BufSize;
            state_read.BufNext = 0;
        }

        size_t decStatus = LZ4F_decompress(
            state_read.dctxPtr,
            outPtr, &dstBytes,
            state_read.Buf + state_read.BufNext,
            &srcBytes,
            NULL);
        if(LZ4F_isError(decStatus)) return 0;

        state_read.BufNext += srcBytes;
        totalBytesRead += dstBytes;
        outPtr += dstBytes;
    }

    return totalBytesRead;
}

size_t LZ4InputSeekableStream::OnSysRead(void *buf, size_t size)
{
    size_t totalBytesRead = LZ4InputStream::OnSysRead(buf, size);
    current_pos += totalBytesRead;
    return totalBytesRead;
}

wxFileOffset LZ4InputSeekableStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    unsigned long long BytesSkip = 0;
    bool need_to_rewind = false;
    switch(mode)
    {
      case wxFromStart:
        if(current_pos > pos)
        {
            need_to_rewind = true;
            BytesSkip = pos;
        }
        else
        {
            BytesSkip = pos - current_pos;
        }
        break;
      default:
      case wxFromCurrent:
        if(pos < 0)
        {
            need_to_rewind = true;
            BytesSkip = current_pos + pos;
        }
        else
        {
            BytesSkip = pos;
        }
        break;
      case wxFromEnd:
        {
            wxFileOffset end_pos = GetLength();
            if(end_pos == 0) return wxInvalidOffset;
            unsigned long long target_pos = end_pos + pos;
            if(current_pos < target_pos)
            {
                need_to_rewind = true;
                BytesSkip = target_pos;
            }
            else
            {
                BytesSkip = target_pos - current_pos;
            }
        }
        break;
    }

    if(need_to_rewind)
    {
        if(!rewind()) return wxInvalidOffset;
    }

    const int tmp_buffer_size = 10485760;/*10MB*/
    UINT8* buffer = new UINT8[tmp_buffer_size];
    while(true)
    {
        int buffer_size = tmp_buffer_size;
        if(BytesSkip <= 0) break;
        if(BytesSkip < buffer_size)
          buffer_size = BytesSkip;
        bool ret = ReadAll(buffer, buffer_size);
        if(!ret)
        {
            delete [] buffer;
            return wxInvalidOffset;
        }
        BytesSkip -= buffer_size;
    }
    delete [] buffer;

    
    return (wxFileOffset) current_pos;
}

bool LZ4InputSeekableStream::rewind()
{
    if(!stream->IsSeekable()) return false;
    stream->SeekI(0);

    LZ4F_resetDecompressionContext(state_read.dctxPtr);
    init_state(state_read.dctxPtr);
    if(!state_read.valid) return false;

    current_pos = 0;
    return true;
}

LZ4OutputStream::LZ4OutputStream(wxOutputStream* _stream, unsigned long long _content_size)
     : stream(_stream), current_pos(0), damaged(false), content_size(_content_size)
{
    LZ4F_errorCode_t status;
    size_t blockSize;
    ZeroMemory(&state_write, sizeof(LZ4StateWrite));

    blockSize = LZ4F_getBlockSize(LZ4F_default);
    state_write.errCode = LZ4F_OK_NoError;
    state_write.WriteMaxSize = blockSize;
    state_write.BufMaxSize = LZ4F_compressBound(blockSize, NULL);
    state_write.Buf = (LZ4_byte*)malloc(state_write.BufMaxSize);
    if(state_write.Buf == NULL) goto ERROR_END;
    status = LZ4F_createCompressionContext(&state_write.cctxPtr, LZ4F_VERSION);
    if(LZ4F_isError(status)) goto ERROR_END;


  NORMAL_END:
    state_write.valid = true;
    return;

  ERROR_END:
    if(state_write.Buf != NULL) free(state_write.Buf);
    if(state_write.cctxPtr != NULL) LZ4F_freeCompressionContext(state_write.cctxPtr);
    state_write.valid = false;
}

LZ4OutputStream::~LZ4OutputStream()
{
    Close();
    if(state_write.valid)
    {
        free(state_write.Buf);
        LZ4F_freeCompressionContext(state_write.cctxPtr);
    }
    delete stream;
}

bool LZ4OutputStream::Close()
{
    if(damaged)
    {
        LZ4F_errorCode_t ret =  LZ4F_compressEnd(state_write.cctxPtr,
                                                 state_write.Buf, state_write.BufMaxSize,
                                                 NULL);
        if(LZ4F_isError(ret)) return false;
        stream->WriteAll(state_write.Buf, ret);
    }
    return true;
}

size_t LZ4OutputStream::OnSysWrite(const void *buf, size_t nbytes)
{
    if(!state_write.valid) return 0;
    if(!damaged)
    {
        LZ4_byte headerBuf[LZ4F_HEADER_SIZE_MAX];
        LZ4F_preferences_t prefs;
        ZeroMemory(&prefs, sizeof(LZ4F_preferences_t));
        prefs.frameInfo.contentSize = content_size;
        LZ4F_errorCode_t headerSize = LZ4F_compressBegin(state_write.cctxPtr, headerBuf, LZ4F_HEADER_SIZE_MAX, content_size == 0 ? NULL : &prefs);
        if (LZ4F_isError(headerSize)) return 0;
        stream->WriteAll(headerBuf, headerSize);
        current_pos += headerSize;
        damaged = true;
    }

    const LZ4_byte* p = (const LZ4_byte*)buf;
    size_t remainingBytes = nbytes;
    while (remainingBytes)
    {
        size_t const chunkSize = (remainingBytes > state_write.WriteMaxSize) ? state_write.WriteMaxSize : remainingBytes;
        size_t cSize = LZ4F_compressUpdate(state_write.cctxPtr,
                                           state_write.Buf, state_write.BufMaxSize,
                                           p, chunkSize,
                                           NULL);
        if (LZ4F_isError(cSize))
        {
            state_write.errCode = cSize;
            return 0;
        }
        stream->WriteAll(state_write.Buf, cSize);
        current_pos += cSize;

        p += chunkSize;
        remainingBytes -= chunkSize;
    }

    return nbytes;
}
