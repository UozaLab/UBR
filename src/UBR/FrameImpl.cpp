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

#include "FrameImpl.h"
#include "resource.h"
#include "Widgets/Defs.h"
#include "Widgets/DiskInfoPanel.h"
#include "Widgets/PrevNextPanel.h"
#include "FileSystem/ForensicAnalysis.h"
#include "FileSystem/VdiskFactory.h"
#include "Worker/Restore.h"
#include "Misc.h"
#include <windows.h>
#include <wx/mstream.h>
#include <wx/msw/private.h>
#include <wx/dir.h>

wxFrame* Container_Util::BaseFrame = nullptr;
wxFrame* Container_Util::CurrentFrame = nullptr;

void Container_Util::SetNewframe(wxFrame* newframe)
{
    if(BaseFrame == nullptr)
    {
        BaseFrame = newframe;
        return;
    }
    
    wxWindow* panel_root = BaseFrame->FindWindow(wxID_PANEL_ROOT);
    const wxWindowList nodes = panel_root->GetChildren();
    for(wxWindowList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        wxWindow* win = *it;
        win->Show(false);
    }
    
    if(CurrentFrame != NULL)
    {
        CurrentFrame->Close();
        delete CurrentFrame;
        panel_root->DestroyChildren();
    }
    
    CurrentFrame = newframe;
    
    wxWindow* found_panel = newframe->FindWindow(wxID_PANEL_ROOT);
    if(found_panel != nullptr)
    {
        found_panel->Reparent(panel_root);
        wxSizer* sizer = found_panel->GetContainingSizer();
        sizer->Detach(found_panel);
        panel_root->GetSizer()->Add(found_panel, 1, wxEXPAND | wxALL, 0);
        panel_root->Layout();
    }
}

//
// ProgressHandler
//
ProgressHandler::ProgressHandler(wxEvtHandler* _event_handler)
     : event_handler(_event_handler), ready_to_goback(false), ready_to_gonext(false), error(false)
{
    event_handler->Bind(myEVT_PROGRESS, &ProgressHandler::OnProgress, this);
    event_handler->Bind(myEVT_MSG, &ProgressHandler::OnMsg, this);
    event_handler->Bind(myEVT_ERROR, &ProgressHandler::OnError, this);
    event_handler->Bind(wxEVT_THREAD, &ProgressHandler::OnThreadEvent, this);
}

ProgressHandler::~ProgressHandler()
{
    event_handler->Unbind(wxEVT_THREAD, &ProgressHandler::OnThreadEvent, this);
    event_handler->Unbind(myEVT_ERROR, &ProgressHandler::OnError, this);
    event_handler->Unbind(myEVT_MSG, &ProgressHandler::OnMsg, this);
    event_handler->Unbind(myEVT_PROGRESS, &ProgressHandler::OnProgress, this);
}

//
// Container_Common_ProgressImpl
//
Container_Common_ProgressImpl::Container_Common_ProgressImpl(FrameImpl* _frameimpl)
     : Container_Common_Progress(nullptr), ProgressHandler(this), frameimpl_p(_frameimpl)
{
}

void Container_Common_ProgressImpl::OnProgress(ProgressEvent& event)
{
    progress->SetProgress(event.GetData());
}
void Container_Common_ProgressImpl::OnMsg(MsgEvent& event)
{
    write_msg(event);
}
void Container_Common_ProgressImpl::OnError(ErrorEvent& event)
{
    progress->SetError(true);
    write_msg(wxString::Format("### ERROR ### %s", event.GetData()), wxColour(255, 0, 0));
}
void Container_Common_ProgressImpl::OnThreadEvent(wxThreadEvent& event)
{
    ThreadState thread_state = event.GetPayload<ThreadState>();

    if(thread_state == THREAD_STARTED)
    {
        frameimpl_p->PauseButtons(true);
    }
    else if(thread_state == THREAD_FINISHED)
    {
        frameimpl_p->PauseButtons(false);
        m_prevnext->SetTextNext(ttt("FinishBtnText"));
        ready_to_gonext = true;
    }
    else if(thread_state == THREAD_TERMINATING)
    {
        m_prevnext->SetTextNext(ttt("CancelingBtnText"));
    }
    else if(thread_state == THREAD_TERMINATED)
    {
        frameimpl_p->PauseButtons(false);
        m_prevnext->SetTextNext(ttt("CanceledBtnText"));
        ready_to_goback = true;
    }
}

void Container_Common_ProgressImpl::write_msg(const wxString& str)
{
    m_textCtrl1->AppendText(wxDateTime::Now().Format(wxString("%H:%M:%S ")));
    m_textCtrl1->AppendText(wxString::Format("%s\r\n", str));
}

void Container_Common_ProgressImpl::write_msg(const wxString& str, wxColor color)
{
    m_textCtrl1->AppendText(wxDateTime::Now().Format(wxString("%H:%M:%S ")));
    long start = m_textCtrl1->GetInsertionPoint();
    m_textCtrl1->AppendText(str);
    long end = m_textCtrl1->GetInsertionPoint();
    wxTextAttr style(color);
    m_textCtrl1->SetStyle(start, end, style);
    m_textCtrl1->AppendText("\r\n");
}

