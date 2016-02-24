// SlotManager.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/slotmanager.h"
#include "arch/frame/dlgconfigsound.h"
#include "arch/frame/dlgconfigmonitor.h"
#include "appleclock.h"
#include "joystick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSlotManager dialog


CSlotManager::CSlotManager( CWnd* pParent /*=NULL*/ )
	: CDialog(CSlotManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSlotManager)
	//}}AFX_DATA_INIT
	m_bSlotChanged = FALSE;
}

CSlotManager::~CSlotManager()
{

}


void CSlotManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSlotManager)
	DDX_Control(pDX, IDC_JOYSTICK, m_cbJoystick);
	DDX_Control(pDX, IDC_BTN_SLOT7, m_btnSlots7);
	DDX_Control(pDX, IDC_BTN_SLOT6, m_btnSlots6);
	DDX_Control(pDX, IDC_BTN_SLOT5, m_btnSlots5);
	DDX_Control(pDX, IDC_BTN_SLOT4, m_btnSlots4);
	DDX_Control(pDX, IDC_BTN_SLOT3, m_btnSlots3);
	DDX_Control(pDX, IDC_BTN_SLOT2, m_btnSlots2);
	DDX_Control(pDX, IDC_BTN_SLOT1, m_btnSlots1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSlotManager, CDialog)
	//{{AFX_MSG_MAP(CSlotManager)
	ON_BN_CLICKED(IDC_BTN_SLOT1, OnBtnSlot1)
	ON_BN_CLICKED(IDC_BTN_SLOT2, OnBtnSlot2)
	ON_BN_CLICKED(IDC_BTN_SLOT3, OnBtnSlot3)
	ON_BN_CLICKED(IDC_BTN_SLOT4, OnBtnSlot4)
	ON_BN_CLICKED(IDC_BTN_SLOT5, OnBtnSlot5)
	ON_BN_CLICKED(IDC_BTN_SLOT6, OnBtnSlot6)
	ON_BN_CLICKED(IDC_BTN_SLOT7, OnBtnSlot7)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT1, OnBtnInsertSlot1)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT2, OnBtnInsertSlot2)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT3, OnBtnInsertSlot3)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT4, OnBtnInsertSlot4)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT5, OnBtnInsertSlot5)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT6, OnBtnInsertSlot6)
	ON_BN_CLICKED(IDC_BTN_INSERTSLOT7, OnBtnInsertSlot7)
	ON_COMMAND(ID_SLOT_NONE, OnSlotNone)
	ON_COMMAND(ID_SLOT_DISK, OnSlotDisk)
	ON_COMMAND(ID_SLOT_MOCKINGBOARD, OnSlotMockingBoard)
	ON_COMMAND(ID_SLOT_PHASOR, OnSlotPhasor)
	ON_COMMAND(ID_SLOT_MOUSE, OnSlotMouse)
	ON_BN_CLICKED(IDC_BTN_SOUND, OnBtnSound)
	ON_BN_CLICKED(IDC_BTN_MONITOR, OnBtnMonitor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlotManager message handlers

BOOL CSlotManager::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CButton* pBtnSlots[7] = {&m_btnSlots1, &m_btnSlots2, &m_btnSlots3, &m_btnSlots4, &m_btnSlots5, &m_btnSlots6, &m_btnSlots7};

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		CCard *pCard = g_pBoard->m_cSlots.GetCard( i );
		if ( pCard )
		{
			pBtnSlots[i]->SetWindowText( pCard->GetDeviceName() );
			pBtnSlots[i]->EnableWindow();
		}
	}
	if ( g_pBoard->m_joystick.m_bHasPCJoystick )
		m_cbJoystick.SetCurSel( g_pBoard->m_joystick.GetDevice() );
	else
	{
		i = g_pBoard->m_joystick.GetDevice();
		if ( i == JM_PCJOYSTICK )
			i = JM_KEYPAD;
		m_cbJoystick.SetCurSel( i );
		m_cbJoystick.DeleteString( JM_PCJOYSTICK );
	}

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSlotManager::OnBtnSlot1() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 0 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot2() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 1 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot3() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 2 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot4() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 3 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot5() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 4 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot6() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 5 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnSlot7() 
{
	CCard* pCard = g_pBoard->m_cSlots.GetCard( 6 );
	ASSERT( pCard );
	pCard->Configure();
}

