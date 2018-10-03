/*-----------------------------------------------------
  tclock.c
  customize the tray clock
  KAZUBON 1997-2001
-------------------------------------------------------*/

#include "tcdll.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitClock(HWND hwnd);
void EndClock(void);
BOOL IsIE4(HWND hwnd);
void CreateTip(HWND hwnd);
void CreateClockDC(HWND hwnd);
void ReadData(HWND hwnd);
void InitSysInfo(HWND hwnd);
void OnTimer(HWND hwnd);
void DrawClockSub(HWND hwnd, HDC hdc, SYSTEMTIME* pt, int beat100);
LRESULT OnCalcRect(HWND hwnd);
void OnTooltipNeedText(UINT code, LPARAM lParam);
void OnCopy(HWND hwnd, LPARAM lParam);
//void UpdateSysRes(HWND hwnd, BOOL bsysres, BOOL bbattery, BOOL bmem);
void UpdateSysRes(HWND hwnd, BOOL bsysres, BOOL bbattery, BOOL bmem, BOOL bmb, BOOL bpermon, BOOL bnet);
void InitDaylightTimeTransition(void);
BOOL CheckDaylightTimeTransition(SYSTEMTIME *lt);

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
#pragma data_seg(".MYDATA")
HHOOK hhook = 0;
HWND hwndTClockMain = NULL;
HWND hwndClock = NULL;
char szShareBuf[81] = { 0 };
#pragma data_seg()
/*------------------------------------------------
  globals
--------------------------------------------------*/
HANDLE hmod = 0;
WNDPROC oldWndProc = NULL;
BOOL bTimer = FALSE;
BOOL bTimerTesting = FALSE;
HDC hdcClock = NULL;
HBITMAP hbmpClock = NULL;
HBITMAP hbmpClockSkin = NULL;
HFONT hFon = NULL;
HWND hwndTip = NULL;
COLORREF colback, colback2, colfore;
char format[1024];
BOOL bHour12, bHourZero;
SYSTEMTIME LastTime;
int beatLast = -1;
int bDispSecond = FALSE;
int nDispBeat = 0;
BOOL bNoClock = FALSE;
int nBlink = 0;
BOOL bIE4 = FALSE, bWin95 = FALSE, bWin98 = FALSE,
	bWinME = FALSE, bWinNT = FALSE, bWin2000 = FALSE, 
	bWinXP = FALSE;
int dwidth = 0, dheight = 0, dvpos = 0, dlineheight = 0, dhpos = 0;
int iClockWidth = -1;
BOOL bPlaying = FALSE;
BOOL bDispSysInfo = FALSE, bTimerSysInfo = FALSE;
BOOL bGetSysRes = FALSE, bGetBattery = FALSE;
BOOL bGetMem = 0;
BOOL bGetMb = FALSE, bGetPm = FALSE, bGetNet = FALSE;
MEMORYSTATUS msMemory;
int iFreeRes[3] = {0,0,0}, iCPUUsage = 0, iBatteryLife = 0;
char sAvailPhysK[11], sAvailPhysM[11];

extern HWND hwndStart;
extern HWND hwndStartMenu;

extern BOOL bStartMenuClock; // startbtn.c

extern BOOL bFillTray;
extern BOOL bSkinTray;
extern BOOL bFlatTray;
extern BOOL bTileTray;
extern BOOL bFillClock;

int tEdgeTop;
int tEdgeLeft;
int tEdgeBottom;
int tEdgeRight;

extern int codepage;

BOOL bFlatRebar = FALSE;
BOOL bStartMenuDestroyed = TRUE;
BOOL bStartMenuAtClock = FALSE;

//extern BOOL bStartMenuDestroyed;

// XButton Messages
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C

//custom taskbar color scheme
//BOOL bCustomColorScheme = FALSE;

BOOL bRefreshClearTaskbar = FALSE;
static BOOL bClockUseTrans = FALSE;

/*------------------------------------------------
  initialize the clock
--------------------------------------------------*/
void InitClock(HWND hwnd)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	char fname[MAX_PATH];
	BOOL b;
	DWORD dw;
	
	hwndClock = hwnd;
	
	dw = GetVersion(); //get the windows version
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
	
	GetModuleFileName(hmod, fname, MAX_PATH);
	del_title(fname); add_title(fname, "tclock2.ini");
	hfind = FindFirstFile(fname, &fd);
	if(hfind != INVALID_HANDLE_VALUE)
	{
		g_bIniSetting = TRUE;
		strcpy(g_inifile, fname);
		FindClose(hfind);
	}
	
	CpuMoni_start(); // cpu.c
	Net_start();     // net.c

	PostMessage(hwndTClockMain, WM_USER, 0, (LPARAM)hwnd);
	
	//レジストリ読み込み
	ReadData(hwndClock);
	InitDaylightTimeTransition();
	//サブクラス化
	oldWndProc = (WNDPROC)GetWindowLong(hwndClock, GWL_WNDPROC);
	SetWindowLong(hwndClock, GWL_WNDPROC, (LONG)WndProc);
	//ダブルクリック受け付けない
	SetClassLong(hwndClock, GCL_STYLE,
	  GetClassLong(hwndClock, GCL_STYLE) & ~CS_DBLCLKS);
	
	//SetDesktopIcons();
	SetDesktop();
	//スタートボタンの初期化
	//スタートメニューの初期化
	SetStartMenu(hwndClock);
	SetStartButton(hwndClock);

	//ツールチップ作成
	CreateTip(hwndClock);
	
	b = GetMyRegLong(NULL, "DropFiles", FALSE);
	DragAcceptFiles(hwnd, b);
	
	bIE4 = IsIE4(hwndClock);
	if(bIE4)
	{
		InitStartMenuIE4();
		InitTaskSwitch(hwndClock);
	}
	
	InitTrayNotify(hwndClock);
	
	if(bWin2000) SetLayeredTaskbar(hwndClock);
	if(bWin2000) InitLayeredStartMenu(hwndClock);
	//if(bWin2000) SetLayeredDesktop(hwndClock);

	//タスクバーの更新
	PostMessage(GetParent(GetParent(hwndClock)), WM_SIZE,
		SIZE_RESTORED, 0);
	InvalidateRect(GetParent(GetParent(hwndClock)), NULL, TRUE);
}

