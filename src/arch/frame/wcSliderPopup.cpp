/*-----------------------------------------------------------------------
| Slider popup Edit														|
| @ Roland Seah															|
| @ 15/02/2002															|
-----------------------------------------------------------------------*/
#include <math.h>
#include "arch/frame/stdafx.h"
#include "arch/frame/wcSliderButton.h"
#include "arch/frame/wcSliderPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define POPUP_WID			124
#define POPUP_HGT			20
#define TRACK_LEN			100
#define POPUP_FRAMETHICK	2
#define TRI_WID				10
#define TRI_HGT				6
#define TRI_X				(10+POPUP_FRAMETHICK)
#define TRI_Y				8



/////////////////////////////////////////////////////////////////////////////
// wcSliderPopup
wcSliderPopup::wcSliderPopup()
{
    m_pParent	= NULL;
	m_Pos		= 0;
	m_OldPos	= 0;
	m_TriPos	= 0;
	m_Min		= 0;
	m_Max		= 100;
	m_Delta		= 1;
}

wcSliderPopup::wcSliderPopup ( CPoint p, int Pos, int nMin, int nMax, CWnd* pParentWnd )
{
    wcSliderPopup::Create (p, Pos, nMin, nMax, pParentWnd);
}


BOOL wcSliderPopup::Create ( CPoint p, int Pos, int nMin, int nMax, CWnd* pParentWnd )
{
	m_pParent  = pParentWnd;
	m_Min	   = nMin;
	m_Max	   = nMax;
	if ( Pos < m_Min )
		Pos = m_Min;
	if ( Pos > m_Max )
		Pos = m_Max;
	
	// p is the location of the mouse click.  Want track to be at pt
	int left = p.x - (GetTriPos(Pos) + TRI_X );
	
	// Get the class name and create the window
	CString szClassName = AfxRegisterWndClass(CS_CLASSDC|CS_SAVEBITS|CS_HREDRAW|CS_VREDRAW,
		0,
		(HBRUSH) (COLOR_BTNFACE+1), 
		0);
	
	if (!CWnd::CreateEx(0, 
				    szClassName, 
					_T(""), 
					WS_VISIBLE|WS_POPUP, 
					left, 
					p.y, 
					POPUP_WID, 
					POPUP_HGT, 
					pParentWnd->GetSafeHwnd(), 
					0, 
					NULL))
					return FALSE;
	
	// Capture all mouse events for the life of this window
	SetCapture ();
	SetFocus ();
	SetPos (Pos, FALSE);
	return TRUE;
}

BEGIN_MESSAGE_MAP(wcSliderPopup, CWnd)
    //{{AFX_MSG_MAP(wcSliderPopup)
	ON_WM_ERASEBKGND()
    ON_WM_NCDESTROY()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATEAPP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// wcSliderPopup message handlers
BOOL wcSliderPopup::OnEraseBkgnd ( CDC* pDC ) 
{
	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}

/* Auto Delete
   */
void wcSliderPopup::OnNcDestroy() 
{
    CWnd::OnNcDestroy();
    delete this;
}

void wcSliderPopup::DrawTriAt ( int Pos, BOOL Clear, BOOL DrawTrack )
{
	CRect Win;
	CClientDC dc (this);
	GetClientRect (&Win);

	CDC dcMem;
	CBitmap Layer;
	dcMem.CreateCompatibleDC (&dc);
	Layer.CreateCompatibleBitmap (&dc, Win.Width(), Win.Height());
	CBitmap* pOldBitmap = dcMem.SelectObject(&Layer);

	// Paint the background
	CBrush Bg (GetSysColor(COLOR_BTNFACE));
	CRect Rect (CPoint(0,0), CSize(Win.Width(), Win.Height()));
	dcMem.FillRect (&Rect, &Bg);

	// Create the pen
	CPen black (PS_SOLID, 1, RGB (0,0,0));
	CPen dgrey (PS_SOLID, 1, RGB (102, 102, 102));
	CPen*old = dcMem.SelectObject (&black);

	// Draw the thumb
	dcMem.MoveTo	   (TRI_X,		   TRI_Y-1);
	dcMem.LineTo	   (TRACK_LEN+TRI_X, TRI_Y-1);
	dcMem.SelectObject (&dgrey);
	dcMem.MoveTo	   (TRI_X, 6);
	dcMem.LineTo	   (TRI_X+TRACK_LEN, TRI_Y-2);
	
	int CenterBase = TRI_WID >> 1;
	int tx		   = TRI_X + Pos - CenterBase;
	int ty		   = TRI_Y;
	int ty2		   = TRI_Y + TRI_HGT;

	// Draw Shadow
	dcMem.SelectObject (&dgrey);
	dcMem.MoveTo	   (tx +1,				 ty2+1);
	dcMem.LineTo	   (tx +1 + TRI_WID,    ty2+1);
	dcMem.LineTo	   (tx +1 + CenterBase, ty+1);

	// Draw Tri
	dcMem.SelectObject (&black);
	dcMem.MoveTo	   (tx,			  ty2);
	dcMem.LineTo	   (tx + TRI_WID,    ty2);
	dcMem.LineTo	   (tx + CenterBase, ty);
	dcMem.LineTo	   (tx,			  ty2);

	// Draw the frame around the window
	dcMem.DrawEdge (Win, EDGE_RAISED, BF_RECT);	

	// Draw to Client Area
	dc.BitBlt (Win.left, Win.top, Win.Width(), Win.Height(),&dcMem, 0, 0, SRCCOPY);
		
	dcMem.SelectObject (old);
	dcMem.SelectObject (pOldBitmap);
	dcMem.DeleteDC ();
	Layer.DeleteObject();
}

