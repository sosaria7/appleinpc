// Screen.cpp: implementation of the CScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "arch/frame/Screen.h"
#include "arch/frame/mainfrm.h"
#include "arch/directx/dikeyboard.h"
#include "arch/directx/dimouse.h"

#include "fontdata.h"
#include "appleclock.h"
#include "memory.h"
#include "aipcdefs.h"

#include <windows.h>
#include <windowsx.h>
#include <math.h>

#define PI	3.141592654

extern CDIKeyboard g_cDIKeyboard;
extern CDIMouse g_cDIMouse;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SAFE_RELEASE(a)	if (a){ delete a; a = NULL; }
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CScreen, CWnd)
//{{AFX_MSG_MAP(CScreen)
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
	ON_MESSAGE(UM_DISPLAY_CHANGE, OnDisplayChanged)
END_MESSAGE_MAP()

IMPLEMENT_SERIAL( CScreen, CObject, 1 );

CScreen::CScreen()
	: CCustomThread("AppleVideoThread")
{
	m_bPowerOn = FALSE;
	m_iBlinkCount = 0;
	m_nVideoMode = SM_COLOR;
	m_bScanline = TRUE;
	m_dFrameRate = 0;

	memset(m_pixelInfo, 0, sizeof(m_pixelInfo));
	m_pixelCarry = 0;
	m_dwClock = 0;
	m_dataLatch = 0;
	m_nColumn = 0;
	m_nLine = 0;

	m_pDisplay = new CDisplay();
//	m_pDisplay = NULL;
	m_pSurfaceDisk = NULL;
	m_pSurfaceMsg = NULL;
	m_pSurfaceMain = NULL;

	m_nMsgVisiable = 0;
	m_bWindowed = TRUE;
	m_bDoubleSize = FALSE;
//	m_bInitializing = FALSE;
	m_iScrMode = SS_TEXT;
	m_szMessage = "";
	m_bMouseCapture = FALSE;

	m_hFont = ::CreateFont( 15, 0, 0, 0, 0, 0, 0, 0, ANSI_CHARSET,
							OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
							ANTIALIASED_QUALITY, FF_DONTCARE, "Arial Narrow" );
	m_bPreview = FALSE;
	m_bTextMode = TRUE;
	m_bRelax = false;
	SetDefaultColors();
}

CScreen::~CScreen()
{
	SAFE_RELEASE(m_pDisplay);
	SAFE_RELEASE(m_pSurfaceDisk);
	SAFE_RELEASE(m_pSurfaceMsg);
	SAFE_RELEASE(m_pSurfaceMain);
}

void CScreen::PowerOn()
{
	this->SetActive(TRUE);
	CaptureInput(FALSE);
}

void CScreen::PowerOff()
{
	this->SetActive(FALSE);
}

BYTE CScreen::GetVideoData()
{
	return (BYTE)( m_dataLatch >> 8 );
}

#define ROR16(v,n)		(WORD)((((WORD)(v))>>n)|(((WORD)(v))<<(16-n)))
#define ROR8(v,n)		(BYTE)((((BYTE)(v))>>n)|(((BYTE)(v))<<(8-n)))
#define ROL16(v,n)		(WORD)((((WORD)(v))<<n)|(((WORD)(v))>>(16-n)))
#define ROL8(v,n)		(BYTE)((((BYTE)(v))<<n)|(((BYTE)(v))>>(8-n)))

static BYTE s_abyBitExtend[] =
{
	0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
	0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
};

void CScreen::Draw( int nLine, int nColumn )
{
	WORD data, data2;
	int index;
	int color;
	int x, y;
	int mode = m_iScrMode & 0x07;
	BOOL b80COL = ( m_iScrMode & SS_80COL ) != 0;
	BOOL bDHIRES = b80COL && ( m_iScrMode & SS_DHIRES ) != 0;
	_int64* _fontData;
	BOOL bAltChar = ( m_iScrMode & SS_ALTCHAR ) != 0;

	if ( nLine < 0 || nLine > 191 || nColumn < 0 || nColumn > 39 )
	{
		m_dataLatch = ( m_dataLatch << 8 ) | 0xa0;
		return;
	}

	if ( bAltChar )
		_fontData = fontData2;
	else
		_fontData = fontData;
	BYTE byteBlinkMask;
	if( m_iBlinkCount & 0x10 )
	{
		byteBlinkMask = 0x40;
	}
	else
	{
		byteBlinkMask = 0xC0;
	}

	if(!(m_iScrMode&SS_HIRES))
		mode |= SS_TEXT;
	
	if ( m_iScrMode & SS_80STORE )		// display page 2 only if 80STORE is off
		mode &= ~SS_PAGE2;

	if (nLine == 191 && nColumn == 39)
	{
		m_bTextMode = ((m_iScrMode & SS_TEXT) != 0);
	}

	y = nLine;
	if (m_iScrMode&SS_TEXT || !(m_iScrMode&SS_HIRES) || (m_iScrMode&SS_MIXED && nLine > 159))
	{
		nLine = nLine & ~0x07;
	}

	if(m_iScrMode&SS_TEXT || (m_iScrMode&SS_MIXED && nLine>159))
	{
		index = m_apbScanAT[mode][nLine][nColumn];
		m_dataLatch = ( m_dataLatch << 8 ) | (BYTE)index;

		if ( ( index & 0xc0 ) == 0x40 && bAltChar == FALSE )	// is blink character ?
		{
			index ^= byteBlinkMask;
		}
		data2 = ( 7 - ( y % 8 ) ) * 8;
		data = (BYTE)( _fontData[index] >> data2 );

		if ( b80COL == TRUE )
		{
			index = m_apbScanAT[mode][nLine][nColumn+0x10000];
			if ( ( index & 0xc0 ) == 0x40 && bAltChar == FALSE )	// is blink character ?
			{
				index ^= byteBlinkMask;
			}
			data = ( data << 7 ) | (BYTE)( _fontData[index] >> data2 );
			bDHIRES = TRUE;
		}
	}
	// GR mode
	else if(!(m_iScrMode&SS_HIRES) )
	{
		index = ( nColumn % 2 ) * 2;	// 0 or 2
		data = m_apbScanAT[mode][nLine][nColumn];
		m_dataLatch = ( m_dataLatch << 8 ) | (BYTE)data;

		if ( ( y % 8 ) < 4 )
		{
			data &= 0x0f;
		}
		else
		{
			data = ( data >> 4 ) & 0x0f;
		}
		data |= ( data << 4 );
		data = ROL8( data, index );
		if ( bDHIRES == TRUE )
		{
			data2 = m_apbScanAT[mode][nLine][nColumn+0x10000];
			if ( ( y % 8 ) < 4 )
			{
				data2 &= 0x0f;
			}
			else
			{
				data2 = ( data2 >> 4 ) & 0x0f;
			}
			data2 |= ( data2 << 4 );
			data2 = ROL8( data2, index );
			data = ( data << 7 ) | ( data2 & 0x7f );
		}
		else
		{
			data |= data << 8;
		}
		b80COL = TRUE;
	}
	// hi-res mode
	else
	{
		data = m_apbScanAT[mode][nLine][nColumn];
		m_dataLatch = (m_dataLatch << 8) | (BYTE)data;

		if ( bDHIRES == TRUE )
		{
			data = ( data << 7 ) | ( m_apbScanAT[mode][nLine][nColumn+0x10000] & 0x7f );
		}
		else
		{
			b80COL = FALSE;
		}
	}

	if ( nColumn == 0 )
	{
		// clear line
		m_pixelCarry = 0;
	}
	if ( b80COL == FALSE )
	{
		data2 = data;
		data = s_abyBitExtend[data&0x0f] + ( s_abyBitExtend[(data>>4)&0x07] << 8 );
		if ( ( data2 & 0x80 ) != 0 )
		{
			data = ( ( data << 1 ) + m_pixelCarry ) & 0x3fff;
		}
		m_pixelCarry = ( data2 >> 6 ) & 1;
	}
	if ( bDHIRES == TRUE )
	{
		if ( ( nColumn % 2 ) == 0 )
		{
			color = 2;
		}
		else
		{
			color = 8;
		}
		x = nColumn * 14 + 3;
	}
	else
	{
		if ( ( nColumn % 2 ) == 0 )
		{
			color = 1;
		}
		else
		{
			color = 4;
		}
		x = nColumn * 14 + 10;
	}

	BYTE *pixelInfo = m_pixelInfo[y] + 2;
	if (nColumn == 0)
	{
		memset(pixelInfo, 0, x );
	}
	for( index = 0; index < 14; index++ )
	{
		if ( ( data & 1 ) != 0 )
		{
			pixelInfo[x] = (BYTE)color;
		}
		else
		{
			pixelInfo[x] = 0;
		}
		x++;
		data >>= 1;
		color <<= 1;
		if ( color >= 0x10 )
		{
			color = 1;
		}
	}
	if (nColumn == 39)
	{
		memset(pixelInfo + x, 0, WIN_WIDTH - x);
	}
}


