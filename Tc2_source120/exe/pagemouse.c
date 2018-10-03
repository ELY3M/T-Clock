/*-------------------------------------------
  pagemouse.c
　　「マウス操作」プロパティページ
　　KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnDropFilesChange(HWND hDlg);
static void OnMouseButton(HWND hDlg);
static void OnMouseClickTime(HWND hDlg, int id);
static void OnMouseFunc(HWND hDlg);
static void OnMouseFileChange(HWND hDlg);
static void OnSansho(HWND hDlg, WORD id);
static void InitMouseFuncList(HWND hDlg);

static char reg_section[] = "Mouse";

typedef struct {
	BOOL disable;
	int func[4];
	char format[4][256];
	char fname[4][256];
} CLICKDATA;
static CLICKDATA *pData = NULL;

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

/*------------------------------------------------
　「マウス操作」ページ用ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK PageMouseProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
			// "Drop files"
			if(id == IDC_DROPFILES && code == CBN_SELCHANGE)
			{
				OnDropFilesChange(hDlg);
				g_bApplyClock = TRUE;
			}
			else if(id == IDC_DROPFILESAPP && code == EN_CHANGE)
				SendPSChanged(hDlg);
			// "..."
			else if(id == IDC_DROPFILESAPPSANSHO ||
				id == IDC_MOUSEFILESANSHO)
				OnSansho(hDlg, id);
			//  "Button"
			else if(id == IDC_MOUSEBUTTON && code == CBN_SELCHANGE)
				OnMouseButton(hDlg);
			// single .... quadruple
			else if(IDC_RADSINGLE <= id && id <= IDC_RADQUADRUPLE)
				OnMouseClickTime(hDlg, id);
			//  Mouse Function
			else if(id == IDC_MOUSEFUNC && code == CBN_SELCHANGE)
			{
				OnMouseFunc(hDlg);
				SendPSChanged(hDlg);
			}
			// Mouse Function - File
			else if(id == IDC_MOUSEFILE && code == EN_CHANGE)
			{
				OnMouseFileChange(hDlg);
				SendPSChanged(hDlg);
			}
			else if(id == IDC_TOOLTIP && code == EN_CHANGE)
			{
				g_bApplyClock = TRUE; SendPSChanged(hDlg);
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				//case PSN_HELP: MyHelp(GetParent(hDlg), 4); break;
			}
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
　ページの初期化
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[256];
	char entry[20];
	BOOL b;
	int i, j;
	HWND hDlgPage;
	HFONT hfont;
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_DROPFILESAPP,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_MOUSEFILE,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TOOLTIP,
			WM_SETFONT, (WPARAM)hfont, 0);
	}
	
	for(i = IDS_NONE; i <= IDS_MOVETO; i++)
		CBAddString(hDlg, IDC_DROPFILES, (LPARAM)MyString(i));
	CBSetCurSel(hDlg, IDC_DROPFILES, 
		GetMyRegLong(reg_section, "DropFiles", 0));
	GetMyRegStr(reg_section, "DropFilesApp", s, 256, "");
	SetDlgItemText(hDlg, IDC_DROPFILESAPP, s);
	
	pData = malloc(sizeof(CLICKDATA) * 5);
	
	for(i = 0; i < 5; i++)
	{
		if(i == 1) continue;
		for(j = 0; j < 4; j++)
		{
			wsprintf(entry, "%d%d", i, j+1);
			pData[i].disable = FALSE;
			pData[i].func[j] = 
				GetMyRegLong(reg_section, entry, MOUSEFUNC_NONE);
			pData[i].format[j][0] = 0;
			pData[i].fname[j][0] = 0;
			if(pData[i].func[j] == MOUSEFUNC_CLIPBOARD)
			{
				wsprintf(entry, "%d%dClip", i, j+1);
				GetMyRegStr(reg_section, entry, pData[i].format[j], 256, "");
			}
			else if(pData[i].func[j] == MOUSEFUNC_OPENFILE)
			{
				wsprintf(entry, "%d%dFile", i, j+1);
				GetMyRegStr(reg_section, entry, pData[i].fname[j], 256, "");
			}
		}
	}
	
	//b = GetMyRegLong("StartButton", "StartButtonHide", FALSE);
	b = GetMyRegLong("StartMenu", "StartMenuClock", FALSE);
	// 「タスクバー」ページの「時計にスタートメニュー」が
	// チェックされているかどうか
	hDlgPage = GetTopWindow(GetParent(hDlg));
	while(hDlgPage)
	{
		if(GetDlgItem(hDlgPage, IDC_STARTBTNHIDE))
		{
			//b = IsDlgButtonChecked(hDlgPage, IDC_STARTBTNHIDE);
			b = IsDlgButtonChecked(hDlgPage, IDC_STARTMENUCLOCK);
			break;
		}
		hDlgPage = GetNextWindow(hDlgPage, GW_HWNDNEXT);
	}
	pData[0].disable = b;
	
	for(i = IDS_LEFTBUTTON; i <= IDS_XBUTTON2; i++)
		CBAddString(hDlg, IDC_MOUSEBUTTON, (LPARAM)MyString(i));
	
	// set mouse functions to combo box
	InitMouseFuncList(hDlg);
	
	OnDropFilesChange(hDlg);
	CBSetCurSel(hDlg, IDC_MOUSEBUTTON, 0);
	OnMouseButton(hDlg);
	
	GetMyRegStr("Tooltip", "Tooltip", s, 256, "");
	if(s[0] == 0) strcpy(s, "\"TClock\" LDATE");
	SetDlgItemText(hDlg, IDC_TOOLTIP, s);
}

/*------------------------------------------------
　更新
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[256], entry[20];
	int n;
	int i, j;
	
	n = CBGetCurSel(hDlg, IDC_DROPFILES);
	SetMyRegLong("", "DropFiles", n > 0);
	SetMyRegLong(reg_section, "DropFiles", n);
	GetDlgItemText(hDlg, IDC_DROPFILESAPP, s, 256);
	SetMyRegStr(reg_section, "DropFilesApp", s);
	
	for(i = 0; i < 5; i++)
	{
		if(i == 1) continue;
		for(j = 0; j < 4; j++)
		{
			wsprintf(entry, "%d%d", i, j+1);
			if(pData[i].func[j] >= 0)
				SetMyRegLong(reg_section, entry, pData[i].func[j]);
			else DelMyReg(reg_section, entry);
			if(pData[i].func[j] == MOUSEFUNC_CLIPBOARD)
			{
				wsprintf(entry, "%d%dClip", i, j+1);
				SetMyRegStr(reg_section, entry, pData[i].format[j]);
			}
			else if(pData[i].func[j] == MOUSEFUNC_OPENFILE)
			{
				wsprintf(entry, "%d%dFile", i, j+1);
				SetMyRegStr(reg_section, entry, pData[i].fname[j]);
			}
		}
	}
	
	GetDlgItemText(hDlg, IDC_TOOLTIP, s, 256);
	SetMyRegStr("Tooltip", "Tooltip", s);
}

/*------------------------------------------------
    
--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	if(pData) free(pData);
}

/*------------------------------------------------
　「ファイルのドロップ」
--------------------------------------------------*/
void OnDropFilesChange(HWND hDlg)
{
	int i, n;
	n = CBGetCurSel(hDlg, IDC_DROPFILES);
	SetDlgItemText(hDlg, IDC_LABDROPFILESAPP,
		MyString(n >= 3?IDS_LABFOLDER:IDS_LABPROGRAM));
	for(i = IDC_LABDROPFILESAPP; i <= IDC_DROPFILESAPPSANSHO; i++)
		ShowDlgItem(hDlg, i, (2 <= n && n <= 4));
	
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  "Button"
--------------------------------------------------*/
void OnMouseButton(HWND hDlg)
{
	int n, button, j;
	
	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;
	if(n > 0) button = n + 1;
	
	for(j = 0; j < 4; j++)
	{
		if(pData[button].func[j] >= 0) break;
	}
	if(j == 4) j = 0;
	CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE,
		IDC_RADSINGLE + j);
	OnMouseClickTime(hDlg, IDC_RADSINGLE + j);
}

