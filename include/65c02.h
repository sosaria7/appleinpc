// 65c02.h: interface for the C65c02 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_65C02_H__5D3D7C73_163F_46A5_8DB1_58D7D3BC6496__INCLUDED_)
#define AFX_65C02_H__5D3D7C73_163F_46A5_8DB1_58D7D3BC6496__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// gloval value

#define RebootSig		0x01
#define ResetSig		0x02
#define DebugStepSig	0x04
#define EnterDebugSig	0x08

#define C_Flag			0x1			/* 6502 Carry	 	   */
#define Z_Flag			0x2			/* 6502 Zero		   */
#define I_Flag			0x4			/* 6502 Interrupt disable  */
#define D_Flag			0x8			/* 6502 Decimal mode	   */
#define B_Flag			0x10		/* 6502 Break		   */
#define X_Flag			0x20		/* 6502 Xtra		   */
#define V_Flag			0x40		/* 6502 Overflow	   */
#define N_Flag			0x80		/* 6502 Neg		   */

#define NZ_Flag			0x82
#define NZC_Flag		0x83
#define NV_Flag			0xC0
#define NVZ_Flag		0xC2
#define NVZC_Flag		0xC3
#define C_Flag_Bit	0		/* 6502 Carry		   */
#define Z_Flag_Bit	1		/* 6502 Zero		   */
#define I_Flag_Bit	2		/* 6502 Interrupt disable  */
#define D_Flag_Bit	3		/* 6502 Decimal mode	   */
#define B_Flag_Bit	4		/* 6502 Break		   */
#define X_Flag_Bit	5		/* 6502 Xtra		   */
#define V_Flag_Bit	6		/* 6502 Overflow	   */
#define N_Flag_Bit	7		/* 6502 Neg		   */

#define REFER_NMIB			0xFFFA
#define REFER_RESET			0xFFFC
#define REFER_IRQ			0xFFFE

#include "localClock.h"
#include "cpu.h"

class CAppleClock;

class C65c02 : public CCpu
{
public:
	BOOL m_initialized;

	C65c02();
	virtual ~C65c02();

	BYTE GetRegF();
	void init_6502();
	void init_optable();
	WORD getRegPC(void);

	// overrided from CCpu
	int Process();
	void Reset();

	void Serialize(CArchive &archive);

public:

	BYTE m_regX;				// 6502 X register
	BYTE m_regY;				// 6502 Y register
	BYTE m_regA;				// 6502 A register
	BYTE m_regF;				// 6502 flags
	BYTE m_regS;				// 6502 Stack pointer
	WORD m_BCD_Table1[512];
	WORD m_BCD_Table2[512];
protected:
	int m_nClock;

protected:
#ifdef _DEBUG
	BYTE	m_current;
	WORD	m_trace[256];
#endif
	WORD	m_regPC;				// 6502 Program Counter

	WORD	getAbs();
	WORD	getAbsX();
	WORD	getAbsY();
	WORD	getAbsXINC();
	WORD	getZp();
	WORD	getZpX();
	WORD	getZpY();
	WORD	getIndX();
	WORD	getIndX16();
	WORD	getIndY();
	WORD	getInd16();
	WORD	getInd();
	void	updateFlagNZ( WORD result );
	void	updateFlagNZC( WORD result );
	void	updateFlag( BYTE result, BYTE flag );
	void	branch( BYTE offset );
	void	checkCross( WORD addr, WORD offset );

};

#endif // !defined(AFX_65C02_H__5D3D7C73_163F_46A5_8DB1_58D7D3BC6496__INCLUDED_)