/*------------------------------------------------
  ending process
--------------------------------------------------*/
void DeleteClockRes(void)
{
	if(hFon) DeleteObject(hFon); hFon = NULL;
	if(hdcClock) DeleteDC(hdcClock); hdcClock = NULL;
	if(hbmpClock) DeleteObject(hbmpClock); hbmpClock = NULL;
	if(hbmpClockSkin) DeleteObject(hbmpClockSkin); hbmpClockSkin = NULL;
}
void EndClock(void)
{
	DragAcceptFiles(hwndClock, FALSE);
	if(hwndTip) DestroyWindow(hwndTip); hwndTip = NULL;
	EndTrayNotify();
	EndTaskSwitch();
	EndStartButton();
	EndStartMenu();
	//EndDesktopIcons();
	EndDesktop();
	ClearStartMenuIE4();
	
	DeleteClockRes();
	
	EndNewAPI(hwndClock);
	EndSysres();
	FreeBatteryLife();
	CpuMoni_end(); // cpu.c
	Net_end();     // net.c

	if(hwndClock && IsWindow(hwndClock))
	{
		if(bTimer) KillTimer(hwndClock, 1); bTimer = FALSE;
		if(bTimerSysInfo) KillTimer(hwndClock, 2);
		SetWindowLong(hwndClock, GWL_WNDPROC, (LONG)oldWndProc);
		oldWndProc = NULL;
	}
	//TClockウィンドウを終了させる
	if(IsWindow(hwndTClockMain))
		PostMessage(hwndTClockMain, WM_USER+2, 0, 0);

	bClockUseTrans = FALSE;
}

/*------------------------------------------------
  IE 4 or later is installed ?
--------------------------------------------------*/
BOOL IsIE4(HWND hwnd)
{
	char classname[80];
	DWORD dw;
	
	dw = GetRegLong(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
		"ClassicShell", 0);
	if(dw) return TRUE;
	
	hwnd = GetParent(GetParent(hwnd));
	if(hwnd == NULL) return FALSE;
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ReBarWindow32") == 0)
			return TRUE;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	return FALSE;
}

/*------------------------------------------------
  subclass procedure of the clock
--------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) // ツールチップ対応
	{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		{
			MSG msg;
			msg.hwnd = hwnd;
			msg.message = message;
			msg.wParam = wParam;
			msg.lParam = lParam;
			msg.time = GetMessageTime();
			msg.pt.x = LOWORD(GetMessagePos());
			msg.pt.y = HIWORD(GetMessagePos());
			//WriteDebug("WM_MOUSEMOVE");
			if(hwndTip)
			{
				SendMessage(hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
				//WriteDebug("TTM_RELAYEVENT");
			}
		}
	}
	
	switch(message)
	{
		case WM_DESTROY:
			DeleteClockRes();
			break;
		// 親ウィンドウから送られ、サイズを返すメッセージ
		case (WM_USER+100):
			if(bNoClock) break;
			return OnCalcRect(hwnd);
		// システムの設定を反映する
		case WM_SYSCOLORCHANGE:
			CreateClockDC(hwnd); //hdcClock作り直し
		case WM_TIMECHANGE:
		// 親ウィンドウから送られる
		case (WM_USER+101):
		{
			HDC hdc;
			if(bNoClock) break;
			hdc = GetDC(hwnd);
			DrawClock(hwnd, hdc);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_SIZE:
			CreateClockDC(hwnd); //hdcClock作り直し
			break;
		case WM_ERASEBKGND:
			return 0;
		/*case WM_SETTEXT:
		{
			return 0;
		}*/
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(bNoClock) break;
			hdc = BeginPaint(hwnd, &ps);
			DrawClock(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_TIMER:
			if(wParam == 1) 
				OnTimer(hwnd);
			else if(wParam == 2)
			{
				//UpdateSysRes(hwnd, bGetSysRes, bGetBattery, bGetMem);
				UpdateSysRes(hwnd, bGetSysRes, bGetBattery, bGetMem, bGetMb, bGetPm, bGetNet);
			}
			else
			{
				if(bNoClock) break;
			}
			return 0;
		// マウスダウン
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			if(nBlink)
			{
				nBlink = 0; InvalidateRect(hwnd, NULL, TRUE);
			}
			if(message == WM_LBUTTONDOWN && bStartMenuClock &&
				hwndStart && IsWindow(hwndStart))
			{
				SetWindowPos(hwndStart, NULL, 0, 0, 0, 0,
					SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
				// スタートメニュー
				bStartMenuAtClock = TRUE;
				PostMessage(hwndStart, WM_LBUTTONDOWN, wParam, lParam);
				//PostMessage(hwndStart, BM_SETSTATE, FALSE, 0);
				//SendMessage(hwndStartMenu, WM_MOVE, 100, 100);
				return 0;
			}
			PostMessage(hwndTClockMain, message, wParam, lParam);
			return 0;
		// マウスアップ
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			PostMessage(hwndTClockMain, message, wParam, lParam);
			if(message == WM_RBUTTONUP) break;
			return 0;
		case WM_MOUSEMOVE:
			return 0;
		case WM_CONTEXTMENU:   // 右クリックメニュー
			PostMessage(hwndTClockMain, message, wParam, lParam);
			return 0;
		case WM_NCHITTEST: // oldWndProcに処理させない
			return DefWindowProc(hwnd, message, wParam, lParam);
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE;
		// ファイルのドロップ
		case WM_DROPFILES:
			PostMessage(hwndTClockMain, WM_DROPFILES, wParam, lParam);
			return 0;
		case WM_NOTIFY: //ツールチップのテキスト表示
		{
			UINT code;
			code = ((LPNMHDR)lParam)->code;
			//WriteDebug("WM_NOTIFY");
			if(code == TTN_NEEDTEXT || code == TTN_NEEDTEXTW)
			{
				//WriteDebug("OnTooltipNeedText");
				OnTooltipNeedText(code, lParam);
				return 0;
			}
			return 0;
		}
		case WM_COMMAND:
			if(LOWORD(wParam) == 102) EndClock();
			return 0;
		case CLOCKM_REFRESHCLOCK: // refresh the clock
		{
			BOOL b;
			ReadData(hwnd);
			InitTrayNotify(hwnd);
			CreateClockDC(hwnd);
			b = GetMyRegLong(NULL, "DropFiles", FALSE);
			DragAcceptFiles(hwnd, b);
			InvalidateRect(hwnd, NULL, FALSE);
			InvalidateRect(GetParent(hwndClock), NULL, TRUE);
			return 0;
		}
		case CLOCKM_REFRESHTASKBAR: // refresh other elements than clock
			//EndDesktopIcons();
			//SetDesktopIcons();
			SetDesktop();
			SetStartButton(hwnd);
			SetStartMenu(hwnd);
			InitTaskSwitch(hwnd);
			InitTrayNotify(hwnd);
			CreateClockDC(hwnd);
			if(bWin2000) SetLayeredTaskbar(hwndClock);
			if(bWin2000) InitLayeredStartMenu(hwndClock);
			PostMessage(GetParent(GetParent(hwnd)), WM_SIZE,
				SIZE_RESTORED, 0);
			InvalidateRect(GetParent(GetParent(hwndClock)), NULL, TRUE);
			return 0;
		case CLOCKM_BLINK: // blink the clock
			if(wParam) { if(nBlink == 0) nBlink = 4; }
			else nBlink = 2;
			return 0;
		case CLOCKM_COPY: // copy format to clipboard
			OnCopy(hwnd, lParam);
			return 0;
		case CLOCKM_REFRESHDESKTOP:
			//SetDesktopIcons();
			SetDesktop();
			return 0;
		case CLOCKM_REFRESHCLEARTASKBAR:
		{
			bRefreshClearTaskbar = TRUE;
			if(bWin2000) SetLayeredTaskbar(hwndClock);
			return 0;
		}
		case WM_WINDOWPOSCHANGING:  // サイズ調整
		{
			LPWINDOWPOS pwp;
			if(bNoClock) break;
			pwp = (LPWINDOWPOS)lParam;
			if(IsWindowVisible(hwnd) && !(pwp->flags & SWP_NOSIZE))
			{
				int h;
				h = (int)HIWORD(OnCalcRect(hwnd));
				if(pwp->cy > h) pwp->cy = h;
				
			}
			break;
		}
	}
	
	//{
		//char s[80];
		//wsprintf(s, "%X", message);
		//WriteDebug(s);
	//}
	
	return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
}

