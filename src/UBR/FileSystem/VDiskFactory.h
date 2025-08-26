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

#ifndef __VDISK_FACTORY__H__
#define __VDISK_FACTORY__H__

#include <windows.h>
#include "smart_ptr.h"
#include "VDisk.h"

class VirtualDiskFactory
{
public:
    static shared_ptr<VirtualDisk> Create(const TCHAR* _filename);
    static shared_ptr<VirtualDisk> Create(int disk_number);
};

#endif
