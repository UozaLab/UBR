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

#ifndef __DiskPanelImpl__
#define __DiskPanelImpl__

#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/statline.h>
#include <vector>

#include "smart_ptr.h"
#include "ProgressPanel.h"
#include "FileSystem/VolumeInfo.h"
#include "FileSystem/PhysicalDiskInfo.h"
#include "RoundedPanel.h"

enum DiskPanelSelectionMode
{
    DISK_PANEL_SELECTION_NOSELECTION,
    DISK_PANEL_SELECTION_PARTITION,
    DISK_PANEL_SELECTION_PARTITION_MULTI,
    DISK_PANEL_SELECTION_DISK,
};

class DiskPanelImpl;
struct SelectionData : public wxObject
{
    DiskPanelSelectionMode mode;
    DiskPanelImpl* dpi;
    int disk_number;
    std::vector<int> partition_numbers;
};

class DiskPanelImpl : public wxPanel
{
private:
    static const int HEAD_PANEL_WIDTH = 80;
    static const int MIN_PANEL_WIDTH = 40;
    bool selected;
    bool hovered;
    bool disabled;
    DiskPanelSelectionMode selection_mode;

private:
    void mouseMoved (wxMouseEvent& e);
    void mouseDown (wxMouseEvent& e);
    void mouseReleased (wxMouseEvent& e);
    void mouseLeftWindow (wxMouseEvent& e);
    void mouseEnterWindow (wxMouseEvent& e);
    void sizeEvent(wxSizeEvent&);

protected:
    wxBoxSizer* bSizer;
    wxEvtHandler* event_handler;
    shared_ptr<PhysicalDiskInfo> physical_disk;
    wxPanel* AddTitlePanel();
    wxPanel* AddDrivePanel(const PARTITION_INFORMATION_EX* partition_info, const VolumeInfo& volume_info, double ratio);
    std::vector<double> ratios;
    std::vector<std::pair<bool, wxPanel*>> child_panels;
    bool loose_mode;
    SelectionData sd;
    wxString guid2string(GUID guid);

public:
    DiskPanelImpl( wxWindow* _parent, wxEvtHandler* eh, shared_ptr<PhysicalDiskInfo> _physical_disk, DiskPanelSelectionMode mode, bool _disabled);
    void UnSelectPanels();

    void SetSelected(bool tf) { selected = tf; }
    bool IsSelected() { return selected; }
    void SetHovered(bool tf) { hovered = tf; }
    bool IsHovered() { return hovered; }
    bool IsDisabled() { return disabled; }

    DECLARE_EVENT_TABLE()
};


template <class W>
class EventPropagate : public W
{
protected:
    wxWindow* parent;
public:
    EventPropagate(wxWindow* _parent) : parent(_parent), W(_parent) {}
    EventPropagate(wxWindow* _parent, bool call_constructor_noarg) : parent(_parent), W() {}
    virtual ~EventPropagate(){}
    virtual void SetPropagateParent(wxWindow* _parent) { parent = _parent; }

    void propagate(wxMouseEvent& e) 
    { 
        wxEvent* ec = e.Clone();
        ec->SetEventObject(this);
        wxQueueEvent(parent->GetEventHandler(), ec);
    }

    DECLARE_EVENT_TABLE()
};

#define BEGIN_EVENT_TABLE_TEMPLATE1_SPECIALIZED(theClass, baseClass, T1) \
    template<> \
    const wxEventTable theClass<T1>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1>::sm_eventTableEntries[0] }; \
    template<> \
    const wxEventTable *theClass<T1>::GetEventTable() const \
        { return &theClass<T1>::sm_eventTable; } \
    template<> \
    wxEventHashTable theClass<T1>::sm_eventHashTable(theClass<T1>::sm_eventTable); \
    template<> \
    wxEventHashTable &theClass<T1>::GetEventHashTable() const \
        { return theClass<T1>::sm_eventHashTable; } \
    template<> \
    const wxEventTableEntry theClass<T1>::sm_eventTableEntries[] = { \


#define IMPL_PROPAGATE_EVENT_TABLE(W) \
BEGIN_EVENT_TABLE_TEMPLATE1_SPECIALIZED(EventPropagate, W, W)\
    EVT_MOTION(EventPropagate<W>::propagate)\
    EVT_LEFT_DOWN(EventPropagate<W>::propagate)\
    EVT_LEFT_UP(EventPropagate<W>::propagate)\
    EVT_ENTER_WINDOW(EventPropagate<W>::propagate)\
    EVT_LEAVE_WINDOW(EventPropagate<W>::propagate)\
END_EVENT_TABLE()


#endif
