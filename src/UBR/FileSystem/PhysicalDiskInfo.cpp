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

#include "PhysicalDiskInfo.h"

VolumeInfo PhysicalDiskUtil::FindVolumeInfo(shared_ptr<PhysicalDiskInfo> pdi, DWORD PartitionNumber)
{
    for(std::vector<VolumeInfo>::iterator itr = pdi->Volumes.begin(), itr_end = pdi->Volumes.end();
        itr != itr_end; itr++)
    {
        if((*itr).StorageDeviceNumber.PartitionNumber == PartitionNumber)
        {
            return (*itr);
        }
    }

    VolumeInfo vi;
    vi.Invalid = true;
    return vi;
}

void PhysicalDiskUtil::UpdateVolumeInfo(shared_ptr<PhysicalDiskInfo> pdi, VolumeInfo volume_info)
{
    for(size_t i=0; i<pdi->Volumes.size(); i++)
    {
        VolumeInfo vi = pdi->Volumes[i];
        if(vi.StorageDeviceNumber.PartitionNumber == volume_info.StorageDeviceNumber.PartitionNumber)
        {
            pdi->Volumes[i] = volume_info;
            return;
        }
    }
}

bool PhysicalDiskUtil::LockAndDismount(shared_ptr<PhysicalDiskInfo> pdi, std::vector<HANDLE>& handles_opened)
{
    bool ret = true;
    handles_opened.clear();

    for(std::vector<VolumeInfo>::iterator itr = pdi->Volumes.begin(), itr_end = pdi->Volumes.end();
        itr != itr_end; itr++)
    {
        VolumeInfo vi = *itr;
        if(vi.VolumeGUIDPath.empty()) continue;

        HANDLE h = CreateFile(vi.VolumeGUIDPath.c_str(),
                              GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, 0, 0);
        if(h == INVALID_HANDLE_VALUE)
        {
            ret = false;
            break;
        }

        handles_opened.push_back(h);

        DWORD unused;
        if(!DeviceIoControl(h, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &unused, NULL))
        {
            ret = false;
            break;
        }

        if(!DeviceIoControl(h, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &unused, NULL))
        {
            ret = false;
            break;
        }
    }

    if(ret == false)
    {
        for(std::vector<HANDLE>::iterator itr = handles_opened.begin(), itr_end = handles_opened.end();
            itr != itr_end; itr++)
        {
            CloseHandle(*itr);
        }
        handles_opened.clear();
    }

    return ret;
}

void PhysicalDiskUtil::UnLock(std::vector<HANDLE>& handles_opened)
{
    for(std::vector<HANDLE>::iterator itr = handles_opened.begin(), itr_end = handles_opened.end();
        itr != itr_end; itr++)
    {
        DWORD unused;
        DeviceIoControl(*itr, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &unused, NULL);

        CloseHandle(*itr);
    }
    handles_opened.clear();
}
