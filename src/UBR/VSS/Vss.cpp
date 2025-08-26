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

#include "Vss.h"
#include <iostream>

#pragma comment(lib, "VssApi.lib")

Vss::Vss()
: coinitialize_called(false), initialized(false)
{
    CHECK_COM(CoInitializeEx(NULL, COINIT_MULTITHREADED));
    coinitialize_called = true;

    CHECK_COM(CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL));

    initialized = true;
END:
    ;
}

Vss::~Vss()
{
    for(std::vector<VSS_SNAPSHOT_PROP>::iterator itr = properties.begin(), itr_end = properties.end(); itr != itr_end; itr++)
    {
        VSS_SNAPSHOT_PROP prop = *itr;
        VssFreeSnapshotProperties(&prop);
    }

    if(coinitialize_called)
        CoUninitialize();
}

bool Vss::initialize_backup_component()
{
    if(!initialized) return false;
    CHECK_COM(CreateVssBackupComponents(&bc));
    CHECK_COM(bc->InitializeForBackup());
    CHECK_COM(bc->SetBackupState(false, true, VSS_BT_COPY, false));
    return true;

END:
    bc = NULL;
    return false;
}

bool Vss::wait_and_check_async_operation(IVssAsync* pAsync)
{
    CHECK_COM(pAsync->Wait());

    HRESULT hrReturned = S_OK;
    CHECK_COM(pAsync->QueryStatus(&hrReturned, NULL));

    if(FAILED(hrReturned)) return false;
    return true;

END:
    return false;
}

bool Vss::CreateSnapshot(const std::vector<tstring>& volumes)
{
	if(!initialized) return false;

    std::vector<VSS_ID> snapshot_ids;
    if(!initialize_backup_component()) goto END;

    {
        CComPtr<IVssAsync> async;
        CHECK_COM(bc->GatherWriterMetadata(&async));
        if(!wait_and_check_async_operation(async)) goto END;
    }

    VSS_ID snapshot_set_id;
    CHECK_COM(bc->StartSnapshotSet(&snapshot_set_id));

    for(std::vector<tstring>::const_iterator itr = volumes.begin(), itr_end = volumes.end(); itr != itr_end; itr++)
    {
        VSS_ID SnapshotID;
        tstring volume_name = *itr + _T("\\");
        CHECK_COM(bc->AddToSnapshotSet((LPWSTR)volume_name.c_str(), GUID_NULL, &SnapshotID));
        snapshot_ids.push_back(SnapshotID);
    }

    {
        CComPtr<IVssAsync> async;
        CHECK_COM(bc->PrepareForBackup(&async));
        if(!wait_and_check_async_operation(async)) goto END;
    }

    {
        CComPtr<IVssAsync> async;
        CHECK_COM(bc->DoSnapshotSet(&async));
        if(!wait_and_check_async_operation(async)) goto END;
    }

    {
        CComPtr<IVssAsync> async;
        CHECK_COM(bc->BackupComplete(&async));
        if(!wait_and_check_async_operation(async)) goto END;
    }

    for(std::vector<VSS_ID>::iterator itr = snapshot_ids.begin(), itr_end = snapshot_ids.end(); itr != itr_end; itr++)
    {
        VSS_SNAPSHOT_PROP prop;
        CHECK_COM(bc->GetSnapshotProperties(*itr, &prop));
        properties.push_back(prop);
    }

    return true;

END:
    return false;
}

tstring Vss::FindSnapshotVolume(const tstring& volume_name)
{
    for(std::vector<VSS_SNAPSHOT_PROP>::iterator itr = properties.begin(), itr_end = properties.end(); itr != itr_end; itr++)
    {
        VSS_SNAPSHOT_PROP prop = *itr;
        if(prop.m_pwszOriginalVolumeName == volume_name)
        {
            return tstring(prop.m_pwszSnapshotDeviceObject);
        }
    }
    return tstring();
}

