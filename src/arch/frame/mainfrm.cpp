// MainFrm.cpp : implementation of the CMainFrame class
//

#include <afxmt.h>
#include <windows.h>
#include <windowsx.h>

#include "arch/frame/stdafx.h"
#include "arch/frame/mainfrm.h"
#include "arch/frame/aipc.h"
#include "arch/directx/dibase.h"
#include "arch/directx/dxsound.h"
#include "arch/directx/dijoystick.h"
#include "arch/directx/dikeyboard.h"
#include "arch/directx/dimouse.h"
#include "arch/CommandLineOption.h"

#include "keyboard.h"
#include "card.h"
#include "phasor.h"
#include "memory.h"
#include "aipcdefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDIJoystick g_cDIJoystick;
CDIKeyboard g_cDIKeyboard;
CDIMouse	g_cDIMouse;

static CString GetStatusFilePath();


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_DEBUG, OnDebug)
	ON_WM_KEYDOWN()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_CONFIGURE_SLOTS, OnConfigureSlots)
	ON_COMMAND(ID_REBOOT, OnReboot)
	ON_COMMAND(ID_RESET, OnReset)
	ON_COMMAND(ID_MONITOR, OnMonitor)
	ON_COMMAND(ID_2XSCREEN, OnChangeSize)
	ON_UPDATE_COMMAND_UI(ID_MONITOR, OnUpdateMonitor)
	ON_COMMAND(ID_POWERON, OnPowerOn)
	ON_UPDATE_COMMAND_UI(ID_POWERON, OnUpdatePowerOn)
	ON_COMMAND(ID_POWEROFF, OnPowerOff)
	ON_UPDATE_COMMAND_UI(ID_POWEROFF, OnUpdatePowerOff)
	ON_UPDATE_COMMAND_UI(ID_RESET, OnUpdateReset)
	ON_WM_KEYUP()
	ON_COMMAND(ID_POWER, OnPower)
	ON_COMMAND(ID_DISKETTE, OnDiskette)
	ON_UPDATE_COMMAND_UI(ID_POWER, OnUpdatePower)
	ON_WM_CLOSE()
	ON_MESSAGE(UM_REQACQUIRE, OnReqAcquire)
	ON_MESSAGE(UM_KEYDOWN, OnMyKeyDown)
	ON_MESSAGE(UM_KEYUP, OnMyKeyUp)
	ON_MESSAGE(UM_KEYREPEAT, OnMyKeyRepeat)
	ON_COMMAND(ID_FULL_SCREEN, OnToggleFullScreen)
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_HARDDISK, OnHarddisk)
	ON_UPDATE_COMMAND_UI(ID_DISKETTE, OnUpdateDiskette)
	ON_UPDATE_COMMAND_UI(ID_HARDDISK, OnUpdateHarddisk)
	ON_UPDATE_COMMAND_UI(ID_SCANLINE, OnUpdateScanline)
	ON_UPDATE_COMMAND_UI(ID_SUSPEND, OnUpdateSuspend)
	ON_UPDATE_COMMAND_UI(ID_RESUME, OnUpdateResume)
	ON_COMMAND(ID_SCANLINE, OnScanline)
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_SUSPEND, &CMainFrame::OnSuspend)
	ON_COMMAND(ID_RESUME, &CMainFrame::OnResume)
	ON_COMMAND(ID_SUSPENDRESUME, &CMainFrame::OnSuspendResume)
	ON_WM_INPUT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_winRect.SetRectEmpty();
	m_bDoubleSize = FALSE;
	m_bFullScreen = FALSE;
	m_bKeyboardCapture = FALSE;
	m_stCursorPos.x = 0;
	m_stCursorPos.y = 0;
	ZeroMemory(&m_stCursorClip, sizeof(m_stCursorClip));
	m_hCursor = NULL;
	g_pBoard = new CAppleClock();
}

