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

#include "FrameImpl.h"
#include "resource.h"
#include "Widgets/Defs.h"
#include "Widgets/DiskInfoPanel.h"
#include "Widgets/PrevNextPanel.h"
#include "FileSystem/ForensicAnalysis.h"
#include "FileSystem/VdiskFactory.h"
#include "Worker/Restore.h"
#include <windows.h>
#include <wx/mstream.h>
#include <wx/msw/private.h>
#include <wx/dir.h>

//
// FrameImpl
//
FrameImpl::FrameImpl( wxWindow* parent )
: BaseFrame( parent ), CurrentFrame(NULL)
{
    HICON hicon = (HICON) LoadIcon(wxGetInstance(), MAKEINTRESOURCE(IDI_ICON1));
    wxIcon icon;
    icon.CreateFromHICON(hicon);
    SetIcon(icon);

    wxFileName file = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    file.AppendDir("lang");
    wxArrayString as;
    int num_files = wxDir::GetAllFiles(file.GetPath(), &as);
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

    m_customControl2->SetPaused(true);

    init_language();

    m_customControl1->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickHome, this);
    m_customControl2->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickBackup, this);
    m_customControl3->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickRestore, this);
    m_customControl4->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickClone, this);
    m_customControl5->Bind( myEVT_SimpleButtonClicked, &FrameImpl::OnClickTools, this);

    OnClickHome(wxCommandEvent());

    di = DiskInfoFactory::CreateDiskInfo();

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

    if(CurrentFrame != NULL)
    {
        CurrentFrame->Close();
        delete CurrentFrame;
    }

    delete MultiLanguage::Instance();
    Destroy();
}

void FrameImpl::OnClickHome( wxCommandEvent& event )
{
    deselect_all(m_customControl1);
    Container_Home* TargetFrame = new Container_HomeImpl(NULL);
    set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_home);

}
void FrameImpl::OnClickBackup( wxCommandEvent& event )
{
    deselect_all(m_customControl2);
#if 0
    Container_Backup* TargetFrame = new Container_BackupImpl(NULL, this);
    set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_backup);
#else
    set_new_frame_and_panel(NULL, NULL);
#endif

}
void FrameImpl::OnClickRestore( wxCommandEvent& event )
{
    deselect_all(m_customControl3);
    Container_RestoreImpl* TargetFrame = new Container_RestoreImpl(NULL, this);
    set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_restore);
}
void FrameImpl::OnClickClone( wxCommandEvent& event )
{
    deselect_all(m_customControl4);
    Container_CloneImpl* TargetFrame = new Container_CloneImpl(NULL, this);
    set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

void FrameImpl::OnClickTools( wxCommandEvent& event )
{
    deselect_all(m_customControl5);
    Container_ToolsImpl* TargetFrame = new Container_ToolsImpl(NULL, this);
    set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_tools);
}

void FrameImpl::pause_buttons(bool pause)
{
    m_customControl1->SetPaused(pause);
    //m_customControl2->SetPaused(pause);
    m_customControl3->SetPaused(pause);
    m_customControl4->SetPaused(pause);
    m_customControl5->SetPaused(pause);

    m_bitmap1->Enable(!pause);
    m_choice1->Show(false);
}

void FrameImpl::set_new_frame_and_panel(wxFrame* newframe, wxPanel* newpanel)
{
    // hide windowds on the m_container_panel
    const wxWindowList nodes = m_container_panel->GetChildren();
    for(wxWindowList::const_iterator& it = nodes.begin(); it != nodes.end(); ++it)
    {
        wxWindow* win = *it;
        win->Show(false);
    }

    // set new frame
    if(newframe != NULL)
    {
        if(CurrentFrame != NULL)
        {
            CurrentFrame->Close();
            delete CurrentFrame;

            const wxWindowList nodes = m_container_panel->GetChildren();
            for(wxWindowList::const_iterator& it = nodes.begin(); it != nodes.end(); ++it)
            {
                wxWindow* win = *it;
                win->Destroy();
            }
        }
        CurrentFrame = newframe;
    }


    // set new panel
    if(newpanel == NULL) return;
    newpanel->Reparent(m_container_panel);
    wxSizer* sizer = newpanel->GetContainingSizer();
    sizer->Detach(newpanel);
    m_container_panel->GetSizer()->Add(newpanel, 1, wxEXPAND | wxALL, 0);
    m_container_panel->Layout();
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
// Container_Common_ProgressImpl
//
Container_Common_ProgressImpl::Container_Common_ProgressImpl( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Common_Progress(parent), frameimpl(_frameimpl), ready_to_goback(false)
{
    this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Common_ProgressImpl::OnClose ) );

    Bind(myEVT_PROGRESS, &Container_Common_ProgressImpl::OnProgress, this);
    Bind(myEVT_MSG, &Container_Common_ProgressImpl::OnMsg, this);
    Bind(myEVT_ERROR, &Container_Common_ProgressImpl::OnError, this);
    Bind(wxEVT_THREAD, &Container_Common_ProgressImpl::OnThreadEvent, this);
    m_prevnext->GetButtonNext()->Bind(myEVT_SimpleButtonClicked, &Container_Common_ProgressImpl::OnClickNextButton, this);
}
Container_Common_ProgressImpl::~Container_Common_ProgressImpl()
{
    this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Common_ProgressImpl::OnClose ) );
}

