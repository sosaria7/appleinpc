// DIKeyboard.cpp: implementation of the CDIKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/directx/dikeyboard.h"
#include "arch/directx/dimouse.h"
#include "aipcdefs.h"

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
	m_bStarted = FALSE;
	m_uRepeat = 33;
	m_uDelay = 490;
}

CDIKeyboard::~CDIKeyboard()
{
	// Shutdown and UnAcquire!
	CDIBase::Shutdown();
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

	// Keyboard Should Always Be An Attached Device!
	// Therefore safe to assume
#if DIRECTINPUT_VERSION > 0x0700
	hr=m_lpDI->CreateDevice(GUID_SysKeyboard,(IDirectInputDevice8A**)&m_lpDIDevice,NULL);
#else
	hr=m_lpDI->CreateDeviceEx(GUID_SysKeyboard,IID_IDirectInputDevice7,(void**)&m_lpDIDevice,NULL);
#endif
	if FAILED(hr) 
	{ 
		CDIBase::Shutdown(); 
		return false; 
	}

	// Now Set The Data Format - Keyboard!
	hr = m_lpDIDevice->SetDataFormat(&c_dfDIKeyboard); 
	if FAILED(hr)
	{ 
		CDIBase::Shutdown(); 
		return false; 
	}

	// Set the cooperative level 
	hr = m_lpDIDevice->SetCooperativeLevel(m_hwnd, 
					   DISCL_FOREGROUND | DISCL_EXCLUSIVE); //DISCL_NONEXCLUSIVE
	if FAILED(hr)
	{ 
		CDIBase::Shutdown(); 
		return false; 
	}

	m_Initialised=true;
	return true;	// Successfully Created DI Devices!
}

//////////////////////////////////////////////////////////////////////
//
//  Poll the device and update the m_buffer data!
//
//////////////////////////////////////////////////////////////////////
bool CDIKeyboard::PollDevice(void)
{
	// Has this object been initialised?
	if ( !m_Initialised || !m_lpDIDevice ) 
	{
		TRACE("Device has not Created in CDIkeyboard::PollDevice()\n");
		return false;
	}

	// Now Get the Keyboard State to the Keyboard Buffer!
    hr = m_lpDIDevice->GetDeviceState( sizeof(m_buffer), (LPVOID)m_buffer );
    if FAILED(hr)
    {
		TRACE("Failed To Obtain Keyboard Data in CDIKeyboard::PollDevice()\nTrying To Restore\n");
		TRACE(GetDIError(hr));

		if( hr==DIERR_INPUTLOST || hr==DIERR_NOTACQUIRED )
		{
			if( !Acquire(true) )
			{
				TRACE("CDIKeyboard::PollDevice Quitting, Could Not Acquire The Device\n");
				return false;
			}
			hr = m_lpDIDevice->GetDeviceState( sizeof(m_buffer), (LPVOID)m_buffer );
		}
		if FAILED(hr)
			return false;
    }
	// Success, We've Acquired the device!
	return true;
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
	CDIBase::SetHWND(hwnd);
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

void CDIKeyboard::Run()
{
	DWORD dwObject;
	HANDLE hEvents[] = { GetShutdownEvent(), m_hKeyboardEvent };
	int wait = 0;
	while( TRUE )
	{
		dwObject = ::WaitForMultipleObjects( 2, hEvents, FALSE, /*33*/ m_uRepeat );
		if ( dwObject == WAIT_OBJECT_0 )
			break;
		else if ( dwObject == WAIT_OBJECT_0 + 1 )
		{
			if ( !PollDevice() )
				break;
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

BOOL CDIKeyboard::OnBeforeActivate()
{
	HRESULT hr;
	m_hKeyboardEvent = ::CreateEvent( NULL, FALSE, FALSE, "KeyboardEvent" );
	hr = m_lpDIDevice->SetEventNotification( m_hKeyboardEvent );
	if FAILED(hr)
	{
		TRACE( "Error SetEventNotification in IDirectInputDevice\n" );
		TRACE( GetDIError(hr) );
		return FALSE;
	}
	::PostMessage( m_hwnd, UM_REQACQUIRE, TRUE, (LPARAM)this );
	return TRUE;
}

void CDIKeyboard::OnAfterDeactivate()
{
	HRESULT hr;
	::PostMessage( m_hwnd, UM_REQACQUIRE, FALSE, (LPARAM)this );
	::Sleep(0);
	hr = m_lpDIDevice->SetEventNotification( NULL );
	while ( hr == DIERR_ACQUIRED )	// not Unacquired yet
	{
		::Sleep(0);
		hr = m_lpDIDevice->SetEventNotification( NULL );
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
