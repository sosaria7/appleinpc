// DiskDrive.h: interface for the CDiskDrive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKDRIVE_H__21765B6D_6C44_4AA4_BCC1_CB140F0E0911__INCLUDED_)
#define AFX_DISKDRIVE_H__21765B6D_6C44_4AA4_BCC1_CB140F0E0911__INCLUDED_

#include "diskimagedos.h"
#include "diskimagepo.h"
#include "lockmgr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define RAW_TRACK_BYTES 6392

#define DDS_MOTORON			( 1 << 0 )
#define DDS_WRITEMODE		( 1 << 1 )
#define DDS_ACTIVE			( 1 << 2 )

// read cycle per 1 nibble data
// 1020480 / ( x * 6656 ) = 5 rps = 300 rpm
// x = 1020480 / ( 5 * 6656 ) = 30.66
#define READ_CLOCK		31

class CDiskDrive : public CObject
{
public:
	CDiskDrive();
	virtual ~CDiskDrive();

	BYTE	ReadWrite( WORD addr, BYTE data );
	void	Umount();
	BOOL	Mount(const char* pszImageName);
	void	Reset();
	BOOL	IsWriteProtected();
	BYTE	ToggleMotor(WORD addr);

	BYTE	WriteNibble(BYTE data);
	BYTE	ReadNibble();
	BOOL	IsEnhanced();

	virtual void Serialize(CArchive &ar);

// attributes
protected:
	void AdjustSpeed();

	int		m_iPosition;
	int		m_iPhysicalTrackNo;
	int		m_iTrackNo;

	DWORD	m_dwLastAppleClock;
	WORD	m_wDiskStatus;
	BOOL	m_bReadFlag;
	CString	m_strImagePath;
	CCSWrapper m_Lock;

	CDiskImage*	m_pDiskImage;

public:
	BOOL m_bEnhanced;
	BOOL IsWriteMode();
	void SetStepper(BYTE byStepperStatus);
	LPCTSTR GetFileName();
};

#endif // !defined(AFX_DISKDRIVE_H__21765B6D_6C44_4AA4_BCC1_CB140F0E0911__INCLUDED_)
