#ifdef USE_LIB_USB

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
// GSkipBaseDevice_Linux_libusb.cpp

#import "GSkipBaseDevice.h"
#import "GTextUtils.h"
#import "GUtils.h"
#include <dirent.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "libusb-1.0/libusb.h"
extern libusb_context *pGoIO_libusbContext;   

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

struct LSkipPacketCircularBuffer
{
	LSkipPacketCircularBuffer(int numRecs);
	~LSkipPacketCircularBuffer();

	void SetQueueAccessMutex(OSMutex pQueueAccessMutex) {m_pQueueAccessMutex = pQueueAccessMutex;}
	void AddRec(GSkipPacket *pRec);
	bool RetrieveRec(GSkipPacket *pRec);
	int NumRecsAvailable();
	void Clear();

	OSMutex m_pQueueAccessMutex; //Not responsible for creation/destruction of this object.
	GSkipPacket *m_pRecs;
	int	m_nFirstRec;
	int m_nNextRec;
	int m_nRecsAllocated;
};

LSkipPacketCircularBuffer::LSkipPacketCircularBuffer(int numRecs)
{
	m_pRecs = new GSkipPacket[numRecs];
	m_pQueueAccessMutex = NULL;
	m_nRecsAllocated = numRecs;
	m_nFirstRec = 0;
	m_nNextRec = 0;
}

LSkipPacketCircularBuffer::~LSkipPacketCircularBuffer()
{
	delete [] m_pRecs;
}

void LSkipPacketCircularBuffer::AddRec(GSkipPacket *pRec)
{
	if (m_pQueueAccessMutex != NULL)
	{
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			if (NumRecsAvailable() == (m_nRecsAllocated - 1))
			{
				//Buffer is full, so advance first record index.
				//Note that even though space for m_nRecsAllocated recs exists, we only report available counts from 0 to (m_nRecsAllocated-1).
				m_nFirstRec++;
				if (m_nFirstRec == m_nRecsAllocated)
					m_nFirstRec = 0;
				if (1 == m_nFirstRec)
					GSTD_TRACE("LSkipPacketCircularBuffer measurement buffer overflowed.");
			}

			m_pRecs[m_nNextRec] = (*pRec);
			m_nNextRec++;
			if (m_nNextRec == m_nRecsAllocated)
				m_nNextRec = 0;

			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
	}
}

bool LSkipPacketCircularBuffer::RetrieveRec(GSkipPacket *pRec)
{
	bool bRecRetrieved = false;
	if (m_pQueueAccessMutex != NULL)
	{
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			if (NumRecsAvailable() > 0)
			{
				(*pRec) = m_pRecs[m_nFirstRec];
				m_nFirstRec++;
				if (m_nFirstRec == m_nRecsAllocated)
					m_nFirstRec = 0;
				bRecRetrieved = true;
			}
			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
	}
	return bRecRetrieved;
}

int LSkipPacketCircularBuffer::NumRecsAvailable()
{
	int numRecs = 0;
	if (m_pQueueAccessMutex != NULL)
	{
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			numRecs = m_nNextRec - m_nFirstRec;
			if (numRecs < 0)
				numRecs += m_nRecsAllocated;
			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
	}
	return numRecs;
}

void LSkipPacketCircularBuffer::Clear()
{
	if (m_pQueueAccessMutex != NULL)
	{
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			m_nFirstRec = 0;
			m_nNextRec = 0;
			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
	}
}


struct LSkipMgr
{
	LSkipMgr();
	~LSkipMgr();
	int Open(const cppstring &filename);
	int Close();
/*
	void AddMeasurementPacket(GSkipPacket *pRec);
	void AddCmdRespPacket(GSkipPacket *pRec);
	void WritePacket(GSkipPacket *pRec);
	*/
	static int	gListenForResponse(void *pParam);
	static int	gExitThread(void *pParam);
	static int	gStartThread(void *pParam);

	OSMutex 			m_pQueueAccessMutex;
	libusb_device_handle *m_hDeviceFile;
	GThread 			*m_pListeningThread;
	LSkipPacketCircularBuffer 	*m_pMesBuf;
	LSkipPacketCircularBuffer	*m_pCmdBuf;
	unsigned char m_lastNumMeasurementsInPacket;
	bool	m_stayAlive;	// this flag is true when opened, false when caller closes (so we can tell timeout from real close)
};

