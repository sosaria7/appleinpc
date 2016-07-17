// F:\night_project\appleinpc\src\arch\frame\dlgsettingsinput.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettingsinput.h"
#include "arch/directx/dikeyboard.h"
#include "appleclock.h"

extern CDIKeyboard g_cDIKeyboard;

const int CDlgSettingsInput::m_anDeadZoneList[] = { 0, 10, 20 };
const int CDlgSettingsInput::m_anSaturationList[] = { 100, 95, 85, 70, 50 };

// CDlgSettingsInput dialog

IMPLEMENT_DYNAMIC(CDlgSettingsInput, CDialogEx)

CDlgSettingsInput::CDlgSettingsInput(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_INPUT, pParent)
{

}

CDlgSettingsInput::~CDlgSettingsInput()
{
}

void CDlgSettingsInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_SliderButtonCtrl(pDX, IDC_KEYBOARD_DELAY, m_cKeyDelay, 0);
	DDX_SliderButtonCtrl(pDX, IDC_KEYBOARD_REPEAT, m_cKeyRepeat, 0);
	DDX_Control(pDX, IDC_JOYSTICK_NONE, m_acJoystick[0]);
	DDX_Control(pDX, IDC_JOYSTICK_NUMPAD, m_acJoystick[1]);
	DDX_Control(pDX, IDC_JOYSTICK_PCJOYSTICK, m_acJoystick[2]);
	DDX_Control(pDX, IDC_JOYDEAD, m_cJoyDead);
	DDX_Control(pDX, IDC_JOYSAT, m_cJoySat);
}


BEGIN_MESSAGE_MAP(CDlgSettingsInput, CDialogEx)
	ON_BN_CLICKED(IDC_JOYSTICK_NONE, OnJoystickChanged)
	ON_BN_CLICKED(IDC_JOYSTICK_NUMPAD, OnJoystickChanged)
	ON_BN_CLICKED(IDC_JOYSTICK_PCJOYSTICK, OnJoystickChanged)
END_MESSAGE_MAP()


// CDlgSettingsInput message handlers


void CDlgSettingsInput::OnOK()
{
	int i;
	int nSelect;

	// keyboard
	g_cDIKeyboard.SetDelayTime(m_cKeyRepeat.GetPos(), m_cKeyDelay.GetPos());

	// joystick
	for (i = 0; i < 3; i++)
	{
		if (m_acJoystick[i].GetCheck() == BST_CHECKED)
		{
			g_pBoard->m_joystick.ChangeDevice(i);
			break;
		}
	}
	nSelect = m_cJoyDead.GetCurSel();
	g_pBoard->m_joystick.SetDeadZone(m_anDeadZoneList[nSelect]);
	nSelect = m_cJoySat.GetCurSel();
	g_pBoard->m_joystick.SetSaturation(m_anSaturationList[nSelect]);

	CDialogEx::OnOK();
}

void CDlgSettingsInput::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnCancel();
}


BOOL CDlgSettingsInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	int nKeyRepeat;
	int nKeyDelay;
	int i;

	// keyboard
	g_cDIKeyboard.GetDelayTime(&nKeyRepeat, &nKeyDelay);
	m_cKeyRepeat.SetRange(nKeyRepeat, 15, 100);
	m_cKeyDelay.SetRange(nKeyDelay, 250, 1000);

	// Joystick
	i = g_pBoard->m_joystick.GetDevice();
	if (!g_pBoard->m_joystick.m_bHasPCJoystick)
	{
		if (i == JM_PCJOYSTICK)
			i = JM_KEYPAD;
		m_acJoystick[JM_PCJOYSTICK].EnableWindow(FALSE);
	}
	m_acJoystick[i].SetCheck(BST_CHECKED);

	CString strValue;
	int nSelect = 0;
	int nCheckValue = g_pBoard->m_joystick.GetDeadZone();
	for (i = 0; i < sizeof(m_anDeadZoneList) / sizeof(m_anDeadZoneList[0]); i++)
	{
		strValue.Format(TEXT("%d"), m_anDeadZoneList[i]);
		m_cJoyDead.AddString(strValue);
		if (m_anDeadZoneList[i] == nCheckValue)
		{
			nSelect = i;
		}
	}
	m_cJoyDead.SetCurSel(nSelect);

	nSelect = 0;
	nCheckValue = g_pBoard->m_joystick.GetSaturation();
	for (i = 0; i < sizeof(m_anSaturationList) / sizeof(m_anSaturationList[0]); i++)
	{
		strValue.Format(TEXT("%d"), m_anSaturationList[i]);
		m_cJoySat.AddString(strValue);
		if (m_anSaturationList[i] == nCheckValue)
		{
			nSelect = i;
		}
	}
	m_cJoySat.SetCurSel(nSelect);
	OnJoystickChanged();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSettingsInput::OnJoystickChanged()
{
	BOOL bEnableDeadZone;
	// TODO: Add your control notification handler code here
	bEnableDeadZone = (m_acJoystick[JM_PCJOYSTICK].GetCheck() == BST_CHECKED);

	m_cJoyDead.EnableWindow(bEnableDeadZone);
	m_cJoySat.EnableWindow(bEnableDeadZone);
}


BOOL CDlgSettingsInput::PreTranslateMessage(MSG* pMsg)
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
