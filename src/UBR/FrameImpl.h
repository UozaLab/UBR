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


class FrameImpl : public BaseFrame
{
    protected:
        wxFrame* CurrentFrame;
        DataHolder dataholder;
        shared_ptr<DiskInfo> di;
        void init_language();

	public:
		FrameImpl( wxWindow* parent );

        DataHolder* Holder() { return  &dataholder; }
        shared_ptr<DiskInfo> GetDiskInfo() { return di; }

        void deselect_all(const SimpleButton* except = NULL);
        void set_new_frame_and_panel(wxFrame* newframe, wxPanel* newpanel);
        void pause_buttons(bool pause);

        void OnClickHome( wxCommandEvent& event );
        void OnClickBackup( wxCommandEvent& event );
        void OnClickRestore( wxCommandEvent& event );
        void OnClickClone( wxCommandEvent& event );
        void OnClickTools( wxCommandEvent& event );

		void OnClose( wxCloseEvent& event );
        void OnChoice( wxCommandEvent& event );
        void OnLeftDown( wxMouseEvent& event );

};

class Container_Util
{
public:
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

class Container_Common_ProgressImpl : public Container_Common_Progress
{
private:

protected:
    FrameImpl* frameimpl;
    bool ready_to_goback;
public:
    Container_Common_ProgressImpl( wxWindow* parent, FrameImpl* _frameimpl );
    virtual ~Container_Common_ProgressImpl();

    virtual void OnClose( wxCloseEvent& event );
    virtual void OnClickNextButton( wxCommandEvent& event );
    void OnProgress(ProgressEvent& event);
    void OnMsg(MsgEvent& event);
    void OnError(ErrorEvent& event);
    void OnThreadEvent(wxThreadEvent& event);

};

class Container_RestoreImpl : public Container_Restore
{
protected:
    FrameImpl* frameimpl;

public:
		Container_RestoreImpl( wxWindow* parent, FrameImpl* _frameimpl );

        void OnClickButton( wxCommandEvent& event );
        void OnClickPrevButton( wxCommandEvent& event );
        void OnClickNextButton( wxCommandEvent& event );
        void OnClose( wxCloseEvent& event );
};

class Container_RestoreImpl_s01 : public Container_Restore_s01
{
protected:
    FrameImpl* frameimpl;

public:
		Container_RestoreImpl_s01( wxWindow* parent, FrameImpl* _frameimpl );

        void OnClickPrevButton( wxCommandEvent& event );
        void OnClickNextButton( wxCommandEvent& event );
        void OnSelectDisk(wxCommandEvent& event);
        void OnClose( wxCloseEvent& event );
};

class Container_RestoreImpl_s02 : public Container_Common_ProgressImpl
{
protected:
    RestoreWorker* restore;

public:
    Container_RestoreImpl_s02( wxWindow* parent, FrameImpl* _frameimpl );

    void OnClickNextButton( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );
};

class Container_BackupImpl : public Container_Backup
{
protected:
    FrameImpl* frameimpl;

public:
		Container_BackupImpl( wxWindow* parent, FrameImpl* _frameimpl );

        virtual void OnClose( wxCloseEvent& event );
};

class Container_HomeImpl : public Container_Home
{
	public:
		Container_HomeImpl( wxWindow* parent );
        virtual void OnPPButtonClick( wxCommandEvent& event );
        virtual void OnPPLeftDown( wxMouseEvent& event );
        virtual void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl : public Container_Clone
{
protected:
    FrameImpl* frameimpl;
	public:
		Container_CloneImpl( wxWindow* parent, FrameImpl* _frameimpl );
        void OnClickFastButton( wxCommandEvent& event );
        void OnClickExactButton( wxCommandEvent& event );
        void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl_s01 : public Container_Clone_s01
{
protected:
    FrameImpl* frameimpl;

	public:
		Container_CloneImpl_s01( wxWindow* parent, FrameImpl* _frameimpl );
        void OnClickNextButton( wxCommandEvent& event );
        void OnClickPrevButton( wxCommandEvent& event );
        void OnSelectDisk(wxCommandEvent& event);
        void OnClose( wxCloseEvent& event );

};

class Container_CloneImpl_s02 : public Container_Clone_s02
{
protected:
    FrameImpl* frameimpl;

public:
    Container_CloneImpl_s02( wxWindow* parent, FrameImpl* _frameimpl );
    void OnClickPrevButton( wxCommandEvent& event );
    void OnClickNextButton( wxCommandEvent& event );
    void OnSelectDisk(wxCommandEvent& event);
    void OnClose( wxCloseEvent& event );
};

class Container_CloneImpl_s03 : public Container_Common_ProgressImpl
{
protected:
    CloneWorker* clone;
public:
    Container_CloneImpl_s03( wxWindow* parent, FrameImpl* _frameimpl );
    void OnClickNextButton( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

};

class Container_ToolsImpl : public Container_Tools
{
protected:
    FrameImpl* frameimpl;
public:
    Container_ToolsImpl( wxWindow* parent, FrameImpl* _frameimpl );
    void OnClose( wxCloseEvent& event );
    void OnClickShellButton( wxCommandEvent& event );
};

#endif

