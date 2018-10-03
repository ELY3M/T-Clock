/*-------------------------------------------
  startmenu.c
    customize start menu
    Kazubon 1997-1999
---------------------------------------------*/

#include "tcdll.h"

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW   0x00020000
#endif

#ifndef TB_GETIMAGELIST
#define TB_GETIMAGELIST         (WM_USER + 49)
#endif

static void OnDrawItem(HWND hwnd, DRAWITEMSTRUCT* pdis);
static BOOL IsStartMenu(HMENU hmenu);
void DeleteStartMenuRes(void);
void DrawTaskbarEdge(HWND hwnd);
HWND GetStartmenu98(void);
HWND GetStartmenu95(void);
BOOL IsStartmenuIconLarge(void);

/*------------------------------------------------
　Globals
--------------------------------------------------*/
static BOOL bStartMenu = FALSE;
static HWND hwndBar = NULL;           // タスクバーのウィンドウハンドル
static WNDPROC oldWndProcBar = NULL;  // ウィンドウプロシージャを保存
static HDC hdcMemMenu = NULL;         // メニュー描画用メモリDC
static HBITMAP hbmpMenu = NULL;       // メニュー描画用ビットマップ
static HDC hdcMemMenuLeft = NULL;     // 「Windows95」部分用メモリDC
static HBITMAP hbmpMenuLeft = NULL;   // 「Windows95」部分用ビットマップ
static int hStartMenu = 0;            // メニューの高さを保存
static COLORREF colMenuLeft;          // 色
static BOOL bTile = FALSE;     // 並べる
static BOOL bStretch = FALSE;     // 並べる
static BOOL bEdgeScale = FALSE;     // 並べる
static BOOL bStartMenuUseTrans = FALSE;

char StartMenuDrawStyle[1024];
char TaskbarEdgeStyle[1024];

static int hEdgeTop = 0;
static int hEdgeBottom = 0;

static LONG oldbarstyle;
static BOOL bFlatTaskbar = FALSE;
static BOOL bDropShadow = FALSE;
extern BOOL bWinXP;
extern BOOL bWin2000;
extern BOOL bWin98;
extern BOOL bStartMenuDestroyed;

static HWND hwndMenuSite = NULL;
static HWND hwndSysPager = NULL;
static BOOL bLargeIcons = FALSE;
static BOOL bLargeIconsNoBanner = FALSE;
static BOOL bSmallIconsShowBanner = FALSE;

//extern BOOL bCustomColorScheme;

//COLORREF ColorTaskbarFace;

//extern HWND hwndTab;

/*------------------------------------------------
　タスクバーのサブクラスプロシージャ
--------------------------------------------------*/
LRESULT CALLBACK WndProcBar(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		// メニューのオーナードロー
		case WM_SETTINGCHANGE:
			{
				if(wParam == SPI_SETWORKAREA)
					return 0;
			}
			break;
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT* pdis;
			
			if(!bStartMenu) break;
			pdis = (DRAWITEMSTRUCT*)lParam;
			// スタートメニューでなかったらそのまま
			if(wParam || !IsStartMenu((HMENU)pdis->hwndItem)) break;
			OnDrawItem(hwnd, pdis);
			return 0;
		}
		// システム色変更
		case WM_SYSCOLORCHANGE:
			if(hdcMemMenu) DeleteDC(hdcMemMenu); hdcMemMenu = NULL;
			if(hbmpMenu) DeleteObject(hbmpMenu); hbmpMenu = NULL;
			break;
		// v1.7  タスクバーの表示が乱れるのを防ぐ
		case WM_EXITSIZEMOVE:
			PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			break;
		
		case WM_DESTROY:
			DeleteStartMenuRes();
			break;

		case WM_NCPAINT:
		{
			if(!bFlatTaskbar) break;
			
			CallWindowProc(oldWndProcBar, hwnd, message, wParam, lParam);

			DrawTaskbarEdge(hwnd);
			return 0;
			/*{
				HDC hdc;
				RECT rtc;
				HBRUSH hbr;
				CallWindowProc(oldWndProcBar, hwnd, message, wParam, lParam);
				//hdc = GetDCEx(hwnd, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
				hdc = GetWindowDC(hwnd);
				// Paint into this DC
				hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
				GetWindowRect(hwnd, &rtc);
				rtc.top = 0;
				rtc.left = 0;
				rtc.bottom = rtc.bottom + 1;
				rtc.right = rtc.right + 1;
				FrameRect(hdc, &rtc, hbr);
				rtc.top = 1;
				rtc.left = 1;
				rtc.bottom = rtc.bottom + 1;
				rtc.right = rtc.right + 1;
				FrameRect(hdc, &rtc, hbr);
				//ExtFloodFill(hdc, rtc.bottom - 2, 0, GetSysColor(COLOR_3DDKSHADOW), FLOODFILLBORDER);
				DeleteObject(hbr);
				ReleaseDC(hwnd, hdc);
				return 0;
			}*/
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			HBRUSH hbr;
			RECT prc;
			
			if(bWinXP) break;

			hdc = BeginPaint(hwnd, &ps);
			hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
			GetClientRect(hwnd, &prc);
			FillRect(hdc, &prc, hbr);
			DeleteObject(hbr);
			EndPaint(hwnd, &ps);

			return 0;
		}

	}
	return CallWindowProc(oldWndProcBar, hwnd, message, wParam, lParam);
}

