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

#include "DiskInfo.h"
#include "VDisk_physical.h"
#include "ForensicAnalysis.h"

#include <setupapi.h>
#pragma comment(lib, "SetupAPI.lib")

shared_ptr<DiskInfo> DiskInfoFactory::CreateDiskInfo()
{
    shared_ptr<std::map<DWORD, std::map<DWORD, VolumeInfo>>> volume_infos = get_volume_infos();
    shared_ptr<DiskInfo> disk_info(new DiskInfo());

    std::vector<DWORD> disk_numbers = get_disk_numbers();
    for(std::vector<DWORD>::iterator itr = disk_numbers.begin(), itr_end = disk_numbers.end();
        itr != itr_end; itr++)
    {
        DWORD disk_number = *itr;

        shared_ptr<Physical> vdisk(new Physical(disk_number));
        shared_ptr<PhysicalDiskInfo> physical_disk = ForensicAnalysis::CreateDiskInfo(vdisk);
        physical_disk->DeviceNumber = disk_number;

        if(volume_infos->count(disk_number))
        {
            std::map<DWORD, VolumeInfo> volumes = (*volume_infos)[disk_number];
            for(std::map<DWORD, VolumeInfo>::iterator itr = volumes.begin(), itr_end = volumes.end(); itr != itr_end; itr++)
            {
                DWORD partition_number = itr->first;
                VolumeInfo vi = PhysicalDiskUtil::FindVolumeInfo(physical_disk, partition_number);
                if(!vi.Invalid)
                {
                    VolumeInfo vi_new = itr->second;
                    vi.PathNames = vi_new.PathNames;
                    vi.VolumeGUIDPath = vi_new.VolumeGUIDPath;
                    if(vi.FilesystemInfo.TypeInfo.PartitionType == PART_TYPE_BitLocker ||
                        vi.FilesystemInfo.TypeInfo.FileSystemType == FS_TYPE_exFAT)
                    {
                        vi.FilesystemInfo.TypeInfo.VolumeName = vi_new.FilesystemInfo.TypeInfo.VolumeName;
                    }
                    PhysicalDiskUtil::UpdateVolumeInfo(physical_disk, vi);
                }
            }
        }

        disk_info->PhysicalDisks[disk_number] = physical_disk;
    }

    return disk_info;
}


std::vector<DWORD> DiskInfoFactory::get_disk_numbers()
{
    std::vector<DWORD> disk_numbers;

    HANDLE disk = INVALID_HANDLE_VALUE;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = NULL;
    GUID diskClassDeviceInterfaceGuid = GUID_DEVINTERFACE_DISK;
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&diskClassDeviceInterfaceGuid,
                                            NULL,
                                            NULL,
                                            DIGCF_DEVICEINTERFACE|DIGCF_PRESENT);
    if( hDevInfo == INVALID_HANDLE_VALUE ) return disk_numbers;;

    int index = 0;
    while(TRUE)
    {
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        ZeroMemory(&deviceInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if(!SetupDiEnumDeviceInterfaces(hDevInfo,
                                        NULL,
                                        &diskClassDeviceInterfaceGuid,
                                        index,
                                        &deviceInterfaceData))
          break;

        DWORD requiredSize;
        SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                        &deviceInterfaceData,
                                        NULL,
                                        0,
                                        &requiredSize,
                                        NULL);
        if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
          break;

        deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
        if(deviceInterfaceDetailData == NULL)
          break;
        ZeroMemory(deviceInterfaceDetailData, requiredSize);
        deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        SP_DEVINFO_DATA deviceInfoData;
        ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if(!SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                            &deviceInterfaceData,
                                            deviceInterfaceDetailData,
                                            requiredSize,
                                            &requiredSize,
                                            &deviceInfoData))
          break;

        disk = CreateFile(deviceInterfaceDetailData->DevicePath,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL );
        if(disk == INVALID_HANDLE_VALUE)
          break;


        // disk type
        DWORD bytesReturned;
        STORAGE_PROPERTY_QUERY storagePropertyQuery;
        ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
        storagePropertyQuery.PropertyId = StorageDeviceProperty;
        storagePropertyQuery.QueryType = PropertyStandardQuery;

        char propQueryOut[8192] = { 0 };
        if(!DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY,
            &storagePropertyQuery, sizeof(storagePropertyQuery),
            &propQueryOut, sizeof(propQueryOut), &bytesReturned, NULL))
            break;
        STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*) &propQueryOut[0];
        if(descriptor->RemovableMedia != 0)
        {
            index++;
            continue;
        }


        // disk number
        STORAGE_DEVICE_NUMBER diskNumber;
        if(!DeviceIoControl(disk,
                            IOCTL_STORAGE_GET_DEVICE_NUMBER,
                            NULL,
                            0,
                            &diskNumber,
                            sizeof( STORAGE_DEVICE_NUMBER ),
                            &bytesReturned,
                            NULL))
          break;

        disk_numbers.push_back(diskNumber.DeviceNumber);

        free(deviceInterfaceDetailData);
        deviceInterfaceDetailData = NULL;
        CloseHandle( disk );
        disk = INVALID_HANDLE_VALUE;
        
        index++;
    }

    if(deviceInterfaceDetailData != NULL) free(deviceInterfaceDetailData);
    SetupDiDestroyDeviceInfoList( hDevInfo );
    if(disk != INVALID_HANDLE_VALUE) CloseHandle(disk);

    return disk_numbers;
}

