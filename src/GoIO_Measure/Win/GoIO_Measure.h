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
// GoIO_Measure.h : main header file for the GoIO_Measure application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

#include <cstdio>

#include <cmath>
#include <cstring>
#include <cctype>
#include <ctime>
#include <exception>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
#include <limits>

// arrgh - the compiler particulars got us, we have to introduce
// namespace std here...
using namespace std;


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
typedef std::wstring			widestring;

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

#include "GoIO_DLL_interface.h"

// CGoIO_MeasureApp:
// See GoIO_Measure.cpp for the implementation of this class
//

class CGoIO_MeasureApp : public CWinApp
{
public:
	CGoIO_MeasureApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CGoIO_MeasureApp theApp;