/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

HGENMENU hmSaveCurrentSession;

OBJLIST<CSession> g_arUserSessions(1, NumericKeySortT), g_arDateSessions(1, NumericKeySortT);

HANDLE hmTBButton[2], hiTBbutton[2], iTBbutton[2];

int g_lastUserId, g_lastDateId;

bool g_bExclHidden;
bool g_bWarnOnHidden;
bool g_bOtherWarnings;
bool g_bCrashRecovery;
bool g_bIncompletedSave;

HWND g_hDlg;
bool DONT = false;
bool g_bLastSessionPresent = false;

MCONTACT session_list[255] = { 0 };
MCONTACT session_list_recovered[255];

CMPlugin g_plugin;

int OptionsInit(WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {60558872-2AAB-45AA-888D-097691C9B683}
	{ 0x60558872, 0x2aab, 0x45aa, { 0x88, 0x8d, 0x9, 0x76, 0x91, 0xc9, 0xb6, 0x83 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx),
	g_lastUserId(MODULENAME, "LastUserId", 0),
	g_lastDateId(MODULENAME, "LastDateId", 0),
	bExclHidden(MODULENAME, "ExclHidden", false),
	bWarnOnHidden(MODULENAME, "WarnOnHidden", false),
	bOtherWarnings(MODULENAME, "OtherWarnings", true),
	bCrashRecovery(MODULENAME, "CrashRecovery", false),
	iTrackCount(MODULENAME, "TrackCount", 10),
	iStartupDelay(MODULENAME, "StartupModeDelay", 1500)
{}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CloseCurrentSession(WPARAM, LPARAM)
{
	while (session_list[0] != 0) {
		if (auto *pDlg = Srmm_FindDialog(session_list[0]))
			pDlg->CloseTab();
		SleepEx(50, TRUE);
	}

	memset(session_list, 0, sizeof(session_list));
	return 0;
}

int SessionPreShutdown(WPARAM, LPARAM)
{
	DONT = 1;

	if (g_hDlg)  DestroyWindow(g_hDlg);
	if (g_hSDlg) DestroyWindow(g_hSDlg);

	g_plugin.setByte("lastSaveCompleted", 1);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void SaveDateSession()
{
	if (session_list[0] != 0) {
		int TimeSize = GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, nullptr, nullptr, nullptr, 0);
		ptrW szTimeBuf((wchar_t*)mir_alloc((TimeSize + 1)*sizeof(wchar_t)));
		GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, nullptr, nullptr, szTimeBuf, TimeSize);

		int DateSize = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, nullptr, 0);
		ptrW szDateBuf((wchar_t*)mir_alloc((DateSize + 1)*sizeof(wchar_t)));
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, szDateBuf, DateSize);

		g_plugin.g_lastDateId = g_plugin.g_lastDateId+1;

		CSession data;
		data.wszName.Format(L"%s - %s", szTimeBuf.get(), szDateBuf.get());
		for (int i = 0; session_list[i]; i++)
			data.contacts.push_back(session_list[i]);

		char szSetting[256];
		mir_snprintf(szSetting, "%s_%d", "SessionDate", int(g_plugin.g_lastDateId));
		g_plugin.setUString(szSetting, data.toString().c_str());

		while (g_arDateSessions.getCount() >= g_plugin.iTrackCount) {
			mir_snprintf(szSetting, "%s_%d", "SessionDate", g_arDateSessions[0].id);
			g_plugin.delSetting(szSetting);
			g_arDateSessions.remove(int(0));
		}
	}

	if (g_bCrashRecovery)
		g_plugin.setByte("lastSaveCompleted", 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Exit dialog

class CExitDlg : public CDlgBase
{

public:
	CExitDlg() :
		CDlgBase(g_plugin, IDD_EXDIALOG)
	{}

	bool OnInitDialog() override
	{
		LoadPosition(m_hwnd, "ExitDlg");
		return true;
	}

	bool OnApply() override
	{
		SaveDateSession();
		return true;
	}

	void OnDestroy() override
	{
		SavePosition(m_hwnd, "ExitDlg");
		g_plugin.setByte("lastempty", 1);
		PostQuitMessage(0);
	}
};

int OkToExit(WPARAM, LPARAM)
{
	int exitmode = g_plugin.getByte("ShutdownMode", 2);
	DONT = 1;
	if (exitmode == 2 && session_list[0] != 0) {
		SaveDateSession();
		g_plugin.setByte("lastempty", 0);
	}
	else if (exitmode == 1 && session_list[0] != 0) {
		CExitDlg().DoModal();
	}
	else g_plugin.setByte("lastempty", 1);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void AddToCurSession(MCONTACT hContact)
{
	if (CheckForDuplicate(session_list, hContact) == -1) {
		for (int i = 0;; i++) {
			if (session_list[i] == 0) {
				session_list[i] = hContact;
				break;
			}
		}
	}
}

static void DelFromCurSession(MCONTACT hContact)
{
	for (int i = 0; session_list[i] != 0; i++) {
		if (session_list[i] == hContact) {
			while (session_list[i + 1] != 0) {
				session_list[i] = session_list[i + 1];
				i++;
			}
			session_list[i] = 0;
		}
	}
}

static int OnSrmmWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;
	if (MWeventdata->uType == MSG_WINDOW_EVT_OPEN) {
		AddToCurSession(MWeventdata->hContact);
		if (g_bCrashRecovery)
			g_plugin.setByte(MWeventdata->hContact, "wasInLastSession", 1);
	}
	else if (MWeventdata->uType == MSG_WINDOW_EVT_CLOSE) {
		if (!DONT)
			DelFromCurSession(MWeventdata->hContact);
		if (g_bCrashRecovery)
			g_plugin.setByte(MWeventdata->hContact, "wasInLastSession", 0);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR BuildFavMenu(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);

	HMENU hMenu = CreatePopupMenu();
	for (auto &it: g_arUserSessions)
		if (it->bIsFavorite)
			AppendMenu(hMenu, MF_STRING, it->id + 1, it->wszName);
	
	if (GetMenuItemCount(hMenu) == 0) {
		DestroyMenu(hMenu);
		return 2;
	}

	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, GetActiveWindow(), nullptr);
	if (res == 0)
		return 1;
	
	res--;
	if (auto *pSession = g_arUserSessions.find((CSession*)&res))
		LoadSession(pSession);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toolbar initialization

static int CreateButtons(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_VISIBLE;

	ttb.pszService = MS_SESSIONS_OPENMANAGER;
	ttb.pszTooltipUp = ttb.name = LPGEN("Open Sessions Manager");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDI_SESSIONS_LOAD);
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_SESSIONS_SAVEUSERSESSION;
	ttb.pszTooltipUp = ttb.name = LPGEN("Save Session");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDD_SESSIONS_SAVE);
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_SESSIONS_RESTORELASTSESSION;
	ttb.pszTooltipUp = ttb.name = LPGEN("Restore Last Session");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDD_SESSIONS_LOADLAST);
	g_plugin.addTTB(&ttb);

	ttb.pszService = MS_SESSIONS_SHOWFAVORITESMENU;
	ttb.pszTooltipUp = ttb.name = LPGEN("Show Favorite Sessions Menu");
	ttb.hIconHandleUp = g_plugin.getIconHandle(IDD_SESSION_CHECKED);
	g_plugin.addTTB(&ttb);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowEvent);
	HookEvent(ME_TTB_MODULELOADED, CreateButtons);

	// Hotkeys
	HOTKEYDESC hkd = {};
	hkd.pszName = "OpenSessionsManager";
	hkd.szSection.a = LPGEN("Sessions");
	hkd.szDescription.a = LPGEN("Open Sessions Manager");
	hkd.pszService = MS_SESSIONS_OPENMANAGER;
	g_plugin.addHotkey(&hkd);

	hkd.pszName = "RestoreLastSession";
	hkd.szDescription.a = LPGEN("Restore last Session");
	hkd.pszService = MS_SESSIONS_RESTORELASTSESSION;
	g_plugin.addHotkey(&hkd);

	hkd.pszName = "SaveSession";
	hkd.szDescription.a = LPGEN("Save Session");
	hkd.pszService = MS_SESSIONS_SAVEUSERSESSION;
	g_plugin.addHotkey(&hkd);

	hkd.pszName = "CloseSession";
	hkd.szDescription.a = LPGEN("Close Session");
	hkd.pszService = MS_SESSIONS_CLOSESESSION;
	g_plugin.addHotkey(&hkd);

	// Main menu
	CMenuItem mi(&g_plugin);
	mi.position = 1000000000;
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Sessions Manager"), 1000000000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "D77B9AB4-AF7E-43DB-A487-BD581704D635");

	SET_UID(mi, 0xd35302fa, 0x8326, 0x4323, 0xa3, 0xe5, 0xb4, 0x41, 0xff, 0xfb, 0xaa, 0x2d);
	mi.name.a = LPGEN("Save session...");
	mi.hIcolibItem = g_plugin.getIconHandle(IDD_SESSIONS_SAVE);
	mi.pszService = MS_SESSIONS_SAVEUSERSESSION;
	hmSaveCurrentSession = Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x8de4d8b1, 0x9a62, 0x4f4e, 0xb0, 0x3d, 0x99, 0x7, 0x80, 0xe8, 0x93, 0xc2);
	mi.name.a = LPGEN("Load session...");
	mi.pszService = MS_SESSIONS_OPENMANAGER;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SESSIONS_LOAD);
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x73ea91d6, 0xb7e5, 0x4f67, 0x96, 0x96, 0xa, 0x24, 0x21, 0x48, 0x6f, 0x15);
	mi.name.a = LPGEN("Close session");
	mi.pszService = MS_SESSIONS_CLOSESESSION;
	mi.hIcolibItem = nullptr;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xe2c4e4ba, 0x5d08, 0x441b, 0xb5, 0x93, 0xc4, 0xe7, 0x9a, 0xfb, 0xa4, 0x6c);
	mi.name.a = LPGEN("Load last session");
	mi.pszService = MS_SESSIONS_RESTORELASTSESSION;
	mi.hIcolibItem = g_plugin.getIconHandle(IDD_SESSIONS_LOADLAST);
	mi.position = 10100000;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Sessions"), "Sessions", IDD_SESSION_CHECKED },
	{ LPGEN("Favorite Session"), "SessionMarked", IDD_SESSION_CHECKED },
	{ LPGEN("Not favorite Session"), "SessionUnMarked", IDD_SESSION_UNCHECKED },
	{ LPGEN("Load Session"), "SessionsLoad", IDI_SESSIONS_LOAD },
	{ LPGEN("Save Session"), "SessionsSave", IDD_SESSIONS_SAVE },
	{ LPGEN("Load last Session"), "SessionsLoadLast", IDD_SESSIONS_LOADLAST }
};

