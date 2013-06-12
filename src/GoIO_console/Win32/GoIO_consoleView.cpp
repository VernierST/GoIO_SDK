// GoIO_consoleView.cpp : implementation of the CGoIO_consoleView class
//

#include "stdafx.h"
#include "GoIO_console.h"

#include "GoIO_consoleDoc.h"
#include "MainFrm.h"
#include "GoIO_consoleView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView

IMPLEMENT_DYNCREATE(CGoIO_consoleView, CListView)

BEGIN_MESSAGE_MAP(CGoIO_consoleView, CListView)
	//{{AFX_MSG_MAP(CGoIO_consoleView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView construction/destruction

CGoIO_consoleView::CGoIO_consoleView()
{
	// TODO: add construction code here

}

CGoIO_consoleView::~CGoIO_consoleView()
{
}

BOOL CGoIO_consoleView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT;

	return CListView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView drawing

void CGoIO_consoleView::OnDraw(CDC* pDC)
{
	CGoIO_consoleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView diagnostics

#ifdef _DEBUG
void CGoIO_consoleView::AssertValid() const
{
	CListView::AssertValid();
}

void CGoIO_consoleView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CGoIO_consoleDoc* CGoIO_consoleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGoIO_consoleDoc)));
	return (CGoIO_consoleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView message handlers

#define NUM_COLUMNS 5

static _TCHAR *_gszColumnLabel[NUM_COLUMNS] =
{
	_T("Time"), _T("IN/OUT"), _T("# bytes"), _T("Ascii"), _T("Hex")
};

static int _gnColumnFmt[NUM_COLUMNS] =
{
	LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_RIGHT, LVCFMT_LEFT, LVCFMT_LEFT
};

static int _gnColumnWidth[NUM_COLUMNS] =
{
	80, 60, 60, 300, 700
};

void CGoIO_consoleView::OnInitialUpdate() 
{
	CListView::OnInitialUpdate();
	
	CListCtrl& ListCtrl = GetListCtrl();
	if (ListCtrl.GetItemCount() > 0)
		ListCtrl.DeleteAllItems();
	else
	{
		int i;
		LV_COLUMN lvc;

		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

		for(i = 0; i<NUM_COLUMNS; i++)
		{
			lvc.iSubItem = i;
			lvc.pszText = _gszColumnLabel[i];
			lvc.cx = _gnColumnWidth[i];
			lvc.fmt = _gnColumnFmt[i];
			ListCtrl.InsertColumn(i,&lvc);
		}
	}
}

void CGoIO_consoleView::RecordGoIOOutput(LPCSTR pBuf, int buf_len)
{
	RecordGoIOString(pBuf, buf_len, "OUT");
}

void CGoIO_consoleView::RecordGoIOInput(LPCSTR pBuf, int buf_len)
{
	RecordGoIOString(pBuf, buf_len, "IN");
}

void CGoIO_consoleView::RecordGoIOString(LPCSTR pBuf, int buf_len, LPCSTR label)
{
	CListCtrl& ListCtrl = GetListCtrl();
	CTime theTime;
	theTime = CTime::GetCurrentTime();
	CString time_string = theTime.Format("%H:%M:%S");
	int new_index = ListCtrl.GetItemCount();
	int i, record_len;
	CString short_string;
	char tmpstring[100];

	while (buf_len > 0)
	{
		ListCtrl.InsertItem(new_index, (LPCSTR) time_string);
		ListCtrl.SetItemText(new_index, 1, label);
		record_len = buf_len;
		if (record_len > REPORT_RECORD_DATA_LENGTH)
			record_len = REPORT_RECORD_DATA_LENGTH;
		wsprintf(tmpstring, "%d", record_len);
		ListCtrl.SetItemText(new_index, 2, tmpstring);

		memcpy(tmpstring, pBuf, record_len);
		for (i = 0; i < record_len; i++)
		{
			if (tmpstring[i] == 0)
				tmpstring[i] = '.';
		}
		short_string = CString(tmpstring, record_len);
		ListCtrl.SetItemText(new_index, 3, (LPCSTR) short_string);

		short_string = "";
		for (i = 0; i < record_len; i++)
		{
			wsprintf(tmpstring, "%02x ", (BYTE) (pBuf[i]));
			short_string += tmpstring;
		}
		ListCtrl.SetItemText(new_index, 4, (LPCSTR) short_string);
		ListCtrl.EnsureVisible(new_index, FALSE);

		buf_len -= record_len;
		pBuf += record_len;
		new_index++;
	}
}

void CGoIO_consoleView::RecordGoIOComment(LPCSTR pBuf)
{
	CListCtrl& ListCtrl = GetListCtrl();
	CTime theTime;
	theTime = CTime::GetCurrentTime();
	CString time_string = theTime.Format("%H:%M:%S");
	int new_index = ListCtrl.GetItemCount();

	ListCtrl.InsertItem(new_index, (LPCSTR) time_string);
	ListCtrl.SetItemText(new_index, 1, "");
	ListCtrl.SetItemText(new_index, 2, "");
	ListCtrl.SetItemText(new_index, 3, pBuf);
	ListCtrl.SetItemText(new_index, 4, "");
	ListCtrl.EnsureVisible(new_index, FALSE);
}
