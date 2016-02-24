// DIMouse.h: interface for the CDIMouse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_)
#define AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arch/customthread.h"
#include "arch/directx/dibase.h"
#include "lockmgr.h"	// Added by ClassView

#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput.lib")

#define DIMOUSEBUFFERSIZE	64
#define DIMOUSEDOUBLECLICK(index,arg)	\
	if(m_Buttons[arg])					\
	{									\
		if((m_rgdod[index].dwTimeStamp-m_ButtonTime[arg])<=m_DoubleClickTime && !m_ButtonsDoubleClicked[arg])	\
		{													\
			m_ButtonsDoubleClicked[arg]=true;				\
			TRACE("You've Double Clicked\n");	\
		}													\
		else m_ButtonsDoubleClicked[arg]=false;				\
	}														\
	m_ButtonTime[arg]=m_rgdod[index].dwTimeStamp;
#define SYNCHRO(exp)			\
	{							\
		CLockMgr<CCSWrapper> guard(m_Lock,TRUE); \
		exp						\
	}
#define SYNC__					\
	{							\
		CLockMgr<CCSWrapper> guard(m_Lock,TRUE);
#define __SYNC					\
	}

class CDIMouse : public CDIBase, public CCustomThread
{
public:
	void SetMouseListener(void* objTo, callback_handler func);
	void OnAfterDeactivate();
	CDIMouse();
	CDIMouse(bool exclusive);
	virtual ~CDIMouse();

	void SetHWND(CWnd* cwnd);
	void SetHWND(HWND hwnd);
	bool PollDevice(void);
	bool SetCoOpLevel(bool exclusive);

	// Returns the Relative Movement of the device
	int  GetRelativeX(void)	{ SYNCHRO( return m_iXr; ); };
	int  GetRelativeY(void)	{ SYNCHRO( return m_iYr; ); };
	int  GetRelativeZ(void)	{ SYNCHRO( return m_iZr; ); };
	int  GetX(void) { SYNCHRO( return m_iX; ); };
	int  GetY(void) { SYNCHRO( return m_iY; ); };
	int  GetZ(void) { SYNCHRO( return m_iZ; ); };

	// Returns the State of the device
	bool IsMouseLeft(void)	{ SYNCHRO( return m_Left; ); };
	bool IsMouseRight(void)	{ SYNCHRO( return m_Right; ); };
	bool IsMouseUp(void)	{ SYNCHRO( return m_Up; ); };
	bool IsMouseDown(void)	{ SYNCHRO( return m_Down; ); };
	bool IsMouseWheelUp(void)	{ SYNCHRO( return m_WheelUp; ); };
	bool IsMouseWheelDown(void)	{ SYNCHRO( return m_WheelDown; ); };

	bool IsButton0(void) { SYNCHRO( return m_Button0; ); };
	bool IsButton1(void) { SYNCHRO( return m_Button1; ); };
	bool IsButton2(void) { SYNCHRO( return m_Button2; ); };
	bool IsButton3(void) { SYNCHRO( return m_Button3; ); };
	bool IsButton4(void) { SYNCHRO( return m_Button4; ); };
	bool IsButton5(void) { SYNCHRO( return m_Button5; ); };
	bool IsButton6(void) { SYNCHRO( return m_Button6; ); };
	bool IsButton7(void) { SYNCHRO( return m_Button7; ); };
	bool IsButtonDown(int button)
					{ SYNCHRO( return m_Buttons[button%8]; ); };
	bool IsDoubleClicked(int button)
					{ SYNCHRO( return m_ButtonsDoubleClicked[button%8]; ); };
	void ResetDoubleClick(void)
					{ SYNCHRO( ZeroMemory(&m_ButtonsDoubleClicked,sizeof(m_ButtonsDoubleClicked)); ); };
	int  GetButtonCount(void)
					{ SYNCHRO( return m_DevCaps.dwButtons; ); };
	int  GetAxisCount(void)
					{ SYNCHRO( return m_DevCaps.dwAxes; ); };
	bool InitMouse();			// Initialise the mouse.

	void ClampX(int nMinX, int nMaxX);
	void ClampY(int nMinY, int nMaxY);
	void ClampZ(int nMinZ, int nMaxZ);

	void SetPosition(int iX, int iY, int iZ);

protected:
	CCSWrapper m_Lock;
	HANDLE m_hMouseEvent;
	BOOL OnBeforeActivate();
	void Run();
	void GetCapabilities();		// Get Mouse Capabilities

	int		m_iXr;					// X Relative Movement
	int		m_iYr;					// Y Relative Movement
	int		m_iZr;					// Z Relative Movement
	int		m_iX;					// X Position
	int		m_iY;					// Y Position
	int		m_iZ;					// Z Position
	int		m_iMaxX;				// X Max Position
	int		m_iMaxY;				// Y Max Position
	int		m_iMaxZ;				// Z Max Position
	int		m_iMinX;				// X Min Position
	int		m_iMinY;				// Y Min Position
	int		m_iMinZ;				// Z Min Position

	bool m_Left;				// Is Mouse Moving Left?
	bool m_Right;				// Is Mouse Moving Right?
	bool m_Up;					// Is Mouse Moving Up?
	bool m_Down;				// Is Mouse Moving Down?
	bool m_WheelUp;				// Is Wheel Moving Away From User?
	bool m_WheelDown;			// Is Wheel Moving Towards Used?
	bool m_WheelAvailable;		// Is A Mouse Wheel Attached?
	bool m_Exclusive;			// Is Mouse Acquired Exclusively?

	bool m_Buttons[8];				// Has Mouse Button Been Pressed?
	bool m_ButtonsDoubleClicked[8];	// Has Mouse Button Been Double Clicked?
	bool m_Button0;				// Has Button 0 Been Pressed?
	bool m_Button1;				// Has Button 1 Been Pressed?
	bool m_Button2;				// Has Button 2 Been Pressed?
	bool m_Button3;				// Has Button 3 Been Pressed?
	bool m_Button4;				// Has Button 4 Been Pressed?
	bool m_Button5;				// Has Button 5 Been Pressed?
	bool m_Button6;				// Has Button 6 Been Pressed?
	bool m_Button7;				// Has Button 7 Been Pressed?

	DWORD m_ButtonTime[8];		// Used To Calculate Double Clicks!

	DIDEVICEOBJECTDATA	m_rgdod[DIMOUSEBUFFERSIZE];
	DIDEVCAPS			m_DevCaps;
	DIMOUSESTATE2		m_MouseState;
	int m_Granularity;

	STCallbackHandler	m_stMouseHandler;
};

#endif // !defined(AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_)
