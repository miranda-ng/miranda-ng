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

#include "utils.h"

int hLangpack;

HINSTANCE hInst;
DWORD mirandaVersion;
LCID packlcid;
//HANDLE hCrashLogFolder, hVerInfoFolder;
HANDLE hVerInfoFolder;

TCHAR* vertxt;
TCHAR* profname;
TCHAR* profpath;

TCHAR CrashLogFolder[MAX_PATH], VersionInfoFolder[MAX_PATH];

bool servicemode, clsdates, dtsubfldr, catchcrashes, needrestart = 0;

extern HWND hViewWnd;

static const PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F62C1D7A-FFA4-4065-A251-4C9DD9101CC8}
	{ 0xf62c1d7a, 0xffa4, 0x4065, { 0xa2, 0x51, 0x4c, 0x9d, 0xd9, 0x10, 0x1c, 0xc8 } }
};

const PLUGININFOEX* GetPluginInfoEx(void) { return &pluginInfoEx; }

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirVersion)
{
	::mirandaVersion = mirVersion;
	return (PLUGININFOEX*)&pluginInfoEx;
}

// MirandaInterfaces - returns the protocol interface to the core
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

INT_PTR StoreVersionInfoToFile(WPARAM, LPARAM lParam)
{
	CreateDirectoryTree(VersionInfoFolder);

	TCHAR path[MAX_PATH];
	mir_sntprintf(path, MAX_PATH, TEXT("%s\\VersionInfo.txt"), VersionInfoFolder);

	HANDLE hDumpFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE) {
		CMString buffer;
		PrintVersionInfo(buffer, (unsigned int)lParam | VI_FLAG_PRNVAR);

		char* bufu = mir_utf8encodeT(buffer.c_str());
		WriteUtfFile(hDumpFile, bufu);
		mir_free(bufu);

		CloseHandle(hDumpFile);

		ShowMessage(3, TranslateT("Version Info stored into file %s"), path);
	}
	else ShowMessage(2, TranslateT("Version Info file %s is inaccessible"), path);

	return 0;
}

INT_PTR StoreVersionInfoToClipboard(WPARAM, LPARAM lParam)
{
	CMString buffer;
	WriteBBFile(buffer, true);
	PrintVersionInfo(buffer, (unsigned int)lParam | VI_FLAG_PRNVAR | VI_FLAG_FORMAT);
	WriteBBFile(buffer, false);

	StoreStringToClip(buffer);

	return 0;
}

INT_PTR UploadVersionInfo(WPARAM, LPARAM lParam)
{
	CMString buffer;
	PrintVersionInfo(buffer);

	VerTrnsfr *trn = (VerTrnsfr*)mir_alloc(sizeof(VerTrnsfr));
	trn->buf = mir_utf8encodeT(buffer.c_str());
	trn->autot = lParam == 0xa1;

	mir_forkthread(VersionInfoUploadThread, trn);

	return 0;
}


INT_PTR ViewVersionInfo(WPARAM wParam, LPARAM)
{
	if (hViewWnd) {
		SetForegroundWindow(hViewWnd);
		SetFocus(hViewWnd);
	}
	else {
		DWORD dwFlags = wParam ? (VI_FLAG_PRNVAR | VI_FLAG_PRNDLL) : VI_FLAG_PRNVAR;
		hViewWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_VIEWVERSION), NULL, DlgProcView, dwFlags);
	}

	return 0;
}

INT_PTR GetVersionInfo(WPARAM wParam, LPARAM lParam)
{
	int result = 1; //failure
	if (lParam != NULL) {
		CMString buffer;
		PrintVersionInfo(buffer, (unsigned int)wParam);
		char **retData = (char **)lParam;
		*retData = mir_utf8encodeT(buffer.c_str());
		if (*retData)
			result = 0; //success
	}
	return result;
}

INT_PTR OpenUrl(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case 0:
		ShellExecute(NULL, TEXT("explore"), CrashLogFolder, NULL, NULL, SW_SHOW);
		break;

	case 1:
		OpenAuthUrl("http://vi.miranda-ng.org/detail/%s");
		break;
	}
	return 0;
}

INT_PTR CopyLinkToClipboard(WPARAM, LPARAM)
{
	TCHAR *tmp;
	TCHAR buffer[MAX_PATH];

	tmp = db_get_wsa(NULL, PluginName, "Username");
	mir_sntprintf(buffer, SIZEOF(buffer), _T("http://vi.miranda-ng.org/detail/%s"), tmp);
	int bufLen = (sizeof(buffer) + 1) * sizeof(TCHAR);
	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, bufLen);
	LPSTR buf = (LPSTR)GlobalLock(hData);
	memcpy(buf, buffer, bufLen);
	
	OpenClipboard(NULL);
	EmptyClipboard();

	SetClipboardData(CF_UNICODETEXT, hData);
	CloseClipboard();
	return 0;
}

