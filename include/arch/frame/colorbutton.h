#if !defined(AFX_COLORBUTTON_H__B7D05142_FE0B_415B_A301_D11102E73248__INCLUDED_)
#define AFX_COLORBUTTON_H__B7D05142_FE0B_415B_A301_D11102E73248__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// colorbutton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CColorButton window

class CColorButton : public CButton
{
// Construction
public:
	CColorButton();

// Attributes
public:
protected:
	COLORREF m_crColor;
	COLORREF m_crBlack;
	BYTE m_byMask;

// Operations
public:
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorButton)
public:
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetColor( COLORREF crColor, BYTE byMask);
	~CColorButton() {}

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorButton)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORBUTTON_H__B7D05142_FE0B_415B_A301_D11102E73248__INCLUDED_)
