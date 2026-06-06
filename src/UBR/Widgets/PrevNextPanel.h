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

#ifndef __PREVNEXT_H__
#define __PREVNEXT_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include "SimpleButton.h"
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>

class PrevNextPanel : public wxPanel
{
	private:

	protected:
		SimpleButton* m_simplebutton_prev;
		SimpleButton* m_simplebutton_next;
        bool enhanced;

	public:
		PrevNextPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );
		~PrevNextPanel();

        wxEvtHandler* GetButtonPrev(){ return m_simplebutton_prev; }
        wxEvtHandler* GetButtonNext(){ return m_simplebutton_next; }

        void SetEnabledPrev(bool enable);
        void SetEnabledNext(bool enable);
        void SetTextPrev(wxString str);
        void SetTextNext(wxString str);
        void SetEnhanceNext(bool _enhanced);

};

#endif
