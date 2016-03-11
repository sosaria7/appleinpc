// Memory.cpp: implementation of the Memory class.
//
//////////////////////////////////////////////////////////////////////


#include "arch/frame/stdafx.h"
#include "arch/frame/mainfrm.h"
#include "arch/directx/dxsound.h"
#include "appleclock.h"
#include "memory.h"
#include "debug.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL( CAppleIOU, CObject, 1 );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BYTE MemReturnRandomData(BYTE highbit)
{
	BYTE val;

	CScreen *pCScreen;
	pCScreen = g_pBoard->m_pScreen;
	val = pCScreen->GetVideoData();

	if ( highbit == 0 )
		return val & ~0x80;
	else if ( highbit == 1 )
		return val | 0x80;
	else
		return val;
}

CAppleIOU::CAppleIOU()
{
	m_pMem = new BYTE[A2MEMSIZE];	// allocate 128K memory
	m_pROM = new BYTE[A2ROMSIZE]; // allocate 16K ROM
	m_bMemTest = FALSE;
	m_nMachineType = MACHINE_APPLE2E;
}

CAppleIOU::~CAppleIOU()
{
	if ( m_pMem )
	{
		delete m_pMem;
		m_pMem = NULL;
	}
	if ( m_pROM )
	{
		delete m_pROM;
		m_pROM = NULL;
	}
}

BOOL CAppleIOU::ReadRomFile()
{	
	CFile romFile;
	CFileException ex;

	char* pszRomName;
	switch (m_nMachineType)
	{
	case MACHINE_APPLE2P:
		pszRomName = "APPLE2.ROM";
		break;
	case MACHINE_APPLE2E:
	default:
		pszRomName = "APPLE2E.ROM";
		break;
	}

	if(!romFile.Open(pszRomName, CFile::modeRead, &ex)){
		AfxMessageBox(IDS_NO_ROMFILE);
		return FALSE;
	}

	// load rom file  - apple //e
	
		DWORD length = (DWORD)romFile.GetLength();
		if(length != 0x4000)		// apple //e rom size
		{
			AfxMessageBox(IDS_INVALID_ROMFILE_LENGTH);
			romFile.Close();
			return FALSE;
		}
		romFile.Read(m_pROM, length);
	
	romFile.Close();
	return TRUE;
}

void CAppleIOU::AppleIOWrite(WORD addr, BYTE data)
{

}

void CAppleIOU::UpdateMemoryMap(void)
{
	int i, aux;
	int diff = m_iLastMemMode ^ m_iMemMode;
	if ( diff & ( MS_BANK1 | MS_WRITERAM | MS_READSAME | MS_RWAUXZP ) )
	{
		if ( ( m_iMemMode & MS_RWAUXZP ) != 0 && m_nMachineType != MACHINE_APPLE2P )
			aux = 0x10000;
		else
			aux = 0;
		int addr;
		if ( m_iMemMode & MS_BANK1 )
			addr = 0xC000;
		else
			addr = 0xD000;
		if ( m_iMemMode & MS_WRITERAM )
		{
			m_pWriteMap[0x0D] = m_pMem + addr + aux;
			if ( m_iMemMode & MS_READSAME )
				m_pReadMap[0x0D] = m_pMem + addr + aux;
			else
				m_pReadMap[0x0D] = m_pROM + 0x1000;
		}
		else
		{
			m_pWriteMap[0x0D] = NULL;
			if ( m_iMemMode & MS_READSAME )
				m_pReadMap[0x0D] = m_pROM + 0x1000;
			else
				m_pReadMap[0x0D] = m_pMem + addr + aux;
		}

		if ( diff & ( MS_WRITERAM | MS_READSAME | MS_RWAUXZP ) )
		{
			for ( i = 0xE; i <= 0xF; i++ )
			{
				int addr = i << 12;
				
				if ( m_iMemMode & MS_WRITERAM )
				{
					m_pWriteMap[i] = m_pMem + addr + aux;
					if ( m_iMemMode & MS_READSAME )
						m_pReadMap[i] = m_pMem + addr + aux;
					else
						m_pReadMap[i] = m_pROM + ( addr - 0xC000 );
				}
				else
				{
					m_pWriteMap[i] = NULL;
					if ( m_iMemMode & MS_READSAME )
						m_pReadMap[i] = m_pROM + ( addr - 0xC000 );
					else
						m_pReadMap[i] = m_pMem + addr + aux;
				}
			} // for ( i = 0xE; i <= 0xF; i++ )
		} // if ( diff & ( MS_WRITERAM | MS_READSAME | MS_RWAUXZP ) )
	}
}


