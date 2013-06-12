// GoIO_console.h : main header file for the GOIO_CONSOLE application
//

#if !defined(AFX_GOIO_CONSOLE_H__93F49ED4_4AB9_450A_9B22_9CB8D8317D4A__INCLUDED_)
#define AFX_GOIO_CONSOLE_H__93F49ED4_4AB9_450A_9B22_9CB8D8317D4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#pragma warning (disable: 4786)

// C

#include <cstdio>

// put cstdlib in namespace std (VC++ bug):
namespace std {
#ifndef _CSTDLIB_
#include <cstdlib>
#endif
}; // end namespace std

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


/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleApp:
// See GoIO_console.cpp for the implementation of this class
//

class CGoIO_consoleApp : public CWinApp
{
public:
	CGoIO_consoleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoIO_consoleApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CGoIO_consoleApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOIO_CONSOLE_H__93F49ED4_4AB9_450A_9B22_9CB8D8317D4A__INCLUDED_)
