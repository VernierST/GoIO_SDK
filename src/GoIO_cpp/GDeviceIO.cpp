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

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
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
int GDeviceIO::Message(StdMsg nMessage, void * pData)
{
	int nResult = kResponse_OK;
	
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

int GDeviceIO::Write(const char * pString)
{
	int nRet = 0;
	if (LockDevice(1) && IsOKToUse())
	{
		int nBytes = strlen(pString);
		nRet = Write((void *)pString, &nBytes);
		if (!UnlockDevice())
			GSTD_ASSERT(0);
	}
	else
		GSTD_ASSERT(0);

	return nRet;
}
*/
int GDeviceIO::Open(GPortRef *pPortRef) 
{ 
	int nRet = 0;

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

int GDeviceIO::Close(void) 
{ 
	int nRet = 0;

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
int GDeviceIO::BytesAvailable(void) 
{ 
	int nRet = 0;

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

int GDeviceIO::Read(void * pBuffer, int * pIONumBytes, int nBufferSize) 
{ 
	int nRet = 0;

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

int GDeviceIO::Write(void * pBuffer, int * pIONumBytes) 
{ 
	int nRet = 0;

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
int GDeviceIO::ClearIO(void) 
{ 
	int nRet = 0;

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

#ifdef LIB_NAMESPACE
}
#endif

