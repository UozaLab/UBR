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

#ifndef __VOLUME__H__
#define __VOLUME__H__

#include <windows.h>
#include "tstring.h"
#include "FsInfo.h"

struct VolumeInfo
{
    bool Invalid;

    FSInfo FilesystemInfo;
    tstring VolumeGUIDPath;
    tstring PathNames;// drive letter
    UINT DriveType;
    STORAGE_DEVICE_NUMBER StorageDeviceNumber;// device#/partition#/type

    VolumeInfo()
    {
        Invalid = false;
        VolumeGUIDPath = _T("");
        PathNames = _T("");
        DriveType = 0;
        StorageDeviceNumber.DeviceNumber = 0;
        StorageDeviceNumber.DeviceType = 0;
        StorageDeviceNumber.PartitionNumber = 0;
    }
};

#endif