shared_ptr<std::map<DWORD, std::map<DWORD, VolumeInfo>>> DiskInfoFactory::get_volume_infos()
{
    // map<DiskNumber, map<PartitionNumber, VolumeName>>
    shared_ptr<std::map<DWORD, std::map<DWORD, VolumeInfo>>> result(new std::map<DWORD, std::map<DWORD, VolumeInfo>>());

    TCHAR VolName[MAX_PATH+1];
    TCHAR VolNameNoBSlash[MAX_PATH+1];
    
    HANDLE hVolume = FindFirstVolume(VolName, MAX_PATH);
    if(hVolume == INVALID_HANDLE_VALUE) return result;

    do
    {
        VolumeInfo vi;
        _tcsncpy_s(VolNameNoBSlash, MAX_PATH+1, VolName, MAX_PATH+1);
        VolNameNoBSlash[_tcslen(VolNameNoBSlash) - 1] = _T('\0');

        HANDLE h = CreateFile(VolNameNoBSlash,
                              FILE_READ_ATTRIBUTES | SYNCHRONIZE | FILE_TRAVERSE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              NULL, OPEN_EXISTING, 0, 0);
        if(h == INVALID_HANDLE_VALUE)
          continue;


        DWORD bytesReturned;
        STORAGE_DEVICE_NUMBER StorageDeviceNumber;
        if(!DeviceIoControl(h,
                            IOCTL_STORAGE_GET_DEVICE_NUMBER,
                            NULL,
                            0,
                            &StorageDeviceNumber,
                            sizeof(STORAGE_DEVICE_NUMBER),
                            &bytesReturned,
                            NULL))
        {
            CloseHandle(h);
            continue;
        }
        CloseHandle(h);

        vi.VolumeGUIDPath = tstring(VolNameNoBSlash);
		vi.StorageDeviceNumber = StorageDeviceNumber;
		(*result)[StorageDeviceNumber.DeviceNumber][StorageDeviceNumber.PartitionNumber] = vi;

        TCHAR VolumeName[MAX_PATH + 1];// UNICODE
        TCHAR FileSystemName[MAX_PATH + 1];
        DWORD SerialNumber;
        DWORD MaxFileNameLength;
        DWORD FileSystemFlags;
        if(!GetVolumeInformation(VolName,
                                 VolumeName,
                                 _countof(VolumeName),
                                 &SerialNumber,
                                 &MaxFileNameLength,
                                 &FileSystemFlags,
                                 FileSystemName,
                                 _countof(FileSystemName)))
        {
            if(GetLastError()!=ERROR_NOT_READY)
			{
				continue;
			}
        }

        vi.FilesystemInfo.TypeInfo.FileSystemName = tstring(FileSystemName);
        vi.FilesystemInfo.TypeInfo.VolumeName = tstring(VolumeName);
        (*result)[StorageDeviceNumber.DeviceNumber][StorageDeviceNumber.PartitionNumber] = vi;

        DWORD length = 0;
        TCHAR PathNames[MAX_PATH + 1];
        if(!GetVolumePathNamesForVolumeName(VolName, PathNames, MAX_PATH, &length))
          continue;

        vi.PathNames = tstring(PathNames);
        (*result)[StorageDeviceNumber.DeviceNumber][StorageDeviceNumber.PartitionNumber] = vi;

    }while(FindNextVolume(hVolume, VolName, MAX_PATH));

    FindVolumeClose(hVolume);

    return result;
}
