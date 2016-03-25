// Card.h: interface for the CCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARD_H__F3CE19E7_6ECF_4B71_B319_87955300BBCE__INCLUDED_)
#define AFX_CARD_H__F3CE19E7_6ECF_4B71_B319_87955300BBCE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define PM_NATIVE			0x00
#define PM_MB				0x02
#define PM_ECHO				0x03

#define MAX_CARD_TYPES		4

class CAppleClock;

class CCard : public CObject
{
public:
	DECLARE_DYNAMIC( CCard );
	CCard();
	virtual ~CCard();

	virtual void WriteRom(WORD addr, BYTE data);
	virtual BYTE ReadRom(WORD addr);
	virtual void WriteExRom(WORD addr, BYTE data);
	virtual BYTE ReadExRom(WORD addr);
	virtual void InitRomImage();
	virtual void Write(WORD addr, BYTE data);
	virtual BYTE Read(WORD addr);
	virtual void Configure(){ };
	virtual void Clock(int clock){ };
	virtual void Reset(){ };
	virtual void PowerOn(){ };
	virtual void PowerOff() { Reset(); };
	virtual CString& GetDeviceName() { return m_strDeviceName; };
	virtual int GetDeviceNum() { return m_iDeviceNum; };
	virtual void SetDipSwitch(int nMode)
	{
		m_nDipSwitch = nMode;
	};
	virtual BOOL HasExtendRom() { return m_bHasExtendRom; }

	virtual int GetDipSwitch() { return m_nDipSwitch; };

	virtual void Serialize( CArchive &ar );

protected:
	BYTE *m_pbyRom;
	CString m_strDeviceName;
	int m_iDeviceNum;
	int m_nDipSwitch;
	BOOL m_bHasExtendRom;
};

#endif // !defined(AFX_CARD_H__F3CE19E7_6ECF_4B71_B319_87955300BBCE__INCLUDED_)
