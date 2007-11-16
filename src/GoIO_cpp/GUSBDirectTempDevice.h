// GUSBDirectTempDevice.h
//

#ifndef _GUSBDIRECTTEMPDEVICE_H_
#define _GUSBDIRECTTEMPDEVICE_H_

#include "GSkipBaseDevice.h"

class GUSBDirectTempDevice : public GSkipBaseDevice
{
public:
						GUSBDirectTempDevice(GPortRef *pPortRef);

	virtual				~GUSBDirectTempDevice();

	virtual int			GetVendorID(void) { return VERNIER_DEFAULT_VENDOR_ID; }
	virtual int			GetProductID(void) { return USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID; }

	virtual unsigned long	GetMaxLocalNonVolatileMemAddr(void);
	virtual unsigned long	GetMaxRemoteNonVolatileMemAddr(void);
	
	static StringVector GetAvailableDevices(void) { return TBaseClass::OSGetAvailableDevicesOfType(VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID); }
	static void					StoreSnapshotOfAvailableDevices(const StringVector &devices) { m_snapshotOfAvailableDevices = devices; }
	static const StringVector &	GetSnapshotOfAvailableDevices(void) { return m_snapshotOfAvailableDevices; }
	
	virtual	long		ReadSensorDDSMemory(unsigned char * /*pBuf*/, unsigned long /*ddsAddr*/, unsigned long /*nBytesToRead*/, 
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);
	virtual	long		WriteSensorDDSMemory(unsigned char * /*pBuf*/, unsigned long /*ddsAddr*/, unsigned long /*nBytesToWrites*/,
							long nTimeoutMs = 1000, bool *pExitFlag = NULL);

	real				GetMeasurementTickInSeconds(void) { return 0.000128; }
	real				GetMinimumMeasurementPeriodInSeconds(void) { return 0.000128*floor(0.5/0.000128 + 0.5); }
	real				GetMaximumMeasurementPeriodInSeconds(void) { return ((unsigned int) 0xffffff)*0.000128; } 

	virtual real		ConvertToVoltage(int raw, EProbeType eProbeType, bool bCalibrateADCReading = true);

private:
	typedef GSkipBaseDevice TBaseClass;

	static StringVector	m_snapshotOfAvailableDevices;
};

#endif // _GUSBDIRECTTEMPDEVICE_H_
