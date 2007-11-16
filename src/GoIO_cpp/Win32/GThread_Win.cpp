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

bool g_bExitThread = false;

namespace { // local namespace

// call back function conforming to AFX_THREADPROC
// this gets passed to a worker thread; it in turn calls
// our GThread's cross-platform StdThreadFunctionPtr function.

UINT local_AllPurposeWin32ThreadProc(LPVOID pGThreadObject);
UINT local_AllPurposeWin32ThreadProc(LPVOID pGThreadObject)
{
	UINT nReturn = (UINT)GThread::Main(pGThreadObject);

	return nReturn;
}

} // end local namespace

bool GThread::OSStartThread(void)
{
	AfxBeginThread(local_AllPurposeWin32ThreadProc, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, 0);
	return true;
}

void GThread::OSStopThread(void)
{ // User pressed stop so main app calls this.  Set our kill thread flag so next time through main thread loop we'll die.
	m_bKillThread = true;
	while (IsThreadAlive())	// spin until thread exits
		Sleep(10);	// Give main process a chance to execute
}

OSMutex GThread::OSCreateMutex(const cppstring &sMutexName)	// optional param -- name this mutex
{
	HANDLE hMutex = CreateMutex(NULL, FALSE, sMutexName.c_str());
	return (OSMutex)hMutex;
}

bool GThread::OSLockMutex(OSMutex pMutex)
{
	return OSTryLockMutex(pMutex, INFINITE);
}

bool GThread::OSTryLockMutex(OSMutex pMutex, // Mutex Handle
							 long nTimeoutMS) // timeout to wait for mutex (in milliseconds)
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
				TRACE(" Attempt lock FAILED.\n");
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
		TRACE(" Attempt unlock FAILED.\n");

	return bRet;
}

void GThread::OSDestroyMutex(OSMutex pMutex)
{ 
	CloseHandle(pMutex);
}

void GThread::OSYield(void)
{ // Sleep for a bit to allow other threads a chance to execute
	GUtils::Sleep(10);
}

void GThread::OSProcessEvents(void)
{ // stub
}