CMainFrame::~CMainFrame()
{
	delete g_pBoard;
	CWnd::~CWnd();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	BOOL bRet;
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	g_pBoard->m_lpwndMainFrame = this;
	g_pBoard->m_pScreen = &m_wndView;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_TOOLBAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
//	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);

	//m_wndToolBar.SetHeight( 32 );
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, WS_CHILD| WS_VISIBLE,
	CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	g_DXSound.Create(this->GetSafeHwnd(), DSSCL_NORMAL);

	// Initialise Joystick
	g_cDIJoystick.SetHWND(m_hWnd);
	g_pBoard->m_joystick.InitPCJoystick();

	// Initialise Keyboard
	g_cDIKeyboard.SetHWND(m_hWnd);
	g_cDIKeyboard.InitKeyboard();

	// Initialise Mouse
	g_cDIMouse.SetHWND(m_hWnd);
	g_cDIMouse.InitMouse();

	m_cMenu.LoadMenu( IDR_MAINFRAME );
	SetMenu( &m_cMenu );

	CCommandLineOption option;
	bRet = option.Parse(GetCommandLine());
	if (bRet == FALSE)
	{
		CString strMessage;
		strMessage.Format(TEXT("%s"), option.m_strErrorMsg);
		this->MessageBox(strMessage, TEXT("Error - Apple in PC"), MB_OK);
		exit(1);
	}

	g_pBoard->Initialize();
//	g_pBoard->CreateThread();
//	g_pBoard->SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
	CFileStatus fileStatus;
	CString strStatusFile;

	if (!option.m_strStatePath.IsEmpty())
		strStatusFile = option.m_strStatePath;
	else
		strStatusFile = GetStatusFilePath();

	// if regular file exists
	if (CFile::GetStatus(strStatusFile, fileStatus) == TRUE && (fileStatus.m_attribute & CFile::Attribute::directory) == 0)
	{
		if (g_pBoard->LoadState(strStatusFile) == FALSE)
		{
			CString strMessage;
			this->MessageBox(TEXT("Fail to load last state"), TEXT("Error - Apple in PC"), MB_OK);
			strMessage.Format(TEXT("Remove status file to fix this error:\n%s"), strStatusFile);
			this->MessageBox(strMessage, TEXT("Error - Apple in PC"), MB_OK);
			exit(1);
			return 1;
		}
	}
	if (!option.m_strStatePath.IsEmpty())
	{
		g_pBoard->SetStateFilePath(option.m_strStatePath, option.m_bSaveOnExit);
	}
	if (!option.m_strDisk1Path.IsEmpty())
	{
		g_pBoard->m_cSlots.SetDiskette1(option.m_strDisk1Path);
	}
	if (!option.m_strDisk2Path.IsEmpty())
	{
		g_pBoard->m_cSlots.SetDiskette2(option.m_strDisk1Path);
	}
	if (!option.m_strHardDiskPath.IsEmpty())
	{
		g_pBoard->m_cSlots.SetHardDisk(option.m_strHardDiskPath);
	}
	if (option.m_bReboot == TRUE)
	{
		g_pBoard->Reboot();
	}
	m_bDoubleSize = m_wndView.IsDoubleSized();

	RECT rc;
	::GetWindowRect( m_hWnd, &rc );
	m_stWindowPos.x = rc.left;
	m_stWindowPos.y = rc.top;

	ResizeWindow();

	g_pBoard->Resume();

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	cs.style &= ~WS_THICKFRAME;
	cs.style &= ~WS_MAXIMIZEBOX;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CMainFrame::OnDebug() 
{
	// TODO: Add your command handler code here
	g_pBoard->OnDebug();
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
//	g_pBoard->m_keyboard.OnKeyDown(nChar, nFlags);
	CFrameWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CFrameWnd::OnGetMinMaxInfo(lpMMI);
	// TODO: Add your message handler code here and/or call default
	if( !m_winRect.IsRectEmpty() && g_pBoard->m_pScreen->m_bWindowed ){
		lpMMI->ptMaxSize.x = m_winRect.Width();
		lpMMI->ptMaxSize.y = m_winRect.Height();
		lpMMI->ptMinTrackSize.x = m_winRect.Width();
		lpMMI->ptMinTrackSize.y = m_winRect.Height();
		lpMMI->ptMaxTrackSize.x = m_winRect.Width();
		lpMMI->ptMaxTrackSize.y = m_winRect.Height();
	}
}

void CMainFrame::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType) 
{
	// TODO: Add your specialized code here and/or call the base class
	CFrameWnd::CalcWindowRect(lpClientRect, nAdjustType);
}