void CAppleIOU::InitMemoryMap()
{
	m_iMemMode = m_iLastMemMode = MS_WRITERAM;
	int i;
	for(i=0x0; i<= 0xF; i++){
		int addr = i<<12;
		if(i<0xC){
			m_pReadMap[i] = m_pMem + addr;
			m_pWriteMap[i] = m_pMem + addr;
		}
		else if(i==0x0C){
			m_pReadMap[i] = m_pROM+addr-0xC000;
			m_pWriteMap[i] = NULL;
		}
		else{
			m_pReadMap[i] = m_pROM+(addr-0xC000);
			m_pWriteMap[i] = m_pMem + addr;
		}
	}
	g_pBoard->m_pScreen->Reset();
	g_pBoard->m_cSlots.Reset();
}

// reboot
void CAppleIOU::Init()
{
	int i;
	for( i=0; i<0x20000; i++)
		m_pMem[i] = 0;
	InitMemoryMap();
}

// on start
void CAppleIOU::InitMemory(int nMachineType)
{
	int i;

	this->m_nMachineType = nMachineType;
	for(i=0; i < 0x20000; i++){
		m_pMem[i] = 0x00;
	}
	ReadRomFile();
	InitMemoryMap();
	g_pBoard->m_pScreen->setLookUp( m_pMem );
}


void CAppleIOU::SwitchAuxMemory(WORD addr)
{
	int offset, i;

	if (m_nMachineType == MACHINE_APPLE2P)
		return;

	switch( addr )
	{
	case RDMAINRAM:
		for( i = 0; i <= 0x0B; i++ )
		{
			offset = i << 12;
			m_pReadMap[i] = m_pMem+offset;
		}
		m_iMemMode &= ~MS_READAUX;
		break;
	case RDCARDRAM:
		for( i = 0; i <= 0x0B; i++ )
		{
			offset = 0x10000 + ( i << 12 );
			m_pReadMap[i] = m_pMem+offset;
		}
		m_iMemMode |= MS_READAUX;
		break;
	case WRMAINRAM:
		for( i = 0; i <= 0x0B; i++ )
		{
			offset = i << 12;
			m_pWriteMap[i] = m_pMem+offset;
		}
		m_iMemMode &= ~MS_WRITEAUX;
		break;
	case WRCARDRAM:
		for( i = 0; i <= 0x0B; i++ )
		{
			offset = 0x10000 + ( i << 12 );
			m_pWriteMap[i] = m_pMem+offset;
		}
		m_iMemMode |= MS_WRITEAUX;
		break;
	case SETSTDZP:
		m_iLastMemMode = m_iMemMode;
		m_iMemMode &=  ~MS_RWAUXZP;
		UpdateMemoryMap();
		return;
	case SETALTZP:
		m_iLastMemMode = m_iMemMode;
		m_iMemMode |= MS_RWAUXZP;
		UpdateMemoryMap();
		break;
	case SETSLOTCXROM:
		m_iMemMode &= ~MS_INTCXROM;
		break;
	case SETINTCXROM:
		m_iMemMode |= MS_INTCXROM;
		break;
	case SETINTC3ROM:
		m_iMemMode &= ~MS_SLOTC3ROM;
		break;
	case SETSLOTC3ROM:
		m_iMemMode |= MS_SLOTC3ROM;
		break;
	case CLR80STORE:
		g_pBoard->m_pScreen->Clr80Store();
		m_iMemMode &= ~MS_80STORE;
		break;
	case SET80STORE:
		g_pBoard->m_pScreen->Set80Store();
		m_iMemMode |= MS_80STORE;
		break;
	case CLRALTCHAR:
	case SETALTCHAR:
	case SET80VID:
	case CLR80VID:
		g_pBoard->m_pScreen->ChangeMode(addr);
		break;
	default:
		break;
	}
}

BYTE CAppleIOU::CheckMode(WORD addr)
{
    BYTE mode = 0;
    switch( addr )
    {
    case RDLCBNK2:
		if ( !( m_iMemMode & MS_BANK1 ) )
			mode = 0x80;
		break;
    case RDLCRAM:
		break;
    case RDRAMRD:
		if ( m_iMemMode & MS_READAUX )
			mode = 0x80;
		break;
    case RDRAMWRT:
		if ( m_iMemMode & MS_WRITEAUX )
			mode = 0x80;
		break;
    case RDALTZP:
		if ( m_iMemMode & MS_RWAUXZP )
			mode = 0x80;
		break;
    case RDINTCXROM:
		if ( m_iMemMode & MS_INTCXROM )
			mode = 0x80;
		break;
    case RDSLOTC3ROM:
		if ( m_iMemMode & MS_SLOTC3ROM )
			mode = 0x80;
		break;
    case RD80STORE:
		if ( m_iMemMode & MS_80STORE )
			mode = 0x80;
		break;

    case RDTEXT:
    case RDMIXED:
    case RDPAGE2:
    case RDHIRES:
    case RDVBLBAR:
    case RDALTCHAR:
    case RD80COL:
		mode = g_pBoard->m_pScreen->CheckMode( addr );
		break;
    }
    return mode;
}

