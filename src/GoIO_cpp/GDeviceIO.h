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

class GDeviceIO OS_STANDARD_BASE_CLASS
{
public:
						GDeviceIO(GPortRef *pPortRef);
	virtual				~GDeviceIO();

//	virtual long		Message(StdMsg nMessage, void * pData);
	
	virtual long 		Open(GPortRef *pPortRef);
	virtual long 		Close(void);
//	virtual long 		BytesAvailable(void);
//	virtual long 		Read(void * pBuffer, long * pIONumBytes, long nBufferSize);
//	virtual long 		Write(void * pBuffer, long * pIONumBytes);
//	virtual long		Write(const char * pString);
	virtual long		ClearIO(void);

	// Thread safety lock for this device
	bool				LockDevice(long nTimeout_ms) { return GThread::OSTryLockMutex(m_pMutex, nTimeout_ms); }
	bool				UnlockDevice(void) { return GThread::OSUnlockMutex(m_pMutex); }

	// accessors & setters

	OSPtr				GetOSData(void) { return m_pOSData; }
	void				SetOSData(OSPtr p) { m_pOSData = p; }

	bool				IsOpen(void) { return m_bOpen; }
	void				SetOpen(bool b) { m_bOpen = b; }

//	long				GetReadTimeout(void) { return m_nReadTimeout; }
//	void				SetReadTimeout(long nTicks) { m_nReadTimeout = nTicks; }

	virtual bool 		OSInitialize(void) { return true;}
	virtual void		OSDestroy(void) {}

	virtual long 		OSOpen(GPortRef * /*pPortRef*/) { return 0L;}
	virtual long 		OSClose(void) { return 0L;}
//	virtual long 		OSBytesAvailable(void) { return 0L; }
//	virtual long 		OSRead(void * /*pBuffer*/, long * /*pIONumBytes*/, long /*nBufferSize*/) { return 0L; }
//	virtual long 		OSWrite(void * /*pBuffer*/, long * /*pIONumBytes*/) { return 0L; }
	virtual long		OSClearIO(void) { return kResponse_Error; }

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
	
//	virtual long		OSExpectSubPackets(short) { return kResponse_OK;}
//	virtual long		OSIgnoreSubPackets(void) { return kResponse_OK; }
//	virtual long		OSSetModeText(bool) { return kResponse_OK; }									 

protected:
	OSPtr				m_pOSData;
	long				m_nReadTimeout; // use for calls to Read()
	
	GPortRef			m_portRef;

						// Text-vs-binary and MBL collect-mode variables: these are
						// here because different subclasses need them.  The real
						// wrinkle is GSerialDevice which is a generic serial port
						// but -can- wrap a USB port in MacOS 9.
//	bool				m_bModeText;
//	long				m_nExpectBytes; // if not an even packet size, throw away leftovers
//	int					m_nNumChannels;	// how many MBL channels we're using for collection'
//	bool				m_bRealTime;

private:
//	typedef GStdObject TBaseClass;

	void				InitMembers(void);

	OSMutex				m_pMutex;	// This device cannot be shared
	bool				m_bOKToUse;	// False if we cannot get the mutex (e.g. multiple instances of LoggerPro)
	bool				m_bOpen;
};


#endif // _GDEVICEIO_H_

