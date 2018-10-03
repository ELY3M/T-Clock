/*-----------------------------------------------------
  main.c
   API, hook procedure
   KAZUBON 1997-2001
-------------------------------------------------------*/

#include "tcdll.h"

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
void InitClock(HWND hwnd);

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
extern HHOOK hhook;
extern HWND hwndTClockMain;
extern HWND hwndClock;

/*------------------------------------------------
  globals
--------------------------------------------------*/
extern HANDLE hmod;
extern WNDPROC oldWndProc;
extern BOOL bWin2000;

extern int nAlphaStartMenu;

/*------------------------------------------------
  entry point of this DLL
--------------------------------------------------*/
//BOOL WINAPI _DllMainCRTStartup(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
{
	hmod = hModule;
	switch (dwFunction)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
		default:
			break;
	}
	return TRUE;
}

/*------------------------------------------------
   API: install my hook
--------------------------------------------------*/
void WINAPI HookStart(HWND hwnd)
{
	HWND hwndBar;
	HANDLE hThread;
	HWND hwndChild;
	char classname[80];
	
	hwndTClockMain = hwnd;
	
	// find the taskbar
	hwndBar = FindWindow("Shell_TrayWnd", NULL);
	if(!hwndBar)
	{
		SendMessage(hwnd, WM_USER+1, 0, 1);
		return;
	}
	
	// get thread ID of taskbar (explorer)
	// Specal thanks to T.Iwata.
	hThread = (HANDLE)GetWindowThreadProcessId(hwndBar, NULL);
	if(!hThread)
	{
		SendMessage(hwnd, WM_USER+1, 0, 2);
		return;
	}
	
	// install an hook to thread of taskbar
	hhook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hmod,
		(DWORD)hThread);
	if(!hhook)
	{
		SendMessage(hwnd, WM_USER+1, 0, 3);
		return;
	}
	
	// refresh the taskbar
	PostMessage(hwndBar, WM_SIZE, SIZE_RESTORED, 0);
	
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
					SendMessage(hwndChild, WM_NULL, 0, 0);
					break;
				}
			}
			break;
		}
		hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
	}
}

/*------------------------------------------------
  API: uninstall my hook
--------------------------------------------------*/
void WINAPI HookEnd(void)
{
	HWND hwnd;
	
	// force the clock to end cunstomizing
	if(hwndClock && IsWindow(hwndClock))
		SendMessage(hwndClock, WM_COMMAND, 102, 0);
	// uninstall my hook
	if(hhook != NULL)
		UnhookWindowsHookEx(hhook); hhook = NULL;
	
	// refresh the clock
	if(hwndClock && IsWindow(hwndClock))
		PostMessage(hwndClock, WM_TIMER, 0, 0);
	hwndClock = NULL;
	
	// refresh the taskbar
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd)
	{
		PostMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

/*------------------------------------------------
  hook procedure
--------------------------------------------------*/
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LPCWPSTRUCT pcwps;
	char classname[80];

	pcwps = (LPCWPSTRUCT)lParam;

	if(nCode >= 0 && pcwps && pcwps->hwnd)
	{
		if(hwndClock == NULL &&   // if this message is sent to the clock
			oldWndProc == NULL &&
			GetClassName(pcwps->hwnd, classname, 80) > 0 &&
			lstrcmpi(classname, "TrayClockWClass") == 0)
		{
			// initialize  cf. wndproc.c
			InitClock(pcwps->hwnd);
		}
		
		if(bWin2000 && nAlphaStartMenu < 255)
		{
			if(GetClassName(pcwps->hwnd, classname, 80) > 0 &&
				(lstrcmpi(classname, "BaseBar") == 0 || 
				lstrcmpi(classname, "DV2ControlHost") == 0))
			{
				SetLayeredStartMenu(pcwps->hwnd);
			}
		}
	}
	
	return CallNextHookEx(hhook, nCode, wParam, lParam);
}

