/*-------------------------------------------
Å@TCDLL.H
Å@KAZUBON 1997-2001
---------------------------------------------*/

#define _WIN32_IE 0x0300

#include <windows.h>
#include <windowsx.h>
#include <winreg.h>
#include <shellapi.h>
#include <winnls.h>
#include <commctrl.h>
#include <string.h>
#include <ctype.h>
#include <winuser.h>
#include <wingdi.h>

#define AC_SRC_ALPHA                0x01

#define IDTIMER_DEKSTOPICONSTYLE 3

#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)
#define CLOCKM_BLINK          (WM_USER+3)
#define CLOCKM_COPY           (WM_USER+4)
#define CLOCKM_REFRESHDESKTOP (WM_USER+5)
#define CLOCKM_REFRESHCLEARTASKBAR	(WM_USER+6)

typedef struct tagCOLORSCHEME {
   DWORD            dwSize;
   COLORREF         clrBtnHighlight;       // highlight color
   COLORREF         clrBtnShadow;          // shadow color
} COLORSCHEME, *LPCOLORSCHEME;

//typedef struct tagTRACKMOUSEEVENT {
    //DWORD cbSize;
    //DWORD dwFlags;
    //HWND  hwndTrack;
    //DWORD dwHoverTime;
//} TRACKMOUSEEVENT, *LPTRACKMOUSEEVENT;

typedef struct {
    UINT cbSize;
    DWORD dwMask;
    int idCommand;
    int iImage;
    BYTE fsState;
    BYTE fsStyle;
    WORD cx;
    DWORD lParam;
    LPSTR pszText;
    int cchText;
} TBBUTTONINFOA, *LPTBBUTTONINFOA;

typedef struct _NMTBCUSTOMDRAW {
    NMCUSTOMDRAW nmcd;
    HBRUSH hbrMonoDither;
    HBRUSH hbrLines;
    HPEN hpenLines;
    COLORREF clrText;
    COLORREF clrMark;
    COLORREF clrTextHighlight;
    COLORREF clrBtnFace;
    COLORREF clrBtnHighlight;
    COLORREF clrHighlightHotTrack;
    RECT rcText;
    int nStringBkMode;
    int nHLStringBkMode;
} NMTBCUSTOMDRAW, * LPNMTBCUSTOMDRAW;

#define TBCDRF_NOEDGES              0x00010000  // Don't draw button edges
#define TBCDRF_HILITEHOTTRACK       0x00020000  // Use color of the button bk when hottracked
#define TBCDRF_NOOFFSET             0x00040000  // Don't offset button if pressed
#define TBCDRF_NOMARK               0x00080000  // Don't draw default highlight of image/text for TBSTATE_MARKED
#define TBCDRF_NOETCHEDEFFECT       0x00100000  // Don't draw etched effect for disabled items

#define TBBUTTONINFO TBBUTTONINFOA
#define LPTBBUTTONINFO LPTBBUTTONINFOA

// tclock.c
void DrawClock(HWND hwnd, HDC hdc);
void FillClock(HWND hwnd, HDC hdc, RECT *prc, int nblink);
extern BOOL bNoClock;
void GetDisplayTime(SYSTEMTIME* pt, int* beat100);

// utl.c
extern BOOL g_bIniSetting;
extern char g_inifile[];

int _strncmp(const char* d, const char* s, size_t n);
HFONT CreateMyFont(char* fontname, int fontsize,
	LONG weight, LONG italic, int angle);
int ext_cmp(char *fname, char *ext);
void add_title(char *path, char *title);
void del_title(char *path);
void parse(char *dst, char *src, int n);
char* MyString(UINT id);
int GetMyRegStr(char* section, char* entry, char* val, int cbData, 
	char* defval);
int GetMyRegStrEx(char* section, char* entry, char* val, int cbData, 
	char* defval);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
COLORREF GetMyRegColor(char* section, char* entry, COLORREF defval);
LONG GetMyRegLongEx(char* section, char* entry, LONG defval);
LONG GetRegLong(HKEY rootkey, char*subkey, char* entry, LONG defval);
int GetRegStr(HKEY rootkey, char*subkey, char* entry,
	char* val, int cbData, char* defval);
BOOL SetMyRegStr(char* subkey, char* entry, char* val);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
void WriteDebug(LPSTR s);
void VerticalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, 
	int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, 
	BOOL ReverseBlt, BOOL useTrans);
void HorizontalTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, 
	int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, 
	DWORD rasterOp);
void FillTileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, 
	int cyDest, HDC hdcSrc, int xSrc, int ySrc, int cxSrc, int cySrc, 
	DWORD rasterOp);
void TileBlt(HDC hdcDest, int xDest, int yDest, int cxDest, int cyDest, HDC hdcSrc, 
    int xSrc, int ySrc, int cxSrc, int cySrc, BOOL useTrans);
BOOL ExecFile(HWND hwnd, char* command);
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL IsXPStyle();
void Pause( HWND hWnd, LPCTSTR pszArgs );
BOOL ConvertTip(char *destination, WCHAR *tip, UINT code);


// FORMAT.C
/*void InitFormat(SYSTEMTIME* lt);
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt);
BOOL IsDispSecond(char* fmt);
BOOL IsDispSysInfo(char* fmt);
int IsDispBeat(char* fmt);
BOOL FindFormatStr(const char* fmt, const char *tofind);*/
void InitFormat(SYSTEMTIME* lt);
void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt);
#define FORMAT_SECOND    0x0001
#define FORMAT_SYSINFO   0x0002
#define FORMAT_BEAT1     0x0004
#define FORMAT_BEAT2     0x0008
#define FORMAT_BATTERY   0x0010
#define FORMAT_MEMORY    0x0020
#define FORMAT_MOTHERBRD 0x0040
#define FORMAT_PERMON    0x0080
#define FORMAT_NET       0x0100
DWORD FindFormat(char* fmt);


// DESKTOP.C
void SetDesktopIcons(void);
void EndDesktopIcons(void);
void SetDesktop(void);
void EndDesktop(void);

// REBAR.C
//void InitRebar(HWND hwndClock);
//void EndRebar(void);
//void RedrawRebar(void);


// STARTBTN.C
void SetStartButton(HWND hwnd);
void EndStartButton(void);
void SetStartButtonBmp(void);
void CheckCursorOnStartButton(void);
LRESULT RebarCustomDraw(NMCUSTOMDRAW *lpcd);

// STARTMENU.C
void SetStartMenu(HWND hwnd);
void EndStartMenu(void);
void InitStartMenuIE4(void);
void ClearStartMenuIE4(void);

// taskswitch.c
void InitTaskSwitch(HWND hwndClock);
void EndTaskSwitch(void);

// traynotifywnd.c
void InitTrayNotify(HWND hwnd);
void EndTrayNotify(void);

// BMP.C
HBITMAP ReadBitmap(HWND hwnd, char* fname, BOOL b);

// newapi.c
void EndNewAPI(HWND hwnd);
void SetLayeredTaskbar(HWND hwndClock);
void SetLayeredStartMenu(HWND hwnd);
void InitLayeredStartMenu(HWND hwndClock);
void GradientFillClock(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2);
void SetLayeredDesktop(HWND hwndClock);
void TransBlt(HDC dhdc, int dx, int dy, int dw, int dh, 
			  HDC shdc, int sx, int sy, int sw, int sh);
void GradientFillClock95(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2);
void TransparentBlt95(HDC hdcDst, int nXDest, int nYDest, int nWidth, int nHeight, 
					  HDC hdcSrc, int nXSrc, int nYSrc);
void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, 
				HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans);
void InitDrawThemeParentBackground(void);
void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT *prc);

// sysres.c
int GetFreeSystemResources(WORD wSysRes);
void EndSysres(void);

void InitBatteryLife(void);
int GetBatteryLifePercent(void);
void FreeBatteryLife(void);

// cpu.c
void CpuMoni_start(void);
int CpuMoni_get(void);
void CpuMoni_end(void);

// mbm.c
int MBMSharedDataUpdate(void);

// permon.c
void PerMoni_start(void);
int PerMoni_get(void);
void PerMoni_end(void);

// net.c
void Net_start(void);
void Net_get(void);
void Net_end(void);

#ifndef CCM_FIRST
#define CCM_FIRST               0x2000
#endif

#ifndef CCM_SETCOLORSCHEME
#define CCM_SETCOLORSCHEME      (CCM_FIRST + 2)
#endif

#ifndef RB_SETBKCOLOR
#define RB_SETBKCOLOR   (WM_USER +  19)
#endif