int LoadSettings(const char *szSetting, void *)
{
	if (!memcmp(szSetting, "SessionUser_", 12)) {
		auto *pSession = new CSession();
		pSession->id = atoi(szSetting + 12);
		pSession->fromString(ptrA(db_get_utfa(0, MODULENAME, szSetting)));
		g_arUserSessions.insert(pSession);
	}
	else if (!memcmp(szSetting, "SessionDate_", 12)) {
		auto *pSession = new CSession();
		pSession->id = atoi(szSetting + 12);
		pSession->fromString(ptrA(db_get_utfa(0, MODULENAME, szSetting)));
		g_arDateSessions.insert(pSession);
	}
	
	return 0;
}

int CMPlugin::Load()
{
	// Icons
	g_plugin.registerIcon(MODULENAME, iconList);

	// Services
	CreateServiceFunction(MS_SESSIONS_SHOWFAVORITESMENU, BuildFavMenu);
	CreateServiceFunction(MS_SESSIONS_OPENMANAGER, OpenSessionsManagerWindow);
	CreateServiceFunction(MS_SESSIONS_RESTORELASTSESSION, LoadLastSession);
	CreateServiceFunction(MS_SESSIONS_SAVEUSERSESSION, SaveUserSessionHandles);
	CreateServiceFunction(MS_SESSIONS_CLOSESESSION, CloseCurrentSession);

	// Events
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SessionPreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);

	Miranda_WaitOnHandle(LaunchSessions);

	// Settimgs
	g_bExclHidden = g_plugin.getByte("ExclHidden", 0) != 0;
	g_bWarnOnHidden = g_plugin.getByte("WarnOnHidden", 0) != 0;
	g_bOtherWarnings = g_plugin.getByte("OtherWarnings", 1) != 0;
	g_bCrashRecovery = g_plugin.getByte("CrashRecovery", 0) != 0;

	// Crash recovery
	if (g_bCrashRecovery)
		g_bIncompletedSave = g_plugin.getByte("lastSaveCompleted", 0) == 0;

	if (g_bIncompletedSave) {
		int i = 0;
		memset(session_list_recovered, 0, sizeof(session_list_recovered));

		for (auto &hContact : Contacts())
			if (g_plugin.getByte(hContact, "wasInLastSession"))
				session_list_recovered[i++] = hContact;
	}

	if (!session_list_recovered[0])
		g_bIncompletedSave = false;

	g_plugin.setByte("lastSaveCompleted", 0);

	if (!g_plugin.getByte("lastempty", 1) || g_bIncompletedSave)
		g_bLastSessionPresent = true;

	// Check for old settings
	CheckImport();

	// Load settings data
	db_enum_settings(0, LoadSettings, MODULENAME);
	return 0;
}