/*------------------------------------------------
  "Single" ... "Quadruple"
--------------------------------------------------*/
void OnMouseClickTime(HWND hDlg, int id)
{
	int n, button;
	int click, i, count, func;
	
	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;
	if(n > 0) button = n + 1;
	
	click = id - IDC_RADSINGLE;
	func = pData[button].func[click];
	
	count = CBGetCount(hDlg, IDC_MOUSEFUNC);
	for(i = 0; i < count; i++)
	{
		if(func == CBGetItemData(hDlg, IDC_MOUSEFUNC, i))
		{
			CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
			break;
		}
	}
	
	OnMouseFunc(hDlg);
}

/*------------------------------------------------
  Mouse Functions combo box
--------------------------------------------------*/
void OnMouseFunc(HWND hDlg)
{
	int n, button, j;
	int click, index, func;
	
	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;
	if(n > 0) button = n + 1;
	
	for(j = 0; j < 4; j++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j))
			break;
	}
	if(j == 4) return;
	click = j;
	
	index = CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = CBGetItemData(hDlg, IDC_MOUSEFUNC, index);
	pData[button].func[click] = func;
	
	ShowDlgItem(hDlg, IDC_LABMOUSEFILE,
		(func == MOUSEFUNC_CLIPBOARD||func == MOUSEFUNC_OPENFILE));
	ShowDlgItem(hDlg, IDC_MOUSEFILE,
		(func == MOUSEFUNC_CLIPBOARD||func == MOUSEFUNC_OPENFILE));
	ShowDlgItem(hDlg, IDC_MOUSEFILESANSHO, func == MOUSEFUNC_OPENFILE);
	
	if(func == MOUSEFUNC_CLIPBOARD)
	{
		SetDlgItemText(hDlg, IDC_LABMOUSEFILE, MyString(IDS_FORMAT));
		if(pData[button].format[click][0] == 0)
			GetMyRegStr("Format", "Format", pData[button].format[click], 256, "");
		SetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].format[click]);
	}
	else if(func == MOUSEFUNC_OPENFILE)
	{
		SetDlgItemText(hDlg, IDC_LABMOUSEFILE, MyString(IDS_FILE));
		SetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].fname[click]);
	}
}

