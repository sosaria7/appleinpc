//////////////////////////////////////////////////////////////////////
//
// Screen.h: interface for the CScreen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREEN_H__ACC50F7A_96E5_40B5_B189_3FCFE88BAEB1__INCLUDED_)
#define AFX_SCREEN_H__ACC50F7A_96E5_40B5_B189_3FCFE88BAEB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <ddraw.h>
#include "arch/directx/ddutil.h"
#include "arch/customthread.h"

#include "lockmgr.h"

#define CLR80VID	0xC00C
#define SET80VID	0xC00D

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

#define PTRIG           0xC070
#define SETIOUDIS	0xC07E
#define RDIOUDIS        0xC07E
#define CLRIOUDIS	0xC07F
#define RDDHIRES	0xC07F

#define CLRROM		0xCFFF

/* LSB are graphics switches */
#define SS_MIXED	(1<<0)
#define SS_TEXT		(1<<1)
#define SS_PAGE2	(1<<2)
#define SS_80COL	(1<<3)
#define SS_HIRES	(1<<4)

#define SS_80STORE	(1<<5)
#define SS_DHIRES	(1<<6)
#define IOU_VIDEO_SS_MASK	(0x7f)

#define SS_ALTCHAR	(1<<7)
#define SS_RAMRD	(1<<8)
#define SS_RAMWRT	(1<<9)
#define SS_SLOTCXROM	(1<<10)
#define SS_ALTZP	(1<<11)
#define SS_SLOTC3ROM	(1<<12)
#define SS_IOUDIS	(1<<13)
#define SS_LCBNK2	(1<<14)
#define SS_LCRAMRD	(1<<15)
#define SS_LCRAM	SS_LCRAMRD
#define SS_LCRAMWRT	(1<<16)
//#define SS_VBLBAR	(1<<17)
#define SS_EXPNROM	(1<<18)		/* 80 column ROM $C800-$CFFF active */

#define SS_AN0		(1<<19)
#define SS_AN1		(1<<20)
#define SS_AN2		(1<<21)
#define SS_AN3		(1<<22)

//#define SS_VBL		(1<<23)

#define SM_COLOR	0	/* HSB based default */
#define SM_COLOR2	1	/* custom color */
#define SM_WHITE	2
#define SM_GREEN	3

#define WIN_WIDTH	576
#define WIN_HEIGHT	392
#define WIN_TOP_MARGIN		4
#define WIN_BOTTOM_MARGIN	(WIN_HEIGHT-192*2-WIN_TOP_MARGIN)
#define FULL_WIDTH	640
#define FULL_HEIGHT	480

#define FULL_BPP	32

#define DISK_VIEW_WIDTH 72
#define DISK_VIEW_HEIGHT 16
#define MSG_VIEW_WIDTH 600
#define MSG_VIEW_HEIGHT 16

#define RGB16(r,g,b)	( (((r)>>3)<<11)|(((g)>>2)<<5)|((b)>>3) )
#define RGB24(r,g,b)	( ( (r)<<16 ) | ( (g)<<8 ) | (b) )
#define RGB24_16(c24)	( (((c24)>>8)&0xF800)|(((c24)>>5)&0x7E0)|(((c24)>>3)&0x1F) )
#define HSB(h,s,b)		( ((((h)+360)%360)<<16) | ((s)<<8) | (b) )

class CAppleClock;

typedef struct
{
    int y;
    int i;
    int q;
} sYIQ;

#define FRAME_CHECK_COUNT	10
#define FRAME_CHECK_POINT	30

class CScreen : public CWnd, public CCustomThread
{
public:
	DECLARE_SERIAL(CScreen);

	CSurface* m_pSurfaceDisk;
	CSurface* m_pSurfaceMsg;
	CSurface* m_pSurfaceMain;

	BOOL m_bWindowed;
	BOOL m_bDoubleSize;
	CDisplay* m_pDisplay;
	BYTE m_aiScreenTable[0x2000][2];
	WORD m_awScanOT[192];
	BYTE* m_apbScanAT[8][192];
	int m_iScrMode;
	BYTE m_pixelInfo[192][WIN_WIDTH+3];
	CCSWrapper m_Lock;
	BOOL m_bPreview;		// for color config dialog
	BYTE m_pixelCarry;
	DWORD m_dataLatch;
	DWORD m_dwClock;
	int m_nLine;
	int m_nColumn;
	DWORD m_adwFrameCheck[FRAME_CHECK_COUNT];
	double m_dFrameRate;
	bool m_bRelax;
	void Serialize(CArchive &ar);

public:
	int m_iBlinkCount;
	CScreen( );
	virtual ~CScreen();

