// GSkipDevice.cpp

#include "stdafx.h"
#include "GSkipDevice.h"
#include "GSkipErr.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const unsigned long kSkipMaxRemoteNonVolatileMemAddr = 127;

real GSkipDevice::k_fSkipMaxDeltaT = ((unsigned int) 0xffff)*0.001;
real GSkipDevice::k_fSkipMinDeltaT = 0.005;

StringVector GSkipDevice::m_snapshotOfAvailableDevices;

GSkipDevice::GSkipDevice(GPortRef *pPortRef)
: TBaseClass(pPortRef)
{
	m_flashRec.signature = 0;//flash rec is ignored until this = VALID_FLASH_SIGNATURE
	if (!OSInitialize())
		GUtils::Trace(GSTD_S("Error - GSkipDevice constructor, OSInitialize() returned false."));
}

GSkipDevice::~GSkipDevice()
{
}

unsigned long GSkipDevice::GetMaxLocalNonVolatileMemAddr(void)
{
	return (sizeof(m_flashRec) - 1);
}

unsigned long GSkipDevice::GetMaxRemoteNonVolatileMemAddr(void)
{
	return kSkipMaxRemoteNonVolatileMemAddr;
}

long GSkipDevice::SendCmdAndGetResponse(
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
	long nResult;
	if (SKIP_CMD_ID_INIT == cmd)
		nResult = SendInitCmdAndGetResponse(pParams, nParamBytes, pRespBuf, pnRespBytes, nTimeoutMs, pExitFlag);
	else
		nResult = TBaseClass::SendCmdAndGetResponse(cmd, pParams, nParamBytes, pRespBuf, pnRespBytes, nTimeoutMs, pExitFlag);

	return nResult;
}

long GSkipDevice::SendInitCmdAndGetResponse(
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
	unsigned char initStatus;
	long initStatusLength = sizeof(initStatus);

	if (LockDevice(1) && IsOKToUse())
	{
		nResult = kResponse_OK; 
		long maxNumRetries = (nTimeoutMs + SKIP_TIMEOUT_MS_CMD_ID_INIT_WITH_BUSY_STATUS - 1)/SKIP_TIMEOUT_MS_CMD_ID_INIT_WITH_BUSY_STATUS;
		if (maxNumRetries > 1)
			nTimeoutMs = SKIP_TIMEOUT_MS_CMD_ID_INIT_WITH_BUSY_STATUS;
		bool bSuccess = false;
		long numRetries;
		GSkipInitParams initParams;
		initParams.reportErrorWhilePoweringUpFlag = 1;
		if (pParams)
			GSTD_ASSERT(nParamBytes == sizeof(initParams));
		else
		{
			pParams = &initParams;
			nParamBytes = sizeof(initParams);
		}

		//Send multiple init commands until the timeout is reached because Skip may ignore commands sent before it is done
		//powering up.
		for (numRetries = 0; (numRetries < maxNumRetries) && (kResponse_OK == nResult) && (!bSuccess); numRetries++)
		{
			nResult = SendCmd(SKIP_CMD_ID_INIT,	pParams, nParamBytes);

			if (kResponse_OK == nResult)
			{
				unsigned int nTimeCmdSent = GUtils::OSGetTimeStamp();

				initStatusLength = sizeof(initStatus);
				initStatus = SKIP_STATUS_CMD_NOT_SUPPORTED;
				if (kResponse_OK == GetInitCmdResponse(&initStatus, &initStatusLength, nTimeoutMs, pExitFlag))
					bSuccess = true;
				else if ((sizeof(initStatus) == initStatusLength) && (SKIP_STATUS_ERROR_SLAVE_POWERUP_INIT == initStatus))
				{
					unsigned int nTimeNow = GUtils::OSGetTimeStamp();
					unsigned int nElapsedTimeMs = nTimeNow - nTimeCmdSent;
					if (nElapsedTimeMs < nTimeoutMs)
						GUtils::Sleep(nTimeoutMs - nElapsedTimeMs);
				}
			}
			else
				initStatusLength = 0;
		}

		if (!bSuccess)
			nResult = kResponse_Error;

		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);	// Can't use this device -- some other thread has it open!

	if (pRespBuf && pnRespBytes)
	{
		if (((*pnRespBytes) >= initStatusLength) && (initStatusLength > 0))
		{
			(*pnRespBytes) = initStatusLength;
			memcpy(pRespBuf, &initStatus, initStatusLength);
		}
		else
			(*pnRespBytes) = 0;
	}

	return nResult;
}

long GSkipDevice::ReadSensorDDSMemory(
	unsigned char *pBuf, 
	unsigned long ddsAddr, 
	unsigned long nBytesToRead, 
	long nTimeoutMs /* = 1000 */, 
	bool *pExitFlag /* = NULL */)
{
	return ReadNonVolatileMemory(false, pBuf, ddsAddr, nBytesToRead, nTimeoutMs, pExitFlag);
}

long GSkipDevice::WriteSensorDDSMemory(
	unsigned char *pBuf, 
	unsigned long ddsAddr, 
	unsigned long nBytesToWrite,
	long nTimeoutMs /* = 1000 */, 
	bool *pExitFlag /* = NULL */)
{
	return WriteNonVolatileMemory(false, pBuf, ddsAddr, nBytesToWrite, nTimeoutMs, pExitFlag);
}

