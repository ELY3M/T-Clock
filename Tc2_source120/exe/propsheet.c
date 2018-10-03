/*-------------------------------------------
  propsheet.c
  show "properties for TClock"
  Kazubon 1997-2001
---------------------------------------------*/

#include "tclock.h"

#define MAX_PAGE  12

int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam);
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void SetMyDialgPos(HWND hwnd);

// dialog procedures of each page
BOOL CALLBACK PageColorProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageClockProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageFormatProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageAlarmProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageMouseProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageSkinProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageStartMenuProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageTaskbarProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageSNTPProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageMiscProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageAboutProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK PageDesktopProc(HWND, UINT, WPARAM, LPARAM);

void SetPropSheetPos(HWND hwnd);

static WNDPROC oldWndProc; // to save default window procedure
static int startpage = 0;  // page to open first

BOOL g_bApplyClock = FALSE;
BOOL g_bApplyTaskbar = FALSE;
BOOL g_bApplyLangDLL = FALSE;
BOOL g_bApplyClear = FALSE;

// menu.c
extern HMENU g_hMenu;

/*-------------------------------------------
  show property sheet
---------------------------------------------*/
void MyPropertySheet(void)
{
	PROPSHEETPAGE psp[MAX_PAGE];
	PROPSHEETHEADER psh;
	/*DLGPROC PageProc[MAX_PAGE] = { PageColorProc, PageClockProc, PageFormatProc,
		PageAlarmProc, PageMouseProc, PageSkinProc, PageTaskbarProc, 
		PageStartMenuProc, PageSNTPProc, PageMiscProc, PageAboutProc, PageDesktopProc };*/
	DLGPROC PageProc[MAX_PAGE] = { PageAboutProc, PageTaskbarProc, PageSkinProc, 
		PageStartMenuProc, PageDesktopProc, PageClockProc, PageColorProc, 
		PageFormatProc, PageSNTPProc, PageAlarmProc, PageMouseProc, PageMiscProc };
	int i;
	
	// already opened
	if(g_hwndSheet && IsWindow(g_hwndSheet))
	{
		SetForegroundWindow98(g_hwndSheet); return;
	}
	
	// set data of each page
	for(i = 0; i < MAX_PAGE; i++)
	{
		memset(&psp[i], 0, sizeof(PROPSHEETPAGE));
	    psp[i].dwSize = sizeof(PROPSHEETPAGE);
		//psp[i].dwFlags = PSP_HASHELP;
		psp[i].hInstance = GetLangModule();
		//psp[i].pszTemplate = MAKEINTRESOURCE(IDD_PAGECOLOR + i);
		psp[i].pfnDlgProc = PageProc[i];
	}

	//i = i + 1;
	//memset(&psp[i], 0, sizeof(PROPSHEETPAGE));
	//psp[i].dwSize = sizeof(PROPSHEETPAGE);
	//psp[i].dwFlags = PSP_HASHELP;
	//psp[i].hInstance = GetLangModule();
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PAGEABOUT);
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PAGETASKBAR);
	psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PAGESKIN);
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PAGESTARTMENU);
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PAGEDESKTOP);
	psp[5].pszTemplate = MAKEINTRESOURCE(IDD_PAGECLOCK);

	psp[6].pszTemplate = MAKEINTRESOURCE(IDD_PAGECOLOR);
	psp[7].pszTemplate = MAKEINTRESOURCE(IDD_PAGEFORMAT);
	psp[8].pszTemplate = MAKEINTRESOURCE(IDD_PAGESNTP);
	psp[9].pszTemplate = MAKEINTRESOURCE(IDD_PAGEALARM);
	psp[10].pszTemplate = MAKEINTRESOURCE(IDD_PAGEMOUSE);
	psp[11].pszTemplate = MAKEINTRESOURCE(IDD_PAGEMISC);

	//psp[i].pfnDlgProc = PageProc[i];

	// set data of property sheet
	memset(&psh, 0, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE |
		PSH_MODELESS | PSH_USECALLBACK; //| PSH_HASHELP;
	psh.hInstance = g_hInst;
	psh.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
	psh.pszCaption = MyString(IDS_PROPERTY);
	psh.nPages = MAX_PAGE + 1;
	psh.nStartPage = startpage;
	psh.ppsp = psp;
	psh.pfnCallback = PropSheetProc;
	
	g_bApplyClock = FALSE;
	g_bApplyTaskbar = FALSE;
	g_bApplyLangDLL = FALSE;
	
	// show it !
	g_hwndSheet = (HWND)PropertySheet(&psh);
	SetForegroundWindow98(g_hwndSheet);
}

