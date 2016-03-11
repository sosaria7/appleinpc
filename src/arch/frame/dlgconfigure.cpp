// dlgconfigure.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigure.h"
#include "arch/frame/wcsliderbutton.h"
#include "arch/frame/dlgcolor.h"
#include "arch/directx/dxsound.h"
#include "arch/directx/dikeyboard.h"
#include "mousecard.h"
#include "diskinterface.h"
#include "phasor.h"
#include "appleclock.h"
#include "joystick.h"
#include "sddiskii.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAppleClock* g_pBoard;
extern CDIKeyboard g_cDIKeyboard;

const int CDlgConfigure::m_anDeadZoneList[] = { 0, 10, 20 };
const int CDlgConfigure::m_anSaturationList[] = { 100, 95, 85, 70, 50 };

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigure dialog


CDlgConfigure::CDlgConfigure(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfigure::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgConfigure)
	m_bMasterMute = FALSE;
	m_bSpeakerMute = FALSE;
	//}}AFX_DATA_INIT
}

CDlgConfigure::~CDlgConfigure()
{
	int i;
	for( i = 0; i < 7; i++ )
	{
		// 새로 생성했던 카드들 중에 삭제되지 않은 것이 있으면 삭제한다.
		if ( m_pCards[i] != NULL && m_pCards[i] != g_pBoard->m_cSlots.GetCard(i) )
		{
			delete m_pCards[i];
			m_pCards[i] = NULL;		// not necessary but...
		}
	}
}

void CDlgConfigure::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfigure)
	DDX_Control(pDX, IDC_JOYSAT, m_cJoySat);
	DDX_Control(pDX, IDC_JOYDEAD, m_cJoyDead);
	DDX_Control(pDX, IDC_SPEAKER_VOLUME, m_slSpeakerVolume);
	DDX_Control(pDX, IDC_MASTER_VOLUME, m_slMasterVolume);
	DDX_Control(pDX, IDC_MASTER_BALANCE, m_slMasterBalance);
	DDX_Check(pDX, IDC_MASTER_MUTE, m_bMasterMute);
	DDX_Check(pDX, IDC_SPEAKER_MUTE, m_bSpeakerMute);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_JOYSTICK_NONE, m_btnJoystick[0]);
	DDX_Control(pDX, IDC_JOYSTICK_NUMPAD, m_btnJoystick[1]);
	DDX_Control(pDX, IDC_JOYSTICK_PCJOYSTICK, m_btnJoystick[2]);
	DDX_Control(pDX, IDC_SLOT1, m_cbSlot[0]);
	DDX_Control(pDX, IDC_SLOT2, m_cbSlot[1]);
	DDX_Control(pDX, IDC_SLOT3, m_cbSlot[2]);
	DDX_Control(pDX, IDC_SLOT4, m_cbSlot[3]);
	DDX_Control(pDX, IDC_SLOT5, m_cbSlot[4]);
	DDX_Control(pDX, IDC_SLOT6, m_cbSlot[5]);
	DDX_Control(pDX, IDC_SLOT7, m_cbSlot[6]);
	DDX_Control(pDX, IDC_SLOT1_SETUP, m_btnSetupCard[0]);
	DDX_Control(pDX, IDC_SLOT2_SETUP, m_btnSetupCard[1]);
	DDX_Control(pDX, IDC_SLOT3_SETUP, m_btnSetupCard[2]);
	DDX_Control(pDX, IDC_SLOT4_SETUP, m_btnSetupCard[3]);
	DDX_Control(pDX, IDC_SLOT5_SETUP, m_btnSetupCard[4]);
	DDX_Control(pDX, IDC_SLOT6_SETUP, m_btnSetupCard[5]);
	DDX_Control(pDX, IDC_SLOT7_SETUP, m_btnSetupCard[6]);
	DDX_SliderButtonCtrl(pDX, IDC_KEYBOARD_DELAY, m_sbKeyDelay, 0);
	DDX_SliderButtonCtrl(pDX, IDC_KEYBOARD_REPEAT, m_sbKeyRepeat, 0);
	DDX_Control(pDX, IDC_MACHINE_NTSC, m_btnMachineNTSC);
	DDX_Control(pDX, IDC_MACHINE_PAL, m_btnMachinePAL);
	DDX_Control(pDX, IDC_MACHINE_2PLUS, m_btnMachineA2p);
	DDX_Control(pDX, IDC_MACHINE_2E, m_btnMachineA2e);
}


