// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GoIO_console.h"
#include "GSkipDevice.h"
#include "GUSBDirectTempDevice.h"
#include "GCyclopsDevice.h"
#include "GMiniGCDevice.h"
#include "GoIO_consoleDoc.h"
#include "GoIO_consoleView.h"
#include "SetMeasPeriodDlg.h"
#include "GUtils.h"

#include "MainFrm.h"

#define TIMER_PERIOD_MS 250

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static BYTE ConvertBCDStringToByte(LPCSTR str);
static void ConvertByteToBCDString(BYTE b, CString &str);

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(IDM_DEVICE0, OnDevice0)
	ON_COMMAND(IDM_DEVICE1, OnDevice1)
	ON_COMMAND(IDM_DEVICE2, OnDevice2)
	ON_COMMAND(IDM_DEVICE3, OnDevice3)
	ON_COMMAND(IDM_DEVICE4, OnDevice4)
	ON_COMMAND(IDM_DEVICE5, OnDevice5)
	ON_COMMAND(IDM_DEVICE6, OnDevice6)
	ON_COMMAND(IDM_DEVICE7, OnDevice7)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(IDM_GET_STATUS, OnUpdateGetStatus)
	ON_COMMAND(IDM_GET_STATUS, OnGetStatus)
	ON_COMMAND(IDM_GET_SENSOR_ID, OnGetSensorId)
	ON_UPDATE_COMMAND_UI(IDM_GET_SENSOR_ID, OnUpdateGetSensorId)
	ON_UPDATE_COMMAND_UI(IDM_SET_MEAS_PERIOD, OnUpdateSetMeasPeriod)
	ON_COMMAND(IDM_SET_MEAS_PERIOD, OnSetMeasPeriod)
	ON_UPDATE_COMMAND_UI(IDM_START_MEAS, OnUpdateStartMeas)
	ON_COMMAND(IDM_START_MEAS, OnStartMeas)
	ON_UPDATE_COMMAND_UI(IDM_STOP_MEAS, OnUpdateStopMeas)
	ON_COMMAND(IDM_STOP_MEAS, OnStopMeas)
	ON_UPDATE_COMMAND_UI(IDM_ORANGE, OnUpdateOrange)
	ON_COMMAND(IDM_ORANGE, OnOrange)
	ON_UPDATE_COMMAND_UI(IDM_LED_OFF, OnUpdateLedOff)
	ON_COMMAND(IDM_LED_OFF, OnLedOff)
	ON_UPDATE_COMMAND_UI(IDM_GREEN, OnUpdateGreen)
	ON_COMMAND(IDM_GREEN, OnGreen)
	ON_UPDATE_COMMAND_UI(IDM_RED, OnUpdateRed)
	ON_COMMAND(IDM_RED, OnRed)
	ON_UPDATE_COMMAND_UI(IDM_READ_SENSOR_MEM, OnUpdateReadSensorMem)
	ON_COMMAND(IDM_READ_SENSOR_MEM, OnReadSensorMem)
	ON_COMMAND(IDM_CALIB0, OnCalib0)
	ON_COMMAND(IDM_CALIB1, OnCalib1)
	ON_COMMAND(IDM_CALIB2, OnCalib2)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(IDC_SEND_PACKET, OnUpdateSendButton)
	ON_BN_CLICKED(IDC_SEND_PACKET, OnSendPacketToSkip)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_timerId = 0;
	m_bIsCollectingMeasurements = false;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndSendBar.Create(this, IDD_PACKET_BAR, CBRS_TOP, IDD_PACKET_BAR);
	m_wndSendBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndSendBar);
	CString str;
	ConvertByteToBCDString(0, str);
	for (int i = IDC_PACKET_BYTE; i <= IDC_PACKET_BYTE8; i++)
	{
		CEdit *pEdit = (CEdit *) m_wndSendBar.GetDlgItem(i);
		if (pEdit)
		{
			pEdit->SetLimitText(2);
			pEdit->SetWindowText(str);
		}
	}

	m_timerId = SetTimer(777, TIMER_PERIOD_MS, NULL);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();

	if ((2 == nIndex) && pDoc)
	{
		StringVector skipVec = GSkipDevice::GetAvailableDevices();
		StringVector jonahVec = GUSBDirectTempDevice::GetAvailableDevices();
		StringVector cyclopsVec = GCyclopsDevice::GetAvailableDevices();
		StringVector miniGCsVec = GMiniGCDevice::GetAvailableDevices();
		cppstring openDeviceName;
		if (pDoc->GetOpenDevicePtr())
		{
			if (pDoc->GetOpenDevicePtr()->GetPortRefPtr())
				openDeviceName = pDoc->GetOpenDevicePtr()->GetPortRefPtr()->GetDisplayName();
		}

		while (pPopupMenu->GetMenuItemCount() > 0)
			pPopupMenu->DeleteMenu(0, MF_BYPOSITION);

		int i, j;
		i = 0;
		for (j = 0; j < skipVec.size(); j++)
		{
			pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, skipVec[j].c_str());
			if (openDeviceName == skipVec[j])
				pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
			i++;
		}
		for (j = 0; j < jonahVec.size(); j++)
		{
			pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, jonahVec[j].c_str());
			if (openDeviceName == jonahVec[j])
				pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
			i++;
		}
		for (j = 0; j < cyclopsVec.size(); j++)
		{
			pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, cyclopsVec[j].c_str());
			if (openDeviceName == cyclopsVec[j])
				pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
			i++;
		}
		for (j = 0; j < miniGCsVec.size(); j++)
		{
			pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, miniGCsVec[j].c_str());
			if (openDeviceName == miniGCsVec[j])
				pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
			i++;
		}
	}
	else
	if ((3 == nIndex) && pDoc)
	{
		while (pPopupMenu->GetMenuItemCount() > 0)
			pPopupMenu->DeleteMenu(0, MF_BYPOSITION);

		GMBLSensor *pSensor = pDoc->GetOpenSensorPtr();
		GSensorDDSRec *pSensorDDSRec = NULL;

		if (pSensor)
			pSensorDDSRec = pSensor->GetDDSRecPtr();

		if (pSensorDDSRec)
		{
			int nActiveIndex = pSensorDDSRec->ActiveCalPage;
			cppsstream ss;
			for (unsigned int k = 0; k <= pSensorDDSRec->HighestValidCalPageIndex; k++)
			{
				ss.str("");
				ss << "Calib " << k << " " << cppstring(pSensorDDSRec->CalibrationPage[k].Units, sizeof(pSensorDDSRec->CalibrationPage[k].Units));
				pPopupMenu->AppendMenu(MF_STRING, IDM_CALIB0 + k, ss.str().c_str());
				if (k == nActiveIndex)
					pPopupMenu->CheckMenuItem(IDM_CALIB0 + k, MF_BYCOMMAND | MF_CHECKED);
			}
		}
	}
}

