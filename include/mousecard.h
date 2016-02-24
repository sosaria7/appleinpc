// MouseCard.h: interface for the CMouseCard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUSECARD_H__DB1618B4_296C_4381_8E1F_BC5D1B4A3D4D__INCLUDED_)
#define AFX_MOUSECARD_H__DB1618B4_296C_4381_8E1F_BC5D1B4A3D4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Card.h"
#include "6821.h"

class CMouseCard : public CCard  
{
public:
protected:
	C6821	m_c6821;

public:
	CMouseCard();
	virtual ~CMouseCard();

	void Reset();
	void Clock(int nClock);
	BYTE ReadRom( WORD wAddr );
	void Write( WORD wAddr, BYTE byData );
	BYTE Read( WORD wAddr );
	DECLARE_DYNAMIC( CMouseCard );

	void Serialize( CArchive &ar );

protected:
	void OnMouseEvent();
	int m_nDataLen;
	BYTE m_byMode;
	void OnWrite();
	void OnCommand();
	void On6821_B(BYTE byData);
	void On6821_A(BYTE byData );

	void InitRomImage();
	friend WRITE_HANDLER( M6821_Listener_A );
	friend WRITE_HANDLER( M6821_Listener_B );
	friend CALLBACK_HANDLER( MouseHandler );

	BYTE	m_by6821B;
	BYTE	m_by6821A;
	BYTE	m_byBuff[8];			// m_byBuff[0] is mode byte
	int		m_nBuffPos;

	BYTE	m_byState;
	int		m_nX;
	int		m_nY;
	BOOL	m_bBtn0;
	BOOL	m_bBtn1;

	BOOL	m_bVBL;
};

#endif // !defined(AFX_MOUSECARD_H__DB1618B4_296C_4381_8E1F_BC5D1B4A3D4D__INCLUDED_)
