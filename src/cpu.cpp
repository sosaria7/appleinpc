// Cpu.cpp: implementation of the CCpu class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "cpu.h"
#include "appleclock.h"

extern DWORD g_localClock;
extern CAppleClock *g_pBoard;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCpu::CCpu()
{
	m_nSpeed = 1;
	m_uException_Register = 0;
	m_nRemain = 0;
	PendingIRQ = 0;
	sClockFunc.func = NULL;
}

CCpu::~CCpu()
{

}
void CCpu::SetClockListener( void* objTo, callback_handler func )
{
	sClockFunc.objTo = objTo;
	sClockFunc.func = func;
}

void CCpu::Clock(int nClock)
{
	int nDelta;
	m_nRemain += nClock * m_nSpeed;

	while( m_nRemain > 0 )
	{
		nDelta = Process();
		if ( nDelta <= 0 )
		{
			m_nRemain = 0;
			break;
		}
		m_nRemain -= nDelta;
		if ( sClockFunc.func )
			sClockFunc.func( this, sClockFunc.objTo, (LPARAM)nDelta );
	}
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
	m_nRemain = 0;
}

void CCpu::Shutdown()
{
	m_uException_Register |= SIG_CPU_SHUTDOWN;
}

void CCpu::Wait()
{
	m_uException_Register |= SIG_CPU_WAIT;
}

DWORD CCpu::GetClock()
{
	return ( g_localClock - m_nRemain / m_nSpeed );
}

void CCpu::SetSpeed(int nSpeed)
{

	if( nSpeed <= 0 )
		nSpeed = 1;
	m_nRemain = m_nRemain * nSpeed / m_nSpeed;
	m_nSpeed = nSpeed;
}

DWORD CCpu::GetCpuClock()
{
	return ( g_localClock * m_nSpeed - m_nRemain );
}

void CCpu::Serialize( CArchive &ar )
{
	CObject::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_uException_Register;
		ar << m_nRemain;
		ar << m_nSpeed;
		ar << PendingIRQ;
	}
	else
	{
		ar >> m_uException_Register;
		ar >> m_nRemain;
		ar >> m_nSpeed;
		ar >> PendingIRQ;
	}
}
