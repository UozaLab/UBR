/*
 * Copyright (c) 2024-2026, Uoza Lab
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

#include "PrivilegeControl.h"

PrivilegeControl::PrivilegeControl()
: process_token(NULL)
{
    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &process_token);
}

PrivilegeControl::~PrivilegeControl()
{
    if(process_token != NULL)
        CloseHandle(process_token);
}

BOOL PrivilegeControl::SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    if(process_token == NULL) return FALSE;

    TOKEN_PRIVILEGES tp;
    LUID luid;

    if(!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        return FALSE; 
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    if(!AdjustTokenPrivileges(process_token, FALSE, &tp, 0, (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL))
    {
        return FALSE; 
    } 

    if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        return FALSE;
    } 

    return TRUE;
}