/*------------------------------------------------
　設定の読み込みとデータの初期化
--------------------------------------------------*/
void ReadData(HWND hwnd)
{
	char fontname[80];
	int fontsize;
	LONG weight, italic;
	SYSTEMTIME lt;
	DWORD dwInfoFormat;
	char FontRotateDirection[1024];
	char ClockDrawStyle[1024];
	int angle;

	colfore = GetMyRegColor("Clock", "ForeColor", 
		GetSysColor(COLOR_BTNTEXT));
	colback = GetMyRegColor("Clock", "BackColor",
		GetSysColor(COLOR_3DFACE));
	if(GetMyRegLong("Clock", "UseBackColor2", TRUE))
		colback2 = GetMyRegColor("Clock", "BackColor2", colback);
	else colback2 = colback;
	
	GetMyRegStr("Clock", "Font", fontname, 80, "");
	
	fontsize = GetMyRegLong("Clock", "FontSize", 9);
	weight = GetMyRegLong("Clock", "Bold", 0);
	if(weight) weight = FW_BOLD;
	else weight = 0;
	italic = GetMyRegLong("Clock", "Italic", 0);
	
	GetMyRegStr("Clock", "FontRotateDirection", FontRotateDirection, 1024, "None");
	if(_strnicmp(FontRotateDirection, "RIGHT", 5) == 0)
		angle = 2700;
	else if(_strnicmp(FontRotateDirection, "LEFT", 4) == 0)
		angle = 900;
	else
		angle = 0;

	if(hFon) DeleteObject(hFon);
	hFon = CreateMyFont(fontname, fontsize, weight, italic, angle);
	
	dheight = (int)(short)GetMyRegLong("Clock", "ClockHeight", 0);
	dwidth = (int)(short)GetMyRegLong("Clock", "ClockWidth", 0);
	dvpos = (int)(short)GetMyRegLong("Clock", "VertPos", 0);
	dhpos = (int)(short)GetMyRegLong("Clock", "HorizPos", 0);
	dlineheight = (int)(short)GetMyRegLong("Clock", "LineHeight", 0);
	
	bNoClock = GetMyRegLong("Clock", "NoClockCustomize", FALSE);
	
	if(!GetMyRegStr("Format", "Format", format, 1024, "") || !format[0])
	{
		bNoClock = TRUE;
	}
	
	dwInfoFormat = FindFormat(format);
	bDispSecond = (dwInfoFormat&FORMAT_SECOND)? TRUE:FALSE;
	nDispBeat = dwInfoFormat & (FORMAT_BEAT1 | FORMAT_BEAT2);
	//bDispSecond = IsDispSecond(format);
	if(!bTimer) SetTimer(hwndClock, 1, 1000, NULL);
	bTimer = TRUE;
	//nDispBeat = IsDispBeat(format);
	
	bHour12 = GetMyRegLong("Format", "Hour12", 0);
	bHourZero = GetMyRegLong("Format", "HourZero", 0);
	
	GetLocalTime(&lt);
	LastTime.wDay = lt.wDay;
	
	InitFormat(&lt);      // format.c
	
	iClockWidth = -1;
	
	InitSysInfo(hwnd);

	tEdgeTop = (int)(short)GetMyRegLong("Clock", "ClockSkinEdgeTop", 1);
	if(tEdgeTop < 1) tEdgeTop = 1;
	tEdgeLeft = (int)(short)GetMyRegLong("Clock", "ClockSkinEdgeLeft", 1);
	if(tEdgeLeft < 1) tEdgeLeft = 1;
	tEdgeBottom = (int)(short)GetMyRegLong("Clock", "ClockSkinEdgeBottom", 1);
	if(tEdgeBottom < 1) tEdgeBottom = 1;
	tEdgeRight = (int)(short)GetMyRegLong("Clock", "ClockSkinEdgeRight", 1);
	if(tEdgeRight < 1) tEdgeRight = 1;

	bClockUseTrans = GetMyRegLong("Clock", "ClockUseTrans", FALSE);

	GetMyRegStr("Clock", "ClockSkinDrawStyle", ClockDrawStyle, 1024, "Stretch");
	if(_strnicmp(ClockDrawStyle, "TILE", 4) == 0)
		bTileTray = TRUE;
	else
		bTileTray = FALSE;

	//bCustomColorScheme = GetMyRegLong("CustomColor", "CustomColorScheme", FALSE);

}

