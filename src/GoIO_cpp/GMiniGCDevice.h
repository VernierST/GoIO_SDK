// GMiniGCDevice.h
//

#ifndef _GMINIGCDEVICE_H_
#define _GMINIGCDEVICE_H_

#include "GSkipDevice.h"

class GMiniGCDevice : public GSkipDevice
{
public:
						GMiniGCDevice(GPortRef *pPortRef);

	virtual int			GetProductID(void) { return MINI_GC_DEFAULT_PRODUCT_ID; }

	static StringVector GetAvailableDevices(void) { return TBaseClass::OSGetAvailableDevicesOfType(VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID); }
	static void			StoreSnapshotOfAvailableDevices(const StringVector &devices) { m_snapshotOfAvailableMiniGCDevices = devices; }
	static const StringVector &	GetSnapshotOfAvailableDevices(void) { return m_snapshotOfAvailableMiniGCDevices; }

private:
	typedef GSkipDevice TBaseClass;

	static StringVector	m_snapshotOfAvailableMiniGCDevices;
};

#endif // _GMINIGCDEVICE_H_
