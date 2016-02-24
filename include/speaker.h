// Speaker.h: interface for the CSpeaker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPEAKER_H__0EF0E8FA_253C_42CF_A9E4_12FD5313C4B5__INCLUDED_)
#define AFX_SPEAKER_H__0EF0E8FA_253C_42CF_A9E4_12FD5313C4B5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "psg.h"

class CSpeaker : public CPSG  
{
public:
	void ChangeSampleRate();
	void Toggle();
	void Update(int length);
	CSpeaker();
	virtual ~CSpeaker();

protected:
	BOOL m_bToggle;
	int m_iLastUpdatePos;
	DWORD m_dwLastAppleClock;
	int m_iUpdateStep;
};

#endif // !defined(AFX_SPEAKER_H__0EF0E8FA_253C_42CF_A9E4_12FD5313C4B5__INCLUDED_)
