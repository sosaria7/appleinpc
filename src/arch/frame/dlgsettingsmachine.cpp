// F:\night_project\appleinpc\src\arch\frame\dlgsettingsmachine.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettingsmachine.h"
#include "appleclock.h"

// CDlgSettingsMachine dialog

IMPLEMENT_DYNAMIC(CDlgSettingsMachine, CDialogEx)

CDlgSettingsMachine::CDlgSettingsMachine(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_MACHINE, pParent)
{

}

CDlgSettingsMachine::~CDlgSettingsMachine()
{
}

void CDlgSettingsMachine::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MACHINE_2PLUS, m_cMachineA2p);
	DDX_Control(pDX, IDC_MACHINE_2E, m_cMachineA2e);
	DDX_Control(pDX, IDC_MACHINE_NTSC, m_cMachineNTSC);
	DDX_Control(pDX, IDC_MACHINE_PAL, m_cMachinePAL);
}


BEGIN_MESSAGE_MAP(CDlgSettingsMachine, CDialogEx)
END_MESSAGE_MAP()


// CDlgSettingsMachine message handlers


void CDlgSettingsMachine::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	BOOL bPalMode = m_cMachinePAL.GetCheck();
	int nMachineType;

	if (m_cMachineA2p.GetCheck() == BST_CHECKED)
		nMachineType = MACHINE_APPLE2P;
	else
		nMachineType = MACHINE_APPLE2E;

	g_pBoard->SetMachineType(nMachineType, bPalMode);

	CDialogEx::OnOK();
}

void CDlgSettingsMachine::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnCancel();
}


BOOL CDlgSettingsMachine::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	BOOL bIsPowerOn = g_pBoard->GetIsActive();

	// TODO:  Add extra initialization here

	// NTSC / PAL
	m_cMachineNTSC.EnableWindow(!bIsPowerOn);
	m_cMachineNTSC.SetCheck(BST_UNCHECKED);
	m_cMachinePAL.EnableWindow(!bIsPowerOn);
	m_cMachinePAL.SetCheck(BST_UNCHECKED);

	if (g_pBoard->m_bPALMode == TRUE)
		m_cMachinePAL.SetCheck(BST_CHECKED);
	else
		m_cMachineNTSC.SetCheck(BST_CHECKED);

	m_cMachineA2p.EnableWindow(!bIsPowerOn);
	m_cMachineA2p.SetCheck(BST_UNCHECKED);
	m_cMachineA2e.EnableWindow(!bIsPowerOn);
	m_cMachineA2e.SetCheck(BST_UNCHECKED);
	switch (g_pBoard->m_nMachineType)
	{
	case MACHINE_APPLE2P:
		m_cMachineA2p.SetCheck(BST_CHECKED);
		break;
	case MACHINE_APPLE2E:
	default:
		m_cMachineA2e.SetCheck(BST_CHECKED);
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CDlgSettingsMachine::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
