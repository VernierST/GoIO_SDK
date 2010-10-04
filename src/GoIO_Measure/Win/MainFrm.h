// MainFrm.h : interface of the CMainFrame class
//


#pragma once

class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
	bool IsCollectingMeasurements()
	{
		return m_bIsCollectingMeasurements;
	}
	void ClearCollectingMeasFlag()
	{
		m_bIsCollectingMeasurements = false;
	}

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL DestroyWindow();

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CDialogBar	m_wndCollectBar;

protected:  // control bar embedded members
//	CStatusBar  m_wndStatusBar;
//	CToolBar    m_wndToolBar;
	UINT_PTR m_timerId;
	bool m_bIsCollectingMeasurements;

	void OnDeviceN(unsigned int N);
	void OnCalibN(unsigned int N);
	void SetLEDColor(int color, int brightness);

// Generated message map functions
protected:
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateGetStatus(CCmdUI* pCmdUI);
	afx_msg void OnGetStatus();
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
	afx_msg void OnCalib0();
	afx_msg void OnCalib1();
	afx_msg void OnCalib2();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateActionSetdisplaydepth(CCmdUI *pCmdUI);
	afx_msg void OnActionSetdisplaydepth();
	afx_msg void OnUpdateGetSensorId(CCmdUI *pCmdUI);
	afx_msg void OnGetSensorId();
	afx_msg void OnUpdateGoioLibVerbose(CCmdUI *pCmdUI);
	afx_msg void OnGoioLibVerbose();
};


