/*-------------------------------------------
  tclock.h
      KAZUBON  1997-1999
---------------------------------------------*/

#define _WIN32_IE 0x0300

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shlobj.h>
#include <stdlib.h>
#include <Shlwapi.h>
//#include <string.h>
#include "resource.h"
#include "../language/resource.h"

// replacement of standard library's functions
#define malloc(s) GlobalAllocPtr(GHND,(s))
#define free(p) GlobalFreePtr(p);
int atoi(const char *p);
int _strnicmp(const char* d, const char* s, size_t n);
int _stricmp(const char* d, const char* s);

// IDs for timer
#define IDTIMER_START				2
#define IDTIMER_MAIN				3
#define IDTIMER_MOUSE				4
#define IDTIMER_DEKSTOPICON			5
#define IDTIMER_DESKTOPICONSTYLE	6


// messages to send the clock
#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)
#define CLOCKM_BLINK          (WM_USER+3)
#define CLOCKM_COPY           (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP (WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)

// for mouse.c and pagemouce.c
#define MOUSEFUNC_NONE       -1
#define MOUSEFUNC_DATETIME    0
#define MOUSEFUNC_EXITWIN     1
#define MOUSEFUNC_RUNAPP      2
#define MOUSEFUNC_MINALL      3
#define MOUSEFUNC_SYNCTIME    4
#define MOUSEFUNC_TIMER       5
#define MOUSEFUNC_CLIPBOARD   6
#define MOUSEFUNC_SCREENSAVER 7
#define MOUSEFUNC_SHOWCALENDER 8
#define MOUSEFUNC_OPENFILE    100

// main.c
extern HINSTANCE g_hInst;           // instance handle
extern HINSTANCE g_hInstResource;   // instance handle of language module
extern HWND      g_hwndMain;        // main window
extern HWND      g_hwndClock;       // clock window
extern HWND      g_hwndSheet;       // property sheet window
extern HWND      g_hDlgTimer;       // timer dialog
extern HWND      g_hDlgCalender;    // calender dialog
extern HICON     g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;
                                    // icons to use frequently
extern char      g_mydir[];         // path to tclock.exe
extern char      g_langdllname[];   // language dll name
extern BOOL      g_bIniSetting;
extern char      g_inifile[];
HINSTANCE LoadLanguageDLL(char *fname); // load language dll
HINSTANCE GetLangModule(void);      // instance handle of language dll
void MyHelp(HWND hDlg, int id);     // show help
void RefreshUs(void);

// propsheet.c
extern BOOL g_bApplyClock;
extern BOOL g_bApplyTaskbar;
extern BOOL g_bApplyLangDLL;
void MyPropertySheet(void);
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex,
	const char *deffile, char *retfile);

// alarm.c
void InitAlarm(void);
void OnTimerAlarm(HWND hwnd, SYSTEMTIME* st);
void EndAlarm(void);
BOOL PlayFile(HWND hwnd, char *fname, DWORD dwLoops);
void StopFile(void);
void OnMCINotify(HWND hwnd);
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL ExecFile(HWND hwnd, char* command);

// alarmday.c
int SetAlarmDay(HWND hDlg, int n);

// soundselect.c
BOOL BrowseSoundFile(HWND hDlg, const char *deffile, char *fname);
BOOL IsMMFile(const char* fname);

// pageformat.c
void InitFormat(void);
void CreateFormat(char* s, int* checks);

// menu.c
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode);

// mouse.c
void InitMouseFunction(void);
void OnDropFiles(HWND hwnd, HDROP hdrop);
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnTimerMouse(HWND hwnd);

// deskcal.c
void DialogCalender(HWND hwnd);

// timer.c
#define MAX_TIMER  20

void DialogTimer(HWND hwnd);
void OnTimerTimer(HWND hwnd, SYSTEMTIME* st);
void EndTimer(void);
int GetTimerInfo(char *dst, int num);
void StopTimer(HWND hwnd, int n);

// sntp.c
BOOL InitSyncTime(HWND hwnd);
void InitSyncTimeSetting(void);
void EndSyncTime(HWND hwnd);
void OnTimerSNTP(HWND hwnd, SYSTEMTIME* st);
void StartSyncTime(HWND hwndParent, char* pServer, int nto);

// utl.c
int atox(const char *p);
void add_title(char *path, char* titile);
void del_title(char *path);
void get_title(char* dst, const char *path);
int ext_cmp(const char *fname, const char *ext);
void parse(char *dst, char *src, int n);
void parsechar(char *dst, char *src, char ch, int n);
void str0cat(char* dst, const char* src);
char* MyString(UINT id);
int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep);
int GetLocaleInfoWA(int ilang, LCTYPE LCType, char* dst, int n);
DWORDLONG M32x32to64(DWORD a, DWORD b);
void SetForegroundWindow98(HWND hwnd);
void WriteDebug(const char* s);
int GetMyRegStr(char* section, char* entry, char* val, int cbData,
	char* defval);
int GetMyRegStrEx(char* section, char* entry, char* val, int cbData,
	char* defval);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
LONG GetMyRegLongEx(char* section, char* entry, LONG defval);
LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);
int GetRegStr(HKEY rootkey, char*subkey, char* entry,
	char* val, int cbData, char* defval);
BOOL SetMyRegStr(char* subkey, char* entry, char* val);
BOOL SetRegStr(HKEY rootkey, char* subkey, char* entry, char* val);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
BOOL DelMyReg(char* subkey, char* entry);
BOOL DelMyRegKey(char* subkey);
void MoveRegToIni(char *section);
COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval);


BOOL CoolMenu_Create(HMENU hmenu, LPCTSTR pVertLable);
VOID CoolMenu_Cleanup(HWND hwnd);
VOID CoolMenu_SetCheck(HWND hwnd, UINT idm);
VOID CoolMenu_SetEnabled(HWND hwnd, UINT idm);

LRESULT CoolMenu_PopupContext(HWND hwnd, LPARAM lParam, int idx);
LRESULT CoolMenu_MeasureItem(WPARAM wParam, LPARAM lParam);
LRESULT CoolMenu_DrawItem(WPARAM wParam, LPARAM lParam);


// TCDLL.DLLのAPI
void WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd(void);

// Macros

#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)

#define CBAddString(hDlg,id,lParam) SendDlgItemMessage((hDlg),(id),CB_ADDSTRING,0,(lParam))
#define CBDeleteString(hDlg,id, i) SendDlgItemMessage((hDlg),(id),CB_DELETESTRING,(i),0)
#define CBFindString(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRING,0,(LPARAM)(s))
#define CBFindStringExact(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,0,(LPARAM)(s))
#define CBGetCount(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCOUNT,0,0)
#define CBGetCurSel(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCURSEL,0,0)
#define CBGetItemData(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_GETITEMDATA,(i),0)
#define CBGetLBText(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_GETLBTEXT,(i),(LPARAM)(s))
#define CBInsertString(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_INSERTSTRING,(i),(LPARAM)(s))
#define CBResetContent(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_RESETCONTENT,0,0)
#define CBSetCurSel(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_SETCURSEL,(i),0)
#define CBSetItemData(hDlg,id,i,lParam) SendDlgItemMessage((hDlg),(id),CB_SETITEMDATA,(i),(lParam))

// アラーム用構造体
typedef struct _tagAlarmStruct
{
	char name[40];
	BOOL bAlarm;
	int hour;
	int minute;
	char fname[1024];
	BOOL bHour12;
	BOOL bRepeat;
	BOOL bBlink;
	int days;
} ALARMSTRUCT;
typedef ALARMSTRUCT* PALARMSTRUCT;

