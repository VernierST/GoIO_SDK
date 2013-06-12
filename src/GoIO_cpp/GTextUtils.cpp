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
// GTextUtils.cpp: implementation of the GTextUtils class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GTextUtils.h"

//#include "GStreamTokenizer.h"
#include "GUtils.h"
//#include "GMessenger.h"
//#include "GMessages.h" 

//#include "GApplicationBrain.h"

#ifdef _DEBUG
#include "GPlatformDebug.h" // for DEBUG_NEW definition
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif 

using namespace std;

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#endif

int GTextUtils::kSmallestFontSize = 8;	// used in font size controls
int GTextUtils::kLargestFontSize = 72;

std::map<narrowstring, int>	GTextUtils::m_mStringIndices;
std::vector<cppstring>		GTextUtils::m_vStringData;

// all special chars are preceeded by this, then have a numerical code, then a semi colon. e.g. degree is "&#176;"
// The numerical code is right out of the HTML4 Entity sets (pg. 425 of O'REILLY XML in a Nutshell)
const cppstring GTextUtils::kSpecialCharFlag = "&#";	
StringVector GTextUtils::kvSpecialChar;
StringVector GTextUtils::kvSpecialCharCode;
StringVector GTextUtils::kvOutgoingSpecialChar;
StringVector GTextUtils::kvOutgoingSpecialCharCode;

/*
cppstring GTextUtils::RealToCPPString(real rNum,			// the number to convert
								  int nPrecision,			// The number or decimal places or sig figs to use. NOTE -1 ==> just use default processing
								  EFloatStyle eFloatStyle)   // true ==> precision is decimal places, false ==> significant figures
{
	// Returns a string representing real number according to nPrecision and
	// eFloatStyle settings.
	// get the current locale setting 
	cppstring sReturn;

	if (!GUtils::OSIsValidNumber(rNum))
	{ // this number is either a NAN or INF
		sReturn = GSTD_STRING(IDSX_NOT_A_NUMBER);
	}
	else
	{ // this number is good
	
 		if (eFloatStyle == kFloatStyle_preferred)
 			eFloatStyle = (nPrecision > 7) ? kFloatStyle_default : kFloatStyle_fixedPrecision;
//		if (eFloatStyle == kFloatStyle_preferred)
//		{
//			if ((nPrecision > 7) && (fabs(rNum) < 100000))
//				eFloatStyle = kFloatStyle_default;
//			else
//				eFloatStyle = kFloatStyle_fixedPrecision;
//		}
	
		// just make 1 string stream 
		cppsstream ssFixed;
		ssFixed.imbue(GetAppBrain()->GetLocale());
		cppsstream ssScientific;
		ssScientific.imbue(GetAppBrain()->GetLocale());
		cppsstream ssDefault;
		ssDefault.imbue(GetAppBrain()->GetLocale());

		if (eFloatStyle == kFloatStyle_fixedPrecision) // use fixed decimal precision
		{
			ssFixed.clear();
			if (nPrecision == 0) // do not display the decimal point 
				ssFixed << fixed << uppercase << setprecision(nPrecision) << rNum;
			else
				ssFixed << fixed << uppercase << showpoint<<  setprecision(nPrecision) << rNum;
		
			cppstring sFixed = ssFixed.str();
// gspam
//			char buffer[1024];
//			sprintf(buffer, "%.*f", nPrecision, rNum);
//			cppstring sFixed = buffer;

			// get rid of "-" if the string is all zeros i.e (-0.0 -> 0.0) 
			if (rNum < 0.0 && rNum > -1.0)
			{
				if (sFixed.find_first_of(GSTD_S("123456789")) == cppstring::npos)
					sFixed.replace(sFixed.find(GSTD_S("-")),1,GSTD_S("")); // get rid of "-"
			}

			ssScientific.clear();
			if (nPrecision == 0) // Windows does not do scientific correctly for precision of 0 (Mac is OK) to work around the specification (scientific or fixed) is omitted and the system decides
				ssScientific << uppercase << setprecision(0) << rNum;
			else
				ssScientific << scientific << uppercase << setprecision(nPrecision) << rNum;
			cppstring sScientific = ssScientific.str();
// gspam
//			sprintf(buffer, "%.*e", nPrecision, rNum);
//			cppstring sScientific = buffer;

			// return the shortest cppstring but make sure to check for 0 length, as the fixed string 
			// can be zero if really large >10e70 are used
			if (sFixed.length() <= sScientific.length() && (sFixed.length() > 0))
				sReturn = sFixed;
			else
				sReturn = sScientific;
		}
		else 
		if (eFloatStyle == kFloatStyle_fixedSigFigs) 
		{	
			cppstring sRounded = RealToCPPStringRoundedToSigDigits(rNum, nPrecision); 
			sReturn = sRounded;
// gspam
//			char buffer[1024];
//			sprintf(buffer, "%.*g", nPrecision, rNum);
//			sReturn = buffer;
		}
		else
		if (eFloatStyle == kFloatStyle_default)
		{	// this block gets executed when editing cells or exporting data as text
			// fixed or scientific is not specified below so
			// setPrecision(n) can mean either n digits or n decimal places 
			// the default of 6 is not sufficient to handle data like 123456.78
			// which gets exported as 123456.7 using the default
			// so make it 12 to handle most cases
			ssDefault.clear();
			ssDefault << uppercase << setprecision(12) << rNum;
			sReturn = ssDefault.str();
		}
	}
	
	return sReturn;
}

cppstring GTextUtils::RealToCPPString(real rNum, int nPrecision, bool bPrecisionDecimal)
{ // RETURN converted number -- either with fixed or sigfigs
	if (bPrecisionDecimal)
		return RealToCPPString(rNum, nPrecision, kFloatStyle_fixedPrecision);

	return RealToCPPString(rNum, nPrecision, kFloatStyle_fixedSigFigs);
}

cppstring GTextUtils::RealToDisplayCPPString(real rNum, int nDigits)
{ // RETURN converted number but don't display more than nDigits
	real fPow = pow(10, nDigits);
	rNum = floor(rNum * fPow + 0.5) / fPow;
	return RealToCPPString(rNum);
}

cppstring GTextUtils::RealToCPPStringRoundedToSigDigits(real rNum , int nSigDigits)
{
	cppstring sResult;

	// get the user's preferred locale store in brain
#ifndef OPUS_DDK
	locale langLocale = GetAppBrain()->GetLocale();
#else
	locale langLocale;
#endif
	cppsstream ss;
	ss.imbue(langLocale);

	//the next line converts the number to a string with appropriate rounding but 
	//does not include significant zeros so we have to put them back into sResult
	
	ss << uppercase << setprecision(nSigDigits)  << rNum;
	sResult = ss.str();

	int nEPosition = sResult.find(GSTD_S("E")); // is it scientific notation ?

	// massage sResult padding with zeros if needed to get required number of digits

	// count the digits up to the E or to the end if no E
	// see if we need to pad with zero to get the required number of sig fig
	int nEndPosition = 0;
	if (nEPosition == cppstring::npos)
		nEndPosition = sResult.length();
	else
		nEndPosition = nEPosition;

	int numDigits = 0;
	gchar ch;
	gchar cDecimalPoint = '.';
	GMessenger::AppMessage(GMsg::kGetDecimalPointChar, static_cast<void *>(&cDecimalPoint));

	int nFirstNonZero = sResult.find_first_of(GSTD_S("123456789"));
	// count the digits preceeding the E if there is an E or to the end otherwise
	// but do not start counting until after the first non zero digit 
	if (nFirstNonZero == cppstring::npos || nFirstNonZero < 0)
		return sResult;
	for (int i = nFirstNonZero; i < nEndPosition ; i++)
	{
		ch = sResult.at(i);
		if (ch != GSTD_S('-') && ch != cDecimalPoint) // only count digits 
			numDigits++;
	}
	
	if (numDigits < nSigDigits)
	{ // the number of digits is less than the required number of sig figs so pad with zeros
		cppstring sPadding;
		int numZerosToPad = nSigDigits - numDigits;
		if (nEPosition == cppstring::npos) // no E
		{ // add a decimal point and pad with zeros after it (e.g. 3 becomes 3.00 for 3 sf)
			if (sResult.find(cDecimalPoint) == cppstring::npos)
				sPadding += cDecimalPoint; 
			for (int j = 0; j < numZerosToPad ; j++)
				sPadding += GSTD_S('0');

			sResult += sPadding;
		}
		else
		{
			if (sResult.find(cDecimalPoint) == cppstring::npos)
				sPadding += cDecimalPoint; // Add a decimal point first, if we don't find one.
			
			// Create the padding of 0's
			for (int j = 0; j < numZerosToPad ; j++)
				sPadding += GSTD_S('0');
			
			// Insert before the E.
			sResult.insert(nEPosition, sPadding);
		}
	}

	return 	sResult;
}
*/
cppstring GTextUtils::LongToCPPString(int nNum) // REVISIT - add base-display arg
{
	cppsstream ss;
	ss << nNum;
	return ss.str();
}

