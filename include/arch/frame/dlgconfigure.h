#if !defined(AFX_DLGCONFIGURE_H__4223E3CA_EF8E_4070_AC7C_9287C4BA31A8__INCLUDED_)
#define AFX_DLGCONFIGURE_H__4223E3CA_EF8E_4070_AC7C_9287C4BA31A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dlgconfigure.h : header file
//

#include "card.h"
#include "arch/frame/wcsliderbutton.h"
#include "afxwin.h"
/////////////////////////////////////////////////////////////////////////////
// CDlgConfigure dialog

class CDlgConfigure : public CDialog
{
// Construction
public:
	CDlgConfigure(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfigure)
	enum { IDD = IDD_CONFIGURE };
	CSliderCtrl	m_slSpeakerVolume;
	CSliderCtrl	m_slMasterVolume;
	CSliderCtrl	m_slMasterBalance;
	BOOL	m_bMasterMute;
	BOOL	m_bSpeakerMute;
	CComboBox	m_cJoySat;
	CComboBox	m_cJoyDead;
	//}}AFX_DATA
	CButton	m_btnJoystick[3];
	CComboBox	m_cbSlot[7];
	CButton		m_btnSetupCard[7];
	wcSliderButton	m_sbKeyDelay;
	wcSliderButton	m_sbKeyRepeat;
	CButton m_btnMachineNTSC;
	CButton m_btnMachinePAL;
	CButton m_btnMachineA2p;
	CButton m_btnMachineA2e;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfigure)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgConfigure)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnColorSetting();
	afx_msg void OnJoystickChange();
	//}}AFX_MSG
	afx_msg void OnSelchangeSlot(UINT uId);
	afx_msg void OnClickedSlotSetup(UINT uId);

	static int DeviceNameToDeviceID(CString strDeviceName);

public:
	virtual  ~CDlgConfigure();
protected:

	static const int m_anDeadZoneList[];
	static const int m_anSaturationList[];

	CCard* m_pCards[7];

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIGURE_H__4223E3CA_EF8E_4070_AC7C_9287C4BA31A8__INCLUDED_)
