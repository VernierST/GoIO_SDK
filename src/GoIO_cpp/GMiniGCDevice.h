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
// GMiniGCDevice.h
//

#ifndef _GMINIGCDEVICE_H_
#define _GMINIGCDEVICE_H_

#include "GSkipDevice.h"

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

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

#ifdef LIB_NAMESPACE
}
#endif

#endif // _GMINIGCDEVICE_H_
