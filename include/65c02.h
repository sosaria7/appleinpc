// 65c02.h: interface for the C65c02 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __65C02_H__
#define __65C02_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// gloval value


#include "cpu.h"


class C65c02 : public CCpu
{
public:
	BOOL m_initialized;

	C65c02();
	virtual ~C65c02();

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

};

#endif
