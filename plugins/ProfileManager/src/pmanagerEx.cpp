/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

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
	wchar_t fn[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), fn, _countof(fn));
	ShellExecute(0, L"open", fn, L"/ForceShowPM", L"", 1);
	CallService("CloseAction", 0, 0);
	return 0;
}

static INT_PTR LoadPM(WPARAM, LPARAM)
{
	wchar_t fn[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), fn, _countof(fn));
	ShellExecute(0, L"open", fn, L"/ForceShowPM", L"", 1);
	return 0;
}

static INT_PTR CheckDb(WPARAM, LPARAM)
{
	if (MessageBox(0, TranslateT("Miranda NG will exit and Database checker will start.\n\nAre you sure you want to do this?"), TranslateT("Check database"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
		wchar_t mirandaPath[MAX_PATH], cmdLine[100];
		PROCESS_INFORMATION pi;
		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		GetModuleFileName(NULL, mirandaPath, _countof(mirandaPath));
		mir_snwprintf(cmdLine, L"\"%s\" /restart:%d /svc:dbchecker", mirandaPath, GetCurrentProcessId());
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

static MUUID uids[_countof(iconList)] = 
{
	{ 0xF57779C7, 0xB593, 0x4851, 0x94, 0x3A, 0xEB, 0x90, 0x95, 0x40, 0x0D, 0x2A },
	{ 0x701BFC6C, 0x6963, 0x4A4C, 0xA6, 0x39, 0x9B, 0xC0, 0x97, 0x64, 0xFD, 0xCE },
	{ 0x3C4409EC, 0xF733, 0x4C33, 0x94, 0x0C, 0x35, 0xAF, 0x36, 0xC6, 0x64, 0x32 },
	{ 0x5A2EDCCD, 0xB43B, 0x48FA, 0x8A, 0xE8, 0xB5, 0x8B, 0xD7, 0xA5, 0x5A, 0x13 }
};

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	Icon_Register(hInst, LPGEN("Profile manager"), iconList, _countof(iconList));

	CreateServiceFunction(SRV_LOAD_PM, LoadPM);
	CreateServiceFunction(SRV_CHECK_DB, CheckDb);
	CreateServiceFunction(SRV_CHANGE_PM, ChangePM);
	CreateServiceFunction(SRV_RESTART_ME, RestartMe);

	CMenuItem mi;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENW("Database"), -500200000);

	for (int i = 0; i < _countof(iconList); i++) {
		mi.name.a = iconList[i].szDescr;
		mi.pszService = iconList[i].szName;
		mi.hIcolibItem = iconList[i].hIcolib;
		mi.uid = uids[i];
		if (i == 3)
			mi.root = NULL;

		Menu_AddMainMenuItem(&mi);
	}
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
