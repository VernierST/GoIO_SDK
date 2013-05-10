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
// GThread.h

// Cross-platform thread class.

// GThread is a relatively simple thread object which
// runs a function in another thread and provides mutex
// functions for safe access to data shared across threads.
// It currently does NOT wrap things like thread priority,
// and more complex concurrency issues.
//
// A GThread can be created either on the stack or with new().
// The GThread constructor takes four params: 
// - A pointer to the function that the thread is to execute
// - A pointer to the parameter passed to the thread function
// - A pointer to generic data to be associated with the thread
// object (not passed to the thread itself - could be context info
// etc. for later use)
// - A bool param indicating whether the thread loops execution.
//
// If the thread does not loop, the function is executed only
// once (though of course it may internally loop or whatever).
//
// If the thread loops, the execution is executed until it
// returns an error (a negative value) or until it is explicitly
// killed.  Loop threads also call OSYield() after every execution
// of the thread function, required for some platforms.
//
// A GThread object does not begin execution (and does not actually
// create the OS-defined thread) until its OSStartThread() method is
// called.  OSStopThread() ceases thread execution and destroys the
// OS thread object, but it retains the function pointer and parameter
// so that the thread can be started and stopped multiple times.
// Note that the OS-specific thread callback is defined privately
// by the GThread implementation; it supplies the call to the
// cross-platform thread function, the loop logic, and any other
// OS-specific implementation requirements.
//
// The GThread class provides static interfaces that will usually
// be needed by users of threads:
// - OSCreateMutex() - creates an OS-defined mutex object
// - OSLockMutex() - lock a mutex to access a resource, blocking other threads
// - OSTryLockMutex() - same as above, but with a timeout parameter
// - OSUnlockMutex() - let other threads access the resource
// - OSDestroyMutex() - destroys mutex object created with OSCreateMutex().

#ifndef _GTHREAD_H_
#define _GTHREAD_H_

#include "GTypes.h"

#ifdef TARGET_OS_LINUX
#include <signal.h>
#endif

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

#ifdef TARGET_OS_LINUX
#define NGIO_PRIVATE_SIGNAL1 (SIGRTMIN + 12)
int NGIO_RegisterIOAbortSignalHandler(int signalNum);
int NGIO_DeregisterIOAbortSignalHandler();
#endif

typedef int (*StdThreadFunctionPtr)(void *);
typedef OSPtr OSThreadReference;
typedef OSPtr OSMutex;
#ifdef TARGET_OS_MAC
typedef int OSSemaphore;
#else
typedef OSPtr OSSemaphore;
#endif

typedef enum
{
	kThreadPriority_BelowNormal,		//lowest
	kThreadPriority_Normal,
	kThreadPriority_AboveNormal,
	kThreadPriority_High,
	kThreadPriority_TimeCritical		//highest
} EThreadPriority;

class GThread OS_STANDARD_BASE_CLASS
{
public:
							GThread(StdThreadFunctionPtr pFunction, 
									StdThreadFunctionPtr pStopFunction, 
									StdThreadFunctionPtr pStartFunction, 
									StdThreadFunctionPtr pLockFunction, 
									StdThreadFunctionPtr pUnlockFunction, 
									void * pThreadParam,
									void * pThreadData = NULL,
									bool bOneShot = false);
	virtual					~GThread();
				
	bool					OSStartThread(EThreadPriority priority = kThreadPriority_Normal);
	void					OSStopThread(void);

	void					OSSetPriority(EThreadPriority priority);
	EThreadPriority			OSGetPriority(void);

static void					OSSetCurrentThreadPriority(EThreadPriority priority);
static	EThreadPriority		OSGetCurrentThreadPriority(void);
	
	OSThreadReference		GetThreadRef(void) const { return m_pThreadRef; }

	static int				Main(void *pGThreadObject);	// cross platform "main" for this thread
	
	// Access/change fields (e.g. to start & stop with a different param or function)
	StdThreadFunctionPtr	GetThreadFunction(void) { return m_pFunction; }
	StdThreadFunctionPtr	GetStopFunction(void) { return m_pStopFunction; }
	StdThreadFunctionPtr	GetStartFunction(void) { return m_pStartFunction; }
	StdThreadFunctionPtr	GetLockFunction(void) { return m_pLockFunction; }
	StdThreadFunctionPtr	GetUnlockFunction(void) { return m_pUnlockFunction; }
	void *					GetThreadParam(void) { return m_pThreadParam; }
	void					SetThreadParam(void * pParam) { m_pThreadParam = pParam; }
	bool					IsKillThread(void) { return m_bKillThread; }
	void					SetThreadAlive(bool b) { m_bThreadAlive = b; }	// Only called when thread is exiting its main worker method (Windows)
	bool					IsThreadAlive(void) { return m_bThreadAlive; }

	bool					IsStart(void) { return m_bStart; }
	void					SetStart(bool b) { m_bStart = b; }
	bool					IsStop(void) { return m_bStop; }
	void					SetStop(bool b) { m_bStop = b; }

	bool					IsOneShot() const { return m_bOneShot; }

	static OSMutex			OSCreateMutex(const cppstring &sMutexName);
	static bool				OSLockMutex(OSMutex pMutex);
	static bool				OSTryLockMutex(OSMutex pMutex, int nTimeoutMS);
	static bool				OSUnlockMutex(OSMutex pMutex);
	static void				OSDestroyMutex(OSMutex pMutex);

