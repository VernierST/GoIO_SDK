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

typedef long (*StdThreadFunctionPtr)(void *);
typedef OSPtr OSThreadReference;
typedef OSPtr OSMutex;

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
				
	bool					OSStartThread(void);
	void					OSStopThread(void);
	
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

	static OSMutex			OSCreateMutex(const cppstring &sMutexName);
	static bool				OSLockMutex(OSMutex pMutex);
	static bool				OSTryLockMutex(OSMutex pMutex, long nTimeoutMS);
	static bool				OSUnlockMutex(OSMutex pMutex);
	static void				OSDestroyMutex(OSMutex pMutex);
	
	static void				OSYield(void); // called to yield processing time (used on Mac)
	
	static void				OSProcessEvents(void);
	
protected:
	
	StdThreadFunctionPtr	m_pFunction;
	StdThreadFunctionPtr	m_pStopFunction;
	StdThreadFunctionPtr	m_pStartFunction;
	StdThreadFunctionPtr	m_pLockFunction;
	StdThreadFunctionPtr	m_pUnlockFunction;

	void *					m_pThreadParam;

	OSThreadReference		m_pThreadRef;	// just used on Mac

	bool					m_bKillThread;	// Check this flag to see if thread should die (Main App, or one-shot thread, should set this)
	bool					m_bThreadAlive;	// Only set to false when thread is actually dead

	bool					m_bStart;		// Use to pause and resume the collection (in repeat mode)
	bool					m_bStop;
};

#endif // GThread.h