void CMainFrame::OnClose()
{
	CFile file;
	CString strStatusFile = GetStatusFilePath();
	if (g_pBoard->SaveState(strStatusFile) == FALSE)
	{
		this->MessageBox(TEXT("Fail to store current state"), TEXT("Error - Apple in PC"), MB_OK);
	}
	g_pBoard->Exit();

	CFrameWnd::OnClose();
}

void CMainFrame::OnConfigureSlots() 
{
	// TODO: Add your command handler code here
	g_pBoard->OnConfigureSlots();
}

void CMainFrame::OnReboot() 
{
	// TODO: Add your command handler code here
	g_pBoard->Reboot();
}

void CMainFrame::OnReset() 
{
	// TODO: Add your command handler code here
	g_pBoard->Reset();
}

void CMainFrame::OnMonitor() 
{
	// TODO: Add your command handler code here
	g_pBoard->ChangeMonitorType();
}

void CMainFrame::OnChangeSize()
{
	m_bDoubleSize = !m_bDoubleSize;
	RECT rc;
	::GetWindowRect(m_hWnd, &rc);
	m_stWindowPos.x = rc.left;
	m_stWindowPos.y = rc.top;
	ResizeWindow();
}

void CMainFrame::OnUpdateMonitor(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int mode=g_pBoard->GetMonitorType();
	if ( mode == SM_COLOR )
		pCmdUI->SetText(_T("Change &Monitor to Color 2\tF8"));
	else if ( mode == SM_COLOR2 )
		pCmdUI->SetText(_T("Change &Monitor to White\tF8"));
	else if( mode == SM_WHITE )
		pCmdUI->SetText(_T("Change &Monitor to Green\tF8"));
	else
		pCmdUI->SetText(_T("Change &Monitor to Color\tF8"));
}

void CMainFrame::OnPowerOn() 
{
	// TODO: Add your command handler code here
	g_pBoard->PowerOn();
	g_pBoard->Resume();
}

void CMainFrame::OnUpdatePowerOn(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->GetIsActive() )
		pCmdUI->Enable( FALSE );
	else
		pCmdUI->Enable( TRUE );
}

void CMainFrame::OnPowerOff() 
{
	// TODO: Add your command handler code here
	g_pBoard->PowerOff();
}

void CMainFrame::OnUpdatePowerOff(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->GetIsActive() )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateReset(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->GetIsActive() )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CMainFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CFrameWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}


void CMainFrame::OnPower() 
{
	// TODO: Add your command handler code here
	if ( g_pBoard->GetIsActive() )
		g_pBoard->PowerOff();
	else
		g_pBoard->PowerOn();

}

void CMainFrame::OnDiskette() 
{
	// TODO: Add your command handler code here
	g_pBoard->m_cSlots.ConfigureDiskette();
}


void CMainFrame::OnHarddisk() 
{
	// TODO: Add your command handler code here
	g_pBoard->m_cSlots.ConfigureHardDisk();
}

void CMainFrame::OnUpdatePower(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->GetIsActive() )
		pCmdUI->SetText("&Power Off\tctrl-F11");
	else
		pCmdUI->SetText("&Power On\tctrl-F11");
}