	static OSSemaphore		OSCreateSemaphore(void);
	static void				OSDestroySemaphore(OSSemaphore pSemaphore);
	static bool				OSSemPost(OSSemaphore pSemaphore);
	static bool				OSSemWait(OSSemaphore pSemaphore);
	
	static void				OSYield(void); // called to yield processing time (used on Mac)
	
	static void				OSProcessEvents(void);
	
protected:
	
	StdThreadFunctionPtr	m_pFunction;
	StdThreadFunctionPtr	m_pStopFunction;
	StdThreadFunctionPtr	m_pStartFunction;
	StdThreadFunctionPtr	m_pLockFunction;
	StdThreadFunctionPtr	m_pUnlockFunction;

	void *					m_pThreadParam;

	OSThreadReference		m_pThreadRef;

	volatile bool			m_bKillThread;	// Check this flag to see if thread should die (Main App, or one-shot thread, should set this)
	bool					m_bThreadAlive;	// Only set to false when thread is actually dead

	bool					m_bStart;		// Use to pause and resume the collection (in repeat mode)
	bool					m_bStop;
	bool					m_bOneShot;
};

class GLiteThread OS_STANDARD_BASE_CLASS
{
public:
							GLiteThread(StdThreadFunctionPtr pFunction, 
									StdThreadFunctionPtr pStopFunction, 
									void * pThreadParam);
	virtual					~GLiteThread();
				
	bool					OSStartThread(EThreadPriority priority = kThreadPriority_Normal);
	void					OSStopThread(void);

	void					OSSetPriority(EThreadPriority priority);
	EThreadPriority			OSGetPriority(void);

	OSThreadReference		GetThreadRef(void) const { return m_pThreadRef; }

	static int				Main(void *pGThreadObject);	// cross platform "main" for this thread
	
	// Access/change fields (e.g. to start & stop with a different param or function)
	StdThreadFunctionPtr	GetThreadFunction(void) { return m_pFunction; }
	StdThreadFunctionPtr	GetStopFunction(void) { return m_pStopFunction; }
	void *					GetThreadParam(void) { return m_pThreadParam; }
	void					SetThreadParam(void * pParam) { m_pThreadParam = pParam; }
	void					SetThreadAlive(bool b) { m_bThreadAlive = b; }
	bool					IsThreadAlive(void) { return m_bThreadAlive; }

protected:
	
	StdThreadFunctionPtr	m_pFunction;
	StdThreadFunctionPtr	m_pStopFunction;

	void *					m_pThreadParam;

	OSThreadReference		m_pThreadRef;

	bool					m_bThreadAlive;	// Only set to false when thread is actually dead
};

//GPriorityMutex is a helper class designed to help avoid priority inversion that can occur when a low priority thread
//blocks a high priority thread because it holds a mutex that the high priority thread is waiting on.
//Use these objects with care: typical usage is to call TryLockMutex at the top of a function and call UnlockMutex at the
//bottom of the function.
//Nested lock/unlock calls on a mutex must follow a stack pattern: lock call A, lock call B, unlock call B, unlock call A is ok;
//but lock call A, lock call B, unlock call A, unlock call B will cause priority errors.
//Also do not attempt to hold locks to 2 different GPriorityMutexes at the same time!

struct GPriorityMutex
{
	GPriorityMutex(EThreadPriority priority, const cppstring &sMutexName)
	{
		m_minimum_priority_while_locked = priority;
		m_OSMutex = GThread::OSCreateMutex(sMutexName);
//		GSTD_ASSERT(m_OSMutex != NULL);
	}
	~GPriorityMutex()
	{
		if (m_OSMutex != NULL)
			GThread::OSDestroyMutex(m_OSMutex);
		m_OSMutex = NULL;
	}
	bool TryLockMutex(int nTimeoutMS, EThreadPriority *pOldPriority)
	{
		bool bSuccess = false;
		if (m_OSMutex != NULL)
		{
			(*pOldPriority) = GThread::OSGetCurrentThreadPriority();
			if ((*pOldPriority) < m_minimum_priority_while_locked)
				GThread::OSSetCurrentThreadPriority(m_minimum_priority_while_locked);
			bSuccess = GThread::OSTryLockMutex(m_OSMutex, nTimeoutMS);
			if (!bSuccess && ((*pOldPriority) < m_minimum_priority_while_locked))
				GThread::OSSetCurrentThreadPriority(*pOldPriority);
		}
		return bSuccess;
	}
	bool UnlockMutex(EThreadPriority oldPriority)
	{
		bool bSuccess = false;
		if (m_OSMutex != NULL)
		{
			bSuccess = GThread::OSUnlockMutex(m_OSMutex);
			EThreadPriority currentPriority = GThread::OSGetCurrentThreadPriority();
			if (currentPriority != oldPriority)
				GThread::OSSetCurrentThreadPriority(oldPriority);
		}
		return bSuccess;
	}

	EThreadPriority m_minimum_priority_while_locked;
	OSMutex m_OSMutex;
};

#ifdef LIB_NAMESPACE
}
#endif

#endif // GThread.h

