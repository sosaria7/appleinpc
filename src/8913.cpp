
/***************************************************************************
  Port of ay8910.c from mame
  The original source is licenses/src/ay8910.c
  MAME license file is licenses/MAME.txt
***************************************************************************/

/***************************************************************************

  ay8910.c


  Emulation of the AY-3-8910 / YM2149 sound chip.

  Based on various code snippets by Ville Hallik, Michael Cuddy,
  Tatsuyuki Satoh, Fabrice Frances, Nicola Salmoria.

***************************************************************************/

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "8913.h"
#include "65c02.h"
#include "appleclock.h"

#include <dsound.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define STEP	0x8000

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C8913::C8913()
{
	int i;
	Reset();
	m_dwClock = CLOCK;
	m_iUpdateStep = (int)( ( (double)STEP * g_DXSound.m_nSampleRate * 8 + m_dwClock / 2 ) / m_dwClock );
	m_nNumOfBuf = 3;
	m_lpwBuf = new LPWORD[3];
	for( i = 0; i < 3; i++ )
		m_lpwBuf[i] = new WORD[SAMPLES_PER_SEC];
	m_iVol = 31;

	m_iLastUpdatePos = 0;
	m_iStartUpdatePos = 0;
	m_dwLastAppleClock = g_pBoard->GetClock();

}

C8913::~C8913()
{
	int i;
	for( i = 0; i < 3; i++ )
		delete m_lpwBuf[i];
	delete m_lpwBuf;
}

// bit 0 : BC1
// bit 1 : BDIR
// 00	INACTIVE
// 01	READ FROM PSG
// 10	WRITE TO PSG
// 11	LATCH ADDRESS
void C8913::SetMode(BYTE mode)
{
	m_byMode = mode & 0x03;
//	SetData( m_byData );
}

