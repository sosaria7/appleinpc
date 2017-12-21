
#include "arch/frame/stdafx.h"
#include "arch/directx/dxsound.h"
#include <math.h>
#include "65c02.h"
#include "appleclock.h"
#include "aipcdefs.h"

#define STEP	0x8000

CDXSound g_DXSound;
WORD g_tblVolume[32];

CDXSound::CDXSound()
{
	int i;
	double out;
	out = MAX_AMPLITUDE;
	// reduce amplitude by 1.5 db
	for (i = 31;i > 0;i--)
	{
		g_tblVolume[i] = (WORD)(out + 0.5);	/* round to nearest */

		out /= 1.188502227;	/* = 10 ^ (1.5/20) = 1.5dB */
	}
	g_tblVolume[0] = 0;


	m_lpSndBuffer = NULL;
	m_lpDS = NULL;
	m_stPSG = NULL;
	m_wCurPos = 0;
	m_dwLastClock = 0;
	m_iRateHigh = -1;
	m_iRateLow = 0;
	m_iToggle = 0;
	m_nSampleRate = 44100;
	m_dwBufferIn = ( m_nSampleRate / 4 ) & ~0x0F;	// 3 / 12
	m_bPlay = FALSE;
	m_nVol = 29;
	m_nPan = 0;
	m_bMute = FALSE;
	m_nCheckStep = 0;
	m_nSamplesInFrame = (m_nSampleRate / 60);
}

CDXSound::~CDXSound()
{
	if ( m_lpDS != NULL )
		m_lpDS->Release();
	stPSG *st, *prev;
	st = m_stPSG;
	while( st )
	{
		prev = st;
		st = st->next;
		delete prev;
	}
}

void CDXSound::DestroyBuffer()
{
	if ( m_lpSndBuffer )
		m_lpSndBuffer->Release();
}

LPDIRECTSOUND CDXSound::Create( HWND hwnd, DWORD dwLevel )
{
	HRESULT hr;

	m_hWnd = hwnd;
	m_lpDS = NULL;
	
	hr = DirectSoundCreate( NULL, &m_lpDS, NULL );
	if( hr != DS_OK )
		return m_lpDS;

	hr = m_lpDS->SetCooperativeLevel( hwnd, dwLevel );//DSSCL_NORMAL );
	if ( hr != DS_OK ) 
	{
		m_lpDS->Release();
		m_lpDS = NULL;
	}
	else if ( !CreateBuffer() )
	{
		DestroyBuffer();
		m_lpDS->Release();
		m_lpDS = NULL;
	}
	return m_lpDS;
}

BOOL CDXSound::CreateBuffer()
{
	DSBUFFERDESC	dsbdesc;
	HRESULT hr;
	WAVEFORMATEX wfmtx;

	wfmtx.wFormatTag = WAVE_FORMAT_PCM;
	wfmtx.nChannels = SND_CHANNEL; 
    wfmtx.nSamplesPerSec = m_nSampleRate; 
    wfmtx.wBitsPerSample = BITS_PER_SAMPLE; 
    wfmtx.nBlockAlign = SND_CHANNEL * BITS_PER_SAMPLE / 8;
    wfmtx.nAvgBytesPerSec = m_nSampleRate * SND_CHANNEL * BITS_PER_SAMPLE / 8; 
    wfmtx.cbSize = 0;

	memset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) );
	dsbdesc.dwSize = sizeof( DSBUFFERDESC );
	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;

	// calcurate bytes for one period
	dsbdesc.dwBufferBytes = SND_BUFFER_SIZE;

	dsbdesc.lpwfxFormat = &wfmtx;
	dsbdesc.guid3DAlgorithm = GUID_NULL;
	// create new buffer
	hr = m_lpDS->CreateSoundBuffer( &dsbdesc, &m_lpSndBuffer, NULL );
	if( hr != DS_OK )
		return FALSE;
	m_lpSndBuffer->SetVolume( DSBVOLUME_MAX );
	m_lpSndBuffer->SetPan( DSBPAN_CENTER );
	return TRUE;
}

void CDXSound::AddPSG(CPSG *psg, int pan)
{
	stPSG *st = new stPSG;
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
	st->psg = psg;
	st->pan = pan;
	st->next = m_stPSG;
	m_stPSG = st;
}

void CDXSound::RemovePSG(CPSG *psg)
{
	stPSG *st, *last;
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
	st = m_stPSG;
	last = NULL;
	while ( st )
	{
		if ( st->psg == psg )
		{
			if ( last )
				last->next = st->next;
			else
				m_stPSG = st->next;
			delete st;
			break;
		}
		last = st;
		st = st->next;
	}
}

