// Joystick.cpp: implementation of the CJoystick class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/directx/dijoystick.h"
#include "arch/directx/dikeyboard.h"
#include "joystick.h"
#include "65c02.h"
#include "appleclock.h"
#include <Xinput.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern "C" {
	typedef DWORD(WINAPI *TFnXInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef DWORD(WINAPI *TFnXInputGetCapabilities)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
};

TFnXInputGetState			s_fnXInputGetState = NULL;
TFnXInputGetCapabilities	s_fnXInputGetCapabilities = NULL;
HMODULE hXInputModule = NULL;

#define JOY_MIN_DEADZONE		0
#define JOY_MAX_DEADZONE		20
#define JOY_MIN_SATURATION		50
#define JOY_MAX_SATURATION		100

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern CDIJoystick g_cDIJoystick;
extern CDIKeyboard g_cDIKeyboard;

CJoystick::CJoystick()
{
	m_dwLastClock = 0;
	m_dwLastPoll = 0;
	m_nJoystickMode = JM_KEYPAD;
	m_bStrobe = FALSE;
	m_bHasPCJoystick = FALSE;
	m_nDeadZone = 10;
	m_nSaturation = 95;
	m_bArrowAsPaddle = TRUE;
	m_bSwapButtons = FALSE;
	m_bIsLegacy = FALSE;
	memset(&m_xinputState, 0, sizeof(m_xinputState));

	if (hXInputModule == NULL)
	{
		hXInputModule = LoadLibrary("xinput1_3.dll");
		if (hXInputModule != NULL)
		{
			s_fnXInputGetState = (TFnXInputGetState)GetProcAddress(hXInputModule, "XInputGetState");
			s_fnXInputGetCapabilities = (TFnXInputGetCapabilities)GetProcAddress(hXInputModule, "XInputGetCapabilities");
			if (s_fnXInputGetState == NULL || s_fnXInputGetCapabilities == NULL)
			{
				s_fnXInputGetState = NULL;
				s_fnXInputGetCapabilities = NULL;
			}
		}
	}
}

CJoystick::~CJoystick()
{

}

#define KEYDOWN(key)	( g_cDIKeyboard.IsKeyPressed( key ) )

BYTE CJoystick::GetStatus(BYTE num)
{
	DWORD retval = 0x7f;
	CKeyboard *keybd;
	BOOL bScroll;
	keybd = &g_pBoard->m_keyboard;
	bScroll = keybd->GetScrollLock() && m_bArrowAsPaddle;

	switch (num & 0x0F)
	{
	case 1:		// button 0
		if (IsButtonDown(0))
			return 0xFF;
		else
			return 0x00;
	case 2:		// button 1
		if (IsButtonDown(1))
			return 0xFF;
		else
			return 0x00;
	case 3:		// button 3
		if (KEYDOWN(DIK_LSHIFT) || KEYDOWN(DIK_RSHIFT))
			return 0xFF;
		else
			return 0x00;
	case 4:		// paddel 0
		retval = (GetPaddleState(0) + 10000) * 0x100 / 20000;
		break;
	case 5:		// paddel 1
		retval = (GetPaddleState(1) + 10000) * 0x100 / 20000;
		break;
	case 6:		// paddel 2
	case 7:		// paddel 3
		retval = 0x7F;
		break;
	default:
		return MemReturnRandomData(2);
	}

//	if ( m_bStrobe == FALSE )
//		return 0x00;

	retval = retval * (10 + retval/86);
	//retval = retval * 11;
	
	DWORD interval = g_pBoard->GetClock() - m_dwLastClock;

	if ( interval > retval )
	{
		m_bStrobe = FALSE;
		return 0x00;
	}

	return 0xFF;
}