/*void InitSysInfo(HWND hwnd)
{
	char tip[1024];
	GetMyRegStr("Tooltip", "Tooltip", tip, 1024, "");
	
	if(bTimerSysInfo) KillTimer(hwndClock, 2);
	
	bDispSysInfo = bTimerSysInfo = FALSE;
	bGetSysRes = FALSE;
	bGetBattery = FALSE;
	bGetMem = 0;
	sAvailPhysK[0] = sAvailPhysM[0] = 0;
	
	if(IsDispSysInfo(format))
	{
		bGetSysRes = TRUE; bDispSysInfo = TRUE;
	}
	if(!bGetSysRes) bGetSysRes = IsDispSysInfo(tip);
	if(bGetSysRes) bTimerSysInfo = TRUE;
	
	if(bWin95 || bWin2000)
	{
		if(FindFormatStr(format, "BL"))
		{
			bGetBattery = TRUE; bDispSysInfo = TRUE;
		}
		if(!bGetBattery) bGetBattery = FindFormatStr(tip, "BL");
		if(bGetBattery) bTimerSysInfo = TRUE;
	}
	if(FindFormatStr(format, "MK") || FindFormatStr(format, "MM"))
		bGetMem = TRUE;
	if(bGetMem) bDispSysInfo = TRUE;
	if(!bGetMem)
		bGetMem = (FindFormatStr(tip, "MK") || FindFormatStr(tip, "MM"));
	if(bGetMem) bTimerSysInfo = TRUE;
	
	if(bTimerSysInfo)
	{
		int interval;
		UpdateSysRes(hwnd, bGetSysRes, bGetBattery, bGetMem);
		interval = (int)GetMyRegLong(NULL, "IntervalSysInfo", 5);
		if(interval <= 0 || 60 < interval) interval = 5;
		SetTimer(hwndClock, 2, interval * 1000, NULL);
	}
}*/
void InitSysInfo(HWND hwnd)
{
	DWORD dwInfoFormat, dwInfoTip;
	char tip[1024];
	GetMyRegStr("Tooltip", "Tooltip", tip, 1024, "");
	
	if(bTimerSysInfo) KillTimer(hwndClock, 2);
	
	bDispSysInfo = bTimerSysInfo = FALSE;
	bGetSysRes = bGetBattery = bGetMem = FALSE;
	memset(&msMemory, 0, sizeof(msMemory));
	
	dwInfoFormat = FindFormat(format);
	dwInfoTip    = FindFormat(tip);
	
	bGetSysRes = ((dwInfoFormat | dwInfoTip) & FORMAT_SYSINFO)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_SYSINFO) bDispSysInfo = TRUE;
	
	if(bWin95 || bWin2000)
	{
		bGetBattery = ((dwInfoFormat | dwInfoTip) & FORMAT_BATTERY)? TRUE:FALSE;
		if(dwInfoFormat & FORMAT_BATTERY) bDispSysInfo = TRUE;
	}
	
	bGetMem = ((dwInfoFormat | dwInfoTip) & FORMAT_MEMORY)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_MEMORY) bDispSysInfo = TRUE;
	
	bGetMb = ((dwInfoFormat | dwInfoTip) & FORMAT_MOTHERBRD)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_MOTHERBRD) bDispSysInfo = TRUE;
	
	bGetPm = ((dwInfoFormat | dwInfoTip) & FORMAT_PERMON)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_PERMON) bDispSysInfo = TRUE;
	
	bGetNet = ((dwInfoFormat | dwInfoTip) & FORMAT_NET)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_NET) bDispSysInfo = TRUE;
	
	if(bGetSysRes || bGetBattery || bGetMem || bGetMb || bGetPm || bGetNet)
	{
		int interval;

		bTimerSysInfo = TRUE;
		UpdateSysRes(hwnd, bGetSysRes, bGetBattery, bGetMem, bGetMb, bGetPm, bGetNet);
		interval = (int)GetMyRegLong(NULL, "IntervalSysInfo", 5);
		if(interval <= 0 || 60 < interval) interval = 5;
		SetTimer(hwndClock, 2, interval * 1000, NULL);
	}
}
/*------------------------------------------------
　ツールチップウィンドウの作成
--------------------------------------------------*/
void CreateTip(HWND hwnd)
{
	TOOLINFO ti;
	
	hwndTip = CreateWindow(TOOLTIPS_CLASS, (LPSTR)NULL,
		TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hmod, NULL); 
	if(!hwndTip) return;

	ti.cbSize = sizeof(TOOLINFO); 
	ti.uFlags = 0;
	ti.hwnd = hwnd;
	ti.hinst = NULL;
	ti.uId = 1;
	ti.lpszText = LPSTR_TEXTCALLBACK;
	ti.rect.left = 0;
	ti.rect.top = 0;
	ti.rect.right = 480; 
	ti.rect.bottom = 480;
	
	SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO)&ti);
	SendMessage(hwndTip, TTM_ACTIVATE, TRUE, 0);
	SendMessage(hwndTip, TTM_SETMAXTIPWIDTH, 0, 300);
}

