// colorbutton.cpp : implementation file
//

#include "arch/frame/stdafx.h"
#include "arch/frame/colorbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorButton

CColorButton::CColorButton()
{
}

BEGIN_MESSAGE_MAP(CColorButton, CButton)
	//{{AFX_MSG_MAP(CColorButton)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorButton message handlers

void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	CRect cRect( lpDrawItemStruct->rcItem );
	CBrush cBrush;
	CPen cPen, cPenBlack;
	CDC cDC;
	BYTE byBit;
	int i;

	cDC.Attach( lpDrawItemStruct->hDC );
	cPenBlack.CreatePen( PS_SOLID, 1, (COLORREF)0 );
	// draw outline
	if ( !IsWindowEnabled() )
		cPen.CreatePen( PS_SOLID, 1, RGB(128, 128, 128 ) );
	else if ( GetFocus() == this )
		cPen.CreatePen( PS_SOLID, 1, RGB(255, 128, 128 ) );
	else
		cPen.CreatePen( PS_SOLID, 1, RGB(0, 0, 0 ) );

	cDC.SelectObject( cPen );
	cDC.Rectangle( cRect );
	cRect.DeflateRect( 1, 1, 1, 1 );
	cDC.SelectObject( cPenBlack );
	cDC.Rectangle( cRect );
	cRect.DeflateRect( 1, 1, 1, 1 );
	cDC.Rectangle( cRect );
	cRect.DeflateRect( 1, 1, 1, 1 );

	cPen.DeleteObject();

	if ( m_byMask == 0x1F )
	{
		cBrush.CreateSolidBrush( m_crColor );
		cDC.FillRect( cRect, &cBrush );
		cBrush.DeleteObject();
	}
	else
	{
		cPen.CreatePen( PS_SOLID, 1, m_crColor );
		byBit = 1;
		for( i = cRect.left; i < cRect.right; i++ )
		{
			if ( m_byMask & byBit )
				cDC.SelectObject( cPen );
			else
				cDC.SelectObject( cPenBlack );
			cDC.MoveTo( i, cRect.top );
			cDC.LineTo( i, cRect.bottom );
			byBit <<= 1;
			if ( byBit == 0x10 )
				byBit = 1;
		}
		if ( !( m_byMask & 0x10 ) )
		{
			cDC.SelectObject( cPenBlack );
			for( i = cRect.top + 1; i < cRect.bottom; i += 2 )
			{
				cDC.MoveTo( cRect.left, i );
				cDC.LineTo( cRect.right, i );
			}
		}
		cPen.DeleteObject();
	}

	cPenBlack.DeleteObject();

	cDC.Detach();
}

void CColorButton::SetColor(COLORREF crColor, BYTE byMask)
{
	m_crColor = crColor;
	m_byMask = byMask;

	Invalidate( FALSE );
}
