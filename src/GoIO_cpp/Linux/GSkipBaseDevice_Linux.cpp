// GSkipBaseDevice_Linux.cpp

#import "GSkipBaseDevice.h"
#import "GTextUtils.h"
#import "GUtils.h"
#include <dirent.h>
#include <poll.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

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
	long Open(const cppstring &filename);
	long Close();
/*
	void AddMeasurementPacket(GSkipPacket *pRec);
	void AddCmdRespPacket(GSkipPacket *pRec);
	void WritePacket(GSkipPacket *pRec);
	*/
	static long	gListenForResponse(void *pParam);
	static long	gExitThread(void *pParam);
	static long	gStartThread(void *pParam);

	OSMutex 			m_pQueueAccessMutex;
	int 				m_hDeviceID;
	GThread 			*m_pListeningThread;
	LSkipPacketCircularBuffer 	*m_pMesBuf;
	LSkipPacketCircularBuffer	*m_pCmdBuf;
	unsigned char m_lastNumMeasurementsInPacket;
};

LSkipMgr::LSkipMgr()
{
	m_pQueueAccessMutex = NULL;
	m_pListeningThread = NULL;
	m_hDeviceID = -1;
	m_lastNumMeasurementsInPacket = 0;

	m_pMesBuf = new LSkipPacketCircularBuffer(2000);
	m_pCmdBuf = new LSkipPacketCircularBuffer(2000);
}

