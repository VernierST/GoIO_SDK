// GSkipBaseDevice.cpp

#include "stdafx.h"

#include "GSkipBaseDevice.h"

#include "GUtils.h"
#include "GTextUtils.h"
#include "WinEnumDevices.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BYTES_IN_MICROSOFT_HID_PACKET 9
#define FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET 1

#define NUM_PACKETS_IN_CMD_RESP_CIRCULAR_BUFFER 1000
#define NUM_PACKETS_IN_MEASUREMENTS_CIRCULAR_BUFFER 2000
#define WRITE_TIMEOUT_MS 1000
#define DESIRED_NUM_HID_BUFFERS 128

#define WIN9X_OPEN_SKIP_DEVICE_MAX_ATTEMPT_COUNT 3
#define WIN9X_OPEN_SKIP_DEVICE_RETRY_PAUSE_MS 1000

const cppstring kSkipPacketQueueMutexName = "_skipQ_mutex_";

static bool bSharingViolationWarningDisplayed = false;


struct CWinSkipPacketCircularBuffer
{
	CWinSkipPacketCircularBuffer(int numRecs);
	~CWinSkipPacketCircularBuffer();

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

struct CWinSkipMgr
{
	CWinSkipMgr();
	~CWinSkipMgr();
	int Open(const cppstring &filename);
	int Close();

	void AddMeasurementPacket(GSkipPacket *pRec);
	void AddCmdRespPacket(GSkipPacket *pRec);

	cppstring m_filename;
	OSMutex m_pQueueAccessMutex;
	HANDLE m_hPollingThread;
	HANDLE m_hPollingThreadExitSignal;
	CWinSkipPacketCircularBuffer *m_pMeasurementPacketBuffer;
	CWinSkipPacketCircularBuffer *m_pCmdRespPacketBuffer;
	HANDLE m_hHidDeviceFile;
	HANDLE m_hOverlappedWriteEvent;
	unsigned char m_lastNumMeasurementsInPacket;
	unsigned char m_lastMeasurementRollingCounter;	//diagnostic
	unsigned char m_bRollingCounterInterrupted;		//diagnostic
	unsigned int m_startMeasurementTimeMs;			//diagnostic
	unsigned int m_lastMeasurementTimeMs;			//diagnostic
	unsigned int m_totalMeasurementsCount;			//diagnostic
	unsigned int m_maxDeltaTimeMs;
	GSkipBaseDevice *m_pDevice;
};

CWinSkipPacketCircularBuffer::CWinSkipPacketCircularBuffer(int numRecs)
{
	m_pRecs = new GSkipPacket[numRecs];
	m_pQueueAccessMutex = NULL;
	m_nRecsAllocated = numRecs;
	m_nFirstRec = 0;
	m_nNextRec = 0;

//spam	GSTD_NEW(m_pRecs, GSkipPacket *, GSkipPacket[numRecs]);
}

CWinSkipPacketCircularBuffer::~CWinSkipPacketCircularBuffer()
{
	delete [] m_pRecs;
}

void CWinSkipPacketCircularBuffer::AddRec(GSkipPacket *pRec)
{
	if (m_pQueueAccessMutex != NULL)
	{
		int oldPriority = GetThreadPriority(GetCurrentThread());
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
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
					GSTD_TRACE("CWinSkipPacketCircularBuffer measurement buffer overflowed.");
			}

			m_pRecs[m_nNextRec] = (*pRec);
			m_nNextRec++;
			if (m_nNextRec == m_nRecsAllocated)
				m_nNextRec = 0;

			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
		SetThreadPriority(GetCurrentThread(), oldPriority);
	}
}

bool CWinSkipPacketCircularBuffer::RetrieveRec(GSkipPacket *pRec)
{
	bool bRecRetrieved = false;
	if (m_pQueueAccessMutex != NULL)
	{
		int oldPriority = GetThreadPriority(GetCurrentThread());
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
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
		SetThreadPriority(GetCurrentThread(), oldPriority);
	}

	return bRecRetrieved;
}

