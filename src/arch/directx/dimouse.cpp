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
	m_Exclusive = FALSE;		// default exclusive mode is false.
	m_hMouseEvent = NULL;
	m_iMaxX = 1000;
	m_iMaxY = 1000;
	m_iMaxZ = 1000;
	m_iMinX = 0;
	m_iMinY = 0;
	m_iMinZ = 0;
	m_iX = 0;
	m_iY = 0;
	m_iZ = 0;
	m_stMouseHandler.func = NULL;
}

CDIMouse::CDIMouse(bool exclusive) :
	CCustomThread("DIMouse Thread")
{
	m_Exclusive = exclusive;
	m_hMouseEvent = NULL;
}


CDIMouse::~CDIMouse()
{
	// Shutdown and UnAcquire!
	CDIBase::Shutdown();
}

//////////////////////////////////////////////////////////////////////
//
// Create The Mouse, Should Already Be Attached.
//
//////////////////////////////////////////////////////////////////////
bool CDIMouse::InitMouse()
{
	m_iXr=0;
	m_iYr=0;
	m_iZr=0;
	ZeroMemory(m_Buttons, sizeof(m_Buttons));

	if(!m_hwnd)
	{
		TRACE("HWND Not set, Can not Initialise Mouse for this application.\nCall SetHWND before calling this method.\n");
		return false;
	}

	if(!m_lpDI)
	{
#if DIRECTINPUT_VERSION > 0x0700
		TRACE("DirectX8 Object Not Initialised in CDIMouse::InitMouse().\n");
#else
		TRACE("DirectX7 Object Not Initialised in CDIMouse::InitMouse().\n");
#endif
		return false;
	}
	// Mouse Should Always Be An Attached Device!
	// Therefore safe to assume
#if DIRECTINPUT_VERSION > 0x0700
	hr = m_lpDI->CreateDevice( GUID_SysMouse, (IDirectInputDevice8A**)&m_lpDIDevice, NULL );
#else
	hr = m_lpDI->CreateDeviceEx( GUID_SysMouse, IID_IDirectInputDevice7,
			(void**)&m_lpDIDevice, NULL);
#endif
	if FAILED(hr) 
	{ 
		TRACE("Failed in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		CDIBase::Shutdown(); 
		return false; 
	}

	// Now Set The Data Format - Mouse!
	hr = m_lpDIDevice->SetDataFormat(&c_dfDIMouse2); 
	if FAILED(hr)
	{ 
		TRACE("Failed in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		CDIBase::Shutdown(); 
		return false; 
	}

	// Set the cooperative level 
	if(!SetCoOpLevel(m_Exclusive))
	{ 
		TRACE("Failed in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		CDIBase::Shutdown(); 
		return false; 
	}

	// Now set the Buffer Data!
	DIPROPDWORD dipdw=
	{
		// The Header
		{
			sizeof(DIPROPDWORD),	// diph.dwSize
			sizeof(DIPROPHEADER),	// diph.dwHeaderSize
			NULL,					// diph.dwObj - No Object
			DIPH_DEVICE,			// diph.dwHow - Entire Device
		},
		// The Data
		DIMOUSEBUFFERSIZE
	};
	hr=m_lpDIDevice->SetProperty(DIPROP_BUFFERSIZE,&dipdw.diph);
	if FAILED(hr)
	{ 
		TRACE("Failed To Set BufferSize Property in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		CDIBase::Shutdown(); 
		return false; 
	}

	// Set the relative Axis Mode
	dipdw.dwData=DIPROPAXISMODE_REL;
	hr=m_lpDIDevice->SetProperty(DIPROP_AXISMODE,&dipdw.diph);
	if FAILED(hr)
	{ 
		TRACE("Failed To Set Relative Axis in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		CDIBase::Shutdown(); 
		return false; 
	}

	GetCapabilities();
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Update the Mouse Device Data
//
// Returns true if successfull, false if failed.
//
//////////////////////////////////////////////////////////////////////
void CDIMouse::GetCapabilities()
{
	ZeroMemory(&m_DevCaps,sizeof(m_DevCaps));
	m_DevCaps.dwSize=sizeof(m_DevCaps);
	

	if(m_lpDIDevice)
	{
		hr=m_lpDIDevice->GetCapabilities(&m_DevCaps);
		if FAILED(hr)
		{ 
			TRACE("Failed To Get Device Capabilities in CDIMouse::GetCapabilities()\n");
			TRACE(GetDIError(hr));
		}
	}

	// Has the mouse got a wheel attached?
	DIDEVICEOBJECTINSTANCE  didoi; 
 
	didoi.dwSize = sizeof(DIDEVICEOBJECTINSTANCE); 
	hr = m_lpDIDevice->GetObjectInfo(&didoi,DIMOFS_Z, DIPH_BYOFFSET);
	m_WheelAvailable = SUCCEEDED(hr);

	////////////////////////////////////////////////////////////
	//
	// Is the mouse is available, get the Granularity of the
	// Mouse Wheel
	//
	////////////////////////////////////////////////////////////
	if(m_WheelAvailable)
	{
		// If A Wheel Mouse, Get the Granularity if any
		DIPROPDWORD dipdw=
		{
			// The Header
			{
				sizeof(DIPROPDWORD),	// diph.dwSize
				sizeof(DIPROPHEADER),	// diph.dwHeaderSize
				DIMOFS_Z,			// diph.dwObj - By Offset
				DIPH_BYOFFSET,			// diph.dwHow - Offset
			},
			// The Data
			NULL
		};

		hr=m_lpDIDevice->GetProperty(DIPROP_GRANULARITY, &dipdw.diph);
		if(FAILED(hr))
		{
			TRACE("Failed To Obtain Wheel Info CDIMouse::GetCapabilities()\n");
			TRACE(GetDIError(hr));
		}
		else
		{
			TRACE("Obtained Wheel Granularity Info CDIMouse::GetCapabilities()\n");
		}

		m_Granularity=(int)dipdw.dwData;
	}
	if(!m_Granularity) m_Granularity=1;
}

//////////////////////////////////////////////////////////////////////
//
// Update the Mouse Device Data
//
// Returns true if successfull, false if failed.
//
//////////////////////////////////////////////////////////////////////
bool CDIMouse::PollDevice()
{
	static DWORD		LastClickTime=NULL;
	static int          loopcount=0;
	DWORD				dwItems=DIMOUSEBUFFERSIZE;

	if(!m_lpDIDevice)
	{
		TRACE("Mouse Object Not Created.  Quitting CDIMouse::PollDevice()\n");
		return false;
	}

	SYNC__

getBufferedData:	// Obvious Really!
	// Check Looping Breakout protection Mechanism
	if(loopcount>20)
	{
		TRACE("Infinite Loop Protection Invoked, Quitting CDIMouse::PollDevice()\n");
		CDIBase::ReInitialise();
		InitMouse();
		TRACE("Trying SetCoOpLevel and then, Quitting CDIMouse::PollDevice()\n");
		Acquire(false);
		Acquire(true);
		return false;
	}

	hr=m_lpDIDevice->Poll();
	if(FAILED(hr))
	{
		// Increment Infinite Loop Protection Counter and try again.
		loopcount++;

		TRACE("Failed To Poll Mouse in CDIMouse::PollDevice()\n");
		TRACE(GetDIError(hr));

		if ( hr == DIERR_INPUTLOST )
		{
			TRACE("Failed To Obtain Device State in CDIMouse::PollDevice()\n");
			TRACE(GetDIError(hr));

			// Try and acquire device and start again.
			if ( Acquire( true ) ) goto getBufferedData;
		}

		// We can't get the device because it has not been acquired so try and acquire it.
		if ( hr == DIERR_NOTACQUIRED )
		{
			TRACE("Device Not Acquired Trying Again CDIMouse::PollDevice()\n");
			if(!Acquire(true))
			{
				TRACE("Unable to acquire Device in CDIMouse::PollDevice() Quitting\n");
				return false;
			}
		}
		// Try and get buffered data if device is buffered!
		goto getBufferedData;
	}

	//hr=m_lpDIDevice->GetDeviceState(sizeof(m_rgdod),&m_rgdod);

	hr=m_lpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_rgdod,&dwItems,0);
	if(FAILED(hr))
	{
		if ( hr == DIERR_INPUTLOST )
		{
			TRACE("Failed To Obtain Device State in CDIMouse::PollDevice()\n");
			TRACE(GetDIError(hr));

			// Increment Infinite Loop Protection Counter and try again.
			loopcount++;

			// Try and acquire device and start again.
			if ( Acquire( true ) ) goto getBufferedData;
		}

		// We can't get the device because it has not been acquired so try and acquire it.
		if ( hr == DIERR_NOTACQUIRED )
		{
			// Increment Infinite Loop Protection Counter.
			loopcount++;

			TRACE("Device Not Acquired Trying Again CDIJoystick::PollDevice()\n");
			if(!Acquire(true))
			{
				TRACE("Unable to acquire Device in CDIJoystick::PollDevice() Quitting\n");
				return false;
			}
			// Try and get buffered data if device is buffered!
			goto getBufferedData;
		}
	}

	// Reset Position Variables!
	m_iXr=0;
	m_iYr=0;
	m_iZr=0;
	m_Left=false;
	m_Right=false;
	m_Up=false;
	m_Down=false;
	m_WheelUp=false;
	m_WheelDown=false;
	
	for(int d=0;d<(int)dwItems;d++)
	{
		switch(m_rgdod[d].dwOfs)
		{
		case DIMOFS_X: m_iXr+=m_rgdod[d].dwData;	// Update X Position
			break;
		case DIMOFS_Y: m_iYr+=m_rgdod[d].dwData;
			break;
		case DIMOFS_Z: m_iZr+=(long)m_rgdod[d].dwData/m_Granularity;
			break;

		case DIMOFS_BUTTON0: m_Button0=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[0]=m_Button0;
							 DIMOUSEDOUBLECLICK(d,0);
			break;
		case DIMOFS_BUTTON1: m_Button1=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[1]=m_Button1;
							 DIMOUSEDOUBLECLICK(d,1);
			break;
		case DIMOFS_BUTTON2: m_Button2=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[2]=m_Button2;
							 DIMOUSEDOUBLECLICK(d,2);
			break;
		case DIMOFS_BUTTON3: m_Button3=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[3]=m_Button3;
							 DIMOUSEDOUBLECLICK(d,3);
			break;
		case DIMOFS_BUTTON4: m_Button4=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[4]=m_Button4;
							 DIMOUSEDOUBLECLICK(d,4);
			break;
		case DIMOFS_BUTTON5: m_Button5=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[5]=m_Button5;
							 DIMOUSEDOUBLECLICK(d,5);
			break;
		case DIMOFS_BUTTON6: m_Button6=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[6]=m_Button6;
							 DIMOUSEDOUBLECLICK(d,6);
			break;
		case DIMOFS_BUTTON7: m_Button7=(m_rgdod[d].dwData & 0x80)?true:false;
							 m_Buttons[7]=m_Button7;
							 DIMOUSEDOUBLECLICK(d,7);
			break;

		default: TRACE("Unknown Mouse Parameter!\n");
			break;
		}
	}

	if(m_iXr<0) m_Left=true;else m_Left=false;
	if(m_iXr>0) m_Right=true;else m_Right=false;
	if(m_iYr<0) m_Up=true;else m_Up=false;
	if(m_iYr>0) m_Down=true;else m_Down=false;

	////////////////////////////////////////////////////////////
	//
	// Is a wheel available on the mouse? If so Check Status
	//
	////////////////////////////////////////////////////////////
	if(m_WheelAvailable)
	{
		hr = m_lpDIDevice->GetDeviceState(sizeof(m_MouseState),(LPVOID)&m_MouseState);
		if(FAILED(hr))
		{
			TRACE("Failed To Get Wheel Device State in CDIMouse::PollDevice()\n");
			TRACE(GetDIError(hr));
		}
		// Un-Comment this line if you want immediate data on buttons!
		//memcpy(m_Buttons,m_MouseState.rgbButtons,8);
		if( !m_iZr && m_MouseState.lZ )
				m_iZr=( m_MouseState.lZ/m_Granularity );

		m_WheelUp=(m_iZr>0)?true:false;		// when we push up the wheel, m_iZr is positive
		m_WheelDown=(m_iZr<0)?true:false;
	}

	m_iX += m_iXr;
	m_iY += m_iYr;
	m_iZ += m_iZr;
	if ( m_iX > m_iMaxX ) m_iX = m_iMaxX; else if ( m_iX < m_iMinX ) m_iX = m_iMinX;
	if ( m_iY > m_iMaxY ) m_iY = m_iMaxY; else if ( m_iY < m_iMinY ) m_iY = m_iMinY;
	if ( m_iZ > m_iMaxZ ) m_iZ = m_iMaxZ; else if ( m_iZ < m_iMinZ ) m_iZ = m_iMinZ;
	__SYNC

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Set the Co-operative Level Of The Device
//
// true = Exclusive Access, False = Background, Non-Exclusive
//
//////////////////////////////////////////////////////////////////////
bool CDIMouse::SetCoOpLevel(bool exclusive)
{
	// Set the cooperative level 
	if( !m_hwnd )
	{
		TRACE("HWND Not set in CDIMouse::SetCoOpLevel(bool exclusive)\n");
		return false;
	}
	m_Exclusive = exclusive;
	if ( !m_lpDIDevice )
		return false;

	hr = m_lpDIDevice->SetCooperativeLevel(m_hwnd, 
										   exclusive?(DISCL_FOREGROUND | DISCL_EXCLUSIVE):
													 (DISCL_BACKGROUND | DISCL_NONEXCLUSIVE));

	if FAILED(hr)
	{ 
		TRACE("Failed to Set Co-Operative Level in CDIMouse::SetCoOpLevel(bool exclusive)\n");
		TRACE("Failed in CDIMouse::InitMouse()\n");
		TRACE(GetDIError(hr));
		return false; 
	}
	TRACE("Co-Operative Level now Set.\n");
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
	CDIBase::SetHWND(hwnd);
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
			if ( !PollDevice() )
				break;
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
	HRESULT hr;
	m_hMouseEvent = ::CreateEvent( NULL, FALSE, FALSE, "MMouseEvent" );
	hr = m_lpDIDevice->SetEventNotification( m_hMouseEvent );
	if FAILED(hr)
	{
		TRACE( "Error SetEventNotification in CDIMouse::OnBeforeActivate\n" );
		TRACE( GetDIError(hr) );
		return FALSE;
	}
	::PostMessage( m_hwnd, UM_REQACQUIRE, TRUE, (LPARAM)this );
	return TRUE;
}

void CDIMouse::OnAfterDeactivate()
{
	HRESULT hr;
	::PostMessage( m_hwnd, UM_REQACQUIRE, FALSE, (LPARAM)this );
	::Sleep(0);
	hr = m_lpDIDevice->SetEventNotification( NULL );
	while ( hr == DIERR_ACQUIRED )			// not Unacquired yet
	{
		::Sleep(0);
		hr = m_lpDIDevice->SetEventNotification( NULL );
	}
	if FAILED(hr)
	{
		TRACE( "Error SetEventNotification in CDIMouse::OnBeforeActivate\n" );
		TRACE( GetDIError(hr) );
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

void CDIMouse::ClampZ(int iMinZ, int iMaxZ)
{
	if ( iMinZ < 0 || iMinZ > iMaxZ )
		return;
	m_iMaxZ = iMaxZ;
	m_iMinZ = iMinZ;
	SYNC__
		if ( m_iZ > m_iMaxZ ) m_iZ = m_iMaxZ; else if ( m_iZ < m_iMinX ) m_iZ = m_iMinZ;
	__SYNC
}

void CDIMouse::SetPosition(int iX, int iY, int iZ)
{
	SYNC__
		m_iX = iX;
		m_iY = iY;
		m_iZ = iZ;
		if ( m_iX > m_iMaxX ) m_iX = m_iMaxX; else if ( m_iX < m_iMinX ) m_iX = m_iMinX;
		if ( m_iY > m_iMaxY ) m_iY = m_iMaxY; else if ( m_iY < m_iMinY ) m_iY = m_iMinY;
		if ( m_iZ > m_iMaxZ ) m_iZ = m_iMaxZ; else if ( m_iZ < m_iMinZ ) m_iZ = m_iMinZ;
	__SYNC
}

void CDIMouse::SetMouseListener(void *objTo, callback_handler func)
{
	m_stMouseHandler.objTo = objTo;
	m_stMouseHandler.func = func;
}
