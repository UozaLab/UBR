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

#include "SimpleButton.h"
#include "Defs.h"

SimpleButton::SimpleButton(wxWindow * parent, const long id, wxPoint position, wxSize size, long style) :
    wxPanel(parent, id, position, size),
    TextColour(COLOR_BUTTON_TEXT),
    TextColourHovered(COLOR_BUTTON_TEXT),
    NormalColour(COLOR_BUTTON_FACE),
    SelectedColour(COLOR_BUTTON_SELECTED),
    HoveredColour(COLOR_BUTTON_HOVER),
    PressedColour(COLOR_BUTTON_PRESSED),
    PenColour(COLOR_BUTTON_TEXT),
    Hovered(false),
    Pressed(false),
    Disabled(false),
    Selected(false),
    Centered(false),
    Transparent(true),
    Bold(false),
    Paused(false),
    FrameMode(false),
    font(*wxNORMAL_FONT),
    Size(12),
    Text(wxEmptyString),
    Description(wxEmptyString)
{
    set_client_size();
}

void SimpleButton::set_client_size()
{
    bool expanded = false;
    wxWindow* win = this->GetParent();
    if(win != NULL)
    {
        wxSizer* sizer = win->GetSizer();
        if(sizer != NULL)
        {
            wxSizerItem* item = sizer->GetItem(this);
            if(item != NULL)
            {
                int flag = item->GetFlag();
                if(flag & wxEXPAND)
                    expanded = true;
            }

        }
    }
    wxSize size = calculate_size();
    SetMinClientSize(size);
    if(!expanded) SetClientSize(size);
}

int SimpleButton::count_linenumbers(wxString str)
{
    if(str.empty()) return 0;
    int count = 0;
    size_t index = 0;
    do
    {
        count++;
        index = str.find_first_of("\n", index);
        if(index != wxString::npos)
            index++;
    } while(index != wxString::npos);
    return count;
}

wxSize SimpleButton::calculate_size()
{
    wxMemoryDC dc;
    wxFont f(font);

    f.SetPointSize(Size);
    dc.SetFont(f);
    long width = dc.GetTextExtent(Text).GetWidth()+10 + ICON_WIDTH;
    long height = (std::max)(ICON_HEIGHT + PANEL_MARGIN + PANEL_MARGIN_BOTTOM, dc.GetCharHeight() + PANEL_MARGIN + PANEL_MARGIN_BOTTOM);

    if(!Description.empty())
    {
        f.SetPointSize(Size-DESCRIPTION_FONT_SHRINK);
        dc.SetFont(f);
        int font_height_description = dc.GetCharHeight();

        f.SetPointSize(Size);
        dc.SetFont(f);

        int lines = count_linenumbers(Description);
        height = (std::max)(ICON_HEIGHT + PANEL_MARGIN_DESC + PANEL_MARGIN_DESC_BOTTOM, dc.GetCharHeight() + font_height_description*lines + DESCRIPTION_TOP_MARGIN + PANEL_MARGIN_DESC + PANEL_MARGIN_DESC_BOTTOM);
    }

    return wxSize(width, height);
}

void SimpleButton::paintEvent(wxPaintEvent &evt)
{
    int width = GetSize().GetWidth();
    int height = GetSize().GetHeight();
    int x = ICON_LEFT_MARGIN;

    wxBufferedPaintDC dc(this);
    wxFont f(Bold ? font.Bold() : font);

    if(FrameMode)
    {
        if(Hovered)
        {
            if(Selected)
              dc.SetPen(wxPen(SelectedColour));
            else
              dc.SetPen(wxPen(PenColour));
        }
        else
            dc.SetPen(*wxTRANSPARENT_PEN);
    }
    else
    {
        if(Transparent || (Transparent && Hovered))
            dc.SetPen(*wxTRANSPARENT_PEN);
        else
            dc.SetPen(wxPen(PenColour));
    }

    if(Disabled || Paused)
    {
        dc.SetBrush(wxBrush(NormalColour));
    }
    else if (Selected)
    {
        if (Pressed) dc.SetBrush(wxBrush(PressedColour));
        else if (Hovered)
        {
            if (FrameMode) dc.SetBrush(wxBrush(COLOR_BUTTON_FRAME_SELECTED));
            else dc.SetBrush(wxBrush(HoveredColour));
        }
        else
        {
            if (FrameMode) dc.SetBrush(wxBrush(COLOR_BUTTON_FRAME_SELECTED));
            else dc.SetBrush(wxBrush(NormalColour));
        }
    }
    else
    {
        if (Pressed) dc.SetBrush(wxBrush(PressedColour));
        else if (Hovered) dc.SetBrush(wxBrush(HoveredColour));
        else dc.SetBrush(wxBrush(NormalColour));
    }
    dc.DrawRectangle(0, 0, width, height);
    if(Selected)
    {
        dc.SetBrush(wxBrush(SelectedColour));
        dc.DrawRectangle(0, 0, 3, height);
    }

    f.SetPointSize(Size);
    dc.SetFont(f);
    if(Disabled ||Paused)
        dc.SetTextForeground(wxColor(127, 127, 127));
    else
    {
        dc.SetTextForeground(Hovered ? TextColourHovered : TextColour);
    }

    int char_height = dc.GetCharHeight();

    wxBitmap& bmp = (Disabled || Paused) ? bitmap_disabled : bitmap;
    if(bmp.IsOk())
    {
        if(Description.empty())
            dc.DrawBitmap(bmp, x, (height-ICON_HEIGHT)/2);
        else
            dc.DrawBitmap(bmp, x, ICON_TOP_MARGIN);
        x += bmp.GetWidth();
        x += ICON_RIGHT_MARGIN;
    }

    if(Description.empty())
    {
        if(Centered)
        {
            wxSize size = dc.GetTextExtent(Text);
            x = (width - size.GetWidth())/2;
            dc.DrawText(Text, x, (height - char_height)/2);
        }
        else
            dc.DrawText(Text, x, (height - char_height)/2);
    }
    else
    {
        f.SetPointSize(Size-DESCRIPTION_FONT_SHRINK);
        dc.SetFont(f);
        int font_height_description = dc.GetCharHeight();

        f.SetPointSize(Size);
        dc.SetFont(f);

        int y = PANEL_MARGIN_DESC;
        dc.DrawText(Text, x, y);
        y += char_height;
        y += DESCRIPTION_TOP_MARGIN;

        f.SetPointSize(Size-DESCRIPTION_FONT_SHRINK);
        dc.SetFont(f);
        dc.DrawText(Description, x, y);

    }
}

