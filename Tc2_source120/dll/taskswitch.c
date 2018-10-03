/*-------------------------------------------
  taskswitch.c
    Customize Task Switcher
    Kazubon 1999
---------------------------------------------*/
#include "tcdll.h"

#ifndef TCS_FLATBUTTONS
#define TCS_FLATBUTTONS         0x0008
#endif
#ifndef TCS_HOTTRACK
#define TCS_HOTTRACK            0x0040
#endif

#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT            0x0800
#endif

#ifndef TCM_FIRST
#define TCM_FIRST               0x1300
#endif

#ifndef TCM_SETITEMSIZE
#define TCM_SETITEMSIZE         (TCM_FIRST + 41)
#endif

#ifndef TCM_SETIMAGELIST
#define TCM_SETIMAGELIST        (TCM_FIRST + 3)
#endif

#ifndef TB_SETBUTTONSIZE
#define TB_SETBUTTONSIZE        (WM_USER + 31)
#endif

#ifndef TB_SETBUTTONWIDTH
#define TB_SETBUTTONWIDTH       (WM_USER + 59)
#endif

#ifndef TB_GETBUTTONSIZE
#define TB_GETBUTTONSIZE        (WM_USER + 58)
#endif

#ifndef TBSTYLE_AUTOSIZE
#define TBSTYLE_AUTOSIZE        0x0010
#endif

#ifndef TBSTYLE_LIST
#define TBSTYLE_LIST            0x100
#endif

#ifndef TB_SETBUTTONINFO
#define TB_SETBUTTONINFO        (WM_USER + 66)
#endif

#ifndef TBIF_TEXT
#define TBIF_TEXT        0x00000002
#endif

#ifndef TBSTYLE_EX_MIXEDBUTTONS
#define TBSTYLE_EX_MIXEDBUTTONS         0x00000008
#endif

#ifndef TB_SETEXTENDEDSTYLE
#define TB_SETEXTENDEDSTYLE     (WM_USER + 84)
#endif

#ifndef TB_GETEXTENDEDSTYLE
#define TB_GETEXTENDEDSTYLE     (WM_USER + 85)
#endif

#ifndef TCS_EX_FLATSEPARATORS
#define TCS_EX_FLATSEPARATORS   0x0001
#endif

#define TCM_SETEXTENDEDSTYLE    (TCM_FIRST + 52)
#define TBSTYLE_TRANSPARENT     0x8000

extern HANDLE hmod;

