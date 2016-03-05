// AppleClock.cpp : implementation file
//
/*
	Main clock: 1 MHz
	Video Timing
		1 scan line = 40 bytes displayed. 1 Byte = 1 Cycle
		40b * 1c = 40c
		Horizontal Blank (HB) = 25c
		Screen draw : 65c * 192 = 12480
		VBL = 65c * 70 = 4550
	Horizontal video frequency: HSYNC = CLOCK/65 = 15.754 kHz ( 65 cycle )
	Video frequency: VSYNC = HSYNC/262 = 60.129 Hz ( 65 * 262 = 17030 cycle )
	VBlank duration: 1/VSYNC * (70/262) = 4443 us ( 65 * 70 = 4550 cycle )
*/

#include <afxmt.h>

#include "arch/frame/stdafx.h"
#include "arch/frame/dlgdebug.h"
#include "arch/frame/dlgconfigure.h"
#include "arch/frame/mainfrm.h"
#include "arch/directx/dxsound.h"

#include "appleclock.h"
#include "localclock.h"
#include "card.h"
#include "phasor.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL g_debug = FALSE;

CAppleClock *g_pBoard = NULL;

// 0.5 sec
#define BOOST_CLOCK_INTERVAL	( CLOCK / 2 )

#undef SEED
#define SEED	0x10
/////////////////////////////////////////////////////////////////////////////
// CAppleClock

CAppleClock::CAppleClock()
: CCustomThread( "AppleThread" )
{
	m_nAppleStatus = ACS_POWEROFF;
	m_dClockSpeed = 0;
	m_pScreen = NULL;
	m_cpu.init_6502();
	m_dwClock = 0;
}

CAppleClock::~CAppleClock()
{
	// safly exit the thread
	Exit();
/*
	DWORD dwExitCode;
	::GetExitCodeThread( m_hThread, &dwExitCode );
	while ( dwExitCode == STILL_ACTIVE )
	{
		::Sleep(20);
		::GetExitCodeThread( m_hThread, &dwExitCode );
	}
*/
}

/////////////////////////////////////////////////////////////////////////////
// CAppleClock message handlers
void CAppleClock::Run() 
{
	// TODO: Add your specialized code here and/or call the base class
	DWORD measure1, measure2 = 0;
	DWORD host_interval, apple_interval;
	DWORD lastAppleClock=m_dwClock;
	DWORD dwClockInc;
	DWORD dwCurTickCount, dwLastTickCount;

	int sig;
	BOOL slept;
	BOOL drawed;
	int i;

	slept = FALSE;
	drawed = FALSE;

	m_pScreen->ClearBuffer();

	dwLastTickCount = measure1 = GetTickCount();

	m_nAppleStatus = ACS_POWERON;

	while( TRUE ){
		while( TRUE ){
			if (SuspendHere())
			{
				dwLastTickCount = measure1 = GetTickCount();
			}
			if ( ShutdownHere() )
				return;
			if ( !( sig=m_queSignal.GetMesg() ) )
				break;
			switch(sig){
			case ACS_REBOOT:
				m_cIOU.Init();
				m_cpu.Reset();
//				m_queSignal.ClearQueue();
				break;
			case ACS_RESET:
				m_cIOU.InitMemoryMap();
				m_cpu.Reset();
//				m_queSignal.ClearQueue();
				break;
			case ACS_DEBUG:
				g_DXSound.Suspend();
				g_debug = TRUE;
				{
					CDlgDebug dlgDebug(&m_cpu, &m_cIOU);
					dlgDebug.DoModal();
				}
				g_debug = FALSE;
				g_DXSound.Resume();
				break;
			}
		}

		for (i = 0; i < 100; i++)
		{
			dwClockInc = m_cpu.Process();
			m_dwClock += dwClockInc;
			m_pScreen->Clock(dwClockInc);
			m_cSlots.Clock(dwClockInc);
			g_DXSound.Clock();
			if (m_queSignal.Size() > 0)
			{
				continue;
			}
		}

		DWORD dwCurClock;
		dwCurClock = this->m_dwClock;

		// measure clock speed
		dwCurTickCount = GetTickCount();

		if ( m_nBoost > 0 )
		{
			m_nBoost -= dwCurClock - lastAppleClock;
			lastAppleClock = dwCurClock;
			if ( m_nBoost < 0 )
			{
				lastAppleClock += m_nBoost;
				m_nBoost = 0;
			}
		}

		host_interval = dwCurTickCount - measure1;
		if ( host_interval > 1000 )
		{
			m_dClockSpeed = (double)(dwCurClock - measure2) / host_interval / ( CLOCK / 1000 );
			measure1 = dwCurTickCount;		// host tick count
			measure2 = dwCurClock;
		}

		if (dwCurTickCount > dwLastTickCount)
			host_interval = dwCurTickCount - dwLastTickCount;
		else
			host_interval = 0;

		apple_interval = ( dwCurClock / ( CLOCK / 1000 ) ) - ( lastAppleClock / ( CLOCK / 1000 ) );

		if ( (int)(apple_interval - host_interval ) > 0
			|| host_interval > 500 )
		{
			if ( host_interval > 500)
			{
				Sleep(1);
				dwLastTickCount = dwCurTickCount;
				slept = TRUE;
			}
			else
			{
				while( (int)(apple_interval - host_interval ) > 0 )
				{
					Sleep(1);
					host_interval = GetTickCount() - dwLastTickCount;
					slept = TRUE;
				}
				dwLastTickCount += apple_interval;
			}
			lastAppleClock = dwCurClock;
		}
	}
	m_pScreen->ClearBuffer();
}


