#if !defined(__DLG_CONFIG_HDD_H__)
#define __DLG_CONFIG_HDD_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// dlgconfighdd.h : header file
//

#include "hdd.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigHDD dialog

class CDlgConfigHDD : public CDialog
{
// Construction
public:
	CDlgConfigHDD(CHDDInterface* pInterface, CWnd* pParent =NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfigHDD)
	enum { IDD = IDD_CONFIGURE_HDD };
	CButton	m_btnEnableDisk1;
	CButton	m_btnBrowseDisk1;
	CString	m_strDisk1Image;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfigHDD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CHDDInterface* m_pInterface;

	// Generated message map functions
	//{{AFX_MSG(CDlgConfigHDD)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnChkDisk1();
	virtual void OnOK();
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif