// 6522.cpp: implementation of the C6522 class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "appleclock.h"
#include "6522.h"
#include "65c02.h"
//#include "appleclock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C6522::C6522()
{
	Reset();
}

C6522::~C6522()
{
	
}

void C6522::Write(BYTE reg, BYTE data)
{
	int ctrl;
    reg &= 0x0F;
    
    switch( reg )
    {
    case SY_ORA:
		m_abyRegs[SY_IFR] &= ~0x03;
		SetIFR7();
		ctrl = ( m_abyRegs[SY_PCR] >> 1 ) & 0x07;		// CA2 Control in PCR
		if ( ctrl != 1 && ctrl != 3 )
			m_abyRegs[SY_PCR] &= ~( SY_BIT_CA1 | SY_BIT_CA2 );
        m_byORA = ( m_byORA & ~m_abyRegs[SY_DDRA] ) | ( data & m_abyRegs[SY_DDRA] );
		break;
    case SY_DDRA:
        m_byORA = ( m_byORA & ~data ) | ( m_abyRegs[SY_ORA] & data );
		break;
    case SY_ORB:
		m_abyRegs[SY_IFR] &= ~0x18;
		SetIFR7();
		ctrl = ( m_abyRegs[SY_PCR] >> 5 ) & 0x07;		// CB2 Control in PCR
		if ( ctrl != 1 && ctrl != 3 )
			m_abyRegs[SY_PCR] &= ~( SY_BIT_CB1 | SY_BIT_CB2 );
        m_byORB = ( m_byORB & ~m_abyRegs[SY_DDRB] ) | ( data & m_abyRegs[SY_DDRB] );
		break;
    case SY_DDRB:
        m_byORB = ( m_byORB & ~data ) | ( m_abyRegs[SY_ORB] & data );
		break;
	case SY_SR:
		m_abyRegs[SY_PCR] &= ~( SY_BIT_SR );
		break;

	case SY_T1C_L:
	case SY_T1L_L:
		m_abyRegs[SY_T1L_L] = data;
		return;

	case SY_T1C_H:
		m_abyRegs[SY_T1L_H] = data;
//		m_abyRegs[SY_T1C_H] = data;
		m_abyRegs[SY_T1C_L] = m_abyRegs[SY_T1L_L];
		m_byExtraClock = 1;		// N+1.5
		m_byIRQBCtl = 0;
		m_abyRegs[SY_IFR] &= ~(SY_BIT_T1);
		SetIFR7();
		break;

	case SY_T1L_H:
//		m_abyRegs[SY_T1L_H] = data;
		m_abyRegs[SY_IFR] &= ~(SY_BIT_T1);
		SetIFR7();
		break;

	case SY_T2C_L:
		m_byT2L_L = data;
		return;

	case SY_T2C_H:
//		m_abyRegs[SY_T2C_H] = data;
		m_abyRegs[SY_T2C_L] = m_byT2L_L;
		m_abyRegs[SY_IFR] &= ~( SY_BIT_T2 );
		SetIFR7();
		break;
	case SY_IFR:
		m_abyRegs[SY_IFR] &=  ~data;
		SetIFR7();
		return;

	case SY_IER:
		if ( ( data & m_abyRegs[SY_IFR] ) > 0x80 )
			m_bIRQB = TRUE;
		break;

    default:
		break;
    }
	m_abyRegs[reg] = data;
}

BYTE C6522::Read(BYTE reg)
{
    reg &= 0x0F;
    switch( reg )
    {
    case SY_ORA:
		m_abyRegs[SY_IFR] &= ~0x03;
		SetIFR7();
	case SY_ORAHLP:
		if ( m_abyRegs[SY_ACR] & 0x01 )	    // PA latching enabled
			return ( m_byIRA & ~m_abyRegs[SY_DDRA] ) | ( m_abyRegs[SY_ORA] & m_abyRegs[SY_DDRA] );
		else
			return ( m_byORA & ~m_abyRegs[SY_DDRA] ) | ( m_abyRegs[SY_ORA] & m_abyRegs[SY_DDRA] );
		break;
    case SY_ORB:
		m_abyRegs[SY_IFR] &= ~0x18;
		SetIFR7();
		if ( m_abyRegs[SY_ACR] & 0x02 )	    // PB latching enabled
			return ( m_byIRB & ~m_abyRegs[SY_DDRB] ) | ( m_abyRegs[SY_ORB] & m_abyRegs[SY_DDRB] );
		else
			return ( m_byORB & ~m_abyRegs[SY_DDRB] ) | ( m_abyRegs[SY_ORB] & m_abyRegs[SY_DDRB] );
		break;
	case SY_SR:
		m_abyRegs[SY_PCR] &= ~( SY_BIT_SR );
		break;

	case SY_T1C_L:
//		Clock( 0 );
		m_abyRegs[SY_IFR] &= ~( SY_BIT_T1 );
		SetIFR7();
		break;
	case SY_T1L_L:
		break;
	case SY_T1L_H:
		return( m_abyRegs[SY_T1C_H] );
		break;
	case SY_T1C_H:
//		Clock( 0 );
		break;
	case SY_T2C_L:
		m_abyRegs[SY_PCR] &= ~( SY_BIT_T2 );
		SetIFR7();
		break;
	case SY_IER:
		return 0x80;
    case SY_DDRA:
    case SY_DDRB:
	case SY_IFR:
    default:
		break;
    }
	
    return m_abyRegs[reg];
}

