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

#include "ProgressPanel.h"

ProgressPanel::ProgressPanel(wxWindow* parent, const long id, 
                             wxPoint position,
                             wxSize size,
                             long style) :
wxPanel(parent, id, position, size),
progress(0),
show_percent(false),
mode_error(false)
{
}

void ProgressPanel::ShowPercent(bool show)
{
    show_percent = show;
    Refresh();
    Update();
}

void ProgressPanel::SetError(bool error)
{
    mode_error = error;
    Refresh();
    Update();
}

void ProgressPanel:: paintEvent (wxPaintEvent &evt)
{
    int width = GetSize().GetWidth();
    int height = GetSize().GetHeight();

    wxBufferedPaintDC dc(this);
    if(mode_error)
    {
        dc.SetBrush(wxBrush(wxColour(200, 0, 0)));
    }
    else
    {
        dc.SetBrush(wxBrush(wxColour(216, 228, 248)));// pale blue
    }
    wxPen pen;
    pen.SetColour(wxColour(255,255,255));
    dc.SetPen(pen);
    dc.DrawRectangle(0, 0, width, height);

    if(mode_error)
    {
        dc.SetBrush(wxBrush(wxColour(90, 0, 0)));
    }
    else
    {
        dc.SetBrush(wxBrush(wxColour(90, 170, 235)));
    }
    dc.DrawRectangle(0, 0, (int)((float) width * (float)progress / 100.0), height);

    if(show_percent)
    {
        wxString Text = wxString::Format("%d%%", progress);
        wxSize size = dc.GetTextExtent(Text);
        int x = (width - size.GetWidth())/2;
        int char_height = dc.GetCharHeight();
        dc.SetTextForeground(wxColor(0, 0, 0));
        dc.DrawText(Text, x, (height - char_height)/2);
    }
}

void ProgressPanel::SetProgress(int value)
{
    progress = value;
    Refresh();
    Update();
}

BEGIN_EVENT_TABLE(ProgressPanel, wxPanel)
    EVT_PAINT(ProgressPanel::paintEvent)
END_EVENT_TABLE()
