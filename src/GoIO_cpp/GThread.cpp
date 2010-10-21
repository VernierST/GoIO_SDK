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
// GThread.cpp

#include "stdafx.h"
#include "GThread.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GThread::GThread(StdThreadFunctionPtr pFunction, 
				 StdThreadFunctionPtr pStopFunction,
				 StdThreadFunctionPtr pStartFunction,
				 StdThreadFunctionPtr pLockFunction,
				 StdThreadFunctionPtr pUnlockFunction,
				 void * pThreadParam,	// GMBLSource *
				 void * /*pThreadData*/,
				 bool bOneShot)
{
	m_bThreadAlive = true;

	m_pThreadRef = NULL;

	m_pFunction = pFunction;
	m_pStopFunction = pStopFunction;
	m_pStartFunction = pStartFunction;
	m_pLockFunction = pLockFunction;
	m_pUnlockFunction = pUnlockFunction;
	m_pThreadParam = pThreadParam;

	m_bKillThread = bOneShot;

	m_bStop = m_bStart = false;
	m_bOneShot = bOneShot;
}

GThread::~GThread()
{
	OSStopThread();
}

int GThread::Main(void *pGThreadObject) // pointer to this GThread object
{ // "main" routine for this thread
	GThread * pThread = static_cast<GThread *> (pGThreadObject);
	int nResult = 0;
	
	if (pThread != NULL)
	{
		StdThreadFunctionPtr pStdFunction = pThread->GetThreadFunction();
		StdThreadFunctionPtr pStopFunction = pThread->GetStopFunction();
		StdThreadFunctionPtr pStartFunction = pThread->GetStartFunction();
		StdThreadFunctionPtr pLockFunction = pThread->GetLockFunction();
		StdThreadFunctionPtr pUnlockFunction = pThread->GetUnlockFunction();
		void * pParam = pThread->GetThreadParam();

		if ((pLockFunction == NULL) || pLockFunction(pParam))
		{
			if (pStdFunction != NULL)
			{
				do
				{
					// Test for thread death before calling function (brain and such may not exist during shutdown)
					if (! pThread->IsOneShot() && pThread->IsKillThread() )	
						break;
					
					// Call SetStop(true) if you need the stop function to execute. It is seperate from OSStopThread.
					if (pThread->IsStop())
					{
						if (pStopFunction != NULL)
							pStopFunction(pParam);
						pThread->SetStop(false);
					}

					// Call SetStart(true) if you need the start function to execute. It is seperate from OSStartThread.
					if (pThread->IsStart())
					{
						if (pStartFunction != NULL)
							pStartFunction(pParam);
						pThread->SetStart(false);
					}

					// Call worker thread function
					if (pStdFunction != NULL)
						nResult = (int) pStdFunction(pParam);
					if (nResult == kResponse_Error || pThread->IsOneShot())	// exit thread on error
						break;

					// Let other threads have a crack at the CPU
					GThread::OSYield();
					GUtils::Sleep(30);

				} while (true);	// Loop until thread death flag is set, or worker function returns break value
			}

			if ((pUnlockFunction != NULL) && !pUnlockFunction(pParam))
				GSTD_ASSERT(0);
		}
		else
			GSTD_ASSERT(0);
			
		pThread->SetThreadAlive(false);
	}

	return nResult;
}

GLiteThread::GLiteThread(StdThreadFunctionPtr pFunction, 
				 StdThreadFunctionPtr pStopFunction,
				 void * pThreadParam)
{
	m_bThreadAlive = false;

	m_pThreadRef = NULL;

	m_pFunction = pFunction;
	m_pStopFunction = pStopFunction;
	m_pThreadParam = pThreadParam;
}

GLiteThread::~GLiteThread()
{
	OSStopThread();
}

int GLiteThread::Main(void *pGLiteThreadObject) // pointer to this GLiteThread object
{ // "main" routine for this thread
	GLiteThread * pLiteThread = static_cast<GLiteThread *> (pGLiteThreadObject);
	int nResult = 0;
	
	if (pLiteThread != NULL)
	{
		StdThreadFunctionPtr pStdFunction = pLiteThread->GetThreadFunction();
		void * pParam = pLiteThread->GetThreadParam();

		if (pStdFunction != NULL)
		{
			// Call worker thread function
			pLiteThread->SetThreadAlive(true);
			nResult = (int) pStdFunction(pParam);
			pLiteThread->SetThreadAlive(false);
		}
	}

	return nResult;
}