void CMainFrame::OnDeviceN(unsigned int N) 
{
	CMenu *pMenu = GetMenu();
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pMenu && pDoc)
	{
		int count = pMenu->GetMenuItemCount();
		CMenu *pSubMenu = pMenu->GetSubMenu(2);
		if (pSubMenu)
		{
			unsigned int menuState = pSubMenu->GetMenuState(N, MF_BYPOSITION);
			if (menuState & MF_CHECKED)
				pDoc->CloseDevice();
			else
			{
				char tmpstring[_MAX_PATH];
				GPortRef *pPortRef = NULL;
				pSubMenu->GetMenuString(N, tmpstring, sizeof(tmpstring), MF_BYPOSITION);
				if (strstr(tmpstring, "pid_0002"))
					pPortRef = new GPortRef(kPortType_USB, tmpstring, tmpstring, VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
				else
				if (strstr(tmpstring, "pid_0003"))
					pPortRef = new GPortRef(kPortType_USB, tmpstring, tmpstring, VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
				else
				if (strstr(tmpstring, "pid_0004"))
					pPortRef = new GPortRef(kPortType_USB, tmpstring, tmpstring, VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
				else
				if (strstr(tmpstring, "pid_0007"))
					pPortRef = new GPortRef(kPortType_USB, tmpstring, tmpstring, VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);
				pDoc->OpenDevice(pPortRef);
				delete pPortRef;
			}
			m_bIsCollectingMeasurements = false;
		}
	}
}

void CMainFrame::OnDevice0() 
{
	OnDeviceN(0);
}

void CMainFrame::OnDevice1() 
{
	OnDeviceN(1);
}

void CMainFrame::OnDevice2() 
{
	OnDeviceN(2);
}

void CMainFrame::OnDevice3() 
{
	OnDeviceN(3);
}

void CMainFrame::OnDevice4() 
{
	OnDeviceN(4);
}

void CMainFrame::OnDevice5() 
{
	OnDeviceN(5);
}

void CMainFrame::OnDevice6() 
{
	OnDeviceN(6);
}

void CMainFrame::OnDevice7() 
{
	OnDeviceN(7);
}

BOOL CMainFrame::DestroyWindow() 
{
	if (m_timerId != 0)
		KillTimer(m_timerId);
	m_timerId = 0;
	
	return CFrameWnd::DestroyWindow();
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	bool bNewMeasurementAvailable = false;
	int iRawMeasVal = 0;
	if (pDoc && pView)
	{
		GDeviceIO *pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GSkipBaseDevice *pSkipBaseDevice = dynamic_cast<GSkipBaseDevice *>(pDevice);
			if (pSkipBaseDevice)
			{
				//Read in available measurement packets and display them.
				long nNumPacketsJustRead;
				GSkipMeasurementPacket measPacket;
				long count = pSkipBaseDevice->OSMeasurementPacketsAvailable();
				while (count > 0)
				{
					nNumPacketsJustRead = 1;
					pSkipBaseDevice->OSReadMeasurementPackets(&measPacket, &nNumPacketsJustRead, 1);
					pView->RecordGoIOInput((LPCSTR) &measPacket, sizeof(measPacket));
					count--;

					GCyclopsDevice *pCyclopsDevice = dynamic_cast<GCyclopsDevice *>(pDevice);
					if (pCyclopsDevice)
					{
						GCyclopsMeasurementPacket *pCyclopsPacket = (GCyclopsMeasurementPacket *) &measPacket;
						GUtils::OSConvertBytesToInt(pCyclopsPacket->measLsByteLsWord, pCyclopsPacket->measMsByteLsWord, 
							pCyclopsPacket->measLsByteMsWord, pCyclopsPacket->measMsByteMsWord, &iRawMeasVal);
					}
					else
					{
						int nMeasIndex = measPacket.nMeasurementsInPacket - 1;
						unsigned char *pMeasInPacket = &measPacket.meas0LsByte;
						pMeasInPacket += nMeasIndex*2;
						short iShort;
						GUtils::OSConvertBytesToShort(pMeasInPacket[0], pMeasInPacket[1], &iShort);
						iRawMeasVal = iShort;
					}
					bNewMeasurementAvailable = true;
				}

				//Read in available command response packets and display them.
				GSkipGenericResponsePacket respPacket;
				count = pSkipBaseDevice->OSCmdRespPacketsAvailable();
				while (count > 0)
				{
					nNumPacketsJustRead = 1;
					pSkipBaseDevice->OSReadCmdRespPackets(&respPacket, &nNumPacketsJustRead, 1);
					pView->RecordGoIOInput((LPCSTR) &respPacket, sizeof(respPacket));
					count--;
				}


				GMBLSensor *pSensor = pDoc->GetOpenSensorPtr();
				if (bNewMeasurementAvailable && pSensor)
				{
					//Display latest measurement value.
					real rVolts = pSkipBaseDevice->ConvertToVoltage(iRawMeasVal, pSensor->GetProbeType());
					real rMeasurement = pSensor->CalibrateData(rVolts);
					cppsstream ss;
					ss << fixed << uppercase << showpoint << setprecision(3) << rMeasurement;
					cppstring strMeas = ss.str();
					m_wndSendBar.SetDlgItemText(IDC_MEAS, strMeas.c_str());
				}
				else
				if (!m_bIsCollectingMeasurements)
					//Clear latest measurement value.
					m_wndSendBar.SetDlgItemText(IDC_MEAS, "");
			}
		}
	}
}

void CMainFrame::OnUpdateGetStatus(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGetStatus() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			//Normally, you would do the following:
//			GSkipGetStatusCmdResponsePayload statusRec;
//			long nBytesRead = sizeof(statusRec);
//			nResult = pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_GET_STATUS, NULL, 0, &statusRec, &nBytesRead);
			//But since we want to display what packets are sent and received, we do this instead:
			pDevice->SendCmd(SKIP_CMD_ID_GET_STATUS, NULL, 0);
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_GET_STATUS;
			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			//The response packet will be read in and displayed in OnTimer().
		}
	}
}

