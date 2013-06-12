// GoIO_consoleDoc.cpp : implementation of the CGoIO_consoleDoc class
//

#include "stdafx.h"
#include "GoIO_console.h"
#include "GUSBDirectTempDevice.h"
#include "GSkipDevice.h"
#include "GCyclopsDevice.h"
#include "MainFrm.h"
#include "GUtils.h"

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
	m_pSensor = NULL;
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


GDeviceIO *CGoIO_consoleDoc::OpenDevice(GPortRef *pPortRef)
{
	CGoIO_consoleView *pView = (CGoIO_consoleView *) (((CFrameWnd *) AfxGetMainWnd())->GetActiveView());
	long nResult;
	int nSensorId = 0;
	ASSERT(pView);
	if (m_pDevice)
	{
		if (m_pDevice->GetPortRefPtr()->GetDisplayName() != pPortRef->GetDisplayName())
		{
			delete m_pDevice;
			m_pDevice = NULL;
		}
	}
	if (!m_pDevice)
	{
		if (VERNIER_DEFAULT_VENDOR_ID == pPortRef->GetUSBVendorID())
		{
			GSkipGenericResponsePacket response;
			memset(&response, 0, sizeof(response));
			response.header = SKIP_INIT_CMD_RESP_HDR;
			response.cmd = SKIP_CMD_ID_INIT;
			long responseCount = sizeof(response.responsePayload);
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_INIT;
			if (USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			{
				GUSBDirectTempDevice *pUSBDirectTempDevice = new GUSBDirectTempDevice(pPortRef);
				if (kResponse_OK != pUSBDirectTempDevice->Open(pPortRef))
					delete pUSBDirectTempDevice;
				else
				{
					m_pDevice = pUSBDirectTempDevice;
					pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Log what we are sending.
					if (kResponse_OK == pUSBDirectTempDevice->SendCmdAndGetResponse(SKIP_CMD_ID_INIT, NULL, 0,
						&response.responsePayload[0], &responseCount))
					{
						pView->RecordGoIOInput((LPCSTR) &response, sizeof(response));//Log what came back.
						pUSBDirectTempDevice->OSClearMeasurementPacketQueue();

						nSensorId = kSensorIdNumber_GoTemp;
					}
				}
			}
            else
			if (CYCLOPS_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID())
			{
				GCyclopsDevice *pCyclopsDevice = new GCyclopsDevice(pPortRef);
				if (kResponse_OK != pCyclopsDevice->Open(pPortRef))
					delete pCyclopsDevice;
				else
				{
					m_pDevice = pCyclopsDevice;
					pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Log what we are sending.
					if (kResponse_OK == pCyclopsDevice->SendCmdAndGetResponse(SKIP_CMD_ID_INIT, NULL, 0,
						&response.responsePayload[0], &responseCount))
					{
						pView->RecordGoIOInput((LPCSTR) &response, sizeof(response));//Log what came back.
						pCyclopsDevice->OSClearMeasurementPacketQueue();

						nSensorId = kSensorIdNumber_GoMotion;
					}
				}
			}
			else
			if ((SKIP_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID()) || (MINI_GC_DEFAULT_PRODUCT_ID == pPortRef->GetUSBProductID()))
			{
				GSkipDevice *pSkipDevice = new GSkipDevice(pPortRef);
				if (kResponse_OK != pSkipDevice->Open(pPortRef))
					delete pSkipDevice;
				else
				{
					m_pDevice = pSkipDevice;
					pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Log what we are sending.
					if (kResponse_OK == pSkipDevice->SendCmdAndGetResponse(SKIP_CMD_ID_INIT, NULL, 0,
						&response.responsePayload[0], &responseCount))
					{
						pView->RecordGoIOInput((LPCSTR) &response, sizeof(response));//Log what came back.
						pSkipDevice->OSClearMeasurementPacketQueue();

						GSkipFlashMemoryRecord flashRec;
						nResult = pSkipDevice->ReadSkipFlashRecord(&flashRec, 2000);
						if (kResponse_OK == nResult)
							pSkipDevice->SetSkipFlashRecord(flashRec);//Do this so that measurements are properly calibrated in ConvertToVoltage().

						//Find out what sensor is connected.
						GSkipBaseDevice *pDevice = (GSkipBaseDevice *) m_pDevice;
						memset(&response, 0, sizeof(response));
						GSkipGetSensorIdCmdResponsePayload *pResponsePayload = (GSkipGetSensorIdCmdResponsePayload *) &response.responsePayload[0];
						long nBytesRead = sizeof(GSkipGetSensorIdCmdResponsePayload);
						nResult = pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_GET_SENSOR_ID, NULL, 0, pResponsePayload, &nBytesRead);

						//Log what was just sent.
						memset(&cmdPacket, 0, sizeof(cmdPacket));
						cmdPacket.cmd = SKIP_CMD_ID_GET_SENSOR_ID;
						pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Display the packet sent by SendCmd().

						if (kResponse_OK == nResult)
						{
							//Log what just came back.
							response.header = SKIP_GET_SENSOR_ID_CMD_RESP_HDR;
							response.cmd = SKIP_CMD_ID_GET_SENSOR_ID;
							pView->RecordGoIOInput((LPCSTR) &response, sizeof(response));

							//Parse out sensor id.
							GUtils::OSConvertBytesToInt(pResponsePayload->lsbyteLswordSensorId, pResponsePayload->msbyteLswordSensorId,
								pResponsePayload->lsbyteMswordSensorId, pResponsePayload->msbyteMswordSensorId, &nSensorId);
						}
					}
				}
			}
		}
	}

	if (m_pDevice)
	{
		//Open a new sensor based on the id that we just read from the device.
		OpenSensor(nSensorId);
	}

	return m_pDevice;
}

