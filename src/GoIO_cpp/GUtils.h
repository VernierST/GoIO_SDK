// GUtils.h
//
// GUtils is a collection of static utility methods designed to wrap
// common (but platform- or compiler-dependent) facilities.

#ifndef _GUTILS_H_
#define _GUTILS_H_

#include "GTypes.h"

// REVISIT - factor out this module...
// GUtils should be the absolute core debug, assert, and trace utils,
// for all source development (including DDK).
// Message-box stuff should be in a separate module, as should
// numerical utilities.

#define GSTD_ASSERT(x) GUtils::Assert((x),WIDENMACRO(__FILE__),__LINE__)

#define TRACE_SEVERITY_LOWEST 1
#define TRACE_SEVERITY_LOW 10
#define TRACE_SEVERITY_MEDIUM 50
#define TRACE_SEVERITY_HIGH 100

//We are defining the SUBSYS_TRACE_THRESH macro so that we can change the name from the build script.
//GCC 3.x is prone to name collision, so we may want to override the default name in some situations.
#ifndef SUBSYS_TRACE_THRESH 
#define SUBSYS_TRACE_THRESH g_subsystemTraceThreshold
#endif
extern int SUBSYS_TRACE_THRESH ;

#define GSTD_TRACE(x) GUtils::Trace(TRACE_SEVERITY_LOW,(x),WIDENMACRO(__FILE__),__LINE__)
#define GSTD_TRACEX(severity, msgtext) GUtils::Trace(severity,(msgtext),WIDENMACRO(__FILE__),__LINE__)

// macros for converting __FILE__ to wide 
#define WIDENMACRO(macro) GSTD_S(macro) // widens a macro
#define WIDEN(x) GTextUtils::ConvertNarrowStringToWide(x)

#ifdef TARGET_OS_MAC
	#if TARGET_CPU_X86
		#include <cmath>
	#else
		#include <math.h>
		extern long __isnand( double x );
		extern long __isfinited( double x );
	#endif
#endif

class GFileRef;

class GUtils
{
public:
	// Debugging / diagnostic utilities
	static bool 			Assert(bool bArgument, const gchar * sFile = GSTD_S(""), int nLine = -1); // ASSERTS if argument is false (also returns argument)
	static void				AssertDialog(const gchar * sFile, int nLine, const cppstring &sStackTrace = GSTD_S("")); // Cross platform error dialog
	static void 			Trace(const cppstring msg, const gchar * psFile = NULL, int nLine = -1); // output a string to trace output
	static void 			Trace(const gchar * pMsg, const gchar * psFile = NULL, int nLine = -1); // output a null-terminated string to trace output
	static void 			Trace(void * pointer, const gchar * psFile = NULL, int nLine = -1); // output a null-terminated string to trace output
	static void 			Trace(int trace_severity, const cppstring msg, const gchar * psFile = NULL, int nLine = -1); // output a string to trace output
	static void 			Trace(int trace_severity, const gchar * pMsg, const gchar * psFile = NULL, int nLine = -1); // output a null-terminated string to trace output
	static void 			Trace(int trace_severity, void * pointer, const gchar * psFile = NULL, int nLine = -1); // output a null-terminated string to trace output
	static void 			OSTrace(const gchar * pMsg); // output a null-terminated string to trace output
	static unsigned long 	TraceClock(void); // output the result of std::clock() to trace output; returns clock time
	static void				OSFloatingPointExceptionReset(void);						
	static void				Beep(void);	// Your basic beep.

	static void 			MessageBox(const cppstring &msg); // Put up a message box showing passed-in cppstring
	
	static bool 			YesNo(const cppstring &msg); // Put up a Yes/No message box (showing sppstring) -- return true if user chooses Yes
	
	static bool 			OKCancel(const cppstring &msg); // Put up an OK/Cancel message box (showing cppstring) -- return true if user chooses OK

	static EYesNoCancel 	YesNoCancel(const cppstring &msg); // Put up a Yes/No/Cancel message box (showing cppstring) -- return enum EYesNoCancel

	static int				CustomMessageBox(cppstring sTitle, cppstring msg, StringVector * pvButtonTitles, int nDefaultButton = 0); // list of buttons, in order, default button first

	static void Sleep(unsigned long msToSleep) { OSSleep(msToSleep); }
	static void OSSleep(unsigned long msToSleep);
	
	static bool			IsBetween(real range1, real value, real range2);	// RETURN true if value is between the ranges (inclusive)

	// Exception handling
	static void			OSDeleteExceptionPtr(OSMemoryExceptionPtr p); // Delete the OS-specific memory execption
	static void			OSDeleteExceptionPtr(OSFileExceptionPtr p); // Delete the OS-specific file execption
	static cppstring	OSGetExceptionErrorString(OSMemoryExceptionPtr p); // return a cppstring of the error in the passed-in memory exception
	static cppstring	OSGetExceptionErrorString(OSFileExceptionPtr p); // return a cppstring of the error in the passed-in file exception

//	static StdIDVector	ResolveIDVector(StdIDVector *pvIDs); // RETURN a StdIDVector based on passed-in vector
//	static StdIDVector	ConvertIDListToVector(const StdIDList * pIDList); // RETURN a StdIDVector based on a passed-in StdIDList
//	static StdIDList	ConvertIDVectorToList(const StdIDVector * pIDList); // RETURN a StdIDList based on a passed-in StdIDVector

