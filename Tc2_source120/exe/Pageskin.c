/*-------------------------------------------
  pageskin.c
  "skin" page of properties
  Kazubon 1997-2001
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnStartBtn(HWND hDlg);
static void OnStartBtnStyle(HWND hDlg);
static void OnStartBtnHide(HWND hDlg);
static void OnSansho(HWND hDlg, WORD id);
static BOOL SelectIconInDLL(HWND hDlg, char* fname);
static void InitColor(HWND hDlg);
static void OnMeasureItemColorCombo(LPARAM lParam);
static void OnDrawItemColorCombo(LPARAM lParam);
static void OnChooseColor(HWND hDlg, WORD id);
static int nFilterIndex = 1;
static void InitComboFont(HWND hDlg);
static void SetComboFontSize(HWND hDlg, int bInit);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
BOOL CALLBACK PageSkinProc(HWND hDlg, UINT message,
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
			// checked "Hide Start Button", "Start menu from Clock"
			if(id == IDC_STARTBTNHIDE || id == IDC_STARTMENUCLOCK 
				|| id == IDC_STARTICONLARGE)
				SendPSChanged(hDlg);
			// 「スタートボタンを改造する」
			if(id == IDC_STARTBTN)
				OnStartBtn(hDlg);
			// 「ビットマップ」「キャプション」テキストボックス
			else if((id == IDC_FILESTART || id == IDC_CAPTIONSTART ||
				id == IDC_FILEMENU)
				&& code == EN_CHANGE)
				SendPSChanged(hDlg);
			// 「...」 スタートメニューのアイコン参照
			else if(id == IDC_SANSHOSTART)
				OnSansho(hDlg, id);
			// 「...」 スタートメニューのビットマップ参照
			else if(id == IDC_STARTBTNSTYLE && code == CBN_SELCHANGE)
				OnStartBtnStyle(hDlg);
			else if((id == IDC_STARTFONT || id == IDC_STARTFONTSIZE) && code == CBN_SELCHANGE)
			{
				if(id == IDC_STARTFONT) SetComboFontSize(hDlg, FALSE);
				SendPSChanged(hDlg);
			}
			else if(id == IDC_STARTBOLD || id == IDC_STARTITALIC)
				SendPSChanged(hDlg);
			else if((id == IDC_MENUSTARTHILIGHT || id == IDC_MENUSTARTLIGHT || 
				id == IDC_MENUSTART3DSHADOW || id == IDC_MENUSTART3DDKSHADOW || 
				id == IDC_MENUSTARTFACE || id == IDC_STARTCOLFORE) && code == CBN_SELCHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_SANCHOSTARTHILIGHT || id == IDC_SANCHOSTARTLIGHT || 
				id == IDC_SANCHOSTART3DSHADOW || id == IDC_SANCHOSTART3DDKSHADOW || 
				id == IDC_SANCHOSTARTFACE || id == IDC_STARTCHOOSECOLFORE)
				OnChooseColor(hDlg, id);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				//case PSN_HELP: MyHelp(GetParent(hDlg), 5); break;
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
	char s[1024];
	HFONT hfont;
	LOGFONT logfont;
	int i;

	InitColor(hDlg);

	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_FILESTART,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_CAPTIONSTART,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_FILEMENU,
			WM_SETFONT, (WPARAM)hfont, 0);
	}

	CheckDlgButton(hDlg, IDC_STARTBTNHIDE,
		GetMyRegLong("StartButton", "StartButtonHide", FALSE));
	CheckDlgButton(hDlg, IDC_STARTMENUCLOCK,
		GetMyRegLong("StartMenu", "StartMenuClock", FALSE));
	
	CheckDlgButton(hDlg, IDC_STARTBTN,
		GetMyRegLong("StartButton", "CustomizeStartButton", FALSE));
	
	GetMyRegStr("StartButton", "StartButtonIcon", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FILESTART, s);
	
	CheckDlgButton(hDlg, IDC_STARTICONLARGE,
		GetMyRegLong("StartButton", "StartButtonIconLarge", FALSE));

	GetMyRegStr("StartButton", "StartButtonCaption", s, 80, MyString(IDS_START));
	SetDlgItemText(hDlg, IDC_CAPTIONSTART, s);

	for(i = 46; i <= 49; i++)
		CBAddString(hDlg, IDC_STARTBTNSTYLE, (LPARAM)MyString(i));
	GetMyRegStr("StartButton", "StartButtonType", s, 80, MyString(46));
	if(_strnicmp(s, "FLAT", 4) == 0)
		CBSetCurSel(hDlg, IDC_STARTBTNSTYLE, 1);
	else if(_strnicmp(s, "COLOR", 5) == 0)
		CBSetCurSel(hDlg, IDC_STARTBTNSTYLE, 2);
	else if(_strnicmp(s, "SKIN", 4) == 0)
		CBSetCurSel(hDlg, IDC_STARTBTNSTYLE, 3);
	else
		CBSetCurSel(hDlg, IDC_STARTBTNSTYLE, 0);
	
	//「フォント」の設定
	InitComboFont(hDlg);
	//「フォントサイズ」の設定
	SetComboFontSize(hDlg, TRUE);
		
	//「Bold」「Italic」の設定
	CheckDlgButton(hDlg, IDC_STARTBOLD,
		GetMyRegLong("StartButton", "Bold", FALSE));
	CheckDlgButton(hDlg, IDC_STARTITALIC, 
		GetMyRegLong("StartButton", "Italic", FALSE));
	
	hfontb = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfontb, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfontb = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_STARTBOLD, WM_SETFONT, (WPARAM)hfontb, 0);

	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfonti = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_STARTITALIC, WM_SETFONT, (WPARAM)hfonti, 0);

	OnStartBtn(hDlg);
	OnStartBtnStyle(hDlg);

}

/*------------------------------------------------
  apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];
	DWORD dw;

	SetMyRegLong("StartButton", "StartButtonHide",
		IsDlgButtonChecked(hDlg, IDC_STARTBTNHIDE));
	SetMyRegLong("StartMenu", "StartMenuClock",
		IsDlgButtonChecked(hDlg, IDC_STARTMENUCLOCK));

	SetMyRegLong("StartButton", "CustomizeStartButton", IsDlgButtonChecked(hDlg, IDC_STARTBTN));
	
	GetDlgItemText(hDlg, IDC_FILESTART, s, 1024);
	SetMyRegStr("StartButton", "StartButtonIcon", s);
	
	SetMyRegLong("StartButton", "StartButtonIconLarge",
		IsDlgButtonChecked(hDlg, IDC_STARTICONLARGE));

	GetDlgItemText(hDlg, IDC_CAPTIONSTART, s, 80);
	SetMyRegStr("StartButton", "StartButtonCaption", s);

	GetDlgItemText(hDlg, IDC_STARTBTNSTYLE, s, 1024);
	if(_strnicmp(s, "FLAT", 4) == 0)
		SetMyRegStr("StartButton", "StartButtonType", "Flat");
	else if(_strnicmp(s, "CUSTOM COLOR", 12) == 0)
		SetMyRegStr("StartButton", "StartButtonType", "Color");
	else if(_strnicmp(s, "SKINNED", 7) == 0)
		SetMyRegStr("StartButton", "StartButtonType", "Skin");
	else
		SetMyRegStr("StartButton", "StartButtonType", "Normal");

	dw = CBGetItemData(hDlg, IDC_MENUSTARTHILIGHT, CBGetCurSel(hDlg, IDC_MENUSTARTHILIGHT));
	SetMyRegLong("StartButton", "3DHILIGHTColor", dw);

	dw = CBGetItemData(hDlg, IDC_MENUSTARTLIGHT, CBGetCurSel(hDlg, IDC_MENUSTARTLIGHT));
	SetMyRegLong("StartButton", "3DLIGHTColor", dw);

	dw = CBGetItemData(hDlg, IDC_MENUSTART3DSHADOW, CBGetCurSel(hDlg, IDC_MENUSTART3DSHADOW));
	SetMyRegLong("StartButton", "3DSHADOWColor", dw);

	dw = CBGetItemData(hDlg, IDC_MENUSTART3DDKSHADOW, CBGetCurSel(hDlg, IDC_MENUSTART3DDKSHADOW));
	SetMyRegLong("StartButton", "3DDKSHADOWColor", dw);

	dw = CBGetItemData(hDlg, IDC_MENUSTARTFACE, CBGetCurSel(hDlg, IDC_MENUSTARTFACE));
	SetMyRegLong("StartButton", "3DFACEColor", dw);

	dw = CBGetItemData(hDlg, IDC_STARTCOLFORE, CBGetCurSel(hDlg, IDC_STARTCOLFORE));
	SetMyRegLong("StartButton", "TextColor", dw);

	CBGetLBText(hDlg, IDC_STARTFONT, CBGetCurSel(hDlg, IDC_STARTFONT), s);
	SetMyRegStr("StartButton", "Font", s);

	if(CBGetCount(hDlg, IDC_STARTFONTSIZE) > 0)
	{
		CBGetLBText(hDlg, IDC_STARTFONTSIZE, CBGetCurSel(hDlg, IDC_STARTFONTSIZE), s);
		SetMyRegLong("StartButton", "FontSize", atoi(s));
	}
	else SetMyRegLong("StartButton", "FontSize", 9);

	SetMyRegLong("StartButton", "Bold", IsDlgButtonChecked(hDlg, IDC_STARTBOLD));
	SetMyRegLong("StartButton", "Italic", IsDlgButtonChecked(hDlg, IDC_STARTITALIC));

}

/*------------------------------------------------
  checked "Hide start button"
--------------------------------------------------*/
void OnStartBtnHide(HWND hDlg)
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_STARTBTNHIDE);
	EnableDlgItem(hDlg, IDC_STARTMENUCLOCK, b);
	for(i = IDC_STARTBTN; i <= IDC_CAPTIONSTART; i++)
		EnableDlgItem(hDlg, i, !b);
	EnableDlgItem(hDlg, 1523, !b);
	EnableDlgItem(hDlg, 1524, !b);

	SendPSChanged(hDlg);
	
	if(b) b = IsDlgButtonChecked(hDlg, IDC_STARTMENUCLOCK);
}

