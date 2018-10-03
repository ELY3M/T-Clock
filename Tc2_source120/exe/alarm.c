/*-------------------------------------------
  alarm.c
    Sound a wave, a media file, or open a file
    KAZUBON 1997-1999
---------------------------------------------*/

#include "tclock.h"

static PALARMSTRUCT pAS = NULL;
static int maxAlarm = 1;
static BOOL bJihou, bJihouRepeat, bJihouBlink;

static WAVEFORMATEX *pFormat = NULL;
static HWAVEOUT hWaveOut = NULL;
static HPSTR pData = NULL;
static WAVEHDR wh;

static BOOL bMCIPlaying = FALSE;
static int countPlay = 0, countPlayNum = 0;
static int nTrack;
static BOOL bTrack;
BOOL bPlayingNonstop = FALSE;

BOOL PlayWave(HWND hwnd, char *fname, DWORD dwLoops);
int PlayMCI(HWND hwnd, int nt);
void StopWave(void);

/*------------------------------------------------
　アラーム/時報の初期化
--------------------------------------------------*/
void InitAlarm(void)
{
	char subkey[20];
	int i;

	maxAlarm = GetMyRegLong("", "AlarmNum", 0);
	if(maxAlarm < 1) maxAlarm = 0;
	if(pAS) free(pAS); pAS = NULL;
	if(maxAlarm > 0)
	{
		pAS = malloc(sizeof(ALARMSTRUCT) * maxAlarm);
		for(i = 0; i < maxAlarm; i++)
		{
			wsprintf(subkey, "Alarm%d", i + 1);
			
			GetMyRegStr(subkey, "Name", pAS[i].name, 40, "");
			pAS[i].bAlarm = GetMyRegLong(subkey, "Alarm", FALSE);
			pAS[i].hour = GetMyRegLong(subkey, "Hour", 12);
			pAS[i].minute = GetMyRegLong(subkey, "Minute", 0);
			GetMyRegStr(subkey, "File", pAS[i].fname, 1024, "");
			pAS[i].bHour12 = GetMyRegLong(subkey, "Hour12", TRUE);
			pAS[i].bRepeat = GetMyRegLong(subkey, "Repeat", FALSE);
			pAS[i].bBlink = GetMyRegLong(subkey, "Blink", FALSE);
			pAS[i].days = GetMyRegLong(subkey, "Days", 0x7f);
		}
	}
	
	bJihou = GetMyRegLong("", "Jihou", FALSE);
	if(bJihou)
	{
		bJihouRepeat = GetMyRegLong("", "JihouRepeat", FALSE);
		bJihouBlink = GetMyRegLong("", "JihouBlink", FALSE);
	}
}

void EndAlarm(void)
{
	if(pAS) free(pAS); pAS = NULL;
	StopFile();
}

/*------------------------------------------------
　アラーム/時報を鳴らす
--------------------------------------------------*/
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st)
{
	int i, rep, h, fday;
	
	if(st->wDayOfWeek > 0)
		fday = 1 << (st->wDayOfWeek - 1);
	else fday = 1 << 6;
	
	for(i = 0; i < maxAlarm; i++)
	{
		if(!pAS[i].bAlarm) continue;
		h = st->wHour;
		if(pAS[i].bHour12)
		{
			if(h == 0) h = 12;
			else if(h >= 13) h -= 12;
		}
		
		if(pAS[i].hour == h && pAS[i].minute == st->wMinute
			&& (pAS[i].days & fday))
		{
			if(pAS[i].bBlink)
				PostMessage(g_hwndClock, CLOCKM_BLINK, FALSE, 0);
			
			if(pAS[i].fname[0])
			{
				if(pAS[i].bRepeat) rep = -1; else rep = 0;
				if(PlayFile(hwnd, pAS[i].fname, rep)) return;
			}
		}
	}
	
	if(bJihou && st->wMinute == 0)
	{
		char fname[1024];
		h = st->wHour;
		if(bJihouBlink)
			PostMessage(g_hwndClock, CLOCKM_BLINK, TRUE, 0);
		if(h == 0) h = 12;
		else if(h >= 13) h -= 12;
		GetMyRegStr("", "JihouFile", fname, 1024, "");
		if(fname[0])
		{
			if(bJihouRepeat) rep = h; else rep = 0;
			PlayFile(hwnd, fname, rep);
		}
	}
}

