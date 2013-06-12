/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
#ifndef ___ENUM_DEVICES_H___
#define ___ENUM_DEVICES_h___ 1

#include "GTypes.h"

#include <Setupapi.h>

int WinEnumDevices(GUID *pClassGuid, unsigned int vendorId, unsigned int productId, StringVector *pDevicesVector);

int WinEnumHIDDevices(unsigned int vendorId, unsigned int productId, StringVector *pDevicesVector);


/////////////////////////////////////////////////////////////////////////////////////////////
//Declarations that support dynamically binding to setupapi.dll.
/////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _UNICODE

typedef HDEVINFO (WINAPI *PSetupDiGetClassDevs)
(
    LPGUID ClassGuid,
    PCWSTR Enumerator,
    HWND   hwndParent,
    DWORD  Flags
);

typedef BOOL (WINAPI *PSetupDiGetDeviceInterfaceDetail)
(
    HDEVINFO                           DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData,
    DWORD                              DeviceInterfaceDetailDataSize,
    PDWORD                             RequiredSize,
    PSP_DEVINFO_DATA                   DeviceInfoData
);

#else

typedef HDEVINFO (WINAPI *PSetupDiGetClassDevs)
(
    LPGUID ClassGuid,
    PCSTR  Enumerator,
    HWND   hwndParent,
    DWORD  Flags
);

typedef BOOL (WINAPI *PSetupDiGetDeviceInterfaceDetail)
(
    HDEVINFO                           DeviceInfoSet,
    PSP_DEVICE_INTERFACE_DATA          DeviceInterfaceData,
    PSP_DEVICE_INTERFACE_DETAIL_DATA_A DeviceInterfaceDetailData,
    DWORD                              DeviceInterfaceDetailDataSize,
    PDWORD                             RequiredSize,
    PSP_DEVINFO_DATA                   DeviceInfoData
);

#endif

typedef BOOL (WINAPI *PSetupDiEnumDeviceInterfaces)
(
    HDEVINFO                  DeviceInfoSet,
    PSP_DEVINFO_DATA          DeviceInfoData,
    LPGUID                    InterfaceClassGuid,
    DWORD                     MemberIndex,
    PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData
);

typedef BOOL (WINAPI *PSetupDiGetDeviceRegistryProperty)
(
    HDEVINFO         DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData,
    DWORD            Property,
    PDWORD           PropertyRegDataType,
    PBYTE            PropertyBuffer,
    DWORD            PropertyBufferSize,
    PDWORD           RequiredSize
);

typedef BOOL (WINAPI *PSetupDiDestroyDeviceInfoList)
(
    HDEVINFO DeviceInfoSet
);

HINSTANCE WinLoadSetupApiLibrary();

extern HINSTANCE hWinSetupApiLibrary;//Store value returned by WinLoadSetupApiLibrary() here.
extern PSetupDiGetClassDevs pSetupDiGetClassDevs;
extern PSetupDiEnumDeviceInterfaces pSetupDiEnumDeviceInterfaces;
extern PSetupDiGetDeviceInterfaceDetail pSetupDiGetDeviceInterfaceDetail;
extern PSetupDiGetDeviceRegistryProperty pSetupDiGetDeviceRegistryProperty;
extern PSetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList;


//Use the following macros to access the SetupApi.dll entry points. These names are identical to the
//microsoft names except that they are prefixed by 'dyn'.
#define dynSetupDiGetClassDevs (*pSetupDiGetClassDevs)
#define dynSetupDiEnumDeviceInterfaces (*pSetupDiEnumDeviceInterfaces)
#define dynSetupDiGetDeviceInterfaceDetail (*pSetupDiGetDeviceInterfaceDetail)
#define dynSetupDiGetDeviceRegistryProperty (*pSetupDiGetDeviceRegistryProperty)
#define dynSetupDiDestroyDeviceInfoList (*pSetupDiDestroyDeviceInfoList)

/////////////////////////////////////////////////////////////////////////////////////////////
//Declarations that support dynamically binding to hid.dll .
/////////////////////////////////////////////////////////////////////////////////////////////
typedef void (__stdcall *PHidD_GetHidGuid)
(
	LPGUID   HidGuid
);

typedef BOOLEAN (__stdcall *PHidD_GetNumInputBuffers)
(
    HANDLE  HidDeviceObject,
    PULONG  NumberBuffers
);

typedef BOOLEAN (__stdcall *PHidD_SetNumInputBuffers)
(
    HANDLE HidDeviceObject,
    ULONG  NumberBuffers
);

HINSTANCE WinLoadHidDLibrary();

extern HINSTANCE hWinHidDLibrary;//Store value returned by WinLoadHidDLibrary() here.
extern PHidD_GetHidGuid pHidD_GetHidGuid;
extern PHidD_GetNumInputBuffers pHidD_GetNumInputBuffers;
extern PHidD_SetNumInputBuffers pHidD_SetNumInputBuffers;

//Use the following macros to access the hid.dll entry points. These names are identical to the
//microsoft names except that they are prefixed by 'dyn'.
#define dynHidD_GetHidGuid (*pHidD_GetHidGuid)
#define dynHidD_GetNumInputBuffers (*pHidD_GetNumInputBuffers)
#define dynHidD_SetNumInputBuffers (*pHidD_SetNumInputBuffers)


#endif