LRESULT CALLBACK WndProcTab(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProcTaskSwitch(HWND, UINT, WPARAM, LPARAM);

void EndTaskSwitch(void);
static WNDPROC oldWndProcTab = NULL;
static WNDPROC oldWndProcTaskSwitch = NULL;

HWND hwndTab = NULL;
static HWND hwndTaskSwitch = NULL;

static LONG oldstyle;
static LONG oldexstyle;
extern BOOL bWinXP;
static BOOL bTaskSwitchFlat = FALSE;
static BOOL bTaskSwitchSep = FALSE;
static BOOL bTaskSwitchIcons = FALSE;
static LONG OldTaskWidth;
//extern BOOL bCustomColorScheme;
//extern COLORREF ColorTaskbarFace;

HIMAGELIST hilOldList = NULL;

/*--------------------------------------------------
   initialize
----------------------------------------------------*/
void InitTaskSwitch(HWND hwndClock)
{
	HANDLE hwnd;
	char classname[80];
	//HBRUSH hbr;
	//HDC hdc;
	//RECT prc;
	//COLORSCHEME colScheme;


	EndTaskSwitch();
	
	bTaskSwitchFlat = GetMyRegLong("Taskbar", "TaskSwitchFlat", FALSE);
	bTaskSwitchIcons = GetMyRegLong("Taskbar", "TaskSwitchIconsOnly", FALSE);
	if(bTaskSwitchFlat)
		bTaskSwitchSep = GetMyRegLong("Taskbar", "TaskSwitchSeparators", FALSE);

	if(!bTaskSwitchFlat && !bTaskSwitchIcons)
		return;
	
	// get window handle of MSTaskSwWClass
	hwndTaskSwitch = NULL;
	hwnd = GetParent(hwndClock);  // TrayNotifyWnd
	hwnd = GetParent(hwnd);       // Shell_TrayWnd
	if(hwnd == NULL) return;
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ReBarWindow32") == 0)
		{
			hwnd = GetWindow(hwnd, GW_CHILD);
			while(hwnd)
			{
				GetClassName(hwnd, classname, 80);
				if(lstrcmpi(classname, "MSTaskSwWClass") == 0)
				{
					hwndTaskSwitch = hwnd;
					break;
				}
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
		}
		else if(lstrcmpi(classname, "MSTaskSwWClass") == 0)
		{
			hwndTaskSwitch = hwnd;
			break;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	if(hwndTaskSwitch == NULL)
		return;

	hwndTab = GetWindow(hwndTaskSwitch, GW_CHILD);

	if(hwndTab == NULL)
		return;
	
	if(bTaskSwitchFlat)
	{
		if(!bWinXP)
		{
			oldstyle = GetWindowLong(hwndTab, GWL_STYLE);
			if(bTaskSwitchSep)
			{
				SetWindowLong(hwndTab, GWL_STYLE, oldstyle|TCS_FLATBUTTONS|TCS_HOTTRACK);
				SendMessage( hwndTab, TCM_SETEXTENDEDSTYLE, 0, TCS_EX_FLATSEPARATORS);
			}
			else
				SetWindowLong(hwndTab, GWL_STYLE, oldstyle|TCS_FLATBUTTONS|TCS_HOTTRACK);
		}else if(bWinXP)
		{
			oldstyle = GetWindowLong(hwndTab, GWL_STYLE);
			SetWindowLong(hwndTab, GWL_STYLE, oldstyle|TBSTYLE_FLAT);
		}
	}

	if(bTaskSwitchIcons)
	{
		if(!bWinXP)
		{
			OldTaskWidth = SendMessage(hwndTab, TCM_SETITEMSIZE, 0, MAKELPARAM(22, 22));
			OldTaskWidth = LOWORD(OldTaskWidth);

			//hilOldList = SendMessage(hwndTab, TCM_SETIMAGELIST, 0, NULL);
		}else
		{
			oldexstyle = SendMessage(hwndTab, TB_GETEXTENDEDSTYLE, 0, 0);
			SendMessage(hwndTab, TB_SETEXTENDEDSTYLE, 0, oldexstyle|TBSTYLE_EX_MIXEDBUTTONS);
		}
	}
	
	oldWndProcTab = (WNDPROC)GetWindowLong(hwndTab, GWL_WNDPROC);
	SetWindowLongA(hwndTab, GWL_WNDPROC, (LONG)WndProcTab);

	/*if(bCustomColorScheme)
	{
		oldWndProcTaskSwitch = (WNDPROC)GetWindowLong(hwndTaskSwitch, GWL_WNDPROC);
		SetWindowLongA(hwndTaskSwitch, GWL_WNDPROC, (LONG)WndProcTaskSwitch);

		hdc = GetWindowDC(hwndTab);
		hbr = CreateSolidBrush(ColorTaskbarFace);
		SetClassLong(hwndTab, GCL_HBRBACKGROUND, (LONG)&hbr);
		GetClientRect(hwndTab, &prc);
		FillRect(hdc, &prc, hbr);
		InvalidateRect(hwndTab, NULL, TRUE);
		//SendMessage(hwndTab, WM_PAINT, 0, 0);
		DeleteObject(hbr);

		colScheme.dwSize = sizeof(colScheme);
		colScheme.clrBtnHighlight = GetSysColor(COLOR_3DFACE);
		colScheme.clrBtnShadow = GetSysColor(COLOR_3DFACE);
		SendMessage(hwndTab, RB_SETCOLORSCHEME, 0, (LPARAM) (LPCOLORSCHEME)&colScheme);
	}*/
}

/*--------------------------------------------------
    undo
----------------------------------------------------*/
void EndTaskSwitch(void)
{
	if(hwndTab && IsWindow(hwndTab))
	{
		if(oldWndProcTab)
			SetWindowLong(hwndTab, GWL_WNDPROC, (LONG)oldWndProcTab);
		oldWndProcTab = NULL;
		//if(oldWndProcTaskSwitch)
			//SetWindowLong(hwndTaskSwitch, GWL_WNDPROC, (LONG)oldWndProcTaskSwitch);
		//oldWndProcTaskSwitch = NULL;

		if(bTaskSwitchIcons)
		{
			if(!bWinXP)
				SendMessage(hwndTab, TCM_SETITEMSIZE, 0, MAKELPARAM(OldTaskWidth, 22));
			else
			{
				SendMessage(hwndTab, TB_SETEXTENDEDSTYLE, 0, oldexstyle);
			}
		}
		if(bTaskSwitchFlat)
			SetWindowLong(hwndTab, GWL_STYLE, oldstyle);
		if(bTaskSwitchSep)
			SendMessage( hwndTab, TCM_SETEXTENDEDSTYLE, TCS_EX_FLATSEPARATORS, 0);
	}
	hwndTab = hwndTaskSwitch = NULL;
}

/*------------------------------------------------
   subclass procedure of SysTabControl32
--------------------------------------------------*/
LRESULT CALLBACK WndProcTab(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		/*case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			HBRUSH hbr;
			RECT prc;
			
			if(!bCustomColorScheme) break;

			hdc = BeginPaint(hwnd, &ps);
			//hdc = GetDC(hwnd);
			hbr = CreateSolidBrush(ColorTaskbarFace);
			GetWindowRect(hwnd, &prc);
			FillRect(hdc, &prc, hbr);
			DeleteObject(hbr);
			
			EndPaint(hwnd, &ps);

			//break;
			return CallWindowProc(oldWndProcTab, hwnd, message, wParam, lParam);;
		}*/

		case TCM_SETITEMSIZE:
			if(bTaskSwitchIcons)
			{
				lParam = MAKELPARAM(22, HIWORD(lParam));
			}else
			{
				if(LOWORD(lParam)-8 >= 22)
					lParam = MAKELPARAM(LOWORD(lParam)-8, HIWORD(lParam));
			}
			break;
		case TCM_INSERTITEMA:
		case TCM_INSERTITEMW:
			PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			break;
		case TCM_DELETEITEM:
			PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			break;
		//case TCM_SETIMAGELIST:
			//return 0;
		//case TB_SETBUTTONSIZE:
			//if(bTaskSwitchIcons)
			//{
				//lParam = MAKELPARAM(23, HIWORD(lParam));
			//}else
			//{
				//if(LOWORD(lParam)-8 >= 22)
					//lParam = MAKELPARAM(LOWORD(lParam)-8, HIWORD(lParam));
			//}
			//return 0;
		//case TB_SETBUTTONWIDTH:
			//if(!bTaskSwitchIcons) break;
			//if(bTaskSwitchIcons)
			//lParam = MAKELPARAM(23, 23);
			//return 0;
		//case TB_INSERTBUTTON:
			//PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			//break;
		//case TB_DELETEBUTTON:
			//PostMessage(GetParent(hwnd), WM_SIZE, SIZE_RESTORED, 0);
			//PostMessage(hwnd, TB_AUTOSIZE, 0, 0);
			//return 0;
		//case TB_AUTOSIZE:
			//if(!bTaskSwitchIcons) break;
			//return 0;
		//case TB_SETBUTTONINFO:
			//{
				//LPTBBUTTONINFO tbbi;

				//if(!bTaskSwitchIcons) break;
				//char tbText[0];
				//tbbi = (LPTBBUTTONINFO)lParam;
				//tbbi->dwMask = TBIF_TEXT;
				//WriteDebug(tbbi->pszText); //= "";
				//break;
			//}

	}
	return CallWindowProc(oldWndProcTab, hwnd, message, wParam, lParam);
}

/*LRESULT CALLBACK WndProcTaskSwitch(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			HBRUSH hbr;
			RECT prc;
			
			if(!bCustomColorScheme) break;

			hdc = BeginPaint(hwnd, &ps);
			hbr = CreateSolidBrush(ColorTaskbarFace);
			GetClientRect(hwnd, &prc);
			FillRect(hdc, &prc, hbr);
			DeleteObject(hbr);
			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_NOTIFY:
		{
			LPNMHDR pnmh;
			LRESULT r;
			//if(bNoClock) break;
			pnmh = (LPNMHDR)lParam;
			//r = CallWindowProc(oldWndProcTask, hwnd, message, wParam, lParam);
			if (pnmh->code == NM_CUSTOMDRAW && pnmh->hwndFrom == hwndTab)
			{
				//LPNMCUSTOMDRAW pnmcd;
				//pnmcd = (LPNMCUSTOMDRAW)lParam;
				return RebarCustomDraw((LPNMTBCUSTOMDRAW)lParam);
			}
			return 0;
		}

	}
	return CallWindowProc(oldWndProcTaskSwitch, hwnd, message, wParam, lParam);
}*/

/*LRESULT RebarCustomDraw(NMTBCUSTOMDRAW *lpcd)
{
    RECT rect;
	HBRUSH hbr;

    if(lpcd->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
        WriteDebug("CDDS_PREPAINT");
		return CDRF_NOTIFYITEMDRAW;
    }
    else if(lpcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)// && lpcd->nmcd.dwItemSpec != 0)
    {
        lpcd->clrHighlightHotTrack = ColorTaskbarFace;
        lpcd->clrBtnFace = ColorTaskbarFace;
		lpcd->nStringBkMode = TRANSPARENT;
		//SetBkMode(lpcd->nmcd.hdc, TRANSPARENT);
		//SetBkColor(lpcd->nmcd.hdc, ColorTaskbarFace);
		lpcd->clrMark = ColorTaskbarFace;
		WriteDebug("CDDS_ITEMPREPAINT");
		return TBCDRF_HILITEHOTTRACK|CDRF_DODEFAULT ;
    }

    WriteDebug("CDRF_DODEFAULT");
	return CDRF_DODEFAULT;
}*/