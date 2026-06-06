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

#include "RoundedPanel.h"
#include "Defs.h"

RoundedPanel::RoundedPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name, int orient, wxColour _backcolor, wxColour _forecolor)
 : wxPanel( parent, id, pos, size, style, name ),
   forecolor(_forecolor), backcolor(_backcolor)
{
    SetBackgroundColour( COLOR_BUTTON_FACE );
    SetForegroundColour( COLOR_BUTTON_FACE );

	rounded_panel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* innerSizer = new wxBoxSizer( orient );
    rounded_panel->SetSizer(innerSizer);
    rounded_panel->Layout();
    innerSizer->Fit( rounded_panel );

	wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
	bSizer1->Add( rounded_panel, 1, wxEXPAND | wxALL, 5 );
	this->SetSizer( bSizer1 );
	this->Layout();
}

RoundedPanel::~RoundedPanel()
{
}

wxPanel* RoundedPanel::GetInnerPanel()
{
    return rounded_panel;
}

wxSizer* RoundedPanel::GetInnerSizer()
{
    return rounded_panel->GetSizer();
}

void RoundedPanel::sizeEvent(wxSizeEvent & evt)
{
    Refresh();
    evt.Skip();
}

void RoundedPanel:: paintEvent (wxPaintEvent &evt)
{
    int width = 0;
    int height = 0;
    GetClientSize( &width, &height );

    wxPaintDC dc(this);

    dc.SetBrush(wxBrush(backcolor));
    dc.SetPen(wxPen(backcolor));
    dc.DrawRectangle(0, 0, width, height);

    dc.SetBrush(wxBrush(forecolor));
    dc.SetPen(wxPen(forecolor));
    dc.DrawRoundedRectangle(0, 0, width, height, 5);

}


BEGIN_EVENT_TABLE(RoundedPanel, wxPanel)
    EVT_SIZE(RoundedPanel::sizeEvent)
    EVT_PAINT(RoundedPanel::paintEvent)
END_EVENT_TABLE()

