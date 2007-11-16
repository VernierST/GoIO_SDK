// GSkipBaseDevice.h
//

#ifndef _GSKIPBASEDEVICE_H_
#define _GSKIPBASEDEVICE_H_

#include "GDeviceIO.h"

#include "GSkipCommExt.h"
#include "GCyclopsCommExt.h"
#include "GSkipComm.h"
#include "GMBLSensor.h"
#include "GVernierUSB.h"

class GSkipBaseDevice : public GDeviceIO
{
public:
						GSkipBaseDevice(GPortRef *pPortRef);

	virtual				~GSkipBaseDevice(); 

	virtual int			GetVendorID(void) = 0;
	virtual int			GetProductID(void) = 0;

	// Platform specific routines:

	// overrides from GDeviceIO:
	bool				OSInitialize(void);
	long 				OSOpen(GPortRef *pPortRef);
	long				OSClose(void);
	void				OSDestroy(void);

	// Note that these are packet oriented, rather than byte oriented.
	long				OSReadMeasurementPackets(void * pBuffer, long * pIONumPackets, long nBufferSizeInPackets);
	long				OSReadCmdRespPackets(void * pBuffer, long * pIONumPackets, long nBufferSizeInPackets);
	long 				OSWriteCmdPackets(void * pBuffer, long nNumPackets);

	long				OSMeasurementPacketsAvailable(unsigned char *pNumMeasurementsInLastPacket = NULL);
	long				OSCmdRespPacketsAvailable(void);

	long 				OSClearIO(void);
	long				OSClearMeasurementPacketQueue();
	long				OSClearCmdRespPacketQueue();

	long 				OSBytesAvailable(void); //not used - will assert!
	long 				OSRead(void * /*pBuffer*/, long * /*pIONumBytes*/, long /*nBufferSize*/); //not used - will assert!
	long 				OSWrite(void * /*pBuffer*/, long * /*pIONumBytes*/); //not used - will assert!

	// End of platform specific routines.

	long				SendCmd(unsigned char cmd, void *pParams, long nParamBytes);
	long				GetNextResponse(void *pRespBuf, long *pnRespBytes, unsigned char *pCmd, bool *pErrRespFlag, 
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);
	virtual long		SendCmdAndGetResponse(unsigned char cmd, void *pParams, long nParamBytes, void *pRespBuf, long *pnRespBytes, 
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);

	long				ReadNonVolatileMemory(bool bLocal, void *pBuf, unsigned long addr, unsigned long nBytesToRead,
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);
	long				WriteNonVolatileMemory(bool bLocal, void *pBuf, unsigned long addr, unsigned long nBytesToRead,
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);

	virtual unsigned long	GetMaxLocalNonVolatileMemAddr(void) = 0;
	virtual unsigned long	GetMaxRemoteNonVolatileMemAddr(void) = 0;

	virtual	long		ReadSensorDDSMemory(unsigned char * /*pBuf*/, unsigned long /*ddsAddr*/, unsigned long /*nBytesToRead*/, 
							long nTimeoutMs = 1000, bool *pExitFlag = NULL) = 0;
	virtual	long		WriteSensorDDSMemory(unsigned char * /*pBuf*/, unsigned long /*ddsAddr*/, unsigned long /*nBytesToWrites*/,
							long nTimeoutMs = 1000, bool *pExitFlag = NULL) = 0;

	virtual real		GetMeasurementTickInSeconds(void) = 0; 
	real				CalculateNearestLegalMeasurementPeriod(real fPeriodInSeconds);
	long				SetMeasurementPeriod(real fPeriodInSeconds, long nTimeoutMs = 1000);
	real				GetMeasurementPeriod(long nTimeoutMs = 1000);

	virtual real		GetMinimumMeasurementPeriodInSeconds(void) = 0;
	virtual real		GetMaximumMeasurementPeriodInSeconds(void) = 0;

	long				MeasurementsAvailable(void);
	virtual intVector	ReadRawMeasurements(long count = -1);
    bool                AreMeasurementsEnabled() { return m_bIsMeasuring; }

	int					GetLatestRawMeasurement(void);

	virtual real		ConvertToVoltage(int raw, EProbeType eProbeType, bool bCalibrateADCReading = true) = 0;

	static StringVector OSGetAvailableDevicesOfType(int nVendorID, int nProductID);

protected:
	long				GetInitCmdResponse(void *pRespBuf, long *pnRespBytes, long nTimeoutMs = 1000, bool *pExitFlag = NULL);

	static real			kVoltsPerBit_ProbeTypeAnalog5V;
	static real			kVoltsOffset_ProbeTypeAnalog5V;
	static real			kVoltsPerBit_ProbeTypeAnalog10V;
	static real			kVoltsOffset_ProbeTypeAnalog10V;

	int					m_nLatestRawMeasurement;
    bool                m_bIsMeasuring;
		
private:
	typedef GDeviceIO TBaseClass;
	
};

#endif // _GSKIPBASEDEVICE_H_
