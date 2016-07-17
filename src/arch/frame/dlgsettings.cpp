// F:\night_project\appleinpc\src\arch\frame\dlgsettings.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettings.h"

// CDlgSettings dialog

int CDlgSettings::s_nTabSelect = 0;

IMPLEMENT_DYNAMIC(CDlgSettings, CDialogEx)

CDlgSettings::CDlgSettings(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS, pParent)
{

}

CDlgSettings::~CDlgSettings()
{
}

void CDlgSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SETTINGS_TAB, m_cTabs);
}


BEGIN_MESSAGE_MAP(CDlgSettings, CDialogEx)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SETTINGS_TAB, &CDlgSettings::OnSelchangeSettingsTab)
END_MESSAGE_MAP()


// CDlgSettings message handlers


BOOL CDlgSettings::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect tabRect;

	m_cTabs.GetWindowRect(tabRect);

	// Set the size and location of the child windows based on the tab control
	m_cTabsRect.left = 15;
	m_cTabsRect.top = 35;
	m_cTabsRect.right = tabRect.Width() - 15;
	m_cTabsRect.bottom = tabRect.Height() - 35;

	// Create the child windows for the main window class
	m_cDlgMachine.Create(IDD_SETTINGS_MACHINE, this);
	m_cDlgSlot.Create(IDD_SETTINGS_SLOT, this);
	m_cDlgInput.Create(IDD_SETTINGS_INPUT, this);
	m_cDlgSound.Create(IDD_SETTINGS_SOUND, this);
	m_cDlgColor.Create(IDD_SETTINGS_COLOR, this);

	m_cDlgMachine.SetBackgroundColor(RGB(255, 255, 255));
	m_cDlgSlot.SetBackgroundColor(RGB(255, 255, 255));
	m_cDlgInput.SetBackgroundColor(RGB(255, 255, 255));
	m_cDlgSound.SetBackgroundColor(RGB(255, 255, 255));
	m_cDlgColor.SetBackgroundColor(RGB(255, 255, 255));

	// This is redundant with the default value, considering what OnShowWindow does
	ShowWindowNumber(s_nTabSelect);

	// Set the titles for each tab
	TCITEM tabItem;
	tabItem.mask = TCIF_TEXT;

	tabItem.pszText = _T(" Machine ");
	m_cTabs.InsertItem(0, &tabItem);

	tabItem.pszText = _T("  Slots  ");
	m_cTabs.InsertItem(1, &tabItem);

	tabItem.pszText = _T("  Input  ");
	m_cTabs.InsertItem(2, &tabItem);

	tabItem.pszText = _T("  Sound  ");
	m_cTabs.InsertItem(3, &tabItem);

	tabItem.pszText = _T("  Color  ");
	m_cTabs.InsertItem(4, &tabItem);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSettings::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow)
	{
		ShowWindowNumber(s_nTabSelect);
	}
}


void CDlgSettings::OnSelchangeSettingsTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	(void)pNMHDR;

	s_nTabSelect = m_cTabs.GetCurFocus();
	// Get the number of the currently selected tab, and show its window
	ShowWindowNumber(s_nTabSelect);
	
	*pResult = 0;
}

void CDlgSettings::ShowWindowNumber(int nTabNumber)
{
	// This example uses four windows
	// Validate the parameter

	if (nTabNumber < 0 || nTabNumber >= TAB_COUNT)
	{
		nTabNumber = 0;
	}

	CDialog *apDlgTabs[TAB_COUNT];

	apDlgTabs[0] = &m_cDlgMachine;
	apDlgTabs[1] = &m_cDlgSlot;
	apDlgTabs[2] = &m_cDlgInput;
	apDlgTabs[3] = &m_cDlgSound;
	apDlgTabs[4] = &m_cDlgColor;

	for (int i = 0; i < TAB_COUNT; i++)
	{
		if (i == nTabNumber)
		{
			apDlgTabs[i]->SetWindowPos(&wndTop, m_cTabsRect.left, m_cTabsRect.top, m_cTabsRect.right, m_cTabsRect.bottom, SWP_SHOWWINDOW);
			m_cTabs.SetCurSel(i);
		}
		else
		{
			apDlgTabs[i]->ShowWindow(SW_HIDE);
		}
	}

}



void CDlgSettings::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	m_cDlgMachine.OnOK();
	m_cDlgSlot.OnOK();
	m_cDlgInput.OnOK();
	m_cDlgSound.OnOK();
	m_cDlgColor.OnOK();

	CDialogEx::OnOK();
}


void CDlgSettings::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	m_cDlgMachine.OnCancel();
	m_cDlgSlot.OnCancel();
	m_cDlgInput.OnCancel();
	m_cDlgSound.OnCancel();
	m_cDlgColor.OnCancel();

	CDialogEx::OnCancel();
}