std::string GTextUtils::LongToString(int nNum) // REVISIT - add base-display arg
{
	std::stringstream ss;
	ss << nNum;
	return ss.str();
}

/*
real GTextUtils::CPPStringToReal(const cppstring &sNumber)
{
	real fResult;
	GTextUtils::IsStringRealNumber(sNumber, &fResult);	// just return the result
	return fResult;
}
real GTextUtils::StringToReal(const narrowstring &sNumber)
{// not as robust as CPPStringToReal but does not need to be
	real fValue;
	std::stringstream ss;
	ss << sNumber;
	ss >> fValue;
	return fValue;
}
*/
int GTextUtils::CPPStringToLong(const cppstring &sInteger)
{
	int nResult = 0L;
	if (GTextUtils::IsStringLong(sInteger))
		nResult = GTextUtils::Gstrtol(sInteger.c_str(), NULL, 10);
	return nResult;
}

int GTextUtils::StringToLong(const narrowstring &sInteger)
{
	int nResult = 0L;
	if (GTextUtils::IsNarrowStringLong(sInteger))
		nResult = strtol(sInteger.c_str(), NULL, 10);
	return nResult;
}

bool GTextUtils::StringIsAllDigits(const cppstring & sNumber)
{// returns true if sNumber is all digits ... no decimals, etc
	bool bResult = true;
	cppstring::const_iterator iChar = sNumber.begin();
	bResult = (sNumber.length() > 0);
	while(iChar != sNumber.end() && bResult)
	{
		if (!isdigit(*iChar)) 
		{
			bResult = false;
			break;
		}
		iChar++;
	}
	return bResult;
}

cppstring GTextUtils::ExtractBaseName(const cppstring & sFullName, int * p_nOutNumberSuffix)
{
	// ExtractBaseName expects a string in the form of [base sequenece][space character][digits]
	// The base sequence is returned as a cppstring.
	// The digits, if present, are converted to an integer value and stored
	// on exit in the p_nOutNumberSuffix parameter if it is non-NULL.
	
	cppstring sBase;
	cppstring sNum;
	
	unsigned int nLastSpacePos = sFullName.find_last_of(' ');
	if (nLastSpacePos != cppstring::npos)
	{
		unsigned int ix;
		for (ix = 0; ix < nLastSpacePos; ix++)
			sBase += sFullName[ix];
		
		for (ix = nLastSpacePos + 1; ix < sFullName.length(); ix++)
			sNum += sFullName[ix];
		
		bool bGotNum = (sNum.length() > 0 && IsStringLong(sNum));
		if (!bGotNum)
			sBase = sFullName;
			
		if (p_nOutNumberSuffix != NULL)
		{
			if (bGotNum)
				*p_nOutNumberSuffix = (int) GTextUtils::CPPStringToLong(sNum);
			else
				*p_nOutNumberSuffix = 0;
		}
	}
	else
	{
		sBase = sFullName;
		if (p_nOutNumberSuffix != NULL)
			*p_nOutNumberSuffix = 0;
	}
	return sBase;
}

cppstring GTextUtils::MatchSubString(const cppstring & sFullName, const cppstring & sFormat, const cppstring & sSubName)
{
	// MatchSubString determines whether sFullName matches sFormat (allowing sSubName to stand for a wildcard)
	// For example, if the format is "Run %n" and "%n" is the wildcard, then "Run 1" will match and return "1".
	cppstring sSubString;
	
	// Find the starting character of the wildcard string (assume 0 if wildcard isn't found)
	unsigned int nStartSub = sFormat.find(sSubName), nEndSub;
	if (nStartSub != cppstring::npos)
	{
		nEndSub = nStartSub + sSubName.length();
		
		// Break the format into two chunks surrounding the wildcard
		cppstring sBeginFormat = sFormat.substr(0, nStartSub);
		cppstring sEndFormat = sFormat.substr(nEndSub, sFormat.length() - sSubName.length());
		
		// Now match the beginning and ending formats and extract the wildcard string
		unsigned int nStartMatch = sFullName.find(sBeginFormat), nEndMatch;
		if (nStartMatch != cppstring::npos)
		{
			nStartMatch += sBeginFormat.length();
			nEndMatch = sFullName.rfind(sEndFormat);
			if (nEndMatch != cppstring::npos)
				sSubString = sFullName.substr(nStartMatch, nEndMatch - nStartMatch);
		}
	}
	
	return sSubString;
}		

