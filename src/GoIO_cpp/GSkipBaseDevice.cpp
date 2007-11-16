// GSkipBaseDevice.cpp

#include "stdafx.h"
#include "GSkipBaseDevice.h"
#include "GCyclopsDevice.h" //Just used to see if SKIP_CMD_ID_START_MEASUREMENTS is starting real time measurements.

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

real GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog5V = 2.5/0x8000;
real GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog5V = 2.5;
real GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog10V = 10.0/0x8000;
real GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog10V = 0.0;

#define NUM_PACKETS_IN_RETRIEVAL_BUFFER 25

/*******************************************************************************
 GSkipBaseDevice:
*******************************************************************************/
GSkipBaseDevice::GSkipBaseDevice(GPortRef *pPortRef)
: TBaseClass(pPortRef)
{
	m_nLatestRawMeasurement = 0;
    m_bIsMeasuring = false;
}

GSkipBaseDevice::~GSkipBaseDevice()
{
	if (IsOpen())
		Close();
	OSDestroy();
}

long GSkipBaseDevice::OSBytesAvailable(void)
{
	GSTD_ASSERT(false); //not used!
	return 0;
}

long GSkipBaseDevice::OSRead(void * /*pBuffer*/, long * /*pIONumBytes*/, long /*nBufferSize*/)
{
	GSTD_ASSERT(false); //not used!
	return 0;
}

long GSkipBaseDevice::OSWrite(void * /*pBuffer*/, long * /*pIONumBytes*/)
{
	GSTD_ASSERT(false); //not used!
	return 0;
}

long GSkipBaseDevice::MeasurementsAvailable(void)
{
	unsigned char nNumMeasurementsInLastPacket;
	long nNumMeasurements = OSMeasurementPacketsAvailable(&nNumMeasurementsInLastPacket);
	return (nNumMeasurements*nNumMeasurementsInLastPacket);
}

