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
	BYTE AppleKeyRead(BYTE addr);
	void AppleKeyWrite(BYTE addr, BYTE data);
	void OnKeyDown(WPARAM wParam, LPARAM lParam);
	void SetCapsLock(BOOL bCaps);
	void SetScrollLock(BOOL bScroll);
	void SetNumLock(BOOL bNumLock);
	BOOL GetScrollLock() { return m_bScroll; }
	BOOL GetCapsLock() { return m_bCaps; }
	BOOL GetNumLock() { return m_bNumLock; }
	CKeyboard();
	virtual ~CKeyboard();
protected:
	BOOL m_bCaps;
	BOOL m_bScroll;
	BOOL m_bNumLock;
	BYTE m_keyStrobe;
	BYTE m_lastKey;
	BYTE m_last;
};

#endif // !defined(AFX_KEYBOARD_H__BEC02B80_AC5F_4DC7_84B1_3D4CA5BA58C3__INCLUDED_)
