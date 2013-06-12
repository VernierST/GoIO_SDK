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
// GThread_Win.cpp

#include "stdafx.h"
#include <afxmt.h>
#include "GThread.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_SEMAPHORE_COUNT 20

bool g_bExitThread = false;

namespace { // local namespace

static int ConvertEThreadPriorityToOSPriority(EThreadPriority ePriority)
{
	int osPriority;
	switch (ePriority)
	{
		case kThreadPriority_BelowNormal:
			osPriority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		case kThreadPriority_Normal:
			osPriority = THREAD_PRIORITY_NORMAL;
			break;
		case kThreadPriority_AboveNormal:
			osPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case kThreadPriority_High:
			osPriority = THREAD_PRIORITY_HIGHEST;
			break;
		case kThreadPriority_TimeCritical:
			osPriority = THREAD_PRIORITY_TIME_CRITICAL;
			break;
	}

	return osPriority;
}

static EThreadPriority ConvertOSPriorityToEThreadPriority(int osPriority)
{
	EThreadPriority ePriority;
	if (THREAD_PRIORITY_ERROR_RETURN == osPriority)
		ePriority = kThreadPriority_BelowNormal;
	else
	if (osPriority >= THREAD_PRIORITY_TIME_CRITICAL)
		ePriority = kThreadPriority_TimeCritical;
	else
	if (osPriority >= THREAD_PRIORITY_HIGHEST)
		ePriority = kThreadPriority_High;
	else
	if (osPriority >= THREAD_PRIORITY_ABOVE_NORMAL)
		ePriority = kThreadPriority_AboveNormal;
	else
	if (osPriority >= THREAD_PRIORITY_NORMAL)
		ePriority = kThreadPriority_Normal;
	else
		ePriority = kThreadPriority_BelowNormal;

	return ePriority;
}

// call back function conforming to AFX_THREADPROC
// this gets passed to a worker thread; it in turn calls
// our GThread's cross-platform StdThreadFunctionPtr function.

UINT local_AllPurposeWin32ThreadProc(LPVOID pGThreadObject);
UINT local_AllPurposeWin32ThreadProc(LPVOID pGThreadObject)
{
	UINT nReturn = (UINT)GThread::Main(pGThreadObject);

	return nReturn;
}

UINT local_AllPurposeWin32LiteThreadProc(LPVOID pGThreadObject);
UINT local_AllPurposeWin32LiteThreadProc(LPVOID pGThreadObject)
{
	UINT nReturn = (UINT)GLiteThread::Main(pGThreadObject);

	return nReturn;
}

} // end local namespace

bool GThread::OSStartThread(EThreadPriority priority /* = kThreadPriority_Normal */)
{
	CWinThread *pWinThread = AfxBeginThread(local_AllPurposeWin32ThreadProc, (LPVOID)this, 
		ConvertEThreadPriorityToOSPriority(priority), 0, 0);
	m_pThreadRef = pWinThread->m_hThread;
	return true;
}

void GThread::OSStopThread(void)
{ // User pressed stop so main app calls this.  Set our kill thread flag so next time through main thread loop we'll die.
	m_bKillThread = true;
	while (IsThreadAlive())	// spin until thread exits
		Sleep(10);	// Give main process a chance to execute
	m_pThreadRef = NULL;
}

void GThread::OSSetPriority(EThreadPriority ePriority)
{
	SetThreadPriority(m_pThreadRef, ConvertEThreadPriorityToOSPriority(ePriority));
}

EThreadPriority GThread::OSGetPriority(void)
{
	return ConvertOSPriorityToEThreadPriority(GetThreadPriority(m_pThreadRef));
}

void GThread::OSSetCurrentThreadPriority(EThreadPriority priority)
{
	SetThreadPriority(GetCurrentThread(), ConvertEThreadPriorityToOSPriority(priority));
}

EThreadPriority GThread::OSGetCurrentThreadPriority(void)
{
	return ConvertOSPriorityToEThreadPriority(GetThreadPriority(GetCurrentThread()));
}