/*
bool GTextUtils::IsStringRealNumber(cppstring sNumber,	// string to test
									real *pfValue)			// [out] if != NULL return valid number here ONLY GOOD IF RETURN VALUE IS TRUE
{ // RETURN true if string is a real number
	real fValue = NAN;
	bool bOK = false;
	bool bReadAllText;

	if ((GetAppBrain() != NULL) && (GetAppBrain()->GetDecimalPointChar() == GSTD_S(',')))
		sNumber = GTextUtils::StringReplace(sNumber, GSTD_S(","), GSTD_S(".")); // Handle locales where a comma is the decimal point 
	cppsstream ss;
	ss << sNumber;
	ss >> fValue;

	bReadAllText = ss.eof();
	if (!bReadAllText)
	{   // make sure we are at the end of the number otherwise its really a string
		// give ss a chance to skip any remaining white space
		// but if any non-white space chars are left, its not a valid number
		gchar c = 0;
		ss >> c; 
		bReadAllText = (ss.eof() && (c == 0));  // we are now at EOF and there we no non-whitespace chars left in stream
	}
	if (bReadAllText && GUtils::OSIsValidNumber(fValue))
	{
		fValue = GUtils::RestrictReal(fValue);
		bOK = true;
	}

	if (pfValue != NULL)
		*pfValue = fValue;

	return bOK;
}
*/
bool GTextUtils::IsStringLong(const cppstring & sNumber,	// string to test
							  int *pnValue)		// [out] if != NULL return valuid number here ONLY GOOD IF RETURN VALUE IS TRUE
{ // RETURN true if string is a int
	bool bOK = false;

	// if sNumber is not a number, then ss >> will not change the value of either nValue0 or nValue1
	// so, if they are different, then sNumber is not a number!
	int nValue0 = 0;
	int nValue1 = 1;
	cppsstream ss0;
	ss0 << sNumber;
	ss0 >> nValue0;
	cppsstream ss1;
	ss1 << sNumber;
	ss1 >> nValue1;
	if (nValue0 == nValue1)
		bOK = true;

	if (pnValue != NULL)
		*pnValue = nValue0;

	return bOK;
}
bool GTextUtils::IsNarrowStringLong(const narrowstring & sNumber,	// string to test
							  int *pnValue)		// [out] if != NULL return valuid number here ONLY GOOD IF RETURN VALUE IS TRUE
{ // RETURN true if string is a int
	bool bOK = false;

	// if sNumber is not a number, then ss >> will not change the value of either nValue0 or nValue1
	// so, if they are different, then sNumber is not a number!
	int nValue0 = 0;
	int nValue1 = 1;
	stringstream ss0;
	ss0 << sNumber;
	ss0 >> nValue0;
	stringstream ss1;
	ss1 << sNumber;
	ss1 >> nValue1;
	if (nValue0 == nValue1)
		bOK = true;

	if (pnValue != NULL)
		*pnValue = nValue0;

	return bOK;
}
cppstring GTextUtils::StringReplace(cppstring sBase,		// string to search
								const cppstring &sSearchFor,	// substring to search for (to be replaced)
								const cppstring &sReplaceWith)	// string to replace sSearchFor with
{ // RETURN sBase with all instances of sSearchFor replaced by sReplaceWith

	if (sBase.empty() || sSearchFor.empty())
		return sBase;

	int nStart = 0;
	while (sBase.find(sSearchFor, nStart) != cppstring::npos)
	{
		nStart = sBase.find(sSearchFor, nStart);
		int nEnd = sSearchFor.length();
		sBase.replace(nStart, nEnd, sReplaceWith);
		nStart += sReplaceWith.length();
	}

	return sBase;
}
std::string GTextUtils::AsciiStringReplace(std::string sBase,		// string to search
									const std::string &sSearchFor,	// substring to search for (to be replaced)
									const std::string &sReplaceWith)	// string to replace sSearchFor with
{ // RETURN sBase with all instances of sSearchFor replaced by sReplaceWith

	if (sBase.empty() || sSearchFor.empty())
		return sBase;

	int nStart = 0;
	while (sBase.find(sSearchFor, nStart) != cppstring::npos)
	{
		nStart = sBase.find(sSearchFor, nStart);
		int nEnd = sSearchFor.length();
		sBase.replace(nStart, nEnd, sReplaceWith);
		nStart += sReplaceWith.length();
	}

	return sBase;
}
/*
EImportTextFormat GTextUtils::ParseImportFormat(ifstream  * pInStream)
{// looks for special format indicators in the ifstream generated from 
 // text import
 	EImportTextFormat eFileFormat = kImportFormat_None;
 	
#ifndef OPUS_DDK
	GStreamTokenizer lineTokenizer (pInStream);
	lineTokenizer.SetDelimiters("\n\r");
	//get the first line 
	std::string sFirstLine;
	sFirstLine = lineTokenizer.NextToken();
	std::string sLine = sFirstLine;
	std::string sVernierFormat = GSTD_NARROWSTRING(IDSX_VERNIER_FORMAT2);
	std::string sGA1Format     = GSTD_NARROWSTRING(IDSX_GA_CONFIGURATION);
	
	if (sFirstLine.find(sVernierFormat.c_str()) !=  std::string::npos)
		eFileFormat = kImportFormat_LoggerPro2;
	else
		if (sFirstLine.find (sGA1Format.c_str()) != std::string::npos)
		eFileFormat = kImportFormat_GA2Windows;

	else 
	{	// check for 2 and only 2 integers on the first line
		// these are most likely the numcols and numDatasets of a GA2MAc
		// .TEXT file without its .TEXT extension
		// and then make sure there are NOT 2 just 2 integers on the next line 
		int nNumberOfTokens = 0; 
		int nNumberOfIntegers= 0;   
		std::string sWord;
		std::stringstream ssLineStream;
		ssLineStream << sLine ;
		GStreamTokenizer wordTokenizer (&ssLineStream);
		wordTokenizer.SetDelimiters("\t,");
		while ((sWord = wordTokenizer.NextToken()).size() != 0)
		{// for each word we count the chars and digits
			nNumberOfTokens += 1;
			if (GTextUtils::StringIsAllDigits(GTextUtils::ConvertNarrowStringToWide(sWord)))
				nNumberOfIntegers += 1;
			
		}// end while
		if (nNumberOfTokens == nNumberOfIntegers && nNumberOfTokens == 2) 
		{// this is almost surely kImportFormat_GA2Mac format but check the next line to make sure
			sLine= lineTokenizer.NextToken();
			ssLineStream << sLine ;
			GStreamTokenizer wordTokenizer (&ssLineStream);
			wordTokenizer.SetDelimiters("\t,");
			nNumberOfTokens = 0;
			nNumberOfIntegers = 0;
			while ((sWord = wordTokenizer.NextToken()).size() != 0)
			{// for each word we count the chars and digits
				nNumberOfTokens += 1;
				if (GTextUtils::StringIsAllDigits(GTextUtils::ConvertNarrowStringToWide(sWord)))
					nNumberOfIntegers += 1;
			
			}// end while
			if (nNumberOfTokens == nNumberOfIntegers)
				eFileFormat = kImportFormat_RawText;
			else
				eFileFormat = kImportFormat_GA2Mac; 

		}
		else
			eFileFormat = kImportFormat_RawText;
	
	} // else block
#endif
	return eFileFormat;
}

int	GTextUtils::GetWordsPerLine(std::stringstream * pInStream  )
{// gets the first line in the stream and returns the number of tab delimited values on that line
	int nWordCount = 0;
#ifndef OPUS_DDK
	int curPos = pInStream->tellg();
	// create a Tokenizer for lines and get the first line 
	GStreamTokenizer lineTokenizer (pInStream);
	lineTokenizer.SetDelimiters("\n\r");
	std::string sLine = lineTokenizer.NextToken();
	std::stringstream ss ;
	ss << sLine << kOSNewlineString;
	if (sLine.size() > 0)
	{
		GStreamTokenizer wordTokenizer (&ss );
		wordTokenizer.SetDelimiters("\t,\r\n");
		std::string sWord;
		while ( (sWord = wordTokenizer.NextToken()).size() != 0)
			nWordCount++;
	}
	// put the stream back to the starting position 
    	pInStream->seekg(curPos);
#endif
	return nWordCount;
}
*/
/*
cppstring GTextUtils::StripPathAndExtension(const cppstring &sPathAndName) // filename with path and possibly extension
{ // RETURN the name with path and extension stripped off
 	cppstring sStrippedName = sPathAndName;
	cppstring sBeginning;
	int nPosition;
	nPosition = sPathAndName.rfind('.'); // strips the ext
	if (nPosition != cppstring::npos)
		sBeginning = sPathAndName.substr(0, nPosition);
	else
		sBeginning = sPathAndName;

	cppstring sSep = GTextUtils::OSGetPathSeparator();
	nPosition = sBeginning.find_last_of(sSep);
	if (nPosition != cppstring::npos) 
		sStrippedName = sBeginning.substr(nPosition+1, sBeginning.length());
	else
		sStrippedName = sBeginning;

	return sStrippedName;
}

cppstring GTextUtils::StripPath(const cppstring & sPathAndName) // Filename with path
{ // RETURN the name with path stripped off
	cppstring sStrippedName = sPathAndName;

	cppstring sSep = GTextUtils::OSGetPathSeparator();
	int nPosition = sPathAndName.find_last_of(sSep);
	if (nPosition != cppstring::npos) 
		sStrippedName = sPathAndName.substr(nPosition+1, sPathAndName.length());

	return sStrippedName;
}

cppstring GTextUtils::GetPath(const cppstring &sFullFileName) // input fully qualified name
{ // RETURN just the path parth of the name (with trailing separator char)
	cppstring sPath;

	cppstring sSep = GTextUtils::OSGetPathSeparator();
	int nPosition = sFullFileName.find_last_of(sSep);
	if (nPosition != cppstring::npos) 
		sPath = sFullFileName.substr(0, nPosition+1);

	return sPath;
}

bool GTextUtils::InvalidDataWorldName(const cppstring &sName)
{ // RETURN true if sName has invalid character(s) in it
	bool bInvalid = false;

	cppstring sSep = GSTD_STRING(IDSX_SEPARATOR);
	if ((sName.find(sSep) != cppstring::npos) ||
		(sName.find('\"') != cppstring::npos))
		bInvalid = true;

	return bInvalid;
}

cppstring GTextUtils::GetExtension(const cppstring &sFileName)
{ // RETURN the file name extension if there is one ... e.g Junk.txt returns .txt
	int nPosition;
	cppstring sExt;
	nPosition = sFileName.find_last_of('.');
	if (nPosition != cppstring::npos)
		sExt = sFileName.substr(nPosition,sFileName.length());
	
	return sExt;
}

cppstring GTextUtils::ParenthesizeString(const cppstring &s)
{
	cppstring sOut = s;
	if (sOut.length() != 0)
		sOut = GSTD_S("(") + sOut + GSTD_S(")");
		
	return sOut;
}	
narrowstring	GTextUtils::ParenthesizeNarrowString(const narrowstring &s)
{// in the narrow char app cppstring == narrowstring so overloaded functions appear to be duplicates 
	narrowstring sOut = s;
	if (sOut.length() != 0)
		sOut = "(" + sOut + ")";
		
	return sOut;
}

cppstring GTextUtils::FilterOutChars(const cppstring &sInString, const cppstring &sFilterChars)
{
	cppstring sOut;
	for (int i = 0; i < sInString.length(); i++)
	{
		if (sFilterChars.find(sInString[i]) == cppstring::npos)
			sOut += sInString[i]; //Not found in filter list, so pass char thru.
	}
		
	return sOut;
}
*/
/*
cppstring GTextUtils::ConvertToUppercase(const cppstring &sInString)
{
	cppstring sOut;
	GSTD_ASSERT(GetAppBrain() != NULL);
	for (int i = 0; i < sInString.length(); i++)
		sOut += std::toupper(sInString[i], GetAppBrain()->GetLocale());

	return sOut;
}

bool GTextUtils::StringsEqualIgnoringCase(const cppstring &s1, const cppstring &s2)
{
	return (ConvertToUppercase(s1) == ConvertToUppercase(s2));
}
	
cppstring GTextUtils::ConvertToOSSpecialChars(cppstring s)
{  // go through s and convert "&#xxx;" to a special OS character

	StringVector *psvOSChars = GetOSSpecialChars();
	StringVector *psvOSCharCodes = GetOSSpecialCharCodes();
	GSTD_ASSERT( psvOSChars->size() == psvOSCharCodes->size() );
	
	if (s.length() > 0)
	{
		char c1 = s[0];
	
		int vOSSize = psvOSCharCodes->size();
		for (int i=0; i<vOSSize; i++)
			s = GTextUtils::StringReplace(s, psvOSCharCodes->at(i), psvOSChars->at(i));

		char c2 = s[0];
	}
		
	return s;
}

cppstring GTextUtils::ConvertFromOSSpecialChars(cppstring s)
{ // go through s and convert any OS special chars to "&#xxx;" 

	// now that we set the charset when writing the output 
	// we don't need to special case these chars which are all
	// included within the windows-1252 charset.
	//
	// we still read the html version on input streams without problem.
	StringVector *psvOSChars = GetOSOutgoingSpecialChars();
	StringVector *psvOSCharCodes = GetOSOutgoingSpecialCharCodes();	
	GSTD_ASSERT( psvOSChars->size() == psvOSCharCodes->size() );
	
	if (s.length() > 0)
	{
		char c1 = s[0];
		
		int vOSSize = psvOSCharCodes->size();
		for (int i=0; i<vOSSize; i++)
			s = GTextUtils::StringReplace(s, psvOSChars->at(i), psvOSCharCodes->at(i));

		char c2 = s[0];
	}
		
	return s;
}
StringVector *GTextUtils::GetOSSpecialChars(void)
{ // RETURN a vector of strings where each string is one special character
  // NOTE that these should match the special char codes returned in GetOSSpecialCharCodes
	if (kvSpecialChar.size() == 0)
	{
		cppstring s;
		
		s += kDegreeChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kMuChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kPlusOrMinusChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");

		s += kOneQuarter;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kOneHalf;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kThreeQuarters;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");

		s += kCapYAcute;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kLwYAcute;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");

		s += kCapSCaron;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kLwSCaron;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");

		s += kSuperScriptTwo;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		
		s += kGreaterThanChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		s += kLessThanChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
		
		// insert new CharCodes here
		
		// Ampersand MUST be last in list to ensure that if the user typed in an entity code it isn't converted to the entenity 
		// by converting '&' to &#38; BEFORE we convert any other stings and turning it back into an ampersand AFTER we have done 
		// all other replacements we sure that if the user typed a string of the form &#<some number>;  we don't convert it
		// which we shouldn't since we don't conver all possible codes.
		
		s += kAmpersandChar;
		kvSpecialChar.push_back(s);
		s = GSTD_S("");
	}
	
	return &kvSpecialChar;
}


StringVector *GTextUtils::GetOSSpecialCharCodes(void)
{
	if (kvSpecialCharCode.size() == 0)
	{
		
		kvSpecialCharCode.push_back(GSTD_S("&#176;")); // degree
		kvSpecialCharCode.push_back(GSTD_S("&#181;")); // MuChar
		kvSpecialCharCode.push_back(GSTD_S("&#177;")); // Plus/Minus

		kvSpecialCharCode.push_back(GSTD_S("&#188;")); // 1/4
		kvSpecialCharCode.push_back(GSTD_S("&#189;")); // 1/2
		kvSpecialCharCode.push_back(GSTD_S("&#190;")); // 3/4

		kvSpecialCharCode.push_back(GSTD_S("&#221;")); // Y^ y acute
		kvSpecialCharCode.push_back(GSTD_S("&#253;")); // y^

		kvSpecialCharCode.push_back(GSTD_S("&#352;")); // S^ chron
		kvSpecialCharCode.push_back(GSTD_S("&#353;")); // s^

		kvSpecialCharCode.push_back(GSTD_S("&#178;")); // ^2
		
		kvSpecialCharCode.push_back(GSTD_S("&#62;")); // '>' (could also be &gt;)
		kvSpecialCharCode.push_back(GSTD_S("&#60;")); // '<' (could also be &lt;)
		
		// insert new CharCodes here
		
		// Ampersand MUST be last in list to ensure that if the user typed in an entity code it isn't converted to the entenity 
		// by converting '&' to &#38; BEFORE we convert any other stings and turning it back into an ampersand AFTER we have done 
		// all other replacements we sure that if the user typed a string of the form &#<some number>;  we don't convert it
		// which we shouldn't since we don't conver all possible codes.
		kvSpecialCharCode.push_back(GSTD_S("&#38;")); // ampersand (could also be &amp;)
	}
	
	return &kvSpecialCharCode;
}

//    A routine that takes a path to a TXT file and 
// 	  adds the strings in the file to our internal database.
//    The input TXT file should be of the form:
// IDSX_STRING_ID1 "String 1"
// IDSX_STRING_ID2 "String 2"
// IDSX_STRING_ID3 "String 3" // Comments are allowed here 
// // or here, or anywhere really, with the standard C thing of "//".

bool GTextUtils::ReadStringsFromFile(cppstring sPath)
{
	// Read in strings from the specified file
	bool bResult = false;
	int nIndex = m_vStringData.size();
	GFileRef theFileRef;
	theFileRef.SetFilePath(sPath);
	theFileRef.SetCodecvtType(kCodecvtUTF8);
	StringVector vsKeysSoFar; // To check for duplicate keys

	if (theFileRef.OSOpenFStream(ios_base::binary | ios_base::in))
	{
		cppistream * piStream = dynamic_cast<cppistream *>(theFileRef.GetFStream());
		if (piStream != NULL) // We opened the strings file
		{
			cppstring sLine;
			while (getline(*piStream, sLine))
			{
				sLine = sLine.substr(0, sLine.find(GSTD_S("//"))); // strip off comments
				if (!sLine.empty())
				{ // We have a non-empty line to work with
					int nWhiteSpace = sLine.find(GSTD_S(" "));
					if (sLine.find(GSTD_S("\t")) < nWhiteSpace)
						nWhiteSpace = sLine.find(GSTD_S("\t"));
					cppstring sWord = sLine.substr(0, nWhiteSpace);
					
					if (sWord.find(GSTD_S("IDSX")) != cppstring::npos)
					{
						#ifdef _DEBUG
							for (size_t jx = 0; jx < vsKeysSoFar.size(); jx++)
								if (sWord == vsKeysSoFar[jx])
								{ // ERROR!!!!  There are duplciate IDSX_XXX in the strings file!
	 								GSTD_TRACE("Duplicate Key: " + sWord);
	 								GMessenger::PostAsyncMessage(kStdObject_AppBrain, GMsg::kAssert, __FILE__, __LINE__);
								}
							vsKeysSoFar.push_back(sWord);
						#endif

						cppstring sBody;
						if (sLine.find(GSTD_S('"')) != cppstring::npos)
						{
							int nFirstQuote = sLine.find(GSTD_S('"'));
							int nLastQuote = sLine.find_last_of(GSTD_S('"'));
							GSTD_ASSERT((nFirstQuote != nLastQuote) && (nFirstQuote != cppstring::npos) && (nLastQuote != cppstring::npos));
							sBody = sLine.substr(nFirstQuote + 1, nLastQuote - nFirstQuote - 1);
							sBody = GTextUtils::StringReplace(sBody, GSTD_S("\\\""), GSTD_S("\""));
							sBody = GTextUtils::StringReplace(sBody, GSTD_S("\\n"), GSTD_S("\n"));
							sBody = GTextUtils::StringReplace(sBody, GSTD_S("\\t"), GSTD_S("\t"));
							sBody = GTextUtils::StringReplace(sBody, GSTD_S("\\\\"), GSTD_S("\\"));
							m_mStringIndices[ ConvertWideStringToNarrow(sWord)] = nIndex++;
							m_vStringData.push_back(sBody);
						}
					}
				}
			}
			bResult = true;	
		}
		theFileRef.OSCloseFStream();
	}
	
	return bResult;
}
*/
/*
void GTextUtils::FreeResourceStrings(void)
{
	m_vStringData.clear();
	m_mStringIndices.clear();
}

int GTextUtils::GetStringIndexByKey(const char * sKey)
{
	if (m_vStringData.empty())
		OSInitResourceStrings();
	std::map<narrowstring, int>::iterator iter = m_mStringIndices.find(sKey);
	GSTD_ASSERT(iter != m_mStringIndices.end());
	return (iter->second);
}

cppstring GTextUtils::GetStringByKey(const char * sKey)
{
	if (m_vStringData.empty())
		OSInitResourceStrings();
	std::map<narrowstring, int>::iterator iter = m_mStringIndices.find(sKey);
	if (iter == m_mStringIndices.end())
	{ // bad!
		cppstring s = sKey;
		GSTD_TRACE("Bad Key: " + s);
		GSTD_ASSERT(false);
	}
	int nIndex = (iter->second);
	if ((nIndex < 0) || (nIndex >= m_vStringData.size()))
	{ // Bad!
		cppstring s = sKey;
		GSTD_TRACE("Bad Key: " + s);
		GSTD_ASSERT(false);
	}
	cppstring sReturnString;
	if ((nIndex >= 0) && (nIndex < m_vStringData.size()))
		return m_vStringData[nIndex];
	else
		return sReturnString;
}
*/
//narrowstring GTextUtils::GetNarrowStringByKey(const char * sKey)
//{
//	if (m_vStringData.empty())
//		OSInitResourceStrings();
//	std::map<narrowstring, int>::iterator iter = m_mStringIndices.find(sKey);
//	GSTD_ASSERT(iter != m_mStringIndices.end());
//	int nIndex = (iter->second);
//	GSTD_ASSERT(nIndex >= 0 && nIndex < m_vStringData.size());
//
//	cppstring s = m_vStringData[nIndex];
// /*
//#ifdef USE_WIDE_CHARS
//	char *pmbcharbuf = (char *)malloc(MB_CUR_MAX);
//	wcstombs(pmbcharbuf, m_vStringData[nIndex].c_str(), MB_CUR_MAX);
//	s = *pmbcharbuf;
//	free(pmbcharbuf);
//#endif
//	*/
//	return GTextUtils::ConvertWideStringToNarrow(s);
//}

