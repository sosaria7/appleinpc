// DiskInterface.h: interface for the CDiskInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__HDD_H__)
#define __HDD_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Card.h"
#include "lockmgr.h"

#define HDD_DRIVES_NO			2
#define HDD_BLOCK_SIZE			512

class CHDDImage : public CObject
{
public:
	DECLARE_SERIAL( CHDDImage );
	CHDDImage();
	virtual ~CHDDImage();
	
	int ReadBlock( int nBlockNo, void* pBuffer );
	int WriteBlock( int nBlockNo, void* pBuffer );
	
	int Mount( const char* szFileName );
	void Umount();
	BOOL IsMounted();
	int GetBlockCount() { return m_nBlockCount; }
	BOOL IsWriteProtected() { return m_bIsWriteProtected;  }

	LPCTSTR GetFileName();

	void Serialize( CArchive &ar );

protected:
	int m_hFile;
	CString	m_strImagePath;
	CCSWrapper m_Lock;
	int m_nBlockCount;
	UINT m_uDataOffset;
	UINT m_uDataLength;
	UINT m_uVolumeNo;
	BOOL m_bIsWriteProtected;
};


class CHDDInterface : public CCard  
{
public:
	DECLARE_SERIAL( CHDDInterface );
	//DECLARE_DYNAMIC( CHDDInterface );

	void SetMotorLightHandler(void* objTo, callback_handler func);
	void Clock(int nClock);
	void PowerOff();
	void PowerOn();
	virtual void Reset();

	CHDDInterface();
	virtual ~CHDDInterface();

	virtual void InitRomImage();

	virtual BYTE ReadRom(WORD addr);
	virtual BYTE ReadExRom(WORD addr);
	virtual void Write(WORD addr, BYTE data);
	virtual BYTE Read(WORD addr);
 	virtual void Configure(); 

	CHDDImage* GetDrive( int nDriveNo );

	void Serialize( CArchive &ar );

	static const BYTE CMD_NONE;
	static const BYTE CMD_STAT;
	static const BYTE CMD_READ;
	static const BYTE CMD_WRITE;

	static const BYTE ERR_WAIT;		// ($00) Wait for response
	static const BYTE ERR_IO;		// ($27) IO Error
	static const BYTE ERR_NDEV;		// ($28) No Device Connected
	static const BYTE ERR_WP;		// ($2B) Write Protect
	static const BYTE ERR_READY;	// ($80) Ready for data

protected:
	CHDDImage m_cHDD[HDD_DRIVES_NO];
	BYTE m_abyData[HDD_BLOCK_SIZE];
	BYTE m_byCommand;
	BYTE m_byCheckSum;
	BYTE m_byStatus;
	int m_nCurOffset;
	int m_nBlockNo;

	DWORD m_dwMotorOn;
	DWORD m_dwWriteLED;
	int m_nCurrentDisk;

	STCallbackHandler stLightHandler;

	void UpdateMotorLight();
	void MotorOn(BOOL bWrite);
};



#endif

