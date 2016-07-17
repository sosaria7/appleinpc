// F:\night_project\appleinpc\src\arch\frame\dlgsettingscolor.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "afxdialogex.h"
#include "aipcdefs.h"
#include "dlgsettingscolor.h"
#include "appleclock.h"

// CDlgSettingsColor dialog

IMPLEMENT_DYNAMIC(CDlgSettingsColor, CDialogEx)

CDlgSettingsColor::CDlgSettingsColor(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_COLOR, pParent)
{
	m_bPreview = FALSE;
	m_bScanline = FALSE;
}

CDlgSettingsColor::~CDlgSettingsColor()
{
}

void CDlgSettingsColor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_PREVIEW, m_bPreview);
	DDX_Check(pDX, IDC_SCANLINE, m_bScanline);
	DDX_SliderButtonCtrl(pDX, IDC_VIDEO_S, m_cSaturation, 0);
	DDX_SliderButtonCtrl(pDX, IDC_VIDEO_B, m_cBright, 0);
	DDX_SliderButtonCtrl(pDX, IDC_VIDEO_H, m_cHue, 0);
	DDX_Control(pDX, IDC_COLOR0, m_acColorBtn[0]);
	DDX_Control(pDX, IDC_COLOR1, m_acColorBtn[1]);
	DDX_Control(pDX, IDC_COLOR2, m_acColorBtn[2]);
	DDX_Control(pDX, IDC_COLOR3, m_acColorBtn[3]);
	DDX_Control(pDX, IDC_COLOR4, m_acColorBtn[4]);
	DDX_Control(pDX, IDC_COLOR5, m_acColorBtn[5]);
	DDX_Control(pDX, IDC_COLOR6, m_acColorBtn[6]);
	DDX_Control(pDX, IDC_COLOR7, m_acColorBtn[7]);
	DDX_Control(pDX, IDC_COLOR8, m_acColorBtn[8]);
	DDX_Control(pDX, IDC_COLOR9, m_acColorBtn[9]);
	DDX_Control(pDX, IDC_COLOR10, m_acColorBtn[10]);
	DDX_Control(pDX, IDC_COLOR11, m_acColorBtn[11]);
	DDX_Control(pDX, IDC_COLOR12, m_acColorBtn[12]);
	DDX_Control(pDX, IDC_COLOR13, m_acColorBtn[13]);
	DDX_Control(pDX, IDC_COLOR14, m_acColorBtn[14]);
	DDX_Control(pDX, IDC_COLOR15, m_acColorBtn[15]);
}


BEGIN_MESSAGE_MAP(CDlgSettingsColor, CDialogEx)
	ON_BN_CLICKED(IDC_PREVIEW, OnClickedPreview)
	ON_BN_CLICKED(IDC_SCANLINE, OnClickedScanline)
	ON_BN_CLICKED(IDC_DEFAULT, OnClickedDefault)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_COLOR_MODE0, IDC_COLOR_MODE3, OnChangeColorMode)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_COLOR0, IDC_COLOR15, OnColorClicked)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_VIDEO_H, IDC_VIDEO_B, OnChangeHSB)
END_MESSAGE_MAP()


// CDlgSettingsColor message handlers

void CDlgSettingsColor::ApplyColor()
{
	int i;
	unsigned int* puColor;
	COLORREF crColor;
	switch (m_nMode)
	{
	case 0:		// color
		puColor = g_pBoard->m_pScreen->GetColorsByHSB();
		for (i = 0; i < 16; i++)
		{
			crColor = (COLORREF)((puColor[i] >> 16) | (puColor[i] & 0xFF00)
				| ((puColor[i] << 16) & 0xFF0000));
			m_acColorBtn[i].SetColor(crColor, 0x1F);
			m_acColorBtn[i].EnableWindow(FALSE);
		}
		break;
	case 1:		// custom color
		puColor = g_pBoard->m_pScreen->GetColors();
		for (i = 0; i < 16; i++)
		{
			crColor = (COLORREF)((puColor[i] >> 16) | (puColor[i] & 0xFF00)
				| ((puColor[i] << 16) & 0xFF0000));
			m_acColorBtn[i].SetColor(crColor, 0x1F);
			m_acColorBtn[i].EnableWindow(TRUE);
		}
		break;

	case 2:		// white / mono
		crColor = g_pBoard->m_pScreen->GetMonoColor();
		crColor = (COLORREF)((crColor >> 16) | (crColor & 0xFF00)
			| ((crColor << 16) & 0xFF0000));
		for (i = 0; i < 16; i++)
		{
			m_acColorBtn[i].SetColor(crColor, i | 0x10);
			m_acColorBtn[i].EnableWindow(TRUE);
		}
		break;

	case 3:		// green
		crColor = g_pBoard->m_pScreen->GetGreenColor();
		crColor = (COLORREF)((crColor >> 16) | (crColor & 0xFF00)
			| ((crColor << 16) & 0xFF0000));
		for (i = 0; i < 16; i++)
		{
			m_acColorBtn[i].SetColor(crColor, i);
			m_acColorBtn[i].EnableWindow(TRUE);
		}
		break;
	}
}