/*
cppstring GTextUtils::GetStringByIndex(int nID)
{
	if (m_vStringData.empty())
		OSInitResourceStrings();
	GSTD_ASSERT(nID >= 0 && nID < m_vStringData.size());
	return m_vStringData[nID];
}
*/
//narrowstring GTextUtils::GetNarrowStringByIndex(int nID)
//{
//	if (m_vStringData.empty())
//		OSInitResourceStrings();
//	GSTD_ASSERT(nID >= 0 && nID < m_vStringData.size());
//
//	cppstring s = m_vStringData[nID];
// /*
//#ifdef USE_WIDE_CHARS
//	char *pmbcharbuf = (char *)malloc(MB_CUR_MAX);
//	wcstombs(pmbcharbuf, m_vStringData[nID].c_str(), MB_CUR_MAX);
//	s = *pmbcharbuf;
//	free(pmbcharbuf);
//#endif
//	*/
//	return ConvertWideStringToNarrow(s);
//}

/*
StringVector GTextUtils::GetRangeOfStrings(int nStartIndex, int nEndIndex)
{
	StringVector vsResult;
	for (int i = nStartIndex; i != nEndIndex; i++)
		vsResult.push_back(GSTD_INDEXSTRING(i));
	return vsResult;
}

narrowstring GTextUtils::RemoveSameRoot(AsciiStringVector *pvStrings)
{ // if all strings have the same root (i.e. text before the last '\') then remove it 
	// RETURN the root (or "" if strings have different roots)
	bool bSameRoot = true;
	narrowstring sRoot;

	for (int i=0; i<pvStrings->size(); i++)
	{ // See if all strings have the same root
		narrowstring sTempRoot; 
		if (pvStrings->at(i).find_last_of('\\') != cppstring::npos)
			sTempRoot = pvStrings->at(i).substr(0, pvStrings->at(i).find_last_of('\\')+1);
		if (sRoot.empty())
			sRoot = sTempRoot;
		if (sRoot != sTempRoot)
		{
			bSameRoot = false;
			sRoot = "";
			break;
		}
	}

	if ((bSameRoot) && !sRoot.empty())
	{ // If all data objects have the same non-empty root, then strip it off
		for (int i=0; i<pvStrings->size(); i++)
			pvStrings->at(i) = pvStrings->at(i).substr(pvStrings->at(i).find_last_of('\\')+1, cppstring::npos);
	}

	return sRoot;
}
*/
/*
short GTextUtils::AdjustTextSizeByAppPref(short nOrigSize, bool bApplyNewSize, void * pOSData)
{
	// This routine adjusts a text size based on application
	// preference settings; the nOrigSize is considered the
	// correct "medium" size.
	
	// If the text size is "large", we increase smaller fonts slightly;
	
	// If bApplyNewSize is true, GTextUtils::OSApplyTextSize() will be called.
		
	short nNewSize = nOrigSize;
#ifndef OPUS_DDK
	if (GetAppBrain()->IsLargeText())
	{
		switch(nOrigSize)
		{
			// we do not enlarge fonts that are larger than 14pt
			case 14:
			case 13:
				nNewSize = 16;
				break;
			case 12:
			case 11:
				nNewSize = 14;
				break;
			case 10:
			case 9:
				nNewSize = 12;
				break;
			case 8:
			case 7:
				nNewSize += 2;
				break;
			default:
				if (nOrigSize <= 6)
					nNewSize = 8;
		}
	}

	// apply the size change to the current port:
	// this is the real OS-specific part...
	if (bApplyNewSize)
		GTextUtils::OSApplyTextSize(nNewSize, pOSData);
#endif // !OPUS_DDK
	
	return nNewSize;
}

EFloatStyle GTextUtils::GetFloatStyle(bool bPrecisionDecimal)
{ // return a float style based on bPrecisionDecimal
	EFloatStyle eStyle = kFloatStyle_fixedPrecision;
	if (!bPrecisionDecimal)
		eStyle = kFloatStyle_fixedSigFigs;

	return eStyle;
}

cppstring GTextUtils::TranslatePathToFile(cppstring sPath)
{ // Write OS before fiel path so we can convert separators properly when we read
	GSTD_ASSERT(GetAppBrain() != NULL);
	if (GetAppBrain()->GetThisOSType() == kOSType_Windows)
		sPath = GSTD_S("W") + sPath;
	else
		sPath = GSTD_S("M") + sPath;

	return sPath;
}

cppstring GTextUtils::TranslatePathFromFile(cppstring sPath)
{ // Convert all separator characters from the proper OS
	if (sPath.size() > 1)
	{
		GSTD_ASSERT(GetAppBrain() != NULL);

		if ((GetAppBrain()->GetThisOSType() == kOSType_Windows) &&
			(sPath[0] == 'M'))
			sPath = GTextUtils::StringReplace(sPath, kMacPathSeparatorString, kWinPathSeparatorString);

		if ((GetAppBrain()->GetThisOSType() != kOSType_Windows) &&
			(sPath[0] == 'W'))
			sPath = GTextUtils::StringReplace(sPath, kWinPathSeparatorString, kMacPathSeparatorString);
		
		sPath = sPath.substr(1);
	}

	return sPath;
}

real GTextUtils::GetVersionFromBaseName(cppstring sFileName,	// actual filename with real numbers
										cppstring sBaseName)	// base file name (with '*' instead of numbers)
{ // pulls out the version number from the filename using the basename as template, e.g. basename might be base*.hex

	real fValue = 0;

	// Strip off paths and extenstions so as not to confuse the issue
	sFileName = GTextUtils::StripPathAndExtension(sFileName);
	sBaseName = GTextUtils::StripPathAndExtension(sBaseName);

	// Make a string that starts with the actual number
	cppstring sNumber = sFileName.substr(sBaseName.find('*'));

	// now extract the numbers, will stop when we reach an non number
	fValue = GTextUtils::CPPStringToLong(sNumber);

	// Divide number by 100000 so that non decimal number (as store din filename) will get converted properly
	fValue /= 100000.0;

	return fValue;
}

unsigned short GTextUtils::GetValueFromCharacterEscape(cppstring sCharEscape)
{
	// sCharEscape should be a standard XML/HTML escape sequence in one of two formats:
	//  &#N;  or  &#xN;
	// In the first case, nnn is a base-10 integer; in the second case, a hexadecimal integer.
	// The method simply converts the value to an integer correctly according to base (presence of the "x"
	// indicated hexadecimal).
	
	// REQUIREMENT: remove whitespace before calling this method
	
	unsigned short nChar = 0;
	
	const gchar * k_sSpace = GSTD_S(" ");
	if ((sCharEscape.length() >= 4) && (sCharEscape[0] == GSTD_S('&')) && (sCharEscape[1] == GSTD_S('#')))
	{
		int nBase = 10;
	
		sCharEscape[0] = GSTD_S(' ');
		sCharEscape[1] = GSTD_S(' ');
		
		if (sCharEscape[2] == GSTD_S('x'))
			sCharEscape[1] = GSTD_S('0');
#ifdef USE_WIDE_CHARS
		nChar = (unsigned short) wcstoul(sCharEscape.c_str(), NULL, 0);
#else
		nChar = (unsigned short) strtoul(sCharEscape.c_str(), NULL, 0);
#endif
	}
	
	return nChar;
}

cppstring GTextUtils::EncodeCharacterEscape(unsigned short nUTF8Char)
{
	cppsstream ss;
	ss << "&x" << hex << nUTF8Char << ";";
	return ss.str();
}
*/
cppstring::cppstring(gchar c)
{
	cppstring s = GSTD_S(" ");
	s[0] = c;
	*this = s;
}

