#if !defined(AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_)
#define AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgConfigDisk.h : header file
//

#include "diskinterface.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigDisk dialog

class CDlgConfigDisk : public CDialog
{
// Construction
public:
	CDlgConfigDisk(CDiskInterface* pInterface, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_CONFIGURE_DISK };
	CButton	m_btnEnableDisk2;
	CButton	m_btnEnableDisk1;
	CButton	m_btnBrowseDisk2;
	CButton	m_btnBrowseDisk1;
	CString	m_strDisk1Image;
	CString	m_strDisk2Image;
	BOOL	m_bEnhanced1;
	BOOL	m_bEnhanced2;

// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CDiskInterface* m_pInterface;

	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnBtnBrowse2();
	afx_msg void OnChkDisk2();
	afx_msg void OnChkDisk1();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_cDisk1Path;
	CEdit m_cDisk2Path;
	afx_msg void OnSetfocusEditFilename1();
	afx_msg void OnKillfocusEditFilename1();
	afx_msg void OnSetfocusEditFilename2();
	afx_msg void OnKillfocusEditFilename2();
	void SetFileName(CEdit *edit, CString path);
};

#endif // !defined(AFX_DLGCONFIGDISK_H__EE2A993B_E71D_406F_9138_2E4448E32306__INCLUDED_)
