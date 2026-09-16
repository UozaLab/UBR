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

#include "Backup.h"
#include "tstring.h"
#include "smart_ptr.h"
#include "Misc.h"
#include "VSS/Vss.h"
#include "FileSystem/VDisk.h"
#include "FileSystem/DiskInfo.h"
#include "FileSystem/VDisk_vss.h"
#include "FileSystem/VDisk_vhdx_wx.h"
#include "FileSystem/VDisk_vhd_wx.h"
#include "FileSystem/VDisk_raw_wx.h"
#include "LZ4Stream.h"
#include "MiscWx.h"
#include <wx/wfstream.h>

FileReader::FileReader(BackupWorker* _worker, shared_ptr<PhysicalVSS> _src)
     : worker(_worker), CustomThread(nullptr), src(_src)
{
}

void * FileReader::Entry()
{
    SetStateStart();
    const int MAX_BLOCK_COUNT = 402653184/*384MB*/ / src->GetBlockSize();

    DWORD entry_count = src->GetTableEntriesCount();

    for(DWORD i=0; i<entry_count; i++)
    {
        if(TerminateRequired()) goto END;
        if(src->CanSkip(i)) continue;
        DWORD ByteRead;
        bool can_skip;
        wxVector<UINT8*>::size_type block_count = 0;
        {
            wxMutexLocker locker(worker->mutex);
            block_count = worker->blockdata_holder.size();
        }
        while(block_count >= MAX_BLOCK_COUNT)
        {
            {
                wxMutexLocker locker(worker->mutex);
                block_count = worker->blockdata_holder.size();
            }
            Sleep(1000);
            if(TerminateRequired()) goto END;
        }

        if(TerminateRequired()) goto END;
        UINT8* blockdata = new UINT8[src->GetBlockSize()];
        if(!src->GetBlockData(blockdata, i, &ByteRead, &can_skip))
        {
            delete [] blockdata;
            goto END;
        }


        {
            wxMutexLocker locker(worker->mutex);
            worker->blockdata_holder.push_back(std::make_pair(i, blockdata));
        }
    }

    {
        wxMutexLocker locker(worker->mutex);
        worker->blockdata_read_complete_success = true;
    }

END:
    {
        wxMutexLocker locker(worker->mutex);
        worker->blockdata_read_end = true;
    }

    SetStateComplete();
    return NULL;
}

FileWriter::FileWriter(BackupWorker* _worker, shared_ptr<PhysicalVSS> _src, shared_ptr<VirtualDisk> _dst)
     : worker(_worker), CustomThread(nullptr), src(_src), dst(_dst)
{
}

void * FileWriter::Entry()
{
    SetStateStart();
    int progress = 0;
    DWORD processed_block_count = 0;

    while(!TerminateRequired())
    {
        double progress_double = ((double)processed_block_count / (double) src->GetActualBlockCount())*100.0;
        if(progress != (int)progress_double)
        {
            progress = (int)progress_double;
            wxQueueEvent(worker->EventHandler(), new ProgressEvent(progress_double));
        }
        
        wxVector<UINT8*>::size_type block_count = 0;
        bool read_complete = false;
        {
            wxMutexLocker locker(worker->mutex);
            block_count = worker->blockdata_holder.size();
            read_complete = worker->blockdata_read_end;
        }

        if(block_count == 0)
        {
            if(TerminateRequired()) goto END;
            if(read_complete) goto END;
            Sleep(1000);
            continue;
        }

        UINT8* blockdata = nullptr;
        DWORD blockindex = 0;
        DWORD ByteWrite;
        {
            wxMutexLocker locker(worker->mutex);
            std::pair<DWORD, UINT8*> pair = worker->blockdata_holder.front();
            blockindex = pair.first;
            blockdata = pair.second;
            worker->blockdata_holder.erase(worker->blockdata_holder.begin());
        }

        if(!dst->SetBlockData(blockdata, blockindex, &ByteWrite))
        {
            delete [] blockdata;
            goto END;
        }

        processed_block_count++;
        delete [] blockdata;
    }

END:
    SetStateComplete();
    return NULL;
}

BackupWorker::BackupWorker(wxEvtHandler* event_handler, const BackupData* _backup_data, shared_ptr<DiskInfo> _di)
: CustomThread(event_handler),
  backup_data(_backup_data), di(_di), blockdata_read_end(false), blockdata_read_complete_success(false)
{
}

