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

GCircularBuffer::GCircularBuffer(
	int numBytes)//number of bytes that buffer holds before it starts to 'roll over'.
{
	m_pBytes = new unsigned char[numBytes+1];
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
	if (bSuccess)
	{
		int i;
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;
		numBytesAvail += count;
		if (numBytesAvail >= (m_nBytesAllocated - 1))
		{
			//Buffer is filling up, so do this the slow way.
			numBytesAvail -= count;
			for (i = 0; i < count; i++)
			{
				if (numBytesAvail >= (m_nBytesAllocated - 1))
				{
					//Buffer is full, so advance first record index.
					//Note that even though space for m_nBytesAllocated Bytes exists, we only report available counts from 0 to (m_nBytesAllocated-1).
					m_nFirstByte++;
					if (m_nFirstByte == m_nBytesAllocated)
						m_nFirstByte = 0;
				}

				m_pBytes[m_nNextByte] = pBytes[i];
				m_nNextByte++;
				if (m_nNextByte == m_nBytesAllocated)
					m_nNextByte = 0;

				numBytesAvail++;
			}
		}
		else
		{
			int testIndex = m_nNextByte + count;
			if (testIndex >= m_nBytesAllocated)
			{
				//The buffer is going to wrap around.
				for (i = 0; i < count; i++)
				{
					m_pBytes[m_nNextByte] = pBytes[i];
					m_nNextByte++;
					if (m_nNextByte == m_nBytesAllocated)
						m_nNextByte = 0;
				}
			}
			else
			{
				//This is the typical case, and it is very fast.
				memcpy(&m_pBytes[m_nNextByte], pBytes, count);
				m_nNextByte = testIndex;
			}
		}

		m_nTotalBytesAdded += count;
	}
	return bSuccess;
}

int GCircularBuffer::RetrieveBytes(unsigned char *pBytes, int count)//Remove count bytes from FIFO buffer.
{
	int numBytesRetrieved = 0;
	bool bSuccess = true;
	if (bSuccess)
	{
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		numBytesRetrieved = count;
		if (numBytesRetrieved > numBytesAvail)
			numBytesRetrieved = numBytesAvail;

		int i;
		for (i = 0; i < numBytesRetrieved; i++)
		{
			pBytes[i] = m_pBytes[m_nFirstByte];
			m_nFirstByte++;
			if (m_nFirstByte == m_nBytesAllocated)
				m_nFirstByte = 0;
		}
	}

	return numBytesRetrieved;
}

//Copy count bytes from buffer, starting with firstByteIndex'th byte.
//firstByteIndex == 0 => first byte in buffer. No bytes are removed from the buffer.
int GCircularBuffer::CopyBytes(unsigned char *pBytes, int firstByteIndex, int count)
{
	int numBytesCopied = 0;
	bool bSuccess = true;
	if (bSuccess)
	{
		int index;
		int numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;

		numBytesAvail = numBytesAvail - firstByteIndex;
		for (numBytesCopied = 0; (numBytesCopied < count) && (numBytesAvail > 0); numBytesCopied++)
		{
			index = m_nFirstByte + firstByteIndex + numBytesCopied;
			if (index >= m_nBytesAllocated)
				index = index - m_nBytesAllocated;
			pBytes[numBytesCopied] = m_pBytes[index];
			numBytesAvail--;
		}
	}

	return numBytesCopied;
}

int GCircularBuffer::NumBytesAvailable()
{
	bool bSuccess = true;
	int numBytesAvail = 0;
	if (bSuccess)
	{
		numBytesAvail = m_nNextByte - m_nFirstByte;
		if (numBytesAvail < 0)
			numBytesAvail += m_nBytesAllocated;
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
	if (bSuccess)
	{
		m_nFirstByte = 0;
		m_nNextByte = 0;
	}
	return bSuccess;
}

unsigned int GCircularBuffer::GetTotalBytesAdded()//diagnostic
{
	return m_nTotalBytesAdded;
}
