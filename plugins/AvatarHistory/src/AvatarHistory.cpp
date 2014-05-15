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
#include "AvatarHistory.h"

HINSTANCE hInst;

DWORD mirVer;

HANDLE hFolder = NULL;

TCHAR profilePath[MAX_PATH];		// database profile path (read at startup only)
TCHAR basedir[MAX_PATH];
int hLangpack = 0;
HANDLE hAvatarWindowsList = NULL;

int OptInit(WPARAM wParam,LPARAM lParam);

TCHAR* GetHistoryFolder(TCHAR *fn);
TCHAR* GetProtocolFolder(TCHAR *fn, char *proto);
TCHAR* GetOldStyleAvatarName(TCHAR *fn, MCONTACT hContact);

void InitMenuItem();

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
	// {DBE8C990-7AA0-458D-BAB7-33EB07238E71}
	{0xdbe8c990, 0x7aa0, 0x458d, {0xba, 0xb7, 0x33, 0xeb, 0x7, 0x23, 0x8e, 0x71}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// services

static INT_PTR GetCachedAvatar(WPARAM wParam, LPARAM lParam)
{
	TCHAR hash[128];
	lstrcpyn(hash, (TCHAR *) lParam, sizeof(hash));
	ConvertToFilename(hash, sizeof(hash));
	return (INT_PTR) GetCachedAvatar((char *) wParam, hash);
}

static INT_PTR IsEnabled(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT) wParam;
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

	char *proto = GetContactProto(hContact);
	if (proto == NULL)
		return 0;

	if (strcmp(META_PROTO, proto) == 0)
		return 0;

	DBVARIANT dbvOldHash = {0};
	bool ret = (db_get_ts(hContact,MODULE_NAME,"AvatarHash",&dbvOldHash) == 0);

	CONTACTAVATARCHANGEDNOTIFICATION* avatar = (CONTACTAVATARCHANGEDNOTIFICATION*)lParam;
	if (avatar == NULL) {
		if (!ret || !_tcscmp(dbvOldHash.ptszVal, _T("-"))) {
			//avoid duplicate "removed avatar" notifications
			//do not notify on an empty profile
			ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Removed avatar, no avatar before... skipping"));
			db_free(&dbvOldHash);
			return 0;
		}
		SkinPlaySound("avatar_removed");

		// Is a flash avatar or avs could not load it
		db_set_ts(hContact, MODULE_NAME, "AvatarHash", _T("-"));

		if (ContactEnabled(hContact, "AvatarPopups", AVH_DEF_AVPOPUPS) && opts.popup_show_removed)
			ShowPopup(hContact, NULL, opts.popup_removed);
	}
	else {
		if (ret && !_tcscmp(dbvOldHash.ptszVal, avatar->hash)) {
			// same avatar hash, skipping
			ShowDebugPopup(hContact, TranslateT("AVH Debug"), TranslateT("Hashes are the same... skipping"));
			db_free(&dbvOldHash);
			return 0;
		}
		SkinPlaySound("avatar_changed");
		db_set_ts(hContact, "AvatarHistory", "AvatarHash", avatar->hash);

		TCHAR history_filename[MAX_PATH] = _T("");

		if (ContactEnabled(hContact, "LogToDisk", AVH_DEF_LOGTODISK)) {
			if (!opts.log_store_as_hash) {
				if (opts.log_per_contact_folders) {
					GetOldStyleAvatarName(history_filename, hContact);
					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), history_filename);

					MCONTACT hMetaContact = db_mc_getMeta(hContact);
					if (hMetaContact && ContactEnabled(hMetaContact, "LogToDisk", AVH_DEF_LOGTOHISTORY)) {
						TCHAR filename[MAX_PATH] = _T("");

						GetOldStyleAvatarName(filename, hMetaContact);
						if (CopyImageFile(avatar->filename, filename))
							ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), filename);
						else
							ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), filename);
					}
				}
			}
			else {
				// See if we already have the avatar
				TCHAR hash[128];
				lstrcpyn(hash, avatar->hash, sizeof(hash));
				ConvertToFilename(hash, sizeof(hash));

				TCHAR *file = GetCachedAvatar(proto, hash);

				if (file != NULL) {
					lstrcpyn(history_filename, file, SIZEOF(history_filename));
					mir_free(file);
				}
				else {
					if (opts.log_keep_same_folder)
						GetHistoryFolder(history_filename);
					else
						GetProtocolFolder(history_filename, proto);

					mir_sntprintf(history_filename, SIZEOF(history_filename), 
							_T("%s\\%s"), history_filename, hash);

					if (CopyImageFile(avatar->filename, history_filename))
						ShowPopup(hContact, TranslateT("Avatar History: Unable to save avatar"), history_filename);
					else
						ShowDebugPopup(hContact, TranslateT("AVH Debug: File copied successfully"), history_filename);
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
			ShowPopup(hContact, NULL, opts.popup_changed);

		if (ContactEnabled(hContact, "LogToHistory", AVH_DEF_LOGTOHISTORY)) {
			TCHAR rel_path[MAX_PATH];
			PathToRelativeT(history_filename, rel_path);
			ptrA blob( mir_utf8encodeT(rel_path));

			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.szModule = GetContactProto(hContact);
			dbei.flags = DBEF_READ | DBEF_UTF;
			dbei.timestamp = (DWORD) time(NULL);
			dbei.eventType = EVENTTYPE_AVATAR_CHANGE;
			dbei.cbBlob = (DWORD) strlen(blob) + 1;
			dbei.pBlob = (PBYTE)(char*)blob;
			db_event_add(hContact, &dbei);
		}
	}

	return 0;
}

