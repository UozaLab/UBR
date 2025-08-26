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

#include "DiskPanelImpl.h"
#include "Defs.h"
#include "resource.h"
#include "Misc.h"
#include "MiscWx.h"
#include "DiskInfoPanel.h"

DiskPanelImpl::DiskPanelImpl( wxWindow* _parent, wxEvtHandler* eh, shared_ptr<PhysicalDiskInfo> _physical_disk, DiskPanelSelectionMode mode, bool _disabled)
: wxPanel(_parent),
  physical_disk(_physical_disk),
  selected(false),
  hovered(false),
  disabled(_disabled),
  event_handler(eh),
  selection_mode(mode)
{
    SetBackgroundColour( disabled ? COLOR_BACKGROUND_DISABLED_RIGHTPANEL : COLOR_BACKGROUND_RIGHTPANEL );
    SetForegroundColour( disabled ? COLOR_BACKGROUND_DISABLED_RIGHTPANEL : COLOR_BACKGROUND_RIGHTPANEL );

    bSizer = new wxBoxSizer( wxHORIZONTAL );
    child_panels.push_back(std::pair<bool, wxPanel*>(true, AddTitlePanel()));

    double ratio_sum = 0;
    for(std::vector<PARTITION_INFORMATION_EX>::iterator itr = physical_disk->Partitions.begin(), itr_end = physical_disk->Partitions.end();
        itr != itr_end; ++itr)
    {
        PARTITION_INFORMATION_EX partition_info = *itr;
        if(partition_info.PartitionStyle == PARTITION_STYLE_MBR && partition_info.Mbr.PartitionType == 0x00)
            continue;
        double ratio = (double) partition_info.PartitionLength.QuadPart / (double) physical_disk->DiskSize;
        ratios.push_back(ratio);
        ratio_sum += ratio;
        child_panels.push_back(std::pair<bool, wxPanel*>(false, AddDrivePanel(&(*itr), PhysicalDiskUtil::FindVolumeInfo(physical_disk, (*itr).PartitionNumber))));
    }
    loose_mode = (ratio_sum < 0.8);


    this->SetSizer(bSizer);
    this->Layout();
	bSizer->Fit( this );
}