int CWinSkipPacketCircularBuffer::NumRecsAvailable()
{
	int numRecs = 0;
	if (m_pQueueAccessMutex != NULL)
	{
		int oldPriority = GetThreadPriority(GetCurrentThread());
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			numRecs = m_nNextRec - m_nFirstRec;
			if (numRecs < 0)
				numRecs += m_nRecsAllocated;

			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
		SetThreadPriority(GetCurrentThread(), oldPriority);
	}

	return numRecs;
}
	
void CWinSkipPacketCircularBuffer::Clear()
{
	if (m_pQueueAccessMutex != NULL)
	{
		int oldPriority = GetThreadPriority(GetCurrentThread());
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		if (GThread::OSLockMutex(m_pQueueAccessMutex))
		{
			m_nFirstRec = 0;
			m_nNextRec = 0;

			GThread::OSUnlockMutex(m_pQueueAccessMutex);
		}
		SetThreadPriority(GetCurrentThread(), oldPriority);
	}
}

DWORD WINAPI InterruptPipeListenCallback(void * pParam)
{
	bool exitFlag = false;
	CWinSkipMgr *pSkipMgr = static_cast<CWinSkipMgr *>(pParam);
	OVERLAPPED HIDOverlapped;
	unsigned char buf[BYTES_IN_MICROSOFT_HID_PACKET];
	DWORD nNumberOfBytesRead;
	DWORD nLastError;
	HANDLE eventArray[2];

	HIDOverlapped.Offset = 0;
	HIDOverlapped.OffsetHigh = 0;
	HIDOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == HIDOverlapped.hEvent)
		exitFlag = true;
	else
	{
		eventArray[0] = HIDOverlapped.hEvent;
		eventArray[1] = pSkipMgr->m_hPollingThreadExitSignal;
	}

	while (!exitFlag)
	{
		buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET] = 0;//For debugging.
		ResetEvent(HIDOverlapped.hEvent);

		if (ReadFile(pSkipMgr->m_hHidDeviceFile, buf, sizeof(buf), &nNumberOfBytesRead, &HIDOverlapped))
		{
			if (pSkipMgr->m_pDevice->GetDiagnosticInputBufferPtr())
				pSkipMgr->m_pDevice->GetDiagnosticInputBufferPtr()->AddBytes(
					&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET], sizeof(GSkipPacket));

			//Add packet to appropriate queue.
			if (buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET] & SKIP_MASK_INPUT_PACKET_TYPE)
				pSkipMgr->AddCmdRespPacket((GSkipPacket *) (&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET]));
			else
				pSkipMgr->AddMeasurementPacket((GSkipPacket *) (&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET]));

			if (WAIT_OBJECT_0 == WaitForSingleObject(pSkipMgr->m_hPollingThreadExitSignal, 0))
				exitFlag = true;				//Parent is shutting us down.
		}
		else
		{
			nLastError = GetLastError();
			if (nLastError != ERROR_IO_PENDING)
			{
				exitFlag = true;				//Something really went wrong!!!!
				CancelIo(pSkipMgr->m_hHidDeviceFile);
			}
			else
			{
				//Wait for the read operation to complete.
				DWORD status = WaitForMultipleObjects(2, eventArray, FALSE, INFINITE);
				switch (status)
				{
					case WAIT_OBJECT_0:
					{
						//Read operation has completed.
						//See if it was successful.
						if (GetOverlappedResult(pSkipMgr->m_hHidDeviceFile, &HIDOverlapped, &nNumberOfBytesRead, false))
						{
							if (pSkipMgr->m_pDevice->GetDiagnosticInputBufferPtr())
								pSkipMgr->m_pDevice->GetDiagnosticInputBufferPtr()->AddBytes(
									&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET], sizeof(GSkipPacket));

							//Add packet to appropriate queue.
							if (buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET] & SKIP_MASK_INPUT_PACKET_TYPE)
								pSkipMgr->AddCmdRespPacket(
									(GSkipPacket *) (&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET]));
							else
								pSkipMgr->AddMeasurementPacket(
									(GSkipPacket *) (&buf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET]));
						}
						else
						{
							exitFlag = true; //Something unexpected.
							CancelIo(pSkipMgr->m_hHidDeviceFile);//Just in case.
						}
						break;
					}
					case (WAIT_OBJECT_0 + 1):
						exitFlag = true; //pSkipMgr->m_hPollingThreadExitSignal is set - parent is shutting us down.
						CancelIo(pSkipMgr->m_hHidDeviceFile);
						break;
					default:
						exitFlag = true; //Something unexpected.
						CancelIo(pSkipMgr->m_hHidDeviceFile);
						break;
				}
			}
		}
	}

	if (NULL != HIDOverlapped.hEvent)
		CloseHandle(HIDOverlapped.hEvent);
	
	return 0;
}
	
