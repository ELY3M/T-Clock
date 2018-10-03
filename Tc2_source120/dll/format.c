//---[s]--- For InternetTime 99/03/16@211 M.Takemura -----

/*-----------------------------------------------------
    format.c
    to make a string to display in the clock
    KAZUBON 1997-1998
-------------------------------------------------------*/

#include "tcdll.h"

int codepage = CP_ACP;
static char DayOfWeekShort[11], DayOfWeekLong[31];
static char MonthShort[11], MonthLong[31];
static char *DayOfWeekEng[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *MonthEng[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char AM[11], PM[11], SDate[5], STime[5];
static char EraStr[11];
static int AltYear;

extern BOOL bHour12, bHourZero;
extern BOOL bWin95, bWin2000;

extern int iFreeRes[3], iCPUUsage, iBatteryLife;
//extern char sAvailPhysK[], sAvailPhysM[];
extern MEMORYSTATUS msMemory;
extern double temperatures[];
extern double voltages[];
extern double fans[];
extern int permon[];
extern int net[];

/*------------------------------------------------
  GetLocaleInfo() for 95/NT
--------------------------------------------------*/
int GetLocaleInfoWA(WORD wLanguageID, LCTYPE LCType, char* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
		r = GetLocaleInfoA(Locale, LCType, dst, n);
	else  // NT
	{
		WCHAR* pw;
		pw = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		*pw = 0;
		r = GetLocaleInfoW(Locale, LCType, pw, n);
		if(r)
			WideCharToMultiByte(codepage, 0, pw, -1, dst, n,
				NULL, NULL);
		GlobalFreePtr(pw);
	}
	return r;
}

/*------------------------------------------------
  GetDateFormat() for 95/NT
--------------------------------------------------*/
int GetDateFormatWA(WORD wLanguageID, DWORD dwFlags, CONST SYSTEMTIME *t,
	char* fmt, char* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
		r = GetDateFormatA(Locale, dwFlags, t, fmt, dst, n);
	
	else  // NT
	{
		WCHAR* pw1, *pw2;
		pw1 = NULL;
		if(fmt)
		{
			pw1 = (WCHAR*)GlobalAllocPtr(GHND,
				sizeof(WCHAR)*(strlen(fmt)+1));
			MultiByteToWideChar(CP_ACP, 0, fmt, -1,
				pw1, strlen(fmt));
		}
		pw2 = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetDateFormatW(Locale, dwFlags, t, pw1, pw2, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw2, -1, dst, n,
				NULL, NULL);
		if(pw1) GlobalFreePtr(pw1);
		GlobalFreePtr(pw2);
	}
	return r;
}

/*------------------------------------------------
  GetTimeFormat() for 95/NT
--------------------------------------------------*/
int GetTimeFormatWA(WORD wLanguageID, DWORD dwFlags, CONST SYSTEMTIME *t,
	char* fmt, char* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
		r = GetTimeFormatA(Locale, dwFlags, t, fmt, dst, n);
	
	else  // NT
	{
		WCHAR* pw1, *pw2;
		pw1 = NULL;
		if(fmt)
		{
			pw1 = (WCHAR*)GlobalAllocPtr(GHND,
				sizeof(WCHAR)*(strlen(fmt)+1));
			MultiByteToWideChar(CP_ACP, 0, fmt, -1,
				pw1, strlen(fmt));
		}
		pw2 = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetTimeFormatW(Locale, dwFlags, t, pw1, pw2, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw2, -1, dst, n,
				NULL, NULL);
		if(pw1) GlobalFreePtr(pw1);
		GlobalFreePtr(pw2);
	}
	return r;
}

/*------------------------------------------------
  load strings of day, month
--------------------------------------------------*/
void InitFormat(SYSTEMTIME* lt)
{
	char s[80], *p;
	int i, ilang, ioptcal;
	
	ilang = GetMyRegLong(NULL, "Locale", (int)GetUserDefaultLangID());
	
	codepage = CP_ACP;
	if(GetLocaleInfoWA((WORD)ilang, LOCALE_IDEFAULTANSICODEPAGE,
		s, 10) > 0)
	{
		p = s; codepage = 0;
		while('0' <= *p && *p <= '9')
			codepage = codepage * 10 + *p++ - '0';
		if(!IsValidCodePage(codepage)) codepage = CP_ACP;
	}
	
	i = lt->wDayOfWeek; i--; if(i < 0) i = 6;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i,
		DayOfWeekShort, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SDAYNAME1 + i,
		DayOfWeekLong, 30);
	i = lt->wMonth; i--;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i,
		MonthShort, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SMONTHNAME1 + i,
		MonthLong, 30);
	
	GetLocaleInfoWA((WORD)ilang, LOCALE_S1159, AM, 10);
	GetMyRegStr(NULL, "AMsymbol", s, 80, AM);
	if(s[0] == 0) strcpy(s, "AM");
	strcpy(AM, s);
	GetLocaleInfoWA((WORD)ilang, LOCALE_S2359, PM, 10);
	GetMyRegStr(NULL, "PMsymbol", s, 80, PM);
	if(s[0] == 0) strcpy(s, "PM");
	strcpy(PM, s);
	
	GetLocaleInfoWA((WORD)ilang, LOCALE_SDATE, SDate, 4);
	GetLocaleInfoWA((WORD)ilang, LOCALE_STIME, STime, 4);
	
	EraStr[0] = 0;
	AltYear = -1;
	
	ioptcal = 0;
	if(GetLocaleInfoWA((WORD)ilang, LOCALE_IOPTIONALCALENDAR,
		s, 10))
	{
		ioptcal = 0;
		p = s;
		while('0' <= *p && *p <= '9')
			ioptcal = ioptcal * 10 + *p++ - '0';
	}
	if(ioptcal < 3) ilang = LANG_USER_DEFAULT;
	
	if(GetDateFormatWA((WORD)ilang,
		DATE_USE_ALT_CALENDAR, lt, "gg", s, 12) != 0);
		strcpy(EraStr, s);
	
	if(GetDateFormatWA((WORD)ilang,
		DATE_USE_ALT_CALENDAR, lt, "yyyy", s, 6) != 0)
	{
		if(s[0])
		{
			p = s;
			AltYear = 0;
			while('0' <= *p && *p <= '9')
				AltYear = AltYear * 10 + *p++ - '0';
		}
	}
}

