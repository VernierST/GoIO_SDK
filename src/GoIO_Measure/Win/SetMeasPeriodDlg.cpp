// SetMeasPeriodDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GoIO_Measure.h"
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
	m_period = 0;
	//}}AFX_DATA_INIT

	m_measurementUnits = "ms";
}


void CSetMeasPeriodDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetMeasPeriodDlg)
	DDX_Text(pDX, IDC_PERIOD, m_period);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetMeasPeriodDlg, CDialog)
	//{{AFX_MSG_MAP(CSetMeasPeriodDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetMeasPeriodDlg message handlers

BOOL CSetMeasPeriodDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CWnd *pWnd = GetDlgItem(IDC_PERIOD_LABEL);
	if (pWnd)
	{
		char tmpstring[100];
		wsprintf(tmpstring, "Measurement Period ( %s )", m_measurementUnits);
		pWnd->SetWindowText(tmpstring);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
