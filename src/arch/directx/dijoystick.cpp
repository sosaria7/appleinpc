// DIJoystick.cpp: implementation of the CDIJoystick class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/directx/dijoystick.h"
#include <dinput.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// DIJoystick.cpp: implementation of the CDIJoystick class.
//
// Written and Developed by Jason Brooks
// (C) 2000 Jason Brooks
//
// You may use this code freely, however a mention in your credits
// would be nice.
//
// For more information, Bug Reports and so on I can be contacted :-
//
// E-Mail:   DirectInput@muckypaws.com
//
// Web:      www.muckypaws.com
// ICQ:      9609400
//
//////////////////////////////////////////////////////////////////////

#define BUFFERSIZE 16

// Set the maxmimum range to which we'll gauge the swing
#define JOYMAX 10000
#define JOYMIN -10000
#define RATIO_MIN	0
#define RATIO_MAX	100

/*				Y
				
				^
				|
				|
X	-10,000 <---*---> +10,000
				|
				|
				\/
*/
// Dead zone is the amount of sway the joystick can have before we start registering movement
// In this case 10%

#define JOYDEAD 1000

// The Saturation Point Is Where the Joystick is deemed to be at Full Swing, in this case 95%
#define JOYSAT  9500

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIJoystick::CDIJoystick()
{
	// Initialise Direct Input
	CDIBase::Initialise();

	m_nNumOfButtons = 0;
	m_ButtonPOS = NULL;
	// Use default joystick
	memcpy( &m_JoystickGUID, &GUID_Joystick, sizeof( GUID ) );
}

//////////////////////////////////////////////////////////////////////
//
// Destroy The Direct Input Joystick Control and tidy up.
//
//////////////////////////////////////////////////////////////////////
CDIJoystick::~CDIJoystick()
{
	Shutdown();
}

//////////////////////////////////////////////////////////////////////
//
// Shutdown the Direct input object and release it.
//
// Basically clean up any memory allocated to this object
//
//////////////////////////////////////////////////////////////////////
void CDIJoystick::Shutdown()
{
	ClearFriendlyButtonNames();


	// Shutdown Direct Input!
	CDIBase::Shutdown();
}

