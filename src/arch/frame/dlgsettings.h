#pragma once
#include "afxcmn.h"
#include "afxdialogex.h"
#include "dlgsettingscolor.h"
#include "dlgsettingsinput.h"
#include "dlgsettingsmachine.h"
#include "dlgsettingsslot.h"
#include "dlgsettingssound.h"

// CDlgSettings dialog

class CDlgSettings : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettings)

public:
	CDlgSettings(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettings();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	static const int TAB_COUNT = 5;

	CDialogEx m_cDlgTabs[TAB_COUNT];
public:
	CTabCtrl m_cTabs;
	CDlgSettingsColor m_cDlgColor;
	CDlgSettingsInput m_cDlgInput;
	CDlgSettingsMachine m_cDlgMachine;
	CDlgSettingsSlot m_cDlgSlot;
	CDlgSettingsSound m_cDlgSound;

	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeSettingsTab(NMHDR *pNMHDR, LRESULT *pResult);

protected:
	CRect m_cTabsRect;
	void ShowWindowNumber(int nTabNumber);
	static int s_nTabSelect;

	virtual void OnOK();
	virtual void OnCancel();
};