void CDlgSettingsColor::ResetScreen()
{
	int i;
	unsigned int* puColors;
	unsigned int auColors[16];
	unsigned int uMono;
	unsigned int uGreen;
	unsigned int uHSB;
	// save current setting
	puColors = g_pBoard->m_pScreen->GetColors();
	for (i = 0; i < 16; i++)
		auColors[i] = puColors[i];
	uMono = g_pBoard->m_pScreen->GetMonoColor();
	uGreen = g_pBoard->m_pScreen->GetGreenColor();
	uHSB = g_pBoard->m_pScreen->GetHSB();
	// reset screen to before
	g_pBoard->m_pScreen->SetColors(m_auColors);
	g_pBoard->m_pScreen->SetMonoColor(m_uMono);
	g_pBoard->m_pScreen->SetGreenColor(m_uGreen);
	g_pBoard->m_pScreen->SetHSB(m_uHSB);
	g_pBoard->m_pScreen->ApplyColors();
	// restore current setting
	g_pBoard->m_pScreen->SetColors(auColors);
	g_pBoard->m_pScreen->SetMonoColor(uMono);
	g_pBoard->m_pScreen->SetGreenColor(uGreen);
	g_pBoard->m_pScreen->SetHSB(uHSB);
}

void CDlgSettingsColor::OnOK()
{
	if (!m_bPreview)
	{
		g_pBoard->m_pScreen->ChangeMonitorType(m_nMode);
		g_pBoard->m_pScreen->ApplyColors();
	}
	g_pBoard->m_pScreen->m_bPreview = m_bPreview;

	CDialogEx::OnOK();
}

void CDlgSettingsColor::OnCancel()
{
	if (m_bPreview)
	{
		g_pBoard->m_pScreen->SetScanline(m_bOrgScanline);
		g_pBoard->m_pScreen->SetColors(m_auColors);
		g_pBoard->m_pScreen->SetMonoColor(m_uMono);
		g_pBoard->m_pScreen->SetGreenColor(m_uGreen);
		g_pBoard->m_pScreen->SetHSB(m_uHSB);
		g_pBoard->m_pScreen->ChangeMonitorType(m_nOrgMode);
		g_pBoard->m_pScreen->ApplyColors();
	}

	CDialogEx::OnCancel();
}


