#pragma once
#include "afxcmn.h"


// CDlgSettingsSound dialog

class CDlgSettingsSound : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettingsSound)

public:
	CDlgSettingsSound(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettingsSound();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_SOUND };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CSliderCtrl m_cBalance;
	CSliderCtrl m_cVolume;
	CSliderCtrl m_cSpeakerVolume;
	BOOL m_bMute;
	BOOL m_bSpeakerMute;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