void Container_Common_ProgressImpl::OnClose( wxCloseEvent& event ) 
{
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_Common_ProgressImpl::OnClickNextButton, this);
    Unbind(wxEVT_THREAD, &Container_Common_ProgressImpl::OnThreadEvent, this);
    Unbind(myEVT_ERROR, &Container_Common_ProgressImpl::OnError, this);
    Unbind(myEVT_MSG, &Container_Common_ProgressImpl::OnMsg, this);
    Unbind(myEVT_PROGRESS, &Container_Common_ProgressImpl::OnProgress, this);
    //Destroy();
}


void Container_Common_ProgressImpl::OnClickNextButton( wxCommandEvent& event )
{
}

void Container_Common_ProgressImpl::OnProgress(ProgressEvent& event)
{
    progress->SetProgress(event.GetData());
}

void Container_Common_ProgressImpl::OnMsg(MsgEvent& event)
{
    m_textCtrl1->AppendText(wxDateTime::Now().Format(wxString("%H:%M:%S ")));
    m_textCtrl1->AppendText(event.GetData());
    m_textCtrl1->AppendText("\r\n");
}

void Container_Common_ProgressImpl::OnError(ErrorEvent& event)
{
    progress->SetError(true);
    m_textCtrl1->AppendText(wxDateTime::Now().Format(wxString("%H:%M:%S ")));
    m_textCtrl1->AppendText(wxString::Format("### ERROR ### %s\r\n", event.GetData()));
}

void Container_Common_ProgressImpl::OnThreadEvent(wxThreadEvent& event)
{
    ThreadState thread_state = event.GetPayload<ThreadState>();

    if(thread_state == THREAD_STARTED)
    {
        frameimpl->pause_buttons(true);
    }
    else if(thread_state == THREAD_FINISHED)
    {
        frameimpl->pause_buttons(false);
        m_prevnext->SetTextNext(ttt("FinishBtnText"));
        ready_to_goback = true;
    }
    else if(thread_state == THREAD_TERMINATING)
    {
        m_prevnext->SetTextNext(ttt("CancelingBtnText"));
    }
    else if(thread_state == THREAD_TERMINATED)
    {
        frameimpl->pause_buttons(false);
        m_prevnext->SetTextNext(ttt("CanceledBtnText"));
        ready_to_goback = true;
    }
}

//
// Container_HomeImpl
//
Container_HomeImpl::Container_HomeImpl( wxWindow* parent )
:
Container_Home( parent )
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
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Disk backup"), wxColour(0x00, 0x00, 0x00), not_implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Partition backup"), wxColour(0x00, 0x00, 0x00), not_implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Disk restore"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Partition restore"), wxColour(0x00, 0x00, 0x00), not_implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Restore to a smaller disk"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Clone"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  VSS"), wxColour(0x00, 0x00, 0x00), implemented),
        GridData(wxColour(0xB8, 0xCC, 0xE4), wxString("Tools"), wxColour(0x00, 0x00, 0x00), wxEmptyString),
        GridData(wxColour(0xDC, 0xE6, 0xF1), wxString("  Recovery boot media creation"), wxColour(0x00, 0x00, 0x00), not_implemented),
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
    OnPPButtonClick(wxCommandEvent());
}
//
// Container_RestoreImpl
//
Container_RestoreImpl::Container_RestoreImpl( wxWindow* parent, FrameImpl* _frameimpl )
: Container_Restore( parent ), frameimpl(_frameimpl)
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
    for(wxWindowList::const_iterator& it = nodes.begin(); it != nodes.end(); ++it)
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
    Container_RestoreImpl_s01* TargetFrame = new Container_RestoreImpl_s01(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_restore);
}

