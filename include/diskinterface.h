// DiskInterface.h: interface for the CDiskInterface class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKINTERFACE_H__03C7C26F_CB29_4C07_84E9_EB3B20111E6F__INCLUDED_)
#define AFX_DISKINTERFACE_H__03C7C26F_CB29_4C07_84E9_EB3B20111E6F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Card.h"
#include "DiskDrive.h"

#define MAX_DRIVES 2

class CDiskInterface : public CCard  
{
public:
	DECLARE_SERIAL( CDiskInterface );
	//DECLARE_DYNAMIC( CDiskInterface );

	void SetMotorLightHandler(void* objTo, callback_handler func);
	void Clock(int nClock);
	void PowerOff();
	void PowerOn();
	virtual void Reset();

	CDiskInterface();
	virtual ~CDiskInterface();

	virtual void InitRomImage();

	virtual void Write(WORD addr, BYTE data);
	virtual BYTE Read(WORD addr);
 	virtual void Configure(); 

	CDiskDrive* GetDrive(int nDrive) {return m_pDiskDrive[nDrive];};

	void Serialize( CArchive &ar );

protected:
	BYTE m_byLatchWData;
	BYTE m_byLatchAddr;
	BYTE m_byLatchData;
	BYTE m_byStepperStatus;
	DWORD m_dwMotorOn;
	DWORD m_dwWriteLED;
	CDiskDrive* m_pDiskDrive[MAX_DRIVES];

	int m_iCurrentDisk;

	STCallbackHandler stLightHandler;

	void ToggleStepper( BYTE byOffset );
	void UpdateMotorLight();
};

#endif // !defined(AFX_DISKINTERFACE_H__03C7C26F_CB29_4C07_84E9_EB3B20111E6F__INCLUDED_)

