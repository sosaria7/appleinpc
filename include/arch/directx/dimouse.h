// DIMouse.h: interface for the CDIMouse class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_)
#define AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arch/customthread.h"
#include "lockmgr.h"	// Added by ClassView

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

class CDIMouse : public CCustomThread
{
public:
	void SetMouseListener(void* objTo, callback_handler func);
	void OnAfterDeactivate();
	CDIMouse();
	CDIMouse(bool exclusive);
	virtual ~CDIMouse();

	void SetHWND(CWnd* cwnd);
	void SetHWND(HWND hwnd);

	int  GetX(void) { SYNCHRO( return m_iX; ); };
	int  GetY(void) { SYNCHRO( return m_iY; ); };
	int  GetMaxX(void) { SYNCHRO(return m_iMaxX; ); };
	int  GetMaxY(void) { SYNCHRO(return m_iMaxY; ); };
	int  GetMinX(void) { SYNCHRO(return m_iMinX; ); };
	int  GetMinY(void) { SYNCHRO(return m_iMinY; ); };

	bool IsButton0(void) { SYNCHRO( return m_Button0; ); };
	bool IsButton1(void) { SYNCHRO( return m_Button1; ); };

	bool InitMouse();			// Initialise the mouse.

	void ClampX(int nMinX, int nMaxX);
	void ClampY(int nMinY, int nMaxY);

	void SetPosition(int iX, int iY);
	void ChangeState(int nXDelta, int nYDelta, unsigned uButtonFlag);

protected:
	CCSWrapper m_Lock;
	HANDLE m_hMouseEvent;
	HWND m_hwnd;

	BOOL OnBeforeActivate();
	void Run();

	int		m_iX;					// X Position
	int		m_iY;					// Y Position
	int		m_iMaxX;				// X Max Position
	int		m_iMaxY;				// Y Max Position
	int		m_iMinX;				// X Min Position
	int		m_iMinY;				// Y Min Position

	bool m_Button0;				// Has Button 0 Been Pressed?
	bool m_Button1;				// Has Button 1 Been Pressed?

	STCallbackHandler	m_stMouseHandler;
};

#endif // !defined(AFX_DIMOUSE_H__61EC9D6C_35D7_40A3_B4DB_FF492D4A5CFA__INCLUDED_)
