// DlgConfigMockingBoard.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigmockingboard.h"
#include "appleclock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAppleClock* g_pBoard;

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigMockingBoard dialog


CDlgConfigMockingBoard::CDlgConfigMockingBoard(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfigMockingBoard::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgConfigMockingBoard)
	m_bMute = FALSE;
	//}}AFX_DATA_INIT
	m_nDipSwitch = PM_MB;
}


void CDlgConfigMockingBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfigMockingBoard)
	DDX_Control(pDX, IDC_PHASOR_MOCKING, m_btnMockingBoard);
	DDX_Control(pDX, IDC_PHASOR_NATIVE, m_btnPhasorNative);
	DDX_Control(pDX, IDC_PHASOR_VOLUME, m_slVolume);
	DDX_Control(pDX, IDC_PHASOR_BALANCE, m_slBalance);
	DDX_Check(pDX, IDC_PHASOR_MUTE, m_bMute);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgConfigMockingBoard, CDialog)
	//{{AFX_MSG_MAP(CDlgConfigMockingBoard)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigMockingBoard message handlers

void CDlgConfigMockingBoard::SetLeftVol(int vol)
{
	if ( vol < 0 )
		vol = 0;
	else if ( vol > 31 )
		vol = 31;
	m_uLeftVol = vol;
}

void CDlgConfigMockingBoard::SetRightVol(int vol)
{
	if ( vol < 0 )
		vol = 0;
	else if ( vol > 31 )
		vol = 31;
	m_uRightVol = vol;
}

int CDlgConfigMockingBoard::GetLeftVol()
{
	return m_uLeftVol;
}

int CDlgConfigMockingBoard::GetRightVol()
{
	return m_uRightVol;
}

BOOL CDlgConfigMockingBoard::OnInitDialog() 
{
	BOOL bPowerOn;

	CDialog::OnInitDialog();
	int nVol, nBal;
	if ( m_uRightVol > m_uLeftVol )
	{
		nVol = m_uRightVol;
		nBal = 6 - m_uLeftVol * 6 / m_uRightVol;
	}
	else
	{
		nVol = m_uLeftVol;
		if ( m_uLeftVol == 0 )
			nBal = 0;
		else
			nBal = m_uRightVol * 6 / m_uLeftVol - 6;
	}
	// 실질적으로 volume은 5~27 사이만 설정할 수 있도록 한다.
	// 5 는 volume 0으로 리턴한다.
	if ( nVol > 27 )
		nVol = 27;
	else if ( nVol < 5 )
		nVol = 5;
	// TODO: Add extra initialization here
	m_slBalance.SetRange( -6, 6, FALSE );	// 13 단계
	m_slBalance.SetTicFreq( 6 );
	m_slBalance.SetPageSize( 2 );
	m_slBalance.SetPos( nBal );
	m_slVolume.SetRange( -27, -5, FALSE );	// 23 단계
	m_slVolume.SetTicFreq( 2 );
	m_slVolume.SetPageSize( 4 );
	m_slVolume.SetPos( -nVol );

	// emulate mode 설정
	if ( m_nDipSwitch == PM_MB )
		m_btnMockingBoard.SetCheck( BST_CHECKED );
	else
		m_btnPhasorNative.SetCheck( BST_CHECKED );

	// power 가 켜져 있는 동안에 emulate mode를 변경할 수가 없다.
	bPowerOn = g_pBoard->GetIsActive();

	m_btnMockingBoard.EnableWindow( !bPowerOn );
	m_btnPhasorNative.EnableWindow( !bPowerOn );
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigMockingBoard::OnOK() 
{
	// TODO: Add extra validation here
	int nVol, nBal;
	nBal = m_slBalance.GetPos();
	nVol = -m_slVolume.GetPos();
	if ( nVol == 5 )
	{
		m_uLeftVol = 0;
		m_uRightVol = 0;
	}
	else
	{
		if ( nBal > 0 )		// right volume is large
		{
			m_uRightVol = nVol;
			m_uLeftVol = ( 6 - nBal ) * nVol / 6;
		}
		else
		{
			m_uLeftVol = nVol;
			m_uRightVol = ( 6 + nBal ) * nVol / 6;
		}
	}
	if ( m_btnMockingBoard.GetCheck() == BST_CHECKED )
		m_nDipSwitch = PM_MB;
	else
		m_nDipSwitch = PM_NATIVE;
	CDialog::OnOK();
}

void CDlgConfigMockingBoard::SetDipSwitch(int nDipSwitch)
{
	m_nDipSwitch = nDipSwitch;
}

int CDlgConfigMockingBoard::GetDipSwitch()
{
	return m_nDipSwitch;
}
