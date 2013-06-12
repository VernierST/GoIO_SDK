/*********************************************************************************

Copyright (c) 2010, Vernier Software & Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Vernier Software & Technology nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL VERNIER SOFTWARE & TECHNOLOGY BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************************/
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GoIO_Measure.h"
#include "GoIO_MeasureDoc.h"
#include "GoIO_MeasureView.h"
#include "SetMeasPeriodDlg.h"
#include "DisplayDepthDlg.h"
#include "GVernierUSB.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TIMER_PERIOD_MS 70

static BYTE ConvertBCDStringToByte(LPCSTR str);
static void ConvertByteToBCDString(BYTE b, CString &str);

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
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
	ON_COMMAND(IDM_CALIB0, OnCalib0)
	ON_COMMAND(IDM_CALIB1, OnCalib1)
	ON_COMMAND(IDM_CALIB2, OnCalib2)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SETDISPLAYDEPTH, &CMainFrame::OnUpdateActionSetdisplaydepth)
	ON_COMMAND(ID_ACTION_SETDISPLAYDEPTH, &CMainFrame::OnActionSetdisplaydepth)
	ON_UPDATE_COMMAND_UI(IDM_GET_SENSOR_ID, &CMainFrame::OnUpdateGetSensorId)
	ON_COMMAND(IDM_GET_SENSOR_ID, &CMainFrame::OnGetSensorId)
	ON_UPDATE_COMMAND_UI(IDM_GOIO_LIB_VERBOSE, &CMainFrame::OnUpdateGoioLibVerbose)
	ON_COMMAND(IDM_GOIO_LIB_VERBOSE, &CMainFrame::OnGoioLibVerbose)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_wndCollectBar.Create(this, IDD_COLLECT_BAR, CBRS_TOP, IDD_COLLECT_BAR);
	m_wndCollectBar.EnableDocking(CBRS_ALIGN_ANY);

	EnableDocking(CBRS_ALIGN_ANY);

	DockControlBar(&m_wndCollectBar);

/*
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
*/
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


