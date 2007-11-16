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

	m_pThreadRef = NULL;	// only used on Mac

	m_pFunction = pFunction;
	m_pStopFunction = pStopFunction;
	m_pStartFunction = pStartFunction;
	m_pLockFunction = pLockFunction;
	m_pUnlockFunction = pUnlockFunction;
	m_pThreadParam = pThreadParam;

	m_bKillThread = bOneShot;

	m_bStop = m_bStart = false;
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
					if (pThread->IsKillThread())	
						break;

					if (pThread->IsStop())
					{
						pStopFunction(pParam);
						pThread->SetStop(false);
					}

					if (pThread->IsStart())
					{
						pStartFunction(pParam);
						pThread->SetStart(false);
					}

					// Call worker thread function
					nResult = (int) pStdFunction(pParam);
					if (nResult == kResponse_Error)	// exit thread on error
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

