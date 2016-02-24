// Keyboard.h: interface for the CKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYBOARD_H__BEC02B80_AC5F_4DC7_84B1_3D4CA5BA58C3__INCLUDED_)
#define AFX_KEYBOARD_H__BEC02B80_AC5F_4DC7_84B1_3D4CA5BA58C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "appleclock.h"
class CAppleClock;

class CKeyboard
{
public:
	static void EnableNumKey(BOOL enable);
	BOOL GetKeyStatus( BYTE key );
	void OnKeyUp(UINT nChar, UINT nFlags);
	BYTE AppleKeyRead(BYTE addr);
	void AppleKeyWrite(BYTE addr, BYTE data);
	void OnKeyDown(WPARAM wParam, LPARAM lParam);
	CKeyboard();
	virtual ~CKeyboard();
protected:
	BOOL m_bCaps;
	BOOL m_bKeyStatus[256];
	BYTE m_keyStrobe;
	BYTE m_lastKey;
	BYTE m_last;
};

#endif // !defined(AFX_KEYBOARD_H__BEC02B80_AC5F_4DC7_84B1_3D4CA5BA58C3__INCLUDED_)