gchar *		GTextUtils::Gstrcpy(gchar * strDestination,const gchar * strSource )
{
	gchar  * pstrReturn;
#ifdef USE_WIDE_CHARS
	pstrReturn = wcscpy(strDestination, strSource);
#else
	pstrReturn = strcpy(strDestination, strSource);
#endif
	return pstrReturn;
}

gchar *	 GTextUtils::Gstrncpy(gchar *strDestination,const gchar * strSource, size_t nCount)
{
	gchar  * pstrReturn;
#ifdef USE_WIDE_CHARS
	pstrReturn = wcsncpy(strDestination, strSource,nCount);
#else
	pstrReturn = strncpy(strDestination, strSource,nCount);
#endif
	return pstrReturn;
}

int	GTextUtils::Gstrtol(const gchar *nptr, gchar **endptr, int nbase)
{
	int nLong;
#ifdef USE_WIDE_CHARS
	nLong = wcstol(nptr,endptr,nbase);
#else
	nLong = strtol(nptr,endptr,nbase);
#endif
	return nLong;
}
int	GTextUtils::Gisspace(const gint n)
{

#ifdef USE_WIDE_CHARS
	return  iswspace(n);
#else
	return  isspace(n);
#endif
	
}
int	GTextUtils::Gisprint(const gint c)
{
#ifdef USE_WIDE_CHARS
	return  iswprint(c);
#else
	return  isprint(c);
#endif
	
}

