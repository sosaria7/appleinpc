// DIJoystick.h: interface for the CDIJoystick class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIJOYSTICK_H__D1950720_938F_4491_A3E3_583B1FB98E2C__INCLUDED_)
#define AFX_DIJOYSTICK_H__D1950720_938F_4491_A3E3_583B1FB98E2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dinput.lib")

#include <dinput.h>
#include "arch/directx/dibase.h"
#define MAXJOYBUTTONS 256

class CDIJoystick : public CDIBase
{
public:
	CDIJoystick();
	virtual ~CDIJoystick();

	bool	Enumerate();						// Start Enumeration of Attached Joystick Devices
	void	SetPreferredDevice(GUID* pguid);	// Set the current GUID for joystick device.
	int		HowManyButtons(void);				// Find out how many buttons the attached device has

	TCHAR*	GetFirstButtonName(void);				// Get First Joystick Button Friendly Name For Enumerated Device (Must be called before GetNextJoystickID()
	TCHAR*	GetNextButtonName();					// Get First Joystick Button Friendly Name For Enumerated Device

	bool	IsJoystickLeft(void) {return m_JoyLeft;};	// Is Joystick Being Moved Left?
	bool	IsJoystickRight(void) {return m_JoyRight;};	// Is Joystick Being Moved Right?
	bool	IsJoystickUp(void) {return m_JoyUp;};		// Is Joystick Being Moved Up?
	bool	IsJoystickDown(void) { return m_JoyDown;};	// Is Joystick Being Moved Down?
//	bool	IsJoystickFire(void) { return m_JoyFire1;};	// Has Any Fire Button Been Pressed?
//	bool	IsJoystickFire(int button) { return m_JoyFire[button%MAXJOYBUTTONS];};	// Has a specific button been pressed?
	bool	IsJoystickFire(int button)
	{
		if ( button >= sizeof( m_dijs.rgbButtons ) )
			return false;
		return ( m_dijs.rgbButtons[button] & 0x80 ) ? true : false;
	};	// Has a specific button been pressed?

	DIJOYSTATE2*	GetJoystickStateInfo(void) { return &m_dijs; };
	void	RunControlPanel(void);			// Run the Control Panel

	bool	InitJoystick(void);				// Initialise the Joystick

	// Derived from CDIBase
	void	SetHWND(CWnd *cwnd);
	void	SetHWND(HWND hwnd);
	bool	PollDevice();					// Update the device state.
	bool	SetDeadZone( int nDeadZone );
	bool	SetSaturation( int nSaturation );

protected:
	int		CountButtons(void);

	void	ClearFriendlyButtonNames(void);	// Clear Down Allocated Memory for Friendly Button Names

	// Derived from CDIBase
	void	Shutdown(void);					// Close down and de-allocate any memory assigned to this object.

private:
	
	int			m_nNumOfButtons;
	POSITION	m_ButtonPOS;		// Used in CPtrList to keep track of next item.
	DIJOYSTATE2	m_dijs;				// Holds Joystick State Information
	GUID		m_JoystickGUID;		// Current Joystick GUID
	CPtrList	m_DIButtonNames;	// Contains a pointer list to Button Names for selected Joystick

	bool m_JoyLeft;					// Generic Direction, Is Joystick Moving Left
	bool m_JoyUp;					// Generic Direction, Is Joystick Moving Up
	bool m_JoyDown;					// Generic Direction, Is Joystick Moving Down
	bool m_JoyRight;				// Generic Direction, Is Joystick Moving Right
//	bool m_JoyFire[MAXJOYBUTTONS];	// Which Fire Button Has Been Pressed?
//	bool m_JoyFire1;				// Has any fire button been pressed?
	
	

	char m_buffer[256];				// A Generic buffer


	//////////////////////////////////////////////////////////////////////
	// Used for Force Feed Back, Not Yet Implemented
	//////////////////////////////////////////////////////////////////////
	LPDIRECTINPUTEFFECT  m_lpTriggerEffect;	// For Force Feedback Effects
	LPDIRECTINPUTEFFECT  m_lpStickyEffect;	// For Force Feedback Effects
	LPDIRECTINPUTEFFECT  m_lpResistEffect;	// For Force Feedback Effects
	bool m_TriggerOK;
	BOOL m_FFAvailable;				// Is device Force Feedback Compatible?
	GUID m_TriggerGuid;				// GUID For Force Feed Back Device?
};
#endif // !defined(AFX_DIJOYSTICK_H__D1950720_938F_4491_A3E3_583B1FB98E2C__INCLUDED_)
