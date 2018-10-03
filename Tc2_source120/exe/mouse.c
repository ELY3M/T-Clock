/*-------------------------------------------
  mouse.c
  mouse operation
  KAZUBON 1997-2001
---------------------------------------------*/

#include "tclock.h"

// XButton Messages
#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP   0x020C
#define XBUTTON1       0x0001
#define XBUTTON2       0x0002
#endif

static char reg_section[] = "Mouse";
static UINT last_mousedown  = 0;
static WORD last_xmousedown = 0;
static DWORD last_tickcount;
static int num_click = 0;
static int exec_button = -1;
static BOOL timer = FALSE;

static int GetMouseFuncNum(int button, int nclick);

/*------------------------------------------------
    initialize registry data
--------------------------------------------------*/
void InitMouseFunction(void)
{
	int i;
	LONG n;
	char *old_entry[] = { "LClick", "LDblClick" };
	char entry[20];
	char s[256];
	
	last_tickcount = GetTickCount();
	
	if(GetMyRegLong(reg_section, "ver230", 0) == 0)
	{
		SetMyRegLong(reg_section, "ver230", 1);
		if(GetMyRegLong(reg_section, "02", -1) < 0)
			SetMyRegLong(reg_section, "02", 0);
	}
	
	// save old data with new format
	for(i = 0; i < 2; i++)
	{
		n = GetMyRegLong(reg_section, old_entry[i], -1);
		if(n < 0) continue;
		
		DelMyReg(reg_section, old_entry[i]);
		wsprintf(entry, "0%d", i + 1);
		SetMyRegLong(reg_section, entry, n);
		if(n == 6)
		{
			GetMyRegStr(reg_section, "ClipFormat", s, 256, "");
			if(s[0])
			{
				DelMyReg(reg_section, "ClipFormat");
				wsprintf(entry, "0%dClip", i + 1);
				SetMyRegStr(reg_section, entry, s);
			}
		}
		else if(n == 100)
		{
			strcpy(entry, old_entry[i]); strcat(entry, "File");
			GetMyRegStr(reg_section, entry, s, 256, "");
			if(s[0])
			{
				DelMyReg(reg_section, entry);
				wsprintf(entry, "0%dFile", i + 1);
				SetMyRegStr(reg_section, entry, s);
			}
		}
	}
}

/*------------------------------------------------
   when files dropped to the clock
--------------------------------------------------*/
void OnDropFiles(HWND hwnd, HDROP hdrop)
{
	char fname[MAX_PATH], sname[MAX_PATH];
	char app[1024];
	SHFILEOPSTRUCT shfos;
	char *buf, *p;
	int i, num;
	int nType;
	
	nType = GetMyRegLong(reg_section, "DropFiles", 0);
	
	num = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
	if(num <= 0) return;
	buf = malloc(num*MAX_PATH);
	if(buf == NULL) return;
	p = buf;
	for(i = 0; i < num; i++)
	{
		DragQueryFile(hdrop, i, fname, MAX_PATH);
		if(nType == 1 || nType == 3 || nType == 4)  // ごみ箱、コピー、移動
		{                             // '\0'で区切られたファイル名
			strcpy(p, fname); p += strlen(p) + 1;
		}
		else if(nType == 2) //プログラムで開く：
		{                   //スペースで区切られた短いファイル名
			if(num > 1) GetShortPathName(fname, sname, MAX_PATH);
			else strcpy(sname, fname);
			strcpy(p, sname);
			p += strlen(p);
			if(num > 1 && i < num - 1) { *p = ' '; p++; }
		}
	}
	*p = 0;
	DragFinish(hdrop);
	
	GetMyRegStr(reg_section, "DropFilesApp", app, 1024, "");
	
	if(nType == 1 || nType == 3 || nType == 4)  // ごみ箱、コピー、移動
	{
		memset(&shfos, 0, sizeof(SHFILEOPSTRUCT));
		shfos.hwnd = NULL;
		if(nType == 1) shfos.wFunc = FO_DELETE;
		else if(nType == 3) shfos.wFunc = FO_COPY;
		else if(nType == 4) shfos.wFunc = FO_MOVE;
		shfos.pFrom = buf;
		if(nType == 3 || nType == 4) shfos.pTo = app;
		shfos.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION;
		SHFileOperation(&shfos);
	}
	else if(nType == 2) //ファイルで開く
	{
		char command[MAX_PATH*2];
		
		strcpy(command, app);
		strcat(command, " ");
		strcat(command, buf);
		ExecFile(hwnd, command);
	}
	free(buf);
}

