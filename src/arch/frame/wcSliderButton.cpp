/*-----------------------------------------------------------------------
| Slider popup Edit														|
| @ Roland Seah															|
| @ 15/02/2002															|
-----------------------------------------------------------------------*/
#include "arch/frame/stdafx.h"
#include "arch/frame/wcSliderButton.h"
#include "arch/frame/wcSliderPopup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Button states

#define BTN_UP			0
#define BTN_DOWN		1
#define BTN_DISABLED	2

/////////////////////////////////////////////////////////////////////////////
// Helper function

BOOL IsWindow (CWnd *pWnd)
{
	if (!pWnd)
		return (FALSE);
	return ::IsWindow (pWnd->m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// wcSliderButton

IMPLEMENT_DYNAMIC (wcSliderButton, CEdit)

wcSliderButton::wcSliderButton ( void )
{
	m_Pos				= 0;
	m_Min				= 0;
	m_Max				= 100;
	m_bButtonLeft		= FALSE;
	m_bMouseCaptured	= FALSE;
	m_nButtonState		= BTN_UP;
	m_rcButtonRect.SetRectEmpty();
}

wcSliderButton::~wcSliderButton()
{
}

BEGIN_MESSAGE_MAP(wcSliderButton, CEdit)
	//{{AFX_MSG_MAP(wcSliderButton)
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_WM_ENABLE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCPAINT()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
	//}}AFX_MSG_MAP
    ON_MESSAGE(CSP_DELTAPOS, OnSliderDelta)
    ON_MESSAGE(CSP_CLOSEUP,  OnSliderClose)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// wcSliderButton message handlers


void wcSliderButton::ButtonClicked(CPoint p)
{
	CString Val;
	GetWindowText (Val);
	m_Pos = atoi (Val.GetBuffer(0));
	CheckPosRange ();
	
	CRect Win = m_rcButtonRect;
//	Win.left -= GetPosPerc() + (m_rcButtonRect.Width()/2);
	
	ClientToScreen (&Win);
	DrawButton (BTN_DOWN);
	p.y = Win.bottom; // set y at botton of button so taht it is not obsurred
	new wcSliderPopup (p, m_Pos, m_Min, m_Max, this);
}

LONG wcSliderButton::OnSliderDelta(UINT lParam, LONG wParam)
{
    CWnd *pParent = GetParent();
    if ( pParent ) 
		 pParent->SendMessage(CSP_DELTAPOS, lParam, (WPARAM) GetDlgCtrlID());
		
	SetPos (lParam);
    return TRUE;
}

LONG wcSliderButton::OnSliderClose(UINT lParam, LONG wParam)
{
	DrawButton (BTN_UP);
    CWnd *pParent = GetParent();
    if (pParent) {
        pParent->SendMessage(CSP_CLOSEUP, lParam, (WPARAM) GetDlgCtrlID());
    }

    return TRUE;
}

BOOL wcSliderButton::Create(BOOL LeftBut, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID) 
{
	// Create this control in any window
	BOOL bResult = CreateEx(dwExStyle, "EDIT", lpszWindowName, dwStyle, rect, pParentWnd, nID);
	if (bResult)
	{
		// call wcSliderButton::FindFolder() to initialize the internal data structures
		SetLeftButton(LeftBut);
		// Force a call to wcSliderButton::OnNcCalcSize() to calculate button size
		SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		// set the font to the font used by the parent window
		if (pParentWnd)
			SetFont (pParentWnd->GetFont());
	}
	return bResult;
}

void wcSliderButton::DrawButton(int nButtonState)
{
	ASSERT (IsWindow(this));
	
	// if the control is disabled, ensure the button is drawn disabled
	if (GetStyle() & WS_DISABLED)
		nButtonState = BTN_DISABLED;

	// Draw the button in the specified state (Up, Down, or Disabled)
	CWindowDC DC(this);		// get the DC for drawing

	DWORD dwStyle = DFCS_SCROLLDOWN;
	if ( nButtonState == BTN_DOWN )
		 dwStyle |= DFCS_PUSHED;
	if ( nButtonState == BTN_DISABLED )
		 dwStyle |=DFCS_INACTIVE;
	DC.DrawFrameControl(&m_rcButtonRect,DFC_SCROLL,dwStyle);
	// update m_nButtonState
	m_nButtonState = nButtonState;
}

void wcSliderButton::OnEnable(BOOL bEnable) 
{
	// enables/disables the control
	CEdit::OnEnable(bEnable);
	DrawButton (bEnable ? BTN_UP : BTN_DISABLED);
}

void wcSliderButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// check for CTRL + 'period' keystroke, if found, simulate a mouse click on the button
//	if ((nChar == 0xBE || nChar == VK_DECIMAL) && GetKeyState(VK_CONTROL) < 0)
//		ButtonClicked();
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

//////////////////////////////////////////////////////////////////////////////
// Because the mouse is captured in OnNcLButtonDown(), we have to respond	//
// to WM_LBUTTONUP and WM_MOUSEMOVE messages.								//
// The m_bMouseCaptured variable is used because CEdit::OnLButtonDown()		//
// also captures the mouse, so using GetCapture() could give an invalid		//
// response.																//
//////////////////////////////////////////////////////////////////////////////

void wcSliderButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CEdit::OnLButtonUp(nFlags, point);
	
	// Release the mouse capture and draw the button as normal. If the
	// cursor is over the button, simulate a click by carrying
	// out the required action.
	if (m_bMouseCaptured)
	{
		m_bMouseCaptured = FALSE;
		if (m_nButtonState != BTN_UP)
			DrawButton(BTN_UP);
	}
}

void wcSliderButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	CEdit::OnMouseMove(nFlags, point);

	// presses and releases the button as the mouse is moved over and
	// off the button. we check the current button state to avoid
	// unnecessary flicker
	if (m_bMouseCaptured)
	{
		ClientToScreen(&point);
		if (ScreenPointInButtonRect(point))
		{
			if (m_nButtonState != BTN_DOWN)
				DrawButton (BTN_DOWN);
		}
		else if (m_nButtonState != BTN_UP)
			DrawButton (BTN_UP);
	}
}

