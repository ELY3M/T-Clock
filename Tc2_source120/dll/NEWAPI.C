/*-------------------------------------------
  newapi.c
  GradientFill and Layerd Window
  Kazubon 1999
---------------------------------------------*/

#include "tcdll.h"

#if _MSC_VER < 1200

typedef USHORT COLOR16;

typedef struct _TRIVERTEX
{
    LONG    x;
    LONG    y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}TRIVERTEX,*PTRIVERTEX,*LPTRIVERTEX;

typedef struct _GRADIENT_RECT
{
    ULONG UpperLeft;
    ULONG LowerRight;
}GRADIENT_RECT,*PGRADIENT_RECT,*LPGRADIENT_RECT;

typedef struct _BLENDFUNCTION
{
    BYTE   BlendOp;
    BYTE   BlendFlags;
    BYTE   SourceConstantAlpha;
    BYTE   AlphaFormat;
} BLENDFUNCTION,*PBLENDFUNCTION;

#define AC_SRC_OVER                 0x00

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define GRADIENT_FILL_TRIANGLE  0x00000002
#define GRADIENT_FILL_OP_FLAG   0x000000ff

#endif

#define WS_EX_LAYERED 0x80000
#define LWA_ALPHA     2
#define LWA_COLORKEY  1

HMODULE hmodMSIMG32 = NULL;
HMODULE hmodUSER32 = NULL;
HMODULE hmodUxTheme = NULL;

BOOL (WINAPI *pGradientFill)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG) = NULL;

BOOL (WINAPI *pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD) = NULL;

BOOL (WINAPI *pTransparentBlt)(HDC,int,int,int,int,HDC,int,int,int,int,UINT) = NULL;

BOOL (WINAPI *pDrawThemeParentBackground)(HWND hwnd, HDC hdc, RECT *prc) = NULL;

static BOOL bInitGradientFill = FALSE;
static BOOL bInitLayeredWindow = FALSE;
static BOOL bInitTransparentBlt = FALSE;
static BOOL bInitDrawThemeParentBackground = FALSE;

static void EndLayeredStartMenu(HWND hwndClock);
static void RefreshRebar(HWND hwndBar);
static void EndLayeredDesktop(HWND hwndClock);
static void RefreshUs(void);

int nAlphaStartMenu = 255;
int nAlphaDesktop = 255;
BOOL bClearTaskbar = FALSE;
extern BOOL bRefreshClearTaskbar;