BEGIN_MESSAGE_MAP(CDlgConfigure, CDialog)
	//{{AFX_MSG_MAP(CDlgConfigure)
	ON_BN_CLICKED(IDC_COLOR_SETTING, OnColorSetting)
	ON_BN_CLICKED(IDC_JOYSTICK_NONE, OnJoystickChange)
	ON_BN_CLICKED(IDC_JOYSTICK_NUMPAD, OnJoystickChange)
	ON_BN_CLICKED(IDC_JOYSTICK_PCJOYSTICK, OnJoystickChange)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_SLOT1, IDC_SLOT7, OnSelchangeSlot)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_SLOT1_SETUP, IDC_SLOT7_SETUP, OnClickedSlotSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfigure message handlers

void CDlgConfigure::OnSelchangeSlot(UINT uId) 
{
	int nCurSel;
	CCard* pCard;
	if ( uId < IDC_SLOT1 || uId > IDC_SLOT7 )
		return;			// it must not occur
	uId -= IDC_SLOT1;
	nCurSel = m_cbSlot[uId].GetCurSel();
	if ( m_pCards[uId] == NULL || m_pCards[uId]->GetDeviceNum() != nCurSel )
	{
		pCard = g_pBoard->m_cSlots.GetCard(uId);
		if ( m_pCards[uId] != NULL )
		{
			// 새로 설치된 카드인 경우 삭제
			// rollback(cancel)할 경우를 대비해서 기존의 카드는 삭제하지 않는다.
			if ( m_pCards[uId] != pCard )
				delete m_pCards[uId];
			m_pCards[uId] = NULL;
		}
		// 기존에 있던 카드와 동일하면 기존의 카드를 사용한다.
		if ( pCard != NULL && pCard->GetDeviceNum() == nCurSel )
			m_pCards[uId] = pCard;
		else
		{
			switch( nCurSel )
			{
			case CARD_EMPTY:		// empty
				m_btnSetupCard[uId].EnableWindow(FALSE);
				return;
			case CARD_MOUSE_INTERFACE:		// mouse interface card
				m_pCards[uId] = new CMouseCard();
				break;
			case CARD_DISK_INTERFACE:		// apple disk ii interface card
				m_pCards[uId] = new CDiskInterface();
				break;
			case CARD_PHASOR:				// phasor/mocking board sound card
				m_pCards[uId] = new CPhasor();
				break;
			case CARD_SD_DISK_II:			// SD DISK][ interface card
				m_pCards[uId] = new CSDDiskII();
				break;
			default:
				return;	// it must not occur
			}
		}
	}
	m_btnSetupCard[uId].EnableWindow(TRUE);
}

void CDlgConfigure::OnClickedSlotSetup(UINT uId) 
{
	if ( uId < IDC_SLOT1_SETUP || uId > IDC_SLOT7_SETUP )
		return;			// it must not occur
	uId -= IDC_SLOT1_SETUP;
	// m_pCards[uId] 값이 NULL인 경우 Empty로 Disable되어 있어야 한다.
	if ( m_pCards[uId] == NULL )
	{
		m_btnSetupCard[uId].EnableWindow(FALSE);
		return;
	}
	m_pCards[uId]->Configure();
}

