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

#include "DiskInfoPanel.h"
#include "Defs.h"

DiskInfoPanel::DiskInfoPanel(wxWindow* parent, wxEvtHandler* eh)
: wxPanel(parent),
  selection_mode(DISK_PANEL_SELECTION_NOSELECTION),
  event_handler(eh)
{
    wxBoxSizer* rootsizer = new wxBoxSizer( wxVERTICAL );

    rounded_panel = new RoundedPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxTAB_TRAVERSAL, wxEmptyString, wxVERTICAL, COLOR_BACKGROUND_RIGHTPANEL, COLOR_BUTTON_FACE);
    rootsizer->Add( rounded_panel, 1, wxEXPAND | wxALL );
	this->SetSizer( rootsizer );
	this->Layout();

    Bind(myEVT_SelectDisk, &DiskInfoPanel::selectdiskEvent, this);
    Bind(myEVT_HoverDisk, &DiskInfoPanel::hoverdiskEvent, this);
    Bind(myEVT_LeaveWindow, &DiskInfoPanel::leavewindowEvent, this);
}

DiskInfoPanel::~DiskInfoPanel()
{
    Unbind(myEVT_LeaveWindow, &DiskInfoPanel::leavewindowEvent, this);
    Unbind(myEVT_HoverDisk, &DiskInfoPanel::hoverdiskEvent, this);
    Unbind(myEVT_SelectDisk, &DiskInfoPanel::selectdiskEvent, this);
}

void DiskInfoPanel::AddDisk(shared_ptr<PhysicalDiskInfo> physical_disk, bool disabled)
{
    if(disk_panels.size() >= 1)
    {
        rounded_panel->GetInnerSizer()->Add(0, 3); // add spacer
    }
    DiskPanelImpl* panel = new DiskPanelImpl( rounded_panel->GetInnerPanel(), this, physical_disk, selection_mode, disabled);
    rounded_panel->GetInnerSizer()->Add(panel, 1, wxEXPAND|wxTOP, 1);
    disk_panels.push_back(panel);
}

void DiskInfoPanel::selectdiskEvent(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    sd->dpi->SetSelected(true);

    for(std::vector<DiskPanelImpl*>::iterator itr = disk_panels.begin(), itr_end = disk_panels.end();
        itr != itr_end; ++itr)
    {
        DiskPanelImpl* dpi = *itr;
        if(sd->dpi != dpi)
        {
            dpi->UnSelectPanels();
        }
    }
    wxQueueEvent(event_handler, event.Clone());
}

void DiskInfoPanel::hoverdiskEvent(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    if(sd->dpi->IsHovered()) return;

    for(std::vector<DiskPanelImpl*>::iterator itr = disk_panels.begin(), itr_end = disk_panels.end();
        itr != itr_end; ++itr)
    {
        DiskPanelImpl* dpi = *itr;
        if(sd->dpi != dpi)
        {
            if(dpi->IsSelected() || dpi->IsDisabled())
            {
                dpi->SetHovered(false);
            }
            else
            {
                dpi->UnSelectPanels();
            }
        }
        else
        {
            dpi->SetHovered(true);
        }
    }
}

void DiskInfoPanel::leavewindowEvent(wxCommandEvent& event)
{
    tostringstream o;
    wxPoint p = ScreenToClient(wxGetMousePosition());
    if(p.x > 5 &&
        p.y > 5 &&
        p.x < GetSize().GetWidth() - 5 &&
        p.y < GetSize().GetHeight() - 5) return;

    for(std::vector<DiskPanelImpl*>::iterator itr = disk_panels.begin(), itr_end = disk_panels.end();
        itr != itr_end; ++itr)
    {
        DiskPanelImpl* dpi = *itr;
        if(dpi->IsSelected())
        {
            dpi->SetHovered(false);
        }
        else
        {
            if(dpi->IsHovered())
                dpi->UnSelectPanels();
        }
    }

}

wxDEFINE_EVENT(myEVT_SelectDisk, wxCommandEvent);
wxDEFINE_EVENT(myEVT_HoverDisk, wxCommandEvent);
wxDEFINE_EVENT(myEVT_LeaveWindow, wxCommandEvent);

