// diskimagepo.cpp: implementation of the CDiskImagePo class.
//
//////////////////////////////////////////////////////////////////////

#include "arch/frame/stdafx.h"
#include "diskimagepo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const static BYTE sg_abyLogicalOrder[] = {
	0x00, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x0F
};
/* PO logical order  0 E D C B A 9 8 7 6 5 4 3 2 1 F */
/*    physical order 0 2 4 6 8 A C E 1 3 5 7 9 B D F */

CDiskImagePo::CDiskImagePo()
{
	m_pbyLogicalOrder = (BYTE*)sg_abyLogicalOrder;
}

CDiskImagePo::~CDiskImagePo()
{

}

BOOL CDiskImagePo::IsMyType(int hFile, const char* szExt )
{
	// 확장자로 확인
	if ( IsMatch( "po", szExt ) )
		return TRUE;
	if ( IsMatch( "do;dsk;nib;apl;iie;prg", szExt ) )
		return FALSE;
	// 모르는 확장자 인경우 내용 확인
	// check for a dos order image of a dos diskette
	return CheckImage( hFile, sg_abyLogicalOrder );
}