void CScreen::Clock( DWORD clock )
{
	int i;
	for( i = 0; i < (int)clock; i++ )
	{
		Draw( m_nLine, m_nColumn - 25 );
		m_nColumn++;
		if ( m_nColumn >= 65 )
		{
			m_nColumn = 0;
			m_nLine++;
		}
		m_dwClock++;
		if (m_dwClock == g_dwFrameClock - g_dwVBLClock)
		{
			Redraw();
		}
		if ( m_dwClock >= g_dwFrameClock)
		{
			m_dwClock -= g_dwFrameClock;
			m_nLine = 0;
			m_nColumn = 0;
			m_iBlinkCount++;
		}
	}
}

BOOL CScreen::IsVBL()
{
	return ( m_dwClock >= g_dwFrameClock - g_dwVBLClock);
}

void CScreen::Redraw()
{
	this->WakeUp();
	Sleep(0);
}

void CScreen::Run()
{
	DWORD dwHostClock;
	int i;
	int nCurFrameIndex = 0;
	bool bIsFirstCycle = TRUE;
	DWORD dwInterval;
	DWORD dwFrameCount = 0;
	DWORD dwTickStart;
	DWORD dwElasped;
	m_dFrameRate = 0;

	dwHostClock = GetTickCount();
	for (i = 0; i < FRAME_CHECK_COUNT; i++)
	{
		m_adwFrameCheck[i] = dwHostClock;
	}
	Suspend(FALSE);
	m_bPowerOn = TRUE;

	while (TRUE)
	{
		m_bRelax = false;
		dwTickStart = GetTickCount();
		SuspendHere();
		if (ShutdownHere())
			break;
		Render();
		dwFrameCount++;
		if (dwFrameCount == FRAME_CHECK_POINT)
		{
			dwFrameCount = 0;
			dwHostClock = GetTickCount();
			dwInterval = dwHostClock - m_adwFrameCheck[nCurFrameIndex];
			m_adwFrameCheck[nCurFrameIndex] = dwHostClock;
			nCurFrameIndex = (nCurFrameIndex + 1) % FRAME_CHECK_COUNT;

			if (dwInterval != 0)
			{
				if (bIsFirstCycle == TRUE)
				{
					m_dFrameRate = (double)(FRAME_CHECK_POINT * nCurFrameIndex * 1000) / dwInterval;
					if (nCurFrameIndex == FRAME_CHECK_COUNT-1)
					{
						bIsFirstCycle = FALSE;
					}
				}
				else
				{
					m_dFrameRate = (double)(FRAME_CHECK_POINT * FRAME_CHECK_COUNT * 1000) / dwInterval;
				}
			}
		}
		dwElasped = GetTickCount() - dwTickStart;
		if (m_bRelax == true && dwElasped < 1000/30 )
			Sleep(1000/30 - dwElasped);
	}
	m_bPowerOn = FALSE;
	Render();
}

