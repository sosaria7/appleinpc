#if !defined(AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_)
#define AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgConfigDisk.h : header file
//

#include "diskinterface.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigDisk dialog

class CDlgConfigDisk : public CDialog
{
// Construction
public:
	CDlgConfigDisk(CDiskInterface* pInterface, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfigDisk)
	enum { IDD = IDD_CONFIGURE_DISK };
	CButton	m_btnEnableDisk2;
	CButton	m_btnEnableDisk1;
	CButton	m_btnBrowseDisk2;
	CButton	m_btnBrowseDisk1;
	CString	m_strDisk1Image;
	CString	m_strDisk2Image;
	BOOL	m_bEnhanced1;
	BOOL	m_bEnhanced2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfigDisk)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CDiskInterface* m_pInterface;

	// Generated message map functions
	//{{AFX_MSG(CDlgConfigDisk)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnBtnBrowse2();
	afx_msg void OnChkDisk2();
	afx_msg void OnChkDisk1();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_)
