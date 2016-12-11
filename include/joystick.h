// Joystick.h: interface for the CJoystick class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JOYSTICK_H__4DF81BD0_F4BF_4630_BCB4_82A4983195E4__INCLUDED_)
#define AFX_JOYSTICK_H__4DF81BD0_F4BF_4630_BCB4_82A4983195E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dinput.h"

#define JM_NONE			0
#define JM_KEYPAD		1
#define JM_PCJOYSTICK	2

#define JE_NOPCJOYSTICK	1
class CAppleClock;

class CJoystick : public CObject
{
public:
	CJoystick();
	virtual ~CJoystick();

	int GetDevice();
	int ChangeDevice(int nMode);
	void Strobe();
	BYTE GetStatus( BYTE num );

	void Serialize( CArchive &ar );
	void SetDeadZone( int nDeadZone );
	void SetSaturation( int nSaturation );
	int GetDeadZone();
	int GetSaturation();
	void Initialize();
	void InitPCJoystick();
	bool GetSwapButtons() { return m_bSwapButtons; }
	bool GetArrowAsPaddle() { return m_bArrowAsPaddle; }
	void SetSwapButtons(bool bSwapButtons) { m_bSwapButtons = bSwapButtons; }
	void SetArrayAsPaddle(bool bArrayAsPaddle) { m_bArrowAsPaddle = bArrayAsPaddle; }
	BOOL m_bHasPCJoystick;

protected:
	void Poll();
	BOOL m_bStrobe;
	DWORD m_dwLastClock;
	DWORD m_dwLastPoll;
	int	m_nJoystickMode;
	int m_nDeadZone;
	int m_nSaturation;
	bool m_bArrowAsPaddle;
	bool m_bSwapButtons;
};

#endif // !defined(AFX_JOYSTICK_H__4DF81BD0_F4BF_4630_BCB4_82A4983195E4__INCLUDED_)
