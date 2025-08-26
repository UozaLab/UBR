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

#include "SectorTask.h"
#include "FileSystem/VdiskStream.h"
#include "Events.h"


void DiskUpdateTask::DiskErase(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, UINT64 sector_address, UINT64 count)
{
    DWORD sector_size = src->GetSectorSize();
    UINT64 address_to_write = sector_address;
    address_to_write *= sector_size;
    DWORD ByteWrite;

    const unsigned int BUFFER_SIZE = 1024*1024*10;
    UINT8* zeros = new UINT8[BUFFER_SIZE];
    ZeroMemory(zeros, BUFFER_SIZE);
    UINT64 count_byte = count;
    count_byte *= sector_size;
    while(count_byte > 0)
    {
        UINT64 byte_to_write = BUFFER_SIZE;
        if(byte_to_write > count_byte) byte_to_write = count_byte;

        LARGE_INTEGER filepointer;
        filepointer.QuadPart = address_to_write;
        SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
        address_to_write += byte_to_write;

        if(!WriteFile(handle_dest, zeros, byte_to_write, &ByteWrite, 0))
        {
            DWORD reason = GetLastError();
            ErrorEvent* ev = new ErrorEvent(wxString::Format("Write error : %d", reason));
            wxQueueEvent(event_handler, ev);
            thread->Terminate();
            break;
        }
        count_byte -= byte_to_write;
    }
    delete [] zeros;
}


void DiskUpdateTask::UpdateMbrGpt(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, shared_ptr<MBRGPT> mbrgpt, HANDLE handle_dest)
{
    DWORD sector_size = src->GetSectorSize();
    wxQueueEvent(event_handler, new MsgEvent(mbrgpt->IsGPT() ? wxString("Update GPT") : wxString("Update MBR")));
    DWORD sectors_to_write = mbrgpt->IsGPT() ? 34 : 1;

    LARGE_INTEGER filepointer;
    DWORD ByteWrite;
    filepointer.QuadPart = 0;
    SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
    if(!WriteFile(handle_dest, mbrgpt->MbrGptSector, sectors_to_write * 512, &ByteWrite, 0))
    {
        DWORD reason = GetLastError();
        ErrorEvent* ev = new ErrorEvent(wxString::Format("Write MBR/GPT error : %d", reason));
        wxQueueEvent(event_handler, ev);
        thread->Terminate();
    }

    if(mbrgpt->IsGPT() && mbrgpt->MbrGptInfo.GptHeader.BackupLBA != 1)
    {
        // Write backup GPT
        shared_ptr<MBRGPT> mbrgpt_backup = mbrgpt->CreateBackupGPT();
        sectors_to_write = 1;
        filepointer.QuadPart = mbrgpt->MbrGptInfo.GptHeader.BackupLBA;
        filepointer.QuadPart *= sector_size;
        SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
        if(!WriteFile(handle_dest, mbrgpt_backup->MbrGptSector + 512/*skip mbr*/, sectors_to_write * 512, &ByteWrite, 0))
        {
            DWORD reason = GetLastError();
            ErrorEvent* ev = new ErrorEvent(wxString::Format("Write GPT(backup header) error : %d", reason));
            wxQueueEvent(event_handler, ev);
            thread->Terminate();
        }

        // Write GPT partition entries to BackupLBA-32
        sectors_to_write = 32;
        filepointer.QuadPart = mbrgpt->MbrGptInfo.GptHeader.BackupLBA - 32;
        filepointer.QuadPart *= sector_size;
        SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
        if(!WriteFile(handle_dest, mbrgpt_backup->MbrGptSector + 512/*skip mbr*/ + 512/*skip header*/, sectors_to_write * 512, &ByteWrite, 0))
        {
            DWORD reason = GetLastError();
            ErrorEvent* ev = new ErrorEvent(wxString::Format("Write GPT(backup partition entries) error : %d", reason));
            wxQueueEvent(event_handler, ev);
            thread->Terminate();
        }

    }
}

void DiskUpdateTask::DataCopy(CustomThread* thread, wxEvtHandler* event_handler, HANDLE handle_dest, const DISK_UPDATER_BYTE& updater_byte)
{
    LARGE_INTEGER filepointer;
    DWORD ByteWrite;
    filepointer.QuadPart = updater_byte.Address;
    SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
    if(!WriteFile(handle_dest, updater_byte.Data, updater_byte.Size, &ByteWrite, 0))
    {
        DWORD reason = GetLastError();
        ErrorEvent* ev = new ErrorEvent(wxString::Format("Data write error(%lld) : %d", updater_byte.Address, reason));
        wxQueueEvent(event_handler, ev);
        thread->Terminate();
    }
}

