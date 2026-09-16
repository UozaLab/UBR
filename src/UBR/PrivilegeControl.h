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

#ifndef __PrivilegeControl_H___
#define __PrivilegeControl_H___

#include <windows.h>
#include <tchar.h>

class PrivilegeControl
{
private:
    HANDLE process_token;
public:
    PrivilegeControl();
    ~PrivilegeControl();
    bool IsValid(){return process_token != NULL;}
    BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
};

#endif
