#pragma once

#include "arch/frame/colorbutton.h"
#include "arch/frame/wcsliderbutton.h"

// CDlgSettingsColor dialog

class CDlgSettingsColor : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettingsColor)

public:
	CDlgSettingsColor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettingsColor();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_COLOR };
#endif

protected:
	unsigned int m_uHSB;
	unsigned int m_auColors[16];
	unsigned int m_uMono;
	unsigned int m_uGreen;
	int m_nMode;
	int m_nOrgMode;
	BOOL m_bOrgScanline;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	BOOL	m_bPreview;
	BOOL	m_bScanline;
	CColorButton	m_acColorBtn[16];
	wcSliderButton	m_cSaturation;
	wcSliderButton	m_cBright;
	wcSliderButton	m_cHue;

	void ApplyColor();
	void ResetScreen();

	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickedPreview();
	afx_msg void OnClickedScanline();
	afx_msg void OnClickedDefault();
	afx_msg void OnChangeColorMode(UINT uId);
	afx_msg void OnColorClicked(UINT uId);
	afx_msg void OnChangeHSB(UINT uId);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
