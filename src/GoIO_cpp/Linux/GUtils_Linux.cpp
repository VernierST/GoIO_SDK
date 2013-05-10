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
// GUtils_Linux.cpp

#include "GUtils.h"
#include "GTextUtils.h"
//#include <sys/timeb.h>
#include <sys/time.h>
#include <errno.h>

using namespace std;

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

namespace { // local namespace
const char * local_makeAssertString(const char * sFile, int nLine, char * pBuffer);
const char * local_makeAssertString(const char * sFile, int nLine, char * pBuffer)
{
	sprintf(pBuffer, "%s Line: %d", sFile, nLine);
	return pBuffer;
}

}

bool GUtils::Assert(bool bArgument, const char * sFile, int nLine)
{
	if(!bArgument)
	{
		char pBuffer[256];
		printf("%s\n", local_makeAssertString(sFile, nLine, pBuffer));
	}
	return bArgument;
}

void GUtils::OSAssertDialog(SAppError * pAppError)
{
}

void GUtils::Beep(void)
{
	// Your basic beep.
        // jentodoi
        printf("\a");
}

void GUtils::MessageBox(const cppstring &msg)
{
       // jentodo
}

bool GUtils::YesNo(const cppstring &msg)
{
        // jentodo
	return false;
}

bool GUtils::OKCancel(const cppstring &msg)
{
        // jentodo
	return false;
}

EYesNoCancel GUtils::YesNoCancel(const cppstring &msg)
{
        // jentodo
	return kYesNoCancel_Cancel;
}

int GUtils::CustomMessageBox(cppstring /*sTitle*/, cppstring msg, StringVector * pvButtonTitles, int nDefaultButton)
{
	return 0;
}

void GUtils::OSTrace(const char *pText)
{
	printf("%s\n", pText);
}

void GUtils::OSDeleteExceptionPtr(OSMemoryExceptionPtr)
{
}

void GUtils::OSDeleteExceptionPtr(OSFileExceptionPtr)
{
}

cppstring GUtils::OSGetExceptionErrorString(OSMemoryExceptionPtr)
{
	cppstring sError = "Memory exception.";
	return sError;
}

cppstring GUtils::OSGetExceptionErrorString(OSFileExceptionPtr)
{
	cppstring sError = "Truly exceptional.";
	return sError;
}

cppstring GUtils::OSGetBuildDateTimeText(void)
{
	cppstring sBuildDate = __DATE__;
	sBuildDate += " ";
	sBuildDate += __TIME__;


	return sBuildDate;
}

cppstring GUtils::OSChooseTextFileForImport(void)
{
	return cppstring();
}

EKeyboardEvent GUtils::OSTranslateKeyboardEvent(void * pData)
{ // RETURN a cross-platform keyboard event
	EKeyboardEvent eEvent = kKeyboardEvent_NullEvent;
	if (pData != NULL)
		eEvent = *static_cast<EKeyboardEvent *>(pData);

	return eEvent;
}

bool GUtils::OSConvertShortToBytes(short nInNum, unsigned char * pLSB, unsigned char * pMSB)
{
	if(!pLSB || !pMSB)
		return false;

	unsigned char * pNum = (unsigned char *) &nInNum;
	*pLSB = *pNum;
	pNum++;
	*pMSB = *pNum;
	return true;
}

bool GUtils::OSConvertBytesToShort(char chLSB, char chMSB, short * pOutShort)
{
	if(!pOutShort)
		return false;

	char * pAssign = (char *) pOutShort;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chMSB;
	return true;
}

bool GUtils::OSConvertIntToBytes(int nInNum, 
									unsigned char * pLSB,
									unsigned char * pLMidB,
									unsigned char * pMMidB,
									unsigned char * pMSB)
{
	if(!pLSB || !pLMidB || !pMMidB || !pMSB)
		return false;

	if(!GSTD_ASSERT(sizeof(int) == 4))
		return false;

	unsigned char * pNum = (unsigned char *) &nInNum;
	*pLSB = *pNum;
	pNum++;
	*pLMidB = *pNum;
	pNum++;
	*pMMidB = *pNum;
	pNum++;
	*pMSB = *pNum;
	return true;
}