intVector GSkipBaseDevice::ReadRawMeasurements(long desiredCount /*=-1*/) // Optional -- can limit the number that will be returned
{
	intVector result;
	long count = desiredCount;
	GSkipMeasurementPacket packets[NUM_PACKETS_IN_RETRIEVAL_BUFFER];

	if (LockDevice(1) && IsOKToUse())
	{ // Make sure we're the only thread that has acces to this device
		long nNumMeasurementsInVec = 0;
		long nNumPacketsJustRead, nNumPacketsToAskFor;
		int measurement;
		if (count < 0)
			count = MeasurementsAvailable();
		while (nNumMeasurementsInVec < count)
		{
			unsigned char nNumMeasurementsInLastPacket;
			nNumPacketsToAskFor = OSMeasurementPacketsAvailable(&nNumMeasurementsInLastPacket);
			if ((nNumMeasurementsInVec + nNumPacketsToAskFor*nNumMeasurementsInLastPacket) > count)
				nNumPacketsToAskFor = (count - nNumMeasurementsInVec)/nNumMeasurementsInLastPacket;
			if (0 == nNumPacketsToAskFor)
				break;

			nNumPacketsJustRead = nNumPacketsToAskFor;
			OSReadMeasurementPackets(&packets, &nNumPacketsJustRead, NUM_PACKETS_IN_RETRIEVAL_BUFFER);

			if (0 == nNumPacketsJustRead)
				break;
			else
			{
				long nPacket;
				for (nPacket = 0; nPacket < nNumPacketsJustRead; nPacket++)
				{
					unsigned char nMeasInPacket = 0;
					unsigned char *pMeasInPacket = &packets[nPacket].meas0LsByte;
					while (nMeasInPacket < packets[nPacket].nMeasurementsInPacket) //Return all the measurements in the packet.
					{
						short shortMeas;
						GUtils::OSConvertBytesToShort(pMeasInPacket[0], pMeasInPacket[1], &shortMeas);
						measurement = shortMeas;
						result.push_back(measurement);
						nNumMeasurementsInVec++;
						nMeasInPacket++;
						pMeasInPacket += 2;
					}
				}
			}
		}

		if ((desiredCount > 0) && (count > desiredCount))
			GSTD_TRACE(GSTD_S("ReadRawMeasurements() is returning more measurements than were asked for."));

		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	if (result.size() > 0)
		m_nLatestRawMeasurement = result[result.size() - 1];

	return result;
}

int	GSkipBaseDevice::GetLatestRawMeasurement()
{
	intVector vec;
	long count = MeasurementsAvailable();

	while (count > 0)
	{
		vec = ReadRawMeasurements(200);
		if (0 == vec.size())
			break;
		else
			count -= vec.size();
	}

	return m_nLatestRawMeasurement;
}

long GSkipBaseDevice::SendCmd(
	unsigned char cmd,	//[in] command code
	void *pParams,		//[in] ptr to cmd specific parameter block, may be NULL.
	long nParamBytes)	//[in] # of bytes in (*pParams).
{
	long nResult;
	GSkipOutputPacket packet;

	if (SKIP_CMD_ID_START_MEASUREMENTS == cmd)
		GSTD_ASSERT((0 == MeasurementsAvailable()) || pParams);
    else if ((SKIP_CMD_ID_STOP_MEASUREMENTS == cmd) || (SKIP_CMD_ID_INIT == cmd))
        m_bIsMeasuring = false;

	memset(&packet, 0, sizeof(packet));
	packet.cmd = cmd;
	if (pParams != NULL)
		memcpy(packet.params, pParams, nParamBytes);

	nResult = OSWriteCmdPackets(&packet, 1);

	if (GUtils::IsLogOpen())
	{
		cppsstream ssCmd;
		ssCmd << GSTD_S("Skip cmd sent: ") << hex << ((unsigned short) packet.cmd) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[0]) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[1]) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[2]) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[3]) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[4]) << GSTD_S("h "); 
		ssCmd << ((unsigned short) packet.params[5]) << GSTD_S("h ");
		ssCmd << ((unsigned short) packet.params[6]) << GSTD_S("h ");
		GSTD_TRACE(ssCmd.str());
	}

	if (kResponse_OK != nResult)
	{
		cppsstream ss;
		ss << endl << GSTD_S("Error writing ") << ((unsigned short) cmd) << GSTD_S(" cmd to skip.") << endl;
		GSTD_TRACE(ss.str());
	}

	return nResult;
}