BOOL CJoystick::IsButtonDown(int num)
{
	BOOL bIsDown = FALSE;

	if (num == 0)
	{
		if (KEYDOWN(DIK_LMENU) || (!m_bSwapButtons ? KEYDOWN(DIK_DELETE) : KEYDOWN(DIK_INSERT)))
			return TRUE;
	}
	else
	{
		if (KEYDOWN(DIK_RMENU) || (!m_bSwapButtons ? KEYDOWN(DIK_INSERT) : KEYDOWN(DIK_DELETE)))
			return TRUE;
	}

	Poll();
	if (m_bHasPCJoystick == TRUE)
	{
		if (m_bIsLegacy == TRUE)
		{
			bIsDown = g_cDIJoystick.IsJoystickFire(num);
		}
		else
		{
			if (num == 0)
				bIsDown = (m_xinputState.Gamepad.wButtons & (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_Y)) != 0;
			else
				bIsDown = (m_xinputState.Gamepad.wButtons & (XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_X)) != 0;
		}
	}
	return bIsDown;
}

// -10000 ~ 9999
int CJoystick::GetPaddleState(int num)
{
	DIJOYSTATE2 *stJState;
	int retval = 0;
	BOOL bScroll;
	CKeyboard *keybd;
	keybd = &g_pBoard->m_keyboard;

	bScroll = keybd->GetScrollLock() && m_bArrowAsPaddle;

	if (num == 0)
	{
		if (KEYDOWN(DIK_NUMPAD1) ||
			KEYDOWN(DIK_NUMPAD4) ||
			KEYDOWN(DIK_NUMPAD7) ||
			(bScroll && KEYDOWN(DIK_LEFT)))
			return -10000;
		if (KEYDOWN(DIK_NUMPAD9) ||
			KEYDOWN(DIK_NUMPAD6) ||
			KEYDOWN(DIK_NUMPAD3) ||
			(bScroll && KEYDOWN(DIK_RIGHT)))
			return 9999;
	}
	else
	{
		if (KEYDOWN(DIK_NUMPAD7) ||
			KEYDOWN(DIK_NUMPAD8) ||
			KEYDOWN(DIK_NUMPAD9) ||
			(bScroll && KEYDOWN(DIK_UP)))
			return -10000;
		if (KEYDOWN(DIK_NUMPAD1) ||
			KEYDOWN(DIK_NUMPAD2) ||
			KEYDOWN(DIK_NUMPAD3) ||
			(bScroll && KEYDOWN(DIK_DOWN)))
			return 9999;
	}

	Poll();
	if (m_bHasPCJoystick == TRUE)
	{
		if (m_bIsLegacy == TRUE)
		{
			stJState = g_cDIJoystick.GetJoystickStateInfo();
			if (num == 0)
				retval = stJState->lX;
			else
				retval = stJState->lY;
		}
		else
		{
			if (num == 0)
			{
				if ((m_xinputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0)
					return -10000;
				else if ((m_xinputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0)
					return 9999;
				else
					retval = m_xinputState.Gamepad.sThumbLX;
			}
			else
				if ((m_xinputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0)
					return -10000;
				else if ((m_xinputState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0)
					return 9999;
				else
					retval = -m_xinputState.Gamepad.sThumbLY - 1;
			retval = retval * 10000 / 32768;
		}
		if (m_nSaturation > 0)
			retval = retval * 100 / m_nSaturation;
		if (retval >= -m_nDeadZone * 100 && retval <= m_nDeadZone * 100)
			retval = 0;
		if (retval > 9999)
			retval = 9999;
		else if (retval < -10000)
			retval = -10000;
	}
	return retval;
}

void CJoystick::Strobe()
{
	m_dwLastClock = g_pBoard->GetClock();
	m_bStrobe = TRUE;
}

int CJoystick::ChangeDevice(int nMode)
{
	if ( m_nJoystickMode == nMode )
		return 0;
	m_nJoystickMode = nMode;
	CKeyboard::EnableNumKey( nMode != JM_KEYPAD );

	return 0;
}

int CJoystick::GetDevice()
{
	return m_nJoystickMode;
}

int CJoystick::GetDeadZone()
{
	return m_nDeadZone;
}

int CJoystick::GetSaturation()
{
	return m_nSaturation;
}

void CJoystick::SetDeadZone( int nDeadZone )
{
	if ( nDeadZone < JOY_MIN_DEADZONE )
	{
		nDeadZone = JOY_MIN_DEADZONE;
	}
	if ( nDeadZone > JOY_MAX_DEADZONE )
	{
		nDeadZone = JOY_MAX_DEADZONE;
	}
	m_nDeadZone = nDeadZone;
}

void CJoystick::SetSaturation( int nSaturation )
{
	if ( nSaturation < JOY_MIN_SATURATION )
	{
		nSaturation = JOY_MIN_SATURATION;
	}
	if ( nSaturation > JOY_MAX_SATURATION )
	{
		nSaturation = JOY_MAX_SATURATION;
	}
	m_nSaturation = nSaturation;
}

void CJoystick::Poll()
{
	DWORD dwCurrentTick = ::GetTickCount();
	if (dwCurrentTick - m_dwLastPoll < 50)
	{
		return;
	}
	if (m_bHasPCJoystick == FALSE)
	{
		if (dwCurrentTick - m_dwLastPoll > 1000)
		{
			InitPCJoystick();
			m_dwLastPoll = dwCurrentTick;
		}
	}
	if (m_bHasPCJoystick == TRUE)
	{
		if (m_bIsLegacy == TRUE) {
			if (g_cDIJoystick.PollDevice() == FALSE)
			{
				m_bHasPCJoystick = FALSE;
				m_bIsLegacy = FALSE;
			}
		}
		else
		{
			if (s_fnXInputGetState != NULL && s_fnXInputGetState(0, &m_xinputState) != ERROR_SUCCESS)
			{
				m_bHasPCJoystick = FALSE;
				m_bIsLegacy = FALSE;
			}
		}
		m_dwLastPoll = dwCurrentTick;
	}
}

void CJoystick::InitPCJoystick()
{
	if (m_bHasPCJoystick == FALSE)
	{
		XINPUT_CAPABILITIES caps;
		if (s_fnXInputGetCapabilities != NULL && s_fnXInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS)
		{
			m_bHasPCJoystick = TRUE;
			m_bIsLegacy = FALSE;
		}
	}
	if (m_bHasPCJoystick == TRUE && m_bIsLegacy == FALSE)
	{
		if (s_fnXInputGetState != NULL && s_fnXInputGetState(0, &m_xinputState) != ERROR_SUCCESS)
		{
			m_bHasPCJoystick = FALSE;
			m_bIsLegacy = FALSE;
		}
	}
	if (m_bHasPCJoystick == FALSE)
	{
		if (g_cDIJoystick.InitJoystick())
		{
			m_bHasPCJoystick = TRUE;
			m_bIsLegacy = TRUE;
		}
	}
}

void CJoystick::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_nJoystickMode;
		ar << m_nDeadZone;
		ar << m_nSaturation;
		ar << m_bArrowAsPaddle;
		ar << m_bSwapButtons;
	}
	else
	{
		ar >> m_nJoystickMode;
		ar >> m_nDeadZone;
		ar >> m_nSaturation;
		if (g_nSerializeVer >= 8)
		{
			ar >> m_bArrowAsPaddle;
			ar >> m_bSwapButtons;
		}
		if ( m_nJoystickMode == JM_PCJOYSTICK )
		{
			if ( m_bHasPCJoystick == FALSE )
			{
				m_nJoystickMode = JM_KEYPAD;
			}
		}
		Initialize();
	}
}

void CJoystick::Initialize()
{

	ChangeDevice( m_nJoystickMode );
	SetDeadZone( m_nDeadZone );
	SetSaturation( m_nSaturation );
}
