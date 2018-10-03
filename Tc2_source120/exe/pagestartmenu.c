/*-------------------------------------------
  pageskin.c
  "skin" page of properties
  Kazubon 1997-2001
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnSanshoMenu(HWND hDlg, WORD id);
static void OnStartMenu(HWND hDlg);
static void OnStartMenuEdgeScale(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnMeasureItemColorCombo(LPARAM lParam);
static void OnDrawItemColorCombo(LPARAM lParam);
static void OnChooseColor(HWND hDlg, WORD id);
static void SetColorFromBmp(HWND hDlg, int idCombo, char* fname);
static int nFilterIndex = 1;
extern BOOL bWin98;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
   dialog procedure of this page
--------------------------------------------------*/
BOOL CALLBACK PageStartMenuProc(HWND hDlg, UINT message,
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
			// 「...」 スタートメニューのビットマップ参照
			if(id == IDC_SANSHOMENU)
				OnSanshoMenu(hDlg, id);
			// 「スタートメニューを改造する」
			else if(id == IDC_STARTMENU)
				OnStartMenu(hDlg);
			// コンボボックス
			else if(id == IDC_COLMENU && code == CBN_SELCHANGE)
				SendPSChanged(hDlg);
			//「...」色の選択
			else if(id == IDC_CHOOSECOLMENU)
				OnChooseColor(hDlg, id);
			else if(id == IDC_MENUSTYLEMENU && code == CBN_SELCHANGE) // 「並べる」
				SendPSChanged(hDlg);
			else if(id == IDC_MENUEDGESCALE)
				OnStartMenuEdgeScale(hDlg);
			else if(id == IDC_MENUEDGETOP && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_MENUEDGEBOTTOM && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_BANNERSMALLICONS)
				SendPSChanged(hDlg);
			else if(id == IDC_NOBANNERLARGEICONS)
				SendPSChanged(hDlg);
			else if(id == IDC_MENUTRANS && code == EN_CHANGE)
				SendPSChanged(hDlg);
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
	DWORD dw;
	int i;
	int temp;

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
	
	
	CheckDlgButton(hDlg, IDC_STARTMENU,
		GetMyRegLong("StartMenu", "CustomizeStartMenu", FALSE));
	
	GetMyRegStrEx("StartMenu", "StartMenuBmp", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FILEMENU, s);
	
	for(i = 50; i <= 52; i++)
		CBAddString(hDlg, IDC_MENUSTYLEMENU, (LPARAM)MyString(i));
	GetMyRegStr("StartMenu", "StartMenuDrawStyle", s, 80, MyString(48));
	if(_strnicmp(s, "TILE", 4) == 0)
		CBSetCurSel(hDlg, IDC_MENUSTYLEMENU, 1);
	else if(_strnicmp(s, "STRETCH", 7) == 0)
		CBSetCurSel(hDlg, IDC_MENUSTYLEMENU, 2);
	else
		CBSetCurSel(hDlg, IDC_MENUSTYLEMENU, 0);

	CheckDlgButton(hDlg, IDC_MENUEDGESCALE,
		GetMyRegLong("StartMenu", "StartMenuEdgeScaling", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPINMENUEDGETOP, UDM_SETRANGE, 0,
		MAKELONG(400, 1));
	SendDlgItemMessage(hDlg, IDC_SPINMENUEDGETOP, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("StartMenu", "StartMenuEdgeTop", 1));
	SendDlgItemMessage(hDlg, IDC_SPINMENUEDGEBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(400, 1));
	SendDlgItemMessage(hDlg, IDC_SPINMENUEDGEBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("StartMenu", "StartMenuEdgeBottom", 1));
	
	if(!bWin98)
	{
		EnableDlgItem(hDlg, IDC_BANNERSMALLICONS, FALSE);
		EnableDlgItem(hDlg, IDC_NOBANNERLARGEICONS, FALSE);
	}

	CheckDlgButton(hDlg, IDC_BANNERSMALLICONS,
		GetMyRegLong("StartMenu", "SmallIconsShowBanner", FALSE));

	CheckDlgButton(hDlg, IDC_NOBANNERLARGEICONS,
		GetMyRegLong("StartMenu", "LargeIconsNoBanner", FALSE));

	dw = GetVersion();
	if(!(dw & 0x80000000) && LOBYTE(LOWORD(dw)) >= 5) ; // Win2000
	else
	{
		int i;
		for(i = IDC_CAPMENUTRANS; i <= IDC_SPINMENUTRANS; i++)
			EnableDlgItem(hDlg, i, FALSE);
	}

		dw = GetMyRegLong("StartMenu", "AlphaStartMenu", 0);
		if(dw > 97) dw = 97;
		SendDlgItemMessage(hDlg, IDC_SPINMENUTRANS, UDM_SETRANGE, 0,
			MAKELONG(97, 0));
		SendDlgItemMessage(hDlg, IDC_SPINMENUTRANS, UDM_SETPOS, 0,
			(int)(short)dw);

	InitColor(hDlg);
	OnStartMenu(hDlg);
	OnStartMenuEdgeScale(hDlg);
}

/*------------------------------------------------
  apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];
	DWORD dw;

	SetMyRegLong("StartMenu", "CustomizeStartMenu", IsDlgButtonChecked(hDlg, IDC_STARTMENU));

	GetDlgItemText(hDlg, IDC_FILEMENU, s, 1024);
	SetMyRegStr("StartMenu", "StartMenuBmp", s);

	dw = CBGetItemData(hDlg, IDC_COLMENU, CBGetCurSel(hDlg, IDC_COLMENU));
	SetMyRegLong("StartMenu", "StartMenuCol", dw);

	GetDlgItemText(hDlg, IDC_MENUSTYLEMENU, s, 1024);
	SetMyRegStr("StartMenu", "StartMenuDrawStyle", s);

	SetMyRegLong("StartMenu", "StartMenuEdgeScaling", IsDlgButtonChecked(hDlg, IDC_MENUEDGESCALE));

	SetMyRegLong("StartMenu", "StartMenuEdgeTop",
		SendDlgItemMessage(hDlg, IDC_SPINMENUEDGETOP, UDM_GETPOS, 0, 0));

	SetMyRegLong("StartMenu", "StartMenuEdgeBottom",
		SendDlgItemMessage(hDlg, IDC_SPINMENUEDGEBOTTOM, UDM_GETPOS, 0, 0));

	SetMyRegLong("StartMenu", "SmallIconsShowBanner", IsDlgButtonChecked(hDlg, IDC_BANNERSMALLICONS));

	SetMyRegLong("StartMenu", "LargeIconsNoBanner", IsDlgButtonChecked(hDlg, IDC_NOBANNERLARGEICONS));

	SetMyRegLong("StartMenu", "AlphaStartMenu",
		SendDlgItemMessage(hDlg, IDC_SPINMENUTRANS, UDM_GETPOS, 0, 0));

}

/*------------------------------------------------
　「...」　スタートメニューのビットマップ参照
--------------------------------------------------*/
void OnSanshoMenu(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_BMPFILE)); str0cat(filter, "*.bmp");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	
	SetColorFromBmp(hDlg, IDC_COLMENU, fname);
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
　「色」コンボボックスの初期化
--------------------------------------------------*/
void InitColor(HWND hDlg)
{
	COLORREF col;
	int i;
	//Windowsデフォルト16色
	int rgb[16][3] = {{0,0,0}, {128,0,0}, {0,128,0}, {128,128,0},
		{0,0,128}, {128,0,128}, {0,128,128}, {192,192,192},
		{128,128,128}, {255,0,0}, {0,255,0}, {255,255,0},
		{0,0,255},{255,0,255}, {0,255,255}, {255,255,255}};
	
	for(i = 0; i < 16; i++) //基本16色
		CBAddString(hDlg, IDC_COLMENU,
			RGB(rgb[i][0], rgb[i][1], rgb[i][2]));
	
	col = GetMyRegLongEx("StartMenu", "StartMenuCol", GetSysColor(COLOR_HIGHLIGHT));
	
	for(i = 0; i < 16; i++)
	{
		if(col == (COLORREF)CBGetItemData(hDlg, IDC_COLMENU, i))
			break;
	}
	if(i == 16) //16色中にないとき
		CBAddString(hDlg, IDC_COLMENU, col);
	CBSetCurSel(hDlg, IDC_COLMENU, i);
}