/*------------------------------------------------
　描画用メモリDCの作成
--------------------------------------------------*/
void CreateClockDC(HWND hwnd)
{
	RECT rc;
	COLORREF col;
	HDC hdc;
	char s[1024];

	if(hdcClock) DeleteDC(hdcClock); hdcClock = NULL;
	if(hbmpClock) DeleteObject(hbmpClock); hbmpClock = NULL;
	if(hbmpClockSkin) DeleteObject(hbmpClockSkin); hbmpClockSkin = NULL;
	
	if(bNoClock) return;
	
	if(bFillTray) 
		hwnd = GetParent(hwnd);
	GetClientRect(hwnd, &rc);
	
	hdc = GetDC(NULL);
	
	hdcClock = CreateCompatibleDC(hdc);
	if(!hdcClock)
	{
		ReleaseDC(NULL, hdc);
		return;
	}
	hbmpClock = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	
	if(bSkinTray)
	{
		if(GetMyRegStr("Clock", "ClockSkin", s, 1024, "") > 0)
		{
			hbmpClockSkin = ReadBitmap(hwnd, s, TRUE);
		}
	}

		if(!hbmpClock)
	{
		DeleteDC(hdcClock); hdcClock = NULL;
		ReleaseDC(NULL, hdc);
		return;
	}
	SelectObject(hdcClock, hbmpClock);
	
	SelectObject(hdcClock, hFon);
	SetBkMode(hdcClock, TRANSPARENT);
	SetTextAlign(hdcClock, TA_CENTER|TA_TOP);
	col = colfore;
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	SetTextColor(hdcClock, col);
	
	FillClock(hwnd, hdcClock, &rc, 0);
	
	ReleaseDC(NULL, hdc);
}

/*------------------------------------------------
   get date/time and beat to display
--------------------------------------------------*/
void GetDisplayTime(SYSTEMTIME* pt, int* beat100)
{
	FILETIME ft, lft;
	SYSTEMTIME lt;
	
	GetSystemTimeAsFileTime(&ft);
	
	if(beat100)
	{
		DWORDLONG dl;
		SYSTEMTIME st;
		int sec;
		
		dl = *(DWORDLONG*)&ft + 36000000000;
		FileTimeToSystemTime((FILETIME*)&dl, &st);
		
		sec = st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
		*beat100 = (sec * 1000) / 864;
	}
	
	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, &lt);
	memcpy(pt, &lt, sizeof(lt));
}

/*------------------------------------------------
　WM_TIMER の処理
--------------------------------------------------*/
void OnTimer(HWND hwnd)
{
	SYSTEMTIME t;
	int beat100;
	HDC hdc;
	BOOL bRedraw;
	
	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	
	if(t.wMilliseconds > 200 || (bWinNT && t.wMilliseconds > 50))
	{
		KillTimer(hwnd, 1);
		bTimerTesting = TRUE;
		SetTimer(hwnd, 1, 1001 - t.wMilliseconds, NULL);
	}
	else if(bTimerTesting)
	{
		KillTimer(hwnd, 1);
		bTimerTesting = FALSE;
		SetTimer(hwnd, 1, 1000, NULL);
	}
	
	if(CheckDaylightTimeTransition(&t))
	{
		CallWindowProc(oldWndProc, hwnd, WM_TIMER, 0, 0);
		GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	}
	
	bRedraw = FALSE;
	if(nBlink > 0) bRedraw = TRUE;
	else if(bDispSecond) bRedraw = TRUE;
	else if(nDispBeat == 1 && beatLast != (beat100/100)) bRedraw = TRUE;
	else if(nDispBeat == 2 && beatLast != beat100) bRedraw = TRUE;
	else if(bDispSysInfo) bRedraw = TRUE;
	else if(LastTime.wHour != (int)t.wHour 
		|| LastTime.wMinute != (int)t.wMinute) bRedraw = TRUE;
	
	if(bNoClock) bRedraw = FALSE;
	
	if(LastTime.wDay != t.wDay || LastTime.wMonth != t.wMonth ||
		LastTime.wYear != t.wYear)
	{
		InitFormat(&t); // format.c
		InitDaylightTimeTransition();
	}
	
	hdc = NULL;
	if(bRedraw) hdc = GetDC(hwnd);
	
	memcpy(&LastTime, &t, sizeof(t));
	
	if(nDispBeat == 1) beatLast = beat100/100;
	else if(nDispBeat > 1) beatLast = beat100;
	
	if(nBlink >= 3 && t.wMinute == 1) nBlink = 0;
	
	if(hdc)
	{
		DrawClockSub(hwnd, hdc, &t, beat100); //描画
		ReleaseDC(hwnd, hdc);
	}
	
	if(nBlink)
	{
		if(nBlink % 2) nBlink++;
		else nBlink--;
	}
	
	//CheckCursorOnStartButton();

	//SetDesktopIcons();

	//if(bStartMenuDestroyed && bWinXP)
		//InitStartMenuIE4();
}

/*------------------------------------------------
　時計の描画
--------------------------------------------------*/
void DrawClock(HWND hwnd, HDC hdc)
{
	SYSTEMTIME t;
	int beat100;
	
	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	DrawClockSub(hwnd, hdc, &t, beat100);
}

