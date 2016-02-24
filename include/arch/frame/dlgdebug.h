#if !defined(AFX_DLGDEBUG_H__CDB82413_19FD_4287_BBAE_8906DB94DC0F__INCLUDED_)
#define AFX_DLGDEBUG_H__CDB82413_19FD_4287_BBAE_8906DB94DC0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgDebug.h : header file
//

#include "65c02.h"
#include "memory.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgDebug dialog

class CDlgDebug : public CDialog
{
// Construction
public:
	void UpdateData();
	int m_aiAddrs[10];
	void addProcess(int num, WORD address);
	CAppleIOU* m_pMemory;
	C65c02* m_pCPU;
	CDlgDebug(C65c02* pCPU, CAppleIOU* pMemory, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgDebug)
	enum { IDD = IDD_DEBUG };
	CStatic	m_flagZ;
	CEdit	m_editData;
	CListCtrl	m_listRegisters;
	CListCtrl	m_listProcess;
	CStatic	m_flagV;
	CStatic	m_flagN;
	CStatic	m_flagI;
	CStatic	m_flagX;
	CStatic	m_flagD;
	CStatic	m_flagC;
	CStatic	m_flagB;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgDebug)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgDebug)
	afx_msg void OnButtonTrace();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGDEBUG_H__CDB82413_19FD_4287_BBAE_8906DB94DC0F__INCLUDED_)