void wcSliderPopup::OnPaint () 
{
    CPaintDC dc (this);

	DrawTriAt (m_TriPos, FALSE, TRUE);

	CRect Win;
	GetClientRect (&Win);
    dc.DrawEdge   (Win, EDGE_RAISED, BF_RECT);
	SetCapture    ();
}

void wcSliderPopup::OnMouseMove ( UINT nFlags, CPoint point ) 
{
    CWnd::OnMouseMove (nFlags, point);

	if ( nFlags & MK_LBUTTON )
	{
		 CRect Win;
		 GetClientRect (&Win);

		 int tPos = point.x - TRI_WID - POPUP_FRAMETHICK;
		 if ( tPos < 0		   ) tPos = 0;
		 if ( tPos > TRACK_LEN ) tPos = TRACK_LEN;
		 m_TriPos = tPos;
		 m_Pos = TriToPos();

		 PositionChange ();
		 SetCapture ();
	}
}

void wcSliderPopup::OnLButtonUp ( UINT nFlags, CPoint point )
{    
    CWnd::OnLButtonUp(nFlags, point);

    DWORD pos = GetMessagePos();
    point = CPoint (LOWORD(pos), HIWORD(pos));

	CRect Win;
	GetClientRect (&Win);
	ClientToScreen (&Win);

	EndSliderPopup (CSP_CLOSEUP);
}

void wcSliderPopup::EndSliderPopup ( int nMessage )
{
    ReleaseCapture ();
	m_pParent->SendMessage (nMessage, (WPARAM)m_Pos, 0);
    DestroyWindow ();
}

void wcSliderPopup::OnKillFocus ( CWnd* pNewWnd )
{
	CWnd::OnKillFocus (pNewWnd);
    ReleaseCapture ();
}

void wcSliderPopup::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CWnd::OnActivateApp (bActive, hTask);

	if ( ! bActive )
		 EndSliderPopup (CSP_CLOSEUP);
}

void wcSliderPopup::PositionChange (void)
{
	DrawTriAt (m_TriPos, FALSE);

	// Sent message for update
	m_pParent->SendMessage (CSP_DELTAPOS, (WPARAM)m_Pos, 0);
}

// If an arrow key is pressed, then move the selection
void wcSliderPopup::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_RIGHT || nChar == VK_UP )
	{
		 if ( m_Pos < m_Max ) 
		 {
			  m_Pos += m_Delta;
			  if ( m_Pos > m_Max )
				   m_Pos = m_Max;
		 }
		 PositionChange ();
	}
	else if ( nChar == VK_LEFT || nChar == VK_DOWN )
	{
		 if ( m_Pos > m_Min ) 
		 {
			  m_Pos -= m_Delta;
			  if ( m_Pos < m_Min )
				   m_Pos = m_Min;
		 }
		 PositionChange ();
	}
    if ( nChar == VK_ESCAPE ) 
    {
         EndSliderPopup(CSP_CLOSEUP);
         return;
    }
    if ( nChar == VK_RETURN || nChar == VK_SPACE )
    {
         EndSliderPopup (CSP_CLOSEUP);
         return;
    }
    CWnd::OnKeyDown (nChar, nRepCnt, nFlags);
}

void wcSliderPopup::SetPos ( int Pos, BOOL bInvalidate/*=TRUE*/)
{
	if ( Pos >= m_Min && Pos <= m_Max )
	{
		 m_Pos = Pos;
		 m_TriPos = GetTriPos (Pos);

		 if ( bInvalidate )
			  PositionChange ();
	}
}

int wcSliderPopup::GetTriPos ( int Pos )
{
	int OffSet = -m_Min;
	
	int nMin = m_Min + OffSet;
	int nMax = m_Max + OffSet;
	int Range = abs (nMax - nMin);
	Pos += OffSet;
	
	return (Pos * TRACK_LEN + Range/2 ) / Range;
}

int wcSliderPopup::TriToPos ( void )
{
	int OffSet = 0;
	if ( m_Min < 0 )
		 OffSet = -m_Min;
	int nMin = m_Min + OffSet;
	int nMax = m_Max + OffSet;
	int Range = abs (nMax - nMin);

	return (int)(((double)m_TriPos * (double)Range + TRACK_LEN/2 ) / (double)TRACK_LEN +(double)m_Min);
}