void CScreen::Render()
{
	DDSURFACEDESC2 ddsd;
    RECT rect={0, 0, WIN_WIDTH, WIN_HEIGHT};
	BOOL bDoubleSize = !m_bWindowed || m_bDoubleSize;

	if (bDoubleSize == TRUE)
	{
		rect.right *= 2;
		rect.bottom *= 2;
	}

    ddsd.dwSize=sizeof(ddsd);
	
	HRESULT result;
	LPDIRECTDRAWSURFACE7 lpddsBack;

	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	lpddsBack = m_pSurfaceMain->GetDDrawSurface();
	//lpddsBack = m_pDisplay->GetBackBuffer();
	if ( lpddsBack == NULL )
		return;

    if FAILED( result=lpddsBack->Lock(&rect,&ddsd,DDLOCK_NOSYSLOCK|DDLOCK_WAIT,NULL) )
	{
        if( result == DDERR_SURFACELOST )
        {
			m_pDisplay->GetFrontBuffer()->Restore();
			lpddsBack->Restore();
			result=lpddsBack->Lock(&rect,&ddsd,DDLOCK_NOSYSLOCK|DDLOCK_WAIT,NULL);
			if FAILED( result )
				return;
        }
		else
		{
			return;
		}
	}


    char* pSurface = (char*)ddsd.lpSurface;
	int lPitch = ddsd.lPitch;
	int lHalfPitch = lPitch;
	if (bDoubleSize == TRUE)
	{
		lPitch *= 2;
	}
	if( !m_bPowerOn || g_pBoard->GetAppleStatus() == ACS_POWEROFF )
	{
		memset( pSurface, 0, WIN_HEIGHT * lPitch );
		lpddsBack->Unlock( NULL );
		if ( Present() == DDERR_SURFACELOST )
		{
			Present();
			::Sleep(0);
		}
		return;
	}

	BYTE *pixelInfo;
	int colorDepth = m_iColorDepth >> 3;
	int x, y;
	typedef unsigned int TColorArr[16];		// color with 16 grade brightness
	TColorArr* colorTable = NULL;
	TColorArr* colorTableScanLine = NULL;
	//	_int64 color;
	int color = 0;
	int colorScanLine = 0;

	//------------------
	switch( m_nVideoMode )
	{
	case SM_COLOR:
		if (m_bTextMode == FALSE )
		{
			colorTable = m_auColorTableByHSB;
			colorTableScanLine = m_auColorTableByHSBScanLine;
		}
		else
		{
			color = m_auColorTableByHSB[15][15];
			colorScanLine = m_auColorTableByHSBScanLine[15][15];
		}
		break;
	case SM_COLOR2:
		if (m_bTextMode == FALSE )
		{
			colorTable = m_auColorTable;
			colorTableScanLine = m_auColorTableScanLine;
		}
		else
		{
			color = m_auColorTable[15][15];
			colorScanLine = m_auColorTableScanLine[15][15];
		}
		break;
	case SM_WHITE:
		color = m_uWhite;
		colorScanLine = m_uWhiteScanLine;
		break;
	case SM_GREEN:
	default:
		color = m_uGreen;
		colorScanLine = m_uGreenScanLine;
		break;
	}

	DWORD lastLineColor[WIN_WIDTH * 2];
	memset(lastLineColor, 0, sizeof(lastLineColor));

	memset(pSurface, 0, WIN_TOP_MARGIN * lPitch);
	memset(pSurface + (WIN_TOP_MARGIN + 192 * 2) * lPitch, 0, WIN_BOTTOM_MARGIN * lPitch);

	for( y = 0; y < 192; y++ )
	{
		// draw pixelInfo to surface
		char* surface;
		//if ( m_bWindowed )
			surface = pSurface+(y*2+WIN_TOP_MARGIN)*lPitch;
		//else
		//	surface = pSurface+(y*2+(int)(FULL_HEIGHT-WIN_HEIGHT)/2)*lPitch
			/* + (int)((FULL_WIDTH-WIN_WIDTH)*FULL_BPP/16 )*/;
		pixelInfo = m_pixelInfo[y] + 2;
		
		BYTE colorIndex = 0;

		DWORD curColor, curColor2;
		DWORD lastColor = 0, lastColor2 = 0;
		DWORD colorHalf, colorHalf2;
		DWORD colorMix, colorMix2;
		int x2;

		unsigned int brightness;
		colorIndex = pixelInfo[0];

		brightness = !!(pixelInfo[0] & 0x0f);

		x2 = 0;
		for (x = 0; x < WIN_WIDTH; x++)
		{
			if (colorTable != NULL)
			{
				brightness = ( brightness << 1 ) & 0x0f;
				if (pixelInfo[x + 1] & 0x0f)
					brightness |= 1;
				colorIndex = ( pixelInfo[x - 2]
							+ pixelInfo[x - 1]
							+ pixelInfo[x]
							+ pixelInfo[x + 1] ) & 0x0f;

				curColor = colorTable[colorIndex][brightness];
				curColor2 = colorTableScanLine[colorIndex][brightness];
			}
			else
			{
				if (pixelInfo[x] == 0)
				{
					curColor = 0;
					curColor2 = 0;
				}
				else
				{
					curColor = color;
					curColor2 = colorScanLine;
				}
			}
			if (m_bScanline == FALSE)
			{
				curColor2 = curColor;
			}

			if (bDoubleSize == TRUE)
			{
				colorHalf = (curColor & m_dwColorHalfMask) >> 1;
				colorHalf2 = (curColor2 & m_dwColorHalfMask) >> 1;

				colorMix = colorHalf + lastColor;
				colorMix2 = colorHalf2 + lastColor2;
				*(DWORD*)surface = colorMix;
				*(DWORD*)(surface + lPitch) = colorMix2;

				colorMix = (colorMix & m_dwColorHalfMask) >> 1;
				colorMix2 = (colorMix2 & m_dwColorHalfMask) >> 1;
				*(DWORD*)(surface - lHalfPitch) = lastLineColor[x2] + colorMix;
				*(DWORD*)(surface + lHalfPitch) = colorMix + colorMix2;
				lastLineColor[x2] = colorMix2;
				x2++;

				surface += colorDepth;

				*(DWORD*)(surface - lHalfPitch) = lastLineColor[x2] + colorHalf;
				*(DWORD*)(surface + lHalfPitch) = colorHalf + colorHalf2;
				lastLineColor[x2] = colorHalf2;
				x2++;

				lastColor = colorHalf;
				lastColor2 = colorHalf2;
			}
			*(DWORD*)surface = curColor;
			*(DWORD*)(surface + lPitch) = curColor2;

			surface += colorDepth;
		}
	}
	
	lpddsBack->Unlock(NULL);

//	if ( bScreenDirty )
	{
		if ( Present() == DDERR_SURFACELOST )
			Present();
		::Sleep(0);
	}

}


BOOL CScreen::InitDirectX()
{
	TRACE("Init start\n");
    LPDIRECTDRAWPALETTE pDDPal = NULL; 
    HRESULT	hr;

//	m_bInitializing = TRUE;		// It will cause WM_DISPLAY_CHANGE message,
								// but should not recreate direct draw
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

	CMainFrame* pParent = (CMainFrame*)g_pBoard->m_lpwndMainFrame;

	SAFE_RELEASE( m_pSurfaceMain );
	SAFE_RELEASE( m_pSurfaceDisk );
	SAFE_RELEASE( m_pSurfaceMsg );
//	SAFE_RELEASE( m_pDisplay );
//	m_pDisplay = new CDisplay();

	if ( m_bWindowed )
	{
		hr = m_pDisplay->CreateWindowedDisplay(m_hWnd, WIN_WIDTH, WIN_HEIGHT);
		if(hr!=DD_OK)
		{
			AfxMessageBox(DDErrorString(hr));
//			m_bInitializing = FALSE;
			return hr;
		}
		m_pDisplay->Clear();
		if (m_bMouseCapture == FALSE)
		{
			g_cDIMouse.SetActive(FALSE, FALSE);
		}
	}
	else
	{
		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
		int nWinWidth;
		int nWinHeight;
		// 화면 크기 90%로 비율 유지해서 중앙에 표시.
		nWinHeight = nScreenHeight * 9 / 10;	// 90%
		nWinWidth = nWinHeight * WIN_WIDTH / WIN_HEIGHT;
		if ( nWinWidth > nScreenWidth * 9 / 10 )
		{
			nWinWidth = nScreenWidth * 9 / 10; // 90%
			nWinHeight = nWinWidth * WIN_HEIGHT / WIN_WIDTH;
		}
		this->m_stMainRect.left = ( nScreenWidth - nWinWidth ) / 2;
		this->m_stMainRect.top = ( nScreenHeight - nWinHeight ) / 2;
		this->m_stMainRect.right = this->m_stMainRect.left + nWinWidth;
		this->m_stMainRect.bottom = this->m_stMainRect.top + nWinHeight;

		hr = m_pDisplay->CreateFullScreenDisplay( pParent->m_hWnd,
												FULL_WIDTH, FULL_HEIGHT, FULL_BPP );
		if(hr!=DD_OK)
		{
			AfxMessageBox(DDErrorString(hr));
			return hr;
		}

		m_pDisplay->Clear();

		// create disk status display surface and redraw
		m_pDisplay->CreateSurfaceFromBitmap( &m_pSurfaceDisk, MAKEINTRESOURCE(IDB_DISK),
			DISK_VIEW_WIDTH, DISK_VIEW_HEIGHT );
		m_pSurfaceDisk->Clear();
		
		// create message display surface.
		m_pDisplay->CreateSurface( &m_pSurfaceMsg, MSG_VIEW_WIDTH, MSG_VIEW_HEIGHT );
		m_pDisplay->Clear();
		
		SetMessage("To return to windowed mode, press \"LEFT CTRL+LEFT ALT\" or \"CTRL+F8\" key.");
		pParent->m_wndStatusBar.SetDiskStatus( -1, 0 );	// redraw disk status
		

		g_cDIKeyboard.SetActive(TRUE);
		g_cDIMouse.SetActive(TRUE);
	}

	if (m_bDoubleSize || !m_bWindowed)
	{
		m_pDisplay->CreateSurface(&m_pSurfaceMain, WIN_WIDTH * 2, WIN_HEIGHT * 2);
	}
	else
	{
		m_pDisplay->CreateSurface(&m_pSurfaceMain, WIN_WIDTH, WIN_HEIGHT);
	}

	m_pSurfaceMain->Clear();
	
	ApplyColors();
//	m_bInitializing = FALSE;
	TRACE("Init end\n");
	return TRUE;
	
}