/*------------------------------------------------
　メニューのオーナードロー
--------------------------------------------------*/
void OnDrawItem(HWND hwnd, DRAWITEMSTRUCT* pdis)
{
	HDC hdc;
	RECT rcBox, rcItem;
	HBRUSH hbr;
	BITMAP bmp;

	hdc = pdis->hDC;
	CopyRect(&rcItem, &(pdis->rcItem));
	GetClipBox(hdc, &rcBox); // メニュー全体の大きさ

	// 描画用メモリDCとビットマップの作成
	if(hdcMemMenu == NULL ||
		(hStartMenu != rcBox.bottom && rcBox.left == 0))
	{
		if(hdcMemMenu) DeleteDC(hdcMemMenu);
		if(hbmpMenu) DeleteObject(hbmpMenu);
				
		hdcMemMenu = CreateCompatibleDC(hdc);
		hbmpMenu = CreateCompatibleBitmap(hdc, rcBox.right, rcBox.bottom);
		SelectObject(hdcMemMenu, hbmpMenu);
		hbr = CreateSolidBrush(GetSysColor(COLOR_MENU));
		FillRect(hdcMemMenu, &rcBox, hbr);
		hStartMenu = rcBox.bottom;
	}
	
	SelectObject(hdcMemMenu, (HFONT)GetCurrentObject(hdc, OBJ_FONT));
	
	// 背景色、文字色の設定
	if(pdis->itemState & ODS_FOCUS)
	{
		SetTextColor(hdcMemMenu, GetSysColor(COLOR_HIGHLIGHT));
		SetBkColor(hdcMemMenu, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		SetTextColor(hdcMemMenu, GetSysColor(COLOR_MENUTEXT));
		SetBkColor(hdcMemMenu, GetSysColor(COLOR_MENU));
	}
	
	// メモリDCにデフォルトの描画をさせる
	pdis->hDC = hdcMemMenu;
	CallWindowProc(oldWndProcBar, hwnd, WM_DRAWITEM, 0, (LPARAM)pdis);
	
	// 「Windows95」の幅が、pdis->rcItem.leftに入ってくる
	rcItem.right = pdis->rcItem.left;
	
	if(rcItem.right > 0)
	{
		COLORREF col;
		
		if(!bTile)
		{
			//「色」で塗りつぶし
			col = colMenuLeft;
			if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
			hbr = CreateSolidBrush(col);
			FillRect(hdcMemMenu, &rcItem, hbr);
			DeleteObject(hbr);
		}
		
		if(hbmpMenuLeft)
			GetObject(hbmpMenuLeft, sizeof(BITMAP), &bmp);
		
		if(hbmpMenuLeft) 
		{
			int i, j;
			// ビットマップ描画
			for(i = 0; ; i++)
			{
				int y, ysrc, h, x, w;
				for(j = 0; ; j++)
				{
					y = rcBox.bottom - ((i + 1) * bmp.bmHeight);
					ysrc = 0;
					h = bmp.bmHeight;
					if(y < 0)
					{
						y = 0;
						ysrc = ((i + 1) * bmp.bmHeight) - rcBox.bottom;
						h -= ysrc;
					}
					x = j * bmp.bmWidth; w = bmp.bmWidth;
					if(x + w > rcItem.right)
					{
						w -= ((j + 1) * bmp.bmWidth) - rcItem.right;
					}
					if(w > 0 && h > 0)
						BitBlt(hdcMemMenu, x, y, w, h,
							hdcMemMenuLeft, 0, ysrc, SRCCOPY);
					if(!bTile || w < bmp.bmWidth) break;
				}
				if(!bTile || y == 0) break;
			}
		}
	}

	// 本当のDCにまとめて描画
	BitBlt(hdc, 0, rcItem.top,
		pdis->rcItem.right, rcItem.bottom - rcItem.top,
		hdcMemMenu, 0, rcItem.top, SRCCOPY);
	pdis->hDC = hdc;
}

/*--------------------------------------------------
　スタートメニュー改造の初期化
----------------------------------------------------*/
void SetStartMenu(HWND hwndClock)
{
	HWND hwndTray;
	
	char fname[1024];
	
	EndStartMenu();
	
	// タスクバーのサブクラス化（v1.7より必ずサブクラス化）
	hwndTray = GetParent(hwndClock); // TrayNotifyWnd
	if(hwndTray == NULL)
		return;
	hwndBar = GetParent(hwndTray);   // Shell_TrayWnd
	if(hwndBar == NULL)
		return;
	oldWndProcBar = (WNDPROC)GetWindowLong(hwndBar, GWL_WNDPROC);
	SetWindowLongA(hwndBar, GWL_WNDPROC, (LONG)WndProcBar);
	
	bStartMenu = GetMyRegLong("StartMenu", "CustomizeStartMenu", FALSE);

	//bFlatTaskbar = GetMyRegLong("Taskbar", "FlatTaskbar", FALSE);
	GetMyRegStr("Taskbar", "TaskbarEdgeStyle", TaskbarEdgeStyle, 1024, "Normal");
	if(_strnicmp(TaskbarEdgeStyle, "FLAT", 4) == 0)
	{
		bFlatTaskbar = TRUE;
		SendMessage(hwndBar, WM_NCPAINT, 1, 0);
	}
	else
	{
		bFlatTaskbar = FALSE;
		SendMessage(hwndBar, WM_NCPAINT, 1, 0);
	}

	if(!bStartMenu && !bFlatTaskbar) return;
	
	GetMyRegStr("StartMenu", "StartMenuDrawStyle", StartMenuDrawStyle, 1024, "Color");
	if(_strnicmp(StartMenuDrawStyle, "STRETCH", 7) == 0)
	{
		bStretch = TRUE;
	}
	if(_strnicmp(StartMenuDrawStyle, "TILE", 4) == 0)
	{
		bTile = TRUE;
	}
	else
	{
		colMenuLeft = GetMyRegColor("StartMenu", "StartMenuCol", GetSysColor(COLOR_HIGHLIGHT));
	}

	bEdgeScale = GetMyRegLong("StartMenu", "StartMenuEdgeScaling", FALSE);
	if(bEdgeScale)
	{
		hEdgeTop = (int)(short)GetMyRegLong("StartMenu", "StartMenuEdgeTop", 1);
		if(hEdgeTop < 1) hEdgeTop = 1;
		hEdgeBottom = (int)(short)GetMyRegLong("StartMenu", "StartMenuEdgeBottom", 1);
		if(hEdgeBottom < 1) hEdgeBottom = 1;
	}

	bStartMenuUseTrans = GetMyRegLong("StartMenu", "StartMenuUseTrans", FALSE);

	GetMyRegStr("StartMenu", "StartMenuBmp", fname, 1024, "");
	
	if(fname[0]) // 「Windows95」部分用のメモリDCとビットマップ作成
	{
		hbmpMenuLeft = ReadBitmap(hwndBar, fname, FALSE);
		if(hbmpMenuLeft)
		{
			HDC hdc;
			hdc = GetDC(hwndBar);
			hdcMemMenuLeft = CreateCompatibleDC(hdc);
			SelectObject(hdcMemMenuLeft, hbmpMenuLeft);
			ReleaseDC(hwndBar, hdc);
		}
	}

	bDropShadow = GetMyRegLong("Taskbar", "DropShadow", FALSE);
	if(bDropShadow)
	{
		oldbarstyle = GetClassLong(hwndBar, GCL_STYLE);
		SetClassLongA(hwndBar, GCL_STYLE, oldbarstyle|CS_SAVEBITS|CS_DROPSHADOW);
	}

	if(bWin98)
	{
		bLargeIconsNoBanner = GetMyRegLong("StartMenu", "LargeIconsNoBanner", FALSE);
		bSmallIconsShowBanner = GetMyRegLong("StartMenu", "SmallIconsShowBanner", FALSE);
	}
	//WriteDebug("Set Startmenu");

	//SetWindowPos(hwndBar, NULL, 0, 0, 800, 30, SWP_NOZORDER|SWP_NOACTIVATE);

	/*if(bCustomColorScheme)
	{
		ColorTaskbarFace = GetMyRegColor("CustomColor", "TaskbarFaceColor", GetSysColor(COLOR_3DFACE));
	}*/

}

/*--------------------------------------------------
　元に戻す
----------------------------------------------------*/
void DeleteStartMenuRes(void)
{
	if(hdcMemMenu) DeleteDC(hdcMemMenu); hdcMemMenu = NULL;
	if(hbmpMenu) DeleteObject(hbmpMenu); hbmpMenu = NULL;
	if(hdcMemMenuLeft) DeleteDC(hdcMemMenuLeft); hdcMemMenuLeft = NULL;
	if(hbmpMenuLeft) DeleteObject(hbmpMenuLeft); hbmpMenuLeft = NULL;
}
void EndStartMenu(void)
{
	if(hwndBar && IsWindow(hwndBar) && oldWndProcBar)
		SetWindowLong(hwndBar, GWL_WNDPROC, (LONG)oldWndProcBar);
	hwndBar = NULL; oldWndProcBar = NULL;
	
	if(bDropShadow)
		SetClassLong(hwndBar, GCL_STYLE, oldbarstyle);

	bStretch = bTile = bStartMenuUseTrans = FALSE;

	DeleteStartMenuRes();
}

/*--------------------------------------------------
　スタートメニューかどうか判別
----------------------------------------------------*/
BOOL IsStartMenu(HMENU hmenu)
{
	int i, count, id;
	
	count = GetMenuItemCount(hmenu);
	for(i = 0; i < count; i++)
	{
		id = GetMenuItemID(hmenu, i);
		// 「ヘルプ」があればスタートメニュー
		if(id == 503) return TRUE;
	}
	return FALSE;
}

// ------------------------------------------------------
// 以下、IE4用

LRESULT CALLBACK WndProcStartMenu(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);
HWND hwndStartMenu = NULL; // スタートメニューのウィンドウハンドル
WNDPROC oldWndProcStartMenu = NULL;
void OnPaintStartmenuIE4(HWND hwnd, HDC hdc, BOOL bPrint);

/*--------------------------------------------------
　スタートメニューウィンドウのサブクラス化
----------------------------------------------------*/
void InitStartMenuIE4(void)
{
	HWND hwnd; //, hwndChild;
	/*HWND hwndFound, hwndTemp;
	char classname[80];
	RECT rc1, rc2, rcTemp;
	
	// スタートメニューを探す
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	hwndFound = NULL;
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "BaseBar") == 0)
		{
			if(GetWindowThreadProcessId(hwnd, NULL) ==
				GetCurrentThreadId())
			{
				hwndChild = GetWindow(hwnd, GW_CHILD);
				///hwndMenuSite = GetWindow(hwnd, GW_CHILD);
				///hwndSysPager = FindWindowEx(hwndMenuSite, NULL, "SysPager", NULL);
				//hwndTemp = FindWindowEx(hwndMenuSite, NULL, "ToolbarWindow32", NULL);
				///if(hwndSysPager != NULL)
				///{
					///hwndFound = hwnd;
					///GetWindowRect(hwnd, &rc1);
					///GetWindowRect(hwndMenuSite, &rc2);
					///if(rc2.left - rc1.left >= 21)
						///bLargeIcons = TRUE;
					///break;
				///}
				//GetClientRect(hwnd, &rc1);
				//GetClientRect(hwndChild, &rc2);
				GetWindowRect(hwnd, &rc1);
				GetWindowRect(hwndChild, &rc2);
				//if(rc1.right - rc2.right == 21 || rc2.right == 0)			
				if(rc2.left - rc1.left >= 21) // || rc2.right == 0)
				{
					if(hwndFound == NULL
						|| (int)hwndFound > (int)hwnd)
						hwndFound = hwnd;
				}
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}

	hwnd = hwndFound;*/

	if(bWin98 || bWinXP)
		hwnd = GetStartmenu98();
	else
		hwnd = GetStartmenu95();

	//WriteDebug(hwnd);
	if(hwnd == NULL) 
	{
		bStartMenuDestroyed = TRUE;
		return;
	}

	// サブクラス化
	//WriteDebug("Found Startmenu");
	oldWndProcStartMenu = (WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC);
	SetWindowLongA(hwnd, GWL_WNDPROC, (LONG)WndProcStartMenu);
	hwndStartMenu = hwnd;
	//if(bWinXP || bWin2000)
	//{
		//GetWindowRect(hwndStartMenu, &rcTemp);
		//if(rcTemp.right - rcTemp.left > 0)
		//{
			//SendMessage(hwndStartMenu, WM_PRINT, 0, 0);
			//ShowWindow(hwndStartMenu, SW_HIDE);
			//ShowWindow(hwndStartMenu, SW_SHOW);
			//OnPaintStartmenuIE4(hwndStartMenu, GetDC(hwndStartMenu), FALSE);
			//WriteDebug("Repainting Startmenu");
		//}
		bStartMenuDestroyed = FALSE;
	//}
}

/*--------------------------------------------------
　スタートメニューウィンドウを元に戻す
----------------------------------------------------*/
void ClearStartMenuIE4(void)
{
	if(hwndStartMenu && IsWindow(hwndStartMenu) && oldWndProcStartMenu)
		SetWindowLong(hwndStartMenu, GWL_WNDPROC, (LONG)oldWndProcStartMenu);
	hwndStartMenu = NULL; oldWndProcStartMenu = NULL;
	hwndSysPager = NULL; hwndMenuSite = NULL;
	bStartMenuDestroyed = TRUE;
	bLargeIcons = FALSE;
}

/*------------------------------------------------
　スタートメニューのサブクラスプロシージャ
--------------------------------------------------*/
LRESULT CALLBACK WndProcStartMenu(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		// メニューの描画
		/*case WM_NCCALCSIZE:
		{
			BOOL fCalcValidRects = (BOOL) wParam;

			if (fCalcValidRects)
			{
				NCCALCSIZE_PARAMS *lpncsp = (NCCALCSIZE_PARAMS*)lParam;

				LONG res = CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);

				lpncsp->rgrc[0].right = lpncsp->rgrc[0].right - 21;
				lpncsp->lppos->cx =lpncsp->lppos->cx - 21;

				return res;
			}
			////if(bLargeIconsNoBanner && (BOOL)wParam)
			////{
				////RECT rc;
				////CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
				////GetClientRect(hwnd, &rc);
				////GetWindowRect(hwnd,&rc);
				////SetWindowPos(hwndMenuSite, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
				////SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right - 21, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED|SWP_NOMOVE|0x1000);

				////return 0;
			////}

		}*/
		case WM_WINDOWPOSCHANGING:
		{
			if(bWin98)
			{
				if(bLargeIconsNoBanner || bSmallIconsShowBanner)
				{
					//RECT rc, rc2;
					LRESULT res;
					LPWINDOWPOS lpwp = (LPWINDOWPOS)lParam;
					//lpwp = (LPWINDOWPOS) lParam;
					res = CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
					if(bLargeIconsNoBanner && IsStartmenuIconLarge())
						lpwp->cx = lpwp->cx - 21;
					else if(bSmallIconsShowBanner && !IsStartmenuIconLarge())
					{
						lpwp->cx = lpwp->cx + 21;
						//OnPaintStartmenuIE4(hwnd, (HDC)wParam, FALSE);
					}
					//GetClientRect(hwnd, &rc);
					//GetWindowRect(hwnd, &rc2);
					//SetWindowPos(hwndMenuSite, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
					//SetWindowPos(hwnd, NULL, lpwp->x, lpwp->y, lpwp->cx, lpwp->cy, lpwp->flags);

					return res;
					//break;
				}//else
				//{
					//OnPaintStartmenuIE4(hwnd, (HDC)wParam, FALSE);
				//WriteDebug("WM_WINDOWPOSCHANGED Startmenu");
			}
			break;
				//}
		}
		case WM_WINDOWPOSCHANGED:
		{
			if(bWin98)
			{
				if(bLargeIconsNoBanner || bSmallIconsShowBanner)
				{
					RECT rc, rc2, rcTB;
					LRESULT res;
					//LPWINDOWPOS lpwp; // = (LPWINDOWPOS)lParam;
					//lpwp = (LPWINDOWPOS) lParam;
					res = CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
					GetClientRect(hwnd, &rc);
					GetWindowRect(hwnd, &rc2);
					GetClientRect(FindWindowEx(hwndMenuSite, NULL, "ToolbarWindow32", NULL), &rcTB);
					if(bLargeIconsNoBanner && IsStartmenuIconLarge())
						SetWindowPos(hwndMenuSite, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
					else if(bSmallIconsShowBanner && !IsStartmenuIconLarge())
						SetWindowPos(hwndMenuSite, NULL, rc.left + 21, rc.top, rc.right, rc.bottom, SWP_NOCOPYBITS|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);

					//SetWindowPos(hwndMenuSite, NULL, rc.left, rc.top, rc.right, rc.bottom, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);

					//SetWindowPos(hwnd, NULL, lpwp->x - 21, lpwp->y, lpwp->cx, lpwp->cy, lpwp->flags);
					//SendMessage(FindWindowEx(hwndMenuSite, NULL, "ToolbarWindow32", NULL), WM_NCPAINT, 1, 0);
					//ShowWindow(hwnd, FALSE);
					//ShowWindow(hwnd, TRUE);
					return res;
					//break;
				}
			}
		}
		case WM_SYSCOLORCHANGE:
			OnPaintStartmenuIE4(hwnd, (HDC)wParam, FALSE);
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			if(!bStartMenu) break;
			hdc = BeginPaint(hwnd, &ps);
			OnPaintStartmenuIE4(hwnd, hdc, FALSE);
			EndPaint(hwnd, &ps);
			//WriteDebug("WM_PAINT Startmenu");
			//if(bLargeIconsNoBanner)
			//{
				//RECT rc;
				//CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
				//GetWindowRect(hwnd, &rc);
				//MoveWindow(hwnd, rc.left, rc.top, rc.right - 21, rc.bottom, FALSE);
				//SetWindowPos(hwnd, NULL, rc.left, rc.top, rc.right - 21, rc.bottom, SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED|SWP_NOMOVE|0x1000);
			//}

			return 0;
		}
		case WM_PRINT:
		{
			LRESULT r;
			if(!bStartMenu) break;
			r = CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
			OnPaintStartmenuIE4(hwnd, (HDC)wParam, TRUE);
			//WriteDebug("WM_PRINT Startmenu");
			return r;
		}
		case WM_DESTROY:
		{
			//if(bWinXP || bWin2000)
			//{
				//bStartMenuDestroyed = TRUE;
				ClearStartMenuIE4();
			//}
			break;
		}
		// システム色変更
	}
	return CallWindowProc(oldWndProcStartMenu, hwnd, message, wParam, lParam);
}