// CMainFrame message handlers

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();

	if ((1 == nIndex) && pDoc)
	{
		int numSkips = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
		int numJonahs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
		int numCyclopses = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
		int numMiniGCs = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		char newDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int iTemp1, iTemp2;
		if (pDoc->GetOpenDevicePtr())
			GoIO_Sensor_GetOpenDeviceName(pDoc->GetOpenDevicePtr(), openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &iTemp1, &iTemp2);
		else
			openDeviceName[0] = 0;

		while (pPopupMenu->GetMenuItemCount() > 0)
			pPopupMenu->DeleteMenu(0, MF_BYPOSITION);

		int i, j;
		i = 0;
		for (j = 0; j < numSkips; j++)
		{
			if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID,
						SKIP_DEFAULT_PRODUCT_ID, j))
			{
				pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, newDeviceName);
				if (0 == lstrcmp(openDeviceName, newDeviceName))
					pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
				i++;
			}
		}
		for (j = 0; j < numJonahs; j++)
		{
			if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID,
						USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID, j))
			{
				pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, newDeviceName);
				if (0 == lstrcmp(openDeviceName, newDeviceName))
					pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
				i++;
			}
		}
		for (j = 0; j < numCyclopses; j++)
		{
			if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID,
						CYCLOPS_DEFAULT_PRODUCT_ID, j))
			{
				pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, newDeviceName);
				if (0 == lstrcmp(openDeviceName, newDeviceName))
					pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
				i++;
			}
		}
		for (j = 0; j < numMiniGCs; j++)
		{
			if (0 == GoIO_GetNthAvailableDeviceName(newDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, VERNIER_DEFAULT_VENDOR_ID,
						MINI_GC_DEFAULT_PRODUCT_ID, j))
			{
				pPopupMenu->AppendMenu(MF_STRING, IDM_DEVICE0 + i, newDeviceName);
				if (0 == lstrcmp(openDeviceName, newDeviceName))
					pPopupMenu->CheckMenuItem(IDM_DEVICE0 + i, MF_BYCOMMAND | MF_CHECKED);
				i++;
			}
		}
	}
	else
	if ((2 == nIndex) && pDoc)
	{
		while (pPopupMenu->GetMenuItemCount() > 0)
			pPopupMenu->DeleteMenu(0, MF_BYPOSITION);

		GOIO_SENSOR_HANDLE pSensor = pDoc->GetOpenDevicePtr();
		if (pSensor)
		{
			unsigned char nActiveIndex;
			GoIO_Sensor_DDSMem_GetActiveCalPage(pSensor, &nActiveIndex);
			unsigned char numCalibrations;
			GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(pSensor, &numCalibrations);
			numCalibrations++;
			char units[30];
			gtype_real32 coeffs[3];
			cppsstream ss;
			for (unsigned int k = 0; k < numCalibrations; k++)
			{
				GoIO_Sensor_DDSMem_GetCalPage(pSensor, k, &coeffs[0], &coeffs[1], &coeffs[2], units, sizeof(units));
				ss.str("");
				ss << "Calib " << k << " " << cppstring(units);
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
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pMenu && pDoc)
	{
		int count = pMenu->GetMenuItemCount();
		CMenu *pSubMenu = pMenu->GetSubMenu(1);
		if (pSubMenu)
		{
			unsigned int menuState = pSubMenu->GetMenuState(N, MF_BYPOSITION);
			if (menuState & MF_CHECKED)
				pDoc->CloseDevice();
			else
			{
				char tmpstring[GOIO_MAX_SIZE_DEVICE_NAME];
				int productId = SKIP_DEFAULT_PRODUCT_ID;//matches pid_0003
				pSubMenu->GetMenuString(N, tmpstring, sizeof(tmpstring), MF_BYPOSITION);
				if (strstr(tmpstring, "pid_0002"))
					productId = USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID;
                else
				if (strstr(tmpstring, "pid_0004"))
					productId = CYCLOPS_DEFAULT_PRODUCT_ID;
                else
				if (strstr(tmpstring, "pid_0007"))
					productId = MINI_GC_DEFAULT_PRODUCT_ID;
				pDoc->OpenDevice(VERNIER_DEFAULT_VENDOR_ID, productId, tmpstring);
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

void CMainFrame::OnTimer(UINT_PTR nIDEvent) 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	bool bNewMeasurementAvailable = false;
	short iRawMeasVal = 0;
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE hDevice = pDoc->GetOpenDevicePtr();
		if (hDevice)
		{
			int measurements[150];
			int numMeasurementsAvailable = GoIO_Sensor_GetNumMeasurementsAvailable(hDevice);
			if (numMeasurementsAvailable > 0)
			{
				//Note that the maxCount parameter to GoIO_Sensor_ReadRawMeasurements() is a multiple of 6.
				numMeasurementsAvailable = GoIO_Sensor_ReadRawMeasurements(hDevice, measurements, 150);
			}

			if (numMeasurementsAvailable > 0)
			{
				//Stuff the new measurements in a circular buffer.
				double rVolts;
				double rMeasurement;
				for (int k = 0; k < numMeasurementsAvailable; k++)
				{
					rVolts = GoIO_Sensor_ConvertToVoltage(hDevice, measurements[k]);
					rMeasurement = GoIO_Sensor_CalibrateData(hDevice, rVolts);
					pDoc->AddMeasurementToCirbuf(rMeasurement);
				}
				pView->Invalidate();
				pView->UpdateWindow();

				//Display latest measurement value on the toolbar.
				cppsstream ss;
				ss << fixed << uppercase << showpoint << setprecision(3) << rMeasurement;
				cppstring strMeas = ss.str();
				m_wndCollectBar.SetDlgItemText(IDC_MEAS, strMeas.c_str());
			}
			else
			if (!m_bIsCollectingMeasurements)
				//Clear latest measurement value.
				m_wndCollectBar.SetDlgItemText(IDC_MEAS, "");
		}
	}
}

void CMainFrame::OnUpdateGetStatus(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGetStatus() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			//Normally, you would do the following:
			GSkipGetStatusCmdResponsePayload statusRec;
			memset(&statusRec, 0, sizeof(statusRec));//Do this because Go! Temp does not set the slave CPU fields.
			int nBytesRead = sizeof(statusRec);
			int nResult = GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_GET_STATUS, NULL, 0, &statusRec, 
				&nBytesRead, SKIP_TIMEOUT_MS_DEFAULT);
			if (0 == nResult)
			{
				int statusSummary = statusRec.status;
 				double version = (statusRec.majorVersionMasterCPU/0x10)*10.0 + (statusRec.majorVersionMasterCPU & 0xf)*1.0 + 
 						(statusRec.minorVersionMasterCPU/0x10)*0.1 + (statusRec.minorVersionMasterCPU & 0xf)*0.01 +
 					  (statusRec.majorVersionSlaveCPU/0x10)*0.001 + (statusRec.majorVersionSlaveCPU & 0xf)*0.0001 + 
 						(statusRec.minorVersionSlaveCPU/0x10)*0.00001 + (statusRec.minorVersionSlaveCPU & 0xf)*0.000001; 
			
				cppsstream ss;
				ss << "Status byte = " << statusSummary << " ; version = ";
				ss << fixed << uppercase << showpoint << setprecision(6) << version;
				cppstring str = ss.str();
				MessageBox(str.c_str(), "SKIP_CMD_ID_GET_STATUS");
			}
		}
	}
}

