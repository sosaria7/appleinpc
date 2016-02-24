#if !defined(OPTABLE__INCLUDED_)
#define OPTABLE__INCLUDED_

#define M_NONE	0
#define M_IMM	1			// #Immediate
#define M_REL	2			// Relative			<-- for branch
#define M_ZP	3			// Zero page
#define M_ZPX	4			// Zero page, X
#define M_ZPY	5			// Zero page, Y
#define	M_ABS	6			// Absolute
#define M_ABSX	7			// Absolute, X
#define M_ABSY	8			// Absolute, Y
#define M_ZIND	9			// (Indirect8)		<-- (65C02)
#define M_IND	10			// (Indirect16)		<-- only for JMP
#define M_INDX	11			// (Indirect8, X)
#define M_INDY	12			// (Indirect8), Y
#define M_ABINDX	13		// (Indirect16, X)	<-- only for JMP(65C02)
#define M_BB	14			// Zero Page, Relative	<-- only for BBR/BBS(65C02)

typedef struct _OP_TABLE{
	char* OPERATOR;
	unsigned int type;
	unsigned short len;
} OP_TABLE;

static OP_TABLE operators_65c02[] = {
		{"BRK",	M_NONE,	1},		/* 0x0 */
		{"ORA",	M_INDX,	2},		/* 0x1 */
		{"???",	M_NONE,	1},		/* 0x2 */
		{"???",	M_NONE,	1},		/* 0x3 */
		{"TSB",	M_ZP,	2},		/* 0x4 */	/* 65C02 */
		{"ORA",	M_ZP,	2},		/* 0x5 */
		{"ASL",	M_ZP,	2},		/* 0x6 */
		{"RMB0",M_ZP,	2},		/* 0x7 */
		{"PHP",	M_NONE,	1},		/* 0x8 */
		{"ORA",	M_IMM,	2},		/* 0x9 */
		{"ASL",	M_NONE,	1},		/* 0xa */
		{"???",	M_NONE,	1},		/* 0xb */
		{"TSB",	M_ABS,	3},		/* 0xc */	/* 65C02 */
		{"ORA",	M_ABS,	3},		/* 0xd */
		{"ASL",	M_ABS,	3},		/* 0xe */
		{"BBR0",M_BB,	3},		/* 0xf */
		{"BPL",	M_REL,	2},		/* 0x10 */
		{"ORA",	M_INDY,	2},		/* 0x11 */
		{"ORA",	M_ZIND,	2},		/* 0x12 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x13 */
		{"TRB",	M_ZP,	2},		/* 0x14 */	/* 65C02 */
		{"ORA",	M_ZPX,	2},		/* 0x15 */
		{"ASL",	M_ZPX,	2},		/* 0x16 */
		{"RMB1",M_ZP,	2},		/* 0x17 */
		{"CLC",	M_NONE,	1},		/* 0x18 */
		{"ORA",	M_ABSY,	3},		/* 0x19 */
		{"INA",	M_NONE,	1},		/* 0x1a */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x1b */
		{"TRB",	M_ABS,	3},		/* 0x1c */	/* 65C02 */
		{"ORA",	M_ABSX,	3},		/* 0x1d */
		{"ASL",	M_ABSX,	3},		/* 0x1e */
		{"BBR1",M_BB,	3},		/* 0x1f */
		{"JSR",	M_ABS,	3},		/* 0x20 */
		{"AND",	M_INDX,	2},		/* 0x21 */
		{"???",	M_NONE,	1},		/* 0x22 */
		{"???",	M_NONE,	1},		/* 0x23 */
		{"BIT",	M_ZP,	2},		/* 0x24 */
		{"AND",	M_ZP,	2},		/* 0x25 */
		{"ROL",	M_ZP,	2},		/* 0x26 */
		{"RMB2",M_ZP,	2},		/* 0x27 */
		{"PLP",	M_NONE,	1},		/* 0x28 */
		{"AND",	M_IMM,	2},		/* 0x29 */
		{"ROL",	M_NONE,	1},		/* 0x2a */
		{"???",	M_NONE,	1},		/* 0x2b */
		{"BIT",	M_ABS,	3},		/* 0x2c */
		{"AND",	M_ABS,	3},		/* 0x2d */
		{"ROL",	M_ABS,	3},		/* 0x2e */
		{"BBR2",M_BB,	3},		/* 0x2f */
		{"BMI",	M_REL,	2},		/* 0x30 */
		{"AND",	M_INDY,	2},		/* 0x31 */
		{"AND",	M_ZIND,	2},		/* 0x32 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x33 */
		{"BIT",	M_ZPX,	2},		/* 0x34 */	/* 65C02 */
		{"AND",	M_ZPX,	2},		/* 0x35 */
		{"ROL",	M_ZPX,	2},		/* 0x36 */
		{"RMB3",M_ZP,	2},		/* 0x37 */
		{"SEC",	M_NONE,	1},		/* 0x38 */
		{"AND",	M_ABSY,	3},		/* 0x39 */
		{"DEY",	M_NONE,	1},		/* 0x3a */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x3b */
		{"BIT",	M_ABSX,	3},		/* 0x3c */	/* 65C02 */
		{"AND",	M_ABSX,	3},		/* 0x3d */
		{"ROL",	M_ABSX,	3},		/* 0x3e */
		{"BBR3",M_BB,	3},		/* 0x3f */
		{"RTI",	M_NONE,	1},		/* 0x40 */
		{"EOR",	M_INDX,	2},		/* 0x41 */
		{"???",	M_NONE,	1},		/* 0x42 */
		{"???",	M_NONE,	1},		/* 0x43 */
		{"???",	M_NONE,	1},		/* 0x44 */
		{"EOR",	M_ZP,	2},		/* 0x45 */
		{"LSR",	M_ZP,	2},		/* 0x46 */
		{"RMB4",M_ZP,	2},		/* 0x47 */
		{"PHA",	M_NONE,	1},		/* 0x48 */
		{"EOR",	M_IMM,	2},		/* 0x49 */
		{"LSR",	M_NONE,	1},		/* 0x4a */
		{"???",	M_NONE,	1},		/* 0x4b */
		{"JMP",	M_ABS,	3},		/* 0x4c */
		{"EOR",	M_ABS,	3},		/* 0x4d */
		{"LSR",	M_ABS,	3},		/* 0x4e */
		{"BBR4",M_BB,	3},		/* 0x4f */
		{"BVC",	M_REL,	2},		/* 0x50 */
		{"EOR",	M_INDY,	2},		/* 0x51 */
		{"EOR",	M_ZIND,	2},		/* 0x52 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x53 */
		{"???",	M_NONE,	1},		/* 0x54 */
		{"EOR",	M_ZPX,	2},		/* 0x55 */
		{"LSR",	M_ZPX,	2},		/* 0x56 */
		{"RMB5",M_ZP,	2},		/* 0x57 */
		{"CLI",	M_NONE,	1},		/* 0x58 */
		{"EOR",	M_ABSY,	3},		/* 0x59 */
		{"PHY",	M_NONE,	1},		/* 0x5a */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x5b */
		{"???",	M_NONE,	1},		/* 0x5c */
		{"EOR",	M_ABSX,	3},		/* 0x5d */
		{"LSR",	M_ABSX,	3},		/* 0x5e */
		{"BBR5",M_BB,	3},		/* 0x5f */
		{"RTS",	M_NONE,	1},		/* 0x60 */
		{"ADC",	M_INDX,	2},		/* 0x61 */
		{"???",	M_NONE,	1},		/* 0x62 */
		{"???",	M_NONE,	1},		/* 0x63 */
		{"STZ",	M_ZP,	2},		/* 0x64 */	/* 65C02 */
		{"ADC",	M_ZP,	2},		/* 0x65 */
		{"ROR",	M_ZP,	2},		/* 0x66 */
		{"RMB6",M_ZP,	2},		/* 0x67 */
		{"PLA",	M_NONE,	1},		/* 0x68 */
		{"ADC",	M_IMM,	2},		/* 0x69 */
		{"ROR",	M_NONE,	1},		/* 0x6a */
		{"???",	M_NONE,	1},		/* 0x6b */
		{"JMP",	M_IND,	3},		/* 0x6c */
		{"ADC",	M_ABS,	3},		/* 0x6d */
		{"ROR",	M_ABS,	3},		/* 0x6e */
		{"BBR6",M_BB,	3},		/* 0x6f */
		{"BVC",	M_REL,	2},		/* 0x70 */
		{"ADC",	M_INDX,	2},		/* 0x71 */
		{"ADC",	M_ZIND,	2},		/* 0x72 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x73 */
		{"STC",	M_ZPX,	2},		/* 0x74 */	/* 65C02 */
		{"ADC",	M_ZPX,	2},		/* 0x75 */
		{"ROR",	M_ZPX,	2},		/* 0x76 */
		{"RMB7",M_ZP,	2},		/* 0x77 */
		{"SEI",	M_NONE,	1},		/* 0x78 */
		{"ADC",	M_ABSY,	3},		/* 0x79 */
		{"PLY",	M_NONE,	1},		/* 0x7a */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x7b */
		{"JMP",	M_ABINDX, 3},	/* 0x7c */	/* 65C02 */
		{"ADC",	M_ABSX,	3},		/* 0x7d */
		{"ROR",	M_ABSX,	3},		/* 0x7e */
		{"BBR7",M_BB,	3},		/* 0x7f */
		{"BRA",	M_REL,	2},		/* 0x80 */	/* 65C02 */
		{"STA",	M_INDX,	2},		/* 0x81 */
		{"???",	M_NONE,	1},		/* 0x82 */
		{"???",	M_NONE,	1},		/* 0x83 */
		{"STY",	M_ZP,	2},		/* 0x84 */
		{"STA",	M_ZP,	2},		/* 0x85 */
		{"STX",	M_ZP,	2},		/* 0x86 */
		{"SMB0",M_ZP,	2},		/* 0x87 */
		{"DEY",	M_NONE,	1},		/* 0x88 */
		{"???",	M_NONE,	1},		/* 0x89 */
		{"TXA",	M_NONE,	1},		/* 0x8a */
		{"???",	M_NONE,	1},		/* 0x8b */
		{"STY",	M_ABS,	3},		/* 0x8c */
		{"STA",	M_ABS,	3},		/* 0x8d */
		{"STX",	M_ABS,	3},		/* 0x8e */
		{"BBS0",M_BB,	3},		/* 0x8f */
		{"BCC",	M_REL,	2},		/* 0x90 */
		{"STA",	M_INDY,	2},		/* 0x91 */
		{"STA",	M_ZIND,	2},		/* 0x92 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0x93 */
		{"STY",	M_ZPX,	2},		/* 0x94 */
		{"STA",	M_ZPX,	2},		/* 0x95 */
		{"STX",	M_ZPX,	2},		/* 0x96 */
		{"SMB1",M_ZP,	2},		/* 0x97 */
		{"TYA",	M_NONE,	1},		/* 0x98 */
		{"STA",	M_ABSY,	3},		/* 0x99 */
		{"TXS",	M_NONE,	1},		/* 0x9a */
		{"???",	M_NONE,	1},		/* 0x9b */
		{"STZ",	M_ABS,	3},		/* 0x9c */	/* 65C02 */
		{"STA",	M_ABSX,	3},		/* 0x9d */
		{"STZ",	M_ABSX,	3},		/* 0x9e */	/* 65C02 */
		{"BBS1",M_BB,	3},		/* 0x9f */
		{"LDY",	M_IMM,	2},		/* 0xa0 */
		{"LDA",	M_INDX,	2},		/* 0xa1 */
		{"LDX",	M_IMM,	2},		/* 0xa2 */
		{"???",	M_NONE,	1},		/* 0xa3 */
		{"LDY",	M_ZP,	2},		/* 0xa4 */
		{"LDA",	M_ZP,	2},		/* 0xa5 */
		{"LDX",	M_ZP,	2},		/* 0xa6 */
		{"SMB2",M_ZP,	2},		/* 0xa7 */
		{"TAY",	M_NONE,	1},		/* 0xa8 */
		{"LDA",	M_IMM,	2},		/* 0xa9 */
		{"TAX",	M_NONE,	1},		/* 0xaa */
		{"???",	M_NONE,	1},		/* 0xab */
		{"LDY",	M_ABS,	3},		/* 0xac */
		{"LDA",	M_ABS,	3},		/* 0xad */
		{"LDX",	M_ABS,	3},		/* 0xae */
		{"BBS2",M_BB,	3},		/* 0xaf */
		{"BCS",	M_REL,	2},		/* 0xb0 */
		{"LDA",	M_INDY,	2},		/* 0xb1 */
		{"LDA",	M_ZIND,	2},		/* 0xb2 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0xb3 */
		{"LDY",	M_ZPX,	2},		/* 0xb4 */
		{"LDA",	M_ZPX,	2},		/* 0xb5 */
		{"LDX",	M_ZPY,	2},		/* 0xb6 */
		{"SMB3",M_ZP,	2},		/* 0xb7 */
		{"CLV",	M_NONE,	1},		/* 0xb8 */
		{"LDA",	M_ABSY,	3},		/* 0xb9 */
		{"TSX",	M_NONE,	1},		/* 0xba */
		{"???",	M_NONE,	1},		/* 0xbb */
		{"LDY",	M_ABSX,	3},		/* 0xbc */
		{"LDA",	M_ABSX,	3},		/* 0xbd */
		{"LDX",	M_ABSY,	3},		/* 0xbe */
		{"BBS3",M_BB,	3},		/* 0xbf */
		{"CPY",	M_IMM,	2},		/* 0xc0 */
		{"CMP",	M_INDX,	2},		/* 0xc1 */
		{"???",	M_NONE,	1},		/* 0xc2 */
		{"???",	M_NONE,	1},		/* 0xc3 */
		{"CPY",	M_ZP,	2},		/* 0xc4 */
		{"CMP",	M_ZP,	2},		/* 0xc5 */
		{"DEC",	M_ZP,	2},		/* 0xc6 */
		{"SMB4",M_ZP,	2},		/* 0xc7 */
		{"INY",	M_NONE,	1},		/* 0xc8 */
		{"CMP",	M_IMM,	2},		/* 0xc9 */
		{"DEX",	M_NONE,	1},		/* 0xca */
		{"WAI",	M_NONE,	1},		/* 0xcb */
		{"CPY",	M_ABS,	3},		/* 0xcc */
		{"CMP",	M_ABS,	3},		/* 0xcd */
		{"DEC",	M_ABS,	3},		/* 0xce */
		{"BBS4",M_BB,	3},		/* 0xcf */
		{"BNE",	M_REL,	2},		/* 0xd0 */
		{"CMP",	M_INDY,	2},		/* 0xd1 */
		{"CMP",	M_ZIND,	2},		/* 0xd2 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0xd3 */
		{"???",	M_NONE,	1},		/* 0xd4 */
		{"CMP",	M_ZPX,	2},		/* 0xd5 */
		{"DEC",	M_ZPX,	2},		/* 0xd6 */
		{"SMB5",M_ZP,	2},		/* 0xd7 */
		{"CLD",	M_NONE,	1},		/* 0xd8 */
		{"CMP",	M_ABSY,	3},		/* 0xd9 */
		{"PHX",	M_NONE,	1},		/* 0xda */	/* 65C02 */
		{"STP",	M_NONE,	1},		/* 0xdb */
		{"???",	M_NONE,	1},		/* 0xdc */
		{"CMP",	M_ABSX,	3},		/* 0xdd */
		{"DEC",	M_ABSX,	3},		/* 0xde */
		{"BBS5",M_BB,	3},		/* 0xdf */
		{"CPX",	M_IMM,	2},		/* 0xe0 */
		{"SBC",	M_INDX,	2},		/* 0xe1 */
		{"???",	M_NONE,	1},		/* 0xe2 */
		{"???",	M_NONE,	1},		/* 0xe3 */
		{"CPX",	M_ZP,	2},		/* 0xe4 */
		{"SBC",	M_ZP,	2},		/* 0xe5 */
		{"INC",	M_ZP,	2},		/* 0xe6 */
		{"SMB6",M_ZP,	2},		/* 0xe7 */
		{"INX",	M_NONE,	1},		/* 0xe8 */
		{"SBC",	M_IMM,	2},		/* 0xe9 */
		{"NOP",	M_NONE,	1},		/* 0xea */
		{"???",	M_NONE,	1},		/* 0xeb */
		{"CPX",	M_ABS,	3},		/* 0xec */
		{"SBC",	M_ABS,	3},		/* 0xed */
		{"INC",	M_ABS,	3},		/* 0xee */
		{"BBS6",M_BB,	3},		/* 0xef */
		{"BEQ",	M_REL,	2},		/* 0xf0 */
		{"SBC",	M_INDY,	2},		/* 0xf1 */
		{"SBC",	M_ZIND,	2},		/* 0xf2 */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0xf3 */
		{"???",	M_NONE,	1},		/* 0xf4 */
		{"SBC",	M_ZPX,	2},		/* 0xf5 */
		{"INC",	M_ZPX,	2},		/* 0xf6 */
		{"SMB7",M_ZP,	2},		/* 0xf7 */
		{"SED",	M_NONE,	1},		/* 0xf8 */
		{"SBC",	M_ABSY,	3},		/* 0xf9 */
		{"PLX",	M_NONE,	1},		/* 0xfa */	/* 65C02 */
		{"???",	M_NONE,	1},		/* 0xfb */
		{"???",	M_NONE,	1},		/* 0xfc */
		{"SBC",	M_ABSX,	3},		/* 0xfd */
		{"INC",	M_ABSX,	3},		/* 0xfe */
		{"BBS7",M_BB,	3},		/* 0xff */
};

#endif