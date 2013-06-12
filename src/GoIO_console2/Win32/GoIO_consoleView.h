// GoIO_consoleView.h : interface of the CGoIO_consoleView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GOIO_CONSOLEVIEW_H__29153F26_3D6C_4009_9792_D64C23E59E02__INCLUDED_)
#define AFX_GOIO_CONSOLEVIEW_H__29153F26_3D6C_4009_9792_D64C23E59E02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define REPORT_RECORD_DATA_LENGTH 32

class CGoIO_consoleDoc;

class CGoIO_consoleView : public CView
{
protected: // create from serialization only
	CGoIO_consoleView();
	DECLARE_DYNCREATE(CGoIO_consoleView)

// Attributes
public:
	CGoIO_consoleDoc* GetDocument();
	void SetGraphHistory(double y_min, double y_max)
	{
		graph_history_y_min = y_min;
		graph_history_y_max = y_max;
	}
	void GetGraphHistory(double &y_min, double &y_max)
	{
		y_min = graph_history_y_min;
		y_max = graph_history_y_max;
	}

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoIO_consoleView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGoIO_consoleView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void RecordGoIOString(LPCSTR pBuf, int buf_len, LPCSTR label);

// Generated message map functions
protected:
	//{{AFX_MSG(CGoIO_consoleView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	double graph_history_y_min;
	double graph_history_y_max;
};

#ifndef _DEBUG  // debug version in GoIO_consoleView.cpp
inline CGoIO_consoleDoc* CGoIO_consoleView::GetDocument()
   { return (CGoIO_consoleDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOIO_CONSOLEVIEW_H__29153F26_3D6C_4009_9792_D64C23E59E02__INCLUDED_)
