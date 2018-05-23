// ZeroSwitch.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();
}
g_plugin;

HHOOK hHook;
HWND hDummyWnd = nullptr, hHelperWnd = nullptr, hMirandaWnd = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {3F1657B1-69CB-4992-9CFC-226C808A5202}
	{ 0x3f1657b1, 0x69cb, 0x4992, { 0x9c, 0xfc, 0x22, 0x6c, 0x80, 0x8a, 0x52, 0x2 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

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
			hHelperWnd = nullptr;
		else
			hDummyWnd = nullptr;
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
	// So first of all we have to create an invisible window and then set it as
	// parent to our window.

	// Register windows class
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = HelperProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = g_plugin.getInst();
	wcex.hIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA, true);
	wcex.hCursor = nullptr;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"ZeroSwitchHlp";
	wcex.hIconSm = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);

	if (NULL == RegisterClassEx(&wcex))
		return; // wtf

	hDummyWnd = CreateWindow(L"ZeroSwitchHlp", L"", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), nullptr);
	if (!hDummyWnd)
		UnregisterClass(L"ZeroSwitchHlp", g_plugin.getInst());
	hHelperWnd = CreateWindow(L"ZeroSwitchHlp", L"Miranda NG", WS_OVERLAPPEDWINDOW | WS_VISIBLE, -100, -100, 90, 90, hDummyWnd, nullptr, g_plugin.getInst(), nullptr);
	if (!hHelperWnd)
	{
		DestroyWindow(hDummyWnd);
		UnregisterClass(L"ZeroSwitchHlp", g_plugin.getInst());
	}
}

void DestroyHelperWnd()
{
	if (hHelperWnd)
	{
		DestroyWindow(hHelperWnd);
		DestroyWindow(hDummyWnd);
		UnregisterClass(L"ZeroSwitchHlp", g_plugin.getInst());
	}
}

LRESULT CALLBACK CallWndRetProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PCWPRETSTRUCT pMes;

	if (nCode >= 0)
	{
		pMes = (PCWPRETSTRUCT)lParam; // Get message details
		if (!hMirandaWnd)
			hMirandaWnd = g_CLI.hwndContactList;

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
	return CallNextHookEx(nullptr, nCode, wParam, lParam); // Pass the message to other hooks in chain
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Load(void)
{
	if (IsWinVerVistaPlus()) {
		MessageBox(nullptr, TranslateT("Plugin works under Windows XP only"), TranslateT("ZeroSwitch plugin failed"), MB_ICONSTOP);
		return 1;
	}

	// Let's setup shop :)
	hHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, nullptr, GetCurrentThreadId());
	if (hHook == nullptr)
		MessageBox(nullptr, TranslateT("Oops, we've got a big hook error here :("), TranslateT("ZeroSwitch plugin failed"), MB_ICONSTOP);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hHook)
		UnhookWindowsHookEx(hHook);
	DestroyHelperWnd();
	return 0;
}
