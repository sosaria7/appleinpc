#if !defined(AFX_APPLESTATUSBAR_H__20D8D936_8BBC_452A_A297_0C420BBA60C6__INCLUDED_)
#define AFX_APPLESTATUSBAR_H__20D8D936_8BBC_452A_A297_0C420BBA60C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppleStatusBar.h : header file
//

#define KEY_STATE_CAPTURE		0
#define KEY_STATE_CAPS			1
#define KEY_STATE_SCROLL		2

/////////////////////////////////////////////////////////////////////////////
// CAppleStatusBar window

class CAppleStatusBar : public CStatusBar
{
// Construction
public:
	CAppleStatusBar();

// Attributes
public:
protected:
	CBitmap m_bmDisk;
	CBitmap m_bmDiskOff;
	CBitmap m_bmDiskRead;
	CBitmap m_bmDiskWrite;

	CBitmap m_bmDiskOff_Full;
	CBitmap m_bmDiskRead_Full;
	CBitmap m_bmDiskWrite_Full;

	CBitmap m_bmCapsOn;
	CBitmap m_bmCapsOff;
	CBitmap m_bmScrollLockOn;
	CBitmap m_bmScrollLockOff;

	int m_iDiskStatus[5];
	bool m_bKeyCaptured;
	bool m_bKeyCaps;
	bool m_bKeyScrollLock;
	CBitmap m_bmHdd;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAppleStatusBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetMessage( LPCTSTR lpszText );
	void DrawDiskLight(HDC hDC, RECT rect);
	void DrawDiskLight_Full(HDC hDC, RECT rect);
	void DrawKeyStatus(HDC hDC, RECT rc);
	void SetDiskStatus(int index, int status);
	void SetKeyStatus(int index, int status);
	void SetFrame(double frame);
	void SetSpeed(double speed);
	BOOL Create(CWnd *pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM);
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItem);
	virtual ~CAppleStatusBar();

	friend CALLBACK_HANDLER(OnDiskLightChange);
	// Generated message map functions
protected:
	CRect m_rectDisk;
	CRect m_rectKeyStatus;
	//{{AFX_MSG(CAppleStatusBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLESTATUSBAR_H__20D8D936_8BBC_452A_A297_0C420BBA60C6__INCLUDED_)
