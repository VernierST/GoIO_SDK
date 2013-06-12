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
// GTextUtils.h
//
// Common text/string-related utilities, including
// text-format data importing facilities and types.
// NOTE CHANGE IN-PROGRESS: Many text-related methods
// currently in GUtils.h will be moved here in the near
// future.

#ifndef _GTEXTUTILS_H_
#define _GTEXTUTILS_H_

#include "GTypes.h"  // for cppsstream

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

#ifndef USE_WIDE_CHARS
	typedef  int gint;
#else
	typedef  wint_t gint;
#endif

class GTextUtils  
{
public:
	static void						OSInitResourceStrings(void);
	static bool						ReadStringsFromFile(cppstring sPath);
	static void						FreeResourceStrings(void);
	
	static int						GetStringIndexByKey(const char * sKey);
	static cppstring				GetStringByKey(const char * sKey);
	static std::string				GetNarrowStringByKey(const char * sKey);
	static bool						OSGetOSStringByKey(const char * sKey, OSStringPtr pOSString);
	
	static cppstring				GetStringByIndex(int nIndex);
	static std::string				GetNarrowStringByIndex(int nIndex);
	static bool						OSGetOSStringByIndex(int nIndex, OSStringPtr pOSString);

	static StringVector				GetRangeOfStrings(int nStartIndex, int nEndIndex);

	static bool						OSTextDataToClipboard(const cppstring & sText, bool bClearClipboard=true);
	static bool						OSIsTextClipDataAvailable(int * pOutLength = NULL);
	static bool						OSGetTextClipData(cppstring * pOutString);
//	static GDeviceRect				GetBestRectForText(cppstring sText, const GDeviceRect &maxRect, int kFontHeight, int nMinWidth = 450);
	
	static bool						OSPrintPlainText(const cppstring & sText, bool bShowJobDialog=false);

	static cppstring				RealToCPPString(real rNum, int nPrecision = 3, EFloatStyle eFloatStyle = kFloatStyle_default);
	static cppstring				RealToCPPString(real rNum, int nPrecision, bool bPrecisionDecimal);
	static cppstring				RealToDisplayCPPString(real rNum, int nDigits);
	static cppstring				RealToCPPStringRoundedToSigDigits(real rNum, int nSigDigits);

	static cppstring				LongToCPPString(int nNum);
	static std::string				LongToString(int nNum);

	static real						CPPStringToReal(const cppstring &sNumber);
	static real						StringToReal(const narrowstring &sNumber);
	static int						CPPStringToLong(const cppstring &sInteger);
	static int						StringToLong(const narrowstring &sInteger);

	static bool						IsStringRealNumber(cppstring sNumber, real *pfValue = NULL); // RETURN true if number is a real
	static bool						IsStringLong(const cppstring & sNumber, int *pnValue = NULL); // RETURN true if number is a int
	static bool						IsNarrowStringLong(const narrowstring & sNumber, int *pnValue = NULL); // RETURN true if number is a int
	static cppstring				StringReplace(cppstring sBase, const cppstring &sSearchFor, const cppstring &sReplaceWith); 
	static std::string				AsciiStringReplace(std::string sBase, const std::string &sSearchFor, const std::string &sReplaceWith); 

	static bool						StringIsAllDigits(const cppstring & sNumber);
	static cppstring				ExtractBaseName(const cppstring & sFullName, int * p_nOutNumberSuffix);
									// For a name like "Run 3", ExtractBaseName will return "Run' and p_nOutNumberSuffix will be set to "3"
	static real						GetVersionFromBaseName(cppstring sFilename, cppstring sBaseName);
	
	static cppstring				MatchSubString(const cppstring & sFullName, const cppstring & sMatchName, const cppstring & sSubName);
	
	static cppstring				ParenthesizeString(const cppstring &sInString);
	static narrowstring				ParenthesizeNarrowString(const narrowstring &sInString);

	static cppstring				FilterOutChars(const cppstring &sInString, const cppstring &sFilterChars);
	static cppstring				ConvertToUppercase(const cppstring &sInString);
	static bool						StringsEqualIgnoringCase(const cppstring &s1, const cppstring &s2);
	
	static EImportTextFormat		ParseImportFormat(ifstream * pInStream);
	static int						GetWordsPerLine(std::stringstream * pInStream);
	static cppstring				StripPathAndExtension(const cppstring &sFileAndPath);
	static cppstring				StripPath(const cppstring &sFileAndPath);
	static cppstring				GetExtension(const cppstring &sFileName);
	static cppstring				GetPath(const cppstring &sFileName);
	static cppstring				OSGetPathSeparator(void);
	static bool						InvalidDataWorldName(const cppstring &sName);
	// standard c string library functions with wide versions
	static gchar *					Gstrcpy(gchar *strDestination,const gchar * strSource );
	static gchar *					Gstrncpy(gchar *strDestination,const gchar * strSource, size_t nCount);
	static int						Gstrcmp(const gchar *str1,const gchar *str2);
	static int						Gstrncmp(const gchar *str1,const gchar *str2, size_t nCount);
	static int						Gstrtol(const gchar *nptr, gchar **endptr, int base);
	static double					Gstrtod(const gchar *nptr, gchar **endptr);
	static size_t					Gstrlen(const gchar * str );
	static int						Gisspace(const gint c);
	static int						Gisalpha (const gint c);
	static int						Gisprint(const gint c);
	static gint						GEOF();
	static	bool					IsEOF(int i);

