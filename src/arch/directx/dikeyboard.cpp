// DIKeyboard.cpp: implementation of the CDIKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/directx/dikeyboard.h"
#include "arch/directx/dimouse.h"
#include "aipcdefs.h"
#include <dinput.h>

extern CDIMouse g_cDIMouse;
extern BYTE akm_normal[];

#define KEYDOWN(name,key) (name[key] & 0x80) 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIKeyboard::CDIKeyboard() :
	CCustomThread( "DIKeyboardThread" )
{
	// Clear buffer information
	ZeroMemory(m_buffer,sizeof(m_buffer));
	ZeroMemory(m_oldbuf,sizeof(m_oldbuf));
	m_hKeyboardEvent = NULL;
	m_last = 0;
	m_bLostKey = FALSE;
	m_uRepeat = 33;
	m_uDelay = 490;
	m_hwnd = NULL;
}

CDIKeyboard::~CDIKeyboard()
{
	this->SetActive(FALSE,TRUE);
}

//////////////////////////////////////////////////////////////////////
//
// This will fail if you have not set the HWND
// Initialise the keyboard
//
// true		= Success
// false	= Failed
//
//////////////////////////////////////////////////////////////////////
bool CDIKeyboard::InitKeyboard()
{
	// Check m_hwnd has been set.
	if(!m_hwnd)
	{
		TRACE("HWND Not set, Can not Initialise Keyboard for this application.\nCall SetHWND before calling this method.\n");
		return false;
	}

	m_Initialised=true;
	return true;	// Successfully Created DI Devices!
}

//////////////////////////////////////////////////////////////////////
//
// Is a specific Key being pressed?
//
// true		= Key Pressed
// false	= Key Not Pressed
//
//////////////////////////////////////////////////////////////////////
bool CDIKeyboard::IsKeyPressed(unsigned char keyname)
{
	if( !m_Initialised )
		return false;
	if ( m_buffer[keyname]&0x80 )
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////
//
// Set or change the HWND to the window with primary focus.
//
//////////////////////////////////////////////////////////////////////
void CDIKeyboard::SetHWND(HWND hwnd)
{
	m_hwnd = hwnd;
}

//////////////////////////////////////////////////////////////////////
//
// Set or change the HWND to the window with primary focus.
//
//////////////////////////////////////////////////////////////////////
void CDIKeyboard::SetHWND(CWnd* cwnd)
{
	SetHWND(cwnd->m_hWnd);
}

void CDIKeyboard::KeyDown(USHORT key)
{
	if (key >= 0x100 || (m_buffer[key] & 0x80) != 0)
	{
		return;
	}
	m_buffer[key] |= 0x80;
	::SetEvent(m_hKeyboardEvent);
}

void CDIKeyboard::KeyUp(USHORT key)
{
	if (key >= 0x100 || (m_buffer[key] & 0x80) == 0)
	{
		return;
	}
	m_buffer[key] &= ~0x80;
	::SetEvent(m_hKeyboardEvent);
}

void CDIKeyboard::Run()
{
	DWORD dwObject;
	HANDLE hEvents[] = { GetShutdownEvent(), m_hKeyboardEvent };
	int wait = 0;
	m_bLostKey = FALSE;

	while( TRUE )
	{
		dwObject = ::WaitForMultipleObjects( 2, hEvents, FALSE, /*33*/ m_uRepeat );
		if ( dwObject == WAIT_OBJECT_0 )
			break;
		else if ( dwObject == WAIT_OBJECT_0 + 1 )
		{
			TRACE("Key Event\n");

			if ( KEYDOWN( m_buffer, DIK_LCONTROL ) && KEYDOWN( m_buffer, DIK_LMENU ) )
				break;
			wait = /*15*/ m_uDelay / m_uRepeat;
			ProcessKey( FALSE );			// precess only changed key
			continue;
		}
		else if ( dwObject == WAIT_TIMEOUT )
		{
			if ( wait > 0 )
				wait--;
			else
				ProcessKey(TRUE);		// repeat
		}
		else
			break;
	}
}

void CDIKeyboard::Restore()
{
	if (m_bLostKey == TRUE)
	{
		m_bLostKey = FALSE;
		this->SetActive(TRUE);
	}
}

BOOL CDIKeyboard::OnBeforeActivate()
{
	m_hKeyboardEvent = ::CreateEvent( NULL, FALSE, FALSE, "KeyboardEvent" );

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x06;
	Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[0].hwndTarget = m_hwnd;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		TRACE("CDIKeyboard::OnBeforeActivate() RegisterRawInputDevices() : %08x\n", GetLastError());
		return FALSE;
	}

	::PostMessage(m_hwnd, UM_REQACQUIRE, TRUE, (LPARAM)this);

	return TRUE;
}

void CDIKeyboard::OnAfterDeactivate()
{
	::PostMessage( m_hwnd, UM_REQACQUIRE, FALSE, (LPARAM)this );

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x06;
	Rid[0].dwFlags = RIDEV_REMOVE;   // remove HID keyboard
	Rid[0].hwndTarget = 0;


	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		TRACE("CDIKeyboard::OnBeforeActivate() RegisterRawInputDevices() : %08x\n", GetLastError());
	}

	CloseHandle( m_hKeyboardEvent );
	ZeroMemory( m_buffer, sizeof(m_buffer) );
	ZeroMemory( m_oldbuf, sizeof(m_oldbuf) );
}


void CDIKeyboard::ProcessKey(BOOL bRepeat)
{
	int i;
	for( i = m_last+1; ; i++ )
	{
		if ( i > 0xFF )
			i &= 0xFF;
		if ( m_buffer[i] ^ m_oldbuf[i] )
		{
			if ( m_buffer[i] & 0x80 )
				::PostMessage( m_hwnd, UM_KEYDOWN, i, (LPARAM)this );		// key down
			else
				::PostMessage( m_hwnd, UM_KEYUP, i, (LPARAM)this );		// key up
			m_oldbuf[i] = m_buffer[i];
		}
		else
		{
			if ( ( m_buffer[i] & 0x80 ) && bRepeat && ( akm_normal[i] & 0x80 ) )
				::PostMessage( m_hwnd, UM_KEYREPEAT, i, (LPARAM)this );
			else if ( i != m_last )
				continue;
		}
		break;
	}
	m_last = (BYTE)i;
}

void CDIKeyboard::SetDelayTime(int nRepeat, int nDelay)
{
	if ( nRepeat < 15 )
		nRepeat = 15;
	else if ( nRepeat > 100 )
		nRepeat = 100;
	if ( nDelay < 250 )
		nDelay = 250;
	else if ( nDelay > 1000 )
		nDelay = 1000;
	m_uRepeat = nRepeat;
	m_uDelay = nDelay;
}

void CDIKeyboard::GetDelayTime(int *pnRepeat, int *pnDelay)
{
	*pnRepeat = m_uRepeat;
	*pnDelay = m_uDelay;
}
