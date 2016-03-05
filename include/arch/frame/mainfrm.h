// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__9DAB2781_4BAC_4D3D_BE80_BB3BF894A70E__INCLUDED_)
#define AFX_MAINFRM_H__9DAB2781_4BAC_4D3D_BE80_BB3BF894A70E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "appleclock.h"
#include "arch/frame/screen.h"
#include "arch/frame/applestatusbar.h"

class CMainFrame : public CFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
	
public:
	CMainFrame();
protected: 

// Attributes
public:
	CAppleStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
	CScreen		m_wndView;
	CMenu		m_cMenu;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual void CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType = adjustBorder);
	//}}AFX_VIRTUAL

// Implementation
public:
	CRect m_winRect;
	POINT m_stWindowPos;
	BOOL  m_bDoubleSize;
	BOOL  m_bFullScreen;
	BOOL  m_bKeyboardCapture;
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void ResizeWindow();

protected:  // control bar embedded members
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDebug();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnConfigureSlots();
	afx_msg void OnReboot();
	afx_msg void OnReset();
	afx_msg void OnMonitor();
	afx_msg void OnChangeSize();
	afx_msg void OnUpdateMonitor(CCmdUI* pCmdUI);
	afx_msg void OnPowerOn();
	afx_msg void OnUpdatePowerOn(CCmdUI* pCmdUI);
	afx_msg void OnPowerOff();
	afx_msg void OnUpdatePowerOff(CCmdUI* pCmdUI);
	afx_msg void OnUpdateReset(CCmdUI* pCmdUI);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPower();
	afx_msg void OnDiskette();
	afx_msg void OnUpdatePower(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg LRESULT OnReqAcquire(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyKeyDown(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyKeyUp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyKeyRepeat(WPARAM wParam, LPARAM lParam);
	afx_msg void OnToggleFullScreen();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnHarddisk();
	afx_msg void OnUpdateDiskette(CCmdUI* pCmdUI);
	afx_msg void OnUpdateHarddisk(CCmdUI* pCmdUI);
	afx_msg void OnUpdateScanline(CCmdUI* pCmdUI);
	afx_msg void OnScanline();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__9DAB2781_4BAC_4D3D_BE80_BB3BF894A70E__INCLUDED_)