void CDXSound::UpdateSoundBuffer()
{
	if ( m_lpDS == NULL )
		return;
	LPVOID	lpWrite1, lpWrite2;
	DWORD	dwLength1, dwLength2;
	DWORD	dwBytesToWrite;
	HRESULT	hr;	

	dwBytesToWrite = m_nSamplesInFrame * SND_CHANNEL * BITS_PER_SAMPLE / 8;

	hr = m_lpSndBuffer->Lock( m_dwBufferIn, dwBytesToWrite, &lpWrite1, &dwLength1, &lpWrite2, &dwLength2, 0 );
	if ( hr == DSERR_BUFFERLOST )
	{
		m_lpSndBuffer->Restore();
		hr = m_lpSndBuffer->Lock( m_dwBufferIn, dwBytesToWrite, &lpWrite1, &dwLength1, &lpWrite2, &dwLength2, 0 );
	}
	if ( hr != DS_OK )
		return;

	m_dwBufferIn += dwBytesToWrite;
	if ( m_dwBufferIn >= SND_BUFFER_SIZE )
		m_dwBufferIn -= SND_BUFFER_SIZE;

	memcpy( lpWrite1, m_abyBuffer, dwLength1 );
	dwBytesToWrite -= dwLength1;
	if ( dwBytesToWrite != 0 )
		memcpy( lpWrite2, m_abyBuffer+dwLength1, dwLength2 );

	m_lpSndBuffer->Unlock( lpWrite1, dwLength1, lpWrite2, dwLength2 );
}

#define CHECK_STEP	8

void CDXSound::Clock()
{
	int i, j;
	int channels;
	DWORD interval;
	long valueL, valueR;
	int nVolR, nVolL;
	DWORD dwDxBuffered, dwBufferedBytes;
	DWORD dwReadPos, dwWritePos;
	DWORD dwStep;

	if ( m_lpDS == NULL || m_lpSndBuffer == NULL )
		return;

	interval = g_pBoard->GetClock() - m_dwLastClock;

	dwStep = (DWORD)(g_dwCPS / 60 / CHECK_STEP);
	if (interval < dwStep)
		return;
	m_dwLastClock += ((DWORD)(interval / dwStep)) * dwStep;

	m_lpSndBuffer->GetCurrentPosition(&dwReadPos, &dwWritePos);
	dwDxBuffered = dwWritePos - dwReadPos;
	if (dwReadPos > dwWritePos)
		dwDxBuffered += SND_BUFFER_SIZE;
	dwBufferedBytes = m_dwBufferIn - dwReadPos;
	if (dwReadPos > m_dwBufferIn)
		dwBufferedBytes += SND_BUFFER_SIZE;

	m_nCheckStep++;
	if (m_nCheckStep < CHECK_STEP && dwBufferedBytes >= m_nSampleRate * 5 / 24 )		// 2.5 / 12
	{
		// enough buffer
		return;
	}

	m_nCheckStep = 0;
	if (m_iRateHigh < 0 || dwBufferedBytes >= (m_nSampleRate / 2) || dwBufferedBytes < dwDxBuffered)	// 6 / 12
	{
		m_iRateHigh = 0;
		m_iRateLow = 0;
		m_dwBufferIn = dwReadPos + ((m_nSampleRate / 4) & ~0x0F);	// 3 / 12
	}
	else if (dwBufferedBytes >= (m_nSampleRate * 5 / 12))
	{
		// enough buffer
		return;
	}
	else if (dwBufferedBytes >= m_nSampleRate / 3)	// 4 / 12
	{
		m_iRateLow = 0;
		m_iRateHigh = m_nSampleRate / 1200;
	}
	else if (dwBufferedBytes < m_nSampleRate / 6) // 2 / 12
	{
		m_iRateHigh = 0;
		m_iRateLow = m_nSampleRate / 1200;
	}
	else		// 2/12 ~ 4/12
	{
		m_iRateHigh = 0;
		m_iRateLow = 0;
	}

	m_nSamplesInFrame = (m_nSampleRate / 60) - m_iRateHigh + m_iRateLow;
	if (m_nSamplesInFrame < 10)
		m_nSamplesInFrame = 10;
	else if ((unsigned)m_nSamplesInFrame > m_nSampleRate / 4)		// buffer size
		m_nSamplesInFrame = m_nSampleRate / 4;


	if ( m_nPan < 0 )
	{
		nVolL = m_nVol;
		nVolR = ( 6 + m_nPan ) * m_nVol / 6;
	}
	else
	{
		nVolR = m_nVol;
		nVolL = ( 6 - m_nPan ) * m_nVol / 6;
	}

	stPSG *st;
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);

	st = m_stPSG;


	while( st )
	{
		st->psg->Update( m_nSamplesInFrame );
		st = st->next;
	}

	int val=0;
	for( i = 0; i < m_nSamplesInFrame; i++ )
	{
		valueL  = valueR = 0;
		if ( !m_bMute )
		{
			st = m_stPSG;
			while( st )
			{
				if ( !st->psg->m_bMute )
				{
					channels = st->psg->m_nNumOfBuf;
					for( j = 0; j < channels; j++ )
					{
						if ( st->pan != -1 )		// right or center
							valueR += (int)st->psg->m_lpwBuf[j][i] * g_tblVolume[st->psg->m_iVol] / (int)( MAX_AMPLITUDE * 1.5 ) ;
						if ( st->pan != 1 )			// left or center
							valueL += (int)st->psg->m_lpwBuf[j][i] * g_tblVolume[st->psg->m_iVol] / (int)( MAX_AMPLITUDE * 1.5 );
					}
				}
				st = st->next;
			}
			valueR = valueR * g_tblVolume[nVolR] / MAX_AMPLITUDE;
			valueL = valueL * g_tblVolume[nVolL] / MAX_AMPLITUDE;
			
			if ( valueR > MAX_AMPLITUDE )
				valueR = MAX_AMPLITUDE;
			else if ( valueR < -MAX_AMPLITUDE )
				valueR = -MAX_AMPLITUDE;
			if ( valueL > MAX_AMPLITUDE )
				valueL = MAX_AMPLITUDE;
			else if ( valueL < -MAX_AMPLITUDE )
				valueL = -MAX_AMPLITUDE;
		}

#if BITS_PER_SAMPLE == 8
		m_abyBuffer[i<<1] = (BYTE)valueL;
		m_abyBuffer[(i<<1)+1] = (BYTE)valueR;
#else
		((LPWORD)m_abyBuffer)[i<<1] = (WORD)valueL;
		((LPWORD)m_abyBuffer)[(i<<1)+1] = (WORD)valueR;
#endif
	}
	UpdateSoundBuffer();
}

