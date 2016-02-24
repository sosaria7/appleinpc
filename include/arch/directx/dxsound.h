#ifndef __DXSOUND_H
#define __DXSOUND_H

#include <mmreg.h>
#include <dsound.h>

#define MAX_CHARACTER	5
#define MAX_ACTION		7

#define SAMPLES_PER_SEC		44100
#define BITS_PER_SAMPLE		16
#define SND_CHANNEL			2

#define SND_BUFFER_SIZE		( SAMPLES_PER_SEC * ( BITS_PER_SAMPLE / 8 ) * SND_CHANNEL )
#define MAX_AMPLITUDE		( ( 1 << ( BITS_PER_SAMPLE - 1 ) ) - 1 )

#include "psg.h"

struct stPSG
{
	CPSG	*psg;
	int		pan;
	stPSG	*next;
};

class CDXSound
{
public:
	void RemovePSG( CPSG *psg );
	void AddPSG( CPSG *psg, int pan );
	BOOL CreateBuffer();
	void DestroyBuffer();
	CDXSound();
	~CDXSound();

	LPDIRECTSOUND Create( HWND hwnd, DWORD dwLevel = DSSCL_NORMAL );

	void ChangeSampleRate(unsigned int nSampleRate);
	unsigned int m_nSampleRate;
	void SetVolume(int nVol);
	int GetVolume();
	void SetPan(int nPan);
	int GetPan();
	void Resume();
	void Suspend();
	void Toggle();
	void Clock();
	void UpdateSoundBuffer();

public:
	BOOL m_bMute;
	WORD m_wCurPos;
	int m_nSamplesInFrame;

protected:
	int m_nToggleValue;
	BOOL m_bPlay;
	BOOL m_iToggle;
	int m_iRateLow;
	int m_iRateHigh;
	BYTE m_abyBuffer[SND_BUFFER_SIZE];
	DWORD m_dwLastClock;
	DWORD m_dwBufferIn;
	HWND m_hWnd;
	LPDIRECTSOUND m_lpDS;
	LPDIRECTSOUNDBUFFER m_lpSndBuffer;
	stPSG* m_stPSG;
	int m_nVol;
	int m_nPan;
};



extern CDXSound g_DXSound;

extern WORD g_tblVolume[32];


#endif