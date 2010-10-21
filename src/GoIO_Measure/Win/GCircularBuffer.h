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
// GCircularBuffer.h
#ifndef _GCIRCULARBUFFER_H_
#define _GCIRCULARBUFFER_H_

class GCircularBuffer
{
public:
	GCircularBuffer(int numBytes);
	virtual ~GCircularBuffer();

	bool AddBytes(unsigned char *pBytes, int count);	//Add count bytes to FIFO buffer.
	int RetrieveBytes(unsigned char *pBytes, int count);//Remove count bytes from FIFO buffer.
	int CopyBytes(unsigned char *pBytes, int firstByteIndex, int count);//Copy count bytes from buffer, starting with firstByteIndex'th byte.
								//firstByteIndex == 0 => first byte in buffer. No bytes are removed from the buffer.
	int NumBytesAvailable();
	int MaxNumBytesAvailable();
	bool Clear();
	unsigned int GetTotalBytesAdded();//diagnostic

protected:
	unsigned char *m_pBytes;
	int	m_nFirstByte;
	int m_nNextByte;
	int m_nBytesAllocated;
	unsigned int m_nTotalBytesAdded;
};

#endif // _GCIRCULARBUFFER_H_

