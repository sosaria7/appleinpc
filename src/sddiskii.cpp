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

#define MOTOR_ON		CLOCK
#define WRITE_ON		( CLOCK / 10 )

const static BYTE rom[]={
	0xa9, 0x20, 0xa2, 0x00, 0xa9, 0x03, 0xa9, 0x3c,
	0x86, 0x46, 0x86, 0x47, 0x86, 0x44, 0xa9, 0x08,
	0x85, 0x45, 0x48, 0x8a, 0x48, 0xe8, 0x86, 0x42,
	0x20, 0x58, 0xff, 0xba, 0xbd, 0x00, 0x01, 0x0a,
	0x0a, 0x0a, 0x0a, 0xaa, 0x85, 0x2b, 0x85, 0x43,
	0xad, 0xff, 0xcf, 0xad, 0x00, 0xc8, 0xc9, 0x53,
	0xd0, 0x0a, 0xad, 0x01, 0xc8, 0xc9, 0x44, 0xd0,
	0x03, 0x20, 0x00, 0xc9, 0xa5, 0x43, 0x29, 0x70,
	0xaa, 0xbd, 0x8a, 0xc0, 0xa5, 0x43, 0x2a, 0x90,
	0x03, 0xbd, 0x8b, 0xc0, 0xa4, 0x42, 0xf0, 0x58,
	0xa5, 0xff, 0x48, 0x88, 0xf0, 0x59, 0x88, 0xd0,
	0x4a, 0xbd, 0x88, 0xc0, 0x10, 0xfb, 0xbd, 0x8c,
	0xc0, 0xc9, 0x81, 0xd0, 0xf9, 0xa9, 0x43, 0x9d,
	0x8d, 0xc0, 0xa5, 0x46, 0x9d, 0x8d, 0xc0, 0xa5,
	0x47, 0x9d, 0x8d, 0xc0, 0xa9, 0x0d, 0x9d, 0x8d,
	0xc0, 0x84, 0xff, 0xbd, 0x88, 0xc0, 0x10, 0xfb,
	0xb1, 0x44, 0x9d, 0x8d, 0xc0, 0x45, 0xff, 0x85,
	0xff, 0xc8, 0xd0, 0xf4, 0xe6, 0x45, 0xb1, 0x44,
	0x9d, 0x8d, 0xc0, 0x45, 0xff, 0x85, 0xff, 0xc8,
	0xd0, 0xf4, 0xc6, 0x45, 0xbd, 0x8d, 0xc0, 0x45,
	0xff, 0xd0, 0xb6, 0x68, 0x85, 0xff, 0x18, 0x60,
	0x18, 0x98, 0xa2, 0xff, 0xa0, 0xff, 0x60, 0xbd,
	0x88, 0xc0, 0x10, 0xfb, 0xbd, 0x8c, 0xc0, 0xc9,
	0x81, 0xd0, 0xf9, 0xa9, 0x42, 0x9d, 0x8d, 0xc0,
	0xa5, 0x46, 0x9d, 0x8d, 0xc0, 0xa5, 0x47, 0x9d,
	0x8d, 0xc0, 0xa9, 0x0d, 0x9d, 0x8d, 0xc0, 0x84,
	0xff, 0xbd, 0x88, 0xc0, 0x10, 0xfb, 0xbd, 0x8d,
	0xc0, 0x91, 0x44, 0x45, 0xff, 0x85, 0xff, 0xc8,
	0xd0, 0xf4, 0xe6, 0x45, 0xbd, 0x8d, 0xc0, 0x91,
	0x44, 0x45, 0xff, 0x85, 0xff, 0xc8, 0xd0, 0xf4,
	0xc6, 0x45, 0xbd, 0x8d, 0xc0, 0x45, 0xff, 0xd0,
	0xb6, 0xf0, 0xa8, 0xb6, 0x00, 0x00, 0x37, 0x3c,
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
	BYTE byReg = addr & 0x0F;
	BYTE byBit0 = addr & 0x01;
	BYTE byData = 0xA0;

	switch( byReg )
	{
	case 0x08:	/* status */
		//byData = 0xA0;		// negative value means GATEWAYON
		if ( !m_cHDD[m_nCurrentDisk].IsMounted() )
		{
			byData &= 0x7F;		// not ready
		}
		break;

	case 0x0A:	/* select driver 1 */
	case 0x0B:	/* select driver 2 */
		m_nCurrentDisk = byBit0;
		UpdateMotorLight();
		break;

	case 0x0C:		// data read. no address add
		m_nCurOffset = 0;
		m_byCheckSum = 0;
		m_byCommand = CMD_READY;

		byData = CMD_READY;
		if ( !m_cHDD[m_nCurrentDisk].IsMounted() )
		{
			byData &= 0x7F;		// not ready
		}

		break;
		
	case 0x0D:		// data read & write & next
		if ( m_byCommand == CMD_READ )
		{
			if ( m_nCurOffset < SDD2_HDD_BLOCK_SIZE )
			{
				byData = m_abyData[m_nCurOffset++];
				m_byCheckSum ^= byData;
			}
			else
			{
				byData = m_byCheckSum;
			}
		}
		else
		{
			if ( m_byCommand == CMD_NONE )
			{
				// ruin the checksum
				m_byCheckSum++;
			}
			byData = m_byCheckSum;
		}
		break;

	default:	/* C0X0-C0X7, not defined */
		break;
	}
	if ( !m_dwMotorOn )
	{
		m_dwMotorOn = MOTOR_ON;		// 1 second
		UpdateMotorLight();
	}
	m_dwMotorOn = MOTOR_ON;

	return byData;
}


