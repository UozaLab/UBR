///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "Widgets/SimpleButton.h"
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include "Widgets/PrevNextPanel.h"
#include "Widgets/ProgressPanel.h"
#include <wx/textctrl.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/grid.h>

///////////////////////////////////////////////////////////////////////////

#define wxID_RESTORE_SELECTFILE 1000
#define wxID_BACKUP_DISK 1001
#define wxID_BACKUP_PART 1002

///////////////////////////////////////////////////////////////////////////////
/// Class BaseFrame
///////////////////////////////////////////////////////////////////////////////
class BaseFrame : public wxFrame
{
	private:

	protected:
		wxScrolledWindow* m_scrolledWindow1;
		SimpleButton* m_customControl1;
		SimpleButton* m_customControl2;
		SimpleButton* m_customControl3;
		SimpleButton* m_customControl4;
		SimpleButton* m_customControl5;
		wxPanel* m_panel33;
		wxStaticBitmap* m_bitmap1;
		wxChoice* m_choice1;
		wxPanel* m_panel_line;
		wxPanel* m_container_panel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnLeftDown( wxMouseEvent& event ) { event.Skip(); }
		virtual void OnChoice( wxCommandEvent& event ) { event.Skip(); }


	public:

		BaseFrame( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("UBR"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 800,500 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~BaseFrame();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Common
///////////////////////////////////////////////////////////////////////////////
class Container_Common : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		PrevNextPanel* m_prevnext;

	public:
		wxPanel* m_panel_common;
		wxScrolledWindow* m_panel_common_selectdisk;

		Container_Common( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Common();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Common_Progress
///////////////////////////////////////////////////////////////////////////////
class Container_Common_Progress : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		wxPanel* m_panel20;
		ProgressPanel* progress;
		wxTextCtrl* m_textCtrl1;
		PrevNextPanel* m_prevnext;

	public:
		wxScrolledWindow* m_panel_root;

		Container_Common_Progress( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Common_Progress();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Home
///////////////////////////////////////////////////////////////////////////////
class Container_Home : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		wxPanel* m_panel52;
		wxTextCtrl* m_textCtrl2;
		wxPanel* m_panel34;
		wxBitmapButton* m_bpButton1;
		wxStaticText* m_staticText11;
		wxGrid* m_grid1;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void OnPPButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnPPLeftDown( wxMouseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_home;
		wxScrolledWindow* m_scrolledwindow;

		Container_Home( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Home();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Clone
///////////////////////////////////////////////////////////////////////////////
class Container_Clone : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		SimpleButton* m_customControl61;
		SimpleButton* m_customControl71;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_clone;
		wxScrolledWindow* m_scrolledwindow;

		Container_Clone( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Clone();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Clone_s01
///////////////////////////////////////////////////////////////////////////////
class Container_Clone_s01 : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		wxStaticText* m_staticText10;
		PrevNextPanel* m_prevnext;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_clone;
		wxScrolledWindow* m_panel_common_selectdisk;

		Container_Clone_s01( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Clone_s01();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Clone_s02
///////////////////////////////////////////////////////////////////////////////
class Container_Clone_s02 : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		PrevNextPanel* m_prevnext;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_clone;
		wxScrolledWindow* m_panel_common_selectdisk;

		Container_Clone_s02( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Clone_s02();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Backup
///////////////////////////////////////////////////////////////////////////////
class Container_Backup : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxPanel* m_panel81;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		SimpleButton* m_customControl61;
		SimpleButton* m_customControl71;
		wxPanel* m_panel_backup_disk;
		wxButton* m_button1;
		wxPanel* m_panel_backup_part;
		wxButton* m_button2;
		wxButton* m_button3;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxScrolledWindow* m_panel_backup;

		Container_Backup( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Backup();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Restore
///////////////////////////////////////////////////////////////////////////////
class Container_Restore : public wxFrame
{
	private:

	protected:
		wxScrolledWindow* m_scrolledwindow;
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		SimpleButton* m_customControl61;
		wxStaticText* m_staticText7;
		wxStaticText* m_staticText8;
		PrevNextPanel* m_prevnext;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_restore;

		Container_Restore( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Restore();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Restore_s01
///////////////////////////////////////////////////////////////////////////////
class Container_Restore_s01 : public wxFrame
{
	private:

	protected:
		wxScrolledWindow* m_scrolledwindow;
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		PrevNextPanel* m_prevnext;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxPanel* m_panel_restore;

		Container_Restore_s01( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Restore_s01();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Restore_s02
///////////////////////////////////////////////////////////////////////////////
class Container_Restore_s02 : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		wxPanel* m_panel20;
		ProgressPanel* progress;
		wxTextCtrl* m_textCtrl1;
		PrevNextPanel* m_prevnext;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxScrolledWindow* m_panel_restore;

		Container_Restore_s02( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 350,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Restore_s02();

};

///////////////////////////////////////////////////////////////////////////////
/// Class Container_Tools
///////////////////////////////////////////////////////////////////////////////
class Container_Tools : public wxFrame
{
	private:

	protected:
		wxPanel* m_panel71;
		wxStaticText* m_staticText81;
		wxPanel* m_panel171;
		SimpleButton* m_customControl61;

		// Virtual event handlers, overide them in your derived class
		virtual void OnClose( wxCloseEvent& event ) { event.Skip(); }


	public:
		wxScrolledWindow* m_panel_tools;

		Container_Tools( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxEmptyString, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~Container_Tools();

};

