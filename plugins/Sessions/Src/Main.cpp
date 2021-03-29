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

HANDLE hmTBButton[2], hiTBbutton[2], iTBbutton[2];

int g_ses_limit;
int g_ses_count;
bool g_bExclHidden;
bool g_bWarnOnHidden;
bool g_bOtherWarnings;
bool g_bCrashRecovery;
bool g_bIncompletedSave;

HWND g_hDlg;
bool DONT = false;
bool isLastTRUE = false;

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
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK ExitDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		LoadPosition(hdlg, "ExitDlg");
		ShowWindow(hdlg, SW_SHOW);
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDOK:
			SavePosition(hdlg, "ExitDlg");
			SaveSessionDate();
			SaveSessionHandles(session_list, false);
			g_plugin.setByte("lastempty", 0);
			DestroyWindow(hdlg);
			break;

		case IDCANCEL:
			SavePosition(hdlg, "ExitDlg");
			g_plugin.setByte("lastempty", 1);
			DestroyWindow(hdlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hdlg);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

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

int DelUserDefSession(int ses_count)
{
	for (auto &hContact : Contacts()) {
		RemoveSessionMark(hContact, 1, ses_count);
		SetInSessionOrder(hContact, 1, ses_count, 0);
	}

	char szSessionName[256];
	mir_snprintf(szSessionName, "%s_%u", "UserSessionDsc", ses_count);
	g_plugin.delSetting(szSessionName);

	mir_snprintf(szSessionName, "%s_%u", "FavUserSession", ses_count);
	g_plugin.delSetting(szSessionName);

	for (int i = ses_count + 1;; i++) {
		mir_snprintf(szSessionName, "%s_%u", "UserSessionDsc", i);
		ptrW szSessionNameBuf(g_plugin.getWStringA(szSessionName));

		mir_snprintf(szSessionName, "%s_%u", "UserSessionDsc", i - 1);
		if (szSessionNameBuf) {
			MarkUserDefSession(i - 1, IsMarkedUserDefSession(i));
			g_plugin.setWString(szSessionName, szSessionNameBuf);
		}
		else {
			g_plugin.delSetting(szSessionName);

			mir_snprintf(szSessionName, "%s_%u", "FavUserSession", i - 1);
			g_plugin.delSetting(szSessionName);
			break;
		}
	}
	g_ses_count--;
	g_plugin.setByte("UserSessionsCount", (BYTE)g_ses_count);
	return 0;
}

int DeleteAutoSession(int ses_count)
{
	for (auto &hContact : Contacts()) {
		RemoveSessionMark(hContact, 0, ses_count);
		SetInSessionOrder(hContact, 0, ses_count, 0);
	}

	char szSessionName[256];
	mir_snprintf(szSessionName, "%s_%u", "SessionDate", ses_count);
	g_plugin.delSetting(szSessionName);

	for (int i = ses_count + 1;; i++) {
		mir_snprintf(szSessionName, "%s_%u", "SessionDate", i);
		ptrW szSessionNameBuf(g_plugin.getWStringA(szSessionName));

		mir_snprintf(szSessionName, "%s_%u", "SessionDate", i - 1);
		if (szSessionNameBuf)
			g_plugin.setWString(szSessionName, szSessionNameBuf);
		else {
			g_plugin.delSetting(szSessionName);
			break;
		}
	}

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

int OkToExit(WPARAM, LPARAM)
{
	int exitmode = g_plugin.getByte("ShutdownMode", 2);
	DONT = 1;
	if (exitmode == 2 && session_list[0] != 0) {
		SaveSessionDate();
		SaveSessionHandles(session_list, false);
		g_plugin.setByte("lastempty", 0);
	}
	else if (exitmode == 1 && session_list[0] != 0) {
		DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXDIALOG), nullptr, ExitDlgProc);
	}
	else g_plugin.setByte("lastempty", 1);
	return 0;
}

static int OnSrmmWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;
	if (MWeventdata->uType == MSG_WINDOW_EVT_OPEN) {
		AddToCurSession(MWeventdata->hContact, 0);
		if (g_bCrashRecovery)
			g_plugin.setByte(MWeventdata->hContact, "wasInLastSession", 1);
	}
	else if (MWeventdata->uType == MSG_WINDOW_EVT_CLOSE) {
		if (!DONT)
			DelFromCurSession(MWeventdata->hContact, 0);
		if (g_bCrashRecovery)
			g_plugin.setByte(MWeventdata->hContact, "wasInLastSession", 0);
	}

	return 0;
}

INT_PTR BuildFavMenu(WPARAM, LPARAM)
{
	POINT pt;
	GetCursorPos(&pt);

	HMENU hMenu = CreatePopupMenu();
	FillFavoritesMenu(hMenu, g_ses_count);
	int res = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, GetActiveWindow(), nullptr);
	if (res == 0) return 1;
	LoadSession(0, (res - 1) + g_ses_limit);
	return 0;
}

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

static int PluginInit(WPARAM, LPARAM)
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

int CMPlugin::Load()
{
	CreateServiceFunction(MS_SESSIONS_SHOWFAVORITESMENU, BuildFavMenu);
	CreateServiceFunction(MS_SESSIONS_OPENMANAGER, OpenSessionsManagerWindow);
	CreateServiceFunction(MS_SESSIONS_RESTORELASTSESSION, LoadLastSession);
	CreateServiceFunction(MS_SESSIONS_SAVEUSERSESSION, SaveUserSessionHandles);
	CreateServiceFunction(MS_SESSIONS_CLOSESESSION, CloseCurrentSession);

	Miranda_WaitOnHandle(LaunchSessions);

	g_ses_count = g_plugin.getByte("UserSessionsCount", 0);
	g_ses_limit = g_plugin.getByte("TrackCount", 10);
	g_bExclHidden = g_plugin.getByte("ExclHidden", 0) != 0;
	g_bWarnOnHidden = g_plugin.getByte("WarnOnHidden", 0) != 0;
	g_bOtherWarnings = g_plugin.getByte("OtherWarnings", 1) != 0;
	g_bCrashRecovery = g_plugin.getByte("CrashRecovery", 0) != 0;

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
		isLastTRUE = true;

	HookEvent(ME_SYSTEM_MODULESLOADED, PluginInit);
	HookEvent(ME_SYSTEM_OKTOEXIT, OkToExit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SessionPreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptionsInit);

	// Icons
	g_plugin.registerIcon(MODULENAME, iconList);
	return 0;
}
