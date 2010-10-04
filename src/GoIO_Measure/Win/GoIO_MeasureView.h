// GoIO_MeasureView.h : interface of the CGoIO_MeasureView class
//


#pragma once

#define REPORT_RECORD_DATA_LENGTH 32

class CGoIO_MeasureView : public CView
{
protected: // create from serialization only
	CGoIO_MeasureView();
	DECLARE_DYNCREATE(CGoIO_MeasureView)

// Attributes
public:
	CGoIO_MeasureDoc* GetDocument() const; // non-debug version is inline
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
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:

// Implementation
public:
	virtual ~CGoIO_MeasureView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void RecordGoIOString(LPCSTR pBuf, int buf_len, LPCSTR label);

// Generated message map functions
protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

	double graph_history_y_min;
	double graph_history_y_max;
};


#ifndef _DEBUG  // debug version in GoIO_MeasureView.cpp
inline CGoIO_MeasureDoc* CGoIO_MeasureView::GetDocument() const
   { return reinterpret_cast<CGoIO_MeasureDoc*>(m_pDocument); }
#endif

