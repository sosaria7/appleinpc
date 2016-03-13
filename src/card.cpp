// Card.cpp: implementation of the CCard class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "card.h"
#include "memory.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC( CCard, CObject );

CCard::CCard()
{
	InitRomImage();
	m_iDeviceNum = CARD_EMPTY;
	m_nDipSwitch = 0;
	m_pbyRom = NULL;
	m_bHasExtendRom = FALSE;
}

CCard::~CCard()
{

}

BYTE CCard::ReadRom(WORD addr)
{
	if ( m_pbyRom )
		return m_pbyRom[addr&0xFF];
	return 0;
}

void CCard::WriteRom(WORD addr, BYTE data)
{
}

BYTE CCard::ReadExRom(WORD addr)
{
	return MemReturnRandomData(2);
}

void CCard::WriteExRom(WORD addr, BYTE data)
{
}

void CCard::InitRomImage()
{
}


BYTE CCard::Read(WORD addr)
{
	return 0x00;
}

void CCard::Write(WORD addr, BYTE data)
{

}

void CCard::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_strDeviceName;
		ar << m_iDeviceNum;
		ar << m_nDipSwitch;
	}
	else
	{
		ar >> m_strDeviceName;
		ar >> m_iDeviceNum;
		ar >> m_nDipSwitch;
	}
}
