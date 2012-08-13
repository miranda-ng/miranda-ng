/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/
#define MIRANDA_VER 0x0A00

#include <tchar.h>
#include <windows.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <win2k.h>

#include "resource.h"

HINSTANCE hInst;

TCHAR fn[MAX_PATH];
TCHAR lmn[MAX_PATH];
TCHAR* pathn;
int hLangpack;
HANDLE hLoadPM, hChangePM;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Miranda IM Profile Changer",
	PLUGIN_MAKE_VERSION(0,0,0,3),
	"Adds a menu item to change or load a different profile of Miranda IM.",
	"Roman Gemini",
	"woobind@ukr.net",
	"© 2008 - 2010 Roman Gemini",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	{0x7eeeb55e, 0x9d83, 0x4e1a, { 0xa1, 0x2f, 0x8f, 0x13, 0xf1, 0xa1, 0x24, 0xfb } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static INT_PTR ChangePM(WPARAM wParam, LPARAM lParam)
{
	GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
	ShellExecute(0, _T("open"), fn, _T("/ForceShowPM"), _T(""), 1);
	CallService("CloseAction", 0, 0);
	return 0;
}

static INT_PTR LoadPM(WPARAM wParam, LPARAM lParam)
{
	GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
	ShellExecute(0, _T("open"), fn, _T("/ForceShowPM"), _T(""), 1);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	CLISTMENUITEM mi;

	mir_getLP(&pluginInfo);

	hLoadPM = CreateServiceFunction("Database/LoadPM", LoadPM);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LoadPM));
	mi.ptszPopupName = _T("Database");
	mi.ptszName = _T("Load profile");
	mi.pszService = "Database/LoadPM";
	Menu_AddMainMenuItem(&mi);

	hChangePM = CreateServiceFunction("Database/ChangePM", ChangePM);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ChangePM));
	mi.ptszPopupName = _T("Database");
	mi.ptszName = _T("Change profile");
	mi.pszService = "Database/ChangePM";
	Menu_AddMainMenuItem(&mi);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(hLoadPM);
	DestroyServiceFunction(hChangePM);

	return 0;
}
