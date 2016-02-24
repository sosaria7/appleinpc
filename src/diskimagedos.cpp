// diskimagedo.cpp: implementation of the DiskImageDo class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "diskimagedos.h"
#include "aipcdefs.h"

#include <io.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const static BYTE sg_abyLogicalOrder[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

BYTE CDiskImageDos::sm_abySectorOrder[] = {
	0x00, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04,
	0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x0F
};

BYTE CDiskImageDos::sm_abyGcrEncTable[64] = 
{
	0x96, 0x97, 0x9A, 0x9B, 0x9D, 0x9E, 0x9F, 0xA6,
	0xA7, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB2, 0xB3,
	0xB4, 0xB5, 0xB6, 0xB7, 0xB9, 0xBA, 0xBB, 0xBC,
	0xBD, 0xBE, 0xBF, 0xCB, 0xCD, 0xCE, 0xCF, 0xD3,
	0xD6, 0xD7, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE,
	0xDF, 0xE5, 0xE6, 0xE7, 0xE9, 0xEA, 0xEB, 0xEC,
	0xED, 0xEE, 0xEF, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6,
	0xF7, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};
BYTE CDiskImageDos::sm_abyGcrDecTable[128];
BOOL CDiskImageDos::sm_bGcrInitialized = FALSE;

CDiskImageDos::CDiskImageDos()
{
	int i;
	m_pbyLogicalOrder = (BYTE*)sg_abyLogicalOrder;

	for( i = 0; i < 16; i++ )
		m_abyLogicalSector[sm_abySectorOrder[m_pbyLogicalOrder[i]]] = i;

	if ( !sm_bGcrInitialized )
	{
		memset( sm_abyGcrDecTable, 0, sizeof(sm_abyGcrDecTable) );
		for( i = 0; i < 64; i++ )
			sm_abyGcrDecTable[sm_abyGcrEncTable[ i ] & 0x7F ] = i << 2;
		sm_bGcrInitialized = TRUE;
	}
}

CDiskImageDos::~CDiskImageDos()
{

}

BOOL CDiskImageDos::ReadBuffer()
{
	long lLen;
	if ( m_hFile == -1 )
		return FALSE;
	lseek( m_hFile, m_nTrack << 12, SEEK_SET );
	lLen = read( m_hFile, m_abyDosBuffer, DOS_TRACK_BYTES );
	Nibblize();
	m_nStatus |= DIS_BUFFER_VALID;
	m_nStatus &= ~DIS_BUFFER_DIRTY;
	return TRUE;
}

void CDiskImageDos::SaveBuffer()
{
	long lLen;
	if ( m_hFile == -1 )
		return;
	Denibblize();
	lseek( m_hFile, m_nTrack << 12, SEEK_SET );
	lLen = write( m_hFile, m_abyDosBuffer, DOS_TRACK_BYTES );
	m_nStatus &= ~DIS_BUFFER_DIRTY;
}

#define WRITE_SYNC(len)		\
	length = len;			\
	while(length--)			\
		WriteNibble(0xFF)

// convert the 256 8-bit bytes into 342 6-bit bytes
// argument nSector is Logical Sector.(by dos order)
void CDiskImageDos::Code62(int nSector)
{
	int i;
	{
		BYTE* pbyPage = m_abyDosBuffer + ( nSector << 8 );
		BYTE offset = 0xAC;
		i = 0;
		while ( offset != 0x02 )
		{
			BYTE value = 0;
#define ADDVALUE(a)		\
			value = (value << 2 ) | (((a) & 0x01) << 1 ) | (((a) & 0x02) >> 1)
			ADDVALUE(pbyPage[offset]);	offset -= 0x56;
			ADDVALUE(pbyPage[offset]);	offset -= 0x56;
			ADDVALUE(pbyPage[offset]);	offset -= 0x53;
#undef ADDVALUE
			m_abyGcrBuffer[i++] = value << 2;
		}
		// i = 256 / 3 = 86 ( last 2 byte is only 4 bit )
		// clear off higher 2 bits of the last 2 byte
		m_abyGcrBuffer[84] &= 0x3F;
		m_abyGcrBuffer[85] &= 0x3F;
		memcpy( m_abyGcrBuffer + 86, pbyPage, 256 );
		m_abyGcrBuffer[342] = 0;
	}
	// exclusive-or the entire data block with itself offset by one byte
	{
		i = 342;
		while( i-- )
			m_abyGcrBuffer[i+1] ^= m_abyGcrBuffer[i];
	}

	{
		for( i = 0; i < 343; i++ )
			WriteNibble( sm_abyGcrEncTable[ m_abyGcrBuffer[i] >> 2 ] );
	}
}