	// Date and time
	static cppstring	OSGetBuildDateTimeText(void);  // RETURN the current date and time this module was compiled as a cppstring
	static cppstring	GetCurrentStdDateTimeText(void); // uses standard C facilities
	static cppstring	GetCurrentStdTimeText(void); // uses standard C facilities

	static real			OSGetSystemClockTime(void); // return time since system startup in seconds, with at least ms resolution
	static unsigned int	OSGetTimeStamp(void); // RETURN a time stamp (in milliseconds)

	// application specific strings
	static cppstring	GetApplicationString(const cppstring & sKey);
	static cppstring	GetApplicationName(void);
	static cppstring	GetApplicationISBN(void);
	static cppstring	GetNativeFileExtension(void);
	static cppstring	GetNativeFileFilter(bool bOpen);
	
	// System Info
	static cppstring	OSGetUserName(void);
	static bool			OSIsQuickTimeAvailable(void);
	static bool			OSAllowThreadedFirmwareUpdate(void);

	// Default Folder 
	static void			OSSetDefaultFolder(const GFileRef & theFolderRef);

	// App Error
	static void			OSAssertDialog(SAppError * pAppError);

	// Unique ID
	static cppstring	GetUniqueIDString(void); // RETURN a unique string for an ID

	// File IO
	static cppstring	OSChooseTextFileForImport(void);

	// Checksum:
	static short		MakeDataChecksum(unsigned char * pBuffer, unsigned short nSize);

	// Byte order
	static bool			OSConvertShortToBytes(short nInNum, unsigned char * pLSB, unsigned char * pMSB);
	static bool			OSConvertBytesToShort(char chLSB, char chMSB, short * pOutShort);

	static bool			OSConvertIntToBytes(int nInNum, 
											unsigned char * pLSB,
											unsigned char * pLMidB,
											unsigned char * pMMidB,
											unsigned char * pMSB);
														
	static bool			OSConvertBytesToInt(unsigned char chLSB,
											unsigned char chLMidB,
											unsigned char chMMidB,
											unsigned char chMSB,
											int * pOutInt);
	
	static bool			OSConvertFloatToBytes(float fInNum, 
											unsigned char * pLSB,
											unsigned char * pLMidB,
											unsigned char * pMMidB,
											unsigned char * pMSB);
														
	static bool			OSConvertBytesToFloat(unsigned char chLSB,
											unsigned char chLMidB,
											unsigned char chMMidB,
											unsigned char chMSB,
											float * pOutFloat);

	static int			OSConvertIntelIntToPlatformInt(int nSourceInt);
	static int			OSConvertMacIntToPlatformInt(int nSourceInt);
	
	static bool			OSIsMainThread(void);
																								
	// ID Vectors and list back and forth to/from strings
//	static cppstring	ConvertIDVectorToString(const StdIDVector &idVect);
//	static StdIDVector	ConvertStringToIDVector(const cppstring &sVect);
//	static cppstring	ConvertIDListToString(const StdIDList &idList);
//	static StdIDList	ConvertStringToIDList(const cppstring &sList);
//	static cppstring	ConvertIDPairsToString(const SStdIDPairVector &vIDPairs);
//	static SStdIDPairVector	ConvertStringToIDPairs(const cppstring &sVect);
//
//	static StdIDVector  GetUniqueIDs(const StdIDVector &vIDs);
//	static StringVector GetUniqueStrings(const StringVector &vs);
//	static StringVector GetNewStrings(const StringVector &oldVec, const StringVector &newVec);
//	static StringVector GetOldStrings(const StringVector &oldVec, const StringVector &newVec);
//	static StdIDList	GetPrunedIDList(StdIDList theList);
//	static StdIDVector	GetPrunedIDVector(StdIDVector *pvIDs);

	static realvector	ConvertStringToRealVector(const cppstring & sText);
	static cppstring	ConvertRealVectorToString(const realvector & vfVals);
	static intVector	ConvertStringToIntVector(const cppstring & sText);
	static cppstring	ConvertIntVectorToString(const intVector & vnVals);

	static cppstring	ConvertBytesToString(unsigned char *pBytes, int nNumBytes);

	// Keyboard event conversion
	static EKeyboardEvent OSTranslateKeyboardEvent(void *pData);

	// Curve Fitting functions and helper functions.
	static long			CalculateLinearFit(SRealPointVector const &vPoints, real *pSlope, real *pIintercept, real *pCorrelation = NULL, real *pfStdDevSlope = NULL, real *pfStdDevIntercept = NULL); // RETURN kResponse_OK or IDSX error string ID
	static long			CalculateLinearFit(realvector const &vX, realvector const &vY, real *pSlope, real *pIintercept, real *pCorrelation = NULL, real *pfStdDevSlope = NULL, real *pfStdDevIntercept = NULL); // RETURN kResponse_OK or IDSX error string ID

