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
// GUtils_Win.cpp
#include "stdafx.h"
#include "GUtils.h"
#include "GTextUtils.h"

/*
#include "WinOpus.h"
#include "WinCustomMessageDlg.h"
#include "WinAssertDlg.h"
#include "WinChildView.h"
#include "WinMainFrame.h"

#include <time.h>
#include <Mmsystem.h>

#include "GTextUtils.h"
#include "GMessenger.h"
#include "GApplicationBrain.h"
#include "GMainWindow.h"

// StackTrace stuff
#include "imageHlp.h"
#include "..\..\..\.\Utilities\Win32\StackTrace\StackTrace.h"

*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// The Following variables are extern'd in StackTrace.h THEY MUST MATCH!!!
bool gbPenter = false;
/*
struct ITrackInt ITrackInfo[STACKTRACE_MAX_THREADS];
DWORD arrIndex[STACKTRACE_MAX_THREADS];		// Array of thread IDs
int arrSize;	// How many valid IDs in above array
//--------------
*/
bool GUtils::Assert(bool bArgument,		// boolean expression to test
					const gchar * sFile,	// File name in calling source file
					int nLine)			// Line of call in calling source file
{ 
	// Log the attempt to assert Use this if you really have to -- it creates gigantic logfiles in no time...
//	cppstring sLog = sFile;
//	sLog = GTextUtils::StripPath(sLog);
//	sLog = "Assert: " + sLog + ": " + GTextUtils::LongToCPPString(nLine);
//	GSTD_LOG(sLog);
#ifndef _UNICODE	
	if (!bArgument)
	{ // If bArgument is false, message the file and line of caller then allow user to exit
static bool bAllowEntry = true;
		if (bAllowEntry)
		{
			bAllowEntry = false;	// prevent reentrancy problems

			gbPenter = false; // Prevent penter from being called for any subsequent functions
/*
			if (AfxGetMainWnd() != NULL)
			{ // Make sure all timers get killed (so we don't keep getting asserts)
				CWinChildView *pView = ((CWinMainFrame*)AfxGetMainWnd())->GetView();
				if (pView != NULL)
					pView->KillAllTimers();
			}

			// Stack trace address resolving -- use the hex function address from the StackTrace.lib to get the function name from the symbol file
			TCHAR moduleName[MAX_PATH];
			TCHAR modShortNameBuf[MAX_PATH];
			MEMORY_BASIC_INFORMATION mbi;
			BYTE symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 1024 ];
			PIMAGEHLP_SYMBOL pSymbol;
			pSymbol = (PIMAGEHLP_SYMBOL)&symbolBuffer[0];
			pSymbol->SizeOfStruct = sizeof(symbolBuffer);
			pSymbol->MaxNameLength = 1023;
			DWORD symDisplacement = 0;
			int nStackCurrentID = 0;
*/

			cppstring sStackTrace;
/*
			// Find the current thread
			DWORD currThread = GetCurrentThreadId(); 
			for (int i = 0; i < arrSize; i++)
				if (currThread == arrIndex[i])
					nStackCurrentID = i;

 			int endLoop = ITrackInfo[nStackCurrentID].size; // Size of the struct for current thread			
			for (int j = 0; j < endLoop; j++)
			{ // Resolve the function addresses for the current thread
   				SymInitialize(GetCurrentProcess(), NULL, FALSE);	// From MSDN imagehlp.lib
				VirtualQuery((void*)(ITrackInfo[nStackCurrentID].funcAddr[j]), &mbi, sizeof(mbi));	// MSDN
				GetModuleFileName((HMODULE)mbi.AllocationBase, moduleName, MAX_PATH );

				_splitpath(moduleName, NULL, NULL, modShortNameBuf, NULL);

				 SymLoadModule(GetCurrentProcess(), NULL, moduleName, NULL, (DWORD)mbi.AllocationBase, 0); // from imagehlp.lib
				 SymSetOptions(SymGetOptions() & ~SYMOPT_UNDNAME);  // from imagehlp.lib
    
				char undName[256];
				if (!SymGetSymFromAddr(GetCurrentProcess(), (ITrackInfo[nStackCurrentID].funcAddr[j]), &symDisplacement, pSymbol))
					strcpy(undName, "<unknown symbol>");	// Couldn't retrieve symbol (no debug info?)
				else
				{ // Unmangle name, throwing away decorations that don't affect uniqueness:
					if ( 0 == UnDecorateSymbolName(
							pSymbol->Name, undName,
							sizeof(undName),
							UNDNAME_NO_MS_KEYWORDS |
							UNDNAME_NO_ACCESS_SPECIFIERS |
							UNDNAME_NO_FUNCTION_RETURNS |
							UNDNAME_NO_ALLOCATION_MODEL |
							UNDNAME_NO_ALLOCATION_LANGUAGE |
							UNDNAME_NO_MEMBER_TYPE))
						strcpy(undName, pSymbol->Name);
				}
				SymUnloadModule(GetCurrentProcess(), (DWORD)mbi.AllocationBase);
				SymCleanup(GetCurrentProcess());
   
				// Add the function name to the string that will record the stack trace
				sStackTrace += GSTD_S(" ");
				sStackTrace += undName;
				sStackTrace += GSTD_S("\n");
			} // End of for 
*/
			GUtils::AssertDialog(sFile, nLine, sStackTrace);

#ifdef _DEBUG
			ASSERT(0);
#else
			exit(0);
#endif
		}
	}
#endif

	return bArgument;
}

