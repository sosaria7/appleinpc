// 65c02.h: interface for the C65c02 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __6502_H__
#define __6502_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

// gloval value


#include "cpu.h"
#include "65c02.h"

class C6502 : public C65c02
{
public:
	C6502();
	virtual ~C6502();

	int Process();
	void Serialize(CArchive &archive);

protected:

};

#endif
