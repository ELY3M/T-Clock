/*-------------------------------------------------------------------------
  net.c
  get network interface info
---------------------------------------------------------------------------*/

#include <windows.h>
#include <iprtrmib.h>

void Net_start(void);
void Net_get(void);
void Net_end(void);

static HMODULE hmodIPHLP = NULL;
static HGLOBAL *buffer = NULL;
static MIB_IFROW *ifr;
static MIB_IFTABLE *ift;
static int count;
static int sec = 5;
int net[4] = { 0, 0, 0, 0 };

static DWORD (WINAPI *pGetIfTable)(PMIB_IFTABLE, PULONG, BOOL);
static DWORD (WINAPI *pGetIfEntry)(PMIB_IFROW);

extern LONG GetMyRegLong(char* section, char* entry, LONG defval);


void Net_start(void)
{
	DWORD bufsize;
	DWORD iftable;
	int i;

	if(hmodIPHLP) Net_end();
	ifr = NULL;
	ift = NULL;

	hmodIPHLP = LoadLibrary("iphlpapi.dll");
	if(hmodIPHLP == NULL) return;
	
	(FARPROC)pGetIfTable = GetProcAddress(hmodIPHLP, "GetIfTable");
	(FARPROC)pGetIfEntry = GetProcAddress(hmodIPHLP, "GetIfEntry");
	
	if(pGetIfTable == NULL || pGetIfEntry == NULL)
	{
		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
		return;
	}

	bufsize = 0;
	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
	if(iftable != ERROR_INSUFFICIENT_BUFFER)
	{
		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
		return;
	}
	buffer = GlobalAlloc(GPTR, bufsize);
	if(buffer == NULL)
	{
		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
		return;
	}
	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
	if(iftable != NO_ERROR)
	{
		GlobalFree(buffer); buffer = NULL;
		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
		return;
	}

	ift = (MIB_IFTABLE *)buffer;
	count = ift->dwNumEntries;

	net[0] = 0;
	net[1] = 0;
	for(i=0; i<count; i++)
	{
		ifr = (ift->table) + i;
		if(ifr->dwType == MIB_IF_TYPE_ETHERNET ||
		   ifr->dwType == MIB_IF_TYPE_PPP)
		{
			net[0] += ifr->dwInOctets/1024;
			net[1] += ifr->dwOutOctets/1024;
		}
	}

	sec = (int)GetMyRegLong(NULL, "IntervalSysInfo", 5);
	if(sec <= 0 || 60 < sec) sec = 5;
}

void Net_get(void)
{
	if(ift && hmodIPHLP)
	{
		int i;
		int recv, send;
		if(sec <= 0) sec = 5;

		recv = send = 0;
		for(i=0; i<count; i++)
		{
			ifr = (ift->table) + i;
			if(ifr->dwType == MIB_IF_TYPE_ETHERNET ||
			   ifr->dwType == MIB_IF_TYPE_PPP)
			{
				pGetIfEntry(ifr);
				recv += ifr->dwInOctets/1024;
				send += ifr->dwOutOctets/1024;
			}
		}
		net[2] = (recv-net[0])/sec;
		net[3] = (send-net[1])/sec;
		net[0] = recv;
		net[1] = send;
	}
	else
	{
		net[0] = 0;
		net[1] = 0;
		net[2] = 0;
		net[3] = 0;
	}
}

void Net_end(void)
{
	if(hmodIPHLP)
	{
		FreeLibrary(hmodIPHLP);
	}
	if(buffer)
	{
		GlobalFree(buffer);
	}

	hmodIPHLP = NULL; buffer = NULL;
}
