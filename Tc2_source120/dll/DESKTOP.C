/*-------------------------------------------
  desktop.c
    Customize Desktop Icon View
    Two_toNe 2002
---------------------------------------------*/
#include "tcdll.h"

#ifndef LVS_SMALLICON
#define LVS_SMALLICON           0x0002
#endif

#ifndef LVS_ICON
#define LVS_ICON           0x0000
#endif

HWND hwndDesktop = NULL;
static LONG oldstyle;

//LRESULT CALLBACK WndProcDesktop(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcDesktop(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam);

static WNDPROC oldWndProcDesktop = NULL;

BOOL bSmallIcons = FALSE;
BOOL bLargeIcons = FALSE;
BOOL bExLargeIcons = FALSE;
BOOL bTransBK = FALSE;
BOOL bCustTextColor = FALSE;
COLORREF DeskTextColor, oldDeskTextColor;
void SetDesktopIconTextBk(BOOL bNoTrans);
void SetDesktopIconTextColor(BOOL bDefault);
void ClearDesktopIconText(void);




extern BOOL bWin2000;

char DesktopIconSize[1024];
static LONG DesktopIconStyle;

char AltRClickApp[1024];
BOOL bAltRClickApp = FALSE;
BOOL bAltRClickThroughIcons = FALSE;
BOOL DesktopIconNoText = FALSE;

BOOL bCallDefaultRClick = FALSE;
BOOL bIgnoreRClick = FALSE;

char caption[80];

/*--------------------------------------------------
   initialize
----------------------------------------------------*/
void SetDesktop(void)
{
	
	SHFILEINFO shfi;
	HIMAGELIST hilBigIcons;
	LVITEM lvItem;
	BOOL b;
	HWND hwnd;
	char caption[80];
	RECT r;

	EndDesktop();

	b = GetMyRegLong("Desktop", "CustomizeDesktop", FALSE);
	if(!b) return;

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

	hwndDesktop = hwnd;

	bTransBK = GetMyRegLong("Desktop", "DesktopTextBkTrans", FALSE);
	if(bTransBK) SetDesktopIconTextBk(TRUE);
	
	bCustTextColor = GetMyRegLong("Desktop", "CustomizeDesktopTextColor", FALSE);
	if(bCustTextColor)
	{
		oldDeskTextColor = ListView_GetTextColor(hwndDesktop);
		DeskTextColor = GetMyRegLong("Desktop", "DesktopTextColor", CLR_DEFAULT);
		SetDesktopIconTextColor(FALSE);
	}


	GetMyRegStr("Desktop", "DesktopIconSize", DesktopIconSize, 1024, "Normal");
	oldstyle = GetWindowLong(hwndDesktop, GWL_STYLE);
	if(_strnicmp(DesktopIconSize, "SMALL", 5) == 0)
	{
		bSmallIcons = TRUE;
		SetWindowLong(hwndDesktop, GWL_STYLE, oldstyle|LVS_SMALLICON);
	}
	else
	{
		bSmallIcons = FALSE;	
	}

	DesktopIconNoText = GetMyRegLong("Desktop", "NoDesktopIconText", FALSE);
	if(DesktopIconNoText)
		ClearDesktopIconText();

	oldWndProcDesktop = (WNDPROC)GetWindowLong(hwndDesktop, GWL_WNDPROC);
	SetWindowLongA(hwndDesktop, GWL_WNDPROC, (LONG)WndProcDesktop);

	bAltRClickApp = GetMyRegLong("Desktop", "AltDesktopRightClick", FALSE);
	if(bAltRClickApp)
	{
		if(GetMyRegStr("Desktop", "AlternateRightClickApp", AltRClickApp, 1024, "") < 1)
			bAltRClickApp = FALSE;
		bAltRClickThroughIcons = GetMyRegLong("Desktop", "AltDesktopRightClickThroughIcons", FALSE);
	}

}