wxPanel* DiskPanelImpl::AddTitlePanel()
{
    wxPanel* m_title_panel;
    wxStaticBitmap* m_bitmap1;
    wxStaticText* m_staticText11;
    wxStaticText* m_staticText12;
    wxStaticText* m_staticText13;

	m_title_panel = new EventPropagate<wxPanel>(this);//new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    m_title_panel->SetBackgroundColour( disabled ? COLOR_BACKGROUND_DISABLED_RIGHTPANEL : COLOR_BACKGROUND_RIGHTPANEL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );

    // bitmap
    HRSRC hbin = FindResource(NULL, MAKEINTRESOURCE(IDB_PNG12), RT_RCDATA);
    const BYTE *bindata = (const BYTE*)LockResource(LoadResource(0, hbin));
    DWORD binsize = SizeofResource(0, hbin);
    wxBitmap bitmap = wxBitmap::NewFromPNGData(bindata, binsize);

	m_bitmap1 = new EventPropagate<wxStaticBitmap>(m_title_panel, true);//( m_title_panel, wxID_ANY, bitmap, wxDefaultPosition, wxDefaultSize, 0 );
    m_bitmap1->Create( m_title_panel, wxID_ANY, bitmap, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer10->Add( m_bitmap1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );

    wxString disk_number;
    disk_number.Printf("Disk %d", physical_disk->DeviceNumber);
    wxString PartitionStyle = (physical_disk->PartitionStyle == PARTITION_STYLE_MBR) ? "MBR":
        (physical_disk->PartitionStyle == PARTITION_STYLE_GPT) ? "GPT":
        (physical_disk->PartitionStyle == PARTITION_STYLE_RAW) ? "RAW":"Unknown";
    wxString disk_size = Unit::HumanReadable(physical_disk->DiskSize);

	m_staticText11 = new EventPropagate<wxStaticText>(m_title_panel, true);// m_title_panel, wxID_ANY, disk_number, wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText11->Create(m_title_panel, wxID_ANY, disk_number, wxDefaultPosition, wxDefaultSize, 0);
	m_staticText11->Wrap( -1 );
	bSizer10->Add( m_staticText11, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );


	bSizer9->Add( bSizer10, 1, wxEXPAND, 1 );

	m_staticText12 = new EventPropagate<wxStaticText>(m_title_panel, true);// m_title_panel, wxID_ANY, PartitionStyle, wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText12->Create(m_title_panel, wxID_ANY, PartitionStyle, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	bSizer9->Add( m_staticText12, 0, wxALL, 2 );

	m_staticText13 = new EventPropagate<wxStaticText>(m_title_panel, true);//( m_title_panel, wxID_ANY, disk_size, wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText13->Create( m_title_panel, wxID_ANY, disk_size, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer9->Add( m_staticText13, 0, wxALL, 2 );


	bSizer9->Add( HEAD_PANEL_WIDTH, 0, 1, wxEXPAND, 1 );


	m_title_panel->SetSizer( bSizer9 );
	m_title_panel->Layout();
	bSizer9->Fit( m_title_panel );
	bSizer->Add( m_title_panel, 0, wxEXPAND | wxALL, 1 );

    return m_title_panel;
}

wxString DiskPanelImpl::guid2string(GUID guid)
{
    GUID Reserved = { 0xE3C9E316, 0x0B5C, 0x4DB8, { 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE } };
    GUID Recovery = { 0xDE94BBA4, 0x06D1, 0x4D40, { 0xA1, 0x6A, 0xBF, 0xD5, 0x01, 0x79, 0xD6, 0xAC } };
    GUID System = { 0xC12A7328, 0xF81F, 0x11D2, { 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B } };

    if(IsEqualGUID(guid, Reserved)) return wxString(_T("Reserved"));
    if(IsEqualGUID(guid, Recovery)) return wxString(_T("Recovery"));
    if(IsEqualGUID(guid, System)) return wxString(_T("EFI System"));
    return wxEmptyString;
}


wxPanel* DiskPanelImpl::AddDrivePanel(const PARTITION_INFORMATION_EX* partition_info, const VolumeInfo& volume_info)
{
    wxPanel* m_panel1;
    ProgressPanel* m_customControl1;
    wxStaticText* m_staticText1;
    wxStaticText* m_staticText2;
    //wxStaticLine* m_staticline3;


	m_panel1 = new EventPropagate<wxPanel>(this);//( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel1->SetForegroundColour( COLOR_BACKGROUND_RIGHTPANEL_DRIVE );
	m_panel1->SetBackgroundColour( COLOR_BACKGROUND_RIGHTPANEL_DRIVE );

	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_customControl1 = new EventPropagate<ProgressPanel>(m_panel1, true); //( m_panel1, wxID_ANY, wxDefaultPosition, wxSize( -1,15 ), 0 );
    m_customControl1->Create( m_panel1, wxID_ANY, wxDefaultPosition, wxSize( -1,15 ), 0 );
    if(!volume_info.Invalid)
    {
        m_customControl1->SetProgress((int) (volume_info.FilesystemInfo.SizeInfo.UsedRatio*100.0));
    }
    else
    {
        m_customControl1->SetProgress(0);
    }
	bSizer3->Add( m_customControl1, 0, wxALL|wxEXPAND, 3 );

    wxString part_size;
    part_size.Printf("%s %s", Unit::HumanReadable((unsigned long long) partition_info->PartitionLength.QuadPart),
                              (volume_info.Invalid) ? wxEmptyString : wxString(volume_info.FilesystemInfo.TypeInfo.FileSystemName));
    wxString fs_info = wxEmptyString;
    GUID MicrosoftBasicData = { 0xEBD0A0A2, 0xB9E5, 0x4433, { 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7 } };
    if(partition_info->PartitionStyle == PARTITION_STYLE_GPT &&
        !IsEqualGUID(partition_info->Gpt.PartitionType, MicrosoftBasicData))
    {
        fs_info = guid2string(partition_info->Gpt.PartitionType);//partition_info->Gpt.Name;
    }
    else
    {
        wxString drivename = wxString(volume_info.PathNames);
        if(drivename.ends_with("\\"))
            drivename = drivename.substr(0, drivename.Length() - 1);
        fs_info.Printf("%s%s", drivename, wxString(volume_info.FilesystemInfo.TypeInfo.VolumeName));
    }

	m_staticText1 = new EventPropagate<wxStaticText>(m_panel1, true);//( m_panel1, wxID_ANY, fs_info, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
    m_staticText1->Create( m_panel1, wxID_ANY, fs_info, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_staticText1->Wrap( -1 );
	m_staticText1->SetForegroundColour( COLOR_BUTTON_TEXT );

	bSizer3->Add( m_staticText1, 0, wxALL, 5 );

	m_staticText2 = new EventPropagate<wxStaticText>(m_panel1, true);//( m_panel1, wxID_ANY, part_size, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
    m_staticText2->Create( m_panel1, wxID_ANY, part_size, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_staticText2->Wrap( -1 );
	m_staticText2->SetForegroundColour( COLOR_BUTTON_TEXT );

	bSizer3->Add( m_staticText2, 0, wxALL, 5 );


	bSizer3->Add( 0, 0, 1, wxEXPAND, 5 );
	m_panel1->SetSizer( bSizer3 );
	m_panel1->Layout();
	bSizer3->Fit( m_panel1 );

    double ratio = (double) partition_info->PartitionLength.QuadPart / (double) physical_disk->DiskSize;
    bSizer->Add( m_panel1, 100*ratio, wxEXPAND|wxBOTTOM|wxRIGHT|wxTOP, 3 );

    wxString used_space = wxEmptyString;
    if(!volume_info.Invalid && volume_info.FilesystemInfo.SizeInfo.SizeCalculated)
        used_space = Unit::HumanReadable(volume_info.FilesystemInfo.SizeInfo.Used);

    wxString hint = wxString::Format("%s\r\n%s : %s%s", 
        fs_info, 
        ttt("Part_Capacity"), part_size,
        (used_space == wxEmptyString) ? wxEmptyString :
        wxString::Format("\r\n%s : %s", ttt("Part_UsedSpace"), used_space));
    m_panel1->SetToolTip(hint);
    m_staticText1->SetToolTip(hint);
    m_staticText2->SetToolTip(hint);
    m_customControl1->SetToolTip(hint);

    return m_panel1;
}

void DiskPanelImpl::UnSelectPanels()
{
    if(disabled) return;
    selected = false;
    hovered = false;
    SetBackgroundColour( COLOR_BACKGROUND_RIGHTPANEL );
    for(std::vector<std::pair<bool, wxPanel*>>::iterator itr = child_panels.begin(), itr_end = child_panels.end();
        itr != itr_end; ++itr)
    {
        bool title_panel = itr->first;
        wxPanel* p = itr->second;
        p->SetBackgroundColour( title_panel ? COLOR_BACKGROUND_RIGHTPANEL : COLOR_BACKGROUND_RIGHTPANEL_DRIVE );
    }
    Refresh();
}

void DiskPanelImpl::sizeEvent(wxSizeEvent & evt)
{
    wxWindow* parent = this->GetParent();
    wxSize size = parent->GetClientSize();
    this->SetMaxSize(wxSize(size.GetWidth(), -1));
    this->SetSize(wxSize(size.GetWidth(), -1));

    for(unsigned int i=1; i<bSizer->GetItemCount(); i++) // skip first item
    {
        wxSizerItem* item = bSizer->GetItem(i);
        wxWindow* win = item->GetWindow();
        if(win == NULL) continue;
        int width = (size.GetWidth()-HEAD_PANEL_WIDTH)*ratios[i-1];
        int width_min = loose_mode ? MIN_PANEL_WIDTH*3 : MIN_PANEL_WIDTH;
        if(width < width_min)
            width = width_min;
        win->SetMaxSize(wxSize(width, -1));
        item->SetInitSize(MIN_PANEL_WIDTH, -1);
    }

    evt.Skip();
}

void DiskPanelImpl::mouseDown (wxMouseEvent& e)
{
    if(selection_mode == DISK_PANEL_SELECTION_NOSELECTION) return;
    if(disabled) return;

    if(selection_mode == DISK_PANEL_SELECTION_DISK)
    {
        if(!selected)
        {
            SetBackgroundColour( COLOR_BACKGROUND_SELECTED );
            for(std::vector<std::pair<bool, wxPanel*>>::iterator itr = child_panels.begin(), itr_end = child_panels.end();
                itr != itr_end; ++itr)
            {
                bool title_panel = itr->first;
                wxPanel* p = itr->second;
                p->SetBackgroundColour( title_panel ? COLOR_BACKGROUND_SELECTED : COLOR_BACKGROUND_SELECTED_PALE );
            }
            Refresh();
        }
        wxCommandEvent* e = new wxCommandEvent(myEVT_SelectDisk);
        {
            sd.dpi = this;
            sd.mode = DISK_PANEL_SELECTION_DISK;
            sd.partition_numbers.clear();
            sd.disk_number = physical_disk->DeviceNumber;
        }
        e->SetEventObject(&sd);
        wxQueueEvent(event_handler, e);
        return;
    }
#if 0
    wxObject* caller = e.GetEventObject();
    wxPanel* panel = dynamic_cast<wxPanel*>(caller);
    if(panel == NULL) return;
    panel->SetBackgroundColour( COLOR_BACKGROUND_SELECTED );
    Refresh();
#endif
}

void DiskPanelImpl::mouseMoved (wxMouseEvent& e)
{
}
void DiskPanelImpl::mouseReleased (wxMouseEvent& e)
{
}

void DiskPanelImpl::mouseLeftWindow (wxMouseEvent& e)
{
    if(selection_mode == DISK_PANEL_SELECTION_NOSELECTION) return;

    wxCommandEvent* ev = new wxCommandEvent(myEVT_LeaveWindow);
    ev->SetEventObject(&sd);
    wxQueueEvent(event_handler, ev);
}

void DiskPanelImpl::mouseEnterWindow (wxMouseEvent& e)
{
    if(selection_mode == DISK_PANEL_SELECTION_NOSELECTION) return;

    wxCommandEvent* ev = new wxCommandEvent(myEVT_HoverDisk);
    {
        sd.dpi = this;
        sd.mode = DISK_PANEL_SELECTION_DISK;
        sd.partition_numbers.clear();
        sd.disk_number = physical_disk->DeviceNumber;
    }
    ev->SetEventObject(&sd);
    wxQueueEvent(event_handler, ev);

    if(disabled) return;
    if(selected) return;
    if(!hovered)
    {
        SetBackgroundColour( COLOR_BACKGROUND_HOVER );
        Refresh();
    }

}

BEGIN_EVENT_TABLE(DiskPanelImpl, wxPanel)
    EVT_MOTION(DiskPanelImpl::mouseMoved)
    EVT_LEFT_DOWN(DiskPanelImpl::mouseDown)
    EVT_LEFT_UP(DiskPanelImpl::mouseReleased)
    EVT_LEAVE_WINDOW(DiskPanelImpl::mouseLeftWindow)
    EVT_ENTER_WINDOW(DiskPanelImpl::mouseEnterWindow)
    EVT_SIZE(DiskPanelImpl::sizeEvent)
END_EVENT_TABLE()


IMPL_PROPAGATE_EVENT_TABLE(wxPanel)
IMPL_PROPAGATE_EVENT_TABLE(wxStaticText)
IMPL_PROPAGATE_EVENT_TABLE(wxStaticBitmap)
IMPL_PROPAGATE_EVENT_TABLE(ProgressPanel)
