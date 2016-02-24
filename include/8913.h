// 8913.h: interface for the C8913 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_8913_H__223C9736_A480_466D_89E3_44C58ECE0FFB__INCLUDED_)
#define AFX_8913_H__223C9736_A480_466D_89E3_44C58ECE0FFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "arch/directx/dxsound.h"
#include "psg.h"

#define	AY_A_TONE_FINE		0x00
#define AY_A_TONE_COARSE	0x01
#define	AY_B_TONE_FINE		0x02
#define AY_B_TONE_COARSE	0x03
#define	AY_C_TONE_FINE		0x04
#define AY_C_TONE_COARSE	0x05
#define AY_NOISE			0x06
#define AY_ENABLE			0x07
#define AY_A_AMPLITUDE		0x08
#define AY_B_AMPLITUDE		0x09
#define AY_C_AMPLITUDE		0x0A
#define AY_ENVELOPE_FINE	0x0B
#define AY_ENVELOPE_COARSE	0x0C
#define AY_ENVELOPE_SHAPE	0x0D
#define AY_PORT_A			0x0E
#define AY_PORT_B			0x0F

class C8913 : public CPSG
{
public:
	C8913();
	virtual ~C8913();
	void SetClockSpeed( DWORD clock );
	void ChangeSampleRate();
	void Update(int length);
	void Clock( WORD clock );
	void Reset();
	BYTE ReadData();
	void SetData(BYTE data);
	void SetMode(BYTE mode);
	void Serialize( CArchive &ar );

protected:
	DWORD m_dwClock;
	BOOL m_bHolding;
	BYTE m_byMode;
	BYTE m_byAddr;
	BYTE m_byData;
	BYTE m_abyRegs[16];
	BYTE m_byNoise;

	BYTE m_byOutputA;
	BYTE m_byOutputB;
	BYTE m_byOutputC;
	BYTE m_byOutputN;

	BYTE m_byEnvelopeA;
	BYTE m_byEnvelopeB;
	BYTE m_byEnvelopeC;

	WORD m_wVolA;
	WORD m_wVolB;
	WORD m_wVolC;
	WORD m_wVolE;

	DWORD m_dwRNG;

	int		m_iUpdateStep;
	int		m_iPeriodA;
	int		m_iPeriodB;
	int		m_iPeriodC;
	int		m_iPeriodN;
	int		m_iPeriodE;

	int		m_iCountA;
	int		m_iCountB;
	int		m_iCountC;
	int		m_iCountN;
	int		m_iCountE;

	char	m_chCountEnv;

	BYTE	m_byHold;
	BYTE	m_byAlternate;
	BYTE	m_byAttack;

private:
	int buffpos;
};

#endif // !defined(AFX_8913_H__223C9736_A480_466D_89E3_44C58ECE0FFB__INCLUDED_)