void DiskUpdateTask::UpdateCrc(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, UINT64 sector_address_from, UINT64 sector_address_to, UINT64 count, UINT8* boot_sector, DWORD boot_sector_size)
{
    wxQueueEvent(event_handler, new MsgEvent(wxString("Update CRC")));
    DWORD sector_size = src->GetSectorSize();
    VirtualDiskStream stream(src);

    UINT8* sector_buff = stream.Read(sector_address_from, count);
    memcpy(sector_buff, boot_sector, boot_sector_size);// update boot sector

    UINT32 sum = 0;
    for(int i = 0; i<sector_size*count; i++)
    {
        if(i == 0x6A || i == 0x6B || i == 0x70) continue;// skip VolumeFlags and PercentInUse fields
        sum = ((sum << 31) | (sum >> 1)) + sector_buff[i];
    }
    delete [] sector_buff;

    UINT8* sector_crc = new UINT8[sector_size];
    UINT32* sector_crc_32 = (UINT32*) sector_crc;
    for(unsigned int i = 0; i<sector_size/4; i++)
    {
        sector_crc_32[i] = sum;
    }
    DISK_UPDATER_BYTE du;
    du.Data = sector_crc;
    du.Size = sector_size;
    du.Address = sector_address_to;
    du.Address *= sector_size;;
    DataCopy(thread, event_handler, handle_dest, du);
    delete [] sector_crc;
}


