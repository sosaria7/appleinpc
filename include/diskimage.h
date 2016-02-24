// DiskImage.h: interface for the CDiskImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKIMAGE_H__8D677F92_8D18_4BC1_B58E_46C13A940CE5__INCLUDED_)
#define AFX_DISKIMAGE_H__8D677F92_8D18_4BC1_B58E_46C13A940CE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "aipcdefs.h"

// 1024000 / ( 32 * 6400 ) = 5 rps = 300 rpm
//#define RAW_TRACK_BYTES	6400
#define RAW_TRACK_BYTES	6656
#define MAX_TRACK_BYTES	6656

#define DIS_BUFFER_VALID	0x01
#define DIS_BUFFER_DIRTY	0x02
#define DIS_WRITE_PROTECTED	0x04

class CDiskImage
{
public:

	char* GetImagePath();
	virtual int GetId(){ return 0; }
	BOOL IsWriteProtected();
	BOOL IsMounted();
	CDiskImage();
	virtual ~CDiskImage();

	inline BYTE Read(UINT nOffset)
	{
		nOffset %= m_nNibblesPerTrack;
		if ( !( m_nStatus & DIS_BUFFER_VALID ) )
			ReadBuffer();
		return m_abyNibBuffer[nOffset];
	};
	inline void Write(UINT nOffset, BYTE byData)
	{
		if ( m_nStatus & DIS_WRITE_PROTECTED )
			return;
		nOffset %= m_nNibblesPerTrack;
		if ( !( m_nStatus & DIS_BUFFER_VALID ) )
			ReadBuffer();
		m_abyNibBuffer[nOffset] = byData;
		m_nStatus |= DIS_BUFFER_DIRTY;
	};
	virtual int Mount(const char* szFileName);
	virtual void Umount();

	virtual void MoveTrack(UINT m_nTrack);
	static BOOL IsMyType(int hFile, const char* szExt )
	{
		return FALSE;
	}

protected:
	virtual BOOL ReadBuffer(){ m_nStatus |= DIS_BUFFER_VALID; return TRUE; };
	virtual void SaveBuffer(){ m_nStatus &= ~DIS_BUFFER_DIRTY; };
	static BOOL IsMatch(const char* szExtList, const char* szExt);

protected:
	UINT	m_nTrack;
	UINT	m_nStatus;
	int		m_hFile;
	BYTE	m_abyNibBuffer[MAX_TRACK_BYTES];
	BYTE	m_abyLogicalSector[16];			// Physical sector to Logical sector
	char	m_szImagePath[PATH_MAX+1];
	int		m_nNibblesPerTrack;
};

#endif // !defined(AFX_DISKIMAGE_H__8D677F92_8D18_4BC1_B58E_46C13A940CE5__INCLUDED_)
