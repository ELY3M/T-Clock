/*-------------------------------------------------------------
  menu.c
  pop-up menu on right button click
  KAZUBON 1997-2001
---------------------------------------------------------------*/

#include "tclock.h"

HMENU g_hMenu = NULL;
static HMENU hPopupMenu = NULL;
void UpdateTimerMenu(HMENU hMenu);

/*------------------------------------------------
   when the clock is right-clicked
   show pop-up menu
--------------------------------------------------*/
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos)
{
	char s[80];
	
	if(!g_hMenu)
	{
		g_hMenu = LoadMenu(GetLangModule(), MAKEINTRESOURCE(IDR_MENU));
		hPopupMenu = GetSubMenu(g_hMenu, 0);
		
		//SetMenuDefaultItem(hPopupMenu, 408, FALSE);
		if(GetMyRegLong("", "CompactMenu", FALSE) && (GetVersion() & 0x80000000))
		{
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
		}
		else if(!GetMyRegLong("", "CompactMenu", FALSE) && (GetVersion() & 0x80000000))
		{
			DeleteMenu(hPopupMenu, 7, MF_BYPOSITION);
			DeleteMenu(hPopupMenu, 7, MF_BYPOSITION);
		}
		else if(GetMyRegLong("", "CompactMenu", FALSE) && !(GetVersion() & 0x80000000))
		{
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
				DeleteMenu(hPopupMenu, 0, MF_BYPOSITION);
		}
	}
	
	UpdateTimerMenu(hPopupMenu);
	
	GetMyRegStr("SNTP", "Server", s, 80, "");
	EnableMenuItem(hPopupMenu, IDC_SYNCTIME, MF_BYCOMMAND|
		(s[0]?MF_ENABLED:MF_GRAYED) );
	
	SetForegroundWindow98(hwnd);
	TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		xPos, yPos, 0, hwnd, NULL);
}

/*------------------------------------------------
    command message
--------------------------------------------------*/
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode)
{
	switch(wID)
	{
		case IDC_REFRESHTCLOCK:
			RefreshUs();
			return;
		case IDC_SHOWPROP: // Show property
			MyPropertySheet();
			return;
		case IDC_SHOWHELP: // Help
			MyHelp(hwnd, 0);
			return;
		case IDC_EXIT: // Exit
			PostMessage(g_hwndClock, WM_COMMAND, 102, 0);
			return;
		case IDC_SHOWCALENDER: // Calender
			DialogCalender(hwnd);
			return;
		case IDC_TIMER: // Timer
			DialogTimer(hwnd);
			return;
		case IDC_SYNCTIME: // Syncronize time
			StartSyncTime(hwnd, NULL, 0);
			return;
		case IDC_DATETIME:
		case IDC_CASCADE:
		case IDC_TILEHORZ:
		case IDC_TILEVERT:
		case IDC_MINALL:
		case IDC_TASKMAN:
		case IDC_TASKBARPROP:
		{
			HWND hwndTray;
			hwndTray = FindWindow("Shell_TrayWnd", NULL);
			if(hwndTray)
				PostMessage(hwndTray, WM_COMMAND, (WPARAM)wID, 0);
			return;
		}
	}
	
	if((IDC_STOPTIMER <= wID && wID < IDC_STOPTIMER + MAX_TIMER))
	{
		// stop timer
		StopTimer(hwnd, wID - IDC_STOPTIMER);
	}
	return;
}

/*------------------------------------------------
    update menu items to stop timers
--------------------------------------------------*/
void UpdateTimerMenu(HMENU hMenu)
{
	int i, j, len;
	UINT id;
	char s[80];
	
	len = GetMenuItemCount(hMenu);
	for(i = 0; i < len; i++)
	{
		id = GetMenuItemID(hMenu, i);
		if(IDC_STOPTIMER <= id && id < IDC_STOPTIMER + MAX_TIMER)
		{
			DeleteMenu(hMenu, id, MF_BYCOMMAND);
			len = GetMenuItemCount(hMenu);
			i--;
		}
	}
	
	for(i = 0; i < len; i++)
	{
		id = GetMenuItemID(hMenu, i);
		if(id == IDC_TIMER) break;
	}
	if(i < len) i++;
	
	for(j = 0; ; i++, j++)
	{
		if(GetTimerInfo(s, j) == 0) break;
		InsertMenu(hMenu, i, MF_BYPOSITION|MF_STRING,
			IDC_STOPTIMER + j, s);
	}
}
