// GCyclopsDevice.h
//

#ifndef _GCYCLOPSDEVICE_H_
#define _GCYCLOPSDEVICE_H_

#include "GSkipBaseDevice.h"

class GCyclopsDevice : public GSkipBaseDevice
{
public:
						GCyclopsDevice(GPortRef *pPortRef);
	virtual				~GCyclopsDevice() {}

	virtual int			GetVendorID(void) { return VERNIER_DEFAULT_VENDOR_ID; }
	virtual int			GetProductID(void) { return CYCLOPS_DEFAULT_PRODUCT_ID; }

	virtual unsigned int	GetMaxLocalNonVolatileMemAddr(void)
                                { return 0; }
	virtual unsigned int	GetMaxRemoteNonVolatileMemAddr(void)
                                { return 0; }

	static StringVector GetAvailableDevices(void) { return TBaseClass::OSGetAvailableDevicesOfType(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID); }
	static void			StoreSnapshotOfAvailableDevices(const StringVector &devices) { m_snapshotOfAvailableDevices = devices; }
	static const StringVector &	GetSnapshotOfAvailableDevices(void) { return m_snapshotOfAvailableDevices; }

	virtual int		SendCmdAndGetResponse(unsigned char cmd, void *pParams, int nParamBytes, void *pRespBuf, int *pnRespBytes, 
							int nTimeoutMs = 1000, bool *pExitFlag = NULL);

	virtual	int		ReadSensorDDSMemory(unsigned char * /*pBuf*/, unsigned int /*ddsAddr*/, unsigned int /*nBytesToRead*/, 
							int nTimeoutMs = 1000, bool *pExitFlag = NULL);
	virtual	int		WriteSensorDDSMemory(unsigned char * /*pBuf*/, unsigned int /*ddsAddr*/, unsigned int /*nBytesToWrites*/,
							int nTimeoutMs = 1000, bool *pExitFlag = NULL) { nTimeoutMs = 1; pExitFlag = NULL; return -1; }

	virtual intVector	ReadRawMeasurements(int count = -1);

	static real k_fCyclopsMaxDeltaT; //Const Min and max delta T
	static real k_fCyclopsMinDeltaT;
	real				GetMeasurementTickInSeconds(void) { return 0.001; }
	real				GetMinimumMeasurementPeriodInSeconds(void) { return k_fCyclopsMinDeltaT; }
	real				GetMaximumMeasurementPeriodInSeconds(void) { return k_fCyclopsMaxDeltaT; } 

	virtual real		ConvertToVoltage(int raw, EProbeType /*eProbeType*/, bool bCalibrateADCReading = true)
                            { bCalibrateADCReading = true; return raw*0.000001; }

	static size_t k_nMaxRemotePoints;

private:
	typedef GSkipBaseDevice TBaseClass;

	static StringVector	m_snapshotOfAvailableDevices;
};

#endif // _GCYCLOPSDEVICE_H_
