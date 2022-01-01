/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Created by Pescuma
Based on work by nullbie

*/

#include "stdafx.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
}
g_plugin;

char gIniFile[MAX_PATH];
char gMirandaDir[MAX_PATH];

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
	// {60E94B84-A799-4021-9449-5B838FC06A7C}
	{ 0x60e94b84, 0xa799, 0x4021, { 0x94, 0x49, 0x5b, 0x83, 0x8f, 0xc0, 0x6a, 0x7c } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_REMOVEPERSONALSETTINGS, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// Ini access functions

BOOL GetSettingBool(const char *section, const char *key, BOOL defaultValue)
{
	char tmp[16];
	if (GetPrivateProfileStringA(section, key, defaultValue ? "true" : "false", tmp, sizeof(tmp), gIniFile) == 0)
		return defaultValue;

	return mir_strcmpi(tmp, "true") == 0;
}

BOOL GetSettings(const char *section, char *buffer, size_t bufferSize)
{
	buffer[0] = '\0\0';
	return (BOOL)GetPrivateProfileSectionA(section, buffer, (uint32_t)bufferSize, gIniFile) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Utils

void DeleteFileOrFolder(const char *name)
{
	uint32_t attibs = GetFileAttributesA(name);

	if (attibs == INVALID_FILE_ATTRIBUTES) { // Not exists
														  // Try to find it
		char tmp[MAX_PATH];
		char *strTmp;

		// Delete files
		WIN32_FIND_DATAA findData;
		HANDLE hwnd = FindFirstFileA(name, &findData);
		if (hwnd != INVALID_HANDLE_VALUE) {
			strncpy(tmp, name, sizeof(tmp) - 1);
			strTmp = strrchr(tmp, '\\');

			if (strTmp != nullptr) {
				strTmp++;
				*strTmp = '\0';
			}
			else {
				mir_strcat(tmp, "\\");
				strTmp = &tmp[mir_strlen(tmp)];
			}

			do {
				if (mir_strcmp(findData.cFileName, ".") && mir_strcmp(findData.cFileName, "..")) {
					mir_strcpy(strTmp, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			} while (FindNextFileA(hwnd, &findData) != 0);

			FindClose(hwnd);
		}
	}
	else if (attibs & FILE_ATTRIBUTE_DIRECTORY) { // Is a directory
																 // Get all files and delete then
		char tmp[MAX_PATH];
		mir_snprintf(tmp, "%s\\*.*", name);

		// Delete files
		WIN32_FIND_DATAA findData;
		HANDLE hwnd = FindFirstFileA(tmp, &findData);
		if (hwnd != INVALID_HANDLE_VALUE) {
			do {
				if (mir_strcmp(findData.cFileName, ".") && mir_strcmp(findData.cFileName, "..")) {
					mir_snprintf(tmp, "%s\\%s", name, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			} while (FindNextFileA(hwnd, &findData) != 0);

			FindClose(hwnd);
		}

		// Delete directory
		RemoveDirectoryA(name);
	}
	else { // Is a File
		SetFileAttributesA(name, FILE_ATTRIBUTE_ARCHIVE);
		DeleteFileA(name);
	}
}

struct DeleteModuleStruct
{
	char buffer[10000];
	size_t pos;
	const char *filter;
	size_t lenFilterMinusOne;
};

int EnumProc(const char *szName, void *lParam)
{
	DeleteModuleStruct *dms = (DeleteModuleStruct *)lParam;
	size_t len = mir_strlen(szName);

	if (dms->filter != nullptr && dms->lenFilterMinusOne > 0) {
		if (len >= dms->lenFilterMinusOne) {
			if (dms->filter[0] == '*') {
				if (mir_strcmp(&dms->filter[1], &szName[len - dms->lenFilterMinusOne]) != 0)
					return 0;
			}
			else { // if (dms->filter[dms->lenFilterMinusOne] == '*')
				if (strncmp(dms->filter, szName, dms->lenFilterMinusOne) != 0)
					return 0;
			}
		}
	}

	// Add to the struct
	if (len > 0 && len < sizeof(dms->buffer) - dms->pos - 2) {
		mir_strcpy(&dms->buffer[dms->pos], szName);
		dms->pos += len + 1;
	}

	return 0;
}

int ModuleEnumProc(const char *szName, void *lParam)
{
	return EnumProc(szName, lParam);
}

void DeleteSettingEx(const char *szModule, const char *szSetting)
{
	size_t lenModule;

	if (szModule == nullptr)
		return;

	lenModule = mir_strlen(szModule);
	if (szModule[0] == '*' || szModule[lenModule - 1] == '*') {
		DeleteModuleStruct dms;
		memset(&dms, 0, sizeof(dms));

		dms.filter = szModule;
		dms.lenFilterMinusOne = lenModule - 1;

		db_enum_modules(ModuleEnumProc, &dms);

		// Delete then
		szModule = dms.buffer;
		while (szModule[0] != '\0') {
			DeleteSettingEx(szModule, szSetting);

			// Get next one
			szModule += mir_strlen(szModule) + 1;
		}
	}
	else {
		size_t lenSetting = szSetting == nullptr ? 0 : mir_strlen(szSetting);
		if (szSetting == nullptr || szSetting[0] == '*' || szSetting[lenSetting - 1] == '*') {
			DeleteModuleStruct dms;
			memset(&dms, 0, sizeof(dms));
			dms.filter = szSetting;
			dms.lenFilterMinusOne = lenSetting - 1;
			db_enum_settings(NULL, EnumProc, szModule, &dms);

			// Delete then
			szSetting = dms.buffer;
			while (szSetting[0] != '\0') {
				db_unset(0, szModule, szSetting);

				// Get next one
				szSetting += mir_strlen(szSetting) + 1;
			}
		}
		else {
			db_unset(0, szModule, szSetting);
		}
	}
}

void DeleteSetting(const char *setting)
{
	char *szModule;
	char *szSetting;

	if (setting == nullptr || setting[0] == '\0') {
		return;
	}

	// Split setting
	szModule = strdup(setting);
	szSetting = strrchr(szModule, '/');
	if (szSetting != nullptr) {
		*szSetting = '\0';
		szSetting++;
	}

	DeleteSettingEx(szModule, szSetting);

	free(szModule);
}

BOOL isMetaContact(MCONTACT hContact)
{
	return mir_strcmp(Proto_GetBaseAccountName(hContact), METACONTACTS_PROTOCOL_NAME) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SetProtocolsOffline()
{
	if (GetSettingBool("GlobalSettings", "SetProtocolsOffline", TRUE)) {
		for (auto &pa : Accounts()) {
			if (!pa->bIsEnabled)
				continue;

			if (pa->szModuleName == nullptr || pa->szModuleName[0] == '\0')
				continue;

			CallProtoService(pa->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
		}

		// Give some time to make it really offline
		Sleep(2000);
	}
}

void RemoveProtocolSettings(const char * protocolName)
{
	char buffer[10000];

	// Remove protocol module settings
	if (GetSettingBool("GlobalSettings", "RemoveWholeProtocolModule", FALSE)) {
		DeleteSettingEx(protocolName, nullptr);
	}
	else if (GetSettings("ProtocolSettings", buffer, sizeof(buffer))) {
		char *name;
		char *value;

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				DeleteSettingEx(protocolName, name);

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}

	// Remove modules by protocol sufixes
	if (GetSettings("ProtocolModuleSufixes", buffer, sizeof(buffer))) {
		char *name;
		char *value;
		char moduleName[256];

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Delete it
			if (name[0] != '\0') {
				mir_snprintf(moduleName, "%s%s", protocolName, name);
				DeleteSettingEx(moduleName, nullptr);
			}

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

void RemoveUsers()
{
	if (GetSettingBool("GlobalSettings", "RemoveAllUsers", TRUE)) {
		// To be faster, remove first all metacontacts (because it syncs histories)
		MCONTACT hContact = db_find_first();
		while (hContact != NULL) {
			MCONTACT hContactOld = hContact;
			hContact = db_find_next(hContact);

			if (isMetaContact(hContactOld))
				db_delete_contact(hContactOld);
		}

		// Now delete all left-overs
		hContact = db_find_first();
		while (hContact != NULL) {
			db_delete_contact(hContact);
			hContact = db_find_first();
		}

		// Delete events from system history
		DB::ECPTR pCursor(DB::Events(0));
		while (pCursor.FetchNext())
			pCursor.DeleteEvent();

		// Now delete groups
		DeleteSettingEx("CListGroups", nullptr);
	}
}

void RemoveSettings()
{
	char buffer[10000];

	// Delete protocol settings
	if (GetSettingBool("GlobalSettings", "RemoveProtocolSettings", TRUE)) {
		for (auto &pa : Accounts()) {
			if (!pa->bIsEnabled)
				continue;

			if (pa->szModuleName == nullptr || pa->szModuleName[0] == '\0')
				continue;

			RemoveProtocolSettings(pa->szModuleName);
		}

		// Get disabled protocols
		if (GetSettings("DisabledProtocols", buffer, sizeof(buffer))) {
			char *name;
			char *value;

			name = buffer;
			while (name[0] != '\0') {
				value = strchr(name, '=');
				if (value == nullptr)
					value = &name[mir_strlen(name)];

				// Has " ?
				if (*name == '"' && *(value - 1) == '"') {
					name++;
					*(value - 1) = '\0';
				}

				// Disable it
				if (name[0] != '\0')
					RemoveProtocolSettings(name);

				// Get next one
				name = value + mir_strlen(value) + 1;
			}
		}
	}


	// Delete other settings
	if (GetSettings("RemoveSettings", buffer, sizeof(buffer))) {
		char *name;
		char *value;

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Delete it
			if (name[0] != '\0')
				DeleteSetting(name);

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

void ExecuteServices()
{
	char buffer[10000];

	if (GetSettings("ExecuteServices", buffer, sizeof(buffer))) {
		char *name;
		char *value;

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				if (ServiceExists(name))
					CallService(name, 0, 0);

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

void RemoveDirectories()
{
	char buffer[10000];
	char dir[MAX_PATH];

	// Remove protocol folders
	if (GetSettingBool("GlobalSettings", "RemoveProtocolFolders", TRUE)) {
		for (auto &pa : Accounts()) {
			if (!pa->bIsEnabled)
				continue;

			if (pa->szModuleName == nullptr || pa->szModuleName[0] == '\0')
				continue;

			mir_snprintf(dir, "%s%s", gMirandaDir, pa->szModuleName);
			DeleteFileOrFolder(dir);
		}
	}

	// Remove other folders
	if (GetSettings("RemoveFilesOrFolders", buffer, sizeof(buffer))) {
		char *name;
		char *value;

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Delete it
			if (name[0] != '\0') {
				mir_snprintf(dir, "%s%s", gMirandaDir, name);
				DeleteFileOrFolder(dir);
			}

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

void DisablePlugins()
{
	char buffer[10000];

	if (GetSettings("DisablePlugins", buffer, sizeof(buffer))) {
		char *name;
		char *value;

		name = buffer;
		while (name[0] != '\0') {
			value = strchr(name, '=');
			if (value == nullptr)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value - 1) == '"') {
				name++;
				*(value - 1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				SetPluginOnWhiteList(name, false);

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR RemoveAllService(WPARAM, LPARAM)
{
	if (gIniFile[0] == '\0') {
		MessageBox(nullptr, TranslateT("Configuration file could not be found!"), TranslateW(MSGBOX_TITLE), MB_OK | MB_ICONERROR);
		return -1;
	}

	if (MessageBox(nullptr, TranslateW(NOTICE_TEXT), TranslateW(MSGBOX_TITLE), MB_YESNO) == IDYES) {
		SetProtocolsOffline();
		RemoveUsers();
		RemoveSettings();
		ExecuteServices();
		RemoveDirectories();
		DisablePlugins();

		MessageBox(nullptr, TranslateT("Settings are deleted now."), TranslateW(MSGBOX_TITLE), MB_OK | MB_ICONINFORMATION);
	}

	return 0;
}

int CMPlugin::Load()
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x2f9f21df, 0xf33c, 0x4640, 0xb9, 0x63, 0xd3, 0x26, 0x8a, 0xb8, 0xb1, 0xf0);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
	mi.name.a = LPGEN("Remove Personal Settings...");
	mi.pszService = "RemovePersonalSettings/RemoveAll";
	Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RemoveAllService);

	// Get ini file name
	gMirandaDir[0] = '\0';
	GetModuleFileNameA(GetModuleHandle(nullptr), gMirandaDir, sizeof(gMirandaDir));

	// Remove last name
	char *strTmp = strrchr(gMirandaDir, '\\');
	if (strTmp != nullptr)
		*strTmp = '\0';

	// Set vars
	mir_strcat(gMirandaDir, "\\");
	mir_strcpy(gIniFile, gMirandaDir);

	// Store last pos
	strTmp = &gIniFile[mir_strlen(gIniFile)];

	// Lets try fist name
	mir_strcpy(strTmp, INI_FILE_NAME);

	if (_access(gIniFile, 4) != 0) {
		// Not found, lets try the other aproach
		mir_strcpy(strTmp, "plugins\\" INI_FILE_NAME);

		if (_access(gIniFile, 4) != 0) {
			// Not found :(
			gIniFile[0] = '\0';
		}
	}

	return 0;
}
