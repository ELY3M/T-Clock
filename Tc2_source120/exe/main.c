/*-------------------------------------------------------------
  main.c
  WinMain, window procedure, and functions for initializing
  KAZUBON 1997-2001
---------------------------------------------------------------*/

#include "tclock.h"
#include <winver.h>

// Globals
HINSTANCE g_hInst;           // instance handle
HINSTANCE g_hInstResource;   // instance handle of language module
HWND      g_hwndMain;        // main window
HWND      g_hwndClock;       // clock window
HWND      g_hwndSheet;       // property sheet window
HWND      g_hDlgTimer;       // timer dialog
HWND	  g_hDlgCalender;	 // calender dialog

HICON     g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;
                             // icons to use frequently
char      g_mydir[MAX_PATH]; // path to tclock.exe
char      g_langdllname[MAX_PATH];  // language dll name
BOOL      g_bIniSetting = FALSE;
char      g_inifile[MAX_PATH];

#define VERSIONM 0x10002
#define VERSIONL 0x0000

#ifndef LVS_SMALLICON
#define LVS_SMALLICON           0x0002
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

char szClassName[] = "TClockMainClass"; // window class name
char szWindowText[] = "TClock";         // caption of the window

static BOOL bMenuOpened = FALSE;

void CheckCommandLine(HWND hwnd);
static void OnTimerMain(HWND hwnd);
void FindTrayServer(HWND hwnd);
static void InitError(int n);
static BOOL CheckTCDLL(void);
static BOOL CheckDLL(char *fname);
static void CheckRegistry(void);
void SetDesktopIconTextBk(BOOL bNoTrans);
static UINT s_uTaskbarRestart = 0;
static BOOL bStartTimer = FALSE;
static int nCountFindingClock = -1;

// XButton Messages
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP   0x020C

// menu.c
extern HMENU g_hMenu;

// alarm.c
extern BOOL bPlayingNonstop;

//desktop stuff
HWND hwndDesktop = NULL;
BOOL bCustDesktop = FALSE;
static LONG oldstyle;
static LONG oldexstyle;

BOOL bWin95 = FALSE, bWin98 = FALSE, bWinME = FALSE, 
	bWinNT = FALSE, bWin2000 = FALSE, bWinXP = FALSE;

BOOL bCompactMenu = FALSE, bHideCal = FALSE;

/*-------------------------------------------
  entry point of program
  not use "WinMain" for compacting the file size
---------------------------------------------*/
void WINAPI WinMainCRTStartup(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd;
	DWORD dw;

	HWND hwndTrayServer;
	int ret;

	/*hwndTrayServer = FindWindow("Shell_TrayWnd", "CTrayServer");

	if(hwndTrayServer > 0)
	{
		ret = MyMessageBox(hwnd, "Tclock2 will not run if TrayServer.exe is running. See readme.txt for info", "TrayServer.exe is running!", MB_OK, MB_ICONEXCLAMATION);
		if(ret)
		{
			ExitProcess(1); 
			return;
		}
	}*/

	//make sure ObjectBar isn't running
	if(FindWindow("ObjectBar Main", "ObjectBar") != NULL)
	{
		ExitProcess(1); 
		return;
	}

	// not to execute the program twice
	hwnd = FindWindow(szClassName, szWindowText);
	FindTrayServer(hwnd);
	if(hwnd != NULL)
	{
		CheckCommandLine(hwnd);
		ExitProcess(1); return;
	}
	

	g_hInst = GetModuleHandle(NULL);
	
	// get the path where .exe is positioned
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);
	del_title(g_mydir);
	
	CheckRegistry();
	
	if(!CheckTCDLL()) { ExitProcess(1); return; }
	
	g_hInstResource = LoadLanguageDLL(g_langdllname);
	if(g_hInstResource == NULL) { ExitProcess(1); return; }
	
	InitCommonControls();
	
	// Message of the taskbar recreating
	// Special thanks to Mr.Inuya
	s_uTaskbarRestart = RegisterWindowMessage("TaskbarCreated");
	
	g_hIconTClock = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
	g_hIconPlay = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR);
	g_hIconStop = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR);
	g_hIconDel = LoadImage(g_hInst, MAKEINTRESOURCE(IDI_DEL), IMAGE_ICON,
		16, 16, LR_DEFAULTCOLOR);
	
	g_hwndSheet = g_hDlgTimer = g_hDlgCalender = NULL;
	
	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = g_hIconTClock;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);
	
	// create a hidden window
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, szWindowText,
		0, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);

	g_hwndMain = hwnd;
	
	CheckCommandLine(hwnd);
	
	dw = GetVersion();
	bWin95 = (dw & 0x80000000);
	bWin98 = ((dw & 0x80000000) && 
		LOBYTE(LOWORD(dw)) >= 4 && HIBYTE(LOWORD(dw)) >= 10);
	bWinME = ((dw & 0x80000000) && 
		LOBYTE(LOWORD(dw)) >= 4 && HIBYTE(LOWORD(dw)) >= 90);
	bWinNT = !(dw & 0x80000000);
	bWin2000 = (!(dw & 0x80000000) && LOBYTE(LOWORD(dw)) >= 5);
	bWinXP = (!(dw & 0x80000000) && 
		LOBYTE(LOWORD(dw)) >= 5 && HIBYTE(LOWORD(dw)) >= 1);

	if(bWin2000 || bWinME) bWin98 = TRUE;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(g_hwndSheet && IsWindow(g_hwndSheet)
			&& IsDialogMessage(g_hwndSheet, &msg))
			;
		else if(g_hDlgTimer && IsWindow(g_hDlgTimer)
			&& IsDialogMessage(g_hDlgTimer, &msg))
			;
		else if(g_hDlgCalender && IsWindow(g_hDlgCalender)
			&& IsDialogMessage(g_hDlgCalender, &msg))
			;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	if(g_hMenu) DestroyMenu(g_hMenu);
	
	if(g_hInstResource) FreeLibrary(g_hInstResource);
	
	ExitProcess(msg.wParam);
}

