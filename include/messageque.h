// MessageQue.h: interface for the CMessageQue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEQUE_H__8822C252_ACB9_45B2_9000_8D91AE55DE8E__INCLUDED_)
#define AFX_MESSAGEQUE_H__8822C252_ACB9_45B2_9000_8D91AE55DE8E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMessageQue
{
public:
	void ClearQueue();
	int GetMesg();
	void AddMessage(int message);
	CMessageQue();
	virtual ~CMessageQue();

protected:
	BYTE m_byteTail;
	BYTE m_byteHead;
	int m_anQueue[256];
};

#endif // !defined(AFX_MESSAGEQUE_H__8822C252_ACB9_45B2_9000_8D91AE55DE8E__INCLUDED_)
