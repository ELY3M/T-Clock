/*-------------------------------------------
  pagetaskbar.c
  "taskbar" page of properties
  Kazubon 1997-1999
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnFlatTasks(HWND hDlg);
static BOOL IsIE4(void);
//static BOOL IsTrayIconsToolbar(void);

extern BOOL g_bApplyTaskbar; // propsheet.c
extern BOOL g_bApplyClear;

extern void SetDesktopIconTextBk(BOOL bNoTrans); // main.c

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
BOOL CALLBACK PageTaskbarProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			// checked other boxes
			if(id == IDC_FLATTRAY ||  
				id == IDC_TASKSWITCHICONS  
				|| id == IDC_TASKSWITCHSEP)
				SendPSChanged(hDlg);
			else if(id == IDC_BARCLEAR)
			{
				g_bApplyClear = TRUE;
				SendPSChanged(hDlg);
			}
			else if(id == IDC_TASKSWITCHFLAT)
				OnFlatTasks(hDlg);
			else if(id == IDC_GRIPPERSTYLE || id == IDC_TASKBAREDGESTYLE 
				&& code == CBN_SELCHANGE)
				SendPSChanged(hDlg);
			// "Transparancy"
			else if(id == IDC_BARTRANS && code == EN_CHANGE)
				SendPSChanged(hDlg);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				//case PSN_HELP: MyHelp(GetParent(hDlg), 10); break;
			}
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
   initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	DWORD dw;
	char s[1024], ss[1024];
	int i, ii;

	dw = GetVersion();

	// setting of "background" and "text"

	for(i = 53; i <= 54; i++)
		CBAddString(hDlg, IDC_GRIPPERSTYLE, (LPARAM)MyString(i));
	GetMyRegStr("Taskbar", "RebarGripperType", s, 80, MyString(53));
	if(_strnicmp(s, "HIDDEN", 6) == 0)
		CBSetCurSel(hDlg, IDC_GRIPPERSTYLE, 1);
	else
		CBSetCurSel(hDlg, IDC_GRIPPERSTYLE, 0);

	EnableDlgItem(hDlg, IDC_LABGRIPPERSTYLE, IsIE4());
	EnableDlgItem(hDlg, IDC_GRIPPERSTYLE, IsIE4());

	for(ii = 66; ii <= 67; ii++)
		CBAddString(hDlg, IDC_TASKBAREDGESTYLE, (LPARAM)MyString(ii));
	GetMyRegStr("Taskbar", "TaskbarEdgeStyle", ss, 80, MyString(66));
	if(_strnicmp(ss, "FLAT", 4) == 0)
		CBSetCurSel(hDlg, IDC_TASKBAREDGESTYLE, 1);
	else
		CBSetCurSel(hDlg, IDC_TASKBAREDGESTYLE, 0);

	CheckDlgButton(hDlg, IDC_TASKSWITCHFLAT,
		GetMyRegLong("Taskbar", "TaskSwitchFlat", FALSE));
	EnableDlgItem(hDlg, IDC_TASKSWITCHFLAT, IsIE4());

	CheckDlgButton(hDlg, IDC_TASKSWITCHSEP,
		GetMyRegLong("Taskbar", "TaskSwitchSeparators", FALSE));
	//if((!(dw & 0x80000000) && LOBYTE(LOWORD(dw)) >= 5 && HIBYTE(LOWORD(dw)) >= 1))
		EnableDlgItem(hDlg, IDC_TASKSWITCHSEP, FALSE);

	CheckDlgButton(hDlg, IDC_TASKSWITCHICONS,
		GetMyRegLong("Taskbar", "TaskSwitchIconsOnly", FALSE));

	CheckDlgButton(hDlg, IDC_FLATTRAY,
		GetMyRegLong("Taskbar", "FlatTray", FALSE));
		
	//dw = GetVersion();
	if(!(dw & 0x80000000) && LOBYTE(LOWORD(dw)) >= 5) ; // Win2000
	else
	{
		int i;
		for(i = IDC_CAPBARTRANS; i <= IDC_SPINBARTRANS; i++)
			EnableDlgItem(hDlg, i, FALSE);
		EnableDlgItem(hDlg, IDC_BARCLEAR, FALSE);
		EnableDlgItem(hDlg, IDC_LABBARCLEAR, FALSE);
	}
	
	dw = GetMyRegLong("Taskbar", "AlphaTaskbar", 0);
	if(dw > 97) dw = 97;
	SendDlgItemMessage(hDlg, IDC_SPINBARTRANS, UDM_SETRANGE, 0,
		MAKELONG(97, 0));
	SendDlgItemMessage(hDlg, IDC_SPINBARTRANS, UDM_SETPOS, 0,
		(int)(short)dw);
	
	CheckDlgButton(hDlg, IDC_BARCLEAR,
		GetMyRegLong("Taskbar", "ClearTaskbar", FALSE));

	OnFlatTasks(hDlg);

}

/*------------------------------------------------
  apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];

	GetDlgItemText(hDlg, IDC_GRIPPERSTYLE, s, 1024);
	SetMyRegStr("Taskbar", "RebarGripperType", s);

	GetDlgItemText(hDlg, IDC_TASKBAREDGESTYLE, s, 1024);
	SetMyRegStr("Taskbar", "TaskbarEdgeStyle", s);

	SetMyRegLong("Taskbar", "TaskSwitchFlat",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCHFLAT));

	SetMyRegLong("Taskbar", "TaskSwitchSeparators",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCHSEP));

	SetMyRegLong("Taskbar", "TaskSwitchIconsOnly",
		IsDlgButtonChecked(hDlg, IDC_TASKSWITCHICONS));

	SetMyRegLong("Taskbar", "FlatTray",
		IsDlgButtonChecked(hDlg, IDC_FLATTRAY));

	SetMyRegLong("Taskbar", "AlphaTaskbar",
		SendDlgItemMessage(hDlg, IDC_SPINBARTRANS, UDM_GETPOS, 0, 0));
		
	SetMyRegLong("Taskbar", "ClearTaskbar",
		IsDlgButtonChecked(hDlg, IDC_BARCLEAR));

}

void OnFlatTasks(HWND hDlg)
{
	BOOL b;
	DWORD dw;

	dw = GetVersion();
	if(!(!(dw & 0x80000000) && LOBYTE(LOWORD(dw)) >= 5 && HIBYTE(LOWORD(dw)) >= 1))
	{
		b = IsDlgButtonChecked(hDlg, IDC_TASKSWITCHFLAT);
		EnableDlgItem(hDlg, IDC_TASKSWITCHSEP, b);
	}

	SendPSChanged(hDlg);
}

/*------------------------------------------------
  IE 4 or later ?
--------------------------------------------------*/
BOOL IsIE4(void)
{
	HWND hwnd;
	char classname[80];
	DWORD dw;
	
	dw = GetRegLong(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",
		"ClassicShell", 0);
	if(dw) return TRUE;
	
	hwnd = FindWindow("Shell_TrayWnd", NULL);
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
  Is the tray icons "ToolbarWindow32" ?
--------------------------------------------------*/
/*BOOL IsTrayIconsToolbar(void)
{
	HWND hwnd;
	char classname[80];
	
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd == NULL) return FALSE;
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
			break;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	if(hwnd == NULL) return FALSE;
	
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ToolbarWindow32") == 0)
			break;
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
	if(hwnd == NULL) return FALSE;
	return TRUE;
}*/

/*------------------------------------------------
　「色」コンボボックスの初期化
--------------------------------------------------*/
