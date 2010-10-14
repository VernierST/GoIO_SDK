// GTypes.h
//
// These are general cross-platform types and enumerations.
// Object-specific types and enumerations are generally
// declared in the relevant class header.
//
// RELATED FILES:
// ---------------
// For runtime type constants for standard objects, see
// GStdObjectTypes.h.
//
// For void-pointer masks of platform-dependent types, see
// GPlatformTypes.h.
///////////////////////////////////////////////////////////////

#ifndef _GTYPES_H_
#define _GTYPES_H_

#ifdef TARGET_OS_WIN
#pragma warning (disable: 4786)
#endif

#include "GStdIncludes.h"
#include "GPlatformTypes.h"

// MFC-Helper:
#ifdef TARGET_OS_WIN
#include "stdafx.h"
#endif

#ifdef TARGET_OS_WIN
#define OS_STANDARD_BASE_CLASS : public CObject
#else
#define OS_STANDARD_BASE_CLASS
#endif

#ifdef _UNICODE
#define USE_WIDE_CHARS 1
#endif

#ifdef USE_WIDE_CHARS
typedef std::wstring			cppstringBase;
typedef std::wstringstream		cppsstream;
typedef wchar_t					gchar;
typedef std::wistream			cppistream;
typedef std::wostream			cppostream;
typedef std::wofstream			cppofstream;
typedef std::wfstream			cppfstream;
typedef std::wiostream			cppiostream;
typedef std::wstreambuf			cppstreambuf;
#define GSTD_S(x)				L##x 
#else
typedef std::string				cppstringBase;
typedef std::stringstream		cppsstream;
typedef char					gchar;
typedef std::istream			cppistream;
typedef std::ostream			cppostream;
typedef std::ofstream			cppofstream;
typedef std::fstream			cppfstream;
typedef std::iostream			cppiostream;
typedef std::streambuf			cppstreambuf;

#define GSTD_S(x)				x
#endif

typedef std::string				narrowstring;
// typedef std::wstring			widestring;

// This class upgrades the standard strings to be able take a resource ID or character when creating a string
class cppstring : public cppstringBase
{ // NOTE definition is in GTextUtils.cpp
public:
				cppstring(void) : cppstringBase() {};
				cppstring(cppstringBase sBase) : cppstringBase(sBase) {};
				cppstring(gchar *pChar) : cppstringBase(pChar) {};
				cppstring(const gchar *pChar) : cppstringBase(pChar) {};
				cppstring(const gchar *pChar, int nNum) : cppstringBase(pChar, nNum) {};
#ifdef TARGET_OS_WIN
				cppstring(CString cString) : cppstringBase(cString) {};
#endif
				cppstring(gchar c);
};



// Standard message responses
enum
{
	kResponse_Error 			= -1,
	kResponse_OK 				= 0L, 	// no error, no exclusive handling
	kResponse_MessageHandled 	= 1	// exclusive handling
};

// Stack based Wait cursor
#ifdef TARGET_OS_WIN
typedef CWaitCursor GStWaitCursor;
#endif

#ifdef TARGET_OS_MAC
class GStWaitCursor
{
public:
	GStWaitCursor();
	virtual ~GStWaitCursor();
	
	static int	sNestLevel;
};
#endif

// This enum is used by the YesNoCancel MessageBox methods
typedef enum
{
	kYesNoCancel_Yes = 0,
	kYesNoCancel_No,
	kYesNoCancel_Cancel
} EYesNoCancel;

// This enum is used by the generic MessageBox methods
typedef enum
{
	kCustomMessage_Button1,
	kCustomMessage_Button2,
	kCustomMessage_Button3
} ECustomMessage;

typedef std::vector<bool>		boolVector;

typedef double 					real;
typedef std::vector<real>		realvector;
typedef std::vector<realvector> realmatrix;
typedef std::vector<int> 		intvector;
typedef std::list<real>			reallist;
typedef std::list<real>::iterator reallistIterator;

typedef int NFontSize;

#ifdef TARGET_OS_WIN
#define NAN std::numeric_limits<real>::quiet_NaN()
#endif

#ifdef TARGET_OS_WIN
typedef CMenu *		OSMenuPtr;
typedef time_t		stdtime_t;
#else
	#ifdef TARGET_OS_MAC
	typedef time_t		stdtime_t;
	#else
	typedef std::time_t stdtime_t;
	#endif
typedef void *		OSMenuPtr;
#endif

typedef std::vector<cppstring>			StringVector;
typedef std::list<cppstring>			StringList;
typedef std::list<cppstring>::iterator	StringListIterator;
// string list for narrow strings only
typedef std::list<std::string>				AsciiStringList;
typedef std::list<std::string>::iterator	AsciiStringListIterator;
typedef std::vector<std::string>			AsciiStringVector;

typedef std::vector<int>				intVector;
typedef std::list<int>					intList;

////////////////////////// DRAWING & COORDINATE STRUCTURES


// } SColor;
struct SColor
{
	SColor() { red = green = blue = 0; }

	unsigned short red;
	unsigned short green;
	unsigned short blue;
};

