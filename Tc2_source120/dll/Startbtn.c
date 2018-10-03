/*-------------------------------------------
  startbtn.c
    customize start button
    Kazubon 1997-1999
---------------------------------------------*/

#include "tcdll.h"

extern HANDLE hmod;


/*------------------------------------------------
  globals
--------------------------------------------------*/
LRESULT CALLBACK WndProcStart(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcTask(HWND, UINT, WPARAM, LPARAM);

HWND hwndStart = NULL;
HWND hwndRebar = NULL;
HWND hwndStartTip = NULL;
BOOL bStartMenuClock = FALSE;
static LONG oldClassStyleStartButton;

static WNDPROC oldWndProcStart = NULL, oldWndProcTask = NULL;
static HWND hwndTask = NULL, hwndTray = NULL;
static HBITMAP hbmpstart = NULL, hbmpstartold = NULL;
static int wStart = -1, hStart = -1;
static BOOL bCustStartButton = FALSE;
static BOOL bHideStartButton = FALSE;
static BOOL bStartButtonFlat = FALSE;
static BOOL bCursorOnStartButton = FALSE;
static BOOL bStartButtonUseTrans = FALSE;

BOOL bCoolStartButton = FALSE;
BOOL bSkinnedStartButton = FALSE;
BOOL bNormalStartButton = FALSE;
COLORREF CoolStartColorHigh, CoolStartHoverColorHigh, CoolStartDownColorHigh,
			CoolStartColorLow, CoolStartHoverColorLow, CoolStartDownColorLow,
			CoolStartColorFace, CoolStartHoverColorFace, CoolStartDownColorFace,
			CoolStartColorHigh2, CoolStartHoverColorHigh2, CoolStartDownColorHigh2,
			CoolStartColorLow2, CoolStartHoverColorLow2, CoolStartDownColorLow2,
			CoolStartColorFore, ColorGripperHigh, ColorGripperLow, ColorRebarFace;

static void OnPaint(HWND hwnd, HDC hdc);
static void SetStartButtonBmp(void);
static void SetTaskWinPos(void);
static void SetRebarGrippers(void);
static BOOL bNormalGrippers = FALSE;
static BOOL bHiddenGrippers = FALSE;
//static BOOL bColorGrippers = FALSE;
static BOOL bIsRebar = FALSE;

char StartButtonType[1024];
char GripperType[1024];
char AltLaunchApp[1024];
BOOL bAltLaunchApp = FALSE;

extern BOOL bWinXP;
extern BOOL bWin2000;
extern BOOL bWin98;

extern HWND hwndStartMenu;
extern BOOL bStartMenuDestroyed;
extern BOOL bStartMenuAtClock;
//extern BOOL bCustomColorScheme;

/*--------------------------------------------------
   initialize
----------------------------------------------------*/
void SetStartButton(HWND hwndClock)
{
	HANDLE hwnd;
	char classname[80];
	
	EndStartButton();
	
	bStartMenuClock = FALSE;
	bStartMenuClock = GetMyRegLong("StartMenu", "StartMenuClock", FALSE);
	
	// "button"と"MSTaskSwWClass"のウィンドウハンドルを得る
	hwndStart = hwndTask = NULL;
	hwndTray = GetParent(hwndClock); // TrayNotifyWnd
	if(hwndTray == NULL)
		return;
	hwnd = GetParent(hwndTray);      // Shell_TrayWnd
	if(hwnd == NULL)
		return;
	
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "Button") == 0)
			hwndStart = hwnd;
		/*else if(lstrcmpi(classname, "MSTaskSwWClass") == 0)
		{
			hwndTask = hwnd;
			bIsRebar = FALSE;
		}*/
		else if(lstrcmpi(classname, "ReBarWindow32") == 0)
		{
			hwndTask = hwnd;
			hwndRebar = hwnd;
			bIsRebar = TRUE;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	if(hwndStart == NULL || hwndTask == NULL)
	{
		hwndStart = hwndTask = NULL; return;
	}
	
	bHideStartButton = GetMyRegLong("StartButton", "StartButtonHide", FALSE);
	if(!bHideStartButton)
		bCustStartButton = GetMyRegLong("StartButton", "CustomizeStartButton", FALSE);

	if(bIsRebar)
	{
		GetMyRegStr("Taskbar", "RebarGripperType", GripperType, 1024, "Normal");
		if(_strnicmp(GripperType, "HIDDEN", 6) == 0)
		{
			bHiddenGrippers = TRUE;
			//bColorGrippers = FALSE;
			//ColorGripperHigh = GetSysColor(COLOR_3DFACE);
			//ColorGripperLow = GetSysColor(COLOR_3DFACE);
			SetRebarGrippers();
		}
		/*else if(_strnicmp(GripperType, "COLOR", 5) == 0)
		{
			bHiddenGrippers = FALSE;
			bColorGrippers = TRUE;
			ColorGripperHigh = GetMyRegColor("Taskbar", "RebarGripperColorHigh", GetSysColor(COLOR_3DHILIGHT));
			ColorGripperLow = GetMyRegColor("Taskbar", "RebarGripperColorLow", GetSysColor(COLOR_3DSHADOW));
			SetRebarGrippers();
		}*/
		else
			bNormalGrippers = TRUE;

		/*if(!bHiddenGrippers && bCustomColorScheme)
		{
			ColorGripperHigh = GetMyRegColor("CustomColor", "RebarGripperColorHigh", GetSysColor(COLOR_3DHILIGHT));
			ColorGripperLow = GetMyRegColor("CustomColor", "RebarGripperColorLow", GetSysColor(COLOR_3DSHADOW));
			ColorRebarFace = GetMyRegColor("CustomColor", "RebarColorFace", GetSysColor(COLOR_3DFACE));
			SetRebarGrippers();
		}*/

	}

	if(!bCustStartButton && !bHideStartButton && !bHiddenGrippers)
		return;

	if(bCustStartButton)
	{
		CoolStartColorFore = GetMyRegColor("StartButton", "TextColor", GetSysColor(COLOR_BTNTEXT));

		GetMyRegStr("StartButton", "StartButtonType", StartButtonType, 1024, "Normal");
		if(_strnicmp(StartButtonType, "COLOR", 5) == 0)
		{
			bCoolStartButton = TRUE;
			CoolStartColorHigh = GetMyRegColor("StartButton", "3DHILIGHTColor", GetSysColor(COLOR_3DHILIGHT));
			CoolStartColorHigh2 = GetMyRegColor("StartButton", "3DLIGHTColor", GetSysColor(COLOR_3DLIGHT));		
			CoolStartColorLow = GetMyRegColor("StartButton", "3DDKSHADOWColor", GetSysColor(COLOR_3DDKSHADOW));
			CoolStartColorLow2 = GetMyRegColor("StartButton", "3DSHADOWColor", GetSysColor(COLOR_3DSHADOW));
			CoolStartColorFace = GetMyRegColor("StartButton", "3DFACEColor", GetSysColor(COLOR_3DFACE));

			CoolStartHoverColorHigh = GetMyRegColor("StartButton", "3DHILIGHTHoverColor", CoolStartColorHigh);
			CoolStartHoverColorHigh2 = GetMyRegColor("StartButton", "3DLIGHTHoverColor", CoolStartColorHigh2);		
			CoolStartHoverColorLow = GetMyRegColor("StartButton", "3DDKSHADOWHoverColor", CoolStartColorLow);
			CoolStartHoverColorLow2 = GetMyRegColor("StartButton", "3DSHADOWHoverColor", CoolStartColorLow2);
			CoolStartHoverColorFace = GetMyRegColor("StartButton", "3DFACEHoverColor", CoolStartColorFace);

			CoolStartDownColorHigh = GetMyRegColor("StartButton", "3DHILIGHTDownColor", CoolStartColorHigh);
			CoolStartDownColorHigh2 = GetMyRegColor("StartButton", "3DLIGHTDownColor", CoolStartColorHigh2);		
			CoolStartDownColorLow = GetMyRegColor("StartButton", "3DDKSHADOWDownColor", CoolStartColorLow);
			CoolStartDownColorLow2 = GetMyRegColor("StartButton", "3DSHADOWDownColor", CoolStartColorLow2);
			CoolStartDownColorFace = GetMyRegColor("StartButton", "3DFACEDownColor", CoolStartColorFace);
		}
		else if(_strnicmp(StartButtonType, "FLAT", 4) == 0)
		{
			bStartButtonFlat = TRUE;
			CoolStartColorHigh = GetSysColor(COLOR_3DHILIGHT);
			CoolStartColorHigh2 = GetSysColor(COLOR_3DFACE);		
			CoolStartColorLow = GetSysColor(COLOR_3DSHADOW);
			CoolStartColorLow2 = GetSysColor(COLOR_3DFACE);
			CoolStartColorFace = GetSysColor(COLOR_3DFACE);
		}
		else if(_strnicmp(StartButtonType, "SKIN", 4) == 0)
		{
			bSkinnedStartButton = TRUE;
			CoolStartColorFace = GetSysColor(COLOR_3DFACE);
		}
		else
		{
			bNormalStartButton = TRUE;
			CoolStartColorHigh = GetSysColor(COLOR_3DHILIGHT);
			CoolStartColorHigh2 = GetSysColor(COLOR_3DLIGHT);
			CoolStartColorLow = GetSysColor(COLOR_3DDKSHADOW);
			CoolStartColorLow2 = GetSysColor(COLOR_3DSHADOW);
			CoolStartColorFace = GetSysColor(COLOR_3DFACE);
		}

		oldClassStyleStartButton = GetClassLong(hwndStart, GCL_STYLE);
		SetClassLong(hwndStart, GCL_STYLE,
			oldClassStyleStartButton & ~(CS_HREDRAW|CS_VREDRAW));
	}

	if(GetMyRegStr("StartButton", "AlternateLaunchApp", AltLaunchApp, 1024, "") > 0)
		bAltLaunchApp = TRUE;
	
	bStartButtonUseTrans = GetMyRegLong("StartButton", "StartButtonUseTrans", FALSE);

	oldWndProcStart = (WNDPROC)GetWindowLong(hwndStart, GWL_WNDPROC);
	SetWindowLongA(hwndStart, GWL_WNDPROC, (LONG)WndProcStart);
	oldWndProcTask = (WNDPROC)GetWindowLong(hwndTask, GWL_WNDPROC);
	SetWindowLongA(hwndTask, GWL_WNDPROC, (LONG)WndProcTask);
	
	if(bHideStartButton) // ボタンを隠す
	{
		RECT rc; POINT pt;
		ShowWindow(hwndStart, SW_HIDE);
		wStart = 0; hStart = 0;
		GetWindowRect(hwndTray, &rc);
		pt.x = rc.left; pt.y = rc.top;
		ScreenToClient(GetParent(hwndTray), &pt);
		SetWindowPos(hwndStart, NULL, pt.x, pt.y,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER|SWP_NOACTIVATE);
	}
	else if(bCustStartButton)
	{
		// ボタン用ビットマップの設定
		SetStartButtonBmp();
	}
	// MSTaskSwWClassの位置・サイズの設定
	if(bCustStartButton || bHideStartButton)
		SetTaskWinPos();
}

