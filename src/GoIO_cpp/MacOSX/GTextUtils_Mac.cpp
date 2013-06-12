// GTextUtils_Mac.cpp

#include "GTextUtils.h"

// #include "GApplicationBrain.h"
// #include "GDrawing.h"
#include "GUtils.h"
#include "GCharacters.h"

// #include "MMacApp.h"
// #include "MResourceConstants.h"
// #include "MUtils.h"

// #include <UScrap.h>

using namespace std;

void GTextUtils::OSInitResourceStrings(void)
{
	/* unsigned int nTimeStamp = GUtils::OSGetTimeStamp();
	bool bResult = false;
	cppstring sFileName = GetMacApp()->GetAppFolderPath() + "Contents:Resources:strings.txt";
	#ifdef _DEBUG
		 // The debug version first checks for files in the appropriate path
		cppstring sAppPath = GetMacApp()->GetAppFolderPath();
		int npos = sAppPath.find("Product_LP:MacOS:Output:");
		if (npos != cppstring::npos)
		{
			cppstring sOpusPath = sAppPath.substr(0, npos);
			bResult = ReadStringsFromFile(sOpusPath + "Core:Common:CoreStrings.txt");
			bResult &= ReadStringsFromFile(sOpusPath + "Product_LP:Common:LPStrings.txt");
		}
		
		if (!bResult)
		{
			m_mStringIndices.clear();
			m_vStringData.clear();
			bResult = ReadStringsFromFile(sFileName);
		}
	#else
		bResult = ReadStringsFromFile(sFileName);
	#endif
	if (!bResult)
	{
		GUtils::MessageBox("Error opening strings file: " + sFileName);
		GSTD_ASSERT(false);
	}
	
	// Now loop through the strings and do OS-specific processing
	cppstring sAmpersand = GSTD_STRING(IDSX_AMPERSAND_SUB);
	cppstring sOKDone = GSTD_STRING(IDSX_OK_DONE_SUB);
	cppstring sDone = GSTD_STRING(IDSX_DONE_REPLACEMENT);
	cppstring sDegreeCode = GSTD_S("&#176;");
	cppstring sDegree = kDegreeChar;
	cppstring sAppName = GApplicationBrain::GetApplicationName();
	cppstring sAppSub = GSTD_STRING(IDSX_APPNAME_SUB);

	for (size_t ix = 0; ix < m_vStringData.size(); ix++)
	{
		cppstring sText = m_vStringData[ix];
		
		// Convert all newlines to OS specific
		if (!UEnvironment::IsRunningOSX())
			sText = GTextUtils::StringReplace(sText, "\n", "\r");
		
		// Revisit: find a faster way to convert the file
		// Convert Windows-1252 to MacRoman
		// sText = MUtils::ConvertStringEncodingToMac(sText);
		
		// Now replace IDSX_AMPERSAND_SUB
		sText = GTextUtils::StringReplace(sText, sAmpersand, GSTD_S(""));
		
		// Replace special characters
		sText = GTextUtils::StringReplace(sText, sDegreeCode, sDegree);

		sText = GTextUtils::StringReplace(sText, GSTD_S("&#153;"), GSTD_S("ª"));

		// Now convert "..." to "É"
		sText = GTextUtils::StringReplace(sText, GSTD_S("..."), GSTD_S("É"));

		// Replace %app (IDSX_APPNAME_SUB) with the actual application name
		sText = GTextUtils::StringReplace(sText, sAppSub, sAppName);

		// Now loop through and replace IDSX_OK_DONE_SUB
		m_vStringData[ix] = GTextUtils::StringReplace(sText, sOKDone, sDone);
	}
	
	cppsstream ss;
	ss << "Init: Reading strings file: " << (GUtils::OSGetTimeStamp() - nTimeStamp) << " ms";
	GSTD_TRACE(ss.str()); */
}

/* bool GTextUtils::OSGetOSStringByKey(const char * sKey,		// ID of string to get
									OSStringPtr pOSString)	// [out] return system string here
{
	StringPtr pStr = (StringPtr) pOSString;
	cppstring s = GTextUtils::GetStringByKey(sKey);
	if (!s.empty() && (s.length() < 256))
	{
		*pStr = (unsigned char)s.length();
		strncpy((char *) pStr+1, s.c_str(), s.length());
		return true;
	}
	return false;
} */

/* bool GTextUtils::OSGetOSStringByIndex(int nID,					// ID of string to get
									  OSStringPtr pOSString)	// [out] return system string here
{
	StringPtr pStr = (StringPtr) pOSString;
	cppstring s = GTextUtils::GetStringByIndex(nID);
	if (!s.empty() && (s.length() < 256))
	{
		*pStr = (unsigned char)s.length();
		strncpy((char *) pStr+1, s.c_str(), s.length());
		return true;
	}
	return false;
} */