LSkipMgr::LSkipMgr()
{
	m_pQueueAccessMutex = NULL;
	m_pListeningThread = NULL;
	m_hDeviceFile = NULL;
	m_lastNumMeasurementsInPacket = 0;

	m_pMesBuf = new LSkipPacketCircularBuffer(2000);
	m_pCmdBuf = new LSkipPacketCircularBuffer(2000);
	m_stayAlive = false;
}

LSkipMgr::~LSkipMgr()
{
	if (NULL != m_hDeviceFile)
		Close();

	if (m_pMesBuf)
	{
		delete m_pMesBuf;
		m_pMesBuf = NULL;
	}
	if (m_pCmdBuf)
	{
		delete m_pCmdBuf;
		m_pCmdBuf = NULL;
	}
}

cppstring local_BusAndAddressToCPPStringw(uint8_t bus, 
                                         uint8_t address)
{ // RETURN a string based on the pointer
	char cPointer[12];
	sprintf(cPointer, "%u:%u", bus, address);
	cppstring sPointer = cPointer;

	return sPointer;
}

int LSkipMgr::Open(const cppstring &filename)
{
	int nResult = kResponse_OK;
	libusb_device **libusbDeviceList;
	int libusbNumDevices;            

	libusbNumDevices = libusb_get_device_list(pGoIO_libusbContext, &libusbDeviceList);

	m_pQueueAccessMutex = GThread::OSCreateMutex(GSTD_S(""));  

	if (m_pMesBuf && m_pCmdBuf && m_pQueueAccessMutex)
	{
		// Get pointer to dev again from string
		libusb_device *dev = NULL;
		for (int ix = 0; ix < libusbNumDevices; ix++) 
		{
			cppstring sDevice = local_BusAndAddressToCPPStringw(libusb_get_bus_number (libusbDeviceList[ix]),
                                                          libusb_get_device_address (libusbDeviceList[ix]));
			if (sDevice == filename)
			{
				dev = libusbDeviceList[ix];
				break;
			}
		}

		m_hDeviceFile = NULL;	
		if (NULL != dev)
		{
			int ret = libusb_open(dev, &m_hDeviceFile);
			if (0 != ret)
			{ // Couldn't open
				printf("failed libusb_open %s (error %d)\n", filename.c_str(), ret);
				m_hDeviceFile = NULL;	
			}
			else
			{ // Open succeeded, unload ldusb (or other kernel module) if present
				m_stayAlive = true;
				ret = libusb_kernel_driver_active(m_hDeviceFile, 0);
				if (0 != ret) 
				{
					printf("active kernel driver on device %s\n", filename.c_str());
					ret = libusb_detach_kernel_driver(m_hDeviceFile, 0);
					if (0 != ret) 
					{
						printf("unable to detach kernel driver for device %s (error %d)\n", filename.c_str(), ret);
						libusb_close(m_hDeviceFile);
						m_hDeviceFile = NULL;	
					}
				}

				if (0 == ret)
				{ // Unload went ok if needed, claim the interface
					ret = libusb_claim_interface(m_hDeviceFile, 0);
					if (0 != ret)
					{
						printf("Unable to claim interface for %s (error %d)\n", filename.c_str(), ret);
						libusb_attach_kernel_driver(m_hDeviceFile, 0);
						libusb_close(m_hDeviceFile);
						m_hDeviceFile = NULL;	
					}
				}
			}
		}

		if (NULL != m_hDeviceFile)
		{
			//First flush any old data from the pipe.
			int nNumberOfBytesRead = 0;
			int bytesReceived = 0;
			unsigned int startTimeMs = GUtils::OSGetTimeStamp();							
			do
			{
				unsigned char buf[8];
				//Reading 8 bytes at a time should get all the bytes out.
				//Asking for > 8 bytes might cause a timeout before all the bytes are out.
				int ret = -999;
				if (NULL != m_hDeviceFile)
					ret = libusb_interrupt_transfer(m_hDeviceFile, /*Endpoint:*/0x81, buf, sizeof(buf), &bytesReceived, /*Timeout:*/10);
				if ((0 != ret) && (ret != LIBUSB_ERROR_TIMEOUT))
				{
					bytesReceived = -1;
					printf("Failed to clear bytes (error %d)\n", ret);
				}
				if (bytesReceived > 0)
					nNumberOfBytesRead += bytesReceived;

				if ((GUtils::OSGetTimeStamp() - startTimeMs) > 3000)
					break;
			}
			while (bytesReceived > 0);

			if (nNumberOfBytesRead > 0)
			{
				printf("Flushed %d bytes when device (%p) opened.\n", nNumberOfBytesRead, m_hDeviceFile);
			}
		}
		else
			nResult = kResponse_Error;

		if (kResponse_OK == nResult)
		{	
			/*jentodo is it because the main thread loop is processing the reads and sleeping for 30ms that everything is so slow.*/
			m_pMesBuf->SetQueueAccessMutex(m_pQueueAccessMutex);
			m_pCmdBuf->SetQueueAccessMutex(m_pQueueAccessMutex);
			m_pListeningThread = new GThread(((StdThreadFunctionPtr)LSkipMgr::gListenForResponse),
						NULL, NULL, NULL, NULL, (void *) this, NULL, false);
			if (!m_pListeningThread)
				nResult = kResponse_Error;
			else
			if (!m_pListeningThread->OSStartThread())
				nResult = kResponse_Error;
		}
	}
	else
		nResult = kResponse_Error;

	libusb_free_device_list(libusbDeviceList, 1);

	return nResult;
}


