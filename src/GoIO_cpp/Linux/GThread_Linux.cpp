// GThread_Linux.cpp

#include "GThread.h"
#include "GUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

void *start_thread(void *thread);
void *start_thread(void *thread)
{
	GThread::Main(thread);

	pthread_exit (NULL);
}

bool GThread::OSStartThread(void)
{
	bool bResult = false;
	m_pThreadRef = (OSThreadReference) new pthread_t();

	if (pthread_create((pthread_t *)m_pThreadRef, NULL, start_thread, (void*)this)==0)
	{
		bResult = true;
	}
	return bResult;
}

void GThread::OSStopThread(void)
{	
	m_bKillThread = true;
	while (IsThreadAlive())	// spin until thread exits
		GUtils::OSSleep(10);	// Give main process a chance to execute
	delete (pthread_t *) m_pThreadRef;
	m_pThreadRef = NULL;
}

OSMutex GThread::OSCreateMutex(const cppstring &/*sMutexName*/)
{
	pthread_mutex_t  *mutex;
	mutex = (pthread_mutex_t*)malloc(sizeof(*mutex));
	pthread_mutexattr_t   mta;
	pthread_mutexattr_init(&mta);
	pthread_mutexattr_settype(&mta,PTHREAD_MUTEX_RECURSIVE);

	if (mutex)
	{
		if(pthread_mutex_init(mutex,&mta)!=0)
		{
			printf("Trouble in paradise\n");
			free(mutex);
			mutex = NULL;
		}
	}
	return (OSMutex)mutex;
}

bool GThread::OSLockMutex(OSMutex pOSMutex)
{
	bool bResult = false;

   	if (pthread_mutex_lock((pthread_mutex_t*)pOSMutex)==0)
	{
		bResult = true;
	}		

	return bResult;
}

bool GThread::OSTryLockMutex(OSMutex pOSMutex, long nTimeoutMS)
{
	bool bResult = false;

	unsigned int sleepDurationMs = 2;
	unsigned int maxNumAttempts = (nTimeoutMS + 1)/sleepDurationMs;
	unsigned int numAttempts = 0;
	if (maxNumAttempts < 1)
		maxNumAttempts = 1;

	while ((!bResult) && (numAttempts < maxNumAttempts))
	{
		if (pthread_mutex_trylock((pthread_mutex_t*)pOSMutex)==0)
			bResult = true;
		else
		{
			numAttempts++;
			if (maxNumAttempts != 1)
				GUtils::OSSleep(sleepDurationMs);//We need a better solution - polling is bogus and unreliable. spam0
		}
	}
	if (!bResult)
	{
		printf("Trying to lock mutex %p on thread %p in GoIO_lib failed\n", pOSMutex, pthread_self());
	}

	return bResult;
}

bool GThread::OSUnlockMutex(OSMutex pOSMutex)
{
   	if (pthread_mutex_unlock((pthread_mutex_t*) pOSMutex) == 0)
	  {
	    return true;
	  }
	return false;
}

void GThread::OSDestroyMutex(OSMutex pOSMutex)
{
	pthread_mutex_destroy((pthread_mutex_t*)pOSMutex);
	free(pOSMutex);
}

void GThread::OSYield(void)
{
	pthread_yield();
}

void GThread::OSProcessEvents(void)
{
}