CWinSkipMgr::CWinSkipMgr()
{
	m_pQueueAccessMutex = NULL;
	m_hPollingThread = NULL;
	m_hPollingThreadExitSignal = NULL;
	m_hHidDeviceFile = INVALID_HANDLE_VALUE;
	m_hOverlappedWriteEvent = NULL;
	m_pMeasurementPacketBuffer = new CWinSkipPacketCircularBuffer(NUM_PACKETS_IN_MEASUREMENTS_CIRCULAR_BUFFER);
	m_pCmdRespPacketBuffer = new CWinSkipPacketCircularBuffer(NUM_PACKETS_IN_CMD_RESP_CIRCULAR_BUFFER);
	m_lastNumMeasurementsInPacket = 0;
	m_maxDeltaTimeMs = 0;
	m_lastMeasurementRollingCounter = 0;
	m_bRollingCounterInterrupted = 1;
	m_startMeasurementTimeMs = 0;
	m_lastMeasurementTimeMs = 0;
	m_totalMeasurementsCount = 0;
	m_pDevice = NULL;
}

CWinSkipMgr::~CWinSkipMgr()
{
	if (INVALID_HANDLE_VALUE != m_hHidDeviceFile)
		Close();

	if (m_pMeasurementPacketBuffer)
		delete m_pMeasurementPacketBuffer;
	m_pMeasurementPacketBuffer = NULL;

	if (m_pCmdRespPacketBuffer)
		delete m_pCmdRespPacketBuffer;
	m_pCmdRespPacketBuffer = NULL;
}