	static cppstring				ConvertNarrowStringToWide(const std::string  sNarrow);
	static std::string				ConvertWideStringToNarrow(const cppstring  sWide);

	static unsigned short			GetValueFromCharacterEscape(cppstring sCharEscape);
									// sCharEscape should be a standard XML/HTML escape sequence in one of two formats:
									//  &#N;  or  &#xN;
									// In the first case, nnn is a base-10 integer; in the second case, a hexadecimal integer.
									// The method simply converts the value to an integer correctly according to base (presence of the "x"
									// indicated hexadecimal).
	
	static cppstring				EncodeCharacterEscape(unsigned short nUTF8Char);
									
	static cppstring				OSGetNativeCharCodeForUTF8(unsigned short nUTF8Char);
									// Typically a Windows-charset or MacRoman character equivalent to a UTF-8 char code.
									// NOTE that if controls & text displays have UTF-8 support this OS method should
									// not do anything, assuming we're also using wide strings.
									
	static unsigned short			OSGetUTF8CharCodeForNative(unsigned short nNativeChar);								
									// REVISIT - the methods below use a narrow decimal-string-escape method of character conversion
									// which should be abandoned in favor of the methods above
									
	static cppstring				ConvertToOSSpecialChars(cppstring s); // go through s and convert "&SpecialCharxx" to the xx'th special OS character
	static cppstring				ConvertFromOSSpecialChars(cppstring s); // go through s and convert any OS special chars to "&SpecialCharxx"
	static StringVector	*			GetOSSpecialChars(void);   // list of all possible web entities (#&###;) to map from incoming text
	static StringVector	*			GetOSSpecialCharCodes(void);  // list of Local char map chars/strings to map web entities TO

	static StringVector *			GetOSOutgoingSpecialChars(void); // list of chars to map to web entities for outgoing text
	static StringVector *			GetOSOutgoingSpecialCharCodes(void); // list of web entities to map chars to

	static const cppstring			kSpecialCharFlag;

	static cppstring				OSConvertReturnLineFeed(cppstring sText);	// do OS-Specific new line conversion
	static bool						OSGetLine(cppsstream * pInStream, cppstring * pOutString);
	static int						OSGetTextLength(cppstring sText);
	static short					AdjustTextSizeByAppPref(short nOrigSize, bool bApplyNewSize=true, void * pOSData=NULL);
									// Routine to nudge text sizes up if user preferences
									// so dictate.  The return value is the new font pt. size.
									// If bApplyNewSize is true (default) this routine actually 
									// applies the larger text size for immediate drawing; pOSData 
									// can be used as needed for draw-releated data (e.g. a draw context).
									
	static short					OSApplyTextSize(short nNewSize, void * pOSData=NULL);
									// Apply new text size for text drawing.  pOSData can be
									// used for a draw context etc.


	static narrowstring				RemoveSameRoot(AsciiStringVector *pvStrings); // if all strings have the same root (i.e. text before the last '\') then remove it (and return it)

	static EFloatStyle				GetFloatStyle(bool bPrecisionDecimal); // return a float style based on bPrecisionDecimal

	static cppstring				TranslatePathToFile(cppstring sPath); // Convert all separator characters to the proper OS
	static cppstring				TranslatePathFromFile(cppstring sPath); // Convert all separator characters from the proper OS

	static int						kSmallestFontSize;	// used in font size controls
	static int						kLargestFontSize;

private:
	static StringVector				kvSpecialChar;	
	static StringVector				kvSpecialCharCode;	

	
	static StringVector				kvOutgoingSpecialChar;	
	static StringVector				kvOutgoingSpecialCharCode;	

	static std::map<narrowstring, int>	m_mStringIndices;
	static std::vector<cppstring>	m_vStringData;
};

#define GSTD_STRING(x) GTextUtils::GetStringByKey(#x)
#define GSTD_INDEX(x) GTextUtils::GetStringIndexByKey(#x)
#define GSTD_INDEXSTRING(x) GTextUtils::GetStringByIndex(x)
#define GSTD_OSSTRING(x, p) GTextUtils::OSGetOSStringByKey(#x, p)
#define GSTD_NARROWSTRING(x) GTextUtils::GetNarrowStringByKey(#x)

#ifdef LIB_NAMESPACE
}
#endif

#endif // _GTEXTUTILS_H_
