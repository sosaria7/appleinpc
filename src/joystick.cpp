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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

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
}

CJoystick::~CJoystick()
{

}

#define KEYDOWN(key)	( g_cDIKeyboard.IsKeyPressed( key ) )

BYTE CJoystick::GetStatus(BYTE num)
{
	DWORD retval = 0x7f;
	CKeyboard *keybd;
	DIJOYSTATE2 *stJState;

	keybd = &g_pBoard->m_keyboard;
	switch( m_nJoystickMode )
	{
	case JM_KEYPAD:
		switch( num & 0x0F )
		{
		case 1:
			return ( KEYDOWN( DIK_LMENU ) || KEYDOWN( DIK_DELETE ) ) ? 0xFF : 0x00;
		case 2:
			return ( KEYDOWN( DIK_RMENU ) || KEYDOWN( DIK_INSERT ) ) ? 0xFF : 0x00;
		case 3:
			return ( KEYDOWN( DIK_LSHIFT ) || KEYDOWN( DIK_RSHIFT ) ) ? 0xFF : 0x00;
			// paddel
		case 4:
			if ( KEYDOWN( DIK_NUMPAD1 )
				|| KEYDOWN( DIK_NUMPAD4 )
				|| KEYDOWN( DIK_NUMPAD7 ) 
				|| KEYDOWN( DIK_LEFT ) )
				retval -= 0x7f;
			if ( KEYDOWN( DIK_NUMPAD3 )
				|| KEYDOWN( DIK_NUMPAD6 )
				|| KEYDOWN( DIK_NUMPAD9 )
				|| KEYDOWN( DIK_RIGHT ) )
				retval += 0x80;
			break;
			// paddel 1
		case 5:
			if ( KEYDOWN( DIK_NUMPAD7 )
				|| KEYDOWN( DIK_NUMPAD8 )
				|| KEYDOWN( DIK_NUMPAD9 ) 
				|| KEYDOWN( DIK_UP ) )
				retval -= 0x7f;
			if ( KEYDOWN( DIK_NUMPAD1 )
				|| KEYDOWN( DIK_NUMPAD2 )
				|| KEYDOWN( DIK_NUMPAD3 ) 
				|| KEYDOWN( DIK_DOWN ) )
				retval += 0x80;
			break;
			// paddel 2
		case 6:
			// paddel 3
		case 7:
			retval = 0x7F;
			break;
		default:
			return MemReturnRandomData(2);
		}
		break;
	case JM_PCJOYSTICK:
		switch( num & 0x0F )
		{
		case 1:		// button 0
			Poll();
			return ( KEYDOWN( DIK_LMENU ) || g_cDIJoystick.IsJoystickFire( 0 ) ) ? 0xFF : 0x00;
		case 2:		// button 1
			Poll();
			return ( KEYDOWN( DIK_RMENU ) || g_cDIJoystick.IsJoystickFire( 1 ) ) ? 0xFF : 0x00;
		case 3:		// button 3
//			Poll();
			return ( KEYDOWN( DIK_LSHIFT ) || KEYDOWN( DIK_RSHIFT ) ) ? 0xFF : 0x00;
		case 4:		// paddel 0
			Poll();
			stJState = g_cDIJoystick.GetJoystickStateInfo();
			retval = ( stJState->lX + 10000 ) * 0xFF / 20000;
			break;
		case 5:		// paddel 1
			Poll();
			stJState = g_cDIJoystick.GetJoystickStateInfo();
			retval = ( stJState->lY + 10000 ) * 0xFF / 20000;
			break;
		case 6:		// paddel 2
		case 7:		// paddel 3
			retval = 0x7F;
			break;
		default:
			return MemReturnRandomData(2);
		}
		break;
	default:		// No Joystick
		switch( num & 0x0F )
		{
		case 1:
			return KEYDOWN( DIK_LMENU ) ? 0xFF : 0x00;
		case 2:
			return KEYDOWN( DIK_RMENU ) ? 0xFF : 0x00;
		case 3:
			return ( KEYDOWN( DIK_LSHIFT ) || KEYDOWN( DIK_RSHIFT ) ) ? 0xFF : 0x00;
		case 4:
		case 5:
		case 6:
		case 7:
			retval = 0x7F;
			break;
		default:
			return MemReturnRandomData(2);
		}
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
	g_cDIJoystick.SetDeadZone( nDeadZone );
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
	g_cDIJoystick.SetSaturation( nSaturation );
}

void CJoystick::Poll()
{
	if ( m_nJoystickMode == JM_PCJOYSTICK )
	{
		DWORD dwClock = g_pBoard->GetClock();
		if ( ( dwClock - m_dwLastPoll ) > 51200 )		// 1/20 second (20 frame)
		{
			m_dwLastPoll = dwClock;
			g_cDIJoystick.PollDevice();
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
	}
	else
	{
		ar >> m_nJoystickMode;
		ar >> m_nDeadZone;
		ar >> m_nSaturation;
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