void InitGradientFill(void)
{
	if(bInitGradientFill) return;
	
	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		(FARPROC)pGradientFill = GetProcAddress(hmodMSIMG32, "GradientFill");
		if(pGradientFill == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitGradientFill = TRUE;
}

void InitTransparentBlt(void)
{
	if(bInitTransparentBlt) return;
	
	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		(FARPROC)pTransparentBlt = GetProcAddress(hmodMSIMG32, "TransparentBlt");
		if(pTransparentBlt == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitTransparentBlt = TRUE;
}

void InitLayeredWindow(void)
{
	if(bInitLayeredWindow) return;
	
	hmodUSER32 = LoadLibrary("user32.dll");
	if(hmodUSER32 != NULL)
	{
		(FARPROC)pSetLayeredWindowAttributes = 
			GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
		if(pSetLayeredWindowAttributes == NULL)
		{
			FreeLibrary(hmodUSER32); hmodUSER32 = NULL;
		}
	}
	bInitLayeredWindow = TRUE;
}

void EndNewAPI(HWND hwndClock)
{
	if(hmodMSIMG32 != NULL) FreeLibrary(hmodMSIMG32);
	hmodMSIMG32 = NULL; pGradientFill = NULL; pTransparentBlt = NULL;
	
	if(pSetLayeredWindowAttributes)
	{
		HWND hwnd;
		LONG exstyle;
		
		hwnd = GetParent(GetParent(hwndClock));
		exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		if(exstyle & WS_EX_LAYERED)
		{
			exstyle &= ~WS_EX_LAYERED;
			SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
			bClearTaskbar = FALSE;
			RefreshRebar(hwnd);
		}
		
		EndLayeredStartMenu(hwndClock);
		//EndLayeredDesktop(hwndClock);
	}
	
	if(hmodUSER32 != NULL) FreeLibrary(hmodUSER32);
	hmodUSER32 = NULL;
	pSetLayeredWindowAttributes = NULL;

	if(hmodUxTheme != NULL) FreeLibrary(hmodUxTheme);
	hmodUxTheme = NULL;
	pDrawThemeParentBackground = NULL;

}

void GradientFillClock(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT gRect;
	
	if(!pGradientFill) InitGradientFill();
	if(!pGradientFill)
	{
		GradientFillClock95(hdc, prc, col1, col2); 
		return;
	}
	
	vert[0].x      = prc->left;
	vert[0].y      = prc->top;
	vert[0].Red    = (COLOR16)GetRValue(col1) * 256;
	vert[0].Green  = (COLOR16)GetGValue(col1) * 256;
	vert[0].Blue   = (COLOR16)GetBValue(col1) * 256;
	vert[0].Alpha  = 0x0000;
	vert[1].x      = prc->right;
	vert[1].y      = prc->bottom; 
	vert[1].Red    = (COLOR16)GetRValue(col2) * 256;
	vert[1].Green  = (COLOR16)GetGValue(col2) * 256;
	vert[1].Blue   = (COLOR16)GetBValue(col2) * 256;
	vert[1].Alpha  = 0x0000;
	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	pGradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_H);
}

void TransBlt(HDC dhdc, int dx, int dy, int dw, int dh, 
			  HDC shdc, int sx, int sy, int sw, int sh)
{
	if(!pTransparentBlt) InitTransparentBlt();
	if(!pTransparentBlt)
	{
		TransparentBlt95(dhdc, dx, dy, dw, dh, shdc, sx, sy); 
		return;
	}

	pTransparentBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, RGB(255, 0, 255));

}

void SetLayeredTaskbar(HWND hwndClock)
{
	LONG exstyle;
	HWND hwnd;
	int alpha;
	
	alpha = GetMyRegLongEx("Taskbar", "AlphaTaskbar", 0);
	bClearTaskbar = GetMyRegLong("Taskbar", "ClearTaskbar", FALSE);
	alpha = 255 - (alpha * 255 / 100);
	if(alpha < 8) alpha = 8; else if(alpha > 255) alpha = 255;
	
	if(!pSetLayeredWindowAttributes && (alpha < 255 || bClearTaskbar)) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;
	
	hwnd = GetParent(GetParent(hwndClock));
	
	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(alpha < 255 || bClearTaskbar) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
	if(bRefreshClearTaskbar)
	{
		bRefreshClearTaskbar = FALSE;
		RefreshUs();
	}
	//SendMessage(FindWindow("Shell_TrayWnd", NULL), WM_NCPAINT, 0, 0);
	RefreshRebar(hwnd);
	
	if(alpha < 255 && !bClearTaskbar)
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);
	else if(bClearTaskbar)
		pSetLayeredWindowAttributes(hwnd, GetSysColor(COLOR_3DFACE), (BYTE)alpha, LWA_COLORKEY|LWA_ALPHA);

}


void SetLayeredDesktop(HWND hwndClock)
{
	LONG exstyle;
	HWND hwnd;
	int alpha;
	
	alpha = GetMyRegLongEx("Desktop", "AlphaDesktopIcons", 0);
	alpha = 255 - (alpha * 255 / 100);
	if(alpha < 8) alpha = 8; else if(alpha > 255) alpha = 255;
	
	if(!pSetLayeredWindowAttributes && alpha < 255) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;
	
	hwnd = FindWindow("Progman", "Program Manager");
	if(!hwnd) return;
	//hwnd = GetWindow(hwnd, GW_CHILD);
	//hwnd = GetWindow(hwnd, GW_CHILD);
	//while(hwnd)
	//{
		//char s[80];
		//GetClassName(hwnd, s, 80);
		//if(lstrcmpi(s, "SysListView32") == 0) break;
		//hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	//}
	//if(!hwnd) return;
	
	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(alpha < 255) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
	
	if(alpha < 255)
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);
}

