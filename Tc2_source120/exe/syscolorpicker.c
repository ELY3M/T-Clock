/*-------------------------------------------
  deskcal.c
    Update Desktop Calendar automatically
    KAZUBON 1997-1999
---------------------------------------------*/

#include "tclock.h"

BOOL CALLBACK DlgProcCalender(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// propsheet.c
void SetMyDialgPos(HWND hwnd);

/*------------------------------------------------
   open "Calender" dialog
--------------------------------------------------*/
void DialogCalender(HWND hwnd)
{
	HWND hwndTray;
	INITCOMMONCONTROLSEX icc;

	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(g_hDlgCalender && IsWindow(g_hDlgCalender))
		;
	else
	{
		icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icc.dwICC = ICC_DATE_CLASSES;
		InitCommonControlsEx(&icc);
		g_hDlgCalender = CreateDialog(GetLangModule(), MAKEINTRESOURCE(121),
			NULL, (DLGPROC)DlgProcCalender);
	}
	SetForegroundWindow98(g_hDlgCalender);
}

/*------------------------------------------------
   dialog procedure of "Calender" dialog
--------------------------------------------------*/
BOOL CALLBACK DlgProcCalender(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)g_hIconTClock);
			SetMyDialgPos(hDlg);
			return TRUE;
		case WM_CLOSE:
			g_hDlgCalender = NULL;
			DestroyWindow(hDlg);
			break;
	}
	return FALSE;
}
