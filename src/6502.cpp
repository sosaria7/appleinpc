// 65c02.cpp: implementation of the C6502 class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "appleclock.h"
#include "memory.h"
#include "6502.h"
#include "65c02_op.h"

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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


C6502::C6502()
	: C65c02()
{
	m_initialized=TRUE;

#ifdef _DEBUG
	m_current=0;
#endif
//	init_6502();
}

C6502::~C6502()
{

}

int C6502::Process()
{
	register BYTE data;
	register WORD addr;
	register WORD result;
	BYTE offset;
	BYTE opcode;
	int clock = 0;
	int preclock = 0;
#ifdef _DEBUG
	WORD opcode_addr = m_regPC;
#endif

	if ((m_uException_Register & SIG_CPU_IRQ))
	{
		// http://6502.org/tutorials/interrupts.html
		m_uException_Register &= ~SIG_CPU_IRQ;
		m_uException_Register &= ~SIG_CPU_WAIT;
		if (!(m_regF & I_Flag) && !(m_regF & B_Flag))
		{
			TRACE_CALL;
			PUSH(m_regPC >> 8);
			PUSH(m_regPC & 0xFF);
			PUSH(m_regF);
			m_regPC = READMEM16(0xFFFE);
			CLOCK(7);
			m_regF |= I_Flag;
#ifdef MC65C02
			m_regF &= ~D_Flag;
#endif
			return clock;
		}
		else
		{
//			PendingIRQ++;
		}
	}

	if ((m_uException_Register & SIG_CPU_SHUTDOWN)
		|| (m_uException_Register & SIG_CPU_WAIT))
	{
		CLOCK(3);
		return clock;
	}

	opcode = READOPCODE8;

	switch (opcode)
	{
		/* ADC Immediate */
	case 0x69:
		IMM; CLOCK(2); ADC; WACC; break;
		/* ADC ZeroPage */
	case 0x65:
		ZP; CLOCK(3); MEM; ADC; WACC; break;
		/* ADC ZeroPage, X */
	case 0x75:
		ZP_X; CLOCK(4); MEM; ADC; WACC; break;
		/* ADC Absolute */
	case 0x6d:
		ABS; CLOCK(4); MEM; ADC; WACC; break;
		/* ADC Absolute, X */
	case 0x7d:
		ABS_X; CLOCK(4); MEM; ADC; WACC; break;
		/* ADC Absolute, Y */
	case 0x79:
		ABS_Y; CLOCK(4); MEM; ADC; WACC; break;
		/* ADC (Indirect, X) */
	case 0x61:
		IND_X; CLOCK(6); MEM; ADC; WACC; break;
		/* ADC (Indirect), Y */
	case 0x71:
		IND_Y; CLOCK(5); MEM; ADC; WACC; break;
		/* AND Immediate */
	case 0x29:
		IMM; CLOCK(2); AND; WACC; break;
		/* AND ZeroPage */
	case 0x25:
		ZP; CLOCK(3); MEM; AND; WACC; break;
		/* AND ZeroPage, X */
	case 0x35:
		ZP_X; CLOCK(4); MEM; AND; WACC; break;
		/* AND Absolute */
	case 0x2d:
		ABS; CLOCK(4); MEM; AND; WACC; break;
		/* AND Absolute, X */
	case 0x3d:
		ABS_X; CLOCK(4); MEM; AND; WACC; break;
		/* AND Absolute, Y */
	case 0x39:
		ABS_Y; CLOCK(4); MEM; AND; WACC; break;
		/* AND (Indirect, X) */
	case 0x21:
		IND_X; CLOCK(6); MEM; AND; WACC; break;
		/* AND (Indirect), Y */
	case 0x31:
		IND_Y; CLOCK(5); MEM; AND; WACC; break;
		/* ASL Accumulator */
	case 0x0a:
		ACC; ASL; WACC; CLOCK(2); break;
		/* ASL ZeroPage */
	case 0x06:
		ZP; MEM; ASL; CLOCK(5); WMEM; break;
		/* ASL ZeroPage, X */
	case 0x16:
		ZP_X; MEM; ASL; CLOCK(6); WMEM; break;
		/* ASL Absolute */
	case 0x0e:
		ABS; MEM; ASL; CLOCK(6); WMEM; break;
		/* ASL Absolute, X */
	case 0x1e:
#ifdef MC65C02
		ABS_X; MEM; ASL; CLOCK(6); WMEM; break;
#else
		ABS_X2; MEM; ASL; CLOCK(7); WMEM; break;
#endif
		/* BCC rr */
	case 0x90:
		BRA_NCOND(C_Flag); CLOCK(2); break;
		/* BCS rr */
	case 0xb0:
		BRA_COND(C_Flag); CLOCK(2); break;
		/* BEQ rr */
	case 0xf0:
		BRA_COND(Z_Flag); CLOCK(2); break;
		/* BIT ZeroPage */
	case 0x24:
		ZP; CLOCK(3); MEM; BIT; break;
		/* BIT Absolute */
	case 0x2c:
		ABS; CLOCK(4); MEM; BIT; break;
		/* BMI rr */
	case 0x30:
		BRA_COND(N_Flag); CLOCK(2); break;
		/* BNE rr */
	case 0xd0:
		BRA_NCOND(Z_Flag); CLOCK(2); break;
		/* BPL rr */
	case 0x10:
		BRA_NCOND(N_Flag); CLOCK(2); break;
		/* BRK */
	case 0x00:
		TRACE_CALL;
#ifdef _DEBUG
		TRACE("BRK at $%04X\n", m_regPC - 1);
		{
			int d_i, d_current;
			char buffer[40];
			d_current = m_current;
			TRACE("Trace PC register\n");
			for (d_i = 0; d_i < 10; d_i++)
			{
				TRACE("    jmp at $%04X\n", m_trace[(--d_current) & 0xFF]);
			}
			TRACE("Stack : $%02X\n    ", m_regS);
			for (d_i = 0; d_i < 10; d_i++)
			{
				//TRACE("%02X ", READMEM8(((m_regS + d_i) & 0xFF) + 0x100));
				snprintf(buffer + d_i * 3, 40 - d_i * 3, "%02X ", READMEM8(((m_regS + d_i) & 0xFF) + 0x100));
			}
			TRACE("%s\n", buffer);
			//TRACE("\n");
		}
#endif
		BRK; CLOCK(7); break;
		/* BVC rr */
	case 0x50:
		BRA_NCOND(V_Flag); CLOCK(2); break;
		/* BVS rr */
	case 0x70:
		BRA_COND(V_Flag); CLOCK(2); break;
		/* CLC */
	case 0x18:
		CLEAR_FLAG(C_Flag); CLOCK(2); break;
		/* CLD */
	case 0xd8:
		CLEAR_FLAG(D_Flag); CLOCK(2); break;
		/* CLI */
	case 0x58:
		CLEAR_FLAG(I_Flag); CHECK_IRQ; CLOCK(2); break;
		/* CLV */
	case 0xb8:
		CLEAR_FLAG(V_Flag); CLOCK(2); break;
		/* CMP Immediate */
	case 0xc9:
		IMM; CMP; CLOCK(2); break;
		/* CMP ZeroPage */
	case 0xc5:
		ZP; CLOCK(3); MEM; CMP; break;
		/* CMP ZeroPage, X */
	case 0xd5:
		ZP_X; CLOCK(4); MEM; CMP; break;
		/* CMP Absolute */
	case 0xcd:
		ABS; CLOCK(4); MEM; CMP; break;
		/* CMP Absolute, X */
	case 0xdd:
		ABS_X; CLOCK(4); MEM; CMP; break;
		/* CMP Absolute, Y */
	case 0xd9:
		ABS_Y; CLOCK(4); MEM; CMP; break;
		/* CMP (Indirect, X) */
	case 0xc1:
		IND_X; CLOCK(6); MEM; CMP; break;
		/* CMP (Indirect), Y */
	case 0xd1:
		IND_Y; CLOCK(5); MEM; CMP; break;
		/* CPX Immediate */
	case 0xe0:
		IMM; CPX; CLOCK(2); break;
		/* CPX ZeroPage */
	case 0xe4:
		ZP; CLOCK(3); MEM; CPX; break;
		/* CPX Absolute */
	case 0xec:
		ABS; CLOCK(4); MEM; CPX; break;
		/* CPY Immediate */
	case 0xc0:
		IMM; CPY; CLOCK(2); break;
		/* CPY ZeroPage */
	case 0xc4:
		ZP; CLOCK(3); MEM; CPY; break;
		/* CPY Absolute */
	case 0xcc:
		ABS; CLOCK(4); MEM; CPY; break;
		/* DEC ZeroPage */
	case 0xc6:
		ZP; MEM; DEC; CLOCK(5); WMEM; break;
		/* DEC ZeroPage, X */
	case 0xd6:
		ZP_X; MEM; DEC; CLOCK(6); WMEM; break;
		/* DEC Absolute */
	case 0xce:
		ABS; MEM; DEC; CLOCK(6); WMEM; break;
		/* DEC Absolute, X */
	case 0xde:
		ABS_X2; MEM; DEC; CLOCK(7); WMEM; break;
		/* DEX */
	case 0xca:
		XREG; DEC; WXREG; CLOCK(2); break;
		/* DEY */
	case 0x88:
		YREG; DEC; WYREG; CLOCK(2); break;
		/* EOR Immediate */
	case 0x49:
		IMM; EOR; WACC; CLOCK(2); break;
		/* EOR ZeroPage */
	case 0x45:
		ZP; CLOCK(3); MEM; EOR; WACC; break;
		/* EOR ZeroPage, X */
	case 0x55:
		ZP_X; CLOCK(4); MEM; EOR; WACC; break;
		/* EOR Absolute */
	case 0x4d:
		ABS; CLOCK(4); MEM; EOR; WACC; break;
		/* EOR Absolute, X */
	case 0x5d:
		ABS_X; CLOCK(4); MEM; EOR; WACC; break;
		/* EOR Absolute, Y */
	case 0x59:
		ABS_Y; CLOCK(4); MEM; EOR; WACC; break;
		/* EOR (Indirect, X) */
	case 0x41:
		IND_X; CLOCK(6); MEM; EOR; WACC; break;
		/* EOR (Indirect), Y */
	case 0x51:
		IND_Y; CLOCK(5); MEM; EOR; WACC; break;
		/* INC ZeroPage */
	case 0xe6:
		ZP; MEM; INC; CLOCK(5); WMEM; break;
		/* INC ZeroPage, X */
	case 0xf6:
		ZP_X; MEM; INC; CLOCK(6); WMEM; break;
		/* INC Absolute */
	case 0xee:
		ABS; MEM; INC; CLOCK(6); WMEM; break;
		/* INC Absolute, X */
	case 0xfe:
		ABS_X2; MEM; INC; CLOCK(7); WMEM; break;
		/* INX */
	case 0xe8:
		XREG; INC; WXREG; CLOCK(2); break;
		/* INY */
	case 0xc8:
		YREG; INC; WYREG; CLOCK(2); break;
		/* JMP Absolute */
	case 0x4c:
		ABS; JMP; CLOCK(3); break;
		/* JMP (Indirect16) */
	case 0x6c:
		// 65c02 : 6 cycle
		// 6502 : 5 cycle
#ifdef MC65C02
		IND16; JMP; CLOCK(6); break;
#else
		IND16; JMP; CLOCK(5); break;
#endif
		/* JSR Absolute */
	case 0x20:
		ABS; JSR; CLOCK(6); break;
		/* LDA Immediate */
	case 0xa9:
		IMM; LOAD; WACC; CLOCK(2); break;
		/* LDA ZeroPage */
	case 0xa5:
		ZP; CLOCK(3); MEM; LOAD; WACC; break;
		/* LDA ZeroPage, X */
	case 0xb5:
		ZP_X; CLOCK(4); MEM; LOAD; WACC; break;
		/* LDA Absolute */
	case 0xad:
		ABS; CLOCK(4); MEM; LOAD; WACC; break;
		/* LDA Absolute, X */
	case 0xbd:
		ABS_X; CLOCK(4); MEM; LOAD; WACC; break;
		/* LDA Absolute, Y */
	case 0xb9:
		ABS_Y; CLOCK(4); MEM; LOAD; WACC; break;
		/* LDA (Indirect, X) */
	case 0xa1:
		IND_X; CLOCK(6); MEM; LOAD; WACC; break;
		/* LDA (Indirect), Y */
	case 0xb1:
		IND_Y; CLOCK(5); MEM; LOAD; WACC; break;
		/* LDX Immediate */
	case 0xa2:
		IMM; LOAD; WXREG; CLOCK(2); break;
		/* LDX ZeroPage */
	case 0xa6:
		ZP; CLOCK(3); MEM; LOAD; WXREG; break;
		/* LDX ZeroPage, Y */
	case 0xb6:
		ZP_Y; CLOCK(4); MEM; LOAD; WXREG; break;
		/* LDX Absolute */
	case 0xae:
		ABS; CLOCK(4); MEM; LOAD; WXREG; break;
		/* LDX Absolute, Y */
	case 0xbe:
		ABS_Y; CLOCK(4); MEM; LOAD; WXREG; break;
		/* LDY Immediate */
	case 0xa0:
		IMM; LOAD; WYREG; CLOCK(2); break;
		/* LDY ZeroPage */
	case 0xa4:
		ZP; CLOCK(3); MEM; LOAD; WYREG; break;
		/* LDY ZeroPage, X */
	case 0xb4:
		ZP_X; CLOCK(4); MEM; LOAD; WYREG; break;
		/* LDY Absolute */
	case 0xac:
		ABS; CLOCK(4); MEM; LOAD; WYREG; break;
		/* LDY Absolute, X */
	case 0xbc:
		ABS_X; CLOCK(4); MEM; LOAD; WYREG; break;
		/* LSR Accmulator */
	case 0x4a:
		ACC; LSR; WACC; CLOCK(2); break;
		/* LSR ZeroPage */
	case 0x46:
		ZP; MEM; LSR; CLOCK(5); WMEM; break;
		/* LSR ZeroPage, X */
	case 0x56:
		ZP_X; MEM; LSR; CLOCK(6); WMEM; break;
		/* LSR Absolute */
	case 0x4e:
		ABS; MEM; LSR; CLOCK(6); WMEM; break;
		/* LSR Absolute, X */
	case 0x5e:
#ifdef MC65C02
		ABS_X; MEM; LSR; CLOCK(6); WMEM; break;
#else
		ABS_X2; MEM; LSR; CLOCK(7); WMEM; break;
#endif
		/* NOP */
	case 0xea:
		CLOCK(2); break;
		/* ORA Immediate */
	case 0x09:
		IMM; ORA; WACC; CLOCK(2); break;
		/* ORA ZeroPage */
	case 0x05:
		ZP; CLOCK(3); MEM; ORA; WACC; break;
		/* ORA ZeroPage, X */
	case 0x15:
		ZP_X; CLOCK(4); MEM; ORA; WACC; break;
		/* ORA Absolute */
	case 0x0d:
		ABS; CLOCK(4); MEM; ORA; WACC; break;
		/* ORA Absolute, X */
	case 0x1d:
		ABS_X; CLOCK(4); MEM; ORA; WACC; break;
		/* ORA Absolute, Y */
	case 0x19:
		ABS_Y; CLOCK(4); MEM; ORA; WACC; break;
		/* ORA (Indirect, X) */
	case 0x01:
		IND_X; CLOCK(6); MEM; ORA; WACC; break;
		/* ORA (Indirect), Y */
	case 0x11:
		IND_Y; CLOCK(5); MEM; ORA; WACC; break;
		/* PHA */
	case 0x48:
		PUSH(this->m_regA); CLOCK(3); break;
		/* PHP */
	case 0x08:
		PUSH(this->m_regF); CLOCK(3); break;
		/* PLA */
	case 0x68:
		POPR; WACC; CLOCK(4); break;
		/* PLP */
	case 0x28:
		POPF; CLOCK(4); break;
		/* ROL Accumulator */
	case 0x2a:
		ACC; ROL; WACC; CLOCK(2); break;
		/* ROL ZeroPage */
	case 0x26:
		ZP; MEM; ROL; CLOCK(5); WMEM; break;
		/* ROL ZeroPage, X */
	case 0x36:
		ZP_X; MEM; ROL; CLOCK(6); WMEM; break;
		/* ROL Absolute */
	case 0x2e:
		ABS; MEM; ROL; CLOCK(6); WMEM; break;
		/* ROL Absolute, X */
	case 0x3e:
#ifdef MC65C02
		ABS_X; MEM; ROL; CLOCK(6); WMEM; break;
#else
		ABS_X2; MEM; ROL; CLOCK(7); WMEM; break;
#endif
		/* ROR Accumulator */
	case 0x6a:
		ACC; ROR; WACC; CLOCK(2); break;
		/* ROR ZeroPage */
	case 0x66:
		ZP; MEM; ROR; CLOCK(5); WMEM; break;
		/* ROR ZeroPage, X */
	case 0x76:
		ZP_X; MEM; ROR; CLOCK(6); WMEM; break;
		/* ROR Absolute */
	case 0x6e:
		ABS; MEM; ROR; CLOCK(6); WMEM; break;
		/* ROR Absolute, X */
	case 0x7e:
#ifdef MC65C02
		ABS_X; MEM; ROR; CLOCK(6); WMEM; break;
#else
		ABS_X2; MEM; ROR; CLOCK(7); WMEM; break;
#endif
		/* RTI */
	case 0x40:
		RTI; /*CHECK_IRQ;*/ CLOCK(6); break;
		/* RTS */
	case 0x60:
		RTS; CLOCK(6); break;
		/* SBC Immediate */
	case 0xe9:
		IMM; CLOCK(2); SBC; WACC; break;
		/* SBC ZeroPage */
	case 0xe5:
		ZP; CLOCK(3); MEM; SBC; WACC; break;
		/* SBC ZeroPage, X */
	case 0xf5:
		ZP_X; CLOCK(4); MEM; SBC; WACC; break;
		/* SBC Absolute */
	case 0xed:
		ABS; CLOCK(4); MEM; SBC; WACC; break;
		/* SBC Absolute, X */
	case 0xfd:
		ABS_X; CLOCK(4); MEM; SBC; WACC; break;
		/* SBC Absolute, Y */
	case 0xf9:
		ABS_Y; CLOCK(4); MEM; SBC; WACC; break;
		/* SBC (Indirect, X) */
	case 0xe1:
		IND_X; CLOCK(6); MEM; SBC; WACC; break;
		/* SBC (Indirect), Y */
	case 0xf1:
		IND_Y; CLOCK(5); MEM; SBC; WACC; break;
		/* SEC */
	case 0x38:
		SET_FLAG(C_Flag); CLOCK(2); break;
		/* SED */
	case 0xf8:
		SET_FLAG(D_Flag); CLOCK(2); break;
		/* SEI */
	case 0x78:
		SET_FLAG(I_Flag); CLOCK(2); break;
		/* STA ZeroPage */
	case 0x85:
		ZP; CLOCK(3); STA; break;
		/* STA ZeroPage, X */
	case 0x95:
		ZP_X; CLOCK(4); STA; break;
		/* STA Absolute */
	case 0x8d:
		ABS; CLOCK(4); STA; break;
		/* STA Absolute, X */
	case 0x9d:
		ABS_X2; CLOCK(5); STA; break;
		/* STA Absolute, Y */
	case 0x99:
		ABS_Y2; CLOCK(5); STA; break;
		/* STA (Indirect, X) */
	case 0x81:
		IND_X; CLOCK(6); STA; break;
		/* STA (Indirect), Y */
	case 0x91:
		IND_Y2; CLOCK(6); STA; break;
		/* STX ZeroPage */
	case 0x86:
		ZP; CLOCK(3); STX; break;
		/* STX ZeroPage, Y */
	case 0x96:
		ZP_Y; CLOCK(4); STX; break;
		/* STX Absolute */
	case 0x8e:
		ABS; CLOCK(4); STX; break;
		/* STY ZeroPage */
	case 0x84:
		ZP; CLOCK(3); STY; break;
		/* STY ZeroPage, X */
	case 0x94:
		ZP_X; CLOCK(4); STY; break;
		/* STY Absolute */
	case 0x8c:
		ABS; CLOCK(4); STY; break;
		/* TAX */
	case 0xaa:
		TAX; CLOCK(2); break;
		/* TAY */
	case 0xa8:
		TAY; CLOCK(2); break;
		/* TSX */
	case 0xba:
		TSX; CLOCK(2); break;
		/* TXA */
	case 0x8a:
		TXA; CLOCK(2); break;
		/* TXS */
	case 0x9a:
		TXS; CLOCK(2); break;
		/* TYA */
	case 0x98:
		TYA; CLOCK(2); break;

#ifdef MC65C02
		/* ADC (Indirect) */
	case 0x72:
		IND; CLOCK(5); MEM; ADC; WACC; break;
		/* AND (Indirect) */
	case 0x32:
		IND; CLOCK(5); MEM; AND; WACC; break;
		/* BBR(bit) ZeroPage rr */
	case 0x0f:
		ZP; MEM; BBR(0x01); CLOCK(5); break;
	case 0x1f:
		ZP; MEM; BBR(0x02); CLOCK(5); break;
	case 0x2f:
		ZP; MEM; BBR(0x04); CLOCK(5); break;
	case 0x3f:
		ZP; MEM; BBR(0x08); CLOCK(5); break;
	case 0x4f:
		ZP; MEM; BBR(0x10); CLOCK(5); break;
	case 0x5f:
		ZP; MEM; BBR(0x20); CLOCK(5); break;
	case 0x6f:
		ZP; MEM; BBR(0x40); CLOCK(5); break;
	case 0x7f:
		ZP; MEM; BBR(0x80); CLOCK(5); break;
		/* BBS(bit) ZeroPage rr */
	case 0x8f:
		ZP; MEM; BBS(0x01); CLOCK(5); break;
	case 0x9f:
		ZP; MEM; BBS(0x02); CLOCK(5); break;
	case 0xaf:
		ZP; MEM; BBS(0x04); CLOCK(5); break;
	case 0xbf:
		ZP; MEM; BBS(0x08); CLOCK(5); break;
	case 0xcf:
		ZP; MEM; BBS(0x10); CLOCK(5); break;
	case 0xdf:
		ZP; MEM; BBS(0x20); CLOCK(5); break;
	case 0xef:
		ZP; MEM; BBS(0x40); CLOCK(5); break;
	case 0xff:
		ZP; MEM; BBS(0x80); CLOCK(5); break;
		/* RMB(bit) ZeroPage */
	case 0x07:
		ZP; MEM; RMB(0x01); CLOCK(5); WMEM; break;
	case 0x17:
		ZP; MEM; RMB(0x02); CLOCK(5); WMEM; break;
	case 0x27:
		ZP; MEM; RMB(0x04); CLOCK(5); WMEM; break;
	case 0x37:
		ZP; MEM; RMB(0x08); CLOCK(5); WMEM; break;
	case 0x47:
		ZP; MEM; RMB(0x10); CLOCK(5); WMEM; break;
	case 0x57:
		ZP; MEM; RMB(0x20); CLOCK(5); WMEM; break;
	case 0x67:
		ZP; MEM; RMB(0x40); CLOCK(5); WMEM; break;
	case 0x77:
		ZP; MEM; RMB(0x80); CLOCK(5); WMEM; break;
		/* SMB(bit) ZeroPage */
	case 0x87:
		ZP; MEM; SMB(0x01); CLOCK(5); WMEM; break;
	case 0x97:
		ZP; MEM; SMB(0x02); CLOCK(5); WMEM; break;
	case 0xa7:
		ZP; MEM; SMB(0x04); CLOCK(5); WMEM; break;
	case 0xb7:
		ZP; MEM; SMB(0x08); CLOCK(5); WMEM; break;
	case 0xc7:
		ZP; MEM; SMB(0x10); CLOCK(5); WMEM; break;
	case 0xd7:
		ZP; MEM; SMB(0x20); CLOCK(5); WMEM; break;
	case 0xe7:
		ZP; MEM; SMB(0x40); CLOCK(5); WMEM; break;
	case 0xf7:
		ZP; MEM; SMB(0x80); CLOCK(5); WMEM; break;
		/* BIT Immediate */
	case 0x89:
		IMM; BIT; CLOCK(2); break;
		/* BIT ZeroPage, X */
	case 0x34:
		ZP_X; MEM; BIT; CLOCK(2); break;
		/* BIT Absolute, X */
	case 0x3c:
		ABS_X; CLOCK(4); MEM; BIT; break;
		/* BRA rr */
	case 0x80:
		BRA; CLOCK(2); break;
		/* CMP (Indirect) */
	case 0xd2:
		IND; CLOCK(5); MEM; CMP; break;
		/* DEA */
	case 0x3a:
		ACC; DEC; WACC; CLOCK(2); break;
		/* EOR (Indirect) */
	case 0x52:
		IND; CLOCK(5); MEM; EOR; WACC; break;
		/* INA */
	case 0x1a:
		ACC; INC; WACC; CLOCK(2); break;
		/* JMP (Absolute, X) */
	case 0x7c:
		IND16_X; JMP; CLOCK(6); break;
		/* LDA (Indirect) */
	case 0xb2:
		IND; CLOCK(5); MEM; LOAD; WACC; break;
		/* ORA (Indirect) */
	case 0x12:
		IND; CLOCK(5); MEM; ORA; WACC; break;
		/* PHX */
	case 0xda:
		PUSH(this->m_regX); CLOCK(3); break;
		/* PHY */
	case 0x5a:
		PUSH(this->m_regY); CLOCK(3); break;
		/* PLX */
	case 0xfa:
		POPR; WXREG; CLOCK(4); break;
		/* PLY */
	case 0x7a:
		POPR; WYREG; CLOCK(4); break;
		/* SBC (Indirect) */
	case 0xf2:
		IND; CLOCK(5); MEM; SBC; WACC; break;
		/* STA (Indirect) */
	case 0x92:
		IND; CLOCK(5); STA; break;
		/* STP */
	case 0xdb:
		STOP; CLOCK(3); break;
		/* STZ ZeroPage */
	case 0x64:
		ZP; CLOCK(3); STZ; break;
		/* STZ ZeroPage, X */
	case 0x74:
		ZP_X; CLOCK(3); STZ; break;
		/* STZ Absolute */
	case 0x9c:
		ABS; CLOCK(4); STZ; break;
		/* STZ Absolute, X */
	case 0x9e:
		ABS_X2; CLOCK(5); STZ; break;
		/* TRB ZeroPage */
	case 0x14:
		ZP; MEM; TRB; CLOCK(5); WMEM; break;
		/* TRB Absolute */
	case 0x1c:
		ABS; MEM; TRB; CLOCK(6); WMEM; break;
		/* TSB ZeroPage */
	case 0x04:
		ZP; MEM; TSB; CLOCK(5); WMEM; break;
		/* TSB Absolute */
	case 0x0c:
		ABS; MEM; TSB; CLOCK(6); WMEM; break;
		/* WAI */
	case 0xcb:
		WAIT; CLOCK(3); break;
#else

#endif
	default:
		CLOCK(2); break;
	}

	return clock;
}


void C6502::Serialize(CArchive &ar)
{
	C65c02::Serialize(ar);
}
