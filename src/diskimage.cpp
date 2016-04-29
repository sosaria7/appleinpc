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

CDiskImage::CDiskImage()
{
	m_nTrack = 0;
	m_nStatus = 0;
	m_hFile = -1;
	m_strImagePath = "";
	m_nNibblesPerTrack = MAX_TRACK_BYTES;
	m_uNumOfTrack = 35;
	m_uDataOffset = 0;
	m_uDataLength = 0;
	m_uVolumeNo = 254;
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
	int hFile = -1;
	BYTE sig[4];
	int nRead;
	S2ImgHeader st2mgHeader;

	m_nStatus = 0;
	Umount();
	if ( szFileName == NULL )
	{
		m_strImagePath = "";
		return E_SUCCESS;
	}

	hFile = _open( szFileName, O_RDWR | O_BINARY );

	if ( hFile == -1 )
	{
		hFile = _open( szFileName, O_RDONLY | O_BINARY );
		if ( hFile == -1 )
			return E_OPEN_FAIL;
		m_nStatus |= DIS_WRITE_PROTECTED;
	}
	m_strImagePath = szFileName;
	m_nTrack = 0;
	m_nStatus &= ~DIS_BUFFER_VALID;


	m_uVolumeNo = 254;
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
		m_uDataOffset = st2mgHeader.dwDataOffset;
		m_uDataLength = st2mgHeader.dwDataLength;
		if ((st2mgHeader.dwFlagsVolumeNumber & 0x100) != 0)
			m_uVolumeNo = st2mgHeader.dwFlagsVolumeNumber & 0xFF;
		if ((st2mgHeader.dwFlagsVolumeNumber & 0x80000000) != 0)
			m_nStatus |= DIS_WRITE_PROTECTED;
	}

	m_hFile = hFile;
	if (!InitImage() || !ReadBuffer())
	{
		m_hFile = -1;
		_close(hFile);
		return E_READ_FAIL;
	}

	return E_SUCCESS;
}

void CDiskImage::Umount()
{
	if ( m_hFile != -1 )
	{
		if ( m_nStatus & DIS_BUFFER_DIRTY )
			SaveBuffer();
		_close( m_hFile );
		m_hFile = -1;
		m_uDataOffset = 0;
		m_uDataLength = 0;
		m_uVolumeNo = 254;
	}
}

void CDiskImage::Flush()
{
	if (m_hFile != -1)
	{
		if (m_nStatus & DIS_BUFFER_DIRTY)
			SaveBuffer();
	}
}

int CDiskImage::Get2mgFormat(int hFile)
{
	int nRead;
	BYTE sig[4];
	DWORD dwImageFormat;
	_lseek(hFile, 0, SEEK_SET);
	nRead = _read(hFile, sig, 4);
	// 2mg image
	if (nRead == 4 && memcmp(sig, "2IMG", 4) == 0)
	{
		_lseek(hFile, 12, SEEK_SET);
		nRead = _read(hFile, &dwImageFormat, 4);
		switch (dwImageFormat)
		{
		case 0:
			return IMAGE_DOS;
		case 1:
			return IMAGE_PRODOS;
		case 2:
			return IMAGE_NIBBLE;
		default:
			break;
		}
	}
	return IMAGE_NONE;
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

LPCTSTR CDiskImage::GetImagePath()
{
	return (LPCTSTR)m_strImagePath;
}

