#pragma once

#include "arch/frame/wcsliderbutton.h"
#include "afxwin.h"

// CDlgSettingsInput dialog

class CDlgSettingsInput : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettingsInput)

public:
	CDlgSettingsInput(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettingsInput();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_INPUT };
#endif

protected:
	static const int m_anDeadZoneList[];
	static const int m_anSaturationList[];

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	wcSliderButton	m_cKeyDelay;
	wcSliderButton	m_cKeyRepeat;
	CButton m_acJoystick[3];
	CComboBox m_cJoyDead;
	CComboBox m_cJoySat;
	BOOL m_bArrowAsPaddle;
	BOOL m_bSwapButtons;

	virtual void OnOK();
	virtual void OnCancel();

	virtual BOOL OnInitDialog();

	afx_msg void OnJoystickChanged();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
