// 65c02.cpp: implementation of the C65c02 class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "appleclock.h"
#include "memory.h"
#include "65c02.h"

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


#define WRITEMEM(addr, data)	( g_pBoard->m_cIOU.WriteMem8( addr, data ) )
#define READMEM(addr)			( g_pBoard->m_cIOU.ReadMem8( addr ) )
#define READMEMW(addr)			( g_pBoard->m_cIOU.ReadMem16( addr ) )
#define READOPCODE()			( READMEM( m_regPC++ ) )
#define READOPCODEW()			( READOPCODE() | ( READOPCODE() << 8 ) )
#define	PUSH(data)				( WRITEMEM( ( m_regS-- ) | 0x100, data ) )
#define POP()					( READMEM( ( ++m_regS ) | 0x100 ) )

#ifdef _DEBUG
int g_breakpoint = -1;
#endif

WORD C65c02::getAbs()		// Absolute
{
	return ( READOPCODEW() );
}


WORD C65c02::getAbsX()		// Absolute, X
{
	WORD addr = READOPCODEW();
	checkCross(addr, m_regX);
	return ( addr+m_regX );
}

WORD C65c02::getAbsY()		// Absolute, Y
{
	WORD addr = READOPCODEW();
	checkCross(addr, m_regY);
	return ( addr+m_regY );
}

// for INC
WORD C65c02::getAbsXINC()		// Absolute, X
{
	WORD addr = READOPCODEW();
	return ( addr+m_regX );
}

WORD C65c02::getZp()			// Zero Page
{
	return ( (WORD)READOPCODE() );
}

WORD C65c02::getZpX()		// Zero Page, X
{
	return ( ( READOPCODE()+m_regX )&0xFF );
}

WORD C65c02::getZpY()		// Zero Page, Y
{
	return ( ( READOPCODE()+m_regY )&0xFF );
}

WORD C65c02::getIndX()		// (Indirect, X)
{
	BYTE addr = READOPCODE()+m_regX;
	return ( READMEMW( (WORD)addr ) );
}

WORD C65c02::getIndX16()		// (Indirect16, X)
{
	WORD addr = READOPCODEW();
	checkCross( addr, m_regX );
	return ( READMEMW( addr + m_regX ) );
}

WORD C65c02::getIndY()		// (Indirect), Y
{
	WORD addr = READMEMW( (WORD)READOPCODE() );
	checkCross( addr, m_regY );
	return ( addr + m_regY );
}

WORD C65c02::getInd16()		// (Indirect16)
{
	WORD addr = READOPCODEW();
	return ( READMEMW( addr ) );
}

WORD C65c02::getInd()		// (Indirect)
{
	BYTE addr = READOPCODE();
	return ( READMEMW( (WORD)addr ) );
}

	// Set Flags
void C65c02::updateFlagNZ(WORD result)
{
	m_regF &= ~NZ_Flag;
	
	if(result&0x280)
		m_regF |= N_Flag;
	
	if(!(result&0xFF))
		m_regF |= Z_Flag;
}

	// Set Flags
void C65c02::updateFlagNZC(WORD result)
{
	m_regF &= ~NZC_Flag;
	
	if(result&0x280)
		m_regF |= N_Flag;
	
	if(!(result&0xFF))
		m_regF |= Z_Flag;
	
	if(result&0x100)
		m_regF |= C_Flag;
}

void C65c02::updateFlag(BYTE result, BYTE flag)
{
	if(result)
		m_regF |= flag;
	else
		m_regF &= ~flag;
}

void C65c02::branch(BYTE offset)
{
	WORD off;
	TRACE_ADDR
	// extends BYTE to WORD with sign
	if ( offset &0x80 )
		off = offset - 0x100;
	else
		off = offset;
	
	checkCross(m_regPC, off);
	
	m_regPC += off;
	m_nClock++;
}

