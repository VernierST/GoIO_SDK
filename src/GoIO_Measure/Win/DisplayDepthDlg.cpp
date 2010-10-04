// DisplayDepthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GoIO_Measure.h"
#include "DisplayDepthDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDisplayDepthDlg dialog


CDisplayDepthDlg::CDisplayDepthDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayDepthDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDisplayDepthDlg)
	m_displayDepth = 0;
	//}}AFX_DATA_INIT
}


void CDisplayDepthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDisplayDepthDlg)
	DDX_Text(pDX, IDC_DEPTH, m_displayDepth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDisplayDepthDlg, CDialog)
	//{{AFX_MSG_MAP(CDisplayDepthDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisplayDepthDlg message handlers
