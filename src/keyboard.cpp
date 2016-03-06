// Keyboard.cpp: implementation of the CKeyboard class.
//
//////////////////////////////////////////////////////////////////////



#include "arch/frame/stdafx.h"

#include "arch/directx/dikeyboard.h"
#include "arch/directx/dimouse.h"
#include "arch/frame/mainfrm.h"

#include "keyboard.h"
#include "applekeymap.h"
#include "appleclock.h"

#include <afxmt.h>

extern CDIKeyboard g_cDIKeyboard;
extern CDIMouse g_cDIMouse;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCriticalSection g_keyboardLock;

CKeyboard::CKeyboard()
{
	m_lastKey=0;
	m_bCaps = FALSE;
}

CKeyboard::~CKeyboard()
{
	
}

#define KEYDOWN(key)	( g_cDIKeyboard.IsKeyPressed( key ) )

void CKeyboard::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	BYTE key;

	BOOL ctrl = KEYDOWN( DIK_LCONTROL ) || KEYDOWN( DIK_RCONTROL );
	BOOL alt = KEYDOWN( DIK_LMENU ) || KEYDOWN( DIK_RMENU );
	BOOL shift = KEYDOWN( DIK_LSHIFT ) || KEYDOWN( DIK_RSHIFT );

	if ( lParam == 0 )		// only for key down, not for repeat
	{
		switch( wParam )
		{
		case DIK_CAPITAL:
			m_bCaps = !m_bCaps;
			return;
		case 0xC6:			// control+pause : break
			g_pBoard->Reset();
			return;

		case DIK_F8:
			if ( ctrl )		// Toggle Full Screen mode
			{
				::PostMessage( g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_FULL_SCREEN, 0 );
				//CScreen* pScreen = g_pBoard->m_pScreen;
				//pScreen->SetFullScreenMode( pScreen->m_bWindowed );		// Toggle Full Screen mode
				return;
			}
			else if ( shift )	// Toggle Message View
			{
				CScreen* pScreen = g_pBoard->m_pScreen;
				pScreen->ToggleMessage();								// Toggle Message View
				return;
			}
			g_pBoard->m_pScreen->ChangeMonitorType();
			return;

		case DIK_F9:
			if ( ctrl )
			{
				if ( g_pBoard->m_cSlots.HasHardDiskInterface() )
				{
					//g_cDIKeyboard.SetActive( FALSE, FALSE );	// don't wait for thread done
					//g_cDIMouse.SetActive( FALSE, FALSE );		// don't wait
					::PostMessage( g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_HARDDISK, 0 );
				}
			}
			else
			{
				if ( g_pBoard->m_cSlots.HasDiskInterface() )
				{
					//g_cDIKeyboard.SetActive( FALSE, FALSE );	// don't wait for thread done
					//g_cDIMouse.SetActive( FALSE, FALSE );		// don't wait
					::PostMessage( g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_DISKETTE, 0 );
				}
			}
			return;

		case DIK_F10:
			//g_cDIKeyboard.SetActive( FALSE, FALSE );	// don't wait for thread done
			//g_cDIMouse.SetActive( FALSE, FALSE );		// don't wait
			::PostMessage( g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_CONFIGURE_SLOTS, 0 );
			return;
			
		case DIK_F11:
			if ( ctrl )
			{
				if ( shift )
					g_pBoard->PowerOff();
				else
				{
					if ( g_pBoard->GetIsActive() )
						g_pBoard->PowerOff();
					else
						g_pBoard->PowerOn();
				}
				return;
			}
			break;
		}
	}


	if ( shift && !ctrl )
		key = akm_shift[wParam];
	else
	{
		key = akm_normal[wParam];
		if ( ctrl )
			key &= ~0x40;
	}

	if ( key != 0 )
	{
		key |= 0x80;
		if ( m_bCaps )
		{
			if ( key >= 0xC1 && key <= 0xDA )
			{
				key |= 0x20;
			}
			else if ( key >= 0xE1 && key <= 0xFA )
			{
				key &= ~0x20;
			}
		}
		m_lastKey = m_keyStrobe = key;
	}
}

void CKeyboard::AppleKeyWrite(BYTE addr, BYTE data)
{
//	if ( addr == 0x10 )		// keyStrobe
	if ( addr >= 0x10 && addr <= 0x1F )		// keyStrobe
		m_lastKey &= 0x7F;
}

BYTE CKeyboard::AppleKeyRead(BYTE addr)
{
	BYTE temp;
	if ( addr == 0x10 )		// keyStrobe
	{
		temp = m_lastKey;
		m_lastKey &= 0x7F;
		return temp;
	}
	if ( addr != 0x00 )
		return m_lastKey & 0x7F;

	return m_lastKey;
}


void CKeyboard::EnableNumKey(BOOL enable)
{
	if ( enable )
	{
		akm_normal[DIK_NUMPAD0] = '0' | 0x80;
		akm_normal[DIK_NUMPAD1] = '1' | 0x80;
		akm_normal[DIK_NUMPAD2] = '2' | 0x80;
		akm_normal[DIK_NUMPAD3] = '3' | 0x80;
		akm_normal[DIK_NUMPAD4] = '4' | 0x80;
		akm_normal[DIK_NUMPAD5] = '5' | 0x80;
		akm_normal[DIK_NUMPAD6] = '6' | 0x80;
		akm_normal[DIK_NUMPAD7] = '7' | 0x80;
		akm_normal[DIK_NUMPAD8] = '8' | 0x80;
		akm_normal[DIK_NUMPAD9] = '9' | 0x80;

		akm_shift[DIK_NUMPAD2] = 0x9C;
		akm_shift[DIK_NUMPAD4] = 0x9D;
		akm_shift[DIK_NUMPAD6] = 0x9E;
		akm_shift[DIK_NUMPAD8] = 0x9F;
	}
	else
	{
		akm_normal[DIK_NUMPAD0] = 0;
		akm_normal[DIK_NUMPAD1] = 0;
		akm_normal[DIK_NUMPAD2] = 0;
		akm_normal[DIK_NUMPAD3] = 0;
		akm_normal[DIK_NUMPAD4] = 0;
		akm_normal[DIK_NUMPAD5] = 0;
		akm_normal[DIK_NUMPAD6] = 0;
		akm_normal[DIK_NUMPAD7] = 0;
		akm_normal[DIK_NUMPAD8] = 0;
		akm_normal[DIK_NUMPAD9] = 0;

		akm_shift[DIK_NUMPAD2] = 0;
		akm_shift[DIK_NUMPAD4] = 0;
		akm_shift[DIK_NUMPAD6] = 0;
		akm_shift[DIK_NUMPAD8] = 0;
	}
}