void C65c02::checkCross(WORD addr, WORD offset)
{
	if ( ( ( addr + offset ) ^ addr ) & 0xFF00 )
		m_nClock++;
}

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
	m_nClock = 0;
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
			m_regPC = READMEMW( 0xFFFE );
			m_nClock += 7;
			m_regF |= I_Flag;
			return m_nClock;
		}
		else
		{
//			PendingIRQ++;
		}
	}

	if ( ( m_uException_Register & SIG_CPU_SHUTDOWN )
		|| ( m_uException_Register & SIG_CPU_WAIT ) )
	{
		m_nClock += 3;
		return m_nClock;
	}
	BYTE opcode = READOPCODE();
	WORD addr;
	BYTE data;
	WORD result;
	switch(opcode){
//--------------------------------
// 65c02 operators
//--------------------------------
		// ADC Immediate
	case 0x69:
		data = READOPCODE();
		result = (WORD)data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 2;
		break;

		// ADC Zero page
	case 0x65:
		data = READMEM( getZp() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 3;
		break;

		// ADC Zero page, X
	case 0x75:
		data = READMEM( getZpX() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 4;
		break;

		// ADC Absolute
	case 0x6D:
		data = READMEM( getAbs() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 4;
		break;

		// ADC Absolute, X
	case 0x7D:
		data = READMEM( getAbsX() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 4;
		break;

		// ADC Absolute, Y
	case 0x79:
		data = READMEM( getAbsY() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 4;
		break;

		// ADC (Indirect, X)
	case 0x61:
		data = READMEM( getIndX() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 6;
		break;

		// ADC (Indirect), Y
	case 0x71:
		data = READMEM( getIndY() );
		result = data + m_regA + (m_regF&C_Flag);
		updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow

		if(m_regF&D_Flag)		// BCD Adjust
			result = m_BCD_Table1[result];
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 5;
		break;

		// AND Immediate
	case 0x29:
		m_regA &= READOPCODE();
		updateFlagNZ(m_regA);
		m_nClock += 2;
		break;

		// AND Zero Page
	case 0x25:
		m_regA &= READMEM( getZp() );
		updateFlagNZ(m_regA);
		m_nClock += 3;
		break;

		// AND Zero Page, X
	case 0x35:
		m_regA &= READMEM( getZpX() );
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// AND Absolute
	case 0x2D:
		m_regA &= READMEM( getAbs() );
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// AND Absolute, X
	case 0x3D:
		m_regA &= READMEM( getAbsX() );
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// AND Absolute, Y
	case 0x39:
		m_regA &= READMEM( getAbsY() );
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// AND (Indirect, X)
	case 0x21:
		m_regA &= READMEM( getIndX() );
		updateFlagNZ(m_regA);
		m_nClock += 6;
		break;

		// AND (Indirect), Y
	case 0x31:
		m_regA &= READMEM( getIndY() );
		updateFlagNZ(m_regA);
		m_nClock += 5;
		break;

		// ASL Accumulator
	case 0x0A:
		result = m_regA << 1;
		updateFlagNZC(result);
		m_regA = result&0xFF;
		m_nClock += 2;
		break;

		// ASL Zero page
	case 0x06:
		addr = getZp();
		result = READMEM( addr );
		result <<= 1;
		updateFlagNZC(result);
		WRITEMEM(addr, result&0xFF);
		m_nClock += 5;
		break;

		// ASL Zero page, X
	case 0x16:
		addr = getZpX();
		result = READMEM( addr );
		result <<= 1;
		updateFlagNZC(result);
		WRITEMEM(addr, result&0xFF);
		m_nClock += 6;
		break;

		// ASL Absolute
	case 0x0E:
		addr = getAbs();
		result = READMEM( addr );
		result <<= 1;
		updateFlagNZC(result);
		WRITEMEM(addr, result&0xFF);
		m_nClock += 6;
		break;

		// ASL Absolute, X
	case 0x1E:
		addr = getAbsX();
		result = READMEM( addr );
		result <<= 1;
		updateFlagNZC(result);
		WRITEMEM(addr, result&0xFF);
		m_nClock += 7;
		break;

		// BCC rr
	case 0x90:
		data = READOPCODE();
		if(!(m_regF&C_Flag))
			branch(data);
		m_nClock += 2;
		break;

		// BCS rr
	case 0xB0:
		data = READOPCODE();
		if(m_regF&C_Flag)
			branch(data);
		m_nClock += 2;
		break;

		// BEQ rr
	case 0xF0:
		data = READOPCODE();
		if(m_regF&Z_Flag)
			branch(data);
		m_nClock += 2;
		break;

		// BIT Zero page
	case 0x24:
		data = READMEM( getZp() );
		updateFlag(data&0x40, V_Flag);
		updateFlagNZ(((data&0x80)<<2)|(data&m_regA));
		m_nClock += 3;
		break;

		// BIT Absolute
	case 0x2C:
		data = READMEM( getAbs() );
		updateFlag(data&0x40, V_Flag);
		updateFlagNZ(((data&0x80)<<2)|(data&m_regA));
		m_nClock += 3;
		break;

		// BMI rr
	case 0x30:
		data = READOPCODE();
		if(m_regF&N_Flag)
			branch(data);
		m_nClock += 2;
		break;

		// BNE rr
	case 0xD0:
		data = READOPCODE();
		if(!(m_regF&Z_Flag))
			branch(data);
		m_nClock += 2;
		break;

		// BPL rr
	case 0x10:
		data = READOPCODE();
		if(!(m_regF&N_Flag))
			branch(data);
		m_nClock += 2;
		break;

		// BRK
	case 0x00:
		TRACE_ADDR
#ifdef _DEBUG
		TRACE( "BRK at $%04X\n", m_regPC - 1 );
		{
			int d_i, d_current;
			d_current = m_current;
			TRACE( "Trace PC register\n" );
			for( d_i = 0; d_i < 10; d_i++ )
			{
				TRACE( "    jmp at $%04X\n", m_trace[(--d_current)&0xFF] );
			}
			TRACE( "Stack : $%02X\n    ", m_regS );
			for( d_i = 0; d_i < 10; d_i++ )
			{
				TRACE( "%02X ", READMEM( ( (m_regS+d_i)&0xFF ) +0x100) );
			}
			TRACE( "\n" );
		}
#endif
		m_regPC++;
		PUSH((m_regPC>>8)&0xFF);
		PUSH(m_regPC&0xFF);
		m_regF |= B_Flag | X_Flag;
		PUSH(m_regF);
		m_regF |= I_Flag;

		m_regPC = READMEMW( (WORD)0xFFFE );
		m_nClock += 7;
		break;

		// BVC rr
	case 0x50:
		data = READOPCODE();
		if(!(m_regF&V_Flag))
			branch(data);
		m_nClock += 2;
		break;

		// BVS rr
	case 0x70:
		data = READOPCODE();
		if(m_regF&V_Flag)
			branch(data);
		m_nClock += 2;
		break;

		// CLC
	case 0x18:
		m_regF &= ~C_Flag;
		m_nClock += 2;
		break;

		// CLD
	case 0xD8:
		m_regF &= ~D_Flag;
		m_nClock += 2;
		break;

		// CLI
	case 0x58:
		m_regF &= ~I_Flag;
		m_nClock += 2;
		if(PendingIRQ){
			PendingIRQ--;
			Assert_IRQ();
		}
		break;
		
		// CLV
	case 0xB8:
		m_regF &= ~V_Flag;
		m_nClock += 2;
		break;
		
		// CMP Immediate
	case 0xC9:
		updateFlagNZC(0x100 + m_regA - READOPCODE());
		m_nClock += 2;
		break;

		// CMP Zero page
	case 0xC5:
		updateFlagNZC( 0x100 + m_regA - READMEM( getZp() ) );
		m_nClock += 3;
		break;

		// CMP Zero page, X
	case 0xD5:
		updateFlagNZC( 0x100 + m_regA - READMEM( getZpX() ) );
		m_nClock += 4;
		break;

		// CMP Absolute
	case 0xCD:
		updateFlagNZC( 0x100 + m_regA - READMEM( getAbs() ) );
		m_nClock += 4;
		break;

		// CMP Absolute, X
	case 0xDD:
		updateFlagNZC( 0x100 + m_regA - READMEM( getAbsX() ) );
		m_nClock += 4;
		break;
		
		// CMP Absolute, Y
	case 0xD9:
		updateFlagNZC( 0x100 + m_regA - READMEM( getAbsY() ) );
		m_nClock += 4;
		break;
		
		// CMP (Indirect, X)
	case 0xC1:
		updateFlagNZC( 0x100 + m_regA - READMEM( getIndX() ) );
		m_nClock += 6;
		break;

		// CMP (Indirect), Y
	case 0xD1:
		updateFlagNZC( 0x100 + m_regA - READMEM( getIndY() ) );
		m_nClock += 5;
		break;

		// CPX Immediate
	case 0xE0:
		updateFlagNZC( 0x100 + m_regX - READOPCODE());
		m_nClock += 2;
		break;

		// CPX Zero page
	case 0xE4:
		updateFlagNZC( 0x100 + m_regX - READMEM( getZp() ) );
		m_nClock += 3;
		break;

		// CPX Absolute
	case 0xEC:
		updateFlagNZC( 0x100 + m_regX - READMEM( getAbs() ) );
		m_nClock += 4;
		break;

		// CPY Immediate
	case 0xC0:
		updateFlagNZC( 0x100 + m_regY - READOPCODE());
		m_nClock += 2;
		break;

		// CPY Zero page
	case 0xC4:
		updateFlagNZC( 0x100 + m_regY - READMEM( getZp() ) );
		m_nClock += 3;
		break;

		// CPY Absolute
	case 0xCC:
		updateFlagNZC( 0x100 + m_regY - READMEM(getAbs() ) );
		m_nClock += 4;
		break;

		// DEC Zero page
	case 0xC6:
		addr = getZp();
		result = READMEM( addr ) + 0xFF;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 5;
		break;

		// DEC Zero page, X
	case 0xD6:
		addr = getZpX();
		result = READMEM( addr )+0xFF;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// DEC Absolute
	case 0xCE:
		addr = getAbs();
		result = READMEM( addr )+0xFF;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// DEC Absolute, X
	case 0xDE:
		addr = getAbsX();
		result = READMEM(addr)+0xFF;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 7;
		break;

		// DEX
	case 0xCA:
		result = m_regX + 0xFF;
		updateFlagNZ(result);
		m_regX = (BYTE)result;
		m_nClock += 2;
		break;

		// DEY
	case 0x88:
		result = m_regY + 0xFF;
		updateFlagNZ(result);
		m_regY = (BYTE)result;
		m_nClock += 2;
		break;

		// EOR Immediate
	case 0x49:
		m_regA ^= READOPCODE();
		updateFlagNZ(m_regA);
		m_nClock += 2;
		break;

		// EOR Zero page
	case 0x45:
		m_regA ^= READMEM(getZp());
		updateFlagNZ(m_regA);
		m_nClock += 3;
		break;

		// EOR Zero page, X
	case 0x55:
		m_regA ^= READMEM(getZpX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// EOR Absolute
	case 0x4D:
		m_regA ^= READMEM(getAbs());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// EOR Absolute, X
	case 0x5D:
		m_regA ^= READMEM(getAbsX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// EOR Absolute, Y
	case 0x59:
		m_regA ^= READMEM(getAbsY());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// EOR (Indirect, X)
	case 0x41:
		m_regA ^= READMEM(getIndX());
		updateFlagNZ(m_regA);
		m_nClock += 6;
		break;

		// EOR (Indirect), Y
	case 0x51:
		m_regA ^= READMEM(getIndY());
		updateFlagNZ(m_regA);
		m_nClock += 5;
		break;

		// INC Zero page
	case 0xE6:
		addr = getZp();
		result = READMEM(addr) + 1;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 5;
		break;

		// INC Zero page, X
	case 0xF6:
		addr = getZpX();
		result = READMEM(addr) + 1;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// INC Absolute
	case 0xEE:
		addr = getAbs();
		result = READMEM(addr) + 1;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// INC Absolute, X
	case 0xFE:
		addr = getAbsXINC();
		result = READMEM(addr) + 1;
		updateFlagNZ(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 7;
		break;

		// INX
	case 0xE8:
		result = m_regX+1;
		updateFlagNZ(result);
		m_regX = (BYTE)result;
		m_nClock += 2;
		break;

		// INY
	case 0xC8:
		result = m_regY+1;
		updateFlagNZ(result);
		m_regY = (BYTE)result;
		m_nClock += 2;
		break;

		// JMP Absolute
	case 0x4C:
		TRACE_ADDR
		m_regPC = getAbs();
		m_nClock += 3;
		break;

		// JMP (Indirect16)
	case 0x6C:
		TRACE_ADDR
		m_regPC = getInd16();
		// 65c02 : 6 cycle
		// 6502 : 5 cycle
		m_nClock += 6;
		break;

		// JSR Absolute
	case 0x20:
		TRACE_ADDR
		addr = getAbs();
		m_regPC--;
		PUSH((BYTE)((m_regPC)>>8));
		PUSH((BYTE)(m_regPC));
		m_regPC = addr;
		m_nClock += 6;
		break;

		// LDA Immediate
	case 0xA9:
		m_regA = READOPCODE();
		updateFlagNZ(m_regA);
		m_nClock += 2;
		break;


		// LDA Zero page
	case 0xA5:
		m_regA = READMEM(getZp());
		updateFlagNZ(m_regA);
		m_nClock += 3;
		break;

		// LDA Zero page, X
	case 0xB5:
		m_regA = READMEM(getZpX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// LDA Absolute
	case 0xAD:
		m_regA = READMEM(getAbs());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// LDA Absolute, X
	case 0xBD:
		m_regA = READMEM(getAbsX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// LDA Absolute, Y
	case 0xB9:
		m_regA = READMEM(getAbsY());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// LDA (Indirect, X)
	case 0xA1:
		m_regA = READMEM(getIndX());
		updateFlagNZ(m_regA);
		m_nClock += 6;
		break;

		// LDA (Indirect), Y
	case 0xB1:
		m_regA = READMEM(getIndY());
		updateFlagNZ(m_regA);
		m_nClock += 5;
		break;

		// LDX Immediate
	case 0xA2:
		m_regX = READOPCODE();
		updateFlagNZ(m_regX);
		m_nClock += 2;
		break;

		// LDX Zero page
	case 0xA6:
		m_regX = READMEM(getZp());
		updateFlagNZ(m_regX);
		m_nClock += 3;
		break;

		// LDX Zero page, Y
	case 0xB6:
		m_regX = READMEM(getZpY());
		updateFlagNZ(m_regX);
		m_nClock += 4;
		break;

		// LDX Absolute
	case 0xAE:
		m_regX = READMEM(getAbs());
		updateFlagNZ(m_regX);
		m_nClock += 4;
		break;

		// LDX Absolute, Y
	case 0xBE:
		m_regX = READMEM(getAbsY());
		updateFlagNZ(m_regX);
		m_nClock += 4;
		break;

		// LDY Immediate
	case 0xA0:
		m_regY = READOPCODE();
		updateFlagNZ(m_regY);
		m_nClock += 2;
		break;

		// LDY Zero page
	case 0xA4:
		m_regY = READMEM(getZp());
		updateFlagNZ(m_regY);
		m_nClock += 3;
		break;

		// LDY Zero page, X
	case 0xB4:
		m_regY = READMEM(getZpX());
		updateFlagNZ(m_regY);
		m_nClock += 4;
		break;

		// LDY Absolute
	case 0xAC:
		m_regY = READMEM(getAbs());
		updateFlagNZ(m_regY);
		m_nClock += 4;
		break;

		// LDY Absolute, X
	case 0xBC:
		m_regY = READMEM(getAbsX());
		updateFlagNZ(m_regY);
		m_nClock += 4;
		break;

		// LSR Accmulator
	case 0x4A:
		result = ((m_regA&0x01)<<8) | (m_regA>>1);
		updateFlagNZC(result);
		m_regA = (BYTE)result;
		m_nClock += 2;
		break;

		// LSR Zero page
	case 0x46:
		addr = getZp();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | (result>>1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 5;
		break;

		// LSR Zero page, X
	case 0x56:
		addr = getZpX();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | (result>>1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// LSR Absolute
	case 0x4E:
		addr = getAbs();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | (result>>1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// LSR Absolute, X
	case 0x5E:
		addr = getAbsX();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | (result>>1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 7;
		break;

		// NOP
	case 0xEA:
		m_nClock += 2;
		break;

		// ORA Immediate
	case 0x09:
		m_regA |= READOPCODE();
		updateFlagNZ(m_regA);
		m_nClock += 2;
		break;

		// ORA Zero page
	case 0x05:
		data = READMEM(getZp());

		m_regA |= data;
		updateFlagNZ(m_regA);
		m_nClock += 3;
		break;

		// ORA Zero page, X
	case 0x15:
		m_regA |= READMEM(getZpX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// ORA Absolute
	case 0x0D:
		m_regA |= READMEM(getAbs());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// ORA Absolute, X

	case 0x1D:
		m_regA |= READMEM(getAbsX());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// ORA Absolute, Y
	case 0x19:
		m_regA |= READMEM(getAbsY());
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// ORA (Indirect, X)
	case 0x01:
		m_regA |= READMEM(getIndX());
		updateFlagNZ(m_regA);
		m_nClock += 6;
		break;

		// ORA (Indirect),Y
	case 0x11:
		m_regA |= READMEM(getIndY());
		updateFlagNZ(m_regA);
		m_nClock += 5;
		break;

		// PHA
	case 0x48:
		PUSH(m_regA);
		m_nClock += 3;
		break;

		// PHP
	case 0x08:
		PUSH(m_regF);
		m_nClock += 3;
		break;

		// PLA
	case 0x68:
		m_regA = POP();
		updateFlagNZ(m_regA);
		m_nClock += 4;
		break;

		// PLP
	case 0x28:
		m_regF = POP();
		m_nClock += 4;
		if( PendingIRQ && !(m_regF&I_Flag)){
			PendingIRQ--;
			Assert_IRQ();
		}
		break;

		// ROL Accumulator
	case 0x2A:
		result = (m_regA << 1) | (m_regF&C_Flag);
		updateFlagNZC(result);
		m_regA = (BYTE)result;
		m_nClock += 2;
		break;

		// ROL Zero page
	case 0x26:
		addr = getZp();
		result = READMEM(addr);
		result = (result << 1) | (m_regF&C_Flag);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 5;
		break;

		// ROL Zero page, X
	case 0x36:
		addr = getZpX();
		result = READMEM(addr);
		result = (result << 1) | (m_regF&C_Flag);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// ROL Absolute
	case 0x2E:
		addr = getAbs();
		result = READMEM(addr);
		result = (result << 1) | (m_regF&C_Flag);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// ROL Absolute, X
	case 0x3E:
		addr = getAbsX();
		result = READMEM(addr);
		result = (result << 1) | (m_regF&C_Flag);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 7;
		break;

		// ROR Accumulator
	case 0x6A:
		result = ((m_regA&0x01)<<8) | ((m_regF&C_Flag)<<7) | (m_regA >> 1);
		updateFlagNZC(result);
		m_regA = (BYTE)result;
		m_nClock += 2;
		break;

		// ROR Zero page
	case 0x66:
		addr = getZp();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | ((m_regF&C_Flag)<<7) | (result >> 1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 5;
		break;

		// ROR Zero page, X
	case 0x76:
		addr = getZpX();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | ((m_regF&C_Flag)<<7) | (result >> 1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// ROR Absolute
	case 0x6E:
		addr = getAbs();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | ((m_regF&C_Flag)<<7) | (result >> 1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 6;
		break;

		// ROR Absolute, X
	case 0x7E:
		addr = getAbsX();
		result = READMEM(addr);
		result = ((result&0x01)<<8) | ((m_regF&C_Flag)<<7) | (result >> 1);
		updateFlagNZC(result);
		WRITEMEM(addr, (BYTE)result);
		m_nClock += 7;
		break;

		// RTI
	case 0x40:
		TRACE_ADDR
		m_regF = POP() | 0x20; /* bit 5 bug of 6502 */
		m_regPC = POP();
		m_regPC += POP()<<8;
		m_nClock += 6;
		if( PendingIRQ && !(m_regF&I_Flag)){
			PendingIRQ--;
			Assert_IRQ();
		}
		break;

		// RTS
	case 0x60:
		TRACE_ADDR
		m_regPC = POP();
		m_regPC += POP()<<8;
		m_regPC++;
		m_nClock += 6;
		break;

		// SBC Immediate
	case 0xE9:
		data = 0xFF - READOPCODE();
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 2;
		break;

		// SBC Zero page
	case 0xE5:
		data = 0xFF - READMEM(getZp());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 3;
		break;

		// SBC Zero page, X
	case 0xF5:
		data = 0xFF - READMEM(getZpX());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 4;
		break;

		// SBC Absolute
	case 0xED:
		data = 0xFF - READMEM(getAbs());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 4;
		break;

		// SBC Absolute, X
	case 0xFD:
		data = 0xFF - READMEM(getAbsX());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 4;
		break;

		// SBC Absolute, Y
	case 0xF9:
		data = 0xFF - READMEM(getAbsY());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 4;
		break;

		// SBC (Indirect, X)
	case 0xE1:
		data = 0xFF - READMEM(getIndX());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 6;
		break;

		// SBC (Indirect), Y
	case 0xF1:
		data = 0xFF - READMEM(getIndY());
		result = data + m_regA + (m_regF & C_Flag);
		updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
		// bcd adjust
		if(m_regF&D_Flag)
			result = m_BCD_Table2[result];
		updateFlagNZC(result);
		m_regA = result & 0xFF;
		m_nClock += 5;
		break;

		// SEC
	case 0x38:
		m_regF |= C_Flag;
		m_nClock += 2;
		break;

		// SED
	case 0xF8:
		m_regF |= D_Flag;
		m_nClock += 2;
		break;

		// SEI
	case 0x78:
		m_regF |= I_Flag;
		m_nClock += 2;
		break;

		// STA Zero page
	case 0x85:
		WRITEMEM(getZp(), m_regA);
		m_nClock += 3;
		break;

		// STA Zero page, X
	case 0x95:
		WRITEMEM(getZpX(), m_regA);
		m_nClock += 4;
		break;

		// STA Absolute
	case 0x8D:
		WRITEMEM(getAbs(), m_regA);
		m_nClock += 4;
		break;

		// STA Absolute, X
	case 0x9D:
		WRITEMEM(getAbsX(), m_regA);
		m_nClock += 5;
		break;

		// STA Absolute, Y
	case 0x99:
		WRITEMEM(getAbsY(), m_regA);
		m_nClock += 5;
		break;

		// STA (Indirect, X)
	case 0x81:
		WRITEMEM(getIndX(), m_regA);
		m_nClock += 6;
		break;

		// STA (Indirect), Y
	case 0x91:
		WRITEMEM(getIndY(), m_regA);
		m_nClock += 6;
		break;

		// STX Zero page
	case 0x86:
		WRITEMEM(getZp(), m_regX);
		m_nClock += 3;
		break;

		// STX Zero page, Y
	case 0x96:
		WRITEMEM(getZpY(), m_regX);
		m_nClock += 4;
		break;

		// STX Absolute
	case 0x8E:
		WRITEMEM(getAbs(), m_regX);
		m_nClock += 4;
		break;

		// STY Zero page
	case 0x84:
		WRITEMEM(getZp(), m_regY);
		m_nClock += 3;
		break;

		// STY Zero page, X
	case 0x94:
		WRITEMEM(getZpX(), m_regY);
		m_nClock += 4;
		break;

		// STY Absolute
	case 0x8C:
		WRITEMEM(getAbs(), m_regY);
		m_nClock += 4;
		break;

		// TAX
	case 0xAA:
		m_regX = m_regA;
		updateFlagNZ(m_regX);
		m_nClock += 2;
		break;

		// TAY
	case 0xA8:
		m_regY = m_regA;
		updateFlagNZ(m_regY);
		m_nClock += 2;
		break;

		// TSX
	case 0xBA:
		m_regX = m_regS;
		updateFlagNZ(m_regX);
		m_nClock += 2;
		break;

		// TXA
	case 0x8A:
		m_regA = m_regX;
		updateFlagNZ(m_regX);
		m_nClock += 2;
		break;

		// TXS
	case 0x9A:
		m_regS = m_regX;
		updateFlagNZ(m_regS);
		m_nClock += 2;
		break;

		// TYA
	case 0x98:
		m_regA = m_regY;
		updateFlagNZ(m_regA);
		m_nClock += 2;
		break;



	default:
		switch( opcode )
		{
			//--------------------------------
			// 65c02 operators
			//--------------------------------
			// ADC (Indirect)
		case 0x72:
			data = READMEM(getInd());
			result = data + m_regA + (m_regF&C_Flag);
			updateFlag(!((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);	// over flow
			
			if(m_regF&D_Flag)		// BCD Adjust
				result = m_BCD_Table1[result];
			updateFlagNZC(result);
			m_regA = result&0xFF;
			m_nClock += 5;
			break;
			
			// AND (Indirect)
		case 0x32:
			m_regA &= READMEM(getInd());
			updateFlagNZ(m_regA);
			m_nClock += 5;
			break;
			
			// BBR Zero rr
#define BBR(bit)	\
	data = READMEM( getZp() );	\
	addr = READOPCODE();		\
	if ( !( data & bit ) )		\
		branch( (BYTE)addr );	\
	m_nClock += 5;

#define BBS(bit)	\
	data = READMEM( getZp() );	\
	addr = READOPCODE();		\
	if ( ( data & bit ) )		\
		branch( (BYTE)addr );	\
	m_nClock += 5;

		case 0x0F:	BBR(0x01);	break;
		case 0x1F:	BBR(0x02);	break;
		case 0x2F:	BBR(0x04);	break;
		case 0x3F:	BBR(0x08);	break;
		case 0x4F:	BBR(0x10);	break;
		case 0x5F:	BBR(0x20);	break;
		case 0x6F:	BBR(0x40);	break;
		case 0x7F:	BBR(0x80);	break;
		case 0x8F:	BBS(0x01);	break;
		case 0x9F:	BBS(0x02);	break;
		case 0xAF:	BBS(0x04);	break;
		case 0xBF:	BBS(0x08);	break;
		case 0xCF:	BBS(0x10);	break;
		case 0xDF:	BBS(0x20);	break;
		case 0xEF:	BBS(0x40);	break;
		case 0xFF:	BBS(0x80);	break;
		
		// RMB zp
#define RMB(bit)		\
	addr = getZp();		\
	data = READMEM( addr );		\
	data &= ~bit;				\
	WRITEMEM( addr, data );		\
	m_nClock += 5;

#define SMB(bit)		\
	addr = getZp();		\
	data = READMEM( addr );		\
	data |= bit;				\
	WRITEMEM( addr, data );		\
	m_nClock += 5;

		case 0x07:	RMB(0x01);	break;
		case 0x17:	RMB(0x02);	break;
		case 0x27:	RMB(0x04);	break;
		case 0x37:	RMB(0x08);	break;
		case 0x47:	RMB(0x10);	break;
		case 0x57:	RMB(0x20);	break;
		case 0x67:	RMB(0x40);	break;
		case 0x77:	RMB(0x80);	break;
		case 0x87:	SMB(0x01);	break;
		case 0x97:	SMB(0x02);	break;
		case 0xA7:	SMB(0x04);	break;
		case 0xB7:	SMB(0x08);	break;
		case 0xC7:	SMB(0x10);	break;
		case 0xD7:	SMB(0x20);	break;
		case 0xE7:	SMB(0x40);	break;
		case 0xF7:	SMB(0x80);	break;

			// BIT Immediate
		case 0x89:
			data = READOPCODE();
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ(((data&0x80)<<2)|(data&m_regA));
			m_nClock += 2;
			break;
			
			// BIT Zero page, X
		case 0x34:
			data = READMEM(getZpX());
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ(((data&0x80)<<2)|(data&m_regA));
			m_nClock += 2;
			break;
			
			// BIT Absolute, X
		case 0x3C:
			data = READMEM(getAbsX());
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ(((data&0x80)<<2)|(data&m_regA));
			m_nClock += 4;
			break;
			
			// BRA	rr
		case 0x80:
			data = READOPCODE();
			m_nClock += 2;
			branch(data);
			break;
			
			// CMP (Indirect8)
		case 0xD2:
			updateFlagNZC(0x100 + m_regA - READMEM(getInd()));
			m_nClock += 5;
			break;
			
			// DEA Accumulator
		case 0x3A:
			result = m_regA+0xFF;
			updateFlagNZ(result);
			m_regA = (BYTE)result;
			m_nClock += 2;
			break;
			
			// EOR (Indirect8)
		case 0x52:
			m_regA ^= READMEM(getInd());
			updateFlagNZ(m_regA);
			m_nClock += 5;
			break;
			
			// INA Accumulator
		case 0x1A:
			result = m_regA + 1;
			updateFlagNZ(result);
			m_regA = (BYTE)result;
			m_nClock += 2;
			break;
			
			// JMP (Absolute, X)
		case 0x7C:
			TRACE_ADDR
			m_regPC = this->getIndX16();
			m_nClock += 6;
			break;
			
			// LDA (Indirect8)
		case 0xB2:
			m_regA = READMEM(getInd());
			updateFlagNZ(m_regA);
			m_nClock += 5;
			break;
			
			// ORA (Indirect8)
		case 0x12:
			m_regA |= READMEM(getInd());
			updateFlagNZ(m_regA);
			m_nClock += 5;
			break;
			
			// PHX
		case 0xDA:
			PUSH(m_regX);
			m_nClock += 3;
			break;
			
			// PLX
		case 0xFA:
			m_regX = POP();
			updateFlagNZ(m_regX);
			m_nClock += 4;
			break;
			
			// PHY
		case 0x5A:
			PUSH(m_regY);
			m_nClock += 3;
			break;
			
			// PLY
		case 0x7A:
			m_regY = POP();
			updateFlagNZ(m_regY);
			m_nClock += 4;
			break;
			
			// SBC (Indirect8)
		case 0xF2:
			data = 0xFF - READMEM(getInd());
			result = data + m_regA + (m_regF & C_Flag);
			updateFlag( !((data^m_regA)&0x80) && ((result^m_regA)&0x80), V_Flag);
			// bcd adjust
			if(m_regF&D_Flag)
				result = m_BCD_Table2[result];
			updateFlagNZC(result);
			m_regA = result & 0xFF;
			m_nClock += 5;
			break;
			
			// STA (Indirect8)
		case 0x92:
			WRITEMEM(getInd(), m_regA);
			m_nClock += 6;
			break;
			
			// STP
		case 0xDB:
			Shutdown();
			m_nClock += 3;
			break;

			// STZ Zero page
		case 0x64:
			WRITEMEM(getZp(), 0);
			m_nClock += 3;
			break;
			
			// STZ Zero page, X
		case 0x74:
			WRITEMEM(getZpX(), 0);
			m_nClock += 3;
			break;
			
			// STZ Absolute
		case 0x9C:
			WRITEMEM(getAbs(), 0);
			m_nClock += 4;
			break;
			
			// STZ Absolute, X
		case 0x9E:
			WRITEMEM(getAbsX(), 0);
			m_nClock += 4;
			break;
			
			// TRB Zero page
		case 0x14:
			addr = getZp();
			data = READMEM(addr);
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ( ((data&0x80)<<2) | ( data&m_regA) );
			WRITEMEM(addr, data & ~m_regA);
			m_nClock += 5;
			break;
			
			// TRB Absolute
		case 0x1C:
			addr = getAbs();
			data = READMEM(addr);
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ( ((data&0x80)<<2) | ( data&m_regA) );
			WRITEMEM(addr, data & ~m_regA);
			m_nClock += 6;
			break;
			
			// TSB Zero page
		case 0x04:
			addr = getZp();
			data = READMEM(addr);
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ( ((data&0x80)<<2) | ( data&m_regA) );
			WRITEMEM(addr, data | m_regA);
			break;
			
			// TSB Absolute
		case 0x0C:
			addr = getAbs();
			data = READMEM(addr);
			updateFlag(data&0x40, V_Flag);
			updateFlagNZ( ((data&0x80)<<2) | ( data&m_regA) );
			WRITEMEM(addr, data|m_regA);
			m_nClock += 6;
			break;

			// WAI
		case 0xCB:
			Wait();
			m_nClock += 3;
			break;

		default:
			m_nClock += 2;
			break;
		}
	}
#ifdef _DEBUG
	if ( m_regPC == g_breakpoint )
	{
		g_pBoard->OnDebug();
		m_nClock = 0;		// break loop
	}
#endif
	return m_nClock;
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
	m_regPC = READMEMW((WORD)0xFFFC);
	m_nClock = 0;
	READMEM(0xC081);
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
