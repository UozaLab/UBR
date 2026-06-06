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

#include "VDiskFactory.h"
#include "VDisk_vhd.h"
#include "VDisk_vhdx.h"
#include "VDisk_raw.h"
#include "VDisk_physical.h"


shared_ptr<VirtualDisk> VirtualDiskFactory::Create(const TCHAR* _filename)
{
    VirtualDisk* vdisk = NULL;
    
    vdisk = new VHD(_filename);
    if(vdisk->IsValid()) return shared_ptr<VirtualDisk>(vdisk);
    delete vdisk;

    vdisk = new VHDX(_filename);
    if(vdisk->IsValid()) return shared_ptr<VirtualDisk>(vdisk);
    delete vdisk;

    vdisk = new RAW(_filename);
    if(vdisk->IsValid()) return shared_ptr<VirtualDisk>(vdisk);
    delete vdisk;

    return shared_ptr<VirtualDisk>();

}

shared_ptr<VirtualDisk> VirtualDiskFactory::Create(int disk_number)
{
    shared_ptr<VirtualDisk> vdisk(new Physical(disk_number));
    return vdisk;
}