void CMainFrame::OnUpdateGetSensorId(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
			int vendorId, productId;
			GoIO_Sensor_GetOpenDeviceName(pDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &vendorId, &productId);
			bEnable = (productId == SKIP_DEFAULT_PRODUCT_ID);
		}
	}
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGetSensorId() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			unsigned char oldSensorNumber, newSensorNumber;

			GoIO_Sensor_DDSMem_GetSensorNumber(pDevice, &oldSensorNumber, 0, 0);
			//Actually query the hardware.
			if (0 != GoIO_Sensor_DDSMem_GetSensorNumber(pDevice, &newSensorNumber, 1, SKIP_TIMEOUT_MS_DEFAULT))
				newSensorNumber = 0;
			if (oldSensorNumber != newSensorNumber)
			{
				//The user must have switched sensors. The easiest way to handle this is to close and then reopen the device.
				char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
				int vendorId, productId;
				GoIO_Sensor_GetOpenDeviceName(pDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &vendorId, &productId);
				pDoc->CloseDevice();
				pDoc->OpenDevice(vendorId, productId, openDeviceName);
				m_bIsCollectingMeasurements = false;
			}
		}
	}
}

void CMainFrame::OnUpdateSetMeasPeriod(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL) && 
			(!m_bIsCollectingMeasurements);//GoIO_Sensor_SetMeasurementPeriod() fails while collecting.
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnSetMeasPeriod() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			CSetMeasPeriodDlg dlg;
			dlg.SetUnits("ms");
			dlg.m_period = 
				(unsigned int) floor(1000.0*GoIO_Sensor_GetMeasurementPeriod(pDevice, SKIP_TIMEOUT_MS_DEFAULT) + 0.5);
			if (IDOK == dlg.DoModal())
			{
				GoIO_Sensor_SetMeasurementPeriod(pDevice, dlg.m_period/1000.0, SKIP_TIMEOUT_MS_DEFAULT);
				pDoc->SetMeasurementPeriodInSeconds(GoIO_Sensor_GetMeasurementPeriod(pDevice, SKIP_TIMEOUT_MS_DEFAULT));
			}
		}
	}
}

void CMainFrame::OnUpdateStartMeas(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL) && (!m_bIsCollectingMeasurements);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnStartMeas() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GoIO_Sensor_ClearIO(pDevice);//Flush any old measurements laying around.
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);

			m_bIsCollectingMeasurements = true;

			pDoc->ClearMeasurementCirbuf();
			pView->SetGraphHistory(1.0, -1.0);//This clears the history.
			pView->Invalidate();
		}
	}
}

void CMainFrame::OnUpdateStopMeas(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL) && m_bIsCollectingMeasurements;
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnStopMeas() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0, NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);

			m_bIsCollectingMeasurements = false;
		}
	}
}

void CMainFrame::OnUpdateOrange(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnOrange() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GSkipSetLedStateParams params;
			params.color = kLEDOrange;
			params.brightness = kSkipOrangeLedBrightness;
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_SET_LED_STATE, &params, sizeof(params), NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);
		}
	}
}