/*------------------------------------------------
　ファイルの再生/実行   dwLoops：繰り返し回数
--------------------------------------------------*/
BOOL PlayFile(HWND hwnd, char *fname, DWORD dwLoops)
{
	BOOL b;
	b = GetMyRegLong("", "MCIWave", FALSE);
	
	if(*fname == 0) return FALSE;
	
	if(!b && ext_cmp(fname, "wav") == 0)
	{
		if(bMCIPlaying) return FALSE;
		return PlayWave(hwnd, fname, dwLoops);
	}
	else if(IsMMFile(fname))
	{
		char command[1024];
		if(bMCIPlaying) return FALSE;
		strcpy(command, "open \"");
		strcat(command, fname);
		strcat(command, "\" alias myfile");
		if(mciSendString(command, NULL, 0, NULL) == 0)
		{
			strcpy(command, "set myfile time format ");
			if(_stricmp(fname, "cdaudio") == 0 || ext_cmp(fname, "cda") == 0)
			{
				strcat(command, "tmsf"); bTrack = TRUE;
			}
			else
			{
				strcat(command, "milliseconds"); bTrack = FALSE;
			}
			mciSendString(command, NULL, 0, NULL);
			
			nTrack = -1;
			if(ext_cmp(fname, "cda") == 0)
			{
				char* p;
				p = fname; nTrack = 0;
				while(*p)
				{
					if('0' <= *p && *p <= '9') nTrack = nTrack * 10 + *p - '0';
					p++;
				}
			}
			if(PlayMCI(hwnd, nTrack) == 0)
			{
				bMCIPlaying = TRUE;
				countPlay = 1; countPlayNum = dwLoops;
			}
			else mciSendString("close myfile", NULL, 0, NULL);
		}
		return bMCIPlaying;
	}
	else ExecFile(hwnd, fname);
	return FALSE;
}

int PlayMCI(HWND hwnd, int nt)
{
	char command[80], s[80];
	char start[40], end[40];
	
	strcpy(command, "play myfile");
	if(nt >= 0)
	{
		wsprintf(s, "status myfile position track %d", nt);
		if(mciSendString(s, start, 40, NULL) == 0)
		{
			strcat(command, " from ");
			strcat(command, start);
			wsprintf(s, "status myfile position track %d", nt+1);
			if(mciSendString(s, end, 40, NULL) == 0)
			{
				strcat(command, " to ");
				strcat(command, end);
			}
		}
	}
	strcat(command, " notify");
	return mciSendString(command, NULL, 0, hwnd);
}

/*------------------------------------------------
　再生停止
--------------------------------------------------*/
void StopFile(void)
{
	StopWave();
	if(bMCIPlaying)
	{
		mciSendString("stop myfile", NULL, 0, NULL);
		mciSendString("close myfile", NULL, 0, NULL);
		bMCIPlaying = FALSE;
		countPlay = 0; countPlayNum = 0;
	}
	bPlayingNonstop = FALSE;
}

void OnMCINotify(HWND hwnd)
{
	if(bMCIPlaying)
	{
		if(countPlay < countPlayNum || countPlayNum < 0)
		{
			mciSendString("seek myfile to start wait", NULL, 0, NULL);
			if(PlayMCI(hwnd, nTrack) == 0)
			{
				countPlay++;
			}
			else
				StopFile();
		}
		else StopFile();
	}
}

