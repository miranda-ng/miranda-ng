/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HINSTANCE hInst;

#define MS_HISTORY_DELETEALLCONTACTHISTORY       "BasicHistory/DeleteAllContactHistory"
#define MS_HISTORY_EXECUTE_TASK       "BasicHistory/ExecuteTask"

HCURSOR     hCurSplitNS, hCurSplitWE;
HANDLE  g_hMainThread=NULL;

HANDLE hServiceShowContactHistory, hServiceDeleteAllContactHistory, hServiceExecuteTask;
HANDLE *hEventIcons = NULL;
int iconsNum = 3;
HANDLE hToolbarButton;
HGENMENU hContactMenu, hDeleteContactMenu;
HGENMENU hTaskMainMenu;
std::vector<HGENMENU> taskMenus;
bool g_SmileyAddAvail = false;
char* metaContactProto = NULL;
const IID IID_ITextDocument={0x8CC497C0, 0xA1DF, 0x11ce, {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}};

#define MODULE "BasicHistory"

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
	// {E25367A2-51AE-4044-BE28-131BC18B71A4}
	{0xe25367a2, 0x51ae, 0x4044, {0xbe, 0x28, 0x13, 0x1b, 0xc1, 0x8b, 0x71, 0xa4}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

TIME_API tmi = {0};
int hLangpack = 0;

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_UIHISTORY, MIID_LAST};

void InitScheduler();
void DeinitScheduler();
int DoLastTask(WPARAM, LPARAM);
INT_PTR ExecuteTaskService(WPARAM wParam, LPARAM lParam);

int PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	bool bHasHistory = db_event_last(hContact) != NULL;
	bool isInList = HistoryWindow::IsInList(GetForegroundWindow());

	Menu_ShowItem(hContactMenu, bHasHistory);
	Menu_ShowItem(hDeleteContactMenu, bHasHistory && isInList);
	return 0;
}

int ToolbarModuleLoaded(WPARAM wParam,LPARAM lParam)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	ttb.name = ttb.pszTooltipUp = LPGEN("Open History");
	ttb.dwFlags = TTBBF_SHOWTOOLTIP;
	ttb.hIconHandleUp = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	hToolbarButton = TopToolbar_AddButton(&ttb);
	return 0;
}

void InitMenuItems()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1000090000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
	mi.pszName = LPGEN("View &History");
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	hContactMenu = Menu_AddContactMenuItem(&mi);

	mi.position = 500060000;
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	Menu_AddMainMenuItem(&mi);

	mi.position = 1000090001;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
	mi.pszName = LPGEN("Delete All User History");
	mi.pszService = MS_HISTORY_DELETEALLCONTACTHISTORY;
	hDeleteContactMenu = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void InitTaskMenuItems()
{
	if (Options::instance->taskOptions.size() > 0) {
		CLISTMENUITEM mi = { sizeof(mi) };
		if (hTaskMainMenu == NULL) {
			mi.position = 500060005;
			mi.flags = CMIF_ROOTPOPUP;
			mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_HISTORY);
			mi.pszName = LPGEN("Execute history task");
			hTaskMainMenu = Menu_AddMainMenuItem(&mi);
		}

		std::vector<TaskOptions>::iterator taskIt = Options::instance->taskOptions.begin();
		std::vector<HGENMENU>::iterator it = taskMenus.begin();
		for (; it != taskMenus.end() && taskIt != Options::instance->taskOptions.end(); ++it, ++taskIt) {
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.hParentMenu = hTaskMainMenu;
			mi.ptszName = (TCHAR*)taskIt->taskName.c_str();
			Menu_ModifyItem((HGENMENU)*it, &mi);
		}

		for (; it != taskMenus.end(); ++it) {
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.flags = CMIM_FLAGS | CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED | CMIF_HIDDEN;
			mi.hParentMenu = hTaskMainMenu;
			Menu_ModifyItem((HGENMENU)*it, &mi);
		}

		int pos = (int)taskMenus.size();
		for (; taskIt != Options::instance->taskOptions.end(); ++taskIt) {
			memset(&mi, 0, sizeof(mi));
			mi.cbSize = sizeof(mi);
			mi.flags = CMIF_CHILDPOPUP | CMIF_ROOTHANDLE | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.pszService = MS_HISTORY_EXECUTE_TASK;
			mi.hParentMenu = hTaskMainMenu;
			mi.popupPosition = pos++;
			mi.ptszName = (TCHAR*)taskIt->taskName.c_str();
			HGENMENU menu = Menu_AddMainMenuItem(&mi);
			taskMenus.push_back(menu);
		}
	}
	else if (hTaskMainMenu != NULL) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_FLAGS | CMIF_ROOTPOPUP | CMIF_HIDDEN;
		Menu_ModifyItem(hTaskMainMenu, &mi);
	}
}