int CWinSkipMgr::Open(const cppstring &filename)
{
	int nResult = kResponse_OK;
	BOOL bStatus ;

	if (INVALID_HANDLE_VALUE != m_hHidDeviceFile)
		Close();

	m_filename = filename;

	m_hHidDeviceFile = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 
		NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (INVALID_HANDLE_VALUE == m_hHidDeviceFile)
	{
		m_hHidDeviceFile = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
		if (INVALID_HANDLE_VALUE == m_hHidDeviceFile)
			nResult = kResponse_Error;
		else
		if (!bSharingViolationWarningDisplayed)
		{
			bSharingViolationWarningDisplayed = true;
//			GUtils::MessageBox(GSTD_STRING(IDSX_SKIP_EXCLUSIVE_ACCESS_FAILURE_WARNING));
			GUtils::MessageBox("Cannot obtain exclusive access to a Go! device.");
		}
	}

	if (kResponse_OK == nResult)
	{
		cppsstream ss;
		unsigned long numHidBuffers;
		GSTD_ASSERT(hWinHidDLibrary != NULL);
		dynHidD_GetNumInputBuffers(m_hHidDeviceFile, &numHidBuffers);
		ss << "Num Hid Buffers before HidD_SetNumInputBuffers() = " << numHidBuffers;
		GSTD_TRACE(ss.str());
		bStatus = dynHidD_SetNumInputBuffers(m_hHidDeviceFile, DESIRED_NUM_HID_BUFFERS);
		dynHidD_GetNumInputBuffers(m_hHidDeviceFile, &numHidBuffers);
		ss.str(GSTD_S(""));
		ss << "Num Hid Buffers after HidD_SetNumInputBuffers() = " << numHidBuffers;
		GSTD_TRACE(ss.str());

		m_pQueueAccessMutex = GThread::OSCreateMutex(kSkipPacketQueueMutexName + GTextUtils::StringReplace(filename, GSTD_S("\\"), GSTD_S("")));
		if (NULL == m_pQueueAccessMutex)
			nResult = kResponse_Error;
	}

	if (kResponse_OK == nResult)
	{
		m_pMeasurementPacketBuffer->SetQueueAccessMutex(m_pQueueAccessMutex);
		m_pCmdRespPacketBuffer->SetQueueAccessMutex(m_pQueueAccessMutex);
		m_hPollingThreadExitSignal = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == m_hPollingThreadExitSignal)
			nResult = kResponse_Error;
	}

	if (kResponse_OK == nResult)
	{
		m_hOverlappedWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (NULL == m_hOverlappedWriteEvent)
			nResult = kResponse_Error;
	}

	if (kResponse_OK == nResult)
	{
		DWORD threadId;
		m_hPollingThread = CreateThread(NULL, 0x1000, InterruptPipeListenCallback, this, 0, &threadId);
		if (NULL == m_hPollingThread)
			nResult = kResponse_Error;
		else
			SetThreadPriority(m_hPollingThread, THREAD_PRIORITY_TIME_CRITICAL);
//spam		SetThreadPriority(m_hPollingThread, THREAD_PRIORITY_HIGHEST);//THREAD_PRIORITY_TIME_CRITICAL not necessary
																			//because HID driver does some buffering.
	}

	if (kResponse_OK != nResult)
		Close();

	return nResult;
}

int CWinSkipMgr::Close()
{
	int nResult = kResponse_OK;

	if (NULL != m_hPollingThread)
	{
		//Signal the exit thread event.
		SetEvent(m_hPollingThreadExitSignal); //This will cause the polling thread to exit.
	    WaitForSingleObject(m_hPollingThread, INFINITE);//Thread becomes signalled upon exit.
		CloseHandle(m_hPollingThread);
		m_hPollingThread = NULL;
	}

	if (NULL != m_hOverlappedWriteEvent)
	{
		CloseHandle(m_hOverlappedWriteEvent);
		m_hOverlappedWriteEvent = NULL;
	}

	if (NULL != m_hPollingThreadExitSignal)
	{
		CloseHandle(m_hPollingThreadExitSignal);
		m_hPollingThreadExitSignal = NULL;
	}

	m_pMeasurementPacketBuffer->SetQueueAccessMutex(NULL);
	m_pCmdRespPacketBuffer->SetQueueAccessMutex(NULL);

	if (NULL != m_pQueueAccessMutex)
	{
		GThread::OSDestroyMutex(m_pQueueAccessMutex);
		m_pQueueAccessMutex = NULL;
	}

	if (INVALID_HANDLE_VALUE != m_hHidDeviceFile)
	{
		CloseHandle(m_hHidDeviceFile);
		m_hHidDeviceFile = INVALID_HANDLE_VALUE;
	}
	
	m_filename = GSTD_S("");
	
	return nResult;
}