long GSkipBaseDevice::GetNextResponse(
	void *pRespBuf,		//[out] ptr to destination buffer, may be NULL.
	long *pnRespBytes,  //[in, out] size of of dest buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	unsigned char *pCmd,//[out] identifies which command this response is for.
	bool *pErrRespFlag, //[out] flag indicating that the response contains error info
	long nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	long nResult = kResponse_OK;
	char *pRespCharBuf = static_cast<char *>(pRespBuf);
	(*pErrRespFlag) = false;
	GSkipGenericResponsePacket packet;
	bool bResponseComplete = false;
	bool bFirstPacketFound = false;
	long nBufSize = 0;
	if (pnRespBytes != NULL)
		nBufSize = (*pnRespBytes);
	long nRespSize = 0;
	long nBytesInPacket, nBytesRemainingInBuffer;
	bool *pMyExitFlag;
	bool myExitFlag = false;
	if (NULL == pExitFlag)
		pMyExitFlag = &myExitFlag;
	else
		pMyExitFlag = pExitFlag;
	unsigned char *packetPayload;

	unsigned int nStartTime = GUtils::OSGetTimeStamp();

	while (((GUtils::OSGetTimeStamp() - nStartTime) <= ((unsigned int) nTimeoutMs)) &&
			(!(*pMyExitFlag)) &&
			(!bResponseComplete) &&
			(kResponse_OK == nResult))
	{
		while (	(!bResponseComplete) &&
				(kResponse_OK == nResult) &&
				(OSCmdRespPacketsAvailable() > 0))
		{
			long transactionPacketCount = 1;
			nResult = OSReadCmdRespPackets(&packet, &transactionPacketCount, 1);
			if (kResponse_OK == nResult)
			{
				nBytesInPacket = packet.header & SKIP_MASK_CMD_RESP_NUMBYTES ;
				packetPayload = &packet.cmd;
				if (packet.header & SKIP_MASK_CMD_RESP_1ST_PACKET_FLAG)
				{
					if (nBytesInPacket > 0)
					{
						nBytesInPacket--;//Don't copy cmd id into response buffer.
						packetPayload++;
						(*pCmd) = packet.cmd;
					}
					else
						(*pCmd) = 0; //This should never happen!!!!
					if (bFirstPacketFound && (0 == (packet.header & SKIP_MASK_INPUT_PACKET_ERROR_FLAG)))
						nResult = kResponse_Error; //Two first packets found! - ok if second packet is error packet.
					bFirstPacketFound = true;
				}
				if (packet.header & SKIP_MASK_INPUT_PACKET_ERROR_FLAG)
				{
					//Early exit - we have an error packet.
					bResponseComplete = true;
					(*pErrRespFlag) = true;
					nRespSize = min(nBytesInPacket, nBufSize);
					if (pRespCharBuf && (nRespSize > 0))
						memcpy(pRespCharBuf, packetPayload, nRespSize);
				}
				if ((!bResponseComplete) && (kResponse_OK == nResult))
				{
					//Copy packet payload.
					nBytesRemainingInBuffer = nBufSize - nRespSize;
					nBytesInPacket = min(nBytesInPacket, nBytesRemainingInBuffer);
					if (pRespCharBuf && (nBytesInPacket > 0))
					{
						memcpy(&pRespCharBuf[nRespSize], packetPayload, nBytesInPacket);
						nRespSize += nBytesInPacket;
					}

					//Is this the last packet?
					if (packet.header & SKIP_MASK_CMD_RESP_LAST_PACKET_FLAG)
						bResponseComplete = true;
				}
			}
		}//while

		if ((!bResponseComplete) && (kResponse_OK == nResult))
			GUtils::Sleep(10);
	}//while

	if (pnRespBytes)
		(*pnRespBytes) = nRespSize;

	if ((kResponse_OK == nResult) && (!bResponseComplete))
		nResult = kResponse_Error; 

	return nResult;
}

long GSkipBaseDevice::GetInitCmdResponse(
	void *pRespBuf,		//[out] ptr to destination buffer, may be NULL.
	long *pnRespBytes,  //[in, out] size of of dest buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	long nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	/* This routine keeps looking at response packets until it sees an INIT cmd response, or times out. */
	/* Skip will respond to an INIT command at almost any time, so this allows us to recover from weird situations. */
	long nResult = kResponse_OK;
	GSkipDefaultResponsePacket packet;
	bool bResponseComplete = false;
	long nBufSize = 0;
	if (pnRespBytes != NULL)
		nBufSize = (*pnRespBytes);
	long nRespSize = 0;
	bool *pMyExitFlag;
	bool myExitFlag = false;
	if (NULL == pExitFlag)
		pMyExitFlag = &myExitFlag;
	else
		pMyExitFlag = pExitFlag;

	unsigned int nStartTime = GUtils::OSGetTimeStamp();

	while (((GUtils::OSGetTimeStamp() - nStartTime) <= ((unsigned int) nTimeoutMs)) &&
			(!(*pMyExitFlag)) &&
			(!bResponseComplete) &&
			(kResponse_OK == nResult))
	{
		while (	(!bResponseComplete) &&
				(kResponse_OK == nResult) &&
				(OSCmdRespPacketsAvailable() > 0))
		{
			long transactionPacketCount = 1;
			nResult = OSReadCmdRespPackets(&packet, &transactionPacketCount, 1);
			if (kResponse_OK == nResult)
			{
				if (packet.header & SKIP_INPUT_PACKET_INIT_RESP)
				{
					//We found it..
					bResponseComplete = true;
					nRespSize = min((unsigned long)sizeof(packet.errorStatus), (unsigned long)nBufSize);
					if (pRespBuf && (nRespSize > 0))
						memcpy(pRespBuf, &packet.errorStatus, nRespSize);

					if (packet.header & SKIP_MASK_INPUT_PACKET_ERROR_FLAG)
					{
						nResult = kResponse_Error;
						cppsstream sss;
						sss << endl << GSTD_S("SKIP_CMD_ID_INIT failed with response ") << hex ;
						sss << ((unsigned short) packet.errorStatus) << GSTD_S("h.") << endl;
						GSTD_TRACE(sss.str());
					}
				}
			}
		}//while

		if ((!bResponseComplete) && (kResponse_OK == nResult))
			GUtils::Sleep(10);
	}//while

	if (pnRespBytes)
		(*pnRespBytes) = nRespSize;

	if ((kResponse_OK == nResult) && (!bResponseComplete))
		nResult = kResponse_Error; 

	if (kResponse_Error == nResult)
	{
		cppsstream ss;
		ss << endl << GSTD_S("Error waiting for response to SKIP_CMD_ID_INIT.") << endl;
		GSTD_TRACE(ss.str());
	}

	return nResult;
}

