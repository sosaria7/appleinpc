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
	enum { IDD = IDD_CONFIGURE_HDD };
	CButton	m_btnEnableDisk1;
	CButton	m_btnBrowseDisk1;
	CString	m_strDisk1Image;


// Overrides
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	CHDDInterface* m_pInterface;

	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnChkDisk1();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	CEdit m_cDisk1Path;
	afx_msg void OnSetfocusEditFilename();
	afx_msg void OnKillfocusEditFilename();
	void SetFileName(CEdit *edit, CString path);
};

#endif