// diskimagenib.h: interface for the CDiskImageNib class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DISKIMAGENIB_H
#define __DISKIMAGENIB_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "diskimagedos.h"
#include "aipcdefs.h"
/* PO logical order  0 E D C B A 9 8 7 6 5 4 3 2 1 F */
/*    physical order 0 2 4 6 8 A C E 1 3 5 7 9 B D F */

class CDiskImageNib : public CDiskImage
{
public:
	virtual int GetId(){ return IMAGE_NIBBLE; }
	CDiskImageNib();
	virtual ~CDiskImageNib();
	static BOOL IsMyType(int hFile, const char* szExt );
protected:
	static BOOL CheckImage(int hFile);
	virtual BOOL ReadBuffer();
	virtual void SaveBuffer();
	virtual int Mount(const char* szFileName);
	virtual void Umount();

protected:
	BOOL StoreData(BYTE* pbyData);
};

#endif