/*------------------------------------------------
  draw the clock
--------------------------------------------------*/
void DrawClockSub(HWND hwnd, HDC hdc, SYSTEMTIME* pt, int beat100)
{
	BITMAP bmp;
	RECT rcFill,  rcClock;
	TEXTMETRIC tm;
	int hf, y, w;
	char s[1024], *p, *sp;
	SIZE sz;
	int xclock, yclock, wclock, hclock, xsrc, ysrc, wsrc, hsrc;
	int xcenter;
	HRGN hRgn, hOldRgn;
	
	if(!hdcClock) CreateClockDC(hwnd);
	
	if(!hdcClock || !hbmpClock) return;
	
	GetObject(hbmpClock, sizeof(BITMAP), (LPVOID)&bmp);
	rcFill.left = rcFill.top = 0;
	rcFill.right = bmp.bmWidth; rcFill.bottom = bmp.bmHeight;
	
	FillClock(hwnd, hdcClock, &rcFill, nBlink);
	
	MakeFormat(s, pt, beat100, format);
	
	GetClientRect(hwndClock, &rcClock);
	if(bFillTray)
	{
		POINT ptTray, ptClock;
		ptTray.x = ptTray.y = 0;
		ClientToScreen(GetParent(hwndClock), &ptTray);
		ptClock.x = ptClock.y = 0;
		ClientToScreen(hwndClock, &ptClock);
		xclock = ptClock.x - ptTray.x;
		yclock = ptClock.y - ptTray.y;
	}
	else
	{
		xclock = 0; yclock = 0;
	}
	wclock = rcClock.right;  hclock = rcClock.bottom;
	
	GetTextMetrics(hdcClock, &tm);
	
	if(bFillTray)
	{
		hRgn = CreateRectRgn(xclock,  yclock,
			xclock + wclock, yclock + hclock);
		hOldRgn = SelectObject(hdcClock, hRgn);
	}
	
	hf = tm.tmHeight - tm.tmInternalLeading;
	p = s;
	y = hf / 4 - tm.tmInternalLeading / 2;
	if(bFillTray) y += yclock;
	xcenter = wclock / 2;
	if(bFillTray) xcenter += xclock;
	w = 0;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		if(*p == 0x0d) { *p = 0; p += 2; }
		if(*p == 0 && sp == s)
		{
			y = (hclock - tm.tmHeight) / 2  - tm.tmInternalLeading / 4;
			if(bFillTray) y += yclock;
		}
		TextOut(hdcClock, xcenter + dhpos, y + dvpos, sp, strlen(sp));
		
		if(GetTextExtentPoint32(hdcClock, sp, strlen(sp), &sz) == 0)
			sz.cx = strlen(sp) * tm.tmAveCharWidth;
		if(w < sz.cx) w = sz.cx;
		
		y += hf; if(*p) y += 2 + dlineheight;
	}
	
	xsrc = 0; ysrc = 0; wsrc = rcFill.right; hsrc = rcFill.bottom;
	if(bFillTray)
	{
		xsrc = xclock; ysrc = yclock;
		wsrc = wclock; hsrc = hclock;
	}
	BitBlt(hdc, 0, 0, wsrc, hsrc, hdcClock, xsrc, ysrc, SRCCOPY);
	
	if(bFillTray)
	{
		SelectObject(hdcClock, hOldRgn);
		DeleteObject(hRgn);
	}
	
	w += tm.tmAveCharWidth * 2;
	w += dwidth;
	if(w > iClockWidth)
	{
		iClockWidth = w;
		PostMessage(GetParent(GetParent(hwndClock)), WM_SIZE,
			SIZE_RESTORED, 0);
	}
}

/*------------------------------------------------
  paint background of clock
--------------------------------------------------*/
void FillClock(HWND hwnd, HDC hdc, RECT *prc, int nblink)
{
	HBRUSH hbr;
	COLORREF col;
	HDC hdcTemp;
	BITMAP bmp;

	if(nblink == 0 || (nblink % 2)) col = colfore;
	else col = colback;
	//if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	SetTextColor(hdc, col);
	
	if(bSkinTray) //
	{
		hdcTemp = CreateCompatibleDC(hdc);
		SelectObject(hdcTemp, hbmpClockSkin);
		GetObject(hbmpClockSkin, sizeof(BITMAP), (LPVOID)&bmp);
		
		hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		FillRect(hdc, prc, hbr);
		DeleteObject(hbr);

		//-----draw corners-----
		//Top Left
		TC2DrawBlt(hdc, 0, 0, tEdgeLeft, tEdgeTop, hdcTemp, 0, 
			0, tEdgeLeft, tEdgeTop, bClockUseTrans);
		//Top Right
		TC2DrawBlt(hdc, prc->right - tEdgeRight, 0, tEdgeRight, tEdgeTop, hdcTemp, 
			bmp.bmWidth - tEdgeRight, 0, tEdgeRight, tEdgeTop, bClockUseTrans);
		//Bottom Left
		TC2DrawBlt(hdc, 0, prc->bottom - tEdgeBottom, tEdgeLeft, tEdgeBottom, hdcTemp,
			0, bmp.bmHeight - tEdgeBottom, tEdgeLeft, tEdgeBottom, bClockUseTrans);
		//Bottom Right
		TC2DrawBlt(hdc, prc->right - tEdgeRight, prc->bottom - tEdgeBottom, tEdgeRight, 
			tEdgeBottom, hdcTemp, bmp.bmWidth - tEdgeRight, bmp.bmHeight - tEdgeBottom, 
			tEdgeRight, tEdgeBottom, bClockUseTrans);
		
		//-----draw edges-----
		//Top
		TC2DrawBlt(hdc, tEdgeLeft, 0, (prc->right - tEdgeRight) - tEdgeLeft, tEdgeTop, 
			hdcTemp, tEdgeLeft, 0, (bmp.bmWidth - tEdgeRight) - tEdgeLeft, tEdgeTop, bClockUseTrans);
		//Left
		TC2DrawBlt(hdc, 0, tEdgeTop, tEdgeLeft, (prc->bottom - tEdgeBottom) - tEdgeTop, 
			hdcTemp, 0, tEdgeTop, tEdgeLeft, (bmp.bmHeight - tEdgeBottom) - tEdgeTop, bClockUseTrans);
		//Bottom
		TC2DrawBlt(hdc, tEdgeLeft, prc->bottom - tEdgeBottom, 
			(prc->right - tEdgeRight) - tEdgeLeft, tEdgeBottom, hdcTemp, tEdgeLeft, 
			bmp.bmHeight - tEdgeBottom, (bmp.bmWidth - tEdgeRight) - tEdgeLeft, 
			tEdgeBottom, bClockUseTrans);
		//Right
		TC2DrawBlt(hdc, prc->right - tEdgeRight, tEdgeTop, tEdgeRight, 
			(prc->bottom - tEdgeBottom) - tEdgeTop, hdcTemp, bmp.bmWidth - tEdgeRight, 
			tEdgeTop, tEdgeRight, (bmp.bmHeight - tEdgeBottom) - tEdgeTop, bClockUseTrans);


		if(!bTileTray) //stretch the inner part of the skin
		{
			TC2DrawBlt(hdc, tEdgeLeft, tEdgeTop, (prc->right - tEdgeRight) - tEdgeLeft,
				(prc->bottom - tEdgeBottom) - tEdgeTop, hdcTemp, tEdgeLeft, tEdgeTop, 
				(bmp.bmWidth - tEdgeRight) - tEdgeLeft, (bmp.bmHeight - tEdgeBottom) - tEdgeTop, 
				bClockUseTrans);
		}else //tile the inner part of the skin, doesn't work right yet
		{
			//WriteDebug("Tile");
			TileBlt(hdc, tEdgeLeft, tEdgeTop, (prc->right - tEdgeRight) - tEdgeLeft,
				(prc->bottom - tEdgeBottom) - tEdgeTop, hdcTemp, tEdgeLeft, tEdgeTop, 
				(bmp.bmWidth - tEdgeRight) - tEdgeLeft, (bmp.bmHeight - tEdgeBottom) - tEdgeTop, 
				bClockUseTrans);
		}

		DeleteDC(hdcTemp);

	//if Tray Fill Style is set to none and we're in XP running a visual style, 
	// we fake the transparent painting of the clock.
	//Doesn't work with some visual styles, but will do for now
	}else if(!bFillTray && !bSkinTray && !bFillClock && bWinXP && IsXPStyle())
	{
		DrawXPClockBackground(hwnd, hdc, 0);

	}else //fill the clock/tray with simple colors
	{
		if(nblink || colback == colback2) //only a single color
		{
			if(nblink == 0 || (nblink % 2)) col = colback;
			else col = colfore;
			//if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
			//SetBkMode(hdc, OPAQUE);
			//SetBkColor(hdc, col);

			hbr = CreateSolidBrush(col);
			FillRect(hdc, prc, hbr);
			DeleteObject(hbr);
		}
		else //paint a gradient color, only works in 98, ME, 2K & XP
		{
			COLORREF col2;
			
			col = colback;
			//if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
			col2 = colback2;
			//if(col2 & 0x80000000) col2 = GetSysColor(col2 & 0x00ffffff);
			
			//if(bWin98)
				GradientFillClock(hdc, prc, col, col2);
			//else
				//GradientFillClock2(hdc, prc, col, col2);

		}
	}
}