void Container_Common_ProgressImpl::write_msg(const MsgEvent& msg)
{
    long start = m_textCtrl1->GetInsertionPoint();
    if(msg.IsControled())
    {
        m_textCtrl1->AppendText(wxDateTime::Now().Format(wxString("%H:%M:%S ")));
        start = m_textCtrl1->GetInsertionPoint();
        m_textCtrl1->AppendText(msg.GetData());
    }
    else
    {
        m_textCtrl1->AppendText(msg.GetData());
    }
    long end = m_textCtrl1->GetInsertionPoint();

    if(msg.IsColorized())
    {
        wxTextAttr style(msg.GetColor());
        m_textCtrl1->SetStyle(start, end, style);
    }
    if(msg.AppendCRLF()) m_textCtrl1->AppendText("\r\n");
}


//
// FrameImpl
//
FrameImpl::FrameImpl()
: BaseFrame( nullptr )
{
    HICON hicon = (HICON) LoadIcon(wxGetInstance(), MAKEINTRESOURCE(IDI_ICON1));
    wxIcon icon;
    icon.CreateFromHICON(hicon);
    SetIcon(icon);

    wxFileName file = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    file.AppendDir("lang");
    wxArrayString as;
    int num_files = wxDir::GetAllFiles(file.GetPath(), &as, "*.txt", wxDIR_FILES);
    for(int i = 0; i < num_files; i++)
    {
        wxFileName lang_file = wxFileName(as[i]);
        m_choice1->Append(lang_file.GetName());
    }
    if(num_files == 0)
    {
        m_choice1->Enable(false);
    }
    m_bitmap1->SetSize(16, 16);
    Utility::SetIcon(m_bitmap1, IDB_PNG13);
    m_choice1->Show(false);

    Utility::SetIcon(m_customControl1, IDB_PNG3, "Home");
    Utility::SetIcon(m_customControl2, IDB_PNG1, "Backup");
    Utility::SetIcon(m_customControl3, IDB_PNG6, "Restore");
    Utility::SetIcon(m_customControl4, IDB_PNG8, "Clone");
    Utility::SetIcon(m_customControl5, IDB_PNG7, "Tools");

    init_language();

    m_customControl1->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickHome, this);
    m_customControl2->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickBackup, this);
    m_customControl3->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickRestore, this);
    m_customControl4->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickClone, this);
    m_customControl5->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickTools, this);

    di = DiskInfoFactory::CreateDiskInfo();

    Container_Util::SetNewframe(this);
    wxCommandEvent ev;
    OnClickHome(ev);
}

void FrameImpl::init_language()
{
    m_customControl1->SetText(ttt("Menu_Home"));
    m_customControl2->SetText(ttt("Menu_Backup"));
    m_customControl3->SetText(ttt("Menu_Restore"));
    m_customControl4->SetText(ttt("Menu_Clone"));
    m_customControl5->SetText(ttt("Menu_Tools"));
}

void FrameImpl::OnChoice( wxCommandEvent& event )
{
    int target = m_choice1->GetSelection();
    wxString selected_item_text = m_choice1->GetString(target);
    MultiLanguage::Instance()->SetLocale(selected_item_text);
    init_language();
    m_choice1->Show(false);
    OnClickHome(event);
}

void FrameImpl::OnLeftDown( wxMouseEvent& event )
{
    m_choice1->Show(true);
    Layout();
}

void FrameImpl::OnClose( wxCloseEvent& event )
{
    m_customControl5->Unbind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickTools, this);
    m_customControl4->Unbind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickClone, this);
    m_customControl3->Unbind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickRestore, this);
    m_customControl2->Unbind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickBackup, this);
    m_customControl1->Unbind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickHome, this);

    Container_Util::DisposeCurrentFrame();
    Destroy();

    delete MultiLanguage::Instance();
}

void FrameImpl::OnClickHome( wxCommandEvent& event )
{
    deselect_all(m_customControl1);
    Container_Util::SetNewframe(new Container_HomeImpl(this));

}
void FrameImpl::OnClickBackup( wxCommandEvent& event )
{
    deselect_all(m_customControl2);
    Container_Util::SetNewframe(new Container_BackupImpl(this));
}
void FrameImpl::OnClickRestore( wxCommandEvent& event )
{
    deselect_all(m_customControl3);
    Container_Util::SetNewframe(new Container_RestoreImpl(this));
}
void FrameImpl::OnClickClone( wxCommandEvent& event )
{
    deselect_all(m_customControl4);
    Container_Util::SetNewframe(new Container_CloneImpl(this));
}

void FrameImpl::OnClickTools( wxCommandEvent& event )
{
    deselect_all(m_customControl5);
    Container_Util::SetNewframe(new Container_ToolsImpl(this));
}

void FrameImpl::PauseButtons(bool pause)
{
    m_customControl1->SetPaused(pause);
    m_customControl2->SetPaused(pause);
    m_customControl3->SetPaused(pause);
    m_customControl4->SetPaused(pause);
    m_customControl5->SetPaused(pause);

    m_bitmap1->Enable(!pause);
    m_bitmap1->Refresh();
    m_choice1->Show(false);
}

void FrameImpl::deselect_all(const SimpleButton* except)
{
    SimpleButton* flat_buttons[] = 
    {
        m_customControl1,
        m_customControl2,
        m_customControl3,
        m_customControl4,
        m_customControl5,
        NULL,
    };

    SimpleButton** target = flat_buttons;
    while(*target != NULL)
    {
        (*target)->SetSelected((except != NULL && (*target) == except));
        target++;
    };
}

