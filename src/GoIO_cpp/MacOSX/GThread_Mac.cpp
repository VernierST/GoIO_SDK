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
// GThread_Mac.cpp

#include "GThread.h"
#include <CoreServices/CoreServices.h>

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#endif

extern "C" {					// Gotta extern C to prevent name mangling on mach / posix symbols (this happens when I move the project to Xcode 3.0)
#include <pthread.h>			// Yay pthreads!
#include <mach/semaphore.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach/task.h>
};

//
// The defines for min and max priority seem to be part of the internal POSIX build, but 
// are not to be found in the public headers. But by word of mouth, they seem to be
// 0 (min) and 31 (max). There are also accessor functions in sched.h but they are not
// documented.
//
#ifndef PTHREAD_MIN_PRIORITY
	#define PTHREAD_MIN_PRIORITY 0
#endif

#ifndef PTHREAD_MAX_PRIORITY
	#define PTHREAD_MAX_PRIORITY 31
#endif

bool local_SetThreadPriority(pthread_t, EThreadPriority);
EThreadPriority local_GetThreadPriority(pthread_t);
void local_AllPurposeMPThreadProc(void * pGThreadObject);

bool local_SetThreadPriority(pthread_t t, EThreadPriority ePriority)	// Set priority ePriority on thread t. Return true if successful.
{
	int nResult = 0;
	struct sched_param stParam;
	int nPolicy;
	// Get the thread's scheduling info:
	pthread_getschedparam(t, &nPolicy, &stParam);
	// Compute a new priority according to ePriority:
	stParam.sched_priority = ceil((((float)ePriority/(float) kThreadPriority_TimeCritical - (float) kThreadPriority_BelowNormal) * (float) (PTHREAD_MAX_PRIORITY - PTHREAD_MIN_PRIORITY)) + PTHREAD_MIN_PRIORITY);

	// For priorities High and Time Critical, use a Round Robin scheduling policy:
	if (ePriority >= kThreadPriority_High)
		nPolicy = SCHED_RR;
	else
		nPolicy = SCHED_OTHER;	// this is the default policy, which is called "Time Shared" scheduling.
	
	// Now set the priority:
	nResult = pthread_setschedparam(t, nPolicy, &stParam);
//	if (nResult != 0)
//		NSLog(@"*** Could not set thread priority, result 0x%x!", nResult);
	return (nResult == 0);
}

EThreadPriority local_GetThreadPriority(pthread_t t)	// Get the priority for thread t.
{
	int result = 0;
	struct sched_param stParam; 
	int nPolicy;
	// Get the priority:
	if (pthread_getschedparam(t, &nPolicy, &stParam) == 0)
	{
		// Compute the EThreadPriority:
		result = ((float)(stParam.sched_priority - PTHREAD_MIN_PRIORITY) / (float) (PTHREAD_MAX_PRIORITY - PTHREAD_MIN_PRIORITY)) * (float) kThreadPriority_TimeCritical - (float) kThreadPriority_BelowNormal;
	}
	return (EThreadPriority) result;
}

void local_AllPurposeMPThreadProc(void * pGThreadObject)	// This is the thread entry point.
{
//	id			pool = [[NSAutoreleasePool alloc] init];	// Non-NSThreads must wrap their innards in an autorelease pool.
	OSStatus	aStatus;

	// All callbacks must also be wrapped in a C++ exception handling:
	try
	{
		// Call into the main cross platform routine:
		aStatus = (OSStatus)GThread::Main(pGThreadObject);	
//		if (aStatus != 0)
//			NSLog(@"*** GThread::Main() exited with status %d", (int) aStatus);
	}
	catch(...)
	{
		GSTD_ASSERT(L"GThread_Mac: all purpose thread proc caught exception!!!");
	}
//	[pool release];
}

bool GThread::OSStartThread(EThreadPriority priority)
{
	bool bResult = false;
	pthread_t threadRef;
	int nThreadErr = pthread_create(&threadRef,											// Thread ref out
									NULL,												// NULL = use default attr.
									(void* (*)(void*)) local_AllPurposeMPThreadProc,	// the thread entry function
									this);												// and the context info / refcon.
	if (nThreadErr == 0)
	{	// Thread created successfully
		if (local_SetThreadPriority(threadRef, priority))	// Set priority
		{	// Priority succeeded
			m_pThreadRef = static_cast<OSThreadReference>(threadRef);
			bResult = true;
			SetThreadAlive(true);
		}
	}
	return bResult;
}

void GThread::OSStopThread(void)
{
	if (m_pThreadRef != NULL)
	{
		pthread_t threadRef = (pthread_t) m_pThreadRef;
		m_bKillThread = true;
		while (true)
		{
			if (!IsThreadAlive())
				break;
			GUtils::OSSleep(100);
		}
		pthread_detach(threadRef);
		m_pThreadRef = NULL;
	}
}

void GThread::OSSetPriority(EThreadPriority ePriority)
{
	if (m_pThreadRef)
		local_SetThreadPriority((pthread_t) m_pThreadRef, ePriority);
}

