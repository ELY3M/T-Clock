/*-------------------------------------------
  traynotify.c
    Customize the tray in taskbar
    Kazubon 2001
---------------------------------------------*/
#include "tcdll.h"

#ifndef TBSTYLE_TRANSPARENT
#define TBSTYLE_TRANSPARENT     0x8000
#endif

LRESULT CALLBACK WndProcTrayNotify(HWND, UINT, WPARAM, LPARAM);
void InitTrayNotify(HWND hwnd);
void EndTrayNotify(void);
BOOL bFillTray = FALSE;
BOOL bSkinTray = FALSE;
BOOL bTileTray = FALSE;
BOOL bFillClock = FALSE;
char TrayDrawStyle[1024];
static HWND hwndTrayNotify = NULL, hwndToolbar = NULL;
static WNDPROC oldWndProcTrayNotify = NULL;
static LONG oldClassStyleTrayNotify;
static LONG oldStyleTrayNotify;
static LONG oldExStyleTrayNotify;
BOOL bFlatTray = FALSE;
static HWND s_hwndClock;
extern COLORREF colback, colback2, colfore;
extern bWinXP;

/*--------------------------------------------------
  initialize
----------------------------------------------------*/
void InitTrayNotify(HWND hwndClock)
{
	HWND hwnd, hwnd2;
	char classname[80];
	char FillType[1024]; 
	//char FillStyle[1024];
	//RECT rcClock, rcTray;

	EndTrayNotify();
	
	GetMyRegStr("Clock", "ClockFillStyle", FillType, 1024, "FILLCLOCK");
	if(_strnicmp(FillType, "SKINCLOCK", 9) == 0)
	{
		bSkinTray = TRUE;
		bFillTray = FALSE;
		bFillClock = FALSE;
	}else if(_strnicmp(FillType, "FILLTRAY", 8) == 0)
	{
		bFillTray = TRUE;
		bSkinTray = FALSE;
		bFillClock = FALSE;
	}else if(_strnicmp(FillType, "SKINTRAY", 8) == 0)
	{
		bFillTray = TRUE;
		bSkinTray = TRUE;
		bFillClock = FALSE;
	}else if(_strnicmp(FillType, "FILLCLOCK", 9) == 0)
	{
		bFillTray = FALSE;
		bSkinTray = FALSE;
		bFillClock = TRUE;
	}else
	{
		bFillTray = FALSE;
		bSkinTray = FALSE;
		bFillClock = FALSE;
	}


	bFlatTray = GetMyRegLong("Taskbar", "FlatTray", FALSE);

	if(!bFillTray && !bFlatTray && !bSkinTray) return;
	
	/*GetMyRegStr("Clock", "ClockSkinDrawStyle", FillStyle, 1024, "STRETCH");
	if(_strnicmp(FillStyle, "TILE", 4) == 0)
	{
		bTileTray = TRUE;
	}else
	{*/
		//bTileTray = FALSE;
	//}

	// get window handle of TrayNotifyWnd
	hwndTrayNotify = GetParent(hwndClock);  // TrayNotifyWnd
	
	// search toolbar
	if(bFillTray)
	{
		hwndToolbar = NULL;
		hwnd = GetWindow(hwndTrayNotify, GW_CHILD);
		while(hwnd)
		{
			GetClassName(hwnd, classname, 80);
			if(lstrcmpi(classname, "ToolbarWindow32") == 0)
			{
				hwndToolbar = hwnd;
				break;
			}else if(lstrcmpi(classname, "SysPager") == 0)
			{
				hwnd2 = GetWindow(hwnd, GW_CHILD);
				while(hwnd2)
				{
					GetClassName(hwnd2, classname, 80);
					if(lstrcmpi(classname, "ToolbarWindow32") == 0)
					{
						hwndToolbar = hwnd2;
						break;
					}
				}
				hwnd2 = GetWindow(hwnd2, GW_HWNDNEXT);

			}
			hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		}
		if(hwndToolbar == NULL)
		{
			bFillTray = FALSE;
		}
	}

	s_hwndClock = hwndClock;

	if(bFillTray)
	{
		
		oldClassStyleTrayNotify = GetClassLong(hwndTrayNotify, GCL_STYLE);
		SetClassLong(hwndTrayNotify, GCL_STYLE,
			oldClassStyleTrayNotify|CS_HREDRAW|CS_VREDRAW);
		
		oldWndProcTrayNotify = 
			(WNDPROC)GetWindowLong(hwndTrayNotify, GWL_WNDPROC);
		SetWindowLong(hwndTrayNotify, GWL_WNDPROC, (LONG)WndProcTrayNotify);
		
		oldStyleTrayNotify = GetWindowLong(hwndTrayNotify, GWL_STYLE);
		SetWindowLongA(hwndTrayNotify, GWL_STYLE,
			oldStyleTrayNotify & ~(WS_CLIPCHILDREN|WS_CLIPSIBLINGS));

		SetClassLong(hwndToolbar, GCL_STYLE,
			GetClassLong(hwndToolbar, GCL_STYLE) &~TBSTYLE_TRANSPARENT);

		
	}
	
	if(bFlatTray)
	{
		oldExStyleTrayNotify = GetWindowLong(hwndTrayNotify, GWL_EXSTYLE);
		SetWindowLong(hwndTrayNotify, GWL_EXSTYLE,
			oldExStyleTrayNotify & ~WS_EX_STATICEDGE);
		SetWindowPos(hwndTrayNotify, NULL, 0, 0, 0, 0,
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
		/*SetWindowPos(GetParent(hwndToolbar), NULL, 2, 2, 0, 0,
			//SWP_FRAMECHANGED|SWP_NOSIZE|SWP_NOZORDER);

		GetClientRect(hwndTrayNotify, &rcTray);
		//OffsetRect(&rcTray, 2, 2);
		GetClientRect(hwndClock, &rcClock);
		SetWindowPos(hwndClock, NULL, 0, 0, rcClock.right, rcTray.bottom,
			SWP_NOMOVE|SWP_NOZORDER);*/
	}
	
	if(bFillTray || bFlatTray)
		InvalidateRect(hwndTrayNotify, NULL, TRUE);
	if(bFillTray)
	{
		if(!bWinXP)
			SendMessage(hwndToolbar, WM_SYSCOLORCHANGE, 0, 0);
		else
			SendMessage(GetParent(hwndToolbar), WM_SYSCOLORCHANGE, 0, 0);
	}
}

/*--------------------------------------------------
  undo
----------------------------------------------------*/
void EndTrayNotify(void)
{

	if(bFillTray && hwndTrayNotify && IsWindow(hwndTrayNotify))
	{
		SetWindowLong(hwndTrayNotify, GWL_STYLE, oldStyleTrayNotify);
		if(oldWndProcTrayNotify)
			SetWindowLong(hwndTrayNotify, GWL_WNDPROC,
				(LONG)oldWndProcTrayNotify);
		
		SetClassLong(hwndTrayNotify, GCL_STYLE, oldClassStyleTrayNotify);
		
		InvalidateRect(hwndTrayNotify, NULL, TRUE);
		SendMessage(hwndToolbar, WM_SYSCOLORCHANGE, 0, 0);
		InvalidateRect(hwndToolbar, NULL, TRUE);
	}
	
	if(bFlatTray && hwndTrayNotify && IsWindow(hwndTrayNotify))
	{
		SetWindowLong(hwndTrayNotify, GWL_EXSTYLE, oldExStyleTrayNotify);
		SetWindowPos(hwndTrayNotify, NULL, 0, 0, 0, 0,
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
	}
	
	hwndTrayNotify = NULL;
	hwndToolbar = NULL;
	oldWndProcTrayNotify = NULL;
	bSkinTray = bFillTray = bFlatTray = bFillClock = FALSE;
}

extern HDC hdcClock;

/*------------------------------------------------
   subclass procedure of TrayNotifyWnd
--------------------------------------------------*/
LRESULT CALLBACK WndProcTrayNotify(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		
	
		/*case WM_NCPAINT:
		{
			RECT rc;
			HDC hdc;
			HBRUSH hbr;

			if(bNoClock) break;
			//CallWindowProc(oldWndProcTrayNotify, hwnd, message, wParam, lParam);

			//GetClientRect(hwnd, &rc);
			//if(bFillTray && !bFlatTray)
				//GetClientRect(hwnd, &rc);
				//hdc = (HDC)wParam;
			//else
			//{
				GetWindowRect(hwnd, &rc);
				hdc = GetWindowDC(hwnd);
			//}
			hbr = CreateSolidBrush(COLOR_3DFACE);

			FrameRect(hdc, &rc, hbr);
			DeleteObject(hbr);

			return 0;
		}*/

			//return 0;
		case WM_ERASEBKGND:
		{
			RECT rc;
			HDC hdc;
			if(bNoClock) break;
			//GetClientRect(hwnd, &rc);
			//if(bFillTray && !bFlatTray)
				GetClientRect(hwnd, &rc);
				hdc = (HDC)wParam;
			//else
			//{
				//GetWindowRect(hwnd, &rc);
				//hdc = GetWindowDC(hwnd);
			//}

			FillClock(hwnd, hdc, &rc, 0);
			return 0;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			if(bNoClock) break;
			hdc = BeginPaint(hwnd, &ps);
			//if(bFillTray && !bFlatTray)
				GetClientRect(hwnd, &rc);
			//else
			//{
				//GetWindowRect(hwnd, &rc);
				//hdc = GetWindowDC(hwnd);
			//}
			FillClock(hwnd, hdc, &rc, 0);
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_SIZE:
			if(bNoClock) break;
			SendMessage(s_hwndClock, WM_SIZE, 0, 0);
			break;
		case WM_NOTIFY:
		{
			LPNMHDR pnmh;
			if(bNoClock) break;
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == NM_CUSTOMDRAW && pnmh->idFrom == 0)
			{
				LPNMTBCUSTOMDRAW pnmcd;
				LPNMCUSTOMDRAW nmcd;
				pnmcd = (LPNMTBCUSTOMDRAW)lParam;
				nmcd = (LPNMCUSTOMDRAW)&pnmcd->nmcd;
				if (nmcd->dwDrawStage  == CDDS_ITEMPREPAINT
					&& hdcClock != NULL)
				{
					POINT ptTray, ptToolbar;
					int x, y;
					RECT rc;
					ptTray.x = ptTray.y = 0;
					ClientToScreen(hwnd, &ptTray);
					ptToolbar.x = ptToolbar.y = 0;
					ClientToScreen(pnmh->hwndFrom, &ptToolbar);
					x = ptToolbar.x - ptTray.x;
					y = ptToolbar.y - ptTray.y;
					GetWindowRect(pnmh->hwndFrom, &rc);
					rc.right = rc.right - rc.left;
					rc.bottom = rc.bottom - rc.top;
					BitBlt(nmcd->hdc, nmcd->rc.left, nmcd->rc.top,
						rc.right, rc.bottom, hdcClock, 
						x + nmcd->rc.left, y + nmcd->rc.top,
						SRCCOPY);

					/*BitBlt(nmcd->hdc, nmcd->rc.left, nmcd->rc.top,
						nmcd->rc.right - nmcd->rc.left + 2,
						nmcd->rc.bottom - nmcd->rc.top,
						hdcClock, x + nmcd->rc.left, y + nmcd->rc.top,
						SRCCOPY);*/
					if(bWinXP)
					{
						BitBlt(GetDC(GetParent(hwndToolbar)), nmcd->rc.left, nmcd->rc.top,
							rc.right, rc.bottom, hdcClock, 
							x + nmcd->rc.left, y + nmcd->rc.top,
							SRCCOPY);

						/*BitBlt(GetDC(GetParent(hwndToolbar)), nmcd->rc.left, nmcd->rc.top,
							nmcd->rc.right - nmcd->rc.left,
							nmcd->rc.bottom - nmcd->rc.top,
							hdcClock, x + nmcd->rc.left, y + nmcd->rc.top,
							SRCCOPY);*/
						//WriteDebug("WM_Notify Tray");
					}
				}
			}
			break;
		}
	}
	return CallWindowProc(oldWndProcTrayNotify, hwnd, message, wParam, lParam);
}
