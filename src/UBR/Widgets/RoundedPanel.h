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

#ifndef __ROUNDEDPANEL__H__
#define __ROUNDEDPANEL__H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/panel.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/dcclient.h>

class RoundedPanel : public wxPanel
{
private:
    void paintEvent(wxPaintEvent&);
    void sizeEvent(wxSizeEvent&);

protected:
    wxPanel* rounded_panel;
    wxColour forecolor;
    wxColour backcolor;

public:
    RoundedPanel( wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxSize( -1, -1 ),
                  long style = wxTAB_TRAVERSAL,
                  const wxString& name = wxEmptyString,
                  int orient = wxVERTICAL,
                  wxColour _backcolor = wxColour(255, 255, 255),
                  wxColour _forecolor = wxColour(255, 255, 255));
    ~RoundedPanel();
    wxPanel* GetInnerPanel();
    wxSizer* GetInnerSizer();

    DECLARE_EVENT_TABLE()
};

#endif
