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
#include "stdafx.h"
#include "GCircularBuffer.h"

#include "GUtils.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

GCircularBuffer::GCircularBuffer(
	int numBytes)//number of bytes that buffer holds before it starts to 'roll over'.
{
	GSTD_NEW(m_pBytes, (unsigned char *), unsigned char[numBytes+1]);
	m_pQueueAccessMutex = NULL;
	//Note that even though space for m_nBytesAllocated Bytes exists, we only report available counts from 0 to (m_nBytesAllocated-1).
	m_nBytesAllocated = numBytes+1;
	m_nFirstByte = 0;
	m_nNextByte = 0;
	m_nTotalBytesAdded = 0;
}

GCircularBuffer::~GCircularBuffer()
{
	delete [] m_pBytes;
}

bool GCircularBuffer::AddBytes(unsigned char *pBytes, int count)//Add count bytes to FIFO buffer.
{
	bool bSuccess = true;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		bool bFullBuffer = false;
		int testCount;
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		if (count < 0)
			count = 0;
		m_nTotalBytesAdded += count;

		if (count > (m_nBytesAllocated - 1))
		{
			//Circular buffer only holds ((m_nBytesAllocated - 1)) bytes.
			pBytes += (count - (m_nBytesAllocated - 1));
			count = (m_nBytesAllocated - 1);
		}

		numBytesAvail += count;
		if (numBytesAvail >= (m_nBytesAllocated - 1))
			bFullBuffer = true;

		testCount = m_nBytesAllocated - m_nNextByte;
		if (testCount > count)
			testCount = count;

		//Copy data up to the physical end of the buffer.
		memcpy(&m_pBytes[m_nNextByte], pBytes, testCount);
		m_nNextByte += testCount;
		if (m_nNextByte >= m_nBytesAllocated)
			m_nNextByte -= m_nBytesAllocated;

		count -= testCount;
		if (count > 0)
		{
			//Copy data that wraps around to the physical beginning of the buffer.
			pBytes += testCount;
			memcpy(&m_pBytes[m_nNextByte], pBytes, count);
			m_nNextByte += count;
			if (m_nNextByte >= m_nBytesAllocated)	//Should not happen!
				m_nNextByte -= m_nBytesAllocated;
		}

		if (bFullBuffer)
			m_nFirstByte = m_nNextByte + 1;
		if (m_nFirstByte == m_nBytesAllocated)
			m_nFirstByte = 0;

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}
	return bSuccess;
}

int GCircularBuffer::RetrieveBytes(unsigned char *pBytes, int count)//Remove count bytes from FIFO buffer.
{
	int numBytesRetrieved = 0;
	int numBytesRetrieved2 = 0;
	bool bSuccess = true;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		int testCount;
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		if (count < 0)
			count = 0;
		numBytesRetrieved = count;
		if (numBytesRetrieved > numBytesAvail)
			numBytesRetrieved = numBytesAvail;

		testCount = m_nBytesAllocated - m_nFirstByte;
		if (numBytesRetrieved > testCount)
			numBytesRetrieved = testCount; 

		if (numBytesRetrieved > 0)
		{
			//Copy data up to the physical end of the buffer.
			memcpy(pBytes, &m_pBytes[m_nFirstByte], numBytesRetrieved);
			m_nFirstByte += numBytesRetrieved;
			if (m_nFirstByte >= m_nBytesAllocated)
				m_nFirstByte -= m_nBytesAllocated;

			count -= numBytesRetrieved;
			if (count > 0)
			{
				numBytesAvail -= numBytesRetrieved;
				pBytes += numBytesRetrieved;

				numBytesRetrieved2 = count;
				if (numBytesRetrieved2 > numBytesAvail)
					numBytesRetrieved2 = numBytesAvail;

				if (numBytesRetrieved2 > 0)
				{
					//Copy data that wraps around to the physical beginning of the buffer.
					memcpy(pBytes, &m_pBytes[m_nFirstByte], numBytesRetrieved2);
					m_nFirstByte += numBytesRetrieved2;
					if (m_nFirstByte >= m_nBytesAllocated)	//Should not happen!
						m_nFirstByte -= m_nBytesAllocated;
				}
			}
		}

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}

	return (numBytesRetrieved + numBytesRetrieved2);
}

//Copy count bytes from buffer, starting with firstByteIndex'th byte.
//firstByteIndex == 0 => first byte in buffer. No bytes are removed from the buffer.
int GCircularBuffer::CopyBytes(unsigned char *pBytes, int firstByteIndex, int count)
{
	int numBytesCopied = 0;
	int numBytesCopied2 = 0;
	bool bSuccess = true;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		int testCount;
		int copyIndex = 0;
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		if (count < 0)
			count = 0;
		if (firstByteIndex < 0)
			firstByteIndex = 0;
		numBytesAvail = numBytesAvail - firstByteIndex;
		copyIndex = m_nFirstByte + firstByteIndex;
		if (copyIndex >= m_nBytesAllocated)
			copyIndex -= m_nBytesAllocated;

		numBytesCopied = count;
		if (numBytesCopied > numBytesAvail)
			numBytesCopied = numBytesAvail;

		testCount = m_nBytesAllocated - copyIndex;
		if (numBytesCopied > testCount)
			numBytesCopied = testCount; 

		if (numBytesCopied > 0)
		{
			//Copy data up to the physical end of the buffer.
			memcpy(pBytes, &m_pBytes[copyIndex], numBytesCopied);
			copyIndex += numBytesCopied;
			if (copyIndex >= m_nBytesAllocated)
				copyIndex -= m_nBytesAllocated;

			count -= numBytesCopied;
			if (count > 0)
			{
				numBytesAvail -= numBytesCopied;
				pBytes += numBytesCopied;

				numBytesCopied2 = count;
				if (numBytesCopied2 > numBytesAvail)
					numBytesCopied2 = numBytesAvail;

				if (numBytesCopied2 > 0)
				{
					//Copy data that wraps around to the physical beginning of the buffer.
					memcpy(pBytes, &m_pBytes[copyIndex], numBytesCopied2);
				}
			}
		}

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}

	return (numBytesCopied + numBytesCopied2);
}

int GCircularBuffer::NumBytesAvailable()
{
	bool bSuccess = true;
	int numBytesAvail = 0;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}
	return numBytesAvail;
}

int GCircularBuffer::MaxNumBytesAvailable()
{
	return (m_nBytesAllocated - 1);
}

bool GCircularBuffer::Clear()
{
	bool bSuccess = true;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		m_nFirstByte = 0;
		m_nNextByte = 0;

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}
	return bSuccess;
}

unsigned int GCircularBuffer::GetTotalBytesAdded()//diagnostic
{
	bool bSuccess = true;
	unsigned int nTotalBytesAdded = 0;
	EThreadPriority oldPriority;
	if (m_pQueueAccessMutex != NULL)
		bSuccess = m_pQueueAccessMutex->TryLockMutex(SHARED_CIRCULAR_BUFFER_TIMEOUT_MS, &oldPriority);

	if (bSuccess)
	{
		nTotalBytesAdded = m_nTotalBytesAdded;

		if (m_pQueueAccessMutex != NULL)
			m_pQueueAccessMutex->UnlockMutex(oldPriority);
	}
	return nTotalBytesAdded;
}

#ifdef LIB_NAMESPACE
}
#endif
