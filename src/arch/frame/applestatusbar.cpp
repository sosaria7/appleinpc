// AppleStatusBar.cpp : implementation file
//
#include "arch/frame/stdafx.h"
#include "arch/frame/aipc.h"
#include "arch/frame/mainfrm.h"
#include "arch/frame/applestatusbar.h"
#include "arch/frame/screen.h"
#include "appleclock.h"
#include "diskdrive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAppleClock* g_pBoard;

CALLBACK_HANDLER(OnDiskLightChange)
{
	g_pBoard->m_lpwndMainFrame->m_wndStatusBar.SetDiskStatus( (int)objTo, lParam );
}

/////////////////////////////////////////////////////////////////////////////
// CAppleStatusBar

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_DISK,
	ID_INDICATOR_CLOCK_SPEED,
	ID_INDICATOR_FRAME_SPEED,
	ID_INDICATOR_KEY_STATUS
};

CAppleStatusBar::CAppleStatusBar()
{
	VERIFY(m_bmDisk.LoadBitmap( IDB_DISK ));
	VERIFY(m_bmDiskRead.LoadBitmap( IDB_DISK_READ ));
	VERIFY(m_bmDiskWrite.LoadBitmap( IDB_DISK_WRITE ));
	VERIFY(m_bmDiskOff.LoadBitmap( IDB_DISK_OFF ));
	VERIFY(m_bmDiskRead_Full.LoadBitmap( IDB_DISK_READ_F ));
	VERIFY(m_bmDiskWrite_Full.LoadBitmap( IDB_DISK_WRITE_F ));
	VERIFY(m_bmDiskOff_Full.LoadBitmap( IDB_DISK_OFF_F ));
	VERIFY(m_bmHdd.LoadBitmap(IDB_HDD));
	VERIFY(m_bmCapsOn.LoadBitmap(IDB_KEY_CAPS));
	VERIFY(m_bmCapsOff.LoadBitmap(IDB_KEY_CAPS_OFF));
	VERIFY(m_bmScrollLockOn.LoadBitmap(IDB_KEY_SCROLL));
	VERIFY(m_bmScrollLockOff.LoadBitmap(IDB_KEY_SCROLL_OFF));
	VERIFY(m_bmNumLockOn.LoadBitmap(IDB_KEY_NUMLOCK));
	VERIFY(m_bmNumLockOff.LoadBitmap(IDB_KEY_NUMLOCK_OFF));
	int i;
	for( i = 0; i < 5; i++ )
		m_iDiskStatus[i] = 0;
	m_bKeyCaptured = false;
	m_bKeyCaps = false;
	m_bKeyScrollLock = false;
	m_bKeyNumLock = false;
}

CAppleStatusBar::~CAppleStatusBar()
{

}


BEGIN_MESSAGE_MAP(CAppleStatusBar, CStatusBar)
	//{{AFX_MSG_MAP(CAppleStatusBar)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAppleStatusBar message handlers

void CAppleStatusBar::DrawItem(LPDRAWITEMSTRUCT lpDrawItem)
{
	switch( lpDrawItem->itemID )
	{
	case 1:
		DrawDiskLight( lpDrawItem->hDC, lpDrawItem->rcItem );
		return;
	case 4:
		DrawKeyStatus(lpDrawItem->hDC, lpDrawItem->rcItem);
		return;
	}
	CStatusBar::DrawItem( lpDrawItem );
}

BOOL CAppleStatusBar::Create(CWnd *pParentWnd, DWORD dwStyle)
{
	if ( !CStatusBar::Create(pParentWnd, dwStyle , AFX_IDW_STATUS_BAR)
		||	!SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)) )
		return FALSE;

	SetPaneInfo(0, GetItemID(0), GetPaneStyle(0), 280);
	SetPaneInfo(1, GetItemID(1), GetPaneStyle(1) | SBT_OWNERDRAW, 106);
	SetPaneInfo(2, GetItemID(2), GetPaneStyle(2), 65);
	SetPaneInfo(3, GetItemID(3), GetPaneStyle(3), 55);
	SetPaneInfo(4, GetItemID(4), GetPaneStyle(4) | SBT_OWNERDRAW, 45);
	GetItemRect(1, &m_rectDisk);
	return TRUE;
}

void CAppleStatusBar::SetMessage( LPCTSTR lpszText )
{
    SetPaneText( 0, lpszText );
}

void CAppleStatusBar::SetSpeed(double speed)
{
	CString szSpeed;
	szSpeed.Format( "%2.3lf MHz", speed );
	SetPaneText( 2, szSpeed );
}

void CAppleStatusBar::SetFrame(double frame)
{
	CString szFrame;
	szFrame.Format( "%3.2lf f/s", frame );
	SetPaneText( 3, szFrame );
}