//
// Container_HomeImpl
//
Container_HomeImpl::Container_HomeImpl( FrameImpl* _frameimpl)
: Container_Home( nullptr ), ContainerCommon(_frameimpl)
{
    Utility::SetIcon(m_bpButton1, IDB_PNG15);
    m_bpButton1->SetBackgroundColour(wxColour(255,255,255));

    wxString version = Utility::GetVersion();
    wxString newline("\r\n");
    wxString newline2 = newline + newline;
    wxString implemented = wxString(" implemented ");
    wxString not_implemented = wxString(" not implemented yet ");
    m_textCtrl2->SetLabel(newline + ttt("Description") + newline2 + _T("https://github.com/UozaLab/UBR") + newline);
    m_staticText81->SetLabel(wxString::Format("UBR %s", version));
    m_grid1->DeleteRows(0, m_grid1->GetNumberRows());

    struct GridData
    {
        wxColor CellColor;
        wxColor ForeColor;
        wxString Comment;
        wxString Implemented;
        GridData(wxColor cell_color, wxString comment, wxColor fore_color, wxString implemented)
            : CellColor(cell_color), Comment(comment), ForeColor(fore_color), Implemented(implemented)
        {}
    };
    GridData gd[] = 
    {
        GridData(wxColour(0x59, 0x59, 0x59), wxEmptyString, wxColour(0xFF, 0xFF, 0xFF), wxEmptyString),
        GridData(wxColour(0xB8, 0xCC, 0xE4), wxString("Backup / Recovery"), wxColour(0x00, 0x00, 0x00), wxEmptyString),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Disk backup"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Partition backup"), wxColour(0x00, 0x00, 0x00), not_implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Disk restore"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Partition restore"), wxColour(0x00, 0x00, 0x00), not_implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Restore to a smaller disk"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Clone"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  VSS"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xB8, 0xCC, 0xE4), wxString("Tools"), wxColour(0x00, 0x00, 0x00), wxEmptyString),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Recovery boot media creation"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  PXE boot support"), wxColour(0x00, 0x00, 0x00), not_implemented),
    };
    int count = sizeof(gd) / sizeof(GridData);
    m_grid1->AppendRows(count + 1);
    for(int row = 0; row < count; row++)
    {
        m_grid1->SetCellBackgroundColour(row, 0, gd[row].CellColor);
        m_grid1->SetCellBackgroundColour(row, 1, gd[row].CellColor);
        m_grid1->SetCellBackgroundColour(row, 2, gd[row].CellColor);
        m_grid1->SetCellTextColour(row, 0, gd[row].ForeColor);
        if(gd[row].Implemented == implemented)
            m_grid1->SetCellTextColour(row, 1, wxColour(0x00, 0x00, 0xFF));
        else 
            m_grid1->SetCellTextColour(row, 1, gd[row].ForeColor);
        m_grid1->SetCellValue(row, 0, gd[row].Comment);
        m_grid1->SetCellValue(row, 1, gd[row].Implemented);
    }
    m_grid1->SetCellValue(0, 1, wxString::Format(" Version %s", version));

    m_grid1->SetCellBackgroundColour(count, 0, wxColour(0xDC, 0xE6, 0xF1));
    m_grid1->SetCellBackgroundColour(count, 1, wxColour(0xDC, 0xE6, 0xF1));
    m_grid1->SetCellBackgroundColour(count, 2, wxColour(0xDC, 0xE6, 0xF1));

    m_grid1->AutoSizeColumn(0);
    m_grid1->AutoSizeColumn(1);
    m_grid1->Show(false);
}

void Container_HomeImpl::OnClose( wxCloseEvent& event )
{
    Destroy();
}

void Container_HomeImpl::OnPPButtonClick( wxCommandEvent& event )
{
    if(m_grid1->IsShown())
    {
        Utility::SetIcon(m_bpButton1, IDB_PNG15);
        m_grid1->Show(false);
    }
    else
    {
        Utility::SetIcon(m_bpButton1, IDB_PNG14);
        m_grid1->Show(true);
    }
    wxSizer* sizer = m_grid1->GetContainingSizer();
    sizer->Layout();
}

void Container_HomeImpl::OnPPLeftDown( wxMouseEvent& event )
{
    wxCommandEvent evt;
    OnPPButtonClick(evt);
}
//
// Container_RestoreImpl
//
Container_RestoreImpl::Container_RestoreImpl( FrameImpl* _frameimpl )
: Container_Restore( nullptr ), ContainerCommon(_frameimpl)
{
    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickNextButton, this);

    m_staticText8->Show(false);
    m_staticText8->SetLabel(ttt("ClickNextToProceed"));
    m_staticText81->SetLabel(ttt("Title_Restore"));
    Utility::SetIcon(m_customControl61, IDB_PNG9, ttt("SelectFileToRestore"));
    m_customControl61->SetFrameMode(true);
    m_customControl61->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickButton, this);
}

void Container_RestoreImpl::OnClose( wxCloseEvent& event ) 
{
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickButton, this);
    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_RestoreImpl::OnClickNextButton, this);
    Destroy();
}

