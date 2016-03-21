// hdd.cpp: implementation of the CHDDInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfighdd.h"
#include "hdd.h"
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


const BYTE CHDDInterface::CMD_NONE			= 0xFF;
const BYTE CHDDInterface::CMD_STAT			= 0x00;
const BYTE CHDDInterface::CMD_READ			= 0x01;
const BYTE CHDDInterface::CMD_WRITE			= 0x02;

const BYTE CHDDInterface::ERR_WAIT			= 0x00;
const BYTE CHDDInterface::ERR_IO			= 0x27;
const BYTE CHDDInterface::ERR_NDEV			= 0x28;
const BYTE CHDDInterface::ERR_WP			= 0x2B;
const BYTE CHDDInterface::ERR_READY			= 0x80;

#define CLOCK		1020484

#define MOTOR_ON		CLOCK
#define WRITE_ON		( CLOCK / 10 )

const static BYTE rom[]={
	0x29, 0x20, 0xa2, 0x00, 0xa5, 0x03, 0x05, 0x3c, 0xad, 0xff, 0xcf, 0x20, 0x72, 0xc9, 0xba, 0xbd,
	0x00, 0x01, 0x0a, 0x0a, 0x0a, 0x0a, 0x85, 0x2b, 0x85, 0x43, 0xa9, 0x00, 0x85, 0x46, 0x85, 0x47,
	0x85, 0x44, 0xa9, 0x08, 0x85, 0x45, 0xa9, 0x01, 0x85, 0x42, 0x20, 0x00, 0xc9, 0x4c, 0x01, 0x08,
	0xad, 0xff, 0xcf, 0x4c, 0x00, 0xc9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x17, 0x30,
	0xa5, 0x3f, 0x48, 0xa5, 0x43, 0x29, 0xf0, 0xaa, 0x0a, 0x90, 0x04, 0xa9, 0x28, 0xd0, 0x56, 0xa4,
	0x42, 0xf0, 0x60, 0x88, 0xf0, 0x74, 0xa9, 0x27, 0x88, 0xd0, 0x4a, 0xa0, 0x03, 0x98, 0x48, 0xa5,
	0x46, 0x9d, 0x82, 0xc0, 0xa5, 0x47, 0x9d, 0x83, 0xc0, 0xa9, 0x02, 0x9d, 0x80, 0xc0, 0xbd, 0x84,
	0xc0, 0xf0, 0xfb, 0x10, 0x30, 0xa0, 0x00, 0x84, 0x3f, 0xb1, 0x44, 0x9d, 0x81, 0xc0, 0x18, 0x65,
	0x3f, 0x85, 0x3f, 0xc8, 0xd0, 0xf3, 0xe6, 0x45, 0xb1, 0x44, 0x9d, 0x81, 0xc0, 0x18, 0x65, 0x3f,
	0x85, 0x3f, 0xc8, 0xd0, 0xf3, 0xc6, 0x45, 0x68, 0xa8, 0xbd, 0x81, 0xc0, 0x45, 0x3f, 0xf0, 0x0c,
	0x88, 0xd0, 0xba, 0xa9, 0x27, 0xa8, 0x68, 0x85, 0x3f, 0x98, 0x38, 0x60, 0x68, 0x85, 0x3f, 0xa9,
	0x00, 0x18, 0x60, 0xa9, 0x00, 0x9d, 0x80, 0xc0, 0xbd, 0x84, 0xc0, 0xf0, 0xfb, 0x10, 0xe6, 0xbd,
	0x83, 0xc0, 0xa8, 0xbd, 0x82, 0xc0, 0xaa, 0x18, 0x90, 0xe2, 0xa0, 0x03, 0x98, 0x48, 0xa5, 0x46,
	0x9d, 0x82, 0xc0, 0xa5, 0x47, 0x9d, 0x83, 0xc0, 0xa9, 0x01, 0x9d, 0x80, 0xc0, 0xbd, 0x84, 0xc0,
	0xf0, 0xfb, 0x10, 0xc1, 0xa0, 0x00, 0x84, 0x3f, 0xbd, 0x81, 0xc0, 0x91, 0x44, 0x18, 0x65, 0x3f,
	0x85, 0x3f, 0xc8, 0xd0, 0xf3, 0xe6, 0x45, 0xbd, 0x81, 0xc0, 0x91, 0x44, 0x18, 0x65, 0x3f, 0x85,
	0x3f, 0xc8, 0xd0, 0xf3, 0xc6, 0x45, 0x68, 0xa8, 0xbd, 0x81, 0xc0, 0x45, 0x3f, 0xf0, 0x9d, 0x88,
	0xd0, 0xba, 0xa9, 0x27, 0xd0, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_SERIAL( CHDDInterface, CObject, 1 );


#pragma pack(1)
struct S2ImgHeader
{
	UINT32	dwMagic;
	UINT32	dwCreatorID;
	UINT16	wHeaderSize;
	UINT16	wVersionNumber;
	UINT32	dwImageFormat;			// 0=DOS3.3 1=ProDos, 2=NIB
	UINT32	dwFlagsVolumeNumber;	// bit31:locked, bit8:volume number set, bit0~7: volume number
	UINT32	dwProDOSBlocks;			// 280 for 5.25"
	UINT32	dwDataOffset;			// Offset to disk data = 64 bytes
	UINT32	dwDataLength;			// Bytes of disk data = 143,360 for 5.25"
	UINT32	dwCommentOffset;		// Offset to optional Comment
	UINT32	dwCommentLength;		// Length of optional Comment
	UINT32	dwCreatorOffset;		// Offset to optional Creator data
	UINT32	dwCreatorLength;		// Length of optional Creator data
	UINT8	reserved[16];
};
#pragma pack()

CHDDInterface::CHDDInterface()
{
	InitRomImage();

	m_iDeviceNum = CARD_HDD;
	m_strDeviceName = "HDD";
	stLightHandler.func = NULL;

	memset( m_abyData, 0, HDD_BLOCK_SIZE );
	m_byCommand = CMD_NONE;
	m_byCheckSum = 0;
	m_nCurOffset = 0;

	m_nCurrentDisk = 0;
	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
	m_bHasExtendRom = TRUE;
}

CHDDInterface::~CHDDInterface()
{
	int i;
	for (i = 0; i < HDD_DRIVES_NO; i++)
	{
		m_cHDD[i].Umount();
	}
}

void CHDDInterface::InitRomImage()
{
	m_pbyRom = (BYTE*)rom;
}

BYTE CHDDInterface::ReadRom(WORD addr)
{
	if ((addr & 0xFF) == 0x05)
	{
		if (m_cHDD[m_nCurrentDisk].IsMounted() == FALSE)
			return 0;
	}
	return CCard::ReadRom(addr);
}

BYTE CHDDInterface::ReadExRom(WORD addr)
{
	addr &= 0x7FF;		// c800~cfff
	if (addr >= sizeof(rom))
	{
		return 0;
	}
	return rom[addr];
}

BYTE CHDDInterface::Read(WORD addr)
{
	BYTE byReg = addr & 0x0F;
	BYTE byBit0 = addr & 0x01;
	BYTE byData = 0xA0;

	switch (byReg)
	{
	case 0x00:		// IO_CMD
		byData = m_byCommand;
		break;

	case 0x01:		// IO_DATA
		if (m_nCurOffset >= HDD_BLOCK_SIZE)
		{
			byData = m_byCheckSum;
			break;
		}
		if (m_byCommand != CMD_READ)
		{
			m_byStatus = ERR_IO;
			break;
		}
		MotorOn(FALSE);
		byData = m_abyData[m_nCurOffset++];
		m_byCheckSum = (BYTE)(m_byCheckSum + byData);
		break;

	case 0x02:		// BLOCK_L
		byData = (BYTE)m_nBlockNo;
		break;

	case 0x03:		// BLOCK_H
		byData = (BYTE)(m_nBlockNo >> 8);
		break;

	case 0x04:		// STATUS
		byData = m_byStatus;
		break;

	default:
		m_byStatus = ERR_IO;
		break;
	}

	return byData;
}


void CHDDInterface::Write(WORD addr, BYTE data)
{
	BYTE byBit0 = addr & 1;
	BYTE byReg = addr & 0x0F;

	switch (addr & 0x0F)
	{
	case 0x00:		// IO_CMD
		m_byCommand = data;
		switch (m_byCommand)
		{
		case CMD_STAT:
			if (m_cHDD[m_nCurrentDisk].IsMounted())
			{
				m_nBlockNo = m_cHDD[m_nCurrentDisk].GetBlockCount();
				m_byStatus = ERR_READY;
			}
			else
			{
				m_nBlockNo = 0;
				m_byStatus = ERR_NDEV;
			}
			break;
		case CMD_READ:
			MotorOn(FALSE);
			if (m_cHDD[m_nCurrentDisk].ReadBlock(m_nBlockNo, m_abyData) == E_SUCCESS)
			{
				m_byStatus = ERR_READY;
			}
			else
			{
				m_byStatus = ERR_IO;
			}
			m_nCurOffset = 0;
			m_byCheckSum = 0;
			break;
		case CMD_WRITE:
			if (m_cHDD[m_nCurrentDisk].IsWriteProtected() == TRUE)
			{
				m_byStatus = ERR_WP;
			}
			else
			{
				MotorOn(FALSE);
				m_nCurOffset = 0;
				m_byCheckSum = 0;
				m_byStatus = ERR_READY;
			}
			break;
		default:
			m_byStatus = ERR_IO;
			break;
		}
		break;
	case 0x01:		// IO_DATA
		if (m_byCommand != CMD_WRITE || m_nCurOffset >= HDD_BLOCK_SIZE)
		{
			m_byStatus = ERR_IO;
			break;
		}
		if (m_cHDD[m_nCurrentDisk].IsWriteProtected() == TRUE)
		{
			m_byStatus = ERR_WP;
			break;
		}
		MotorOn(TRUE);
		m_abyData[m_nCurOffset++] = data;
		m_byCheckSum = (BYTE)(m_byCheckSum + data);
		if (m_nCurOffset == HDD_BLOCK_SIZE)
		{
			if (m_cHDD[m_nCurrentDisk].WriteBlock(m_nBlockNo, m_abyData) != E_SUCCESS)
			{
				m_byStatus = ERR_IO;
				m_byCheckSum++;		// ruin checksum
				break;
			}
		}
		break;
	case 0x02:		// BLOCK_L
		m_nBlockNo = (m_nBlockNo & 0xFF00) | data;
		break;
	case 0x03:
		m_nBlockNo = (m_nBlockNo & 0x00FF) | (data << 8);
		break;
	case 0x04:
		break;
	default:
		break;
	}
}


void CHDDInterface::Configure()
{
	CDlgConfigHDD dlgConfigDisk(this);

	dlgConfigDisk.DoModal();
}

void CHDDInterface::Reset()
{
	m_byCommand = CMD_NONE;
	m_byCheckSum = 0;
	m_nCurOffset = 0;
	
	m_nCurrentDisk = 0;

	m_dwMotorOn = 0;
	m_dwWriteLED = 0;
	UpdateMotorLight();
}

void CHDDInterface::PowerOn()
{
}

void CHDDInterface::PowerOff()
{
	Reset();
}

void CHDDInterface::MotorOn(BOOL bWrite)
{
	BOOL bLightChanged = FALSE;
	if (bWrite == TRUE)
	{
		if (m_dwWriteLED == 0)
		{
			bLightChanged = TRUE;
		}
		m_dwWriteLED = WRITE_ON;
	}
	if (!m_dwMotorOn)
	{
		bLightChanged = TRUE;
	}
	m_dwMotorOn = MOTOR_ON;

	if (bLightChanged)
		UpdateMotorLight();
}

void CHDDInterface::UpdateMotorLight()
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


void CHDDInterface::Clock(int nClock)
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

void CHDDInterface::SetMotorLightHandler(void *objTo, callback_handler func)
{
	stLightHandler.objTo = objTo;
	stLightHandler.func = func;
}


void CHDDInterface::Serialize( CArchive &ar )
{
	int i;
	CCard::Serialize( ar );

	if ( ar.IsStoring() )
	{
		for( i = 0; i < HDD_DRIVES_NO; i++ )
		{
			m_cHDD->Serialize( ar );
		}
		ar.Write( m_abyData, HDD_BLOCK_SIZE );
		ar << m_byCommand;
		ar << m_byCheckSum;
		ar << m_byStatus;
		ar << m_nCurOffset;
		ar << m_nBlockNo;
		ar << m_dwMotorOn;
		ar << m_dwWriteLED;
		ar << m_nCurrentDisk;
	}
	else
	{
		for( i = 0; i < HDD_DRIVES_NO; i++ )
		{
			m_cHDD->Serialize( ar );
		}
		ar.Read( m_abyData, HDD_BLOCK_SIZE );
		ar >> m_byCommand;
		ar >> m_byCheckSum;
		ar >> m_byStatus;
		ar >> m_nCurOffset;
		ar >> m_nBlockNo;
		ar >> m_dwMotorOn;
		ar >> m_dwWriteLED;
		ar >> m_nCurrentDisk;
		UpdateMotorLight();
	}
}

CHDDImage* CHDDInterface::GetDrive( int nDriveNo )
{
	if (nDriveNo < 0 || nDriveNo >= HDD_DRIVES_NO)
	{
		return NULL;
	}
	return &m_cHDD[nDriveNo];
}




const static BYTE sg_abyDOSLogicalOrder[] = {
	0x00, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x0F
};

IMPLEMENT_SERIAL( CHDDImage, CObject, 1 );

CHDDImage::CHDDImage()
{
	m_hFile = -1;
	m_strImagePath = "";
	m_uDataOffset = 0;
	m_uDataLength = 0;
	m_nBlockCount = 0;
	m_bIsWriteProtected = 0;
}

CHDDImage::~CHDDImage()
{
	Umount();
}

int CHDDImage::Mount( const char* pszFileName )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	int hFile;
	BYTE sig[4];
	int nRead;
	S2ImgHeader st2mgHeader;

	Umount();

	m_bIsWriteProtected = FALSE;

	hFile = _open(pszFileName, O_RDWR | O_BINARY);

	if (hFile == -1)
	{
		hFile = _open(pszFileName, O_RDONLY | O_BINARY);
		if (hFile == -1)
			return E_OPEN_FAIL;
		m_bIsWriteProtected = TRUE;
	}

	m_uDataOffset = 0;
	m_uDataLength = (UINT)_lseek(hFile, 0, SEEK_END);

	_lseek(hFile, 0, SEEK_SET);
	nRead = _read(hFile, sig, 4);
	// 2mg image
	if (nRead == 4 && memcmp(sig, "2IMG", 4) == 0)
	{
		_lseek(hFile, 0, SEEK_SET);
		nRead = _read(hFile, &st2mgHeader, sizeof(st2mgHeader));
		if (nRead != sizeof(st2mgHeader))
		{
			_close(hFile);
			return E_READ_FAIL;
		}
		if (st2mgHeader.dwImageFormat != IMAGE_PRODOS && st2mgHeader.dwImageFormat != IMAGE_DOS)
		{
			_close(hFile);
			return E_UNKNOWN_FORMAT;
		}
		m_uDataOffset = st2mgHeader.dwDataOffset;
		m_uDataLength = st2mgHeader.dwDataLength;
		if ((st2mgHeader.dwFlagsVolumeNumber & 0x100) != 0)
			m_uVolumeNo = st2mgHeader.dwFlagsVolumeNumber & 0xFF;
		if ((st2mgHeader.dwFlagsVolumeNumber & 0x80000000) != 0)
			m_bIsWriteProtected = TRUE;
	}
	m_nBlockCount = (int)(m_uDataLength / HDD_BLOCK_SIZE);

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
	m_nBlockCount = 0;
}

