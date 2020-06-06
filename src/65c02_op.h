#ifndef __MC6502_OP_H__
#define __MC6502_OP_H__


#define READMEM8( a )		(BYTE)g_pBoard->m_cIOU.ReadMem8( a )
#define READMEM16( a )		(WORD)( g_pBoard->m_cIOU.ReadMem16( a ) )
#define WRITEMEM8( a, d )	g_pBoard->m_cIOU.WriteMem8( a, d )

#define READOPCODE8		( addr = this->m_regPC, this->m_regPC++, READMEM8( addr ) )
#define READOPCODE16	( addr = this->m_regPC, this->m_regPC+=2, READMEM16( addr ) )

#define CALC_ADDR(addr, off)					\
	if ( ( ( addr + off ) ^ addr ) & 0xff00 ) {	\
		clock++;					\
	}											\
	addr += off

#ifdef _DEBUG
#define TRACE_CALL  m_trace[(m_current++)&0xFF] = opcode_addr
//#define TRACE_JMP	m_trace[(m_current++)&0xFF] = opcode_addr
#endif
#ifndef TRACE_CALL
#define TRACE_CALL
#endif
#ifndef TRACE_JMP
#define TRACE_JMP
#endif

/* Absolute */
#define ABS			addr = READOPCODE16
/* Absolute, X */
#define ABS_X		addr = READOPCODE16; CALC_ADDR( addr, this->m_regX )
/* Absolute, X : no additional clock */
#define ABS_X2		addr = READOPCODE16; addr += this->m_regX
/* Absolute, Y */
#define ABS_Y		addr = READOPCODE16; CALC_ADDR( addr, this->m_regY )
/* Absolute, Y : no additional clock */
#define ABS_Y2		addr = READOPCODE16; addr += this->m_regY
/* INC Absolute, X */
#define INC_ABS_X	addr = READOPCODE16 + this->m_regX
/* ZeroPage */
#define ZP			addr = READOPCODE8
/* ZeroPage, X */
#define ZP_X		addr = ( READOPCODE8 + this->m_regX ) & 0xff
/* ZeroPage, Y */
#define ZP_Y		addr = ( READOPCODE8 + this->m_regY ) & 0xff
/* (Indirect) */
#define IND			ZP; addr = READMEM16( addr );
/* (Indirect16) */
#define IND16		ABS; addr = READMEM16( addr );
/* (Indirect, X) */
#define IND_X		ZP_X; addr = READMEM16( addr );
/* (Indirect16, X) */
#define IND16_X		ABS_X2; addr = READMEM16( addr );
/* (Indirect), Y */
#define IND_Y		ZP; addr = READMEM16( addr ); CALC_ADDR( addr, this->m_regY );
/* (Indirect), Y : no additional clock */
#define IND_Y2		ZP; addr = READMEM16( addr ); addr += this->m_regY

#define IMM			data = READOPCODE8
#define MEM			data = READMEM8( addr )
#define WMEM		WRITEMEM8( addr, (BYTE)result )
#define ACC			data = this->m_regA
#define WACC		this->m_regA = (BYTE)result
#define XREG		data = this->m_regX
#define WXREG		this->m_regX = (BYTE)result
#define YREG		data = this->m_regY
#define WYREG		this->m_regY = (BYTE)result

#define SET_FLAG(flag)		this->m_regF |= flag
#define CLEAR_FLAG(flag)	this->m_regF &= ~flag
#define TEST_FLAG(flag)		( ( this->m_regF & flag ) != 0 )

#define UPDATEFLAG_NZ						\
	CLEAR_FLAG( NZ_Flag );					\
	if ( ( result & 0x280 ) != 0 )			\
		SET_FLAG( N_Flag );					\
	if ( ( result & 0xff ) == 0 )			\
		SET_FLAG( Z_Flag )
#define UPDATEFLAG_NZC						\
	CLEAR_FLAG( NZC_Flag );					\
	if ( ( result & 0x280 ) != 0 )			\
		SET_FLAG( N_Flag );					\
	if ( ( result & 0xff ) == 0 )			\
		SET_FLAG( Z_Flag );					\
	if ( ( result & 0x100 ) != 0 )			\
		SET_FLAG( C_Flag )

#define BRANCH(off)							\
	TRACE_JMP;								\
	CALC_ADDR(this->m_regPC, (char)off);	\
	clock++

#define CLOCK(c)							\
	clock += c;								\
	if (m_clockListener != NULL)			\
		m_clockListener->Clock(clock);

#define CLOCK2(c)							\
	clock += c;								\
	if (m_clockListener != NULL)			\
		m_clockListener->Clock(c);

#define UPDATEFLAG( cond, flag )			\
	if ( cond )								\
		SET_FLAG( flag );					\
	else									\
		CLEAR_FLAG( flag )

#define ADD_SUB(bcdTable)					\
	UPDATEFLAG ( !((data^this->m_regA)&0x80) && ((result^this->m_regA)&0x80), V_Flag );	\
	if ( TEST_FLAG(D_Flag) ) {				\
		result = bcdTable[result];			\
		CLOCK2(1);							\
	}										\
	UPDATEFLAG_NZC;

#define ADD									\
	result = this->m_regA + data;				\
	ADD_SUB(m_BCD_Table1)
#define ADC									\
	result = this->m_regA + data + ( this->m_regF & C_Flag );	\
	ADD_SUB(m_BCD_Table1)
#define SBB									\
	data = (BYTE)( 0x100 - data );		\
	result = this->m_regA + data;				\
	ADD_SUB(m_BCD_Table2)
