///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "BaseFrame.h"

///////////////////////////////////////////////////////////////////////////

BaseFrame::BaseFrame( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	this->SetBackgroundColour( wxColour( 240, 240, 240 ) );

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer1->AddGrowableCol( 2 );
	fgSizer1->AddGrowableRow( 0 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

	m_scrolledWindow1 = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxSize( 150,-1 ), wxVSCROLL );
	m_scrolledWindow1->SetScrollRate( 5, 5 );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	m_customControl1 = new SimpleButton( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer7->Add( m_customControl1, 0, wxBOTTOM|wxEXPAND|wxTOP, 3 );

	m_customControl2 = new SimpleButton( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer7->Add( m_customControl2, 0, wxBOTTOM|wxEXPAND|wxTOP, 3 );

	m_customControl3 = new SimpleButton( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer7->Add( m_customControl3, 0, wxBOTTOM|wxEXPAND|wxTOP, 3 );

	m_customControl4 = new SimpleButton( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer7->Add( m_customControl4, 0, wxBOTTOM|wxEXPAND|wxTOP, 3 );

	m_customControl5 = new SimpleButton( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer7->Add( m_customControl5, 0, wxBOTTOM|wxEXPAND|wxTOP, 3 );


	bSizer7->Add( 0, 0, 1, wxEXPAND, 5 );

	m_panel33 = new wxPanel( m_scrolledWindow1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxHORIZONTAL );

	m_bitmap1 = new wxStaticBitmap( m_panel33, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	m_bitmap1->SetToolTip( wxT("Select Language") );

	bSizer49->Add( m_bitmap1, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3 );

	wxArrayString m_choice1Choices;
	m_choice1 = new wxChoice( m_panel33, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0 );
	m_choice1->SetSelection( 0 );
	m_choice1->SetMaxSize( wxSize( 70,-1 ) );

	bSizer49->Add( m_choice1, 0, wxALL, 2 );


	m_panel33->SetSizer( bSizer49 );
	m_panel33->Layout();
	bSizer49->Fit( m_panel33 );
	bSizer7->Add( m_panel33, 0, 0, 0 );


	m_scrolledWindow1->SetSizer( bSizer7 );
	m_scrolledWindow1->Layout();
	fgSizer1->Add( m_scrolledWindow1, 1, wxALL|wxEXPAND, 3 );

	m_panel_line = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 1,-1 ), wxTAB_TRAVERSAL );
	m_panel_line->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW ) );
	m_panel_line->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	fgSizer1->Add( m_panel_line, 1, wxEXPAND, 0 );

	m_container_panel = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxSize( 350,-1 ), wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxVERTICAL );


	m_container_panel->SetSizer( bSizer71 );
	m_container_panel->Layout();
	fgSizer1->Add( m_container_panel, 1, wxEXPAND | wxALL, 0 );


	this->SetSizer( fgSizer1 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( BaseFrame::OnClose ) );
	m_bitmap1->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( BaseFrame::OnLeftDown ), NULL, this );
	m_choice1->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( BaseFrame::OnChoice ), NULL, this );
}

BaseFrame::~BaseFrame()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( BaseFrame::OnClose ) );
	m_bitmap1->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( BaseFrame::OnLeftDown ), NULL, this );
	m_choice1->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( BaseFrame::OnChoice ), NULL, this );

}