/*-------------------------------------------
  callback procedure of property sheet
---------------------------------------------*/
int CALLBACK PropSheetProc(HWND hDlg, UINT uMsg, LPARAM  lParam)
{
	LONG style;
	
	if(uMsg == PSCB_INITIALIZED)
	{
		// hide ? button
		style = GetWindowLong(hDlg, GWL_EXSTYLE);
		style ^= WS_EX_CONTEXTHELP;
		SetWindowLong(hDlg, GWL_EXSTYLE, style);
		
		// subclass the window
		oldWndProc = (WNDPROC)SetWindowLong(hDlg, GWL_WNDPROC,
			(LONG)SubclassProc);
		
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);
	}
	return 0;
}

/*--------------------------------------------------------
   window procedure of subclassed property sheet
---------------------------------------------------------*/
LRESULT CALLBACK SubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT l;
	
	switch(message)
	{
		case WM_SHOWWINDOW: // adjust the window position
			SetMyDialgPos(hwnd);
			return 0;
	}
	
	// default
	l = CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
	
	switch(message)
	{
		case WM_COMMAND:
		{
			WORD id;
			id = LOWORD(wParam);
			// close the window by "OK" or "Cancel"
			if(id == IDOK || id == IDCANCEL)
			{
				// MyHelp(hwnd, -1);
				startpage = SendMessage(
					(HWND)SendMessage(hwnd, PSM_GETTABCONTROL, 0, 0),
					TCM_GETCURSEL, 0, 0);
				if(startpage < 0) startpage = 0;
				DestroyWindow(hwnd);
				g_hwndSheet = NULL;
			}
			// apply settings
			if(id == IDOK || id == 0x3021)
			{
				if(g_bApplyClock)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
					g_bApplyClock = FALSE;
				}
				if(g_bApplyClear)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHCLEARTASKBAR, 0, 0);
					g_bApplyClear = FALSE;
				}
				if(g_bApplyTaskbar)
				{
					SendMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
					g_bApplyTaskbar = FALSE;
				}
			}
			if(id == IDOK || id == IDCANCEL)
			{
				// reload language dll
				if(g_bApplyLangDLL)
				{
					char fname[MAX_PATH];
					HINSTANCE hInst;
					hInst = LoadLanguageDLL(fname);
					if(hInst != NULL)
					{
						if(g_hDlgTimer && IsWindow(g_hDlgTimer))
							PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
						if(g_hDlgCalender && IsWindow(g_hDlgCalender))
							PostMessage(g_hDlgCalender, WM_CLOSE, 0, 0);
						if(g_hMenu) DestroyMenu(g_hMenu);
						g_hMenu = NULL;
						if(g_hInstResource) FreeLibrary(g_hInstResource);
						g_hInstResource = hInst;
						strcpy(g_langdllname, fname);
					}
				}
			}
			break;
		}
		// close by "x" button
		case WM_SYSCOMMAND:
		{
			if((wParam & 0xfff0) == SC_CLOSE)
				PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
			break;
		}
	}
	return l;
}

/*------------------------------------------------
   adjust the window position
--------------------------------------------------*/
void SetMyDialgPos(HWND hwnd)
{
	HWND hwndTray;
	RECT rc, rcTray;
	int wscreen, hscreen, wProp, hProp;
	int x, y;

	GetWindowRect(hwnd, &rc);
	wProp = rc.right - rc.left;
	hProp = rc.bottom - rc.top;
	
	wscreen = GetSystemMetrics(SM_CXSCREEN);
	hscreen = GetSystemMetrics(SM_CYSCREEN);
	
	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(hwndTray == NULL) return;
	GetWindowRect(hwndTray, &rcTray);
	if(rcTray.right - rcTray.left > 
		rcTray.bottom - rcTray.top)
	{
		x = wscreen - wProp - 32;
		if(rcTray.top < hscreen / 2)
			y = rcTray.bottom + 2;
		else
			y = rcTray.top - hProp - 32;
		if(y < 0) y = 0;
	}
	else
	{
		y = hscreen - hProp - 2;
		if(rcTray.left < wscreen / 2)
			x = rcTray.right + 2;
		else
			x = rcTray.left - wProp - 2;
		if(x < 0) x = 0;
	}

	MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
}

/*------------------------------------------------
   select file
--------------------------------------------------*/
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex,
	const char *deffile, char *retfile)
{
	OPENFILENAME ofn;
	char fname[MAX_PATH], ftitle[MAX_PATH], initdir[MAX_PATH];
	BOOL r;
	
	memset(&ofn, '\0', sizeof(OPENFILENAME));
	
	strcpy(initdir, g_mydir);
	if(deffile[0])
	{
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		hfind = FindFirstFile(deffile, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			FindClose(hfind);
			strcpy(initdir, deffile);
			del_title(initdir);
		}
	}
	
	fname[0] = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = nFilterIndex;
	ofn.lpstrFile= fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;
	ofn.Flags = OFN_HIDEREADONLY|OFN_EXPLORER|OFN_FILEMUSTEXIST;
	
	r = GetOpenFileName(&ofn);
	if(!r) return r;
	
	strcpy(retfile, ofn.lpstrFile);
	
	return r;
}