void CGoIO_consoleDoc::CloseDevice()
{
	CloseSensor();

	if (m_pDevice)
		delete m_pDevice;
	m_pDevice = NULL;
}

GMBLSensor *CGoIO_consoleDoc::OpenSensor(int id)
{
	CGoIO_consoleView *pView = (CGoIO_consoleView *) (((CFrameWnd *) AfxGetMainWnd())->GetActiveView());
	ASSERT(pView);
	if (m_pDevice)
	{
		GSkipBaseDevice *pDevice = (GSkipBaseDevice *) m_pDevice;

		if (m_pSensor)
			CloseSensor();

		m_pSensor = new GMBLSensor;
		m_pSensor->SetID(id);
		if (m_pSensor->IsSmartSensor())
		{
			pView->RecordGoIOComment("Unlogged traffic occurs here - see CGoIO_consoleDoc::OpenSensor().");

			GSensorDDSRec DDSRec;
			long nResult = pDevice->ReadSensorDDSMemory((unsigned char *) &DDSRec, 0, sizeof(DDSRec), 2000);
			if ((kResponse_OK == nResult) && GMBLSensor::VerifyDDSChecksum(DDSRec, false))
				m_pSensor->SetDDSRec(DDSRec, true);
		}

		cppsstream ss;
		ss << "Sensor id = " << id;
		if (m_pSensor->IsSmartSensor())
		{
			cppstring sensorName = cppstring(m_pSensor->GetDDSRecPtr()->SensorLongName, sizeof(m_pSensor->GetDDSRecPtr()->SensorLongName));
			ss << " ( " << sensorName << " ) ";
		}
		SetTitle(ss.str().c_str());

		cppstring sUnits = m_pSensor->GetUnits();
		CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
		if (pFrame)
		{
			CWnd *pWnd = pFrame->m_wndSendBar.GetDlgItem(IDC_UNITS);
			if (pWnd)
				pWnd->SetWindowText(sUnits.c_str());
		}

		if ((SKIP_DEFAULT_PRODUCT_ID == m_pDevice->GetPortRefPtr()->GetUSBProductID()) ||
			(MINI_GC_DEFAULT_PRODUCT_ID == m_pDevice->GetPortRefPtr()->GetUSBProductID()))
		{
			//Setup the analog input channel based on the sensor's probe type.
			GSkipGenericResponsePacket response;
			memset(&response, 0, sizeof(response));
			long responseCount = sizeof(response.responsePayload);
			GSkipOutputPacket cmdPacket;
			memset(&cmdPacket, 0, sizeof(cmdPacket));
			cmdPacket.cmd = SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL;
			GSkipSetAnalogInputChannelParams *pParams = (GSkipSetAnalogInputChannelParams *) &cmdPacket.params[0];
			pParams->analogInputChannel = (m_pSensor->GetProbeType() == kProbeTypeAnalog10V) ? 
				SKIP_ANALOG_INPUT_CHANNEL_VIN : SKIP_ANALOG_INPUT_CHANNEL_VIN_LOW;

			pView->RecordGoIOOutput((LPCSTR) &cmdPacket, sizeof(cmdPacket));//Log what we are sending.
			if (kResponse_OK == pDevice->SendCmdAndGetResponse(SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL, pParams, 
				sizeof(GSkipSetAnalogInputChannelParams), &response.responsePayload[0], &responseCount))
			{
				response.header = SKIP_CMD_RESP_DEFAULT_HDR;
				response.cmd = SKIP_CMD_ID_SET_ANALOG_INPUT_CHANNEL;
				pView->RecordGoIOInput((LPCSTR) &response, sizeof(response));//Log what came back.
			}
		}
	}

	return m_pSensor;
}

void CGoIO_consoleDoc::CloseSensor()
{
	if (m_pSensor)
		delete m_pSensor;
	m_pSensor = NULL;

	SetTitle("");

	CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
	if (pFrame)
	{
		CWnd *pWnd = pFrame->m_wndSendBar.GetDlgItem(IDC_UNITS);
		if (pWnd)
			pWnd->SetWindowText("");
	}
}
