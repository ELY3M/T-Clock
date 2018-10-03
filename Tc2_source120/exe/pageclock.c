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
static void OnCheckColor2(HWND hDlg);
static void OnFillStyleCombo(HWND hDlg);
static void OnSanshoClockSkin(HWND hDlg, WORD id);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC
BOOL bSecondColor = TRUE;

extern BOOL bWin95, bWin98, bWinME, bWinNT, bWin2000, bWinXP;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	//g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageClockProc(HWND hDlg, UINT message,
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
			if((id == IDC_COLBACK || id == IDC_COLBACK2) &&
				code == CBN_SELCHANGE)
			{
				SendPSChanged(hDlg);
			}
			else if(id == IDC_FILLSTYLEMENU && code == CBN_SELCHANGE)
			{
				OnFillStyleCombo(hDlg);
				SendPSChanged(hDlg);
			}
			else if(id == IDC_CHKCOLOR2)
				OnCheckColor2(hDlg);
			else if(id == IDC_CHOOSECOLBACK || id == IDC_CHOOSECOLBACK2)
				OnChooseColor(hDlg, id);
			else if(id == IDC_SANSHOCLOCKSKIN)
				OnSanshoClockSkin(hDlg, id);
			else if(id == IDC_CLOCKSKINEDGELEFT && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_CLOCKSKINEDGETOP && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_CLOCKSKINEDGERIGHT && code == EN_CHANGE)
				SendPSChanged(hDlg);
			else if(id == IDC_CLOCKSKINEDGEBOTTOM && code == EN_CHANGE)
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
	///DWORD dwVer;
	int i;
	char s[1024];

	// setting of "background" and "text"
	InitColor(hDlg);
	
	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, FALSE);
		bSecondColor = FALSE;
	}
	DeleteDC(hdc);
	
	
	
	CheckDlgButton(hDlg, IDC_CHKCOLOR2,
		GetMyRegLong("Clock", "UseBackColor2", TRUE));
	OnCheckColor2(hDlg);
	
	CBAddString(hDlg, IDC_FILLSTYLEMENU, (LPARAM)MyString(13));
	if(bWinME || bWin2000 || bWinXP)
	{
		for(i = 56; i <= 59; i++)
			CBAddString(hDlg, IDC_FILLSTYLEMENU, (LPARAM)MyString(i));

	}else if(bWin98)
	{
		for(i = 56; i <= 57; i++)
			CBAddString(hDlg, IDC_FILLSTYLEMENU, (LPARAM)MyString(i));

	}else // Win95 or WinNT
	{
		//for(i = IDC_CHKCOLOR2; i <= IDC_CHOOSECOLBACK2; i++)
			//EnableDlgItem(hDlg, i, FALSE);
		for(i = 56; i <= 57; i++)
			CBAddString(hDlg, IDC_FILLSTYLEMENU, (LPARAM)MyString(i));

	}


	GetMyRegStr("Clock", "ClockFillStyle", s, 80, MyString(56));
	if(_strnicmp(s, "FILLCLOCK", 8) == 0)
		CBSetCurSel(hDlg, IDC_FILLSTYLEMENU, 1);
	else if(_strnicmp(s, "SKINCLOCK", 9) == 0)
		CBSetCurSel(hDlg, IDC_FILLSTYLEMENU, 2);
	else if(_strnicmp(s, "SKINTRAY", 8) == 0)
		CBSetCurSel(hDlg, IDC_FILLSTYLEMENU, 4);
	else if(_strnicmp(s, "FILLTRAY", 8) == 0)
		CBSetCurSel(hDlg, IDC_FILLSTYLEMENU, 3);
	else
		CBSetCurSel(hDlg, IDC_FILLSTYLEMENU, 0);

	GetMyRegStr("Clock", "ClockSkin", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FILECLOCKSKIN, s);

	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGELEFT, UDM_SETRANGE, 0,
		MAKELONG(200, 1));
	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGELEFT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockSkinEdgeLeft", 1));

	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGETOP, UDM_SETRANGE, 0,
		MAKELONG(200, 1));
	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGETOP, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockSkinEdgeTop", 1));

	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGERIGHT, UDM_SETRANGE, 0,
		MAKELONG(200, 1));
	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGERIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockSkinEdgeRight", 1));

	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGEBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(200, 1));
	SendDlgItemMessage(hDlg, IDC_SPINSKINEDGEBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Clock", "ClockSkinEdgeBottom", 1));

	OnFillStyleCombo(hDlg);

}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char s[1024];
	
	//背景色の保存
	dw = CBGetItemData(hDlg, IDC_COLBACK, CBGetCurSel(hDlg, IDC_COLBACK));
	SetMyRegLong("Clock", "BackColor", dw);
	
	SetMyRegLong("Clock", "UseBackColor2",
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2));
	dw = CBGetItemData(hDlg, IDC_COLBACK2, CBGetCurSel(hDlg, IDC_COLBACK2));
	SetMyRegLong("Clock", "BackColor2", dw);
	
	GetDlgItemText(hDlg, IDC_FILECLOCKSKIN, s, 1024);
	SetMyRegStr("Clock", "ClockSkin", s);

	SetMyRegLong("Clock", "ClockSkinEdgeLeft",
		SendDlgItemMessage(hDlg, IDC_SPINSKINEDGELEFT, UDM_GETPOS, 0, 0));

	SetMyRegLong("Clock", "ClockSkinEdgeTop",
		SendDlgItemMessage(hDlg, IDC_SPINSKINEDGETOP, UDM_GETPOS, 0, 0));

	SetMyRegLong("Clock", "ClockSkinEdgeRight",
		SendDlgItemMessage(hDlg, IDC_SPINSKINEDGERIGHT, UDM_GETPOS, 0, 0));

	SetMyRegLong("Clock", "ClockSkinEdgeBottom",
		SendDlgItemMessage(hDlg, IDC_SPINSKINEDGEBOTTOM, UDM_GETPOS, 0, 0));

	GetDlgItemText(hDlg, IDC_FILLSTYLEMENU, s, 1024);
	SetMyRegStr("Clock", "ClockFillStyle", s);
	if(_strnicmp(s, "(NONE)", 6) == 0)
	{
		SetMyRegLong("Clock", "BackColor", 0x80000000|COLOR_3DFACE);
		SetMyRegLong("Clock", "UseBackColor2", 0);
	}
}

