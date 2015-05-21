// ZeroSwitch.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

HINSTANCE hInst;
HHOOK hHook;
HWND hDummyWnd = NULL, hHelperWnd = NULL, hMirandaWnd = NULL;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {3F1657B1-69CB-4992-9CFC-226C808A5202}
	{ 0x3f1657b1, 0x69cb, 0x4992, { 0x9c, 0xfc, 0x22, 0x6c, 0x80, 0x8a, 0x52, 0x2 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

LRESULT CALLBACK HelperProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE) // Helper window got activated
			SetActiveWindow(hMirandaWnd);
		break;
	case WM_DESTROY:
		if (hWnd == hHelperWnd)
			hHelperWnd = NULL;
		else
			hDummyWnd = NULL;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void CreateHelperWnd()
{
	WNDCLASSEX wcex;
	// First we must check if Miranda's main window has the "Title bar" border style...
	if ((GetWindowLongPtr(hMirandaWnd, GWL_STYLE) & WS_CAPTION) && !(GetWindowLongPtr(hMirandaWnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW))
		return;

	if (hHelperWnd)
		return; // We need only one helper window
	// Widows taskbar workaround
	// We don't want our helper window to be displayed on Windows taskbar.
	// So firt of all we have to create an invisible window and then set it as
	// parent to our window.

	// Register windows class
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = HelperProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadSkinnedIconBig(SKINICON_OTHER_MIRANDA);
	wcex.hCursor = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("ZeroSwitchHlp");
	wcex.hIconSm = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);

	if (NULL == RegisterClassEx(&wcex))
		return; // wtf

	hDummyWnd = CreateWindow(_T("ZeroSwitchHlp"), _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
	if (!hDummyWnd)
		UnregisterClass(_T("ZeroSwitchHlp"), hInst);
	hHelperWnd = CreateWindow(_T("ZeroSwitchHlp"), _T("Miranda NG"), WS_OVERLAPPEDWINDOW | WS_VISIBLE, -100, -100, 90, 90, hDummyWnd, NULL, hInst, NULL);
	if (!hHelperWnd)
	{
		DestroyWindow(hDummyWnd);
		UnregisterClass(_T("ZeroSwitchHlp"), hInst);
	}
}

void DestroyHelperWnd()
{
	if (hHelperWnd)
	{
		DestroyWindow(hHelperWnd);
		DestroyWindow(hDummyWnd);
		UnregisterClass(_T("ZeroSwitchHlp"), hInst);
	}
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PCWPRETSTRUCT pMes;

	if (nCode >= 0)
	{
		pMes = (PCWPRETSTRUCT)lParam; // Get message details
		if (!hMirandaWnd)
			hMirandaWnd = FindWindow(_T("Miranda"), NULL);

		if (pMes->hwnd == hMirandaWnd)
		{
			if (pMes->message == WM_SHOWWINDOW) // We are interested only in this message
			{
				if (pMes->wParam == TRUE) // The window is being shown
					CreateHelperWnd();
				else
					DestroyHelperWnd();
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam); // Pass the message to other hooks in chain
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	if (IsWinVerVistaPlus()) {
		MessageBox(NULL, TranslateT("Plugin works under Windows XP only"), TranslateT("ZeroSwitch plugin failed"), MB_ICONSTOP);
		return 1;
	}

	// Let's setup shop :)
	hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, NULL, GetCurrentThreadId());
	if (hHook == NULL)
		MessageBox(NULL, TranslateT("Oops, we've got a big hook error here :("), TranslateT("ZeroSwitch plugin failed"), MB_ICONSTOP);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hHook)
		UnhookWindowsHookEx(hHook);
	DestroyHelperWnd();
	return 0;
}