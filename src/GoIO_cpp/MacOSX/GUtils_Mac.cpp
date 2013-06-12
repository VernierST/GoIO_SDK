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
// GUtils_Mac.cpp

#include "GUtils.h"

// #include <UException.h>
// #include <LString.h>
// #include <UStandardDialogs.h>
#include <Carbon/Carbon.h>
#undef TARGET_OS_MAC

// #include "GApplicationBrain.h"
#include "GTextUtils.h"
// #include "GFileRef.h"

// #include "MMacApp.h"
// #include "MMessageBox.h"
// #include "MDialogBox.h"
// #include "MResourceConstants.h"

using namespace std;

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
	/* pAppError->sErrorMessage = GTextUtils::StringReplace(pAppError->sErrorMessage, "\n", "\r");
	if (!UEnvironment::IsRunningOSX())
		pAppError->sURL = GTextUtils::StringReplace(pAppError->sURL, "\n", "\r");
	MDialogBox::ShowModal(kPPob_InternalErrorDialog, static_cast<void *>(pAppError)); */
}

void GUtils::Beep(void)
{
}

void GUtils::MessageBox(const cppstring &msg)
{
	// MMessageBox::Show(msg);
}

bool GUtils::YesNo(const cppstring &msg)
{
	// return (MMessageBox::Show(msg, kMBoxType_YesNo) == kYesNoCancel_Yes);
	return false;
}

bool GUtils::OKCancel(const cppstring &msg)
{
	// return (MMessageBox::Show(msg, kMBoxType_OKCancel) == kYesNoCancel_Yes);
	return false;
}

EYesNoCancel GUtils::YesNoCancel(const cppstring &msg)
{
	// return (EYesNoCancel) MMessageBox::Show(msg, kMBoxType_YesNoCancel);
	return kYesNoCancel_Cancel;
}

int GUtils::CustomMessageBox(cppstring /*sTitle*/, cppstring msg, StringVector * pvButtonTitles, int nDefaultButton)
{
	// return MMessageBox::Show(msg, kMBoxType_Custom, pvButtonTitles, nDefaultButton);
	return 0;
}

void GUtils::OSTrace(const char *pText)
{
	printf(pText);
/* #ifndef OPUS_DDK
	::GetMacApp()->Trace(pText);
#else
	::GetDDKBrain()->OSOutputText(pText);
#endif */
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
/* #ifdef OPUS_DDK
	cppstring s;
	return s;
#else
	// returns name of text file user wants to import
	
	bool bResult;
	cppstring sFileName;
	GFileRef pSelectedFile;
	
    // put up the system file dialog to get the filename
	bResult = GFileRef::OSUserSelectFile( &pSelectedFile, true );
		
	if (bResult)
		sFileName = pSelectedFile.GetFilePath();
		
	return sFileName;
#endif	*/
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
#if TARGET_RT_LITTLE_ENDIAN
	unsigned char * pNum = (unsigned char *) &nInNum;
	*pLSB = *pNum;
	pNum++;
	*pMSB = *pNum;
#else
	unsigned char * pNum = (unsigned char *) &nInNum;
	*pMSB = *pNum;
	pNum++;
	*pLSB = *pNum;
#endif
	return true;
}

bool GUtils::OSConvertBytesToShort(char chLSB, char chMSB, short * pOutShort)
{
	if(!pOutShort)
		return false;
		
#if TARGET_RT_LITTLE_ENDIAN
	char * pAssign = (char *) pOutShort;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chMSB;
#else
	char * pAssign = (char *) pOutShort;
	*pAssign = chMSB;
	pAssign++;
	*pAssign = chLSB;
#endif
	return true;
}

bool GUtils::OSConvertIntToBytes(int nInNum,
									unsigned char * pLSB,
									unsigned char * pLMidB,
									unsigned char * pMMidB,
									unsigned char * pMSB)
{
	if (!pMSB || !pMMidB || !pLMidB || !pLSB)
		return false;
	
	if(!GSTD_ASSERT(sizeof(int) == 4))
		return false;
	// If this assert ever hits, this routine should be revised - pack
	// additional high-sig bytes with zeros
	
#if TARGET_RT_LITTLE_ENDIAN	
	unsigned char * pNum = (unsigned char *) &nInNum;
	*pLSB = *pNum;
	pNum++;
	*pLMidB = *pNum;
	pNum++;
	*pMMidB = *pNum;
	pNum++;
	*pMSB = *pNum;
#else
	unsigned char * pNumPtr = (unsigned char *) &nInNum;
	*pMSB = *pNumPtr;
	pNumPtr++;
	*pMMidB = *pNumPtr;
	pNumPtr++;
	*pLMidB = *pNumPtr;
	pNumPtr++;
	*pLSB = *pNumPtr;
#endif	
	return true;
}
														
bool GUtils::OSConvertBytesToInt(unsigned char chLSB, 
								unsigned char chLMidB,
								unsigned char chMMidB,
								unsigned char chMSB,
								int * pOutInt)
{
	if (!pOutInt)
		return false;
	
	if(!GSTD_ASSERT(sizeof(int) == 4))
		return false;
		
	// If this assert ever hits, this routine should be revised - pack
	// additional high-sig bytes with zeros

#if TARGET_RT_LITTLE_ENDIAN	
	char * pAssign = (char *) pOutInt;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chLMidB;
	pAssign++;
	*pAssign = chMMidB;
	pAssign++;
	*pAssign = chMSB;
#else
	unsigned char * pNumByte = (unsigned char *) (pOutInt);
	*pNumByte = chMSB;
	pNumByte++;
	*pNumByte = chMMidB;
	pNumByte++;
	*pNumByte = chLMidB;
	pNumByte++;
	*pNumByte = chLSB;
#endif
	
	return true;
}

