// Copyright (C) 2021 mana
//
// This file is part of ddrescue-windows.
//
// ddrescue-windows is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// ddrescue-windows is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ddrescue-windows.  If not, see <http://www.gnu.org/licenses/>.

#include "missing.h"

#include <map>
#include <Windows.h>
#include <iostream>

#define _WIN32_DCOM
#include <comdef.h>
#include <wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")

#include <fcntl.h>
#include <io.h>

int sigaction(int signum, const struct sigaction* act,
              struct sigaction* oldact)
{
    signal(signum, act->sa_handler);

    return 0;
}

int sigemptyset(sigset_t* set)
{
    return 0;
}


int tcflush(int fildes, int queue_selector)
{
    return 0;
}

static std::vector<PhyDiskInfo> phy_disk;

static std::map<HANDLE, std::wstring> handle_map;


HRESULT GetDevicesInfo()
{
    HRESULT hr;
    hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize COM library. Error code = 0x"
            << std::hex << hr << std::endl;
        return hr;
    }

    hr = CoInitializeSecurity(
        NULL,                        // Security descriptor
        -1,                          // COM negotiates authentication service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication level for proxies
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation level for proxies
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities of the client or server
        NULL);                       // Reserved

    if (FAILED(hr))
    {
        std::cout << "Failed to initialize security. Error code = 0x"
            << std::hex << hr << std::endl;
        CoUninitialize();
        return hr;                  // Program has failed.
    }

    IWbemLocator* pLoc = 0;
    hr = CoCreateInstance(CLSID_WbemLocator, 0,
        CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hr))
    {
        std::cout << "Failed to create IWbemLocator object. Err code = 0x"
            << std::hex << hr << std::endl;
        CoUninitialize();
        return hr;     // Program has failed.
    }

    IWbemServices* pSvc = 0;

    // Connect to the root\default namespace with the current user.
    hr = pLoc->ConnectServer(
        BSTR(L"ROOT\\CIMV2"),  //namespace
        NULL,       // User name
        NULL,       // User password
        0,         // Locale
        NULL,     // Security flags
        0,         // Authority
        0,        // Context object
        &pSvc);   // IWbemServices proxy


    if (FAILED(hr))
    {
        std::cout << "Could not connect. Error code = 0x"
            << std::hex << hr << std::endl;
        pLoc->Release();
        CoUninitialize();
        return hr;      // Program has failed.
    }

    std::cout << "Connected to WMI" << std::endl;


    // Set the proxy so that impersonation of the client occurs.
    hr = CoSetProxyBlanket(pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hr))
    {
        std::cout << "Could not set proxy blanket. Error code = 0x"
            << std::hex << hr << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return hr;      // Program has failed.
    }


    IEnumWbemClassObject* pEnumerator = NULL;
    hr = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hr))
    {
        std::cout << "Query for Win32_DiskDrive failed."
            << " Error code = 0x"
            << std::hex << hr << std::endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.hr
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        PhyDiskInfo info;

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // Get the value of the Name property

        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        info.Name = vtProp.bstrVal;
        std::wcout << " Name : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        info.Model = vtProp.bstrVal;
        std::wcout << " Model : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"BytesPerSector", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_I4)
        {
            goto WMI_ERROR;
        }
        info.BytesPerSector = vtProp.lVal;
        std::wcout << " BytesPerSector : " << vtProp.lVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"Size", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        info.Size = _wtoll(vtProp.bstrVal);
        std::wcout << " Size : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"TotalCylinders", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        info.TotalCylinders = _wtoll(vtProp.bstrVal);
        std::wcout << " TotalCylinders : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"TotalHeads", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_I4)
        {
            goto WMI_ERROR;
        }
        std::wcout << " TotalHeads : " << vtProp.lVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"TotalSectors", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        std::wcout << " TotalSectors : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"TotalTracks", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_BSTR)
        {
            goto WMI_ERROR;
        }
        std::wcout << " TotalTracks : " << vtProp.bstrVal << std::endl;
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"TracksPerCylinder", 0, &vtProp, 0, 0);
        if (FAILED(hr) || vtProp.vt != VT_I4)
        {
            goto WMI_ERROR;
        }
        std::wcout << " TracksPerCylinder : " << vtProp.lVal << std::endl;
        VariantClear(&vtProp);
        pclsObj->Release();

        phy_disk.push_back(info);

        std::cout << " -------------- " << std::endl;
    }



    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;   // Program successfully completed.

WMI_ERROR:
    std::cout << "Query for Win32_DiskDrive failed."
        << " Error code = 0x"
        << std::hex << hr << std::endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    return 1;
}

int __cdecl OpenLowLevel(
    _In_z_ char const* _FileName,
    _In_   int         _OpenFlag,
    ...
)
{
    DWORD flag = GENERIC_READ;
    flag |= (_OpenFlag & _O_WRONLY) ? GENERIC_WRITE : 0;

    DWORD createMode = createMode = (_OpenFlag & _O_TRUNC) ? TRUNCATE_EXISTING : CREATE_ALWAYS;
    if (strncmp(_FileName, "\\\\", 2) == 0)
    {
        createMode = OPEN_EXISTING;
    }

    HANDLE hFile = CreateFileA(
        _FileName,
        flag,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        createMode,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

        return -1;
    }

    WCHAR wideFilename[MAX_PATH];
    if (!MultiByteToWideChar(CP_ACP, 0, _FileName, -1, wideFilename, MAX_PATH))
    {
        return -1;
    }

    handle_map.insert_or_assign(hFile, std::wstring(wideFilename));

    return _open_osfhandle((intptr_t)hFile, _OpenFlag);
}

long long __cdecl LseekFixed(
    _In_ int  _FileHandle,
    _In_ long long _Offset,
    _In_ int  _Origin
)
{
    HANDLE hFile = (HANDLE)_get_osfhandle(_FileHandle);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    if (_Origin == SEEK_END)
    {
        // dirty hack
        if (handle_map.count(hFile))
        {
            std::wstring& fname = handle_map.at(hFile);
            auto i = std::find_if(phy_disk.begin(), phy_disk.end(),
                [&fname](auto it)->bool {return it.Name == fname; }
            );

            if (i == phy_disk.end())
            {
                return -1;
            }
            else
            {
                return _lseeki64(_FileHandle, _Offset + i->Size, SEEK_SET);
            }
        }
        else
        {
            return _lseeki64(_FileHandle, _Offset, _Origin);
        }
    }
    else
    {
        return _lseeki64(_FileHandle, _Offset, _Origin);
    }

}