cppstring GTextUtils::OSConvertReturnLineFeed(cppstring sText)
{ // RETURN sText with all line feed return pairs in the proper format for Mac
	// replace all windows \n\n\r (where does extra \n come from?) with just \n
	sText = GTextUtils::StringReplace(sText, "\n\n\r", kOSNewlineString);

	// and get any remaining line windows line endings
	sText = GTextUtils::StringReplace(sText, "\r\n", kOSNewlineString);
	sText = GTextUtils::StringReplace(sText, "\n", kOSNewlineString);

	return sText;
}

cppstring GTextUtils::OSGetPathSeparator(void)
{
	return ":";
}

bool GTextUtils::OSTextDataToClipboard(const cppstring & sText, bool bClearClipboard)
{
	bool bResult = true;
	/* if(sText.length() > 0)
	{
		narrowstring sAscii = GTextUtils::ConvertWideStringToNarrow(sText);
		try
		{
			UScrap::SetData(ResType_Text,
							sAscii.c_str(),
							sAscii.length(),
							bClearClipboard);
		}
		catch(LException)
		{
			bResult = false;
			::SysBeep(1);
		}
	} */
	return bResult;
}

bool GTextUtils::OSIsTextClipDataAvailable(int * pOutLength)
{
	/* SInt32 nDataLength = UScrap::GetData(ResType_Text, NULL);
	if(pOutLength)
		*pOutLength = nDataLength;
	return (nDataLength > 0); */
	return false;
}

bool GTextUtils::OSGetTextClipData(cppstring * pOutString)
{
	/* if(!pOutString)
		return false;
		
	SInt32 nDataLength = UScrap::GetData(ResType_Text, NULL);
	if(nDataLength > 0)
	{
		Handle hBuffer = ::NewHandle(nDataLength + 1);
		nDataLength = UScrap::GetData(ResType_Text, hBuffer);
		::HLock(hBuffer);
		(*hBuffer)[nDataLength] = '\0';
		*pOutString = *hBuffer;
		DisposeHandle(hBuffer);
	}
	return (pOutString->length() > 0); */
	return false;
}

bool GTextUtils::OSPrintPlainText(const cppstring & sText, bool bShowJobDialog)
{
	// VERY LIMITED FUNCTIONALITY AT PRESENT!
	// We don't even support printing more than one page of text...
	/* bool bDidPrint = false;
	bool bShowWarmFuzzy = true;
	
	if (!GetMacApp()->IsPrinterInitialized())
		GDrawing::InitializePrinter();

	// Open a printing context...
	bool bWeStartedSession = false;
	bool bCreatedSession = false;
	LPrintSpec & theSpec = UPrinting::GetAppPrintSpec();
	OSStatus err = noErr;
	
	PMPrintSession thePMSession = theSpec.GetPrintSession();
	if (thePMSession == NULL)
	{
		err = PMCreateSession(&thePMSession);
		if (err == noErr)
		{
			GSTD_ASSERT(thePMSession != NULL);
			bCreatedSession = true;
		}
	}
	
	if ((err == noErr) && (thePMSession != NULL))
	{
		if (!theSpec.IsInSession())
		{
			theSpec.BeginSession(thePMSession);
			err = theSpec.GetError();
			if (err == noErr)
				bWeStartedSession = true;
		}
	}

	if (bWeStartedSession)
	{
		Rect paperRect;
		theSpec.GetPaperRect(paperRect);	
		Rect drawRect = paperRect;
		
		// We always use 1/2" margins...
		::InsetRect(&drawRect, GDrawing::GetPrinterDPI() / 2, GDrawing::GetPrinterDPI() / 2);
		
		// We want a fresh print settings
		::PMSessionDefaultPrintSettings(thePMSession, theSpec.GetPrintSettings());		
		::PMSetPageRange(theSpec.GetPrintSettings(), 1, 1);
		
		bool bOKToPrint;
		
		if (!bShowJobDialog || UPrinting::AskPrintJob(theSpec))
		{
			StPrintContext * pPrintContext;
			bool bTryAgain = false;
			
			do
			{
				try
				{
					GSTD_NEW(pPrintContext, (StPrintContext *), StPrintContext(theSpec));
					pPrintContext->BeginPage();
					bTryAgain = false;
					bOKToPrint = true;
				}
				
				catch (...)
				{
					pPrintContext = NULL;
					
					// This probably means there is no current printer set up
					if (!bShowJobDialog)
					{
						bShowWarmFuzzy = false;
						bOKToPrint = UPrinting::AskPrintJob(theSpec);
						if (bOKToPrint)
							bTryAgain = !bTryAgain;
					}
				}
			} while (bTryAgain);
			
			if (bOKToPrint && (pPrintContext != NULL))
			{
				if (pPrintContext->GetGrafPtr())
				{
					SInt16 nJust = UTextTraits::SetPortTextTraits(kTxtr_Courier12);
					::TextSize(10);
					real fScale = GDrawing::GetPrinterDPI() / GDrawing::GetMonitorDPI();
					if (fScale > 0.0 && fScale != 1.0)
						::TextSize((fScale * 10.0)+0.5);
					UTextDrawing::DrawWithJustification(const_cast<char *> (sText.c_str()), sText.length(), drawRect, nJust);
				}
				pPrintContext->EndPage();
				delete pPrintContext;
				bDidPrint = true;
			}
		}
		
		// End printing session...
		if (bWeStartedSession && theSpec.IsInSession())
			theSpec.EndSession();
	}
	else
		MUtils::HandleOSError(err);
	
	if (bCreatedSession && (thePMSession != NULL))
		PMRelease(thePMSession);
		
	return (bDidPrint && bShowWarmFuzzy); */
	return false;
}