/*------------------------------------------------
　「スタートボタンを改造する」
--------------------------------------------------*/
void OnStartBtn(HWND hDlg)
{
	BOOL b;
	int i;
	
	b = IsDlgButtonChecked(hDlg, IDC_STARTBTN);
	for(i = IDC_LABFILESTART; i <= IDC_CAPTIONSTART; i++)
		EnableDlgItem(hDlg, i, b);
	for(i = IDC_LABSTARTFACE; i <= IDC_STARTICONLARGE; i++)
		EnableDlgItem(hDlg, i, b);
	EnableDlgItem(hDlg, 1523, b);
	EnableDlgItem(hDlg, 1524, b);
	SendPSChanged(hDlg);
}

void OnStartBtnStyle(HWND hDlg)
{
	BOOL b;
	int i;
	char s[1024];

	GetDlgItemText(hDlg, IDC_STARTBTNSTYLE, s, 1024);
	if(_strnicmp(s, "CUSTOM COLOR", 12) == 0)
		b = TRUE;
	else
		b = FALSE;

	for(i = IDC_LABSTARTHILIGHT; i <= IDC_FRAMECUSTCOLOR; i++)
		EnableDlgItem(hDlg, i, b);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
　「...」　スタートボタンのアイコン参照
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	char filter[160], deffile[MAX_PATH], fname[MAX_PATH+10];
	char s[MAX_PATH+10], num[10];
	HFILE hf = HFILE_ERROR; char head[2];
	
	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_BMPICONFILE)); str0cat(filter, "*.bmp;*.ico");
	str0cat(filter, MyString(IDS_EXEDLLFILE)); str0cat(filter, "*.exe;*.dll;*.icl");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");
	
	deffile[0] = 0;
	GetDlgItemText(hDlg, id - 1, s, MAX_PATH);
	if(s[0])
	{
		parse(deffile, s, 0);
		parse(num, s, 1);
		hf = _lopen(deffile, OF_READ);
	}
	if(hf != HFILE_ERROR)
	{
		_lread(hf, head, 2);
		_lclose(hf);
		
		if(head[0] == 'M' && head[1] == 'Z') //実行ファイル
		{
			// 「アイコンの選択」ダイアログ
			if(SelectIconInDLL(hDlg, deffile))
			{
				SetDlgItemText(hDlg, id - 1, deffile);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return;
	_lread(hf, head, 2);
	_lclose(hf);
	if(head[0] == 'M' && head[1] == 'Z') //実行ファイル
	{
		// 「アイコンの選択」ダイアログ
		if(!SelectIconInDLL(hDlg, fname)) return;
	}
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

//------------------------------------------------------------------
// 以下、アイコン選択ダイアログ

BOOL CALLBACK DlgProcSelectIcon(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam);
char* fname_SelectIcon;

/*-----------------------------------------------------------------
　アイコンの選択
　fnameは、
　　実行ファイルのとき、"A:\WINDOWS\SYSTEM\SHELL32.DLL,8"
	それ以外のとき、　　"C:\MY PROGRAM\TCLOCK\NIKO.BMP" などとなる
-------------------------------------------------------------------*/
BOOL SelectIconInDLL(HWND hDlg, char* fname)
{
	fname_SelectIcon = fname;
	if(DialogBox(GetLangModule(), MAKEINTRESOURCE(118),
		hDlg, DlgProcSelectIcon) != IDOK) return FALSE;
	return TRUE;
}

static BOOL InitSelectIcon(HWND hDlg);
static void EndSelectIcon(HWND hDlg);
static void OnOKSelectIcon(HWND hDlg);
static void OnMeasureItemListSelectIcon(HWND hDlg, LPARAM lParam);
static void OnDrawItemListSelectIcon(LPARAM lParam);
static void OnSanshoSelectIcon(HWND hDlg);

/*------------------------------------------------
　アイコン選択ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK DlgProcSelectIcon(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			if(!InitSelectIcon(hDlg))
				EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case WM_MEASUREITEM:
			OnMeasureItemListSelectIcon(hDlg, lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItemListSelectIcon(lParam);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			if(id == IDC_SANSHOICON) OnSanshoSelectIcon(hDlg);
			else if(id == IDOK || id == IDCANCEL)
			{
				if(id == IDOK) OnOKSelectIcon(hDlg);
				EndSelectIcon(hDlg);
				EndDialog(hDlg, id);
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
　アイコン選択ダイアログの初期化
--------------------------------------------------*/
BOOL InitSelectIcon(HWND hDlg)
{
	int i, count, index;
	HICON hicon, hiconl;
	char msg[MAX_PATH];
	char fname[MAX_PATH], num[10];
	
	parse(fname, fname_SelectIcon, 0);
	parse(num, fname_SelectIcon, 1);
	if(num[0] == 0) index = 0;
	else index = atoi(num);
	
	count = (int)ExtractIcon(g_hInst, fname, (UINT)-1);
	if(count == 0)
	{
		strcpy(msg, MyString(IDS_NOICON));
		strcat(msg, "\n");
		strcat(msg, fname);
		MyMessageBox(hDlg, msg, "TClock", MB_OK, MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	EndSelectIcon(hDlg);
	SendDlgItemMessage(hDlg, IDC_LISTICON, LB_RESETCONTENT, 0, 0);
	
	for(i = 0; i < count; i++)
	{
		hiconl = NULL; hicon = NULL;
		ExtractIconEx(fname, i, &hiconl, &hicon, 1);
		if(hiconl) DestroyIcon(hiconl);
		SendDlgItemMessage(hDlg, IDC_LISTICON, LB_ADDSTRING, 0,
			(LPARAM)hicon);
	}
	SetDlgItemText(hDlg, IDC_FNAMEICON, fname);
	SendDlgItemMessage(hDlg, IDC_LISTICON, LB_SETCURSEL,
		index, 0);
	strcpy(fname_SelectIcon, fname);
	return TRUE;
}

/*------------------------------------------------
　アイコン選択ダイアログの後始末
--------------------------------------------------*/
void EndSelectIcon(HWND hDlg)
{
	int i, count;
	HICON hicon;
	count = SendDlgItemMessage(hDlg, IDC_LISTICON, LB_GETCOUNT, 0, 0);
	for(i = 0; i < count; i++)
	{
		hicon = (HICON)SendDlgItemMessage(hDlg, IDC_LISTICON,
			LB_GETITEMDATA, i, 0);
		if(hicon) DestroyIcon(hicon);
	}
}

/*------------------------------------------------
　アイコン選択ダイアログの「OK」
--------------------------------------------------*/
void OnOKSelectIcon(HWND hDlg)
{
	char num[10];
	int index;
	
	GetDlgItemText(hDlg, IDC_FNAMEICON, fname_SelectIcon, MAX_PATH);
	index = SendDlgItemMessage(hDlg, IDC_LISTICON, LB_GETCURSEL, 0, 0);
	wsprintf(num, ",%d", index);
	strcat(fname_SelectIcon, num);
}

/*------------------------------------------------
　アイコンリストのサイズを決める
--------------------------------------------------*/
void OnMeasureItemListSelectIcon(HWND hDlg, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pMis;
	RECT rc;

	pMis = (LPMEASUREITEMSTRUCT)lParam;
	GetClientRect(GetDlgItem(hDlg, pMis->CtlID), &rc);
	pMis->itemHeight = rc.bottom;
	pMis->itemWidth = 32;
}

/*------------------------------------------------
　アイコンリストの描画
--------------------------------------------------*/
void OnDrawItemListSelectIcon(LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDis;
	HBRUSH hbr;
	COLORREF col;
	RECT rc;
	int cxicon, cyicon;

	pDis = (LPDRAWITEMSTRUCT)lParam;
	
	switch(pDis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			if(pDis->itemState & ODS_SELECTED)
				col = GetSysColor(COLOR_HIGHLIGHT);
			else col = GetSysColor(COLOR_WINDOW);
			hbr = CreateSolidBrush(col);
			FillRect(pDis->hDC, &pDis->rcItem, hbr);
			DeleteObject(hbr);
			if(!(pDis->itemState & ODS_FOCUS)) break;
		}
		case ODA_FOCUS:
		{
			if(pDis->itemState & ODS_FOCUS)
				col = GetSysColor(COLOR_WINDOWTEXT);
			else
				col = GetSysColor(COLOR_WINDOW);
			hbr = CreateSolidBrush(col);
			FrameRect(pDis->hDC, &pDis->rcItem, hbr);
			DeleteObject(hbr);
			break;
		}
	}
	
	if(pDis->itemData == 0) return;
	
	cxicon = GetSystemMetrics(SM_CXSMICON);
	cyicon = GetSystemMetrics(SM_CYSMICON);

	CopyRect(&rc, &(pDis->rcItem));
	DrawIconEx(pDis->hDC,
		rc.left + (rc.right - rc.left - cxicon)/2,
		rc.top + (rc.bottom - rc.top - cyicon)/2,
		(HICON)pDis->itemData,
		cxicon, cyicon, 0, NULL, DI_NORMAL);
}

/*------------------------------------------------
　アイコンの選択の中のファイルの参照
--------------------------------------------------*/
void OnSanshoSelectIcon(HWND hDlg)
{
	char filter[160], deffile[MAX_PATH], fname[MAX_PATH];
	HFILE hf = HFILE_ERROR;
	char head[2];
	
	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_BMPICONFILE)); str0cat(filter, "*.bmp;*.ico");
	str0cat(filter, MyString(IDS_EXEDLLFILE)); str0cat(filter, "*.exe;*.dll;*.icl");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, IDC_FNAMEICON, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 2, deffile, fname))
		return;
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return;
	_lread(hf, head, 2);
	_lclose(hf);
	strcpy(fname_SelectIcon, fname);
	
	if(head[0] == 'M' && head[1] == 'Z') //実行ファイル
	{
		if(InitSelectIcon(hDlg))
			PostMessage(hDlg, WM_NEXTDLGCTL,
				(WPARAM)GetDlgItem(hDlg, IDC_LISTICON), TRUE);
	}
	else
	{
		EndSelectIcon(hDlg);
		EndDialog(hDlg, IDOK);
	}
}
void InitColor(HWND hDlg)
{
	COLORREF col;
	int i, j;
	WORD id;
	//Windowsデフォルト16色
	int rgb[16][3] = {{0,0,0}, {128,0,0}, {0,128,0}, {128,128,0},
		{0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
		{128,128,128}, {255,0,0}, {0,255,0}, {255,255,0},
		{0,0,255},{255,0,255}, {0,255,255}, {255,255,255}};
	
	for(i = 0; i < 6; i++)
	{
		if(i == 0) id = IDC_MENUSTARTHILIGHT;
		else if(i == 1) id = IDC_MENUSTARTLIGHT;
		else if(i == 2) id = IDC_MENUSTART3DSHADOW;
		else if(i == 3) id = IDC_MENUSTART3DDKSHADOW;
		else if(i == 4) id = IDC_MENUSTARTFACE;
		else if(i == 5) id = IDC_STARTCOLFORE;

		for(j = 0; j < 16; j++) //基本16色
			CBAddString(hDlg, id,
				RGB(rgb[j][0], rgb[j][1], rgb[j][2]));
		//ボタンの...色
		CBAddString(hDlg, id, 0x80000000|COLOR_3DHILIGHT);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DLIGHT);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DSHADOW);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DDKSHADOW);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DFACE);
		CBAddString(hDlg, id, 0x80000000|COLOR_BTNTEXT);

		//コンボボックスの色を選択
		if(i == 0)
			col = GetMyRegLong("StartButton", "3DHILIGHTColor",
				0x80000000 | COLOR_3DHILIGHT);
		else if(i == 1)
			col = GetMyRegLong("StartButton", "3DLIGHTColor", 0x80000000 | COLOR_3DLIGHT);
		else if(i == 2)
			col = GetMyRegLong("StartButton", "3DSHADOWColor", 0x80000000 | COLOR_3DSHADOW);
		else if(i == 3)
			col = GetMyRegLong("StartButton", "3DDKSHADOWColor", 0x80000000 | COLOR_3DDKSHADOW);
		else if(i == 4)
			col = GetMyRegLong("StartButton", "3DFACEColor", 0x80000000 | COLOR_3DFACE);
		else if(i == 5)
			col = GetMyRegLong("StartButton", "TextColor", 0x80000000 | COLOR_BTNTEXT);

		for(j = 0; j < 22; j++)
		{
			if(col == (COLORREF)CBGetItemData(hDlg, id, j))
				break;
		}
		if(j == 22) //20色中にないとき
			CBAddString(hDlg, id, col);
		CBSetCurSel(hDlg, id, j);
	}
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
			if(16 <= pdis->itemID && pdis->itemID <= 21)
			{
				char s[80];
				
				strcpy(s, MyString(IDS_3DHILIGHT + pdis->itemID - 16));
				SetBkMode(pdis->hDC, TRANSPARENT);
				GetTextMetrics(pdis->hDC, &tm);
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

/*------------------------------------------------
　色の選択 「...」ボタン
--------------------------------------------------*/
void OnChooseColor(HWND hDlg, WORD id)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	WORD idCombo;
	int i;
	
	idCombo = id - 1;
	
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
		if(CBGetCount(hDlg, idCombo) == 21)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, 21, cc.rgbResult);
		i = 21;
	}
	CBSetCurSel(hDlg, idCombo, i);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

