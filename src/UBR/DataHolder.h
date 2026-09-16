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

#ifndef __DATA_HOLDER__H__
#define __DATA_HOLDER__H__

#include <wx/wx.h>
#include <vector>
#include "Widgets/DiskPanelImpl.h"
#include "FileSystem/VDisk.h"

class RestoreData
{
public:
    void Clear()
    {
        filepath.Empty();
        mode = DISK_PANEL_SELECTION_NOSELECTION;
        disk_number = 0;
        partition_numbers.clear();
    }
public:
    wxString filepath;
    DiskPanelSelectionMode mode;
    int disk_number;
    std::vector<int> partition_numbers;
};

class CloneData
{
public:
    void Clear()
    {
        mode = DISK_PANEL_SELECTION_NOSELECTION;
        disk_number_src = 0;
        disk_number_dst = 0;
        partition_numbers_src.clear();
        partition_numbers_dst.clear();
        vss = true;
        exact_mode = false;
    }
public:
    DiskPanelSelectionMode mode;
    int disk_number_src;
    int disk_number_dst;
    std::vector<int> partition_numbers_src;
    std::vector<int> partition_numbers_dst;
    bool vss;
    bool exact_mode;
};

class BackupData
{
public:
    void Clear()
    {
        filepath.Empty();
        mode = DISK_PANEL_SELECTION_NOSELECTION;
        disk_number = 0;
        partition_numbers_src.clear();
        vss = true;
        compress = true;
        exact_mode = false;
        filetype = FILE_TYPE_VHDX;
    }
public:
    wxString filepath;
    DiskPanelSelectionMode mode;
    int disk_number;
    std::vector<int> partition_numbers_src;
    bool vss;
    bool compress;
    bool exact_mode;
    FileType filetype;
};

class DataHolder
{
public:
    RestoreData restore_data;
    CloneData clone_data;
    BackupData backup_data;
};

#endif
