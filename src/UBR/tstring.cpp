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

#include <windows.h>
#include "tstring.h"

std::wstring string2wstring(const std::string& str)
{
    UINT codepage = CP_UTF8;
    int size_needed = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string wstring2string(const std::wstring &wstr)
{
    UINT codepage = CP_UTF8;
    int size_needed = WideCharToMultiByte(codepage, 0, &wstr[0], (int)wstr.size(), 0, 0, 0, 0);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(codepage, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, 0, 0);
    return strTo;
}


tstring string2tstring(const std::string& str)
{
    tstring result;
#ifdef _UNICODE
    result = string2wstring(str);
#else
    result = str;
#endif
    return result;
}

tstring wstring2tstring(const std::wstring& wstr)
{
    tstring result;
#ifdef _UNICODE
    result = wstr;
#else
    result = wstring2string(wstr);
#endif
    return result;
}

