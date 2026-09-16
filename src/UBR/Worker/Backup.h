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

#ifndef __Backup__H__
#define __Backup__H__

#include <wx/wx.h>
#include "smart_ptr.h"
#include "Events.h"
#include "DataHolder.h"
#include "FileSystem/DiskInfo.h"
#include "FileSystem/VDisk_vhd_common.h"
#include "FileSystem/VDisk_vss.h"

class BackupWorker;

class FileReader : public CustomThread
{
  protected:
    shared_ptr<PhysicalVSS> src;
    BackupWorker* worker;

  public:
    FileReader(BackupWorker* _worker, shared_ptr<PhysicalVSS> _src);
    virtual void *Entry();
};

class FileWriter : public CustomThread
{
  protected:
    shared_ptr<VirtualDisk> src;
    shared_ptr<VirtualDisk> dst;
    BackupWorker* worker;
    DWORD entry_count;

  public:
    FileWriter(BackupWorker* _worker, shared_ptr<PhysicalVSS> _src, shared_ptr<VirtualDisk> _dst);
    virtual void *Entry();
};

class BackupWorker : public CustomThread
{
  protected:
    shared_ptr<DiskInfo> di;
    const BackupData* backup_data;
    wxVector<std::pair<DWORD, UINT8*> > blockdata_holder;
    bool blockdata_read_complete_success;
    bool blockdata_read_end;
    wxMutex mutex;

  public:
    BackupWorker(wxEvtHandler* event_handler, const BackupData* _backup_data, shared_ptr<DiskInfo> _di);
    ~BackupWorker();
    virtual void *Entry();

    friend class FileReader;
    friend class FileWriter;
};


#endif