char* CScreen::DDErrorString(HRESULT hr)
{
	switch (hr)
	{
	case DDERR_ALREADYINITIALIZED:           return "DDERR_ALREADYINITIALIZED";
	case DDERR_CANNOTATTACHSURFACE:          return "DDERR_CANNOTATTACHSURFACE";
	case DDERR_CANNOTDETACHSURFACE:          return "DDERR_CANNOTDETACHSURFACE";
	case DDERR_CURRENTLYNOTAVAIL:            return "DDERR_CURRENTLYNOTAVAIL";
	case DDERR_EXCEPTION:                    return "DDERR_EXCEPTION";
	case DDERR_GENERIC:                      return "DDERR_GENERIC";
	case DDERR_HEIGHTALIGN:                  return "DDERR_HEIGHTALIGN";
	case DDERR_INCOMPATIBLEPRIMARY:          return "DDERR_INCOMPATIBLEPRIMARY";
	case DDERR_INVALIDCAPS:                  return "DDERR_INVALIDCAPS";
	case DDERR_INVALIDCLIPLIST:              return "DDERR_INVALIDCLIPLIST";
	case DDERR_INVALIDMODE:                  return "DDERR_INVALIDMODE";
	case DDERR_INVALIDOBJECT:                return "DDERR_INVALIDOBJECT";
	case DDERR_INVALIDPARAMS:                return "DDERR_INVALIDPARAMS";
	case DDERR_INVALIDPIXELFORMAT:           return "DDERR_INVALIDPIXELFORMAT";
	case DDERR_INVALIDRECT:                  return "DDERR_INVALIDRECT";
	case DDERR_LOCKEDSURFACES:               return "DDERR_LOCKEDSURFACES";
	case DDERR_NO3D:                         return "DDERR_NO3D";
	case DDERR_NOALPHAHW:                    return "DDERR_NOALPHAHW";
	case DDERR_NOCLIPLIST:                   return "DDERR_NOCLIPLIST";
	case DDERR_NOCOLORCONVHW:                return "DDERR_NOCOLORCONVHW";
	case DDERR_NOCOOPERATIVELEVELSET:        return "DDERR_NOCOOPERATIVELEVELSET";
	case DDERR_NOCOLORKEY:                   return "DDERR_NOCOLORKEY";
	case DDERR_NOCOLORKEYHW:                 return "DDERR_NOCOLORKEYHW";
	case DDERR_NODIRECTDRAWSUPPORT:          return "DDERR_NODIRECTDRAWSUPPORT";
	case DDERR_NOEXCLUSIVEMODE:              return "DDERR_NOEXCLUSIVEMODE";
	case DDERR_NOFLIPHW:                     return "DDERR_NOFLIPHW";
	case DDERR_NOGDI:                        return "DDERR_NOGDI";
	case DDERR_NOMIRRORHW:                   return "DDERR_NOMIRRORHW";
	case DDERR_NOTFOUND:                     return "DDERR_NOTFOUND";
	case DDERR_NOOVERLAYHW:                  return "DDERR_NOOVERLAYHW";
	case DDERR_NORASTEROPHW:                 return "DDERR_NORASTEROPHW";
	case DDERR_NOROTATIONHW:                 return "DDERR_NOROTATIONHW";
	case DDERR_NOSTRETCHHW:                  return "DDERR_NOSTRETCHHW";
	case DDERR_NOT4BITCOLOR:                 return "DDERR_NOT4BITCOLOR";
	case DDERR_NOT4BITCOLORINDEX:            return "DDERR_NOT4BITCOLORINDEX";
	case DDERR_NOT8BITCOLOR:                 return "DDERR_NOT8BITCOLOR";
	case DDERR_NOTEXTUREHW:                  return "DDERR_NOTEXTUREHW";
	case DDERR_NOVSYNCHW:                    return "DDERR_NOVSYNCHW";
	case DDERR_NOZBUFFERHW:                  return "DDERR_NOZBUFFERHW";
	case DDERR_NOZOVERLAYHW:                 return "DDERR_NOZOVERLAYHW";
	case DDERR_OUTOFCAPS:                    return "DDERR_OUTOFCAPS";
	case DDERR_OUTOFMEMORY:                  return "DDERR_OUTOFMEMORY";
	case DDERR_OUTOFVIDEOMEMORY:             return "DDERR_OUTOFVIDEOMEMORY";
	case DDERR_OVERLAYCANTCLIP:              return "DDERR_OVERLAYCANTCLIP";
	case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
	case DDERR_PALETTEBUSY:                  return "DDERR_PALETTEBUSY";
	case DDERR_COLORKEYNOTSET:               return "DDERR_COLORKEYNOTSET";
	case DDERR_SURFACEALREADYATTACHED:       return "DDERR_SURFACEALREADYATTACHED";
	case DDERR_SURFACEALREADYDEPENDENT:      return "DDERR_SURFACEALREADYDEPENDENT";
	case DDERR_SURFACEBUSY:                  return "DDERR_SURFACEBUSY";
	case DDERR_CANTLOCKSURFACE:              return "DDERR_CANTLOCKSURFACE";
	case DDERR_SURFACEISOBSCURED:            return "DDERR_SURFACEISOBSCURED";
	case DDERR_SURFACELOST:                  return "DDERR_SURFACELOST";
	case DDERR_SURFACENOTATTACHED:           return "DDERR_SURFACENOTATTACHED";
	case DDERR_TOOBIGHEIGHT:                 return "DDERR_TOOBIGHEIGHT";
	case DDERR_TOOBIGSIZE:                   return "DDERR_TOOBIGSIZE";
	case DDERR_TOOBIGWIDTH:                  return "DDERR_TOOBIGWIDTH";
	case DDERR_UNSUPPORTED:                  return "DDERR_UNSUPPORTED";
	case DDERR_UNSUPPORTEDFORMAT:            return "DDERR_UNSUPPORTEDFORMAT";
	case DDERR_UNSUPPORTEDMASK:              return "DDERR_UNSUPPORTEDMASK";
	case DDERR_VERTICALBLANKINPROGRESS:      return "DDERR_VERTICALBLANKINPROGRESS";
	case DDERR_WASSTILLDRAWING:              return "DDERR_WASSTILLDRAWING";
	case DDERR_XALIGN:                       return "DDERR_XALIGN";
	case DDERR_INVALIDDIRECTDRAWGUID:        return "DDERR_INVALIDDIRECTDRAWGUID";
	case DDERR_DIRECTDRAWALREADYCREATED:     return "DDERR_DIRECTDRAWALREADYCREATED";
	case DDERR_NODIRECTDRAWHW:               return "DDERR_NODIRECTDRAWHW";
	case DDERR_PRIMARYSURFACEALREADYEXISTS:  return "DDERR_PRIMARYSURFACEALREADYEXISTS";
	case DDERR_NOEMULATION:                  return "DDERR_NOEMULATION";
	case DDERR_REGIONTOOSMALL:               return "DDERR_REGIONTOOSMALL";
	case DDERR_CLIPPERISUSINGHWND:           return "DDERR_CLIPPERISUSINGHWND";
	case DDERR_NOCLIPPERATTACHED:            return "DDERR_NOCLIPPERATTACHED";
	case DDERR_NOHWND:                       return "DDERR_NOHWND";
	case DDERR_HWNDSUBCLASSED:               return "DDERR_HWNDSUBCLASSED";
	case DDERR_HWNDALREADYSET:               return "DDERR_HWNDALREADYSET";
	case DDERR_NOPALETTEATTACHED:            return "DDERR_NOPALETTEATTACHED";
	case DDERR_NOPALETTEHW:                  return "DDERR_NOPALETTEHW";
	case DDERR_BLTFASTCANTCLIP:              return "DDERR_BLTFASTCANTCLIP";
	case DDERR_NOBLTHW:                      return "DDERR_NOBLTHW";
	case DDERR_NODDROPSHW:                   return "DDERR_NODDROPSHW";
	case DDERR_OVERLAYNOTVISIBLE:            return "DDERR_OVERLAYNOTVISIBLE";
	case DDERR_NOOVERLAYDEST:                return "DDERR_NOOVERLAYDEST";
	case DDERR_INVALIDPOSITION:              return "DDERR_INVALIDPOSITION";
	case DDERR_NOTAOVERLAYSURFACE:           return "DDERR_NOTAOVERLAYSURFACE";
	case DDERR_EXCLUSIVEMODEALREADYSET:      return "DDERR_EXCLUSIVEMODEALREADYSET";
	case DDERR_NOTFLIPPABLE:                 return "DDERR_NOTFLIPPABLE";
	case DDERR_CANTDUPLICATE:                return "DDERR_CANTDUPLICATE";
	case DDERR_NOTLOCKED:                    return "DDERR_NOTLOCKED";
	case DDERR_CANTCREATEDC:                 return "DDERR_CANTCREATEDC";
	case DDERR_NODC:                         return "DDERR_NODC";
	case DDERR_WRONGMODE:                    return "DDERR_WRONGMODE";
	case DDERR_IMPLICITLYCREATED:            return "DDERR_IMPLICITLYCREATED";
	case DDERR_NOTPALETTIZED:                return "DDERR_NOTPALETTIZED";
	case DDERR_UNSUPPORTEDMODE:              return "DDERR_UNSUPPORTEDMODE";
	case DDERR_NOMIPMAPHW:                   return "DDERR_NOMIPMAPHW";
	case DDERR_INVALIDSURFACETYPE:           return "DDERR_INVALIDSURFACETYPE";
	case DDERR_DCALREADYCREATED:             return "DDERR_DCALREADYCREATED";
	case DDERR_CANTPAGELOCK:                 return "DDERR_CANTPAGELOCK";
	case DDERR_CANTPAGEUNLOCK:               return "DDERR_CANTPAGEUNLOCK";
	case DDERR_NOTPAGELOCKED:                return "DDERR_NOTPAGELOCKED";
	case DDERR_NOTINITIALIZED:               return "DDERR_NOTINITIALIZED";
	}
	return "Unknown Error";
}


