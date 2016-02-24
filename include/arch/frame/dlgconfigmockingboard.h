#if !defined(AFX_DLGCONFIGMOCKINGBOARD_H__8256FD6E_641D_4769_8F51_D0C59475BCD2__INCLUDED_)
#define AFX_DLGCONFIGMOCKINGBOARD_H__8256FD6E_641D_4769_8F51_D0C59475BCD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgConfigMockingBoard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigMockingBoard dialog

class CDlgConfigMockingBoard : public CDialog
{
// Construction
public:
	void SetDipSwitch(int nDipSwitch);
	int GetDipSwitch();
	int GetRightVol();
	int GetLeftVol();
	void SetRightVol(int vol);
	void SetLeftVol(int vol);
	CDlgConfigMockingBoard(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CDlgConfigMockingBoard)
	enum { IDD = IDD_CONFIGURE_MOCKINGBOARD };
	CButton	m_btnMockingBoard;
	CButton	m_btnPhasorNative;
	CSliderCtrl	m_slVolume;
	CSliderCtrl	m_slBalance;
	BOOL	m_bMute;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfigMockingBoard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	unsigned int m_uLeftVol;
	unsigned int m_uRightVol;
	int m_nDipSwitch;

	// Generated message map functions
	//{{AFX_MSG(CDlgConfigMockingBoard)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIGMOCKINGBOARD_H__8256FD6E_641D_4769_8F51_D0C59475BCD2__INCLUDED_)
