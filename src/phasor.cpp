// Phasor.cpp: implementation of the CPhasor class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/dlgconfigmockingboard.h"
#include "arch/directx/dxsound.h"
#include "phasor.h"
#include "appleclock.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC( CPhasor, CCard );

CPhasor::CPhasor()
{
	m_strDeviceName = "Phasor";
	m_iDeviceNum = CARD_PHASOR;
	g_DXSound.AddPSG( &m_8913[0], 1 );
	g_DXSound.AddPSG( &m_8913[1], 1 );
	g_DXSound.AddPSG( &m_8913[2], -1 );
	g_DXSound.AddPSG( &m_8913[3], -1 );
#ifdef HAVE_VOTRAX			// not implemented yet
	g_DXSound.AddPSG( &m_cVotrax[0], 1 );
	g_DXSound.AddPSG( &m_cVotrax[1], -1 );
#endif
	m_byMode = 0;			// Native Phasor Mode
}

CPhasor::~CPhasor()
{
	g_DXSound.RemovePSG( &m_8913[0] );
	g_DXSound.RemovePSG( &m_8913[1] );
	g_DXSound.RemovePSG( &m_8913[2] );
	g_DXSound.RemovePSG( &m_8913[3] );
#ifdef HAVE_VOTRAX
	g_DXSound.RemovePSG( &m_cVotrax[0] );
	g_DXSound.RemovePSG( &m_cVotrax[1] );
#endif
}

void CPhasor::WriteRom(WORD addr, BYTE data)
{
	BYTE orb;
	BYTE mode;
	BYTE cs, cs1;

	if ( addr & 0x40 )
	{
#ifdef HAVE_VOTRAX
		cs = ( addr >> 5 ) & 1;				// I guess...
		m_cVotrax[cs].Write( addr & 0x03, data );
		m_6522[cs].SetCA1( m_cVotrax[0].m_bBusy );
#endif
		return;
	}

	if ( m_byMode & 0x1 )
		cs = ( ( addr & 0x80 ) >> 6 ) | ( ( addr & 0x10 ) >> 4 );
	else									// MockingBoard Mode
		cs = ( ( addr >> 7 ) & 1 ) + 1;		// 1 or 2

	if ( cs & 1 )
	{
		orb = m_6522[0].GetORB();
		m_6522[0].Write( addr & 0x0F, data );
		mode = m_6522[0].GetORB();
		if ( m_byMode & 1 )					// Native Phasor Mode
			cs1 = ~( mode >> 3 ) & 3;
		else
			cs1 = 1;
		if ( mode != orb )
		{
			if ( !( mode & 0x04 ) )
			{
				if ( cs1 & 1 )
					m_8913[0].Reset();
				if ( cs1 & 2 )
					m_8913[1].Reset();
			}
			else
			{
				if ( cs1 & 1 )
					m_8913[0].SetMode( mode & 0x03 );
				if ( cs1 & 2 )
					m_8913[1].SetMode( mode & 0x03 );
			}
		}
		if ( cs1 & 1 )
			m_8913[0].SetData( m_6522[0].GetORA() );
		if ( cs1 & 2 )
			m_8913[1].SetData( m_6522[0].GetORA() );
	}
	if ( cs & 2 )
	{
		orb = m_6522[1].GetORB();
		m_6522[1].Write( addr & 0x0F, data );
		mode = m_6522[1].GetORB();
		if ( m_byMode & 0x01 )		// Native Phasor Mode
			cs1 = ~( mode >> 3 ) & 3;
		else
			cs1 = 1;
		if ( mode != orb )
		{
			if ( !( mode & 0x04 ) )
			{
				if ( cs1 & 1 )
					m_8913[2].Reset();
				if ( cs1 & 2 )
					m_8913[3].Reset();
			}
			else
			{
				if ( cs1 & 1 )
					m_8913[2].SetMode( mode & 0x03 );
				if ( cs1 & 2 )
					m_8913[3].SetMode( mode & 0x03 );
			}
		}
		if ( cs1 & 1 )
			m_8913[2].SetData( m_6522[1].GetORA() );
		if ( cs1 & 2 )
			m_8913[3].SetData( m_6522[1].GetORA() );
	}
}

BYTE CPhasor::ReadRom(WORD addr)
{
	BYTE retval = 0;
//	BYTE cs = ( ( addr & 0x80 ) >> 6 ) | ( ( addr & 0x10 ) >> 4 );
	BYTE cs;

	if ( m_byMode & 0x01 )
		cs = ( ( addr & 0x80 ) >> 6 ) | ( ( addr & 0x10 ) >> 4 );
	else									// MockingBoard Mode
		cs = ( ( addr >> 7 ) & 1 ) + 1;		// 1 or 2

	if ( cs & 1 )
		retval |= m_6522[0].Read( addr & 0x0F );
	if ( cs & 2 )
		retval |= m_6522[1].Read( addr & 0x0F );
	return retval;
}