Container_Common::Container_Common( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_panel_common = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_common->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );

	m_panel_common_selectdisk = new wxScrolledWindow( m_panel_common, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_panel_common_selectdisk->SetScrollRate( 5, 5 );
	m_panel_common_selectdisk->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Select Disk"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_panel_common_selectdisk->SetSizer( bSizer20 );
	m_panel_common_selectdisk->Layout();
	bSizer20->Fit( m_panel_common_selectdisk );
	bSizer36->Add( m_panel_common_selectdisk, 1, wxEXPAND, 0 );

	m_prevnext = new PrevNextPanel( m_panel_common, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer36->Add( m_prevnext, 0, wxALL|wxEXPAND, 0 );


	m_panel_common->SetSizer( bSizer36 );
	m_panel_common->Layout();
	bSizer36->Fit( m_panel_common );
	bSizer4->Add( m_panel_common, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer4 );
	this->Layout();

	this->Centre( wxBOTH );
}

Container_Common::~Container_Common()
{
}

Container_Common_Progress::Container_Common_Progress( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_root = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_panel_root->SetScrollRate( 5, 5 );
	m_panel_root->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_panel_root, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("title"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_panel_root, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel171->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_panel20 = new wxPanel( m_panel171, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel20->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );

	progress = new ProgressPanel( m_panel20, wxID_ANY, wxDefaultPosition, wxSize( -1,15 ), 0 );
	bSizer32->Add( progress, 0, wxALL|wxEXPAND, 5 );

	m_textCtrl1 = new wxTextCtrl( m_panel20, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_BESTWRAP|wxTE_MULTILINE|wxTE_READONLY|wxBORDER_STATIC );
	bSizer32->Add( m_textCtrl1, 1, wxALL|wxEXPAND, 5 );


	m_panel20->SetSizer( bSizer32 );
	m_panel20->Layout();
	bSizer32->Fit( m_panel20 );
	bSizer191->Add( m_panel20, 1, wxEXPAND | wxALL, 5 );

	m_prevnext = new PrevNextPanel( m_panel171, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_prevnext, 0, wxALL|wxEXPAND, 5 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_panel_root->SetSizer( bSizer20 );
	m_panel_root->Layout();
	bSizer20->Fit( m_panel_root );
	bSizer8->Add( m_panel_root, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );
}

Container_Common_Progress::~Container_Common_Progress()
{
}

Container_Home::Container_Home( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_panel_home = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_home->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer5;
	bSizer5 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_home, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_panel52 = new wxPanel( m_panel171, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel52->SetBackgroundColour( wxColour( 255, 255, 255 ) );

	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxVERTICAL );

	m_textCtrl2 = new wxTextCtrl( m_panel52, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,100 ), wxTE_MULTILINE|wxTE_NO_VSCROLL|wxTE_READONLY|wxTE_RICH|wxBORDER_NONE );
	bSizer76->Add( m_textCtrl2, 0, wxALL|wxEXPAND, 5 );


	bSizer76->Add( 0, 10, 0, wxEXPAND, 5 );

	m_panel34 = new wxPanel( m_panel52, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer50;
	bSizer50 = new wxBoxSizer( wxHORIZONTAL );

	m_bpButton1 = new wxBitmapButton( m_panel34, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 18,18 ), wxBU_AUTODRAW|wxBORDER_NONE );
	bSizer50->Add( m_bpButton1, 0, wxALIGN_CENTER_VERTICAL, 0 );

	m_staticText11 = new wxStaticText( m_panel34, wxID_ANY, wxT("Project progress"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer50->Add( m_staticText11, 0, wxALIGN_CENTER_VERTICAL, 0 );


	m_panel34->SetSizer( bSizer50 );
	m_panel34->Layout();
	bSizer50->Fit( m_panel34 );
	bSizer76->Add( m_panel34, 0, wxEXPAND | wxALL, 0 );

	m_grid1 = new wxGrid( m_panel52, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

	// Grid
	m_grid1->CreateGrid( 5, 3 );
	m_grid1->EnableEditing( false );
	m_grid1->EnableGridLines( true );
	m_grid1->EnableDragGridSize( false );
	m_grid1->SetMargins( 3, 3 );

	// Columns
	m_grid1->AutoSizeColumns();
	m_grid1->EnableDragColMove( false );
	m_grid1->EnableDragColSize( false );
	m_grid1->SetColLabelSize( 1 );
	m_grid1->SetColLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );

	// Rows
	m_grid1->EnableDragRowSize( false );
	m_grid1->SetRowLabelSize( 1 );
	m_grid1->SetRowLabelAlignment( wxALIGN_CENTER, wxALIGN_CENTER );

	// Label Appearance

	// Cell Defaults
	m_grid1->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	m_grid1->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_SCROLLBAR ) );

	bSizer76->Add( m_grid1, 0, wxLEFT, 20 );


	bSizer76->Add( 0, 0, 1, wxEXPAND, 5 );


	m_panel52->SetSizer( bSizer76 );
	m_panel52->Layout();
	bSizer76->Fit( m_panel52 );
	bSizer191->Add( m_panel52, 1, wxEXPAND | wxALL, 5 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 7 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer5->Add( m_scrolledwindow, 1, wxEXPAND, 0 );


	m_panel_home->SetSizer( bSizer5 );
	m_panel_home->Layout();
	bSizer5->Fit( m_panel_home );
	bSizer4->Add( m_panel_home, 1, wxEXPAND | wxALL, 0 );


	this->SetSizer( bSizer4 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Home::OnClose ) );
	m_bpButton1->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Container_Home::OnPPButtonClick ), NULL, this );
	m_staticText11->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( Container_Home::OnPPLeftDown ), NULL, this );
}