void Container_RestoreImpl::OnClickPrevButton( wxCommandEvent& event )
{
    m_staticText7->SetLabel("");
    m_staticText8->Show(false);
    wxWindow* win = m_customControl61;

    wxSizer* sizer = win->GetContainingSizer();
    const wxWindowList nodes = sizer->GetContainingWindow()->GetChildren();
    for(wxWindowList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        wxWindow* w = *it;
        if(w != win && w!=m_prevnext && w!=m_staticText7 && w!=m_staticText8)
            w->Destroy();
    }
    m_customControl61->Show(true);
    sizer->Layout();
    m_prevnext->SetEnabledNext(false);
    m_prevnext->SetEnabledPrev(false);
}

void Container_RestoreImpl::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_RestoreImpl_s01(frame_impl));
}

void Container_RestoreImpl::OnClickButton( wxCommandEvent& event )
{
    wxObject* caller = event.GetEventObject();
    wxWindow* win = dynamic_cast<wxWindow*>(caller);
    if(win == NULL) return;

    wxFileDialog openFileDialog(this, ttt("OpenImageFile"), "", "",
        "VHD files|*.vhdx;*.vhd;*.vhdx.lz4;*.vhd.lz4|RAW files|*.raw;*.raw.lz4|All|*", 
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filepath = openFileDialog.GetPath();
    frame_impl->Holder()->restore_data.filepath = filepath;
    shared_ptr<VirtualDisk> vdisk = VirtualDiskFactory::Create(filepath);
    if(vdisk == NULL || !vdisk->IsValid())
    {
        m_staticText7->SetLabel("Error: Invalid file.");
        return;
    }
    m_staticText7->SetLabel(filepath + " [" + vdisk->GetFileFormat() + "]");
    m_staticText81->SetLabel(ttt("Title_Restore_1"));

    shared_ptr<PhysicalDiskInfo> pd = ForensicAnalysis::CreateDiskInfo(vdisk);
    wxSizer* sizer = win->GetContainingSizer();

    sizer->Insert(2, 10, 2, 0, wxBOTTOM);
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->AddDisk(pd, false);
    sizer->Insert(3, panel, 0, wxEXPAND |wxRIGHT|wxLEFT)->SetBorder(12);

    m_customControl61->Show(false);
    m_prevnext->SetEnabledNext(true);
    m_prevnext->SetEnabledPrev(true);
    m_staticText8->Show(true);
    Container_Util::SetFrameLayout(panel);
}

//
// Container_RestoreImpl_s01
//
Container_RestoreImpl_s01::Container_RestoreImpl_s01( FrameImpl* _frameimpl )
: Container_Restore_s01( nullptr ), ContainerCommon(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frame_impl->GetDiskInfo();
    for(std::map<DWORD, shared_ptr<PhysicalDiskInfo>>::iterator itr=di->PhysicalDisks.begin(), itr_end = di->PhysicalDisks.end();
        itr != itr_end; ++itr)
    {
        panel->AddDisk(itr->second, false);
    }

    sizer->Insert(0, panel, 0, wxEXPAND |wxRIGHT|wxLEFT|wxTOP)->SetBorder(12);

    Bind(myEVT_SelectDisk, &Container_RestoreImpl_s01::OnSelectDisk, this);
    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s01::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s01::OnClickNextButton, this);

    m_staticText81->SetLabel(ttt("Title_Restore_s01"));
    m_prevnext->SetEnabledPrev(true);
    m_prevnext->SetTextNext(ttt("StartRestoreBtnText"));
    m_prevnext->SetEnhanceNext(true);

    Container_Util::SetFrameLayout(panel);
}

void Container_RestoreImpl_s01::OnClose( wxCloseEvent& event ) 
{
    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s01::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s01::OnClickNextButton, this);
    Unbind(myEVT_SelectDisk, &Container_RestoreImpl_s01::OnSelectDisk, this);
    Destroy();
}

void Container_RestoreImpl_s01::OnSelectDisk(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;

    frame_impl->Holder()->restore_data.disk_number = sd->disk_number;
    frame_impl->Holder()->restore_data.mode = sd->mode;
    frame_impl->Holder()->restore_data.partition_numbers = sd->partition_numbers;

    m_prevnext->SetEnabledNext(true);
}

void Container_RestoreImpl_s01::OnClickPrevButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_RestoreImpl(frame_impl));
}
void Container_RestoreImpl_s01::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_RestoreImpl_s02(frame_impl));
}

//
// Container_RestoreImpl_s02
//
Container_RestoreImpl_s02::Container_RestoreImpl_s02( FrameImpl* _frameimpl )
: Container_Common_ProgressImpl(_frameimpl), ContainerCommon(_frameimpl)
{
    progress->ShowPercent(true);
    m_staticText81->SetLabel(ttt("Title_Restore_s02"));
    m_prevnext->SetTextNext(ttt("CancelBtnText"));
    m_prevnext->SetEnabledNext(true);
    m_textCtrl1->SetBackgroundColour(COLOR_BACKGROUND_TEXTCTRL);

    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s02::OnClickNextButton, this);

    restore = new RestoreWorker(this, &frame_impl->Holder()->restore_data, frame_impl->GetDiskInfo());
    restore->Run();
}

void Container_RestoreImpl_s02::OnClose( wxCloseEvent& event ) 
{
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_RestoreImpl_s02::OnClickNextButton, this);
    delete restore;
    Destroy();
}

void Container_RestoreImpl_s02::OnClickNextButton( wxCommandEvent& event )
{
    
    if(ready_to_gonext || ready_to_goback)
    {
        frame_impl->OnClickHome(event);
    }
    else
    {
        restore->Terminate();
    }
}