BOOL GetNumFormat(char **sp, char x, int *len, int *slen)
{
	char *p;
	int n, ns;

	p = *sp;
	n = 0;
	ns = 0;

	while (*p == '_')
	{
		ns++;
		p++;
	}
	if (*p != x) return FALSE;
	while (*p == x)
	{
		n++;
		p++;
	}

	*len = n+ns;
	*slen = ns;
	*sp = p;
	return TRUE;
}

void SetNumFormat(char **dp, int n, int len, int slen)
{
	char *p;
	int minlen,i;

	p = *dp;

	for (i=10,minlen=1; i<1000000000; i*=10,minlen++)
		if (n < i) break;

	while (minlen < len)
	{
		if (slen > 0) { *p++ = ' '; slen--; }
		else { *p++ = '0'; }
		len--;
	}

	for (i=minlen-1; i>=0; i--)
	{
		*(p+i) = (char)((n%10)+'0');
		n/=10;
	}
	p += minlen;

	*dp = p;
}


/*------------------------------------------------
   make a string from date and time format
--------------------------------------------------*/
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt)
{
	char *sp, *dp, *p;
	DWORD TickCount = 0;
	
	sp = fmt; dp = s;
	while(*sp)
	{
		if(*sp == '\"')
		{
			sp++;
			while(*sp != '\"' && *sp)
			{
				p = CharNext(sp);
				while(sp != p) *dp++ = *sp++;
			}
			if(*sp == '\"') sp++;
		}
		else if(*sp == '/')
		{
			p = SDate; while(*p) *dp++ = *p++;
			sp++;
		}
		else if(*sp == ':')
		{
			p = STime; while(*p) *dp++ = *p++;
			sp++;
		}
		
		// for testing
		else if(*sp == 'S' && *(sp + 1) == 'S' && *(sp + 2) == 'S')
		{
			SetNumFormat(&dp, (int)pt->wMilliseconds, 3, 0);
			sp += 3;
		}
		
		else if(*sp == 'y' && *(sp + 1) == 'y')
		{
			int len;
			len = 2;
			if (*(sp + 2) == 'y' && *(sp + 3) == 'y') len = 4;

			SetNumFormat(&dp, (len==2)?(int)pt->wYear%100:(int)pt->wYear, len, 0);
			sp += len;
		}
		else if(*sp == 'm')
		{
			if(*(sp + 1) == 'm' && *(sp + 2) == 'e')
			{
				*dp++ = MonthEng[pt->wMonth-1][0];
				*dp++ = MonthEng[pt->wMonth-1][1];
				*dp++ = MonthEng[pt->wMonth-1][2];
				sp += 3;
			}
			else if(*(sp + 1) == 'm' && *(sp + 2) == 'm')
			{
				if(*(sp + 3) == 'm')
				{
					p = MonthLong;
					while(*p) *dp++ = *p++;
					sp += 4;
				}
				else
				{
					p = MonthShort;
					while(*p) *dp++ = *p++;
					sp += 3;
				}
			}
			else
			{
				if(*(sp + 1) == 'm')
				{
					*dp++ = (char)((int)pt->wMonth / 10) + '0';
					sp += 2;
				}
				else
				{
					if(pt->wMonth > 9)
						*dp++ = (char)((int)pt->wMonth / 10) + '0';
					sp++;
				}
				*dp++ = (char)((int)pt->wMonth % 10) + '0';
			}
		}
		else if(*sp == 'a' && *(sp + 1) == 'a' && *(sp + 2) == 'a')
		{
			if(*(sp + 3) == 'a')
			{
				p = DayOfWeekLong;
				while(*p) *dp++ = *p++;
				sp += 4;
			}
			else
			{
				p = DayOfWeekShort;
				while(*p) *dp++ = *p++;
				sp += 3;
			}
		}
		else if(*sp == 'd')
		{
			if(*(sp + 1) == 'd' && *(sp + 2) == 'e')
			{
				p = DayOfWeekEng[pt->wDayOfWeek];
				while(*p) *dp++ = *p++;
				sp += 3;
			}
			else if(*(sp + 1) == 'd' && *(sp + 2) == 'd')
			{
				if(*(sp + 3) == 'd')
				{
					p = DayOfWeekLong;
					while(*p) *dp++ = *p++;
					sp += 4;
				}
				else
				{
					p = DayOfWeekShort;
					while(*p) *dp++ = *p++;
					sp += 3;
				}
			}
			else
			{
				if(*(sp + 1) == 'd')
				{
					*dp++ = (char)((int)pt->wDay / 10) + '0';
					sp += 2;
				}
				else
				{
					if(pt->wDay > 9)
						*dp++ = (char)((int)pt->wDay / 10) + '0';
					sp++;
				}
				*dp++ = (char)((int)pt->wDay % 10) + '0';
			}
		}
		else if(*sp == 'h')
		{
			int hour;
			hour = pt->wHour;
			if(bHour12)
			{
				if(hour > 12) hour -= 12;
				else if(hour == 0) hour = 12;
				if(hour == 12 && bHourZero) hour = 0;
			}
			if(*(sp + 1) == 'h')
			{
				*dp++ = (char)(hour / 10) + '0';
				sp += 2;
			}
			else
			{
              if(hour > 9) {
					*dp++ = (char)(hour / 10) + '0';
              }
				sp++;
			}
			*dp++ = (char)(hour % 10) + '0';
		}
        else if (*sp == 'w' )
		{
          char xs_diff[3];
          int xdiff;
          int hour;

          xs_diff[0] = (char)(*(sp+2));
          xs_diff[1] = (char)(*(sp+3));
          xs_diff[2] = (char)'\x0';
          xdiff = atoi( xs_diff );
          if ( *(sp+1) == '-' ) xdiff = -xdiff;
          hour = ( pt->wHour + xdiff )%24;
          if ( hour < 0 ) hour += 24;
          if ( bHour12 ) {
            if(hour > 12) hour -= 12;
            else if(hour == 0) hour = 12;
            if(hour == 12 && bHourZero) hour = 0;
          }
          *dp++ = (char)(hour / 10) + '0';
          *dp++ = (char)(hour % 10) + '0';
          sp += 4;
		}
		else if(*sp == 'n')
		{
			if(*(sp + 1) == 'n')
			{
				*dp++ = (char)((int)pt->wMinute / 10) + '0';
				sp += 2;
			}
			else
			{
				if(pt->wMinute > 9)
					*dp++ = (char)((int)pt->wMinute / 10) + '0';
				sp++;
			}
			*dp++ = (char)((int)pt->wMinute % 10) + '0';
		}
		else if(*sp == 's')
		{
			if(*(sp + 1) == 's')
			{
				*dp++ = (char)((int)pt->wSecond / 10) + '0';
				sp += 2;
			}
			else
			{
				if(pt->wSecond > 9)
					*dp++ = (char)((int)pt->wSecond / 10) + '0';
				sp++;
			}
			*dp++ = (char)((int)pt->wSecond % 10) + '0';
		}
		else if(*sp == 't' && *(sp + 1) == 't')
		{
			if(pt->wHour < 12) p = AM; else p = PM;
			while(*p) *dp++ = *p++;
			sp += 2;
		}
		else if(*sp == 'A' && *(sp + 1) == 'M')
		{
			if(*(sp + 2) == '/' &&
				*(sp + 3) == 'P' && *(sp + 4) == 'M')
			{
				if(pt->wHour < 12) *dp++ = 'A';
				else *dp++ = 'P';
				*dp++ = 'M'; sp += 5;
			}
			else if(*(sp + 2) == 'P' && *(sp + 3) == 'M')
			{
				if(pt->wHour < 12) p = AM; else p = PM;
				while(*p) *dp++ = *p++;
				sp += 4;
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'a' && *(sp + 1) == 'm' && *(sp + 2) == '/' &&
			*(sp + 3) == 'p' && *(sp + 4) == 'm')
		{
			if(pt->wHour < 12) *dp++ = 'a';
			else *dp++ = 'p';
			*dp++ = 'm'; sp += 5;
		}
		else if(*sp == '\\' && *(sp + 1) == 'n')
		{
			*dp++ = 0x0d; *dp++ = 0x0a;
			sp += 2;
		}
		// internet time
		else if (*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@')
		{
			*dp++ = '@';
			*dp++ = beat100 / 10000 + '0';
			*dp++ = (beat100 % 10000) / 1000 + '0';
			*dp++ = (beat100 % 1000) / 100 + '0';
			sp += 3;
			if(*sp == '.' && *(sp + 1) == '@')
			{
				*dp++ = '.';
				*dp++ = (beat100 % 100) / 10 + '0';
				sp += 2;
			}
		}
		// alternate calendar
		else if(*sp == 'Y' && AltYear > -1)
		{
			int n = 1;
			while(*sp == 'Y') { n *= 10; sp++; }
			if(n < AltYear)
			{
				n = 1; while(n < AltYear) n *= 10;
			}
			while(1)
			{
				*dp++ = (AltYear % n) / (n/10) + '0';
				if(n == 10) break;
				n /= 10;
			}
		}
		else if(*sp == 'g')
		{
			char *p2;
			p = EraStr;
			while(*p && *sp == 'g')
			{
				p2 = CharNextExA((WORD)codepage, p, 0);
				while(p != p2) *dp++ = *p++;
				sp++;
			}
			while(*sp == 'g') sp++;
		}
		else if(*sp == 'R') // System Resources
		{
			int i, per, len, slen;
			i = 3;
			if(*(sp + 1) == 'S') i = 0;
			else if(*(sp + 1) == 'G') i = 1;
			else if(*(sp + 1) == 'U') i = 2;
			if(i < 3 && bWin95)
			{
				sp += 2;
				per = iFreeRes[i];
				if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
				{
					SetNumFormat(&dp, per, len, slen);
				}
				else
				{
					if(per > 99) *dp++ = (char)((per / 100) + '0');
					if(per > 9)  *dp++ = (char)((per % 100) / 10 + '0');
					*dp++ = (char)((per % 10) + '0');
				}
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'C') // CPU Usage
		{
			if(iCPUUsage >= 0 && *(sp + 1) == 'U')
			{
				int len, slen;

				sp += 2;
				if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
				{
					SetNumFormat(&dp, iCPUUsage, len, slen);
				}
				else
				{
					if(iCPUUsage > 99)
						*dp++ = (char)((iCPUUsage % 1000) / 100 + '0');
					*dp++ = (char)((iCPUUsage % 100) / 10 + '0');
					*dp++ = (char)((iCPUUsage % 10) + '0');
				}
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'B' && *(sp + 1) == 'L') // Battery Life Percentage
		{
			sp += 2;
			if(bWin95 || bWin2000)
			{
				if(iBatteryLife <= 100)
				{
					int len, slen;
					if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
					{
						SetNumFormat(&dp, iBatteryLife, len, slen);
					}
					else
					{
						if(iBatteryLife > 99)
							*dp++ = (char)((iBatteryLife % 1000) / 100 + '0');
						*dp++ = (char)((iBatteryLife % 100) / 10 + '0');
						*dp++ = (char)((iBatteryLife % 10) + '0');
					}
				}
			}
		}
		else if(*sp == 'M') // Available Physical Memory
		{
			int ms, mst, len, slen;
			ms = -1;
			if(*(sp + 1) == 'K')
			{
				sp += 2;
				ms = msMemory.dwAvailPhys/1024;
			}
			else if(*(sp + 1) == 'M')
			{
				sp += 2;
				ms = msMemory.dwAvailPhys/(1024*1024);
			}
			else if(*(sp + 1) == 'T')
			{
				if(*(sp + 2) == 'P')      ms = msMemory.dwTotalPhys;
				else if(*(sp + 2) == 'F') ms = msMemory.dwTotalPageFile;
				else if(*(sp + 2) == 'V') ms = msMemory.dwTotalVirtual;
				if(ms != -1)
				{
					if(*(sp + 3) == 'K') { ms/=1024; sp+=4; }
					else if(*(sp + 3) == 'M') { ms/=1024*1024; sp+=4; }
					else ms = -1;
				}
			}
			else if(*(sp + 1) == 'A')
			{
				if(*(sp + 2) == 'P')
				{
					ms  = msMemory.dwAvailPhys;
					mst = msMemory.dwTotalPhys;
				}
				else if(*(sp + 2) == 'F')
				{
					ms  = msMemory.dwAvailPageFile;
					mst = msMemory.dwTotalPageFile;
				}
				else if(*(sp + 2) == 'V')
				{
					ms  = msMemory.dwAvailVirtual;
					mst = msMemory.dwTotalVirtual;
				}
				if(ms != -1)
				{
					if(*(sp + 3) == 'K') { ms/=1024; sp+=4; }
					else if(*(sp + 3) == 'M') { ms/=1024*1024; sp+=4; }
					else if(*(sp + 3) == 'P') { mst/=100; if(!mst) ms=0; else ms=ms/mst; sp+=4; }
					else ms = -1;
				}
			}
			else if(*(sp + 1) == 'U')
			{
				if(*(sp + 2) == 'P')
				{
					ms  = msMemory.dwTotalPhys - msMemory.dwAvailPhys;
					mst = msMemory.dwTotalPhys;
				}
				else if(*(sp + 2) == 'F')
				{
					ms  = msMemory.dwTotalPageFile - msMemory.dwAvailPageFile;
					mst = msMemory.dwTotalPageFile;
				}
				else if(*(sp + 2) == 'V')
				{
					ms  = msMemory.dwTotalVirtual - msMemory.dwAvailVirtual;
					mst = msMemory.dwTotalVirtual;
				}
				if(ms != -1)
				{
					if(*(sp + 3) == 'K') { ms/=1024; sp+=4; }
					else if(*(sp + 3) == 'M') { ms/=1024*1024; sp+=4; }
					else if(*(sp + 3) == 'P') { mst/=100; if(!mst) ms=0; else ms=ms/mst; sp+=4; }
					else ms = -1;
				}
			}

			if(ms != -1)
			{
				if(GetNumFormat(&sp, 'x', &len, &slen) == FALSE) { len=1; slen=0; }
				SetNumFormat(&dp, ms, len, slen);
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'B' && *(sp + 1) == 'T') // Board Temperature
		{
			int len, slen, i, mbm;

			sp += 2;
			i = 0;
			if(*sp >= '0' && *sp <= '7')
			{
				i = *sp - '0';
				sp++;
			}
			mbm = (int)temperatures[i];
			if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
			{
				SetNumFormat(&dp, mbm, len, slen);
			}
			else
			{
				SetNumFormat(&dp, mbm, 1, 0);
			}
		}
		else if(*sp == 'B' && *(sp + 1) == 'V') // Board Voltage
		{
			int len, slen, i, mbm;
			double mbmd;

			sp += 2;
			i = 0;
			if(*sp >= '0' && *sp <= '7')
			{
				i = *sp - '0';
				sp++;
			}
			mbmd = voltages[i];
			if(mbmd < 0)
			{
				*dp++ = (char)'-';
				mbmd = -mbmd;
			}
			mbm = (int)mbmd;
			if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
			{
				SetNumFormat(&dp, mbm, len, slen);
			}
			else
			{
				SetNumFormat(&dp, mbm, 1, 0);
			}
			mbmd = (mbmd-(int)mbmd);
			if(*sp == '.')
			{
				sp++;
				if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
				{
					*dp++ = (char)'.';
					if(len > 3) len = 3;
					for(i=0; i<len; i++) mbmd *= 10;
					mbm = (int)mbmd;
					SetNumFormat(&dp, mbm, len, 0);
				}
			}
			else
			{
				*dp++ = (char)'.';
				mbmd *= 1000;
				mbm = (int)mbmd;
				SetNumFormat(&dp, mbm, 3, 0);
			}
		}
		else if(*sp == 'B' && *(sp + 1) == 'F') // Board Fan
		{
			int len, slen, i, mbm;

			sp += 2;
			i = 0;
			if(*sp >= '0' && *sp <= '7')
			{
				i = *sp - '0';
				sp++;
			}
			mbm = (int)fans[i];
			if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
			{
				SetNumFormat(&dp, mbm, len, slen);
			}
			else
			{
				SetNumFormat(&dp, mbm, 1, 0);
			}
		}
		else if(*sp == 'P' && *(sp + 1) == 'M') // Performance Monitor
		{
			int len, slen, i, pm;

			sp += 2;
			i = 0;
			if(*sp >= '0' && *sp <= '3')
			{
				i = *sp - '0';
				sp++;
			}
			pm = (int)permon[i];
			if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
			{
				SetNumFormat(&dp, pm, len, slen);
			}
			else
			{
				SetNumFormat(&dp, pm, 1, 0);
			}
		}
		else if(*sp == 'N') // Network Interface
		{
			int len, slen, i, nt;

			i = -1;
			if(*(sp + 1) == 'R')
			{
				if(*(sp + 2) == 'A')      i = 0;
				else if(*(sp + 2) == 'S') i = 2;
			}
			if(*(sp + 1) == 'S')
			{
				if(*(sp + 2) == 'A')      i = 1;
				else if(*(sp + 2) == 'S') i = 3;
			}
			if(i != -1)
			{
				sp += 3;
				nt = (int)net[i];
				if(GetNumFormat(&sp, 'x', &len, &slen) == TRUE)
				{
					SetNumFormat(&dp, nt, len, slen);
				}
				else
				{
					SetNumFormat(&dp, nt, 1, 0);
				}
			}
			else *dp++ = *sp++;
		}
		else if(*sp == 'L' && _strncmp(sp, "LDATE", 5) == 0)
		{
			char s[80], *p;
			GetDateFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				DATE_LONGDATE, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 5;
		}
		else if(*sp == 'D' && _strncmp(sp, "DATE", 4) == 0)
		{
			char s[80], *p;
			GetDateFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				DATE_SHORTDATE, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 4;
		}
		else if(*sp == 'T' && _strncmp(sp, "TIME", 4) == 0)
		{
			char s[80], *p;
			GetTimeFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
				TIME_FORCE24HOURFORMAT, pt, NULL, s, 80);
			p = s;
			while(*p) *dp++ = *p++;
			sp += 4;
		}
		else if(*sp == 'S')
		{
			int len, slen, st;
			sp++;
			if(GetNumFormat(&sp, 'd', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = TickCount/86400000;		//day
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'a', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = TickCount/3600000;		//hour
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'h', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/3600000)%24;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 'n', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/60000)%60;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(GetNumFormat(&sp, 's', &len, &slen) == TRUE)
			{
				if (!TickCount) TickCount = GetTickCount();
				st = (TickCount/1000)%60;
				SetNumFormat(&dp, st, len, slen);
			}
			else if(*sp == 'T')
			{
				DWORD dw;
				int sth, stm, sts;
				if (!TickCount) TickCount = GetTickCount();
				dw = TickCount;
				dw /= 1000;
				sts = dw%60; dw /= 60;
				stm = dw%60; dw /= 60;
				sth = dw;

				SetNumFormat(&dp, sth, 1, 0);
				*dp++ = ':';
				SetNumFormat(&dp, stm, 2, 0);
				*dp++ = ':';
				SetNumFormat(&dp, sts, 2, 0);

				sp++;
			}
			else
				*dp++ = 'S';
		}
		else
		{
			p = CharNext(sp);
			while(sp != p) *dp++ = *sp++;
		}
	}
	*dp = 0;
}

/*------------------------------------------------
  check format
--------------------------------------------------*/
DWORD FindFormat(char* fmt)
{
	char *sp;
	DWORD ret = 0;
	
	sp = fmt;
	while(*sp)
	{
		if(*sp == '\"')
		{
			sp++;
			while(*sp != '\"' && *sp) sp++;
			if(*sp == '\"') sp++;
		}
		else if(*sp == 's')
		{
			sp++;
			ret |= FORMAT_SECOND;
		}
		else if (*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@')
		{
			sp += 3;
			if(*sp == '.' && *(sp + 1) == '@')
			{
				ret |= FORMAT_BEAT2;
				sp += 2;
			}
			else
				ret |= FORMAT_BEAT1;
		}
		else if(*sp == 'R' && 
			(*(sp + 1) == 'S' || *(sp + 1) == 'G' || *(sp + 1) == 'U') )
		{
			sp += 2;
			ret |= FORMAT_SYSINFO;
		}
		else if(*sp == 'C' && *(sp + 1) == 'U')
		{
			sp += 2;
			ret |= FORMAT_SYSINFO;
		}
		else if(*sp == 'B' && *(sp + 1) == 'L')
		{
			sp += 2;
			ret |= FORMAT_BATTERY;
		}
		else if(*sp == 'M' && (*(sp + 1) == 'K' || *(sp + 1) == 'M'))
		{
			sp += 2;
			ret |= FORMAT_MEMORY;
		}
		else if(*sp == 'M' &&
			(*(sp + 1) == 'T' || *(sp + 1) == 'A' || *(sp + 1) == 'U') &&
			(*(sp + 2) == 'P' || *(sp + 2) == 'F' || *(sp + 2) == 'V') &&
			(*(sp + 3) == 'K' || *(sp + 3) == 'M' || *(sp + 3) == 'P'))
		{
			sp += 4;
			ret |= FORMAT_MEMORY;
		}
		else if(*sp == 'B' && *(sp + 1) == 'T')
		{
			sp += 2;
			ret |= FORMAT_MOTHERBRD;
		}
		else if(*sp == 'B' && *(sp + 1) == 'V')
		{
			sp += 2;
			ret |= FORMAT_MOTHERBRD;
		}
		else if(*sp == 'B' && *(sp + 1) == 'F')
		{
			sp += 2;
			ret |= FORMAT_MOTHERBRD;
		}
		else if(*sp == 'P' && *(sp + 1) == 'M')
		{
			sp += 2;
			ret |= FORMAT_PERMON;
		}
		else if(*sp == 'N' &&
			(*(sp + 1) == 'R' || *(sp + 1) == 'S') &&
			(*(sp + 2) == 'S' || *(sp + 2) == 'A'))
		{
			sp += 3;
			ret |= FORMAT_NET;
		}
		else sp = CharNext(sp);
	}

	return ret;
}

int GetProcName(char* dst)
{

}