Container_Home::~Container_Home()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Home::OnClose ) );
	m_bpButton1->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Container_Home::OnPPButtonClick ), NULL, this );
	m_staticText11->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( Container_Home::OnPPLeftDown ), NULL, this );

}

Container_Clone::Container_Clone( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_panel_clone = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_clone->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_clone, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Clone"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_customControl61 = new SimpleButton( m_panel171, wxID_BACKUP_DISK, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl61, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );

	m_customControl71 = new SimpleButton( m_panel171, wxID_BACKUP_PART, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl71, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer36->Add( m_scrolledwindow, 1, wxEXPAND, 0 );


	m_panel_clone->SetSizer( bSizer36 );
	m_panel_clone->Layout();
	bSizer36->Fit( m_panel_clone );
	bSizer4->Add( m_panel_clone, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer4 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone::OnClose ) );
}

Container_Clone::~Container_Clone()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone::OnClose ) );

}

Container_Clone_s01::Container_Clone_s01( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_panel_clone = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_clone->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );

	m_panel_common_selectdisk = new wxScrolledWindow( m_panel_clone, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_panel_common_selectdisk->SetScrollRate( 5, 5 );
	m_panel_common_selectdisk->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Select Disk"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );


	bSizer191->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText10 = new wxStaticText( m_panel171, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer191->Add( m_staticText10, 0, wxALL, 5 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_panel_common_selectdisk->SetSizer( bSizer20 );
	m_panel_common_selectdisk->Layout();
	bSizer20->Fit( m_panel_common_selectdisk );
	bSizer36->Add( m_panel_common_selectdisk, 1, wxEXPAND, 0 );

	m_prevnext = new PrevNextPanel( m_panel_clone, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer36->Add( m_prevnext, 0, wxALL|wxEXPAND, 0 );


	m_panel_clone->SetSizer( bSizer36 );
	m_panel_clone->Layout();
	bSizer36->Fit( m_panel_clone );
	bSizer4->Add( m_panel_clone, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer4 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone_s01::OnClose ) );
}

Container_Clone_s01::~Container_Clone_s01()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone_s01::OnClose ) );

}

Container_Clone_s02::Container_Clone_s02( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );

	m_panel_clone = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_clone->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );

	m_panel_common_selectdisk = new wxScrolledWindow( m_panel_clone, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_panel_common_selectdisk->SetScrollRate( 5, 5 );
	m_panel_common_selectdisk->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Select Disk"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_panel_common_selectdisk, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_panel_common_selectdisk->SetSizer( bSizer20 );
	m_panel_common_selectdisk->Layout();
	bSizer20->Fit( m_panel_common_selectdisk );
	bSizer36->Add( m_panel_common_selectdisk, 1, wxEXPAND, 0 );

	m_prevnext = new PrevNextPanel( m_panel_clone, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer36->Add( m_prevnext, 0, wxALL|wxEXPAND, 0 );


	m_panel_clone->SetSizer( bSizer36 );
	m_panel_clone->Layout();
	bSizer36->Fit( m_panel_clone );
	bSizer4->Add( m_panel_clone, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer4 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone_s02::OnClose ) );
}

