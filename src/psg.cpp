// PSG.cpp: implementation of the CPSG class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "psg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPSG::CPSG()
{
	m_bMute = FALSE;
	m_iVol = 32;
	m_lpwBuf = NULL;
	m_nNumOfBuf = 0;
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
