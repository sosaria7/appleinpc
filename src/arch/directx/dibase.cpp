// DIBase.cpp: implementation of the CDIBase class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/directx/diBase.h"
#include <dinput.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDIBase::CDIBase()
{
	hr=NULL;
	m_hOldCursor=NULL;
	m_hInstance = NULL;
	m_hCursorWait=::LoadCursor(m_hInstance,IDC_WAIT);
	m_hInstance=GetModuleHandle(NULL);
	m_hwnd=NULL;
	m_Initialised=false;
	m_lpDI=NULL;
	m_lpDIDevice=NULL;
	m_EnumerationStarted=false;
	Initialise();
}

CDIBase::~CDIBase()
{
	Shutdown();
	::DestroyCursor(m_hCursorWait);
}

//////////////////////////////////////////////////////////////////////
//
// Initialise Direct Input
//
//////////////////////////////////////////////////////////////////////
bool CDIBase::Initialise()
{
	HRESULT         hr; 

#if DIRECTINPUT_VERSION > 0x0700
	hr = DirectInput8Create(m_hInstance, DIRECTINPUT_VERSION, 
			IID_IDirectInput8, (void**)&m_lpDI, NULL);
#else
	hr = DirectInputCreateEx(m_hInstance, DIRECTINPUT_VERSION, 
			IID_IDirectInput7, (void**)&m_lpDI, NULL); 
#endif
	if FAILED(hr) 
	{ 
	    // DirectInput not available; 
	    // take appropriate action
#if DIRECTINPUT_VERSION > 0x0700
		TRACE("Failed To Initialise Direct Input 8 in CDIBase::Initialise\n");
#else
		TRACE("Failed To Initialise Direct Input 7 in CDIBase::Initialise\n");
#endif
		TRACE(GetDIError(hr));
		Shutdown(); 
		return false; 
	}

	m_Initialised=true;
	return true;	// Successfully Created Direct Input 7 Object
}