// SColor operators:
extern cppostream & operator<<(cppostream & s, const SColor & c);
extern cppistream & operator>>(cppistream & s, SColor & c);
extern bool operator==(const SColor & c1, const SColor & c2);
extern bool operator!=(const SColor & c1, const SColor & c2);

typedef std::vector<SColor> SColorVector;

// } SRealPoint;
struct SRealPoint
{
	SRealPoint() { x = y = 0.0; }
	SRealPoint(real xx, real yy) { x = xx; y = yy; }

	real x;
	real y;
};

typedef std::vector<SRealPoint> SRealPointVector;

// } SDevicePoint;
struct SDevicePoint
{
	SDevicePoint() { x = y = 0; }
	SDevicePoint(int xx, int yy) { x = xx; y = yy; }

	int x;
	int y;
};
// ooperators:
extern bool operator==(const SDevicePoint & pt1, const SDevicePoint & pt2);
extern bool operator!=(const SDevicePoint & pt1, const SDevicePoint & pt2);

typedef std::vector<SDevicePoint> SDevicePointVector;

// ...see also GRealRect.h

////////////////////////// DRAWING ENUMERATIONS

typedef enum
{
	kCorner_TopLeft,
	kCorner_TopRight,
	kCorner_BottomLeft,
	kCorner_BottomRight
} ECorner;

typedef enum
{
	kSide_Left,
	kSide_Top,
	kSide_Right,
	kSide_Bottom
} ESide;

typedef enum
{
	RectAnchor_default = 0,
	RectAnchor_topLeft = RectAnchor_default,
	RectAnchor_center,
	RectAnchor_top,
	RectAnchor_left,
	RectAnchor_bottom,
	RectAnchor_right,
	RectAnchor_topRight,
	RectAnchor_bottomLeft,
	RectAnchor_bottomRight
} ERectAnchor;

// These values correspond with logical units AND there are the same number as we offer the user in the UI
typedef enum 
{
	kLine_HairLine = 0,
	kLine_1Pt,
	kLine_2Pt,
	kLine_3Pt,
	kLine_4Pt,
	kLine_5Pt,
	kLine_6Pt,
	kLine_numLineThicknessOptions
} EThickness;

typedef enum
{
	kColor_transparent = -2,
	kColor_custom = -1,
	kColor_pink,
	kColor_darkPink,
	kColor_red,
	kColor_tomato,
	kColor_brickRed,
	kColor_darkRed,
	kColor_cream,
	kColor_lemon,
	kColor_yellow,
	kColor_goldenrod,
	kColor_peach,
	kColor_butterscotch,
	kColor_orange,
	kColor_darkOrange,
	kColor_pumpkin,
	kColor_lightGreen,
	kColor_paleGreen,
	kColor_green,
	kColor_teal,
	kColor_hunterGreen,
	kColor_darkGreen,
	kColor_skyBlue,
	kColor_steelBlue,
	kColor_azure,
	kColor_royalBlue,
	kColor_blue,
	kColor_darkBlue,
	kColor_cobalt,
	kColor_blueZircon,
	kColor_tanzanite,
	kColor_cornflower,
	kColor_midnightBlue,
	kColor_indigo,
	kColor_thistle,
	kColor_lavender,
	kColor_violet,
	kColor_pansy,
	kColor_plum,
	kColor_darkPurple,
	kColor_taupe,
	kColor_bisque,
	kColor_white,
	kColor_lightGray,
	kColor_gray,
	kColor_darkGray,
	kColor_graphite,
	kColor_black,

	kColor_numStandardColors

} EColor;

typedef enum 
{
	kCursor_Standard,
	kCursor_ResizeNWSE,
	kCursor_ResizeNESW,
	kCursor_ResizeWE,
	kCursor_ResizeNS,
	kCursor_MoveObject,
	kCursor_MovingObject,
	kCursor_CrossHairs,
	kCursor_IBeam,
	kCursor_Plus,
	kCursor_MoveGraph,
	kCursor_IllegalDropSingle,
	kCursor_GoodDropSingle,
	kCursor_GoodXAxisDropSingle,
	kCursor_GoodYAxisDropSingle,
	kCursor_GoodRAxisDropSingle,
	kCursor_GoodTAxisDropSingle,
	kCursor_IllegalDrop,
	kCursor_GoodDrop,
	kCursor_GoodXAxisDrop,
	kCursor_GoodYAxisDrop,
	kCursor_GoodRAxisDrop,
	kCursor_GoodTAxisDrop,
	kCursor_StretchyAxisWE,
	kCursor_StretchyAxisNS,
	kCursor_GrabbyLeftBracket,
	kCursor_GrabbyRightBracket,
	kCursor_XOptions,
	kCursor_YOptions,
	kCursor_ResizeLine,
	kCursor_ScrollLeft,
	kCursor_ScrollRight,
	kCursor_ScrollUp,
	kCursor_ScrollDown,
	kCursor_ResizeColumn,
	kCursor_CloseBox,
	kCursor_MinimizeBox,
	kCursor_DrawPrediction,
	kCursor_GoodSensorDrop,
	kCursor_IllegalSensorDrop,
	kCursor_VAAddPoint,
	kCursor_Wait
} ECursorType;