void CDXSound::Toggle()
{
	m_iToggle = 2;
}

void CDXSound::Suspend()
{
	if ( !m_bPlay )
		return;
	m_lpSndBuffer->Stop();
	m_bPlay = FALSE;
}

void CDXSound::Resume()
{
	if ( m_bPlay )
		return;
	if ( m_lpDS == NULL || m_lpSndBuffer == NULL )
		return;
	LPVOID lpWrite1, lpWrite2;
	DWORD dwLength1, dwLength2;
	m_lpSndBuffer->SetCurrentPosition( 0 );
	m_dwBufferIn = ( m_nSampleRate / 4 ) & ~0x0F;	// 3 / 12
	m_lpSndBuffer->Lock( 0, 0, &lpWrite1, &dwLength1, &lpWrite2, &dwLength2, DSBLOCK_ENTIREBUFFER );
	memset( lpWrite1, 0, dwLength1 );
	m_lpSndBuffer->Unlock( lpWrite1, dwLength1, lpWrite2, dwLength2 );
	m_nSamplesInFrame = m_nSampleRate / 60;		// frames per second = 60
	m_lpSndBuffer->Play( 0, 0, DSBPLAY_LOOPING );
	m_bPlay = TRUE;
	m_iRateHigh = -1;
	Sleep(1);
}


void CDXSound::SetVolume(int nVol)
{
	if ( nVol > 31 )
		nVol = 31;
	if ( nVol < 0 )
		nVol = 0;
	m_nVol = nVol;
}

int CDXSound::GetVolume()
{
	return m_nVol;
}

void CDXSound::SetPan(int nPan)
{
	if ( nPan < -6 )
		nPan = -6;
	else if ( nPan > 6 )
		nPan = 6;
	m_nPan = nPan;
}

int CDXSound::GetPan()
{
	return m_nPan;
}

void CDXSound::ChangeSampleRate(unsigned int nSampleRate)
{
	if ( nSampleRate != 44100 && nSampleRate != 22050 && nSampleRate != 11025 )
		return;
	if ( nSampleRate == m_nSampleRate )
		return;
	DestroyBuffer();
	m_nSampleRate = nSampleRate;
	CreateBuffer();

	stPSG *st;
	CLockMgr<CCSWrapper> guard(m_Lock, TRUE);
	st = m_stPSG;

	while( st )
	{
		st->psg->ChangeSampleRate();
		st = st->next;
	}

	if ( m_bPlay )
	{
		m_bPlay = FALSE;
		Resume();
	}
}