Container_Clone_s02::~Container_Clone_s02()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Clone_s02::OnClose ) );

}

Container_Backup::Container_Backup( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_backup = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_panel_backup->SetScrollRate( 5, 5 );
	m_panel_backup->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_panel_backup, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );

	m_panel81 = new wxPanel( m_panel71, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel81->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxVERTICAL );


	bSizer121->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel81, wxID_ANY, wxT("Backup"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer121->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer121->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel81->SetSizer( bSizer121 );
	m_panel81->Layout();
	bSizer121->Fit( m_panel81 );
	bSizer111->Add( m_panel81, 0, wxEXPAND | wxALL, 0 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_panel_backup, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_customControl61 = new SimpleButton( m_panel171, wxID_BACKUP_DISK, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl61, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );

	m_customControl71 = new SimpleButton( m_panel171, wxID_BACKUP_PART, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl71, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_panel_backup->SetSizer( bSizer20 );
	m_panel_backup->Layout();
	bSizer20->Fit( m_panel_backup );
	bSizer8->Add( m_panel_backup, 1, wxEXPAND | wxALL, 0 );

	m_panel_backup_disk = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxVERTICAL );

	m_button1 = new wxButton( m_panel_backup_disk, wxID_ANY, wxT("MyButton"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer7->Add( m_button1, 0, wxALL, 5 );


	m_panel_backup_disk->SetSizer( bSizer7 );
	m_panel_backup_disk->Layout();
	bSizer7->Fit( m_panel_backup_disk );
	bSizer8->Add( m_panel_backup_disk, 1, wxEXPAND | wxALL, 5 );

	m_panel_backup_part = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxVERTICAL );

	m_button2 = new wxButton( m_panel_backup_part, wxID_ANY, wxT("MyButton"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer81->Add( m_button2, 0, wxALL, 5 );

	m_button3 = new wxButton( m_panel_backup_part, wxID_ANY, wxT("MyButton"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer81->Add( m_button3, 0, wxALL, 5 );


	m_panel_backup_part->SetSizer( bSizer81 );
	m_panel_backup_part->Layout();
	bSizer81->Fit( m_panel_backup_part );
	bSizer8->Add( m_panel_backup_part, 1, wxEXPAND | wxALL, 5 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Backup::OnClose ) );
}

Container_Backup::~Container_Backup()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Backup::OnClose ) );

}

Container_Restore::Container_Restore( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_restore = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_restore->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_restore, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Restore"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_customControl61 = new SimpleButton( m_panel171, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl61, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );

	m_staticText7 = new wxStaticText( m_panel171, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END|wxST_NO_AUTORESIZE );
	m_staticText7->Wrap( -1 );
	bSizer191->Add( m_staticText7, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );


	bSizer191->Add( 0, 0, 1, wxEXPAND, 5 );

	m_staticText8 = new wxStaticText( m_panel171, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	bSizer191->Add( m_staticText8, 0, wxALL, 12 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer35->Add( m_scrolledwindow, 1, wxEXPAND | wxALL, 0 );

	m_prevnext = new PrevNextPanel( m_panel_restore, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer35->Add( m_prevnext, 0, wxALL|wxEXPAND, 0 );


	m_panel_restore->SetSizer( bSizer35 );
	m_panel_restore->Layout();
	bSizer35->Fit( m_panel_restore );
	bSizer8->Add( m_panel_restore, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore::OnClose ) );
}

Container_Restore::~Container_Restore()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore::OnClose ) );

}

Container_Restore_s01::Container_Restore_s01( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_restore = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_restore->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_restore, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Restore - Select a destination disk"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer34->Add( m_scrolledwindow, 1, wxALL|wxEXPAND, 0 );

	m_prevnext = new PrevNextPanel( m_panel_restore, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	bSizer34->Add( m_prevnext, 0, wxALL|wxEXPAND, 0 );


	m_panel_restore->SetSizer( bSizer34 );
	m_panel_restore->Layout();
	bSizer34->Fit( m_panel_restore );
	bSizer8->Add( m_panel_restore, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore_s01::OnClose ) );
}

Container_Restore_s01::~Container_Restore_s01()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore_s01::OnClose ) );

}

