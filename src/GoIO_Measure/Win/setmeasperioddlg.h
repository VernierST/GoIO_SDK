#if !defined(AFX_SETMEASPERIODDLG_H__1A27C70B_6F0B_47E1_B9F1_A57E7AE47AF1__INCLUDED_)
#define AFX_SETMEASPERIODDLG_H__1A27C70B_6F0B_47E1_B9F1_A57E7AE47AF1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SetMeasPeriodDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetMeasPeriodDlg dialog

class CSetMeasPeriodDlg : public CDialog
{
// Construction
public:
	CSetMeasPeriodDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetMeasPeriodDlg)
	enum { IDD = IDD_SET_MEASUREMENT_PERIOD };
	UINT	m_period;
	//}}AFX_DATA


	void SetUnits(const CString &units)
	{
		m_measurementUnits = units;
	}
	void GetUnits(CString *pUnits)
	{
		*pUnits = m_measurementUnits;
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetMeasPeriodDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetMeasPeriodDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CString m_measurementUnits;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETMEASPERIODDLG_H__1A27C70B_6F0B_47E1_B9F1_A57E7AE47AF1__INCLUDED_)