void CDiskImageDos::Decode62(int nSector)
{
	int i;
	for( i = 0; i < 342; i++ )
		m_abyGcrBuffer[i] = sm_abyGcrDecTable[ ReadNibble() & 0x7F ];

	// Exclusive-OR
	{
		BYTE byLast = m_abyGcrBuffer[0];
		for( i = 1; i < 342; i++ )
			byLast = ( m_abyGcrBuffer[i] ^= byLast );

		if ( ( byLast & 0xFC ) != sm_abyGcrDecTable[ ReadNibble() & 0x7F ] )
		{
			TRACE( "CDiskImageDos::Decode62 - Invalid Checksum. Track:%d, Sector:%d.\n",
				m_nTrack, m_abyLogicalSector[nSector] );
		}
	}
	
#define DECODE62(n)																\
	pbyPage[byOffset] = ( pbyHighBit[byOffset] & 0xFC )							\
							| ( ( (*pbyLowBit) >> ( n * 2 + 3 ) ) & 1 )			\
							| ( ( (*pbyLowBit) >> ( n * 2 + 1 ) ) & 2 )
	// convert the 342 6-bit bytes to 256 8-bit bytes
	{
		BYTE* pbyLowBit = m_abyGcrBuffer;
		BYTE* pbyHighBit = m_abyGcrBuffer + 0x56;
		BYTE* pbyPage = m_abyDosBuffer + ( nSector << 8 );
		BYTE byOffset = 0xAC;
		while( byOffset != 2 )
		{
			if ( byOffset >= 0xAC )
				DECODE62(2);
			byOffset -= 0x56;
			DECODE62(1);
			byOffset -= 0x56;
			DECODE62(0);
			byOffset -= 0x53;
			pbyLowBit++;
		}
	}
#undef DECODE62
}

void CDiskImageDos::Nibblize()
{
	int sector, length;
	m_iGcrPos = ( MAX_TRACK_BYTES - 1 ) - ( ( m_nTrack*768 ) % MAX_TRACK_BYTES );
	memset( m_abyNibBuffer, 0xFF, MAX_TRACK_BYTES );
	// write gap one, which contains 48 self-sync bytes
	WRITE_SYNC( 64 );
	for( sector = 0; sector < 16; sector++ )
	{
		// write the address field, which contains:
		//	- PROLOGUE ( D5 AA 96 )
		//	- VOLUME NUMBER ("4 AND 4" ENCODED)
		//	- TRACK NUMBER ("4 AND 4" ENCODED)
		//	- SECTOR NUMBER ("4 AND 4" ENCODED)
		//	- CHECKSUM ("4 AND 4" ENCODED)
		//	- EPILOGUE ( DE AA EB )
		WriteNibble(0xD5);
		WriteNibble(0xAA);
		WriteNibble(0x96);
		WriteNibble(0xFF);			// Volume = 254
		WriteNibble(0xFE);
#define CODE44A(a)	((((a) >> 1) & 0x55) | 0xAA)
#define CODE44B(a)	(((a) & 0x55) | 0xAA )
		WriteNibble( CODE44A( (BYTE)m_nTrack ) );			// track
		WriteNibble( CODE44B( (BYTE)m_nTrack ) );
		WriteNibble( CODE44A( sector ) );					// sector
		WriteNibble( CODE44B( sector ) );
		WriteNibble( CODE44A(0xFE ^ ((BYTE)m_nTrack) ^ sector) );	// checksum
		WriteNibble( CODE44B(0xFE ^ ((BYTE)m_nTrack) ^ sector) );
#undef CODE44A
#undef CODE44B
		WriteNibble( 0xDE );
		WriteNibble( 0xAA );
		WriteNibble( 0xEB );

		// write gap two, which contains six self-sync bytes
		WRITE_SYNC( 6 );

		// write the data field, which contains
		//	- PROLOGUE ( D5 AA AD )
		//	- 343 6-BIT BYTES OF NIBBLIZED DATA, INCLUDING A 6-BIT CHECKSUM
		//	- EPILOGUE ( DE AA EB )
		WriteNibble( 0xD5 );
		WriteNibble( 0xAA );
		WriteNibble( 0xAD );
		Code62( sm_abySectorOrder[m_pbyLogicalOrder[sector]] );	// 343 bytes
		WriteNibble( 0xDE );
		WriteNibble( 0xAA );
		WriteNibble( 0xEB );

		// write gap three, which contains 27 self-sync bytes
		WRITE_SYNC( 27 );
	}
}
#undef WRITE_SYNC

