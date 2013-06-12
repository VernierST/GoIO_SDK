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

#include "stdafx.h"
#include "WinEnumDevices.h"
#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Variables used to implement dynamic binding to setupapi.dll .
HINSTANCE hWinSetupApiLibrary = NULL;
PSetupDiGetClassDevs pSetupDiGetClassDevs = NULL;
PSetupDiEnumDeviceInterfaces pSetupDiEnumDeviceInterfaces = NULL;
PSetupDiGetDeviceInterfaceDetail pSetupDiGetDeviceInterfaceDetail = NULL;
PSetupDiGetDeviceRegistryProperty pSetupDiGetDeviceRegistryProperty = NULL;
PSetupDiDestroyDeviceInfoList pSetupDiDestroyDeviceInfoList = NULL;

//Variables used to implement dynamic binding to hid.dll .
HINSTANCE hWinHidDLibrary = NULL;
PHidD_GetHidGuid pHidD_GetHidGuid = NULL;
PHidD_GetNumInputBuffers pHidD_GetNumInputBuffers = NULL;
PHidD_SetNumInputBuffers pHidD_SetNumInputBuffers = NULL;


HINSTANCE WinLoadSetupApiLibrary()
{
	HINSTANCE hLib = LoadLibrary("setupapi.dll");
	if (NULL != hLib)
	{
		bool bError = false;

#ifdef UNICODE
		pSetupDiGetClassDevs = (PSetupDiGetClassDevs) GetProcAddress(hLib, "SetupDiGetClassDevsW");
		bError = bError || (NULL == pSetupDiGetClassDevs);
		pSetupDiGetDeviceInterfaceDetail = (PSetupDiGetDeviceInterfaceDetail) GetProcAddress(hLib, "SetupDiGetDeviceInterfaceDetailW");
		bError = bError || (NULL == pSetupDiGetDeviceInterfaceDetail);
		pSetupDiGetDeviceRegistryProperty = (PSetupDiGetDeviceRegistryProperty) GetProcAddress(hLib, "SetupDiGetDeviceRegistryPropertyW");
		bError = bError || (NULL == pSetupDiGetDeviceRegistryProperty);
#else
		pSetupDiGetClassDevs = (PSetupDiGetClassDevs) GetProcAddress(hLib, "SetupDiGetClassDevsA");
		bError = bError || (NULL == pSetupDiGetClassDevs);
		pSetupDiGetDeviceInterfaceDetail = (PSetupDiGetDeviceInterfaceDetail) GetProcAddress(hLib, "SetupDiGetDeviceInterfaceDetailA");
		bError = bError || (NULL == pSetupDiGetDeviceInterfaceDetail);
		pSetupDiGetDeviceRegistryProperty = (PSetupDiGetDeviceRegistryProperty) GetProcAddress(hLib, "SetupDiGetDeviceRegistryPropertyA");
		bError = bError || (NULL == pSetupDiGetDeviceRegistryProperty);
#endif
		pSetupDiEnumDeviceInterfaces = (PSetupDiEnumDeviceInterfaces) GetProcAddress(hLib, "SetupDiEnumDeviceInterfaces");
		bError = bError || (NULL == pSetupDiEnumDeviceInterfaces);
		pSetupDiDestroyDeviceInfoList = (PSetupDiDestroyDeviceInfoList) GetProcAddress(hLib, "SetupDiDestroyDeviceInfoList");
		bError = bError || (NULL == pSetupDiDestroyDeviceInfoList);

		if (bError)
		{
			FreeLibrary(hLib);
			hLib = NULL;
			pSetupDiGetClassDevs = NULL;
			pSetupDiEnumDeviceInterfaces = NULL;
			pSetupDiGetDeviceInterfaceDetail = NULL;
			pSetupDiGetDeviceRegistryProperty = NULL;
			pSetupDiDestroyDeviceInfoList = NULL;
		}
	}

	return hLib;
}

