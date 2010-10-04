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