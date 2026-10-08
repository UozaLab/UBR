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

#ifndef __Decompress__H__
#define __Decompress__H__

#include <wx/wx.h>
#include "Events.h"
#include "DataHolder.h"

class DecompressWorker : public CustomThread
{
protected:
    const CompressData* compress_data;

public:
    DecompressWorker(wxEvtHandler* event_handler, const CompressData* _compress_data);
    virtual void *Entry();

};


#endif