/*------------------------------------------------
　スタートメニューの描画
--------------------------------------------------*/
void OnPaintStartmenuIE4(HWND hwnd, HDC hdc, BOOL bPrint)
{
	RECT rc, rcWin, rcChild;
	POINT pt;
	COLORREF col;
	HBRUSH hbr;
	BITMAP bmp;
	int hClient, wClient;
	int sy, sh, dy;

	GetWindowRect(GetWindow(hwnd, GW_CHILD), &rcChild);
	GetWindowRect(hwnd, &rcWin);
	GetClientRect(hwnd, &rc);
	pt.x = 0; pt.y = 0;
	ClientToScreen(hwnd, &pt);
	//WriteDebug("OnPaint Startmenu");
	if(pt.x == rcChild.left) return;
	
	rc.right = 21;
	wClient = rc.right; hClient = rc.bottom;
	if(bPrint)
	{
		int dx, dy;
		dx = pt.x - rcWin.left; dy = pt.y - rcWin.top;
		rc.left += dx; rc.right += dx;
		rc.top += dy; rc.bottom += dy;
	}
	//「色」で塗りつぶし
	col = colMenuLeft;
	//if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	hbr = CreateSolidBrush(col);
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);
	if(hbmpMenuLeft)
	{
		//WriteDebug("Paint Startmenu");
		GetObject(hbmpMenuLeft, sizeof(BITMAP), &bmp);
		if(bTile || bStretch)
		{
			if(bEdgeScale)
			{
				sy = hEdgeTop;
				sh = (bmp.bmHeight - hEdgeTop) - hEdgeBottom;
				dy = rc.bottom - hEdgeBottom;
			}else if(!bEdgeScale)
			{
				sy = 0;
				sh = bmp.bmHeight;
				dy = rc.bottom;
			}

			if(bTile)
			{
				VerticalTileBlt(hdc, rc.left, 3, bmp.bmWidth, dy, hdcMemMenuLeft,
					0, sy, bmp.bmWidth, sh, TRUE, bStartMenuUseTrans);
			}else
			{
				TC2DrawBlt(hdc, rc.left, 3, bmp.bmWidth, dy, hdcMemMenuLeft,
					0, sy, bmp.bmWidth, sh, bStartMenuUseTrans);
			}

			if(bEdgeScale)
			{
				TC2DrawBlt(hdc, rc.left, rc.top, bmp.bmWidth, hEdgeTop, hdcMemMenuLeft,
					0, 0, bmp.bmWidth, hEdgeTop, bStartMenuUseTrans);
				TC2DrawBlt(hdc, rc.left, rc.bottom - hEdgeBottom, bmp.bmWidth, hEdgeBottom,
					hdcMemMenuLeft, 0, bmp.bmHeight - hEdgeBottom, bmp.bmWidth, hEdgeBottom,
					bStartMenuUseTrans);
			}
		}else
		{
			TC2DrawBlt(hdc, rc.top, rc.bottom - bmp.bmHeight, bmp.bmWidth, bmp.bmHeight,
				hdcMemMenuLeft, 0, 0, bmp.bmHeight, bmp.bmHeight, bStartMenuUseTrans);
		}

	}

}