void SimpleButton:: SetText(wxString s)
{
    Text = s;
    InvalidateBestSize();
    set_client_size();
    Refresh();
}

void SimpleButton:: SetDescription(wxString s)
{
    Description = s;
    InvalidateBestSize();
    set_client_size();
    Refresh();
}

void SimpleButton::SetTextColour(wxColour c)
{
    TextColour = c;
    Refresh();
}

void SimpleButton::SetHoveredTextColour(wxColour c)
{
    TextColourHovered = c;
    Refresh();
}


void SimpleButton::SetNormalColour(wxColour c)
{
    NormalColour = c;
    Refresh();
}

void SimpleButton::SetHoveredColour(wxColour c)
{
    HoveredColour = c;
    Refresh();
}

void SimpleButton::SetPressedColour(wxColour c)
{
    PressedColour = c;
    Refresh();
}

void SimpleButton::SetPenColour(wxColour c)
{
    PenColour = c;
    Refresh();
}


void SimpleButton:: mouseMoved(wxMouseEvent &evt)
{
    if (!Hovered)
    {
        Hovered = true;
        Refresh();
    }
}

void SimpleButton:: mouseDown(wxMouseEvent &evt)
{
    if (!Pressed)
    {
        Pressed = true;
    }
    Refresh();
}

void SimpleButton:: mouseReleased(wxMouseEvent &evt)
{
    if (Pressed)
    {
        Pressed = false;
        if(!Disabled && !Paused)
        {
            wxCommandEvent* e = new wxCommandEvent(myEVT_SimpleButtonClicked, GetId() );
            e->SetEventObject(this);
            wxQueueEvent(GetEventHandler(), e);
        }

    }
    Refresh();
}

void SimpleButton:: mouseLeftWindow(wxMouseEvent &evt)
{
    Pressed = false;
    Hovered = false;

    Refresh();
}

void SimpleButton:: sizeEvent(wxSizeEvent &evt)
{
    Refresh();
}

void SimpleButton::SetBitmap(const wxBitmap& bitmap)
{
    if(bitmap.GetWidth() == ICON_WIDTH && bitmap.GetHeight() == ICON_HEIGHT)
    {
        this->bitmap = bitmap;
    }
    else
    {
        // scaling
        wxImage image = bitmap.ConvertToImage();    
        this->bitmap = wxBitmap(image.Scale(ICON_WIDTH, ICON_HEIGHT));
    }
    bitmap_disabled = this->bitmap.ConvertToDisabled ();
    Refresh();
}

void SimpleButton::SetBold(bool bold)
{
    Bold = bold;
}

void SimpleButton::SetPaused(bool pause)
{
    Paused = pause;
    Refresh();
}

void SimpleButton::SetFrameMode(bool enabled)
{
    FrameMode = enabled;
}


void SimpleButton::SetTransparent(bool transparent)
{
    Transparent = transparent;
}


void SimpleButton::SetDisabled(bool disabled)
{
    Disabled = disabled;
    Refresh();
}

void SimpleButton::SetSelected(bool selected)
{
    Selected = selected;
    Refresh();
}

void SimpleButton::SetAlignCentered(bool centered)
{
    Centered = centered;
    Refresh();
}


wxDEFINE_EVENT(myEVT_SimpleButtonClicked, wxCommandEvent);

BEGIN_EVENT_TABLE(SimpleButton, wxPanel)

    EVT_PAINT(SimpleButton::paintEvent)
    EVT_MOTION(SimpleButton::mouseMoved)
    EVT_LEFT_DOWN(SimpleButton::mouseDown)
    EVT_LEFT_UP(SimpleButton::mouseReleased)
    EVT_LEAVE_WINDOW(SimpleButton::mouseLeftWindow)
    EVT_SIZE(SimpleButton::sizeEvent)

END_EVENT_TABLE()