/*--------------------------------------------------
    reset start button
----------------------------------------------------*/
void EndStartButton(void)
{
	if(bHiddenGrippers && hwndTask && IsWindow(hwndTask))
	{
		COLORSCHEME colScheme2;

		colScheme2.dwSize = sizeof(colScheme2);
		colScheme2.clrBtnHighlight = GetSysColor(COLOR_3DHILIGHT);
		colScheme2.clrBtnShadow = GetSysColor(COLOR_3DSHADOW);

		//if(bCustomColorScheme)
			//SendMessage(hwndTask, RB_SETBKCOLOR, 0, (LPARAM)(COLORREF)GetSysColor(COLOR_3DFACE));

		SendMessage(hwndTask, RB_SETCOLORSCHEME, 0, (LPARAM) (LPCOLORSCHEME)&colScheme2);
		InvalidateRect(hwndTask, NULL, TRUE);

	}

	if(hwndStart && IsWindow(hwndStart))
	{
		if(hbmpstartold != NULL)
		{
			SendMessage(hwndStart, BM_SETIMAGE,
				0, (LPARAM)hbmpstartold);
			hbmpstartold = NULL;
		}
		SetClassLong(hwndStart, GCL_STYLE, oldClassStyleStartButton);

		if(oldWndProcStart)
			SetWindowLong(hwndStart, GWL_WNDPROC, (LONG)oldWndProcStart);
		oldWndProcStart = NULL;
		SetWindowPos(hwndStart, NULL, 0, 0, 0, 0,
			SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		ShowWindow(hwndStart, SW_SHOW);
	}
	hwndStart = NULL;
	
	if(hbmpstart) DeleteObject(hbmpstart); hbmpstart = NULL;
	
	if(hwndTask && IsWindow(hwndTask) && oldWndProcTask)
		SetWindowLong(hwndTask, GWL_WNDPROC, (LONG)oldWndProcTask);
	oldWndProcTask = NULL; hwndStart = NULL;
	
	bCustStartButton = bHideStartButton = bStartButtonFlat = bCoolStartButton = FALSE;
	bNormalStartButton = bSkinnedStartButton = bStartButtonUseTrans = FALSE;

	bHiddenGrippers = bNormalGrippers = FALSE;

	bAltLaunchApp = FALSE;

}

/*------------------------------------------------
   subclass procedure of start button
--------------------------------------------------*/
LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SYSCOLORCHANGE:  // システムの設定変更
		case WM_WININICHANGE:
			if(bCustStartButton && !bHideStartButton && !bSkinnedStartButton)
				PostMessage(hwnd, WM_USER+10, 0, 0L);
			return 0;
		case (WM_USER + 10):     // 再初期化
			SetStartButtonBmp();
			return 0;
		case WM_ERASEBKGND:
			return 1;
		case WM_WINDOWPOSCHANGING:  // サイズを変更させない
		{
			LPWINDOWPOS pwp;
			if(!(bCustStartButton || bHideStartButton)) break;
			pwp = (LPWINDOWPOS)lParam;
			if(!(pwp->flags & SWP_NOSIZE))
			{
				if(wStart > 0) pwp->cx = wStart;
				if(hStart > 0) pwp->cy = hStart;
			}
			if(bHideStartButton)
			{
				RECT rc; POINT pt;
				GetWindowRect(hwndTray, &rc);
				pt.x = rc.left; pt.y = rc.top;
				ScreenToClient(GetParent(hwndTray), &pt);
				pwp->x = pt.x; pwp->y = pt.y;
				pwp->cx = rc.right - rc.left;
				pwp->cy = rc.bottom - rc.top;
			}
			break;
		}
		case WM_DESTROY:
			if(hbmpstartold)
				SendMessage(hwndStart, BM_SETIMAGE,
					0, (LPARAM)hbmpstartold);
			hbmpstartold = NULL;
			if(hbmpstart) DeleteObject(hbmpstart); hbmpstart = NULL;
			break;

		// -------- for "flat start button" -----------
		case WM_PAINT:
		{
			HDC hdc;
			PAINTSTRUCT ps;
			if(!bCustStartButton) break;
			hdc = BeginPaint(hwnd, &ps);
			OnPaint(hwnd, hdc);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case BM_SETSTATE:
		{
			LRESULT r;
			HDC hdc;
			//if(bStartMenuAtClock)
			//{
				//PostMessage(hwnd, BM_SETSTATE, FALSE, 0);
				//bStartMenuAtClock = FALSE;
			//}
			if(!bCustStartButton) break;
			if(!bAltLaunchApp)
				r = CallWindowProc(oldWndProcStart, hwnd, message, wParam, lParam);
			hdc = GetDC(hwnd);
			OnPaint(hwnd, hdc);
			ReleaseDC(hwnd, hdc);
			return 0;
		}
		case WM_SETFOCUS:
			if(bCustStartButton) break;
			return 0;
		case WM_MOUSEMOVE:
		{
			if(!bCursorOnStartButton && (bStartButtonFlat || bSkinnedStartButton))
			{
				TRACKMOUSEEVENT lpEventTrack;
				lpEventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				lpEventTrack.dwFlags = TME_LEAVE;
				lpEventTrack.hwndTrack = hwnd;
				bCursorOnStartButton = TRUE;
				_TrackMouseEvent(&lpEventTrack);
				InvalidateRect(hwnd, NULL, FALSE);
				//if(bStartButtonFlat || bSkinnedStartButton)
				//{
					//CheckCursorOnStartButton();
				//}
			}
			break;

			//return 0;
		}
		case WM_LBUTTONDOWN:
		{
			////if(!bAltLaunchApp)
			////{
				LRESULT r;
				r = CallWindowProc(oldWndProcStart, hwnd, message, wParam, lParam);
				//if(bStartMenuAtClock)
				//{
					//SendMessage(hwnd, BM_SETSTATE, FALSE, 0);
					//bStartMenuAtClock = FALSE;
				//}

				if(bStartMenuDestroyed) // && (bWinXP || bWin2000))
				{
					Pause(hwnd,"250");
					//ClearStartMenuIE4();
					InitStartMenuIE4();
					//WriteDebug("WM_LBUTTONDOWN");
					//bStartMenuDestroyed = FALSE;

					//SendMessage(hwndStartMenu, WM_SYSCOLORCHANGE, 0, 0);
				}
				//break;
				//}
				//InitStartMenuIE4();
				return r;
			////}else
			////{
				////SendMessage(hwnd, BM_SETSTATE, TRUE, 0);
				////if(AltLaunchApp[0]) ExecFile(hwnd, AltLaunchApp);
				////return 0;
			////}
		}
			//if(bAltLaunchApp)
			//{
				//SendMessage(hwnd, BM_SETSTATE, TRUE, 0);
				//if(AltLaunchApp[0]) ExecFile(hwnd, AltLaunchApp);
				//return 0;
			//}
		//case WM_KILLFOCUS:
		//{
			//char classname[80];

			//GetClassName((HWND)GetForegroundWindow, classname, 80);
			//WriteDebug(classname);
			//InitStartMenuIE4();
			//Pause(hwnd,"250");
			//Pause(hwnd,"1000");
			//ClearStartMenuIE4();
			//InitStartMenuIE4();

			//SendMessage(hwndStartMenu, WM_SYSCOLORCHANGE, 0, 0);
			//ShowWindow(hwndStartMenu, FALSE);
			//ShowWindow(hwndStartMenu, TRUE);
			//WriteDebug("WM_LBUTTONUP");
			//wParam;
			//return 0;
		//}
			//if(bAltLaunchApp)
			//{
				//if(AltLaunchApp[0]) ExecFile(hwnd, AltLaunchApp);
				//SendMessage(hwnd, BM_SETSTATE, TRUE, 0);
				//return 0;
			//}
		case WM_MOUSELEAVE:
		{
			bCursorOnStartButton = FALSE;
			InvalidateRect(hwnd, NULL, FALSE);
			//WriteDebug("WM_MOUSELEAVE");
			break;
		}

	}
	return CallWindowProc(oldWndProcStart, hwnd, message, wParam, lParam);
}

