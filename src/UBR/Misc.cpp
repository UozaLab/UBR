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

#include "Misc.h"
#include <iomanip>
#include <windows.h>
#include <winreg.h>

tstring Unit::HumanReadable(unsigned long long size)
{
    tostringstream oss;

    if(size < 1024ULL)
    {
        oss << size << "B";
    }
    else if(size < 1048576ULL)
    {
        double val = (double)size;
        val /= 1024;
        oss << std::fixed << std::setprecision(1) << val << "KB";
    }
    else if(size < 1073741824ULL)
    {
        double val = (double)size;
        val /= 1048576ULL;
        oss << std::fixed << std::setprecision(1) << val << "MB";
    }
    else
    {
        double val = (double)size;
        val /= 1073741824ULL;
        oss << std::fixed << std::setprecision(1) << val << "GB";
    }

    oss << std::resetiosflags(std::ios_base::floatfield);
    return oss.str();
}

bool WinPE::RunOnPE()
{
    DWORD result;
    HKEY key = NULL;
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinPE"),
                          0, KEY_QUERY_VALUE, &key);
    return (result == ERROR_SUCCESS);
}
