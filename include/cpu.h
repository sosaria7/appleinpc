// Cpu.h: interface for the CCpu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CPU_H__4143AA29_8CA5_4EBB_9EDB_87147331C999__INCLUDED_)
#define AFX_CPU_H__4143AA29_8CA5_4EBB_9EDB_87147331C999__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "clocklistener.h"

#define	SIG_CPU_SHUTDOWN	(1<<0)
#define	SIG_CPU_RES			(1<<1)
#define	SIG_CPU_NMI			(1<<2)
#define	SIG_CPU_IRQ			(1<<3)
#define SIG_CPU_WAIT		(1<<4)

CALLBACK_HANDLER(Clock);

class CCpu : public CObject
{
public:
	CCpu();
	virtual ~CCpu();
	virtual void Assert_NMI();
	virtual void Assert_IRQ();
	virtual void Shutdown();
	virtual void Reset();
	virtual void Wait();
	virtual int Process() = NULL;

	virtual void Serialize( CArchive& ar );
	void setClockListener(CClockListener* clockListener) { m_clockListener = clockListener; };

	unsigned m_uException_Register;
protected:
	int PendingIRQ;
	CClockListener* m_clockListener;
};

#endif // !defined(AFX_CPU_H__4143AA29_8CA5_4EBB_9EDB_87147331C999__INCLUDED_)
