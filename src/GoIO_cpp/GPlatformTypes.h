// GPlatformTypes.h
//
// This file defines void pointers for common platform- or framework-specific
// types.  These independent pointer names allow indirect (parameter) use of
// these types in platform-independent code without requiring a full-fledged
// layer of abstraction.

#ifndef _GPLATFORMTYPES_H_
#define _GPLATFORMTYPES_H_

#ifndef _GSTDINCLUDES_H_
#include "GStdIncludes.h"
#endif

/////////////////////////////////////// EXPLICIT SYSTEM TYPES
#ifdef TARGET_OS_MAC
typedef struct SystemRect
{
	int x, y, w, h;
} SystemRect;

typedef struct SystemPoint
{
	int x, y;
} SystemPoint;
#else 
#ifdef TARGET_OS_WIN
// Windows System types...
#include "stdafx.h"
#endif
//
typedef CRect SystemRect;
typedef CPoint SystemPoint;
//
#endif // TARGET_OS_MAC || TARGET_OS_WIN
//////////////////////////////////////////////////////////////


/////////////////////////////////////// EXCEPTION OBJECT TYPES
#ifdef TARGET_OS_WIN
typedef CMemoryException * OSMemoryExceptionPtr; 
typedef CFileException * OSFileExceptionPtr;
typedef CException OSExceptionClass;
#endif

#ifdef TARGET_OS_MAC
typedef std::bad_alloc OSMemoryExceptionPtr;
typedef std::bad_exception OSFileExceptionPtr;
typedef void * OSExceptionClass;
#endif

#ifdef TARGET_OS_LINUX
typedef std::bad_alloc OSMemoryExceptionPtr;
typedef std::exception * OSFileExceptionPtr;
typedef std::exception OSExceptionClass;
#endif


//////////////////////////////////////////////////////////////


/////////////////////////////////////// VOID OS TYPES
//
typedef void * 		OSPtr; // any platform-dependent reference
//
typedef OSPtr 		OSApplicationPtr;	// the framework-defined application object
typedef OSPtr 		OSFrameWindowPtr;	// our main window object type
typedef OSPtr 		OSDialogPtr;		// a dialog object
typedef OSPtr 		OSDataPtr;			// OS-specific data for an object (Canvas ptr, etc.)
typedef OSPtr		OSDrawContextPtr;	// a drawing context (CDC, GrafPtr, etc.)
typedef OSPtr		OSPenPtr;			// a pen, brush, or pen-state object
typedef OSPtr		OSBitPatternPtr;	// a drawing-pattern bitmap
typedef OSPtr		OSRegionPtr;		// a rect or region
typedef OSPtr		OSStringPtr;		// a string object (CString, LString)
typedef OSPtr		OSPascalString;		// a pascal string (Mac only, but distinguish from LString)
typedef OSPtr		OSFilePtr;			// a file object or structure (CFile, FSSpec)
typedef OSPtr		OSFilePath;			// a platform-format file path string
typedef OSPtr		OSPicturePtr;		// a platform-specific bitmap or picture format
typedef OSPtr		OSRectPtr;			// a CRect (MFC) or Rect (MacOS)
typedef OSPtr		OSFontAttrPtr;		// some structure representing text font, size, weight
typedef OSPtr		OSSerialConfigPtr;	// some structure representing serial configuration (handshaking)
typedef OSPtr		OSExceptionPtr;
//
typedef long		OSHandle;			// for any sort of handle
typedef long		OSMessage;			// for framework-defined message or command value
//////////////////////////////////////////////////////////////

#endif // _GPLATFORMTYPES_H_