//
// Container_CloneImpl
//
Container_CloneImpl::Container_CloneImpl( FrameImpl* _frameimpl )
: Container_Clone( nullptr ), ContainerCommon(_frameimpl)
{
    m_staticText81->SetLabel(ttt("Title_Clone"));

    Utility::SetIcon(m_customControl61, IDB_PNG16, ttt("FastCloneBtnTitle"));
    m_customControl61->SetDescription(ttt("FastCloneBtnDesc"));
    Utility::SetIcon(m_customControl71, IDB_PNG17, ttt("ExactCloneBtnTitle"));
    m_customControl71->SetDescription(ttt("ExactCloneBtnDesc"));
    m_customControl61->SetFrameMode(true);
    m_customControl71->SetFrameMode(true);
    m_customControl61->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl71->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_CloneImpl::OnClickFastButton, this);
    m_customControl71->Bind( myEVT_SimpleButtonClicked, &Container_CloneImpl::OnClickExactButton, this);
}

void Container_CloneImpl::OnClose( wxCloseEvent& event )
{
    m_customControl71->Unbind( myEVT_SimpleButtonClicked, &Container_CloneImpl::OnClickExactButton, this);
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_CloneImpl::OnClickFastButton, this);
    Destroy();
}

void Container_CloneImpl::OnClickFastButton( wxCommandEvent& event )
{
    frame_impl->Holder()->clone_data.exact_mode = false;
    Container_Util::SetNewframe(new Container_CloneImpl_s01(frame_impl));
}

void Container_CloneImpl::OnClickExactButton( wxCommandEvent& event )
{
    frame_impl->Holder()->clone_data.exact_mode = true;
    Container_Util::SetNewframe(new Container_CloneImpl_s01(frame_impl));
}

//
// Container_CloneImpl_s01
//
Container_CloneImpl_s01::Container_CloneImpl_s01( FrameImpl* _frameimpl )
: Container_Clone_s01( nullptr ), ContainerCommon(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frame_impl->GetDiskInfo();
    for(std::map<DWORD, shared_ptr<PhysicalDiskInfo>>::iterator itr=di->PhysicalDisks.begin(), itr_end = di->PhysicalDisks.end();
        itr != itr_end; ++itr)
    {
        panel->AddDisk(itr->second, false);
    }
    sizer->Insert(0, panel, 0, wxEXPAND |wxRIGHT|wxLEFT|wxTOP)->SetBorder(12);

    m_staticText81->SetLabel(ttt("Title_Clone_s01"));

    Bind(myEVT_SelectDisk, &Container_CloneImpl_s01::OnSelectDisk, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s01::OnClickNextButton, this);
    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s01::OnClickPrevButton, this);

    m_prevnext->SetEnabledPrev(true);
    m_prevnext->SetEnabledNext(false);
    m_staticText10->SetLabel(wxEmptyString);

    Container_Util::SetFrameLayout(panel);
}

void Container_CloneImpl_s01::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s01::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s01::OnClickNextButton, this);
    Unbind(myEVT_SelectDisk, &Container_CloneImpl_s01::OnSelectDisk, this);
    Destroy();
}

void Container_CloneImpl_s01::OnSelectDisk(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    {
        frame_impl->Holder()->clone_data.disk_number_src = sd->disk_number;
        frame_impl->Holder()->clone_data.mode = sd->mode;
        frame_impl->Holder()->clone_data.partition_numbers_src = sd->partition_numbers;
        frame_impl->Holder()->clone_data.vss = true;
    }
    m_prevnext->SetEnabledNext(true);
    m_staticText10->SetLabel(ttt("ClickNextToProceed"));

}

void Container_CloneImpl_s01::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_CloneImpl_s02(frame_impl));
}

void Container_CloneImpl_s01::OnClickPrevButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_CloneImpl(frame_impl));
}

//
// Container_CloneImpl_s02
//
Container_CloneImpl_s02::Container_CloneImpl_s02( FrameImpl* _frameimpl )
:Container_Clone_s02( nullptr ), ContainerCommon(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frame_impl->GetDiskInfo();
    for(std::map<DWORD, shared_ptr<PhysicalDiskInfo>>::iterator itr=di->PhysicalDisks.begin(), itr_end = di->PhysicalDisks.end();
        itr != itr_end; ++itr)
    {
        panel->AddDisk(itr->second, (itr->first == frame_impl->Holder()->clone_data.disk_number_src));
    }

    sizer->Insert(0, panel, 0, wxEXPAND |wxRIGHT|wxLEFT|wxTOP)->SetBorder(12);

    Bind(myEVT_SelectDisk, &Container_CloneImpl_s02::OnSelectDisk, this);
    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s02::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s02::OnClickNextButton, this);

    m_staticText81->SetLabel(ttt("Title_Clone_s02"));

    m_prevnext->SetEnabledPrev(true);
    m_prevnext->SetEnabledNext(false);
    m_prevnext->SetTextNext(ttt("StartCloneBtnText"));
    m_prevnext->SetEnhanceNext(true);
}

void Container_CloneImpl_s02::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s02::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s02::OnClickNextButton, this);
    Destroy();
}

void Container_CloneImpl_s02::OnSelectDisk(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    {
        frame_impl->Holder()->clone_data.disk_number_dst = sd->disk_number;
        frame_impl->Holder()->clone_data.partition_numbers_dst = sd->partition_numbers;
    }
    if(frame_impl->Holder()->clone_data.disk_number_src == frame_impl->Holder()->clone_data.disk_number_dst)
        return;

    m_prevnext->SetEnabledNext(true);

}

