// Iou.h: interface for the CIou class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOU_H__5014797B_2A4B_4AAB_A519_35C564856FCC__INCLUDED_)
#define AFX_IOU_H__5014797B_2A4B_4AAB_A519_35C564856FCC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIou : public CObject
{
public:
	virtual void WriteMem16(int nAddr, WORD wData) = NULL;
	virtual WORD ReadMem16(int nAddr) = NULL;
	CIou();
	virtual ~CIou();
};

#endif // !defined(AFX_IOU_H__5014797B_2A4B_4AAB_A519_35C564856FCC__INCLUDED_)