Container_Restore_s02::Container_Restore_s02( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_restore = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_restore, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Restore - Progress"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_panel20 = new wxPanel( m_panel171, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );

	progress = new ProgressPanel( m_panel20, wxID_ANY, wxDefaultPosition, wxSize( -1,15 ), 0 );
	bSizer32->Add( progress, 0, wxALL|wxEXPAND, 5 );

	m_textCtrl1 = new wxTextCtrl( m_panel20, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_BESTWRAP|wxTE_MULTILINE|wxTE_READONLY|wxBORDER_STATIC );
	bSizer32->Add( m_textCtrl1, 1, wxALL|wxEXPAND, 5 );


	m_panel20->SetSizer( bSizer32 );
	m_panel20->Layout();
	bSizer32->Fit( m_panel20 );
	bSizer191->Add( m_panel20, 1, wxEXPAND | wxALL, 5 );

	m_prevnext = new PrevNextPanel( m_panel171, wxID_RESTORE_SELECTFILE, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_prevnext, 0, wxALL|wxEXPAND, 5 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer63->Add( m_scrolledwindow, 1, wxEXPAND | wxALL, 5 );


	m_panel_restore->SetSizer( bSizer63 );
	m_panel_restore->Layout();
	bSizer63->Fit( m_panel_restore );
	bSizer8->Add( m_panel_restore, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore_s02::OnClose ) );
}

Container_Restore_s02::~Container_Restore_s02()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Restore_s02::OnClose ) );

}

Container_Tools::Container_Tools( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxVERTICAL );

	m_panel_tool = new wxPanel( this, wxID_PANEL_ROOT, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxVERTICAL );

	m_scrolledwindow = new wxScrolledWindow( m_panel_tool, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scrolledwindow->SetScrollRate( 5, 5 );
	m_scrolledwindow->SetBackgroundColour( wxColour( 245, 245, 245 ) );

	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );

	m_panel71 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel71->SetBackgroundColour( wxColour( 225, 225, 225 ) );

	wxBoxSizer* bSizer111;
	bSizer111 = new wxBoxSizer( wxVERTICAL );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );

	m_staticText81 = new wxStaticText( m_panel71, wxID_ANY, wxT("Tools"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetFont( wxFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
	m_staticText81->SetForegroundColour( wxColour( 26, 26, 26 ) );

	bSizer111->Add( m_staticText81, 0, wxLEFT, 12 );


	bSizer111->Add( 0, 8, 0, wxEXPAND, 5 );


	m_panel71->SetSizer( bSizer111 );
	m_panel71->Layout();
	bSizer111->Fit( m_panel71 );
	bSizer20->Add( m_panel71, 0, wxEXPAND | wxALL, 0 );

	m_panel171 = new wxPanel( m_scrolledwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer191;
	bSizer191 = new wxBoxSizer( wxVERTICAL );

	m_customControl61 = new SimpleButton( m_panel171, wxID_BACKUP_DISK, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl61, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );

	m_customControl71 = new SimpleButton( m_panel171, wxID_BACKUP_DISK, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer191->Add( m_customControl71, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 12 );


	m_panel171->SetSizer( bSizer191 );
	m_panel171->Layout();
	bSizer191->Fit( m_panel171 );
	bSizer20->Add( m_panel171, 1, wxEXPAND | wxALL, 0 );


	m_scrolledwindow->SetSizer( bSizer20 );
	m_scrolledwindow->Layout();
	bSizer20->Fit( m_scrolledwindow );
	bSizer62->Add( m_scrolledwindow, 1, wxEXPAND | wxALL, 0 );


	m_panel_tool->SetSizer( bSizer62 );
	m_panel_tool->Layout();
	bSizer62->Fit( m_panel_tool );
	bSizer8->Add( m_panel_tool, 1, wxEXPAND, 0 );


	this->SetSizer( bSizer8 );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Tools::OnClose ) );
}

Container_Tools::~Container_Tools()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( Container_Tools::OnClose ) );

}