long GSkipBaseDevice::SendCmdAndGetResponse(
	unsigned char cmd,	//[in] command code
	void *pParams,		//[in] ptr to cmd specific parameter block, may be NULL.
	long nParamBytes,	//[in] # of bytes in (*pParams).
	void *pRespBuf,		//[out] ptr to destination buffer, may be NULL.
	long *pnRespBytes,  //[in, out] size of of dest buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	long nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	long nResult = kResponse_Error;
	bool bTimeout = false;

	if (LockDevice(1) && IsOKToUse())
	{ // Make sure we're the only thread that has acces to this device
		nResult = SendCmd(cmd,	pParams, nParamBytes);

		if (kResponse_OK == nResult)
		{
			if (SKIP_CMD_ID_INIT == cmd)
				nResult = GetInitCmdResponse(pRespBuf,	pnRespBytes, nTimeoutMs, pExitFlag);
			else
			{
				unsigned char responseCmd;
				bool bError;
				cppsstream ss;
				nResult = GetNextResponse(pRespBuf,	pnRespBytes, &responseCmd, &bError, nTimeoutMs, pExitFlag);
				if (kResponse_OK != nResult)
				{
					bTimeout = true;
					ss << endl << GSTD_S("Error waiting for response to ") << hex << ((unsigned short) cmd) << GSTD_S("h cmd from Skip. Timeout??") << endl;
					GSTD_TRACE(ss.str());
				}
				else
				if (bError)
				{
					ss << endl << GSTD_S("Skip reported an error over the wire. cmd sent = ") << hex << ((unsigned short) cmd) << GSTD_S("h. cmd returned = ");
					ss << ((unsigned short) responseCmd) << "h." << endl;
					GSTD_TRACE(ss.str());
					nResult = kResponse_Error;
				}
				else
				if (cmd != responseCmd)
				{
					ss << endl << GSTD_S("Skip reported cmd response mismatch. cmd sent = ") << hex << ((unsigned short) cmd) << GSTD_S("h. cmd returned = ");
					ss << ((unsigned short) responseCmd) << GSTD_S("h.") << endl;
					GSTD_TRACE(ss.str());
					nResult = kResponse_Error;
				}
			}
		}

		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);	// Can't use this device -- some other thread has it open!

    if (kResponse_OK == nResult)
    {
		//Keep track if we are starting measurements.
	    if (SKIP_CMD_ID_START_MEASUREMENTS == cmd) //Check for STOP in SendCmd().
			m_bIsMeasuring = true;
    }
	else
	if (bTimeout)
	{
		//The Skip/Jonah firmware does not recover from DDS memory failures very well, so we will send an INIT command
		//if we timed out trying to read or write nonvolatile memory.
		switch (cmd)
		{
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_1BYTE:
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_2BYTES:
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_3BYTES:
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_4BYTES:
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_5BYTES:
			case SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_6BYTES:
			case SKIP_CMD_ID_READ_LOCAL_NV_MEM:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_1BYTE:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_2BYTES:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_3BYTES:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_4BYTES:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_5BYTES:
			case SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_6BYTES:
			case SKIP_CMD_ID_READ_REMOTE_NV_MEM:
				{
					bool bWasMeasuring = m_bIsMeasuring;
					long status2 = SendCmdAndGetResponse(SKIP_CMD_ID_INIT, NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_CMD_ID_INIT_WO_BUSY_STATUS);
					if ((kResponse_OK == status2) && bWasMeasuring)
					{
						//SKIP_CMD_ID_INIT turned off measurements - turn them back on.
						OSClearMeasurementPacketQueue();//Not supposed to turn on measurements with old measurements pending.
						SendCmdAndGetResponse(SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL);
					}
				}
				break;
			default:
				break;
		}
	}

	return nResult;
}