bool GUtils::OSConvertBytesToInt(unsigned char chLSB, 
								   unsigned char chLMidB,
								   unsigned char chMMidB,
		   						   unsigned char chMSB, 
								   int * pOutInt)
{
	if(!pOutInt)
		return false;

	if(!GSTD_ASSERT(sizeof(int) == 4))
		return false;

	char * pAssign = (char *) pOutInt;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chLMidB;
	pAssign++;
	*pAssign = chMMidB;
	pAssign++;
	*pAssign = chMSB;
	return true;
}

bool GUtils::OSConvertFloatToBytes(float fInNum, 
									unsigned char * pLSB,
									unsigned char * pLMidB,
									unsigned char * pMMidB,
									unsigned char * pMSB)
{
	if(!GSTD_ASSERT(sizeof(float) == 4))
		return false;

	if(!pLSB || !pLMidB || !pMMidB || !pMSB)
		return false;

	unsigned char * pNum = (unsigned char *) &fInNum;
	*pLSB = *pNum;
	pNum++;
	*pLMidB = *pNum;
	pNum++;
	*pMMidB = *pNum;
	pNum++;
	*pMSB = *pNum;
	return true;
}

bool GUtils::OSConvertBytesToFloat(unsigned char chLSB, 
								   unsigned char chLMidB,
								   unsigned char chMMidB,
		   						   unsigned char chMSB, 
								   float * pOutFloat)
{
	if(!pOutFloat)
		return false;

	if(!GSTD_ASSERT(sizeof(float) == 4))
		return false;

	char * pAssign = (char *) pOutFloat;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chLMidB;
	pAssign++;
	*pAssign = chMMidB;
	pAssign++;
	*pAssign = chMSB;
	return true;
}

int	GUtils::OSConvertMacIntToPlatformInt(int nSourceInt)
{
	int nDestInt;
	char *pSourceInt = (char *) &nSourceInt;
	char *pDestInt = (char *) &nDestInt;

	pDestInt[3] = pSourceInt[0];
	pDestInt[2] = pSourceInt[1];
	pDestInt[1] = pSourceInt[2];
	pDestInt[0] = pSourceInt[3];

	return nDestInt;
}

int	GUtils::OSConvertIntelIntToPlatformInt(int nSourceInt)
{
	int nDestInt;
	char *pSourceInt = (char *) &nSourceInt;
	char *pDestInt = (char *) &nDestInt;

	pDestInt[3] = pSourceInt[0];
	pDestInt[2] = pSourceInt[1];
	pDestInt[1] = pSourceInt[2];
	pDestInt[0] = pSourceInt[3];

	return nDestInt;
}


cppstring GUtils::OSGetUserName(void)
{
	cppstring sName;

	// REVISIT - how do we get user-name under OS X? (BSD Function or Carbon?)

	return sName;
}

#ifndef OPUS_DDK
void GUtils::OSSetDefaultFolder(const GFileRef &)
{
	// this is not needed on the mac
}
#endif

bool GUtils::OSIsQuickTimeAvailable(void)
{
	int version=false;


	return version;
}

bool GUtils::OSAllowThreadedFirmwareUpdate(void)
{
	// return UEnvironment::IsRunningOSX();
	return true;
}

real GUtils::OSGetSystemClockTime(void)
{
	return clock();
}

unsigned int GUtils::OSGetTimeStamp()
{
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  unsigned long long timeMs = currentTime.tv_sec;
  timeMs = timeMs*1000;
  timeMs += currentTime.tv_usec/1000;
  return (unsigned int) timeMs;
}

void GUtils::OSSleep(unsigned int msToSleep)
{
  struct timeval tv;
  unsigned int usToSleep = msToSleep*1000;
  tv.tv_sec = usToSleep/1000000;
  tv.tv_usec = usToSleep % 1000000;
  select (0, NULL, NULL, NULL, &tv);
}

bool GUtils::OSIsMainThread(void)
{
	// return (MPCurrentTaskID() == MMacApp::sMainTaskID);
	return true;
}

void GUtils::OSFloatingPointExceptionReset(void)
{ // stub
}

#ifdef LIB_NAMESPACE
}
#endif