	static bool			IsMonotonic(SRealPointVector const &vPoints);	// RETURN true if x component of points is monotonic (not both increasing and decreasing)
	static bool			IsMonotonic(realvector const &vReal); // RETURN true if points are monotonic

	static int			GetDecimalPlacesUsed(real fValue);	// RETURN the number of places that are not 0
	static size_t		CalculateNumberOfDivisions(real fStart, real fEnd, real fIncrement); // RETURN number of increments to use

#ifdef TARGET_OS_WIN
	static bool			OSIsValidNumber(real fVal) { return (!_isnan(fVal) && _finite(fVal)); } // RETURN true if fVal is OK
#endif

#ifdef TARGET_OS_MAC
	#if TARGET_CPU_X86
		static bool			OSIsValidNumber(real fVal) { return (!std::isnan(fVal) && std::isfinite(fVal)); } // RETURN true if fVal is OK
	#else 
		static bool			OSIsValidNumber(real fVal) { return (!__isnand(fVal) && __isfinited(fVal)); } // RETURN true if fVal is OK
	#endif
#endif

#ifdef TARGET_OS_LINUX
	static bool			OSIsValidNumber(real fVal) { return (!isnan(fVal) && isfinite(fVal)); } // RETURN true if fVal is OK
#endif

	static real			kRealMin;
	static real			kRealMax;
	static real			kfMetersPerFoot;
	static real			RestrictReal(real fValue); // return fValue restricted to the range defined by kRealMin and kRealMax
	static bool			RealEquivalence(real fTolerance, real fValue1, real fValue2); // returns true if to real as equivlent withint the specificed tolerance

	static cppstring	GetStringFromTimeUnit(ETimeUnit eTimeUnit, bool bAbbreviation = true, bool bPlural = true);
	static StringVector	GetTimeUnitStrings(bool bAbbreviation = true, bool bPlural = true);
	static ETimeUnit	GetTimeUnitFromString(const cppstring & sUnit, bool bStrict = false);
	static SColumnHeader	GetTimeColumnHeader(ETimeUnit eTime);
	static real			ConvertTimeToSeconds(real fTime, ETimeUnit eTimeUnit); 
	static real			ConvertSecondsToTime(real fTime, ETimeUnit eTimeUnit); 
	static real			ConvertRateInSecondsToTime(real fRateInSeconds, ETimeUnit eTimeUnit);

	static real			ConvertFeetToMeters(real fFeet) { return fFeet * 0.3048; }
	static real			ConvertMetersToFeet(real fMeters) { return fMeters / 0.3048; }

//	static StdIDVector	GetIDVectorIntersection(StdIDVector const &vIDs1, StdIDVector const &vIDs2); // RETURN the IDs that are in both v1 and v2

	static cppstring	ToLower(const cppstring &s); // RETURN s, in lower case
	static narrowstring	ToNarrowLower(const narrowstring &s); // RETURN s, in lower case

	// Logfile methods
	static cppofstream *	pLogOStream;
	static bool			IsLogOpen(void);
	static cppstring	OpenLog(void);
	static void			CloseLog(void);
	static void			WriteToLog(cppstring sText, cppstring sFile, int nLine, cppstring sFunction);
	static void			WriteAddressToLog(void * pointer, cppstring sFile, int nLine);

	// Used to calculate Live Readouts strings for the toolbar.
	static StringVector GenerateIdealStringVector(OSPtr pOSData, int nIdealSize, int *nActualSize, const StringVector & vsLongStrings, const StringVector & vsShortStrings,int nLongSize, int nShortSize);
	static int			CalculateStringVectorSize(OSPtr pOSData, int nSeperatorSize, const StringVector & vsStrings);	
};

// class to stop and then restart MBL related things (live readouts and initial sensor polling)
class StStopMBLActivity
{
public:
					StStopMBLActivity(void);
	virtual			~StStopMBLActivity();

private:
	bool			m_bRestartLiveReadouts;
};

#ifdef TARGET_OS_WIN
#ifdef _DEBUG
#define OPUS_NEW DEBUG_NEW
#else
#define OPUS_NEW new
#endif
#endif

// global allocation macro
#define GSTD_NEW(pointer, cast, constructor) pointer = cast OPUS_NEW constructor; // GSTD_LOGADDRESS(pointer); // uncomment to track memory allocations in the logfile

#ifdef TARGET_OS_WIN
#define GSTD_LOG(x) if (GUtils::IsLogOpen()) GUtils::WriteToLog((x), WIDENMACRO(__FILE__), __LINE__, GSTD_S(""));	// No function macro on Windows.  bummer.
#else
#define GSTD_LOG(x) if (GUtils::IsLogOpen()) GUtils::WriteToLog((x), WIDENMACRO(__FILE__), __LINE__, WIDENMACRO(__FUNCTION__));
#endif

#define GSTD_LOGADDRESS(x)if (GUtils::IsLogOpen()) GUtils::WriteAddressToLog((x), WIDENMACRO(__FILE__), __LINE__);	


#endif // _GUTILS_H_
