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
	m_bCaps = TRUE;
	m_bScroll = FALSE;
	m_bNumLock = FALSE;
	m_pszAutoKeyData = NULL;
	m_nAutoKeyOffset = 0;
}

CKeyboard::~CKeyboard()
{
	if (m_pszAutoKeyData != NULL)
		delete m_pszAutoKeyData;
}

#define KEYDOWN(key)	( g_cDIKeyboard.IsKeyPressed( key ) )

void CKeyboard::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	int key;

	BOOL ctrl = KEYDOWN( DIK_LCONTROL ) || KEYDOWN( DIK_RCONTROL );
	BOOL alt = KEYDOWN( DIK_LMENU ) || KEYDOWN( DIK_RMENU );
	BOOL shift = KEYDOWN( DIK_LSHIFT ) || KEYDOWN( DIK_RSHIFT );
	CJoystick *joy;
	joy = &g_pBoard->m_joystick;
	bool bArrowAsPaddle = joy->GetArrowAsPaddle() && m_bScroll;

	if ( lParam == 0 )		// only for key down, not for repeat
	{
		switch( wParam )
		{
		case DIK_CAPITAL:
			SetCapsLock(!m_bCaps);
			return;
		case DIK_SCROLL:
			SetScrollLock(!m_bScroll);
			break;
		case DIK_NUMLOCK:
			SetNumLock(!m_bNumLock);
			break;
		case 0xC6:			// control+pause : break
			g_pBoard->Reset();
			return;

		case DIK_F8:
			if ( ctrl )		// Toggle Full Screen mode
			{
				::PostMessage( g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_FULL_SCREEN, 0 );
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

		case DIK_F5:
			::PostMessage(g_pBoard->m_lpwndMainFrame->m_hWnd, WM_COMMAND, ID_SUSPENDRESUME, 0);
			break;

		default:
			break;
		}
	}

	switch (wParam)
	{
	case DIK_LEFT:
	case DIK_RIGHT:
	case DIK_DOWN:
	case DIK_UP:
		if (bArrowAsPaddle && akm_shift[DIK_NUMPAD2] == 0)
		{
			return;
		}
		break;
	default:
		break;
	}

	if ( shift )
		key = akm_shift[wParam];
	else
		key = akm_normal[wParam];

	if ( ctrl )
	{
		if (key >= ('a' + 0x80) && key <= ('z' + 0x80))
		{
			key = akm_normal[wParam];
		}
		if (key >= ('A' + 0x80) && key <= ('Z' + 0x80))
		{
			key ^= 0x40;
		}
		switch (key)
		{
		case ('\\' + 0x80):		// 0x5c -> 0x1c
		case ('^' + 0x80):		// 0x5e -> 0x1e
		case ('_' + 0x80):		// 0x5f -> 0x1f
			key ^= 0x40;
			break;
		case ('[' + 0x80):		// 0x5b -> 0x1b
		case ('{' + 0x80):		// 0x7b -> 0x1b
			key = 0x9b;
			break;
		case (']' + 0x80):		// 0x5d -> 0x1d
		case ('}' + 0x80):		// 0x7d -> 0x1d
			key = 0x9d;
			break;
		case ('|' + 0x80):
			key = 0xff;
			break;
		case ('2' + 0x80):
		case ('@' + 0x80):
			key = 0x80;
			break;
		default:
			// no effect with ctrl-key
			break;
		}
	}

	if ( key != 0 && m_pszAutoKeyData == NULL )
	{
		key |= 0x80;
		if ( !m_bCaps )
		{
			if ( key >= ('A' + 0x80) && key <= ('Z' + 0x80))
			{
				key |= 0x20;
			}
			else if (key >= ('a' + 0x80) && key <= ('z' + 0x80))
			{
				key &= ~0x20;
			}
		}
		m_lastKey = (BYTE)key;
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
		akm_normal[DIK_NUMPADCOMMA] = ',' | 0X80;
		akm_normal[DIK_MULTIPLY] = '*' | 0X80;
		akm_normal[DIK_SUBTRACT] = '-' | 0X80;
		akm_normal[DIK_ADD] = '+' | 0X80;
		akm_normal[DIK_DECIMAL] = '.' | 0X80;
		akm_normal[DIK_DIVIDE] = '/' | 0X80;

		akm_normal[DIK_NUMPADENTER] = akm_normal[DIK_RETURN];

		akm_shift[DIK_NUMPAD2] = 0x9C;
		akm_shift[DIK_NUMPAD4] = 0x9D;
		akm_shift[DIK_NUMPAD6] = 0x9E;
		akm_shift[DIK_NUMPAD8] = 0x9F;
		akm_shift[DIK_NUMPADENTER] = akm_shift[DIK_RETURN];
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
		akm_normal[DIK_NUMPADENTER] = 0;
		akm_normal[DIK_NUMPADCOMMA] = 0;
		akm_normal[DIK_MULTIPLY] = 0;
		akm_normal[DIK_SUBTRACT] = 0;
		akm_normal[DIK_ADD] = 0;
		akm_normal[DIK_DECIMAL] = 0;
		akm_normal[DIK_DIVIDE] = 0;

		akm_shift[DIK_NUMPAD2] = 0;
		akm_shift[DIK_NUMPAD4] = 0;
		akm_shift[DIK_NUMPAD6] = 0;
		akm_shift[DIK_NUMPAD8] = 0;
		akm_shift[DIK_NUMPADENTER] = 0;
	}
}


void CKeyboard::SetCapsLock(BOOL bCaps)
{
	m_bCaps = bCaps;
	g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetKeyStatus(KEY_STATE_CAPS, bCaps);
}

void CKeyboard::SetScrollLock(BOOL bScroll)
{
	m_bScroll = bScroll;
	g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetKeyStatus(KEY_STATE_SCROLL, bScroll);
}

void CKeyboard::SetNumLock(BOOL bNumLock)
{
	m_bNumLock = bNumLock;

	CKeyboard::EnableNumKey(m_bNumLock);
	g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetKeyStatus(KEY_STATE_NUMLOCK, bNumLock);
}

void CKeyboard::Clock(DWORD clock)
{
	char ch = 0;
	if (m_pszAutoKeyData != NULL)
	{
		// wait until Key Strobe goes off
		if (!(m_lastKey & 0x80))
		{
			CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
			if (m_pszAutoKeyData != NULL)
			{
				while (TRUE)
				{
					ch = m_pszAutoKeyData[m_nAutoKeyOffset++];

					if (ch >= 0x01 && ch <= 0x7F && ch != '\n')
					{
						m_lastKey = ch | 0x80;
					}
					else if (ch == 0)
					{
						delete m_pszAutoKeyData;
						m_pszAutoKeyData = NULL;
						m_nAutoKeyOffset = 0;
					}
					else
					{
						// skip character
						continue;
					}
					break;
				}
			}
		}
	}

}

void CKeyboard::SetAutoKeyData(char* pszAutoKeyData)
{
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
	if (m_pszAutoKeyData != NULL)
	{
		delete m_pszAutoKeyData;
		m_pszAutoKeyData = NULL;
		m_nAutoKeyOffset = 0;
	}
	if (pszAutoKeyData != NULL)
	{
		m_pszAutoKeyData = new char[strlen(pszAutoKeyData) + 1];
		strcpy(m_pszAutoKeyData, pszAutoKeyData);
	}
}

BOOL CKeyboard::IsOnAutoKeyData()
{
	return m_pszAutoKeyData != NULL;
}