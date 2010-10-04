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

