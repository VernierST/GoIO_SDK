// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__273DB87C_5453_45F3_BF9F_ACF322642FA8__INCLUDED_)
#define AFX_MAINFRM_H__273DB87C_5453_45F3_BF9F_ACF322642FA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GSkipBaseDevice.h"

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// control bar embedded members
	CDialogBar	m_wndSendBar;

protected:
	CStatusBar  m_wndStatusBar;
//	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnDevice0();
	afx_msg void OnDevice1();
	afx_msg void OnDevice2();
	afx_msg void OnDevice3();
	afx_msg void OnDevice4();
	afx_msg void OnDevice5();
	afx_msg void OnDevice6();
	afx_msg void OnDevice7();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUpdateGetStatus(CCmdUI* pCmdUI);
	afx_msg void OnGetStatus();
	afx_msg void OnGetSensorId();
	afx_msg void OnUpdateGetSensorId(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSetMeasPeriod(CCmdUI* pCmdUI);
	afx_msg void OnSetMeasPeriod();
	afx_msg void OnUpdateStartMeas(CCmdUI* pCmdUI);
	afx_msg void OnStartMeas();
	afx_msg void OnUpdateStopMeas(CCmdUI* pCmdUI);
	afx_msg void OnStopMeas();
	afx_msg void OnUpdateOrange(CCmdUI* pCmdUI);
	afx_msg void OnOrange();
	afx_msg void OnUpdateLedOff(CCmdUI* pCmdUI);
	afx_msg void OnLedOff();
	afx_msg void OnUpdateGreen(CCmdUI* pCmdUI);
	afx_msg void OnGreen();
	afx_msg void OnUpdateRed(CCmdUI* pCmdUI);
	afx_msg void OnRed();
	afx_msg void OnUpdateReadSensorMem(CCmdUI* pCmdUI);
	afx_msg void OnReadSensorMem();
	afx_msg void OnCalib0();
	afx_msg void OnCalib1();
	afx_msg void OnCalib2();
	//}}AFX_MSG
	afx_msg void OnUpdateSendButton(CCmdUI* pCmdUI);
	afx_msg void OnSendPacketToSkip();
	DECLARE_MESSAGE_MAP()

	void OnDeviceN(unsigned int N);
	void OnCalibN(unsigned int N);
	void SetLEDColor(ELEDSettings setting);

	int m_timerId;
	bool m_bIsCollectingMeasurements;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__273DB87C_5453_45F3_BF9F_ACF322642FA8__INCLUDED_)
