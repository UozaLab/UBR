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

#ifndef __PROGRESSPANEL_H__
#define __PROGRESSPANEL_H__

#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/dcclient.h>
#include <wx/gdicmn.h>
#include <wx/dcbuffer.h>
#include <wx/graphics.h>


class ProgressPanel : public wxPanel
{
private:
    void paintEvent (wxPaintEvent&);
    int progress;
    bool show_percent;
    bool mode_error;

public:
    ProgressPanel() { progress=0;show_percent=false;mode_error=false; }
    ProgressPanel(wxWindow* parent, const long id, 
                 wxPoint position = wxDefaultPosition,
                 wxSize size = wxDefaultSize,
                 long style = 0);
    bool Create(wxWindow* parent, const long id, 
                 wxPoint position = wxDefaultPosition,
                 wxSize size = wxDefaultSize,
                 long style = 0) { return wxPanel::Create(parent, id, position, size, style); }

    void SetProgress(int value);
    void ShowPercent(bool show);
    void SetError(bool error);

    DECLARE_EVENT_TABLE()
};

#endif