OSMutex GThread::OSCreateMutex(const cppstring &sMutexName)	// optional param -- name this mutex
{
	HANDLE hMutex;
	if (0 == sMutexName.size())
		hMutex = CreateMutex(NULL, FALSE, NULL);
	else
		hMutex = CreateMutex(NULL, FALSE, sMutexName.c_str());
	return (OSMutex)hMutex;
}

bool GThread::OSLockMutex(OSMutex pMutex)
{
	return OSTryLockMutex(pMutex, INFINITE);
}

bool GThread::OSTryLockMutex(OSMutex pMutex, // Mutex Handle
							 int nTimeoutMS) // timeout to wait for mutex (in milliseconds)
{
	bool bResult = false;

    switch (WaitForSingleObject((HANDLE)pMutex, nTimeoutMS)) 
    {
        case WAIT_ABANDONED:	// Got ownership of an abandoned mutex object.
        case WAIT_OBJECT_0:		// The thread got normal mutex ownership.
            bResult = true;
            break; 

        // Cannot get mutex ownership due to time-out.
        case WAIT_TIMEOUT: 
			if (nTimeoutMS == 1)	// Only trace 1ms timeouts because these are the ones in the DeviceIO which shouldn't ever fail
				TRACE(GSTD_S(" Attempt lock FAILED.\n"));
            break; 
	}

	return bResult;
}

bool GThread::OSUnlockMutex(OSMutex pMutex)
{
	bool bRet = false;
	if (ReleaseMutex((HANDLE)pMutex) == TRUE)
		bRet = true;
	else
		TRACE(GSTD_S(" Attempt unlock FAILED.\n"));

	return bRet;
}

void GThread::OSDestroyMutex(OSMutex pMutex)
{ 
	CloseHandle(pMutex);
}

OSSemaphore GThread::OSCreateSemaphore(void)
{
	HANDLE hSemaphore = CreateSemaphore(NULL, 0, MAX_SEMAPHORE_COUNT, NULL);

	return (OSSemaphore) hSemaphore;
}

void GThread::OSDestroySemaphore(OSSemaphore pSemaphore)
{ 
	CloseHandle(pSemaphore);
}

bool GThread::OSSemPost(OSSemaphore pSemaphore)
{
	long oldCount;
	return (0 != ReleaseSemaphore((HANDLE) pSemaphore, 1, &oldCount));
}

bool GThread::OSSemWait(OSSemaphore pSemaphore)
{
	bool bResult = false;

    switch (WaitForSingleObject((HANDLE)pSemaphore, INFINITE)) 
    {
        case WAIT_OBJECT_0:	
            bResult = true;
            break; 

		default:
            bResult = false;
			break;
	}

	return bResult;
}

void GThread::OSYield(void)
{ // Sleep for a bit to allow other threads a chance to execute
	GUtils::Sleep(10);
}

void GThread::OSProcessEvents(void)
{ // stub
}

bool GLiteThread::OSStartThread(EThreadPriority priority /* = kThreadPriority_Normal */)
{
	CWinThread *pWinThread = AfxBeginThread(local_AllPurposeWin32LiteThreadProc, (LPVOID)this, 
		ConvertEThreadPriorityToOSPriority(priority), 0, 0);
	m_pThreadRef = pWinThread->m_hThread;
	return true;
}

void GLiteThread::OSStopThread(void)
{
	if (m_pStopFunction)
		m_pStopFunction(m_pThreadParam);

	while (IsThreadAlive())	// spin until thread exits
		GUtils::OSSleep(10);	// Give main process a chance to execute

	m_pThreadRef = NULL;
}

void GLiteThread::OSSetPriority(EThreadPriority ePriority)
{
	SetThreadPriority(m_pThreadRef, ConvertEThreadPriorityToOSPriority(ePriority));
}

EThreadPriority GLiteThread::OSGetPriority(void)
{
	return ConvertOSPriorityToEThreadPriority(GetThreadPriority(m_pThreadRef));
}



