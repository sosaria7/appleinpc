// diskimagenib.cpp: implementation of the CDiskImageNib class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "diskimagenib.h"

#include <io.h>
#include <fcntl.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDiskImageNib::CDiskImageNib()
{
	m_nNibblesPerTrack = MAX_TRACK_BYTES;
}

CDiskImageNib::~CDiskImageNib()
{

}

BOOL CDiskImageNib::InitImage()
{
	m_nNibblesPerTrack = (int)(m_uDataLength / 35);
	if (m_nNibblesPerTrack == 0 || m_nNibblesPerTrack > MAX_TRACK_BYTES)
	{
		m_nNibblesPerTrack = MAX_TRACK_BYTES;
	}
	m_uNumOfTrack = (UINT)(m_uDataLength / m_nNibblesPerTrack);
	return TRUE;
}

BOOL CDiskImageNib::ReadBuffer()
{
	long lLen;
	BYTE abyBuff[MAX_TRACK_BYTES];

	if ( m_hFile == -1 )
		return FALSE;

	if (m_nTrack >= m_uNumOfTrack)
		return FALSE;

	_lseek(m_hFile, m_uDataOffset + (m_nTrack * m_nNibblesPerTrack), SEEK_SET );
	lLen = _read( m_hFile, abyBuff, m_nNibblesPerTrack );
	m_nStatus |= DIS_BUFFER_VALID;
	m_nStatus &= ~DIS_BUFFER_DIRTY;
	if ( !StoreData( abyBuff ) )
	{
		m_nStatus ^= ~DIS_BUFFER_VALID;
		return FALSE;
	}
	return TRUE;
}

void CDiskImageNib::SaveBuffer()
{
	long lLen;

	if ( m_hFile == -1 )
		return;

	if (m_nTrack >= m_uNumOfTrack)
		return;

	_lseek(m_hFile, m_uDataOffset + (m_nTrack * m_nNibblesPerTrack), SEEK_SET );
	lLen = _write( m_hFile, m_abyNibBuffer, m_nNibblesPerTrack );
	m_nStatus &= ~DIS_BUFFER_DIRTY;
	return;
}

BOOL CDiskImageNib::CheckImage(int hFile)
{
	BYTE abyBuff[ 512 ];
	int i;
	BYTE ch;
	DWORD val = 0;
	_lseek(hFile, 0, SEEK_SET );
	if (_read(hFile, abyBuff, 512 ) != 512 )
		return FALSE;
	for( i = 0; i < 512; i++ )
	{
		ch = abyBuff[i];
//		if ( !( ch & 0x80 ) )
//			return FALSE;
		val = ((val << 8) | ch) & 0x00FFFFFF;
		if ( val == 0xD5AA96 || val == 0xD5AAB5 || val == 0xD5AAAD )
			break;
	}
	return( i < 512 );
}

BOOL CDiskImageNib::IsMyType(int hFile)
{
	int nSize;

	nSize = _lseek(hFile, 0, SEEK_END );
	if ( ( nSize % 35 ) == 0 )
	{
		nSize /= 35;
		if ( nSize < 6384 || nSize > MAX_TRACK_BYTES )
			return FALSE;
	}
	else if ( ( nSize % MAX_TRACK_BYTES ) == 0 )
	{
		nSize /= MAX_TRACK_BYTES;
		if ( nSize < 35 || nSize > 40 )
		{
			return FALSE;
		}
	}
	return CheckImage( hFile );
}

#define WRITE_SYNC(len)		\
	length = len;			\
	while(length--)			\
		m_abyNibBuffer[i++] = 0xFF;

#define WRITE_NIBBLE(data)	\
	m_abyNibBuffer[i++] = data;	\
	if ( i >= MAX_TRACK_BYTES )	\
		break;

#define COPY_NIBBLE(len)	\
	length = len;			\
	while( length-- )		\
	{						\
		WRITE_NIBBLE( pbyData[j++] );		\
		if ( j >= m_nNibblesPerTrack )		\
			j = 0;			\
	}						\
	if ( i >= MAX_TRACK_BYTES )	\
		break;

BOOL CDiskImageNib::StoreData(BYTE *pbyData)
{
	memcpy( m_abyNibBuffer, pbyData, m_nNibblesPerTrack );
	if ( m_nNibblesPerTrack < MAX_TRACK_BYTES )
	{
		memset( m_abyNibBuffer+m_nNibblesPerTrack, 0xff, MAX_TRACK_BYTES - m_nNibblesPerTrack );
	}
	return TRUE;
/*
	int i, j, k, cnt;
	int length, start, turn;
	BYTE sig, ch;

	i = j = k = 0;
	turn = 0;
	cnt = 0;
	sig = 0;
	start = 0;

	WRITE_SYNC(64);
	while( turn < 2 )
	{
		if ( j < k )
			turn++;
		k = j;
		ch = pbyData[j++];
		if ( j >= m_nNibblesPerTrack )
			j = 0;
		if ( ch != 0xD5 )
			continue;
		if ( pbyData[j] != 0xAA )
			continue;
		
		ch = pbyData[(j+1)%m_nNibblesPerTrack];
		if ( ch == 0xAD )		// data field
		{
			if ( sig == 0xB5 )	// dos 3.2, 13 sector
			{
				WRITE_SYNC(16);
				WRITE_NIBBLE(0xD5);
				COPY_NIBBLE(416);
				WRITE_SYNC(32);
				cnt++;
			}
			else if ( sig == 0x96 )	// dos 3.2, 16 sector
			{
				WRITE_SYNC(6);
				WRITE_NIBBLE(0xD5);
				COPY_NIBBLE(348);
				WRITE_SYNC(27);
				cnt++;
			}
			sig = 0;
		}
		if ( ch == 0xB5 )
		{
			if ( i == 64 )
				start = j;
			else if ( start == j )
				break;
			sig = 0xB5;			// dos 3.2 13 sector
		}
		else if ( ch == 0x96 )
		{
			if ( i == 64 )
				start = j;
			else if ( start == j )
				break;
			sig = 0x96;			// dos 3.2 16 sector
		}
		else
			continue;
		
		WRITE_NIBBLE(0xD5);
		COPY_NIBBLE(13);
	}
	while ( i < MAX_TRACK_BYTES )
		m_abyNibBuffer[i++] = 0xFF;
	if ( cnt == 16 || cnt == 13 )
		return TRUE;
	return FALSE;
	*/
}