//////////////////////////////////////////////////////////////////////
//
// Start the Enumeration Of Attached Joystick Devices.
//
//////////////////////////////////////////////////////////////////////
bool CDIJoystick::Enumerate()
{
#if DIRECTINPUT_VERSION > 0x0700
	return CDIBase::Enumerate(DI8DEVCLASS_GAMECTRL);
#else
	return CDIBase::Enumerate(DIDEVTYPE_JOYSTICK);
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Return First Joystick Button Name Data If Possible
//
// return  NULL if Failed or No Joysticks Available
//
//////////////////////////////////////////////////////////////////////
TCHAR* CDIJoystick::GetFirstButtonName()
{
	if(m_DIButtonNames.IsEmpty()) 
	{
		TRACE("Joysticks Have Been Enumerated However None Were Found Attached To This System\n"
						  "Therefore I am Returning NULL from CDIJoystick::GetFirstButtonName()\n");
		return NULL;
	}

	m_ButtonPOS=m_DIButtonNames.GetHeadPosition();

	TCHAR* info=static_cast<TCHAR*>(m_DIButtonNames.GetNext(m_ButtonPOS));
	return info;	
}

//////////////////////////////////////////////////////////////////////
//
// Return First Joystick Button Name Data If Possible
//
// return  NULL if Failed or No Joysticks Available
//
//////////////////////////////////////////////////////////////////////
TCHAR* CDIJoystick::GetNextButtonName()
{
	if(!m_ButtonPOS) return NULL;

	return static_cast<TCHAR*>(m_DIButtonNames.GetNext(m_ButtonPOS));
}

//////////////////////////////////////////////////////////////////////
//
// Return How Many Buttons The Attached Device Has Installed
// When giving the player an option of which joystick to use
// You may wish to evaluate the buttons available per attached device.
// Returns the number of buttons for the currently selected device.
//
//////////////////////////////////////////////////////////////////////
int CDIJoystick::HowManyButtons()
{
	return m_nNumOfButtons;
}

int CDIJoystick::CountButtons()
{
    DIDEVICEOBJECTINSTANCE didoi;
    DWORD x;
    DWORD dwOfs;
	int count=0;
	HRESULT hr=NULL;
	
	ClearFriendlyButtonNames();
	if ( !m_lpDIDevice )
	{
		TRACE( "Device has not created.\n" );
		return 0;
	}
		
	ZeroMemory(&didoi,sizeof(DIDEVICEOBJECTINSTANCE));
	didoi.dwSize = sizeof( didoi );
	
	for ( x = 0; x < 32; x++ )
	{
		dwOfs = DIJOFS_BUTTON( x ); 
		
		hr=m_lpDIDevice->GetObjectInfo( &didoi, dwOfs, DIPH_BYOFFSET );
		
		if ( SUCCEEDED( hr ) )
		{
			count++;
			TCHAR* name=new char[sizeof(didoi.tszName)];
			
			// Should include UNICODE support here.
			strcpy(name,didoi.tszName);
			
			// Add the button name to the Pointer List for future reference.
			m_DIButtonNames.AddTail(name);
		}
		else
		{
//			TRACE(GetDIError(hr));
		}
	}
	m_nNumOfButtons = count;
	return count;	// How many buttons did we find?
}

////////////////////////////////////////////////////////////////////////
//
// Shutdown the the link list of Friendly Button Names
//
////////////////////////////////////////////////////////////////////////
void CDIJoystick::ClearFriendlyButtonNames()
{
	try
	{
		if(!m_DIButtonNames.IsEmpty())
		{
			POSITION pos=m_DIButtonNames.GetHeadPosition();
			LPVOID del=NULL;

			while(pos)
			{
				del=static_cast<LPVOID>(m_DIButtonNames.GetNext(pos));
				if(del)
				{
					delete del;
				}
			}
		}
		m_DIButtonNames.RemoveAll();
	}
	catch(...)
	{
		TRACE("Some unforseen error occurred in CDIJoystick::ClearFriendlyButtonNames()\n");
	}
}

//////////////////////////////////////////////////////////////////////
//
// Initialise The Joystick!
//
//////////////////////////////////////////////////////////////////////
bool CDIJoystick::InitJoystick()
{
    // Set range. 
    // Note: range, deadzone, and saturation are being set for the
    // entire device. This could have unwanted effects on
    // sliders, dials, etc.

    DIPROPRANGE diprg; 
 
    diprg.diph.dwSize       = sizeof( diprg ); 
    diprg.diph.dwHeaderSize = sizeof( diprg.diph ); 
    diprg.diph.dwObj        = 0; 
    diprg.diph.dwHow        = DIPH_DEVICE; 
    diprg.lMin              = JOYMIN; 
    diprg.lMax              = JOYMAX; 
 
    HRESULT hr;
    DIDEVICEINSTANCE    diDeviceInstance;

	// Just a precaution when Enumerating Devices and button Names if you create
	// An options Dialog before creating your main gaming window.
	// Then simply use the desktop window, temporarily!
	if( !m_hwnd ) m_hwnd = ::GetDesktopWindow();

	if(!m_lpDI)
	{
#if DIRECTINPUT_VERSION > 0x0700
		TRACE("DirectX8 Object Not Initialised in CDIMouse::InitJoystick().\n");
#else
		TRACE("DirectX7 Object Not Initialised in CDIMouse::InitJoystick().\n");
#endif
		return false;
	}

    // Release device if it already exists.
    if ( m_lpDIDevice )
    {
        //if ( m_FFAvailable ) ReleaseEffects(); 
        
		Acquire( false );

        hr=m_lpDIDevice->Release();
		if ( FAILED(hr) )
		{
			TRACE("Error Releasing Interface in CDIJoystick::InitDevice()\n");
			TRACE( GetDIError(hr) );
		} 
		else m_lpDIDevice=NULL;
    }

	// Attempt to create the device based on the GUID passed to this routine
#if DIRECTINPUT_VERSION > 0x0700
	hr=m_lpDI->CreateDevice( m_JoystickGUID, (IDirectInputDevice8A**)&m_lpDIDevice, NULL);
#else
	hr=m_lpDI->CreateDeviceEx( m_JoystickGUID, IID_IDirectInputDevice7, (void**)&m_lpDIDevice, NULL);
#endif

	if(FAILED(hr))
	{
		TRACE("Failed to Create DI 7 interface to device in CDIJoystick::InitJoystick()\n");
		return false;
	}

    // Find out what type it is and set data format accordingly.

    diDeviceInstance.dwSize = sizeof( DIDEVICEINSTANCE );

    hr=m_lpDIDevice->GetDeviceInfo( &diDeviceInstance );
	if ( FAILED( hr ) ) 
	{
		TRACE("Failed to obtain device info in CDIJoystick::InitDevice()\n");
		TRACE(GetDIError(hr));
		return false;
	}

	// Set the data format to be a Joystick
	hr = m_lpDIDevice->SetDataFormat( &c_dfDIJoystick2 );
	if ( FAILED( hr ) ) 
	{
		TRACE("Failed to create device in CDIJoystick::InitDevice()\n");
		TRACE(GetDIError(hr));
		return false;
	}

    // Set cooperative level. 
    DWORD cl, cl1;
	cl = DISCL_EXCLUSIVE;

	// Set foreground level for release version, but use background level
	// for debugging so we don't keep losing the device when switching to 
	// a debug window. 

    //cl1 = DISCL_FOREGROUND;
	cl1 = DISCL_BACKGROUND;
#ifdef _DEBUG
    cl1 = DISCL_BACKGROUND;
#endif

	// now set the co-operation level.
	hr=m_lpDIDevice->SetCooperativeLevel( m_hwnd, cl | cl1 );
    if ( FAILED( hr ) )
    {
        TRACE( "Failed to set game device cooperative level.\n" );
		TRACE(GetDIError(hr));
        return false;
    }

    // Set up the data buffer.
    DIPROPDWORD dipdw =
    {
        // The header.
        {
            sizeof( DIPROPDWORD ),      // diph.dwSize
            sizeof( DIPROPHEADER ),     // diph.dwHeaderSize
            0,                          // diph.dwObj
            DIPH_DEVICE,                // diph.dwHow
        },
        // Number of elements in data buffer.
        BUFFERSIZE              // dwData
    };

    hr=m_lpDIDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
	if(FAILED(hr))
    {
        TRACE( "Failed to set up Data Buffer property.\n" );
		TRACE(GetDIError(hr));
        return false;
    }

	// Resest the Force Feedback Flag
    m_FFAvailable = FALSE;

    if ( FAILED( m_lpDIDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
        return FALSE; 

    // Set deadzone.
    dipdw.diph.dwSize       = sizeof( dipdw ); 
    dipdw.diph.dwHeaderSize = sizeof( dipdw.diph ); 
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = JOYDEAD;

    if ( FAILED( m_lpDIDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph ) ) )
        return FALSE;

    // Set saturation.
    dipdw.dwData            = JOYSAT;
    if ( FAILED( m_lpDIDevice->SetProperty( DIPROP_SATURATION, &dipdw.diph ) ) )
        return FALSE;

    // Find out if force feedback available.
    DIDEVCAPS didc;
    didc.dwSize = sizeof( DIDEVCAPS );
    if ( FAILED( m_lpDIDevice->GetCapabilities( &didc ) ) ) 
        return FALSE;
    m_FFAvailable = ( didc.dwFlags & DIDC_FORCEFEEDBACK );

    // If it's a force feedback stick, turn off autocenter so it doesn't
    // get in the way of our effects.
    if ( m_FFAvailable )
    {
        DIPROPDWORD DIPropAutoCenter;
        DIPropAutoCenter.diph.dwSize = sizeof( DIPropAutoCenter );
        DIPropAutoCenter.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        DIPropAutoCenter.diph.dwObj = 0;
        DIPropAutoCenter.diph.dwHow = DIPH_DEVICE;
        DIPropAutoCenter.dwData = 0;

        m_lpDIDevice->SetProperty( DIPROP_AUTOCENTER, 
                               &DIPropAutoCenter.diph );
    }
	// Count number of buttons in this Joystick and Initialise Button names 
	CountButtons();
    return TRUE;
}


//////////////////////////////////////////////////////////////////////
//
// Set the preferred GUID for the joystick device
//
//////////////////////////////////////////////////////////////////////
void CDIJoystick::SetPreferredDevice(GUID *pguid)
{
	if ( !memcmp( &m_JoystickGUID, pguid, sizeof(GUID) ) )
	{
		memcpy( &m_JoystickGUID, pguid, sizeof(GUID) );
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
//
// Update member variables to reflect the state of the device
//
//////////////////////////////////////////////////////////////////////////////////////////
bool CDIJoystick::PollDevice()
{
    int loopcount=0;
     
    HRESULT            hr;
    //DIDEVICEOBJECTDATA rgdod[BUFFERSIZE];
    //DWORD              dwItems; 

	ZeroMemory(&m_dijs,sizeof(m_dijs));

	// Has device been initialised ?
	if (!m_lpDIDevice) 
	{
		TRACE("Device has not Created in CDIJoystick::PollDevice()\n");
		return false;
	}

	hr=m_lpDIDevice->Poll();  // May be unnecessary but never hurts.
	if(FAILED(hr))
	{
		TRACE("Failed To Poll Joystick in CDIJoystick::PollDevice()\n");
		TRACE(GetDIError(hr));
	}

	while( loopcount++ <= 20 )
	{
		hr = m_lpDIDevice->GetDeviceState( sizeof( DIJOYSTATE2 ), &m_dijs ); 
		
		// The data stream was interrupted. Reacquire the device and try again.
		if ( hr == DIERR_INPUTLOST )
		{
			TRACE("Failed To Obtain Immediate Device State in CDIJoystick::PollDevice()\n");
			TRACE(GetDIError(hr));
			
			// Try and acquire device and start again.
			if ( Acquire( true ) )
				continue;
		}
		
        // We can't get the device because it has not been acquired so try and acquire it.
		if ( hr == DIERR_NOTACQUIRED )
		{
			TRACE("Device Not Acquired Trying Again immediate CDIJoystick::PollDevice()\n");
			if(!Acquire(true))
			{
				TRACE("Unable to acquire Immediate Device in CDIJoystick::PollDevice() Quitting\n");
				return false;
			}
			// Try and get buffered data if device is buffered!
			continue;
		}
		break;
	}

	if ( FAILED(hr))
    {
		TRACE("Unable to obtain Immediate Data from Device in CDIJoystick::PollDevice() Quitting\n");
		return false;
    }

	// First set immediate direction if your only interested in basic movement
    if ( m_dijs.lX < 0 ) m_JoyLeft=true; else m_JoyLeft=false;
	if ( m_dijs.lX > 0 ) m_JoyRight=true; else m_JoyRight=false;
	if ( m_dijs.lY < 0 ) m_JoyUp=true; else m_JoyUp=false;
	if ( m_dijs.lY > 0 ) m_JoyDown=true; else m_JoyDown=false;

// removed by Keonwoo Kim
// this loop may need much cpu time.
/*
	m_JoyFire1=false;

#ifdef _DEBUG
	int firecount=0;
#endif
	for(int i=0;i<sizeof(m_dijs.rgbButtons);i++)
	{
		if(m_dijs.rgbButtons[i]&0x80)
		{
#ifdef _DEBUG
			firecount++;
#endif
			m_JoyFire[i]=true;
			m_JoyFire1=true;
		}
		else m_JoyFire[i]=false;
	}
#ifdef _DEBUG
	if(firecount>0) 
	{
		TRACE("Many Buttons Pressed\n");
	}
#endif
*/
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Run the joystick Control Panel!
//
//////////////////////////////////////////////////////////////////////
void CDIJoystick::RunControlPanel()
{
	if(!m_lpDI) return;
	m_lpDI->RunControlPanel(m_hwnd,NULL);
}

void CDIJoystick::SetHWND(HWND hwnd)
{
	CDIBase::SetHWND( hwnd );
}

void CDIJoystick::SetHWND(CWnd *cwnd)
{
	SetHWND( cwnd->m_hWnd );
}

bool CDIJoystick::SetDeadZone( int nDeadZone )
{
	if ( m_lpDIDevice == NULL || nDeadZone < RATIO_MIN || nDeadZone > RATIO_MAX/5 )
	{
		return FALSE;
	}
    // Set up the data buffer.
    DIPROPDWORD dipdw =
    {
        // The header.
        {
            sizeof( DIPROPDWORD ),      // diph.dwSize
            sizeof( DIPROPHEADER ),     // diph.dwHeaderSize
            0,                          // diph.dwObj
            DIPH_DEVICE,                // diph.dwHow
        },
        // Number of elements in data buffer.
        BUFFERSIZE              // dwData
    };
    // Set deadzone.
    dipdw.diph.dwSize       = sizeof( dipdw ); 
    dipdw.diph.dwHeaderSize = sizeof( dipdw.diph ); 
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = (DWORD)( nDeadZone * JOYMAX / RATIO_MAX );

    if ( FAILED( m_lpDIDevice->SetProperty( DIPROP_DEADZONE, &dipdw.diph ) ) )
        return false;
	return true;
}

bool CDIJoystick::SetSaturation( int nSaturation )
{
	if ( m_lpDIDevice == NULL || nSaturation < RATIO_MAX/2 || nSaturation > RATIO_MAX )
	{
		return FALSE;
	}
    // Set up the data buffer.
    DIPROPDWORD dipdw =
    {
        // The header.
        {
            sizeof( DIPROPDWORD ),      // diph.dwSize
            sizeof( DIPROPHEADER ),     // diph.dwHeaderSize
            0,                          // diph.dwObj
            DIPH_DEVICE,                // diph.dwHow
        },
        // Number of elements in data buffer.
        BUFFERSIZE              // dwData
    };
    // Set saturation.
    dipdw.diph.dwSize       = sizeof( dipdw ); 
    dipdw.diph.dwHeaderSize = sizeof( dipdw.diph ); 
    dipdw.diph.dwObj        = 0;
    dipdw.diph.dwHow        = DIPH_DEVICE;
    dipdw.dwData            = (DWORD)( nSaturation * JOYMAX / RATIO_MAX );

    if ( FAILED( m_lpDIDevice->SetProperty( DIPROP_SATURATION, &dipdw.diph ) ) )
        return false;

	return true;
}
