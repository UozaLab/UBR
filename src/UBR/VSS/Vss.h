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

#ifndef __VSS_H__
#define __VSS_H__

#include <windows.h>
#include <atlbase.h>

#include <vss.h>
#include <vswriter.h>
#include <vsbackup.h>
#include <vsmgmt.h>

#include <vector>
#include "tstring.h"

#define CHECK_COM( Call )                                                 \
{                                                                         \
    HRESULT hrInternal = Call;                                            \
    if(FAILED(hrInternal) && hrInternal != RPC_E_TOO_LATE)                \
    {                                                                     \
        tostringstream o;                                                 \
        o << _T("ERROR[VSS] ") << string2tstring(#Call);                  \
        o << " HRESULT(0x" << std::hex << hrInternal << ")" << std::endl; \
        OutputDebugString(o.str().c_str());                               \
        goto END;                                                         \
    }                                                                     \
}

class Vss
{
private:
    bool coinitialize_called;
    bool initialized;
    std::vector<VSS_SNAPSHOT_PROP> properties;
    CComPtr<IVssBackupComponents> bc;

    bool wait_and_check_async_operation(IVssAsync*  pAsync);
    bool initialize_backup_component();

public:
    Vss();
    ~Vss();

    bool CreateSnapshot(const std::vector<tstring>& volumes);
    tstring FindSnapshotVolume(const tstring& volume_name);
};

#endif
