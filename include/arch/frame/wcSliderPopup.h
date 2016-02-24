/*-----------------------------------------------------------------------
| Slider popup Edit														|
| @ Roland Seah															|
| @ 15/02/2002															|
-----------------------------------------------------------------------*/
#ifndef _WC_SLIDER_POPUP_H_
#define _WC_SLIDER_POPUP_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class wcSliderPopup : public CWnd
{
// Construction
public:
    wcSliderPopup			( void );
    wcSliderPopup			( CPoint p, int Pos, int nMin, int nMax, CWnd* pParentWnd );
                 
// Operations
public:
    BOOL	Create				( CPoint p, int Pos, int nMin, int nMax, CWnd* pParentWnd );
	void	SetPos				( int Pos, BOOL bInvalidate = TRUE );
	int		GetPos				( void )				{ return m_Pos; };
	void	SetRange			( int nMin, int nMax )	{ m_Min = nMin; m_Max = nMax; };
	void	SetDelta			( int Dlt )				{ m_Delta = Dlt; };

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(wcSliderPopup)
    //}}AFX_VIRTUAL

protected:
    void	PositionChange		( void );
    void	EndSliderPopup		( int nMessage );
	void	DrawTriAt			( int Pos, BOOL Clear, BOOL DrawTrack = FALSE );
	int		GetTriPos			( int Pos );
	int		TriToPos			( void );

// protected attributes
protected:
    CRect          m_WindowRect;
    CWnd*          m_pParent;
	int			   m_Pos;
	int			   m_OldPos;
	int			   m_TriPos;

	int			   m_Min;
	int			   m_Max;
	int			   m_Delta;

    // Generated message map functions
protected:
    //{{AFX_MSG(wcSliderPopup)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnNcDestroy();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_COLOURPOPUP_H__D0B75902_9830_11D1_9C0F_00A0243D1382__INCLUDED_)