BOOL CDlgConfigure::OnInitDialog() 
{
	int i;
	int nKeyDelay, nKeyRepeat;
	BOOL bIsPowerOn;
	int nVol, nPan;
//	CEdit* pComboEdit;

	CDialog::OnInitDialog();

	bIsPowerOn = g_pBoard->GetIsActive();
	for( i = 0; i < 7; i++ )
	{
		m_pCards[i] = g_pBoard->m_cSlots.GetCard(i);
		if ( m_pCards[i] != NULL )
		{
			m_cbSlot[i].SetCurSel( m_pCards[i]->GetDeviceNum() );
			m_btnSetupCard[i].EnableWindow(TRUE);
		}
		else
		{
			m_cbSlot[i].SetCurSel(CARD_EMPTY);
			m_btnSetupCard[i].EnableWindow(FALSE);
		}
		m_cbSlot[i].EnableWindow( !bIsPowerOn );
		/*
		if ( bIsPowerOn )
		{
			pComboEdit = (CEdit*)(GetDlgItem( IDC_SLOT1 + i )->GetWindow(GW_CHILD ));
			pComboEdit->EnableWindow(TRUE);
			pComboEdit->SetReadOnly();
		}
		*/
	}
	// not implemented yet
	CheckRadioButton( IDC_MACHINE_2PLUS, IDC_MACHINE_2C, IDC_MACHINE_2E );

	// NTSC / PAL
	m_btnMachineNTSC.EnableWindow(!bIsPowerOn);
	m_btnMachineNTSC.SetCheck(BST_UNCHECKED);
	m_btnMachinePAL.EnableWindow(!bIsPowerOn);
	m_btnMachinePAL.SetCheck(BST_UNCHECKED);

	if (g_pBoard->m_bPALMode == TRUE)
		m_btnMachinePAL.SetCheck(BST_CHECKED);
	else
		m_btnMachineNTSC.SetCheck(BST_CHECKED);

	m_btnMachineA2p.EnableWindow(!bIsPowerOn);
	m_btnMachineA2p.SetCheck(BST_UNCHECKED);
	m_btnMachineA2e.EnableWindow(!bIsPowerOn);
	m_btnMachineA2e.SetCheck(BST_UNCHECKED);
	switch (g_pBoard->m_nMachineType)
	{
	case MACHINE_APPLE2P:
		m_btnMachineA2p.SetCheck(BST_CHECKED);
		break;
	case MACHINE_APPLE2E:
	default:
		m_btnMachineA2e.SetCheck(BST_CHECKED);
		break;
	}

	// keyboard
	g_cDIKeyboard.GetDelayTime( &nKeyRepeat, &nKeyDelay );
	m_sbKeyRepeat.SetRange( nKeyRepeat, 15, 100 );
	m_sbKeyDelay.SetRange(  nKeyDelay, 250, 1000 );


	// Joystick
	i = g_pBoard->m_joystick.GetDevice();
	if ( !g_pBoard->m_joystick.m_bHasPCJoystick )
	{
		if ( i == JM_PCJOYSTICK )
			i = JM_KEYPAD;
		m_btnJoystick[JM_PCJOYSTICK].EnableWindow(FALSE);
	}
	m_btnJoystick[i].SetCheck(BST_CHECKED);

	CString strValue;
	int nSelect = 0;
	int nCheckValue = g_pBoard->m_joystick.GetDeadZone();
	for( i = 0; i < sizeof(m_anDeadZoneList)/sizeof(m_anDeadZoneList[0]); i++ )
	{
		strValue.Format( TEXT("%d"), m_anDeadZoneList[i] );
		m_cJoyDead.AddString( strValue );
		if ( m_anDeadZoneList[i] == nCheckValue )
		{
			nSelect = i;
		}
	}
	m_cJoyDead.SetCurSel( nSelect );

	nSelect = 0;
	nCheckValue = g_pBoard->m_joystick.GetSaturation();
	for( i = 0; i < sizeof(m_anSaturationList)/sizeof(m_anSaturationList[0]); i++ )
	{
		strValue.Format( TEXT("%d"), m_anSaturationList[i] );
		m_cJoySat.AddString( strValue );
		if ( m_anSaturationList[i] == nCheckValue )
		{
			nSelect = i;
		}
	}
	m_cJoySat.SetCurSel( nSelect );
	OnJoystickChange();

	// Master Volume
	m_slMasterVolume.SetRange( -26, 0, FALSE );
	m_slMasterVolume.SetTicFreq( 2 );
	m_slMasterVolume.SetPageSize( 4 );
	m_slMasterBalance.SetRange( -6, 6, FALSE );
	m_slMasterBalance.SetTicFreq( 3 );
	m_slMasterBalance.SetPageSize( 3 );

	nPan = g_DXSound.GetPan();
	m_slMasterBalance.SetPos( nPan );
	m_bMasterMute = g_DXSound.m_bMute;

	nVol = g_DXSound.GetVolume();
	if ( nVol <= 5 )
		m_slMasterVolume.SetPos( 0 );
	else
		m_slMasterVolume.SetPos( 5 - nVol );

	m_slSpeakerVolume.SetRange( -26, 0, FALSE );
	m_slSpeakerVolume.SetTicFreq( 2 );
	m_slSpeakerVolume.SetPageSize( 2 );

	nVol = g_pBoard->m_cSpeaker.m_iVol;
	if ( nVol <= 5 )
		m_slSpeakerVolume.SetPos( 0 );
	else
		m_slSpeakerVolume.SetPos( 5 - nVol );
	m_bSpeakerMute = g_pBoard->m_cSpeaker.m_bMute;

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfigure::OnOK() 
{
	// TODO: Add extra validation here
	int i;
	int nVol;
	int nSelect;
	UpdateData(TRUE);

	for( i = 0; i < 7; i++ )
	{
		if ( m_pCards[i] != g_pBoard->m_cSlots.GetCard(i) )
		{
			g_pBoard->m_cSlots.RemoveCard(i);
			g_pBoard->m_cSlots.InsertCard(i, m_pCards[i]);
		}
	}
	g_pBoard->m_cSlots.Initialize();
	g_pBoard->m_pScreen->SetMouseCapture(g_pBoard->m_cSlots.HasMouseInterface());

	// keyboard
	g_cDIKeyboard.SetDelayTime( m_sbKeyRepeat.GetPos(), m_sbKeyDelay.GetPos() );

	// joystick
	for ( i = 0; i < 3; i++ )
	{
		if ( m_btnJoystick[i].GetCheck() == BST_CHECKED )
		{
			g_pBoard->m_joystick.ChangeDevice( i );
			break;
		}
	}
	nSelect = m_cJoyDead.GetCurSel();
	g_pBoard->m_joystick.SetDeadZone( m_anDeadZoneList[nSelect] );
	nSelect = m_cJoySat.GetCurSel();
	g_pBoard->m_joystick.SetSaturation( m_anSaturationList[nSelect] );

	// master volume
	nVol = m_slMasterVolume.GetPos();
	if ( nVol == 0 )
		g_DXSound.SetVolume( 0 );
	else
		g_DXSound.SetVolume( 5 - nVol );
	g_DXSound.SetPan( m_slMasterBalance.GetPos() );
	g_DXSound.m_bMute = m_bMasterMute;
	nVol = m_slSpeakerVolume.GetPos();
	if ( nVol == 0 )
		g_pBoard->m_cSpeaker.SetVolume( 0 );
	else
		g_pBoard->m_cSpeaker.SetVolume( 5 - nVol );	// nVol is -26 to 0
	g_pBoard->m_cSpeaker.m_bMute = m_bSpeakerMute;

	BOOL bPalMode = m_btnMachinePAL.GetCheck();
	int nMachineType;

	if (m_btnMachineA2p.GetCheck() == BST_CHECKED)
		nMachineType = MACHINE_APPLE2P;
	else
		nMachineType = MACHINE_APPLE2E;

	g_pBoard->SetMachineType(nMachineType, bPalMode);

	CDialog::OnOK();
}

void CDlgConfigure::OnCancel() 
{
	// TODO: Add extra cleanup here
	int i;
	for( i = 0; i < 7; i++ )
	{
		// 새로 생성했던 카드들 중에 삭제되지 않은 것이 있으면 삭제한다.
		if ( m_pCards[i] != NULL && m_pCards[i] != g_pBoard->m_cSlots.GetCard(i) )
		{
			delete m_pCards[i];
			m_pCards[i] = NULL;		// not necessary but...
		}
	}
	
	CDialog::OnCancel();
}


void CDlgConfigure::OnColorSetting() 
{
	// TODO: Add your control notification handler code here
	CDlgColor dlgColor;
	dlgColor.DoModal();
}

void CDlgConfigure::OnJoystickChange() 
{
	BOOL bEnableDeadZone;
	// TODO: Add your control notification handler code here
	bEnableDeadZone = ( m_btnJoystick[JM_PCJOYSTICK].GetCheck() == BST_CHECKED );

	m_cJoyDead.EnableWindow( bEnableDeadZone );
	m_cJoySat.EnableWindow( bEnableDeadZone );
}
