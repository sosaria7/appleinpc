// DiskDrive.cpp: implementation of the CDiskDrive class.
//
//////////////////////////////////////////////////////////////////////
#include <io.h>
#include <fcntl.h>

#define MEM(addr)	g_pBoard->m_cIOU.ReadMem8(addr)

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "localclock.h"
#include "gcr_table.h"
#include "diskdrive.h"
#include "appleclock.h"
#include "diskimagenib.h"
#include "arch/frame/mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define NO_OF_PHASES 8
#define MAX_PHYSICAL_TRACK_NO (40*NO_OF_PHASES)

static int g_aiPhysicalSector[16];
static BYTE g_aiLogicalSector[16] =
{
	0x0, 0x7, 0xE, 0x6, 0xD, 0x5, 0xC, 0x4,
	0xB, 0x3, 0xA, 0x2, 0x9, 0x1, 0x8, 0xF
};

static int g_aiStepperMovementTable[16][NO_OF_PHASES] =
{
	{  0,  0,  0,  0,  0,  0,  0,  0 },	/* all electromagnets off */
	{  0, -1, -2, -3,  0,  3,  2,  1 },	/* EM 1 on */
	{  2,  1,  0, -1, -2, -3,  0,  3 },	/* EM 2 on */
	{  1,  0, -1, -2, -3,  0,  3,  2 },	/* EMs 1 & 2 on */
	{  0,  3,  2,  1,  0, -1, -2, -3 },	/* EM 3 on */
	{  0, -1,  0,  1,  0, -1,  0,  1 },	/* EMs 1 & 3 on */
	{  3,  2,  1,  0, -1, -2, -3,  0 },	/* EMs 2 & 3 on */
	{  2,  1,  0, -1, -2, -3,  0,  3 },	/* EMs 1, 2 & 3 on */
	{ -2, -3,  0,  3,  2,  1,  0, -1 },	/* EM 4 on */
	{ -1, -2, -3,  0,  3,  2,  1,  0 },	/* EMs 1 & 4 on */
	{  0,  1,  0, -1,  0,  1,  0, -1 },	/* EMs 2 & 4 */
	{  0, -1, -2, -3,  0,  3,  2,  1 },	/* EMs 1, 2 & 4 on */
	{ -3,  0,  3,  2,  1,  0, -1, -2 },	/* EMs 3 & 4 on */
	{ -2, -3,  0,  3,  2,  1,  0, -1 },	/* EMs 1, 3 & 4 on */
	{  0,  3,  2,  1,  0, -1, -2, -3 },	/* EMs 2, 3 & 4 on */
	{  0,  0,  0,  0,  0,  0,  0,  0 }	/* all electromagnets on */
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDiskDrive::CDiskDrive()
{
	m_wDiskStatus = 0;
	m_bEnhanced = TRUE;
	m_dwLastAppleClock = 0;
	m_pDiskImage = NULL;
	m_bReadFlag = TRUE;

	Reset();
}

CDiskDrive::~CDiskDrive()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	Umount();
}

BYTE CDiskDrive::ToggleMotor(WORD addr)
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	if ( addr & 1 )
		m_wDiskStatus |= DDS_MOTORON;
	else
		m_wDiskStatus &= ~DDS_MOTORON;

	return MemReturnRandomData(1);
}

void CDiskDrive::SetStepper(BYTE byStepperStatus)
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	int phase;
	if ( m_wDiskStatus & DDS_MOTORON )
	{
		phase = m_iPhysicalTrackNo & 0x07;
		m_iPhysicalTrackNo += g_aiStepperMovementTable[byStepperStatus][phase];
		if ( m_iPhysicalTrackNo < 0 )
			m_iPhysicalTrackNo = 0;
		else if ( m_iPhysicalTrackNo >= MAX_PHYSICAL_TRACK_NO ) 
			m_iPhysicalTrackNo = MAX_PHYSICAL_TRACK_NO-1;
		if ( !( m_iPhysicalTrackNo & 3 ) && m_pDiskImage != NULL )
		{
			m_iTrackNo = ( m_iPhysicalTrackNo >> 2 );
			m_pDiskImage->MoveTrack( m_iTrackNo );
		}
	}
}

void CDiskDrive::Reset()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
//	SaveTrackBuffer();

	m_wDiskStatus &= 0;

	m_iPhysicalTrackNo = 0;
	m_iTrackNo = 0;
	m_iPosition = 0;
	m_dwLastAppleClock = 0;

	if (m_pDiskImage != NULL)
	{
		m_pDiskImage->Flush();
	}
}

