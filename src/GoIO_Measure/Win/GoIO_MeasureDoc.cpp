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
// GoIO_MeasureDoc.cpp : implementation of the CGoIO_MeasureDoc class
//

#include "stdafx.h"
#include "GoIO_Measure.h"

#include "MainFrm.h"
#include "GoIO_MeasureDoc.h"
#include "GoIO_MeasureView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGoIO_MeasureDoc

IMPLEMENT_DYNCREATE(CGoIO_MeasureDoc, CDocument)

BEGIN_MESSAGE_MAP(CGoIO_MeasureDoc, CDocument)
END_MESSAGE_MAP()


// CGoIO_MeasureDoc construction/destruction

CGoIO_MeasureDoc::CGoIO_MeasureDoc()
{
	m_pDevice = NULL;
	m_pMeasCirBuf = new GCircularBuffer(DEFAULT_MAX_NUM_MEASUREMENTS_IN_CIRBUF*sizeof(double));
	m_pTimeCirBuf = new GCircularBuffer(DEFAULT_MAX_NUM_MEASUREMENTS_IN_CIRBUF*sizeof(double));
	ClearMeasurementCirbuf();
	m_measPeriodInSeconds = 1000000.0;
}

CGoIO_MeasureDoc::~CGoIO_MeasureDoc()
{
	if (m_pDevice)
		CloseDevice();

	if (m_pTimeCirBuf)
		delete m_pTimeCirBuf;
	m_pTimeCirBuf = NULL;

	if (m_pMeasCirBuf)
		delete m_pMeasCirBuf;
	m_pMeasCirBuf = NULL;
}

BOOL CGoIO_MeasureDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	if (GetOpenDevicePtr() != NULL)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(m_pDevice, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);
		CloseDevice();
		OpenDevice(openVendorId, openProductId, openDeviceName);
		CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
		if (pFrame)
			pFrame->ClearCollectingMeasFlag();
	}
	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGoIO_MeasureDoc serialization

void CGoIO_MeasureDoc::Serialize(CArchive& ar)
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
// CGoIO_MeasureDoc diagnostics

#ifdef _DEBUG
void CGoIO_MeasureDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGoIO_MeasureDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGoIO_MeasureDoc commands

GOIO_SENSOR_HANDLE CGoIO_MeasureDoc::OpenDevice(int vendorId, int productId, LPCSTR pDeviceName)
{
	CGoIO_MeasureView *pView = (CGoIO_MeasureView *) (((CFrameWnd *) AfxGetMainWnd())->GetActiveView());
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
			pFrame->m_wndCollectBar.SetDlgItemText(IDC_UNITS, tmpstring);

		SetMeasurementPeriodInSeconds(GoIO_Sensor_GetMeasurementPeriod(m_pDevice, SKIP_TIMEOUT_MS_DEFAULT));
	}

	return m_pDevice;
}

void CGoIO_MeasureDoc::CloseDevice()
{
	if (m_pDevice)
		GoIO_Sensor_Close(m_pDevice);
	m_pDevice = NULL;

	ClearMeasurementCirbuf();

	CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();
	if (pFrame)
	{
		pFrame->m_wndCollectBar.SetDlgItemText(IDC_MEAS, "");
		pFrame->m_wndCollectBar.SetDlgItemText(IDC_UNITS, "");
		CGoIO_MeasureView *pView = (CGoIO_MeasureView *) (pFrame->GetActiveView());
		if (pView)
		{
			pView->SetGraphHistory(1.0, -1.0);//This clears the history.
			pView->Invalidate();
		}
	}

	SetTitle("");
}

void CGoIO_MeasureDoc::AddMeasurementToCirbuf(double measurement)
{
	double timestamp, old_meas;

	int N = GetNumMeasurementsInCirbuf();
	if (0 == N)
		timestamp = 0.0;
	else
	{
		GetNthMeasurementInCirbuf(N - 1, &old_meas, &timestamp);
		timestamp += GetMeasurementPeriodInSeconds();
	}
	m_pMeasCirBuf->AddBytes((unsigned char *) &measurement, sizeof(measurement));
	m_pTimeCirBuf->AddBytes((unsigned char *) &timestamp, sizeof(timestamp));
}

void CGoIO_MeasureDoc::ClearMeasurementCirbuf()
{
	m_pMeasCirBuf->Clear();
	m_pTimeCirBuf->Clear();
}

bool CGoIO_MeasureDoc::GetNthMeasurementInCirbuf(int N, double *pMeasurement, double *pTime) //(N == 0) => first measurement.
{
	bool bSuccess = false;
	double measurement = 0.0;
	double time = 0.0;
	int count = m_pMeasCirBuf->CopyBytes((unsigned char *) &measurement, N*sizeof(double), sizeof(double));
	if (sizeof(double) == count)
	{
		count = m_pTimeCirBuf->CopyBytes((unsigned char *) &time, N*sizeof(double), sizeof(double));
		if (sizeof(double) == count)
		{
			bSuccess = true;
			*pMeasurement = measurement;
			*pTime = time;
		}
	}

	return bSuccess;
}

void CGoIO_MeasureDoc::SetMaxNumMeasurementsInCirBuf(int maxNum)
{
	if (m_pMeasCirBuf)
	{
		if (m_pMeasCirBuf->MaxNumBytesAvailable() != (maxNum*sizeof(double)))
		{
			delete m_pMeasCirBuf;
			m_pMeasCirBuf = NULL;
		}
	}
	if (m_pTimeCirBuf)
	{
		if (m_pTimeCirBuf->MaxNumBytesAvailable() != (maxNum*sizeof(double)))
		{
			delete m_pTimeCirBuf;
			m_pTimeCirBuf = NULL;
		}
	}
	if (!m_pMeasCirBuf)
		m_pMeasCirBuf = new GCircularBuffer(maxNum*sizeof(double));
	if (!m_pTimeCirBuf)
		m_pTimeCirBuf = new GCircularBuffer(maxNum*sizeof(double));
}

