// DIMouse.cpp: implementation of the CDIMouse class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/directx/dimouse.h"
#include "aipcdefs.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIMouse::CDIMouse() :
	CCustomThread("DIMouse Thread")
{
	m_hMouseEvent = NULL;
	m_iMaxX = 1000;
	m_iMaxY = 1000;
	m_iMinX = 0;
	m_iMinY = 0;
	m_iX = 0;
	m_iY = 0;
	m_Button0 = FALSE;
	m_Button1 = FALSE;
	m_hwnd = NULL;
	m_stMouseHandler.func = NULL;
}

CDIMouse::CDIMouse(bool exclusive) :
	CCustomThread("DIMouse Thread")
{
	m_hMouseEvent = NULL;
}


CDIMouse::~CDIMouse()
{
	// Shutdown and UnAcquire!
	this->SetActive(FALSE, TRUE);
}

//////////////////////////////////////////////////////////////////////
//
// Create The Mouse, Should Already Be Attached.
//
//////////////////////////////////////////////////////////////////////
bool CDIMouse::InitMouse()
{
	if(!m_hwnd)
	{
		TRACE("HWND Not set, Can not Initialise Mouse for this application.\nCall SetHWND before calling this method.\n");
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
//
// Set the Windows Handle to which the Mouse Input will be attached
//
// Used for Win32 based applications without MFC
//
//////////////////////////////////////////////////////////////////////
void CDIMouse::SetHWND(HWND hwnd)
{
	this->m_hwnd = hwnd;
}

//////////////////////////////////////////////////////////////////////
//
// Set the Windows Handle to which the Mouse Input will be attached
//
// Used for MFC based applications.
//
//////////////////////////////////////////////////////////////////////
void CDIMouse::SetHWND(CWnd *cwnd)
{
	SetHWND( cwnd->m_hWnd );
}

void CDIMouse::Run()
{
	DWORD dwObject;
	HANDLE hEvents[] = { GetShutdownEvent(), m_hMouseEvent };
	while( TRUE )
	{
		dwObject = ::WaitForMultipleObjects( 2, hEvents, FALSE, INFINITE );
		if ( dwObject == WAIT_OBJECT_0 )
			break;
		else if ( dwObject == WAIT_OBJECT_0 + 1 )
		{
			::PostMessage( m_hwnd, UM_MOUSE_EVENT, 0, (LPARAM)this );
			SYNC__
				if ( m_stMouseHandler.func )
					m_stMouseHandler.func( this, m_stMouseHandler.objTo, 0 );
			__SYNC
			::Sleep(20);
		}
		else
			break;
	}
}


BOOL CDIMouse::OnBeforeActivate()
{
	m_hMouseEvent = ::CreateEvent( NULL, FALSE, FALSE, "MMouseEvent" );

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = m_hwnd;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		TRACE("CDIKeyboard::OnBeforeActivate() RegisterRawInputDevices() : %08x\n", GetLastError());
		return FALSE;
	}

	::PostMessage(m_hwnd, UM_REQACQUIRE, TRUE, (LPARAM)this);

	return TRUE;
}

void CDIMouse::OnAfterDeactivate()
{
	::PostMessage(m_hwnd, UM_REQACQUIRE, FALSE, (LPARAM)this);

	RAWINPUTDEVICE Rid[1];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = RIDEV_REMOVE;   // re HID mouse
	Rid[0].hwndTarget = 0;

	m_Button0 = FALSE;
	m_Button1 = FALSE;

	if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) {
		//registration failed. Call GetLastError for the cause of the error
		TRACE("CDIKeyboard::OnBeforeActivate() RegisterRawInputDevices() : %08x\n", GetLastError());
	}

	CloseHandle( m_hMouseEvent );
}


void CDIMouse::ClampX(int iMinX, int iMaxX)
{
	if ( iMinX < 0 || iMinX > iMaxX )
		return;
	m_iMaxX = iMaxX;
	m_iMinX = iMinX;
	SYNC__
		if ( m_iX > m_iMaxX ) m_iX = m_iMaxX; else if ( m_iX < m_iMinX ) m_iX = m_iMinX;
	__SYNC
}

void CDIMouse::ClampY(int iMinY, int iMaxY)
{
	if ( iMinY < 0 || iMinY > iMaxY )
		return;
	m_iMaxY = iMaxY;
	m_iMinY = iMinY;
	SYNC__
		if ( m_iY > m_iMaxY ) m_iY = m_iMaxY; else if ( m_iY < m_iMinX ) m_iY = m_iMinY;
	__SYNC
}

void CDIMouse::SetPosition(int iX, int iY)
{
	SYNC__
		m_iX = iX;
		m_iY = iY;
		if ( m_iX > m_iMaxX ) m_iX = m_iMaxX; else if ( m_iX < m_iMinX ) m_iX = m_iMinX;
		if ( m_iY > m_iMaxY ) m_iY = m_iMaxY; else if ( m_iY < m_iMinY ) m_iY = m_iMinY;
	__SYNC
}

void CDIMouse::ChangeState(int nXDelta, int nYDelta, unsigned uButtonFlag)
{
	SYNC__
	m_iX += nXDelta;
	m_iY += nYDelta;
	if (m_iX > m_iMaxX) m_iX = m_iMaxX; else if (m_iX < m_iMinX) m_iX = m_iMinX;
	if (m_iY > m_iMaxY) m_iY = m_iMaxY; else if (m_iY < m_iMinY) m_iY = m_iMinY;
	if (uButtonFlag & RI_MOUSE_LEFT_BUTTON_DOWN)
	{
		m_Button0 = TRUE;
	}
	if (uButtonFlag & RI_MOUSE_LEFT_BUTTON_UP)
	{
		m_Button0 = FALSE;
	}
	if (uButtonFlag & RI_MOUSE_RIGHT_BUTTON_DOWN)
	{
		m_Button1 = TRUE;
	}
	if (uButtonFlag & RI_MOUSE_RIGHT_BUTTON_UP)
	{
		m_Button1 = FALSE;
	}
	if (uButtonFlag & RI_MOUSE_MIDDLE_BUTTON_UP)
	{
		SetActive(FALSE);
	}
	SetEvent(m_hMouseEvent);
	__SYNC
}

void CDIMouse::SetMouseListener(void *objTo, callback_handler func)
{
	m_stMouseHandler.objTo = objTo;
	m_stMouseHandler.func = func;
}