int CScreen::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	CMainFrame* pParent = g_pBoard->m_lpwndMainFrame;
	InitDirectX();	
	SetTimer(1, 1000, NULL);
	// TODO: Add your specialized creation code here
	return 0;
}

void CScreen::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	// measure clock speed
	{
		CMainFrame* pParent = g_pBoard->m_lpwndMainFrame;
		pParent->m_wndStatusBar.SetFrame(m_dFrameRate);
		pParent->m_wndStatusBar.SetSpeed(g_pBoard->m_dClockSpeed);
		switch( m_nMsgVisiable )
		{
		case 0:
			break;
		case 1:
			HideMessage();
			break;
		default:
			m_nMsgVisiable--;
			break;
		}
	}
	CWnd::OnTimer(nIDEvent);
}

void CScreen::OnDebug()
{
}

BOOL CScreen::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	//	return CWnd::OnEraseBkgnd(pDC);
}

void CScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
//	int status = g_pBoard->GetAppleStatus();
	if ( Present() == DDERR_SURFACELOST )
		Present();
	// Do not call CWnd::OnPaint() for painting messages
}

HRESULT CScreen::Present()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );

    HRESULT hr;
	LPDIRECTDRAWSURFACE7 lpdds;

    if( NULL == m_pDisplay->GetFrontBuffer() || NULL == m_pDisplay->GetBackBuffer() || m_pSurfaceMain == NULL )
        return E_POINTER;

    while( 1 )
    {
        if( m_bWindowed )
		{
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			POINT p1={rect.left, rect.top};
			POINT p2={rect.right, rect.bottom};
			::ClientToScreen(m_hWnd, &p1);
			::ClientToScreen(m_hWnd, &p2);
			rect.left = p1.x;
			rect.top = p1.y;
			rect.right = p2.x;
			rect.bottom = p2.y;
			
			lpdds = m_pSurfaceMain->GetDDrawSurface();
			if ( lpdds == NULL )
			{
				return E_FAIL;
			}
            hr = m_pDisplay->GetFrontBuffer()->Blt( &rect, lpdds, NULL, DDBLT_WAIT, NULL );
		}
        else
		{
			lpdds = m_pSurfaceMain->GetDDrawSurface();
			if ( lpdds == NULL )
			{
				return E_FAIL;
			}
			m_pDisplay->Clear();
            m_pDisplay->StretchBlt( &this->m_stMainRect, lpdds, NULL, DDBLT_WAIT, NULL );
			m_pDisplay->Blt( m_stMainRect.left, m_stMainRect.bottom + 4, m_pSurfaceMsg, NULL );
			m_pDisplay->Blt( m_stMainRect.right - 70, m_stMainRect.bottom + 4, m_pSurfaceDisk, NULL );

			hr = m_pDisplay->GetFrontBuffer()->Flip( NULL, 0 );
		}
		
        if( hr == DDERR_SURFACELOST )
        {
            m_pDisplay->GetFrontBuffer()->Restore();
            m_pDisplay->GetBackBuffer()->Restore();
        }
		
        if( hr != DDERR_WASSTILLDRAWING )
            return hr;
    }
}

BYTE CScreen::ChangeMode(WORD addr)
{
	switch(addr){
	case TXTCLR:
		m_iScrMode &= ~SS_TEXT;
		break;
	case TXTSET:
		m_iScrMode |= SS_TEXT;
		break;
	case MIXCLR:
		m_iScrMode &= ~SS_MIXED;
		break;
	case MIXSET:
		m_iScrMode |= SS_MIXED;
		break;
	case LOWSCR:
		m_iScrMode &= ~SS_PAGE2;
		break;
	case HISCR:
//		if ( !( m_iScrMode & SS_80COL ) )
		m_iScrMode |= SS_PAGE2;
		break;
	case LOWRES:
		m_iScrMode &= ~SS_HIRES;
		break;
	case HIRES:
		m_iScrMode |= SS_HIRES;
		break;
	case SETDHIRES:
		m_iScrMode |= SS_DHIRES;
		break;
	case CLRDHIRES:
		m_iScrMode &= ~SS_DHIRES;
		break;
	case SET80VID:
		m_iScrMode |= SS_80COL;
//		m_iScrMode &= ~SS_PAGE2;
		break;
	case CLR80VID:
		m_iScrMode &= ~SS_80COL;
		break;
	case SETALTCHAR:
		m_iScrMode |= SS_ALTCHAR;
		break;
	case CLRALTCHAR:
		m_iScrMode &= ~SS_ALTCHAR;
		break;
	}
	return 0x00;
}

