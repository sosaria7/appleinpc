#pragma once
#include "afxwin.h"


// CDlgSettingsMachine dialog

class CDlgSettingsMachine : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettingsMachine)

public:
	CDlgSettingsMachine(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettingsMachine();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_MACHINE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual void OnOK();
	virtual void OnCancel();
	CButton m_cMachineA2p;
	CButton m_cMachineA2e;
	CButton m_cMachineNTSC;
	CButton m_cMachinePAL;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	CEdit m_cSateFilename;
	CString m_strStateFileName;
public:
	void SetFileName(CEdit *edit, CString path);
	afx_msg void OnClickedStateLoad();
	afx_msg void OnClickedStateSave();
	afx_msg void OnClickedStateBrowse();
	afx_msg void OnSetfocusStateFilename();
	afx_msg void OnKillfocusStateFilename();
};