void CWinSkipMgr::AddMeasurementPacket(GSkipPacket *pRec)
{
/*
	cppsstream ss;
	ss << "Input meas packet: " << hex << ((unsigned short) pRec->data[0]) << "h ";
	ss << ((unsigned short) pRec->data[1]) << "h ";
	ss << ((unsigned short) pRec->data[2]) << "h ";
	ss << ((unsigned short) pRec->data[3]) << "h ";
	ss << ((unsigned short) pRec->data[4]) << "h ";
	ss << ((unsigned short) pRec->data[5]) << "h ";
	ss << ((unsigned short) pRec->data[6]) << "h ";
	ss << ((unsigned short) pRec->data[7]) << "h ";
	GSTD_TRACE(ss.str());
*/
	m_pMeasurementPacketBuffer->AddRec(pRec);

	GSkipMeasurementPacket *pMeasRec = (GSkipMeasurementPacket *) pRec;
	if (0 == m_bRollingCounterInterrupted)
	{
		unsigned char testCounter = m_lastMeasurementRollingCounter + m_lastNumMeasurementsInPacket;
		if (testCounter != pMeasRec->nRollingCounter)
		{
			cppsstream ss;
			ss << endl << "Skip measurement rolling counter error - expected " << ((unsigned short) testCounter) << ", got " << ((unsigned short) pMeasRec->nRollingCounter) << endl;
			GSTD_TRACE(ss.str());
			ss.str(GSTD_S(""));
			ss << "Max delta time(ms) between AddMeasurementPacket() calls is " << m_maxDeltaTimeMs;
			GSTD_TRACE(ss.str());
		}
		m_totalMeasurementsCount += pMeasRec->nMeasurementsInPacket;
		if (m_totalMeasurementsCount > 30000)
		{
			cppsstream ss2;
			m_lastMeasurementTimeMs = GUtils::OSGetTimeStamp();
			unsigned int bigDeltaMs = m_lastMeasurementTimeMs - m_startMeasurementTimeMs;
			double freq = m_totalMeasurementsCount;
			freq = (freq*1000.0)/bigDeltaMs;
			ss2 << "Measurement frequency for the last 30000 measurements is " << freq << " hz.";
			GSTD_TRACE(ss2.str());
			m_startMeasurementTimeMs = m_lastMeasurementTimeMs;
			m_totalMeasurementsCount = 0;
		}

		unsigned int testMs = GUtils::OSGetTimeStamp() - m_lastMeasurementTimeMs;
		if (testMs > m_maxDeltaTimeMs)
			m_maxDeltaTimeMs = testMs;
	}
	else
	{
		m_startMeasurementTimeMs = GUtils::OSGetTimeStamp();
		m_totalMeasurementsCount = 0;
		m_maxDeltaTimeMs = 0;
		m_bRollingCounterInterrupted = 0;
	}

	m_lastNumMeasurementsInPacket = pMeasRec->nMeasurementsInPacket;
	m_lastMeasurementRollingCounter = pMeasRec->nRollingCounter;
	m_lastMeasurementTimeMs = GUtils::OSGetTimeStamp();
}

void CWinSkipMgr::AddCmdRespPacket(GSkipPacket *pRec)
{
/*
	cppsstream ss;
	ss << "Input cmd resp packet: " << hex << ((unsigned short) pRec->data[0]) << "h ";
	ss << ((unsigned short) pRec->data[1]) << "h ";
	ss << ((unsigned short) pRec->data[2]) << "h ";
	ss << ((unsigned short) pRec->data[3]) << "h ";
	ss << ((unsigned short) pRec->data[4]) << "h ";
	ss << ((unsigned short) pRec->data[5]) << "h ";
	ss << ((unsigned short) pRec->data[6]) << "h ";
	ss << ((unsigned short) pRec->data[7]) << "h ";
	GSTD_TRACE(ss.str());
*/
	m_pCmdRespPacketBuffer->AddRec(pRec);

	GSkipGenericResponsePacket *pRespRec = (GSkipGenericResponsePacket *) pRec;
	if (pRespRec->header & SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG)
	{
		if ((SKIP_CMD_ID_STOP_MEASUREMENTS == pRespRec->cmd) || (SKIP_CMD_ID_INIT == pRespRec->cmd))
			m_bRollingCounterInterrupted = 1;//This will prevent rolling counter trace output for the next measurement.
	}
}

bool GSkipBaseDevice::OSInitialize()
{
	GSTD_NEW(m_pOSData, (OSPtr), CWinSkipMgr());

	CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
	pSkipMgr->m_pDevice = this;

	return true;
}

void GSkipBaseDevice::OSDestroy()
{
	if (NULL != m_pOSData)
		delete ((CWinSkipMgr *) m_pOSData);
	m_pOSData = NULL;
}