LRESULT CMainFrame::OnReqAcquire(WPARAM wParam, LPARAM lParam)
{
	if (lParam == (LPARAM)&g_cDIKeyboard)
	{
		if (wParam)
		{
			g_pBoard->m_keyboard.SetCapsLock((GetKeyState(VK_CAPITAL) & 0x0001) == 0);
			g_pBoard->m_keyboard.SetScrollLock((GetKeyState(VK_SCROLL) & 0x0001) != 0);
			g_pBoard->m_keyboard.SetNumLock((GetKeyState(VK_NUMLOCK) & 0x0001) != 0);
			m_wndStatusBar.SetKeyStatus(KEY_STATE_CAPTURE, true);
		}
		else
		{
			BYTE abyKeyState[256];
			if (GetKeyboardState(abyKeyState) != FALSE)
			{
				abyKeyState[VK_SCROLL] = g_pBoard->m_keyboard.GetScrollLock() != FALSE;
				abyKeyState[VK_CAPITAL] = g_pBoard->m_keyboard.GetCapsLock() == FALSE;
				abyKeyState[VK_NUMLOCK] = g_pBoard->m_keyboard.GetNumLock() != FALSE;
				SetKeyboardState(abyKeyState);
			}
			m_wndStatusBar.SetKeyStatus(KEY_STATE_CAPTURE, false);
		}
	}
	if ( wParam )
	{
		if (lParam == (LPARAM)&g_cDIMouse)
		{
			if (m_hCursor == NULL)
			{
				m_hCursor = SetCursor(NULL);
				GetCursorPos(&m_stCursorPos);
				RECT rect;
				::GetClipCursor(&m_stCursorClip);
				GetWindowRect(&rect);
				::ClipCursor(&rect);
			}
		}
	}
	else
	{
		g_cDIMouse.SetActive(FALSE, FALSE);	// don't wait for mouse exit. mouse will wait for unacquiring it self.
		if (lParam == NULL)
		{
			g_cDIKeyboard.SetActive(FALSE, FALSE);
		}

		if ( m_bFullScreen == TRUE )
		{
			m_bFullScreen = FALSE;
			m_wndView.SetScreenMode(FALSE, m_bDoubleSize);
			ResizeWindow();
		}
		if (m_hCursor != NULL)
		{
			SetCursorPos(m_stCursorPos.x, m_stCursorPos.y);
			SetCursor(m_hCursor);
			::ClipCursor(&m_stCursorClip);
			m_hCursor = NULL;
		}
	}

	return 0;
}

LRESULT CMainFrame::OnMyKeyDown(WPARAM wParam, LPARAM lParam)
{
	g_pBoard->m_keyboard.OnKeyDown(wParam, 0);
	return 0;
}