/*------------------------------------------------------------
   when the clock clicked
   
   registry format 
   name    value
   03      3           left button triple click -> Minimize All
   32      100         x-1 button  double click -> Run Notepad
   32File  C:\Windows\notepad.exe
--------------------------------------------------------------*/
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LONG n_func;
	int button;
	UINT doubleclick_time;
	char s[10];
	int i;
	BOOL bDown = FALSE;
	
	if(timer) KillTimer(hwnd, IDTIMER_MOUSE);
	timer = FALSE;
	
	if(message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)
	{
		BOOL b;
		//b = GetMyRegLong("StartButton", "StartButtonHide", FALSE);
		b = GetMyRegLong("StartMenu", "StartMenuClock", FALSE);
		if(b) return;
	}
	
	switch(message)
	{
		case WM_LBUTTONDOWN: 
		case WM_LBUTTONUP:   button = 0; break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:   button = 1; break;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:   button = 2; break;
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			if(HIWORD(wParam) == XBUTTON1) button = 3;
			else if(HIWORD(wParam) == XBUTTON2) button = 4;
			else return;
			break;
		default: return;
	}
	
	switch(message)
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			if(last_mousedown != message) num_click = 0;
			last_mousedown = message;
			if(last_mousedown == WM_XBUTTONDOWN)
				last_xmousedown = HIWORD(wParam);
			bDown = TRUE;
			break;
		case WM_LBUTTONUP:
			if(last_mousedown != WM_LBUTTONDOWN) last_mousedown = 0;
			break;
		case WM_RBUTTONUP:
			if(last_mousedown != WM_RBUTTONDOWN) last_mousedown = 0;
			break;
		case WM_MBUTTONUP:
			if(last_mousedown != WM_MBUTTONDOWN) last_mousedown = 0;
			break;
		case WM_XBUTTONUP:
			if(last_mousedown != WM_XBUTTONDOWN || 
				last_xmousedown != HIWORD(wParam))
			{
				last_mousedown = 0; last_xmousedown = 0;
			}
			break;
	}
	
	if(last_mousedown == 0) { num_click = 0; return; }
	
	// Mouse double click speed
	GetMyRegStr(reg_section, "DoubleClickSpeed", s, 10, "");
	if(s[0]) doubleclick_time = atoi(s);
	else     doubleclick_time = GetDoubleClickTime();
	
	if(GetTickCount() - last_tickcount > doubleclick_time)
		num_click = 0;
	last_tickcount = GetTickCount();
	
	if(bDown)
	{
		n_func = GetMouseFuncNum(button, num_click + 1);
		if(n_func >= 0 && n_func != MOUSEFUNC_SCREENSAVER)
		{
			for(i = num_click + 2; i <= 4; i++)
			{
				n_func = GetMouseFuncNum(button, i);
				if(n_func >= 0) return;
			}
			num_click++;
			exec_button = button;
			OnTimerMouse(hwnd);
		}
		return;
	}
	
	num_click++;
	
	n_func = GetMouseFuncNum(button, num_click);
	if(n_func < 0) return;
	
	for(i = num_click + 1; i <= 4; i++)
	{
		n_func = GetMouseFuncNum(button, i);
		if(n_func >= 0)
		{
			exec_button = button;
			timer = TRUE;
			SetTimer(hwnd, IDTIMER_MOUSE, doubleclick_time, 0);
			return;
		}
	}
	
	exec_button = button;
	OnTimerMouse(hwnd);
}

/*------------------------------------------------
   execute mouse function
--------------------------------------------------*/
void OnTimerMouse(HWND hwnd)
{
	int button;
	LONG n_func;
	char entry[20];
	
	button = exec_button;
	if(timer) KillTimer(hwnd, IDTIMER_MOUSE); timer = FALSE;
	
	n_func = GetMouseFuncNum(button, num_click);
	
	if(n_func < 0) return;
	
	switch (n_func)
	{
		case MOUSEFUNC_DATETIME:
		case MOUSEFUNC_EXITWIN:
		case MOUSEFUNC_RUNAPP:
		case MOUSEFUNC_MINALL:
		{
			WPARAM wParam;
			HWND hwndTray;
			if(n_func == MOUSEFUNC_DATETIME)
				wParam = IDC_DATETIME;
			else if(n_func == MOUSEFUNC_EXITWIN)
				wParam = IDC_EXITWIN;
			else if(n_func == MOUSEFUNC_RUNAPP)
				wParam = IDC_RUNAPP;
			else if(n_func == MOUSEFUNC_MINALL)
				wParam = IDC_MINALL;
			hwndTray = FindWindow("Shell_TrayWnd", NULL);
			if(hwndTray) PostMessage(hwndTray, WM_COMMAND, wParam, 0);
			break;
		}
		case MOUSEFUNC_SYNCTIME:
			StartSyncTime(hwnd, NULL, 0); break;
		case MOUSEFUNC_TIMER:
			DialogTimer(hwnd); break;
		case MOUSEFUNC_SHOWCALENDER:
		{
			DialogCalender(hwnd);
			break;
		}
		case MOUSEFUNC_CLIPBOARD:
		{
			LPARAM lParam;
			lParam = MAKELONG((WORD)button, (WORD)num_click);
			PostMessage(g_hwndClock, CLOCKM_COPY, 0, lParam);
			break;
		}
		case MOUSEFUNC_SCREENSAVER:
		{
			SendMessage(GetDesktopWindow(), WM_SYSCOMMAND, SC_SCREENSAVE, 0);
			break;
		}
		case MOUSEFUNC_OPENFILE:
		{
			char fname[1024];
			wsprintf(entry, "%d%dFile", button, num_click);
			GetMyRegStr(reg_section, entry, fname, 1024, "");
			if(fname[0]) ExecFile(hwnd, fname);
		}
	}
}

int GetMouseFuncNum(int button, int nclick)
{
	char entry[20];
	wsprintf(entry, "%d%d", button, nclick);
	return GetMyRegLong(reg_section, entry, -1);
}
