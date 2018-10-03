/*-------------------------------------------
  utl.c
    その他の関数
    KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

/*-------------------------------------------
  ランタイム関数の代用
---------------------------------------------*/
int atoi(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 10 + *p - '0';
		p++;

	}
	return r;
}

int atox(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 16 + *p - '0';
		else if('A' <= *p && *p <= 'F')
			r = r * 16 + *p - 'A' + 10;
		else if('a' <= *p && *p <= 'f')
			r = r * 16 + *p - 'a' + 10;
		p++;

	}
	return r;
}

__inline int toupper(int c)
{
	if('a' <= c && c <= 'z')
		c -= 'a' - 'A';
	return c;
}

int _strnicmp(const char* d, const char* s, size_t n)
{
	int c1, c2;
	unsigned int i;
	for(i = 0; i < n; i++)
	{
		if(*s == 0 && *d == 0) break;
		c1 = toupper(*d); c2 = toupper(*s);
		if(c1 != c2) return (c1 - c2);
		d++; s++;
	}
	return 0;
}

int _stricmp(const char* d, const char* s)
{
	int c1, c2;
	while(1)
	{
		if(*s == 0 && *d == 0) break;
		c1 = toupper(*d); c2 = toupper(*s);
		if(c1 != c2) return (c1 - c2);
		d++; s++;
	}
	return 0;
}

/*-------------------------------------------
　パス名にファイル名をつける
---------------------------------------------*/
void add_title(char *path, char *title)
{
	char *p;
	
	p = path;
	
	if(*p == 0) ;
	else if(*title && *(title + 1) == ':') ;
	else if(*title == '\\')
	{
		if(*p && *(p + 1) == ':') p += 2;
	}
	else
	{
		while(*p)
		{
			if((*p == '\\' || *p == '/') && *(p + 1) == 0)
			{
				break;
			}
			p = CharNext(p);
		}
		*p++ = '\\';
	}
	while(*title) *p++ = *title++;
	*p = 0;
}

/*-------------------------------------------
　パス名からファイル名をとりのぞく
---------------------------------------------*/
void del_title(char *path)
{
	char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	*ep = 0;
}

/*-------------------------------------------
　パス名からファイル名を得る
---------------------------------------------*/
void get_title(char* dst, const char *path)
{
	const char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	
	if(*ep == '\\' || *ep == '/') ep++;
	
	while(*ep) *dst++ = *ep++;
	*dst = 0;
}

/*------------------------------------------------
　ファイルの拡張子の比較
--------------------------------------------------*/
int ext_cmp(const char *fname, const char *ext)
{
	const char* p, *sp;
	
	sp = NULL; p = fname;
	while(*p)
	{
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}
	
	if(sp == NULL) sp = p;
	if(*sp == '.') sp++;
	
	while(1)
	{
		if(*sp == 0 && *ext == 0) return 0;
		if(toupper(*sp) != toupper(*ext))
			return (toupper(*sp) - toupper(*ext));
		sp++; ext++;
	}
	return 0;
}

/*------------------------------------------------
	カンマで区切られた文字列を取り出す
--------------------------------------------------*/
void parse(char *dst, char *src, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0) 
	{
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ',') *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

/*------------------------------------------------
	文字で区切られた文字列を取り出す
--------------------------------------------------*/
void parsechar(char *dst, char *src, char ch, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ch) src++;
		if(*src == ch) src++;
	}
	if(*src == 0) 
	{
		*dst = 0; return;
	}
	
	while(*src == ' ') src++;
	
	dp = dst;
	while(*src && *src != ch) *dst++ = *src++;
	*dst = 0;
	
	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

/*------------------------------------------------
　'\0'で終了する文字列を追加する
　最後は"\0\0"で終了
--------------------------------------------------*/
void str0cat(char* dst, const char* src)
{
	char* p;
	p = dst;
	while(*p) { while(*p) p++; p++; }
	strcpy(p, src);
	while(*p) p++; p++; *p = 0;
}

/*-------------------------------------------
  returns a resource string
---------------------------------------------*/
char* MyString(UINT id)
{
	static char buf[80];
	HINSTANCE hInst;
	
	buf[0] = 0;
	hInst = GetLangModule();
	if(hInst) LoadString(hInst, id, buf, 80);
	
	return buf;
}

/*-------------------------------------------
  アイコンつきメッセージボックス
---------------------------------------------*/
int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep)
{
	MSGBOXPARAMS mbp;
	
	memset(&mbp, 0, sizeof(MSGBOXPARAMS));
	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = hwnd;
	mbp.hInstance = g_hInst;
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_ICON1);
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if(uBeep != 0xFFFFFFFF)
		MessageBeep(uBeep);
	return MessageBoxIndirect(&mbp);
}

