// apple2.h : main header file for the APPLE2 application
//

#if !defined(AFX_APPLE2_H__BD43CFF0_6E38_404C_83F6_3C673167822F__INCLUDED_)
#define AFX_APPLE2_H__BD43CFF0_6E38_404C_83F6_3C673167822F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "arch/resource.h"       // main symbols
#include "aipcdefs.h"

/////////////////////////////////////////////////////////////////////////////
// CApple2App:
// See apple2.cpp for the implementation of this class
//

class CApple2App : public CWinApp
{
public:
	CApple2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CApple2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CApple2App)
	afx_msg void OnAppAbout();
	afx_msg void OnHelpCmdUsage();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_APPLE2_H__BD43CFF0_6E38_404C_83F6_3C673167822F__INCLUDED_)
