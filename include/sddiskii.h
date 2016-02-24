// DiskInterface.h: interface for the CDiskInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__SDDISKII_H__)
#define __SDDISKII_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Card.h"
#include "lockmgr.h"

#define SDD2_HDD_DRIVES			2
#define SDD2_HDD_BLOCK_SIZE		512
#define SDD2_LATCH_SIZE			3

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
	
	LPCTSTR GetFileName();

	void Serialize( CArchive &ar );

protected:
	int m_hFile;
	CString	m_strImagePath;
	CCSWrapper m_Lock;
};


class CSDDiskII : public CCard  
{
public:
	DECLARE_SERIAL( CSDDiskII );
	//DECLARE_DYNAMIC( CSDDiskII );

	void SetMotorLightHandler(void* objTo, callback_handler func);
	void Clock(int nClock);
	void PowerOff();
	void PowerOn();
	virtual void Reset();

	CSDDiskII();
	virtual ~CSDDiskII();

	virtual void InitRomImage();

	virtual void Write(WORD addr, BYTE data);
	virtual BYTE Read(WORD addr);
 	virtual void Configure(); 

	CHDDImage* GetDrive( int nDriveNo );

	void Serialize( CArchive &ar );

	static const BYTE CMD_NONE;
	static const BYTE CMD_READ;
	static const BYTE CMD_WRITE;
	static const BYTE CMD_READY;
	static const BYTE CMD_ACK;
	static const BYTE CMD_END;

protected:
	CHDDImage m_cHDD[SDD2_HDD_DRIVES];
	BYTE m_abyData[SDD2_HDD_BLOCK_SIZE];
	BYTE m_abyLatch[SDD2_LATCH_SIZE];
	BYTE m_byCommand;
	BYTE m_byCheckSum;
	int m_nCurOffset;
	int m_nBlockNo;

	DWORD m_dwMotorOn;
	DWORD m_dwWriteLED;
	int m_nCurrentDisk;

	STCallbackHandler stLightHandler;

	void UpdateMotorLight();
};



#endif // !defined(AFX_DISKINTERFACE_H__03C7C26F_CB29_4C07_84E9_EB3B20111E6F__INCLUDED_)