BOOL CHDDImage::IsMounted()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	
	return ( m_hFile != -1 );
}

int CHDDImage::ReadBlock( int nBlockNo, void* pBuffer )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	long pos;
	int nReadBytes;
	if ( m_hFile == -1 )
	{
		memset( pBuffer, 0, HDD_BLOCK_SIZE );
		return E_READ_FAIL;
	}

	TRACE("read hdd block: %d\n", nBlockNo);
	pos = _lseek( m_hFile, m_uDataOffset + nBlockNo * HDD_BLOCK_SIZE, SEEK_SET );
	if ( pos < 0 )
	{
		return E_READ_FAIL;
	}

	nReadBytes = _read( m_hFile, pBuffer, HDD_BLOCK_SIZE );
	if ( nReadBytes != HDD_BLOCK_SIZE )
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
	if (m_hFile == -1 || m_bIsWriteProtected == TRUE)
	{
		return E_WRITE_FAIL;
	}
	pos = _lseek( m_hFile, m_uDataOffset + nBlockNo * HDD_BLOCK_SIZE, SEEK_SET );
	if ( pos < 0 )
	{
		return E_WRITE_FAIL;
	}
	
	nWrittenBytes = _write( m_hFile, pBuffer, HDD_BLOCK_SIZE );
	if ( nWrittenBytes != HDD_BLOCK_SIZE )
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
