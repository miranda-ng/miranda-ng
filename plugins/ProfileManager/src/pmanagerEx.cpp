/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "Common.h"

#define SRV_LOAD_PM    "Database/LoadPM"
#define SRV_CHANGE_PM  "Database/ChangePM"
#define SRV_CHECK_DB   "Database/CheckDb"
#define SRV_RESTART_ME "System/RestartMe"

HINSTANCE hInst;
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
	// {7EEEB55E-9D83-4E1A-A12F-8F13F1A124FbB}
	{ 0x7eeeb55e, 0x9d83, 0x4e1a, { 0xa1, 0x2f, 0x8f, 0x13, 0xf1, 0xa1, 0x24, 0xfb } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ChangePM(WPARAM, LPARAM)
{
	TCHAR fn[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
	ShellExecute(0, _T("open"), fn, _T("/ForceShowPM"), _T(""), 1);
	CallService("CloseAction", 0, 0);
	return 0;
}

static INT_PTR LoadPM(WPARAM, LPARAM)
{
	TCHAR fn[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), fn, SIZEOF(fn));
	ShellExecute(0, _T("open"), fn, _T("/ForceShowPM"), _T(""), 1);
	return 0;
}

static INT_PTR CheckDb(WPARAM, LPARAM)
{
	if (MessageBox(0, TranslateT("Miranda NG will exit and Database checker will start.\n\nAre you sure you want to do this?"), TranslateT("Check Database"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		TCHAR mirandaPath[MAX_PATH], cmdLine[100];
		PROCESS_INFORMATION pi;
		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
		mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /svc:dbchecker"), mirandaPath, GetCurrentProcessId());
		CallService("CloseAction", 0, 0);
		CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
	return 0;
}

static INT_PTR RestartMe(WPARAM, LPARAM)
{
	CallService(MS_SYSTEM_RESTART, 1, 0);
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Load profile"), SRV_LOAD_PM, IDI_LoadPM },
	{ LPGEN("Change profile"), SRV_CHANGE_PM, IDI_ChangePM },
	{ LPGEN("Check database"), SRV_CHECK_DB, IDI_Dbchecker },
	{ LPGEN("Restart"), SRV_RESTART_ME, IDI_Restart }
};

static int OnModulesLoaded(WPARAM, LPARAM)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -500200000;
	mi.pszPopupName = LPGEN("Database");

	for (int i = 0; i < SIZEOF(iconList); i++) {
		mi.pszName = iconList[i].szDescr;
		mi.pszService = iconList[i].szName;
		mi.icolibItem = iconList[i].hIcolib;
		if (i == 3)
			mi.pszPopupName = NULL;
		Menu_AddMainMenuItem(&mi);
	}

	Menu_AddTrayMenuItem(&mi);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	Icon_Register(hInst, LPGEN("Profile manager"), iconList, SIZEOF(iconList));

	CreateServiceFunction(SRV_LOAD_PM, LoadPM);
	CreateServiceFunction(SRV_CHECK_DB, CheckDb);
	CreateServiceFunction(SRV_CHANGE_PM, ChangePM);
	CreateServiceFunction(SRV_RESTART_ME, RestartMe);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