BYTE CAppleIOU::ReadMem8(int nAddr)
{
	BYTE page;
	WORD offset;
	nAddr &= 0xFFFF;
	page = nAddr >> 12;
	offset = nAddr & 0xFFF;

	// video memory
	// if 80STORE is ON, Use page 1 video to Aux memory
	if( m_nMachineType != MACHINE_APPLE2P &&
		( m_iMemMode & MS_80STORE ) &&
		( ( nAddr>=0x0400 && nAddr<0x0800) || (nAddr>=0x2000 && nAddr<0x4000) ) )
	{
		if ( nAddr < 0x800 || ( g_pBoard->m_pScreen->CheckMode(RDHIRES)&0x80 ) )
		{
			if ( m_iMemMode & MS_HISCR )
				return m_pMem[ nAddr + 0x10000 ];
			else
				return m_pMem[ nAddr ];
		}
	}

	switch (nAddr >> 8)
	{
	case 0x00: case 0x01:	// Zero Page
		if (m_iMemMode & MS_RWAUXZP)
			return m_pMem[nAddr + 0x10000];
		else
			return m_pMem[nAddr];

	case 0xC0:
		if (m_bMemTest)
			return 0;

		else
		{
			switch (nAddr & 0xFF0)
			{
			case 0x000:
				return g_pBoard->m_keyboard.AppleKeyRead(nAddr & 0xFF);
				break;
			case 0x010:
				return CheckMode(nAddr) | g_pBoard->m_keyboard.AppleKeyRead(nAddr & 0xFF);
			case 0x020:
				break;
			case 0x030:
				g_pBoard->m_cSpeaker.Toggle();
				break;
				// set video mode
			case 0x040:
				break;
			case 0x050:
				if (nAddr == LOWSCR)
					m_iMemMode &= ~MS_HISCR;
				else if (nAddr == HISCR)
					m_iMemMode |= MS_HISCR;
				if (m_nMachineType != MACHINE_APPLE2P || nAddr < SETDHIRES)
					g_pBoard->m_pScreen->ChangeMode(nAddr);
				break;
			case 0x060:
				return g_pBoard->m_joystick.GetStatus(nAddr & 0xFF);
				break;
			case 0x070:
				if (nAddr == 0xC070)
					g_pBoard->m_joystick.Strobe();
				break;
			case 0x080:
				m_iLastMemMode = m_iMemMode;
				m_iMemMode = (m_iMemMode & ~0x0F) | (nAddr & 0x0F);
				UpdateMemoryMap();
				break;
				// slot I/O
			case 0x090: case 0x0A0: case 0x0B0: case 0x0C0:
			case 0x0D0: case 0x0E0: case 0x0F0:
				return g_pBoard->m_cSlots.Read(nAddr);
				break;
			}
		}
		return MemReturnRandomData(2);
	case 0xC3:
		if ((m_iMemMode & MS_SLOTC3ROM) != 0 || m_nMachineType == MACHINE_APPLE2P)
		{
			return g_pBoard->m_cSlots.ReadRom(nAddr);
		}
		m_iMemMode |= MS_INTCXROM2;
		break;

	case 0xC1: case 0xC2: case 0xC4: case 0xC5:
	case 0xC6: case 0xC7:
		if ((m_iMemMode & MS_INTCXROM) == 0 || m_nMachineType == MACHINE_APPLE2P)
		{
			return g_pBoard->m_cSlots.ReadRom(nAddr);
		}
		break;

	case 0xC8: case 0xC9: case 0xCA: case 0xCB:
	case 0xCC: case 0xCD: case 0xCE: case 0xCF:
		if (((m_iMemMode & MS_INTCXROM) == 0 && (m_iMemMode & MS_INTCXROM2) == 0) || m_nMachineType == MACHINE_APPLE2P)
		{
			return MemReturnRandomData(2);
		}
		if (nAddr == 0xCFFF)
		{
			m_iMemMode &= ~MS_INTCXROM2;
		}
	default:
		break;
	}

	return m_pReadMap[page][offset];
}