/*--------------------------------------------------
    reset desktop icons
----------------------------------------------------*/
void EndDesktop(void)
{

	if(hwndDesktop && IsWindow(hwndDesktop))
	{
		if(oldWndProcDesktop)
			SetWindowLong(hwndDesktop, GWL_WNDPROC, (LONG)oldWndProcDesktop);

		if(bTransBK) SetDesktopIconTextBk(FALSE);
		if(bCustTextColor) SetDesktopIconTextColor(TRUE);
		if(DesktopIconNoText) SendMessage(hwndDesktop, WM_COMMAND, 28931, 0);

		if(bSmallIcons) 
			SetWindowLong(hwndDesktop, GWL_STYLE, oldstyle);
		bLargeIcons = bSmallIcons = DesktopIconNoText = FALSE;

	}
	hwndDesktop = NULL;
	bTransBK = bCustTextColor = FALSE;
	bCallDefaultRClick = bAltRClickApp = FALSE;

}

LRESULT CALLBACK WndProcDesktop(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SETREDRAW :
			if(DesktopIconNoText)
			{
				LRESULT res;

				res = CallWindowProc(oldWndProcDesktop, hwnd, message, wParam, lParam);
				ClearDesktopIconText();
				return res;
			}
			break;

		case WM_STYLECHANGED:
			if(bSmallIcons)
			{
				CallWindowProc(oldWndProcDesktop, hwnd, message, wParam, lParam);
				SetWindowLong(hwndDesktop, GWL_STYLE, oldstyle|LVS_SMALLICON);
				return 0;
			}
			break;
		case LVM_SETTEXTBKCOLOR:
			if(!bTransBK) break;
			return 0;
		case LVM_SETTEXTCOLOR:
			if(!bCustTextColor) break;
			return 0;
		case LVM_HITTEST:
			if(bAltRClickApp && !bAltRClickThroughIcons)
			{
				LRESULT r;
				LPLVHITTESTINFO lphti;
				lphti = (LPLVHITTESTINFO)lParam;
				r = CallWindowProc(oldWndProcDesktop, hwnd, message, wParam, lParam);
				if((lphti->flags == LVHT_ONITEMICON || lphti->flags == LVHT_ONITEMLABEL))
				{
					bIgnoreRClick = TRUE;
				}
				else
				{
					bIgnoreRClick = FALSE;
				}
				return r;
			}
			break;
		case WM_LBUTTONUP:
			{
				if(bIgnoreRClick)
					bIgnoreRClick = FALSE;

				break;
			}
		case WM_RBUTTONDOWN:
			{
				if(!bIgnoreRClick)
				{
					if(bAltRClickApp && !(wParam == MK_CONTROL))
						return 0;
				}
			}
			break;
		case WM_RBUTTONUP:
			{
				if(!bIgnoreRClick)
				{
					if(bAltRClickApp && !(wParam == MK_CONTROL))
					{
						ExecFile(hwnd, AltRClickApp);
						return 0;
					}
				}
				bIgnoreRClick = FALSE;
			}
			break;
	}
	return CallWindowProc(oldWndProcDesktop, hwnd, message, wParam, lParam);
}

void SetDesktopIconTextBk(BOOL bTrans)
{
	COLORREF col;
	
	if(bTrans)
		col = CLR_NONE;
	else
		col = GetSysColor(COLOR_DESKTOP);

	ListView_SetTextBkColor(hwndDesktop, col);

	ListView_RedrawItems(hwndDesktop, 0, ListView_GetItemCount(hwndDesktop));
	
	InvalidateRect(hwndDesktop, NULL, TRUE);
}

void SetDesktopIconTextColor(BOOL bDefault)
{
	COLORREF col;
	
	if(bDefault)
	{
		col = oldDeskTextColor  & 0x00ffffff;
	}
	else
	{
		if(DeskTextColor & 0x80000000) 
			col = GetSysColor(DeskTextColor & 0x00ffffff);
		else
			col = DeskTextColor;
	}

	ListView_SetTextColor(hwndDesktop, col);

	ListView_RedrawItems(hwndDesktop, 0, ListView_GetItemCount(hwndDesktop));
	
	InvalidateRect(hwndDesktop, NULL, TRUE);
}

void ClearDesktopIconText(void)
{
	LVITEM lvItem;
	int i, ii;

	GetMyRegStr("Desktop", "Blank", caption, 80, "");

	ii = SendMessage(hwndDesktop, LVM_GETITEMCOUNT, 0, 0);

	lvItem.state = LVIF_TEXT;
	lvItem.stateMask = 0;
	lvItem.cchTextMax = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = caption;

	for(i = 0; i < ii; i++)
	{
		lvItem.iItem = i;

		SendMessage(hwndDesktop, LVM_SETITEMTEXT, i, (LPARAM)&lvItem);
	}

}

