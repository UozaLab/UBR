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

#include "PrevNextPanel.h"
#include "Defs.h"
#include "MiscWx.h"


PrevNextPanel::PrevNextPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) 
: wxPanel( parent, id, pos, size, style, name ),
  enhanced(false)
{
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxHORIZONTAL );


	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_simplebutton_prev = new SimpleButton( this, wxID_ANY);
    m_simplebutton_prev->SetAlignCentered(true);
    m_simplebutton_prev->SetDisabled(true);
    m_simplebutton_prev->SetText(ttt("PrevBtnText"));
    m_simplebutton_prev->SetBold(true);
	bSizer1->Add( m_simplebutton_prev, 0, wxALIGN_BOTTOM | wxALL, 5 );

	m_simplebutton_next = new SimpleButton( this, wxID_ANY);
    m_simplebutton_next->SetAlignCentered(true);
    m_simplebutton_next->SetDisabled(true);
    m_simplebutton_next->SetText(ttt("NextBtnText"));
    m_simplebutton_next->SetBold(true);
	bSizer1->Add( m_simplebutton_next, 0, wxALIGN_BOTTOM | wxALL, 5 );


	bSizer1->Add( 0, 0, 1, wxEXPAND, 5 );


	this->SetSizer( bSizer1 );
	this->Layout();
}

PrevNextPanel::~PrevNextPanel()
{
}

void PrevNextPanel::SetEnabledPrev(bool enable)
{
    m_simplebutton_prev->SetDisabled(!enable);
    m_simplebutton_prev->SetTransparent(!enable);
    m_simplebutton_prev->SetPenColour(COLOR_BUTTON_PEN);
    m_simplebutton_prev->SetPressedColour(COLOR_BUTTON_PRESSED_FRAME);
}

void PrevNextPanel::SetEnabledNext(bool enable)
{
    m_simplebutton_next->SetDisabled(!enable);
    m_simplebutton_next->SetTransparent(!enable);
    if(enable)
    {
        if(enhanced)
        {
            m_simplebutton_next->SetNormalColour(COLOR_BUTTON_FACE_ENHANCE);
            m_simplebutton_next->SetTextColour(COLOR_BUTTON_TEXT_ENHANCE);
            m_simplebutton_next->SetHoveredTextColour(COLOR_BUTTON_TEXT_ENHANCE);
            m_simplebutton_next->SetPenColour(COLOR_BUTTON_PEN_ENHANCE);
            m_simplebutton_next->SetHoveredColour(COLOR_BUTTON_HOVER_ENHANCE);
            m_simplebutton_next->SetPressedColour(COLOR_BUTTON_PRESSED_ENHANCE);
        }
        else
        {
            m_simplebutton_next->SetNormalColour(COLOR_BUTTON_FACE_VIVID);
            m_simplebutton_next->SetTextColour(COLOR_BUTTON_TEXT_VIVID);
            m_simplebutton_next->SetHoveredTextColour(COLOR_BUTTON_TEXT_VIVID);
            m_simplebutton_next->SetPenColour(COLOR_BUTTON_PEN_VIVID);
            m_simplebutton_next->SetHoveredColour(COLOR_BUTTON_HOVER_VIVID);
            m_simplebutton_next->SetPressedColour(COLOR_BUTTON_PRESSED_VIVID);
        }
    }
    else
    {
        m_simplebutton_next->SetNormalColour(COLOR_BUTTON_FACE);
        m_simplebutton_next->SetTextColour(COLOR_BUTTON_TEXT);
        m_simplebutton_next->SetPenColour(COLOR_BUTTON_PEN);
    }
}

void PrevNextPanel::SetTextPrev(wxString str)
{
    m_simplebutton_prev->SetText(str);
}
void PrevNextPanel::SetTextNext(wxString str)
{
    m_simplebutton_next->SetText(str);
}

void PrevNextPanel::SetEnhanceNext(bool _enhanced)
{
    enhanced = _enhanced;
}