/*------------------------------------------------
　時計に必要なサイズを返す
　戻り値：上位WORD 高さ　下位WORD 幅
--------------------------------------------------*/
LRESULT OnCalcRect(HWND hwnd)
{
	SYSTEMTIME t;
	int beat100;
	LRESULT w, h;
	HDC hdc;
	TEXTMETRIC tm;
	char s[1024], *p, *sp;
	SIZE sz;
	int hf;
	
	if(!(GetWindowLong(hwnd, GWL_STYLE)&WS_VISIBLE)) return 0;
	
	hdc = GetDC(hwnd);
	
	if(hFon) SelectObject(hdc, hFon);
	GetTextMetrics(hdc, &tm);
	
	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	MakeFormat(s, &t, beat100, format);
	
	p = s; w = 0; h = 0;
	hf = tm.tmHeight - tm.tmInternalLeading;
	while(*p)
	{
		sp = p;
		while(*p && *p != 0x0d) p++;
		if(*p == 0x0d) { *p = 0; p += 2; }
		if(GetTextExtentPoint32(hdc, sp, strlen(sp), &sz) == 0)
			sz.cx = strlen(sp) * tm.tmAveCharWidth;
		if(w < sz.cx) w = sz.cx;
		h += hf; if(*p) h += 2 + dlineheight;
	}
	w += tm.tmAveCharWidth * 2;
	if(iClockWidth < 0) iClockWidth = w;
	else w = iClockWidth;
	w += dwidth;
	
	h += hf / 2 + dheight;
	if(h < 4) h = 4;
	
	ReleaseDC(hwnd, hdc);
	
	return (h << 16) + w;
}

/*------------------------------------------------
　ツールチップの表示
--------------------------------------------------*/
void OnTooltipNeedText(UINT code, LPARAM lParam)
{
	SYSTEMTIME t;
	int beat100;
	char fmt[1024], s[1024];
	int widecharlen;
    char newTip[256];

	GetMyRegStr("Tooltip", "Tooltip", fmt, 1024, "");
	if(fmt[0] == 0) strcpy(fmt, "\"TClock\" LDATE");
	GetDisplayTime(&t, &beat100);
	MakeFormat(s, &t, beat100, fmt);
	
	//if(code == TTN_GETDISPINFO)
	//{
		//lpttd = (LPNMTTDISPINFO)lpnmhdr;
		//SendMessage(((LPNMHDR)lParam)->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
		//((LPTOOLTIPTEXT)lParam)->szText = s;
	//}

    /*ConvertTip(newTip, (WCHAR *)((LPTOOLTIPTEXT)lParam)->szText, code);

    if (stricmp(((LPTOOLTIPTEXT)lParam)->szText, newTip))
            strcpy(((LPTOOLTIPTEXT)lParam)->szText, newTip);*/

	if(code == TTN_NEEDTEXT)
		strcpy(((LPTOOLTIPTEXT)lParam)->szText, s);
	else
	{
		MultiByteToWideChar(CP_ACP, 0, s, -1,
			((LPTOOLTIPTEXTW)lParam)->szText, 80);
	}
	SetWindowPos(hwndTip, HWND_TOPMOST, 0, 0, 0, 0,
		SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);

}