void CAppleStatusBar::SetKeyStatus(int index, int status)
{
	CScreen* pScreen = NULL;
	bool bChanged;
	bool* pbTarget;
	bool bFlag = status != 0;

	switch (index)
	{
	case KEY_STATE_CAPTURE:
		pbTarget = &m_bKeyCaptured;
		break;
	case KEY_STATE_CAPS:
		pbTarget = &m_bKeyCaps;
		break;
	case KEY_STATE_SCROLL:
		pbTarget = &m_bKeyScrollLock;
		break;
	case KEY_STATE_NUMLOCK:
		pbTarget = &m_bKeyNumLock;
		break;
	default:
		return;
	}
	bChanged = *pbTarget != bFlag;
	*pbTarget = bFlag;

	if (bChanged)
	{
		if (g_pBoard != NULL)
			pScreen = g_pBoard->m_pScreen;

		if (pScreen == NULL || pScreen->m_bWindowed)
		{
			InvalidateRect(&m_rectKeyStatus, FALSE);
		}
	}
}

void CAppleStatusBar::SetDiskStatus(int index, int status)
{
	int i;
	if ( index < -1 || index >= 3 )
		return;
	int old1 = 0, old2 = 0;
	int w;
	BOOL bChanged = TRUE;

	if (index == 2)		// hdd
	{
		index = 4;
		old1 = m_iDiskStatus[index];

		w = status >> 2;
		if ((status & 3) != 0)
		{
			m_iDiskStatus[index] = 1 + w;
		}
		else
		{
			m_iDiskStatus[index] = 0;
		}
		bChanged = (old1 != m_iDiskStatus[index]);
	}
	else if ( index >= 0 )
	{
		index <<= 1;
		w = status >> 2;
		
		old1 = m_iDiskStatus[index];
		old2 = m_iDiskStatus[index+1];
		
		m_iDiskStatus[index] = ( status & 1 );
		m_iDiskStatus[index+1] = ( status >> 1 ) & 1;
		if ( m_iDiskStatus[index] )
			m_iDiskStatus[index] += w;
		else if ( m_iDiskStatus[index+1] )
			m_iDiskStatus[index+1] += w;
		bChanged = (old1 != m_iDiskStatus[index] || old2 != m_iDiskStatus[index + 1]);
	}
	if ( bChanged )
	{
		CScreen* pScreen = NULL;
		
		if ( g_pBoard != NULL )
			pScreen = g_pBoard->m_pScreen;

		if ( pScreen == NULL || pScreen->m_bWindowed )
		{
			InvalidateRect(&m_rectDisk, FALSE);
		}
		else
		{
			RECT rc = { 1, 1, DISK_VIEW_WIDTH-2, DISK_VIEW_HEIGHT-2 };
			HRESULT hr;
			HDC hDC;
			CSurface* pSurface = pScreen->GetDiskSurface();

			for( i = 0; i < 5; i++ )
			{
				if ( m_iDiskStatus[i] != 0 )
				{
					break;
				}
			}
			if ( i >= 5 )
			{
				// all off
				pSurface->Clear();
			}
			else
			{
				LPDIRECTDRAWSURFACE7 lpdds = pSurface->GetDDrawSurface();
				if ( lpdds == NULL )
					return;
				hr = lpdds->GetDC(&hDC);
				if FAILED(hr)
					return;
				if ( hDC )
				{
					DrawDiskLight_Full( hDC, rc );
					lpdds->ReleaseDC(hDC);
				}
			}
			pScreen->UpdateDiskSurface();
		}
	}
}

void CAppleStatusBar::OnSize(UINT nType, int cx, int cy) 
{
	CStatusBar::OnSize(nType, cx, cy);
	GetItemRect(1, &m_rectDisk);
	GetItemRect(4, &m_rectKeyStatus);
	// TODO: Add your message handler code here
}

void CAppleStatusBar::DrawDiskLight(HDC hDC, RECT rc)
{
	CDC dc;
	dc.Attach(hDC);
	CRect rect(rc);
	CDC srcDC;
	BITMAP info;
	CBitmap* pOldBitmap;
	int margin;

	srcDC.CreateCompatibleDC(NULL);
	pOldBitmap = srcDC.SelectObject(&m_bmDisk);
	m_bmDisk.GetObject(sizeof(BITMAP), &info);
	
	dc.BitBlt( rect.left, rect.top+(rect.Height()-info.bmHeight)/2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY );
	
	rect.left += info.bmWidth + 2;
	
	m_bmDiskOff.GetObject(sizeof(BITMAP), &info);
	margin = (rect.Height() - info.bmHeight) / 2;
	
	int i;
	for( i = 0; i < 4; i++ )
	{
		if ( m_iDiskStatus[i] == 1 )
			srcDC.SelectObject( &m_bmDiskRead );
		else if ( m_iDiskStatus[i] == 2 )
			srcDC.SelectObject( &m_bmDiskWrite );
		else
			srcDC.SelectObject( &m_bmDiskOff );
		
		dc.BitBlt( rect.left, rect.top + margin, info.bmWidth, info.bmHeight,
			&srcDC, 0, 0, SRCCOPY );
		rect.left += info.bmWidth;
	}

	rect.left += 4;

	// draw hdd
	srcDC.SelectObject(&m_bmHdd);
	m_bmHdd.GetObject(sizeof(BITMAP), &info);
	dc.BitBlt(rect.left, rect.top + (rect.Height() - info.bmHeight) / 2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);
	rect.left += info.bmWidth + 2;

	// draw hdd lightm_bmHdd
	if (m_iDiskStatus[4] == 1)
		srcDC.SelectObject(&m_bmDiskRead);
	else if (m_iDiskStatus[4] == 2)
		srcDC.SelectObject(&m_bmDiskWrite);
	else
		srcDC.SelectObject(&m_bmDiskOff);

	dc.BitBlt(rect.left, rect.top + margin
		, info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);

	srcDC.SelectObject(pOldBitmap);
	dc.Detach();
}

