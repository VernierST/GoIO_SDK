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
#if defined (TARGET_OS_MAC) || defined (TARGET_OS_LINUX)
typedef struct SystemRect
{
	int x, y, w, h;
} SystemRect;

typedef struct SystemPoint
{
	int x, y;
} SystemPoint;
#endif  //TARGET_OS_MAC | TARGET_OS_LINUX

#ifdef TARGET_OS_WIN
// Windows System types...
#include "stdafx.h"
//
typedef CRect SystemRect;
typedef CPoint SystemPoint;
#endif  //TARGET_OS_WIN
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
typedef int			OSHandle;			// for any sort of handle
typedef int			OSMessage;			// for framework-defined message or command value
//////////////////////////////////////////////////////////////

#endif // _GPLATFORMTYPES_H_
