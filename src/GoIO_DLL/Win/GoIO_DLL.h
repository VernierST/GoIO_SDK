// GoIO_DLL.h : main header file for the GOIO_DLL DLL
//

#if !defined(AFX_GOIO_DLL_H__2309DD8B_A4AA_4A48_808F_774B84C25AA1__INCLUDED_)
#define AFX_GOIO_DLL_H__2309DD8B_A4AA_4A48_808F_774B84C25AA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CGoIO_DLLApp
// See GoIO_DLL.cpp for the implementation of this class
//

class CGoIO_DLLApp : public CWinApp
{
public:
	CGoIO_DLLApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoIO_DLLApp)
	public:
	virtual int ExitInstance();
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CGoIO_DLLApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOIO_DLL_H__2309DD8B_A4AA_4A48_808F_774B84C25AA1__INCLUDED_)