void DrawTaskbarEdge(HWND hwnd)
{
	HDC hdc;
	RECT rtc;
	HBRUSH hbr;
	
	hdc = GetWindowDC(hwnd);

	//if(bFlatTaskbar && !bCustomColorScheme)
	//{
		hbr = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		GetWindowRect(hwnd, &rtc);
		rtc.top = 0;
		rtc.left = 0;
		rtc.bottom = rtc.bottom + 1;
		rtc.right = rtc.right + 1;
		FrameRect(hdc, &rtc, hbr);
		rtc.top = 1;
		rtc.left = 1;
		rtc.bottom = rtc.bottom + 1;
		rtc.right = rtc.right + 1;
		FrameRect(hdc, &rtc, hbr);
		DeleteObject(hbr);
		ReleaseDC(hwnd, hdc);
	/*}
	else if(bFlatTaskbar && bCustomColorScheme)
	{
		hbr = CreateSolidBrush(ColorTaskbarFace);
		GetWindowRect(hwnd, &rtc);
		rtc.top = 0;
		rtc.left = 0;
		rtc.bottom = rtc.bottom - 1;
		rtc.right = rtc.right - 1;
		FrameRect(hdc, &rtc, hbr);
		rtc.top = 1;
		rtc.left = 1;
		rtc.bottom = rtc.bottom + 1;
		rtc.right = rtc.right + 1;
		FrameRect(hdc, &rtc, hbr);
		rtc.top = 2;
		rtc.left = 2;
		rtc.bottom = rtc.bottom + 1;
		rtc.right = rtc.right + 1;
		FrameRect(hdc, &rtc, hbr);
		rtc.top = 3;
		rtc.left = 3;
		rtc.bottom = rtc.bottom + 1;
		rtc.right = rtc.right + 1;
		FrameRect(hdc, &rtc, hbr);
		DeleteObject(hbr);
		ReleaseDC(hwnd, hdc);

	}*/

}