int LSkipMgr::Close()
{
	m_stayAlive = false;
    	if (m_pListeningThread)
   	{
    		delete m_pListeningThread;
    		m_pListeningThread = NULL;
   	}

	if (m_pMesBuf)
		m_pMesBuf->SetQueueAccessMutex(NULL);

	if (m_pCmdBuf)
		m_pCmdBuf->SetQueueAccessMutex(NULL);

	if (m_pQueueAccessMutex)
	{
		GThread::OSDestroyMutex(m_pQueueAccessMutex);
		m_pQueueAccessMutex = NULL;
	}

	if (NULL != m_hDeviceFile)
	{
		libusb_attach_kernel_driver(m_hDeviceFile, 0);
		libusb_close(m_hDeviceFile);
		m_hDeviceFile = NULL;	
	}

	return kResponse_OK;
}

int LSkipMgr::gListenForResponse(void *pParam)
{
	int nResult = kResponse_OK;
	LSkipMgr *pMgr = (LSkipMgr *)pParam;

	if (NULL != pMgr)
	{
		while (NULL != pMgr->m_hDeviceFile)	
		{
			unsigned char buf[8];
			int bytesReceived;

			int ret = -999;

			if (NULL != pMgr->m_hDeviceFile)
				ret = libusb_interrupt_transfer(pMgr->m_hDeviceFile, /*Endpoint:*/0x81, buf, sizeof(buf), &bytesReceived, /*Timeout*/100); // can't use infinite timeout -- glib pthread_kill no longer signals

			if (0 == ret)
			{ // Success
				//Add packet to appropriate queue.
				if ((buf[0] & SKIP_MASK_INPUT_PACKET_TYPE))
				{
					if (NULL != pMgr->m_pCmdBuf)
						pMgr->m_pCmdBuf->AddRec((GSkipPacket *) (&buf[0]));
				}
				else
				if (NULL != pMgr->m_pMesBuf)
				{
					pMgr->m_pMesBuf->AddRec((GSkipPacket *) (&buf[0]));
					GSkipMeasurementPacket *pMeasRec = (GSkipMeasurementPacket *) (&buf[0]);
					pMgr->m_lastNumMeasurementsInPacket = pMeasRec->nMeasurementsInPacket;
				}
			}
			else
			{ // Error
				if (!pMgr->m_stayAlive || (LIBUSB_ERROR_TIMEOUT != ret))
				{ // timeouts are handled here -- just keep looping and reading
					if (LIBUSB_ERROR_TIMEOUT != ret)
						printf("Error (%d): Reading from %p\n", ret, pMgr->m_hDeviceFile);
					break;
				}
			}
		}
	}

	return nResult;
}