void CMainFrame::OnUpdateLedOff(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnLedOff() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GSkipSetLedStateParams params;
			params.color = SKIP_LED_COLOR_BLACK;
			params.brightness = 0;
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_SET_LED_STATE, &params, sizeof(params), NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);
		}
	}
}

void CMainFrame::OnUpdateGreen(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnGreen() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GSkipSetLedStateParams params;
			params.color = SKIP_LED_COLOR_GREEN;
			params.brightness = SKIP_LED_BRIGHTNESS_MAX;
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_SET_LED_STATE, &params, sizeof(params), NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);
		}
	}
}

void CMainFrame::OnUpdateRed(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
		bEnable = (pDoc->GetOpenDevicePtr() != NULL);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnRed() 
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			GSkipSetLedStateParams params;
			params.color = SKIP_LED_COLOR_RED;
			params.brightness = SKIP_LED_BRIGHTNESS_MAX;
			GoIO_Sensor_SendCmdAndGetResponse(pDevice, SKIP_CMD_ID_SET_LED_STATE, &params, sizeof(params), NULL, NULL, 
				SKIP_TIMEOUT_MS_DEFAULT);
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
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	if (pDoc)
	{
		GOIO_SENSOR_HANDLE pDevice = pDoc->GetOpenDevicePtr();
		if (pDevice)
		{
			unsigned char numCalibrations;
			GoIO_Sensor_DDSMem_GetHighestValidCalPageIndex(pDevice, &numCalibrations);
			numCalibrations++;
			unsigned char oldN;
			GoIO_Sensor_DDSMem_GetActiveCalPage(pDevice, &oldN);
			if ((N < numCalibrations) && (oldN != N))
			{
				char tmpstring[50];
				float ftemp[3];
				GoIO_Sensor_DDSMem_SetActiveCalPage(pDevice, N);
				GoIO_Sensor_DDSMem_GetCalPage(pDevice, N, &ftemp[0], &ftemp[1], &ftemp[2], tmpstring, sizeof(tmpstring));
				CWnd *pWnd = m_wndCollectBar.GetDlgItem(IDC_UNITS);
				if (pWnd)
					pWnd->SetWindowText(tmpstring);
				pDoc->ClearMeasurementCirbuf();
				CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
				if (pView)
				{
					pView->SetGraphHistory(1.0, -1.0);//This clears the history.
					pView->Invalidate();
				}
			}
		}
	}
}



void CMainFrame::OnUpdateActionSetdisplaydepth(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::OnActionSetdisplaydepth()
{
	CGoIO_MeasureDoc *pDoc = (CGoIO_MeasureDoc *) GetActiveDocument();
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) GetActiveView();
	if (pDoc && pView)
	{
		CDisplayDepthDlg dlg;
		dlg.m_displayDepth = pDoc->GetMaxNumMeasurementsInCirbuf();
		if (IDOK == dlg.DoModal())
		{
			if (dlg.m_displayDepth != pDoc->GetMaxNumMeasurementsInCirbuf())
			{
				pDoc->SetMaxNumMeasurementsInCirBuf(dlg.m_displayDepth);
				pView->SetGraphHistory(1.0, -1.0);//This clears the history.
				pView->Invalidate();
			}
		}
	}
}

void CMainFrame::OnUpdateGoioLibVerbose(CCmdUI *pCmdUI)
{
	BOOL bEnable = TRUE;
	pCmdUI->Enable(bEnable);
	gtype_int32 thresh = GOIO_TRACE_SEVERITY_HIGH;
	GoIO_Diags_GetDebugTraceThreshold(&thresh);
	pCmdUI->SetCheck((thresh <= GOIO_TRACE_SEVERITY_LOW) ? 1 : 0);
}

void CMainFrame::OnGoioLibVerbose()
{
	gtype_int32 thresh = GOIO_TRACE_SEVERITY_HIGH;
	GoIO_Diags_GetDebugTraceThreshold(&thresh);
	if (thresh <= GOIO_TRACE_SEVERITY_LOW)
		thresh = GOIO_TRACE_SEVERITY_HIGH;
	else
		thresh = GOIO_TRACE_SEVERITY_LOW;
	GoIO_Diags_SetDebugTraceThreshold(thresh);
}