BackupWorker::~BackupWorker()
{
    for(wxVector<std::pair<DWORD, UINT8*> >::iterator itr = blockdata_holder.begin(), itr_end = blockdata_holder.end();
        itr != itr_end; itr++)
    {
        std::pair<DWORD, UINT8*> pair = *itr;
        UINT8* blockdata = pair.second;
        delete [] blockdata;
    }
    blockdata_holder.clear();
}

void* BackupWorker::Entry()
{
    SetStateStart();
    bool fast_mode = (backup_data->filetype == FILE_TYPE_RAW) ? false : !backup_data->exact_mode;
    DWORD blocksize = backup_data->filetype == FILE_TYPE_VHDX ? 4194304/*4MB*/
                      : backup_data->filetype == FILE_TYPE_VHD ? 2097152/*2MB*/
                      : 134217728/*128MB*/;

    wxQueueEvent(event_handler, new MsgEvent("Creating a volume snapshot"));
    shared_ptr<PhysicalVSS> src(new PhysicalVSS(backup_data->disk_number, di, blocksize, backup_data->vss));
    if(src->IsVssSnapshotCreated())
        wxQueueEvent(event_handler, new MsgEvent("Snapshot created"));
    else
    {
        if(!backup_data->vss)
        {
            wxQueueEvent(event_handler, new MsgEvent("Skip creating a snapshot"));
        }
        else if(SystemEnvironment::RunOnPE())
        {
            wxQueueEvent(event_handler, new MsgEvent("Run on WinPE"));
            wxQueueEvent(event_handler, new MsgEvent("Skip creating a snapshot"));
        }
        else
            wxQueueEvent(event_handler, new MsgEvent("Failed to create a snapshot"));
    }

    if(fast_mode) src->CreateEmptyClusterRanges();

    wxOutputStream* out_stream;
    wxString filepath = backup_data->filepath;
    if(backup_data->compress)
    {
        if(!filepath.Lower().EndsWith(_T(".lz4")))
          filepath += ".lz4";
        out_stream = new LZ4OutputStream(new wxFileOutputStream(filepath),
                                         (backup_data->filetype == FILE_TYPE_RAW) ? src->GetDiskSize() : 0);
    }
    else
    {
        out_stream = new wxFileOutputStream(filepath);
    }

    shared_ptr<VirtualDisk> dest(backup_data->filetype == FILE_TYPE_VHDX ? (VirtualDisk*) new VHDX_VSS(out_stream, src)
                                 : backup_data->filetype == FILE_TYPE_VHD ? (VirtualDisk*) new VHD_VSS(out_stream, src)
                                 : (VirtualDisk*) new RAW_VSS(out_stream, src));

    FileReader reader(this, src);
    FileWriter writer(this, src, dest);
    reader.Run();
    writer.Run();

    ThreadState reader_state;
    ThreadState writer_state;
    bool reader_finished = false;
    bool writer_finished = false;
    bool err = false;
    do
    {
        if(TerminateRequired())
        {
            reader.Terminate();
            writer.Terminate();
        }
        reader_state = reader.GetState();
        writer_state = writer.GetState();
        reader_finished = (reader_state == THREAD_FINISHED) || (reader_state == THREAD_TERMINATED);
        writer_finished = (writer_state == THREAD_FINISHED) || (writer_state == THREAD_TERMINATED);
        {
            wxMutexLocker locker(mutex);
            err = (reader_finished && !blockdata_read_complete_success) || (writer_finished && !reader_finished);
        }
        if(err)
        {
            reader.Terminate();
            writer.Terminate();
        }
        Sleep(1000);
    } while(!reader_finished || !writer_finished);
    
    if(err)
    {
        wxQueueEvent(event_handler, new ErrorEvent("Cannot read or write block data."));
        goto END;
    }


    if(!TerminateRequired())
    {
        wxQueueEvent(event_handler, new ProgressEvent(100.));
    }


END:

    if(TerminateRequired())
      wxQueueEvent(event_handler, new MsgEvent(_T("Terminated")));
    else
    {
        wxQueueEvent(event_handler, new MsgEvent(ttt("FinishMsg"), wxColour(0, 0, 255)));
        wxQueueEvent(event_handler, new MsgEvent(ttt("FinishMsg2")));
    }

    SetStateComplete();

    return NULL;
}