void C8913::SetData(BYTE data)
{
	m_byData = data;
	switch (m_byMode)
	{
	case 0:		// INACTIVE
	case 1:		// READ FROM PSG
		break;
	case 2:		// WRITE TO PSG
		WriteReg(m_byAddr, data);
		break;
	case 3:		// LATCH ADDRESS
		m_byAddr = m_byData;
		break;
	}
}
void C8913::WriteReg(BYTE reg, BYTE data)
{
	int iOldPeriod;

	if (reg >= 0x10)
	{
		return;
	}
	if (reg == AY_ENVELOPE_SHAPE || m_abyRegs[reg] != data)
	{
		/* update the output buffer before changing the register */
		this->UpdateStream();
	}
	m_abyRegs[reg] = m_byData;
	switch (reg)
	{
	case AY_A_TONE_FINE:
	case AY_A_TONE_COARSE:
		m_abyRegs[AY_A_TONE_COARSE] &= 0x0F;
		iOldPeriod = m_iPeriodA;
		m_iPeriodA = ((m_abyRegs[AY_A_TONE_COARSE] << 8) | m_abyRegs[AY_A_TONE_FINE]) * m_iUpdateStep;
		if (m_iPeriodA == 0)
			m_iPeriodA = m_iUpdateStep;
		m_iCountA += m_iPeriodA - iOldPeriod;
		if (m_iCountA <= 0)
			m_iCountA = 1;
		break;
	case AY_B_TONE_FINE:
	case AY_B_TONE_COARSE:
		m_abyRegs[AY_B_TONE_COARSE] &= 0x0F;
		iOldPeriod = m_iPeriodB;
		m_iPeriodB = ((m_abyRegs[AY_B_TONE_COARSE] << 8) | m_abyRegs[AY_B_TONE_FINE]) * m_iUpdateStep;
		if (m_iPeriodB == 0)
			m_iPeriodB = m_iUpdateStep;
		m_iCountB += m_iPeriodB - iOldPeriod;
		if (m_iCountB <= 0)
			m_iCountB = 1;
		break;
	case AY_C_TONE_FINE:
	case AY_C_TONE_COARSE:
		m_abyRegs[AY_C_TONE_COARSE] &= 0x0F;
		iOldPeriod = m_iPeriodC;
		m_iPeriodC = ((m_abyRegs[AY_C_TONE_COARSE] << 8) | m_abyRegs[AY_C_TONE_FINE]) * m_iUpdateStep;
		if (m_iPeriodC == 0)
			m_iPeriodC = m_iUpdateStep;
		m_iCountC += m_iPeriodC - iOldPeriod;
		if (m_iCountC <= 0)
			m_iCountC = 1;
		break;
	case AY_NOISE:
		m_abyRegs[AY_NOISE] &= 0x1f;
		iOldPeriod = m_iPeriodN;
		m_iPeriodN = m_abyRegs[AY_NOISE] * m_iUpdateStep;
		if (m_iPeriodN == 0)
			m_iPeriodN = 0x20 * m_iUpdateStep;
		//				m_iPeriodN = m_iUpdateStep;
		m_iCountN += m_iPeriodN - iOldPeriod;
		if (m_iCountN <= 0)
			m_iCountN = 1;
		break;
	case AY_ENABLE:
		break;
	case AY_A_AMPLITUDE:
		m_abyRegs[AY_A_AMPLITUDE] &= 0x1F;
		m_byEnvelopeA = m_abyRegs[AY_A_AMPLITUDE] & 0x10;
		m_wVolA = m_byEnvelopeA ? m_wVolE : m_awVolume[m_abyRegs[AY_A_AMPLITUDE] ? m_abyRegs[AY_A_AMPLITUDE] * 2 + 1 : 0];
		break;
	case AY_B_AMPLITUDE:
		m_abyRegs[AY_B_AMPLITUDE] &= 0x1F;
		m_byEnvelopeB = m_abyRegs[AY_B_AMPLITUDE] & 0x10;
		m_wVolB = m_byEnvelopeB ? m_wVolE : m_awVolume[m_abyRegs[AY_B_AMPLITUDE] ? m_abyRegs[AY_B_AMPLITUDE] * 2 + 1 : 0];
		break;
	case AY_C_AMPLITUDE:
		m_abyRegs[AY_C_AMPLITUDE] &= 0x1F;
		m_byEnvelopeC = m_abyRegs[AY_C_AMPLITUDE] & 0x10;
		m_wVolC = m_byEnvelopeC ? m_wVolE : m_awVolume[m_abyRegs[AY_C_AMPLITUDE] ? m_abyRegs[AY_C_AMPLITUDE] * 2 + 1 : 0];
		break;
	case AY_ENVELOPE_FINE:
	case AY_ENVELOPE_COARSE:
		iOldPeriod = m_iPeriodE;
		m_iPeriodE = ((m_abyRegs[AY_ENVELOPE_FINE] + 256 * m_abyRegs[AY_ENVELOPE_COARSE])) * m_iUpdateStep;
		if (m_iPeriodE == 0) m_iPeriodE = m_iUpdateStep / 2;
		m_iCountE += m_iPeriodE - iOldPeriod;
		if (m_iCountE <= 0) m_iCountE = 1;
		break;

	case AY_ENVELOPE_SHAPE:
		/* envelope shapes:
		C AtAlH
		0 0 x x  \___

		0 1	x x  /___

		1 0 0 0  \\\\

		1 0 0 1  \___

		1 0 1 0  \/\/
		___
		1 0 1 1  \

		1 1 0 0  ////
		___
		1 1 0 1  /

		1 1 1 0  /\/\

		1 1 1 1  /___

		The envelope counter on the AY-3-8910 has 16 steps. On the YM2149 it
		has twice the steps, happening twice as fast. Since the end result is
		just a smoother curve, we always use the YM2149 behaviour.
		*/
		m_abyRegs[AY_ENVELOPE_SHAPE] &= 0x0f;
		m_byAttack = (m_abyRegs[AY_ENVELOPE_SHAPE] & 0x04) ? 0x1f : 0x00;
		if ((m_abyRegs[AY_ENVELOPE_SHAPE] & 0x08) == 0)
		{
			/* if Continue = 0, map the shape to the equivalent one which has Continue = 1 */
			m_byHold = 1;
			m_byAlternate = m_byAttack;
		}
		else
		{
			m_byHold = m_abyRegs[AY_ENVELOPE_SHAPE] & 0x01;
			m_byAlternate = m_abyRegs[AY_ENVELOPE_SHAPE] & 0x02;
		}
		m_iCountE = m_iPeriodE;
		m_chCountEnv = 0x1f;
		m_bHolding = FALSE;
		m_wVolE = m_awVolume[m_chCountEnv ^ m_byAttack];
		if (m_byEnvelopeA) m_wVolA = m_wVolE;
		if (m_byEnvelopeB) m_wVolB = m_wVolE;
		if (m_byEnvelopeC) m_wVolC = m_wVolE;
		break;
	}
}

