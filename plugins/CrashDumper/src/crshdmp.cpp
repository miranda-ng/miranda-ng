/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMPlugin g_plugin;

LCID packlcid;
HANDLE hVerInfoFolder;
HMODULE hMsftedit;

wchar_t* vertxt;
wchar_t* profname;
wchar_t* profpath;

wchar_t CrashLogFolder[MAX_PATH], VersionInfoFolder[MAX_PATH];

CDlgBase *pViewDialog = nullptr;

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
	// {F62C1D7A-FFA4-4065-A251-4C9DD9101CC8}
	{ 0xf62c1d7a, 0xffa4, 0x4065, { 0xa2, 0x51, 0x4c, 0x9d, 0xd9, 0x10, 0x1c, 0xc8 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	bCatchCrashes(MODULENAME, "CatchCrashes", 1),
	bClassicDates(MODULENAME, "ClassicDates", 1),
	bUseSubFolder(MODULENAME, "SubFolders", 1),
	bSuccessPopups(MODULENAME, "SuccessPopups", 1)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaInterfaces - returns the protocol interface to the core

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR StoreVersionInfoToFile(WPARAM, LPARAM lParam)
{
	CreateDirectoryTreeW(VersionInfoFolder);

	wchar_t path[MAX_PATH];
	mir_snwprintf(path, L"%s\\VersionInfo.txt", VersionInfoFolder);

	HANDLE hDumpFile = CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hDumpFile != INVALID_HANDLE_VALUE) {
		CMStringW buffer;
		PrintVersionInfo(buffer, (unsigned int)lParam | VI_FLAG_PRNVAR);

		WriteUtfFile(hDumpFile, T2Utf(buffer.c_str()));
		CloseHandle(hDumpFile);

		ShowMessage(3, TranslateT("Version Info stored into file %s"), path);
	}
	else ShowMessage(2, TranslateT("Version Info file %s is inaccessible"), path);

	return 0;
}

INT_PTR StoreVersionInfoToClipboard(WPARAM, LPARAM lParam)
{
	CMStringW buffer;
	WriteBBFile(buffer, true);
	PrintVersionInfo(buffer, (unsigned int)lParam | VI_FLAG_PRNVAR | VI_FLAG_FORMAT);
	WriteBBFile(buffer, false);

	Utils_ClipboardCopy(buffer);
	return 0;
}

INT_PTR GetVersionInfo(WPARAM wParam, LPARAM lParam)
{
	int result = 1; //failure
	if (lParam != NULL) {
		CMStringW buffer;
		PrintVersionInfo(buffer, (unsigned int)wParam);
		char **retData = (char **)lParam;
		*retData = mir_utf8encodeW(buffer.c_str());
		if (*retData)
			result = 0; //success
	}
	return result;
}

INT_PTR OpenUrl(WPARAM, LPARAM)
{
	ShellExecute(nullptr, L"explore", CrashLogFolder, nullptr, nullptr, SW_SHOW);
	return 0;
}

INT_PTR ServiceModeLaunch(WPARAM, LPARAM)
{
	g_plugin.bServiceMode = true;
	ViewVersionInfo(1, 0);
	return SERVICE_ONLYDB;
}

static int FoldersPathChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hVerInfoFolder, VersionInfoFolder, MAX_PATH, L"");
	return 0;
}

