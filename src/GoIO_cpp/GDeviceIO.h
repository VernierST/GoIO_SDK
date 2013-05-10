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
// GDeviceIO.h
//
// GDeviceIO is a cross-platform shim for data-streaming devices
// such as serial or USB devices.  Few assumptions are made about
// the types of parameters that may be required - void pointers are
// used so that any required parameter structure (OS-specific or general)
// can be adapted as needed.
//
// Ideally, implementations of this class should hide details such
// as handshaking, high-bit checksums, packet padding, etc. from the user.
//
// This class is intended to be a base class for concrete implementations which
// will no doubt need their own data members and helper methods.  However, the
// base methods are stubs rather than pure virtual methods.
//
#ifndef _GDEVICEIO_H_
#define _GDEVICEIO_H_

#include "GTypes.h"
//#include "GStdObject.h"
#include "GPortRef.h"
#include "GThread.h"

#include "GVernierUSB.h"

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

class GDeviceIO OS_STANDARD_BASE_CLASS
{
public:
						GDeviceIO(GPortRef *pPortRef);
	virtual				~GDeviceIO();

//	virtual int		Message(StdMsg nMessage, void * pData);
	
	virtual int 		Open(GPortRef *pPortRef);
	virtual int 		Close(void);
//	virtual int 		BytesAvailable(void);
//	virtual int 		Read(void * pBuffer, int * pIONumBytes, int nBufferSize);
//	virtual int 		Write(void * pBuffer, int * pIONumBytes);
//	virtual int		Write(const char * pString);
	virtual int		ClearIO(void);

	// Thread safety lock for this device
	bool				LockDevice(int nTimeout_ms) { return GThread::OSTryLockMutex(m_pMutex, nTimeout_ms); }
	bool				UnlockDevice(void) { return GThread::OSUnlockMutex(m_pMutex); }

	// accessors & setters

	OSPtr				GetOSData(void) { return m_pOSData; }
	void				SetOSData(OSPtr p) { m_pOSData = p; }

	bool				IsOpen(void) { return m_bOpen; }
	void				SetOpen(bool b) { m_bOpen = b; }

//	int				GetReadTimeout(void) { return m_nReadTimeout; }
//	void				SetReadTimeout(int nTicks) { m_nReadTimeout = nTicks; }

	virtual bool 		OSInitialize(void) { return true;}
	virtual void		OSDestroy(void) {}

	virtual int 		OSOpen(GPortRef * /*pPortRef*/) { return 0L;}
	virtual int 		OSClose(void) { return 0L;}
//	virtual int 		OSBytesAvailable(void) { return 0L; }
//	virtual int 		OSRead(void * /*pBuffer*/, int * /*pIONumBytes*/, int /*nBufferSize*/) { return 0L; }
//	virtual int 		OSWrite(void * /*pBuffer*/, int * /*pIONumBytes*/) { return 0L; }
	virtual int		OSClearIO(void) { return kResponse_Error; }

	GPortRef *			GetPortRefPtr(void) { return &m_portRef; }
	GPortRef			GetPortRef(void) { return m_portRef; }
	void				SetPortRef(const GPortRef & src) { m_portRef.Assign(src); }

	bool				IsOKToUse(void) { return m_bOKToUse; }
	
						// text-binary, MBL mode members - see notes below
//	bool				IsModeText(void) const { return m_bModeText; }
//	void				SetModeText(bool b) { m_bModeText = b; OSSetModeText(b); }

//	void				SetBinaryMode(bool b) { SetModeText(!b); }
//	bool				IsBinaryMode(void) const { return !IsModeText(); }
	
//	void				SetRealTime(bool b) { m_bRealTime = b; }
//	bool				IsRealTime(void) const { return m_bRealTime; }
	
//	void				SetNumChannels(int n) { m_nNumChannels = n; }
//	int					GetNumChannels(void) const { return m_nNumChannels; }
	
	virtual bool		OSIsUSBDevice(void) { return true; }
	
//	virtual int		OSExpectSubPackets(short) { return kResponse_OK;}
//	virtual int		OSIgnoreSubPackets(void) { return kResponse_OK; }
//	virtual int		OSSetModeText(bool) { return kResponse_OK; }									 

protected:
	OSPtr				m_pOSData;
	int					m_nReadTimeout; // use for calls to Read()
	
	GPortRef			m_portRef;

						// Text-vs-binary and MBL collect-mode variables: these are
						// here because different subclasses need them.  The real
						// wrinkle is GSerialDevice which is a generic serial port
						// but -can- wrap a USB port in MacOS 9.
//	bool				m_bModeText;
//	int					m_nExpectBytes; // if not an even packet size, throw away leftovers
//	int					m_nNumChannels;	// how many MBL channels we're using for collection'
//	bool				m_bRealTime;

private:
//	typedef GStdObject TBaseClass;

	void				InitMembers(void);

	OSMutex				m_pMutex;	// This device cannot be shared
	bool				m_bOKToUse;	// False if we cannot get the mutex (e.g. multiple instances of LoggerPro)
	bool				m_bOpen;
};

#ifdef LIB_NAMESPACE
}
#endif

#endif // _GDEVICEIO_H_

