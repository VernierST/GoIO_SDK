// SetMeasPeriodDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GoIO_console.h"
#include "SetMeasPeriodDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetMeasPeriodDlg dialog


CSetMeasPeriodDlg::CSetMeasPeriodDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetMeasPeriodDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetMeasPeriodDlg)
	m_period_ms = 0;
	//}}AFX_DATA_INIT
}


void CSetMeasPeriodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetMeasPeriodDlg)
	DDX_Text(pDX, IDC_EDIT1, m_period_ms);
	DDV_MinMaxUInt(pDX, m_period_ms, 5, 60000);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetMeasPeriodDlg, CDialog)
	//{{AFX_MSG_MAP(CSetMeasPeriodDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetMeasPeriodDlg message handlers
