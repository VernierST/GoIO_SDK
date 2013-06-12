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
// GSkipBaseDevice_Mac.cpp

#import <sstream>
#import "VST_USB.h"

#import "GSkipBaseDevice.h"
#import "GTextUtils.h"
#import "GUtils.h"

int local_ReadPackets(GSkipBaseDevice *pDevice, void * pBuffer, int * pIONumPackets, int nBufferSizeInPackets, int nPipe);
int local_PacketsAvailable(GSkipBaseDevice *pDevice, int nPipe);
int local_ClearPacketQueue(GSkipBaseDevice *pDevice, int nPipe);
int local_NumLastMeasurements(GSkipBaseDevice *pDevice);

bool GSkipBaseDevice::OSInitialize(void)
{
	bool bResult = false;
	SetOSData(NULL);
	UInt32 nLocationID = strtoul(GetPortRefPtr()->GetDisplayName().c_str(), NULL, 0);
	VST_USBSpec * pSpec = NULL;
	VST_GetUSBSpecByLocation(GetPortRefPtr()->GetUSBVendorID(), GetPortRefPtr()->GetUSBProductID(), nLocationID, true, &pSpec);
	if (pSpec != NULL)
	{
		TUSBBulkDevice usbDevice = VST_MakeUSBBulkDeviceFromSpec(pSpec);
		if (usbDevice == NULL)
		{
			// If another app (including Classic) has the device we
			// can simply retry a few times.  The driver should be able to get a lock
			// on the device at some point.
			int nTry = 0;
			while ((usbDevice == NULL) && (nTry < 4))
			{
				usbDevice = VST_MakeUSBBulkDeviceFromSpec(pSpec);
				nTry++;
			}
		}
		
		delete pSpec;
		
		if (usbDevice != NULL)
		{
			SetOSData(usbDevice);
			bResult = true;
		}
		else
			GSTD_TRACE(GSTD_S("GSkipBaseDevice::OSInitialize -- Couldn't create USB device."));
	}

	return bResult;
}

StringVector GSkipBaseDevice::OSGetAvailableDevicesOfType(int nVendorID, int nProductID)
{
	StringVector vPortNames;
	VST_USBSpecArrayRef specArray;
	VST_GetUSBSpecsForDevice(nVendorID, nProductID, true, &specArray);
	if (specArray)
	{
		int nNumUSBPorts = CFArrayGetCount(specArray);
		for (int i = 0; i < nNumUSBPorts; i++)
		{
			VST_USBSpec *pUSBSpec = (VST_USBSpec *)CFArrayGetValueAtIndex(specArray, (CFIndex)i);
			GSTD_ASSERT(pUSBSpec != NULL);
			
			// cppsstream (aka std::stringstream) doesn't work here when optimizations are turned on!
			// BUT! : std::strstream ( the alternative that was tried instead)'s str() function returns a string w/o null termination, so 
			// instantiating a cppstring with it gives undesired results
			// So, for now, we'll use cppsstream, and make sure that optimisations are OFF. blech
			//std::strstream ss;

			cppsstream ss;
			
			UInt32 nLocationID;
			VST_GetUSBSpecLocation(pUSBSpec, &nLocationID);

			ss << "0x" << hex;
			ss << nLocationID;
			ss << " (USB)";	// Legacy support: LP uses this suffix to distinguish the "location" from serial ports.
			
// debug
//			cppstring temp; 
//			temp = ss.str();
			
			vPortNames.push_back(ss.str());
		}
	}
	VST_ReleaseUSBSpecArray(specArray);

	return vPortNames;
}