HWND GetStartmenu98(void)
{
	HWND hwnd, hwndFound, hwndToolbar, hwndSyspager;
	char classname[80];
	RECT rc1, rc2;
	
	// スタートメニューを探す
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	hwndFound = NULL;
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "BaseBar") == 0)
		{
			if(GetWindowThreadProcessId(hwnd, NULL) ==
				GetCurrentThreadId())
			{
				hwndMenuSite = GetWindow(hwnd, GW_CHILD);
				hwndToolbar = FindWindowEx(hwndMenuSite, NULL, "ToolbarWindow32", NULL);
				if(hwndToolbar != NULL)
				{
					hwndSyspager = FindWindowEx(hwndMenuSite, NULL, "SysPager", NULL);
					if(hwndSyspager != NULL)
					{
						hwndFound = hwnd;
						GetWindowRect(hwnd, &rc1);
						GetWindowRect(hwndMenuSite, &rc2);
						if(rc2.left - rc1.left >= 21)
							bLargeIcons = TRUE;
						return hwndFound;
					}
				}
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	return NULL;
}

HWND GetStartmenu95(void)
{
	HWND hwnd, hwndChild, hwndFound;
	char classname[80];
	RECT rc1, rc2;
	
	// スタートメニューを探す
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	hwndFound = NULL;
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "BaseBar") == 0)
		{
			if(GetWindowThreadProcessId(hwnd, NULL) ==
				GetCurrentThreadId())
			{
				hwndChild = GetWindow(hwnd, GW_CHILD);
				GetWindowRect(hwnd, &rc1);
				GetWindowRect(hwndChild, &rc2);
				if(rc2.left - rc1.left >= 21)
				{
					if(hwndFound == NULL
						|| (int)hwndFound > (int)hwnd)
						hwndFound = hwnd;
				}
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	return hwndFound;
}

BOOL IsStartmenuIconLarge(void)
{
	HWND hwnd, hwndFound, hwndToolbar, hwndMenuSite2;
	char classname[80];
	//RECT rc1, rc2;
	HIMAGELIST hilList;
	int ix, iy, ii;

	// スタートメニューを探す
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	hwndFound = NULL;
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "BaseBar") == 0)
		{
			if(GetWindowThreadProcessId(hwnd, NULL) ==
				GetCurrentThreadId())
			{
				hwndMenuSite2 = GetWindow(hwnd, GW_CHILD);
				hwndToolbar = FindWindowEx(hwndMenuSite2, NULL, "ToolbarWindow32", NULL);
				if(hwndToolbar != NULL)
				{
					hilList = (HIMAGELIST)SendMessage(hwndToolbar, TB_GETIMAGELIST, 0, 0);
					ImageList_GetIconSize(hilList, &ix, &iy);
					ii = GetSystemMetrics(SM_CYICON);
					if(iy == ii)
						return TRUE;
					else
						return FALSE;

					//hwndFound = hwnd;
					//GetWindowRect(hwnd, &rc1);
					//GetWindowRect(hwndMenuSite, &rc2);
					//if(rc2.left - rc1.left >= 21)
						//return TRUE;
				}
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	return FALSE;
}