INT_PTR ServiceModeLaunch(WPARAM, LPARAM)
{
	servicemode = true;
	ViewVersionInfo(1, 0);
	return SERVICE_ONLYDB;
}

static int FoldersPathChanged(WPARAM, LPARAM)
{
	FOLDERSGETDATA fgd = { 0 };
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = MAX_PATH;
	fgd.flags = FF_TCHAR;
//	fgd.szPathT = CrashLogFolder;
//	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hCrashLogFolder, (LPARAM)&fgd);

	fgd.szPathT = VersionInfoFolder;
	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hVerInfoFolder, (LPARAM)&fgd);
	return 0;
}

int OptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = PluginName;
	odp.pszGroup = LPGEN("Services");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

static int ToolbarModulesLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.pszService = MS_CRASHDUMPER_STORETOCLIP;
	ttb.name = ttb.pszTooltipUp = LPGEN("Version Information To Clipboard");
	ttb.hIconHandleUp = GetIconHandle(IDI_VITOCLIP);
	ttb.dwFlags = TTBBF_VISIBLE;
	TopToolbar_AddButton(&ttb);

	ttb.pszService = MS_CRASHDUMPER_STORETOFILE;
	ttb.name = ttb.pszTooltipUp = LPGEN("Version Information To File");
	ttb.hIconHandleUp = GetIconHandle(IDI_VITOFILE);
	ttb.dwFlags = 0;
	TopToolbar_AddButton(&ttb);

	ttb.pszService = MS_CRASHDUMPER_VIEWINFO;
	ttb.name = ttb.pszTooltipUp = LPGEN("Show Version Information");
	ttb.hIconHandleUp = GetIconHandle(IDI_VISHOW);
	TopToolbar_AddButton(&ttb);

	ttb.pszService = MS_CRASHDUMPER_UPLOAD;
	ttb.name = ttb.pszTooltipUp = LPGEN("Upload Version Information");
	ttb.hIconHandleUp = GetIconHandle(IDI_VIUPLOAD);
	TopToolbar_AddButton(&ttb);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	char temp[MAX_PATH];
	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)SIZEOF(temp), (LPARAM)temp);
	crs_a2t(vertxt, temp);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		replaceStrT(profpath, _T("%miranda_userdata%"));

		// Removed because it isn't available on Load()
