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
DWORD g_localClock = 0;

// 0.5 sec
#define BOOST_CLOCK_INTERVAL	( CLOCK / 2 )

#undef SEED
#define SEED	0x10
/////////////////////////////////////////////////////////////////////////////
// CAppleClock

CALLBACK_HANDLER( Clock )
{
	((CAppleClock*)objTo)->ClockInc( (DWORD)lParam );
}

CAppleClock::CAppleClock()
: CCustomThread( "AppleThread" )
{
	m_nAppleStatus = ACS_POWEROFF;
	m_dClockSpeed = 0;
	m_bRedraw = FALSE;
	m_pScreen = NULL;
	SetScanFreq( 60 );
	m_cpu.init_6502();
	m_cpu.SetClockListener( this, Clock );
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
	DWORD sec, measure1, measure2 = 0, dwLastClock = 0;
	DWORD cpu_interval, apple_interval;
	DWORD lastAppleClock=0;
	DWORD dwClockInc;
	int sig;
	BOOL slept;
	BOOL drawed;
	slept = FALSE;
	drawed = FALSE;

	m_pScreen->ClearBuffer();

	m_dwLastCPUClock = measure1 = GetTickCount();

	m_nAppleStatus = ACS_POWERON;

	while( TRUE ){
		while( TRUE ){
			SuspendHere();
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


#ifdef _DEBUG
		g_localClock += 100;
		m_cpu.Clock(100);
#else
		g_localClock += 1000;
		m_cpu.Clock(1000);
#endif
		DWORD dwCurClock;
		dwCurClock = GetClock();
		dwClockInc = dwCurClock - dwLastClock;
		dwLastClock = dwCurClock;

		// measure clock speed
		sec = GetTickCount();

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

//		sec = timeGetTime();
		cpu_interval = sec - measure1;
		if ( cpu_interval > 3000 )
		{
			apple_interval = GetCpuClock()-measure2;
			m_dClockSpeed = (double)( apple_interval ) / ( cpu_interval << 10 );
			measure1 = sec;
			measure2 = GetCpuClock();
		}

		// adjust apple's clock 1Mhz
		// if m_cpu.g_localClock is 1, real time is 0.001ms(1Mhz)
		// in slow machine, may not be 1Mhz, but sleep in 1 seconds.
		if ( sec > m_dwLastCPUClock )
			cpu_interval = sec - m_dwLastCPUClock;
		else
			cpu_interval = 0;
		//apple_interval = dwCurClock-lastAppleClock;
		DWORD temp = ( dwCurClock / ( CLOCK / 1000 ) ) - ( lastAppleClock / ( CLOCK / 1000 ) );

		if ( (int)( temp - cpu_interval ) > 0 
			|| cpu_interval > 500 )
		{
			if ( cpu_interval > 500)
			{
				Sleep(1);
				m_dwLastCPUClock = sec;
				slept = TRUE;
			}
			else
			{
				while( (int)( temp - cpu_interval ) > 0 )
				{
					Sleep(1);
					cpu_interval = GetTickCount() - m_dwLastCPUClock;
					slept = TRUE;
				}
				m_dwLastCPUClock += temp;
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
}

void CAppleClock::PowerOff()
{
	SetActive(FALSE);
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
	m_dwLastCPUClock = GetTickCount();
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

void CAppleClock::SetScanFreq(int freq)
{
	if ( freq > 60 )
		freq = 60;
	else if ( freq < 15 )
		freq = 15;
	m_dwScanCount = m_dwScanFreq = SEED * CLOCK / freq;
}

int CAppleClock::GetScanFreq()
{
	return ( SEED * CLOCK / m_dwScanFreq );
}

DWORD CAppleClock::GetClock()
{
	return( m_cpu.GetClock() );
}

DWORD CAppleClock::GetCpuClock()
{
	return( m_cpu.GetCpuClock() );
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
	m_dwScanCount = 0;
	g_localClock = 0;
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

void CAppleClock::ClockInc(DWORD dwClockInc)
{
	DWORD dwClockIncSEED;

	m_pScreen->Clock( dwClockInc );
	m_cSlots.Clock( dwClockInc );

	g_DXSound.Clock();

	// video refresh
	dwClockIncSEED = dwClockInc * SEED;
	if ( m_dwScanCount < dwClockIncSEED )
	{
		m_dwScanCount += m_dwScanFreq;
		m_bRedraw = TRUE;
	}
	m_dwScanCount -= dwClockIncSEED;
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
		ar << g_localClock;
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
		ar >> g_localClock;
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

