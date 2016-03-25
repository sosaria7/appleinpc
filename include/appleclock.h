#if !defined(AFX_APPLECLOCK_H__9316A5A3_197F_4B8F_8FAB_7E3A63861227__INCLUDED_)
#define AFX_APPLECLOCK_H__9316A5A3_197F_4B8F_8FAB_7E3A63861227__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AppleClock.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CAppleClock thread

#include "arch/frame/screen.h"
#include "arch/customthread.h"
#include "keyboard.h"
#include "memory.h"
#include "65c02.h"
#include "joystick.h"

#include "messageque.h"
#include "slots.h"
#include "speaker.h"
#include "lockmgr.h"
#include "iou.h"

class CMainFrame;

class CAppleClock : public CCustomThread, public CObject
{
// Attributes
public:
	CCpu*		m_pCpu;
	CAppleIOU	m_cIOU;
	CKeyboard	m_keyboard;
	CJoystick	m_joystick;
	CScreen		*m_pScreen;
	CMainFrame* m_lpwndMainFrame;
	CSlots		m_cSlots;

	CSpeaker	m_cSpeaker;

	CEvent	m_cMessage;
	BOOL m_bPALMode;
	int m_nMachineType;

protected:
	CMessageQue m_queSignal;

// Operations
public:
	CAppleClock();
	virtual ~CAppleClock();

	void OnAfterDeactivate();
	BOOL OnBeforeActivate();
	void Exit();
	DWORD GetClock();
	BOOL Initialize();
	void Resume();
	void Suspend(BOOL bWait=TRUE);
	int GetAppleStatus();
	void PowerOff();
	void PowerOn();
	double m_dClockSpeed;
	void ChangeMonitorType();
	int GetMonitorType();
	void Reboot();
	void Reset();
	void SetSignal(int signal);
	void OnConfigureSlots();

	void SpeedUp();
	void SpeedStable();

	void OnDebug();

	virtual void Run();

	void Serialize( CArchive &archive );
	void SetMachineType(int nMachineType, BOOL bPalMode);

// Implementation
protected:
	DWORD m_dwClock;
	int m_nBoost;
	int m_nAppleStatus;
};

extern CAppleClock *g_pBoard;
extern int g_nSerializeVer;
extern DWORD g_dwCPS;
extern DWORD g_dwVBLClock;
extern DWORD g_dwFrameClock;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLECLOCK_H__9316A5A3_197F_4B8F_8FAB_7E3A63861227__INCLUDED_)