short GTextUtils::OSApplyTextSize(short nNewSize, void *)
{
	// Change the text size
	// ::TextSize(nNewSize);	
	// return nNewSize;
	return 0;
}

bool GTextUtils::OSGetLine(cppsstream * pInStream, cppstring * pOutString)
{
	bool bOK = false;
	if (std::getline(*pInStream, *pOutString, kOSNewlineChar))
	{
		bOK = true;
		// getline paranoia - make sure we don't have any newlines
		*pOutString = GTextUtils::StringReplace(*pOutString, "\r", "");
		*pOutString = GTextUtils::StringReplace(*pOutString, "\n", "");
	}
	
	return bOK;
}

int	GTextUtils::OSGetTextLength(cppstring sText)
{// using the currect font return the horizontal space needed to draw sText
	// return GDrawing::OSCalcTextWidth(NULL,sText);
	return 0;
}

// list of chars to map to web entities for outgoing text
StringVector * GTextUtils::GetOSOutgoingSpecialChars(void)
{
	if (kvOutgoingSpecialChar.size() == 0)
	{
		cppstring s;
		
		// its important to list ampersand FIRST so that if the user has entered an ampersand it is converted before we convert
		// other special chars.  When we switch back we convert the ampersand LAST so it doesn't get confused and read as part of
		// another entity.
		s += kAmpersandChar;
		kvOutgoingSpecialChar.push_back(s);
		s = GSTD_S("");
		
		s += kGreaterThanChar;
		kvOutgoingSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kLessThanChar;
		kvOutgoingSpecialChar.push_back(s);
		s = GSTD_S("");
	}
	
	return &kvOutgoingSpecialChar;
}

// list of web entities to map chars to
StringVector * GTextUtils::GetOSOutgoingSpecialCharCodes(void)
{
	if (kvOutgoingSpecialCharCode.size() == 0)
	{
		// its important to list ampersand FIRST so that if the user has entered an ampersand it is converted before we convert
		// other special chars.  When we switch back we convert the ampersand LAST so it doesn't get confused and read as part of
		// another entity.
		kvOutgoingSpecialCharCode.push_back(GSTD_S("&#38;")); // ampersand (could also be &amp;)
		
		kvOutgoingSpecialCharCode.push_back(GSTD_S("&#62;")); // '>' (could also be &gt;)
		kvOutgoingSpecialCharCode.push_back(GSTD_S("&#60;")); // '<' (could also be &lt;)
	}
	
	return &kvOutgoingSpecialCharCode;
}

cppstring  GTextUtils::OSGetNativeCharCodeForUTF8(unsigned short nUTF8Char)
{
	// Typically a Windows-charset or MacRoman character equivalent to a UTF-8 char code.
	// NOTE that if controls & text displays have UTF-8 support this OS method should
	// not do anything, assuming we're also using wide strings.
	
	cppstring  nChar = "";
	
	switch(nUTF8Char)
	{
		case GUTF8CharCode::kAmpersand:
			nChar += '&';
			break;
		case GUTF8CharCode::kCopyright:
			nChar += '©';
			break;
		case GUTF8CharCode::kRegistered:
			nChar += '¨';
			break;
		case GUTF8CharCode::kDegree:
			nChar += '¡';
			break;
		case GUTF8CharCode::kPlusOrMinus:
			nChar += '±';
			break;
		case GUTF8CharCode::kMu:
			nChar += 'µ';
			break;
		case GUTF8CharCode::kBullet:
			nChar += '¥';
			break;
		case GUTF8CharCode::kLowerCasePi:
			nChar += '¹';
			break;
		case GUTF8CharCode::kSuperScriptTwo:
			nChar += kSuperScriptTwo;
			break;
		default:
			break;
	}
	return nChar;
}

unsigned short GTextUtils::OSGetUTF8CharCodeForNative(unsigned short nMacRomanChar)
{
	unsigned short nChar = 0;
	
	switch((char)nMacRomanChar)
	{
		case '&':
			nChar = GUTF8CharCode::kAmpersand;
			break;
		case '©':
		 	nChar = GUTF8CharCode::kCopyright;
			break;
		case '¨':
		 	nChar = GUTF8CharCode::kRegistered;
			break;
		case '¡':
		 	nChar = GUTF8CharCode::kDegree;
			break;
		case '±':	
		 	nChar = GUTF8CharCode::kPlusOrMinus;
			break;
		case 'µ':
		 	nChar = GUTF8CharCode::kMu;
			break;
		case '¥':
		 	nChar = GUTF8CharCode::kBullet;
			break;
		case '¹':
		 	nChar = GUTF8CharCode::kLowerCasePi;
			break;
		default:
			break;
	}
	
	return nChar;
}


