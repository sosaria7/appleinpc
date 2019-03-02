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
	, m_bArrowAsPaddle(FALSE)
	, m_bSwapButtons(FALSE)
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
	DDX_Control(pDX, IDC_JOYDEAD, m_cJoyDead);
	DDX_Control(pDX, IDC_JOYSAT, m_cJoySat);
	DDX_Check(pDX, IDC_ARRAW_AS_PADDLE, m_bArrowAsPaddle);
	DDX_Check(pDX, IDC_SWAP_BUTTON_1_2, m_bSwapButtons);
}


BEGIN_MESSAGE_MAP(CDlgSettingsInput, CDialogEx)

END_MESSAGE_MAP()


// CDlgSettingsInput message handlers


void CDlgSettingsInput::OnOK()
{
	int nSelect;

	UpdateData(TRUE);

	// keyboard
	g_cDIKeyboard.SetDelayTime(m_cKeyRepeat.GetPos(), m_cKeyDelay.GetPos());

	// joystick
	nSelect = m_cJoyDead.GetCurSel();
	g_pBoard->m_joystick.SetDeadZone(m_anDeadZoneList[nSelect]);
	nSelect = m_cJoySat.GetCurSel();
	g_pBoard->m_joystick.SetSaturation(m_anSaturationList[nSelect]);
	g_pBoard->m_joystick.SetArrayAsPaddle(m_bArrowAsPaddle != 0);
	g_pBoard->m_joystick.SetSwapButtons(m_bSwapButtons != 0);

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

	m_bSwapButtons = g_pBoard->m_joystick.GetSwapButtons();
	m_bArrowAsPaddle = g_pBoard->m_joystick.GetArrowAsPaddle();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
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