void EndLayeredDesktop(HWND hwndClock)
{
	LONG exstyle;
	HWND hwnd;
	
	hwnd = FindWindow("Progman", "Program Manager");
	if(!hwnd) return;
	//hwnd = GetWindow(hwnd, GW_CHILD);
	//hwnd = GetWindow(hwnd, GW_CHILD);
	//while(hwnd)
	//{
		//char s[80];
		//GetClassName(hwnd, s, 80);
		//if(lstrcmpi(s, "SysListView32") == 0) break;
		//hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	//}
	//if(!hwnd) return;
	
	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(exstyle & WS_EX_LAYERED)
	{
		exstyle &= ~WS_EX_LAYERED;
		SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
	}
	
}

void InitLayeredStartMenu(HWND hwndClock)
{
	EndLayeredStartMenu(hwndClock);
	
	nAlphaStartMenu = GetMyRegLongEx("StartMenu", "AlphaStartMenu", 0);
	nAlphaStartMenu = 255 - (nAlphaStartMenu * 255 / 100);
	if(nAlphaStartMenu < 8) nAlphaStartMenu = 8;
	else if(nAlphaStartMenu > 255) nAlphaStartMenu = 255;
}

void EndLayeredStartMenu(HWND hwndClock)
{
	HWND hwnd;
	LONG exstyle;
	char classname[80];
	
	nAlphaStartMenu = 255;
	
	hwnd = GetDesktopWindow();
	hwnd = GetWindow(hwnd, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if((lstrcmpi(classname, "BaseBar") == 0 || lstrcmpi(classname, "DV2ControlHost") == 0) &&
			GetWindowThreadProcessId(hwnd, NULL) == 
				GetWindowThreadProcessId(hwndClock, NULL))
		{
			exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			if(exstyle & WS_EX_LAYERED)
			{
				exstyle &= ~WS_EX_LAYERED;
				SetWindowLong(hwnd, GWL_EXSTYLE, exstyle);
			}
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

void SetLayeredStartMenu(HWND hwnd)
{
	LONG exstyle;
	
	if(!pSetLayeredWindowAttributes) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;
	
	exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
	if(!(exstyle & WS_EX_LAYERED))
	{
		SetWindowLong(hwnd, GWL_EXSTYLE, exstyle|WS_EX_LAYERED);
		pSetLayeredWindowAttributes(hwnd, 0, 
			(BYTE)nAlphaStartMenu, LWA_ALPHA);
	}
}

/*--------------------------------------------------
    redraw ReBarWindow32 forcely
----------------------------------------------------*/
void RefreshRebar(HWND hwndBar)
{
	HWND hwnd;
	char classname[80];
	
	hwnd = GetWindow(hwndBar, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ReBarWindow32") == 0)
		{
			InvalidateRect(hwnd, NULL, TRUE);
			hwnd = GetWindow(hwnd, GW_CHILD);
			while(hwnd)
			{
				InvalidateRect(hwnd, NULL, TRUE);
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
			break;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

//force ourselves to redraw all our settings
void RefreshUs(void)
{
	HWND hwndBar;
	HWND hwndChild;
	char classname[80];

	hwndBar = FindWindow("Shell_TrayWnd", NULL);

	// find the clock window
	hwndChild = GetWindow(hwndBar, GW_CHILD);
	while(hwndChild)
	{
		GetClassName(hwndChild, classname, 80);
		//WriteDebug(classname);
		if(lstrcmpi(classname, "TrayNotifyWnd") == 0)
		{
			hwndChild = GetWindow(hwndChild, GW_CHILD);
			while(hwndChild)
			{
				GetClassName(hwndChild, classname, 80);
				//WriteDebug(classname);
				if(lstrcmpi(classname, "TrayClockWClass") == 0)
				{
					SendMessage(hwndChild, CLOCKM_REFRESHCLOCK, 0, 0);
					SendMessage(hwndChild, CLOCKM_REFRESHTASKBAR, 0, 0);
					break;
				}
			}
			break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}

//windows 95 & NT4 don't have a GradientFill function
//so we use our own (code modified from old LiteStep code)
void GradientFillClock95(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2)
{
	int m_nBuffHeight;
	int m_nBuffWidth;
	HDC m_hBuffDC;
	HBITMAP m_hBuffBmp;
	RECT r;
	int nWidth;
	int nHeight;

	CopyRect(&r, prc);

	nWidth = r.right;
	nHeight = r.bottom;

	m_nBuffHeight = 0;
	m_nBuffWidth = 0;

	m_hBuffDC = NULL;
	m_hBuffBmp = NULL;

	if(nWidth != m_nBuffWidth || nHeight != m_nBuffHeight)
	{
		RECT r2;
		double stepX;
		double cr, cg, cb, dr, dg, db;
		HBRUSH hBrush;
		int gradSteps;
		int i;

		gradSteps = nWidth/2;

		if(m_hBuffDC)
			DeleteDC(m_hBuffDC);

		if(m_hBuffBmp)
			DeleteObject(m_hBuffBmp);

		m_nBuffWidth = nWidth;
		m_nBuffHeight = nHeight;
	
		m_hBuffDC = CreateCompatibleDC(hdc);
		m_hBuffBmp = CreateCompatibleBitmap(hdc, m_nBuffWidth, m_nBuffHeight);
		SelectObject(m_hBuffDC, m_hBuffBmp);

		// get current color
		cr = GetRValue(col1);
		cg = GetGValue(col1);
		cb = GetBValue(col1);

		// Get delta color
		dr = (cr - GetRValue(col2))/gradSteps;
		dg = (cg - GetRValue(col2))/gradSteps;
		db = (cb - GetRValue(col2))/gradSteps;

		stepX = ((double)(r.right-r.left))/(double)gradSteps;

		for(i = 0; i < gradSteps + 1; i++)
		{
			hBrush = CreateSolidBrush(RGB((int)cr,(int)cg,(int)cb));

			cr -= dr;
			cg -= dg;
			cb -= db;

			SetRect(&r2, (int)(stepX*(i - 1)),0,(int)(stepX * i),m_nBuffHeight);
			FillRect(m_hBuffDC, &r2, hBrush);
			DeleteObject(hBrush);		
		}
	}

	// copy our buffer to the screen
	BitBlt(hdc, r.left, r.top, m_nBuffWidth, m_nBuffHeight, m_hBuffDC, 0, 0, SRCCOPY);

	DeleteDC(m_hBuffDC);
	DeleteObject(m_hBuffBmp);

}

//windows 95 & NT4 don't have a TransparentBlt function
//so we use our own (code modified from old LiteStep code)
void TransparentBlt95(HDC hdcDst, int nXDest, int nYDest, int nWidth, 
					  int nHeight, HDC hdcSrc, int nXSrc, int nYSrc)
{
	HDC hdcMem, hdcMask, hdcDstCpy;
	HBITMAP hbmMask, hbmMem, hbmDstCpy;
	HBITMAP hbmOldMem, hbmOldMask, hbmOldDstCpy;

	// create a destination compatble dc containing
	// a copy of the destination dc
	hdcDstCpy	= CreateCompatibleDC(hdcDst);
	hbmDstCpy	= CreateCompatibleBitmap(hdcDst, nWidth, nHeight);
	hbmOldDstCpy = (HBITMAP)SelectObject(hdcDstCpy, hbmDstCpy);

	StretchBlt(hdcDstCpy,0,0,nWidth,nHeight,hdcDst,nXDest,nYDest,nWidth,nHeight,SRCCOPY);
	
	// create a destination compatble dc containing
	// a copy of the source dc
	hdcMem	= CreateCompatibleDC(hdcDst);
	hbmMem	= CreateCompatibleBitmap(hdcDst, nWidth, nHeight);
	hbmOldMem = (HBITMAP)SelectObject(hdcMem, hbmMem);

	StretchBlt(hdcMem,0,0,nWidth,nHeight,hdcSrc,nXSrc,nYSrc,nWidth,nHeight,SRCCOPY);

	// the transparent color should be selected as
	// bkcolor into the memory dc
	SetBkColor(hdcMem, RGB(255, 0, 255));

	// Create monochrome bitmap for the mask
	hdcMask	= CreateCompatibleDC(hdcDst);
	hbmMask = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
	hbmOldMask = (HBITMAP)SelectObject(hdcMask, hbmMask);

	// Create the mask from the memory dc
	StretchBlt(hdcMask,0,0,nWidth,nHeight,hdcMem,0,0,nWidth,nHeight,SRCCOPY);

	// Set the background in hdcMem to black. Using SRCPAINT with black
	// and any other color results in the other color, thus making
	// black the transparent color
	SetBkColor(hdcMem, RGB(0,0,0));
	SetTextColor(hdcMem, RGB(255,255,255));

	StretchBlt(hdcMem,0,0,nWidth,nHeight,hdcMask,0,0,nWidth,nHeight,SRCAND);

	// Set the foreground to black. See comment above.
	SetBkColor(hdcDst, RGB(255,255,255));
	SetTextColor(hdcDst, RGB(0,0,0));

	StretchBlt(hdcDstCpy,0,0,nWidth,nHeight,hdcMask,0,0,nWidth,nHeight,SRCAND);

	// Combine the foreground with the background
	StretchBlt(hdcDstCpy,0,0,nWidth,nHeight,hdcMem,0,0,nWidth,nHeight,SRCPAINT);

	// now we have created the image we want to blt
	// in the destination copy dc
	StretchBlt(hdcDst,nXDest,nYDest,nWidth,nHeight,hdcDstCpy,0,0,nWidth,nHeight,SRCCOPY);

	SelectObject(hdcMask, hbmOldMask);
	DeleteObject(hbmMask);
	DeleteDC(hdcMask);

	SelectObject(hdcMem, hbmOldMem);
	DeleteObject(hbmMem);
	DeleteDC(hdcMem);

	SelectObject(hdcDstCpy, hbmOldDstCpy);
	DeleteObject(hbmDstCpy);
	DeleteDC(hdcDstCpy);
}

void TC2DrawBlt(HDC dhdc, int dx, int dy, int dw, int dh, 
				HDC shdc, int sx, int sy, int sw, int sh, BOOL useTrans)
{
	if(useTrans)
		TransBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh);
	else
		StretchBlt(dhdc, dx, dy, dw, dh, shdc, sx, sy, sw, sh, SRCCOPY);
}

void InitDrawThemeParentBackground(void)
{
	if(bInitDrawThemeParentBackground) return;
	
	hmodUxTheme = LoadLibrary("UxTheme.dll");
	if(hmodUxTheme != NULL)
	{
		(FARPROC)pDrawThemeParentBackground = GetProcAddress(hmodUxTheme, "DrawThemeParentBackground");
		if(pDrawThemeParentBackground == NULL)
		{
			FreeLibrary(hmodUxTheme); hmodUxTheme = NULL;
		}
	}
	bInitDrawThemeParentBackground = TRUE;
}

void DrawXPClockBackground(HWND hwnd, HDC hdc, RECT *prc)
{
	if(!pDrawThemeParentBackground) InitDrawThemeParentBackground();
	if(!pDrawThemeParentBackground)return;

	pDrawThemeParentBackground(hwnd, hdc, prc);

}