HINSTANCE WinLoadHidDLibrary()
{
	HINSTANCE hLib = LoadLibrary("hid.dll");
	if (NULL != hLib)
	{
		bool bError = false;

		pHidD_GetHidGuid = (PHidD_GetHidGuid) GetProcAddress(hLib, "HidD_GetHidGuid");
		bError = (NULL == pHidD_GetHidGuid);
		pHidD_GetNumInputBuffers = (PHidD_GetNumInputBuffers) GetProcAddress(hLib, "HidD_GetNumInputBuffers");
		bError = bError || (NULL == pHidD_GetNumInputBuffers);
		pHidD_SetNumInputBuffers = (PHidD_SetNumInputBuffers) GetProcAddress(hLib, "HidD_SetNumInputBuffers");
		bError = bError || (NULL == pHidD_SetNumInputBuffers);

		if (bError)
		{
			FreeLibrary(hLib);
			hLib = NULL;
			pHidD_GetHidGuid = NULL;
			pHidD_GetNumInputBuffers = NULL;
			pHidD_SetNumInputBuffers = NULL;
		}
	}

	return hLib;
}


int WinEnumHIDDevices(unsigned int vendorId, unsigned int productId, StringVector *pDevicesVector)
{
	int numDevicesFound = 0;
	if (hWinHidDLibrary != NULL)
	{
		GUID hidguid;
 		dynHidD_GetHidGuid(&hidguid);
		numDevicesFound = WinEnumDevices(&hidguid, vendorId, productId, pDevicesVector);
	}

	return numDevicesFound;
}

int WinEnumDevices(GUID *pClassGuid, unsigned int vendorId, unsigned int productId, StringVector *pDevicesVector)
{
	int numDevicesFound = 0;
	if (hWinSetupApiLibrary != NULL)
	{
		HDEVINFO devs = dynSetupDiGetClassDevs(pClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
		if (devs != INVALID_HANDLE_VALUE)
		{
			SP_DEVINFO_DATA dev_info;
			SP_DEVICE_INTERFACE_DATA ifdata;
			dev_info.cbSize = sizeof(dev_info);
			ifdata.cbSize = sizeof(ifdata);
			int i;
			char hardware_id_string[_MAX_PATH];
			char test_id_string[_MAX_PATH];
			char details[_MAX_PATH];
			PSP_INTERFACE_DEVICE_DETAIL_DATA pDetails = (PSP_INTERFACE_DEVICE_DETAIL_DATA) details;
			pDetails->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			DWORD reqLength;
			GSTD_ASSERT(hWinHidDLibrary != NULL);
  			GUID hidguid;
  			dynHidD_GetHidGuid(&hidguid);
  			if (hidguid == (*pClassGuid))
				sprintf(test_id_string, "HID\\VID_%04x&PID_%04x", vendorId, productId);
			else
				sprintf(test_id_string, "USB\\VID_%04x&PID_%04x", vendorId, productId);

			for (i = 0; dynSetupDiEnumDeviceInterfaces(devs, NULL, pClassGuid, i, &ifdata); i++)
			{
				if (dynSetupDiGetDeviceInterfaceDetail(devs, &ifdata, pDetails, sizeof(details), &reqLength, &dev_info))
				{
					if (dynSetupDiGetDeviceRegistryProperty(devs, &dev_info, 
							SPDRP_HARDWAREID, NULL, (BYTE *) hardware_id_string, sizeof(hardware_id_string), NULL))
					{
						if (_strnicmp(hardware_id_string, test_id_string, strlen(test_id_string)) == 0)
						{
							//ID strings match.
							DWORD flags;
							unsigned int testStatus = dynSetupDiGetDeviceRegistryProperty(devs, &dev_info, 
									SPDRP_CONFIGFLAGS, NULL, (BYTE *) &flags, sizeof(flags), NULL);
							if (testStatus != 0)
							{
								//We are able to read SPDRP_CONFIGFLAGS, so we think the system is done with
								//adding the device to the device database.
								GSTD_ASSERT(reqLength < sizeof(details));
								pDevicesVector->push_back(pDetails->DevicePath);
								numDevicesFound++;
							}
						}
					}
				}
			}

			dynSetupDiDestroyDeviceInfoList(devs);
		}
	}
	return numDevicesFound;
}

