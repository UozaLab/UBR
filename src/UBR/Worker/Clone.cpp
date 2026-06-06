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

#include "Clone.h"
#include "SectorTask.h"
#include "FileSystem/VDisk.h"
#include "FileSystem/VdiskFactory.h"
#include "FileSystem/DiskInfo.h"
#include "FileSystem/PhysicalDiskInfo.h"
#include "FileSystem/VDisk_vss.h"
#include "FileSystem/MbrGpt.h"
#include "FileSystem/Fs.h"
#include "FileSystem/VDiskStream.h"
#include "FileSystem/ForensicAnalysis.h"
#include "VSS/Vss.h"
#include "tstring.h"
#include "Misc.h"
#include <vector>

CloneWorker::CloneWorker(wxEvtHandler* event_handler, const CloneData* _clone_data, shared_ptr<DiskInfo> _di)
: CustomThread(event_handler),
  clone_data(_clone_data), di(_di)
{
}

void* CloneWorker::Entry()
{
    SetStateStart();

    std::vector<HANDLE> handles_locked;
    Vss vss;
    shared_ptr<DiskUpdaterCollection> updaters(new DiskUpdaterCollection());

    //
    // open disk
    //
    HANDLE dest_handle = CreateFile(
        wxString::Format("\\\\.\\PhysicalDrive%lu", clone_data->disk_number_dst).c_str(),
        GENERIC_READ  | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
        NULL);
    if(dest_handle == INVALID_HANDLE_VALUE)
    {
        wxQueueEvent(event_handler, new ErrorEvent(wxString::Format("Failed to open disk%d", clone_data->disk_number_dst)));
        goto END;
    }



    {
    //
    // lock disk
    //
    shared_ptr<PhysicalDiskInfo> pdi = di->PhysicalDisks[clone_data->disk_number_dst];
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Lock Disk%d", clone_data->disk_number_dst)));
    if(!PhysicalDiskUtil::LockAndDismount(pdi, handles_locked))
    {
        wxQueueEvent(event_handler, new ErrorEvent(wxString::Format("Cannot lock disk%d", clone_data->disk_number_dst)));
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

    if(clone_data->exact_mode)
    {
        LONGLONG disk_size_src = di->PhysicalDisks[clone_data->disk_number_src]->DiskSize;
        LONGLONG disk_size_dst = di->PhysicalDisks[clone_data->disk_number_dst]->DiskSize;
        if(disk_size_dst < disk_size_src)
        {
            wxQueueEvent(event_handler, new ErrorEvent(_T("Destination disk is too small")));
            goto END;
        }
    }

    //
    // Create new MBR/GPT
    //
    {
    wxQueueEvent(event_handler, new MsgEvent("Creating a volume snapshot"));
    shared_ptr<PhysicalVSS> src(new PhysicalVSS(clone_data->disk_number_src, di));
    if(src->IsVssSnapshotCreated())
        wxQueueEvent(event_handler, new MsgEvent("Snapshot created"));
    else
    {
        if(SystemEnvironment::RunOnPE())
        {
            wxQueueEvent(event_handler, new MsgEvent("Run on WinPE"));
            wxQueueEvent(event_handler, new MsgEvent("Skip creating a snapshot"));
        }
        else
            wxQueueEvent(event_handler, new MsgEvent("Failed to create a snapshot"));
    }
    shared_ptr<VirtualDisk> dest = VirtualDiskFactory::Create(clone_data->disk_number_dst);
    MBRGPT src_mbrgpt(src);
    if(!src_mbrgpt.IsValid())
    {
        wxQueueEvent(event_handler, new ErrorEvent(_T("Invalid disk")));
        goto END;
    }

    src->CreateEmptyClusterRanges();
    src->SetLargeScaleMode(true);

    if(clone_data->exact_mode)
    {
        ULONGLONG sectors_to_clone = src->GetDiskSize();
        sectors_to_clone /= src->GetSectorSize();
        updaters->Append(sectors_to_clone, 0, 0, false);

        DiskUpdateTask::Update(this, event_handler, src, dest_handle, updaters, true);
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
    wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Unlock Disk%d", clone_data->disk_number_dst)));

    if(dest_handle != INVALID_HANDLE_VALUE)
        CloseHandle(dest_handle);

    if(TerminateRequired())
        wxQueueEvent(event_handler, new MsgEvent(_T("Terminated")));
    else
        wxQueueEvent(event_handler, new MsgEvent(_T("Done")));
    SetStateComplete();

    return NULL;
}
