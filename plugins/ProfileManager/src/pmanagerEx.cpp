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
#include <m_system.h>
#include <m_genmenu.h>
#include <m_utils.h>
#include <win2k.h>

#include "resource.h"

HINSTANCE hInst;

TCHAR fn[MAX_PATH];
TCHAR lmn[MAX_PATH];
TCHAR* pathn;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Miranda NG Profile Changer",
	PLUGIN_MAKE_VERSION(0,0,0,5),
	"Adds a menu item to change or load a different profile of Miranda NG, restart or run a dbchecker.",
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

static INT_PTR CheckDb(WPARAM wParam, LPARAM lParam)
{
	if (MessageBox( 0, TranslateT("Miranda NG will exit and Database checker will start.\n\nAre you sure you want to do this?"), TranslateT("Check Database"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 ) == IDYES) {
		TCHAR mirandaPath[MAX_PATH], cmdLine[100];
		PROCESS_INFORMATION pi;
		STARTUPINFO si = {0};
		si.cb = sizeof(si);
		GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
		mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /svc:dbchecker"), mirandaPath, GetCurrentProcessId());
		CallService("CloseAction", 0, 0);
		CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
	return 0;
}

static INT_PTR RestartMe(WPARAM wParam, LPARAM lParam)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[100];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
	TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
	CallService("CloseAction", 0, 0);
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	mir_free(profilename);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction("Database/LoadPM", LoadPM);

   // !!!!!!!! check it later
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LoadPM));
	mi.ptszPopupName = _T("Database");
	mi.ptszName = _T("Load profile");
	mi.pszService = "Database/LoadPM";
	Menu_AddMainMenuItem(&mi);

	CreateServiceFunction("Database/ChangePM", ChangePM);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ChangePM));
	mi.ptszPopupName = _T("Database");
	mi.ptszName = _T("Change profile");
	mi.pszService = "Database/ChangePM";
	Menu_AddMainMenuItem(&mi);

	CreateServiceFunction("Database/CheckDb", CheckDb);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_Dbchecker));
	mi.ptszPopupName = _T("Database");
	mi.ptszName = _T("Check database");
	mi.pszService = "Database/CheckDb";
	Menu_AddMainMenuItem(&mi);

	CreateServiceFunction("System/RestartMe", RestartMe);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -500200000;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_Restart));
	mi.ptszPopupName = NULL;
	mi.ptszName = _T("Restart");
	mi.pszService = "System/RestartMe";
	Menu_AddMainMenuItem(&mi);
	Menu_AddTrayMenuItem(&mi);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