	void Redraw();
	void OnDebug();
	HRESULT Present();
	char* DDErrorString(HRESULT hr);
	BYTE ChangeMode(WORD addr, int nDelay);
	void ReInitialize();
	void SetDefaultColors();
	void Draw( int nLine, int nColumn );
	void Clock( DWORD clock );
	void PowerOn();
	void PowerOff();
	void SetMouseCapture(BOOL bCapture);

	BOOL IsVBL();

	void SetColors(unsigned int* pColors);
	void SetHSB(unsigned int uHSB);
	unsigned int* GetColors();
	unsigned int* GetColorsByHSB();
	unsigned int GetMonoColor();
	unsigned int GetGreenColor();
	BOOL GetScanline();
	void SetMonoColor(unsigned int uColor);
	void SetGreenColor(unsigned int uColor);
	void SetScanline(BOOL bScanline);

	void HideMessage();
	void ToggleMessage();
	void SetMessage(TCHAR* szText);
	void UpdateDiskSurface();
	CSurface* GetDiskSurface();
	void SetScreenMode(BOOL bFullScreen, BOOL bDoubleSize);
	void Reset();
	unsigned int ApplyRGBFormat(unsigned int rgb32, LPDDPIXELFORMAT DDpf);
	unsigned int ApplyDarkRGBFormat(unsigned int rgb32, LPDDPIXELFORMAT lpDDpf, double rate);
	void Clr80Store();
	void Set80Store();
	BYTE CheckMode(WORD addr);
	unsigned int GetHSB();
	void ApplyColors();
	sYIQ Rgb2ntsc(unsigned int rgb32);
	unsigned int Ntsc2rgb(sYIQ* yiq);
	sYIQ ComposeYIQ(sYIQ* yiq1, sYIQ* yiq2);
	void ChangeMonitorType();
	void ChangeMonitorType(int type);
	int GetMonitorType();
	BYTE m_abPosTable[120];
	void setLookUp(BYTE *pMemory);
	BYTE GetVideoData();

	void ClearBuffer();
	void Render();
	void Run();
	void Relax();
	BOOL IsDoubleSized() { return m_bDoubleSize; }
protected:
	DDSURFACEDESC m_ddsdDesc;
	LPDIRECTDRAWSURFACE m_lpddsPrimary;
	LPDIRECTDRAW m_lpddDraw;

	// color defines
	unsigned int m_uRGBGreen;
	unsigned int m_uRGBMono;
	unsigned int m_auRGBColor[16];
	unsigned int m_auRGBColorByHSB[16];
	unsigned int m_uHSB;
	// actual colors by system color depth
	unsigned int m_uGreen;
	unsigned int m_uWhite;
	unsigned int m_auColorTable[16][16];
	unsigned int m_auColorTableScanLine[16][16];
	unsigned int m_auColorTableByHSB[16][16];	// by HSB
	unsigned int m_auColorTableByHSBScanLine[16][16];		// by HSB
	unsigned int m_uGreenScanLine;
	unsigned int m_uWhiteScanLine;

	int m_iScrModeHold;
	int m_nScrModeDelay;

	BYTE m_nVideoMode;
	BOOL m_nMsgVisiable;
	BOOL m_bTextMode;
	BOOL m_bMouseCapture;

	TCHAR* m_szMessage;
	HFONT m_hFont;

	RECT m_stMainRect;

	int m_iColorDepth;
	DWORD m_dwColorHalfMask;
	DWORD m_nTime;
	BOOL m_bPowerOn;
	BOOL m_bScanline;

//functions
protected:
	BOOL InitDirectX();
	void CaptureInput(BOOL bMouseCapture);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreen)
	public:
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CScreen)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnDisplayChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCREEN_H__ACC50F7A_96E5_40B5_B189_3FCFE88BAEB1__INCLUDED_)

