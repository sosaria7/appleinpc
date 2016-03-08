// sddiskii.cpp: implementation of the CSDDiskII class.
//
// Emulation for Ian's SD DISK][ emulator
// The firmware is written by Ian (SD Kim)
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigsddisk.h"
#include "sddiskii.h"
#include "debug.h"
#include "aipcdefs.h"
#include "appleclock.h"

#include <io.h>
#include <fcntl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const BYTE CSDDiskII::CMD_NONE			= 0x00;
const BYTE CSDDiskII::CMD_READ			= 0x42;
const BYTE CSDDiskII::CMD_WRITE			= 0x43;
const BYTE CSDDiskII::CMD_READY			= 0x81;
const BYTE CSDDiskII::CMD_ACK			= 0x80;
const BYTE CSDDiskII::CMD_END			= 0x0D;

const static BYTE rom[]={
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL( CSDDiskII, CObject, 1 );

CSDDiskII::CSDDiskII()
{
	InitRomImage();

	m_iDeviceNum = CARD_SD_DISK_II;
	m_strDeviceName = "SD DISK][ HDD";
	stLightHandler.func = NULL;

	memset( m_abyData, 0, SDD2_HDD_BLOCK_SIZE );
	memset( m_abyLatch, 0, SDD2_LATCH_SIZE );
	m_byCommand = CMD_NONE;
	m_byCheckSum = 0;
	m_nCurOffset = 0;

	m_nCurrentDisk = 0;
	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
}

CSDDiskII::~CSDDiskII()
{
	int i;
	for( i = 0; i < SDD2_HDD_DRIVES; i++ )
	{
		m_cHDD[i].Umount();
	}
}

void CSDDiskII::InitRomImage()
{
	m_pbyRom = (BYTE*)rom;
}


BYTE CSDDiskII::Read(WORD addr)
{
	(void)addr;
	return 0;
}


void CSDDiskII::Write(WORD addr, BYTE data)
{
	(void)addr;
	(void)data;
}


void CSDDiskII::Configure()
{
	CDlgConfigSDDisk dlgConfigDisk(this);

	dlgConfigDisk.DoModal();
}

void CSDDiskII::Reset()
{
	m_byCommand = CMD_NONE;
	m_byCheckSum = 0;
	m_nCurOffset = 0;
	
	m_nCurrentDisk = 0;

	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
	UpdateMotorLight();
}

void CSDDiskII::PowerOn()
{
}

void CSDDiskII::PowerOff()
{
	Reset();
}

void CSDDiskII::UpdateMotorLight()
{
	if ( stLightHandler.func )
	{
		if ( m_dwMotorOn )
		{
			int w = m_dwWriteLED ? 5 : 1;
			stLightHandler.func( this, stLightHandler.objTo, m_nCurrentDisk + w );
		}
		else
			stLightHandler.func( this, stLightHandler.objTo, 0 );
	}
}


void CSDDiskII::Clock(int nClock)
{
	BOOL bLightChange = FALSE;
	if ( m_dwMotorOn != 0 )
	{
		if ( m_dwMotorOn <= (DWORD)nClock )
		{
			m_dwMotorOn = 0;
			m_dwWriteLED = 0;
			bLightChange = TRUE;
		}
		else
		{
			m_dwMotorOn -= nClock;
		}
	}
	if ( m_dwWriteLED != 0 )
	{
		if ( m_dwWriteLED <= (DWORD)nClock )
		{
			m_dwWriteLED = 0;
			bLightChange = TRUE;
		}
		else
		{
			m_dwWriteLED -= nClock;
		}
	}
	if ( bLightChange )
		UpdateMotorLight();
}

void CSDDiskII::SetMotorLightHandler(void *objTo, callback_handler func)
{
	stLightHandler.objTo = objTo;
	stLightHandler.func = func;
}


void CSDDiskII::Serialize( CArchive &ar )
{
	int i;
	CCard::Serialize( ar );

	if ( ar.IsStoring() )
	{
		for( i = 0; i < SDD2_HDD_DRIVES; i++ )
		{
			m_cHDD->Serialize( ar );
		}
		ar.Write( m_abyData, SDD2_HDD_BLOCK_SIZE );
		ar.Write( m_abyLatch, SDD2_LATCH_SIZE );
		ar << m_byCommand;
		ar << m_byCheckSum;
		ar << m_nCurOffset;
		ar << m_nBlockNo;
		ar << m_dwMotorOn;
		ar << m_dwWriteLED;
		ar << m_nCurrentDisk;
	}
	else
	{
		for( i = 0; i < SDD2_HDD_DRIVES; i++ )
		{
			m_cHDD->Serialize( ar );
		}
		ar.Read( m_abyData, SDD2_HDD_BLOCK_SIZE );
		ar.Read( m_abyLatch, SDD2_LATCH_SIZE );
		ar >> m_byCommand;
		ar >> m_byCheckSum;
		ar >> m_nCurOffset;
		ar >> m_nBlockNo;
		ar >> m_dwMotorOn;
		ar >> m_dwWriteLED;
		ar >> m_nCurrentDisk;
		UpdateMotorLight();
	}
}

CHDDImage* CSDDiskII::GetDrive( int nDriveNo )
{
	if ( nDriveNo < 0 || nDriveNo >= SDD2_HDD_DRIVES )
	{
		return NULL;
	}
	return &m_cHDD[nDriveNo];
}






IMPLEMENT_SERIAL( CHDDImage, CObject, 1 );

CHDDImage::CHDDImage()
{
	m_hFile = -1;
	m_strImagePath = "";
}

CHDDImage::~CHDDImage()
{
	Umount();
}

int CHDDImage::Mount( const char* pszFileName )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	int hFile;
	Umount();
	hFile = _open( pszFileName, O_RDWR | O_BINARY );
	if ( hFile == -1 )
	{
		return E_OPEN_FAIL;
	}
	m_hFile = hFile;
	m_strImagePath = pszFileName;
	return E_SUCCESS;
}

void CHDDImage::Umount()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	if ( m_hFile != -1 )
	{
		_close( m_hFile );
		m_hFile = -1;
	}
	m_strImagePath = "";
}

BOOL CHDDImage::IsMounted()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	
	//return ( m_hFile != -1 );
	return TRUE;
}

int CHDDImage::ReadBlock( int nBlockNo, void* pBuffer )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	(void)nBlockNo;
	(void)pBuffer;

	return E_SUCCESS;
}

int CHDDImage::WriteBlock( int nBlockNo, void* pBuffer )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	return E_SUCCESS;
}

LPCTSTR CHDDImage::GetFileName()
{
	return (LPCTSTR)m_strImagePath;
}


void CHDDImage::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );
	
	if ( ar.IsStoring() )
	{
		ar << m_strImagePath;
	}
	else
	{
		ar >> m_strImagePath;
		if ( m_strImagePath != "" )
		{
			CString strImagePath = m_strImagePath;
			Mount( strImagePath );
		}
	}
}
