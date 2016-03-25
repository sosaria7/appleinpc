// PSG.cpp: implementation of the CPSG class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "psg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define MAX_AMPLITUDE		0x7FFF;
CPSG::CPSG()
{
	m_bMute = FALSE;
	m_iVol = 32;
	m_lpwBuf = NULL;
	m_nNumOfBuf = 0;

	int i;
	double out;
	out = MAX_AMPLITUDE;
	// reduce amplitude by 1.5 db
	for (i = 31; i > 0; i--)
	{
		m_awVolume[i] = (WORD)(out + 0.5);	/* round to nearest */

		out /= 1.148153621;	/* = 10 ^ (.6/10) = 0.6dB */
	}
	m_awVolume[0] = 0;
}

CPSG::~CPSG()
{

}

void CPSG::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_bMute;
		ar << m_iVol;
	}
	else
	{
		ar >> m_bMute;
		ar >> m_iVol;
	}
}
