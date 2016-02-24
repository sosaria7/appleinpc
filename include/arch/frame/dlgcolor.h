#if !defined(AFX_DLGCOLOR_H__EE8D0637_155C_4F94_8215_CC1F0F4D34AD__INCLUDED_)
#define AFX_DLGCOLOR_H__EE8D0637_155C_4F94_8215_CC1F0F4D34AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dlgcolor.h : header file
//
#include "arch/frame/colorbutton.h"
#include "arch/frame/wcsliderbutton.h"
/////////////////////////////////////////////////////////////////////////////
// CDlgColor dialog

class CDlgColor : public CDialog
{
// Attributes
public:
protected:
	unsigned int m_uHSB;
	unsigned int m_auColors[16];
	unsigned int m_uMono;
	unsigned int m_uGreen;
	int m_nMode;
	int m_nOrgMode;
	BOOL m_bOrgScanline;

// Construction
public:
	CDlgColor(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgColor)
	enum { IDD = IDD_CONFIGURE_COLOR };
	BOOL	m_bPreview;
	BOOL	m_bScanline;
	//}}AFX_DATA
	CColorButton	m_acColorBtn[16];
	wcSliderButton	m_cSaturation;
	wcSliderButton	m_cBright;
	wcSliderButton	m_cHue;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgColor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:

protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgColor)
	virtual BOOL OnInitDialog();
	afx_msg void OnPreview();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDefault();
	afx_msg void OnScanline();
	//}}AFX_MSG
	afx_msg void OnChangeColorMode(UINT uId);
	afx_msg void OnColorClicked(UINT uId);
	afx_msg void OnChangeHSB(UINT uId);

	void ApplyColor();
	void ResetScreen();

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCOLOR_H__EE8D0637_155C_4F94_8215_CC1F0F4D34AD__INCLUDED_)