static int ToolbarModulesLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszService = MS_CRASHDUMPER_STORETOCLIP;
	ttb.name = ttb.pszTooltipUp = LPGEN("Version Information To Clipboard");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_VITOCLIP);
	ttb.dwFlags = TTBBF_VISIBLE;
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_CRASHDUMPER_STORETOFILE;
	ttb.name = ttb.pszTooltipUp = LPGEN("Version Information To File");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_VITOFILE);
	ttb.dwFlags = 0;
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_CRASHDUMPER_VIEWINFO;
	ttb.name = ttb.pszTooltipUp = LPGEN("Show Version Information");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_VISHOW);
	g_plugin.addTTB(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	char temp[MAX_PATH];
	Miranda_GetVersionText(temp, _countof(temp));
	crs_a2t(vertxt, temp);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		replaceStrW(profpath, L"%miranda_userdata%");

		// Removed because it isn't available on Load()
		//		hCrashLogFolder = FoldersRegisterCustomPathW(MODULENAME, LPGEN("Crash Reports"), CrashLogFolder);
		hVerInfoFolder = FoldersRegisterCustomPathW(MODULENAME, LPGEN("Version Information"), VersionInfoFolder);

		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersPathChanged);
		FoldersPathChanged(0, 0);
	}

	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Version Information"), 2000089999, g_plugin.getIconHandle(IDI_VI));
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "9A7A9C76-7FD8-4C05-B402-6C46060C2D78");

	SET_UID(mi, 0x52930e40, 0xb2ee, 0x4433, 0xad, 0x77, 0xf5, 0x42, 0xe, 0xf6, 0x57, 0xc1);
	mi.position = 2000089995;
	mi.name.a = LPGEN("Copy to clipboard");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_VITOCLIP);
	mi.pszService = MS_CRASHDUMPER_STORETOCLIP;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x54109094, 0x494e, 0x4535, 0x9c, 0x3a, 0xf6, 0x9e, 0x9a, 0xf7, 0xcd, 0xbe);
	mi.position = 2000089996;
	mi.name.a = LPGEN("Store to file");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_VITOFILE);
	mi.pszService = MS_CRASHDUMPER_STORETOFILE;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x4004f9ee, 0x2c5a, 0x420a, 0xb1, 0x54, 0x3e, 0x47, 0xc1, 0xde, 0x46, 0xec);
	mi.position = 2000089997;
	mi.name.a = LPGEN("Show");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_VISHOW);
	mi.pszService = MS_CRASHDUMPER_VIEWINFO;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x8526469a, 0x8ab4, 0x4dd4, 0xad, 0xbf, 0x51, 0xfd, 0x71, 0x10, 0xd3, 0x3c);
	mi.position = 2000089998;
	mi.name.a = LPGEN("Show with DLLs");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_VIUPLOAD);
	mi.pszService = MS_CRASHDUMPER_VIEWINFO;
	Menu_ConfigureItem(Menu_AddMainMenuItem(&mi), MCI_OPT_EXECPARAM, 1);

	if (g_plugin.bCatchCrashes && !g_plugin.bNeedRestart) {
		SET_UID(mi, 0xecae52f2, 0xd601, 0x4f85, 0x87, 0x9, 0xec, 0x8e, 0x84, 0xfe, 0x1b, 0x3c);
		mi.position = 2000099990;
		mi.name.a = LPGEN("Open crash report directory");
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_FILE);
		mi.pszService = MS_CRASHDUMPER_URL;
		Menu_AddMainMenuItem(&mi);
	}

	HOTKEYDESC hk = {};
	hk.szSection.a = MODULENAME;

	hk.szDescription.a = LPGEN("Copy Version Info to clipboard");
	hk.pszName = "CopyVerInfo";
	hk.pszService = MS_CRASHDUMPER_STORETOCLIP;
	g_plugin.addHotkey(&hk);

	hk.szDescription.a = LPGEN("Show Version Info");
	hk.pszName = "ShowVerInfo";
	hk.pszService = MS_CRASHDUMPER_VIEWINFO;
	g_plugin.addHotkey(&hk);

	if (g_plugin.bCatchCrashes && !g_plugin.bNeedRestart)
		SetExceptionHandler();

	HookEvent(ME_TTB_MODULELOADED, ToolbarModulesLoaded);

	if (g_plugin.bServiceMode)
		ViewVersionInfo(0, 0);

	return 0;
}

int CMPlugin::Load()
{
	hMsftedit = LoadLibrary(L"Msftedit.dll");
	if (hMsftedit == nullptr)
		return 1;

	profname = Utils_ReplaceVarsW(L"%miranda_profilename%.dat");
	profpath = Utils_ReplaceVarsW(L"%miranda_userdata%");
	if (g_plugin.bCatchCrashes && !g_plugin.bNeedRestart)
		mir_snwprintf(CrashLogFolder, L"%s\\CrashLog", profpath);
	wcsncpy_s(VersionInfoFolder, profpath, _TRUNCATE);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	
	packlcid = (LCID)Langpack_GetDefaultLocale();

	InitIcons();

	if (g_plugin.bCatchCrashes && !g_plugin.bNeedRestart)
		InitExceptionHandler();

	CreateServiceFunction(MS_CRASHDUMPER_STORETOFILE, StoreVersionInfoToFile);
	CreateServiceFunction(MS_CRASHDUMPER_STORETOCLIP, StoreVersionInfoToClipboard);
	CreateServiceFunction(MS_CRASHDUMPER_VIEWINFO, ViewVersionInfo);
	CreateServiceFunction(MS_CRASHDUMPER_GETINFO, GetVersionInfo);
	CreateServiceFunction(MS_CRASHDUMPER_URL, OpenUrl);
	CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceModeLaunch);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if ((g_plugin.bCatchCrashes && !g_plugin.bNeedRestart) || (!g_plugin.bCatchCrashes && g_plugin.bNeedRestart))
		DestroyExceptionHandler();

	mir_free(profpath);
	mir_free(profname);
	mir_free(vertxt);
	FreeLibrary(hMsftedit);
	return 0;
}