/*--------------------------------------------------
   subclass procedure of
   "MSTaskSwWClass"/"ReBarWindow32" class window
----------------------------------------------------*/
LRESULT CALLBACK WndProcTask(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		//case WM_PAINT:
			//return 0;

		case WM_SYSCOLORCHANGE:
		{
			LRESULT r;
			if(!bHiddenGrippers) break;

			r = CallWindowProc(oldWndProcTask, hwnd, message, wParam, lParam);
			SetRebarGrippers();
			return r;
		}
		case WM_WINDOWPOSCHANGING: // 位置・サイズの制限
		{
			LPWINDOWPOS pwp;
			RECT rcBar, rcTray;
			
			if(!(bCustStartButton || bHideStartButton)) break;
			
			pwp = (LPWINDOWPOS)lParam;
			
			GetClientRect(GetParent(hwndStart), &rcBar); // タスクバー
			GetWindowRect(hwndTray, &rcTray); // TrayNotifyWnd
			
			// タスクバーが横置きのとき
			if(rcBar.right > rcBar.bottom)
			{
				if(bHiddenGrippers && !bWinXP)
				{
					//pwp->x = wStart - 2;
					//pwp->cx = rcTray.left - 2 - wStart + 6; // 横幅
					pwp->x = 2 + wStart; // 右位置
					pwp->cx = rcTray.left - 2 - wStart - 2; // 横幅

				}
				else
				{
					pwp->x = 2 + wStart; // 右位置
					pwp->cx = rcTray.left - 2 - wStart - 2; // 横幅
				}
				
				if(wStart > 0)
				{
					pwp->x += 2; pwp->cx -= 2;
				}
			}
			else // 縦置きのとき
			{
				if(rcTray.top < pwp->y)
				{
					if(bHiddenGrippers && !bWinXP)
					{
						//pwp->cy = rcBar.bottom - 2 - hStart + 6; // 高さ
						//pwp->y = hStart - 2; // 上位置
						pwp->cy = rcBar.bottom - 2 - hStart - 2; // 高さ
						pwp->y = 2 + hStart; // 上位置

					}else
					{
						pwp->cy = rcBar.bottom - 2 - hStart - 2; // 高さ
						pwp->y = 2 + hStart; // 上位置
					}
				}
				else
				{
					if(bHiddenGrippers && !bWinXP)
					{
						//pwp->cy = rcTray.top - 2 - hStart + 6; // 高さ
						//pwp->y = hStart - 2; // 上位置
						pwp->cy = rcTray.top - 2 - hStart - 2; // 高さ
						pwp->y = 2 + hStart; // 上位置

					}else
					{
						pwp->cy = rcTray.top - 2 - hStart - 2; // 高さ
						pwp->y = 2 + hStart; // 上位置
					}
				}
				if(hStart > 0)
				{
					pwp->y += 1; pwp->cy -= 2;
				}
			}
			break;
		}
	}
	return CallWindowProc(oldWndProcTask, hwnd, message, wParam, lParam);
}