long GSkipBaseDevice::ReadNonVolatileMemory(
	bool bLocal,		//[in] Send SKIP_CMD_ID_READ_LOCAL_NV_MEM if true, else send SKIP_CMD_ID_READ_REMOTE_NV_MEM.
	void *pBuf,			//[out] ptr to destination buffer.
	unsigned long addr, //[in] addr of the first location in the NV memory to read.
	unsigned long nBytesToRead,//[in]
	long nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	unsigned long nMaxValidAddr = bLocal ? GetMaxLocalNonVolatileMemAddr() : GetMaxRemoteNonVolatileMemAddr();
	GSTD_ASSERT((addr + nBytesToRead - 1) <= nMaxValidAddr);
	GSkipReadI2CMemParams params;
	params.addr = static_cast<unsigned char>(addr);
	params.count = static_cast<unsigned char>(nBytesToRead);
	long nBytesRead = nBytesToRead;

	long nResult = SendCmdAndGetResponse(bLocal ? SKIP_CMD_ID_READ_LOCAL_NV_MEM : SKIP_CMD_ID_READ_REMOTE_NV_MEM,
		&params, sizeof(params), pBuf, &nBytesRead, nTimeoutMs, pExitFlag);

	if ((kResponse_OK == nResult) && (nBytesRead != (long)nBytesToRead))
		nResult = kResponse_Error;

	return nResult;
}

long GSkipBaseDevice::WriteNonVolatileMemory(
	bool bLocal,		//[in] Send SKIP_CMD_ID_WRITE_NV_MEM if true, else send SKIP_CMD_ID_WRITE_REMOTE_NV_MEM
	void *pBuf,			//[out] ptr to source buffer
	unsigned long addr, //[in] addr of the first location in the NV memory to write.
	unsigned long nBytesToWrite,//[in]
	long nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	long nResult = kResponse_OK;
	unsigned long nMaxValidAddr = bLocal ? GetMaxLocalNonVolatileMemAddr() : GetMaxRemoteNonVolatileMemAddr();
	GSTD_ASSERT((addr + nBytesToWrite - 1) <= nMaxValidAddr);
	GSkipWriteI2CMemParams params;
	char *pSrcBuf = static_cast<char *>(pBuf);
	bool *pMyExitFlag;
	bool myExitFlag = false;
	if (NULL == pExitFlag)
		pMyExitFlag = &myExitFlag;
	else
		pMyExitFlag = pExitFlag;
	unsigned char baseCmd = bLocal ? SKIP_CMD_ID_WRITE_LOCAL_NV_MEM_1BYTE : SKIP_CMD_ID_WRITE_REMOTE_NV_MEM_1BYTE;
	unsigned char cmd;
	unsigned long nBytesWritten = 0;
	unsigned long nBytesToWriteThisPacket;

	unsigned int nStartTime = GUtils::OSGetTimeStamp();
	unsigned int nCurrentTime = GUtils::OSGetTimeStamp();

	while (((nCurrentTime - nStartTime) <= ((unsigned int) nTimeoutMs)) &&
			(!(*pMyExitFlag)) &&
			(nBytesWritten < nBytesToWrite) &&
			(kResponse_OK == nResult))
	{
		nBytesToWriteThisPacket = nBytesToWrite - nBytesWritten;
		if (nBytesToWriteThisPacket > 6)
			nBytesToWriteThisPacket = 6;
		cmd = static_cast<unsigned char>(baseCmd + nBytesToWriteThisPacket - 1);
		params.addr = static_cast<unsigned char>(addr + nBytesWritten);
		memcpy(params.payload, &pSrcBuf[nBytesWritten], static_cast<size_t>(nBytesToWriteThisPacket));

		nResult = SendCmdAndGetResponse(cmd, &params, sizeof(params), NULL, NULL, 
			nTimeoutMs - (nCurrentTime - nStartTime), pMyExitFlag);

		if (kResponse_OK == nResult)
		{
			nBytesWritten += nBytesToWriteThisPacket;
			nCurrentTime = GUtils::OSGetTimeStamp();
		}
	}

	if ((kResponse_OK == nResult) && (nBytesWritten != nBytesToWrite))
		nResult = kResponse_Error;

	return nResult;
}