/*--------------------------------------------------------
  Retreive a file name and option from a command string
----------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* opt)
{
	const char *p, *pe;
	char *pd;
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	
	p = command; pd = fname;
	pe = NULL;
	for(; ;)
	{
		if(*p == ' ' || *p == 0)
		{
			*pd = 0;
			hfind = FindFirstFile(fname, &fd);
			if(hfind != INVALID_HANDLE_VALUE)
			{
				FindClose(hfind);
				pe = p;
			}
			if(*p == 0) break;
		}
		*pd++ = *p++;
	}
	if(pe == NULL) pe = p;
	
	p = command; pd = fname;
	for(; p != pe; )
	{
		*pd++ = *p++;
	}
	*pd = 0;
	if(*p == ' ') p++;
	
	pd = opt;
	for(; *p; ) *pd++ = *p++;
	*pd = 0;
}

/*------------------------------------------------
  Open a file
--------------------------------------------------*/
BOOL ExecFile(HWND hwnd, char* command)
{
	char fname[MAX_PATH], opt[MAX_PATH];
	
	if(*command == 0) return FALSE;
	
	GetFileAndOption(command, fname, opt);
	
	if((int)ShellExecute(hwnd, NULL, fname,
		opt[0]?opt:NULL, "", SW_SHOW) < 32) return FALSE;
	return TRUE;
}

/*------------------------------------------------
　WAVEの再生   dwLoops：繰り返し回数
　参考：Visual C++ 1.x のサンプルREVERSE
--------------------------------------------------*/
BOOL PlayWave(HWND hwnd, char *fname, DWORD dwLoops)
{
	HMMIO hmmio;
	MMCKINFO mmckinfoParent;
	MMCKINFO mmckinfoSubchunk;
	LONG lFmtSize;
	LONG lDataSize;
	
	if(hWaveOut != NULL) return FALSE;
	
	if(!(hmmio = mmioOpen(fname, NULL, MMIO_READ | MMIO_ALLOCBUF)))
		return FALSE;
	
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if(mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF))
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}

	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
		MMIO_FINDCHUNK))
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	lFmtSize = mmckinfoSubchunk.cksize;
	pFormat = (WAVEFORMATEX*)malloc(lFmtSize);
	if(pFormat == NULL)
	{
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, (HPSTR)pFormat, lFmtSize) != lFmtSize)
	{
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	/*
	if(pFormat->wFormatTag != WAVE_FORMAT_PCM)
	{
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	*/

	if(waveOutOpen(&hWaveOut, (UINT)WAVE_MAPPER, (LPWAVEFORMATEX)pFormat,
		0, 0, (DWORD)WAVE_FORMAT_QUERY))
	{
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	mmioAscend(hmmio, &mmckinfoSubchunk, 0);
    
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent,
		MMIO_FINDCHUNK))
    {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}

	lDataSize = mmckinfoSubchunk.cksize;
	if(lDataSize == 0)
    {
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	pData = (HPSTR)malloc(lDataSize);
	if(pData == NULL)
	{
		free(pFormat); pFormat = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	
	if(mmioRead(hmmio, pData, lDataSize) != lDataSize)
	{
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		mmioClose(hmmio, 0);
		return FALSE;
	}
	mmioClose(hmmio, 0);
	
	if(waveOutOpen((LPHWAVEOUT)&hWaveOut, (UINT)WAVE_MAPPER,
		(LPWAVEFORMATEX)pFormat, (UINT)hwnd, 0,
		(DWORD)CALLBACK_WINDOW))
    {
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}
	
	memset(&wh, 0, sizeof(WAVEHDR));
	wh.lpData = pData;
	wh.dwBufferLength = lDataSize;
	if(dwLoops != 0)
	{
		wh.dwFlags = WHDR_BEGINLOOP|WHDR_ENDLOOP;
		wh.dwLoops = dwLoops;
	}
	if(waveOutPrepareHeader(hWaveOut, &wh, sizeof(WAVEHDR)))
	{
		waveOutClose(hWaveOut); hWaveOut = NULL;
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}
	
	if(waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR)) != 0)
	{
		waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
		waveOutClose(hWaveOut);	hWaveOut = NULL;
		free(pFormat); pFormat = NULL;
		free(pData); pData = NULL;
		return FALSE;
	}

	return TRUE;
}

/*------------------------------------------------
　WAVE再生停止
--------------------------------------------------*/
void StopWave(void)
{
	if(hWaveOut == NULL) return;

	waveOutReset(hWaveOut);
	waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);	    
	hWaveOut = NULL;
	free(pFormat); pFormat = NULL;
	free(pData); pData = NULL;
}