BYTE C8913::ReadData()
{
	if ( m_byMode == 1 && m_byAddr < 0x10 )	// read from PSG
		return( m_abyRegs[m_byAddr] );
	else
		return 0;
}



void C8913::Reset()
{
	int i;
	for (i = 0; i < 16; i++)
		m_abyRegs[i] = 0;
	m_abyRegs[AY_ENABLE] = 0x38;

	m_byAddr = 0;
	m_byData = 0;
	m_byMode = 0;
	m_byNoise = 0xFF;
	m_dwRNG = 1;
	m_iPeriodA = m_iUpdateStep;
	m_iPeriodB = m_iUpdateStep;
	m_iPeriodC = m_iUpdateStep;
	m_iPeriodN = m_iUpdateStep * 20;
	m_iPeriodE = m_iUpdateStep / 2;
	m_iCountA = 1;
	m_iCountB = 1;
	m_iCountC = 1;
	m_iCountN = 1;
	m_iCountE = 1;
	m_chCountEnv = 0;
	m_byOutputA = 0;
	m_byOutputB = 0;
	m_byOutputC = 0;
	m_byOutputN = 0;
}


void C8913::Clock(WORD clock)
{

}

void C8913::UpdateBuffer(int length)
{
	long retval = 0;
	int len = length;
	
	int outn;
	WORD *bufa, *bufb, *bufc;

	if (m_iStartUpdatePos > 0)
	{
		// move data to start of the buffer
		memmove_s(m_lpwBuf[0], SAMPLES_PER_SEC*sizeof(WORD), m_lpwBuf[0] + m_iStartUpdatePos, m_iLastUpdatePos - m_iStartUpdatePos);
		memmove_s(m_lpwBuf[1], SAMPLES_PER_SEC*sizeof(WORD), m_lpwBuf[1] + m_iStartUpdatePos, m_iLastUpdatePos - m_iStartUpdatePos);
		memmove_s(m_lpwBuf[2], SAMPLES_PER_SEC*sizeof(WORD), m_lpwBuf[2] + m_iStartUpdatePos, m_iLastUpdatePos - m_iStartUpdatePos);
		m_iLastUpdatePos -= m_iStartUpdatePos;
		m_iStartUpdatePos = 0;
		m_dwLastAppleClock = g_pBoard->GetClock();
	}

	if (length <= m_iLastUpdatePos)
	{
		// already enough data in buffer
		return;
	}

	bufa = m_lpwBuf[0] + m_iLastUpdatePos;
	bufb = m_lpwBuf[1] + m_iLastUpdatePos;
	bufc = m_lpwBuf[2] + m_iLastUpdatePos;
	
	length -= m_iLastUpdatePos;
	m_iLastUpdatePos += length;

	/* The 8910 has three outputs, each output is the mix of one of the three */
	/* tone generators and of the (single) noise generator. The two are mixed */
	/* BEFORE going into the DAC. The formula to mix each channel is: */
	/* (ToneOn | ToneDisable) & (NoiseOn | NoiseDisable). */
	/* Note that this means that if both tone and noise are disabled, the output */
	/* is 1, not 0, and can be modulated changing the volume. */
	
	
	/* If the channels are disabled, set their output to 1, and increase the */
	/* counter, if necessary, so they will not be inverted during this update. */
	/* Setting the output to 1 is necessary because a disabled channel is locked */
	/* into the ON state (see above); and it has no effect if the volume is 0. */
	/* If the volume is 0, increase the counter, but don't touch the output. */
	if ( m_abyRegs[AY_ENABLE] & 0x01 )
	{
		if ( m_iCountA <= length*STEP )
			m_iCountA += length*STEP;
		m_byOutputA = 1;
	}
	else if ( m_abyRegs[AY_A_AMPLITUDE] == 0 )
	{
		/* note that I do count += length, NOT count = length + 1. You might think */
		/* it's the same since the volume is 0, but doing the latter could cause */
		/* interferencies when the program is rapidly modulating the volume. */
		if ( m_iCountA <= length*STEP )
			m_iCountA += length*STEP;
	}
	if ( m_abyRegs[AY_ENABLE] & 0x02 )
	{
		if ( m_iCountB <= length*STEP)
			m_iCountB += length*STEP;
		m_byOutputB = 1;
	}
	else if ( m_abyRegs[AY_B_AMPLITUDE] == 0 )
	{
		if ( m_iCountB <= length*STEP )
			m_iCountB += length*STEP;
	}
	if ( m_abyRegs[AY_ENABLE] & 0x04 )
	{
		if ( m_iCountC <= length*STEP)
			m_iCountC += length*STEP;
		m_byOutputC = 1;
	}
	else if ( m_abyRegs[AY_C_AMPLITUDE] == 0 )
	{
		if ( m_iCountC <= length*STEP )
			m_iCountC += length*STEP;
	}
	
	/* for the noise channel we must not touch OutputN - it's also not necessary */
	/* since we use outn. */
	if ( ( m_abyRegs[AY_ENABLE] & 0x38 ) == 0x38 )	/* all off */
	{
		if ( m_iCountN <= length*STEP )
			m_iCountN += length*STEP;
	}
	
	outn = ( m_byOutputN | m_abyRegs[AY_ENABLE] );
	
	
	/* buffering loop */
	while (length)
	{
		int vola, volb, volc;
		int left;
		
		
		/* vola, volb and volc keep track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vola = volb = volc = 0;
		
		left = STEP;
		do
		{
			int nextevent;
			
			
			if ( m_iCountN < left )
				nextevent = m_iCountN;
			else
				nextevent = left;
			
			if ( outn & 0x08 )
			{
				if ( m_byOutputA )
					vola += m_iCountA;
				m_iCountA -= nextevent;
				/* PeriodA is the half period of the square wave. Here, in each */
				/* loop I add PeriodA twice, so that at the end of the loop the */
				/* square wave is in the same status (0 or 1) it was at the start. */
				/* vola is also incremented by PeriodA, since the wave has been 1 */
				/* exactly half of the time, regardless of the initial position. */
				/* If we exit the loop in the middle, OutputA has to be inverted */
				/* and vola incremented only if the exit status of the square */
				/* wave is 1. */
				while ( m_iCountA <= 0 )
				{
					m_iCountA += m_iPeriodA;
					if ( m_iCountA > 0 )
					{
						m_byOutputA ^= 1;
						if ( m_byOutputA )
							vola += m_iPeriodA;
						break;
					}
					m_iCountA += m_iPeriodA;
					vola += m_iPeriodA;
				}
				if ( m_byOutputA )
					vola -= m_iCountA;
			}
			else
			{
				m_iCountA -= nextevent;
				while ( m_iCountA <= 0 )
				{
					m_iCountA += m_iPeriodA;
					if ( m_iCountA > 0 )
					{
						m_byOutputA ^= 1;
						break;
					}
					m_iCountA += m_iPeriodA;
				}
			}

			if ( outn & 0x10 )
			{
				if ( m_byOutputB )
					volb += m_iCountB;
				m_iCountB -= nextevent;
				while ( m_iCountB <= 0 )
				{
					m_iCountB += m_iPeriodB;
					if ( m_iCountB > 0 )
					{
						m_byOutputB ^= 1;
						if ( m_byOutputB )
							volb += m_iPeriodB;
						break;
					}
					m_iCountB += m_iPeriodB;
					volb += m_iPeriodB;
				}
				if ( m_byOutputB )
					volb -= m_iCountB;
			}
			else
			{
				m_iCountB -= nextevent;
				while ( m_iCountB <= 0 )
				{
					m_iCountB += m_iPeriodB;
					if ( m_iCountB > 0 )
					{
						m_byOutputB ^= 1;
						break;
					}
					m_iCountB += m_iPeriodB;
				}
			}

			if ( outn & 0x20 )
			{
				if ( m_byOutputC )
					volc += m_iCountC;
				m_iCountC -= nextevent;
				while ( m_iCountC <= 0 )
				{
					m_iCountC += m_iPeriodC;
					if ( m_iCountC > 0 )
					{
						m_byOutputC ^= 1;
						if ( m_byOutputC )
							volc += m_iPeriodC;
						break;
					}
					m_iCountC += m_iPeriodC;
					volc += m_iPeriodC;
				}
				if ( m_byOutputC )
					volc -= m_iCountC;
			}
			else
			{
				m_iCountC -= nextevent;
				while ( m_iCountC <= 0 )
				{
					m_iCountC += m_iPeriodC;
					if ( m_iCountC > 0 )
					{
						m_byOutputC ^= 1;
						break;
					}
					m_iCountC += m_iPeriodC;
				}
			}

			m_iCountN -= nextevent;
			if ( m_iCountN <= 0 )
			{
				/* Is noise output going to change? */
				if ( ( m_dwRNG + 1 ) & 2 )	/* (bit0^bit1)? */
				{
					m_byOutputN = ~m_byOutputN;
					outn = ( m_byOutputN | m_abyRegs[AY_ENABLE] );
				}
				
				/* The Random Number Generator of the 8910 is a 17-bit shift */
				/* register. The input to the shift register is bit0 XOR bit2 */
				/* (bit0 is the output). */
				
				/* The following is a fast way to compute bit17 = bit0^bit2. */
				/* Instead of doing all the logic operations, we only check */
				/* bit0, relying on the fact that after two shifts of the */
				/* register, what now is bit2 will become bit0, and will */
				/* invert, if necessary, bit15, which previously was bit17. */
				if ( m_dwRNG & 1 )
					m_dwRNG ^= 0x28000;
				m_dwRNG >>= 1;
				m_iCountN += m_iPeriodN;
			}
			
			left -= nextevent;
		} while (left > 0);

		/* update envelope */
		if ( m_bHolding == FALSE )
		{
			m_iCountE -= STEP;
			if ( m_iCountE <= 0 )
			{
				do
				{
					m_chCountEnv--;
					m_iCountE += m_iPeriodE;
				} while (m_iCountE <= 0);

				/* check envelope current position */
				if ( m_chCountEnv < 0 )
				{
					if ( m_byHold )
					{
						if ( m_byAlternate )
							m_byAttack ^= 0x1f;
						m_bHolding = TRUE;
						m_chCountEnv = 0;
					}
					else
					{
						/* if CountEnv has looped an odd number of times (usually 1), */
						/* invert the output. */
						if ( m_byAlternate && ( m_chCountEnv & 0x20 ) )
 							m_byAttack ^= 0x1f;

						m_chCountEnv &= 0x1f;
					}
				}

				m_wVolE = m_awVolume[m_chCountEnv ^ m_byAttack];
				/* reload volume */
				if ( m_byEnvelopeA ) m_wVolA = m_wVolE;
				if ( m_byEnvelopeB ) m_wVolB = m_wVolE;
				if ( m_byEnvelopeC ) m_wVolC = m_wVolE;
			}
		}

		*(bufa++) = ( vola * m_wVolA ) / STEP;
		*(bufb++) = ( volb * m_wVolB ) / STEP;
		*(bufc++) = ( volc * m_wVolC ) / STEP;
		length--;
	}
}


