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

