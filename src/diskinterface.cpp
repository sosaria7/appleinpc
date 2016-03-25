// DiskInterface.cpp: implementation of the CDiskInterfaceInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigdisk.h"
#include "diskinterface.h"
#include "gcr_table.h"
#include "debug.h"
#include "aipcdefs.h"
#include "appleclock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CLOCK		1020484

#define MOTOR_ON		CLOCK
#define WRITE_ON		( CLOCK / 10 )
//IMPLEMENT_DYNAMIC( CDiskInterface, CCard );
const static BYTE rom[]={
	0xA2, 0x20, 0xA0, 0x00, 0xA2, 0x03, 0x86, 0x3C,
	0x8A, 0x0A, 0x24, 0x3C, 0xF0, 0x10, 0x05, 0x3C,
	0x49, 0xFF, 0x29, 0x7E, 0xB0, 0x08, 0x4A, 0xD0,
	0xFB, 0x98, 0x9D, 0x56, 0x03, 0xC8, 0xE8, 0x10,
	0xE5, 0x20, 0x58, 0xFF, 0xBA, 0xBD, 0x00, 0x01,
	0x0A, 0x0A, 0x0A, 0x0A, 0x85, 0x2B, 0xAA, 0xBD,
	0x8E, 0xC0, 0xBD, 0x8C, 0xC0, 0xBD, 0x8A, 0xC0,
	0xBD, 0x89, 0xC0, 0xA0, 0x50, 0xBD, 0x80, 0xC0,
	0x98, 0x29, 0x03, 0x0A, 0x05, 0x2B, 0xAA, 0xBD,
	0x81, 0xC0, 0xA9, 0x56, 0x20, 0xA8, 0xFC, 0x88,
	0x10, 0xEB, 0x85, 0x26, 0x85, 0x3D, 0x85, 0x41,
	0xA9, 0x08, 0x85, 0x27, 0x18, 0x08, 0xBD, 0x8C,
	0xC0, 0x10, 0xFB, 0x49, 0xD5, 0xD0, 0xF7, 0xBD,
	0x8C, 0xC0, 0x10, 0xFB, 0xC9, 0xAA, 0xD0, 0xF3,
	0xEA, 0xBD, 0x8C, 0xC0, 0x10, 0xFB, 0xC9, 0x96,
	0xF0, 0x09, 0x28, 0x90, 0xDF, 0x49, 0xAD, 0xF0,
	0x25, 0xD0, 0xD9, 0xA0, 0x03, 0x85, 0x40, 0xBD,
	0x8C, 0xC0, 0x10, 0xFB, 0x2A, 0x85, 0x3C, 0xBD,
	0x8C, 0xC0, 0x10, 0xFB, 0x25, 0x3C, 0x88, 0xD0,
	0xEC, 0x28, 0xC5, 0x3D, 0xD0, 0xBE, 0xA5, 0x40,
	0xC5, 0x41, 0xD0, 0xB8, 0xB0, 0xB7, 0xA0, 0x56,
	0x84, 0x3C, 0xBC, 0x8C, 0xC0, 0x10, 0xFB, 0x59,
	0xD6, 0x02, 0xA4, 0x3C, 0x88, 0x99, 0x00, 0x03,
	0xD0, 0xEE, 0x84, 0x3C, 0xBC, 0x8C, 0xC0, 0x10,
	0xFB, 0x59, 0xD6, 0x02, 0xA4, 0x3C, 0x91, 0x26,
	0xC8, 0xD0, 0xEF, 0xBC, 0x8C, 0xC0, 0x10, 0xFB,
	0x59, 0xD6, 0x02, 0xD0, 0x87, 0xA0, 0x00, 0xA2,
	0x56, 0xCA, 0x30, 0xFB, 0xB1, 0x26, 0x5E, 0x00,
	0x03, 0x2A, 0x5E, 0x00, 0x03, 0x2A, 0x91, 0x26,
	0xC8, 0xD0, 0xEE, 0xE6, 0x27, 0xE6, 0x3D, 0xA5,
	0x3D, 0xCD, 0x00, 0x08, 0xA6, 0x2B, 0x90, 0xDB,
	0x4C, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL( CDiskInterface, CObject, 1 );

CDiskInterface::CDiskInterface()
{
	InitRomImage();
	for (int i = 0; i < MAX_DRIVES; i++)
		m_pDiskDrive[i] = new CDiskDrive();
	m_iDeviceNum = CARD_DISK_INTERFACE;
	m_strDeviceName = "Apple DISK ][";
	stLightHandler.func = NULL;

	m_iCurrentDisk = 0;
	m_byLatchAddr = 0;
	m_byLatchData = 0;
	m_byStepperStatus = 0;
	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
}

CDiskInterface::~CDiskInterface()
{
	for (int i = 0; i < MAX_DRIVES; i++)
	{
		if (m_pDiskDrive[i])
			delete m_pDiskDrive[i];
	}
}

void CDiskInterface::InitRomImage()
{
	m_pbyRom = (BYTE*)rom;
}


BYTE CDiskInterface::Read(WORD addr)
{
	BYTE byLatchAddr = 0;
	BYTE byBit0 = addr & 1;
	BYTE byReg = addr & 0x0F;

	byLatchAddr = m_byLatchAddr;	// remember previous latch address

	switch( byReg )
	{
	case 0x08:	/* motor off */
		if ( m_dwMotorOn == MOTOR_ON )
		{
			m_dwMotorOn--;
			g_pBoard->SpeedStable();
		}
		break;
	case 0x09:	/* motor on */
		if ( !m_dwMotorOn )
		{
			m_dwMotorOn = MOTOR_ON;		// 1 second
			m_dwWriteLED = m_pDiskDrive[m_iCurrentDisk]->IsWriteMode() ? WRITE_ON : 0;
			m_pDiskDrive[m_iCurrentDisk]->ToggleMotor( 1 );
			m_pDiskDrive[m_iCurrentDisk]->SetStepper( m_byStepperStatus );
			UpdateMotorLight();
			break;
		}
		m_dwMotorOn = MOTOR_ON;		// 1 second
		break;
	case 0x0A:	/* select driver 1 */
	case 0x0B:	/* select driver 2 */
		if ( byBit0 != m_iCurrentDisk )		// byBit0 is new disk drive
		{
			if ( m_dwMotorOn )
			{
				m_pDiskDrive[m_iCurrentDisk]->ToggleMotor( 0 );
				m_pDiskDrive[byBit0]->ToggleMotor( 1 );
				m_pDiskDrive[byBit0]->SetStepper( m_byStepperStatus );
				m_dwWriteLED = m_pDiskDrive[byBit0]->IsWriteMode() ? WRITE_ON : 0;
				m_iCurrentDisk = byBit0;
				UpdateMotorLight();
			}
			else
				m_iCurrentDisk = byBit0;
		}
		break;
	case 0x0C:		// read or write as current mode
	case 0x0D:		// write protect test
	case 0x0E:		// read
	case 0x0F:		// write
		m_byLatchAddr = byReg;
		break;
	default:	/* C0X0-C0X7, stepper toggle */
		ToggleStepper( byReg );
	}
	if ( m_dwMotorOn )		// update latch data
	{
		if ( m_dwMotorOn == MOTOR_ON && m_pDiskDrive[m_iCurrentDisk]->IsEnhanced() )
		{
			g_pBoard->SpeedUp();
		}
		m_byLatchData = m_pDiskDrive[m_iCurrentDisk]->ReadWrite( byLatchAddr, m_byLatchData );
		BOOL bWriteMode = m_pDiskDrive[m_iCurrentDisk]->IsWriteMode();
		if ( bWriteMode )		// Disk drive LED
		{
			if ( m_dwWriteLED != WRITE_ON )
			{
				m_dwWriteLED = WRITE_ON;
				UpdateMotorLight();
			}
		}
		else
			if ( m_dwWriteLED == WRITE_ON )
				m_dwWriteLED --;
	}
	if ( !byBit0 )
		return m_byLatchData;
	return 0xA0;
}


void CDiskInterface::Write(WORD addr, BYTE data)
{
	BYTE byLatchAddr;
	BYTE byBit0 = addr & 1;
	BYTE byReg = addr & 0x0F;

	byLatchAddr = m_byLatchAddr;	// remember previous latch address

	switch( addr & 0x0F )
	{
	case 0x08:	/* motor off */
		if ( m_dwMotorOn == MOTOR_ON )
			m_dwMotorOn--;
		break;
	case 0x09:	/* motor on */
		if ( !m_dwMotorOn )
		{
			m_dwMotorOn = MOTOR_ON;		// 1 second
			m_dwWriteLED = m_pDiskDrive[m_iCurrentDisk]->IsWriteMode() ? WRITE_ON : 0;
			m_pDiskDrive[m_iCurrentDisk]->ToggleMotor( 1 );
			m_pDiskDrive[m_iCurrentDisk]->SetStepper( m_byStepperStatus );
			UpdateMotorLight();
			break;
		}
		m_dwMotorOn = MOTOR_ON;		// 1 second
		break;
	case 0x0A:	/* select driver 1 */
	case 0x0B:	/* select driver 2 */
		if ( byBit0 != m_iCurrentDisk )		// byBit0 is new disk drive
		{
			if ( m_dwMotorOn )
			{
				m_pDiskDrive[m_iCurrentDisk]->ToggleMotor( 0 );
				m_pDiskDrive[byBit0]->ToggleMotor( 1 );
				m_pDiskDrive[byBit0]->SetStepper( m_byStepperStatus );
				m_dwWriteLED = m_pDiskDrive[byBit0]->IsWriteMode() ? WRITE_ON : 0;
				UpdateMotorLight();
			}
			m_iCurrentDisk = byBit0;
		}
		break;
	case 0x0C:		// read or write as current mode
	case 0x0D:		// write protect test
	case 0x0E:		// read
	case 0x0F:		// write
		m_byLatchData = data;
		m_byLatchAddr = byReg;
		break;
	default:	/* C0X0-C0X7, stepper toggle */
		ToggleStepper( byReg );
	}
	if ( m_dwMotorOn )		// write latch data
	{
		if ( m_dwMotorOn == MOTOR_ON && m_pDiskDrive[m_iCurrentDisk]->IsEnhanced() )
		{
			g_pBoard->SpeedUp();
		}
		m_byLatchData = m_pDiskDrive[m_iCurrentDisk]->ReadWrite( byLatchAddr, m_byLatchData );
		BOOL bWriteMode = m_pDiskDrive[m_iCurrentDisk]->IsWriteMode();
		if ( bWriteMode )		// Disk drive LED
		{
			if ( m_dwWriteLED != WRITE_ON )
			{
				m_dwWriteLED = WRITE_ON;
				UpdateMotorLight();
			}
		}
		else
			if ( m_dwWriteLED == WRITE_ON )
				m_dwWriteLED --;
	}
}


void CDiskInterface::Configure()
{
	CDlgConfigDisk dlgConfigDisk(this);

	dlgConfigDisk.DoModal();
}

void CDiskInterface::Reset()
{
	m_pDiskDrive[0]->Reset();
	m_pDiskDrive[1]->Reset();
	m_iCurrentDisk = 0;
	m_byLatchAddr = 0;
	m_byLatchData = 0;
	m_byStepperStatus = 0;
	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
	g_pBoard->SpeedStable();
	UpdateMotorLight();
}

void CDiskInterface::PowerOn()
{
}

void CDiskInterface::PowerOff()
{
	Reset();
}

void CDiskInterface::UpdateMotorLight()
{
	if ( stLightHandler.func )
	{
		if ( m_dwMotorOn )
		{
			int w = m_dwWriteLED ? 5 : 1;
			stLightHandler.func( this, stLightHandler.objTo, m_iCurrentDisk + w );
		}
		else
			stLightHandler.func( this, stLightHandler.objTo, 0 );
	}
}

void CDiskInterface::ToggleStepper(BYTE byOffset)
{
	int	magnet;

	magnet = ( byOffset & 0x0E ) >> 1;
	if ( byOffset & 0x01 )
		m_byStepperStatus |= (1 << magnet); 
	else
		m_byStepperStatus &= ~(1 << magnet); 
	m_pDiskDrive[m_iCurrentDisk]->SetStepper( m_byStepperStatus );
}

void CDiskInterface::Clock(int nClock)
{
	BOOL bLightChange = FALSE;
	if ( m_dwMotorOn != MOTOR_ON && m_dwMotorOn != 0 )
	{
		if ( m_dwMotorOn <= (DWORD)nClock )
		{
			m_dwMotorOn = 0;
			m_dwWriteLED = 0;
			m_pDiskDrive[m_iCurrentDisk]->ToggleMotor( 0 );
			bLightChange = TRUE;
			g_pBoard->SpeedStable();
		}
		else
			m_dwMotorOn -= nClock;
	}
	if ( m_dwWriteLED != WRITE_ON && m_dwWriteLED != 0 )
	{
		if ( m_dwWriteLED <= (DWORD)nClock )
		{
			m_dwWriteLED = 0;
			bLightChange = TRUE;
		}
		else
			m_dwWriteLED -= nClock;
	}
	if ( bLightChange )
		UpdateMotorLight();
}

void CDiskInterface::SetMotorLightHandler(void *objTo, callback_handler func)
{
	stLightHandler.objTo = objTo;
	stLightHandler.func = func;
}

void CDiskInterface::Serialize( CArchive &ar )
{
	CCard::Serialize( ar );
	int i;

	if ( ar.IsStoring() )
	{
		ar << m_iCurrentDisk;
		ar << m_byLatchAddr;
		ar << m_byLatchData;
		ar << m_byStepperStatus;
		ar << m_dwMotorOn;
		ar << m_dwWriteLED;

		for ( i = 0; i < MAX_DRIVES; i++ )
		{
			m_pDiskDrive[i]->Serialize( ar );
		}
	}
	else
	{
		ar >> m_iCurrentDisk;
		ar >> m_byLatchAddr;
		ar >> m_byLatchData;
		ar >> m_byStepperStatus;
		ar >> m_dwMotorOn;
		ar >> m_dwWriteLED;

		for ( i = 0; i < MAX_DRIVES; i++ )
		{
			m_pDiskDrive[i]->Serialize( ar );
		}
		UpdateMotorLight();
	}
}
