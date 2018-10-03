/*-------------------------------------------------------------------------
  permon.c
  get performance monitor counter
  Kazubon 2001
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pdh.h>

void PerMoni_start(void);
int PerMoni_get(void);
void PerMoni_end(void);

int permon[4] = { 0, 0, 0, 0 };

static HMODULE hmodPDH = NULL;
static HQUERY hQuery = NULL;
static HCOUNTER hCounter[4] = { NULL, NULL, NULL, NULL };

static wchar_t *pwszCounter[4] = 
{
	L"\\Network Interface(MS TCP Loopback interface)\\Bytes Sent/sec",
	L"\\Network Interface(MS TCP Loopback interface)\\Bytes Received/sec",
	L"\\PhysicalDisk(_Total)\\Avg. Disk Bytes/Read",
	L"\\PhysicalDisk(_Total)\\Avg. Disk Bytes/Write"
};

static LONG (WINAPI *pPdhOpenQuery)(LPVOID, DWORD, HQUERY);
static LONG (WINAPI *pPdhAddCounter)(HQUERY, LPCWSTR, DWORD, HCOUNTER);
static LONG (WINAPI *pPdhCollectQueryData)(HQUERY);
static LONG (WINAPI *pPdhGetFormattedCounterValue)(HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
static LONG (WINAPI *pPdhCloseQuery)(HQUERY);

extern BOOL bWinNT;
extern LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);

void PerMoni_start(void)
{
	int i;

	if(bWinNT) // WinNT4, 2000
	{
		if(hmodPDH) PerMoni_end();
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
			for(i=0; i<4; i++)
			{
				if(pPdhAddCounter(hQuery, pwszCounter[i], 0, &hCounter[i]) != ERROR_SUCCESS)
				{
					hQuery = NULL;
					break;
				}
			}
			if(hQuery)
			{
				if(pPdhCollectQueryData(hQuery) != ERROR_SUCCESS)
					hQuery = NULL;
			}
		}
		else hQuery = NULL;
		
		if(hQuery == NULL)
		{
			FreeLibrary(hmodPDH); hmodPDH = NULL;
		}
	}
	else // Win95,98,Me
	{
	}
}

int PerMoni_get(void)
{
	int i;

	for(i=0; i<4; i++)
		permon[i] = 0;

	if(bWinNT)
	{
		PDH_FMT_COUNTERVALUE FmtValue;
		
		if(hmodPDH)
		{
			pPdhCollectQueryData(hQuery);

			for(i=0; i<4; i++)
			{
				if(pPdhGetFormattedCounterValue(hCounter[i], PDH_FMT_DOUBLE, NULL,
					&FmtValue) == ERROR_SUCCESS)
				{
					permon[i] = (int)(FmtValue.doubleValue/1024);
				}
			}
		}
	}
	else
	{
	}
	return -1;
}

void PerMoni_end(void)
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
	}
}
