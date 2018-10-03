/*-------------------------------------------
  pagemisc.c
　　「その他」プロパティページ
　　KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

BOOL FileExists(HWND hDlg);
static void RemoveStartup(HWND hDlg);
static void OnStartup(HWND hDlg);
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

/*------------------------------------------------
　「その他」ページ用ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK PageMiscProc(HWND hDlg, UINT message,
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
			// 「時計を改造しない」
			if(id == IDC_NOCLOCK)
			{
				g_bApplyClock = TRUE;
				SendPSChanged(hDlg);
			}
			// チェックボックス
			else if(id == IDC_MCIWAVE)
				SendPSChanged(hDlg);
			// 何秒後に開始
			else if(id == IDC_DELAYSTART && code == EN_CHANGE)
				SendPSChanged(hDlg);
			// 「スタートアップ」にショートカットをつくる
			else if(id == IDC_STARTUP || IDC_COMPACTMENU || IDC_CALENDARHIDE)
				SendPSChanged(hDlg);
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				//case PSN_HELP: MyHelp(GetParent(hDlg), 6); break;
			}
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
　ページの初期化
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HFONT hfont;
	
	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_COMDESKCAL,
			WM_SETFONT, (WPARAM)hfont, 0);
	
	CheckDlgButton(hDlg, IDC_NOCLOCK,
		GetMyRegLong("Clock", "NoClockCustomize", FALSE));
	
	CheckDlgButton(hDlg, IDC_MCIWAVE,
		GetMyRegLong("", "MCIWave", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPINDELAYSTART, UDM_SETRANGE, 0,
		MAKELONG(600, 0));
	SendDlgItemMessage(hDlg, IDC_SPINDELAYSTART, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("", "DelayStart", 0));
	if(FileExists(hDlg))
		CheckDlgButton(hDlg, IDC_STARTUP, TRUE);
	
	CheckDlgButton(hDlg, IDC_COMPACTMENU,
		GetMyRegLong("", "CompactMenu", FALSE));

	CheckDlgButton(hDlg, IDC_CALENDARHIDE,
		GetMyRegLong("Calendar", "HideCalendar", FALSE));

}

/*------------------------------------------------
　更新
--------------------------------------------------*/
void OnApply(HWND hDlg)
{

	SetMyRegLong("Clock", "NoClockCustomize", IsDlgButtonChecked(hDlg, IDC_NOCLOCK));
	
	SetMyRegLong("", "MCIWave", IsDlgButtonChecked(hDlg, IDC_MCIWAVE));
	
	SetMyRegLong("", "DelayStart",
		SendDlgItemMessage(hDlg, IDC_SPINDELAYSTART, UDM_GETPOS, 0, 0));

	if(IsDlgButtonChecked(hDlg, IDC_STARTUP))
		OnStartup(hDlg);
	else
		RemoveStartup(hDlg);

	SetMyRegLong("", "CompactMenu", IsDlgButtonChecked(hDlg, IDC_COMPACTMENU));
		 
	SetMyRegLong("Calendar", "HideCalendar", IsDlgButtonChecked(hDlg, IDC_CALENDARHIDE));

}

BOOL FileExists(HWND hDlg)
{
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], path[MAX_PATH], path2[MAX_PATH];
	int	retval;	
	char *lpStr1, *lpStr2;

	//char buffer_1[] = "\TClock.lnk";
	//char buffer_2[] = "\Tclock2.lnk";

	if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR &&
		SHGetPathFromIDList(pidl, dstpath) == TRUE)
		;
	else return FALSE;

	strcpy(path, dstpath);
	strcat(path, "\\TClock2.lnk");
	lpStr1 = path;
	retval = PathFileExists(lpStr1);
	if(retval == 1)
		return TRUE;
	else
	{
		strcpy(path2, dstpath);
		strcat(path2, "\\TClock.lnk");
		lpStr2 = path2;
		retval = PathFileExists(lpStr2);
		if(retval == 1)
			return TRUE;
	}
	return FALSE;
}

void RemoveStartup(HWND hDlg)
{
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], path[MAX_PATH], path2[MAX_PATH];
	int	retval;	
	char *lpStr1, *lpStr2;

	//char buffer_1[] = "\TClock.lnk";
	//char buffer_2[] = "\Tclock2.lnk";
	if(!FileExists(hDlg)) return;

	if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR &&
		SHGetPathFromIDList(pidl, dstpath) == TRUE)
		;
	else return;

	if(MyMessageBox(hDlg, "Remove shortcut from Startup folder.\nAre you sure?",
		"TClock2", MB_YESNO, MB_ICONQUESTION) != IDYES) return;

	strcpy(path, dstpath);
	strcat(path, "\\TClock2.lnk");
	lpStr1 = path;
	retval = DeleteFile(lpStr1);
	if(retval == 1)
		return;
	else
	{
		strcpy(path2, dstpath);
		strcat(path2, "\\TClock.lnk");
		lpStr2 = path2;
		retval = DeleteFile(lpStr2);
		if(retval == 1)
			return;
	}
	return;
}

void OnStartup(HWND hDlg)
{
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], myexe[MAX_PATH];

	if(FileExists(hDlg)) return;

	if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR &&
		SHGetPathFromIDList(pidl, dstpath) == TRUE)
		;
	else return;
	
	if(MyMessageBox(hDlg, MyString(IDS_STARTUPLINK),
		"TClock2", MB_YESNO, MB_ICONQUESTION) != IDYES) return;

	GetModuleFileName(g_hInst, myexe, MAX_PATH);
	CreateLink(myexe, dstpath, "TClock2");
}

/*------------------------------------------------
　ショートカットの作成
--------------------------------------------------*/
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name) 
{
	HRESULT hres;
	IShellLink* psl;
	
	CoInitialize(NULL);
	
	hres = CoCreateInstance(&CLSID_ShellLink, NULL, 
		CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
	if(SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		char path[MAX_PATH];
		
/*		path[0] = '\"';
		strcpy(path+1, fname);
		strcat(path, "\"");*/
		psl->lpVtbl->SetPath(psl, fname);
		psl->lpVtbl->SetDescription(psl, name);
		strcpy(path, fname);
		del_title(path);
		psl->lpVtbl->SetWorkingDirectory(psl, path);
		
		hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile,
			&ppf);
		
		if(SUCCEEDED(hres))
		{
			WORD wsz[MAX_PATH]; 
			char lnkfile[MAX_PATH];
			strcpy(lnkfile, dstpath);
			add_title(lnkfile, (char*)name);
			strcat(lnkfile, ".lnk");
			
			MultiByteToWideChar(CP_ACP, 0, lnkfile, -1,
				wsz, MAX_PATH);
			
			hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
			ppf->lpVtbl->Release(ppf);
		}
		psl->lpVtbl->Release(psl);
	}
	CoUninitialize();
	
	if(SUCCEEDED(hres)) return TRUE;
	else return FALSE;
}
