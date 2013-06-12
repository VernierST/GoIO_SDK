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
// GThread_Linux.cpp

#include "GThread.h"
#include "GUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

static int g_NGIO_IOAbortSignalId = 0;

static void *start_thread(void *thread)
{
	GThread::Main(thread);

	pthread_exit (NULL);
}

bool GThread::OSStartThread(EThreadPriority /* priority = kThreadPriority_Normal */)
{
	bool bResult = false;
	m_pThreadRef = (OSThreadReference) new pthread_t();

	if (pthread_create((pthread_t *)m_pThreadRef, NULL, start_thread, (void*)this)==0)
		bResult = true;
	else
	{
		delete (pthread_t *) m_pThreadRef;
		m_pThreadRef = NULL;
	}

	return bResult;
}

void GThread::OSStopThread(void)
{
	if (m_pThreadRef)
	{
		m_bKillThread = true;
		while (IsThreadAlive())	// spin until thread exits
			GUtils::OSSleep(10);	// Give main process a chance to execute

		pthread_join(*((pthread_t *) m_pThreadRef), NULL);//Clean up.
		delete (pthread_t *) m_pThreadRef;
		m_pThreadRef = NULL;
	}
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
			GSTD_TRACE(GSTD_S("Trouble in paradise"));
			free(mutex);
			mutex = NULL;
		}
		else
		{
#ifdef _NGIO_LOGGING_ENABLED_
			char tmpstring[100];//spam
			sprintf(tmpstring, "Creating mutex %p on thread %p in NGIO_lib.", mutex, pthread_self());
			GSTD_TRACE(tmpstring);
#endif
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
		//	printf("Locked mutext: %d\n",pOSMutex);
	}		

	return bResult;
}

bool GThread::OSTryLockMutex(OSMutex pOSMutex, int nTimeoutMS)
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
		char tmpstring[100];//spam
		sprintf(tmpstring, "Trying to lock mutex %p on thread %p in IO_lib failed", pOSMutex, pthread_self());
		GSTD_TRACE(tmpstring);
	}

	return bResult;
}

bool GThread::OSUnlockMutex(OSMutex pOSMutex)
{
   	return (pthread_mutex_unlock((pthread_mutex_t*) pOSMutex) == 0);
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

void GThread::OSSetPriority(EThreadPriority /* ePriority */)
{
}

EThreadPriority GThread::OSGetPriority(void)
{
	return kThreadPriority_Normal;
}

void GThread::OSSetCurrentThreadPriority(EThreadPriority /* priority */)
{
}

EThreadPriority GThread::OSGetCurrentThreadPriority(void)
{
	return kThreadPriority_Normal;
}

OSSemaphore GThread::OSCreateSemaphore(void)
{
	sem_t *sem;
	sem = (sem_t *) malloc(sizeof(*sem));

	if (sem)
	{
		if (sem_init(sem, 0, 0) != 0)
		{
			GSTD_TRACE(GSTD_S("Trouble in paradise"));
			free(sem);
			sem = NULL;
		}
	}
	return (OSSemaphore) sem;
}

void GThread::OSDestroySemaphore(OSSemaphore pSemaphore)
{
	sem_destroy((sem_t *) pSemaphore);
	free(pSemaphore);
}

bool GThread::OSSemPost(OSSemaphore pSemaphore)
{
	return (0 == sem_post((sem_t *) pSemaphore));
}

bool GThread::OSSemWait(OSSemaphore pSemaphore)
{
	return (0 == sem_wait((sem_t *) pSemaphore));
}

static void *start_lite_thread(void *thread)
{
	GLiteThread::Main(thread);

	pthread_exit (NULL);
}

bool GLiteThread::OSStartThread(EThreadPriority priority /* = kThreadPriority_Normal */)
{
	bool bResult = false;
	m_pThreadRef = (OSThreadReference) new pthread_t();

	if (pthread_create((pthread_t *)m_pThreadRef, NULL, start_lite_thread, (void*)this)==0)
		bResult = true;
	else
	{
		delete (pthread_t *) m_pThreadRef;
		m_pThreadRef = NULL;
	}

	return bResult;
}

void GLiteThread::OSStopThread(void)
{
	if (m_pThreadRef)
	{
		if (m_pStopFunction)
			m_pStopFunction(m_pThreadParam);

		while (IsThreadAlive())	// spin until thread exits
			GUtils::OSSleep(10);	// Give main process a chance to execute

		pthread_join(*((pthread_t *) m_pThreadRef), NULL);//Clean up.
		delete (pthread_t *) m_pThreadRef;
		m_pThreadRef = NULL;
	}
}

void GLiteThread::OSSetPriority(EThreadPriority /* ePriority */)
{
}

EThreadPriority GLiteThread::OSGetPriority(void)
{
	return kThreadPriority_Normal;
}

static void abort_signal_handler(int status)
{
}

//Register a handler for the specified IO abort signal. We issue this signal to cancel pending IO operations during
//device shutdown.
int NGIO_RegisterIOAbortSignalHandler(int signalNum)
{
	int result = 0;
	if (signalNum <= 0)
		result = -1;
	else if (0 == g_NGIO_IOAbortSignalId)
	{
		struct sigaction saio;
		saio.sa_handler = abort_signal_handler;
		sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
		saio.sa_flags = 0;
		saio.sa_restorer = NULL;

		result = sigaction(signalNum, &saio, NULL);
		if (0 == result)
			g_NGIO_IOAbortSignalId = signalNum;
		else
			GSTD_TRACE(GSTD_S("Register IO Abort sigaction() failed"));

	}

	return result;
}

int NGIO_DeregisterIOAbortSignalHandler()
{
	if (g_NGIO_IOAbortSignalId != 0)
	{
		struct sigaction saio;
		saio.sa_handler = SIG_DFL;
		sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
		saio.sa_flags = 0;
		saio.sa_restorer = NULL;

		int status = sigaction(g_NGIO_IOAbortSignalId, &saio, NULL);
		if (0 != status)
			GSTD_TRACE(GSTD_S("Deregister IO Abort sigaction() failed"));

		g_NGIO_IOAbortSignalId = 0;
	}

	return 0;
}

#ifdef LIB_NAMESPACE
}
#endif