int CDiskDrive::Mount( const char* pszImageName )
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	char szExt[4];
	int i, j;
	char ch;
	int hFile, nErrNo;
	int nImageFormat;

	CDiskImage* pNewImage;

	hFile = _open( pszImageName, O_RDONLY | O_BINARY );
	if ( hFile == -1 )
		return E_OPEN_FAIL;

	i = j = 0;
	// 확장자를 찾는다.
	while ( (ch=pszImageName[i++]) != 0 )
	{
		if ( ch == FILE_SEPARATOR )
			j = 0;
		else if ( ch == '.' )
			j = i;
	}
	memset( szExt, 0, 4 );
	// 확장자가 존재할 경우 확장자를 복사한다.
	if ( j != 0 )
		strncpy( szExt, pszImageName + j, 3 );

	nImageFormat = CDiskImage::Get2mgFormat(hFile);
	if (nImageFormat == IMAGE_NONE)
	{
		if (CDiskImageDos::IsMyExtension(szExt))
		{
			nImageFormat = IMAGE_DOS;
		}
		else if (CDiskImagePo::IsMyExtension(szExt))
		{
			nImageFormat = IMAGE_PRODOS;
		}
		else if (CDiskImageNib::IsMyExtension(szExt))
		{
			nImageFormat = IMAGE_NIBBLE;
		}
		else if (CDiskImageDos::IsMyType(hFile))
		{
			nImageFormat = IMAGE_DOS;
		}
		else if (CDiskImagePo::IsMyType(hFile))
		{
			nImageFormat = IMAGE_PRODOS;
		}
		else if (CDiskImageNib::IsMyType(hFile))
		{
			nImageFormat = IMAGE_NIBBLE;
		}
		else if (_stricmp(szExt, "dsk") == 0)
		{
			nImageFormat = IMAGE_DOS;
		}
	}
	_close(hFile);

	if (m_pDiskImage && m_pDiskImage->GetId() == nImageFormat)
	{
		pNewImage = m_pDiskImage;
	}
	else
	{
		switch (nImageFormat)
		{
		case IMAGE_DOS:
			pNewImage = new CDiskImageDos();
			break;
		case IMAGE_PRODOS:
			pNewImage = new CDiskImagePo();
			break;
		case IMAGE_NIBBLE:
			pNewImage = new CDiskImageNib();
			break;
		default:
			// unknown image type
			return E_UNKNOWN_FORMAT;
		}
	}

	nErrNo = pNewImage->Mount( pszImageName );
	if ( nErrNo == E_SUCCESS )
	{
		// if new CDiskImage object is created, delete the old one
		if ( m_pDiskImage != pNewImage )
		{
			if ( m_pDiskImage )
				delete m_pDiskImage;
			m_pDiskImage = pNewImage;
		}
		m_strImagePath = pszImageName;
		return E_SUCCESS;
	}
	m_strImagePath = "";

	// if new CDiskImage object is created, delete the new one because mounting has failed
	if ( m_pDiskImage != pNewImage )
		delete pNewImage;
	return nErrNo;
}

void CDiskDrive::Umount()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	m_strImagePath = "";
	if ( m_pDiskImage != NULL )
	{
		m_pDiskImage->Umount();
		delete m_pDiskImage;
		m_pDiskImage = NULL;
	}
}

BYTE CDiskDrive::ReadWrite(WORD addr, BYTE data)
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	switch( addr & 0x03 )
	{
	case 0x00:	// read or write as current mode
		if ( m_wDiskStatus & DDS_WRITEMODE )
			data = WriteNibble( data );
		else
			data = ReadNibble();
		break;
	case 0x01:	// write protect test
		data = ( IsWriteProtected() ) ? 0xFF : 0x00;
		break;
	case 0x02:	// read
		data = ReadNibble();
		m_wDiskStatus &= ~DDS_WRITEMODE;
		break;
	case 0x03:	// write
		data = WriteNibble( data );
		m_wDiskStatus |= DDS_WRITEMODE;
		break;
	}
	return data;
}

BYTE CDiskDrive::ReadNibble()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	DWORD interval;
	DWORD dwClock;
	int   nOffset;

	if ( !( m_wDiskStatus & DDS_MOTORON ) )
	{
		if ( m_pDiskImage == NULL
			|| !m_pDiskImage->IsMounted() )
			return 0x16;
		else
			return m_pDiskImage->Read( m_iPosition ) & 0x7F;
	}

	dwClock = g_pBoard->GetClock();
