// GoIO_consoleDoc.cpp : implementation of the CGoIO_consoleDoc class
//

#include "stdafx.h"
#include "GoIO_console.h"
#include "MainFrm.h"

#include "GoIO_consoleView.h"
#include "GoIO_consoleDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleDoc

IMPLEMENT_DYNCREATE(CGoIO_consoleDoc, CDocument)

BEGIN_MESSAGE_MAP(CGoIO_consoleDoc, CDocument)
	//{{AFX_MSG_MAP(CGoIO_consoleDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleDoc construction/destruction

CGoIO_consoleDoc::CGoIO_consoleDoc()
{
	m_pDevice = NULL;
	ClearMeasurementCirbuf();
	m_measPeriodInSeconds = 1000000.0;
}

CGoIO_consoleDoc::~CGoIO_consoleDoc()
{
	if (m_pDevice)
		CloseDevice();
}

BOOL CGoIO_consoleDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	ClearMeasurementCirbuf();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) (((CFrameWnd *) AfxGetMainWnd())->GetActiveView());
	if (pView)
		pView->SetGraphHistory(1.0, -1.0);//This clears the history.

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleDoc serialization

void CGoIO_consoleDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleDoc diagnostics

#ifdef _DEBUG
void CGoIO_consoleDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGoIO_consoleDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGoIO_consoleDoc commands

GOIO_SENSOR_HANDLE CGoIO_consoleDoc::OpenDevice(int vendorId, int productId, LPCSTR pDeviceName)
{
	CGoIO_consoleView *pView = (CGoIO_consoleView *) (((CFrameWnd *) AfxGetMainWnd())->GetActiveView());
	ASSERT(pView);
	char tmpstring[50];
	if (m_pDevice)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(m_pDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);

		if (0 != lstrcmp(openDeviceName, pDeviceName))
		{
			//Close the open device if it does not match the new one.
			CloseDevice();
		}
	}
	if (!m_pDevice)
		m_pDevice = GoIO_Sensor_Open(pDeviceName, vendorId, productId, 0);

	if (m_pDevice)
	{
		unsigned char charId;
		GoIO_Sensor_DDSMem_GetSensorNumber(m_pDevice, &charId, 0, 0);
		int id = charId;
		cppsstream ss;
		ss << "Sensor id = " << id;

		GoIO_Sensor_DDSMem_GetLongName(m_pDevice, tmpstring, sizeof(tmpstring));
		if (lstrlen(tmpstring) != 0)
		{
			cppstring sensorName = tmpstring;
			ss << " ( " << sensorName << " ) ";
		}
		SetTitle(ss.str().c_str());

		float ftemp[3];
		unsigned char calPageIndex;
		GoIO_Sensor_DDSMem_GetActiveCalPage(m_pDevice, &calPageIndex);
		GoIO_Sensor_DDSMem_GetCalPage(m_pDevice, calPageIndex,
			&ftemp[0], &ftemp[1], &ftemp[2], tmpstring, sizeof(tmpstring));
		CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
		if (pFrame)
			pFrame->m_wndSendBar.SetDlgItemText(IDC_UNITS, tmpstring);

		SetMeasurementPeriodInSeconds(GoIO_Sensor_GetMeasurementPeriod(m_pDevice, SKIP_TIMEOUT_MS_DEFAULT));
	}

	return m_pDevice;
}

void CGoIO_consoleDoc::CloseDevice()
{
	if (m_pDevice)
		GoIO_Sensor_Close(m_pDevice);
	m_pDevice = NULL;

	ClearMeasurementCirbuf();

	CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
	if (pFrame)
	{
		pFrame->m_wndSendBar.SetDlgItemText(IDC_MEAS, "");
		pFrame->m_wndSendBar.SetDlgItemText(IDC_UNITS, "");
		CGoIO_consoleView *pView = (CGoIO_consoleView *) (pFrame->GetActiveView());
		if (pView)
		{
			pView->SetGraphHistory(1.0, -1.0);//This clears the history.
			pView->Invalidate();
		}
	}

	SetTitle("");
}