void Container_CloneImpl_s02::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_CloneImpl_s03(frame_impl));
}

void Container_CloneImpl_s02::OnClickPrevButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_CloneImpl_s01(frame_impl));
}

//
// Container_CloneImpl_s03
//
Container_CloneImpl_s03::Container_CloneImpl_s03( FrameImpl* _frameimpl )
: Container_Common_ProgressImpl(_frameimpl), ContainerCommon(_frameimpl)
{
    progress->ShowPercent(true);
    m_staticText81->SetLabel(ttt("Title_Clone_s03"));
    m_prevnext->SetTextNext(ttt("CancelBtnText"));
    m_prevnext->SetEnabledNext(true);
    m_textCtrl1->SetBackgroundColour(COLOR_BACKGROUND_TEXTCTRL);

    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s03::OnClickNextButton, this);
    
    clone = new CloneWorker(this, &frame_impl->Holder()->clone_data, frame_impl->GetDiskInfo());
    clone->Run();
}

void Container_CloneImpl_s03::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s03::OnClickNextButton, this);

    Destroy();
    delete clone;
}

void Container_CloneImpl_s03::OnClickNextButton( wxCommandEvent& event )
{
    if(ready_to_gonext || ready_to_goback)
    {
        frame_impl->OnClickHome(event);
    }
    else
    {
        clone->Terminate();
    }
}


//
// Container_ToolsImpl
//
Container_ToolsImpl::Container_ToolsImpl( FrameImpl* _frameimpl )
: Container_Tools( nullptr ), ContainerCommon(_frameimpl)
{
    m_staticText81->SetLabel(ttt("Title_Tools"));
    Utility::SetIcon(m_customControl61, IDB_PNG2, ttt("ToolsShell"));
    m_customControl61->SetDescription(ttt("ToolsShellDesc"));
    m_customControl61->SetFrameMode(true);
    m_customControl61->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_ToolsImpl::OnClickShellButton, this);

    Utility::SetIcon(m_customControl71, IDB_PNG18, ttt("ToolsWinPE"));
    m_customControl71->SetDescription(ttt("ToolsWinPEDesc"));
    m_customControl71->SetFrameMode(true);
    m_customControl71->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl71->Bind( myEVT_SimpleButtonClicked, &Container_ToolsImpl::OnClickCreateWinPEButton, this);

}

void Container_ToolsImpl::OnClose( wxCloseEvent& event )
{
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_ToolsImpl::OnClickShellButton, this);
    Destroy();
}

void Container_ToolsImpl::OnClickCreateWinPEButton( wxCommandEvent& event )
{
    wxFileName file = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    file.SetFullName(SystemEnvironment::RunAs32bit() ? "PEMaker32.exe" : "PEMaker64.exe");
    if(!file.FileExists()) return;

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    LPTSTR cmd = _tcsdup(file.GetFullPath().c_str());
    CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    free(cmd);
}

void Container_ToolsImpl::OnClickShellButton( wxCommandEvent& event )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    LPTSTR cmd = _tcsdup(_T("cmd.exe"));

    CreateProcess(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE|CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(cmd);
}

//
// Container_BackupImpl
//
Container_BackupImpl::Container_BackupImpl( FrameImpl* _frameimpl )
: Container_Backup( nullptr ), ContainerCommon(_frameimpl)
{
    frame_impl->Holder()->backup_data.Clear();
    m_staticText81->SetLabel(ttt("Title_Backup"));

    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frame_impl->GetDiskInfo();
    for(std::map<DWORD, shared_ptr<PhysicalDiskInfo>>::iterator itr=di->PhysicalDisks.begin(), itr_end = di->PhysicalDisks.end();
        itr != itr_end; ++itr)
    {
        panel->AddDisk(itr->second, false);
    }
    sizer->Insert(0, panel, 0, wxEXPAND |wxRIGHT|wxLEFT|wxTOP)->SetBorder(12);

    Bind(myEVT_SelectDisk, &Container_BackupImpl::OnSelectDisk, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_BackupImpl::OnClickNextButton, this);
    
    m_prevnext->SetEnabledPrev(false);
    m_prevnext->SetEnabledNext(false);
    m_staticText10->SetLabel(wxEmptyString);
    
    Container_Util::SetFrameLayout(panel);
}

void Container_BackupImpl::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_BackupImpl::OnClickNextButton, this);
    Unbind(myEVT_SelectDisk, &Container_BackupImpl::OnSelectDisk, this);
    Destroy();
}

void Container_BackupImpl::OnSelectDisk(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    {
        frame_impl->Holder()->backup_data.disk_number = sd->disk_number;
        frame_impl->Holder()->backup_data.mode = sd->mode;
        frame_impl->Holder()->backup_data.partition_numbers_src = sd->partition_numbers;
    }
    m_prevnext->SetEnabledNext(true);
    m_staticText10->SetLabel(ttt("ClickNextToProceed"));

}

void Container_BackupImpl::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_BackupImpl_s01(frame_impl));
}