//		hCrashLogFolder = FoldersRegisterCustomPathT(PluginName, LPGEN("Crash Reports"), CrashLogFolder);
		hVerInfoFolder = FoldersRegisterCustomPathT(PluginName, LPGEN("Version Information"), VersionInfoFolder);

		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersPathChanged);
		FoldersPathChanged(0, 0);
	}

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.popupPosition = 2000089999;
	mi.position = 2000089999;
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	mi.icolibItem = GetIconHandle(IDI_VI);
	mi.ptszName = LPGENT("Version Information");
	mi.pszPopupName = (char *)-1;
	HANDLE hMenuRoot = Menu_AddMainMenuItem(&mi);

	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR;
	mi.pszPopupName = (char *)hMenuRoot;
	mi.popupPosition = 0;

	mi.position = 2000089995;
	mi.ptszName = LPGENT("Copy to clipboard");
	mi.icolibItem = GetIconHandle(IDI_VITOCLIP);
	mi.pszService = MS_CRASHDUMPER_STORETOCLIP;
	Menu_AddMainMenuItem(&mi);

	mi.position = 2000089996;
	mi.ptszName = LPGENT("Store to file");
	mi.icolibItem = GetIconHandle(IDI_VITOFILE);
	mi.pszService = MS_CRASHDUMPER_STORETOFILE;
	Menu_AddMainMenuItem(&mi);

	mi.position = 2000089997;
	mi.ptszName = LPGENT("Show");
	mi.icolibItem = GetIconHandle(IDI_VISHOW);
	mi.pszService = MS_CRASHDUMPER_VIEWINFO;
	Menu_AddMainMenuItem(&mi);

	mi.popupPosition = 1;
	mi.position = 2000089998;
	mi.ptszName = LPGENT("Show with DLLs");
	mi.icolibItem = GetIconHandle(IDI_VIUPLOAD);
	mi.pszService = MS_CRASHDUMPER_VIEWINFO;
	Menu_AddMainMenuItem(&mi);

	mi.popupPosition = 0;
	mi.position = 2000089999;
	mi.ptszName = LPGENT("Upload");
	mi.icolibItem = GetIconHandle(IDI_VIUPLOAD);
	mi.pszService = MS_CRASHDUMPER_UPLOAD;
	Menu_AddMainMenuItem(&mi);

	mi.popupPosition = 0;
	mi.position = 2000089999;
	mi.ptszName = LPGENT("Copy link to clipboard");
	mi.icolibItem = GetIconHandle(IDI_LINKTOCLIP);//need icon
	mi.pszService = MS_CRASHDUMPER_URLTOCLIP;
	Menu_AddMainMenuItem(&mi);

	if (catchcrashes && !needrestart) {
		mi.position = 2000099990;
		mi.ptszName = LPGENT("Open crash report directory");
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_FILE);
		mi.pszService = MS_CRASHDUMPER_URL;
		Menu_AddMainMenuItem(&mi);
	}

	mi.popupPosition = 1;
	mi.position = 2000099991;
	mi.ptszName = LPGENT("Open online Version Info");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_EVENT_URL);
	mi.pszService = MS_CRASHDUMPER_URL;
	Menu_AddMainMenuItem(&mi);

	HOTKEYDESC hk = { 0 };
	hk.cbSize = sizeof(hk);
	hk.pszSection = PluginName;

	hk.pszDescription = LPGEN("Copy Version Info to clipboard");
	hk.pszName = "CopyVerInfo";
	hk.pszService = MS_CRASHDUMPER_STORETOCLIP;
	Hotkey_Register(&hk);

	hk.pszDescription = LPGEN("Show Version Info");
	hk.pszName = "ShowVerInfo";
	hk.pszService = MS_CRASHDUMPER_VIEWINFO;
	Hotkey_Register(&hk);
	
	UploadInit();

	if (catchcrashes && !needrestart)
		SetExceptionHandler();

	HookEvent(ME_TTB_MODULELOADED, ToolbarModulesLoaded);

	if (servicemode)
		ViewVersionInfo(0, 0);
	else if (db_get_b(NULL, PluginName, "UploadChanged", 0) && !ProcessVIHash(false))
		UploadVersionInfo(0, 0xa1);

	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	DestroyAllWindows();
	UploadClose();
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	if (LoadLibraryA("Msftedit.dll") == NULL)
		return 1;

	clsdates = db_get_b(NULL, PluginName, "ClassicDates", 1) != 0;
	dtsubfldr = db_get_b(NULL, PluginName, "SubFolders", 1) != 0;
	catchcrashes = db_get_b(NULL, PluginName, "CatchCrashes", 1) != 0;

	mir_getLP(&pluginInfoEx);

	profname = Utils_ReplaceVarsT(_T("%miranda_profilename%.dat"));
	profpath = Utils_ReplaceVarsT(_T("%miranda_userdata%"));
	if (catchcrashes && !needrestart)
		mir_sntprintf(CrashLogFolder, MAX_PATH, TEXT("%s\\CrashLog"), profpath);
	mir_sntprintf(VersionInfoFolder, MAX_PATH, TEXT("%s"), profpath);


	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);

	packlcid = (LCID)Langpack_GetDefaultLocale();

	InitIcons();

	if (catchcrashes && !needrestart)
		InitExceptionHandler();

	CreateServiceFunction(MS_CRASHDUMPER_STORETOFILE, StoreVersionInfoToFile);
	CreateServiceFunction(MS_CRASHDUMPER_STORETOCLIP, StoreVersionInfoToClipboard);
	CreateServiceFunction(MS_CRASHDUMPER_VIEWINFO, ViewVersionInfo);
	CreateServiceFunction(MS_CRASHDUMPER_GETINFO, GetVersionInfo);
	CreateServiceFunction(MS_CRASHDUMPER_UPLOAD, UploadVersionInfo);
	CreateServiceFunction(MS_CRASHDUMPER_URL, OpenUrl);
	CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceModeLaunch);
	CreateServiceFunction(MS_CRASHDUMPER_URLTOCLIP, CopyLinkToClipboard);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	DestroyAllWindows();

	if ((catchcrashes && !needrestart) || (!catchcrashes && needrestart))
		DestroyExceptionHandler();

	mir_free(profpath);
	mir_free(profname);
	mir_free(vertxt);
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	hInst = hinstDLL;
	return TRUE;
}