void wcSliderButton::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	// calculate the size of the client area and the button.
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);
	// set button area equal to client area of edit control
	m_rcButtonRect = lpncsp->rgrc[0];
	if (m_bButtonLeft)		// draw the button on the left side of the control
	{
		// shrink left side of client area by 80% of the height of client area
		lpncsp->rgrc[0].left += (lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top) * 8/10;
		// shrink the button so its right side is at the left side of client area
		m_rcButtonRect.right = lpncsp->rgrc[0].left;
	}
	else					// draw the button on the right side of the control
	{
		// shrink right side of client area by 80% of the height of client area
		lpncsp->rgrc[0].right -= (lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top) * 8/10;
		// shrink the button so its left side is at the right side of client area
		m_rcButtonRect.left = lpncsp->rgrc[0].right;
	}
	if (bCalcValidRects)
		// convert button coordinates from parent client coordinates to control window coordinates
		m_rcButtonRect.OffsetRect(-lpncsp->rgrc[1].left, -lpncsp->rgrc[1].top);
	m_rcButtonRect.NormalizeRect();
}

UINT wcSliderButton::OnNcHitTest(CPoint point) 
{
	// If the mouse is over the button, OnNcHitTest() would normally return
	// HTNOWHERE, and we would not get any mouse messages. So we return 
	// HTBORDER to ensure we get them.
	UINT where = CEdit::OnNcHitTest(point);
	if (where == HTNOWHERE && ScreenPointInButtonRect(point))
		where = HTBORDER;
	return where;
}

void wcSliderButton::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	CEdit::OnNcLButtonDown(nHitTest, point);
	
	if (ScreenPointInButtonRect(point))
	{
		// Capture mouse input, set the focus to this control,
		// and draw the button as pressed
		m_bMouseCaptured = TRUE;
		SetFocus();
		DrawButton(BTN_DOWN);
		ButtonClicked(point);
	}
}

void wcSliderButton::OnNcPaint() 
{
	CEdit::OnNcPaint();				// draws the border around the control
	DrawButton (m_nButtonState);	// draw the button in its current state
}

void wcSliderButton::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
	// Select all the text
	SetSel(0,-1);
}