/*------------------------------------------------
　「色」コンボボックスの初期化
--------------------------------------------------*/
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
	
	for(i = 0; i < 2; i++)
	{
		if(i == 0) id = IDC_COLBACK;
		else if(i == 1) id = IDC_COLBACK2;

		for(j = 0; j < 16; j++) //基本16色
			CBAddString(hDlg, id,
				RGB(rgb[j][0], rgb[j][1], rgb[j][2]));
		//ボタンの...色
		CBAddString(hDlg, id, 0x80000000|COLOR_3DFACE);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DSHADOW);
		CBAddString(hDlg, id, 0x80000000|COLOR_3DHILIGHT);
		CBAddString(hDlg, id, 0x80000000|COLOR_BTNTEXT);
		
		//コンボボックスの色を選択
		if(i == 0)
			col = GetMyRegLong("Clock", "BackColor",
				0x80000000 | COLOR_3DFACE);
		else if(i == 1)
			col = GetMyRegLong("Clock", "BackColor2", col);
		for(j = 0; j < 20; j++)
		{
			if(col == (COLORREF)CBGetItemData(hDlg, id, j))
				break;
		}
		if(j == 20) //20色中にないとき
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
  enable/disable to use "background 2"
--------------------------------------------------*/
void OnCheckColor2(HWND hDlg)
{
	BOOL b;
	b = IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2);
	EnableDlgItem(hDlg, IDC_COLBACK2, b);
	EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, b);
	SendPSChanged(hDlg);
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

void OnFillStyleCombo(HWND hDlg)
{
	char s[1024];
	int i, j;
	//BOOL b;

	//b = IsWindowEnabled(hDlg);
	if(!bSecondColor)
		j = IDC_CHOOSECOLBACK;
	else if(bSecondColor && IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2))
		j = IDC_CHOOSECOLBACK2;
	else
		j = IDC_CHKCOLOR2;

	GetDlgItemText(hDlg, IDC_FILLSTYLEMENU, s, 1024);
	if(_strnicmp(s, "(NONE)", 6) == 0)
	{
		for(i = ICD_FRAMECOLOR; i <= j; i++)
			EnableDlgItem(hDlg, i, FALSE);
		for(i = IDC_FRAMESKIN; i <= IDC_SPINSKINEDGEBOTTOM; i++)
			EnableDlgItem(hDlg, i, FALSE);
	}
	else if(_strnicmp(s, "FILLTRAY", 8) == 0 || _strnicmp(s, "FILLCLOCK", 9) == 0)
	{
		for(i = ICD_FRAMECOLOR; i <= j; i++)
			EnableDlgItem(hDlg, i, TRUE);
		for(i = IDC_FRAMESKIN; i <= IDC_SPINSKINEDGEBOTTOM; i++)
			EnableDlgItem(hDlg, i, FALSE);
		EnableDlgItem(hDlg, ICD_FRAMECOLOR, TRUE);
		EnableDlgItem(hDlg, IDC_FRAMESKIN, FALSE);
		EnableDlgItem(hDlg, IDC_CLOCKSKINTRANS, FALSE);
	}
	else if(_strnicmp(s, "SKINCLOCK", 9) == 0 || _strnicmp(s, "SKINTRAY", 8) == 0)
	{
		for(i = ICD_FRAMECOLOR; i <= j; i++)
			EnableDlgItem(hDlg, i, FALSE);
		for(i = IDC_FRAMESKIN; i <= IDC_SPINSKINEDGEBOTTOM; i++)
			EnableDlgItem(hDlg, i, TRUE);
		EnableDlgItem(hDlg, IDC_FRAMESKIN, TRUE);
		EnableDlgItem(hDlg, ICD_FRAMECOLOR, FALSE);
		EnableDlgItem(hDlg, IDC_CLOCKSKINTRANS, TRUE);
	}

}

void OnSanshoClockSkin(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];
	
	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_BMPFILE)); str0cat(filter, "*.bmp");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");
	
	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);
	
	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;
	
	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

