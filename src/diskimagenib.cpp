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
	m_nNibblesPerTrack = 0;
}

CDiskImageNib::~CDiskImageNib()
{

}
int CDiskImageNib::Mount(const char* szFileName)
{
	int nRet;
	nRet = CDiskImage::Mount( szFileName );
	if ( nRet == E_SUCCESS )
	{
		m_nNibblesPerTrack = (int)( lseek( m_hFile, 0, SEEK_END ) / 35 );
		if ( m_nNibblesPerTrack > MAX_TRACK_BYTES )
		{
			m_nNibblesPerTrack = MAX_TRACK_BYTES;
		}
	}
	else
	{
		m_nNibblesPerTrack = 0;
	}
	return nRet;
}
void CDiskImageNib::Umount()
{
	CDiskImage::Umount();
	m_nNibblesPerTrack = 0;
}

BOOL CDiskImageNib::ReadBuffer()
{
	long lLen;
	BYTE abyBuff[MAX_TRACK_BYTES];

	if ( m_hFile == -1 )
		return FALSE;

	if ( m_nNibblesPerTrack == 0 )
	{
		m_nNibblesPerTrack = (int)( lseek( m_hFile, 0, SEEK_END ) / 35 );
		if ( m_nNibblesPerTrack > MAX_TRACK_BYTES )
		{
			m_nNibblesPerTrack = MAX_TRACK_BYTES;
		}
	}

	lseek( m_hFile, m_nTrack * m_nNibblesPerTrack, SEEK_SET );
	lLen = read( m_hFile, abyBuff, m_nNibblesPerTrack );
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

	lseek( m_hFile, m_nTrack * m_nNibblesPerTrack, SEEK_SET );
	lLen = write( m_hFile, m_abyNibBuffer, m_nNibblesPerTrack );
	m_nStatus &= ~DIS_BUFFER_DIRTY;
	return;
}

BOOL CDiskImageNib::CheckImage(int hFile)
{
	BYTE abyBuff[ 512 ];
	int i;
	char ch;
	DWORD val;
	lseek( hFile, 0, SEEK_SET );
	if ( read( hFile, abyBuff, 512 ) != 512 )
		return FALSE;
	for( i = 0; i < 508; i++ )
	{
		ch = abyBuff[i];
		if ( !( ch & 0x80 ) )
			return FALSE;
		val = *(DWORD*)(abyBuff+i);
		val = DWORD_ORDER(val) & 0x00FFFFFF;
		if ( val == 0x96AAD5 || val == 0xB5AAD5 || val == 0xADAAD5 )
			break;
	}
	return( i < 508 );
}

BOOL CDiskImageNib::IsMyType(int hFile, const char* szExt )
{
	int nSize;

	// 확장자로 확인
	if ( IsMatch( "do;dsk;po;apl;iie;prg", szExt ) )
		return FALSE;

	nSize = lseek( hFile, 0, SEEK_END );
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