void CScreen::setLookUp(BYTE *pMemory)
{
	int i,j;

	for(i=0;i<192;i++){
		m_awScanOT[i]=(i&7)*0x400 + ((i>>3)&7)*0x80 + (i>>6)*0x28;
		for(int j=0; j < 40; j++){
			m_aiScreenTable[m_awScanOT[i]+j][1] = i;
			m_aiScreenTable[m_awScanOT[i]+j][0] = j;
		}
	}
	
	for(i=0;i<192;i++){
		// page1, graphics = 0 
		m_apbScanAT[0][i]=m_awScanOT[i]+pMemory+0x2000;
		// page1, text
		// SS_TEXT, SS_TEXT|SS_MIXED
		m_apbScanAT[2][i]=m_apbScanAT[3][i]=m_awScanOT[i&~7]+pMemory+0x400;
		// page1, mixed
		// SS_MIXED
		m_apbScanAT[1][i]=m_apbScanAT[i>159?2:0][i];
		
		// page2, graphic
		// SS_PAGE2
		m_apbScanAT[4][i]=m_apbScanAT[0][i]+0x2000;
		// page2, text
		// SS_PAGE2|SS_TEXT, SS_PAGE2|SS_TEXT|SS_MIXED
		m_apbScanAT[6][i]=m_apbScanAT[7][i]=m_apbScanAT[2][i]+0x400;
		// page2, mixed
		// SS_PAGE2|SS_MIXED
		m_apbScanAT[5][i]=m_apbScanAT[i>159?6:4][i];
	}
	
	for(i=0; i<3; i++)
	{
		for(j=0; j<40; j++)
			m_abPosTable[i*40+j] = (BYTE)(i<<6);
	}
}

int CScreen::GetMonitorType()
{
	return m_nVideoMode;
}

void CScreen::ChangeMonitorType()
{
	m_nVideoMode = (m_nVideoMode+1)&0x03;
}

void CScreen::ChangeMonitorType( int type )
{
	m_nVideoMode = type & 0x03;
}

sYIQ CScreen::Rgb2ntsc( unsigned int rgb32 )
{
    int r, g, b;
    sYIQ yiq;

    r = ( rgb32 >> 16 ) & 0xFF;
    g = ( rgb32 >> 8 ) & 0xFF;
    b = rgb32 & 0xFF;

    yiq.y = (int)( .299 * r + .587 * g + .114 * b );
    yiq.i = (int)( .596 * r - .275 * g - .321 * b );
    yiq.q = (int)( .212 * r - .523 * g + .311 * b );

    return yiq;
}

unsigned int CScreen::Ntsc2rgb( sYIQ* yiq )
{
    int r, g, b;

    r = (int)( yiq->y + .956 * yiq->i + .620 * yiq->q );
    g = (int)( yiq->y - .272 * yiq->i - .647 * yiq->q );
    b = (int)( yiq->y - 1.108 * yiq->i + 1.705 * yiq->q );
    if ( r > 255 ) r = 255;
    else if ( r < 0 ) r = 0;
    if ( g > 255 ) g = 255;
    else if ( g < 0 ) g = 0;
    if ( b > 255 ) b = 255;
    else if ( b < 0 ) b = 0;

    return (r<<16)|(g<<8)|b;
}

sYIQ CScreen::ComposeYIQ( sYIQ* yiq1, sYIQ* yiq2 )
{
	sYIQ yiq;
	yiq.y = yiq1->y + yiq2->y;
	yiq.i = yiq1->i + yiq2->i;
	yiq.q = yiq1->q + yiq2->q;
	return yiq;
}

// bright affection by pixel distance
#define BRIGHT_BASE		(1.0/((1+BRIGHT_8)*(1+BRIGHT_4)*(1+BRIGHT_2)*(1+BRIGHT_1)))
#define BRIGHT_8		.00		// 2 left pixel
#define BRIGHT_4		.05		// left pixel
#define BRIGHT_2		.15		// target pixel
#define BRIGHT_1		.05		// right pixel
#define BRIGHT(a,b,c,d)	(BRIGHT_BASE*(1+BRIGHT_8*a)*(1+BRIGHT_4*b)*(1+BRIGHT_2*c)*(1+BRIGHT_1*d))
void CScreen::ApplyColors()
{
	int i, j;
	
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );	
	
	DDPIXELFORMAT DDpf;

	static const double bright[16] = {
		.00,
		BRIGHT(0,0,0,1),
		BRIGHT(0,0,1,0),
		BRIGHT(0,0,1,1),
		BRIGHT(0,1,0,0),
		BRIGHT(0,1,0,1),
		BRIGHT(0,1,1,0),
		BRIGHT(0,1,1,1),
		BRIGHT(1,0,0,0),
		BRIGHT(1,0,0,1),
		BRIGHT(1,0,1,0),
		BRIGHT(1,0,1,1),
		BRIGHT(1,1,0,0),
		BRIGHT(1,1,0,1),
		BRIGHT(1,1,1,0),
		BRIGHT(1,1,1,1)
	};
	DDpf.dwSize = sizeof(DDPIXELFORMAT);
	m_pDisplay->GetFrontBuffer()->GetPixelFormat(&DDpf);
	
	m_iColorDepth = DDpf.dwRGBBitCount;

	m_dwColorHalfMask = ~((DDpf.dwRBitMask & ~(DDpf.dwRBitMask << 1)) | (DDpf.dwGBitMask & ~(DDpf.dwGBitMask << 1)) | (DDpf.dwBBitMask & ~(DDpf.dwBBitMask << 1)));

	for( i = 0; i < 16; i++ )
	{
		for(j = 0; j < 16; j++)
		{
			m_auColorTableByHSB[i][j] = ApplyDarkRGBFormat(m_auRGBColorByHSB[i], &DDpf, bright[j]);
			m_auColorTable[i][j] = ApplyDarkRGBFormat(m_auRGBColor[i], &DDpf, bright[j]);
			m_auColorTableByHSBScanLine[i][j] = ApplyDarkRGBFormat(m_auRGBColorByHSB[i], &DDpf, bright[j] * .7);
			m_auColorTableScanLine[i][j] = ApplyDarkRGBFormat(m_auRGBColor[i], &DDpf, bright[j] * .7);
		}
	}

	m_uWhite = ApplyRGBFormat( m_uRGBMono, &DDpf );
	m_uWhiteScanLine = ApplyDarkRGBFormat( m_uRGBMono, &DDpf, .5 );
	m_uGreen = ApplyRGBFormat( m_uRGBGreen, &DDpf );
	m_uGreenScanLine = ApplyDarkRGBFormat( m_uRGBGreen, &DDpf, .5 );
	
	Redraw();
}

void CScreen::SetDefaultColors()
{
	m_uRGBGreen = RGB24( 64, 255, 64 );
	m_uRGBMono = RGB24( 255, 255, 255 );
	SetHSB( HSB(0, 72, 72) );
	for( int i = 0; i < 16; i++ )
	{
		m_auRGBColor[i] = m_auRGBColorByHSB[i];
	}
	/*
	m_auRGBColor[ 0] = RGB24(   0,   0,   0 );
	m_auRGBColor[ 1] = RGB24( 124,  32,  64 );
	m_auRGBColor[ 2] = RGB24(  64,  48, 124 );
	m_auRGBColor[ 3] = RGB24( 188,  80, 188 );
	m_auRGBColor[ 4] = RGB24(   0,  92,  60 );
	m_auRGBColor[ 5] = RGB24( 124, 124, 124 );
	m_auRGBColor[ 6] = RGB24(  64, 140, 184 );
	m_auRGBColor[ 7] = RGB24( 188, 172, 248 );
	m_auRGBColor[ 8] = RGB24(  60,  76,   0 );
	m_auRGBColor[ 9] = RGB24( 184, 108,  64 );
	m_auRGBColor[10] = RGB24( 124, 124, 124 );
	m_auRGBColor[11] = RGB24( 248, 156, 188 );
	m_auRGBColor[12] = RGB24(  60, 168,  60 );
	m_auRGBColor[13] = RGB24( 184, 200, 124 );
	m_auRGBColor[14] = RGB24( 124, 216, 184 );
	m_auRGBColor[15] = RGB24( 248, 248, 248 );
	*/
}