IconItem iconList[] =
{
	{ LPGEN("Incoming message"), "BasicHistory_in", IDI_INM },
	{ LPGEN("Outgoing message"), "BasicHistory_out", IDI_OUTM },
	{ LPGEN("Status change"),    "BasicHistory_status", IDI_STATUS },
	{ LPGEN("Show Contacts"),    "BasicHistory_show", IDI_SHOW },
	{ LPGEN("Hide Contacts"),    "BasicHistory_hide", IDI_HIDE },
	{ LPGEN("Find Next"),        "BasicHistory_findnext", IDI_FINDNEXT },
	{ LPGEN("Find Previous"),    "BasicHistory_findprev", IDI_FINDPREV },
	{ LPGEN("Plus in export"),   "BasicHistory_plusex", IDI_PLUSEX },
	{ LPGEN("Minus in export"),  "BasicHistory_minusex", IDI_MINUSEX },
};										 

void InitIcolib()
{
}

HICON LoadIconEx(int iconId, int big)
{
	for (int i=0; i < SIZEOF(iconList); i++)
		if ( iconList[i].defIconID == iconId)
			return Skin_GetIconByHandle(iconList[i].hIcolib, big);

	return 0;
}

INT_PTR ShowContactHistory(WPARAM hContact, LPARAM lParam)
{
	HistoryWindow::Open(hContact);
	return 0;
}

int PreShutdownHistoryModule(WPARAM, LPARAM)
{
	HistoryWindow::Deinit();
	DeinitScheduler();
	return 0;
}

int HistoryContactDelete(WPARAM wParam, LPARAM)
{
	HistoryWindow::Close(wParam);
	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	InitMenuItems();
	
	TCHAR ftpExe[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, ftpExe))) {
		_tcscat_s(ftpExe, _T("\\WinSCP\\WinSCP.exe"));
		DWORD atr = GetFileAttributes(ftpExe);
		if (atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY) {
#ifdef _WIN64
			if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILESX86, NULL, SHGFP_TYPE_CURRENT, ftpExe))) {
				_tcscat_s(ftpExe, _T("\\WinSCP\\WinSCP.exe"));
				atr = GetFileAttributes(ftpExe);
				if (!(atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY))
					Options::instance->ftpExePathDef = ftpExe;
			}
#endif
		}
		else Options::instance->ftpExePathDef = ftpExe;
	}

	TCHAR* log = _T("%miranda_logpath%\\BasicHistory\\ftplog.txt");
	TCHAR* logAbsolute = Utils_ReplaceVarsT(log);
	Options::instance->ftpLogPath = logAbsolute;
	mir_free(logAbsolute);
	Options::instance->Load();
	InitTaskMenuItems();

	HookEvent(ME_TTB_MODULELOADED, ToolbarModuleLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdownHistoryModule);
	HookEvent(ME_DB_CONTACT_DELETED, HistoryContactDelete);
	HookEvent(ME_FONT_RELOAD, HistoryWindow::FontsChanged);
	HookEvent(ME_SYSTEM_OKTOEXIT, DoLastTask);
	
	if (ServiceExists(MS_SMILEYADD_REPLACESMILEYS))
		g_SmileyAddAvail = true;

	InitScheduler();
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	hTaskMainMenu = NULL;
	DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),&g_hMainThread,0,FALSE,DUPLICATE_SAME_ACCESS);
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);

	hServiceShowContactHistory = CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowContactHistory);
	hServiceDeleteAllContactHistory = CreateServiceFunction(MS_HISTORY_DELETEALLCONTACTHISTORY, HistoryWindow::DeleteAllUserHistory);
	hServiceExecuteTask = CreateServiceFunction(MS_HISTORY_EXECUTE_TASK, ExecuteTaskService);

	Options::instance = new Options();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, Options::InitOptions);

	EventList::Init();
	
	Icon_Register(hInst, LPGEN("History"), iconList, SIZEOF(iconList));
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (g_hMainThread)
		CloseHandle(g_hMainThread);
	g_hMainThread=NULL;
	
	DestroyServiceFunction(hServiceShowContactHistory);
	DestroyServiceFunction(hServiceDeleteAllContactHistory);
	DestroyServiceFunction(hServiceExecuteTask);
	
	HistoryWindow::Deinit();
	
	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);
	
	EventList::Deinit();
	
	if (Options::instance != NULL) {
		Options::instance->Unload();
		delete Options::instance;
		Options::instance = NULL;
	}

	delete [] hEventIcons;

	return 0;
}
