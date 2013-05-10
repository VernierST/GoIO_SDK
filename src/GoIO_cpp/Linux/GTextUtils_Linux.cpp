// GTextUtils_Linux.cpp



#include "GTextUtils.h"

#include "GUtils.h"

#include "GCharacters.h"





using namespace std;

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif


void GTextUtils::OSInitResourceStrings(void)

{

}





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

	return bResult;

}



bool GTextUtils::OSIsTextClipDataAvailable(int * pOutLength)

{

	return false;

}



bool GTextUtils::OSGetTextClipData(cppstring * pOutString)

{

	return false;

}



bool GTextUtils::OSPrintPlainText(const cppstring & sText, bool bShowJobDialog)

{

	return false;

}



short GTextUtils::OSApplyTextSize(short nNewSize, void *)

{

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

#ifdef LIB_NAMESPACE
}
#endif