void CMainFrame::OnUpdateGetSensorId(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
			bEnable = (pDevice->GetPortRefPtr()->GetUSBProductID() == SKIP_DEFAULT_PRODUCT_ID);
	}
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGetSensorId() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			GSkipGenericResponsePacket respPacket;
			memset(&respPacket, 0, sizeof(respPacket));
			GSkipGetSensorIdCmdResponsePayload *pResponsePayload = (GSkipGetSensorIdCmdResponsePayload *) &respPacket.responsePayload[0];
			long nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
			long nResult = pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, pResponsePayload, &nBytesRead);

			//Log what was just sent.
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_GET_SENSOR_ID;
			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().

			if (kResponse_OK == nResult)
			{
				//Log what just came back.
				respPacket.header = SKIP_GET_SENSOR_ID_CMD_RESP_HDR;
				respPacket.cmd = SKIP_CMD_ID_GET_SENSOR_ID;
				pView->RecordGoIOInput((LPCSTR) &respPacket, sizeof(respPacket));

				int nSensorId;
				GUtils::OSConvertBytesToInt(pResponsePayload->lsbyteLswordSensorId, pResponsePayload->msbyteLswordSensorId,
					pResponsePayload->lsbyteMswordSensorId, pResponsePayload->msbyteMswordSensorId, &nSensorId);

				//Open a new sensor based on the id that we just read from the device.
				pDoc->OpenSensor(nSensorId);
			}
		}
	}
}