/*------------------------------------------------
  get locale info for 95/NT
--------------------------------------------------*/
int GetLocaleInfoWA(int ilang, LCTYPE LCType, char* dst, int n)
{
	int r;
	LCID Locale;
	
	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	if(GetVersion() & 0x80000000) // 95
		r = GetLocaleInfoA(Locale, LCType, dst, n);
	else  // NT
	{
		WCHAR* pw;
		pw = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetLocaleInfoW(Locale, LCType, pw, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw, -1, dst, n,
				NULL, NULL);
		GlobalFreePtr(pw);
	}
	return r;
}

/*-------------------------------------------
  32bit x 32bit = 64bit
---------------------------------------------*/
DWORDLONG M32x32to64(DWORD a, DWORD b)
{
	ULARGE_INTEGER r;
	DWORD *p1, *p2, *p3;
	memset(&r, 0, 8);
	p1 = &r.LowPart;
	p2 = (DWORD*)((BYTE*)p1 + 2);
	p3 = (DWORD*)((BYTE*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return *(DWORDLONG*)(&r);
}

/*-------------------------------------------
  SetForegroundWindow for Windows98
---------------------------------------------*/
void SetForegroundWindow98(HWND hwnd)
{
	DWORD dwVer;
	
	dwVer = GetVersion();
	if(((dwVer & 0x80000000) && 
	       LOBYTE(LOWORD(dwVer)) >= 4 && HIBYTE(LOWORD(dwVer)) >= 10) ||
	   (!(dwVer & 0x80000000) && LOBYTE(LOWORD(dwVer)) >= 5)) // Win98/2000
	{
		DWORD thread1, thread2;
		DWORD pid;
		thread1 = GetWindowThreadProcessId(
			GetForegroundWindow(), &pid);
		thread2 = GetCurrentThreadId();
		AttachThreadInput(thread2, thread1, TRUE);
		SetForegroundWindow(hwnd);
		AttachThreadInput(thread2, thread1, FALSE);
		BringWindowToTop(hwnd);
	}
	else  // Win95/NT
		SetForegroundWindow(hwnd);
}

/*-------------------------------------------
  for debugging
---------------------------------------------*/
void WriteDebug(const char* s)
{
	HFILE hf;
	char fname[] = "DEBUG.TXT";
	
	hf = _lopen(fname, OF_WRITE);
	if(hf == HFILE_ERROR)
		hf = _lcreat(fname, 0);
	if(hf == HFILE_ERROR) return;
	_llseek(hf, 0, 2);
	_lwrite(hf, s, strlen(s));
	_lwrite(hf, "\x0d\x0a", 2);
	_lclose(hf);
}

//TClock用のレジストリのキー
char mykey[] = "Software\\TClock2";

/*------------------------------------------------
　自分のレジストリから文字列を得る
--------------------------------------------------*/
int GetMyRegStr(char* section, char* entry, char* val, int cbData,
	char* defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = cbData;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)val, &size) == 0)
			{
				if(size == 0) *val = 0;
				r = size;
				b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE)
		{
			strcpy(val, defval);
			r = strlen(defval);
		}
	}
	
	return r;
}

int GetMyRegStrEx(char* section, char* entry, char* val, int cbData,
	char* defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);
		if(r == cbData)
			SetMyRegStr(section, entry, defval);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = cbData;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)val, &size) == 0)
			{
				if(size == 0) *val = 0;
				r = size;
				b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE)
		{
			SetMyRegStr(section, entry, defval);
			strcpy(val, defval);
			r = strlen(defval);
		}
	}
	
	return r;
}

/*------------------------------------------------
　自分のレジストリからLONG値を得る
--------------------------------------------------*/
LONG GetMyRegLong(char* section, char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	LONG r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = 4;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)&r, &size) == 0)
			{
				if(size == 4) b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE) r = defval;
	}
	return r;
}

LONG GetMyRegLongEx(char* section, char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	LONG r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
		if(r = defval)
			SetMyRegLong(section, entry, defval);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = 4;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)&r, &size) == 0)
			{
				if(size == 4) b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE) 
		{
			r = defval;
			SetMyRegLong(section, entry, defval);
		}
	}
	return r;
}