void CDiskImageDos::Denibblize()
{
	int i;
	int nSectorLeft = 16;
	int nOffset = 0;
	int nSector = -2;
	int nLastGcrPos = 0;
	int nLoopCnt = 0;
	BYTE byData;
	BOOL arbDecoded[16];
	memset( arbDecoded, 0, sizeof(arbDecoded) );
	m_iGcrPos = 0;
	while( nSectorLeft )
	{
		if ( ReadNibble() == 0xD5 )
		{
			if ( ReadNibble() == 0xAA &&
				( (byData=ReadNibble()) == 0x96 || byData == 0xAD ) )
			{
				if ( byData == 0x96 )		// address field
				{
					if ( nSector >= 0 )
						TRACE( "CDiskImageDos::Denibblize - Address field without Data field: %d,%d.\n", m_nTrack, nSector );

					ReadNibble(); ReadNibble(); ReadNibble(); ReadNibble();		// skip volume, track
					nSector = ( ( ReadNibble() << 1 ) | 1 ) & ReadNibble();
					if ( nSector > 15 )
					{
						TRACE( "CDiskImageDos::Denibblize - Invalid sector number: %d,%d.\n", m_nTrack, nSector );
						nSector &= -1;
					}
					if ( arbDecoded[nSector] )
						TRACE( "CDiskImageDos::Denibblize - Duplicated sector: %d,%d.\n", m_nTrack, nSector );
				}
				else						// data field
				{
					if ( nSector >= 0 )		// before this, address field is need
					{
						arbDecoded[nSector] = TRUE;
						Decode62( sm_abySectorOrder[m_pbyLogicalOrder[nSector]] );
						nSector = -1;
						nSectorLeft--;
					}
					else if ( nSector == -1 )
						TRACE( "CDiskImageDos::Denibblize - Data field without Address field.\n" );
				}
			}
			else
			{
				m_iGcrPos = ( m_iGcrPos ) ? m_iGcrPos-1 : MAX_TRACK_BYTES-1;	// check previous byte
//				TRACE( "CDiskImageDos::Denibblize - Unknown Signature.\n.", byData );
			}
		}
		if ( m_iGcrPos-nLastGcrPos < 0 )
		{
			nLoopCnt++;
			if ( nLoopCnt == 2 )
			{
				TRACE( "CDiskImageDos::Denibblize - There is not all sectors.\n" );
				break;
			}
		}
		nLastGcrPos = m_iGcrPos;
	}
	for( i = 0; i < 15; i++ )
	{
		if ( !arbDecoded[i] )
			TRACE( "CDiskImageDos::Denibblize - Sector is not found: %d.\n", i );
	}
}

BOOL CDiskImageDos::CheckImage(int hFile, const BYTE* order)
{
	int i, err;
	BYTE ch;
	WORD val[2];

	err = FALSE;
	for ( i = 1; i < 16 &&  !err; i++ )
	{
		lseek( hFile, 0x11002 + ( order[i] << 8 ), SEEK_SET );
		if ( read( hFile, &ch, 1 ) != 1 )
			err = TRUE;
		if ( !err && ch != i - 1 )
			err = TRUE;
	}
	if ( !err )
		return TRUE;
	// check for a dos order image of a prodos diskette
	err = 0;
	for ( i = 2; i < 6 && !err; i++ )
	{
		lseek( hFile, 0x100 + ( order[i] << 9 ), SEEK_SET );
		if ( read( hFile, val, 4 ) != 4 )
		{
			err = TRUE;
			break;
		}

		if ( ( WORD_ORDER( val[0] ) != ( (i == 5) ? 0 : 6-i ) )
		  || ( WORD_ORDER( val[1] ) != ( (i == 2) ? 0 : 8-i ) ) )
		  err = TRUE;
	}
	return !err;
}
BOOL CDiskImageDos::IsMyType(int hFile, const char* szExt )
{
	// 확장자로 확인
	if ( IsMatch( "dsk;do", szExt ) )
		return TRUE;
	if ( IsMatch( "po;nib;apl;iie;prg", szExt ) )
		return FALSE;
	// 모르는 확장자 인경우 내용 확인
	// check for a dos order image of a dos diskette
	return CheckImage( hFile, sg_abyLogicalOrder );
}