int GTextUtils::Gisalpha(const gint c)
{
#ifdef USE_WIDE_CHARS
	return  iswalpha(c);
#else
	return  isalpha(c);
#endif
}
bool	GTextUtils::IsEOF(int ch)
{

#ifdef USE_WIDE_CHARS
	return  ch == WEOF;
#else
	return ch == EOF;
#endif
	
}

gint GTextUtils::GEOF()
{
#ifdef USE_WIDE_CHARS
	return WEOF;
#else
	return EOF;
#endif
}
double	GTextUtils::Gstrtod(const gchar *nptr, gchar **endptr)
{
	double fValue;
#ifdef USE_WIDE_CHARS
	fValue = wcstod(nptr,endptr);
#else
	fValue = strtod(nptr,endptr);
#endif
	return fValue;
}

size_t GTextUtils::Gstrlen(const gchar * str)
{
	size_t len = 0;
#ifdef USE_WIDE_CHARS
	len = wcslen(str);
#else
	len = strlen(str);
#endif
	return len;
}

int	GTextUtils::Gstrcmp(const gchar *str1,const gchar *str2)
{
	int  nReturn = 0;
#ifdef USE_WIDE_CHARS
	nReturn = wcscmp(str1,str2);
#else
	nReturn = strcmp(str1,str2);
#endif
	return nReturn;
}