/*

int LSkipMgr::gExitThread(void *pParam)
{
	int nResult = kResponse_OK;
	pthread_exit(NULL);
	return nResult;
}

int LSkipMgr::gStartThread(void *pParam)
{
	return kResponse_OK;
}

*/
bool GSkipBaseDevice::OSInitialize(void)
{
	bool bResult = true;
	m_pOSData = (OSPtr) new LSkipMgr();
	return bResult;
}

StringVector GSkipBaseDevice::OSGetAvailableDevicesOfType(
	int nVendorID, 	// Vendor ID (e.g. VERNIER_DEFAULT_VENDOR_ID)
	int nProductID)	// Product ID (e.g. USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID)
{
	StringVector vPortNames;
	libusb_device **libusbDeviceList;
	int libusbNumDevices;            

	libusbNumDevices = libusb_get_device_list(pGoIO_libusbContext, &libusbDeviceList);
 
	for (int ix = 0; ix < libusbNumDevices; ix++) 
	{
		libusb_device *dev = libusbDeviceList[ix];

		//get the device descriptor (to get the vid/pid)
		struct libusb_device_descriptor devDesc;
		int ret = libusb_get_device_descriptor(dev, &devDesc);
		if (0 == ret)
		{ // Got a device, check its vid/pid
			if ((devDesc.idVendor == nVendorID) && 
				(devDesc.idProduct == nProductID)) 
			{ // Match!  Store pointer as string so we can get it again when opening this device
				vPortNames.push_back(local_BusAndAddressToCPPStringw(libusb_get_bus_number (dev),
                                                             libusb_get_device_address (dev)));
			}
		}
		else
		{ // Something screwed up
			fprintf(stderr, "%s:%s:%d unable to get device descriptor for device %d (error %d)\n",__FILE__, __FUNCTION__, __LINE__, ix, ret);
		}
	}
	
	libusb_free_device_list(libusbDeviceList, 1);

	return vPortNames;
}

int GSkipBaseDevice::OSOpen(GPortRef *pPortRef)
{
	int nResult = kResponse_Error;

	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			((LSkipMgr*)m_pOSData)->Open(pPortRef->GetLocation());
			nResult = kResponse_OK;
			UnlockDevice();
		}
	}

	return nResult;
}

int GSkipBaseDevice::OSClose(void)
{
	int nResult = kResponse_Error;

	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			((LSkipMgr*)m_pOSData)->Close();
			nResult = kResponse_OK;
			UnlockDevice();
		}
	}

	return nResult;
}

void GSkipBaseDevice::OSDestroy(void)
{
	if (m_pOSData)
		delete ((LSkipMgr *) m_pOSData);
}

