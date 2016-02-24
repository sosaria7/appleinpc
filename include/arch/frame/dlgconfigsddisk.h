#if !defined(AFX_DLGCONFIGSDDISK_H__39F6C26E_6F82_4A72_9C21_085546FCB01C__INCLUDED_)
#define AFX_DLGCONFIGSDDISK_H__39F6C26E_6F82_4A72_9C21_085546FCB01C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dlgconfigsddisk.h : header file
//

#include "sddiskii.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigSDDisk dialog

class CDlgConfigSDDisk : public CDialog
{
// Construction
public:
	CDlgConfigSDDisk(CSDDiskII* pInterface, CWnd* pParent =NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfigSDDisk)
	enum { IDD = IDD_CONFIGURE_SD_DISK };
	CButton	m_btnEnableDisk2;
	CButton	m_btnEnableDisk1;
	CButton	m_btnBrowseDisk2;
	CButton	m_btnBrowseDisk1;
	CString	m_strDisk1Image;
	CString	m_strDisk2Image;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfigSDDisk)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CSDDiskII* m_pInterface;

	// Generated message map functions
	//{{AFX_MSG(CDlgConfigSDDisk)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnBtnBrowse2();
	afx_msg void OnChkDisk2();
	afx_msg void OnChkDisk1();
	virtual void OnOK();
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIGSDDISK_H__39F6C26E_6F82_4A72_9C21_085546FCB01C__INCLUDED_)