void wcSliderButton::OnSize(UINT nType, int cx, int cy) 
{
	CEdit::OnSize(nType, cx, cy);
	// Force a recalculation of the button's size and position
	SetWindowPos (NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

BOOL wcSliderButton::ScreenPointInButtonRect(CPoint point)
{
	// Checks if the given point (in screen coordinates) is in the button rectangle
	CRect ControlRect;
	GetWindowRect(&ControlRect);

	// convert point from screen coordinates to window coordinates
	point.Offset(-ControlRect.left, -ControlRect.top);
	return m_rcButtonRect.PtInRect(point);
}

void wcSliderButton::SetLeftButton(BOOL Left)
{
	m_bButtonLeft = Left;
}

void wcSliderButton::SetPos ( int Pos )
{
	if ( Pos >= m_Min && Pos <= m_Max )
	{
		 m_Pos = Pos;
		 CString Tmp;
		 Tmp.Format ("%d", m_Pos);
		 SetWindowText (Tmp);
	}
}

void wcSliderButton::CheckPosRange ( void )
{
	BOOL Change = FALSE;
	if ( m_Pos < m_Min )
	{
		 m_Pos = m_Min;
		 Change = TRUE;
	}

	if ( m_Pos > m_Max )
	{
		 m_Pos = m_Max;
		 Change = TRUE;
	}

	if ( Change )
		 SetPos (m_Pos);
}

int wcSliderButton::GetPosPerc ( void )
{
	int OffSet = 0;
	if ( m_Min < 0 )
		 OffSet = -m_Min;
	int nMin = m_Min + OffSet;
	int nMax = m_Max + OffSet;
	int Range = abs (nMax - nMin);

	return (m_Pos+OffSet) * 100 / Range;
}

void wcSliderButton::OnUpdate () 
{
	TCHAR buf[512];
	GetWindowText (buf, sizeof( buf ) / sizeof(TCHAR));

	int Value = atoi(buf);
	
	if ( Value >= m_Min &&  Value <= m_Max )
	{
		 m_Pos = Value;
		 CWnd *pParent = GetParent();
		 if ( pParent ) 
			  pParent->SendMessage(CSP_DELTAPOS, m_Pos, (WPARAM) GetDlgCtrlID());
	}
	else
		 OnBadInput();
}

void wcSliderButton::OnBadInput()
{
	MessageBeep((UINT)-1);
}

void wcSliderButton::OnChar ( UINT nChar, UINT nRepCnt, UINT nFlags ) 
{
	if ( nChar == ' ' ) 
		 return;
	int oldValue = GetPos();
	CEdit::OnChar (nChar, nRepCnt, nFlags);

	int val = IsValid();
	CString s;

	switch ( val )
	{
		case VALID:
			 s.Format ("%d", GetPos());
			 break;
		case MINUS_PLUS: break;
		default:
			 SetPos (oldValue);
			 SetSel (0, -1);
			 MSG msg;
			 while (::PeekMessage(&msg, m_hWnd, WM_CHAR, WM_CHAR, PM_REMOVE));
			 break;
	}
}

int	wcSliderButton::IsValid ( void ) const
{
	CString str;
	GetWindowText(str);
	int res = VALID;
	int f;
	char lp[30];

	if ( (str.GetLength() == 1) && ((str[0] == '+') || (str[0] == '-')) ) 
		 res = MINUS_PLUS;
	else
		if ( sscanf(str, "%ld%s", &f, lp) != 1 ) 
			 res = INVALID_CHAR;
	else
		if ( f > m_Max || f < m_Min ) 
			 res = OUT_OF_RANGE;

	return res;
}

/////////////////////////////////////////////////////////////////////////////
// DDV_FileEditCtrl & DDX_FileEditCtrl

void DDX_SliderButtonCtrl(CDataExchange *pDX, int nIDC, wcSliderButton &rCFEC, BOOL LeftBut)
{
	// Subclass the specified wcSliderButton class object to the edit control
	// with the ID nIDC. dwFlags is used to setup the control
	ASSERT (pDX->m_pDlgWnd->GetDlgItem(nIDC));
	if (rCFEC.m_hWnd == NULL)					// not yet subclassed
	{
		ASSERT (!pDX->m_bSaveAndValidate);
		// subclass the control to the edit control with the ID nIDC
		HWND hWnd = pDX->PrepareEditCtrl(nIDC);
		if (!rCFEC.SubclassWindow(hWnd))
		{										// failed to subclass the edit control
			ASSERT(FALSE);
			AfxThrowNotSupportedException();
		}
		// call wcSliderButton::SetFlags() to initialize the internal data structures
		rCFEC.SetLeftButton(LeftBut);
		// Force a call to wcSliderButton::OnNcCalcSize() to calculate button size
		rCFEC.SetWindowPos(NULL,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

