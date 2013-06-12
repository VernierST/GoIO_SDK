// GoIO_consoleDoc.h : interface of the CGoIO_consoleDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_)
#define AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GDeviceIO.h"
#include "GMBLSensor.h"

class CGoIO_consoleDoc : public CDocument
{
protected: // create from serialization only
	CGoIO_consoleDoc();
	DECLARE_DYNCREATE(CGoIO_consoleDoc)

// Attributes
public:
	GDeviceIO *GetOpenDevicePtr() { return m_pDevice; }
	GMBLSensor *GetOpenSensorPtr() { return m_pSensor; }

// Operations
public:
	GDeviceIO *OpenDevice(GPortRef *pPortRef);
	void CloseDevice();
	GMBLSensor *OpenSensor(int id);
	void CloseSensor();

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


	GDeviceIO *m_pDevice;
	GMBLSensor *m_pSensor;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOIO_CONSOLEDOC_H__8FDFDA77_BC76_45A1_AFCD_CB8D9A631AE5__INCLUDED_)