LRESULT CMainFrame::OnMyKeyUp(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

LRESULT CMainFrame::OnMyKeyRepeat(WPARAM wParam, LPARAM lParam)
{
	g_pBoard->m_keyboard.OnKeyDown(wParam, 1);
	return 0;
}

void CMainFrame::OnToggleFullScreen() 
{
	// TODO: Add your command handler code here
	m_bFullScreen = !m_bFullScreen;
	if ( m_bFullScreen == TRUE )
	{
		// fullscreen으로 바뀌기 전의 창 위치를 기억한다.
		RECT rc;
		::GetWindowRect( m_hWnd, &rc );
		m_stWindowPos.x = rc.left;
		m_stWindowPos.y = rc.top;
	}
	m_wndView.SetScreenMode(m_bFullScreen, m_bDoubleSize);
	ResizeWindow();
}

void CMainFrame::ResizeWindow()
{
	// Resize Window with window style, apple display window

	RECT rc, rcWork;
	//::GetClientRect( m_hWnd, &rc );

	if ( ::IsIconic( m_hWnd ) != 0 )
	{
		this->ShowWindow( SW_SHOWNORMAL );
	}
	m_winRect.SetRectEmpty();

	m_wndView.SetScreenMode(m_bFullScreen, m_bDoubleSize);

	if ( m_bFullScreen == TRUE )
	{
		this->m_wndStatusBar.ShowWindow( FALSE );
		this->m_wndToolBar.ShowWindow( FALSE );
		this->SetMenu(NULL); 
		ModifyStyle( WS_CAPTION, 0 );
		::SetRect( &rc, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) );
		::SetWindowPos( m_hWnd, HWND_NOTOPMOST, 0, 0, rc.right, rc.bottom,
			SWP_NOZORDER );
	}
	else
	{

		ModifyStyle( 0, WS_CAPTION );
		this->SetMenu( &this->m_cMenu );
		this->m_wndStatusBar.ShowWindow( TRUE );
		this->m_wndToolBar.ShowWindow( TRUE );
		if ( m_bDoubleSize == TRUE )
		{
			::SetRect( &rc, 0, 0, WIN_WIDTH * 2, WIN_HEIGHT * 2 );
		}
		else
		{
			::SetRect( &rc, 0, 0,  WIN_WIDTH, WIN_HEIGHT );
		}
		::AdjustWindowRectEx( &rc, GetWindowStyle(m_hWnd), ::GetMenu(m_hWnd) != NULL,
			GetWindowExStyle(m_hWnd) );
		
		::SetWindowPos( m_hWnd, NULL, 0, 0, rc.right-rc.left, rc.bottom-rc.top,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
		
		::SetWindowPos( m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE );
		
		//  Make sure our window does not hang outside of the work area
		::SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWork, 0 );

		::SetWindowPos( m_hWnd, NULL, m_stWindowPos.x, m_stWindowPos.y, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		::GetWindowRect( m_hWnd, &rc );
		
		if( rc.right > rcWork.right ) rc.left = rcWork.right - (rc.right-rc.left);
		if( rc.bottom > rcWork.bottom ) rc.top = rcWork.bottom - (rc.bottom-rc.top);
		if( rc.left < rcWork.left ) rc.left = rcWork.left;
		if( rc.top  < rcWork.top )  rc.top  = rcWork.top;
		::SetWindowPos( m_hWnd, NULL, rc.left, rc.top, 0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
		
		m_stWindowPos.x = rc.left;
		m_stWindowPos.y = rc.top;

		// Resize window with tool bar, status bar
		CRect rcClientStart;
		CRect rcClientNow;
		
		GetClientRect(rcClientStart);
		
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST,
			AFX_IDW_CONTROLBAR_LAST,
			0, reposQuery, rcClientNow);
		/*
		CPoint ptOffset(rcClientNow.left - rcClientStart.left,
			rcClientNow.top - rcClientStart.top);
		
		// add size of child windows : status bar, tool bar
		CRect rcChild;
		CWnd* pwndChild = GetWindow(GW_CHILD);
		while (pwndChild)
		{
			pwndChild->GetWindowRect(rcChild);
			ScreenToClient(rcChild);
			rcChild.OffsetRect(ptOffset);
			pwndChild->MoveWindow(rcChild, FALSE);
			pwndChild = pwndChild->GetNextWindow();
		}
		*/
		CRect rcWindow;
		GetWindowRect(rcWindow);
		
		rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
		rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
		MoveWindow(rcWindow, FALSE);
		m_winRect.CopyRect( &rcWindow );
		
		if ( m_bDoubleSize == TRUE )
		{
			m_cMenu.CheckMenuItem( ID_2XSCREEN, MF_CHECKED );
		}
		else
		{
			m_cMenu.CheckMenuItem( ID_2XSCREEN, MF_UNCHECKED );
		}
	}
}

static CString GetStatusFilePath()
{
	char szPath[MAX_PATH];
	CString strPath;
	int nLen;
	GetModuleFileName( NULL, szPath, MAX_PATH );


	strPath = szPath;
	nLen = strPath.ReverseFind( '\\' );
	if ( nLen > 0 )
	{
		strPath = strPath.Left( nLen );
	}
	else
	{
		strPath = "";
	}
	strPath += "\\status.dat";

	return strPath;
}


void CMainFrame::OnKillFocus(CWnd* pNewWnd) 
{
	CFrameWnd::OnKillFocus(pNewWnd);

	m_bKeyboardCapture = g_cDIKeyboard.GetIsActive();
	TRACE("keyboard capture=%d\n", m_bKeyboardCapture);

	::PostMessage( m_hWnd, UM_REQACQUIRE, FALSE, (LPARAM)NULL );
}

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	CFrameWnd::OnSetFocus(pOldWnd);

	if (m_bKeyboardCapture == TRUE)
	{
		g_cDIKeyboard.SetActive(TRUE, FALSE);
	}
	else
	{
		g_cDIKeyboard.Restore();
	}
	::PostMessage(m_hWnd, UM_REQACQUIRE, TRUE, (LPARAM)NULL);
}

