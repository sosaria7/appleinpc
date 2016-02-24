//---------------------------------------------------------------------------
//
// CustomThread.h
//
// SUBSYSTEM: 
//				
// MODULE:    
//
// DESCRIPTION:
//
// AUTHOR:		Ivo Ivanov
//                                                                         
//---------------------------------------------------------------------------
#if !defined(_CUSTOMTHREAD_H_)
#define _CUSTOMTHREAD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//---------------------------------------------------------------------------
//
// Includes
//
//---------------------------------------------------------------------------
#include <tchar.h>
#include <windows.h>

#include "lockmgr.h"

#define THREAD_EVENT_SHUTDOWN	0
#define THREAD_EVENT_RESUME		1
#define THREAD_EVENT_WAKEUP		2
//---------------------------------------------------------------------------
//
// class CCustomThread  
//
// It is an abstract class that enables creation of separate threads of 
// execution.
//                                                                         
//---------------------------------------------------------------------------
class CCustomThread
{
public:
	int GetPriority();
	void SetPriority( int nPriority );
	virtual void Resume();
	virtual void Suspend(BOOL bWait=FALSE);
	CCustomThread(TCHAR* pszThreadGuid);
	virtual ~CCustomThread();
	//
	// Activate / Stop the thread 
	//
	void SetActive(BOOL bValue, BOOL bWait=TRUE);
	//
	// Indicates whether the driver has been activated
	//
	BOOL GetIsActive();
	//
	// Setup the attribute
	//
	void SetIsActive(BOOL bValue);
	//
	BOOL GetIsSuspended()
	{
		return m_bSuspended;
	}
	// 메시지 처리등을 위해 suspend되어 있는 thread를 일시적으로 깨운다.
	// 다음 SuspendHere 함수에서 다시 suspend된다.
	void WakeUp();

private:
	//
	// Primary thread entry point
	//
	static unsigned __stdcall ThreadFunc(void* pvParam);
	//
	// Thread attributes
	//
	BOOL			m_bThreadActive;
	DWORD			m_dwThreadId;
	static HANDLE	sm_hThread;
	CCSWrapper		m_CritSec;
	CCSWrapper		m_Running;
	BOOL			m_bSuspended;
	HANDLE			m_hEvents[3];
	int				m_nPriority;
	HANDLE			m_hThread;
	//
	// The name of the shut down event
	//
	TCHAR         m_szThreadGuid[255];
protected:
	//
	// Return the handle to the thread's shut down event
	//
	HANDLE GetShutdownEvent() const
	{
		return m_hEvents[THREAD_EVENT_SHUTDOWN];
	}
	//
	// if suspend is requested, suspend here and return TRUE
	//
	inline BOOL	SuspendHere()
	{
		if ( m_hEvents[THREAD_EVENT_RESUME] != NULL
			&& ::WaitForSingleObject( m_hEvents[THREAD_EVENT_RESUME], 0 ) == WAIT_TIMEOUT )
		{
			m_bSuspended = TRUE;
			::WaitForMultipleObjects( 3, m_hEvents, FALSE, INFINITE );
			m_bSuspended = FALSE;
			return TRUE;
		}
		return FALSE;
	}

	inline BOOL ShutdownHere()
	{
		if ( m_hEvents[THREAD_EVENT_SHUTDOWN] != NULL
			&& ::WaitForSingleObject( m_hEvents[THREAD_EVENT_SHUTDOWN], 0 ) == WAIT_OBJECT_0 )
			return TRUE;
		return FALSE;
	}
	//
	// A user supplied implementation of the thread function.
	// Override Run() and insert the code that should be executed when 
	// the thread runs.
	//
	virtual void Run() = 0;
	//
	// Perform action prior to activate the thread
	//
	virtual BOOL OnBeforeActivate();
	//
	// Called after the thread function exits
	//
	virtual void OnAfterDeactivate();
};

#endif // !defined(_CUSTOMTHREAD_H_)
//----------------------------End of the file -------------------------------