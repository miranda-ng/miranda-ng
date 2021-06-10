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

#define MS_HISTORY_EXECUTE_TASK       "BasicHistory/ExecuteTask"

HCURSOR hCurSplitNS, hCurSplitWE;

HANDLE *hEventIcons = nullptr;
int iconsNum = 3;
HANDLE hToolbarButton;
HGENMENU hContactMenu;
HGENMENU hTaskMainMenu;
std::vector<HGENMENU> taskMenus;
bool g_SmileyAddAvail = false;
char* metaContactProto = nullptr;
const IID IID_ITextDocument = { 0x8CC497C0, 0xA1DF, 0x11ce, {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D} };

CMPlugin g_plugin;

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
	// {E25367A2-51AE-4044-BE28-131BC18B71A4}
	{0xe25367a2, 0x51ae, 0x4044, {0xbe, 0x28, 0x13, 0x1b, 0xc1, 0x8b, 0x71, 0xa4}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

void InitScheduler();
void DeinitScheduler();
int DoLastTask(WPARAM, LPARAM);
INT_PTR ExecuteTaskService(WPARAM wParam, LPARAM lParam);

int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hContactMenu, db_event_last(hContact) != NULL);
	return 0;
}

int ToolbarModuleLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	ttb.name = ttb.pszTooltipUp = LPGEN("Open History");
	ttb.dwFlags = TTBBF_SHOWTOOLTIP;
	ttb.hIconHandleUp = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	hToolbarButton = g_plugin.addTTB(&ttb);
	return 0;
}

void InitMenuItems()
{
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x28848d7a, 0x6995, 0x4799, 0x82, 0xd7, 0x18, 0x40, 0x3d, 0xe3, 0x71, 0xc4);
	mi.position = 1000090000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	mi.name.a = LPGEN("View &history");
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	hContactMenu = Menu_AddContactMenuItem(&mi);

	mi.position = 500060000;
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	Menu_AddMainMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
}

void InitTaskMenuItems()
{
	if (Options::instance->taskOptions.size() > 0) {
		if (hTaskMainMenu == nullptr) {
			CMenuItem mi(&g_plugin);
			SET_UID(mi, 0xbf66499, 0x1b39, 0x47a2, 0x9b, 0x74, 0xa6, 0xae, 0x89, 0x95, 0x59, 0x59);
			mi.position = 500060005;
			mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
			mi.name.a = LPGEN("Execute history task");
			hTaskMainMenu = Menu_AddMainMenuItem(&mi);
		}

		std::vector<TaskOptions>::iterator taskIt = Options::instance->taskOptions.begin();
		std::vector<HGENMENU>::iterator it = taskMenus.begin();
		for (; it != taskMenus.end() && taskIt != Options::instance->taskOptions.end(); ++it, ++taskIt)
			Menu_ModifyItem(*it, taskIt->taskName.c_str());

		for (; it != taskMenus.end(); ++it)
			Menu_ShowItem(*it, false);

		int pos = (int)taskMenus.size();
		for (; taskIt != Options::instance->taskOptions.end(); ++taskIt) {
			CMenuItem mi(&g_plugin);
			mi.flags = CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
			mi.pszService = MS_HISTORY_EXECUTE_TASK;
			mi.root = hTaskMainMenu;
			mi.name.w = (wchar_t*)taskIt->taskName.c_str();
			HGENMENU menu = Menu_AddMainMenuItem(&mi);
			Menu_ConfigureItem(menu, MCI_OPT_EXECPARAM, pos++);
			taskMenus.push_back(menu);
		}
	}
	else if (hTaskMainMenu != nullptr)
		Menu_ShowItem(hTaskMainMenu, false);
}

IconItem iconList[] =
{
	{ LPGEN("Incoming message"), "BasicHistory_in", IDI_INM },
	{ LPGEN("Outgoing message"), "BasicHistory_out", IDI_OUTM },
	{ LPGEN("Status change"),    "BasicHistory_status", IDI_STATUS },
	{ LPGEN("Show contacts"),    "BasicHistory_show", IDI_SHOW },
	{ LPGEN("Hide contacts"),    "BasicHistory_hide", IDI_HIDE },
	{ LPGEN("Find next"),        "BasicHistory_findnext", IDI_FINDNEXT },
	{ LPGEN("Find previous"),    "BasicHistory_findprev", IDI_FINDPREV },
	{ LPGEN("Plus in export"),   "BasicHistory_plusex", IDI_PLUSEX },
	{ LPGEN("Minus in export"),  "BasicHistory_minusex", IDI_MINUSEX },
};

void InitIcolib()
{
}

HICON LoadIconEx(int iconId, bool big)
{
	for (auto &it : iconList)
		if (it.defIconID == iconId)
			return IcoLib_GetIconByHandle(it.hIcolib, big);

	return nullptr;
}

INT_PTR ShowContactHistory(WPARAM hContact, LPARAM)
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

int ModulesLoaded(WPARAM, LPARAM)
{
	InitMenuItems();

	wchar_t ftpExe[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, ftpExe))) {
		wcscat_s(ftpExe, L"\\WinSCP\\WinSCP.exe");
		DWORD atr = GetFileAttributes(ftpExe);
		if (atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY) {
			#ifdef _WIN64
			if (SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_PROGRAM_FILESX86, nullptr, SHGFP_TYPE_CURRENT, ftpExe))) {
				wcscat_s(ftpExe, L"\\WinSCP\\WinSCP.exe");
				atr = GetFileAttributes(ftpExe);
				if (!(atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY))
					Options::instance->ftpExePathDef = ftpExe;
			}
			#endif
		}
		else Options::instance->ftpExePathDef = ftpExe;
	}

	wchar_t *log = L"%miranda_logpath%\\BasicHistory\\ftplog.txt";
	wchar_t *logAbsolute = Utils_ReplaceVarsW(log);
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

int CMPlugin::Load()
{
	hTaskMainMenu = nullptr;

	hCurSplitNS = LoadCursor(nullptr, IDC_SIZENS);
	hCurSplitWE = LoadCursor(nullptr, IDC_SIZEWE);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowContactHistory);
	CreateServiceFunction(MS_HISTORY_EXECUTE_TASK, ExecuteTaskService);

	Options::instance = new Options();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, Options::InitOptions);

	HistoryEventList::Init();

	g_plugin.registerIcon(LPGEN("History"), iconList);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	HistoryWindow::Deinit();

	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurSplitWE);

	if (Options::instance != nullptr) {
		delete Options::instance;
		Options::instance = nullptr;
	}

	delete[] hEventIcons;
	return 0;
}
