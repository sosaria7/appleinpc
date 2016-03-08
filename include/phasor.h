// Phasor.h: interface for the CPhasor class.

//

//////////////////////////////////////////////////////////////////////



#if !defined(AFX_PHASOR_H__84DFED54_4C93_4700_9642_F8C00E6EDCAD__INCLUDED_)

#define AFX_PHASOR_H__84DFED54_4C93_4700_9642_F8C00E6EDCAD__INCLUDED_



#if _MSC_VER > 1000

#pragma once

#endif // _MSC_VER > 1000

#include "Card.h"
#include "6522.h"
#include "8913.h"
#ifdef HAVE_VOTRAX
	#include "votrax.h"
#endif

class CPhasor : public CCard
{

public:
	void SetDipSwitch( int nDipSwitch );
	virtual void Reset();
	virtual void PowerOn();
	DECLARE_DYNAMIC( CPhasor );

	void Clock(int clock);
	CPhasor();
	virtual ~CPhasor();

	virtual void WriteRom(WORD addr, BYTE data);
	virtual BYTE ReadRom(WORD addr);
	virtual void InitRomImage();

	virtual void Write(WORD addr, BYTE data);
	virtual BYTE Read(WORD addr);
 	virtual void Configure();

	virtual void Serialize(CArchive &ar);

protected:
	BYTE m_byMode;
	C6522	m_6522[2];
	C8913	m_8913[4];
#ifdef HAVE_VOTRAX
	CVotrax	m_cVotrax[2];
#endif
};



#endif // !defined(AFX_PHASOR_H__84DFED54_4C93_4700_9642_F8C00E6EDCAD__INCLUDED_)

