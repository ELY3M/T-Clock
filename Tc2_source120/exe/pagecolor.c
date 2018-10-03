/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void InitColor(HWND hDlg);
static void OnMeasureItemColorCombo(LPARAM lParam);
static void OnDrawItemColorCombo(LPARAM lParam);
static void OnChooseColor(HWND hDlg, WORD id);
static void InitComboFont(HWND hDlg);
static void SetComboFontSize(HWND hDlg, int bInit);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC

extern BOOL bWin95, bWin98, bWinME, bWinNT, bWin2000, bWinXP;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageColorProc(HWND hDlg, UINT message,
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
			if((id == IDC_COLFORE || id == IDC_FONT || 
				id == IDC_FONTSIZE || id == IDC_CLOCKROTATE) && code == CBN_SELCHANGE)
			{
				if(id == IDC_FONT) SetComboFontSize(hDlg, FALSE);
				SendPSChanged(hDlg);
			}
			else if(id == IDC_CHOOSECOLFORE)
				OnChooseColor(hDlg, id);
			else if(id == IDC_BOLD || id == IDC_ITALIC)
				SendPSChanged(hDlg);
			else if((id == IDC_CLOCKHEIGHT || id == IDC_CLOCKWIDTH ||
				id == IDC_VERTPOS || id == IDC_LINEHEIGHT || id == IDC_HORIZPOS)
				&& code == EN_CHANGE)
				SendPSChanged(hDlg);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				//case PSN_HELP: MyHelp(GetParent(hDlg), 1); break;
			}
			return TRUE;
		case WM_DESTROY:
			DeleteObject(hfontb);
			DeleteObject(hfonti);
			break;
	}
	return FALSE;
}

