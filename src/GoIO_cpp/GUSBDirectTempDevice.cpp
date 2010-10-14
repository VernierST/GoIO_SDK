// GUSBDirectTempDevice.cpp

#include "stdafx.h"
#include "GUSBDirectTempDevice.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const unsigned int kUSBDirectTempMaxLocalNonVolatileMemAddr = 127;
static const unsigned int kUSBDirectTempMaxRemoteNonVolatileMemAddr = 0;

StringVector GUSBDirectTempDevice::m_snapshotOfAvailableDevices;

static const real kDegreesCelsiusPerBit = 0.0078125;

GUSBDirectTempDevice::GUSBDirectTempDevice(GPortRef *pPortRef)
: TBaseClass(pPortRef)
{
	if (!OSInitialize())
		GUtils::Trace(GSTD_S("Error - GUSBDirectTempDevice constructor, OSInitialize() returned false."));
}

GUSBDirectTempDevice::~GUSBDirectTempDevice()
{
}

unsigned int GUSBDirectTempDevice::GetMaxLocalNonVolatileMemAddr(void)
{
	return kUSBDirectTempMaxLocalNonVolatileMemAddr;
}

unsigned int GUSBDirectTempDevice::GetMaxRemoteNonVolatileMemAddr(void)
{
	return kUSBDirectTempMaxRemoteNonVolatileMemAddr;
}

int GUSBDirectTempDevice::ReadSensorDDSMemory(
	unsigned char *pBuf, 
	unsigned int ddsAddr, 
	unsigned int nBytesToRead, 
	int nTimeoutMs /* = 1000 */, 
	bool *pExitFlag /* = NULL */)
{
	return ReadNonVolatileMemory(true, pBuf, ddsAddr, nBytesToRead, nTimeoutMs, pExitFlag);
}

int GUSBDirectTempDevice::WriteSensorDDSMemory(
	unsigned char *pBuf, 
	unsigned int ddsAddr, 
	unsigned int nBytesToWrite,
	int nTimeoutMs /* = 1000 */, 
	bool *pExitFlag /* = NULL */)
{
	return WriteNonVolatileMemory(true, pBuf, ddsAddr, nBytesToWrite, nTimeoutMs, pExitFlag);
}

real GUSBDirectTempDevice::ConvertToVoltage(int raw, EProbeType /* eProbeType */, bool /* bCalibrateADCReading = true */)
{
	return (GSkipBaseDevice::kVoltsPerBit_ProbeTypeAnalog5V*raw + GSkipBaseDevice::kVoltsOffset_ProbeTypeAnalog5V);
}