//
// Container_BackupImpl_s01
//
Container_BackupImpl_s01::Container_BackupImpl_s01( FrameImpl* _frameimpl )
: Container_Backup_s01( nullptr ), ContainerCommon(_frameimpl)
{
    m_staticText81->SetLabel(ttt("Title_Backup_s01"));

    Utility::SetIcon(m_customControl61, IDB_PNG12, "VHDX");
    m_customControl61->SetDescription(ttt("BackupVHDXBtnDesc"));
    Utility::SetIcon(m_customControl71, IDB_PNG12, "VHD");
    m_customControl71->SetDescription(ttt("BackupVHDBtnDesc"));
    Utility::SetIcon(m_customControl81, IDB_PNG12, "RAW");
    m_customControl81->SetDescription(ttt("BackupRAWBtnDesc"));
    m_customControl61->SetFrameMode(true);
    m_customControl71->SetFrameMode(true);
    m_customControl81->SetFrameMode(true);
    m_customControl61->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl71->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl81->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickVHDXButton, this);
    m_customControl71->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickVHDButton, this);
    m_customControl81->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickRAWButton, this);

    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickPrevButton, this);
    m_prevnext->SetEnabledPrev(true);
    m_prevnext->SetEnabledNext(false);

    shared_ptr<DiskInfo> di = frame_impl->GetDiskInfo();
    if(di->PhysicalDisks[frame_impl->Holder()->backup_data.disk_number]->DiskSize > 2199023255552ULL/*2TB*/)
      m_customControl71->SetPaused(true);
}

void Container_BackupImpl_s01::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickPrevButton, this);
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickVHDXButton, this);
    m_customControl71->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickVHDButton, this);
    m_customControl81->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s01::OnClickRAWButton, this);
    Destroy();
}

void Container_BackupImpl_s01::deselect_all(const SimpleButton* except)
{
    SimpleButton* flat_buttons[] = 
    {
        m_customControl61,
        m_customControl71,
        m_customControl81,
        NULL,
    };

    SimpleButton** target = flat_buttons;
    while(*target != NULL)
    {
        (*target)->SetSelected((except != NULL && (*target) == except));
        target++;
    };
}

bool Container_BackupImpl_s01::show_file_dialog(FileType filetype)
{
    wxString wildCard = (filetype == FILE_TYPE_VHDX) ? "Compressed VHDX files|*.vhdx.lz4|VHDX files|*.vhdx|All|*" :
                        (filetype == FILE_TYPE_VHD) ? "Compressed VHD files|*.vhd.lz4|VHD files|*.vhd|All|*" : 
                        "Compressed RAW files|*.raw.lz4|RAW files|*.raw|All|*";
    wxFileDialog saveFileDialog(this, ttt("SaveImageFile"), "", "", wildCard, wxFD_SAVE);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return false;

    wxString filepath = saveFileDialog.GetPath();
    frame_impl->Holder()->backup_data.filepath = filepath;
    frame_impl->Holder()->backup_data.filetype = filetype;
    return true;
}

void Container_BackupImpl_s01::OnClickVHDXButton( wxCommandEvent& event )
{
    if(!show_file_dialog(FILE_TYPE_VHDX)) return;
    deselect_all(m_customControl61);
    Container_Util::SetNewframe(new Container_BackupImpl_s02(frame_impl));
}

void Container_BackupImpl_s01::OnClickVHDButton( wxCommandEvent& event )
{
    if(!show_file_dialog(FILE_TYPE_VHD)) return;
    deselect_all(m_customControl71);
    Container_Util::SetNewframe(new Container_BackupImpl_s02(frame_impl));
}

void Container_BackupImpl_s01::OnClickRAWButton( wxCommandEvent& event )
{
    if(!show_file_dialog(FILE_TYPE_RAW)) return;
    deselect_all(m_customControl81);
    Container_Util::SetNewframe(new Container_BackupImpl_s02(frame_impl));
}

void Container_BackupImpl_s01::OnClickPrevButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_BackupImpl(frame_impl));
}

//
// Container_BackupImpl_s02
//
Container_BackupImpl_s02::Container_BackupImpl_s02( FrameImpl* _frameimpl )
: Container_Backup_s02( nullptr ), ContainerCommon(_frameimpl)
{
    m_staticText81->SetLabel(ttt("Title_Backup_s02"));

    m_prevnext->GetButtonPrev()->Bind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickNextButton, this);

    m_customControl61->SetFrameMode(true);
    m_customControl71->SetFrameMode(true);
    m_customControl81->SetFrameMode(true);
    Utility::SetIcon(m_customControl61, IDB_PNG20, ttt("BackupVSSBtn"));
    Utility::SetIcon(m_customControl71, IDB_PNG21, ttt("BackupCompressBtn"));
    Utility::SetIcon(m_customControl81, IDB_PNG19, ttt("BackupFastBtn"));
    m_customControl61->SetDescription(ttt("BackupVSSBtnDesc"));
    m_customControl71->SetDescription(ttt("BackupCompressBtnDesc"));
    m_customControl81->SetDescription(ttt("BackupFastBtnDesc"));
    
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickVSSButton, this);
    m_customControl71->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickCompressButton, this);
    m_customControl81->Bind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickExactButton, this);

    m_prevnext->SetEnabledPrev(true);
    m_prevnext->SetTextNext(ttt("StartBackupBtnText"));
    m_prevnext->SetEnhanceNext(true);
    m_prevnext->SetEnabledNext(true);
    if(SystemEnvironment::RunOnPE())
      m_customControl61->SetPaused(true);
    else
      m_customControl61->SetSelected(frame_impl->Holder()->backup_data.vss);

    
    FileType file_type = frame_impl->Holder()->backup_data.filetype;
    wxString file_ext_str(file_type == FILE_TYPE_VHD ? ".vhd"
                         : file_type == FILE_TYPE_VHDX ? ".vhdx"
                         : ".raw");
    wxString file_ext_compressed_str = file_ext_str + ".lz4";
    wxString filepath = frame_impl->Holder()->backup_data.filepath;
    bool tail_valid = filepath.Lower().EndsWith(file_ext_str) || 
                      filepath.Lower().EndsWith(file_ext_compressed_str);
    bool dup_ext = filepath.Lower().EndsWith(file_ext_compressed_str + file_ext_compressed_str);
    if(dup_ext)
      frame_impl->Holder()->backup_data.filepath = filepath.Mid(0, filepath.Length() - file_ext_compressed_str.Length());
    if(!tail_valid)
      frame_impl->Holder()->backup_data.filepath += file_ext_compressed_str;

    bool tail_lz4 = frame_impl->Holder()->backup_data.filepath.Lower().EndsWith(".lz4");
    frame_impl->Holder()->backup_data.compress = tail_lz4;
    m_customControl71->SetSelected(frame_impl->Holder()->backup_data.compress);

    if(file_type == FILE_TYPE_RAW)
    {
        m_customControl81->SetPaused(true);
    }
    else
    {
        m_customControl81->SetSelected(!frame_impl->Holder()->backup_data.exact_mode);
    }
    set_filepath_text();
}

