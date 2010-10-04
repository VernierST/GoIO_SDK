// GoIO_MeasureDoc.h : interface of the CGoIO_MeasureDoc class
//
#include "GoIO_DLL_interface.h"

#pragma once

#include "GCircularBuffer.h"

#define DEFAULT_MAX_NUM_MEASUREMENTS_IN_CIRBUF 501

class CGoIO_MeasureDoc : public CDocument
{
protected: // create from serialization only
	CGoIO_MeasureDoc();
	DECLARE_DYNCREATE(CGoIO_MeasureDoc)

// Attributes
public:
	GOIO_SENSOR_HANDLE GetOpenDevicePtr() { return m_pDevice; }

// Operations
public:
	GOIO_SENSOR_HANDLE OpenDevice(int vendorId, int productId, LPCSTR pDeviceName);
	void CloseDevice();

	void AddMeasurementToCirbuf(double measurement);
	int GetNumMeasurementsInCirbuf() { return m_pMeasCirBuf->NumBytesAvailable()/sizeof(double); }
	void ClearMeasurementCirbuf();
	void SetMaxNumMeasurementsInCirBuf(int maxNum);
	int GetMaxNumMeasurementsInCirbuf() { return m_pMeasCirBuf->MaxNumBytesAvailable()/sizeof(double); }
	bool GetNthMeasurementInCirbuf(int N, double *pMeasurement, double *pTime); //(N == 0) => first measurement.
	void SetMeasurementPeriodInSeconds(double period) { m_measPeriodInSeconds = period; }
	double GetMeasurementPeriodInSeconds() { return m_measPeriodInSeconds; }

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CGoIO_MeasureDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	GOIO_SENSOR_HANDLE m_pDevice;
	double m_measPeriodInSeconds;
	GCircularBuffer *m_pMeasCirBuf;
	GCircularBuffer *m_pTimeCirBuf;
};