/*--------------------------------------------------
　スタートボタンのビットマップとサイズの設定
----------------------------------------------------*/
void SetStartButtonBmp(void)
{
	char s[1024], caption[80];
	HBITMAP hbmpicon, hbmpold;
	HICON hicon, hiconl;
	HDC hdc, hdcMem;
	HFONT hfont;
	BITMAP bmp;
	int whbmp, hhbmp, cxicon, cyicon;
	char fontname[80];
	int fontsize;
	LONG weight, italic;

	if(hwndStart == NULL) return;
	
	hbmpicon = NULL; hicon = NULL;
	if(GetMyRegLong("StartButton", "StartButtonIconLarge", FALSE))
	{
		cxicon = GetSystemMetrics(SM_CXICON);
		cyicon = GetSystemMetrics(SM_CYICON);
	}
	else
	{
		cxicon = GetSystemMetrics(SM_CXSMICON);
		cyicon = GetSystemMetrics(SM_CYSMICON);
	}

	// ファイルからアイコン用ビットマップの読み込み
	if(GetMyRegStr("StartButton", "StartButtonIcon", s, 1024, "") > 0)
	{
		char fname[MAX_PATH], head[2];
		HFILE hf;
		
		parse(fname, s, 0);
		hf = _lopen(fname, OF_READ);
		if(hf != HFILE_ERROR)
		{
			_lread(hf, head, 2);
			_lclose(hf);
			if(head[0] == 'B' && head[1] == 'M') //ビットマップの場合
				hbmpicon = ReadBitmap(hwndStart, fname, TRUE);
			else if(head[0] == 'M' && head[1] == 'Z') //実行ファイルの場合
			{
				char numstr[10], *p; int n;
				parse(numstr, s, 1);
				n = 0; p = numstr;
				while(*p)
				{
					if(*p < '0' || '9' < *p) break;
					n = n * 10 + *p++ - '0';
				}
				if(ExtractIconEx(fname, n, &hiconl, &hicon, 1) < 2)
				{
					if(!hicon) hicon = CopyIcon(hiconl);
				}
				else if(ExtractIconEx(fname, n, &hiconl, &hicon, 1) > 1)
				{
					if(GetMyRegLong("StartButton", "StartButtonIconLarge", FALSE))
						hicon = CopyIcon(hiconl);;
				}
				DestroyIcon(hiconl);

			}
			else // アイコンの場合
			{
				hicon = (HICON)LoadImage(hmod, fname,
					IMAGE_ICON, cxicon, cyicon,
					LR_DEFAULTCOLOR|LR_LOADFROMFILE);
			}
		}
	}
	
	if(hbmpicon)
	{
		GetObject(hbmpicon, sizeof(BITMAP), (LPVOID)&bmp);
		cxicon = bmp.bmWidth; 
		cyicon = bmp.bmHeight;
	}

	// キャプションの取得
	GetMyRegStr("StartButton", "StartButtonCaption", caption, 80, "");
	
	hdc = GetDC(hwndStart);
	
	// ボタン用のフォント = タイトルバーのフォント + BOLD
	hfont = NULL;
	whbmp = cxicon; hhbmp = cyicon;
	if(caption[0] && !bSkinnedStartButton)
	{
		NONCLIENTMETRICS ncm;
		SIZE sz;
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

		GetMyRegStr("StartButton", "Font", fontname, 80, ncm.lfCaptionFont.lfFaceName);
		
		fontsize = GetMyRegLong("StartButton", "FontSize", ncm.lfCaptionFont.lfHeight);
		weight = GetMyRegLong("StartButton", "Bold", 1);
		if(weight) weight = FW_BOLD;
		else weight = 0;
		italic = GetMyRegLong("StartButton", "Italic", 0);
		
		if(hfont) DeleteObject(hfont);
		hfont = CreateMyFont(fontname, fontsize, weight, italic, 0);

		//ncm.lfCaptionFont.lfWeight = FW_BOLD;
		//hfont =  CreateFontIndirect(&(ncm.lfCaptionFont));
		SelectObject(hdc, hfont);
		
		//キャプションの幅を得る
		GetTextExtentPoint32(hdc, caption, strlen(caption), &sz);
		whbmp = sz.cx;
		if(hbmpicon || hicon) whbmp += cxicon + 2;
		hhbmp = sz.cy;
		if((hbmpicon || hicon) && cyicon > sz.cy)
			hhbmp = cyicon;
	}
	
	// ビットマップの作成
	hdcMem = CreateCompatibleDC(hdc);
	hbmpstart = CreateCompatibleBitmap(hdc, whbmp, hhbmp);
	SelectObject(hdcMem, hbmpstart);
	
	{ // 背景色で塗りつぶし
		RECT rc; HBRUSH hbr;
		SetRect(&rc, 0, 0, whbmp, hhbmp);
		hbr = CreateSolidBrush(CoolStartColorFace);
		FillRect(hdcMem, &rc, hbr);
		DeleteObject(hbr);
	}
	
	// ビットマップにアイコンの絵を描画
	if(hbmpicon)
	{
		HDC hdcicon;
		hdcicon = CreateCompatibleDC(hdc);
		SelectObject(hdcicon, hbmpicon);
			
		TC2DrawBlt(hdcMem, 0, (hhbmp - cyicon)/2, cxicon, 
			cyicon, hdcicon, 0, 0, cxicon, cyicon, bStartButtonUseTrans);

		DeleteDC(hdcicon);
		DeleteObject(hbmpicon);
	}
	if(hicon)
	{
		DrawIconEx(hdcMem, 0, (hhbmp - cyicon)/2,
			hicon, cxicon, cyicon, 0, NULL, DI_NORMAL);
		DestroyIcon(hicon);
	}
	
	// ビットマップにキャプションを書く
	if(caption[0] && !bSkinnedStartButton)
	{
		TEXTMETRIC tm;
		int x, y;

		GetTextMetrics(hdc, &tm);
		SelectObject(hdcMem, hfont);
		x = 0; if(hbmpicon || hicon) x = cxicon + 2;
		y = (hhbmp - tm.tmHeight) / 2;
		SetBkMode(hdcMem, TRANSPARENT);
		SetTextColor(hdcMem, CoolStartColorFore);
		TextOut(hdcMem, x, y, caption, strlen(caption));
	}
	
	DeleteDC(hdcMem);
	ReleaseDC(hwndStart, hdc);
	if(hfont) DeleteObject(hfont);

	// ボタンにビットマップを設定
	hbmpold = (HBITMAP)SendMessage(hwndStart,
		BM_SETIMAGE, 0, (LPARAM)hbmpstart);

	// 以前のビットマップを保存 / 破棄
	if(hbmpstartold == NULL) hbmpstartold = hbmpold;
	else DeleteObject(hbmpold);
	
	// ボタンのサイズの設定  上限：160x80
	if(!bSkinnedStartButton)
	{
		wStart = whbmp + 8;
		if(wStart > 160) wStart = 160;
		hStart = GetSystemMetrics(SM_CYCAPTION) + 3;
		if(hhbmp + 6 > hStart) hStart = hhbmp + 6;
		if(hStart > 80) hStart = 80;
	}else
	{
		wStart = whbmp;
		hStart = hhbmp / 3;
	}
	SetWindowPos(hwndStart, NULL, 0, 0,
		wStart, hStart,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}

/*--------------------------------------------------
　MSTaskSwWClassの位置・サイズの設定
----------------------------------------------------*/
void SetTaskWinPos(void)
{
	RECT rcBar, rcTask, rcTray;
	POINT pt;
	int x, y, w, h;

	GetClientRect(GetParent(hwndStart), &rcBar);  // Shell_TrayWnd
	GetWindowRect(hwndTray, &rcTray); // TrayNotifyWnd
	GetWindowRect(hwndTask, &rcTask);             // MSTaskSwWClass
	
	// MSTaskSwWClassの右上位置
	pt.x = rcTask.left; pt.y = rcTask.top;
	ScreenToClient(GetParent(hwndStart), &pt);
	
	x = pt.x; y = pt.y;
	w = rcTask.right - rcTask.left;
	h = rcTask.bottom - rcTask.top;
	
	// タスクバーが横置きのとき
	if(rcBar.right > rcBar.bottom)
	{
		if(bHiddenGrippers && !bWinXP)
		{
			x = wStart - 2;
			w = rcTray.left - 2 - wStart + 6; // 横幅
		}
		else
		{
			x = 2 + wStart; // 右位置
			w = rcTray.left - 2 - wStart - 2; // 横幅
		}
		
		if(wStart > 0)
		{
			x += 2; w -= 2;
		}
	}
	else // 縦置きのとき
	{
		y = 2 + hStart;
		h = rcTray.top - 2 - hStart - 2;
		if(hStart > 0)
		{
			y += 1; h -= 2;
		}
	}
	SetWindowPos(hwndTask, NULL, x, y, w, h,
		SWP_NOZORDER|SWP_NOACTIVATE);
}

/*--------------------------------------------------
   draw "flat start button"
----------------------------------------------------*/
void OnPaint(HWND hwnd, HDC hdc)
{
	HDC hdcMem1, hdcMem2;
	HBITMAP hbmp, hbmpTemp;
	HBRUSH hbr;
	BITMAP bmp;
	RECT rc;
	int x, y, w, h;
	BOOL bPushed;
	
	if(bStartMenuAtClock)
	{
		bPushed = FALSE;
		bStartMenuAtClock = FALSE;
	}
	else
	{
		bPushed = (SendMessage(hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED)?1:0;
	}
	
	hdcMem1 = CreateCompatibleDC(hdc);
	if(bWinXP) //hbmpstart != NULL)
		hbmp = hbmpstart;
	else
		hbmp = (HBITMAP)SendMessage(hwnd, BM_GETIMAGE, IMAGE_BITMAP, 0);

	SelectObject(hdcMem1, hbmp);
	GetObject(hbmp, sizeof(BITMAP), (LPVOID)&bmp);
	w = bmp.bmWidth; h = bmp.bmHeight;
	
	hdcMem2 = CreateCompatibleDC(hdc);
	GetClientRect(hwnd, &rc);
	if(!bSkinnedStartButton)
		hbmpTemp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	else
		hbmpTemp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom * 3);

	SelectObject(hdcMem2, hbmpTemp);
	
	hbr = CreateSolidBrush(CoolStartColorFace);

	FillRect(hdcMem2, &rc, hbr);
	if(bSkinnedStartButton) FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	
	if(!bSkinnedStartButton)
	{
		x = (rc.right - w)/2 + (!bCustStartButton ? 2:0) + (int)bPushed;
		y = (rc.bottom - h)/2 + (int)bPushed;
	}else
	{
		x = 0; 
		y = 0;
	}

	BitBlt(hdcMem2, x, y, w, h, hdcMem1, 0, 0, SRCCOPY);

	if(!bSkinnedStartButton)
	{
		if(bPushed || bCursorOnStartButton || bCoolStartButton || bNormalStartButton) // draw frame
		{
			HPEN hpen, hpenold;
			int color;
			
			color = (bPushed?CoolStartColorLow:CoolStartColorHigh);

			hpen = CreatePen(PS_SOLID, 1, color);
			hpenold = SelectObject(hdcMem2, hpen);
			MoveToEx(hdcMem2, 0, 0, NULL);
			LineTo(hdcMem2, rc.right, 0);
			MoveToEx(hdcMem2, 0, 0, NULL);
			LineTo(hdcMem2, 0, rc.bottom);
			SelectObject(hdcMem2, hpenold);
			DeleteObject(hpen);
			

			color = (bPushed?CoolStartColorHigh:CoolStartColorLow);

			hpen = CreatePen(PS_SOLID, 1, color);
			hpenold = SelectObject(hdcMem2, hpen);
			MoveToEx(hdcMem2, rc.right-1, 0, NULL);
			LineTo(hdcMem2, rc.right-1, rc.bottom);
			MoveToEx(hdcMem2, 0, rc.bottom-1, NULL);
			LineTo(hdcMem2, rc.right, rc.bottom-1);
			SelectObject(hdcMem2, hpenold);
			DeleteObject(hpen);


			color = (bPushed?CoolStartColorLow2:CoolStartColorHigh2);
			
			hpen = CreatePen(PS_SOLID, 1, color);
			hpenold = SelectObject(hdcMem2, hpen);
			MoveToEx(hdcMem2, 1, 1, NULL);
			LineTo(hdcMem2, rc.right - 1, 1);
			MoveToEx(hdcMem2, 1, 1, NULL);
			LineTo(hdcMem2, 1, rc.bottom - 1);
			SelectObject(hdcMem2, hpenold);
			DeleteObject(hpen);

			color = (bPushed?CoolStartColorHigh2:CoolStartColorLow2);

			hpen = CreatePen(PS_SOLID, 1, color);
			hpenold = SelectObject(hdcMem2, hpen);
			MoveToEx(hdcMem2, rc.right-2, 1, NULL);
			LineTo(hdcMem2, rc.right-2, rc.bottom-1);
			MoveToEx(hdcMem2, 1, rc.bottom-2, NULL);
			LineTo(hdcMem2, rc.right-1, rc.bottom-2);
			SelectObject(hdcMem2, hpenold);
			DeleteObject(hpen);
		}

		BitBlt(hdc, 0, 0,
			rc.right, rc.bottom, hdcMem2, 0, 0, SRCCOPY);
	}else
	{
		if(bPushed)
		{
			TC2DrawBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem2, 0, (h / 3) * 2, w, (h / 3), bStartButtonUseTrans);
		}else if(!bPushed && bCursorOnStartButton)
		{
			TC2DrawBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem2, 0, (h / 3), w, (h / 3), bStartButtonUseTrans);
		}else
		{
			TC2DrawBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem2, 0, 0, w, (h / 3), bStartButtonUseTrans);
		}
	}

	DeleteDC(hdcMem1);
	DeleteDC(hdcMem2);
	DeleteObject(hbmpTemp);

	//if(!bPushed && !bCursorOnStartButton)
		//SendMessage(GetParent(hwnd), WM_PAINT, 0, 0);
}

