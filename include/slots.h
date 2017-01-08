// Slots.h: interface for the CSlots class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SLOTS_H__3A324547_6F9B_457B_8806_63D948497F5F__INCLUDED_)
#define AFX_SLOTS_H__3A324547_6F9B_457B_8806_63D948497F5F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_SLOTS	7

#include "card.h"

class CSlots : public CObject
{
public:
	DECLARE_SERIAL( CSlots );
	CSlots();
	virtual ~CSlots();

	void SetDipSwitch(int nSlot, int nDipSwitch);
	BOOL InsertCard(int nSlot, int nDeviceNum);
	BOOL InsertCard(int nSlot, CCard* pCard);
	void RemoveCard( int nSlot );
	CCard* GetCard( int slot );
	void Clock( DWORD clock );
	void Reset();
	void Write( WORD addr, BYTE data );
	void WriteRom( WORD addr, BYTE data );
	BYTE ReadRom( WORD addr );
	void PowerOff();
	BYTE Read( WORD addr );
	void PowerOn();
	BOOL Initialize();
	void ConfigureDiskette();
	void ConfigureHardDisk();
	BOOL HasDiskInterface();
	BOOL HasHardDiskInterface();
	BOOL HasMouseInterface();
	void SetDiskette1(CString strDiskPath);
	void SetDiskette2(CString strDiskPath);
	void SetHardDisk(CString strDiskPath);

	void Serialize( CArchive &ar );

protected:
	CCard	*m_slots[MAX_SLOTS];	// slots
	int	m_nDiskSlotNum;
	int m_nHardDiskSlotNum;
	int m_nMouseSlotNum;
	int m_nLastSlotNum;
};

#endif // !defined(AFX_SLOTS_H__3A324547_6F9B_457B_8806_63D948497F5F__INCLUDED_)