void CAppleClock::OnDebug() 
{
	if ( m_nAppleStatus == ACS_POWEROFF )
	{
		Suspend(FALSE);
		PowerOn();
	}
	//m_queSignal.AddMessage(ACS_DEBUG);
	Suspend( TRUE );
	g_debug = TRUE;
	{
		CDlgDebug dlgDebug(&m_cpu, &m_cIOU);
		dlgDebug.DoModal();
	}
	g_debug = FALSE;
	Resume();
}

void CAppleClock::OnConfigureSlots()
{
	int stat = m_nAppleStatus;
	if ( GetIsActive() )
		Suspend(TRUE);
	CDlgConfigure dlgConfigure;
	dlgConfigure.DoModal();
	if ( GetIsActive() )
		Resume();
}

int CAppleClock::GetMonitorType()
{
	return m_pScreen->GetMonitorType();
}

void CAppleClock::ChangeMonitorType()
{
	m_pScreen->ChangeMonitorType();
}

void CAppleClock::SetSignal(int signal)
{
	m_queSignal.AddMessage(signal);
	WakeUp();
}

void CAppleClock::Reset()
{
	SetSignal(ACS_RESET);
	return;
}

void CAppleClock::Reboot()
{
	SetSignal(ACS_REBOOT);
	return;
}

void CAppleClock::PowerOn()
{
	SetPriority( THREAD_PRIORITY_HIGHEST );
	SetActive(TRUE);
	m_pScreen->SetMouseCapture(m_cSlots.HasMouseInterface());
	m_pScreen->PowerOn();
}

void CAppleClock::PowerOff()
{
	SetActive(FALSE);
	m_pScreen->PowerOff();
}

void CAppleClock::Suspend(BOOL bWait)
{
	g_DXSound.Suspend();
	CCustomThread::Suspend(bWait);
	m_pScreen->RedrawAll();
	m_pScreen->Redraw();
}

void CAppleClock::Resume()
{
	CCustomThread::Resume();
	g_DXSound.Resume();
}

int CAppleClock::GetAppleStatus()
{
	return m_nAppleStatus;
}

BOOL CAppleClock::Initialize()
{
	SpeedStable();
	m_cSlots.InsertCard( 3, CARD_PHASOR );					// slot 4
	m_cSlots.SetDipSwitch( 3, PM_MB );
	m_cSlots.InsertCard( 5, CARD_DISK_INTERFACE );			// slot 6
	m_cSlots.Initialize();
	m_cIOU.InitMemory();
	g_DXSound.AddPSG( &m_cSpeaker, 0 );
	m_joystick.Initialize();

	//m_cIOU.Init();
	m_cpu.Reset();

	return TRUE;
}

DWORD CAppleClock::GetClock()
{
	return this->m_dwClock;
}

// do not call in apple thread. it will cause dead lock.
void CAppleClock::Exit()
{
	PowerOff();
}


BOOL CAppleClock::OnBeforeActivate()
{
	m_cSlots.PowerOn();
	m_queSignal.ClearQueue();
	g_DXSound.Resume();
	return TRUE;
}

void CAppleClock::OnAfterDeactivate()
{
	m_queSignal.ClearQueue();
	g_DXSound.Suspend();

	m_cIOU.Init();
	m_cpu.Reset();
	m_cSlots.PowerOff();

	SpeedStable();
	m_nAppleStatus = ACS_POWEROFF;

	m_pScreen->RedrawAll();
	m_pScreen->Redraw();
}

void CAppleClock::SpeedUp()
{
	m_nBoost = BOOST_CLOCK_INTERVAL;
}

void CAppleClock::SpeedStable()
{
	m_nBoost = 0;
}

void CAppleClock::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );
	BOOL bActive = GetIsActive();

	if ( bActive )
	{
		Suspend(TRUE);
	}
	if ( ar.IsStoring() )
	{
		ar << m_dwClock;
		ar << m_nAppleStatus;
		m_cpu.Serialize(ar);
		m_cIOU.Serialize(ar);
		m_cSlots.Serialize(ar);
		m_cSpeaker.Serialize(ar);
		m_joystick.Serialize(ar);
		m_pScreen->Serialize(ar);
	}
	else
	{
		ar >> m_dwClock;
		ar >> m_nAppleStatus;
		m_cpu.Serialize(ar);
		m_cIOU.Serialize(ar);
		m_cSlots.Serialize(ar);
		m_cSpeaker.Serialize(ar);
		m_joystick.Serialize(ar);
		m_pScreen->Serialize(ar);

		if ( m_nAppleStatus == ACS_POWERON )
		{
			PowerOn();
		}
	}
	if ( bActive )
	{
		Resume();
	}

}