unsigned int CScreen::GetHSB()
{
	return m_uHSB;
}

BYTE CScreen::CheckMode(WORD addr)
{
	BYTE mode = 0;
	switch( addr )
	{
	case RDTEXT:
		if ( m_iScrMode & SS_TEXT )
			mode = 0x80;
		break;
	case RDMIXED:
		if ( m_iScrMode & SS_MIXED )
			mode = 0x80;
		break;
	case RDPAGE2:
		if ( m_iScrMode & SS_PAGE2 )
			mode = 0x80;
		break;
	case RDHIRES:
		if ( m_iScrMode & SS_HIRES )
			mode = 0x80;
		break;
	case RD80STORE:
		if ( m_iScrMode & SS_80STORE )
			mode = 0x80;
		break;
	case RDVBLBAR:
		if ( !IsVBL() )		// not VBL, but VBL_BAR ( when Virtical Blank, it goes low )
			mode = 0x80;
		break;
	case RD80COL:
		if ( m_iScrMode & SS_80COL )
			mode = 0x80;
		break;
	case RDALTCHAR:
		if ( m_iScrMode & SS_ALTCHAR )
			mode = 0x80;
		break;
	}
	return mode;
}

void CScreen::Set80Store()
{
	m_iScrMode |= SS_80STORE;
}

void CScreen::Clr80Store()
{
	m_iScrMode &= ~SS_80STORE;
}

void CScreen::ReInitialize()
{
	HRESULT hr;
	
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
    // Check the cooperative level before rendering
    if( FAILED( hr = m_pDisplay->GetDirectDraw()->TestCooperativeLevel() ) )
    {
		switch( hr )
		{
		case DDERR_EXCLUSIVEMODEALREADYSET:
		case DDERR_NOEXCLUSIVEMODE:
			// Do nothing because some other app has exclusive mode
//			Sleep(10);
			break;
		
		case DDERR_WRONGMODE:
			// The display mode changed on us. Update the
			// DirectDraw surfaces accordingly
			m_bWindowed = TRUE;			// is not exclusive mode
			TRACE("WRONGMODE\n");

			g_cDIKeyboard.SetActive(FALSE, FALSE);
			g_cDIMouse.SetActive(FALSE, FALSE);
			InitDirectX();
			break;
		default:
			break;
        }
    }
	memset( m_pixelInfo, 0, sizeof(m_pixelInfo) );
}

unsigned int CScreen::ApplyRGBFormat(unsigned int rgb32, LPDDPIXELFORMAT lpDDpf)
{
	unsigned int r = rgb32 & 0xFF0000;
	unsigned int g = rgb32 & 0x00FF00;
	unsigned int b = rgb32 & 0x0000FF;
	DWORD testbit;
	testbit = 0x800000;
	while ( testbit && !( lpDDpf->dwRBitMask & testbit ) )
	{
		testbit >>= 1;
		r >>= 1;
	}
	r &= lpDDpf->dwRBitMask;

	testbit = 0x008000;
	while ( testbit && !( lpDDpf->dwGBitMask & testbit ) )
	{
		testbit >>= 1;
		g >>= 1;
	}
	g &= lpDDpf->dwGBitMask;

	testbit = 0x000080;
	while ( testbit && !( lpDDpf->dwBBitMask & testbit ) )
	{
		testbit >>= 1;
		b >>= 1;
	}
	b &= lpDDpf->dwBBitMask;
	return( r | g | b );
}

unsigned int CScreen::ApplyDarkRGBFormat(unsigned int rgb32, LPDDPIXELFORMAT lpDDpf, double rate)
{
	unsigned int r = rgb32 & 0xFF0000;
	unsigned int g = rgb32 & 0x00FF00;
	unsigned int b = rgb32 & 0x0000FF;
	sYIQ yiq;
	DWORD testbit;
	if ( rate <= 0 )
	{
		return 0;
	}
	if ( rate > 1 )
	{
		rate = 1;
	}

	yiq = Rgb2ntsc( rgb32 );
	yiq.y = (int)( yiq.y * rate );

	rgb32 = Ntsc2rgb( &yiq );

	testbit = 0x800000;
	while ( testbit && !( lpDDpf->dwRBitMask & testbit ) )
	{
		testbit >>= 1;
		r >>= 1;
	}
	r = (unsigned int)( r * rate );
	r &= lpDDpf->dwRBitMask;
	
	testbit = 0x008000;
	while ( testbit && !( lpDDpf->dwGBitMask & testbit ) )
	{
		testbit >>= 1;
		g >>= 1;
	}
	g = (unsigned int)( g * rate );
	g &= lpDDpf->dwGBitMask;
	
	testbit = 0x000080;
	while ( testbit && !( lpDDpf->dwBBitMask & testbit ) )
	{
		testbit >>= 1;
		b >>= 1;
	}
	b = (unsigned int)( b * rate );
	b &= lpDDpf->dwBBitMask;
	return( r | g | b );
}

void CScreen::Reset()
{
	m_iScrMode = SS_TEXT;
}

void CScreen::CaptureInput(BOOL bMouseCapture)
{
	g_cDIKeyboard.SetActive(TRUE, FALSE);
	if (bMouseCapture == TRUE)
	{
		g_cDIMouse.SetActive(TRUE, FALSE);
		SetMessage("press \"LEFT CTRL+ALT\" key to release mouse and keyboard");
	}
	else
	{
		SetMessage("press \"LEFT CTRL+ALT\" key to release keyboard");
	}
}

void CScreen::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CaptureInput(m_bMouseCapture);

	CWnd::OnLButtonUp(nFlags, point);
}

void CScreen::OnMove(int x, int y) 
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	CWnd::OnMove(x, y);
	// TODO: Add your message handler code here
}

void CScreen::SetScreenMode(BOOL bFullScreen, BOOL bDoubleSize)
{
	BOOL bWindowed = !bFullScreen;

	if ( m_bWindowed != bWindowed || m_bDoubleSize != bDoubleSize)
	{
		BOOL suspended = g_pBoard->GetIsSuspended();
		if ( !suspended )
			g_pBoard->Suspend(TRUE);
		CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
		m_bWindowed = bWindowed;
		m_bDoubleSize = bDoubleSize;
		InitDirectX();
		if ( !suspended )
			g_pBoard->Resume();
	}
}

CSurface* CScreen::GetDiskSurface()
{
	return m_pSurfaceDisk;
}

void CScreen::UpdateDiskSurface()
{
	//Present();
}

void CScreen::SetMessage(TCHAR *szText)
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
	if ( szText != NULL )
		m_szMessage = szText;
	if ( m_bWindowed )
	{
	    g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetMessage( m_szMessage );
	}
	else
	{
	    if ( !m_pSurfaceMsg )
			return;
	    m_pSurfaceMsg->Clear();
	    if ( m_hFont != NULL )
	    {
			m_pSurfaceMsg->DrawText( m_hFont, m_szMessage, 0, 0, COLORREF(0), COLORREF(0xC0C0C0) );
	    }
	    //m_pDisplay->Blt( 0, 460, m_pSurfaceMsg, NULL );
//	    if ( !m_bInitializing )
	    Present();
	}
	m_nMsgVisiable = 5;
}