/*------------------------------------------------
  copy date/time text to clipboard
--------------------------------------------------*/
void OnCopy(HWND hwnd, LPARAM lParam)
{
	SYSTEMTIME t;
	int beat100;
	char entry[20], fmt[256], s[1024], *pbuf;
	HGLOBAL hg;
	//BOOL br, bb, bm;
	BOOL br, bb, bm, bmb, bpm, bn;
	DWORD dwInfoFormat;

	GetDisplayTime(&t, &beat100);
	wsprintf(entry, "%d%dClip",
		(int)LOWORD(lParam), (int)HIWORD(lParam));
	GetMyRegStr("Mouse", entry, fmt, 256, "");
	if(fmt[0] == 0) strcpy(fmt, format);
	
	dwInfoFormat = FindFormat(fmt);
	br = (dwInfoFormat&FORMAT_SYSINFO)? TRUE:FALSE;
	bb = (dwInfoFormat&FORMAT_BATTERY)? TRUE:FALSE;
	bm = (dwInfoFormat&FORMAT_MEMORY) ? TRUE:FALSE;
	bmb = (dwInfoFormat&FORMAT_MOTHERBRD) ? TRUE:FALSE;
	bpm = (dwInfoFormat&FORMAT_PERMON) ? TRUE:FALSE;
	bn = (dwInfoFormat&FORMAT_NET) ? TRUE:FALSE;
	if(br || bb || bm || bmb || bpm || bn)
		UpdateSysRes(hwnd, br, bb, bm, bmb, bpm, bn);
	
	/*br = IsDispSysInfo(fmt);
	bb = FindFormatStr(fmt, "BL");
	bm = (FindFormatStr(fmt, "MK") || FindFormatStr(fmt, "MM"));
	if(br || bb || bm)
		UpdateSysRes(hwnd, br, bb, bm);*/
	
	MakeFormat(s, &t, beat100, fmt);
	
	if(!OpenClipboard(hwnd)) return;
	EmptyClipboard();
	hg = GlobalAlloc(GMEM_DDESHARE, strlen(s) + 1);
	pbuf = (char*)GlobalLock(hg);
	strcpy(pbuf, s);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard(); 
}

void UpdateSysRes(HWND hwnd, BOOL bsysres, BOOL bbattery, BOOL bmem, BOOL bmb, BOOL bpermon, BOOL bnet)
{
	int i;
	
	if(bsysres)
	{
//		char s[10];
		
		if(bWin95)
		{
			for(i = 0; i < 3; i++)
				iFreeRes[i] = GetFreeSystemResources((WORD)i);
		}
		iCPUUsage = CpuMoni_get(); // cpu.c
		
//		wsprintf(s, "%d", iCPUUsage);
//		WriteDebug(s);
	}
	if((bWin95 || bWin2000) && bbattery)
	{
		iBatteryLife = GetBatteryLifePercent();
	}
	if(bmem)
	{
		msMemory.dwLength = sizeof(msMemory);
		GlobalMemoryStatus(&msMemory);
	}
	if(bmb)
	{
		MBMSharedDataUpdate();
	}
	if(bpermon)
	{
//		PerMoni_get();
	}
	if(bnet)
	{
		Net_get();
	}
}
/*void UpdateSysRes(HWND hwnd, BOOL bsysres, BOOL bbattery, BOOL bmem)
{
	int i;
	
	if(bsysres)
	{
//		char s[10];
		
		if(bWin95)
		{
			for(i = 0; i < 3; i++)
				iFreeRes[i] = GetFreeSystemResources((WORD)i);
		}
		iCPUUsage = CpuMoni_get(); // cpu.c
		
//		wsprintf(s, "%d", iCPUUsage);
//		WriteDebug(s);
	}
	if((bWin95 || bWin2000) && bbattery)
	{
		iBatteryLife = GetBatteryLifePercent();
	}
	if(bmem)
	{
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);
		wsprintf(sAvailPhysK, "%d", ms.dwAvailPhys / 1024);
		wsprintf(sAvailPhysM, "%d", ms.dwAvailPhys / (1024 * 1024));
	}
}*/

int iHourTransition = -1, iMinuteTransition = -1;

/*------------------------------------------------
  initialize time-zone information
--------------------------------------------------*/
void InitDaylightTimeTransition(void)
{
	SYSTEMTIME lt, *plt;
	TIME_ZONE_INFORMATION tzi;
	DWORD dw;
	BOOL b;
	
	iHourTransition = iMinuteTransition = -1;
	
	GetLocalTime(&lt);
	
	b = FALSE;
	memset(&tzi, 0, sizeof(tzi));
	dw = GetTimeZoneInformation(&tzi);
	if(dw == TIME_ZONE_ID_STANDARD
	  && tzi.DaylightDate.wMonth == lt.wMonth
	  && tzi.DaylightDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.DaylightDate);
	}
	if(dw == TIME_ZONE_ID_DAYLIGHT
	  && tzi.StandardDate.wMonth == lt.wMonth
	  && tzi.StandardDate.wDayOfWeek == lt.wDayOfWeek)
	{
		b = TRUE; plt = &(tzi.StandardDate);
	}
	
	if(b && plt->wDay < 5)
	{
		if(((lt.wDay - 1) / 7 + 1) == plt->wDay)
		{
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	}
	else if(b && plt->wDay == 5)
	{
		FILETIME ft;
		SystemTimeToFileTime(&lt, &ft);
		*(DWORDLONG*)&ft += 6048000000000i64;
		FileTimeToSystemTime(&ft, &lt);
		if(lt.wDay < 8)
		{
			iHourTransition = plt->wHour;
			iMinuteTransition = plt->wMinute;
		}
	}
}

BOOL CheckDaylightTimeTransition(SYSTEMTIME *plt)
{
	if((int)plt->wHour == iHourTransition &&
	   (int)plt->wMinute >= iMinuteTransition)
	{
		iHourTransition = iMinuteTransition = -1;
		return TRUE;
	}
	else return FALSE;
}

