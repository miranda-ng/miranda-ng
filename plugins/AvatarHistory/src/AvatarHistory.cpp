/*
Avatar History Plugin
---------

 This plugin uses the event provided by Avatar Service to 
 automatically back up contacts' avatars when they change.
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#include "stdafx.h"

uint32_t mirVer;

HANDLE hFolder = nullptr;

wchar_t profilePath[MAX_PATH];		// database profile path (read at startup only)
wchar_t basedir[MAX_PATH];
CMPlugin g_plugin;
MWindowList hAvatarWindowsList = nullptr;

int OptInit(WPARAM wParam, LPARAM lParam);

wchar_t* GetHistoryFolder(wchar_t *fn);
wchar_t* GetProtocolFolder(wchar_t *fn, char *proto);
wchar_t* GetOldStyleAvatarName(wchar_t *fn, MCONTACT hContact);

void InitMenuItem();

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
	// {DBE8C990-7AA0-458D-BAB7-33EB07238E71}
	{0xdbe8c990, 0x7aa0, 0x458d, {0xba, 0xb7, 0x33, 0xeb, 0x7, 0x23, 0x8e, 0x71}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// services

static INT_PTR GetCachedAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t hash[128];

	wcsncpy_s(hash, (wchar_t*)lParam, _TRUNCATE);
	ConvertToFilename(hash, _countof(hash));
	return (INT_PTR)GetCachedAvatar((char*)wParam, hash);
}

static INT_PTR IsEnabled(WPARAM wParam, LPARAM)
{
	MCONTACT hContact = (MCONTACT)wParam;
	return ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK)
		|| ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS)
		|| ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY);
}

/////////////////////////////////////////////////////////////////////////////////////////
// events

// fired when the contacts avatar changes
// wParam = hContact
// lParam = struct avatarCacheEntry *cacheEntry
// the event CAN pass a NULL pointer in lParam which means that the avatar has changed,
// but is no longer valid (happens, when a contact removes his avatar, for example).
// DONT DESTROY the bitmap handle passed in the struct avatarCacheEntry *
// 
// It is also possible that this event passes 0 as wParam (hContact), in which case,
// a protocol picture (pseudo - avatar) has been changed. 

static int AvatarChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	char *proto = Proto_GetBaseAccountName(hContact);
	if (proto == nullptr)
		return 0;

	if (mir_strcmp(META_PROTO, proto) == 0)
		return 0;

	DBVARIANT dbvOldHash = { 0 };
	bool ret = (g_plugin.getWString(hContact, "AvatarHash", &dbvOldHash) == 0);

	CONTACTAVATARCHANGEDNOTIFICATION* avatar = (CONTACTAVATARCHANGEDNOTIFICATION*)lParam;
	if (avatar == nullptr) {
		if (!ret || !mir_wstrcmp(dbvOldHash.pwszVal, L"-")) {
			//avoid duplicate "removed avatar" notifications
			//do not notify on an empty profile
			ShowDebugPopup(hContact, L"AVH Debug", L"Removed avatar, no avatar before... skipping");
			db_free(&dbvOldHash);
			return 0;
		}
		Skin_PlaySound("avatar_removed");

		// Is a flash avatar or avs could not load it
		g_plugin.setWString(hContact, "AvatarHash", L"-");

		if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS) && opts.popup_show_removed)
			ShowPopup(hContact, nullptr, opts.popup_removed);
	}
	else {
		if (ret && !mir_wstrcmp(dbvOldHash.pwszVal, avatar->hash)) {
			// same avatar hash, skipping
			ShowDebugPopup(hContact, L"AVH Debug", L"Hashes are the same... skipping");
			db_free(&dbvOldHash);
			return 0;
		}
		Skin_PlaySound("avatar_changed");
		g_plugin.setWString(hContact, "AvatarHash", avatar->hash);

		wchar_t history_filename[MAX_PATH] = L"";

		if (ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK)) {
			if (!opts.log_store_as_hash) {
				if (opts.log_per_contact_folders) {
					GetOldStyleAvatarName(history_filename, hContact);
					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar history: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, L"AVH Debug: File copied successfully", history_filename);

					MCONTACT hMetaContact = db_mc_getMeta(hContact);
					if (hMetaContact && ContactEnabled(hMetaContact, "LogToDisk", AVH_DEF_LOGTOHISTORY)) {
						wchar_t filename[MAX_PATH] = L"";

						GetOldStyleAvatarName(filename, hMetaContact);
						if (CopyImageFile(avatar->filename, filename))
							ShowPopup(hContact, TranslateT("Avatar history: Unable to save avatar"), filename);
						else
							ShowDebugPopup(hContact, L"AVH Debug: File copied successfully", filename);
					}
				}
			}
			else {
				// See if we already have the avatar
				wchar_t hash[128];

				wcsncpy_s(hash, avatar->hash, _TRUNCATE);
				ConvertToFilename(hash, _countof(hash));

				wchar_t *file = GetCachedAvatar(proto, hash);

				if (file != nullptr) {
					mir_wstrncpy(history_filename, file, _countof(history_filename));
					mir_free(file);
				}
				else {
					if (opts.log_keep_same_folder)
						GetHistoryFolder(history_filename);
					else
						GetProtocolFolder(history_filename, proto);

					mir_snwprintf(history_filename,
						L"%s\\%s", history_filename, hash);

					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar history: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, L"AVH Debug: File copied successfully", history_filename);
				}

				if (opts.log_per_contact_folders) {
					CreateOldStyleShortcut(hContact, history_filename);

					MCONTACT hMetaContact = db_mc_getMeta(hContact);
					if (hMetaContact && ContactEnabled(hMetaContact, "LogToDisk", AVH_DEF_LOGTOHISTORY))
						CreateOldStyleShortcut(hMetaContact, history_filename);
				}
			}
		}

		if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS) && opts.popup_show_changed)
			ShowPopup(hContact, nullptr, opts.popup_changed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY)) {
			wchar_t rel_path[MAX_PATH];
			PathToRelativeW(history_filename, rel_path);
			T2Utf blob(rel_path);

			DBEVENTINFO dbei = {};
			dbei.szModule = Proto_GetBaseAccountName(hContact);
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.timestamp = (uint32_t)time(0);
			dbei.eventType = EVENTTYPE_AVATAR_CHANGE;
			dbei.cbBlob = (uint32_t)mir_strlen(blob) + 1;
			dbei.pBlob = blob;
			db_event_add(hContact, &dbei);
		}
	}

	return 0;
}

static int PreShutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(hAvatarWindowsList, WM_CLOSE, 0, 0);
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	mir_snwprintf(basedir, L"%s\\Avatars History", profilePath);

	hFolder = FoldersRegisterCustomPathW(LPGEN("Avatars"), LPGEN("Avatar History"),
		PROFILE_PATHW L"\\" CURRENT_PROFILEW L"\\Avatars History");

	InitPopups();

	HookEvent(ME_AV_CONTACTAVATARCHANGED, AvatarChanged);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK FirstRunDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)createDefaultOverlayedIcon(TRUE));
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)createDefaultOverlayedIcon(FALSE));
		TranslateDialogDefault(hwnd);

		CheckDlgButton(hwnd, IDC_MIR_PROTO, BST_CHECKED);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			int ret = 0;

			if (IsDlgButtonChecked(hwnd, IDC_MIR_SAME))
				ret = IDC_MIR_SAME;
			else if (IsDlgButtonChecked(hwnd, IDC_MIR_PROTO))
				ret = IDC_MIR_PROTO;
			else if (IsDlgButtonChecked(hwnd, IDC_MIR_SHORT))
				ret = IDC_MIR_SHORT;
			else if (IsDlgButtonChecked(hwnd, IDC_SHORT))
				ret = IDC_SHORT;
			else if (IsDlgButtonChecked(hwnd, IDC_DUP))
				ret = IDC_DUP;

			EndDialog(hwnd, ret);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	}

	return FALSE;
}

int CMPlugin::Load()
{
	CoInitialize(nullptr);

	// Is first run?
	if (g_plugin.getByte("FirstRun", 1)) {
		// Show dialog
		int ret = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FIRST_RUN), nullptr, FirstRunDlgProc, 0);
		if (ret == 0)
			return -1;

		// Write settings

		g_plugin.setByte("LogToDisk", 1);

		if (ret == IDC_MIR_SAME)
			g_plugin.setByte("LogKeepSameFolder", 1);
		else
			g_plugin.setByte("LogKeepSameFolder", 0);

		if (ret == IDC_MIR_SHORT || ret == IDC_SHORT || ret == IDC_DUP)
			g_plugin.setByte("LogPerContactFolders", 1);
		else
			g_plugin.setByte("LogPerContactFolders", 0);

		if (ret == IDC_DUP)
			g_plugin.setByte("StoreAsHash", 0);
		else
			g_plugin.setByte("StoreAsHash", 1);

		if (ret == IDC_MIR_SAME || ret == IDC_MIR_PROTO || ret == IDC_MIR_SHORT)
			g_plugin.setByte("LogToHistory", 1);
		else
			g_plugin.setByte("LogToHistory", 0);

		g_plugin.setByte("FirstRun", 0);
	}

	LoadOptions();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SKIN_ICONSCHANGED, IcoLibIconsChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	CreateServiceFunction(MS_AVATARHISTORY_ENABLED, IsEnabled);
	CreateServiceFunction(MS_AVATARHISTORY_GET_CACHED_AVATAR, GetCachedAvatar);

	Profile_GetPathW(MAX_PATH, profilePath);

	g_plugin.addSound("avatar_changed", LPGENW("Avatar history"), LPGENW("Contact changed avatar"));
	g_plugin.addSound("avatar_removed", LPGENW("Avatar history"), LPGENW("Contact removed avatar"));

	hAvatarWindowsList = WindowList_Create();

	SetupIcoLib();
	InitMenuItem();
	return 0;
}

int CMPlugin::Unload()
{
	WindowList_Destroy(hAvatarWindowsList);
	return 0;
}