void CAppleStatusBar::DrawDiskLight_Full(HDC hDC, RECT rc)
{
	CDC dc;
	dc.Attach(hDC);
	CRect rect(rc);
	CDC srcDC;
	BITMAP info;
	CBitmap* pOldBitmap;
	
	srcDC.CreateCompatibleDC(NULL);

	pOldBitmap = srcDC.SelectObject(&m_bmDisk);
	/*
	m_bmDisk.GetObject(sizeof(BITMAP), &info);
	
	dc.BitBlt( rect.left, rect.top+(rect.Height()-info.bmHeight)/2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY );
	
	rect.left += info.bmWidth + 2;
	*/

	m_bmDiskOff_Full.GetObject(sizeof(BITMAP), &info);
	rect.top += ( rect.Height()-info.bmHeight ) / 2;
	
	int i;
	for( i = 0; i < 4; i++ )
	{
		if ( m_iDiskStatus[i] == 1 )
		{
			srcDC.SelectObject( &m_bmDiskRead_Full );
		}
		else if ( m_iDiskStatus[i] == 2 )
		{
			srcDC.SelectObject( &m_bmDiskWrite_Full );
		}
		else
		{
			srcDC.SelectObject( &m_bmDiskOff_Full );
		}
		
		dc.BitBlt( rect.left, rect.top, info.bmWidth, info.bmHeight,
			&srcDC, 0, 0, SRCCOPY );
		rect.left += info.bmWidth;
	}

	// blank space
	rect.left += info.bmWidth;

	// draw hdd light
	if (m_iDiskStatus[4] == 1)
		srcDC.SelectObject(&m_bmDiskRead_Full);
	else if (m_iDiskStatus[4] == 2)
		srcDC.SelectObject(&m_bmDiskWrite_Full);
	else
		srcDC.SelectObject(&m_bmDiskOff_Full);

	dc.BitBlt(rect.left, rect.top, info.bmWidth, info.bmHeight,
		&srcDC, 0, 0, SRCCOPY);

	srcDC.SelectObject(pOldBitmap);
	dc.Detach();
}

void CAppleStatusBar::DrawKeyStatus(HDC hDC, RECT rc)
{
	CDC dc;
	dc.Attach(hDC);
	CRect rect(rc);
	CDC srcDC;
	BITMAP info;
	CBitmap* pOldBitmap;
	CBitmap* pBitmap;

	srcDC.CreateCompatibleDC(NULL);
	
	dc.FillSolidRect(&rc, RGB(255, 255, 255));

	pBitmap = m_bKeyCaptured ? &m_bmDiskRead : &m_bmDiskOff;

	pOldBitmap = srcDC.SelectObject(pBitmap);
	pBitmap->GetObject(sizeof(BITMAP), &info);

	dc.BitBlt(rect.left, rect.top + (rect.Height() - info.bmHeight) / 2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);
	rect.left += info.bmWidth + 1;

	// caps lock
	pBitmap = m_bKeyCaps ? &m_bmCapsOn : &m_bmCapsOff;

	srcDC.SelectObject(pBitmap);
	pBitmap->GetObject(sizeof(BITMAP), &info);

	dc.BitBlt(rect.left, rect.top + (rect.Height() - info.bmHeight) / 2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);
	rect.left += info.bmWidth + 1;

	// scroll lock
	pBitmap = m_bKeyScrollLock ? &m_bmScrollLockOn : &m_bmScrollLockOff;

	srcDC.SelectObject(pBitmap);
	pBitmap->GetObject(sizeof(BITMAP), &info);

	dc.BitBlt(rect.left, rect.top + (rect.Height() - info.bmHeight) / 2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);
	rect.left += info.bmWidth + 1;

	// num lock
	pBitmap = m_bKeyNumLock ? &m_bmNumLockOn : &m_bmNumLockOff;

	srcDC.SelectObject(pBitmap);
	pBitmap->GetObject(sizeof(BITMAP), &info);

	dc.BitBlt(rect.left, rect.top + (rect.Height() - info.bmHeight) / 2,
		info.bmWidth, info.bmHeight, &srcDC, 0, 0, SRCCOPY);
	rect.left += info.bmWidth + 1;

	srcDC.SelectObject(pOldBitmap);
	dc.Detach();
}
