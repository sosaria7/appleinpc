// DiskImage.cpp: implementation of the CDiskImage class.
//
//////////////////////////////////////////////////////////////////////
#include "arch/frame/stdafx.h"
#include "diskimage.h"

#include "diskimagedos.h"
#include "diskimagepo.h"
#include "aipcdefs.h"
#include "appleclock.h"

#include <io.h>
#include <fcntl.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDiskImage::CDiskImage()
{
	m_nTrack = 0;
	m_nStatus = 0;
	m_hFile = -1;
	m_szImagePath[0] = '\0';
	m_szImagePath[PATH_MAX] = '\0';
	m_nNibblesPerTrack = MAX_TRACK_BYTES;
}

CDiskImage::~CDiskImage()
{
	Umount();
}

void CDiskImage::MoveTrack(UINT nTrack)
{
	if ( m_nTrack != nTrack )
	{
		if ( m_nStatus & DIS_BUFFER_DIRTY )
			SaveBuffer();
		m_nTrack = nTrack;
		m_nStatus &= ~DIS_BUFFER_VALID;
	}
}

int CDiskImage::Mount(const char *szFileName)
{
	int hFile;

	m_nStatus = 0;
	Umount();
	if ( szFileName == NULL )
	{
		m_szImagePath[0] = 0;
		return E_SUCCESS;
	}
	hFile = open( szFileName, O_RDWR | O_BINARY );
	if ( hFile == -1 )
	{
		hFile = open( szFileName, O_RDONLY | O_BINARY );
		if ( hFile == -1 )
			return E_OPEN_FAIL;
		strncpy( m_szImagePath, szFileName, PATH_MAX );
		m_hFile = hFile;
		m_nStatus |= DIS_WRITE_PROTECTED;
		return E_SUCCESS;
	}
	strncpy( m_szImagePath, szFileName, PATH_MAX );
	m_hFile = hFile;
	m_nTrack = 0;
	m_nStatus &= ~DIS_BUFFER_VALID;
	if ( !ReadBuffer() )
		return E_READ_FAIL;
	return E_SUCCESS;
}

void CDiskImage::Umount()
{
	if ( m_hFile != -1 )
	{
		if ( m_nStatus & DIS_BUFFER_DIRTY )
			SaveBuffer();
		close( m_hFile );
		m_hFile = -1;
	}
}


BOOL CDiskImage::IsMounted()
{
	return ( m_hFile != -1 );
}

BOOL CDiskImage::IsWriteProtected()
{
	return ( m_hFile != -1 && ( m_nStatus & DIS_WRITE_PROTECTED ) );
}


// extlist 는 공백없이 ';'으로 구분된 확장자 목록
// ext는 extlist 에 포함되어있는지 확인할 확장자.
BOOL CDiskImage::IsMatch(const char* extlist, const char* ext)
{
	int i;
	int j;
	char ch, ch1;
	i = j = 0;

	while ( (ch=extlist[i++]) != 0 )
	{
		if ( ch == ';' )
		{
			if ( j != -1 && ext[j] == 0 )		// match
				return TRUE;
			j = 0;
			continue;
		}
		if ( j == -1 )				// compare next
			continue;
		ch1 = ext[j];
		if ( ch >= 'A' && ch <= 'Z' )
			ch += 'a' - 'A';
		if ( ch1 >= 'A' && ch1 <= 'Z' )
			ch1 += 'a' - 'A';
		if ( ch != ch1 )
		{
			j = -1;
			continue;
		}
		j++;
	}
	if ( j != -1 && ext[j] == 0 )		// match
		return TRUE;
	return FALSE;
}

char* CDiskImage::GetImagePath()
{
	return m_szImagePath;
}

