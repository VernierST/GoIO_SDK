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
#include "GCircularBuffer.h"

#define SKIP_HOST_IO_STATUS_TIMED_OUT	1

class GSkipBaseDevice : public GDeviceIO
{
public:
						GSkipBaseDevice(GPortRef *pPortRef);

	virtual				~GSkipBaseDevice(); 

	virtual int			GetVendorID(void) = 0;
	virtual int			GetProductID(void) = 0;

	long 				Open(GPortRef *pPortRef);// override from GDeviceIO

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

	void				GetLastCmdResponseStatus(unsigned char *pLastCmd, unsigned char *pLastCmdStatus,
							unsigned char *pLastCmdWithErrorRespSentOvertheWire, unsigned char *pLastErrorSentOvertheWire);

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

	unsigned int		GetHostIOStatus() { return m_hostIOStatus;}

	virtual real		ConvertToVoltage(int raw, EProbeType eProbeType, bool bCalibrateADCReading = true) = 0;

	void				SetDiagnosticsFlag(bool bFlag) { m_bDiagnosticsEnabled = bFlag; }
	bool				GetDiagnosticsFlag() { return m_bDiagnosticsEnabled; }
	GCircularBuffer		*GetDiagnosticInputBufferPtr() { return m_diagnosticInputBufferPtr; }
	GCircularBuffer		*GetDiagnosticOutputBufferPtr() { return m_diagnosticOutputBufferPtr; }

	static StringVector OSGetAvailableDevicesOfType(int nVendorID, int nProductID);

protected:
	virtual long				GetInitCmdResponse(void *pRespBuf, long *pnRespBytes, long nTimeoutMs = 1000, bool *pExitFlag = NULL);

	static real			kVoltsPerBit_ProbeTypeAnalog5V;
	static real			kVoltsOffset_ProbeTypeAnalog5V;
	static real			kVoltsPerBit_ProbeTypeAnalog10V;
	static real			kVoltsOffset_ProbeTypeAnalog10V;

	int					m_nLatestRawMeasurement;
    bool                m_bIsMeasuring;
	unsigned int		m_hostIOStatus;
	unsigned char		m_lastCmd;
	unsigned char		m_lastCmdRespStatus;
	unsigned char		m_lastCmdWithErrorRespSentOvertheWire;
	unsigned char		m_lastErrorSentOvertheWire;
	bool				m_bDiagnosticsEnabled;
	GCircularBuffer		*m_diagnosticInputBufferPtr;
	GCircularBuffer		*m_diagnosticOutputBufferPtr;
	GPriorityMutex		*m_pTraceQueueAccessMutex;
		
private:
	typedef GDeviceIO TBaseClass;
	
};

#endif // _GSKIPBASEDEVICE_H_

