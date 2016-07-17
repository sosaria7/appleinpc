#pragma once
#include "afxwin.h"
#include "card.h"

// CDlgSettingsSlot dialog

class CDlgSettingsSlot : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSettingsSlot)

public:
	CDlgSettingsSlot(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSettingsSlot();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_SLOT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_acSlots[7];
	CButton m_acSetupCard[7];
	CCard* m_pCards[7];
	afx_msg void OnSelchangeSlot(UINT uId);
	afx_msg void OnClickedSlotSetup(UINT uId);
	int DeviceNameToDeviceID(CString strDeviceName);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