int	GTextUtils::Gstrncmp(const gchar *str1,const gchar *str2, size_t nCount)
{
	int  nReturn = 0;
#ifdef USE_WIDE_CHARS
	nReturn = wcsncmp(str1,str2,nCount);
#else
	nReturn = strncmp(str1,str2,nCount);
#endif
	return nReturn;
}

cppstring GTextUtils::ConvertNarrowStringToWide(const narrowstring sNarrow)
{
// input a narrow (ASCII )string ... it will not be multibyte 
// convert to wide if USE_WIDE_CHARS

#ifdef USE_WIDE_CHARS
	cppstring sResult(sNarrow.c_str());

#else
	cppstring sResult(sNarrow); // the types are the same... does nothing
#endif
	return sResult;
}

std::string	GTextUtils::ConvertWideStringToNarrow(const cppstring  sWide)
{
#ifdef USE_WIDE_CHARS
	char * pMultiByteBuf;
	pMultiByteBuf   = (char *)malloc( sWide.length()*2 );
	size_t i = wcstombs( pMultiByteBuf, sWide.c_str(), sWide.length()*2 );
	std::string sNarrow (pMultiByteBuf);
	delete pMultiByteBuf;
#else
	std::string sNarrow(sWide); // since cppstring is already narrow there is nothing to do
#endif 
	return sNarrow;
}

