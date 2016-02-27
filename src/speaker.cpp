// Speaker.cpp: implementation of the CSpeaker class.
//
//////////////////////////////////////////////////////////////////////
#include "arch/frame/stdafx.h"
#include "arch/directx/dxsound.h"
#include "speaker.h"
#include "appleclock.h"
#include "aipcdefs.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern WORD g_tblVolume[32];
#ifndef STEP
#define STEP	0x8000
#endif

CSpeaker::CSpeaker()
{
	int clock;
	m_nNumOfBuf = 1;
	m_lpwBuf = new LPWORD[1];
	m_lpwBuf[0] = new WORD[SAMPLES_PER_SEC * BITS_PER_SAMPLE / 8];
	clock = CLOCK;		// 1MHz
	m_iUpdateStep = (int)( ( (double)STEP * g_DXSound.m_nSampleRate + clock / 2 ) / clock );
	m_dwLastAppleClock = 0;
	m_iLastUpdatePos = -1;
	m_bToggle = FALSE;
	m_iVol = 28;
}

CSpeaker::~CSpeaker()
{
	delete m_lpwBuf[0];
	delete m_lpwBuf;
}

void CSpeaker::Update(int length)
{
//	int currentStep = ( g_pBoard->GetClock() - m_dwLastAppleClock ) * m_iUpdateStep;
	int currentPos = length - 1;
	int i;
	if ( currentPos >= (signed)g_DXSound.m_nSampleRate * BITS_PER_SAMPLE / 8 )
		currentPos = g_DXSound.m_nSampleRate * BITS_PER_SAMPLE / 8 - 1;

	if ( m_iLastUpdatePos == -1 && m_bToggle )
	{
		m_bToggle = FALSE;
		for ( i = 0; i <= currentPos; i++ )
			m_lpwBuf[0][i] = MAX_AMPLITUDE - MAX_AMPLITUDE * i / currentPos;
	}
	else
	{
		for( i = m_iLastUpdatePos+1; i <= currentPos; i++ )
			m_lpwBuf[0][i] = ( m_bToggle ) ? MAX_AMPLITUDE : 0;
	}

//	m_bToggle = FALSE;

	m_iLastUpdatePos = -1;
	m_dwLastAppleClock = g_pBoard->GetClock();
}

void CSpeaker::Toggle()
{
	unsigned int currentStep = ( g_pBoard->GetClock() - m_dwLastAppleClock ) * m_iUpdateStep;
	unsigned int currentPos = currentStep / STEP;
	int left = STEP - currentStep + ( currentPos * STEP );
	unsigned int i;

	if ( currentPos >= SAMPLES_PER_SEC * BITS_PER_SAMPLE / 8 )
	{
		currentPos = SAMPLES_PER_SEC * BITS_PER_SAMPLE / 8 - 1;
		left = 0;
	}

	for( i = m_iLastUpdatePos+1; i <= currentPos; i++ )
		m_lpwBuf[0][i] = ( m_bToggle ) ? MAX_AMPLITUDE : 0;


	m_iLastUpdatePos = currentPos;
	m_bToggle = !m_bToggle;
	m_lpwBuf[0][currentPos] += ( m_bToggle ) ? MAX_AMPLITUDE * left / STEP : -MAX_AMPLITUDE * left / STEP;
}

void CSpeaker::ChangeSampleRate()
{
	int clock;
	clock = CLOCK;		// 1MHz
	m_iUpdateStep = (int)( ( (double)STEP * g_DXSound.m_nSampleRate + clock / 2 ) / clock );
	m_dwLastAppleClock = g_pBoard->GetClock();
	m_iLastUpdatePos = -1;
	m_bToggle = FALSE;
}