void CSDDiskII::Write(WORD addr, BYTE data)
{
	BYTE byBit0 = addr & 1;
	BYTE byReg = addr & 0x0F;

	switch( addr & 0x0F )
	{
	case 0x08:	/* status */
		break;

	case 0x0A:	/* select driver 1 */
	case 0x0B:	/* select driver 2 */
		m_nCurrentDisk = byBit0;
		UpdateMotorLight();
		break;

	case 0x0C:		// data read. no address add
		m_nCurOffset = 0;
		m_byCheckSum = 0;
		m_byCommand = CMD_READY;
		m_dwMotorOn = MOTOR_ON;
		break;
		
	case 0x0D:		// data read & write & next
		if ( m_byCommand == CMD_READY )
		{
			if ( m_nCurOffset < SDD2_LATCH_SIZE )
			{
				m_abyLatch[m_nCurOffset++] = data;
			}
			else if ( data == CMD_END )
			{
				m_byCommand = m_abyLatch[0];
				m_nBlockNo = ( m_abyLatch[2] << 8 ) | m_abyLatch[1];
				m_nCurOffset = 0;
				if ( m_byCommand == CMD_READ )
				{
					if ( m_cHDD[m_nCurrentDisk].ReadBlock( m_nBlockNo, m_abyData ) != E_SUCCESS )
					{
						m_byCommand = CMD_NONE;
						m_byCheckSum++;
					}
				}
			}
			else
			{
				m_byCommand = CMD_NONE;
				m_nCurOffset = 0;
			}
		}
		else if ( m_byCommand == CMD_WRITE )
		{
			if ( m_nCurOffset < SDD2_HDD_BLOCK_SIZE )
			{
				m_abyData[m_nCurOffset++] = data;
				m_byCheckSum ^= data;
				if ( m_nCurOffset == SDD2_HDD_BLOCK_SIZE )
				{
					if ( m_cHDD[m_nCurrentDisk].WriteBlock( m_nBlockNo, m_abyData ) != E_SUCCESS )
					{
						// failed. ruin the checksum
						m_byCommand = CMD_NONE;
						m_byCheckSum++;
					}
				}
			}
			m_dwWriteLED = WRITE_ON;
		}
		else
		{
			m_byCommand = CMD_NONE;
			m_nCurOffset = 0;
		}

		break;
	default:	/* C0X0-C0X7, not defined */
		break;

	}
	if ( !m_dwMotorOn )
	{
		m_dwMotorOn = MOTOR_ON;		// 1 second
		UpdateMotorLight();
	}
	m_dwMotorOn = MOTOR_ON;
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
	hFile = open( pszFileName, O_RDWR | O_BINARY );
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
		close( m_hFile );
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

	long pos;
	int nReadBytes;
	if ( m_hFile == -1 )
	{
		memset( pBuffer, 0, SDD2_HDD_BLOCK_SIZE );
		return E_SUCCESS;
		//return E_READ_FAIL;
	}
	pos = lseek( m_hFile, nBlockNo * SDD2_HDD_BLOCK_SIZE, SEEK_SET );
	if ( pos < 0 )
	{
		return E_READ_FAIL;
	}

	nReadBytes = read( m_hFile, pBuffer, SDD2_HDD_BLOCK_SIZE );
	if ( nReadBytes != SDD2_HDD_BLOCK_SIZE )
	{
		return E_READ_FAIL;
	}
	return E_SUCCESS;
}

int CHDDImage::WriteBlock( int nBlockNo, void* pBuffer )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	
	long pos;
	int nWrittenBytes;
	if ( m_hFile == -1 )
	{
		return E_SUCCESS;
		//return E_WRITE_FAIL;
	}
	pos = lseek( m_hFile, nBlockNo * SDD2_HDD_BLOCK_SIZE, SEEK_SET );
	if ( pos < 0 )
	{
		return E_WRITE_FAIL;
	}
	
	nWrittenBytes = write( m_hFile, pBuffer, SDD2_HDD_BLOCK_SIZE );
	if ( nWrittenBytes != SDD2_HDD_BLOCK_SIZE )
	{
		return E_WRITE_FAIL;
	}
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