int GSkipBaseDevice::OSReadMeasurementPackets(
	void * pBuffer, //[out] ptr to destination buffer
	int * pIONumPackets, //[in, out] number of packets desired on input, number of packets read on output
	int nBufferSizeInPackets) //[in] size of destination buffer in packets
{
	int nResult = kResponse_Error;
	int nPacketsRead = 0;
	if (NULL != m_pOSData)
	{
		LSkipMgr *pSkipMgr = (LSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		int nPacketsDesired = (*pIONumPackets);
		if (nPacketsDesired > nBufferSizeInPackets)
			nPacketsDesired = nBufferSizeInPackets;

		if (LockDevice(1) && IsOKToUse())
		{
			nResult = kResponse_OK;
			while (nPacketsRead < nPacketsDesired)
			{
				if (pSkipMgr->m_pMesBuf->RetrieveRec((GSkipPacket *) pBuf))
				{
					nPacketsRead++;
					pBuf += sizeof(GSkipPacket);
				}
				else
					break;
			}

			UnlockDevice();
		}
	}

	(*pIONumPackets) = nPacketsRead;
	return nResult;
}

int GSkipBaseDevice::OSReadCmdRespPackets(
	void * pBuffer, //[out] ptr to destination buffer
	int * pIONumPackets, //[in, out] number of packets desired on input, number of packets read on output
	int nBufferSizeInPackets) //[in] size of destination buffer in packets
{
	int nResult = kResponse_Error;
	int nPacketsRead = 0;
	if (NULL != m_pOSData)
	{
		LSkipMgr *pSkipMgr = (LSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		int nPacketsDesired = (*pIONumPackets);
		if (nPacketsDesired > nBufferSizeInPackets)
			nPacketsDesired = nBufferSizeInPackets;

		if (LockDevice(1) && IsOKToUse())
		{
			nResult = kResponse_OK;
			while (nPacketsRead < nPacketsDesired)
			{
				if (pSkipMgr->m_pCmdBuf->RetrieveRec((GSkipPacket *) pBuf))
				{
					nPacketsRead++;
					pBuf += sizeof(GSkipPacket);
				}
				else
					break;
			}

			UnlockDevice();
		}
	}

	(*pIONumPackets) = nPacketsRead;
	return nResult;
}

int GSkipBaseDevice::OSWriteCmdPackets(
	void * pBuffer, 	// pointer to command packet
	int nNumPackets)	// Unused? TODO (gupshaw): Check with Elliot
{
	LSkipMgr *pMgr = (LSkipMgr *)m_pOSData;

	int nResult = kResponse_Error;

	if (NULL != pMgr)
	{
		unsigned char buf[8];	// make sure we always write 8 bytes (backfill with 0's)
		memset(buf, 0, 8);
		GSkipPacket *pkt = (GSkipPacket*)pBuffer;

		memcpy(buf, (unsigned char*)pBuffer, sizeof(pkt));

		if ((NULL != pMgr->m_hDeviceFile) && LockDevice(1) && IsOKToUse())
		{
			// Leave timeout at a (long!) 3s because under vmWare, 1s was NOT enough
			int bytesSent = libusb_control_transfer(/*Device handle*/pMgr->m_hDeviceFile, /*bmRequestType*/(LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT), 
													/*bRequest*/0x09, /*wValue*/(1 << 8), /*wIndex*/0, /*data*/(unsigned char*)buf, 
													/*size of data*/sizeof(buf), /*Timeout*/3000);
			if (bytesSent == sizeof(buf))
			{ // Success
				nResult = kResponse_OK;
			}
			else
			{ // Error
				printf("Error (%d): Failed to write %d bytes to %p\n", bytesSent, sizeof(buf), pMgr->m_hDeviceFile);
			}
			UnlockDevice();
		}
	}

	return nResult;
}

int GSkipBaseDevice::OSMeasurementPacketsAvailable(unsigned char *pNumMeasurementsInLastPacket)
{
	int nReturn = 0;

	(*pNumMeasurementsInLastPacket) = 1;

	if (m_pOSData && LockDevice(1) && IsOKToUse())
	{
		(*pNumMeasurementsInLastPacket) = ((LSkipMgr*)m_pOSData)->m_lastNumMeasurementsInPacket;
		if (((LSkipMgr*)m_pOSData)->m_pMesBuf)
			nReturn = ((LSkipMgr*)m_pOSData)->m_pMesBuf->NumRecsAvailable();

		UnlockDevice ();
	}

	return nReturn;
}

int GSkipBaseDevice::OSCmdRespPacketsAvailable(void)
{
	int nReturn = 0;

	if (m_pOSData && LockDevice(1) && IsOKToUse())
	{
		if (((LSkipMgr*)m_pOSData)->m_pCmdBuf)
			nReturn = ((LSkipMgr*)m_pOSData)->m_pCmdBuf->NumRecsAvailable();
		UnlockDevice ();
	}
	return nReturn;
}

int GSkipBaseDevice::OSClearMeasurementPacketQueue()
{
	int nResult = kResponse_Error;

	if (m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			((LSkipMgr*)m_pOSData)->m_pMesBuf->Clear();
			((LSkipMgr*)m_pOSData)->m_lastNumMeasurementsInPacket = 0;
			nResult = kResponse_OK;
			UnlockDevice();
		}
	}

	return nResult;
}

int GSkipBaseDevice::OSClearCmdRespPacketQueue()
{
	int nResult = kResponse_Error;

	if (m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			((LSkipMgr*)m_pOSData)->m_pCmdBuf->Clear();
			nResult = kResponse_OK;
			UnlockDevice();
		}
	}

	return nResult;
}

int GSkipBaseDevice::OSClearIO(void)
{
	int nResult = kResponse_OK;

	OSClearMeasurementPacketQueue();
	OSClearCmdRespPacketQueue();

	return nResult;
}

#ifdef LIB_NAMESPACE
}
#endif

#endif //ifdef USE_LIB_USB
