/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
// GUSBDirectTempDevice.cpp

#include "stdafx.h"
#include "GUSBDirectTempDevice.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
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

#ifdef LIB_NAMESPACE
}
#endif