void DiskUpdateTask::Update(CustomThread* thread, wxEvtHandler* event_handler, shared_ptr<VirtualDisk> src, HANDLE handle_dest, shared_ptr<DiskUpdaterCollection> updater_collection, bool show_progress)
{
    int progress = 0;
    UINT64 sector_sum = 0;
    UINT64 sector_processed_sum = 0;
    for(int i=0;i<updater_collection->Count();i++)
    {
        DISK_UPDATER disk_updater = updater_collection->Get(i);
        if(!disk_updater.sector) continue;

        sector_sum += disk_updater.SectorUpdater.Count;
    }
    if(sector_sum == 0) sector_sum = 1;// to avoid divide by zero exception


    for(int i=0;i<updater_collection->Count();i++)
    {
        if(thread->TerminateRequired())
            break;

        double progress_double = ((double)sector_processed_sum / (double) sector_sum)*100.0;
        if(progress != (int)progress_double)
        {
            progress = (int)progress_double;
            if(show_progress)
                wxQueueEvent(event_handler, new ProgressEvent(progress_double));
        }


        DISK_UPDATER disk_updater = updater_collection->Get(i);
        if(disk_updater.sector && disk_updater.zerofill)
        {
            DiskErase(thread, event_handler, src, handle_dest, disk_updater.SectorUpdater.AddressTo, disk_updater.SectorUpdater.Count);
            sector_processed_sum += disk_updater.SectorUpdater.Count;
            continue;
        }
        else if(disk_updater.sector && disk_updater.crc)
        {
            UpdateCrc(thread, event_handler, src, handle_dest, 
                disk_updater.SectorUpdater.AddressFrom, disk_updater.SectorUpdater.AddressTo, disk_updater.SectorUpdater.Count,
                disk_updater.ByteUpdater.Data, disk_updater.ByteUpdater.Size);
            sector_processed_sum += disk_updater.SectorUpdater.Count;
            continue;
        }
        else if(!disk_updater.sector)
        {
            DataCopy(thread, event_handler, handle_dest, disk_updater.ByteUpdater);// Byte copy
            continue;
        }


        DWORD sector_size = src->GetSectorSize();
        DWORD block_size_byte = src->GetBlockSize();
        DWORD block_size_sector = block_size_byte / src->GetSectorSize();
        UINT8* buff = new UINT8[block_size_byte];

        DISK_UPDATER_SECTOR sector_updater = disk_updater.SectorUpdater;
        wxQueueEvent(event_handler, new MsgEvent(wxString::Format("Copy Sector : %lld -> %lld (#%lld)", sector_updater.AddressFrom, sector_updater.AddressTo, sector_updater.Count)));


        DWORD block_start_index = sector_updater.AddressFrom / block_size_sector;
        DWORD offset_start_sector = sector_updater.AddressFrom - (block_start_index * block_size_sector);
        DWORD offset_start_bytes = offset_start_sector * sector_size;

        DWORD block_end_index = (sector_updater.AddressFrom + sector_updater.Count - 1) / block_size_sector;
        DWORD offset_end_sector = (sector_updater.AddressFrom + sector_updater.Count) - (block_end_index * block_size_sector);
        DWORD offset_end_bytes = offset_end_sector * sector_size;

        UINT64 write_sector_addr = sector_updater.AddressTo;
        UINT64 read_sector_addr = sector_updater.AddressFrom;

        for(DWORD i=block_start_index; i<=block_end_index; i++)
        {
            if(thread->TerminateRequired()) break;

            DWORD ByteRead;
            bool can_skip;
            if(!src->GetBlockData(buff, i, &ByteRead, &can_skip))
            {
                DWORD reason = GetLastError();
                ErrorEvent* ev = new ErrorEvent(wxString::Format("Read error : %d", reason));
                wxQueueEvent(event_handler, ev);
                thread->Terminate();
                break;
            }

            UINT8* buff_to_write = buff;
            DWORD sectors_to_write = block_size_sector;
            if(i == block_start_index)
            {
                buff_to_write += offset_start_bytes;
                sectors_to_write -= offset_start_sector;
            }
            if(i == block_end_index)
            {
                sectors_to_write -= (block_size_sector - offset_end_sector);
            }

            bool skip = (can_skip && sector_updater.can_skip);
            std::vector<DISK_UPDATER> intercepts = updater_collection->FindIntercept(read_sector_addr, sectors_to_write);
            if(intercepts.size() != 0)
            {
                if(skip)
                    ZeroMemory(buff, block_size_byte);
                for(std::vector<DISK_UPDATER>::iterator itr = intercepts.begin(), itr_end = intercepts.end();
                    itr != itr_end; itr++)
                {
                    DISK_UPDATER du = *itr;
                    UINT64 offset = du.SectorUpdater.AddressFrom - read_sector_addr;
                    offset *= sector_size;
                    memcpy(buff_to_write + offset, du.ByteUpdater.Data, du.ByteUpdater.Size);
                }
                skip = false;
            }

            std::vector<DISK_BITOP> bitops = updater_collection->FindBitop(read_sector_addr, sectors_to_write);
            if(bitops.size() != 0)
            {
                if(skip)
                    ZeroMemory(buff, block_size_byte);

                for(std::vector<DISK_BITOP>::iterator itr = bitops.begin(), itr_end = bitops.end(); itr != itr_end; itr++)
                {
                    DISK_BITOP db = *itr;
                    UINT64 offset = db.IntersectFrom - read_sector_addr;
                    offset *= sector_size;
                    offset += db.IntersectIndexFrom;

                    UINT64 count = 0;
                    if(db.IntersectFrom == db.IntersectTo)
                    {
                        count = db.IntersectIndexTo - db.IntersectIndexFrom + 1;
                    }
                    else
                    {
                        count = db.IntersectTo - db.IntersectFrom + 1;
                        count *= sector_size;
                        count -= db.IntersectIndexFrom;
                        count -= sector_size;
                        count += db.IntersectIndexTo;
                        count++;
                    }
                    for(UINT64 i = offset; i < offset+count; i++)
                    {
                        if(db.Operation == BITOP_OR)
                            buff_to_write[i] |= db.Value;
                        if(db.Operation == BITOP_AND)
                            buff_to_write[i] &= db.Value;
                    }
                }

                skip = false;
            }

            if(!skip)
            {
                LARGE_INTEGER filepointer;
                DWORD ByteWrite;
                filepointer.QuadPart = write_sector_addr;
                filepointer.QuadPart *= sector_size;
                SetFilePointerEx(handle_dest, filepointer, NULL, FILE_BEGIN);
                if(!WriteFile(handle_dest, buff_to_write, sectors_to_write * sector_size, &ByteWrite, 0))
                {
                    DWORD reason = GetLastError();
                    ErrorEvent* ev = new ErrorEvent(wxString::Format("Write error : %d", reason));
                    wxQueueEvent(event_handler, ev);
                    thread->Terminate();
                    break;
                }
            }

            write_sector_addr += sectors_to_write;
            read_sector_addr += sectors_to_write;
            sector_processed_sum += sectors_to_write;
            double progress_double = ((double)sector_processed_sum / (double) sector_sum)*100.0;
            if(progress != (int)progress_double)
            {
                progress = (int)progress_double;
                if(show_progress)
                    wxQueueEvent(event_handler, new ProgressEvent(progress_double));
            }
        }
        delete [] buff;
    }
}