static int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hAvatarWindowsList,WM_CLOSE,0,0);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	mir_sntprintf(basedir, SIZEOF(basedir), _T("%s\\Avatars History"), profilePath);

	hFolder = FoldersRegisterCustomPathT( LPGEN("Avatars"), LPGEN("Avatar History"),
		PROFILE_PATHT _T("\\") CURRENT_PROFILET _T("\\Avatars History"));

	InitPopups();

	HookEvent(ME_AV_CONTACTAVATARCHANGED, AvatarChanged);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK FirstRunDlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg) {
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)createDefaultOverlayedIcon(TRUE));
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)createDefaultOverlayedIcon(FALSE));
		TranslateDialogDefault(hwnd);

		CheckDlgButton(hwnd, IDC_MIR_PROTO, BST_CHECKED);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			{
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
		}
		break;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		return TRUE;
	}

	return FALSE;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	CoInitialize(NULL);

	// Is first run?
	if ( db_get_b(NULL, MODULE_NAME, "FirstRun", 1)) {
		// Show dialog
		int ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FIRST_RUN), NULL, FirstRunDlgProc, 0);
		if (ret == 0)
			return -1;

		// Write settings

		db_set_b(NULL, MODULE_NAME, "LogToDisk", 1);

		if (ret == IDC_MIR_SAME)
			db_set_b(NULL, MODULE_NAME, "LogKeepSameFolder", 1);
		else
			db_set_b(NULL, MODULE_NAME, "LogKeepSameFolder", 0);

		if (ret == IDC_MIR_SHORT || ret == IDC_SHORT || ret == IDC_DUP)
			db_set_b(NULL, MODULE_NAME, "LogPerContactFolders", 1);
		else
			db_set_b(NULL, MODULE_NAME, "LogPerContactFolders", 0);

		if (ret == IDC_DUP)
			db_set_b(NULL, MODULE_NAME, "StoreAsHash", 0);
		else
			db_set_b(NULL, MODULE_NAME, "StoreAsHash", 1);

		if (ret == IDC_MIR_SAME || ret == IDC_MIR_PROTO || ret == IDC_MIR_SHORT)
			db_set_b(NULL, MODULE_NAME, "LogToHistory", 1);
		else
			db_set_b(NULL, MODULE_NAME, "LogToHistory", 0);

		db_set_b(NULL, MODULE_NAME, "FirstRun", 0);
	}

	LoadOptions();

	HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SKIN2_ICONSCHANGED, IcoLibIconsChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);

	CreateServiceFunction(MS_AVATARHISTORY_ENABLED, IsEnabled);
	CreateServiceFunction(MS_AVATARHISTORY_GET_CACHED_AVATAR, GetCachedAvatar);

	if (CallService(MS_DB_GETPROFILEPATHT, MAX_PATH, (LPARAM)profilePath) != 0)
		_tcscpy(profilePath, _T(".")); // Failed, use current dir

	SkinAddNewSoundExT("avatar_changed",LPGENT("Avatar History"),LPGENT("Contact changed avatar"));
	SkinAddNewSoundExT("avatar_removed",LPGENT("Avatar History"),LPGENT("Contact removed avatar"));

	hAvatarWindowsList = WindowList_Create();

	SetupIcoLib();
	InitMenuItem();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hAvatarWindowsList);
	return 0;
}