int GSkipBaseDevice::OSOpen(GPortRef * /*pPortRef*/)
{
	int nResult = kResponse_Error;
	if (NULL != m_pOSData)
	{
		int nAttemptCount = 0;
		int nMaxAttemptCount = 1;
		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		if (!::GetVersionEx(&vi))
			vi.dwMajorVersion = 5;//Assume a new operating system.

		if (vi.dwMajorVersion < 5)
			nMaxAttemptCount = WIN9X_OPEN_SKIP_DEVICE_MAX_ATTEMPT_COUNT;

		do
			if (LockDevice(1) && IsOKToUse())
			{
				CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
				GSTD_ASSERT(GetPortRefPtr() != NULL);
				nResult = pSkipMgr->Open(GetPortRefPtr()->GetDisplayName());
				nAttemptCount++;

				UnlockDevice();

				if (kResponse_Error == nResult)
				{
					cppsstream ss;
					ss << "Error opening Skip/Jonah with id " << GetPortRefPtr()->GetDisplayName();
					GSTD_TRACE(ss.str());

					if (nAttemptCount < nMaxAttemptCount)
						GUtils::Sleep(WIN9X_OPEN_SKIP_DEVICE_RETRY_PAUSE_MS);
					else
						break;
				}
			}
			else
				nResult = kResponse_Error;

		while (kResponse_Error == nResult);
	}

	return nResult;
}

int GSkipBaseDevice::OSClose()
{
	int nResult = kResponse_Error;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			nResult = pSkipMgr->Close();

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}

	return nResult;
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
		nResult = kResponse_OK;
		CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		int nPacketsDesired = (*pIONumPackets);
		if (nPacketsDesired > nBufferSizeInPackets)
			nPacketsDesired = nBufferSizeInPackets;

		if (LockDevice(1) && IsOKToUse())
		{
			while (nPacketsRead < nPacketsDesired)
			{
				if (pSkipMgr->m_pMeasurementPacketBuffer->RetrieveRec((GSkipPacket *) pBuf))
				{
					nPacketsRead++;
					pBuf += sizeof(GSkipPacket);
				}
				else
					break;
			}

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
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
		nResult = kResponse_OK;
		CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
		unsigned char *pBuf = (unsigned char *) pBuffer;
		int nPacketsDesired = (*pIONumPackets);
		if (nPacketsDesired > nBufferSizeInPackets)
			nPacketsDesired = nBufferSizeInPackets;

		if (LockDevice(1) && IsOKToUse())
		{
			while (nPacketsRead < nPacketsDesired)
			{
				if (pSkipMgr->m_pCmdRespPacketBuffer->RetrieveRec((GSkipPacket *) pBuf))
				{
					nPacketsRead++;
					pBuf += sizeof(GSkipPacket);
				}
				else
					break;
			}

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}

	(*pIONumPackets) = nPacketsRead;
	return nResult;
}

