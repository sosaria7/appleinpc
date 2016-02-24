//---------------------------------------------------------------------------
//
// CustomThread.cpp
//
// SUBSYSTEM: 
//              Monitoring process creation and termination  
// MODULE:    
//              Thread management 
//
// DESCRIPTION:
//              This is an abstract class that enables creation of separate 
//              threads of execution in an application. 
//
// AUTHOR:		Ivo Ivanov
//                                                                         
//---------------------------------------------------------------------------

#include "arch/customthread.h"
#include <process.h>
#include <assert.h>
#include <stdio.h>

//---------------------------------------------------------------------------
//
// Thread function prototype
//
//---------------------------------------------------------------------------
typedef unsigned (__stdcall *PTHREAD_START) (void *);

//---------------------------------------------------------------------------
//
// class CCustomThread 
//
// It is an abstract class that enables creation of separate threads of 
// execution.
//                                                                         
//---------------------------------------------------------------------------

HANDLE CCustomThread::sm_hThread = NULL;

CCustomThread::CCustomThread(TCHAR* pszThreadGuid):
	m_bThreadActive(FALSE),
	m_bSuspended(FALSE),
	m_dwThreadId(NULL)
{
	TCHAR szBuff[255];
	if (NULL != pszThreadGuid)
		_tcscpy(m_szThreadGuid, pszThreadGuid);
	else
		_tcscpy(m_szThreadGuid, TEXT(""));
	sprintf( m_szThreadGuid, "%s_%ld", m_szThreadGuid, ::GetTickCount() );
	
	m_hEvents[THREAD_EVENT_SHUTDOWN] = ::CreateEvent(NULL, FALSE, FALSE, m_szThreadGuid);

	sprintf( szBuff, "%s_resume", m_szThreadGuid );
	m_hEvents[THREAD_EVENT_RESUME] = ::CreateEvent(NULL, TRUE, TRUE, szBuff);

	sprintf( szBuff, "%s_wakeup", m_szThreadGuid );
	m_hEvents[THREAD_EVENT_WAKEUP] = ::CreateEvent(NULL, FALSE, TRUE, szBuff);

	m_nPriority = THREAD_PRIORITY_NORMAL;
	m_hThread = (HANDLE)-1;
}

CCustomThread::~CCustomThread()
{	
	int i;
	SetActive( FALSE );
	for( i = 0; i < sizeof(m_hEvents)/sizeof(HANDLE); i++ )
	{
		if ( m_hEvents[i] )
		{
			CloseHandle( m_hEvents[i] );
			m_hEvents[i] = NULL;
		}
	}
}

//
// Activate / Stop the thread 
//
void CCustomThread::SetActive(BOOL bValue, BOOL bWait)
{
	BOOL bCurrent = GetIsActive();

	if (bValue != bCurrent)
	{
		if (!bCurrent)
		{

			ULONG ulResult = _beginthreadex(
				(void *)NULL,
				(unsigned)0,
				(PTHREAD_START)CCustomThread::ThreadFunc,
				(PVOID)this,
				(unsigned)0,
				(unsigned *)&m_dwThreadId
				);
			m_hThread = (HANDLE)ulResult;

			if (ulResult != -1)
			{
				SetThreadPriority( m_hThread, m_nPriority );
				//
				// Wait until the thread gets activated
				//
				if ( bWait )
					while (!GetIsActive())
						::Sleep(100);
			}

		} 
		else
		{
			if ( GetIsActive() )
			{
				::SetEvent( m_hEvents[THREAD_EVENT_SHUTDOWN] );
				if ( bWait )
					CLockMgr<CCSWrapper> guard(m_Running, TRUE);
				//
				// Wait until the thread is done
				//
//				while (GetIsActive())
//					;
			} // if
		}
	} // if
}

//
// Indicates whether the driver has been activated
//
BOOL CCustomThread::GetIsActive()
{
	CLockMgr<CCSWrapper> lockMgr(m_CritSec, TRUE);	
	return m_bThreadActive;
}

//
// Setup the attribute
//
void CCustomThread::SetIsActive(BOOL bValue)
{
	CLockMgr<CCSWrapper> lockMgr(m_CritSec, TRUE);	
	m_bThreadActive = bValue;
}


//
// Primary thread entry point
//
unsigned __stdcall CCustomThread::ThreadFunc(void* pvParam)
{
	CCustomThread* pMe = (CCustomThread*)( pvParam );
	{
		CLockMgr<CCSWrapper> guard(pMe->m_Running, TRUE);
		
		//
		// Perform action prior to activate the thread
		//
		if (!pMe->OnBeforeActivate())
		{
			_endthreadex(0);
			return 0;
		}
		
		// retrieves a pseudo handle for the current thread
		sm_hThread = GetCurrentThread();
		try
		{
			pMe->SetIsActive( TRUE );
			// Execute the user supplied method
			pMe->Run();
		}
		catch (...)
		{
			// Handle all exceptions
		}
		pMe->SetIsActive( FALSE );
		
		//
		// Called after the thread function exits
		//
		pMe->OnAfterDeactivate();
	}
	_endthreadex(0);
	return 0;
}


//
// Perform action prior to activate the thread
//
BOOL CCustomThread::OnBeforeActivate()
{
	// Provide default implementation
	return TRUE;
}

//
// Called after the thread function exits
//
void CCustomThread::OnAfterDeactivate()
{
	// Do nothing
}

//----------------------------End of the file -------------------------------


void CCustomThread::Suspend(BOOL bWait)
{
	::ResetEvent( m_hEvents[THREAD_EVENT_RESUME] );
	if ( bWait && m_bThreadActive )
	{
		while ( !m_bSuspended )
			::Sleep(100);
	}
}

void CCustomThread::Resume()
{
	::SetEvent( m_hEvents[THREAD_EVENT_RESUME] );
}

void CCustomThread::SetPriority(int nPriority)
{
	m_nPriority = nPriority;
	if ( GetIsActive() )
		SetThreadPriority( m_hThread, nPriority );
}

int CCustomThread::GetPriority()
{
	if ( GetIsActive() )
		return GetThreadPriority( m_hThread );
	return m_nPriority;
}

void CCustomThread::WakeUp()
{
	::SetEvent( m_hEvents[THREAD_EVENT_WAKEUP] );
}