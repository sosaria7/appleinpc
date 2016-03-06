// DIKeyboard.h: interface for the CDIKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIKEYBOARD_H__4D4177D5_C36A_44A0_80FF_FD46D1A12647__INCLUDED_)
#define AFX_DIKEYBOARD_H__4D4177D5_C36A_44A0_80FF_FD46D1A12647__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arch/customthread.h"

class CDIKeyboard : public CCustomThread
{
public:
	CDIKeyboard();								// Constructor
	virtual ~CDIKeyboard();						// Destructor
	void SetHWND(HWND);							// Overridden SetHWND
	void SetHWND(CWnd*);						// Overridden SetHWND
	bool IsKeyPressed(unsigned char keyname);	// Has a specific DI_KEY been pressed.
	bool InitKeyboard(void);
	void Restore();

public:
	void GetDelayTime(int* pnRepeat, int* pnDelay);
	void SetDelayTime(int nRepeat, int nDelay);
	void KeyDown(USHORT key);
	void KeyUp(USHORT key);
	BOOL m_bLostKey;
	char m_buffer[256]; 
	char m_oldbuf[256];

protected:
	void ProcessKey(BOOL bRepeat);
	void OnAfterDeactivate();
	BOOL OnBeforeActivate();
	void Run(void);

	HANDLE m_hKeyboardEvent;
	BYTE m_last;
	unsigned int m_uDelay;
	unsigned int m_uRepeat;
	BOOL m_Initialised;
	HWND m_hwnd;
};

#endif // !defined(AFX_DIKEYBOARD_H__4D4177D5_C36A_44A0_80FF_FD46D1A12647__INCLUDED_)
