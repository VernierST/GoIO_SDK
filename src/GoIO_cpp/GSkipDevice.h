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
// GSkipDevice.h
//

#ifndef _GSKIPDEVICE_H_
#define _GSKIPDEVICE_H_

#include "GSkipBaseDevice.h"

class GSkipDevice : public GSkipBaseDevice
{
public:
						GSkipDevice(GPortRef *pPortRef);
	virtual				~GSkipDevice();

	virtual int			GetVendorID(void) { return VERNIER_DEFAULT_VENDOR_ID; }
	virtual int			GetProductID(void) { return SKIP_DEFAULT_PRODUCT_ID; }

	virtual unsigned int	GetMaxLocalNonVolatileMemAddr(void);
	virtual unsigned int	GetMaxRemoteNonVolatileMemAddr(void);

	static StringVector GetAvailableDevices(void) { return TBaseClass::OSGetAvailableDevicesOfType(VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID); }
	static void					StoreSnapshotOfAvailableDevices(const StringVector &devices) { m_snapshotOfAvailableDevices = devices; }
	static const StringVector &	GetSnapshotOfAvailableDevices(void) { return m_snapshotOfAvailableDevices; }

	virtual int		SendCmdAndGetResponse(unsigned char cmd, void *pParams, int nParamBytes, void *pRespBuf, int *pnRespBytes, 
							int nTimeoutMs = 1000, bool *pExitFlag = NULL);
	virtual	int		ReadSensorDDSMemory(unsigned char * /*pBuf*/, unsigned int /*ddsAddr*/, unsigned int /*nBytesToRead*/, 
							int nTimeoutMs = 1000, bool *pExitFlag = NULL);
	virtual	int		WriteSensorDDSMemory(unsigned char * /*pBuf*/, unsigned int /*ddsAddr*/, unsigned int /*nBytesToWrites*/,
							int nTimeoutMs = 1000, bool *pExitFlag = NULL);

	static real k_fSkipMaxDeltaT; //Const Min and max delta T
	static real k_fSkipMinDeltaT;

	real				GetMeasurementTickInSeconds(void) { return 0.001; }
	real				GetMinimumMeasurementPeriodInSeconds(void) { return k_fSkipMinDeltaT; }
	real				GetMaximumMeasurementPeriodInSeconds(void) { return k_fSkipMaxDeltaT; } 

	virtual real		ConvertToVoltage(int raw, EProbeType eProbeType, bool bCalibrateADCReading = true);
	int					ConvertVoltageToRaw(real fVoltage, EProbeType eProbeType);

	void				SetSkipFlashRecord(const GSkipFlashMemoryRecord &rec) { m_flashRec = rec; }
	void				GetSkipFlashRecord(GSkipFlashMemoryRecord *pRec) { *pRec = m_flashRec; }

	int					WriteSkipFlashRecord(const GSkipFlashMemoryRecord &rec, int nTimeoutMs);
	int					ReadSkipFlashRecord(GSkipFlashMemoryRecord *pRec, int nTimeoutMs);

private:
	typedef GSkipBaseDevice TBaseClass;

	int					SendInitCmdAndGetResponse(void *pParams, int nParamBytes, void *pRespBuf, int *pnRespBytes, 
							int nTimeoutMs, bool *pExitFlag = NULL);

	static StringVector	m_snapshotOfAvailableDevices;
	GSkipFlashMemoryRecord m_flashRec;
};

#endif // _GSKIPDEVICE_H_
