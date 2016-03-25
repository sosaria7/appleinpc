// PSG.h: interface for the CPSG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSG_H__EBB84D23_6A7F_42E7_89EB_3528943030D0__INCLUDED_)
#define AFX_PSG_H__EBB84D23_6A7F_42E7_89EB_3528943030D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPSG : public CObject
{
public:
	virtual long GetPulse(){ return 0; };
	CPSG();
	virtual ~CPSG();
	virtual void Update(int length){ };
	virtual void ChangeSampleRate(){ };
	virtual void SetVolume( int iVol )
	{
		if ( iVol > 31 )
			iVol = 31;
		if ( iVol < 0 )
			iVol = 0;
		m_iVol = iVol;
	}
	virtual void Serialize( CArchive &ar );

	WORD m_awVolume[32];

public:
	BOOL m_bMute;
	WORD **m_lpwBuf;
	int	m_iVol;
	unsigned int m_nNumOfBuf;
};

#endif // !defined(AFX_PSG_H__EBB84D23_6A7F_42E7_89EB_3528943030D0__INCLUDED_)
