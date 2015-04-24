/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"
#include "alarms.h"


#define SERVICENAME _T("mp")
#define COMMANDPREFIX _T("/") SERVICENAME

#define WMP_PAUSE	32808
#define WMP_NEXT	0x497B

TCHAR szGamePrefix[] = COMMANDPREFIX;

HINSTANCE hInst;
int hLangpack;

HANDLE hTopToolbarButton;

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
	// {4DD7762B-D612-4f84-AA86-068F17859B6D}
	{0x4dd7762b, 0xd612, 0x4f84, {0xaa, 0x86, 0x6, 0x8f, 0x17, 0x85, 0x9b, 0x6d}}
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

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
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

void ShowPopup(MCONTACT hContact, const TCHAR *msg)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		TCHAR *lpzContactName = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);

		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = hIconList1;
		mir_tstrncpy(ppd.lptzContactName, lpzContactName,MAX_CONTACTNAME);
		mir_tstrncpy(ppd.lptzText, msg,  MAX_SECONDLINE);
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = RGB(0,0,0);
		ppd.PluginWindowProc = PopupDlgProc;
		ppd.PluginData = 0;
		ppd.iSeconds = 3;

		//Now that every field has been filled, we want to see the popup.
		PUAddPopupT(&ppd);
	}
}

HBITMAP LoadBmpFromIcon(int IdRes)
{
	HICON hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IdRes));

	RECT rc;
	BITMAPINFOHEADER bih = {0};

	HBRUSH hBkgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	bih.biSize = sizeof(bih);
	bih.biBitCount = 24;
	bih.biPlanes = 1;
	bih.biCompression = BI_RGB;
	bih.biHeight = 16;
	bih.biWidth = 20;
	rc.top = rc.left = 0;
	rc.right = bih.biWidth;
	rc.bottom = bih.biHeight;
	HDC hdc = GetDC(NULL);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, bih.biWidth, bih.biHeight);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
	FillRect(hdcMem, &rc, hBkgBrush);
	DrawIconEx(hdcMem, 2, 0, hIcon, 16, 16, 0, NULL, DI_NORMAL);


	SelectObject(hdcMem, hoBmp);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdc);
	DeleteObject(hBkgBrush);

	DeleteObject(hIcon);

	return hBmp;
}

static int InitTopToolbarButton(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconUp = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TBUP));
	ttb.hIconDn = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TBDN));
	ttb.pszService = MODULE "/NewAlarm";
	ttb.dwFlags = TTBBF_VISIBLE;
	ttb.name = ttb.pszTooltipUp = LPGEN("Set alarm");
	hTopToolbarButton = TopToolbar_AddButton(&ttb);
	return 0;
}

static int MainInit(WPARAM, LPARAM)
{
	// TopToolbar support
	HookEvent(ME_TTB_MODULELOADED, InitTopToolbarButton);

	InitFrames();
	return 0;
}

static int MainDeInit(WPARAM, LPARAM)
{
	DeinitFrames();
	DeinitList();
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	// ensure datetime picker is loaded
	INITCOMMONCONTROLSEX ccx;
	ccx.dwSize = sizeof(ccx);
	ccx.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccx);

	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainDeInit);

	LoadOptions();
	InitList();

	// initialize icons
	InitIcons();

	HookEvent(ME_OPT_INITIALISE, OptInit);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