void CAppleIOU::WriteMem8(int nAddr, BYTE byData)
{
	BYTE page;
	WORD offset;
	nAddr &= 0xFFFF;

	page = nAddr>>12;
	offset = nAddr&0xFFF;
	// video memory
	if((nAddr>=0x0400 && nAddr<0x0C00) ||
		(nAddr>=0x2000 && nAddr<0x6000))
	{

		BOOL page1 = ( nAddr < 0x800 || ( nAddr >=0x2000 && nAddr < 0x4000 ) );

		if (m_nMachineType != MACHINE_APPLE2P &&
			( m_iMemMode & MS_80STORE ) && page1 &&
			( nAddr < 0x800 || ( g_pBoard->m_pScreen->CheckMode(RDHIRES)&0x80 ) ) )
		{
			if ( m_iMemMode & MS_HISCR )
			{
				m_pMem[ nAddr + 0x10000 ] = byData;
				g_pBoard->m_pScreen->writeMemory( nAddr, byData, TRUE );
			}
			else
			{
				m_pMem[ nAddr ] = byData;
				g_pBoard->m_pScreen->writeMemory( nAddr, byData, FALSE );
			}
		}
		else
		{
			m_pWriteMap[page][offset] = byData;
			g_pBoard->m_pScreen->writeMemory( nAddr, byData, m_iMemMode & MS_WRITEAUX );
		}
		return;
	}

	switch (nAddr >> 8)
	{
	case 0x00: case 0x01:
		if (m_iMemMode & MS_RWAUXZP)
			m_pMem[nAddr + 0x10000] = byData;
		else
			m_pMem[nAddr] = byData;
		break;

	case 0xC0:
		switch (nAddr & 0xFF0)
		{
		case 0x000: case 0x010:
			SwitchAuxMemory(nAddr);
			g_pBoard->m_keyboard.AppleKeyWrite(nAddr & 0xFF, byData);
			break;
		case 0x020:
			break;
		case 0x030:
			g_pBoard->m_cSpeaker.Toggle();
			break;
		case 0x040:
			//TRACE("Write $%04X:#$%02X\n", nAddr, byData);
			break;
			// set video mode
		case 0x050:
			if (nAddr == LOWSCR)
				m_iMemMode &= ~MS_HISCR;
			else if (nAddr == HISCR)
				m_iMemMode |= MS_HISCR;
			if (m_nMachineType != MACHINE_APPLE2P || nAddr < SETDHIRES)
				g_pBoard->m_pScreen->ChangeMode(nAddr);
			break;
		case 0x060:
			break;
		case 0x070:
			if (nAddr == 0xC070)
				g_pBoard->m_joystick.Strobe();
			//TRACE("Write $%04X:#$%02X\n", nAddr, byData);
			break;
			// change memory mode (Language Card)
		case 0x080:
			m_iLastMemMode = m_iMemMode;
			m_iMemMode = (m_iMemMode & ~0x0F) | (nAddr & 0x0F);
			UpdateMemoryMap();
			break;
			// slot I/O
		case 0x090: case 0x0A0: case 0x0B0: case 0x0C0:
		case 0x0D0: case 0x0E0: case 0x0F0:
			g_pBoard->m_cSlots.Write(nAddr, byData);
			break;
		}
		break;

	case 0xC3:
		if ((m_iMemMode & MS_SLOTC3ROM) != 0 || m_nMachineType == MACHINE_APPLE2P)
		{
			g_pBoard->m_cSlots.WriteRom(nAddr, byData);
			break;
		}
		m_iMemMode |= MS_INTCXROM2;
		break;

	case 0xC1: case 0xC2: case 0xC4: case 0xC5:
	case 0xC6: case 0xC7:
		if ((m_iMemMode & MS_INTCXROM) == 0 || m_nMachineType == MACHINE_APPLE2P)
		{
			g_pBoard->m_cSlots.WriteRom(nAddr, byData);
		}
		break;

	case 0xC8: case 0xC9: case 0xCA: case 0xCB:
	case 0xCC: case 0xCD: case 0xCE: case 0xCF:
		if (nAddr == 0xCFFF)
		{
			m_iMemMode &= ~MS_INTCXROM2;
		}
		break;

	default:
		m_pWriteMap[page][offset] = byData;
		break;
	}
}

WORD CAppleIOU::ReadMem16(int nAddr)
{
	return ReadMem8( nAddr ) |
		( ReadMem8( nAddr+1 ) << 8 );
}

void CAppleIOU::WriteMem16(int nAddr, WORD wData)
{
	WriteMem8( nAddr, (BYTE)wData );
	WriteMem8( nAddr+1, (BYTE)(wData>>8) );
}

void CAppleIOU::Serialize(CArchive &ar)
{
	CObject::Serialize(ar);
	
	if ( ar.IsStoring() )
	{
		ar.Write( m_pMem, A2MEMSIZE );
		ar << m_iMemMode;
		ar << m_nMachineType;
	}
	else
	{
		ar.Read( m_pMem, A2MEMSIZE );
		ar >> m_iMemMode;
		if (g_nSerializeVer >= 7)
		{
			ar >> m_nMachineType;
		}
		m_iLastMemMode = ~m_iMemMode;
		UpdateMemoryMap();
		if ( m_iMemMode & MS_READAUX )
		{
			SwitchAuxMemory( RDCARDRAM );
		}
		if ( m_iMemMode & MS_WRITEAUX )
		{
			SwitchAuxMemory( WRCARDRAM );
		}
	}
}
