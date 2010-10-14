// GCyclopsDevice.cpp

#include "stdafx.h"
#include "GCyclopsDevice.h"
#include "GSensorDDSMem.h"

#include "GUtils.h"


#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUM_PACKETS_IN_RETRIEVAL_BUFFER 20

real GCyclopsDevice::k_fCyclopsMaxDeltaT = 180000*0.001;
#ifdef OPUS_LITE
real GCyclopsDevice::k_fCyclopsMinDeltaT = 0.04;
#else
real GCyclopsDevice::k_fCyclopsMinDeltaT = 0.02;
#endif
size_t GCyclopsDevice::k_nMaxRemotePoints = CYCLOPS_MAX_MEASUREMENT_COUNT;

StringVector GCyclopsDevice::m_snapshotOfAvailableDevices;

GCyclopsDevice::GCyclopsDevice(GPortRef *pPortRef)
: TBaseClass(pPortRef)
{
	if (!OSInitialize())
		GUtils::Trace(GSTD_S("Error - GCyclopsDevice constructor, OSInitialize() returned false."));
}

intVector GCyclopsDevice::ReadRawMeasurements(int desiredCount /*=-1*/) // Optional -- can limit the number that will be returned
{
	intVector result;
	int count = desiredCount;
	GCyclopsMeasurementPacket packets[NUM_PACKETS_IN_RETRIEVAL_BUFFER];

	if (LockDevice(1) && IsOKToUse())
	{ // Make sure we're the only thread that has acces to this device
		int nNumMeasurementsInVec = 0;
		int nNumPacketsJustRead, nNumPacketsToAskFor;
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
				int nPacket;
				for (nPacket = 0; nPacket < nNumPacketsJustRead; nPacket++)
				{
                    GCyclopsMeasurementPacket *pPacket = &packets[nPacket];
					GUtils::OSConvertBytesToInt(pPacket->measLsByteLsWord, pPacket->measMsByteLsWord, 
                        pPacket->measLsByteMsWord, pPacket->measMsByteMsWord, &measurement);
					result.push_back(measurement);
					nNumMeasurementsInVec++;
				}
			}
		}

		if ((desiredCount > 0) && (count > desiredCount))
			GSTD_TRACE( GSTD_S("ReadRawMeasurements() is returning more measurements than were asked for."));

		UnlockDevice();
	}
	else
		GSTD_ASSERT(0);

	if (result.size() > 0)
		m_nLatestRawMeasurement = result[result.size() - 1];

	return result;
}

int GCyclopsDevice::SendCmdAndGetResponse(
	unsigned char cmd,	//[in] command code
	void *pParams,		//[in] ptr to cmd specific parameter block, may be NULL.
	int nParamBytes,	//[in] # of bytes in (*pParams).
	void *pRespBuf,		//[out] ptr to destination buffer, may be NULL.
	int *pnRespBytes,  //[in, out] size of of dest buffer on input, size of response on output, may be NULL if pRespBuf is NULL.
	int nTimeoutMs /* = 1000 */,//[in] # of milliseconds to wait before giving up.
	bool *pExitFlag /* = NULL */)//[in] ptr to flag that another thread can set to force early exit. 
						//		THIS FLAG MUST BE FALSE FOR THIS ROUTINE TO RUN.
						//		Ignore this if NULL.
{
	int nResult = TBaseClass::SendCmdAndGetResponse(cmd, pParams, nParamBytes, pRespBuf, pnRespBytes, nTimeoutMs, pExitFlag);

    if (kResponse_OK == nResult)
    {
		//Keep track if we are starting measurements.
	    if (SKIP_CMD_ID_START_MEASUREMENTS == cmd) //Check for STOP in SendCmd().
		{
			if (!pParams)
				m_bIsMeasuring = true;
			else
			{
				GSTD_ASSERT(sizeof(GCyclopsStartMeasurementsParams) == nParamBytes);
				GCyclopsStartMeasurementsParams *pCyclopsParams = static_cast<GCyclopsStartMeasurementsParams *>(pParams);
				if ((0 == pCyclopsParams->lsbyteMeasurementCount) && (0 == pCyclopsParams->msbyteMeasurementCount))
					m_bIsMeasuring = true;
				else
					m_bIsMeasuring = false;//We are starting non real time measurements. This ends eventually - we will
										   //pretend that it has stopped already.
			}
		}
    }

	return nResult;
}

int GCyclopsDevice::ReadSensorDDSMemory(
    unsigned char *pBuf, 
    unsigned int ddsAddr, 
    unsigned int nBytesToRead, 
	int /* nTimeoutMs */, 
    bool * /* pExitFlag = NULL */)
{
    int nResult = -1;
    if ((0 == ddsAddr) && (sizeof(GSensorDDSRec) == nBytesToRead))
    {
        nResult = 0;
		GSensorDDSRec rec;
	    memset(&rec, 0, sizeof(GSensorDDSRec));
        rec.SensorNumber = kSensorIdNumber_GoMotion;
		strcpy(rec.SensorLongName, "Position");
		strcpy(rec.SensorShortName, "x");
	    rec.MinSamplePeriod = (float) 0.020;
	    rec.TypSamplePeriod = (float) 0.100;
	    rec.YminValue = 0.0;
	    rec.YmaxValue = (float) 1000.0;
	    rec.CalibrationEquation = kEquationType_Linear;
		rec.CalibrationPage[0].CalibrationCoefficientA = 0.0;
		rec.CalibrationPage[0].CalibrationCoefficientB = (float) 1.0;
		rec.CalibrationPage[0].CalibrationCoefficientC = 0.0;
		strcpy(rec.CalibrationPage[0].Units, "(m)");
		rec.CalibrationPage[1].CalibrationCoefficientA = 0.0;
		rec.CalibrationPage[1].CalibrationCoefficientB = (float) 3.2808399;
		rec.CalibrationPage[1].CalibrationCoefficientC = 0.0;
		strcpy(rec.CalibrationPage[1].Units, "(ft)");
        rec.ActiveCalPage = 0;
        rec.HighestValidCalPageIndex = 1;

	    //OperationType is a LabPro specific field which implies probeType. Ouch!
	    rec.OperationType = 14;
        rec.Checksum = GMBLSensor::CalculateDDSDataChecksum(rec);

		GMBLSensor::MarshallDDSRec((GSensorDDSRec *) pBuf, rec);
    }

    return nResult;
}