void CMainFrame::OnUpdateDiskette(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->m_cSlots.HasDiskInterface() )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateHarddisk(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->m_cSlots.HasHardDiskInterface() )
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

void CMainFrame::OnUpdateScanline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if ( g_pBoard->m_pScreen->GetScanline() == TRUE )
	{
		pCmdUI->SetCheck( TRUE );
	}
	else
	{
		pCmdUI->SetCheck( FALSE );
	}
}

void CMainFrame::OnScanline() 
{
	// TODO: Add your command handler code here
	if ( g_pBoard->m_pScreen->GetScanline() == TRUE )
	{
		g_pBoard->m_pScreen->SetScanline( FALSE );
	}
	else
	{
		g_pBoard->m_pScreen->SetScanline( TRUE );
	}
		
}

void CMainFrame::OnSuspend()
{
	// TODO: Add your command handler code here
	g_pBoard->Suspend(FALSE);
}


void CMainFrame::OnResume()
{
	// TODO: Add your command handler code here
	g_pBoard->Resume();
}

void CMainFrame::OnSuspendResume()
{
	// TODO: Add your command handler code here
	if (g_pBoard->GetIsSuspended())
		OnResume();
	else
		OnSuspend();
}

void CMainFrame::OnUpdateSuspend(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (g_pBoard->GetIsActive() && !g_pBoard->GetIsSuspended())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if (g_pBoard->GetIsSuspended())
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_SUSPEND, TRUE);
	}
	else
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_SUSPEND, FALSE);
	}
}

void CMainFrame::OnUpdateResume(CCmdUI* pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (g_pBoard->GetIsActive() && g_pBoard->GetIsSuspended())
		pCmdUI->Enable(TRUE);
	else
		pCmdUI->Enable(FALSE);
	if (g_pBoard->GetIsSuspended())
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_RESUME, FALSE);
	}
	else
	{
		m_wndToolBar.GetToolBarCtrl().HideButton(ID_RESUME, TRUE);
	}
}


//#include <ntddkbd.h>
#define KEY_MAKE                          0
#define KEY_BREAK                         1

void CMainFrame::OnRawInput(UINT nInputcode, HRAWINPUT hRawInput)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your message handler code here and/or call default

	UINT dwSize;
	DWORD dwCheck;
	USHORT wCode;

	GetRawInputData(hRawInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL)
	{
		return;
	}

	if (GetRawInputData(hRawInput, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
	{
		TRACE("GetRawInputData does not return correct size !\n");
		delete[] lpb;
		return;
	}

	RAWINPUT* raw = (RAWINPUT*)lpb;

	if (raw->header.dwType == RIM_TYPEKEYBOARD)
	{
/*
		TRACE(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n",
			raw->data.keyboard.MakeCode,
			raw->data.keyboard.Flags,
			raw->data.keyboard.Reserved,
			raw->data.keyboard.ExtraInformation,
			raw->data.keyboard.Message,
			raw->data.keyboard.VKey);
*/
		dwCheck = raw->data.keyboard.Flags & 1;
		wCode = raw->data.keyboard.MakeCode;
		if ((raw->data.keyboard.Flags & 2) != 0)
		{
			wCode |= 0x80;
		}

		if (dwCheck == KEY_MAKE)
		{
			g_cDIKeyboard.KeyDown(wCode);
		}
		else if (dwCheck == KEY_BREAK)
		{
			g_cDIKeyboard.KeyUp(wCode);
		}

	}
	else if (raw->header.dwType == RIM_TYPEMOUSE)
	{
/*
		TRACE("Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n",
			raw->data.mouse.usFlags,
			raw->data.mouse.ulButtons,
			raw->data.mouse.usButtonFlags,
			raw->data.mouse.usButtonData,
			raw->data.mouse.ulRawButtons,
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			raw->data.mouse.ulExtraInformation);
*/
		g_cDIMouse.ChangeState(raw->data.mouse.lLastX, raw->data.mouse.lLastY, raw->data.mouse.usButtonFlags);
	}

	delete[] lpb;


	CFrameWnd::OnRawInput(nInputcode, hRawInput);
}