void Container_BackupImpl_s02::OnClose( wxCloseEvent& event )
{
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickVSSButton, this);
    m_customControl71->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickCompressButton, this);
    m_customControl81->Unbind( myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickExactButton, this);

    m_prevnext->GetButtonPrev()->Unbind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickPrevButton, this);
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s02::OnClickNextButton, this);
    Destroy();
}

void Container_BackupImpl_s02::OnClickPrevButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_BackupImpl_s01(frame_impl));
}
void Container_BackupImpl_s02::OnClickNextButton( wxCommandEvent& event )
{
    Container_Util::SetNewframe(new Container_BackupImpl_s03(frame_impl));
}
void Container_BackupImpl_s02::OnClickVSSButton( wxCommandEvent& event )
{
    frame_impl->Holder()->backup_data.vss = !frame_impl->Holder()->backup_data.vss;
    m_customControl61->SetSelected(frame_impl->Holder()->backup_data.vss);
}
void Container_BackupImpl_s02::OnClickCompressButton( wxCommandEvent& event )
{
    frame_impl->Holder()->backup_data.compress = !frame_impl->Holder()->backup_data.compress;
    m_customControl71->SetSelected(frame_impl->Holder()->backup_data.compress);
    set_filepath_text();
}
void Container_BackupImpl_s02::OnClickExactButton( wxCommandEvent& event )
{
    frame_impl->Holder()->backup_data.exact_mode = !frame_impl->Holder()->backup_data.exact_mode;
    m_customControl81->SetSelected(!frame_impl->Holder()->backup_data.exact_mode);
}

void Container_BackupImpl_s02::set_filepath_text()
{
    wxString filepath = frame_impl->Holder()->backup_data.filepath;
    bool tail_lz4 = filepath.Lower().EndsWith(".lz4");
    bool compress = frame_impl->Holder()->backup_data.compress;
    if(compress && !tail_lz4)
      filepath += _T(".lz4");
    if(!compress && tail_lz4)
      filepath = filepath.Mid(0, filepath.Length() - 4);
    bool file_exists = wxFile::Exists(filepath);

    FileType file_type = frame_impl->Holder()->backup_data.filetype;
    wxString file_type_str(file_type == FILE_TYPE_VHD ? "VHD"
                         : file_type == FILE_TYPE_VHDX ? "VHDX"
                         : "RAW");
    
    wxString label;
    if(file_exists)
      label = wxString::Format("%s%s [%s](%s)", ttt("BackupPath"), filepath, file_type_str, ttt("BackupPathWarn"));
    else
      label = wxString::Format("%s%s [%s]", ttt("BackupPath"), filepath, file_type_str);

    m_staticText10->SetLabel(label);
    frame_impl->Holder()->backup_data.filepath = filepath;
}

//
// Container_BackupImpl_s03
//
Container_BackupImpl_s03::Container_BackupImpl_s03( FrameImpl* _frameimpl )
: Container_Common_ProgressImpl(_frameimpl), ContainerCommon(_frameimpl)
{
    progress->ShowPercent(true);
    m_staticText81->SetLabel(ttt("Title_Backup_s03"));
    m_prevnext->SetTextNext(ttt("CancelBtnText"));
    m_prevnext->SetEnabledNext(true);
    m_textCtrl1->SetBackgroundColour(COLOR_BACKGROUND_TEXTCTRL);

    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s03::OnClickNextButton, this);

    backup = new BackupWorker(this, &frame_impl->Holder()->backup_data, frame_impl->GetDiskInfo());
    backup->Run();
}

void Container_BackupImpl_s03::OnClose( wxCloseEvent& event )
{
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_BackupImpl_s03::OnClickNextButton, this);
    Destroy();
    delete backup;
}

void Container_BackupImpl_s03::OnClickNextButton( wxCommandEvent& event )
{
    if(ready_to_gonext || ready_to_goback)
    {
        frame_impl->OnClickHome(event);
    }
    else
    {
        backup->Terminate();
    }
}