BOOL CALLBACK EnumStartFontFamExProc(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
BOOL CALLBACK EnumStartSizeProcEx(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
int nStartFontSizes[] = 
	{4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
int logpixelsy;

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	char s[80];
	int i;
	
	hdc = GetDC(NULL);
	
	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, IDC_STARTFONT);
	lf.lfCharSet = GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumStartFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumStartFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumStartFontFamExProc, (LPARAM)hcombo, 0);
	ReleaseDC(NULL, hdc);
	
	GetMyRegStr("StartButton", "Font", s, 80, "");
	if(s[0] == 0)
	{
		HFONT hfont;
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if(hfont)
		{
			GetObject(hfont, sizeof(lf),(LPVOID)&lf);
			strcpy(s, lf.lfFaceName);
		}
	}
	i = CBFindStringExact(hDlg, IDC_STARTFONT, s);
	if(i == LB_ERR) i = 0;
	CBSetCurSel(hDlg, IDC_STARTFONT, i);
}

/*------------------------------------------------
　「フォントサイズ」コンボボックスの設定
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char s[160];
	DWORD size;
	LOGFONT lf;
	int i;

	//以前のsizeを保存
	if(bInit) // WM_INITDIALOGのとき
	{
		size = GetMyRegLong("StartButton", "FontSize", 9);
		if(size == 0) size = 9;
	}
	else   // IDC_FONTが変更されたとき
	{
		CBGetLBText(hDlg, IDC_STARTFONTSIZE,
			CBGetCurSel(hDlg, IDC_STARTFONTSIZE), (LPARAM)s);
		size = atoi(s);
	}
	
	CBResetContent(hDlg, IDC_STARTFONTSIZE);
	
	hdc = GetDC(NULL);
	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	// s = フォント名
	CBGetLBText(hDlg, IDC_STARTFONT, CBGetCurSel(hDlg, IDC_STARTFONT), (LPARAM)s);
	
	//フォントのサイズを列挙してコンボボックスに入れる
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, s);
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_STARTFONT, CBGetCurSel(hDlg, IDC_STARTFONT));
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumStartSizeProcEx,
		(LPARAM)GetDlgItem(hDlg, IDC_STARTFONTSIZE), 0);
	
	//EnumFontFamilies(hdc, s,
	//	(FONTENUMPROC)EnumStartSizeProc, (LPARAM)GetDlgItem(hDlg, IDC_STARTFONTSIZE));
	
	ReleaseDC(NULL, hdc);
	
	// sizeに近いものを選択
	for(; size > 0; size--)
	{
		wsprintf(s, "%d", size);
		i = CBFindStringExact(hDlg, IDC_STARTFONTSIZE, s);
		if(i != LB_ERR)
		{
			CBSetCurSel(hDlg, IDC_STARTFONTSIZE, i); return;
		}
	}
	CBSetCurSel(hDlg, IDC_STARTFONTSIZE, 0);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
BOOL CALLBACK EnumStartFontFamExProc(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	// if(FontType & RASTER_FONTTYPE) return 1;
	if(pelf->elfLogFont.lfFaceName[0] != '@' && 
		SendMessage((HWND)hCombo, CB_FINDSTRINGEXACT, 0, 
			(LPARAM)pelf->elfLogFont.lfFaceName) == LB_ERR)
	{
		int index;
		index = SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)pelf->elfLogFont.lfFaceName);
		if(index >= 0)
			SendMessage((HWND)hCombo, CB_SETITEMDATA,
				index, (LPARAM)pelf->elfLogFont.lfCharSet);
	}
	return 1;
}

/*------------------------------------------------
　フォントの列挙コールバック
　コンボボックスにフォントサイズを入れる
--------------------------------------------------*/
BOOL CALLBACK EnumStartSizeProcEx(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	char s[80];
	int num, i, count;
	
	//トゥルータイプフォントまたは、
	//トゥルータイプでもラスタフォントでもない場合
	if((FontType & TRUETYPE_FONTTYPE) || 
		!( (FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE) ))
	{
		// nFontSizesの数字をそのまま入れる
		for (i = 0; i < 20; i++)
		{
			wsprintf(s, "%d", nStartFontSizes[i]);
			SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
		}
		return FALSE;
	}
	
	//それ以外の場合、１つ１つ数字を入れていく
	num = (lpntm->ntmTm.tmHeight - lpntm->ntmTm.tmInternalLeading) * 72 / logpixelsy;
	count = SendMessage((HWND)hCombo, CB_GETCOUNT, 0, 0);
	for(i = 0; i < count; i++)
	{
		SendMessage((HWND)hCombo, CB_GETLBTEXT, i, (LPARAM)s);
		if(num == atoi(s)) return TRUE;
		else if(num < atoi(s))
		{
			wsprintf(s, "%d", num);
			SendMessage((HWND)hCombo, CB_INSERTSTRING, i, (LPARAM)s);
			return TRUE;
		}
	}
	wsprintf(s, "%d", num);
	SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
	return TRUE;
}