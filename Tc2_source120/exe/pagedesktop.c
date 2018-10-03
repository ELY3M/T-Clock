/*-------------------------------------------
  pagetaskbar.c
  "taskbar" page of properties
  Kazubon 1997-1999
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnMeasureItemColorCombo(LPARAM lParam);
static void OnDrawItemColorCombo(LPARAM lParam);
static void OnDesktop(HWND hDlg);
static void OnDesktopText(HWND hDlg);
static void OnDesktopRClick(HWND hDlg);
static void OnChooseColor(HWND hDlg, WORD id);
static void OnSanshoMenu(HWND hDlg, WORD id);
static BOOL IsIE4(void);

extern BOOL g_bApplyTaskbar; // propsheet.c

extern void SetDesktopIconTextBk(BOOL bNoTrans); // main.c

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
BOOL CALLBACK PageDesktopProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_MEASUREITEM:
			OnMeasureItemColorCombo(lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItemColorCombo(lParam);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			// checked other boxes
			if(id == IDC_TRANSDESKTOPICONBK || IDC_DESKTOPNOICONTEXT || 
				id == IDC_DESKTOPALTRCLICKTHROUGHICONS)
				SendPSChanged(hDlg);
			else if(id == IDC_DESKTOPALTRCLICKAPPBROWSE)
				OnSanshoMenu(hDlg, id);
			else if(id == IDC_DESKTOPALTRCLICKAPP && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_CUSTOMIZEDESKTOP)
				OnDesktop(hDlg);
			else if(id == IDC_LABDESKTOPICONTEXT)
				OnDesktopText(hDlg);
			else if(id == IDC_LABDESKTOPALTRCLICKAPP)
				OnDesktopRClick(hDlg);
			else if(id == IDC_SANSHODESKTOPICONTEXT)
				OnChooseColor(hDlg, id);
			else if(id == IDC_DESKTOPICONTEXT 
				|| id == IDC_DESKTOPICONSIZE && code == CBN_SELCHANGE)
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
	char s[1024], ss[1024], sss[1024];
	int i, ii, iii;

	dw = GetVersion();

	// setting of "background" and "text"
	InitColor(hDlg);
		
	CheckDlgButton(hDlg, IDC_TRANSDESKTOPICONBK,
		GetMyRegLong("Desktop", "DesktopTextBkTrans", FALSE));

	CheckDlgButton(hDlg, IDC_CUSTOMIZEDESKTOP,
		GetMyRegLong("Desktop", "CustomizeDesktop", FALSE));

	for(iii = 69; iii <= 70; iii++)
		CBAddString(hDlg, IDC_DESKTOPICONSIZE, (LPARAM)MyString(iii));
	GetMyRegStr("Desktop", "DesktopIconSize", sss, 80, MyString(69));
	if(_strnicmp(sss, "SMALL", 5) == 0)
		CBSetCurSel(hDlg, IDC_DESKTOPICONSIZE, 0);
	else
		CBSetCurSel(hDlg, IDC_DESKTOPICONSIZE, 1);

	CheckDlgButton(hDlg, IDC_LABDESKTOPICONTEXT,
		GetMyRegLong("Desktop", "CustomizeDesktopTextColor", FALSE));

	CheckDlgButton(hDlg, IDC_LABDESKTOPALTRCLICKAPP,
		GetMyRegLong("Desktop", "AltDesktopRightClick", FALSE));

	GetMyRegStr("Desktop", "AlternateRightClickApp", s, 1024, "");
	SetDlgItemText(hDlg, IDC_DESKTOPALTRCLICKAPP, s);

	CheckDlgButton(hDlg, IDC_DESKTOPALTRCLICKTHROUGHICONS,
		GetMyRegLong("Desktop", "AltDesktopRightClickThroughIcons", FALSE));

	CheckDlgButton(hDlg, IDC_DESKTOPNOICONTEXT,
		GetMyRegLong("Desktop", "NoDesktopIconText", FALSE));

	OnDesktop(hDlg);

}

/*------------------------------------------------
  apply - save settings
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char s[1024];

	SetMyRegLong("Desktop", "DesktopTextBkTrans",
		IsDlgButtonChecked(hDlg, IDC_TRANSDESKTOPICONBK));

	SetMyRegLong("Desktop", "CustomizeDesktop",
		IsDlgButtonChecked(hDlg, IDC_CUSTOMIZEDESKTOP));

	GetDlgItemText(hDlg, IDC_DESKTOPICONSIZE, s, 1024);
	SetMyRegStr("Desktop", "DesktopIconSize", s);

	SetMyRegLong("Desktop", "CustomizeDesktopTextColor",
		IsDlgButtonChecked(hDlg, IDC_LABDESKTOPICONTEXT));

	dw = CBGetItemData(hDlg, IDC_DESKTOPICONTEXT, CBGetCurSel(hDlg, IDC_DESKTOPICONTEXT));
	SetMyRegLong("Desktop", "DesktopTextColor", dw);

	SetMyRegLong("Desktop", "AltDesktopRightClick",
		IsDlgButtonChecked(hDlg, IDC_LABDESKTOPALTRCLICKAPP));

	GetDlgItemText(hDlg, IDC_DESKTOPALTRCLICKAPP, s, 1024);
	SetMyRegStr("Desktop", "AlternateRightClickApp", s);

	SetMyRegLong("Desktop", "AltDesktopRightClickThroughIcons",
		IsDlgButtonChecked(hDlg, IDC_DESKTOPALTRCLICKTHROUGHICONS));

	SetMyRegLong("Desktop", "NoDesktopIconText",
		IsDlgButtonChecked(hDlg, IDC_DESKTOPNOICONTEXT));

}

void OnDesktop(HWND hDlg)
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_CUSTOMIZEDESKTOP);
	for(i = IDC_TRANSDESKTOPICONBK; i <= IDC_DESKTOPALTRCLICKTHROUGHICONS; i++)
		EnableDlgItem(hDlg, i, b);

	if(b)
	{
		b = IsDlgButtonChecked(hDlg, IDC_LABDESKTOPICONTEXT);
		{
			EnableDlgItem(hDlg, IDC_SANSHODESKTOPICONTEXT, b);
			EnableDlgItem(hDlg, IDC_DESKTOPICONTEXT, b);
		}
		b = IsDlgButtonChecked(hDlg, IDC_LABDESKTOPALTRCLICKAPP);
		{
			EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKAPP, b);
			EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKAPPBROWSE, b);
			EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKTHROUGHICONS, b);
		}
	}

	SendPSChanged(hDlg);
}

void OnDesktopText(HWND hDlg)
{
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_LABDESKTOPICONTEXT);

	EnableDlgItem(hDlg, IDC_SANSHODESKTOPICONTEXT, b);
	EnableDlgItem(hDlg, IDC_DESKTOPICONTEXT, b);

	SendPSChanged(hDlg);
}

void OnDesktopRClick(HWND hDlg)
{
	BOOL b;
	
	b = IsDlgButtonChecked(hDlg, IDC_LABDESKTOPALTRCLICKAPP);

	EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKAPP, b);
	EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKAPPBROWSE, b);
	EnableDlgItem(hDlg, IDC_DESKTOPALTRCLICKTHROUGHICONS, b);

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
void InitColor(HWND hDlg)
{
	COLORREF col;
	int j;
	WORD id;
	//Windowsデフォルト16色
	int rgb[16][3] = {{0,0,0}, {128,0,0}, {0,128,0}, {128,128,0},
		{0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
		{128,128,128}, {255,0,0}, {0,255,0}, {255,255,0},
		{0,0,255},{255,0,255}, {0,255,255}, {255,255,255}};
	
	id = IDC_DESKTOPICONTEXT;

	for(j = 0; j < 16; j++) //基本16色
		CBAddString(hDlg, id,
			RGB(rgb[j][0], rgb[j][1], rgb[j][2]));
	//ボタンの...色
	CBAddString(hDlg, id, 0x80000000|COLOR_3DFACE);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DSHADOW);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DHILIGHT);
	CBAddString(hDlg, id, 0x80000000|COLOR_BTNTEXT);
	
	col = GetMyRegLong("Desktop", "DesktopTextColor",
		0x80000000 | COLOR_BTNTEXT);
	for(j = 0; j < 20; j++)
	{
		if(col == (COLORREF)CBGetItemData(hDlg, id, j))
			break;
	}
	if(j == 20) //20色中にないとき
		CBAddString(hDlg, id, col);
	CBSetCurSel(hDlg, id, j);
}

/*------------------------------------------------
　「色」コンボボックスの高さの設定
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pmis;
	
	pmis = (LPMEASUREITEMSTRUCT)lParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
　「色」コンボボックスのオーナードロー
--------------------------------------------------*/
void OnDrawItemColorCombo(LPARAM lParam)
{
	LPDRAWITEMSTRUCT pdis;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	int y;

	pdis = (LPDRAWITEMSTRUCT)lParam;
	
	if(IsWindowEnabled(pdis->hwndItem))
	{
		col = pdis->itemData;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	}
	else col = col = GetSysColor(COLOR_3DFACE);
	
	switch(pdis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			hbr = CreateSolidBrush(col);
			FillRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);

			// print color names
			if(16 <= pdis->itemID && pdis->itemID <= 19)
			{
				char s[80];
				
				strcpy(s, MyString(IDS_BTNFACE + pdis->itemID - 16));
				SetBkMode(pdis->hDC, TRANSPARENT);
				GetTextMetrics(pdis->hDC, &tm);
				if(pdis->itemID == 19)
					SetTextColor(pdis->hDC, RGB(255,255,255));
				else
					SetTextColor(pdis->hDC, RGB(0,0,0));
				y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
				TextOut(pdis->hDC, pdis->rcItem.left + 4, pdis->rcItem.top + y,
					s, strlen(s));
			}
			if(!(pdis->itemState & ODS_FOCUS)) break;
		}
		case ODA_FOCUS:
		{
			if(pdis->itemState & ODS_FOCUS)
				hbr = CreateSolidBrush(0);
			else
				hbr = CreateSolidBrush(col);
			FrameRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			break;
		}
	}
}

void OnChooseColor(HWND hDlg, WORD id)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	WORD idCombo;
	int i;
	
	idCombo = IDC_DESKTOPICONTEXT; //id - 1;
	
	//最初に選ばれている色
	col = CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo));
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	
	for(i = 0; i < 16; i++) colarray[i] = RGB(255,255,255);
	
	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.hInstance = g_hInst;
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	
	if(!ChooseColor(&cc)) return;
	
	for(i = 0; i < 16; i++)
	{
		if(cc.rgbResult == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	if(i == 16) //基本16色ではないとき
	{
		if(CBGetCount(hDlg, idCombo) == 20)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, 20, cc.rgbResult);
		i = 20;
	}
	CBSetCurSel(hDlg, idCombo, i);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

void OnSanshoMenu(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	filter[0] = filter[1] = 0;
	str0cat(filter, "Program Files"); str0cat(filter, "*.exe;*.lnk");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}
