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

#ifndef __MISC_WX_H___
#define __MISC_WX_H___

#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <map>
#include "Widgets/SimpleButton.h"
#include "SimpleIni.h"
#include "resource.h"
#include "resource_lang.h"

class Utility
{
public:
    void static SetIcon(SimpleButton* button, int iconid, wxString text);
    void static SetIcon(wxBitmapButton* button, int iconid);
    void static SetIcon(wxStaticBitmap* bmp, int iconid);
    wxString static GetVersion();

};

class MultiLanguage
{
private:
    static MultiLanguage* _instance;
    wxString locale_opened;
protected:
    CSimpleIniW ini;
    CSimpleIniW ini_res;
    wxString locale;
    bool damaged;
    bool resource_loaded;

    MultiLanguage();
public:
    static MultiLanguage* Instance();
    void SetLocale(wxString lng)
    {
        locale = lng;
        damaged = true;
    }
    wxString GetString(wxString category, wxString name);
    std::map<wxString, int>* GetLangMap()
    {
        static std::map<wxString, int> id_map
        {
            {"en-us", IDR_LANG_EN_US},
            {"ja-jp", IDR_LANG_JA_JP},
        };
        return &id_map;
    }
};

wxString ttt(wxString name);


#endif
