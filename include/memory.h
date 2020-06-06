// Memory.h: interface for the Memory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORY_H__A8396EA1_6860_4A73_87B0_B68FAECE64F3__INCLUDED_)
#define AFX_MEMORY_H__A8396EA1_6860_4A73_87B0_B68FAECE64F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#define A2MEMSIZE	0x20000		// 128K
#define A2ROMSIZE	0x4000		// 16k

#define KEYBOARD	0xC000
#define	CLR80STORE	0xC000
#define SET80STORE	0xC001
#define RDMAINRAM	0xC002
#define RDCARDRAM	0xC003
#define WRMAINRAM	0xC004
#define WRCARDRAM	0xC005
#define SETSLOTCXROM	0xC006
#define SETINTCXROM	0xC007
#define SETSTDZP	0xC008
#define SETALTZP	0xC009
#define SETINTC3ROM	0xC00A
#define SETSLOTC3ROM	0xC00B
#define	CLR80VID	0xC00C
#define SET80VID	0xC00D
#define CLRALTCHAR	0xC00E
#define SETALTCHAR	0xC00F

#define KBDSTRB		0xC010
#define	RDLCBNK2	0xC011
#define	RDLCRAM		0xC012
#define	RDRAMRD		0xC013
#define	RDRAMWRT	0xC014
#define	RDINTCXROM	0xC015
#define	RDALTZP		0xC016
#define	RDSLOTC3ROM	0xC017
#define	RD80STORE	0xC018
#define	RDVBLBAR	0xC019
#define	RDTEXT		0xC01A
#define	RDMIXED		0xC01B
#define	RDPAGE2		0xC01C
#define	RDHIRES		0xC01D
#define	RDALTCHAR	0xC01E
#define	RD80COL		0xC01F

#define TAPEOUT		0xC020
#define PRNTOUT		0xC021		/* Virtual printer */
#define SPKR		0xC030

#define TXTCLR		0xC050
#define TXTSET		0xC051
#define MIXCLR		0xC052
#define MIXSET		0xC053
#define LOWSCR		0xC054
#define	HISCR		0xC055
#define LOWRES		0xC056
#define HIRES		0xC057
#define CLRAN0		0xC058
#define SETAN0		0xC059
#define CLRAN1		0xC05A
#define SETAN1		0xC05B
#define CLRAN2		0xC05C
#define SETAN2		0xC05D
#define CLRAN3		0xC05E
#define SETDHIRES	0xC05E
#define SETAN3		0xC05F
#define CLRDHIRES	0xC05F

#define	TAPEIN		0xC060
#define	BUTN0		0xC061
#define	BUTN1		0xC062
#define	BUTN2		0xC063
#define PADDL0		0xC064
#define PADDL1		0xC065
#define PADDL2		0xC066
#define PADDL3		0xC067

#define PTRIG		0xC070
#define SETIOUDIS	0xC07E
#define RDIOUDIS	0xC07E
#define CLRIOUDIS	0xC07F
#define RDDHIRES	0xC07F

#define CLRROM		0xCFFF

// Language Card setting
#define MS_WRITERAM		( 1 << 0 )
#define MS_READSAME		( 1 << 1 )
#define MS_BANK1		( 1 << 3 )
#define MS_READAUX		( 1 << 4 )
#define MS_WRITEAUX		( 1 << 5 )
#define MS_RWAUXZP		( 1 << 6 )
#define MS_INTCXROM		( 1 << 9 )
#define MS_SLOTC3ROM	( 1 << 10 )
#define MS_80STORE		( 1 << 7 )
#define MS_HISCR		( 1 << 8 )
#define MS_INTCXROM2	( 1 << 11 )

#include "arch/frame/screen.h"
#include "keyboard.h"	// Added by ClassView
#include "iou.h"

class CAppleClock;

class CAppleIOU : public CIou
{
public:
	DECLARE_SERIAL( CAppleIOU );
	CAppleIOU();
	virtual ~CAppleIOU();

	void AppleIOWrite(WORD addr, BYTE data);
	void InitMemory(int nMachineType);
	BOOL ReadRomFile();

	void Serialize(CArchive& ar);


	BYTE* m_pReadMap[0x10];
	BYTE* m_pWriteMap[0x10];

protected:
	BYTE* m_pMem;	// current memory ( main or aux )
	BYTE* m_pROM;	// 16k

	int m_nMachineType;

public:
	BOOL m_bMemTest;

	int m_iMemMode;
	int	m_iLastMemMode;
	BOOL m_bLastWriteRamFlag;
	BYTE CheckMode(WORD addr);
	void SwitchAuxMemory(WORD addr);
	void Init();
	void InitMemoryMap(void);
	void UpdateMemoryMap(void);

	// CIou Overrides
	BYTE ReadMem8(int nAddr);
	void WriteMem8(int nAddr, BYTE byData);
	WORD ReadMem16(int nAddr);
	void WriteMem16(int nAddr, WORD wData);
};

extern BYTE MemReturnRandomData(BYTE highbit);

#endif // !defined(AFX_MEMORY_H__A8396EA1_6860_4A73_87B0_B68FAECE64F3__INCLUDED_)