/*------------------------------------------------
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HDC hdc;
	LOGFONT logfont;
	HFONT hfont;
	///DWORD dwVer;
	char s[1024];
	int i;

	// setting of "background" and "text"
	InitColor(hDlg);

	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSECOLFORE, FALSE);
	}
	DeleteDC(hdc);
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FONT, WM_SETFONT, (WPARAM)hfont, 0);
	
	//「フォント」の設定
	InitComboFont(hDlg);
	//「フォントサイズ」の設定
	SetComboFontSize(hDlg, TRUE);
		
	//「Bold」「Italic」の設定
	CheckDlgButton(hDlg, IDC_BOLD,
		GetMyRegLong("Clock", "Bold", FALSE));
	CheckDlgButton(hDlg, IDC_ITALIC, 
		GetMyRegLong("Clock", "Italic", FALSE));
	
	hfontb = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfontb, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfontb = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_BOLD, WM_SETFONT, (WPARAM)hfontb, 0);

	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfonti = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_ITALIC, WM_SETFONT, (WPARAM)hfonti, 0);

	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(200, -32));
	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockHeight", 0));
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockWidth", 0));
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "LineHeight", 0));
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "VertPos", 0));
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "HorizPos", 0));

	for(i = 72; i <= 74; i++)
		CBAddString(hDlg, IDC_CLOCKROTATE, (LPARAM)MyString(i));
	GetMyRegStr("Clock", "FontRotateDirection", s, 80, MyString(72));
	if(_strnicmp(s, "LEFT", 4) == 0)
		CBSetCurSel(hDlg, IDC_CLOCKROTATE, 1);
	else if(_strnicmp(s, "RIGHT", 5) == 0)
		CBSetCurSel(hDlg, IDC_CLOCKROTATE, 2);
	else
		CBSetCurSel(hDlg, IDC_CLOCKROTATE, 0);

}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char s[80];
	char ss[1024];

	//文字色の保存
	dw = CBGetItemData(hDlg, IDC_COLFORE, CBGetCurSel(hDlg, IDC_COLFORE));
	SetMyRegLong("Clock", "ForeColor", dw);
	
	//フォント名の保存
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), s);
	SetMyRegStr("Clock", "Font", s);
	
	//フォントサイズの保存
	if(CBGetCount(hDlg, IDC_FONTSIZE) > 0)
	{
		CBGetLBText(hDlg, IDC_FONTSIZE, CBGetCurSel(hDlg, IDC_FONTSIZE), s);
		SetMyRegLong("Clock", "FontSize", atoi(s));
	}
	else SetMyRegLong("Clock", "FontSize", 9);

	//「Bold」「Italic」の保存
	SetMyRegLong("Clock", "Bold", IsDlgButtonChecked(hDlg, IDC_BOLD));
	SetMyRegLong("Clock", "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
	
	SetMyRegLong("Clock", "ClockHeight",
		SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("Clock", "ClockWidth",
		SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("Clock", "LineHeight",
		SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("Clock", "VertPos",
		SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_GETPOS, 0, 0));
	SetMyRegLong("Clock", "HorizPos",
		SendDlgItemMessage(hDlg, IDC_SPINHPOS, UDM_GETPOS, 0, 0));

	GetDlgItemText(hDlg, IDC_CLOCKROTATE, ss, 1024);
	if(_strnicmp(ss, "LEFT", 4) == 0)
		SetMyRegStr("Clock", "FontRotateDirection", "Left");
	else if(_strnicmp(ss, "RIGHT", 5) == 0)
		SetMyRegStr("Clock", "FontRotateDirection", "Right");
	else
		SetMyRegStr("Clock", "FontRotateDirection", "None");

}

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
	
	id = IDC_COLFORE;

	for(j = 0; j < 16; j++) //基本16色
		CBAddString(hDlg, id,
			RGB(rgb[j][0], rgb[j][1], rgb[j][2]));
	//ボタンの...色
	CBAddString(hDlg, id, 0x80000000|COLOR_3DFACE);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DSHADOW);
	CBAddString(hDlg, id, 0x80000000|COLOR_3DHILIGHT);
	CBAddString(hDlg, id, 0x80000000|COLOR_BTNTEXT);
	
	col = GetMyRegLong("Clock", "ForeColor",
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

// 参考：Visual C++ 4.x のサンプルWORDPADのFORMATBA.CPP

BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf, 
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
int nFontSizes[] = 
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
	hcombo = GetDlgItem(hDlg, IDC_FONT);
	lf.lfCharSet = GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	ReleaseDC(NULL, hdc);
	
	GetMyRegStr("Clock", "Font", s, 80, "");
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
	i = CBFindStringExact(hDlg, IDC_FONT, s);
	if(i == LB_ERR) i = 0;
	CBSetCurSel(hDlg, IDC_FONT, i);
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
		size = GetMyRegLong("Clock", "FontSize", 9);
		if(size == 0) size = 9;
	}
	else   // IDC_FONTが変更されたとき
	{
		CBGetLBText(hDlg, IDC_FONTSIZE,
			CBGetCurSel(hDlg, IDC_FONTSIZE), (LPARAM)s);
		size = atoi(s);
	}
	
	CBResetContent(hDlg, IDC_FONTSIZE);
	
	hdc = GetDC(NULL);
	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);
	
	// s = フォント名
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), (LPARAM)s);
	
	//フォントのサイズを列挙してコンボボックスに入れる
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, s);
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT));
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumSizeProcEx,
		(LPARAM)GetDlgItem(hDlg, IDC_FONTSIZE), 0);
	
	//EnumFontFamilies(hdc, s,
	//	(FONTENUMPROC)EnumSizeProc, (LPARAM)GetDlgItem(hDlg, IDC_FONTSIZE));
	
	ReleaseDC(NULL, hdc);
	
	// sizeに近いものを選択
	for(; size > 0; size--)
	{
		wsprintf(s, "%d", size);
		i = CBFindStringExact(hDlg, IDC_FONTSIZE, s);
		if(i != LB_ERR)
		{
			CBSetCurSel(hDlg, IDC_FONTSIZE, i); return;
		}
	}
	CBSetCurSel(hDlg, IDC_FONTSIZE, 0);
}

/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf, 
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
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf, 
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
			wsprintf(s, "%d", nFontSizes[i]);
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



