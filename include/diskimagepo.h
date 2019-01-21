// diskimagepo.h: interface for the CDiskImagePo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISKIMAGEPO_H__44FA5C8E_4106_4349_8249_25F81157F9FD__INCLUDED_)
#define AFX_DISKIMAGEPO_H__44FA5C8E_4106_4349_8249_25F81157F9FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "diskimagedos.h"
#include "aipcdefs.h"
/* PO logical order  0 E D C B A 9 8 7 6 5 4 3 2 1 F */
/*    physical order 0 2 4 6 8 A C E 1 3 5 7 9 B D F */

class CDiskImagePo : public CDiskImageDos
{
public:
	virtual int GetId(){ return IMAGE_PRODOS; }
	CDiskImagePo();
	virtual ~CDiskImagePo();
	static BOOL IsMyType(int hFile);
	static BOOL IsMyExtension(const char* szExt) { return IsMatch("po", szExt); }
};

#endif // !defined(AFX_DISKIMAGEPO_H__44FA5C8E_4106_4349_8249_25F81157F9FD__INCLUDED_)