void CMainFrame::OnUpdateSetMeasPeriod(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnSetMeasPeriod() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			CSetMeasPeriodDlg dlg;
			dlg.m_period_ms = (unsigned int) floor(1000.0*pDevice->GetMeasurementPeriod() + 0.5);
			if (IDOK == dlg.DoModal())
			{
				//Normally, you would do the following:
//				pDevice->SetMeasurementPeriod(dlg.m_period_ms/1000.0);
				//But since we want to display what packets are sent and received, we do this instead:
				GSkipOutputPacket cmdPacket;
				memset(&cmdPacket, 0, sizeof(cmdPacket));
				cmdPacket.cmd = SKIP_CMD_ID_SET_MEASUREMENT_PERIOD;
				GSkipSetMeasurementPeriodParams *pParams = (GSkipSetMeasurementPeriodParams *) &cmdPacket.params[0];
				int	nNumTicks = floor((dlg.m_period_ms/1000.0)/pDevice->GetMeasurementTickInSeconds() + 0.5);
				GUtils::OSConvertIntToBytes(nNumTicks, &(pParams->lsbyteLswordMeasurementPeriod), &(pParams->msbyteLswordMeasurementPeriod),
					&(pParams->lsbyteMswordMeasurementPeriod), &(pParams->msbyteMswordMeasurementPeriod));
				pDevice->SendCmd(SKIP_CMD_ID_SET_MEASUREMENT_PERIOD, pParams, sizeof(GSkipSetMeasurementPeriodParams));

				pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
				//The response packet will be read in and displayed in OnTimer().
			}
		}
	}
}