BOOL CDlgSettingsColor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	unsigned int* puColor;
	int i;
	int nHue, nSaturation, nBright;
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	m_nOrgMode = m_nMode = g_pBoard->m_pScreen->GetMonitorType();
	CheckRadioButton(IDC_COLOR_MODE0, IDC_COLOR_MODE3, IDC_COLOR_MODE0 + m_nMode);

	ApplyColor();
	// remember original colors for cancel
	puColor = g_pBoard->m_pScreen->GetColors();
	for (i = 0; i < 16; i++)
		m_auColors[i] = puColor[i];
	m_uMono = g_pBoard->m_pScreen->GetMonoColor();
	m_uGreen = g_pBoard->m_pScreen->GetGreenColor();
	m_uHSB = g_pBoard->m_pScreen->GetHSB();
	m_bPreview = g_pBoard->m_pScreen->m_bPreview;
	m_bScanline = g_pBoard->m_pScreen->GetScanline();
	m_bOrgScanline = m_bScanline;

	nHue = (m_uHSB >> 16) % 360;
	nSaturation = (m_uHSB >> 8) & 0xFF;
	nBright = m_uHSB & 0xFF;
	if (nHue > 180)
		nHue -= 360;
	if (nHue > 90)
		nHue = 90;
	else if (nHue < -90)
		nHue = -90;
	m_cHue.SetRange(nHue, -90, 90);
	m_cSaturation.SetRange(nSaturation, 0, 255);
	m_cBright.SetRange(nBright, 0, 255);
	if (m_nMode == 0)
	{
		m_cHue.EnableWindow(TRUE);
		m_cSaturation.EnableWindow(TRUE);
		m_cBright.EnableWindow(TRUE);
	}
	else
	{
		m_cHue.EnableWindow(FALSE);
		m_cSaturation.EnableWindow(FALSE);
		m_cBright.EnableWindow(FALSE);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgSettingsColor::OnClickedPreview()
{
	UpdateData(TRUE);
	if (m_bPreview)
	{
		g_pBoard->m_pScreen->ApplyColors();
	}
	else
	{
		ResetScreen();
	}
}


void CDlgSettingsColor::OnClickedScanline()
{
	UpdateData(TRUE);

	g_pBoard->m_pScreen->SetScanline(m_bScanline);

	if (m_bPreview)
		g_pBoard->m_pScreen->ApplyColors();
}


void CDlgSettingsColor::OnClickedDefault()
{
	unsigned int uHSB;
	int nHue, nSaturation, nBright;

	g_pBoard->m_pScreen->SetDefaultColors();

	uHSB = g_pBoard->m_pScreen->GetHSB();
	nHue = (uHSB >> 16) % 360;
	nSaturation = (uHSB >> 8) & 0xFF;
	nBright = uHSB & 0xFF;
	if (nHue > 180)
		nHue -= 360;
	if (nHue > 90)
		nHue = 90;
	else if (nHue < -90)
		nHue = -90;
	m_cHue.SetPos(nHue);
	m_cSaturation.SetPos(nSaturation);
	m_cBright.SetPos(nBright);

	ApplyColor();
	if (m_bPreview)
		g_pBoard->m_pScreen->ApplyColors();
}

void CDlgSettingsColor::OnChangeColorMode(UINT uId)
{
	int nMode;
	nMode = uId - IDC_COLOR_MODE0;
	if (nMode != m_nMode)
	{
		m_nMode = nMode;
		if (m_nMode == 0)
		{
			m_cHue.EnableWindow(TRUE);
			m_cSaturation.EnableWindow(TRUE);
			m_cBright.EnableWindow(TRUE);
		}
		else
		{
			m_cHue.EnableWindow(FALSE);
			m_cSaturation.EnableWindow(FALSE);
			m_cBright.EnableWindow(FALSE);
		}
		ApplyColor();
		if (m_bPreview)
			g_pBoard->m_pScreen->ApplyColors();
		g_pBoard->m_pScreen->ChangeMonitorType(m_nMode);
	}
}

void CDlgSettingsColor::OnColorClicked(UINT uId)
{
	unsigned int* puColor;
	COLORREF crColor;
	CColorDialog dlg;
	uId -= IDC_COLOR0;

	dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;

	switch (m_nMode)
	{
	case 1:		// custom color
		puColor = g_pBoard->m_pScreen->GetColors();
		crColor = puColor[uId];
		break;
	case 2:		// white / mono
		crColor = g_pBoard->m_pScreen->GetMonoColor();
		break;
	case 3:		// green
		crColor = g_pBoard->m_pScreen->GetGreenColor();
		break;
	default:
		return;
	}

	crColor = (COLORREF)(crColor >> 16) | (crColor & 0x00FF00)
		| ((crColor << 16) & 0xFF0000);
	dlg.m_cc.rgbResult = crColor;

	dlg.DoModal();
	crColor = dlg.GetColor();
	crColor = (COLORREF)(crColor >> 16) | (crColor & 0x00FF00)
		| ((crColor << 16) & 0xFF0000);
	switch (m_nMode)
	{
	case 1:
		puColor[uId] = crColor;
		g_pBoard->m_pScreen->SetColors(puColor);
		break;
	case 2:
		g_pBoard->m_pScreen->SetMonoColor(crColor);
		break;
	case 3:
		g_pBoard->m_pScreen->SetGreenColor(crColor);
		break;
	}
	ApplyColor();
	if (m_bPreview)
		g_pBoard->m_pScreen->ApplyColors();
}

void CDlgSettingsColor::OnChangeHSB(UINT uId)
{
	int nHue, nSaturation, nBright;

	nHue = m_cHue.GetPos();
	if (nHue < 0)
		nHue += 360;
	nSaturation = m_cSaturation.GetPos();
	nBright = m_cBright.GetPos();
	g_pBoard->m_pScreen->SetHSB(HSB(nHue, nSaturation, nBright));

	if (m_nMode == 0)
	{
		ApplyColor();

		if (m_bPreview)
			g_pBoard->m_pScreen->ApplyColors();
	}
}


BOOL CDlgSettingsColor::PreTranslateMessage(MSG* pMsg)
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
