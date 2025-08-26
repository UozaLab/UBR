/*
 * Copyright (c) 2024-2025, UozaLab
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

#include "Restore.h"
#include "SectorTask.h"
#include "smart_ptr.h"
#include "FileSystem/VDisk.h"
#include "FileSystem/VdiskFactory.h"
#include "FileSystem/DiskInfo.h"
#include "FileSystem/PhysicalDiskInfo.h"
#include "FileSystem/MbrGpt.h"
#include "FileSystem/Fs.h"
#include "FileSystem/VDiskStream.h"
#include "FileSystem/ForensicAnalysis.h"
#include <vector>

RestoreWorker::RestoreWorker(wxEvtHandler* event_handler, const RestoreData* _restore_data, shared_ptr<DiskInfo> _di)
: CustomThread(event_handler),
  restore_data(_restore_data), di(_di)
{
}

void* RestoreWorker::Entry()
{
    SetStateStart();

    std::vector<HANDLE> handles_locked;
    shared_ptr<DiskUpdaterCollection> updaters(new DiskUpdaterCollection());
    //
    // open disk
    //
    HANDLE dest_handle = CreateFile(
        wxString::Format("\\\\.\\PhysicalDrive%lu", restore_data->disk_number).c_str(),
		GENERIC_READ  | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
		NULL);
    if(dest_handle == INVALID_HANDLE_VALUE)
    {
        wxQueueEvent(event_handler, new ErrorEvent(wxString::Format("Failed to open disk%d", restore_data->disk_number)));
        goto END;
    }

    {
    //
    // lock disk
    //
    shared_ptr<PhysicalDiskInfo> pdi = di->PhysicalDisks[restore_data->disk_number];
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Lock Disk%d", restore_data->disk_number)));
    if(!PhysicalDiskUtil::LockAndDismount(pdi, handles_locked))
    {
        wxQueueEvent(event_handler, new ErrorEvent(wxString::Format("Cannot lock disk%d", restore_data->disk_number)));
        goto END;
    }

    //
    // Clear Sector0
    //
    //DISK_UPDATER_BYTE updator_byte;
    //UINT8 sector0[512] = {};
    //updator_byte.Address = 0;
    //updator_byte.Size = 512;
    //updator_byte.Data = sector0;
    //DiskUpdateTask::DataCopy(this, event_handler, dest_handle, updator_byte);

    //
    // Create new MBR/GPT
    //
    {
    shared_ptr<VirtualDisk> src = VirtualDiskFactory::Create(restore_data->filepath);
    shared_ptr<VirtualDisk> dest = VirtualDiskFactory::Create(restore_data->disk_number);
    MBRGPT src_mbrgpt(src);
    if(!src_mbrgpt.IsValid())
    {
        wxQueueEvent(event_handler, new ErrorEvent(_T("Invalid disk")));
        goto END;
    }

    shared_ptr<MBRGPT> mod_mbrgpt = src_mbrgpt.CreateShrinked(dest);
    if(!mod_mbrgpt->IsValid())
    {
        wxQueueEvent(event_handler, new ErrorEvent(mod_mbrgpt->GetReason()));
        goto END;
    }
    if(mod_mbrgpt->IsModified())
    {
        wxQueueEvent(event_handler, new MsgEvent(_T("Shrink Mode")));
    }


    //
    // sector copy(MBR/GPT)
    //
    ULONGLONG backup_sector_count = src_mbrgpt.GetBackupSectorCount();
    updaters->Append(backup_sector_count, 0, 0);
    DiskUpdateTask::Update(this, event_handler, src, dest_handle, updaters, false);
    updaters->Clear();

    //
    // sector update(MBR/GPT)
    //
    if(mod_mbrgpt->IsModified())
        DiskUpdateTask::UpdateMbrGpt(this, event_handler, src, mod_mbrgpt, dest_handle);


    //
    // sector copy(DATA)
    //
    if(mod_mbrgpt->IsGPT())
    {
        for(unsigned int i=0; i<mod_mbrgpt->MbrGptInfo.GptHeader.PartitionEntryCount; i++)
        {
            GUID UnusedData = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
            if(IsEqualGUID(mod_mbrgpt->MbrGptInfo.GPE[i].PartitionType, UnusedData)) continue;
            if(mod_mbrgpt->GetTargetPartitionNumberToShrink() == (i+1)) continue;
            updaters->Append(mod_mbrgpt->MbrGptInfo.GPE[i].LastLBA - mod_mbrgpt->MbrGptInfo.GPE[i].FirstLBA + 1,
                             src_mbrgpt.MbrGptInfo.GPE[i].FirstLBA,
                             mod_mbrgpt->MbrGptInfo.GPE[i].FirstLBA);

        }
    }
    else
    {
        for(unsigned int i=0;i<MBR_PARTITION_ENTRY_COUNT;i++)
        {
            if(mod_mbrgpt->MbrGptInfo.PE[i].PartitionType == 0x00) continue;
            if(mod_mbrgpt->GetTargetPartitionNumberToShrink() == (i+1)) continue;

            updaters->Append(mod_mbrgpt->MbrGptInfo.PE[i].NumberOfSectors,
                             src_mbrgpt.MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector,
                             mod_mbrgpt->MbrGptInfo.PE[i].LBAOfFirstAbsoluteSector);
        }
    }

    //
    // sector update(FileSystem)
    //
    src->SetLargeScaleMode(true);
    if(mod_mbrgpt->IsModified())
    {
        shared_ptr<VirtualDiskStream> stream(new VirtualDiskStream(src));
        FileSystem fs(stream);
        updaters->Append(*fs.GetShrinkedSectorData(mod_mbrgpt));
    }

    DiskUpdateTask::Update(this, event_handler, src, dest_handle, updaters);

    //
    // Update disk properties
    //
    DWORD BytesReturened;
    DeviceIoControl(dest_handle, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &BytesReturened, NULL);

    }
    }



END:
    PhysicalDiskUtil::UnLock(handles_locked);
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Unlock Disk%d", restore_data->disk_number)));

    if(dest_handle != INVALID_HANDLE_VALUE)
        CloseHandle(dest_handle);

    if(TerminateRequired())
        wxQueueEvent(event_handler, new MsgEvent(_T("Terminated")));
    else
        wxQueueEvent(event_handler, new MsgEvent(_T("Done")));
    SetStateComplete();

    return NULL;
}