void CMainFrame::OnUpdateStartMeas(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnStartMeas() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			pDevice->OSClearMeasurementPacketQueue();//Flush any old measurements laying around.

			//Normally, you would do the following:
//			nResult = pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL);
			//But since we want to display what packets are sent and received, we do this instead:
			pDevice->SendCmd(SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0);
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_START_MEASUREMENTS;
			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			//The response packet will be read in and displayed in OnTimer().

			m_bIsCollectingMeasurements = true;
		}
	}
}

void CMainFrame::OnUpdateStopMeas(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnStopMeas() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			//Normally, you would do the following:
//			nResult = pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0, NULL, NULL);
			//But since we want to display what packets are sent and received, we do this instead:
			pDevice->SendCmd(SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0);
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_STOP_MEASUREMENTS;
			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			//The response packet will be read in and displayed in OnTimer().

			m_bIsCollectingMeasurements = false;
		}
	}
}

void CMainFrame::OnUpdateOrange(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnOrange() 
{
	SetLEDColor(kLEDSettings_Orange);
}

void CMainFrame::OnUpdateLedOff(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnLedOff() 
{
	SetLEDColor(kLEDSettings_Off);
}

void CMainFrame::OnUpdateGreen(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGreen() 
{
	SetLEDColor(kLEDSettings_Green);
}

void CMainFrame::OnUpdateRed(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnRed() 
{
	SetLEDColor(kLEDSettings_Red);
}

void CMainFrame::SetLEDColor(ELEDSettings eLED)
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_SET_LED_STATE;
			GSkipSetLedStateParams *pParams = (GSkipSetLedStateParams *) &cmdPacket.params[0];
			pParams->color = 0x0;
			pParams->brightness = kSkipMaxLedBrightness;
			switch (eLED)
			{
				case kLEDSettings_Off:
					pParams->color = kLEDOff;
					break;

				case kLEDSettings_Red:
					pParams->color = kLEDRed;
					break;

				case kLEDSettings_Green:
					pParams->color = kLEDGreen;
					break;

				case kLEDSettings_Orange:
					pParams->color = kLEDOrange;
					pParams->brightness = kSkipOrangeLedBrightness;
					break;
			}

			//Normally, you would do the following:
//			pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_SET_LED_STATE, pParams, sizeof(GSkipSetLedStateParams), NULL, NULL);
			//But since we want to display what packets are sent and received, we do this instead:
			pDevice->SendCmd(SKIP_CMD_ID_SET_LED_STATE, pParams, sizeof(GSkipSetLedStateParams));

			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			//The response packet will be read in and displayed in OnTimer().
		}
	}
}

void CMainFrame::OnUpdateReadSensorMem(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnReadSensorMem() 
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
            if (pDevice->GetPortRefPtr()->GetUSBProductID() == CYCLOPS_DEFAULT_PRODUCT_ID)
            {
                //Do nothing - Cyclops does not have DDS memory.
            }
            else
            {
			    //Normally, you would use ReadNonVolatileMemory() to do this.
			    //But since we want to display what packets are sent and received, we do this instead:
			    GSkipOutputPacket cmdPacket;
			    memset(&cmdPacket, 0, sizeof(cmdPacket));
			    if (pDevice->GetPortRefPtr()->GetUSBProductID() == SKIP_DEFAULT_PRODUCT_ID)
				    cmdPacket.cmd = SKIP_CMD_ID_READ_REMOTE_NV_MEM;
			    else if (pDevice->GetPortRefPtr()->GetUSBProductID() == MINI_GC_DEFAULT_PRODUCT_ID)
				    cmdPacket.cmd = SKIP_CMD_ID_READ_REMOTE_NV_MEM;
			    else
				    cmdPacket.cmd = SKIP_CMD_ID_READ_LOCAL_NV_MEM;//USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID
			    GSkipReadI2CMemParams *pParams = (GSkipReadI2CMemParams *) &cmdPacket.params[0];
			    pParams->addr = 0;
			    pParams->count = 128;
			    pDevice->SendCmd(cmdPacket.cmd, pParams, sizeof(GSkipReadI2CMemParams));

			    pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			    //The response packet will be read in and displayed in OnTimer().
            }
		}
	}
}