void C8913::UpdateStream()
{
	unsigned int currentStep = (g_pBoard->GetClock() - m_dwLastAppleClock) * m_iUpdateStep / 8;
	unsigned int currentPos = currentStep / STEP;

	if (currentPos > SAMPLES_PER_SEC )
	{
		currentPos = SAMPLES_PER_SEC;
	}

	this->UpdateBuffer(currentPos);

}

void C8913::Update(int length)
{
	int currentPos = length;

	if (currentPos > SAMPLES_PER_SEC)
	{
		currentPos = SAMPLES_PER_SEC;
	}

	this->UpdateBuffer(currentPos);

	m_iStartUpdatePos = currentPos;
	m_dwLastAppleClock = g_pBoard->GetClock();
}

void C8913::ChangeSampleRate()
{
	m_iUpdateStep = (int)( ( (double)STEP * g_DXSound.m_nSampleRate * 8 + m_dwClock / 2 ) / m_dwClock );

	// period A
	m_iPeriodA = ( ( m_abyRegs[AY_A_TONE_COARSE] << 8 ) | m_abyRegs[AY_A_TONE_FINE] ) * m_iUpdateStep;
	if ( m_iPeriodA == 0 )
		m_iPeriodA = m_iUpdateStep;
	// period B
	m_iPeriodB = ( ( m_abyRegs[AY_B_TONE_COARSE] << 8 ) | m_abyRegs[AY_B_TONE_FINE] ) * m_iUpdateStep;
	if ( m_iPeriodB == 0 )
		m_iPeriodB = m_iUpdateStep;
	// period C
	m_iPeriodC = ( ( m_abyRegs[AY_C_TONE_COARSE] << 8 ) | m_abyRegs[AY_C_TONE_FINE] ) * m_iUpdateStep;
	if ( m_iPeriodC == 0 )
		m_iPeriodC = m_iUpdateStep;
	// Noise
	m_iPeriodN = m_abyRegs[AY_NOISE] * m_iUpdateStep;
	if ( m_iPeriodN == 0)
		m_iPeriodN = 0x20 * m_iUpdateStep;
	// Envelope
	m_iPeriodE = ( ( m_abyRegs[AY_ENVELOPE_FINE] + 256 * m_abyRegs[AY_ENVELOPE_COARSE] ) ) * m_iUpdateStep;
	if ( m_iPeriodE == 0 )
		m_iPeriodE = m_iUpdateStep / 2;

	m_iCountA = 1;
	m_iCountB = 1;
	m_iCountC = 1;
	m_iCountN = 1;
	m_iCountE = 1;

	m_iLastUpdatePos = 0;
	m_iStartUpdatePos = 0;
	m_dwLastAppleClock = g_pBoard->GetClock();
}