/*-------------------------------------------
   Command Line Option
   /prop : Show Tclock2 Properties
   /exit : Exit Tclock2
   /refresh : refresh Tclock2's settings
   /regtoini : dump registry settings to Tclock.ini (doesn't work yet)
---------------------------------------------*/
void CheckCommandLine(HWND hwnd)
{
	char *p;
	p = GetCommandLine();
	while(*p)
	{
		if(*p == '/')
		{
			p++;
			if(_strnicmp(p, "prop", 4) == 0)
			{
				PostMessage(hwnd, WM_COMMAND, IDC_SHOWPROP, 0);
				p += 4;
			}
			else if(_strnicmp(p, "exit", 4) == 0)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				p += 4;
			}
			else if (_strnicmp(p, "refresh", 7) == 0)
			{
				RefreshUs();
			}
			else if (_strnicmp(p, "regtoini", 8) == 0)
			{
				MoveRegToIni(NULL);
			}
		}
		p++;
	}
}

/*-------------------------------------------
   the window procedure
---------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hmenu, hsub;

	switch (message)
	{
		case WM_CREATE:
		{
			int nDelay;
			InitAlarm();  // initialize alarms
			InitFormat(); // initialize a Date/Time format
			//bCompactMenu = GetMyRegLong("", "CompactMenu", FALSE);
			//bHideCal = GetMyRegLong("Calendar", "HideCalendar", FALSE);
			nDelay = GetMyRegLong("", "DelayStart", 0);
			if(nDelay > 0)
			{
				SetTimer(hwnd, IDTIMER_START, nDelay * 1000, NULL);
				bStartTimer = TRUE;
			}
			else SendMessage(hwnd, WM_TIMER, IDTIMER_START, 0);
			InitSyncTime(hwnd);
			InitMouseFunction();
			SetDesktopIconTextBk(FALSE);
			SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);

			//hmenu = GetMenu(hwnd);

			//hsub = GetSubMenu(hmenu, 0);
			//CoolMenu_Create(hsub, szWindowText);

			return 0;
		}
		case WM_TIMER:
			if(wParam == IDTIMER_START)
			{
				if(bStartTimer) KillTimer(hwnd, wParam);
				bStartTimer = FALSE;
				HookStart(hwnd); // install a hook
				nCountFindingClock = 0;
			}
			else if(wParam == IDTIMER_MAIN)
				OnTimerMain(hwnd);
			else if(wParam == IDTIMER_MOUSE)
				OnTimerMouse(hwnd);
			//else if(wParam == IDTIMER_DEKSTOPICON)
			//{
				//KillTimer(hwnd, wParam);
				//SetDesktopIconTextBk(FALSE);
			//}
			return 0;
		
		case WM_DESTROY:
			EndSyncTime(hwnd);
			EndAlarm();
			EndTimer();
			// MyHelp(hwnd, -1);
			KillTimer(hwnd, IDTIMER_MAIN);
			if(bStartTimer)
			{
				KillTimer(hwnd, IDTIMER_START);
				bStartTimer = FALSE;
			}
			else HookEnd();  // uninstall a hook
			PostQuitMessage(0);
			return 0;
		case WM_ENDSESSION:
			if(wParam)
			{
				EndSyncTime(hwnd);
				EndAlarm();
				EndTimer();
				if(bStartTimer)
				{
					KillTimer(hwnd, IDTIMER_START);
					bStartTimer = FALSE;
				}
				else HookEnd();  // uninstall a hook
			}
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			return 0;
		}
		
		// Messages sent/posted from TCDLL.dll
		case WM_USER:
			nCountFindingClock = -1;
			g_hwndClock = (HWND)lParam;
			return 0;
		case (WM_USER+1):   // error
			nCountFindingClock = -1;
			InitError(lParam);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case (WM_USER+2):   // exit
			if(g_hwndSheet && IsWindow(g_hwndSheet))
				PostMessage(g_hwndSheet, WM_CLOSE, 0, 0);
			if(g_hDlgTimer && IsWindow(g_hDlgTimer))
				PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
			if(g_hDlgCalender && IsWindow(g_hDlgCalender))
				PostMessage(g_hDlgCalender, WM_CLOSE, 0, 0);
			g_hwndSheet = NULL;
			g_hDlgTimer = NULL;
			g_hDlgCalender = NULL;
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case MM_MCINOTIFY:
			OnMCINotify(hwnd);
			return 0;
		case MM_WOM_DONE: // stop playing wave
		case (WM_USER+3):
			StopFile();
			return 0;
		case WM_WININICHANGE:
		/*
		{
			char s[160];
			strcpy(s, "WM_WININICHANGE ");
			if(lParam) strcat(s, (char*)lParam);
			else strcat(s, "(null)");
			WriteDebug(s);
		}
		*/
		{
				HWND hwndBar;
				HWND hwndChild;
				char classname[80];

				hwndBar = FindWindow("Shell_TrayWnd", NULL);

				// find the clock window
				hwndChild = GetWindow(hwndBar, GW_CHILD);
				while(hwndChild)
				{
					GetClassName(hwndChild, classname, 80);
					//WriteDebug(classname);
					if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
					{
						hwndChild = GetWindow(hwndChild, GW_CHILD);
						while(hwndChild)
						{
							GetClassName(hwndChild, classname, 80);
							//WriteDebug(classname);
							if(lstrcmpi(classname, "TrayClockWClass") == 0)
							{
								SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
								break;
							}
						}
						break;
					}
					hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
				}

			//SetTimer(hwnd, IDTIMER_DEKSTOPICON, 3000, NULL);
			
			return 0;
		}
		case WM_SYSCOLORCHANGE:
			PostMessage(hwnd, WM_USER+10, 1,0);
			//SetTimer(hwnd, IDTIMER_DEKSTOPICON, 3000, NULL);
			return 0;
		// context menu
		case WM_COMMAND:
			OnTClockCommand(hwnd, LOWORD(wParam), HIWORD(wParam)); // menu.c
			return 0;
		// messages transfered from the dll
		case WM_CONTEXTMENU:
			// menu.c
			OnContextMenu(hwnd, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_DROPFILES:
			OnDropFiles(hwnd, (HDROP)wParam); // mouse.c
			return 0;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			if(!bPlayingNonstop)
				PostMessage(hwnd, WM_USER+3, 0, 0);
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			OnMouseMsg(hwnd, message, wParam, lParam); // mouse.c
			return 0;
		case WM_ENTERMENULOOP:
			bMenuOpened = TRUE;
			break;
		case WM_EXITMENULOOP:
			bMenuOpened = FALSE;
			break;
		//case WM_MEASUREITEM :

			//return CoolMenu_MeasureItem(wParam, lParam);
		//case WM_DRAWITEM :

			//return CoolMenu_DrawItem(wParam, lParam);
	}
	
	if(message == s_uTaskbarRestart) // When Explorer is hung up,
	{                                // and the taskbar is recreated.
		HookEnd();
		SetTimer(hwnd, IDTIMER_START, 1000, NULL);
		bStartTimer = TRUE;
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/*-------------------------------------------------------
  show a message when TClock failed to customize the clock
---------------------------------------------------------*/
void InitError(int n)
{
	char s[160];
	
	wsprintf(s, "%s: %d", MyString(IDS_NOTFOUNDCLOCK), n);
	MyMessageBox(NULL, s, "Error", MB_OK, MB_ICONEXCLAMATION);
}

/*-------------------------------------------------------
   Timer
   synchronize, alarm, timer, execute Desktop Calendar...
---------------------------------------------------------*/
static int hourLast = -1, minuteLast = -1;
static int daySaved = -1;

void OnTimerMain(HWND hwnd)
{
	SYSTEMTIME st;
	BOOL b;
	
	GetLocalTime(&st);
	b = TRUE;
	if(hourLast == (int)st.wHour &&
		minuteLast == (int)st.wMinute) b = FALSE;
	hourLast = st.wHour;
	minuteLast = st.wMinute;
	if(daySaved >= 0 && st.wDay != daySaved)
		;
	else daySaved = st.wDay;
	
	OnTimerSNTP(hwnd, &st); // sntp.c
	if(b) OnTimerAlarm(hwnd, &st); // alarm.c
	OnTimerTimer(hwnd, &st); // timer.c
	if(b) SetDesktopIconTextBk(FALSE);
	
	// the clock window exists ?
	if(0 <= nCountFindingClock && nCountFindingClock < 20)
		nCountFindingClock++;
	else if(nCountFindingClock == 20)
	{
		nCountFindingClock++;
		/*
		InitError(4);
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		*/
	}
}

/*-------------------------------------------
  load a language dll
---------------------------------------------*/
HINSTANCE LoadLanguageDLL(char *langdllname)
{
	HINSTANCE hInst = NULL;
	char fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind = INVALID_HANDLE_VALUE;
	
	GetMyRegStr("", "LangDLL", fname, MAX_PATH, "");
	if(fname[0])
	{
		hfind = FindFirstFile(fname, &fd);
		if(hfind != INVALID_HANDLE_VALUE) FindClose(hfind);
	}
	if(hfind == INVALID_HANDLE_VALUE)
	{
		strcpy(fname, g_mydir); add_title(fname, "lang*.tclock2");
		hfind = FindFirstFile(fname, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			FindClose(hfind);
			del_title(fname); add_title(fname, fd.cFileName);
		}
	}
	
	if(hfind != INVALID_HANDLE_VALUE)
	{
		if(!CheckDLL(fname)) return NULL;
		hInst = LoadLibrary(fname);
	}
	
	if(hInst == NULL)
		MyMessageBox(NULL, "Can't load a language module.",
			"Error", MB_OK, MB_ICONEXCLAMATION);
	else strcpy(langdllname, fname);
	return hInst;
}

HINSTANCE GetLangModule(void)
{
	return g_hInstResource;
}

/*-------------------------------------------
  Check version of dll
---------------------------------------------*/
BOOL CheckTCDLL(void)
{
	char fname[MAX_PATH];
	
	strcpy(fname, g_mydir); add_title(fname, "tc2dll.tclock");
	return CheckDLL(fname);
}

/*-------------------------------------------
  Check version of dll
---------------------------------------------*/
BOOL CheckDLL(char *fname)
{
	DWORD size;
	char *pBlock;
	VS_FIXEDFILEINFO *pffi;
	BOOL br = FALSE;
	
	size = GetFileVersionInfoSize(fname, 0);
	if(size > 0)
	{
		pBlock = malloc(size);
		if(GetFileVersionInfo(fname, 0, size, pBlock))
		{
			UINT tmp;
			if(VerQueryValue(pBlock, "\\", &pffi, &tmp))
			{
				if(pffi->dwFileVersionMS == VERSIONM &&
					HIWORD(pffi->dwFileVersionLS) == VERSIONL)
				{
					br = TRUE;
				}
			}
		}
		free(pBlock);
	}
	if(!br)
	{
		char msg[MAX_PATH+30];
		
		strcpy(msg, "Invalid file version: ");
		get_title(msg + strlen(msg), fname);
		MyMessageBox(NULL, msg,
			"Error", MB_OK, MB_ICONEXCLAMATION);
	}
	return br;
}

/*-------------------------------------------
  Show "TClock Help"
---------------------------------------------*/
void MyHelp(HWND hwnd, int id)
{
	char helpurl[1024];
	char s[20];
	
	GetMyRegStr("", "HelpURL", helpurl, 1024, "");
	if(helpurl[0] == 0)
	{
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		strcpy(helpurl, g_mydir);
		add_title(helpurl, "help\\index.html");
		hfind = FindFirstFile(helpurl, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
			FindClose(hfind);
		else
			strcpy(helpurl, MyString(IDS_HELPURL));
	}
	if(id != 0)
	{
		if(strlen(helpurl) > 0 && helpurl[ strlen(helpurl) - 1 ] != '/')
			del_title(helpurl);
		wsprintf(s, "%d.html", id);
		add_title(helpurl, s);
	}
	
	ShellExecute(hwnd, NULL, helpurl, NULL, "", SW_SHOW);
}

/*------------------------------------------------
   initialize the registy
--------------------------------------------------*/
void CheckRegistry(void)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	char fname[MAX_PATH];
	char s[80];
	
	SetMyRegStr(NULL, "ExePath", g_mydir);
	DelMyRegKey("DLL");
	
	strcpy(fname, g_mydir);
	add_title(fname, "tclock2.ini");
	hfind = FindFirstFile(fname, &fd);
	if(hfind != INVALID_HANDLE_VALUE)
	{
		FindClose(hfind);
		g_bIniSetting = TRUE;
		strcpy(g_inifile, fname);
		
		if(!GetMyRegLong(NULL, "RegToIni", FALSE))
		{
			MoveRegToIni(NULL);
			SetMyRegLong(NULL, "RegToIni", TRUE);
		}
	}
	
	SetMyRegStr(NULL, "ExePath", g_mydir);
	
	GetMyRegStr("Clock", "Font", s, 80, "");
	if(s[0] == 0)
	{
		HFONT hfont;
		LOGFONT lf;
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if(hfont)
		{
			GetObject(hfont, sizeof(lf),(LPVOID)&lf);
			SetMyRegStr("Clock", "Font", lf.lfFaceName);
		}
	}
}

/*--------------------------------------------------
  set transparent background of desktop icons text
----------------------------------------------------*/
void SetDesktopIconTextBk(BOOL bNoTrans)
{
	BOOL b;
	HWND hwnd;
	COLORREF col, coltext;
	

	b = GetMyRegLong("Desktop", "DesktopTextBk", FALSE);
	if(b)
	{
		hwnd = FindWindow("Progman", "Program Manager");
		if(!hwnd) return;
		hwnd = GetWindow(hwnd, GW_CHILD);
		hwnd = GetWindow(hwnd, GW_CHILD);
		while(hwnd)
		{
			char s[80];
			GetClassName(hwnd, s, 80);
			if(lstrcmpi(s, "SysListView32") == 0) break;
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
		if(!hwnd) return;
	}
	else return;
	
	if(b)
	{
		if(ListView_GetTextBkColor(hwnd) == CLR_NONE) return;
		col = CLR_NONE;
	}
	else
	{
		if(ListView_GetTextBkColor(hwnd) != CLR_NONE) return;
		col = GetSysColor(COLOR_DESKTOP);
	}

	ListView_SetTextBkColor(hwnd, col);
	ListView_RedrawItems(hwnd, 0, ListView_GetItemCount(hwnd));
	
	hwnd = GetParent(hwnd);
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		InvalidateRect(hwnd, NULL, TRUE);
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

int GetClockWnd(void)
{
	HWND hwndBar;
	HWND hwndChild;
	char classname[80];

	hwndBar = FindWindow("Shell_TrayWnd", NULL);

	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild)
	{
		GetClassName(hwndChild, classname, 80);
		//WriteDebug(classname);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
		{
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild)
			{
				GetClassName(hwndChild, classname, 80);
				//WriteDebug(classname);
				if(lstrcmpi(classname, "TrayClockWClass") == 0)
				{
					return hwndChild;
				}
			}
			break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
	return 0;
}

void RefreshUs(void)
{
	HWND hwndBar;
	HWND hwndChild;
	char classname[80];

	hwndBar = FindWindow("Shell_TrayWnd", NULL);

	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild)
	{
		GetClassName(hwndChild, classname, 80);
		//WriteDebug(classname);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
		{
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild)
			{
				GetClassName(hwndChild, classname, 80);
				//WriteDebug(classname);
				if(lstrcmpi(classname, "TrayClockWClass") == 0)
				{
					SendMessage(hwndChild, CLOCKM_REFRESHCLOCK, 0, 0);
					SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
					break;
				}
			}
			break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}

void FindTrayServer(HWND hwnd)
{
	HWND hwndTrayServer;

	hwndTrayServer = FindWindow("Shell_TrayWnd", "CTrayServer");

	if(hwndTrayServer > 0)
		SendMessage(hwndTrayServer, WM_CLOSE, 0, 0);

}