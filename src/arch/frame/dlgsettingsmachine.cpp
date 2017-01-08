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
	, m_bSaveStateOnExit(FALSE)
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
	DDX_Control(pDX, IDC_STATE_FILENAME, m_cSateFilename);
	DDX_Check(pDX, IDC_SAVE_STATE_ON_EXIT, m_bSaveStateOnExit);
}


BEGIN_MESSAGE_MAP(CDlgSettingsMachine, CDialogEx)
	ON_BN_CLICKED(IDC_STATE_LOAD, &CDlgSettingsMachine::OnClickedStateLoad)
	ON_BN_CLICKED(IDC_STATE_SAVE, &CDlgSettingsMachine::OnClickedStateSave)
	ON_BN_CLICKED(IDC_STATE_BROWSE, &CDlgSettingsMachine::OnClickedStateBrowse)
	ON_EN_SETFOCUS(IDC_STATE_FILENAME, &CDlgSettingsMachine::OnSetfocusStateFilename)
	ON_EN_KILLFOCUS(IDC_STATE_FILENAME, &CDlgSettingsMachine::OnKillfocusStateFilename)
END_MESSAGE_MAP()


// CDlgSettingsMachine message handlers


void CDlgSettingsMachine::OnOK()
{
	UpdateData(TRUE);

	BOOL bPalMode = m_cMachinePAL.GetCheck();
	int nMachineType;

	if (m_cMachineA2p.GetCheck() == BST_CHECKED)
		nMachineType = MACHINE_APPLE2P;
	else
		nMachineType = MACHINE_APPLE2E;

	g_pBoard->SetMachineType(nMachineType, bPalMode);
	g_pBoard->SetStateFilePath(m_strStateFileName, m_bSaveStateOnExit);

	CDialogEx::OnOK();
}

void CDlgSettingsMachine::OnCancel()
{
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
	m_strStateFileName = g_pBoard->GetStateFilePath();
	SetFileName(&m_cSateFilename, m_strStateFileName);

	m_bSaveStateOnExit = g_pBoard->GetSaveStateOnExit();

	UpdateData(FALSE);

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


void CDlgSettingsMachine::OnClickedStateLoad()
{
	if (!m_strStateFileName.IsEmpty())
	{
		int nRet = ::AfxGetMainWnd()->MessageBox(TEXT("Do you want to load state?"), TEXT("aipc information"), MB_ICONQUESTION |MB_YESNO);
		if (nRet == IDYES)
		{
			g_pBoard->ReserveLoadState(m_strStateFileName);
			GetParent()->PostMessage(WM_CLOSE, 0, 0);
		}
	}
}


void CDlgSettingsMachine::OnClickedStateSave()
{
	if (!m_strStateFileName.IsEmpty())
	{
		UpdateData(TRUE);

		CString strOrgFileName = g_pBoard->GetStateFilePath();
		BOOL bSaveStateOnExit = g_pBoard->GetSaveStateOnExit();

		g_pBoard->SetStateFilePath(m_strStateFileName, m_bSaveStateOnExit);
		g_pBoard->SaveState(m_strStateFileName);
		g_pBoard->SetStateFilePath(strOrgFileName, bSaveStateOnExit);
	}
}


void CDlgSettingsMachine::OnClickedStateBrowse()
{
	CString fileName;
	TCHAR buffer[4096] = TEXT("");
	TCHAR** lppPart = { NULL };

	if (m_strStateFileName.IsEmpty())
		fileName = CString(TEXT("state1.dat"));
	else
		fileName = m_strStateFileName;

	if (GetFullPathName(fileName, 4096, buffer, lppPart) != 0)
		fileName = buffer;
	else
		fileName = m_strStateFileName;

	CFileDialog dlgFile(TRUE, TEXT("dat"), fileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "State Files (*.dat)|*.dat|All Files (*.*)|*.*||");

	if (dlgFile.DoModal() == IDOK)
	{
		m_strStateFileName = dlgFile.GetPathName();
		SetFileName(&m_cSateFilename, m_strStateFileName);
	}
}

void CDlgSettingsMachine::SetFileName(CEdit *edit, CString path)
{
	edit->SetWindowText(path.Mid(path.ReverseFind('\\') + 1));
}

void CDlgSettingsMachine::OnSetfocusStateFilename()
{
	m_cSateFilename.SetWindowText(m_strStateFileName);
}


void CDlgSettingsMachine::OnKillfocusStateFilename()
{
	m_cSateFilename.GetWindowText(m_strStateFileName);
	SetFileName(&m_cSateFilename, m_strStateFileName);
}