bool GUtils::OSConvertFloatToBytes(float fInNum,
									unsigned char * pLSB,
									unsigned char * pLMidB,
									unsigned char * pMMidB,
									unsigned char * pMSB)
{
	if (!pMSB || !pMMidB || !pLMidB || !pLSB)
		return false;
	
	if(!GSTD_ASSERT(sizeof(float) == 4))
		return false;
	
#if TARGET_RT_LITTLE_ENDIAN	
	unsigned char * pNum = (unsigned char *) &fInNum;
	*pLSB = *pNum;
	pNum++;
	*pLMidB = *pNum;
	pNum++;
	*pMMidB = *pNum;
	pNum++;
	*pMSB = *pNum;
#else
	unsigned char * pNumPtr = (unsigned char *) &fInNum;
	*pMSB = *pNumPtr;
	pNumPtr++;
	*pMMidB = *pNumPtr;
	pNumPtr++;
	*pLMidB = *pNumPtr;
	pNumPtr++;
	*pLSB = *pNumPtr;
#endif
	return true;
}
														
bool GUtils::OSConvertBytesToFloat(unsigned char chLSB, 
								unsigned char chLMidB,
								unsigned char chMMidB,
								unsigned char chMSB,
								float * pOutFloat)
{
	if (!pOutFloat)
		return false;
	
	if(!GSTD_ASSERT(sizeof(float) == 4))
		return false;
		
	// If this assert ever hits, this routine should be revised - pack
	// additional high-sig bytes with zeros
#if TARGET_RT_LITTLE_ENDIAN	
	char * pAssign = (char *) pOutFloat;
	*pAssign = chLSB;
	pAssign++;
	*pAssign = chLMidB;
	pAssign++;
	*pAssign = chMMidB;
	pAssign++;
	*pAssign = chMSB;
#else
	unsigned char * pNumByte = (unsigned char *) (pOutFloat);
	*pNumByte = chMSB;
	pNumByte++;
	*pNumByte = chMMidB;
	pNumByte++;
	*pNumByte = chLMidB;
	pNumByte++;
	*pNumByte = chLSB;
#endif	
	return true;
}

int	GUtils::OSConvertIntelIntToPlatformInt(int nSourceInt)
{
#if TARGET_RT_LITTLE_ENDIAN
	return nSourceInt;
#else
	int nDestInt;
	char *pSourceInt = (char *) &nSourceInt;
	char *pDestInt = (char *) &nDestInt;

	pDestInt[3] = pSourceInt[0];
	pDestInt[2] = pSourceInt[1];
	pDestInt[1] = pSourceInt[2];
	pDestInt[0] = pSourceInt[3];
	return nDestInt;
#endif
}

int	GUtils::OSConvertMacIntToPlatformInt(int nSourceInt)
{
#if TARGET_RT_LITTLE_ENDIAN	
	int nDestInt;
	char *pSourceInt = (char *) &nSourceInt;
	char *pDestInt = (char *) &nDestInt;

	pDestInt[3] = pSourceInt[0];
	pDestInt[2] = pSourceInt[1];
	pDestInt[1] = pSourceInt[2];
	pDestInt[0] = pSourceInt[3];

	return nDestInt;
#else
	return nSourceInt;
#endif
}

cppstring GUtils::OSGetUserName(void)
{
	cppstring sName;
	
#ifndef TARGET_API_MAC_CARBON
	const ResIDT kSystemUserNameStringResourceID = -16096;
	// Source: IM:More Mac. Toolbox:Ch.1 - Resource Mgr:Reference:Resources in the System File
	
	StringHandle hString = ::GetString(kSystemUserNameStringResourceID);
	
	GSTD_ASSERT(hString != NULL);
	StHandleLocker locker((Handle)hString); // don't lock/unlock string if it's already locked
	
	LStr255 sStr255((ConstStringPtr)*hString);
	sName = LString::PToCStr(sStr255);
	
	// Do not release this hString resource, the system is using it
#endif
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
//	long version;
//	OSStatus err;
//	
//	err = ::Gestalt(gestaltQuickTimeVersion, &version);
//	if (err == noErr && version >= 0x03008000)
//		return true;
//	
	return false;
}

bool GUtils::OSAllowThreadedFirmwareUpdate(void)
{
	// return UEnvironment::IsRunningOSX();
	return true;
}

real GUtils::OSGetSystemClockTime(void)
{
	return ::GetCurrentEventTime();
}

unsigned int GUtils::OSGetTimeStamp()
{
	return (unsigned int)fmod(GetCurrentEventTime() * 1000.0, 0x10000000);
}

void GUtils::OSSleep(unsigned int msToSleep)
{
	AbsoluteTime absTime = ::AddDurationToAbsolute(msToSleep * durationMillisecond, ::UpTime());
	::MPDelayUntil(&absTime);
}

bool GUtils::OSIsMainThread(void)
{
	// return (MPCurrentTaskID() == MMacApp::sMainTaskID);
	return true;
}

void GUtils::OSFloatingPointExceptionReset(void)
{ // stub
}