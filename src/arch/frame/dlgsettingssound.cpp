// F:\night_project\appleinpc\src\arch\frame\dlgsettingssound.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettingssound.h"
#include "arch/directx/dxsound.h"
#include "appleclock.h"

// CDlgSettingsSound dialog

IMPLEMENT_DYNAMIC(CDlgSettingsSound, CDialogEx)

CDlgSettingsSound::CDlgSettingsSound(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_SOUND, pParent)
	, m_bMute(FALSE)
	, m_bSpeakerMute(FALSE)
{

}

CDlgSettingsSound::~CDlgSettingsSound()
{
}

void CDlgSettingsSound::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MASTER_BALANCE, m_cBalance);
	DDX_Control(pDX, IDC_MASTER_VOLUME, m_cVolume);
	DDX_Control(pDX, IDC_SPEAKER_VOLUME, m_cSpeakerVolume);
	DDX_Check(pDX, IDC_MASTER_MUTE, m_bMute);
	DDX_Check(pDX, IDC_SPEAKER_MUTE, m_bSpeakerMute);
}


BEGIN_MESSAGE_MAP(CDlgSettingsSound, CDialogEx)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDlgSettingsSound message handlers


HBRUSH CDlgSettingsSound::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here
	if (m_cBalance.m_hWnd == pWnd->m_hWnd ||
		m_cVolume.m_hWnd == pWnd->m_hWnd ||
		m_cSpeakerVolume.m_hWnd == pWnd->m_hWnd )
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);
	}
	// TODO:  Return a different brush if the default is not desired
	return hbr;
}


void CDlgSettingsSound::OnOK()
{
	int nVol;

	UpdateData(TRUE);

	// master volume
	nVol = m_cVolume.GetPos();
	if (nVol == 0)
		g_DXSound.SetVolume(0);
	else
		g_DXSound.SetVolume(5 - nVol);
	g_DXSound.SetPan(m_cBalance.GetPos());
	g_DXSound.m_bMute = m_bMute;
	nVol = m_cSpeakerVolume.GetPos();
	if (nVol == 0)
		g_pBoard->m_cSpeaker.SetVolume(0);
	else
		g_pBoard->m_cSpeaker.SetVolume(5 - nVol);	// nVol is -26 to 0
	g_pBoard->m_cSpeaker.m_bMute = m_bSpeakerMute;

	CDialogEx::OnOK();
}

void CDlgSettingsSound::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialogEx::OnCancel();
}


BOOL CDlgSettingsSound::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int nPan;
	int nVol;

	// Master Volume
	m_cVolume.SetRange(-26, 0, FALSE);
	m_cVolume.SetTicFreq(2);
	m_cVolume.SetPageSize(4);
	m_cBalance.SetRange(-6, 6, FALSE);
	m_cBalance.SetTicFreq(3);
	m_cBalance.SetPageSize(3);

	nPan = g_DXSound.GetPan();
	m_cBalance.SetPos(nPan);
	m_bMute = g_DXSound.m_bMute;

	nVol = g_DXSound.GetVolume();
	if (nVol <= 5)
		m_cVolume.SetPos(0);
	else
		m_cVolume.SetPos(5 - nVol);

	// Speaker Volume
	m_cSpeakerVolume.SetRange(-26, 0, FALSE);
	m_cSpeakerVolume.SetTicFreq(2);
	m_cSpeakerVolume.SetPageSize(2);

	nVol = g_pBoard->m_cSpeaker.m_iVol;
	if (nVol <= 5)
		m_cSpeakerVolume.SetPos(0);
	else
		m_cSpeakerVolume.SetPos(5 - nVol);
	m_bSpeakerMute = g_pBoard->m_cSpeaker.m_bMute;

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}



BOOL CDlgSettingsSound::PreTranslateMessage(MSG* pMsg)
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
