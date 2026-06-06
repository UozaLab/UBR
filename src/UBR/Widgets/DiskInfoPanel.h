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

#ifndef __DISKINFOPANEL_H__
#define __DISKINFOPANEL_H__

#include <vector>
#include <wx/wx.h>
#include <wx/panel.h>
#include "smart_ptr.h"
#include "RoundedPanel.h"
#include "DiskPanelImpl.h"
#include "FileSystem/VolumeInfo.h"


class DiskInfoPanel : public wxPanel
{
protected:
    RoundedPanel* rounded_panel;
    wxEvtHandler* event_handler;
    std::vector<DiskPanelImpl*> disk_panels;
    DiskPanelSelectionMode selection_mode;

public:
    DiskInfoPanel(wxWindow* parent, wxEvtHandler* eh);
    ~DiskInfoPanel();

    void AddDisk(shared_ptr<PhysicalDiskInfo> physical_disk, bool disabled);
    void SetSelectionMode(DiskPanelSelectionMode mode) { selection_mode = mode; }

    void selectdiskEvent(wxCommandEvent& event);
    void hoverdiskEvent(wxCommandEvent& event);
    void leavewindowEvent(wxCommandEvent& event);

};

wxDECLARE_EVENT(myEVT_SelectDisk, wxCommandEvent);
wxDECLARE_EVENT(myEVT_HoverDisk, wxCommandEvent);
wxDECLARE_EVENT(myEVT_LeaveWindow, wxCommandEvent);

#endif