/*------------------------------------------------
  get DWORD value from registry
--------------------------------------------------*/
LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval)
{
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	b = FALSE;
	if(RegOpenKey(rootkey, subkey, &hkey) == 0)
	{
		size = 4;
		if(RegQueryValueEx(hkey, entry, 0, &regtype,
			(LPBYTE)&r, &size) == 0)
		{
			if(size == 4) b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(b == FALSE) r = defval;
	return r;
}

/*------------------------------------------------
　レジストリから文字列を得る
--------------------------------------------------*/
int GetRegStr(HKEY rootkey, char*subkey, char* entry,
	char* val, int cbData, char* defval)
{
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	b = FALSE;
	if(RegOpenKey(rootkey, subkey, &hkey) == 0)
	{
		size = cbData;
		if(RegQueryValueEx(hkey, entry, 0, &regtype,
			(LPBYTE)val, &size) == 0)
		{
			if(size == 0) *val = 0;
			b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(b == FALSE)
	{
		strcpy(val, defval);
		r = strlen(defval);
	}
	return r;
}

/*-------------------------------------------
　レジストリに文字列を書き込む
---------------------------------------------*/
BOOL SetMyRegStr(char* section, char* entry, char* val)
{
	HKEY hkey;
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = FALSE;
		if(WritePrivateProfileString(key, entry, val, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_SZ,
				(CONST BYTE*)val, strlen(val)) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

/*-------------------------------------------
　レジストリに文字列を書き込む
---------------------------------------------*/
BOOL SetRegStr(HKEY rootkey, char* subkey, char* entry, char* val)
{
	HKEY hkey;
	BOOL r;

	if(RegCreateKey(rootkey, subkey, &hkey) == 0)
	{
		if(RegSetValueEx(hkey, entry, 0, REG_SZ,
			(CONST BYTE*)val, strlen(val)) == 0)
		{
			r = TRUE;
		}
		RegCloseKey(hkey);
	}
	return r;
}

/*-------------------------------------------
　レジストリにDWORD値を書き込む
---------------------------------------------*/
BOOL SetMyRegLong(char* section, char* entry, DWORD val)
{
	HKEY hkey;
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		char s[20];
		wsprintf(s, "%d", val);
		r = FALSE;
		if(WritePrivateProfileString(key, entry, s, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_DWORD,
				(CONST BYTE*)&val, 4) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}


/*-------------------------------------------
　レジストリの値を削除
---------------------------------------------*/
BOOL DelMyReg(char* section, char* entry)
{
	BOOL r;
	char key[80];
	HKEY hkey;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = FALSE;
		if(WritePrivateProfileString(key, entry, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegDeleteValue(hkey, entry) == 0)
				r = TRUE;
			RegCloseKey(hkey);
		}
	}
	return r;
}

/*-------------------------------------------
　レジストリのキーを削除
---------------------------------------------*/
BOOL DelMyRegKey(char* section)
{
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = FALSE;
		if(WritePrivateProfileSection(key, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegDeleteKey(HKEY_CURRENT_USER, key) == 0)
			r = TRUE;
	}
	return r;
}

void MoveRegToIni(char *section)
{
	char key[80], section2[80], section3[80], name[80], value[1024];
	DWORD cbName, cbValue, type;
	HKEY hkey;
	int i;
	
	if(!g_bIniSetting) return;
	
	strcpy(key, mykey);
	if(section == NULL)
	{
		strcpy(section2, "Main");
	}
	else
	{
		strcat(key, "\\"); strcat(key, section);
		strcpy(section2, section);
	}
	
	if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
	{
		for(i = 0; ; i++)
		{
			cbName = 80; cbValue = 1024;
			type = 0;
			if(RegEnumValue(hkey, i, name, &cbName, 0, &type,
				(LPBYTE)value, &cbValue) != ERROR_SUCCESS) break;
			
			if(type == REG_DWORD)
			{
				char s[20];
				wsprintf(s, "%d", *(int *)value);
				WritePrivateProfileString(section2, name, s, g_inifile);
			}
			else if(type == REG_SZ)
			{
				value[cbValue] = 0;
				WritePrivateProfileString(section2, name, value, g_inifile);
			}
		}
		
		for(i = 0; ; i++)
		{
			if(RegEnumKey(hkey, i, name, cbName) != ERROR_SUCCESS)
				break;
			
			section3[0] = 0;
			if(section != NULL)
			{
				strcpy(section3, section); strcat(section3, "\\");
			}
			strcat(section3, name);
			MoveRegToIni(section3);
		}
		
		RegCloseKey(hkey);
	}
}

COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	LONG r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = 4;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)&r, &size) == 0)
			{
				if(size == 4) b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE) r = defval;
	}
	if(r & 0x80000000) r = GetSysColor(r & 0x00ffffff);

	return r;
}

void OnChooseColor(HWND hDlg, WORD id, WORD idCombo)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	int i;
	
	
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
	
	//PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	//SendPSChanged(hDlg);
}