#define SBC									\
	data = (BYTE)( 0xff - data );		\
	result = this->m_regA + data + ( this->m_regF & C_Flag );	\
	ADD_SUB(m_BCD_Table2)
#define AND									\
	result = this->m_regA & data;				\
	UPDATEFLAG_NZ;
#define ASL									\
	result = data << 1;						\
	UPDATEFLAG_NZC;

#define BRA_COND(flag)						\
	offset = READOPCODE8;					\
	if ( TEST_FLAG( flag ) )				\
		{ BRANCH( offset ); }
#define BRA_NCOND(flag)						\
	offset = READOPCODE8;					\
	if ( !TEST_FLAG( flag ) )				\
		{ BRANCH( offset ); }
#define BRA									\
	offset = READOPCODE8;					\
	BRANCH( offset )

#define BIT									\
	result = ( (data&0x80)<<2 ) | ( data & this->m_regA );	\
	UPDATEFLAG( data & 0x40, V_Flag );		\
	UPDATEFLAG_NZ
#define BRK									\
	this->m_regPC++;							\
	PUSH( (BYTE)( this->m_regPC >> 8 ) );	\
	PUSH( (BYTE)( this->m_regPC ) );		\
	SET_FLAG( B_Flag | X_Flag );			\
	PUSH( this->m_regF );						\
	SET_FLAG( I_Flag );						\
	this->m_regPC = READMEM16( 0xfffe )
#define CMP									\
	result = 0x100 + this->m_regA - data;		\
	UPDATEFLAG_NZC
#define CPX									\
	result = 0x100 + this->m_regX - data;		\
	UPDATEFLAG_NZC
#define CPY									\
	result = 0x100 + this->m_regY - data;		\
	UPDATEFLAG_NZC
#define DEC									\
	result = data + 0xff;					\
	UPDATEFLAG_NZ
#define EOR									\
	result = this->m_regA ^ data;				\
	UPDATEFLAG_NZ
#define INC									\
	result = data + 1;						\
	UPDATEFLAG_NZ
#define JMP									\
	TRACE_JMP;								\
	this->m_regPC = addr
#define JSR									\
	TRACE_CALL;								\
	this->m_regPC--;							\
	PUSH( (BYTE)( this->m_regPC >> 8 ) );	\
	PUSH( (BYTE)( this->m_regPC ) );		\
	this->m_regPC = addr
#define LOAD								\
	result = data;							\
	UPDATEFLAG_NZ
#define LSR									\
	result = ( (data&1)<<8 ) | ( data>>1 );	\
	UPDATEFLAG_NZC
#define ORA									\
	result = this->m_regA | data;				\
	UPDATEFLAG_NZ
#define ROL									\
	result = ( data << 1 ) | ( this->m_regF & C_Flag );	\
	UPDATEFLAG_NZC
#define ROR									\
	result = ( (data&1)<<8 ) | ((this->m_regF&C_Flag)<<7) | (data>>1);	\
	UPDATEFLAG_NZC
#define RTI									\
	TRACE_CALL;								\
	this->m_regF = POP | 0x20; /* bit 5 bug of 6502 */ \
	this->m_regPC = POP;						\
	this->m_regPC += POP << 8
#define RTS									\
	TRACE_CALL;								\
	this->m_regPC = POP;						\
	this->m_regPC += ( POP << 8 ) + 1

#define STA									\
	WRITEMEM8( addr, (BYTE)this->m_regA )
#define STX									\
	WRITEMEM8( addr, (BYTE)this->m_regX )
#define STY									\
	WRITEMEM8( addr, (BYTE)this->m_regY )
#define STZ									\
	WRITEMEM8( addr, (BYTE)0 )

#define PUSH( d )							\
	WRITEMEM8( ( this->m_regS-- ) | 0x100, d )
#define POP									\
	READMEM8( ( ++this->m_regS ) | 0x100 )
#define POPR								\
	result = POP;							\
	UPDATEFLAG_NZ
#define POPF								\
	this->m_regF = POP;	\
	CHECK_IRQ

#define TRANS( s, d )						\
	result = d = s;							\
	UPDATEFLAG_NZ

#define TAX		TRANS( this->m_regA, this->m_regX )
#define TAY		TRANS( this->m_regA, this->m_regY )
#define TSX		TRANS( this->m_regS, this->m_regX )
#define TXA		TRANS( this->m_regX, this->m_regA )
#define TXS		TRANS( this->m_regX, this->m_regS )
#define TYA		TRANS( this->m_regY, this->m_regA )

#define BBR(bit)							\
	offset = READOPCODE8;					\
	if ( ( data & bit ) == 0 )				\
		{ BRANCH( offset ); }
#define BBS(bit)							\
	offset = READOPCODE8;					\
	if ( ( data & bit ) != 0 )				\
		{ BRANCH( offset ); }
#define RMB(bit)							\
	result = data & ~bit
#define SMB(bit)							\
	result = data | bit

#define TRB									\
	UPDATEFLAG ( data & this->m_regA, Z_Flag );	\
	result = data & ~this->m_regA
#define TSB									\
	UPDATEFLAG ( data & this->m_regA, Z_Flag );	\
	result = data | this->m_regA;

#define CHECK_IRQ	\
	if( PendingIRQ && !(m_regF&I_Flag)){	\
		PendingIRQ--;	\
		Assert_IRQ();	\
	}

#define STOP	\
	Shutdown()

#define WAIT	\
	Wait()

#endif
