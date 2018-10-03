/*-------------------------------------------------------------------------
  cpu.c
  get percentage of CPU usage
  Kazubon 2001
---------------------------------------------------------------------------*/

/* *** Special thanks to Naoki KOBAYASHI *** */

#include <windows.h>
#include <pdh.h>

void CpuMoni_start(void);
int CpuMoni_get(void);
void CpuMoni_end(void);

static HMODULE hmodPDH = NULL;
static HQUERY hQuery = NULL;
static HCOUNTER hCounter = NULL;

static LONG (WINAPI *pPdhOpenQuery)(LPVOID, DWORD, HQUERY);
static LONG (WINAPI *pPdhAddCounter)(HQUERY, LPCWSTR, DWORD, HCOUNTER);
static LONG (WINAPI *pPdhCollectQueryData)(HQUERY);
static LONG (WINAPI *pPdhGetFormattedCounterValue)(HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
static LONG (WINAPI *pPdhCloseQuery)(HQUERY);

extern BOOL bWinNT;
extern LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);

void CpuMoni_start(void)
{
	if(bWinNT) // WinNT4, 2000
	{
		if(hmodPDH) CpuMoni_end();
		hmodPDH = LoadLibrary("pdh.dll");
		if(hmodPDH == NULL) return;
		
		(FARPROC)pPdhOpenQuery = GetProcAddress(hmodPDH, "PdhOpenQuery");
		(FARPROC)pPdhAddCounter = GetProcAddress(hmodPDH, "PdhAddCounterW");
		(FARPROC)pPdhCollectQueryData = 
			GetProcAddress(hmodPDH, "PdhCollectQueryData");
		(FARPROC)pPdhGetFormattedCounterValue = 
			GetProcAddress(hmodPDH, "PdhGetFormattedCounterValue");
		(FARPROC)pPdhCloseQuery = GetProcAddress(hmodPDH, "PdhCloseQuery");
		
		if(pPdhOpenQuery == NULL || pPdhAddCounter == NULL || 
			pPdhCollectQueryData == NULL || 
			pPdhGetFormattedCounterValue == NULL || pPdhCloseQuery == NULL)
		{
			FreeLibrary(hmodPDH); hmodPDH = NULL;
			return;
		}
		
		if(pPdhOpenQuery(NULL, 0, &hQuery) == ERROR_SUCCESS)
		{
			if(pPdhAddCounter(hQuery, L"\\Processor(_Total)\\% Processor Time",
				0, &hCounter) == ERROR_SUCCESS)
			{
				if(pPdhCollectQueryData(hQuery) == ERROR_SUCCESS) ;
				else hQuery = NULL;
			}
			else hQuery = NULL;
		}
		else hQuery = NULL;
		
		if(hQuery == NULL)
		{
			FreeLibrary(hmodPDH); hmodPDH = NULL;
		}
	}
	else // Win95,98,Me
	{
		GetRegLong(HKEY_DYN_DATA, "PerfStats\\StartStat",
			"KERNEL\\CPUUsage", 0);
	}
}

int CpuMoni_get(void)
{
	if(bWinNT)
	{
		PDH_FMT_COUNTERVALUE FmtValue;
		
		if(hmodPDH)
		{
			pPdhCollectQueryData(hQuery);
			if(pPdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL,
				&FmtValue) == ERROR_SUCCESS)
			{
				/*
				LONG *p;
				p = &(FmtValue.longValue);
				return (int)(*(p + 1));
				*/
				return (int)FmtValue.doubleValue;
			}
		}
	}
	else
	{
		return (int)GetRegLong(HKEY_DYN_DATA, "PerfStats\\StatData",
			"KERNEL\\CPUUsage", 0);
	}
	return -1;
}

void CpuMoni_end(void)
{
	if(bWinNT)
	{
		if(hmodPDH)
		{
			pPdhCloseQuery(hQuery);
			FreeLibrary(hmodPDH);
		}
		hmodPDH = NULL; hQuery = NULL;
	}
	else
	{
		GetRegLong(HKEY_DYN_DATA, "PerfStats\\StopStat",
			"KERNEL\\CPUUsage", 0);
	}
}