real GSkipDevice::ConvertToVoltage(int raw, EProbeType eProbeType, bool bCalibrateADCReading /* = true */)
{
	if (bCalibrateADCReading && (SKIP_VALID_FLASH_SIGNATURE == m_flashRec.signature))
	{
		if (kProbeTypeAnalog10V == eProbeType)
			raw = (int) floor((raw + m_flashRec.vinOffset)*m_flashRec.vinSlope + 0.5);
		else
			raw = (int) floor((raw + m_flashRec.vinLowOffset)*m_flashRec.vinLowSlope + 0.5);
	}

	if (kProbeTypeAnalog10V == eProbeType)
		return (GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog10V*raw + GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog10V);
	else
		return (GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog5V*raw + GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog5V);
}

long GSkipDevice::ConvertVoltageToRaw(real fVoltage, EProbeType eProbeType)
{// this routine will convert a voltage to the raw value
 // TODO jspam, probably need to add a reverse calibrate method as well to be consistent. but for now I don't
	// need that as I'm calculating offsets.
	long raw = 0;
	if (kProbeTypeAnalog10V == eProbeType)
		raw = (long) ((fVoltage - GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog10V)/GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog10V);
	else
		raw = (long) ((fVoltage - GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog5V)/GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog5V);
	
	return raw;
}

long GSkipDevice::ReadSkipFlashRecord(
	GSkipFlashMemoryRecord *pFlashRec, //[o] ptr to loc to store flash record. The numeric fields in this record
										//came from Skip in big endian format. This routine converts those fields to
										//the endian format that is native to the CPU currently running.
	long nTimeoutMs)
{
	long nResult = ReadNonVolatileMemory(true, pFlashRec, 0, sizeof(GSkipFlashMemoryRecord), nTimeoutMs);
	if (kResponse_OK != nResult)
		pFlashRec->signature = 0;//flash rec is ignored until this = VALID_FLASH_SIGNATURE
	else
	if (SKIP_VALID_FLASH_SIGNATURE == pFlashRec->signature)
	{
		//Handle endian issues.
		unsigned char * pLSB;
		unsigned char * pLMidB;
		unsigned char * pMMidB;
		unsigned char * pMSB;
		pMSB = (unsigned char *) &(pFlashRec->vinOffset);
		pLSB = pMSB + 1;
		GUtils::OSConvertBytesToShort(*pLSB, *pMSB, &(pFlashRec->vinOffset));
		
		pMSB = (unsigned char *) &(pFlashRec->vinLowOffset);
		pLSB = pMSB + 1;
		GUtils::OSConvertBytesToShort(*pLSB, *pMSB, &(pFlashRec->vinLowOffset));

		pMSB = (unsigned char *) &(pFlashRec->vinSlope);
		pMMidB = pMSB + 1;
		pLMidB = pMMidB + 1;
		pLSB = pLMidB + 1;
		GUtils::OSConvertBytesToFloat(*pLSB, *pLMidB, *pMMidB, *pMSB, &(pFlashRec->vinSlope));

		pMSB = (unsigned char *) &(pFlashRec->vinLowSlope);
		pMMidB = pMSB + 1;
		pLMidB = pMMidB + 1;
		pLSB = pLMidB + 1;
		GUtils::OSConvertBytesToFloat(*pLSB, *pLMidB, *pMMidB, *pMSB, &(pFlashRec->vinLowSlope));
	}

	return nResult;
}

long GSkipDevice::WriteSkipFlashRecord(
	const GSkipFlashMemoryRecord &flashRec, //[i] The numeric fields in this record are stored on Skip
										//in big endian format. This routine converts the numbers from the
										//endian format that is native to the current CPU into big endian
										//before writing them to Skip.
	long nTimeoutMs)
{
	//Convert numeric fields in flashRec to big endian format.
	GSkipFlashMemoryRecord bigEndianFlashRec = flashRec;

	unsigned char * pLSB;
	unsigned char * pLMidB;
	unsigned char * pMMidB;
	unsigned char * pMSB;
	pMSB = (unsigned char *) &bigEndianFlashRec.vinOffset;
	pLSB = pMSB + 1;
	GUtils::OSConvertShortToBytes(flashRec.vinOffset, pLSB, pMSB);

	pMSB = (unsigned char *) &bigEndianFlashRec.vinLowOffset;
	pLSB = pMSB + 1;
	GUtils::OSConvertShortToBytes(flashRec.vinLowOffset, pLSB, pMSB);

	pMSB = (unsigned char *) &bigEndianFlashRec.vinSlope;
	pMMidB = pMSB + 1;
	pLMidB = pMMidB + 1;
	pLSB = pLMidB + 1;
	GUtils::OSConvertFloatToBytes(flashRec.vinSlope, pLSB, pLMidB, pMMidB, pMSB);

	pMSB = (unsigned char *) &bigEndianFlashRec.vinLowSlope;
	pMMidB = pMSB + 1;
	pLMidB = pMMidB + 1;
	pLSB = pLMidB + 1;
	GUtils::OSConvertFloatToBytes(flashRec.vinLowSlope, pLSB, pLMidB, pMMidB, pMSB);

	long nResult = WriteNonVolatileMemory(true, &bigEndianFlashRec, 0, sizeof(bigEndianFlashRec), nTimeoutMs);

	return nResult;
}
