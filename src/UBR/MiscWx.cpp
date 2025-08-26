/*
 * Copyright (c) 2024-2025, UozaLab
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

#include "MiscWx.h"
#include "resource.h"
#include "resource_lang.h"


void Utility::SetIcon(SimpleButton* button, int iconid, wxString text)
{
    HRSRC hbin = FindResource(NULL, MAKEINTRESOURCE(iconid), RT_RCDATA);
    const BYTE *bindata = (const BYTE*)LockResource(LoadResource(0, hbin));
    DWORD binsize = SizeofResource(0, hbin);

    wxBitmap bitmap = wxBitmap::NewFromPNGData(bindata, binsize);
    button->SetBitmap(bitmap);
    button->SetText(text);
}

void Utility::SetIcon(wxBitmapButton* button, int iconid)
{
    HRSRC hbin = FindResource(NULL, MAKEINTRESOURCE(iconid), RT_RCDATA);
    const BYTE *bindata = (const BYTE*)LockResource(LoadResource(0, hbin));
    DWORD binsize = SizeofResource(0, hbin);

    wxBitmap bitmap = wxBitmap::NewFromPNGData(bindata, binsize);
    button->SetBitmap(bitmap);
}

void Utility::SetIcon(wxStaticBitmap* bmp, int iconid)
{
    HRSRC hbin = FindResource(NULL, MAKEINTRESOURCE(iconid), RT_RCDATA);
    const BYTE *bindata = (const BYTE*)LockResource(LoadResource(0, hbin));
    DWORD binsize = SizeofResource(0, hbin);

    wxBitmap bitmap = wxBitmap::NewFromPNGData(bindata, binsize);
    bmp->SetBitmap(bitmap);
}

MultiLanguage* MultiLanguage::_instance = NULL;

MultiLanguage* MultiLanguage::Instance()
{
    if(_instance == NULL)
    {
        _instance = new MultiLanguage();
    }
    return _instance;
}

wxString ttt(wxString name)
{
    return MultiLanguage::Instance()->GetString("Strings", name);
}

MultiLanguage::MultiLanguage()
{
    locale="en"; 
    ini.SetUnicode();
    ini.SetQuotes();
    ini.SetMultiLine();
    damaged = true;

    ini_res.SetUnicode();
    ini_res.SetQuotes();
    ini_res.SetMultiLine();
    resource_loaded = false;
}

wxString MultiLanguage::GetString(wxString category, wxString name)
{
    if(!resource_loaded)
    {
        HMODULE handle = ::GetModuleHandle(NULL);
        HRSRC rc = ::FindResource(handle, MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT")/*MAKEINTRESOURCE(TEXT)*/);
        HGLOBAL rcData = ::LoadResource(handle, rc);
        DWORD size = ::SizeofResource(handle, rc);

        char* buffer = new char[size+1];
        ::memcpy(buffer, rcData, size);
        buffer[size] = 0;

        ini_res.LoadData(buffer, size);
        
        delete[] buffer;
        resource_loaded = true;
    }
    if(damaged)
    {
        wxFileName file = wxFileName(wxStandardPaths::Get().GetExecutablePath());
        file.AppendDir("lang");
        file.SetName(locale);
        file.SetExt("txt");
        ini.Reset();
        ini.LoadFile(file.GetFullPath().wchar_str());

        damaged = false;
    }
    const wchar_t* val = ini.GetValue(category, name, wxEmptyString);
    wxString result(val);
    if(result == wxEmptyString)
    {
        val = ini_res.GetValue(category, name, wxEmptyString);
        result = wxString(val);
    }
    result.Replace("\\n", "\n", true);
    return result;
}

wxString Utility::GetVersion()
{
    TCHAR exe_name[MAX_PATH + 1];
    ZeroMemory(exe_name, MAX_PATH+1);
    if(!GetModuleFileName(NULL, exe_name, MAX_PATH)) return wxEmptyString;

    DWORD handle;
    DWORD size = GetFileVersionInfoSize(exe_name, &handle);
    if(size == 0) return wxEmptyString;
    if(size >= 2048) return wxEmptyString;

    UINT8 buff[2048];
    ZeroMemory(buff, 2048);
    if(!GetFileVersionInfo(exe_name, NULL, size, buff)) return wxEmptyString;

    UINT length;
    VS_FIXEDFILEINFO* ver_info;
    VerQueryValue(buff, _T("\\"), reinterpret_cast<LPVOID*>(&ver_info), &length);

    WORD v1 = HIWORD(ver_info->dwProductVersionMS);
    WORD v2 = LOWORD(ver_info->dwProductVersionMS);
    WORD v3 = HIWORD(ver_info->dwProductVersionLS);
    WORD v4 = LOWORD(ver_info->dwProductVersionLS);

    return wxString::Format("%d.%d.%d", v1, v2, v3);
}

