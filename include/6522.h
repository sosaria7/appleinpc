// 6522.h: interface for the C6522 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_6522_H__E7D9408B_2309_402B_B3B0_9BCE8D6EBC9E__INCLUDED_)
#define AFX_6522_H__E7D9408B_2309_402B_B3B0_9BCE8D6EBC9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// output register A
#define SY_ORB		0x00
// output register B
#define SY_ORA		0x01
// data direction register B
#define SY_DDRB		0x02
// data direction register A
#define SY_DDRA		0x03
// read : T1 Low-Order Counter
// write: T1 Low-Order Latches
#define SY_T1C_L	0x04
// read & write : T1 High-Order Counter
#define SY_T1C_H	0x05
// read & write: T1 Low-Order Latches
#define	SY_T1L_L	0x06
// read & write: T1 High-Order Latches
#define SY_T1L_H	0x07
// read : T2 Low-Order Counter
// write: T2 Low-Order Latches
#define SY_T2C_L	0x08
// read & write : T2 High-Order Counter
#define SY_T2C_H	0x09
// Shift Register
#define SY_SR		0x0A
// Auxiliary Control Register
#define SY_ACR		0x0B
// Peripheral Control Register
#define SY_PCR		0x0C
// Interrupt Flag Register
#define	SY_IFR		0x0D
// Interrupt Enable Register
#define SY_IER		0x0E
// output register a w/o handshaking
#define SY_ORAHLP	0x0F

#define SY_BIT_CA2	( 1 << 0 )
#define SY_BIT_CA1	( 1 << 1 )
#define SY_BIT_SR	( 1 << 2 )
#define SY_BIT_CB2	( 1 << 3 )
#define SY_BIT_CB1	( 1 << 4 )
#define SY_BIT_T2	( 1 << 5 )
#define SY_BIT_T1	( 1 << 6 )
#define SY_BIT_IRQ	( 1 << 7 )

class C6522 : public CObject
{
public:
	C6522();
	virtual ~C6522();

	void SetCB1(BOOL cb1);
	void SetCA2(BOOL ca2);
	void SetCA1(BOOL ca1);
	BYTE m_byIRB;
	BYTE m_byIRA;
	BYTE GetORB();
	BYTE GetORA();
	void Clock(WORD clock);
	BOOL GetIRQB();
	void Reset();
	BYTE Read( BYTE reg );
	void Write(BYTE reg, BYTE data);
	BOOL m_bDoubleClock;

	virtual void Serialize( CArchive &ar );

protected:
	void SetIFR7();
	BOOL m_bIRQB;
	BYTE m_abyRegs[16];
	BYTE m_byORB;
	BYTE m_byORA;
private:
	BOOL m_bCA1;
	BOOL m_bCA2;
	BOOL m_bCB1;
	BOOL m_bCB2;
};

#endif // !defined(AFX_6522_H__E7D9408B_2309_402B_B3B0_9BCE8D6EBC9E__INCLUDED_)