void Container_RestoreImpl::OnClickButton( wxCommandEvent& event )
{
    wxObject* caller = event.GetEventObject();
    wxWindow* win = dynamic_cast<wxWindow*>(caller);
    if(win == NULL) return;

    wxFileDialog openFileDialog(this, ttt("OpenImageFile"), "", "",
        "VHD files (*.vhdx;*.vhd)|*.vhdx;*.vhd|RAW files (*.raw)|*.raw|All (*)|*", 
        wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    wxString filepath = openFileDialog.GetPath();
    frameimpl->Holder()->restore_data.filepath = filepath;
    shared_ptr<VirtualDisk> vdisk = VirtualDiskFactory::Create(filepath);
    if(vdisk == NULL)
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
Container_RestoreImpl_s01::Container_RestoreImpl_s01( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Restore_s01( parent ), frameimpl(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frameimpl->GetDiskInfo();
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

    frameimpl->Holder()->restore_data.disk_number = sd->disk_number;
    frameimpl->Holder()->restore_data.mode = sd->mode;
    frameimpl->Holder()->restore_data.partition_numbers = sd->partition_numbers;

    m_prevnext->SetEnabledNext(true);
}

void Container_RestoreImpl_s01::OnClickPrevButton( wxCommandEvent& event )
{
    Container_RestoreImpl* TargetFrame = new Container_RestoreImpl(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_restore);
}
void Container_RestoreImpl_s01::OnClickNextButton( wxCommandEvent& event )
{
    Container_RestoreImpl_s02* TargetFrame = new Container_RestoreImpl_s02(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_root);
}

//
// Container_RestoreImpl_s02
//
Container_RestoreImpl_s02::Container_RestoreImpl_s02( wxWindow* parent, FrameImpl* _frameimpl )
: Container_Common_ProgressImpl( parent, _frameimpl )
{
    progress->ShowPercent(true);
    m_staticText81->SetLabel(ttt("Title_Restore_s02"));
    m_prevnext->SetTextNext(ttt("CancelBtnText"));
    m_prevnext->SetEnabledNext(true);
    m_textCtrl1->SetBackgroundColour(COLOR_BACKGROUND_TEXTCTRL);

    restore = new RestoreWorker(this, &frameimpl->Holder()->restore_data, frameimpl->GetDiskInfo());
    restore->Run();
}

void Container_RestoreImpl_s02::OnClose( wxCloseEvent& event ) 
{
    Container_Common_ProgressImpl::OnClose(event);
    delete restore;
    Destroy();
}

void Container_RestoreImpl_s02::OnClickNextButton( wxCommandEvent& event )
{
    if(ready_to_goback)
        frameimpl->OnClickHome(event);
    else
        restore->Terminate();
}


//
// Container_BackupImpl
//
Container_BackupImpl::Container_BackupImpl( wxWindow* parent, FrameImpl* _frameimpl )
: Container_Backup( parent ), frameimpl(_frameimpl)
{
    Utility::SetIcon(m_customControl61, IDB_PNG12, "Disk Backup");
    m_customControl61->SetDescription("test\r\ntest2\r\ntest3");
    Utility::SetIcon(m_customControl71, IDB_PNG1, "Partition Backup");
    m_customControl71->SetDescription("test\r\n");
}

void Container_BackupImpl::OnClose( wxCloseEvent& event ) 
{
    Destroy();
}

//
// Container_CloneImpl
//
Container_CloneImpl::Container_CloneImpl( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Clone( parent ), frameimpl(_frameimpl)
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
    frameimpl->Holder()->clone_data.exact_mode = false;
    Container_CloneImpl_s01* TargetFrame = new Container_CloneImpl_s01(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

void Container_CloneImpl::OnClickExactButton( wxCommandEvent& event )
{
    frameimpl->Holder()->clone_data.exact_mode = true;
    Container_CloneImpl_s01* TargetFrame = new Container_CloneImpl_s01(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

//
// Container_CloneImpl_s01
//
Container_CloneImpl_s01::Container_CloneImpl_s01( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Clone_s01( parent ), frameimpl(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frameimpl->GetDiskInfo();
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
    m_prevnext->GetButtonNext()->Unbind(myEVT_SimpleButtonClicked, &Container_CloneImpl_s01::OnClickNextButton, this);
    Unbind(myEVT_SelectDisk, &Container_CloneImpl_s01::OnSelectDisk, this);
    Destroy();
}

void Container_CloneImpl_s01::OnSelectDisk(wxCommandEvent& event)
{
    SelectionData* sd = dynamic_cast<SelectionData*>(event.GetEventObject());
    if(sd == NULL) return;
    {
        frameimpl->Holder()->clone_data.disk_number_src = sd->disk_number;
        frameimpl->Holder()->clone_data.mode = sd->mode;
        frameimpl->Holder()->clone_data.partition_numbers_src = sd->partition_numbers;
        frameimpl->Holder()->clone_data.vss = true;
    }
    m_prevnext->SetEnabledNext(true);
    m_staticText10->SetLabel(ttt("ClickNextToProceed"));

}

void Container_CloneImpl_s01::OnClickNextButton( wxCommandEvent& event )
{
    Container_CloneImpl_s02* TargetFrame = new Container_CloneImpl_s02(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

void Container_CloneImpl_s01::OnClickPrevButton( wxCommandEvent& event )
{
    Container_CloneImpl* TargetFrame = new Container_CloneImpl(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

//
// Container_CloneImpl_s02
//
Container_CloneImpl_s02::Container_CloneImpl_s02( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Clone_s02( parent ), frameimpl(_frameimpl)
{
    wxSizer* sizer = m_panel171->GetSizer();
    DiskInfoPanel* panel = new DiskInfoPanel(sizer->GetContainingWindow(), this);
    panel->SetSelectionMode(DISK_PANEL_SELECTION_DISK);

    shared_ptr<DiskInfo> di = frameimpl->GetDiskInfo();
    for(std::map<DWORD, shared_ptr<PhysicalDiskInfo>>::iterator itr=di->PhysicalDisks.begin(), itr_end = di->PhysicalDisks.end();
        itr != itr_end; ++itr)
    {
        panel->AddDisk(itr->second, (itr->first == frameimpl->Holder()->clone_data.disk_number_src));
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
        frameimpl->Holder()->clone_data.disk_number_dst = sd->disk_number;
        frameimpl->Holder()->clone_data.partition_numbers_dst = sd->partition_numbers;
    }
    if(frameimpl->Holder()->clone_data.disk_number_src == frameimpl->Holder()->clone_data.disk_number_dst)
        return;

    m_prevnext->SetEnabledNext(true);

}

void Container_CloneImpl_s02::OnClickNextButton( wxCommandEvent& event )
{
    Container_CloneImpl_s03* TargetFrame = new Container_CloneImpl_s03(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_root);
}

void Container_CloneImpl_s02::OnClickPrevButton( wxCommandEvent& event )
{
    Container_CloneImpl_s01* TargetFrame = new Container_CloneImpl_s01(NULL, frameimpl);
    frameimpl->set_new_frame_and_panel(TargetFrame, TargetFrame->m_panel_clone);
}

//
// Container_CloneImpl_s03
//
Container_CloneImpl_s03::Container_CloneImpl_s03( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Common_ProgressImpl( parent,_frameimpl )
{
    progress->ShowPercent(true);
    m_staticText81->SetLabel(ttt("Title_Clone_s03"));
    m_prevnext->SetTextNext(ttt("CancelBtnText"));
    m_prevnext->SetEnabledNext(true);
    m_textCtrl1->SetBackgroundColour(COLOR_BACKGROUND_TEXTCTRL);

    clone = new CloneWorker(this, &frameimpl->Holder()->clone_data, frameimpl->GetDiskInfo());
    clone->Run();
}

void Container_CloneImpl_s03::OnClose( wxCloseEvent& event )
{
    Container_Common_ProgressImpl::OnClose(event);
    Destroy();
    delete clone;
}

void Container_CloneImpl_s03::OnClickNextButton( wxCommandEvent& event )
{
    if(ready_to_goback)
        frameimpl->OnClickHome(event);
    else
        clone->Terminate();
}


//
// Container_ToolsImpl
//
Container_ToolsImpl::Container_ToolsImpl( wxWindow* parent, FrameImpl* _frameimpl )
:
Container_Tools( parent ), frameimpl(_frameimpl)
{
    m_staticText81->SetLabel(ttt("Title_Tools"));
    Utility::SetIcon(m_customControl61, IDB_PNG2, ttt("ToolsShell"));
    m_customControl61->SetDescription(ttt("ToolsShellDesc"));
    m_customControl61->SetFrameMode(true);
    m_customControl61->SetNormalColour(COLOR_BUTTON_FACE_FRAME);
    m_customControl61->Bind( myEVT_SimpleButtonClicked, &Container_ToolsImpl::OnClickShellButton, this);
}

void Container_ToolsImpl::OnClose( wxCloseEvent& event )
{
    m_customControl61->Unbind( myEVT_SimpleButtonClicked, &Container_ToolsImpl::OnClickShellButton, this);
    Destroy();
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
}