void C8913::SetClockSpeed(DWORD clock)
{
	m_dwClock = clock;
	ChangeSampleRate();
}


void C8913::Serialize( CArchive &ar )
{
	CPSG::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_dwClock;
		ar << m_bHolding;
		ar << m_byMode;
		ar << m_byAddr;
		ar << m_byData;
		ar.Write( m_abyRegs, sizeof(m_abyRegs) );
		ar << m_byNoise;
		
		ar << m_byOutputA;
		ar << m_byOutputB;
		ar << m_byOutputC;
		ar << m_byOutputN;
		
		ar << m_byEnvelopeA;
		ar << m_byEnvelopeB;
		ar << m_byEnvelopeC;
		
		ar << m_wVolA;
		ar << m_wVolB;
		ar << m_wVolC;
		ar << m_wVolE;
		
		ar << m_dwRNG;
		
		ar << m_chCountEnv;
		
		ar << m_byHold;
		ar << m_byAlternate;
		ar << m_byAttack;
	}
	else
	{
		ar >> m_dwClock;
		ar >> m_bHolding;
		ar >> m_byMode;
		ar >> m_byAddr;
		ar >> m_byData;
		ar.Read( m_abyRegs, sizeof(m_abyRegs) );
		ar >> m_byNoise;
		
		ar >> m_byOutputA;
		ar >> m_byOutputB;
		ar >> m_byOutputC;
		ar >> m_byOutputN;
		
		ar >> m_byEnvelopeA;
		ar >> m_byEnvelopeB;
		ar >> m_byEnvelopeC;
		
		ar >> m_wVolA;
		ar >> m_wVolB;
		ar >> m_wVolC;
		ar >> m_wVolE;
		
		ar >> m_dwRNG;
		
		ar >> m_chCountEnv;
		
		ar >> m_byHold;
		ar >> m_byAlternate;
		ar >> m_byAttack;
		ChangeSampleRate();
	}
}