bool CDIBase::Release()
{
	if (m_lpDIDevice) 
	{ 
		// Always unacquire device before calling Release().
		try
		{
			Acquire(false);
			m_lpDIDevice->Release();
			m_lpDIDevice = NULL; 
		}
		catch(...)
		{
			TRACE("Failed to Release Pointer in CDIBase::Shutdown\n");
			return false;
		}
	} 
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Shutdown the Direct input object and release it.
//
// Basically clean up any memory allocated to this object
//
//////////////////////////////////////////////////////////////////////
void CDIBase::Shutdown()
{
	RemoveDeviceList();
	// Shutdown Direct Input!
	if (m_lpDI) 
	{ 
		Release();
		try
		{
			m_lpDI->Release();
			m_lpDI = NULL; 
		}
		catch(...)
		{
#if DIRECTINPUT_VERSION > 0x0700
			TRACE("Failed to Release DI8 Pointer in CDIBase::Shutdown\n");
#else
			TRACE("Failed to Release DI7 Pointer in CDIBase::Shutdown\n");
#endif
		}
	} 
	m_Initialised=false;
}

////////////////////////////////////////////////////////////////////////////////
//
// Re-Initialise the DX7 Object
// return true=Success, false=failure
//
////////////////////////////////////////////////////////////////////////////////
bool CDIBase::ReInitialise(void)
{
	Shutdown();
	return Initialise();
}

////////////////////////////////////////////////////////////////////////////////
//
// Either Acquire or Unacquire the Device! true=Acquire, false=Unacquire
//
// Return true=Operation Successful, false if not.
//
////////////////////////////////////////////////////////////////////////////////
bool CDIBase::Acquire(bool state)
{
    HRESULT hr;

	if(!m_lpDIDevice)
	{
		TRACE("Error in CDIBase::Acquire(bool state)\nDevice Has Not Been Created.\n");
		return false;
	}

    if ( !state )  // Unacquire.
    {
        hr = m_lpDIDevice->Unacquire();
    }
    else       // Acquire.
    {
        // This could take a while with FF.
        m_hOldCursor=::SetCursor( m_hCursorWait );
        hr = m_lpDIDevice->Acquire();
		while( hr == DIERR_INPUTLOST )
			hr = m_lpDIDevice->Acquire();
        if ( SUCCEEDED( hr ) ) 
        {
			TRACE( "Successfully Acquired Device In CDIBase::Acquire\n" );
        }
		::SetCursor( m_hOldCursor );
    }

	if(FAILED(hr))
	{
		TRACE("Failed in CDIBase::Acquire(bool state)\n");
		TRACE(GetDIError(hr));
		return false;
	}

    m_DoubleClickTime = ::GetDoubleClickTime();

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// Set the windows Handle
//
//////////////////////////////////////////////////////////////////////
void CDIBase::SetHWND(HWND hwnd)
{
	m_hwnd=hwnd;
}

//////////////////////////////////////////////////////////////////////
//
// Set the windows Handle
//
//////////////////////////////////////////////////////////////////////
void CDIBase::SetHWND(CWnd *cwnd)
{
	m_hwnd=cwnd->m_hWnd;
}

//////////////////////////////////////////////////////////////////////
//
// Return Error Text From HRESULT
//
//////////////////////////////////////////////////////////////////////
TCHAR* CDIBase::GetDIError(HRESULT error)
{
	switch(error)
	{
	case E_PENDING : return _T("E_PENDING : Data Not Available.\n");
		break;
	case E_HANDLE :  return _T("E_HANDLE : The HWND parameter is not a valid top-level window that belongs to the process.\n");
		break;
	case DIERR_UNSUPPORTED : return _T("DIERR_UNSUPPORTED : The function called is not supported at this time. This value is equal to the E_NOTIMPL standard COM return value.\n");
		break;
	case DIERR_UNPLUGGED : return _T("DIERR_UNPLUGGED : The operation could not be completed because the device is not plugged in.\n");
		break;
	case DIERR_REPORTFULL : return _T("DIERR_REPORTFULL : More information was requested to be sent than can be sent to the device.\n");
		break;
	case DIERR_READONLY : return _T("DIERR_READONLY : The specified property cannot be changed. This value is equal to the E_ACCESSDENIED standard COM return value.\n");
		break;
	case DIERR_OUTOFMEMORY : return _T("DIERR_OUTOFMEMORY : The DirectInput subsystem could not allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value.\n");
		break;
//	case DIERR_OTHERAPPHASPRIO : return _T("DIERR_OTHERAPPHASPRIO : Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value. This error can be returned when an application has only foreground access to a device but is attempting to acquire the device while in the background. ");
//		break;
	case DIERR_OLDDIRECTINPUTVERSION : return _T("DIERR_OLDDIRECTINPUTVERSION : The application requires a newer version of DirectInput.\n");
		break;
	case DIERR_OBJECTNOTFOUND : return _T("DIERR_OBJECTNOTFOUND : The requested object does not exist.\n");
		break;
	case DIERR_NOTINITIALIZED : return _T("DIERR_NOTINITIALIZED : This object has not been initialized.\n");
		break;
//	case DIERR_NOTFOUND : return _T("DIERR_NOTFOUND : The requested object does not exist.\n");
//		break;
	case DIERR_NOTEXCLUSIVEACQUIRED : return _T("DIERR_NOTEXCLUSIVEACQUIRED : The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode.\n");
		break;
	case DIERR_NOTDOWNLOADED : return _T("DIERR_NOTDOWNLOADED : The effect is not downloaded.\n");
		break;
	case DIERR_NOTBUFFERED : return _T("DIERR_NOTBUFFERED : The device is not buffered. Set the DIPROP_BUFFERSIZE property to enable buffering.\n");
		break;
	case DIERR_NOTACQUIRED : return _T("DIERR_NOTACQUIRED : The operation cannot be performed unless the device is acquired.\n");
		break;
	case DIERR_NOINTERFACE : return _T("DIERR_NOINTERFACE : The specified interface is not supported by the object. This value is equal to the E_NOINTERFACE standard COM return value.\n");
		break;
	case DIERR_NOAGGREGATION : return _T("DIERR_NOAGGREGATION : This object does not support aggregation.\n");
		break;
	case DIERR_MOREDATA : return _T("DIERR_MOREDATA : Not all the requested information fit into the buffer.\n");
		break;
	case DIERR_INVALIDPARAM : return _T("DIERR_INVALIDPARAM : An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value.\n");
		break;
	case DIERR_INPUTLOST : return _T("DIERR_INPUTLOST : Access to the input device has been lost. It must be reacquired.\n");
		break;
	case DIERR_INCOMPLETEEFFECT : return _T("DIERR_INCOMPLETEEFFECT : The effect could not be downloaded because essential information is missing. For example, no axes have been associated with the effect, or no type-specific information has been supplied.\n");
		break;
//	case DIERR_HANDLEEXISTS : return _T("DIERR_HANDLEEXISTS : The device already has an event notification associated with it. This value is equal to the E_ACCESSDENIED standard COM return value.\n");
//		break;
	case DIERR_GENERIC : return _T("DIERR_GENERIC : An undetermined error occurred inside the DirectInput subsystem. This value is equal to the E_FAIL standard COM return value.\n");
		break;
	case DIERR_HASEFFECTS : return _T("DIERR_HASEFFECTS : The device cannot be reinitialized because there are still effects attached to it.\n");
		break;
	case DIERR_EFFECTPLAYING : return _T("DIERR_EFFECTPLAYING : The parameters were updated in memory but were not downloaded to the device because the device does not support updating an effect while it is still playing.\n");
		break;
	case DIERR_DEVICENOTREG : return _T("DIERR_DEVICENOTREG : The device or device instance is not registered with DirectInput. This value is equal to the REGDB_E_CLASSNOTREG standard COM return value.\n");
		break;
	case DIERR_DEVICEFULL : return _T("DIERR_DEVICEFULL : The device is full.\n");
		break;
	case DIERR_BETADIRECTINPUTVERSION : return _T("DIERR_BETADIRECTINPUTVERSION : The application was written for an unsupported prerelease version of DirectInput.\n");
		break;
	case DIERR_BADDRIVERVER : return _T("DIERR_BADDRIVERVER : The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.\n");
		break;
	case DIERR_ALREADYINITIALIZED : return _T("DIERR_ALREADYINITIALIZED : This object is already initialized\n");
		break;
	case DIERR_ACQUIRED : return _T("DIERR_ACQUIRED : The operation cannot be performed while the device is acquired.\n");
		break;
	case DI_TRUNCATEDANDRESTARTED : return _T("DI_TRUNCATEDANDRESTARTED : Equal to DI_EFFECTRESTARTED | DI_TRUNCATED\n");
		break;
	case DI_TRUNCATED : return _T("DI_TRUNCATED : The parameters of the effect were successfully updated, but some of them were beyond the capabilities of the device and were truncated to the nearest supported value.\n");
		break;
	case DI_PROPNOEFFECT : return _T("DI_PROPNOEFFECT : The change in device properties had no effect. This value is equal to the S_FALSE standard COM return value.\n");
		break;
#if DIRECTINPUT_VERSION > 0x0700
	case DI_POLLEDDEVICE : return _T("DI_POLLEDDEVICE : The device is a polled device. As a result, device buffering does not collect any data and event notifications is not signaled until the IDirectInputDevice8::Poll method is called.\n");
#else
	case DI_POLLEDDEVICE : return _T("DI_POLLEDDEVICE : The device is a polled device. As a result, device buffering does not collect any data and event notifications is not signaled until the IDirectInputDevice7::Poll method is called.\n");
#endif
		break;
	case DI_OK : return _T("DI_OK : The operation completed successfully. This value is equal to the S_OK standard COM return value.\n");
		break;
//	case DI_NOTATTACHED : return _T("DI_NOTATTACHED : The device exists but is not currently attached. This value is equal to the S_FALSE standard COM return value.\n");
//		break;
//	case DI_NOEFFECT : return _T("DI_NOEFFECT : The operation had no effect. This value is equal to the S_FALSE standard COM return value.\n");
//		break;
	case DI_EFFECTRESTARTED : return _T("DI_EFFECTRESTARTED : The effect was stopped, the parameters were updated, and the effect was restarted.\n");
		break;
	case DI_DOWNLOADSKIPPED : return _T("The parameters of the effect were successfully updated, but the effect could not be downloaded because the associated device was not acquired in exclusive mode.\n");
		break;
//	case DI_BUFFEROVERFLOW : return _T("DI_BUFFEROVERFLOW : The device buffer overflowed and some input was lost. This value is equal to the S_FALSE standard COM return value.\n");
//		break;
	default: return _T("Unknown Error Code.\n");
	}
}

// Enumerate Functions...
bool CDIBase::Enumerate(DWORD dwDevType)
{
	HRESULT hr=NULL;

	if ( !m_Initialised || !m_lpDI )
	{
		TRACE( "Direct Input hase not initialised.\n" );
		return FALSE;
	}
	m_EnumerationStarted = FALSE;
	RemoveDeviceList();

	hr=m_lpDI->EnumDevices( dwDevType, EnumDevicesProc, this, DIEDFL_ATTACHEDONLY );
    if FAILED(hr)
	{
		TRACE("Error in CDIBase::Enumerate(DWORD dwDevType)\n");
		TRACE(GetDIError(hr));
		return FALSE; 
	}

	TRACE("Enumerating Devices\n");
	return TRUE;
}

BOOL CDIBase::EnumDevicesProc(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	CDIBase *obj=(CDIBase*)(pvRef);
	obj->AddDeviceInfo(lpddi);

	return DIENUM_CONTINUE;   
}

bool CDIBase::AddDeviceInfo(LPCDIDEVICEINSTANCE lpddi)
{
	m_EnumerationStarted=true;

	LPCDIDEVICEINSTANCE lpdi2=new DIDEVICEINSTANCE;
	if(lpdi2)
	{
		memcpy( (void*)lpdi2, lpddi, sizeof(DIDEVICEINSTANCE) );

		if( m_DIDeviceList.AddHead( (void*)lpdi2) )
			return true;
		TRACE("Failed To Add Device Info in CDIBase::AddDeviceInfo(LPCDIDEVICEINSTANCE lpddi)\n");
	}
	return false;
}

LPCDIDEVICEINSTANCE CDIBase::GetFirstDeviceID()
{
	if(!m_EnumerationStarted) 
	{
		TRACE("Devices Have Not Yet Been Enumerated Returning NULL from CDIBase::GetFirstDeviceID()\n");
		return NULL;
	}

	if(m_DIDeviceList.IsEmpty()) 
	{
		TRACE("Devices Have Been Enumerated However None Were Found Attached To This System\n"
						  "Therefore I am Returning NULL from CDIBase::GetFirstDeviceID()\n");
		return NULL;
	}

	m_DevicePOS=m_DIDeviceList.GetHeadPosition();

	return GetNextDeviceID();
}

LPCDIDEVICEINSTANCE CDIBase::GetNextDeviceID()
{
	if(!m_DevicePOS) return NULL;

	return static_cast<LPCDIDEVICEINSTANCE>(m_DIDeviceList.GetNext(m_DevicePOS));
}

void CDIBase::RemoveDeviceList()
{
	// Remove Device Information
	if(!m_DIDeviceList.IsEmpty())
	{
		POSITION pos=m_DIDeviceList.GetHeadPosition();
		LPVOID del=NULL;

		while(pos)
		{
			del=static_cast<LPVOID>(m_DIDeviceList.GetNext(pos));
			if(del)
			{
				delete del;
			}
		}
		m_DIDeviceList.RemoveAll();
	}
}