#if 1
	if ( m_bEnhanced )
	{
		WORD dwPC;

		if ( m_pDiskImage == NULL
			|| !m_pDiskImage->IsMounted() )
		{
			m_bReadFlag = !m_bReadFlag;
			if ( m_bReadFlag )
				return( 0x96 );
			else
				return( 0x16 );
		}

		dwPC = ((C65c02*)g_pBoard->m_pCpu)->getRegPC();
		g_pBoard->m_cIOU.m_bMemTest = TRUE;
		BYTE temp;

		// Unidisk interface Rom / Dos 3.3 / Prodos / Ultima 4
		if  ( MEM(dwPC) == 0x10 && MEM(dwPC+1) == 0xFB && MEM(dwPC+3) == 0xD5
				&& ( ( ( ( temp = MEM(dwPC+2) ) == 0x49 || temp == 0xC9 ) && MEM(dwPC+4) == 0xD0 )
				  || ( temp == 0x29 && MEM(dwPC+4)==0xC9 && MEM(dwPC+5) == 0xD5 && MEM(dwPC+6) == 0xD0 ) )
			)

		{
			g_pBoard->m_cIOU.m_bMemTest = FALSE;
			int origpos = m_iPosition;
			while ( m_pDiskImage->Read(m_iPosition) != 0xD5 )
			{
				m_iPosition++;
				if ( m_iPosition >= m_pDiskImage->GetNibblesPerTrack())
					m_iPosition = 0;
				if ( m_iPosition == origpos )
				{
					m_iPosition++;
					if ( m_iPosition >= m_pDiskImage->GetNibblesPerTrack())
						m_iPosition = 0;
					m_bEnhanced = FALSE;
					TRACE( "corrupted header.");
					break;
				}
			}
			m_bReadFlag = TRUE;
			m_dwLastAppleClock = dwClock;
			return m_pDiskImage->Read(m_iPosition);
		}

		g_pBoard->m_cIOU.m_bMemTest = FALSE;
		m_dwLastAppleClock = dwClock;
		m_bReadFlag = !m_bReadFlag;

		if ( m_bReadFlag )
		{
			m_iPosition++;
			if ( m_iPosition >= m_pDiskImage->GetNibblesPerTrack())
				m_iPosition = 0;
			return m_pDiskImage->Read( m_iPosition );
		}
		else
			return ( m_pDiskImage->Read( m_iPosition ) & 0x7F );

#if defined(_DEBUG) && 0
		// for test
		g_pBoard->m_cIOU.m_bMemTest = TRUE;
		for ( int i = 0; i < 8; i++ )
		{
			if ( READMEM( dwPC + i ) == 0xD5 )
			{
				int a = 0;
			}
		}
		g_pBoard->m_cIOU.m_bMemTest = FALSE;
#endif
	}
#endif
	interval =  dwClock - m_dwLastAppleClock;
	nOffset = interval % READ_CLOCK;

	interval /= READ_CLOCK;
	DWORD dwClockInc = interval * READ_CLOCK;
	m_dwLastAppleClock += dwClockInc;
	m_iPosition = ( m_iPosition + interval ) % m_pDiskImage->GetNibblesPerTrack();
	if ( m_pDiskImage == NULL
		|| !m_pDiskImage->IsMounted() )
	{
		if ( nOffset >= 8 )
			return 0x16;
		else
			return 0x96;
	}
	if ( nOffset >= 8 )
		return( m_pDiskImage->Read( m_iPosition ) & 0x7F );
	return m_pDiskImage->Read( m_iPosition );
}

BYTE CDiskDrive::WriteNibble(BYTE data)
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	if ( m_pDiskImage == NULL
			|| !m_pDiskImage->IsMounted()
			|| !( m_wDiskStatus & DDS_MOTORON ) )
		return( data & 0x7F );

	DWORD dwClock = g_pBoard->GetClock();
	DWORD interval =  dwClock - m_dwLastAppleClock;

	int iOffset = interval % READ_CLOCK;

	interval /= READ_CLOCK;

	DWORD dwClockInc = interval * READ_CLOCK;
	m_dwLastAppleClock += dwClockInc;
	m_iPosition = ( m_iPosition + interval ) % m_pDiskImage->GetNibblesPerTrack();

	if ( data & 0x80 )
	{
		m_pDiskImage->Write( m_iPosition, data );
		if ( iOffset >= READ_CLOCK - 8 )
			m_pDiskImage->Write( m_iPosition + 1, 0 );
		return data;
	}

	return( data & 0x7F );
}


LPCTSTR CDiskDrive::GetFileName()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	if ( m_pDiskImage != NULL && m_pDiskImage->IsMounted() )
		return m_pDiskImage->GetImagePath();
	return "";
}

BOOL CDiskDrive::IsWriteProtected()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	return ( m_pDiskImage != NULL ) ? m_pDiskImage->IsWriteProtected() : FALSE;
}

BOOL CDiskDrive::IsWriteMode()
{
	return ( m_wDiskStatus & DDS_WRITEMODE ) ? TRUE : FALSE;
}

void CDiskDrive::AdjustSpeed()
{
}

void CDiskDrive::Serialize(CArchive &ar)
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_iPosition;
		ar << m_iPhysicalTrackNo;
		ar << m_iTrackNo;
		ar << m_dwLastAppleClock;
		ar << m_wDiskStatus;
		ar << m_bEnhanced;
		ar << m_bReadFlag;
		ar << m_strImagePath;
	}
	else
	{
		ar >> m_iPosition;
		ar >> m_iPhysicalTrackNo;
		ar >> m_iTrackNo;
		ar >> m_dwLastAppleClock;
		ar >> m_wDiskStatus;
		ar >> m_bEnhanced;
		ar >> m_bReadFlag;
		ar >> m_strImagePath;

		if ( m_strImagePath != "" )
		{
			Mount( (const char*)m_strImagePath );
			if ( m_pDiskImage != NULL )
			{
				m_pDiskImage->MoveTrack( m_iTrackNo );
			}
		}
	}
}

BOOL CDiskDrive::IsEnhanced()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	return ( m_pDiskImage != NULL && m_pDiskImage->IsMounted() && m_bEnhanced == TRUE );
}