/*
GDeviceRect GTextUtils::GetBestRectForText( cppstring sText, const GDeviceRect & maxRect, int nFontHeight, int nMinWidth)
{
	// Given a quantity of text and current font size & face settings,
	// calculate a good display rectangle.  Note that the main usage
	// for the routine is currently for a dynamically-sized messagebox.
	// This isn't exactly "wrap" mode - this routine will
	// constrain first-line width if width-to-height proportion is more than 
	// 20.
	//
	// In calculating height of rects that will require text wrap, this
	// routine always adds 1 line to the total to account for additional
	// blank space.
		
	int nTextWidth = 0;
	int nNumLines = 1;
	
	int kMaxWidth = (maxRect.right - maxRect.left);
	int kMaxHeight = (maxRect.bottom - maxRect.top);
	int kTotalTextLength = sText.length();
	int kMaxCharsPerLine = sText.length();
	double kMaxProportion = 3.0; // max width-to-height proportion
	int nNumNewlines = 0;
	
	nTextWidth = GTextUtils::OSGetTextLength(sText);
	if (nTextWidth < 0)
		nTextWidth = kMaxWidth+1;
	
	GDeviceRect bestRect(0,0,0,0);

	cppstring sTempString = sText;
	int nLongestLine = 0, nMaxLineWidth = 0;
	
	// Count the number of newlines that occur in sText
	int pos = 0;
	cppstring sEnd =  kOSNewlineString;
	while (pos != cppstring::npos)
	{
		int nFound = sText.find(sEnd, pos);
		if (nFound != cppstring::npos)
		{
			nNumNewlines++;
			if (nFound - pos > nLongestLine)
				nLongestLine = nFound - pos;
			pos = nFound + 1;
		}
		else
			break;
	}
	
	// Find out approximately how many characters are the max for
	// given max width - we're not worried about exactness with non-proportional
	// fonts etc.
	if (nLongestLine == 0)
	{
		while ((kMaxCharsPerLine > 1) && (nTextWidth > kMaxWidth))
		{
			--kMaxCharsPerLine;
			sTempString = GSTD_S("");
			sTempString.insert(0, sText, 0, kMaxCharsPerLine);
			nTextWidth = GTextUtils::OSGetTextLength(sTempString);
			if (nTextWidth < 0)
				nTextWidth = kMaxWidth + 1;
		}
		nLongestLine = kMaxCharsPerLine;
	}
	else
	{
		sTempString = GSTD_S("");
		sTempString.insert(0, sText, 0, nLongestLine);
		nTextWidth = GTextUtils::OSGetTextLength(sTempString);
		if (nTextWidth < 0)
			nTextWidth = kMaxWidth + 1;
		nMaxLineWidth = nTextWidth;
	}
	
	
	// Now determine number of approximate number of lines
	if (nNumNewlines > 0)
		nNumNewlines--;
	else
		nNumNewlines = 1;
	
	nNumLines = nNumNewlines + sText.length() / nLongestLine;

	// Assign the dimensions we've found to bestRect, which is currently 0,0,0,0:
	bestRect.bottom = bestRect.top + nFontHeight * nNumLines;
	bestRect.right = bestRect.left + nTextWidth + 25;
	
	// Now fix proportion - while bestRect is too wide, narrow it by
	// a character-per-line at a time...
	
	while ((double)nTextWidth > (double)(bestRect.bottom - bestRect.top) * kMaxProportion)
	{
		sTempString = sTempString.substr(0,sTempString.length() -1);
		nTextWidth = GTextUtils::OSGetTextLength(sTempString);
		if (nTextWidth < 0)
			nTextWidth = kMaxWidth+1;
		nLongestLine--;
		if (nLongestLine < 1)
			break;
		
		// Add in a factor that accounts for automatic word wrap
		int nWordWrapLines = max(0, (nMaxLineWidth / nTextWidth));
			
		nNumLines = nNumNewlines + nWordWrapLines + sText.length() / nLongestLine;
		bestRect.right = bestRect.left + nTextWidth + 25;
		bestRect.bottom = bestRect.top + nFontHeight * nNumLines;
		if (bestRect.Width() < nMinWidth)
			break;
	}
	
	if (kMaxCharsPerLine < 1 || nTextWidth < 1)
	{
		// We did not get a good rect
		bestRect.SetRect(0,0,0,0);
	}
	else
	{
		bestRect.bottom += nFontHeight; // add 1 more line for good measure
	
		// make sure our bestRect is not bigger than maxRect
		if (bestRect.top < maxRect.top) bestRect.top = maxRect.top;
		if (bestRect.left < maxRect.left) bestRect.left = maxRect.left;
		if (bestRect.bottom > maxRect.bottom) bestRect.bottom = maxRect.bottom;
		if (bestRect.right > maxRect.right) bestRect.right = maxRect.right;
	}
	
	return bestRect;
}
*/

#ifdef LIB_NAMESPACE
}
#endif
