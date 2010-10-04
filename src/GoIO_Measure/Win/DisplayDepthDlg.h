#if !defined(AFX_DISPLAYDEPTHDLG_H__07240A63_627F_45A4_A127_8BD050800F26__INCLUDED_)
#define AFX_DISPLAYDEPTHDLG_H__07240A63_627F_45A4_A127_8BD050800F26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisplayDepthDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDisplayDepthDlg dialog

class CDisplayDepthDlg : public CDialog
{
// Construction
public:
	CDisplayDepthDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDisplayDepthDlg)
	enum { IDD = IDD_SET_DISPLAY_DEPTH };
	UINT	m_displayDepth;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisplayDepthDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDisplayDepthDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYDEPTHDLG_H__07240A63_627F_45A4_A127_8BD050800F26__INCLUDED_)
