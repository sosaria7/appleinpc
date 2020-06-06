// Cpu.cpp: implementation of the CCpu class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "cpu.h"
#include "appleclock.h"

extern CAppleClock *g_pBoard;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCpu::CCpu()
{
	m_uException_Register = 0;
	PendingIRQ = 0;
	m_clockListener = NULL;
}

CCpu::~CCpu()
{

}

void CCpu::Assert_NMI()
{
	m_uException_Register |= SIG_CPU_NMI;
}

void CCpu::Assert_IRQ()
{
	m_uException_Register |= SIG_CPU_IRQ;
}

void CCpu::Reset()
{
	m_uException_Register |= SIG_CPU_RES;
}

void CCpu::Shutdown()
{
	m_uException_Register |= SIG_CPU_SHUTDOWN;
}

void CCpu::Wait()
{
	m_uException_Register |= SIG_CPU_WAIT;
}

void CCpu::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );
	int dummy = 0;

	if ( ar.IsStoring() )
	{
		ar << m_uException_Register;
		ar << dummy;
		ar << dummy;
		ar << PendingIRQ;
	}
	else
	{
		ar >> m_uException_Register;
		ar >> dummy;
		ar >> dummy;
		ar >> PendingIRQ;
	}
}
