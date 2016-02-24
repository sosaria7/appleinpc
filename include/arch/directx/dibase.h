// DIBase.h: interface for the CDIBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBASE_H__836FAAB2_7FCF_49CC_B59D_0EE9D9381021__INCLUDED_)
#define AFX_DIBASE_H__836FAAB2_7FCF_49CC_B59D_0EE9D9381021__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dinput.h>

class CDIBase  
{
public:
	bool Release();
	CDIBase();
	virtual ~CDIBase();
	virtual void SetHWND(CWnd*);		// Set the windows handle
	virtual void SetHWND(HWND);			// Set the windows handle
	virtual bool Acquire(bool);			// Acquire the device

	virtual LPCDIDEVICEINSTANCE GetFirstDeviceID();
	virtual LPCDIDEVICEINSTANCE GetNextDeviceID();
protected:
	void RemoveDeviceList(void);
	virtual bool Enumerate(DWORD dwDevType);
	static BOOL CALLBACK EnumDevicesProc(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	virtual bool AddDeviceInfo(LPCDIDEVICEINSTANCE lpddi);

	virtual bool Initialise(void);		// Initialise Direct Input
	virtual void Shutdown(void);		// Shutdown Direct Input
	
	virtual bool PollDevice(void)=NULL;	// Obtain Data From Device
	virtual bool ReInitialise(void);	// Try and reinitialise the DX7 Object
	TCHAR*  GetDIError(HRESULT);		// Get text description of DI Error

	bool		m_Initialised;	// Has Direct Input Been Successfully Initialised.
	HINSTANCE	m_hInstance;	// This Modules Instance
	HWND		m_hwnd;			// Windows Handle for this instance
	HCURSOR		m_hCursorWait;	// Windows Handle to a Wait Cursor
	HCURSOR		m_hOldCursor;	// Windows Handle to Previous Cursor

	DWORD		m_DoubleClickTime;	// Windows Double Click Time

	HRESULT					hr;				// Used for error codes in calls to DX
#if DIRECTINPUT_VERSION > 0x0700
	LPDIRECTINPUT8			m_lpDI;			// Pointer to Direct Input 8
	LPDIRECTINPUTDEVICE8	m_lpDIDevice;	// Pointer to Direct Input 8 Device
#else
	LPDIRECTINPUT7			m_lpDI;			// Pointer to Direct Input 7
	LPDIRECTINPUTDEVICE7	m_lpDIDevice;	// Pointer to Direct Input 7 Device
#endif
private:
	POSITION	m_DevicePOS;				// Used in CPtrList to keep track of next item.
	bool		m_EnumerationStarted;		// Has enumeration started and device been found?
	CPtrList	m_DIDeviceList;				// Contains a pointer list to Attached Devices
};

#endif // !defined(AFX_DIBASE_H__836FAAB2_7FCF_49CC_B59D_0EE9D9381021__INCLUDED_)
