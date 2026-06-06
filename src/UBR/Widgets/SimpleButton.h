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

#ifndef __SIMPLEBUTTON__H__
#define __SIMPLEBUTTON__H__

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/log.h>

#define PANEL_MARGIN 3
#define PANEL_MARGIN_BOTTOM 3
#define PANEL_MARGIN_DESC 8
#define PANEL_MARGIN_DESC_BOTTOM 8
#define ICON_WIDTH 32
#define ICON_HEIGHT 32
#define ICON_TOP_MARGIN 5
#define ICON_LEFT_MARGIN 10
#define ICON_RIGHT_MARGIN 15
#define DESCRIPTION_TOP_MARGIN 10
#define DESCRIPTION_FONT_SHRINK 2

class SimpleButton : public wxPanel
{

private:
    wxBitmap bitmap;
    wxBitmap bitmap_disabled;

    wxColour TextColour;
    wxColour TextColourHovered;
    wxColour NormalColour;
    wxColour HoveredColour;
    wxColour SelectedColour;
    wxColour PressedColour;
    wxColour PenColour;

    bool Hovered;
    bool Pressed;
    bool Disabled;
    bool Selected;
    bool Centered;
    bool Transparent;
    bool Bold;
    bool Paused;
    bool FrameMode;

    wxFont font;
    int Size;
    wxString Text;
    wxString Description;
    
    void paintEvent(wxPaintEvent&);
    void mouseMoved(wxMouseEvent&);
    void mouseDown(wxMouseEvent&);
    void mouseReleased(wxMouseEvent&);
    void mouseLeftWindow(wxMouseEvent&);
    void sizeEvent(wxSizeEvent&);

    wxSize calculate_size();
    void set_client_size();
    int count_linenumbers(wxString str);

public:
    SimpleButton(wxWindow* parent, const long id,
                 wxPoint position = wxDefaultPosition, 
                 wxSize size= wxDefaultSize, long style = 0);

    void SetText(wxString);
    void SetDescription(wxString);
    void SetNormalColour(wxColour);
    void SetTextColour(wxColour);
    void SetHoveredTextColour(wxColour);
    void SetHoveredColour(wxColour);
    void SetPressedColour(wxColour);
    void SetPenColour(wxColour);
    void SetDisabled(bool disabled);
    void SetSelected(bool selected);
    void SetAlignCentered(bool centered);
    void SetBold(bool bold);
    void SetPaused(bool pause);
    void SetFrameMode(bool enabled);
    void SetTransparent(bool transparent);
    void SetBitmap(const wxBitmap& bitmap);

    DECLARE_EVENT_TABLE()
};

wxDECLARE_EVENT(myEVT_SimpleButtonClicked, wxCommandEvent);

#endif