int GSkipBaseDevice::OSOpen(GPortRef * /*pPortRef*/)
{
	int nResult = kResponse_Error;
	
	if (LockDevice(1) && IsOKToUse())
	{
		TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(GetOSData());
		
		// port may be NULL if initialization failed
		if (usbDevice == NULL)
			return kResponse_Error;
		
		int err = VST_OpenUSBPortForIO((VST_USBBulkDevice *)usbDevice);
		if (err == (int)kCFM_IOExclusiveAccess)
		{
			// If another app (including Classic) has the device we
			// can simply retry a few times.  The driver should be able to get a lock
			// on the device at some point.
			int nTry = 0;
			while ((err == (int)kCFM_IOExclusiveAccess) && (nTry < 4))
			{
				err = VST_OpenUSBPortForIO((VST_USBBulkDevice *)usbDevice);
				nTry++;					
			}
		}
		
		if (err == 0)
			nResult = kResponse_OK;
		else
		{
			cppsstream ssErr;
			ssErr << "GUSBDirectTempDevice:OSOpen -- Error: " << err;
			GSTD_LOG(ssErr.str());
		}
		
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
	
	return nResult;
}

int GSkipBaseDevice::OSClose(void)
{	
	if (LockDevice(1) && IsOKToUse())
	{
		TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(GetOSData());
		if ((usbDevice != NULL) && IsOpen())
			VST_CloseUSBPort((VST_USBBulkDevice *)usbDevice);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
	
	return kResponse_OK;
}

int GSkipBaseDevice::OSClearIO(void)
{
	int nResult = kResponse_Error;
	
	if (LockDevice(1) && IsOKToUse())
	{
		nResult = OSClearMeasurementPacketQueue();
		if (nResult != kResponse_Error)
			nResult = GSkipBaseDevice::OSClearCmdRespPacketQueue();
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	return nResult;
}

void GSkipBaseDevice::OSDestroy(void)
{
	if (LockDevice(1) && IsOKToUse())
	{
		TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(GetOSData());
		if (usbDevice != NULL)
		{
			VST_CloseUSBPort((VST_USBBulkDevice *)usbDevice);
			VST_ReleaseUSBBulkDevice((VST_USBBulkDevice *)usbDevice);
		}
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
}

int local_ReadPackets(GSkipBaseDevice *pDevice, void * pBuffer, int * pIONumPackets, int nBufferSizeInPackets, int nPipe)
{
	unsigned int nSkipPacketSize = sizeof(GSkipPacket);
	int nBufferSizeInBytes = nBufferSizeInPackets * nSkipPacketSize;
	int nRequestBytes = *pIONumPackets * nSkipPacketSize;
	int nResult = kResponse_Error;
	
	TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(pDevice->GetOSData());
	if (usbDevice != NULL)
	{
		UInt32 nIONumBytes = min(nRequestBytes, nBufferSizeInBytes);
		int err = VST_ReadBytes((VST_USBBulkDevice *)usbDevice, pBuffer, &nIONumBytes, (UInt32)2000, nPipe);
		
		if (err == (int)kCFM_IOUSBTransactionTimeout || err == (int)kCFM_IOReturnOverrun)
			err = noErr; // These are tolerated errors - revisit, & return overrun error?
		
		if (err == 0)
			nResult = kResponse_OK;
		
		*pIONumPackets = nIONumBytes / nSkipPacketSize;
		if ((nIONumBytes % nSkipPacketSize) != 0)
			GSTD_TRACE("GSkipBaseDevice: Read fractional number of packets");
	}
	
	return nResult;
}

int GSkipBaseDevice::OSReadMeasurementPackets(void * pBuffer, int * pIONumPackets, int nBufferSizeInPackets)
{
	int nReturn = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_ReadPackets(this, pBuffer, pIONumPackets, nBufferSizeInPackets, kMeasurementPipe);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	return nReturn;
}

int GSkipBaseDevice::OSReadCmdRespPackets(void * pBuffer, int * pIONumPackets, int nBufferSizeInPackets)
{
	int nReturn = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_ReadPackets(this, pBuffer, pIONumPackets, nBufferSizeInPackets, kCommandPipe);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	return nReturn;
}

int GSkipBaseDevice::OSWriteCmdPackets(void * pBuffer, int nNumPackets)
{
	int nResult = kResponse_Error;
	UInt32 nIONumBytes = nNumPackets * sizeof(GSkipPacket);
	
	if (LockDevice(1) && IsOKToUse())
	{
		TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(GetOSData());
		if (usbDevice != NULL)
		{
			int err = VST_WriteBytes((VST_USBBulkDevice *)usbDevice, pBuffer, nIONumBytes);
			if (err == 0)
				nResult = kResponse_OK;
		}
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
	
	return nResult;
}

int local_PacketsAvailable(GSkipBaseDevice *pDevice, int nPipe)
{
	int nResult = 0L;
	
	TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(pDevice->GetOSData());
	if (usbDevice != NULL)
		nResult = VST_BytesAvailable((VST_USBBulkDevice *)usbDevice, nPipe);
	
	return nResult / sizeof(GSkipPacket);
}


int local_NumLastMeasurements(GSkipBaseDevice *pDevice)
{
	int nResult = 0L;

	TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(pDevice->GetOSData());
	if (usbDevice != NULL)
		nResult = VST_GetNumberOfMeasurementsInLastPacket((VST_USBBulkDevice *)usbDevice);

	return nResult;
}

int GSkipBaseDevice::OSMeasurementPacketsAvailable(unsigned char *pNumMeasurementsInLastPacket)
{
	int nReturn = 0;
	
	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_PacketsAvailable(this, kMeasurementPipe);
		if (pNumMeasurementsInLastPacket != NULL)
			*pNumMeasurementsInLastPacket = local_NumLastMeasurements(this);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
	
	return nReturn;
}

int GSkipBaseDevice::OSCmdRespPacketsAvailable(void)
{
	int nReturn = 0;
	
	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_PacketsAvailable(this, kCommandPipe);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);
	
	return nReturn;
}

int local_ClearPacketQueue(GSkipBaseDevice *pDevice, int nPipe)
{
	TUSBBulkDevice usbDevice = static_cast<TUSBBulkDevice>(pDevice->GetOSData());
	if (usbDevice != NULL)
		VST_ClearInput((VST_USBBulkDevice *)usbDevice, nPipe);
	
	return kResponse_OK;
}
int GSkipBaseDevice::OSClearMeasurementPacketQueue()
{
	int nReturn = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_ClearPacketQueue(this, kMeasurementPipe);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	return nReturn;
}

int GSkipBaseDevice::OSClearCmdRespPacketQueue()
{
	int nReturn = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nReturn = local_ClearPacketQueue(this, kCommandPipe);
		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	return nReturn;
}