typedef enum
{
	kImportFormat_None,
	kImportFormat_LoggerPro2, 
	kImportFormat_GA2Mac,
	kImportFormat_GA2Windows,
	kImportFormat_RawText,
	//kImportFormat_RawTextWithHeaders,  // tabbed text preceeded by three line of header not supported 
	kImportFormat_SingleColNameHeader // any single header 

} EImportTextFormat;

typedef enum
{
	kFloatStyle_default,
	kFloatStyle_fixedPrecision,
	kFloatStyle_fixedSigFigs,
	kFloatStyle_preferred // uses fixedPrecision until the precision is greater than 7, then default
} EFloatStyle;

////////////////////////// MOUSE STATE
// } SMouseState; 
struct SMouseState
{
	SMouseState() { bShift = bControl = bMouseDown = false; }

	bool			bShift;		// was the shift key pressed when this mouse event happened?
	bool			bControl;	// was the Ctrl key pressed when this mouse event happened?
	bool			bMouseDown;  // mouse button is down
	SDevicePoint	dPoint;		// the point (in device coords) where the mouse event took place
};

// Used for mouse click handling and resizing objects
typedef enum
{
	kPoint_NotOverObject = -2,	// point is not involved with this object at all
	kPoint_MoveArea,			// point is on a border (but not a handle)
	kPoint_TopLeftHandle = 0,		// Point is on a handle...
	kPoint_TopRightHandle,
	kPoint_TopMiddleHandle,
	kPoint_BottomLeftHandle,
	kPoint_BottomRightHandle,
	kPoint_BottomMiddleHandle,
	kPoint_LeftMiddleHandle,
	kPoint_RightMiddleHandle,
	kPoint_CloseBox,			// used for Helpers
	kPoint_MinimizeBox,			// ditto
	kPoint_Inside				// Point is on the inside of a "mouse smart" object 
} EPointLocation;

// EDataType is used for the data content of
// drag-and-drop and the clipboard...
typedef enum
{
	kDataType_Text,
	kDataType_Number,
	kDataType_DataObjects, // columns and / or data sets
	kDataType_Function,
	kDataType_Picture,
	kDataType_Movie,
	kDataType_URL,
	kDataType_GUIObjects,
	kDataType_OSData
} EDataType;

// These are used by the graphs when examining points
// ....
// REVISIT - this is redundant with GMsg messaging
// constants, remove it & revise code to use messages...
typedef enum
{
	kKeyboardEvent_NullEvent = 0,
	kKeyboardEvent_LeftArrow,
	kKeyboardEvent_RightArrow,
	kKeyboardEvent_UpArrow,
	kKeyboardEvent_DownArrow,
	kKeyboardEvent_Delete,
	kKeyboardEvent_Escape,
	kKeyboardEvent_Return,
	kKeyboardEvent_Tab,
	kKeyboardEvent_PageUp,
	kKeyboardEvent_PageDown
} EKeyboardEvent;

// This enum is used by to get/set how many math functions to show in the rest of the app
typedef enum
{
	kMathType_All = 0,
	kMathType_Basic,
	kMathType_None,
	kNumMathTypes
} EMathType;

///////// tips
// } STips;
struct STips
{
	STips() { bShowTips = false; nTipIndex = 0; }

	bool				bShowTips;
	StringVector		sTipvect;
	unsigned int		nTipIndex;
};

// enum for all the OS's we suport
typedef enum 
{
	kOSType_Unknown = -1,
	kOSType_Mac = 0,
	kOSType_MacOSX,
	kOSType_Windows
} EOSType;

// enum for all creator apps
typedef enum
{
	kAppType_Unknown = -1,
	kAppType_GraphicalAnalysis = 0,
	kAppType_LoggerPro,
	kAppType_LoggerLite
} EAppType;

// Time units
typedef enum
{
	kTimeUnit_millisecond = 0,
	kTimeUnit_second,
	kTimeUnit_minute,
	kTimeUnit_hour,
	kTimeUnit_day,
	kTimeUnit_year,
	kTimeUnit_timeOfDay,
	kTimeUnit_date,
	kTimeUnti_dataTime
} ETimeUnit;

// Distance units
typedef enum
{
	kDistanceUnit_foot = 0,
	kDistanceUnit_meter
} EDistanceUnit;


// This struct is used by MBL code (and others) to setup data columns
// } SColumnHeader;
struct SColumnHeader
{
	SColumnHeader() { }

	cppstring	sName;
	cppstring	sShortName;
	cppstring	sUnits;
};
typedef std::vector<SColumnHeader> SColumnHeaderVector;

// This struct is sent is used when an internal application error is generated
// } SAppError;
struct SAppError
{
	cppstring sErrorMessage;
	cppstring sURL;
};

// Cross-platform exceptions base class:
#ifndef _GEXCEPTION_H_
//#include "GException.h"
#endif

#endif // _GTYPES_H_
