// GDeviceIO.cpp

#include "stdafx.h"
#include "GDeviceIO.h"

//#include "GMessenger.h"
//#include "GMessages.h"
#include "GTextUtils.h"
#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const cppstring kDeviceMutexName = "DeviceMutex";

GDeviceIO::GDeviceIO(GPortRef *pPortRef)
{
	GSTD_ASSERT(pPortRef != NULL);
	SetPortRef(*pPortRef);
	InitMembers();
}

void GDeviceIO::InitMembers(void)
{
	m_pOSData = NULL;
	m_nReadTimeout = 2000;
	m_bOpen = false;
	m_pMutex = GThread::OSCreateMutex(GTextUtils::StringReplace(kDeviceMutexName + GetPortRefPtr()->GetLocation(), GSTD_S("\\"), GSTD_S("")));
	m_bOKToUse = LockDevice(1);
}

GDeviceIO::~GDeviceIO()
{
	if (IsOpen())
		Close();

	if (m_pMutex != NULL)
	{	
		if (IsOKToUse() && !UnlockDevice())
			GSTD_ASSERT(0);
		GThread::OSDestroyMutex(m_pMutex);
	}
	m_pMutex = NULL;
}

/*
long GDeviceIO::Message(StdMsg nMessage, void * pData)
{
	long nResult = kResponse_OK;
	
	bool * pBool = NULL;
	
	switch(nMessage)
	{
		// These are normally USB messages, but are given stub implementation
		// here in case any USB devices are masquerading as serial ports...
		case GMsg::kUSBExpectSubPackets:
		{
			short * pShort = static_cast<short *> (pData);
			if(pShort)
				OSExpectSubPackets(*pShort);
			else
				nResult = kResponse_Error; // bad param
			break;
		}
		
		case GMsg::kUSBIgnoreSubPacket:
			nResult = OSIgnoreSubPackets();
			break;
		
		case GMsg::kSetUSBTextMode:
		{
			pBool = static_cast<bool *> (pData);
			if (pBool)
				SetModeText(*pBool);
			else
				SetModeText(true);
			break;
		}
		
		case GMsg::kSetUSBRealTime:
		{
			pBool = static_cast<bool *> (pData);
			if (pBool)
				SetRealTime(*pBool);
			else
				SetRealTime(true);
			break;
		}
		
		case GMsg::kSetUSBNumChannels:
		{
			int * pInt = static_cast<int *> (pData);
			if (pInt != NULL)
				SetNumChannels(*pInt);
			else
				nResult = kResponse_Error; // bad param
		}
		default:
			nResult = TBaseClass::Message(nMessage, pData);
			break;
	}
	
	return nResult;
}

long GDeviceIO::Write(const char * pString)
{
	long nRet = 0;
	if (LockDevice(1) && IsOKToUse())
	{
		long nBytes = strlen(pString);
		nRet = Write((void *)pString, &nBytes);
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}
*/
long GDeviceIO::Open(GPortRef *pPortRef) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSOpen(pPortRef); 
		if (nRet == kResponse_OK)
			SetOpen(true);
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		nRet = -1;

	return nRet;
}

long GDeviceIO::Close(void) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSClose(); 
		if (nRet == kResponse_OK)
			SetOpen(false);
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}
/*
long GDeviceIO::BytesAvailable(void) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSBytesAvailable(); 
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}

long GDeviceIO::Read(void * pBuffer, long * pIONumBytes, long nBufferSize) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSRead(pBuffer, pIONumBytes, nBufferSize); 
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}

long GDeviceIO::Write(void * pBuffer, long * pIONumBytes) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSWrite(pBuffer, pIONumBytes); 
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}
*/
long GDeviceIO::ClearIO(void) 
{ 
	long nRet = 0;

	if (LockDevice(1) && IsOKToUse())
	{
		nRet = OSClearIO(); 
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}
