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

#ifndef __MyProject1BaseFrame__
#define __MyProject1BaseFrame__

#include "BaseFrame.h"
#include "smart_ptr.h"
#include "Widgets/SimpleButton.h"
#include "MiscWx.h"
#include "DataHolder.h"
#include "Worker/Restore.h"
#include "Worker/Clone.h"
#include "Worker/Events.h"
#include "FileSystem/PhysicalDiskInfo.h"
#include "FileSystem/DiskInfo.h"

class ProgressHandler
{
private:

protected:
    bool ready_to_goback;
    bool ready_to_gonext;
    bool error;
    wxEvtHandler* event_handler;
    void write_msg(wxTextCtrl* ctrl, const wxString& str);
    void write_msg(wxTextCtrl* ctrl, const wxString& str, wxColor color);
    void write_msg(wxTextCtrl* ctrl, const MsgEvent& msg);

public:
    ProgressHandler(wxEvtHandler* _event_handler);
    virtual ~ProgressHandler();

    virtual void OnProgress(ProgressEvent& event){ event.Skip(); }
    virtual void OnMsg(MsgEvent& event){ event.Skip(); }
    virtual void OnError(ErrorEvent& event){ event.Skip(); }
    virtual void OnThreadEvent(wxThreadEvent& event){ event.Skip(); }

};

class FrameImpl : public BaseFrame
{
    protected:
        DataHolder dataholder;
        shared_ptr<DiskInfo> di;
        void init_language();
        void deselect_all(const SimpleButton* except = NULL);
        void pause_buttons(bool pause);

  public:
		FrameImpl();
        DataHolder* Holder() { return  &dataholder; }
        shared_ptr<DiskInfo> GetDiskInfo() { return di; }

        void OnClickHome( wxCommandEvent& event );
        void OnClickBackup( wxCommandEvent& event );
        void OnClickRestore( wxCommandEvent& event );
        void OnClickClone( wxCommandEvent& event );
        void OnClickTools( wxCommandEvent& event );

		void OnClose( wxCloseEvent& event );
        void OnChoice( wxCommandEvent& event );
        void OnLeftDown( wxMouseEvent& event );

};

class ContainerCommon
{
  protected:
    FrameImpl* frame_impl;
  public:
    ContainerCommon(FrameImpl* _frame_impl)
         :frame_impl(_frame_impl) {}
};

class Container_RestoreImpl : public Container_Restore, ContainerCommon
{
public:
		Container_RestoreImpl( FrameImpl* _frameimpl );

        void OnClickButton( wxCommandEvent& event );
        void OnClickPrevButton( wxCommandEvent& event );
        void OnClickNextButton( wxCommandEvent& event );
        void OnClose( wxCloseEvent& event );
};

class Container_RestoreImpl_s01 : public Container_Restore_s01, ContainerCommon
{
public:
		Container_RestoreImpl_s01( FrameImpl* _frameimpl );

        void OnClickPrevButton( wxCommandEvent& event );
        void OnClickNextButton( wxCommandEvent& event );
        void OnSelectDisk(wxCommandEvent& event);
        void OnClose( wxCloseEvent& event );
};

class Container_RestoreImpl_s02 : public Container_Common_Progress, ProgressHandler, ContainerCommon
{
protected:
    RestoreWorker* restore;

public:
    Container_RestoreImpl_s02( FrameImpl* _frameimpl );

    void OnClickNextButton( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );
};

class Container_BackupImpl : public Container_Backup, ContainerCommon
{
public:
		Container_BackupImpl( FrameImpl* _frameimpl );

        virtual void OnClose( wxCloseEvent& event );
};

class Container_HomeImpl : public Container_Home, ContainerCommon
{
public:
    Container_HomeImpl( FrameImpl* _frameimpl);
    virtual void OnPPButtonClick( wxCommandEvent& event );
    virtual void OnPPLeftDown( wxMouseEvent& event );
    virtual void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl : public Container_Clone, ContainerCommon
{
public:
	Container_CloneImpl( FrameImpl* _frameimpl );
    void OnClickFastButton( wxCommandEvent& event );
    void OnClickExactButton( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl_s01 : public Container_Clone_s01, ContainerCommon
{
public:
    Container_CloneImpl_s01( FrameImpl* _frameimpl );
    void OnClickNextButton( wxCommandEvent& event );
    void OnClickPrevButton( wxCommandEvent& event );
    void OnSelectDisk(wxCommandEvent& event);
    void OnClose( wxCloseEvent& event );

};

class Container_CloneImpl_s02 : public Container_Clone_s02, ContainerCommon
{
public:
    Container_CloneImpl_s02( FrameImpl* _frameimpl );
    void OnClickPrevButton( wxCommandEvent& event );
    void OnClickNextButton( wxCommandEvent& event );
    void OnSelectDisk(wxCommandEvent& event);
    void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl_s03 : public Container_Common_Progress, ProgressHandler, ContainerCommon
{
protected:
    CloneWorker* clone;
public:
    Container_CloneImpl_s03( FrameImpl* _frameimpl );
    void OnClickNextButton( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

};

class Container_ToolsImpl : public Container_Tools, ContainerCommon
{
public:
    Container_ToolsImpl( FrameImpl* _frameimpl );
    void OnClose( wxCloseEvent& event );
    void OnClickShellButton( wxCommandEvent& event );
    void OnClickCreateWinPEButton( wxCommandEvent& event );
};

class Container_Util
{
  private:
    static wxFrame* BaseFrame;
    static wxFrame* CurrentFrame;
  public:
    static void DisposeCurrentFrame()
    {
        if(CurrentFrame != nullptr)
        {
            CurrentFrame->Destroy();
            delete CurrentFrame;
        }
        CurrentFrame = nullptr;
    }

    static void SetNewframe(wxFrame* newframe)
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
        
            const wxWindowList nodes = panel_root->GetChildren();
            for(wxWindowList::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
            {
                wxWindow* win = *it;
                win->Destroy();
            }
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
    
    static bool SetFrameLayout(wxWindow* child)
    {
        wxWindow* target = child;
        do
        {
            if(typeid(*target) ==  typeid(FrameImpl))
            {
                target->Layout();
                return true;
            }
            target = target->GetParent();
        }while(target != NULL);

        return false;
    }
};

#endif