/*--------------------------------------------------
   called when clock window receive WM_TIMER.
   check cursor position, and draw "flat start button"
----------------------------------------------------*/
void CheckCursorOnStartButton(void)
{
	POINT pt;
	RECT rc;
	
	if(hwndStart == NULL) return;
	if(!bStartButtonFlat && !bSkinnedStartButton) return;
	
	GetCursorPos(&pt);
	GetWindowRect(hwndStart, &rc);
	if(PtInRect(&rc, pt))
	{
		if(!bCursorOnStartButton)
		{
			bCursorOnStartButton = TRUE;
			InvalidateRect(hwndStart, NULL, FALSE);
		}
	}
	else
	{
		if(bCursorOnStartButton)
		{
			bCursorOnStartButton = FALSE;
			InvalidateRect(hwndStart, NULL, FALSE);
		}
	}
}

void SetRebarGrippers(void)
{
	COLORSCHEME colScheme;

	colScheme.dwSize = sizeof(colScheme);
	colScheme.clrBtnHighlight = GetSysColor(COLOR_3DFACE);
	colScheme.clrBtnShadow = GetSysColor(COLOR_3DFACE);
	
	/*if(bCustomColorScheme)
	{
		SendMessage(hwndTask, RB_SETBKCOLOR, 0, (LPARAM)(COLORREF)ColorRebarFace);
		if(bWinXP)
		{
			HWND tHwnd;

			tHwnd = FindWindowEx(hwndTask, NULL, "MSTaskSwWClass", "Running Applications");
			if(tHwnd)
			{
				HBRUSH hbr;
				RECT prc;

				hbr = CreateSolidBrush(ColorRebarFace);
				GetClientRect(tHwnd, &prc);
				FillRect(GetDC(tHwnd), &prc, hbr);
				InvalidateRect(tHwnd, &prc, TRUE);
				DeleteObject(hbr);

				//SetBkColor(GetDC(tHwnd), ColorRebarFace);
				//WriteDebug("Found MSTaskSwWClass");
				//if(GetDC(tHwnd))
					//WriteDebug("Found MSTaskSwWClass hdc");

			}
		}
	}*/

	SendMessage(hwndTask, RB_SETCOLORSCHEME, 0, (LPARAM) (LPCOLORSCHEME)&colScheme);
	InvalidateRect(hwndTask, NULL, TRUE);

}


