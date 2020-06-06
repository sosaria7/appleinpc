#ifndef __CLOCKLISTENER_H__
#define __CLOCKLISTENER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CClockListener
{
public:
	virtual void Clock(unsigned int clock) = 0;
};

#endif