long GSkipBaseDevice::SetMeasurementPeriod(real fPeriodInSeconds, long nTimeoutMs/* = 1000*/)
{
	GSTD_ASSERT(fPeriodInSeconds >= 0.0);
	if (fPeriodInSeconds < GetMinimumMeasurementPeriodInSeconds())
		fPeriodInSeconds = GetMinimumMeasurementPeriodInSeconds();
	else
	if (fPeriodInSeconds > GetMaximumMeasurementPeriodInSeconds())
		fPeriodInSeconds = GetMaximumMeasurementPeriodInSeconds();

	GSkipSetMeasurementPeriodParams params;
	int	nNumTicks = (int) floor(fPeriodInSeconds/GetMeasurementTickInSeconds() + 0.5);
	GUtils::OSConvertIntToBytes(nNumTicks, &params.lsbyteLswordMeasurementPeriod, &params.msbyteLswordMeasurementPeriod,
		&params.lsbyteMswordMeasurementPeriod, &params.msbyteMswordMeasurementPeriod);

	long nResult = SendCmdAndGetResponse(SKIP_CMD_ID_SET_MEASUREMENT_PERIOD, &params, sizeof(params), NULL, NULL, nTimeoutMs);

	return nResult;
}

real GSkipBaseDevice::GetMeasurementPeriod(long nTimeoutMs/* = 1000*/)
{
	real fPeriodInSeconds = 1000000.0;
	GSkipGetMeasurementPeriodCmdResponsePayload payload;
	long nBytesRead = sizeof(payload);
	long nResult = SendCmdAndGetResponse(SKIP_CMD_ID_GET_MEASUREMENT_PERIOD, NULL, 0, &payload, &nBytesRead, nTimeoutMs);
	if (kResponse_OK == nResult)
	{
		int nNumTicks;
		GUtils::OSConvertBytesToInt(payload.lsbyteLswordMeasurementPeriod, payload.msbyteLswordMeasurementPeriod,
			payload.lsbyteMswordMeasurementPeriod, payload.msbyteMswordMeasurementPeriod, &nNumTicks);

		fPeriodInSeconds = GetMeasurementTickInSeconds() * nNumTicks;
	}

	return fPeriodInSeconds;
}

real GSkipBaseDevice::CalculateNearestLegalMeasurementPeriod(real fPeriodInSeconds)
{
	GSTD_ASSERT(fPeriodInSeconds >= 0.0);
	real fAdjustedPeriod = fPeriodInSeconds;
	int	nNumTicks = (int) floor(fPeriodInSeconds/GetMeasurementTickInSeconds() + 0.5);
	fAdjustedPeriod = GetMeasurementTickInSeconds() * nNumTicks;

	if (fAdjustedPeriod < GetMinimumMeasurementPeriodInSeconds())
		fAdjustedPeriod = GetMinimumMeasurementPeriodInSeconds();
	else
	if (fAdjustedPeriod > GetMaximumMeasurementPeriodInSeconds())
		fAdjustedPeriod = GetMaximumMeasurementPeriodInSeconds();

	return fAdjustedPeriod;
}