/*------------------------------------------------
　「スタートメニューを改造する」
--------------------------------------------------*/
void OnStartMenu(HWND hDlg)
{
	BOOL b;
	int i;
	HDC hdc;
	BOOL bb;
	int j;

	b = IsDlgButtonChecked(hDlg, IDC_STARTMENU);
	bb = IsDlgButtonChecked(hDlg, IDC_MENUEDGESCALE);
	if(bb) 
		j = IDC_SPINMENUEDGEBOTTOM;
	else 
		j = IDC_MENUEDGESCALE;
	for(i = IDC_LABFILEMENU; i <= j; i++)
		EnableDlgItem(hDlg, i, b);
	//EnableDlgItem(hDlg, IDC_BANNERSMALLICONS, b);
	//EnableDlgItem(hDlg, IDC_NOBANNERLARGEICONS, b);
	SendPSChanged(hDlg);
	
	//環境が256色以下のときは、色の選択を無効に
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
		EnableDlgItem(hDlg, IDC_CHOOSECOLMENU, FALSE);
	DeleteDC(hdc);
}

void OnStartMenuEdgeScale(HWND hDlg)
{
	BOOL b;
	int i;

	b = IsDlgButtonChecked(hDlg, IDC_MENUEDGESCALE);
	for(i = IDC_LABEDEGETOP; i <= IDC_SPINMENUEDGEBOTTOM; i++)
		EnableDlgItem(hDlg, i, b);
	SendPSChanged(hDlg);
	
	//環境が256色以下のときは、色の選択を無効に
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
	
	pdis = (LPDRAWITEMSTRUCT)lParam;
	
	col = pdis->itemData;
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);

	switch(pdis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			hbr = CreateSolidBrush(col);
			FillRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
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
	
	idCombo = IDC_COLMENU; //id - 1;
	
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
		if(CBGetCount(hDlg, idCombo) == 16)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, 16, cc.rgbResult);
		i = 16;
	}
	CBSetCurSel(hDlg, idCombo, i);
	
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
   Select "Color" combo box automatically.