LSkipMgr::~LSkipMgr()
{
	if (m_hDeviceID != -1)
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

long LSkipMgr::Open(const cppstring &filename)
{
	long nResult = kResponse_OK;

	m_pQueueAccessMutex = GThread::OSCreateMutex(GSTD_S(""));  

	if (m_pMesBuf && m_pCmdBuf && m_pQueueAccessMutex)
	{

		m_hDeviceID = open(filename.c_str(), O_RDWR|O_EXCL);
		if (m_hDeviceID != -1)
		{
			//spam
			//Flush the input buffers - it would be nice if ClearIO() did this.
			int numBytesRead = 0;
			int numNewBytesRead = 0;
			unsigned char buf[8000];
			struct pollfd fds[1];
			int timeout_msecs = 5;

			fds[0].fd = m_hDeviceID;
			fds[0].events = POLLIN | POLLPRI;

			while (poll(fds, 1, timeout_msecs)>0)
			{
				numNewBytesRead = read(fds[0].fd,&buf,sizeof(buf));
				numBytesRead += numNewBytesRead;
			}
			printf("Go input queue contained %d bytes when device opened.\n:", numBytesRead);
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

	return nResult;
}


long LSkipMgr::Close()
{
	long nResult = kResponse_Error;

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

	close(m_hDeviceID);
	m_hDeviceID=-1;
	nResult = kResponse_OK;

	return nResult;
}

long LSkipMgr::gListenForResponse(void *pParam)
{
	long nResult = kResponse_OK;
	LSkipMgr *pMgr = (LSkipMgr *)pParam;

	if (pMgr)
	{
		unsigned char buf[8];  //jenhack
		struct pollfd fds[1];
		int timeout_msecs = 5;
		int count = 0;

		/* Open STREAMS device. */
		fds[0].fd = pMgr->m_hDeviceID;
		fds[0].events = POLLIN | POLLPRI;

		while ((poll(fds, 1, timeout_msecs)>0) && (count < 20))
		{
		/*Jentodo - do we need to sync with the writes before doing this.  I don't think so but I want to check*/
			int nNumberOfBytesRead = read (fds[0].fd,&buf,sizeof(buf));

			if(nNumberOfBytesRead==sizeof(buf))
			{
				//Add packet to appropriate queue.
				if ((buf[0] & SKIP_MASK_INPUT_PACKET_TYPE))
				{
					if (pMgr->m_pCmdBuf)
						pMgr->m_pCmdBuf->AddRec((GSkipPacket *) (&buf[0]));
				}
				else
				{
					if (pMgr->m_pMesBuf)
					{
						pMgr->m_pMesBuf->AddRec((GSkipPacket *) (&buf[0]));
						GSkipMeasurementPacket *pMeasRec = (GSkipMeasurementPacket *) (&buf[0]);
						pMgr->m_lastNumMeasurementsInPacket = pMeasRec->nMeasurementsInPacket;
					}
				}
			}
			else
			{
				printf("Bad we did not get all the bytes. Dropped %d bytes.\n",nNumberOfBytesRead);
			}
			count++;
		}
	}
	return nResult;
}

/*

long LSkipMgr::gExitThread(void *pParam)
{
	long nResult = kResponse_OK;
	pthread_exit(NULL);
	return nResult;
}

long LSkipMgr::gStartThread(void *pParam)
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

StringVector GSkipBaseDevice::OSGetAvailableDevicesOfType(int nVendorID, int nProductID)
{
	StringVector vPortNames;
	cppstring sBaseDir = GSTD_S("/sys/class/usb");
	DIR *directory = opendir (sBaseDir.c_str());
	struct dirent *entry = (struct dirent *)0;

	if (directory)
	{
		while ((entry = readdir (directory)))
		{	//Look for LD devices.
			if(strncmp(entry->d_name,"ldusb",5)==0)
			{
				cppstring sFile = sBaseDir;
				sFile.append("/");
				sFile.append(entry->d_name);
				sFile.append("/device/../idVendor");
				FILE *pVendorID = fopen(sFile.c_str(),"r");
				if (pVendorID)
				{
					char vendorID[100];
					int bytesread = fread (vendorID,sizeof(char),100,pVendorID);
					long rFoundVendorID = strtol(vendorID,NULL,16);
					if (bytesread && rFoundVendorID == nVendorID)
					{//Now that we have found vendor go find product.
						sFile = sBaseDir;
						sFile.append("/");
						sFile.append(entry->d_name);
						sFile.append("/device/../idProduct");

						FILE *pProductID = fopen(sFile.c_str(),"r");
						char productID[100];
						if (pProductID)
						{
							fread(productID,sizeof(char),100,pProductID);
							long rFoundProdID = strtol(productID,NULL,16);
							if (rFoundProdID == nProductID)
							{//We found a device.
								cppstring  sDevice = GSTD_S("/dev/");
								sDevice.append(entry->d_name);
								vPortNames.push_back(sDevice);
							}
							fclose(pProductID);
						}
					}
					fclose(pVendorID);
				}
			}
		}
		closedir(directory);
	}
	return vPortNames;
}

long GSkipBaseDevice::OSOpen(GPortRef *pPortRef)
{
	long nResult = kResponse_Error;

	if (m_pOSData)
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

long GSkipBaseDevice::OSClose(void)
{
	long nResult = kResponse_Error;

	if (m_pOSData)
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

long GSkipBaseDevice::OSReadMeasurementPackets(
	void * pBuffer, //[out] ptr to destination buffer
	long * pIONumPackets, //[in, out] number of packets desired on input, number of packets read on output
	long nBufferSizeInPackets) //[in] size of destination buffer in packets
{
	long nResult = kResponse_Error;
	long nPacketsRead = 0;
	if (NULL != m_pOSData)
	{
		LSkipMgr *pSkipMgr = (LSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		long nPacketsDesired = (*pIONumPackets);
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

long GSkipBaseDevice::OSReadCmdRespPackets(
	void * pBuffer, //[out] ptr to destination buffer
	long * pIONumPackets, //[in, out] number of packets desired on input, number of packets read on output
	long nBufferSizeInPackets) //[in] size of destination buffer in packets
{
	long nResult = kResponse_Error;
	long nPacketsRead = 0;
	if (NULL != m_pOSData)
	{
		LSkipMgr *pSkipMgr = (LSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		long nPacketsDesired = (*pIONumPackets);
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

long GSkipBaseDevice::OSWriteCmdPackets(void * pBuffer, long nNumPackets)
{
	long nResult = kResponse_Error;

	if (m_pOSData)
	{
		GSkipPacket *pkt = (GSkipPacket*)pBuffer;

		if (LockDevice(1) && IsOKToUse())
		{
			write (((LSkipMgr*)m_pOSData)->m_hDeviceID, pkt, sizeof(*pkt));
			nResult = kResponse_OK;
			UnlockDevice();
		}
	}
	return nResult;
}

long GSkipBaseDevice::OSMeasurementPacketsAvailable(unsigned char *pNumMeasurementsInLastPacket)
{
	long nReturn = 0;

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

long GSkipBaseDevice::OSCmdRespPacketsAvailable(void)
{
	long nReturn = 0;

	if (m_pOSData && LockDevice(1) && IsOKToUse())
	{
		if (((LSkipMgr*)m_pOSData)->m_pCmdBuf)
			nReturn = ((LSkipMgr*)m_pOSData)->m_pCmdBuf->NumRecsAvailable();
		UnlockDevice ();
	}
	return nReturn;
}

long GSkipBaseDevice::OSClearMeasurementPacketQueue()
{
	long nResult = kResponse_Error;

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

long GSkipBaseDevice::OSClearCmdRespPacketQueue()
{
	long nResult = kResponse_Error;

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

long GSkipBaseDevice::OSClearIO(void)
{
	long nResult = kResponse_OK;

	OSClearMeasurementPacketQueue();
	OSClearCmdRespPacketQueue();

	return nResult;
}

