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
