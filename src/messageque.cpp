// MessageQue.cpp: implementation of the CMessageQue class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "messageque.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageQue::CMessageQue()
{
	m_byteHead=0;
	m_byteTail=0;
}

CMessageQue::~CMessageQue()
{

}

void CMessageQue::AddMessage(int message)
{
	if(m_byteTail==(BYTE)(m_byteHead-1))
		return;
	m_anQueue[m_byteTail++] = message;
}

int CMessageQue::GetMesg()
{
	if(m_byteTail==m_byteHead)
		return 0;
	return m_anQueue[m_byteHead++];
}

void CMessageQue::ClearQueue()
{
	m_byteHead = m_byteTail;
}
