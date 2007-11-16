// GThread_Mac.cpp

#include "GThread.h"

#include <Carbon/Carbon.h>
#undef TARGET_OS_MAC

#include "GUtils.h"

typedef struct
{
	MPSemaphoreID nSemaphoreID;
	MPTaskID nTaskID;
	int nRefCount;
} MPMutex;

typedef struct
{
	MPTaskID nTaskID;
	MPQueueID nQueueID;
} MPThread;

namespace { // local namespace

bool		g_bUsingMPThreads = true;

OSStatus local_AllPurposeMPThreadProc(void * pGThreadObject);
OSStatus local_AllPurposeMPThreadProc(void * pGThreadObject)
{
	return (OSStatus)GThread::Main(pGThreadObject);
}

}

bool GThread::OSStartThread(void)
{
	bool bResult = false;
	if(g_bUsingMPThreads)
	{
		OSStatus err;
		MPThread * pThread = NULL;
		pThread = new MPThread;
		
		// create queue for result code
		err = ::MPCreateQueue(&pThread->nQueueID);
		GSTD_ASSERT(err == noErr);

		err = ::MPCreateTask(local_AllPurposeMPThreadProc,
										(void *)this,
										0, // default stack size
										pThread->nQueueID, // use notify queue
										NULL,
										NULL,
										(MPTaskOptions) NULL, // default task options
										&pThread->nTaskID);
		GSTD_ASSERT(err == noErr);
		
		bResult = (err == noErr);
		if (bResult)
			m_pThreadRef = static_cast<OSThreadReference>(pThread);
										
	}
	return bResult;
}

void GThread::OSStopThread(void)
{
	if ((m_pThreadRef != NULL) && g_bUsingMPThreads)
	{
		OSStatus err, threadErr;
		MPThread * pThread = static_cast<MPThread *>(m_pThreadRef);
		if(pThread != NULL)
		{
			m_bKillThread = true;
			threadErr = noErr;
			
			// wait for notification that thread is dead
			err = ::MPWaitOnQueue(pThread->nQueueID, NULL, NULL, (void**) &threadErr, kDurationMillisecond * 3000);
			if (err != noErr)
			{
				cppsstream ss;
				ss << "ERROR " << err << " waiting for thread to shutdown" << endl;
				GSTD_TRACE(ss.str());
			}
			delete pThread;
		}
		m_pThreadRef = NULL;
	}
	// add classic Thread mgr or POSIX thread support here
}

OSMutex GThread::OSCreateMutex(const cppstring &/*sMutexName*/)
{
	if(g_bUsingMPThreads)
	{
		MPMutex * pMutex = NULL;
		MPSemaphoreID pSemaphore = NULL;
		OSErr err = ::MPCreateSemaphore(1, 1, &pSemaphore);
		//GSTD_TRACE("MPCreateSemaphore   <*>");
		if(err != noErr)
		{
			if(pSemaphore != NULL)
			{
				//GSTD_TRACE("MPDeleteSemaphore   <0>");
				::MPDeleteSemaphore(pSemaphore);
				pSemaphore = NULL;
			}
		}
		else
		{
			// create and initialize mutex object
			pMutex = new MPMutex;
			pMutex->nSemaphoreID = pSemaphore;
			pMutex->nTaskID = 0;
			pMutex->nRefCount = 0;
		}
		
		return (OSMutex) pMutex;
	}
	// add classic Thread mgr or POSIX thread support here
	
	return NULL;
}

bool GThread::OSLockMutex(OSMutex pOSMutex)
{	
	bool bResult = false;
	if(g_bUsingMPThreads)
	{
		MPMutex * pMutex = static_cast<MPMutex *>(pOSMutex);
		if (pMutex != NULL)
		{
			if (pMutex->nTaskID == MPCurrentTaskID())
			{
				pMutex->nRefCount++;
				bResult = true;
			}
			else
			{
				//GSTD_TRACE("MPWaitOnSemaphore   <+>");
				OSStatus err = ::MPWaitOnSemaphore(pMutex->nSemaphoreID, kDurationForever);
				bResult = (err == noErr);
				bool bBad = (pMutex->nRefCount != 0 || pMutex->nTaskID != 0);
				GSTD_ASSERT(!bBad);
				pMutex->nTaskID = MPCurrentTaskID();
				pMutex->nRefCount++;
			}
		}
	}
	// add classic Thread mgr or POSIX thread support here
	return bResult;
}

bool GThread::OSTryLockMutex(OSMutex pOSMutex, long nTimeoutMS)
{
	bool bResult = false;
	if(g_bUsingMPThreads)
	{
		MPMutex * pMutex = static_cast<MPMutex *>(pOSMutex);
		if (pMutex != NULL)
		{
			if (pMutex->nTaskID == MPCurrentTaskID())
			{
				pMutex->nRefCount++;
				bResult = true;
			}
			else
			{
				OSStatus err = ::MPWaitOnSemaphore(pMutex->nSemaphoreID, kDurationMillisecond * nTimeoutMS);
				if (err != noErr)
					bResult = false;
				else
				{
					bResult = true;
					bool bBad = (pMutex->nRefCount != 0 || pMutex->nTaskID != 0);
					GSTD_ASSERT(!bBad);
					pMutex->nTaskID = MPCurrentTaskID();
					pMutex->nRefCount++;
				}
			}
		}
	}
	// add classic Thread mgr or POSIX thread support here
	return bResult;
}

bool GThread::OSUnlockMutex(OSMutex pOSMutex)
{
	bool bResult = false;
	if (g_bUsingMPThreads)
	{
		MPMutex * pMutex = static_cast<MPMutex *>(pOSMutex);
		if (pMutex != NULL)
		{
			if (pMutex->nTaskID == MPCurrentTaskID())
			{
				if (pMutex->nRefCount > 0)
				{
					pMutex->nRefCount--;
					if (pMutex->nRefCount == 0)
					{
						pMutex->nTaskID = 0;
						::MPSignalSemaphore(pMutex->nSemaphoreID);
					}
					bResult = true;
				}
				else
					GSTD_ASSERT(0); // Unlocking a mutex that doesn't have any locks
			}
			else
				GSTD_ASSERT(0); // attempt to unlock a mutex that is held by another thread
		}
	}
	// add classic Thread mgr or POSIX thread support here

	return bResult;	// REVISIT: What is unlock failed?  Need to return false...
}

void GThread::OSDestroyMutex(OSMutex pOSMutex)
{
	if(g_bUsingMPThreads)
	{
		MPMutex * pMutex = static_cast<MPMutex *>(pOSMutex);
		if (pMutex != NULL)
		{
			::MPDeleteSemaphore(pMutex->nSemaphoreID);
			delete pMutex;
		}
	}
	// add classic Thread mgr or POSIX thread support here
}

void GThread::OSYield(void)
{
	// called to yield processing time
	if(g_bUsingMPThreads)
		::MPYield();
	// add classic Thread mgr or POSIX thread support here
}

void GThread::OSProcessEvents(void)
{
}