int GSkipBaseDevice::OSWriteCmdPackets(
	void * pBuffer, //[in] ptr to source buffer
	int nNumPackets)//[in] number of GSkipPackets to write
{
	int nResult = kResponse_Error;
	int nPacketsWritten = 0;
	DWORD nLastError;
	if (NULL != m_pOSData)
	{
		nResult = kResponse_OK;
		CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
		unsigned char *pSrcBuf = (unsigned char *) pBuffer;
		unsigned char xferBuf[BYTES_IN_MICROSOFT_HID_PACKET];
		DWORD nNumberOfBytesWritten = 0;
		OVERLAPPED HIDOverlapped;
		if (INVALID_HANDLE_VALUE == pSkipMgr->m_hHidDeviceFile)
			nResult = kResponse_Error;
		else
		if (WAIT_OBJECT_0 == WaitForSingleObject(pSkipMgr->m_hPollingThread, 0))
			nResult = kResponse_Error;	//Listening thread that gets response is kaput.

		if (LockDevice(1) && IsOKToUse())
		{
			while ((nPacketsWritten < nNumPackets) && (kResponse_OK == nResult))
			{
				HIDOverlapped.Offset = 0;
				HIDOverlapped.OffsetHigh = 0;
				HIDOverlapped.hEvent = pSkipMgr->m_hOverlappedWriteEvent;
				ResetEvent(HIDOverlapped.hEvent);
				xferBuf[0] = 0;
				memcpy(&xferBuf[FIRST_PAYLOAD_BYTE_INDEX_IN_MICROSOFT_HID_PACKET], pSrcBuf, sizeof(GSkipPacket));
				if (!WriteFile(pSkipMgr->m_hHidDeviceFile, xferBuf, sizeof(xferBuf), &nNumberOfBytesWritten, &HIDOverlapped))
				{
					nLastError = GetLastError();
					if (nLastError != ERROR_IO_PENDING)
					{
						nResult = kResponse_Error;
						CancelIo(pSkipMgr->m_hHidDeviceFile);
					}
					else
					{
						//Wait for the write operation to complete.
						if (WAIT_OBJECT_0 != WaitForSingleObject(HIDOverlapped.hEvent, WRITE_TIMEOUT_MS))
						{
							nResult = kResponse_Error;
							CancelIo(pSkipMgr->m_hHidDeviceFile);
						}
					}
				}

				if (kResponse_OK == nResult)
				{
					nPacketsWritten++;
					pSrcBuf += sizeof(GSkipPacket);
				}
			}

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}

	return nResult;
}

int GSkipBaseDevice::OSMeasurementPacketsAvailable(unsigned char *pNumMeasurementsInLastPacket /* = NULL */)
{
	int nPackets = 0;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			nPackets = pSkipMgr->m_pMeasurementPacketBuffer->NumRecsAvailable();
			if (pNumMeasurementsInLastPacket)
				(*pNumMeasurementsInLastPacket) = pSkipMgr->m_lastNumMeasurementsInPacket;

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}

	return nPackets;
}

int GSkipBaseDevice::OSCmdRespPacketsAvailable()
{
	int nPackets = 0;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			nPackets = pSkipMgr->m_pCmdRespPacketBuffer->NumRecsAvailable();

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}

	return nPackets;
}

int GSkipBaseDevice::OSClearIO()
{
	int nResult = kResponse_OK;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			pSkipMgr->m_pMeasurementPacketBuffer->Clear();
			pSkipMgr->m_lastNumMeasurementsInPacket = 0;
			pSkipMgr->m_pCmdRespPacketBuffer->Clear();

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}
	else
		nResult = kResponse_Error;

	return nResult;
}

int GSkipBaseDevice::OSClearMeasurementPacketQueue()
{
	int nResult = kResponse_OK;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			pSkipMgr->m_pMeasurementPacketBuffer->Clear();
			pSkipMgr->m_lastNumMeasurementsInPacket = 0;

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}
	else
		nResult = kResponse_Error;

	return nResult;
}

int GSkipBaseDevice::OSClearCmdRespPacketQueue()
{
	int nResult = kResponse_OK;
	if (NULL != m_pOSData)
	{
		if (LockDevice(1) && IsOKToUse())
		{
			CWinSkipMgr *pSkipMgr = (CWinSkipMgr *) m_pOSData;
			pSkipMgr->m_pCmdRespPacketBuffer->Clear();

			UnlockDevice();
		}
		else
			GSTD_ASSERT(0);
	}
	else
		nResult = kResponse_Error;

	return nResult;
}

StringVector GSkipBaseDevice::OSGetAvailableDevicesOfType(int nVendorID, // VERNIER
														  int nProductID) // Go! Temp etc.
{ // RETURN a string vector with the names of all devices of the desired type
	StringVector vsNames;

	WinEnumHIDDevices(nVendorID, nProductID, &vsNames);

	for (size_t ix = 0; ix < vsNames.size(); ix++)
		GSTD_LOG(GSTD_S("Enumerating GoDevice at: ") + GTextUtils::LongToCPPString(nVendorID) + GSTD_S(":") + GTextUtils::LongToCPPString(nProductID) + vsNames[ix]);

	return vsNames;
}
