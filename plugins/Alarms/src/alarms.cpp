/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

#define SERVICENAME L"mp"
#define COMMANDPREFIX L"/" SERVICENAME

#define WMP_PAUSE	32808
#define WMP_NEXT	0x497B

wchar_t szGamePrefix[] = COMMANDPREFIX;

CMPlugin g_plugin;

HANDLE hTopToolbarButton;

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
	// {4DD7762B-D612-4f84-AA86-068F17859B6D}
	{0x4dd7762b, 0xd612, 0x4f84, {0xaa, 0x86, 0x6, 0x8f, 0x17, 0x85, 0x9b, 0x6d}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
			PUDeletePopup(hWnd);
			return TRUE;
		}
		break;

	case UM_FREEPLUGINDATA:
		return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowPopup(MCONTACT hContact, const wchar_t *msg)
{
	wchar_t *lpzContactName = Clist_GetContactDisplayName(hContact);

	POPUPDATAW ppd;
	ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
	ppd.lchIcon = hIconList1;
	mir_wstrncpy(ppd.lpwzContactName, lpzContactName, MAX_CONTACTNAME);
	mir_wstrncpy(ppd.lpwzText, msg, MAX_SECONDLINE);
	ppd.colorBack = GetSysColor(COLOR_BTNFACE);
	ppd.colorText = RGB(0, 0, 0);
	ppd.PluginWindowProc = PopupDlgProc;
	ppd.PluginData = nullptr;
	ppd.iSeconds = 3;
	PUAddPopupW(&ppd);
}

HBITMAP LoadBmpFromIcon(int IdRes)
{
	HICON hIcon = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IdRes));

	BITMAPINFOHEADER bih = { 0 };
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;
	bih.biHeight = 16;
	bih.biWidth = 20;

	RECT rc;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;

	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);

	HBRUSH hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	FillRect(hdcMem, &rc, hBkgBrush);
	DrawIconEx(hdcMem, 2, 0, hIcon, 16, 16, 0, nullptr, DI_NORMAL);

	SelectObject(hdcMem, hoBmp);
	DeleteDC(hdcMem);
	ReleaseDC(nullptr, hdc);
	DeleteObject(hBkgBrush);

	DeleteObject(hIcon);

	return hBmp;
}

static int InitTopToolbarButton(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.hIconUp = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_TBUP));
	ttb.hIconDn = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_TBDN));
	ttb.pszService = MODULENAME "/NewAlarm";
	ttb.dwFlags = TTBBF_VISIBLE;
	ttb.name = ttb.pszTooltipUp = LPGEN("Set alarm");
	hTopToolbarButton = g_plugin.addTTB(&ttb);
	return 0;
}

static int MainInit(WPARAM, LPARAM)
{
	// TopToolbar support
	HookTemporaryEvent(ME_TTB_MODULELOADED, InitTopToolbarButton);

	InitFrames();
	return 0;
}

static int MainDeInit(WPARAM, LPARAM)
{
	DeinitFrames();
	DeinitList();
	return 0;
}

int CMPlugin::Load()
{
	// ensure datetime picker is loaded
	INITCOMMONCONTROLSEX ccx;
	ccx.dwSize = sizeof(ccx);
	ccx.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccx);

	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainDeInit);

	LoadOptions();
	InitList();

	// initialize icons
	InitIcons();

	HookEvent(ME_OPT_INITIALISE, OptInit);

	return 0;
}