void CScreen::ToggleMessage()
{
	if ( m_nMsgVisiable )
		HideMessage();
	else
		SetMessage(NULL);
}

void CScreen::HideMessage()
{
	CLockMgr<CCSWrapper> guard( m_Lock, TRUE );
    if ( m_bWindowed )
	{
		g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetMessage( "" );
	}
	else
	{
		if ( !m_pSurfaceMsg )
			return;
		m_pSurfaceMsg->Clear();
		//m_pDisplay->Blt( 0, 460, m_pSurfaceMsg, NULL );
		Present();
	}
	m_nMsgVisiable = 0;
}

// OnDisplayChange의 요청에 의해 DirectDrawBuffer에 lock이 걸리지
// 않은 상태로 이 함수가 실행 되어 dead lock을 방지한다.
LRESULT CScreen::OnDisplayChanged(WPARAM wParam, LPARAM lParam)
{
	g_pBoard->m_pScreen->ReInitialize();
	return 0;
}

// WM_DISPLAYCHANGE 메시지가 발생할 때는 DirectDrawBuffer에 lock이
// 걸린 상태로 넘어오므로 이 다음에 CScreen의 lock이 걸리면
// dead lock이 걸릴 수 있다. 또한 window 관련 처리를 위해
// main thread 에서 이에 대한 처리를 한다.
LRESULT CScreen::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
	::PostMessage( m_hWnd, UM_DISPLAY_CHANGE, wParam, lParam );
	return 0;
}

unsigned int* CScreen::GetColors()
{
	return m_auRGBColor;
}

unsigned int* CScreen::GetColorsByHSB()
{
	return m_auRGBColorByHSB;
}

#define HUE_PHASE1	(-30)
#define HUE_PHASE2	(-40)
#define SAT_RATIO	(0.7)
#define BRT_RATIO	(0.9)

void CScreen::SetHSB(unsigned int uHSB)
{
	int i;
	int nHue, nSaturation, nBright;
	sYIQ yiq[16];
	
	m_uHSB = uHSB;
	nHue = m_uHSB >> 16;
	nSaturation = ( m_uHSB >> 8 ) & 0xFF;
	nBright = m_uHSB & 0xFF;

	yiq[ 1].y = (int)( nBright * BRT_RATIO );
	yiq[ 8].y = nBright;
	yiq[ 4].y = (int)( nBright * BRT_RATIO );
	yiq[ 2].y = nBright;
	yiq[ 1].i = (int)(  nSaturation * cos( (nHue    +HUE_PHASE1) * ( 2 * PI ) / 360 ) );
	yiq[ 8].i = (int)(  nSaturation * cos( (nHue+ 90+HUE_PHASE2) * ( 2 * PI ) / 360 ) * SAT_RATIO );
	yiq[ 4].i = (int)(  nSaturation * cos( (nHue+180+HUE_PHASE1) * ( 2 * PI ) / 360 ) );
	yiq[ 2].i = (int)(  nSaturation * cos( (nHue+270+HUE_PHASE2) * ( 2 * PI ) / 360 ) * SAT_RATIO );
	yiq[ 1].q = (int)( -nSaturation * sin( (nHue    +HUE_PHASE1) * ( 2 * PI ) / 360 ) );
	yiq[ 8].q = (int)( -nSaturation * sin( (nHue+ 90+HUE_PHASE2) * ( 2 * PI ) / 360 ) * SAT_RATIO );
	yiq[ 4].q = (int)( -nSaturation * sin( (nHue+180+HUE_PHASE1) * ( 2 * PI ) / 360 ) );
	yiq[ 2].q = (int)( -nSaturation * sin( (nHue+270+HUE_PHASE2) * ( 2 * PI ) / 360 ) * SAT_RATIO );

	yiq[ 0].y = 0;
	yiq[ 0].i = 0;
	yiq[ 0].q = 0;
	yiq[ 5] = ComposeYIQ( yiq+ 1, yiq+ 4 );
	yiq[10] = ComposeYIQ( yiq+ 2, yiq+ 8 );
	yiq[15] = ComposeYIQ( yiq+ 5, yiq+10 );
	
	yiq[ 9] = ComposeYIQ( yiq+ 1, yiq+ 8 );
	yiq[12] = ComposeYIQ( yiq+ 8, yiq+ 4 );
	yiq[ 6] = ComposeYIQ( yiq+ 4, yiq+ 2 );
	yiq[ 3] = ComposeYIQ( yiq+ 2, yiq+ 1 );
	
	yiq[11] = ComposeYIQ( yiq+10, yiq+ 1 );
	yiq[13] = ComposeYIQ( yiq+ 5, yiq+ 8 );
	yiq[14] = ComposeYIQ( yiq+10, yiq+ 4 );
	yiq[ 7] = ComposeYIQ( yiq+ 5, yiq+ 2 );
	
	for( i = 0; i < 16; i++ ){
		m_auRGBColorByHSB[i] = Ntsc2rgb( yiq + i );
	}
}

void CScreen::SetColors(unsigned int *pColors)
{
	int i;
	for( i = 0; i < 16; i++ )
		m_auRGBColor[i] = pColors[i];
}

unsigned int CScreen::GetMonoColor()
{
	return m_uRGBMono;
}

unsigned int CScreen::GetGreenColor()
{
	return m_uRGBGreen;
}

BOOL CScreen::GetScanline()
{
	return m_bScanline;
}

void CScreen::SetMonoColor(unsigned int uColor)
{
	m_uRGBMono = uColor;
}

void CScreen::SetGreenColor(unsigned int uColor)
{
	m_uRGBGreen = uColor;
}

void CScreen::SetScanline(BOOL bScanline)
{
	m_bScanline = bScanline;
}

void CScreen::Serialize(CArchive &ar)
{
	CObject::Serialize(ar);
	BOOL bDoubleSize;

	if ( ar.IsStoring() )
	{
		ar << m_iScrMode;
		ar << m_bPreview;
		ar << m_uRGBGreen;
		ar << m_uRGBMono;
		ar.Write( m_auRGBColor, sizeof(m_auRGBColor) );
		ar << m_uHSB;
		ar << m_nVideoMode;
		ar << m_bScanline;
		ar << m_dwClock;
		ar << m_dataLatch;
		ar << m_bDoubleSize;
	}
	else
	{
		ar >> m_iScrMode;
		ar >> m_bPreview;
		ar >> m_uRGBGreen;
		ar >> m_uRGBMono;	
		ar.Read( m_auRGBColor, sizeof(m_auRGBColor) );
		ar >> m_uHSB;
		ar >> m_nVideoMode;
		if ( g_nSerializeVer >= 4 )
		{
			ar >> m_bScanline;
		}
		if ( g_nSerializeVer >= 5 )
		{
			ar >> m_dwClock;
			ar >> m_dataLatch;
			m_nLine = m_dwClock / 65;
			m_nColumn = m_dwClock % 65;
		}
		if (g_nSerializeVer >= 9)
		{
			ar >> bDoubleSize;
		}
		SetHSB( m_uHSB );
		ApplyColors();

		SetScreenMode(!m_bWindowed, bDoubleSize);
	}
}

void CScreen::ClearBuffer()
{
	memset( m_pixelInfo, 0, sizeof(m_pixelInfo) );
}

void CScreen::SetMouseCapture(BOOL bCapture)
{
	m_bMouseCapture = bCapture;
}

void CScreen::Relax()
{
	m_bRelax = true;
}
