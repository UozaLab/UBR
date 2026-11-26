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

#ifndef __SECTOR_TASK__H__
#define __SECTOR_TASK__H__

#include <wx/wx.h>
#include <vector>
#include "Events.h"
#include "Misc.h"
#include "smart_ptr.h"
#include "FileSystem/FsInfo.h"
#include "FileSystem/Vdisk.h"
#include "FileSystem/MbrGpt.h"

struct Blockdata
{
    DWORD block_index;
    UINT8* block_data;
    bool can_skip;
    bool updater_can_skip;
};

struct SyncControl
{
    bool blockdata_read_complete_success;
    bool blockdata_read_end;
    wxMutex mutex;
    wxVector<Blockdata> blockdata_holder;
    DISK_UPDATER updater;
    shared_ptr<DiskUpdaterCollection> updater_collection;

    bool show_progress;
    UINT64 sector_processed_sum;
    UINT64 sector_sum;
    
    SyncControl(DISK_UPDATER _updater, shared_ptr<DiskUpdaterCollection> _updater_collection);
};

class DiskReader : public CustomThread
{
  protected:
    shared_ptr<VirtualDisk> src;
    SyncControl* sync;
    CustomThread* caller;
    ConditionVariable* cv;

  public:
    DiskReader(CustomThread* _caller, SyncControl* _sync, ConditionVariable* _cv, shared_ptr<VirtualDisk> _src);
    virtual void *Entry();
};

class DiskWriter : public CustomThread
{
  protected:
    shared_ptr<VirtualDisk> src;
    HANDLE handle_dest;
    SyncControl* sync;
    ConditionVariable* cv;
    CustomThread* caller;
    LONGLONG shift_bytes;
    
  public:
    DiskWriter(CustomThread* _caller, SyncControl* _sync, ConditionVariable* _cv, shared_ptr<VirtualDisk> _src, HANDLE _handle_dest, LONGLONG _shift_bytes);
    virtual void *Entry();
};

class DiskUpdateTask
{
public:
    static void Update(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, shared_ptr<DiskUpdaterCollection> updater_collection, bool show_progress = true, LONGLONG shift_bytes = 0LL);

    static void DiskErase(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, UINT64 sector_address, UINT64 count);
    static void UpdateMbrGpt(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, shared_ptr<MBRGPT> mbrgpt, HANDLE handle_dest);
    static void DataCopy(CustomThread* thread, wxEvtHandler* event_handler, HANDLE handle_dest, const DISK_UPDATER_BYTE& updater_byte);
    static void UpdateCrc(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, UINT64 sector_address_from, UINT64 sector_address_to, UINT64 count, UINT8* boot_sector, DWORD boot_sector_size);
};

#endif