void CPhasor::InitRomImage()
{
}

void CPhasor::Write(WORD addr, BYTE data)
{
	DWORD clock;

	clock = g_dwCPS;

	if (!(m_byMode & 02))		// Native Phasor Mode
	{
		m_byMode = addr & 0x01;
		if (addr & 0x04)
			clock = g_dwCPS * 2;
	}

	m_8913[0].SetClockSpeed(clock);
	m_8913[1].SetClockSpeed(clock);
	m_8913[2].SetClockSpeed(clock);
	m_8913[3].SetClockSpeed(clock);
}

BYTE CPhasor::Read(WORD addr)
{
	DWORD clock;

	clock = g_dwCPS;

	if (!(m_byMode & 02))		// Native Phasor Mode
	{
		m_byMode = addr & 0x01;
		if (addr & 0x04)
			clock = g_dwCPS * 2;
	}

	m_8913[0].SetClockSpeed(clock);
	m_8913[1].SetClockSpeed(clock);
	m_8913[2].SetClockSpeed(clock);
	m_8913[3].SetClockSpeed(clock);
	return 0;
}

void CPhasor::Configure()
{

	CDlgConfigMockingBoard dlg;
	dlg.m_bMute = m_8913[0].m_bMute && m_8913[3].m_bMute;
	dlg.SetRightVol( m_8913[0].m_iVol );
	dlg.SetLeftVol( m_8913[2].m_iVol );
	dlg.SetDipSwitch( m_byMode );
	if ( dlg.DoModal() == IDOK )
	{
		m_8913[0].m_bMute = dlg.m_bMute;
		m_8913[1].m_bMute = dlg.m_bMute;
		m_8913[2].m_bMute = dlg.m_bMute;
		m_8913[3].m_bMute = dlg.m_bMute;
		m_8913[0].m_iVol = dlg.GetRightVol();
		m_8913[1].m_iVol = dlg.GetRightVol();
		m_8913[2].m_iVol = dlg.GetLeftVol();
		m_8913[3].m_iVol = dlg.GetLeftVol();
		SetDipSwitch( dlg.GetDipSwitch() );
	}
}

void CPhasor::Clock(int clock)
{
	m_6522[0].Clock( clock );
	m_6522[1].Clock( clock );
#ifdef HAVE_VOTRAX
	m_6522[0].SetCA1( m_cVotrax[0].m_bBusy );
	m_6522[1].SetCA1( m_cVotrax[1].m_bBusy );
#endif
	if ( m_6522[0].GetIRQB() )
		g_pBoard->m_cpu.Assert_IRQ();

	if ( m_6522[1].GetIRQB() )
		g_pBoard->m_cpu.Assert_IRQ();
}

void CPhasor::PowerOn()
{
	m_8913[0].SetClockSpeed(g_dwCPS);
	m_8913[1].SetClockSpeed(g_dwCPS);
	m_8913[2].SetClockSpeed(g_dwCPS);
	m_8913[3].SetClockSpeed(g_dwCPS);
}

void CPhasor::Reset()
{
	m_6522[0].Reset();
	m_6522[1].Reset();
	m_8913[0].Reset();
	m_8913[1].Reset();
	m_8913[2].Reset();
	m_8913[3].Reset();
	Read( 0 );			// Reset Mode
}


void CPhasor::SetDipSwitch(int nDipSwitch)
{
	BYTE byMode;
	if ( m_nDipSwitch == nDipSwitch )
		return;
	byMode = nDipSwitch & 0x03;

	if ( byMode == PM_MB )
		m_strDeviceName = "Phasor : MB";
	else if ( byMode == PM_ECHO )
		m_strDeviceName = "Phasor : ECHO";
	else
		m_strDeviceName = "Phasor";

	m_byMode = byMode;

	Reset();

	CCard::SetDipSwitch( nDipSwitch );
	return;
}


void CPhasor::Serialize( CArchive &ar )
{
	CCard::Serialize( ar );

	if ( ar.IsStoring() )
	{
		ar << m_byMode;
		m_6522[0].Serialize( ar );
		m_6522[1].Serialize( ar );
		m_8913[0].Serialize( ar );
		m_8913[1].Serialize( ar );
		m_8913[2].Serialize( ar );
		m_8913[3].Serialize( ar );
	}
	else
	{
		ar >> m_byMode;
		m_6522[0].Serialize( ar );
		m_6522[1].Serialize( ar );
		m_8913[0].Serialize( ar );
		m_8913[1].Serialize( ar );
		m_8913[2].Serialize( ar );
		m_8913[3].Serialize( ar );
	}
}
