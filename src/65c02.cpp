// 65c02.cpp: implementation of the C65c02 class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MC65C02

#include "arch/frame/stdafx.h"
#include "appleclock.h"
#include "memory.h"
#include "65c02.h"
#include "65c02_op.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define TRACE_ADDR  m_trace[(m_current++)&0xFF] = ( m_regPC - 1 ) & 0xFFFF;
#else
#define TRACE_ADDR
#endif


#ifdef _DEBUG
int g_breakpoint = -1;
#endif

C65c02::C65c02()
{
	m_initialized=TRUE;

#ifdef _DEBUG
	m_current=0;
#endif
//	init_6502();
}

C65c02::~C65c02()
{

}

int C65c02::Process()
{
	register BYTE data;
	register WORD addr;
	register WORD result;
	BYTE offset;
	BYTE opcode;
	int clock = 0;

	if ( ( m_uException_Register & SIG_CPU_IRQ ) )
	{
		m_uException_Register &= ~SIG_CPU_IRQ;
		m_uException_Register &= ~SIG_CPU_WAIT;
		if ( !( m_regF & I_Flag ) && !( m_regF & B_Flag ) )
		{
			TRACE_ADDR
			PUSH( m_regPC >> 8 );
			PUSH( m_regPC & 0xFF );
			PUSH( m_regF );
			m_regPC = READMEM16( 0xFFFE );
			clock += 7;
			m_regF |= I_Flag;
			return clock;
		}
		else
		{
//			PendingIRQ++;
		}
	}

	if ( ( m_uException_Register & SIG_CPU_SHUTDOWN )
		|| ( m_uException_Register & SIG_CPU_WAIT ) )
	{
		clock += 3;
		return clock;
	}

	opcode = READOPCODE8;

	switch (opcode)
	{
		/* ADC Immediate */
	case 0x69:
		IMM; ADC; WACC; CLOCK(2); break;
		/* ADC ZeroPage */
	case 0x65:
		ZP; MEM; ADC; WACC; CLOCK(3); break;
		/* ADC ZeroPage, X */
	case 0x75:
		ZP_X; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute */
	case 0x6d:
		ABS; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute, X */
	case 0x7d:
		ABS_X; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC Absolute, Y */
	case 0x79:
		ABS_Y; MEM; ADC; WACC; CLOCK(4); break;
		/* ADC (Indirect, X) */
	case 0x61:
		IND_X; MEM; ADC; WACC; CLOCK(6); break;
		/* ADC (Indirect), Y */
	case 0x71:
		IND_Y; MEM; ADC; WACC; CLOCK(5); break;
		/* AND Immediate */
	case 0x29:
		IMM; AND; WACC; CLOCK(2); break;
		/* AND ZeroPage */
	case 0x25:
		ZP; MEM; AND; WACC; CLOCK(3); break;
		/* AND ZeroPage, X */
	case 0x35:
		ZP_X; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute */
	case 0x2d:
		ABS; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute, X */
	case 0x3d:
		ABS_X; MEM; AND; WACC; CLOCK(4); break;
		/* AND Absolute, Y */
	case 0x39:
		ABS_Y; MEM; AND; WACC; CLOCK(4); break;
		/* AND (Indirect, X) */
	case 0x21:
		IND_X; MEM; AND; WACC; CLOCK(6); break;
		/* AND (Indirect), Y */
	case 0x31:
		IND_Y; MEM; AND; WACC; CLOCK(5); break;
		/* ASL Accumulator */
	case 0x0a:
		ACC; ASL; WACC; CLOCK(2); break;
		/* ASL ZeroPage */
	case 0x06:
		ZP; MEM; ASL; WMEM; CLOCK(5); break;
		/* ASL ZeroPage, X */
	case 0x16:
		ZP_X; MEM; ASL; WMEM; CLOCK(6); break;
		/* ASL Absolute */
	case 0x0e:
		ABS; MEM; ASL; WMEM; CLOCK(6); break;
		/* ASL Absolute, X */
	case 0x1e:
		ABS_X; MEM; ASL; WMEM; CLOCK(7); break;
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
		ZP; MEM; BIT; CLOCK(3); break;
		/* BIT Absolute */
	case 0x2c:
		ABS; MEM; BIT; CLOCK(3); break;
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
		TRACE_ADDR
#ifdef _DEBUG
			TRACE("BRK at $%04X\n", m_regPC - 1);
		{
			int d_i, d_current;
			d_current = m_current;
			TRACE("Trace PC register\n");
			for (d_i = 0; d_i < 10; d_i++)
			{
				TRACE("    jmp at $%04X\n", m_trace[(--d_current) & 0xFF]);
			}
			TRACE("Stack : $%02X\n    ", m_regS);
			for (d_i = 0; d_i < 10; d_i++)
			{
				TRACE("%02X ", READMEM8(((m_regS + d_i) & 0xFF) + 0x100));
			}
			TRACE("\n");
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
		ZP; MEM; CMP; CLOCK(3); break;
		/* CMP ZeroPage, X */
	case 0xd5:
		ZP_X; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute */
	case 0xcd:
		ABS; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute, X */
	case 0xdd:
		ABS_X; MEM; CMP; CLOCK(4); break;
		/* CMP Absolute, Y */
	case 0xd9:
		ABS_Y; MEM; CMP; CLOCK(4); break;
		/* CMP (Indirect, X) */
	case 0xc1:
		IND_X; MEM; CMP; CLOCK(6); break;
		/* CMP (Indirect), Y */
	case 0xd1:
		IND_Y; MEM; CMP; CLOCK(5); break;
		/* CPX Immediate */
	case 0xe0:
		IMM; CPX; CLOCK(2); break;
		/* CPX ZeroPage */
	case 0xe4:
		ZP; MEM; CPX; CLOCK(3); break;
		/* CPX Absolute */
	case 0xec:
		ABS; MEM; CPX; CLOCK(4); break;
		/* CPY Immediate */
	case 0xc0:
		IMM; CPY; CLOCK(2); break;
		/* CPY ZeroPage */
	case 0xc4:
		ZP; MEM; CPY; CLOCK(3); break;
		/* CPY Absolute */
	case 0xcc:
		ABS; MEM; CPY; CLOCK(4); break;
		/* DEC ZeroPage */
	case 0xc6:
		ZP; MEM; DEC; WMEM; CLOCK(5); break;
		/* DEC ZeroPage, X */
	case 0xd6:
		ZP_X; MEM; DEC; WMEM; CLOCK(6); break;
		/* DEC Absolute */
	case 0xce:
		ABS; MEM; DEC; WMEM; CLOCK(6); break;
		/* DEC Absolute, X */
	case 0xde:
		ABS_X; MEM; DEC; WMEM; CLOCK(7); break;
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
		ZP; MEM; EOR; WACC; CLOCK(3); break;
		/* EOR ZeroPage, X */
	case 0x55:
		ZP_X; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute */
	case 0x4d:
		ABS; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute, X */
	case 0x5d:
		ABS_X; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR Absolute, Y */
	case 0x59:
		ABS_Y; MEM; EOR; WACC; CLOCK(4); break;
		/* EOR (Indirect, X) */
	case 0x41:
		IND_X; MEM; EOR; WACC; CLOCK(6); break;
		/* EOR (Indirect), Y */
	case 0x51:
		IND_Y; MEM; EOR; WACC; CLOCK(5); break;
		/* INC ZeroPage */
	case 0xe6:
		ZP; MEM; INC; WMEM; CLOCK(5); break;
		/* INC ZeroPage, X */
	case 0xf6:
		ZP_X; MEM; INC; WMEM; CLOCK(6); break;
		/* INC Absolute */
	case 0xee:
		ABS; MEM; INC; WMEM; CLOCK(6); break;
		/* INC Absolute, X */
	case 0xfe:
		ABS_X; MEM; INC; WMEM; CLOCK(7); break;
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
		ZP; MEM; LOAD; WACC; CLOCK(3); break;
		/* LDA ZeroPage, X */
	case 0xb5:
		ZP_X; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute */
	case 0xad:
		ABS; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute, X */
	case 0xbd:
		ABS_X; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA Absolute, Y */
	case 0xb9:
		ABS_Y; MEM; LOAD; WACC; CLOCK(4); break;
		/* LDA (Indirect, X) */
	case 0xa1:
		IND_X; MEM; LOAD; WACC; CLOCK(6); break;
		/* LDA (Indirect), Y */
	case 0xb1:
		IND_Y; MEM; LOAD; WACC; CLOCK(5); break;
		/* LDX Immediate */
	case 0xa2:
		IMM; LOAD; WXREG; CLOCK(2); break;
		/* LDX ZeroPage */
	case 0xa6:
		ZP; MEM; LOAD; WXREG; CLOCK(3); break;
		/* LDX ZeroPage, Y */
	case 0xb6:
		ZP_Y; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDX Absolute */
	case 0xae:
		ABS; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDX Absolute, Y */
	case 0xbe:
		ABS_Y; MEM; LOAD; WXREG; CLOCK(4); break;
		/* LDY Immediate */
	case 0xa0:
		IMM; LOAD; WYREG; CLOCK(2); break;
		/* LDY ZeroPage */
	case 0xa4:
		ZP; MEM; LOAD; WYREG; CLOCK(3); break;
		/* LDY ZeroPage, X */
	case 0xb4:
		ZP_X; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LDY Absolute */
	case 0xac:
		ABS; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LDY Absolute, X */
	case 0xbc:
		ABS_X; MEM; LOAD; WYREG; CLOCK(4); break;
		/* LSR Accmulator */
	case 0x4a:
		ACC; LSR; WACC; CLOCK(2); break;
		/* LSR ZeroPage */
	case 0x46:
		ZP; MEM; LSR; WMEM; CLOCK(5); break;
		/* LSR ZeroPage, X */
	case 0x56:
		ZP_X; MEM; LSR; WMEM; CLOCK(6); break;
		/* LSR Absolute */
	case 0x4e:
		ABS; MEM; LSR; WMEM; CLOCK(6); break;
		/* LSR Absolute, X */
	case 0x5e:
		ABS_X; MEM; LSR; WMEM; CLOCK(7); break;
		/* NOP */
	case 0xea:
		CLOCK(2); break;
		/* ORA Immediate */
	case 0x09:
		IMM; ORA; WACC; CLOCK(2); break;
		/* ORA ZeroPage */
	case 0x05:
		ZP; MEM; ORA; WACC; CLOCK(3); break;
		/* ORA ZeroPage, X */
	case 0x15:
		ZP_X; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute */
	case 0x0d:
		ABS; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute, X */
	case 0x1d:
		ABS_X; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA Absolute, Y */
	case 0x19:
		ABS_Y; MEM; ORA; WACC; CLOCK(4); break;
		/* ORA (Indirect, X) */
	case 0x01:
		IND_X; MEM; ORA; WACC; CLOCK(6); break;
		/* ORA (Indirect), Y */
	case 0x11:
		IND_Y; MEM; ORA; WACC; CLOCK(5); break;
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
		ZP; MEM; ROL; WMEM; CLOCK(5); break;
		/* ROL ZeroPage, X */
	case 0x36:
		ZP_X; MEM; ROL; WMEM; CLOCK(6); break;
		/* ROL Absolute */
	case 0x2e:
		ABS; MEM; ROL; WMEM; CLOCK(6); break;
		/* ROL Absolute, X */
	case 0x3e:
		ABS_X; MEM; ROL; WMEM; CLOCK(7); break;
		/* ROR Accumulator */
	case 0x6a:
		ACC; ROR; WACC; CLOCK(2); break;
		/* ROR ZeroPage */
	case 0x66:
		ZP; MEM; ROR; WMEM; CLOCK(5); break;
		/* ROR ZeroPage, X */
	case 0x76:
		ZP_X; MEM; ROR; WMEM; CLOCK(6); break;
		/* ROR Absolute */
	case 0x6e:
		ABS; MEM; ROR; WMEM; CLOCK(6); break;
		/* ROR Absolute, X */
	case 0x7e:
		ABS_X; MEM; ROR; WMEM; CLOCK(7); break;
		/* RTI */
	case 0x40:
		RTI; CHECK_IRQ; CLOCK(6); break;
		/* RTS */
	case 0x60:
		RTS; CLOCK(6); break;
		/* SBC Immediate */
	case 0xe9:
		IMM; SBC; WACC; CLOCK(2); break;
		/* SBC ZeroPage */
	case 0xe5:
		ZP; MEM; SBC; WACC; CLOCK(3); break;
		/* SBC ZeroPage, X */
	case 0xf5:
		ZP_X; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute */
	case 0xed:
		ABS; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute, X */
	case 0xfd:
		ABS_X; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC Absolute, Y */
	case 0xf9:
		ABS_Y; MEM; SBC; WACC; CLOCK(4); break;
		/* SBC (Indirect, X) */
	case 0xe1:
		IND_X; MEM; SBC; WACC; CLOCK(6); break;
		/* SBC (Indirect), Y */
	case 0xf1:
		IND_Y; MEM; SBC; WACC; CLOCK(5); break;
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
		ZP; STA; CLOCK(3); break;
		/* STA ZeroPage, X */
	case 0x95:
		ZP_X; STA; CLOCK(4); break;
		/* STA Absolute */
	case 0x8d:
		ABS; STA; CLOCK(4); break;
		/* STA Absolute, X */
	case 0x9d:
		ABS_X; STA; CLOCK(5); break;
		/* STA Absolute, Y */
	case 0x99:
		ABS_Y; STA; CLOCK(5); break;
		/* STA (Indirect, X) */
	case 0x81:
		IND_X; STA; CLOCK(6); break;
		/* STA (Indirect), Y */
	case 0x91:
		IND_Y; STA; CLOCK(6); break;
		/* STX ZeroPage */
	case 0x86:
		ZP; STX; CLOCK(3); break;
		/* STX ZeroPage, Y */
	case 0x96:
		ZP_Y; STX; CLOCK(4); break;
		/* STX Absolute */
	case 0x8e:
		ABS; STX; CLOCK(4); break;
		/* STY ZeroPage */
	case 0x84:
		ZP; STY; CLOCK(3); break;
		/* STY ZeroPage, X */
	case 0x94:
		ZP_X; STY; CLOCK(4); break;
		/* STY Absolute */
	case 0x8c:
		ABS; STY; CLOCK(4); break;
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
		IND; MEM; ADC; WACC; CLOCK(5); break;
		/* AND (Indirect) */
	case 0x32:
		IND; MEM; AND; WACC; CLOCK(5); break;
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
		ZP; MEM; RMB(0x01); WMEM; CLOCK(5); break;
	case 0x17:
		ZP; MEM; RMB(0x02); WMEM; CLOCK(5); break;
	case 0x27:
		ZP; MEM; RMB(0x04); WMEM; CLOCK(5); break;
	case 0x37:
		ZP; MEM; RMB(0x08); WMEM; CLOCK(5); break;
	case 0x47:
		ZP; MEM; RMB(0x10); WMEM; CLOCK(5); break;
	case 0x57:
		ZP; MEM; RMB(0x20); WMEM; CLOCK(5); break;
	case 0x67:
		ZP; MEM; RMB(0x40); WMEM; CLOCK(5); break;
	case 0x77:
		ZP; MEM; RMB(0x80); WMEM; CLOCK(5); break;
		/* SMB(bit) ZeroPage */
	case 0x87:
		ZP; MEM; SMB(0x01); WMEM; CLOCK(5); break;
	case 0x97:
		ZP; MEM; SMB(0x02); WMEM; CLOCK(5); break;
	case 0xa7:
		ZP; MEM; SMB(0x04); WMEM; CLOCK(5); break;
	case 0xb7:
		ZP; MEM; SMB(0x08); WMEM; CLOCK(5); break;
	case 0xc7:
		ZP; MEM; SMB(0x10); WMEM; CLOCK(5); break;
	case 0xd7:
		ZP; MEM; SMB(0x20); WMEM; CLOCK(5); break;
	case 0xe7:
		ZP; MEM; SMB(0x40); WMEM; CLOCK(5); break;
	case 0xf7:
		ZP; MEM; SMB(0x80); WMEM; CLOCK(5); break;
		/* BIT Immediate */
	case 0x89:
		IMM; BIT; CLOCK(2); break;
		/* BIT ZeroPage, X */
	case 0x34:
		ZP_X; MEM; BIT; CLOCK(2); break;
		/* BIT Absolute, X */
	case 0x3c:
		ABS_X; MEM; BIT; CLOCK(4); break;
		/* BRA rr */
	case 0x80:
		BRA; CLOCK(2); break;
		/* CMP (Indirect) */
	case 0xd2:
		IND; MEM; CMP; CLOCK(5); break;
		/* DEA */
	case 0x3a:
		ACC; DEC; WACC; CLOCK(2); break;
		/* EOR (Indirect) */
	case 0x52:
		IND; MEM; EOR; WACC; CLOCK(5); break;
		/* INA */
	case 0x1a:
		ACC; INC; WACC; CLOCK(2); break;
		/* JMP (Absolute, X) */
	case 0x7c:
		IND16_X; JMP; CLOCK(6); break;
		/* LDA (Indirect) */
	case 0xb2:
		IND; MEM; LOAD; WACC; CLOCK(5); break;
		/* ORA (Indirect) */
	case 0x12:
		IND; MEM; ORA; WACC; CLOCK(5); break;
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
		IND; MEM; SBC; WACC; CLOCK(5); break;
		/* STA (Indirect) */
	case 0x92:
		IND; STA; CLOCK(6); break;
		/* STP */
	case 0xdb:
		STOP; CLOCK(3); break;
		/* STZ ZeroPage */
	case 0x64:
		ZP; STZ; CLOCK(3); break;
		/* STZ ZeroPage, X */
	case 0x74:
		ZP_X; STZ; CLOCK(3); break;
		/* STZ Absolute */
	case 0x9c:
		ABS; STZ; CLOCK(3); break;
		/* STZ Absolute, X */
	case 0x9e:
		ABS_X; STZ; CLOCK(4); break;
		/* TRB ZeroPage */
	case 0x14:
		ZP; MEM; TRB; WMEM; CLOCK(5); break;
		/* TRB Absolute */
	case 0x1c:
		ABS; MEM; TRB; WMEM; CLOCK(6); break;
		/* TSB ZeroPage */
	case 0x04:
		ZP; MEM; TSB; WMEM; CLOCK(5); break;
		/* TSB Absolute */
	case 0x0c:
		ABS; MEM; TSB; WMEM; CLOCK(6); break;
		/* WAI */
	case 0xcb:
		WAIT; CLOCK(3); break;
#else

#endif
	default:
		CLOCK(2); break;
	}


#ifdef _DEBUG
	if ( m_regPC == g_breakpoint )
	{
		g_pBoard->OnDebug();
		clock = 0;		// break loop
	}
#endif
	return clock;
}


void C65c02::init_6502()
{
	this->m_uException_Register = 0;
	m_nClock = 0;
	init_optable();

	for(int i = 0; i < 512; i++ ) {
	   m_BCD_Table1[i] = ((i & 0x0F) <= 0x09) ? i : i + 0x06;
	   m_BCD_Table1[i] += ((m_BCD_Table1[i] & 0xF0) <= 0x90) ? 0 : 0x60;
	   if ( m_BCD_Table1[i] > 0x1FF )
	      m_BCD_Table1[i] -= 0x100;
	   m_BCD_Table2[i] = ((i & 0x0F) <= 0x09) ? i : i - 0x06;
	   m_BCD_Table2[i] -= ((m_BCD_Table2[i] & 0xF0) <= 0x90) ? 0 : 0x60;
	}

	// init registers
	CCpu::Reset();

	m_regA = 0;
	m_regX = 0;
	m_regY = 0;
	m_regF = 0x24;
	m_regS = 0xFF;
	m_regPC = 0;
	m_uException_Register = 0;
	PendingIRQ = 0;
}

void C65c02::init_optable()
{
	m_initialized=TRUE;
}

WORD C65c02::getRegPC(void)
{
	return m_regPC;
}

void C65c02::Reset()
{
	CCpu::Reset();
	m_regA = 0;
	m_regX = 0;
	m_regY = 0;
	m_regF = 0x24;
	m_regS = 0xFF;
	m_regPC = READMEM16((WORD)0xFFFC);
	m_nClock = 0;
	READMEM8(0xC081);
	m_uException_Register = 0;
}

BYTE C65c02::GetRegF()
{
	return( m_regF );
}

void C65c02::Serialize(CArchive &ar)
{
	CCpu::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_regX;
		ar << m_regY;
		ar << m_regA;
		ar << m_regF;
		ar << m_regS;
		ar << m_regPC;
		ar << m_nClock;
	}
	else
	{
		ar >> m_regX;
		ar >> m_regY;
		ar >> m_regA;
		ar >> m_regF;
		ar >> m_regS;
		ar >> m_regPC;
		ar >> m_nClock;
	}
}