void CMainFrame::OnUpdateSendButton(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnSendPacketToSkip()
{
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	CGoIO_consoleView *pView = (CGoIO_consoleView *) GetActiveView();
	if (pDoc && pView)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) (pDoc->GetOpenDevicePtr());
		if (pDevice)
		{
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			LPBYTE pPacketDatum = (LPBYTE) &cmdPacket;
			for (int i = IDC_PACKET_BYTE; i <= IDC_PACKET_BYTE8; i++)
			{
				CString str;
				CEdit *pEdit = (CEdit *) m_wndSendBar.GetDlgItem(i);
				if (pEdit)
				{
					pEdit->GetWindowText(str);
					*pPacketDatum = ConvertBCDStringToByte(str);
					ConvertByteToBCDString(*pPacketDatum, str);
					pEdit->SetWindowText(str);
					pPacketDatum++;
				}
			}
			pDevice->SendCmd(cmdPacket.cmd, &cmdPacket.params[0], sizeof(cmdPacket.params));

			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().
			//The response packet will be read in and displayed in OnTimer().
		}
	}
}

static BYTE ConvertBCDStringToByte(LPCSTR pStr)
{
	WORD val = 0;
	BYTE testByte = (*pStr);
	while (testByte != 0)
	{
		if ((testByte >= 'a') && (testByte <= 'f'))
			testByte = 10 + testByte - 'a';
		else
		if ((testByte >= 'A') && (testByte <= 'F'))
			testByte = 10 + testByte - 'A';
		else
		if ((testByte >= '0') && (testByte <= '9'))
			testByte = testByte - '0';
		else
			testByte = 0;

		val = val*16 + testByte;
		
		pStr++;
		testByte = (*pStr);
	}

	return ((BYTE) val);
}

static void ConvertByteToBCDString(BYTE b, CString &str)
{
	str = "";
	BYTE firstDigit = b/16;
	BYTE secondDigit = b % 16;
	BYTE firstChar, secondChar;
	firstChar = (firstDigit < 10) ? ('0' + firstDigit) : ('A' + firstDigit - 10);
	str += firstChar;
	secondChar = (secondDigit < 10) ? ('0' + secondDigit) : ('A' + secondDigit - 10);
	str += secondChar;
}


void CMainFrame::OnCalib0() 
{
	OnCalibN(0);
}

void CMainFrame::OnCalib1() 
{
	OnCalibN(1);
}

void CMainFrame::OnCalib2() 
{
	OnCalibN(2);
}

void CMainFrame::OnCalibN(unsigned int N)
{
	GSensorDDSRec *pDDSRec = NULL;
	GMBLSensor *pSensor = NULL;
	CGoIO_consoleDoc *pDoc = (CGoIO_consoleDoc *) GetActiveDocument();
	if (pDoc)
		pSensor = pDoc->GetOpenSensorPtr();
	if (pSensor)
		pDDSRec = pSensor->GetDDSRecPtr();
	if (pDDSRec)
	{
		if (N <= pDDSRec->HighestValidCalPageIndex)
		{
			pDDSRec->ActiveCalPage = N;
			cppstring sUnits = pSensor->GetUnits();
			CWnd *pWnd = m_wndSendBar.GetDlgItem(IDC_UNITS);
			if (pWnd)
				pWnd->SetWindowText(sUnits.c_str());
		}
	}
}
