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
#include "6502.h"
#include "65c02.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL g_debug = FALSE;

CAppleClock *g_pBoard = NULL;

#ifdef _DEBUG
static int g_breakpoint = -1;
#endif


#define LINE_CLOCK			65
#define DRAW_CLOCK			(LINE_CLOCK*192)	// 12480

// 1MHz
//#define CLOCK	1024000
//#define CLOCK	8192000
//#define CLOCK	1024000
// http://mirrors.apple2.org.za/ground.icaen.uiowa.edu/MiscInfo/Empson/cpucycles
// http://mirrors.apple2.org.za/ground.icaen.uiowa.edu/MiscInfo/Empson/videocycles
// 14.31818*65/(65*14+2) = 1.020484
#define CLOCK				1020484
// 65c * 262line = 17030
#define SCREEN_CLOCK		17030
#define VBL_CLOCK			(SCREEN_CLOCK-DRAW_CLOCK)

// PAL
// 14.25045*65/(65*14+2) = 1.015657
#define CLOCK_PAL			1015657
// 65c * 312line = 20280
#define SCREEN_CLOCK_PAL	20280
#define VBL_CLOCK_PAL		(SCREEN_CLOCK_PAL-DRAW_CLOCK)



DWORD g_dwCPS = CLOCK;
DWORD g_dwVBLClock = VBL_CLOCK;
DWORD g_dwFrameClock = SCREEN_CLOCK;

// 0.005 sec
#define BOOST_CLOCK_INTERVAL	( CLOCK/200 )

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

	m_dwClock = 0;
	m_bPALMode = FALSE;
	m_nMachineType = MACHINE_APPLE2E;
	m_pCpu = new C65c02();

	SetMachineType(MACHINE_APPLE2E, FALSE);
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
	DWORD dwCPMS = g_dwCPS / 1000;

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
				m_pCpu->Reset();
//				m_queSignal.ClearQueue();
				break;
			case ACS_RESET:
				m_cIOU.InitMemoryMap();
				m_pCpu->Reset();
//				m_queSignal.ClearQueue();
				break;
			case ACS_DEBUG:
				g_DXSound.Suspend();
				g_debug = TRUE;
				{
					CDlgDebug dlgDebug((C65c02*)m_pCpu, &m_cIOU);
					dlgDebug.DoModal();
				}
				g_debug = FALSE;
				g_DXSound.Resume();
				break;
			}
		}

		for (i = 0; i < 100; i++)
		{
			dwClockInc = m_pCpu->Process();
			m_dwClock += dwClockInc;
			m_pScreen->Clock(dwClockInc);
			m_cSlots.Clock(dwClockInc);
			g_DXSound.Clock();

			if (m_nBoost > 0)
			{
				m_nBoost -= dwClockInc;
				lastAppleClock += dwClockInc;
				if (m_nBoost < 0)
				{
					lastAppleClock += m_nBoost;
					m_nBoost = 0;
				}
			}

			if (m_queSignal.Size() > 0)
			{
				break;
			}

#ifdef _DEBUG
			if (((C65c02*)m_pCpu)->getRegPC() == g_breakpoint)
			{
				Suspend(FALSE);
				::PostMessage(this->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_DEBUG, 0);
				break;
			}
#endif
		}

		DWORD dwCurClock;
		dwCurClock = this->m_dwClock;

		// measure clock speed
		dwCurTickCount = GetTickCount();

		host_interval = dwCurTickCount - measure1;
		if ( host_interval > 1000 )
		{
			m_dClockSpeed = (double)(dwCurClock - measure2) / host_interval / 1000;
			measure1 = dwCurTickCount;		// host tick count
			measure2 = dwCurClock;
		}

		if (dwCurTickCount > dwLastTickCount)
			host_interval = dwCurTickCount - dwLastTickCount;
		else
			host_interval = 0;

		apple_interval = (dwCurClock - lastAppleClock) / dwCPMS;

		if ( (int)(apple_interval - host_interval ) > 0
			|| host_interval > 500 )
		{
			if ( host_interval > 500 || (int)( apple_interval - host_interval ) > 1000 )
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
			lastAppleClock += apple_interval * dwCPMS;
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
		CDlgDebug dlgDebug((C65c02*)m_pCpu, &m_cIOU);
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
	m_cSlots.InsertCard(3, CARD_PHASOR);					// slot 4
	m_cSlots.SetDipSwitch(3, PM_MB);
	m_cSlots.InsertCard(5, CARD_DISK_INTERFACE);			// slot 6
	m_cSlots.InsertCard(6, CARD_HDD);						// slot 7
	m_cSlots.Initialize();
	m_cIOU.InitMemory(m_nMachineType);
	g_DXSound.AddPSG(&m_cSpeaker, 0);
	m_joystick.Initialize();

	//m_cIOU.Init();
	m_pCpu->Reset();

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
	m_pCpu->Reset();
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

void CAppleClock::SetMachineType(int nMachineType, BOOL bPalMode)
{
	if (this->m_nMachineType != nMachineType)
	{
		this->m_nMachineType = nMachineType;
		delete this->m_pCpu;
		if (this->m_nMachineType == MACHINE_APPLE2E)
		{
			this->m_pCpu = new C65c02();
		}
		else
		{
			this->m_pCpu = new C6502();
		}
		this->m_cIOU.InitMemory(this->m_nMachineType);
		this->m_pCpu->Reset();
	}
	if (this->m_bPALMode != bPalMode)
	{
		this->m_bPALMode = bPalMode;
		if (m_bPALMode == TRUE)
		{
			g_dwCPS = CLOCK_PAL;
			g_dwFrameClock = SCREEN_CLOCK_PAL;
			g_dwVBLClock = VBL_CLOCK_PAL;
		}
		else
		{
			g_dwCPS = CLOCK;
			g_dwFrameClock = SCREEN_CLOCK;
			g_dwVBLClock = VBL_CLOCK;
		}
		m_cSpeaker.ChangeSampleRate();
	}
}

void CAppleClock::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );
	BOOL bActive = GetIsActive();
	BOOL bPalMode;
	int nMachineType = MACHINE_APPLE2E;

	if ( bActive )
	{
		Suspend(TRUE);
	}
	if ( ar.IsStoring() )
	{
		ar << m_dwClock;
		ar << m_nAppleStatus;
		ar << m_bPALMode;
		ar << m_nMachineType;
		m_pCpu->Serialize(ar);
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
		if (g_nSerializeVer >= 6)
			ar >> bPalMode;
		if (g_nSerializeVer >= 7)
			ar >> nMachineType;
		SetMachineType(nMachineType, bPalMode);

		m_pCpu->Serialize(ar);
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

