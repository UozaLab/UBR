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

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

#include <memory>

#ifndef __GNUC__
#if (_MSC_VER <= 1900) // Visual Studio 2015
using std::tr1::shared_ptr;
using std::tr1::enable_shared_from_this;
#else
using std::shared_ptr;
using std::enable_shared_from_this;
#endif
#else
using std::shared_ptr;
using std::enable_shared_from_this;
#endif

#endif