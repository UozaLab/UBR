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

// hacks for compiling SimpleIni.h under c++03(VS2008) environment
#ifndef __SIMPLE_INI_INC_H__
#define __SIMPLE_INI_INC_H__

#define using / ## /
#define constexpr / ## /

extern int SI_OK;
extern int SI_UPDATED;
extern int SI_INSERTED;

extern int SI_FAIL;
extern int SI_NOMEM;
extern int SI_FILE;

typedef int SI_Error;

#include "SimpleIni.h"

#undef constexpr
#undef using

#endif