void GUtils::OSAssertDialog(SAppError * pAppError)
{
	::MessageBox(NULL, pAppError->sErrorMessage.c_str(), NULL, MB_OK);
//	CWinAssertDlg dlg;
//	dlg.SetAppError(pAppError);
//	dlg.DoModal();
}

void GUtils::Beep(void)
{ // Your basic beep.
	::MessageBeep(-1);
}

//=====================================================================================
// The following overloaded methods simply display a message box given different inputs
void GUtils::MessageBox(const cppstring &msg)
{
//	StStopMBLActivity stopLiveReadouts;
//	CWinOpusApp *pApp = (CWinOpusApp *)AfxGetApp(); GSTD_ASSERT(pApp != NULL);
//	if (pApp->IsUserInteraction())
//	{
//		pApp->PushInAMessageBox();
		::AfxMessageBox(msg.c_str());
//		pApp->PopInAMessageBox();
//	}
}
/*
//=====================================================================================
// The following overloaded methods display a "Yes No" message box given different inputs
bool GUtils::YesNo(const cppstring &msg)
{
	StStopMBLActivity stopLiveReadouts;
	bool bReturn = true;
	CWinOpusApp *pApp = (CWinOpusApp *)AfxGetApp(); GSTD_ASSERT(pApp != NULL);
	if (pApp->IsUserInteraction())
	{
		pApp->PushInAMessageBox();
		bReturn = ::AfxMessageBox(msg.c_str(), MB_YESNO) == IDYES;
		pApp->PopInAMessageBox();
	}

	return bReturn;
}

//=====================================================================================
// The following overloaded methods display an "OK Cancel" message box given different inputs
bool GUtils::OKCancel(const cppstring &msg)
{
	StStopMBLActivity stopLiveReadouts;
	bool bReturn = true;
	CWinOpusApp *pApp = (CWinOpusApp *)AfxGetApp(); GSTD_ASSERT(pApp != NULL);
	if (pApp->IsUserInteraction())
	{
		pApp->PushInAMessageBox();
		bReturn = ::AfxMessageBox(msg.c_str(), MB_OKCANCEL) == IDOK;
		pApp->PopInAMessageBox();
	}

	return bReturn;
}

//=====================================================================================
// The following overloaded methods display a "Yes No Cancel" message box 
EYesNoCancel GUtils::YesNoCancel(const cppstring &msg)
{
	StStopMBLActivity stopLiveReadouts;
	EYesNoCancel eReturn = kYesNoCancel_Yes;
	CWinOpusApp *pApp = (CWinOpusApp *)AfxGetApp(); GSTD_ASSERT(pApp != NULL);
	if (pApp->IsUserInteraction())
	{
		pApp->PushInAMessageBox();
		switch (::AfxMessageBox(msg.c_str(), MB_YESNOCANCEL))
		{
			case IDYES:
				eReturn = kYesNoCancel_Yes;
				break;
			case IDNO:
				eReturn = kYesNoCancel_No;
				break;
			default:
				eReturn = kYesNoCancel_Cancel;
				break;
		}
		pApp->PopInAMessageBox();
	}

	return eReturn;
}

int GUtils::CustomMessageBox(cppstring sTitle, 
							 cppstring msg, 
							 StringVector *pvButtons,
							 int nDefaultButton)	// index for which button should be default
{ // Return one of three buttons that user pressed
	CWinCustomMessageDlg dlg;
	dlg.SetTitle(sTitle);
	dlg.SetCustomMessage(msg);
	if (pvButtons->size() > 0)
		dlg.SetButton1Text(pvButtons->at(0));
	if (pvButtons->size() > 1)
		dlg.SetButton2Text(pvButtons->at(1));
	if (pvButtons->size() > 2)
		dlg.SetButton3Text(pvButtons->at(2));

	return dlg.DoModal();
}
*/
void GUtils::OSTrace(const gchar * pString)
{
	if (GTextUtils::Gstrlen(pString) > 512)
	{
		gchar msg[512];
		memcpy(msg, pString, 511);
		msg[511] = 0;
		OutputDebugString(msg);
//		TRACE(msg);
	}
	else
		OutputDebugString(pString);
//		TRACE(pString);
}

