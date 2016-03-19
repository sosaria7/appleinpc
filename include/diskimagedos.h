// DiskImageDo.h: interface for the DiskImageDo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKIMAGEDO_H__7E89F4AD_AE17_433A_8349_715EECBF322B__INCLUDED_)
#define AFX_DISKIMAGEDO_H__7E89F4AD_AE17_433A_8349_715EECBF322B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "diskimage.h"
#include "aipcdefs.h"

#define DOS_TRACK_BYTES		4096	/* 256 * 16 */
#define DOS_IMAGE_BYTES		( DOS_TRACK_BYTES * 35 )	/* 143,360 */
/* DO logical order  0 1 2 3 4 5 6 7 8 9 A B C D E F */
/*    physical order 0 D B 9 7 5 3 1 E C A 8 6 4 2 F */

class CDiskImageDos : public CDiskImage  
{
public:
	CDiskImageDos();
	virtual ~CDiskImageDos();

	static BOOL IsMyType(int hFile, const char* szExt );
	virtual int GetId(){ return IMAGE_DOS; }

protected:
	virtual BOOL ReadBuffer();
	virtual void SaveBuffer();
	virtual void Nibblize();				// disk image format data to nibble
	virtual void Denibblize();				// nibble data to disk image format
	virtual void Code62( int nSector );
	virtual BOOL InitImage();

	void Decode62(int nSector);
	static BOOL CheckImage( int hFile, const BYTE* order );
	inline void WriteNibble(BYTE data)
	{
		m_abyNibBuffer[m_iGcrPos] = data;
		m_iGcrPos = ( m_iGcrPos == MAX_TRACK_BYTES-1 ) ? 0 : m_iGcrPos+1;
	};
	inline BYTE ReadNibble()
	{
		BYTE retval;
		do {
			retval = m_abyNibBuffer[m_iGcrPos];
			m_iGcrPos = ( m_iGcrPos == MAX_TRACK_BYTES-1 ) ? 0 : m_iGcrPos+1;
		} while( retval == 0 );
		return retval;
	}
protected:
	BYTE	m_abyDosBuffer[DOS_TRACK_BYTES];
	static BYTE sm_abyGcrEncTable[64];
	static BYTE sm_abyGcrDecTable[128];
	static BOOL sm_bGcrInitialized;
	static BYTE	sm_abySectorOrder[16];

	BYTE	m_abyGcrBuffer[343];		// 86 + 256 + 1 (last 1 is checksum)
	int		m_iGcrPos;
	// 각 Logical Sector의 내용이 파일에서 어느 위치에 있는지 정보..
	// 파일에서의 Logical Sector 순서가 아니다.. 그것의 역
	// dsk : 0 1 2 3 4 5 6 7 8 9 a b c d e f
	// po  : 0 e d c b a 9 8 7 6 5 4 3 2 1 f
	BYTE*	m_pbyLogicalOrder;
};

#endif // !defined(AFX_DISKIMAGEDO_H__7E89F4AD_AE17_433A_8349_715EECBF322B__INCLUDED_)
