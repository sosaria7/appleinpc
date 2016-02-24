/*-----------------------------------------------------------------------
| Slider popup Edit														|
| @ Roland Seah															|
| @ 15/02/2002															|
-----------------------------------------------------------------------*/
#ifndef _WC_SLIDER_BUTTON_H
#define _WC_SLIDER_BUTTON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// CColourPopup messages
#define CSP_DELTAPOS         WM_USER + 1201        // Colour Picker Selection change
#define CSP_CLOSEUP          WM_USER + 1202        // Colour Picker close up

class wcSliderButton : public CEdit
{
	DECLARE_DYNAMIC (wcSliderButton)
// Construction
public:
	wcSliderButton(void);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(wcSliderButton)
	public:
	//}}AFX_VIRTUAL
	virtual BOOL Create(BOOL LeftBut, DWORD dwExStyle, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

// Implementation
public:
	virtual ~wcSliderButton();
	void	SetLeftButton			( BOOL Left );
	void	SetPos					( int Pos );
	int		GetPos					( void )				{ return m_Pos; };
	void	SetRange				( int nMin, int nMax )	{ m_Min = nMin; m_Max = nMax; };
	void	SetRange				( int Pos, int nMin, int nMax )	{ SetRange (nMin, nMax); SetPos (Pos); };

	// Generated message map functions
protected:
	enum 
	{
		VALID			= 0x00, 
		OUT_OF_RANGE	= 0x01, 
		INVALID_CHAR	= 0x02, 
		MINUS_PLUS		= 0x03
	};

	void			ButtonClicked(CPoint p );							// handles a mouse click on the button
	void			DrawButton				( int nButtonState = 0 );	// draws the button
	BOOL			ScreenPointInButtonRect	( CPoint point );			// checks if the given point is in the browse button
	void			CheckPosRange			( void );
	int				GetPosPerc				( void );
	void			OnBadInput				( void );
	virtual int		IsValid			( void ) const; 
	

	//{{AFX_MSG(wcSliderButton)
	afx_msg void OnUpdate();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
    afx_msg LONG OnSliderDelta(UINT lParam, LONG wParam);
    afx_msg LONG OnSliderClose(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL			m_bButtonLeft;			// browse button on left side of control?
	BOOL			m_bMouseCaptured;		// button has captured the mouse?
	int				m_nButtonState;			// current button state (up, down, or disabled)
	CRect			m_rcButtonRect;			// window coordinates of the button
	int				m_Pos;
	int				m_Min;
	int				m_Max;
};

/////////////////////////////////////////////////////////////////////////////
// DDV_/DDX_FileEditCtrl functions

void DDX_SliderButtonCtrl (CDataExchange *pDX, int nIDC, wcSliderButton &rCFEC, BOOL LeftBut);

#endif