/*------------------------------------------------
  Format/File - Mouse Function
--------------------------------------------------*/
void OnMouseFileChange(HWND hDlg)
{
	int n, button, j;
	int click, index, func;
	
	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;
	if(n > 0) button = n + 1;
	
	for(j = 0; j < 4; j++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j))
			break;
	}
	if(j == 4) return;
	click = j;
	
	index = CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = CBGetItemData(hDlg, IDC_MOUSEFUNC, index);
	
	if(func == MOUSEFUNC_CLIPBOARD)
		GetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].format[click], 1024);
	else if(func == MOUSEFUNC_OPENFILE)
		GetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].fname[click], 1024);
}

/*------------------------------------------------
　「...」　ファイルの参照
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	int n;
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	if(id == IDC_DROPFILESAPPSANSHO)
	{
		n = CBGetCurSel(hDlg, IDC_DROPFILES);
		if(n >= 3)
		{
			BROWSEINFO bi;
			LPITEMIDLIST pidl;
			memset(&bi, 0, sizeof(BROWSEINFO));
			bi.hwndOwner = hDlg;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			pidl = SHBrowseForFolder(&bi);
			if(pidl)
			{
				SHGetPathFromIDList(pidl, fname);
				SetDlgItemText(hDlg, id - 1, fname);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}
	
	filter[0] = 0;
	if(id == IDC_DROPFILESAPPSANSHO)
	{
		str0cat(filter, MyString(IDS_PROGRAMFILE));
		str0cat(filter, "*.exe");
	}
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  set mouse functions to combo box
--------------------------------------------------*/
void InitMouseFuncList(HWND hDlg)
{
	int index;
	
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_NONE));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_NONE);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_PROPDATE));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_DATETIME);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_EXITWIN));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_EXITWIN);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_RUN));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_RUNAPP);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_ALLMIN));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_MINALL);
	index = CBAddString(hDlg, IDC_MOUSEFUNC,(LPARAM)MyString(IDS_SCREENSAVER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SCREENSAVER);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_SYNCTIME));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SYNCTIME);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_TIMER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_TIMER);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_MOUSECOPY));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_CLIPBOARD);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_OPENFILE));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_OPENFILE);
	index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(IDS_SHOWCALENDER));
	CBSetItemData(hDlg, IDC_MOUSEFUNC, index, MOUSEFUNC_SHOWCALENDER);

}