void C6522::Reset()
{
    int i;
    for( i = 0; i < 16; i++ )
		m_abyRegs[i] = 0;
    m_byORA = 0;
    m_byORB = 0;
    m_byIRA = 0;
    m_byIRB = 0;
    m_bIRQB = FALSE;
    m_bCA1 = FALSE;
    m_bCA2 = FALSE;
    m_bCB1 = FALSE;
    m_bCB2 = FALSE;
	m_byExtraClock = 0;
	m_byIRQBCtl = 0;

	m_bDoubleClock = FALSE;
}

BOOL C6522::GetIRQB()
{
	if (m_byIRQBCtl == 0 && m_bIRQB == TRUE)
	{
		m_bIRQB = FALSE;
		if (!(m_abyRegs[SY_ACR] & 0x40))	// oneshot
			m_byIRQBCtl = 2;
		return TRUE;
	}
	m_bIRQB = FALSE;
	return FALSE;
}

void C6522::Clock(WORD clock)
{
	WORD temp;
	BYTE bPB7 = m_byORB & 0x80;
	if ( m_bDoubleClock )
		clock <<= 1;

	temp = m_abyRegs[SY_T1C_L] | ( m_abyRegs[SY_T1C_H] << 8 );

	if (m_byIRQBCtl == 1)
	{
		m_byIRQBCtl = 0;
		m_bIRQB = TRUE;
	}
	if (m_byExtraClock > 0)
	{
		m_byExtraClock--;
		clock--;
	}

	if (temp < clock)
	{
		m_abyRegs[SY_IFR] |= SY_BIT_T1 | SY_BIT_IRQ;
		if (m_byIRQBCtl != 2)
		{
			if ((m_abyRegs[SY_IER] & m_abyRegs[SY_IFR]) > 0x80)
			{
				// irq set in next half clock.
				if (temp + 1 == clock)
				{
					m_byIRQBCtl = 1;
				}
				else
				{
					m_bIRQB = TRUE;
				}
			}
			if (m_abyRegs[SY_ACR] & 0x80)		// output enabled
			{
				m_byORB ^= 0x80;
			}
		}

		temp += (m_abyRegs[SY_T1L_L] | (m_abyRegs[SY_T1L_H] << 8)) + 1;
		m_byExtraClock = 1;	// need 2 more clock
	}

	temp -= clock;
	m_abyRegs[SY_T1C_L] = temp & 0xFF;
	m_abyRegs[SY_T1C_H] = temp >> 8;
}

BYTE C6522::GetORA()
{
    return m_byORA;
}

BYTE C6522::GetORB()
{
    return m_byORB;
}

void C6522::SetCA1(BOOL ca1)
{
    if ( ca1 == m_bCA1 )
		return;
    m_bCA1 = ca1;
    if ( m_abyRegs[SY_PCR] & ( 1 << 0 ) )   // Positive Active Edge
    {
		if ( ca1 )
		{
			m_byIRA = m_byORA;
			m_abyRegs[SY_IFR] |= 0x02;
			SetIFR7();
		}
    }
    else				    // Negative Active Edge
    {
		if ( !ca1 )
		{
			m_byIRA = m_byORA;
			m_abyRegs[SY_IFR] |= 0x02;
			SetIFR7();
		}
    }
	
}

void C6522::SetCA2(BOOL ca2)
{
    if ( ca2 == m_bCA2 )
		return;
    m_bCA2 = ca2;
}

void C6522::SetCB1(BOOL cb1)
{
    if ( cb1 == m_bCB1 )
		return;
    m_bCB1 = cb1;
    if ( m_abyRegs[SY_PCR] & ( 1 << 4 ) )   // Positive Active Edge
    {
		if ( cb1 )
		{
			m_byIRB = m_byORB;
			m_abyRegs[SY_IFR] |= 0x10;
			SetIFR7();
		}
    }
    else				    // Negative Active Edge
    {
		if ( !cb1 )
		{
			m_byIRB = m_byORB;
			m_abyRegs[SY_IFR] |= 0x10;
			SetIFR7();
		}
    }
}

void C6522::SetIFR7()
{
	if ( m_abyRegs[SY_IFR] & 0x7F )
		m_abyRegs[SY_IFR] |= SY_BIT_IRQ;
	else
		m_abyRegs[SY_IFR] &= ~SY_BIT_IRQ;

	if ( ( m_abyRegs[SY_IER] & m_abyRegs[SY_IFR] ) > 0x80 )
		m_bIRQB = TRUE;
}

void C6522::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_byIRA;
		ar << m_byIRB;
		ar << m_bIRQB;
		ar << m_byORB;
		ar << m_byORA;
		ar << m_bCA1;
		ar << m_bCA2;
		ar << m_bCB1;
		ar << m_bCB2;
		ar.Write( m_abyRegs, sizeof(m_abyRegs) );
		ar << m_byExtraClock;
		ar << m_byIRQBCtl;
		ar << m_byT2L_L;
	}
	else
	{
		ar >> m_byIRA;
		ar >> m_byIRB;
		ar >> m_bIRQB;
		ar >> m_byORB;
		ar >> m_byORA;
		ar >> m_bCA1;
		ar >> m_bCA2;
		ar >> m_bCB1;
		ar >> m_bCB2;
		ar.Read( m_abyRegs, sizeof(m_abyRegs) );
		if (g_nSerializeVer >= 11)
		{
			ar >> m_byExtraClock;
		}
		if (g_nSerializeVer >= 13)
		{
			ar >> m_byIRQBCtl;
			ar >> m_byT2L_L;
		}
	}
}
