// GoIO_consoleDoc.h : interface of the CGoIO_consoleDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_)
#define AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GoIO_DLL_interface.h"

#define MAX_NUM_MEASUREMENTS_IN_CIRBUF 501

class CGoIO_consoleDoc : public CDocument
{
protected: // create from serialization only
	CGoIO_consoleDoc();
	DECLARE_DYNCREATE(CGoIO_consoleDoc)

// Attributes
public:
	GOIO_SENSOR_HANDLE GetOpenDevicePtr() { return m_pDevice; }

// Operations
public:
	GOIO_SENSOR_HANDLE OpenDevice(int vendorId, int productId, LPCSTR pDeviceName);
	void CloseDevice();

	void AddMeasurementToCirbuf(double measurement)
	{
		if (MAX_NUM_MEASUREMENTS_IN_CIRBUF == m_numMeasurementsInCirbuf)
		{
			m_firstCirbufMeasurementIndex++;
			m_firstCirbufMeasurementIndex = m_firstCirbufMeasurementIndex % MAX_NUM_MEASUREMENTS_IN_CIRBUF;
			m_numMeasurementsInCirbuf--;
		}
		m_measurements_cirbuf[(m_firstCirbufMeasurementIndex + m_numMeasurementsInCirbuf) % MAX_NUM_MEASUREMENTS_IN_CIRBUF]
			= measurement;
		m_numMeasurementsInCirbuf++;
	}
	int GetNumMeasurementsInCirbuf() { return m_numMeasurementsInCirbuf; }
	void ClearMeasurementCirbuf()
	{
		m_firstCirbufMeasurementIndex = 0;
		m_numMeasurementsInCirbuf = 0;
	}
	double GetNthMeasurementInCirbuf(int N) //(N == 0) => first measurement.
	{
		if (N >= m_numMeasurementsInCirbuf)
			return 0.0;
		else
			return m_measurements_cirbuf[(m_firstCirbufMeasurementIndex + N) % MAX_NUM_MEASUREMENTS_IN_CIRBUF];
	}
	void SetMeasurementPeriodInSeconds(double period) { m_measPeriodInSeconds = period; }
	double GetMeasurementPeriodInSeconds() { return m_measPeriodInSeconds; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoIO_consoleDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGoIO_consoleDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGoIO_consoleDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


	GOIO_SENSOR_HANDLE m_pDevice;
	double m_measurements_cirbuf[MAX_NUM_MEASUREMENTS_IN_CIRBUF];
	int m_numMeasurementsInCirbuf;
	int m_firstCirbufMeasurementIndex;
	double m_measPeriodInSeconds;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_)