--------------------------------------------------*/
#define WIDTHBYTES(i) ((i+31)/32*4)

void SetColorFromBmp(HWND hDlg, int idCombo, char* fname)
{
	HFILE hf;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	int numColors;
	BYTE pixel[3];
	COLORREF col;
	int i, index;
	HDC hdc;
	
	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return;
	
	if(_lread(hf, &bmfh, sizeof(bmfh)) != sizeof(bmfh) ||
		bmfh.bfType != *(WORD*)"BM" ||
		_lread(hf, &bmih, sizeof(bmih)) != sizeof(bmih) ||
		bmih.biSize != sizeof(bmih) ||
		bmih.biCompression != BI_RGB ||
		!(bmih.biBitCount <= 8 || bmih.biBitCount == 24))
	{
		_lclose(hf); return;
	}
	numColors = bmih.biClrUsed;
	if(numColors == 0)
	{
		if(bmih.biBitCount <= 8) numColors = 1 << bmih.biBitCount;
		else numColors = 0;
	}
	if(numColors > 0 &&
		_llseek(hf, sizeof(RGBQUAD)*numColors, FILE_CURRENT) == HFILE_ERROR)
	{
		_lclose(hf); return;
	}
	if(_llseek(hf,
			WIDTHBYTES(bmih.biWidth*bmih.biBitCount)*(bmih.biHeight-1),
			FILE_CURRENT) == HFILE_ERROR ||
		_lread(hf, pixel, sizeof(pixel)) != sizeof(pixel))
	{
		_lclose(hf); return;
	}
	if(bmih.biBitCount < 24)
	{
		index = -1;
		if(bmih.biBitCount == 8) index = pixel[0];
		else if(bmih.biBitCount == 4)
			index = (pixel[0] & 0xF0) >> 4;
		else if(bmih.biBitCount == 1)
			index = (pixel[0] & 0x80) >> 7;
		if(_llseek(hf, sizeof(bmfh)+sizeof(bmih)+sizeof(RGBQUAD)*index,
			FILE_BEGIN) == HFILE_ERROR ||
			_lread(hf, pixel, sizeof(pixel)) != sizeof(pixel))
		{
			index = -1;
		}
	}
	_lclose(hf);
	if(index == -1) return;
	col = RGB(pixel[2], pixel[1], pixel[0]);
	
	for(i = 0; i < 16; i++)
	{
		if(col == (COLORREF)CBGetItemData(hDlg, idCombo, i)) break;
	}
	if(i == 16)
	{
		int screencolor;
		hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
		screencolor = GetDeviceCaps(hdc, BITSPIXEL);
		DeleteDC(hdc);
		if(screencolor <= 8) return;
		
		if(CBGetCount(hDlg, idCombo) == 16)
			CBAddString(hDlg, idCombo, col);
		else CBSetItemData(hDlg, idCombo, 16, col);
	}
	CBSetCurSel(hDlg, idCombo, i);
}

