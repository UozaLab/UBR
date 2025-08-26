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

#ifndef __Restore__H__
#define __Restore__H__

#include <wx/wx.h>
#include "smart_ptr.h"
#include "Events.h"
#include "DataHolder.h"
#include "FileSystem/DiskInfo.h"

class RestoreWorker : public CustomThread
{
protected:
    const RestoreData* restore_data;
    shared_ptr<DiskInfo> di;

public:
    RestoreWorker(wxEvtHandler* event_handler, const RestoreData* _restore_data, shared_ptr<DiskInfo> _di);
    virtual void *Entry();

};


#endif