EThreadPriority GThread::OSGetPriority(void)
{	
	EThreadPriority eResult = kThreadPriority_Normal;
	if (m_pThreadRef)
		eResult = local_GetThreadPriority((pthread_t) m_pThreadRef);
	return eResult;
}

void GThread::OSSetCurrentThreadPriority(EThreadPriority priority)
{
	local_SetThreadPriority(pthread_self(), priority);
}

EThreadPriority GThread::OSGetCurrentThreadPriority(void)
{
	return local_GetThreadPriority(pthread_self());
}

OSMutex GThread::OSCreateMutex(const cppstring &/*sMutexName*/)
{
	pthread_mutex_t* pMutexRef = OPUS_NEW pthread_mutex_t;
	GSTD_ASSERT(pMutexRef != NULL);
	pthread_mutexattr_t attr;
	GSTD_ASSERT(pthread_mutexattr_init(&attr) == 0);								// create mutex attributes
	GSTD_ASSERT(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0);	// this allows the mutex to be locked multiple times on the same thread.
	GSTD_ASSERT(pthread_mutex_init(pMutexRef, &attr) == 0);							// create mutex
	GSTD_ASSERT(pthread_mutexattr_destroy(&attr) == 0);								// destroy attributes.
	
	return (OSMutex) pMutexRef;
}

bool GThread::OSLockMutex(OSMutex pOSMutex)
{
	GSTD_ASSERT(pOSMutex != NULL);
	pthread_mutex_t* pMutexRef = (pthread_mutex_t*) pOSMutex;
	return (pthread_mutex_lock(pMutexRef)==0);
}

bool GThread::OSTryLockMutex(OSMutex pOSMutex, int nTimeoutMS)
{
	GSTD_ASSERT(pOSMutex != NULL);
	pthread_mutex_t* pMutexRef = (pthread_mutex_t*) pOSMutex;
	int nLockErr;
	int nCt = 0;
	do
	{
		nLockErr = pthread_mutex_trylock(pMutexRef);
		if (nLockErr != 0)
		{
			nCt++;
			usleep(1000);	// sleep 1 millisecond.
			if (nCt > nTimeoutMS)
				break;
		}

	} while (nLockErr != 0);

	return (nLockErr == 0);
}

bool GThread::OSUnlockMutex(OSMutex pOSMutex)
{
	GSTD_ASSERT(pOSMutex != NULL);
	pthread_mutex_t* pMutexRef = (pthread_mutex_t*) pOSMutex;

	return (pthread_mutex_unlock(pMutexRef) == 0);
}

void GThread::OSDestroyMutex(OSMutex pOSMutex)
{
	GSTD_ASSERT(pOSMutex != NULL);
	pthread_mutex_t* pMutexRef = (pthread_mutex_t*) pOSMutex;
	pthread_mutex_destroy(pMutexRef);
	delete pMutexRef;
}

void GThread::OSYield(void)
{
	pthread_yield_np();
}

void GThread::OSProcessEvents(void)
{
	// NSSpam leftover from OS 9 - does nothing
}

OSSemaphore GThread::OSCreateSemaphore(void)
{	// Re-Revisited: since it looks like the POSIX semaphore implementation spin-locks in sem_wait, I replaced with mach semaphores, 
	// which are unnamed, do not spin, and work on 10.3.9.
	// (#19641) JK 20080220.
	semaphore_t sem = 0;
	semaphore_create(mach_task_self(),&sem, SYNC_POLICY_FIFO, 0);
	return (OSSemaphore) sem;
}

void GThread::OSDestroySemaphore(OSSemaphore pSemaphore)
{ 
	semaphore_destroy(mach_task_self(),(semaphore_t)pSemaphore);
}

bool GThread::OSSemPost(OSSemaphore pSemaphore)
{
	return semaphore_signal((semaphore_t)pSemaphore) == 0;
}

bool GThread::OSSemWait(OSSemaphore pSemaphore)
{
	return semaphore_wait((semaphore_t)pSemaphore) == 0;
}

bool GLiteThread::OSStartThread(EThreadPriority priority /* = kThreadPriority_Normal */)
{
	bool bResult = false;
	pthread_t threadRef;
	int nThreadErr = pthread_create(&threadRef,											// Thread ref out
									NULL,												// NULL = use default attr.
									(void* (*)(void*)) GLiteThread::Main,				// the thread entry function
									this);												// and the context info / refcon.
	if (nThreadErr == 0)
	{	// Thread created successfully
		if (local_SetThreadPriority(threadRef, priority))	// Set priority
		{	// Priority succeeded
			m_pThreadRef = static_cast<OSThreadReference>(threadRef);
			bResult = true;
			SetThreadAlive(true);
		}
	}
	return bResult;
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
	local_SetThreadPriority((pthread_t)m_pThreadRef, ePriority);
}

EThreadPriority GLiteThread::OSGetPriority(void)
{
	return local_GetThreadPriority((pthread_t)m_pThreadRef);
}


