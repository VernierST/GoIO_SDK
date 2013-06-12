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

IMPLEMENT_DYNCREATE(CGoIO_consoleView, CView)

BEGIN_MESSAGE_MAP(CGoIO_consoleView, CView)
	//{{AFX_MSG_MAP(CGoIO_consoleView)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView construction/destruction

CGoIO_consoleView::CGoIO_consoleView()
{
	graph_history_y_min = 1.0;	//Invalid at first.
	graph_history_y_max = -1.0;	//Invalid at first.
}

CGoIO_consoleView::~CGoIO_consoleView()
{
}

BOOL CGoIO_consoleView::PreCreateWindow(CREATESTRUCT& cs)
{

	return CView::PreCreateWindow(cs);
}


/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView drawing

void CGoIO_consoleView::OnDraw(CDC* pDC)
{
	CGoIO_consoleDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	char tmpstring[100];

	CRect clientRect;
	GetClientRect(&clientRect);

	GOIO_SENSOR_HANDLE hDevice = pDoc->GetOpenDevicePtr();
	if (hDevice && (pDoc->GetNumMeasurementsInCirbuf() > 0))
	{
		CRect graphRect = clientRect;
		int meas_count_x_range, i;
		double meas_x_range, meas_x_min, meas_x_max;
		double meas_y_range, meas_y_min, meas_y_max, testy;
		int x, y, deltaX, deltaY;

		//Calculate what portion of the client area will hold the graph.
		if ((graphRect.Width() > 200) && (graphRect.Height() > 200))
		{
			graphRect.top += 25;
			graphRect.bottom -= 50;
			graphRect.left += 100;
			graphRect.right -= 25;
		}

		//Calculate full graph time range.
		int numMeasurements = pDoc->GetNumMeasurementsInCirbuf();
		if (numMeasurements <= 51)
			meas_count_x_range = 51;
		else if (numMeasurements <= 101)
			meas_count_x_range = 101;
		else if (numMeasurements <= 201)
			meas_count_x_range = 201;
		else if (numMeasurements <= 301)
			meas_count_x_range = 301;
		else if (numMeasurements <= 401)
			meas_count_x_range = 401;
		else
			meas_count_x_range = 501;
		meas_x_min = 0.0;
		meas_x_max = pDoc->GetMeasurementPeriodInSeconds()*(meas_count_x_range - 1);
		meas_x_range = meas_x_max - meas_x_min;

		//Calculate full graph y range.
		meas_y_min = pDoc->GetNthMeasurementInCirbuf(0);
		meas_y_max = meas_y_min;
		for (i = 1; i < numMeasurements; i++)
		{
			testy = pDoc->GetNthMeasurementInCirbuf(i);
			if (testy < meas_y_min)
				meas_y_min = testy;
			if (testy > meas_y_max)
				meas_y_max = testy;
		}
		meas_y_range = meas_y_max - meas_y_min;

		//Make sure that meas_y_range corresponds to a voltage delta of at least 0.1 volts.
		double y_0_volts, y_dot1_volts, deltay_dot1_volts;
		y_0_volts = GoIO_Sensor_CalibrateData(hDevice, 2.5);
		y_dot1_volts = GoIO_Sensor_CalibrateData(hDevice, 2.6);
		deltay_dot1_volts = y_dot1_volts - y_0_volts;
		if (deltay_dot1_volts < 0.0)
			deltay_dot1_volts = -deltay_dot1_volts;
		if (deltay_dot1_volts > meas_y_range)
		{
			meas_y_range = deltay_dot1_volts;
			meas_y_max = meas_y_min + meas_y_range;
		}

		double history_y_min;
		double history_y_max;
		GetGraphHistory(history_y_min, history_y_max);
		if (history_y_max > history_y_min)
		{
			if (history_y_max > meas_y_max)
				meas_y_max = history_y_max;
			if (history_y_min < meas_y_min)
				meas_y_min = history_y_min;
			meas_y_range = meas_y_max - meas_y_min;
		}
		SetGraphHistory(meas_y_min, meas_y_max);

		//Do some drawing now.
		pDC->FillRect(&clientRect, CBrush::FromHandle((HBRUSH) ::GetStockObject(WHITE_BRUSH)));

		CPen *pOldPen = pDC->SelectObject(CPen::FromHandle((HPEN) ::GetStockObject(BLACK_PEN)));

		pDC->MoveTo(graphRect.left, clientRect.top);
		pDC->LineTo(graphRect.left, clientRect.bottom);
		pDC->MoveTo(graphRect.right, clientRect.top);
		pDC->LineTo(graphRect.right, clientRect.bottom);

		CPen bluePen;
		bluePen.CreatePen(PS_SOLID, 0, RGB(0, 0, 200));
		pDC->SelectObject(&bluePen);

		pDC->MoveTo(clientRect.left, graphRect.bottom);
		pDC->LineTo(clientRect.right, graphRect.bottom);
		pDC->MoveTo(clientRect.left, graphRect.top);
		pDC->LineTo(clientRect.right, graphRect.top);

		cppsstream ss;
		ss << fixed << showpoint << setprecision(3) << meas_x_min << " secs";
		cppstring str = ss.str();
		lstrcpy(tmpstring, str.c_str());
		x = graphRect.left + 10;
		y = graphRect.bottom + 10;
		pDC->TextOut(x, y, tmpstring, lstrlen(tmpstring));

		ss.str("");
		ss << fixed << showpoint << setprecision(3) << meas_x_max << " secs";
		str = ss.str();
		lstrcpy(tmpstring, str.c_str());
		CRect textRect;
		pDC->DrawText(tmpstring, lstrlen(tmpstring), &textRect, DT_CALCRECT);
		x = graphRect.right - textRect.Width() - 10;
		y = graphRect.bottom + 10;
		pDC->TextOut(x, y, tmpstring, lstrlen(tmpstring));

		float ftemp[3];
		char units[40];
		unsigned char calPageIndex;
		GoIO_Sensor_DDSMem_GetActiveCalPage(hDevice, &calPageIndex);
		GoIO_Sensor_DDSMem_GetCalPage(hDevice, calPageIndex,
			&ftemp[0], &ftemp[1], &ftemp[2], units, sizeof(units));

		pDC->SetTextColor(RGB(0, 0, 200));
		pDC->SetBkMode(TRANSPARENT);

		ss.str("");
		ss << fixed << showpoint << setprecision(3) << meas_y_min << " " << cppstring(units);
		str = ss.str();
		lstrcpy(tmpstring, str.c_str());
		pDC->DrawText(tmpstring, lstrlen(tmpstring), &textRect, DT_CALCRECT);
		x = clientRect.left + 10;
		y = graphRect.bottom - textRect.Height() - 10;
		pDC->TextOut(x, y, tmpstring, lstrlen(tmpstring));

		ss.str("");
		ss << fixed << showpoint << setprecision(3) << meas_y_max << " " << cppstring(units);
		str = ss.str();
		lstrcpy(tmpstring, str.c_str());
		x = clientRect.left + 10;
		y = graphRect.top + 10;
		pDC->TextOut(x, y, tmpstring, lstrlen(tmpstring));


		CPen redPen;
		redPen.CreatePen(PS_SOLID, 3, RGB(200, 0, 0));
		pDC->SelectObject(&redPen);

		double meas_x_frac, meas_y_frac;

		meas_y_frac = (pDoc->GetNthMeasurementInCirbuf(0) - meas_y_min)/meas_y_range;
		deltaY = (int) floor(meas_y_frac*graphRect.Height() + 0.5);
		deltaX = 0;

		x = graphRect.left + deltaX;
		y = graphRect.bottom - deltaY;
		pDC->MoveTo(x, y);
		
		for (i = 0; i < numMeasurements; i++)
		{
			meas_y_frac = (pDoc->GetNthMeasurementInCirbuf(i) - meas_y_min)/meas_y_range;
			deltaY = (int) floor(meas_y_frac*graphRect.Height() + 0.5);
			meas_x_frac = i;
			meas_x_frac = meas_x_frac/(meas_count_x_range - 1);
			deltaX = (int) floor(meas_x_frac*graphRect.Width() + 0.5);
			x = graphRect.left + deltaX;
			y = graphRect.bottom - deltaY;
			pDC->LineTo(x, y);
		}

		if (pOldPen)
			pDC->SelectObject(pOldPen);
	}
	else
		pDC->FillRect(&clientRect, CBrush::FromHandle((HBRUSH) ::GetStockObject(WHITE_BRUSH)));
}

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView diagnostics

#ifdef _DEBUG
void CGoIO_consoleView::AssertValid() const
{
	CView::AssertValid();
}

void CGoIO_consoleView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGoIO_consoleDoc* CGoIO_consoleView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGoIO_consoleDoc)));
	return (CGoIO_consoleDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleView message handlers

void CGoIO_consoleView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}

BOOL CGoIO_consoleView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}