/*
void GUtils::OSDeleteExceptionPtr(OSMemoryExceptionPtr p)
{ // Get rid of this object
	p->Delete();
}

void GUtils::OSDeleteExceptionPtr(OSFileExceptionPtr p)
{ // Get rid of this object
	p->Delete();
}

cppstring GUtils::OSGetExceptionErrorString(OSFileExceptionPtr p)
{ // Get an error string from the exception object describing this error
	gchar cError[256];
	p->GetErrorMessage(cError, 256);
	cppstring sError = cError;
	return sError;
}

cppstring GUtils::OSGetExceptionErrorString(OSMemoryExceptionPtr p)
{ // Get an error string from the exception object describing this error
	gchar cError[256];
	p->GetErrorMessage(cError, 256);
	cppstring sError = cError;
	return sError;
}

cppstring GUtils::OSGetBuildDateTimeText(void)
{ // RETURN the Date and Time this module was compiled
	CString sBuildDateTime = " ";
	sBuildDateTime += __DATE__; // Microsoft macro for the current date
	sBuildDateTime += ' ';
	sBuildDateTime += __TIME__;	// Microsoft macro for the current time

#if _DEBUG
	sBuildDateTime += " (Debug)"; // Append "(Debug)" if in debug mode
#endif

	return sBuildDateTime;
}

cppstring GUtils::OSChooseTextFileForImport(void)
{ // RETURN name of text file user wants to import
	cppstring sReturnName;

	// Let user specify a filename based on our default extensions
	cppstring sString = GSTD_STRING(IDSX_IMPORT_TEXT_FILE_EXTENSION);
	CString sDefaultExtension = sString.c_str();
	sString = GSTD_STRING(IDSX_IMPORT_TEXT_FILE_FILTER);
	CString sFilter = sString.c_str();
	CFileDialog openDlg(TRUE, sDefaultExtension, NULL, OFN_HIDEREADONLY, sFilter);
	if (openDlg.DoModal() == IDOK)
	{ // User pressed OK,
		CWaitCursor wait;
		sReturnName = openDlg.GetPathName();
	} 

	return sReturnName;
}

EKeyboardEvent GUtils::OSTranslateKeyboardEvent(void *pData)
{ // RETURN a cross-platform keyboard event
	EKeyboardEvent eEvent = kKeyboardEvent_NullEvent;
	char *pch = static_cast<char *>(pData);
	WORD wParam = 0x00FF & *pch;
	switch (wParam)
	{
		case VK_LEFT:
			eEvent = kKeyboardEvent_LeftArrow;
			break;
		case VK_TAB:
		case VK_RIGHT:
			eEvent = kKeyboardEvent_RightArrow;
			break;
		case VK_UP:
			eEvent = kKeyboardEvent_UpArrow;
			break;
		case VK_DOWN:
			eEvent = kKeyboardEvent_DownArrow;
			break;
		case VK_BACK:
			eEvent = kKeyboardEvent_Delete;
			break;
		case VK_ESCAPE:
			eEvent = kKeyboardEvent_Escape;
			break;
		case VK_RETURN:
			eEvent = kKeyboardEvent_Return;
			break;
	}

	return eEvent;
}

bool GUtils::OSIsQuickTimeAvailable(void)
{// this is os specific call to verify if QuickTime available, by making the call 
 // we also initialize quicktime (on windows)
	OSErr nError = InitializeQTML(0);
	// Note quick time returns 0 onsuccess so must change so returns true on sucess
	bool bReturn = false;
	if (nError ==0)
		bReturn = true;

	return bReturn;
}
*/
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

int	GUtils::OSConvertIntelIntToPlatformInt(int nSourceInt)
{
	return nSourceInt;
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
/*
cppstring GUtils::OSGetUserName(void)
{
	cppstring sResult = GSTD_STRING(IDSX_YOUR_NAME_HERE);
	LONG lRes;
	HKEY hKey;
	char szBuffer[256];
	CString fullKeyName = "Software\\Microsoft\\Windows\\CurrentVersion\\";
	lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fullKeyName, 0, KEY_READ, &hKey);

	cppstring sRegisteredOwner = GSTD_STRING(IDSX_REGISTERED_OWNER);
	
	if (lRes == ERROR_SUCCESS)
	{
		DWORD dwBytes = 256;
		if (RegQueryValueEx(hKey, sRegisteredOwner.c_str(), NULL, NULL, (LPBYTE)szBuffer, &dwBytes) == ERROR_SUCCESS)
		{
			narrowstring s(szBuffer);
			sResult = WIDEN(s);
		}
		lRes = RegCloseKey(hKey);
		ASSERT(lRes == ERROR_SUCCESS);
	}

	return sResult;
}
*/
unsigned int GUtils::OSGetTimeStamp(void)
{ // Return a time stamp (in milliseconds)
	return GetTickCount();
}
/*
void GUtils::OSSetDefaultFolder(const GFileRef & theFolderRef)
{ // Set the system default folder to sNewFolder
	::SetCurrentDirectory(theFolderRef.GetFilePath().c_str());
}

real GUtils::OSGetSystemClockTime(void)
{
	return ((real)clock() / (real)CLOCKS_PER_SEC);
}
*/
void GUtils::OSSleep(unsigned int msToSleep)
{
	::Sleep(msToSleep);
}
/*
bool GUtils::OSIsMainThread(void)
{
	// gspam -- add a way to tell is this is the main thread...
	return false;
}

void GUtils::OSFloatingPointExceptionReset()
{// 
	_fpreset();
}

bool GUtils::OSAllowThreadedFirmwareUpdate(void)
{
	return true;
}
*/