void CSlotManager::OnBtnInsertSlot1() 
{
	m_nSlotToChange = 0;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot2() 
{
	m_nSlotToChange = 1;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot3() 
{
	m_nSlotToChange = 2;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot4() 
{
	m_nSlotToChange = 3;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot5() 
{
	m_nSlotToChange = 4;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot6() 
{
	m_nSlotToChange = 5;
	ShowPopMenu();
}

void CSlotManager::OnBtnInsertSlot7() 
{
	m_nSlotToChange = 6;
	ShowPopMenu();
}

void CSlotManager::OnSlotNone() 
{
	CButton* pBtnSlots[7] = {&m_btnSlots1, &m_btnSlots2, &m_btnSlots3, &m_btnSlots4, &m_btnSlots5, &m_btnSlots6, &m_btnSlots7};

	if ( !g_pBoard->m_cSlots.GetCard( m_nSlotToChange ) )
		return;

	RemoveCard( m_nSlotToChange );

	pBtnSlots[m_nSlotToChange]->SetWindowText("Empty");
	pBtnSlots[m_nSlotToChange]->EnableWindow(FALSE);
	m_bSlotChanged = TRUE;
}

void CSlotManager::OnSlotDisk() 
{
	InsertCard( DN_DISKINTERFACE );
}

void CSlotManager::OnSlotMockingBoard() 
{
	InsertCard( DN_PHASOR | ( PM_MB << 8 ) );	// Phasor in MockingBoard Emulation Mode
}

void CSlotManager::OnSlotPhasor()
{
	InsertCard( DN_PHASOR | ( PM_NATIVE << 8 ) );	// Phasor in Native Mode
}

void CSlotManager::OnSlotMouse()
{
	InsertCard( DN_MOUSE );	// Phasor in Native Mode
}

void CSlotManager::OnOK() 
{
	g_pBoard->m_joystick.ChangeDevice( m_cbJoystick.GetCurSel() );
	CDialog::OnOK();
}

void CSlotManager::InsertCard(int nDeviceNum)
{
	int nOption = nDeviceNum >> 8;
	int nDevice = nDeviceNum & 0xFF;

	int nOrigDevice;
	CButton* pBtnSlots[7] = {&m_btnSlots1, &m_btnSlots2, &m_btnSlots3, &m_btnSlots4, &m_btnSlots5, &m_btnSlots6, &m_btnSlots7};

	CCard *pCard;
	pCard = g_pBoard->m_cSlots.GetCard( m_nSlotToChange );
	if ( pCard )
	{
		nOrigDevice = pCard->GetDeviceNum();
		if ( nOrigDevice == nDevice )
		{
			if ( pCard->GetDipSwitch() != nOption )
			{
				m_bSlotChanged = pCard->SetDipSwitch( nOption );
				pBtnSlots[m_nSlotToChange]->SetWindowText(pCard->GetDeviceName());
			}
			return;
		}
	}

	RemoveCard( m_nSlotToChange );

	g_pBoard->m_cSlots.InsertCard( m_nSlotToChange, nDeviceNum );

	pCard = g_pBoard->m_cSlots.GetCard( m_nSlotToChange );

	if ( pCard )
	{
		pBtnSlots[m_nSlotToChange]->SetWindowText(pCard->GetDeviceName());
		pBtnSlots[m_nSlotToChange]->EnableWindow();
	}
	else
	{
		pBtnSlots[m_nSlotToChange]->SetWindowText("Empty");
		pBtnSlots[m_nSlotToChange]->EnableWindow(FALSE);
	}

	m_bSlotChanged = TRUE;
}

void CSlotManager::RemoveCard(int nSlot)
{
	g_pBoard->m_cSlots.RemoveCard( nSlot );
}

void CSlotManager::ShowPopMenu()
{
	CMenu menuSlot, *pPopMenu;
	CPoint ptMouse;

	CCard* pCard = g_pBoard->m_cSlots.GetCard( m_nSlotToChange );
	int nDeviceNum = DN_UNKNOWN;
	if ( pCard )
		nDeviceNum = pCard->GetDeviceNum();

	menuSlot.LoadMenu(IDR_CONFIG_SLOT);
	pPopMenu = menuSlot.GetSubMenu(0);

	if ( nDeviceNum == DN_DISKINTERFACE || !g_pBoard->m_cSlots.IsAvailable( DN_DISKINTERFACE ) )
		pPopMenu->EnableMenuItem( ID_SLOT_DISK, MF_GRAYED );

	if ( nDeviceNum == DN_PHASOR )
	{
		int nDipSW = pCard->GetDipSwitch();
		switch( nDipSW )
		{
		case PM_MB:
			pPopMenu->EnableMenuItem( ID_SLOT_MOCKINGBOARD, MF_GRAYED );
			break;
		case PM_NATIVE:
			pPopMenu->EnableMenuItem( ID_SLOT_PHASOR, MF_GRAYED );
			break;
		}
	}
	else if ( !g_pBoard->m_cSlots.IsAvailable( DN_PHASOR ) )
	{
		pPopMenu->EnableMenuItem( ID_SLOT_MOCKINGBOARD, MF_GRAYED );
		pPopMenu->EnableMenuItem( ID_SLOT_PHASOR, MF_GRAYED );
	}

	GetCursorPos(&ptMouse);
	pPopMenu->TrackPopupMenu(TPM_LEFTALIGN, ptMouse.x, ptMouse.y, this);
}

void CSlotManager::OnBtnSound() 
{
	// TODO: Add your control notification handler code here
	CDlgConfigSound dlgConfigSound;
	dlgConfigSound.DoModal();
}

void CSlotManager::OnBtnMonitor() 
{
	// TODO: Add your control notification handler code here
	CDlgConfigMonitor dlgConfigMonitor;
	dlgConfigMonitor.DoModal();
}
