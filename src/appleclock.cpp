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
#include "arch/frame/dlgsettings.h"
#include "arch/frame/mainfrm.h"
#include "arch/directx/dxsound.h"
#include "arch/directx/dikeyboard.h"

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
extern CDIKeyboard g_cDIKeyboard;

#ifdef _DEBUG
static int g_breakpoint = -1;
#endif

#define STATUS_VERSION		(11)
#define STATUS_MIN_VERSION	(3)
#define STATUS_MAGIC	0x89617391
int g_nSerializeVer = 0;

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
#define DRIFT_CLOCK_INTERVAL	( CLOCK/200 )

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
	m_bReserveLoadState = FALSE;
	m_strStateFilePath = TEXT("");

	SetMachineType(MACHINE_APPLE2E, FALSE);
}

CAppleClock::~CAppleClock()
{
	// safly exit the thread
	//Exit();
	PowerOff();
	delete m_pCpu;
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
	DWORD lastAppleClock=m_dwClock;
	DWORD measure2 = m_dwClock;
	DWORD dwClockInc;
	DWORD dwDriftAppleClock = 0;
	double TPC;		// tic per apple clock
	double temp;
	double remain = 0;
	int nJitter;

	LARGE_INTEGER measure1;
	LARGE_INTEGER curTickCount, lastTickCount;
	LARGE_INTEGER freq;
	LARGE_INTEGER host_interval, apple_interval;
	LARGE_INTEGER host_interval_hold;

	int sig;
	BOOL slept;
	BOOL drawed;
	int i;

	QueryPerformanceFrequency(&freq);
	TPC = (double)freq.QuadPart / g_dwCPS;

	slept = FALSE;
	drawed = FALSE;

	m_pScreen->ClearBuffer();

	QueryPerformanceCounter(&curTickCount);
	measure1 = lastTickCount = curTickCount;

	m_nAppleStatus = ACS_POWERON;

	while( TRUE ){
		while( TRUE ){
			if (SuspendHere())
			{
				QueryPerformanceCounter(&curTickCount);
				lastTickCount = curTickCount;
				measure1 = curTickCount;
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

			if (m_nDrift > 0 || dwDriftAppleClock > 0)
			{
				if (dwDriftAppleClock == 0)
				{
					QueryPerformanceCounter(&curTickCount);
					host_interval_hold.QuadPart = curTickCount.QuadPart - lastTickCount.QuadPart;
				}

				m_nDrift -= dwClockInc;
				dwDriftAppleClock += dwClockInc;
				if (m_nDrift <= 0)
				{
					dwDriftAppleClock += m_nDrift;

					lastAppleClock += dwDriftAppleClock;
					dwDriftAppleClock = 0;
					lastTickCount.QuadPart = curTickCount.QuadPart - host_interval_hold.QuadPart;

					m_nDrift = 0;
					break;
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
		QueryPerformanceCounter(&curTickCount);

		host_interval.QuadPart = curTickCount.QuadPart - measure1.QuadPart;
		if (host_interval.QuadPart > freq.QuadPart)	// 1 second
		{
			m_dClockSpeed = (double)(dwCurClock - measure2) / host_interval.QuadPart * freq.QuadPart / 1000000;
			measure1 = curTickCount;		// host tick count
			measure2 = dwCurClock;
		}

		if (dwDriftAppleClock > 0)
		{
			continue;
		}

		host_interval.QuadPart = ( curTickCount.QuadPart - lastTickCount.QuadPart );

		temp = (dwCurClock - lastAppleClock) * TPC + remain;		// convert apple clock to tick count
		apple_interval.QuadPart = (LONGLONG)temp;
		remain = temp - apple_interval.QuadPart;

		nJitter = (int)(apple_interval.QuadPart - host_interval.QuadPart);
		if (nJitter < -freq.QuadPart)	// apple is too slow
		{
			// could not chase the apple speed
			lastTickCount = curTickCount;
			lastAppleClock = dwCurClock;
		}
		else if (nJitter > freq.QuadPart)	// apple is too fast (something wrong)
		{
			Sleep(1);
			lastTickCount = curTickCount;	// adjust host time
			lastAppleClock = dwCurClock;
			slept = TRUE;
		}
		else if (nJitter > 0)
		{
			while ((int)(apple_interval.QuadPart - host_interval.QuadPart) > 0)
			{
				Sleep(1);
				QueryPerformanceCounter(&curTickCount);
				host_interval.QuadPart = (curTickCount.QuadPart - lastTickCount.QuadPart);
				slept = TRUE;
			}

			lastTickCount.QuadPart += apple_interval.QuadPart;
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
	{
		CDlgSettings dlgSettings;
		dlgSettings.DoModal();
	}
	if (GetIsActive() || m_bReserveLoadState == TRUE)
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
	m_pScreen->Redraw();
}

void CAppleClock::Resume()
{
	if (m_bReserveLoadState == TRUE && !m_strStateFilePath.IsEmpty())
	{
		CString strStateFilePath = m_strStateFilePath;

		LoadState(m_strStateFilePath);
		m_bReserveLoadState = FALSE;

		m_strStateFilePath = strStateFilePath;
	}
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
	if (m_bSaveStateOnExit == TRUE && !m_strStateFilePath.IsEmpty())
	{
		SaveState(m_strStateFilePath);
	}
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

	m_pScreen->Redraw();
}

void CAppleClock::SpeedUp()
{
	m_nDrift = DRIFT_CLOCK_INTERVAL;
	m_pScreen->Relax();
}

void CAppleClock::SpeedStable()
{
	m_nDrift = 0;
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
			Suspend(FALSE);
			PowerOn();
		}
	}
}

BOOL CAppleClock::SaveState(CString strPath)
{
	CFile file;
	BOOL bSuccess = FALSE;

	if (file.Open(strPath, CFile::modeCreate | CFile::modeWrite))
	{
		CArchive ar(&file, CArchive::store);

		bSuccess = TRUE;
		g_pBoard->Suspend(TRUE);

		int nVal, nVal2;
		try
		{
			g_nSerializeVer = STATUS_VERSION;

			ar << STATUS_MAGIC;
			ar << STATUS_VERSION;
			nVal = 0;
			ar << nVal;		// double size (not used)
			g_pBoard->Serialize(ar);
			ar << g_DXSound.GetPan();
			ar << g_DXSound.GetVolume();
			ar << g_DXSound.m_bMute;
			g_cDIKeyboard.GetDelayTime(&nVal, &nVal2);
			ar << nVal;
			ar << nVal2;
			ar << m_strStateFilePath;
			ar << m_bSaveStateOnExit;
		}
		catch (CFileException* fe)
		{
			(void)fe;
			bSuccess = FALSE;
		}
		catch (CArchiveException* ae)
		{
			(void)ae;
			bSuccess = FALSE;
		}
		ar.Close();
		file.Close();
	}
	return bSuccess;
}

BOOL CAppleClock::LoadState(CString strPath)
{
	CFile file;
	BOOL bSuccess = FALSE;

	if (file.Open(strPath, CFile::modeRead))
	{
		int nVal, nVal2;

		bSuccess = TRUE;
		CArchive ar(&file, CArchive::load);
		try
		{
			ar >> nVal;
			ar >> nVal2;
			if (nVal != STATUS_MAGIC || nVal2 < STATUS_MIN_VERSION)
			{
				throw new CArchiveException();
			}
			g_nSerializeVer = nVal2;

			ar >> nVal;		// double size
			g_pBoard->Serialize(ar);
			ar >> nVal;
			g_DXSound.SetPan(nVal);
			ar >> nVal;
			g_DXSound.SetVolume(nVal);
			ar >> g_DXSound.m_bMute;
			ar >> nVal;
			ar >> nVal2;
			g_cDIKeyboard.SetDelayTime(nVal, nVal2);
			if (g_nSerializeVer >= 9)
			{
				ar >> m_strStateFilePath;
				ar >> m_bSaveStateOnExit;
			}
		}
		catch (CFileException* fe)
		{
			(void)fe;
			bSuccess = FALSE;
		}
		catch (CArchiveException* ae)
		{
			(void)ae;
			bSuccess = FALSE;
		}
		ar.Close();
		file.Close();
	}
	return bSuccess;
}

