/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <newpluginapi.h>
#include <m_clist.h>
#include <m_skin.h>
#include <m_langpack.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_database.h>

#include "version.h"

#define MIID_REMOVEPERSONALSETTINGS { 0x5eaec989, 0x8ff, 0x4820, { 0xb8, 0x6c, 0x2b, 0x6e, 0xf0, 0x8e, 0x33, 0x73 } }

#define INI_FILE_NAME  "RemovePersonalSettings.ini"

#define PLUGINDISABLELIST "PluginDisable"

#define METACONTACTS_PROTOCOL_NAME "MetaContacts"

#define NOTICE_TEXT LPGEN("All your personal settings will be erased!\n\
Make sure you are running this from a copy of your profile (and not over the original one).\n\
Running this will erase files/folders under Miranda main folder.\n\n\
Are you sure you want to remove all your personal settings?\n\n\
(You cannot say that I don't told you about the risks :P)")

#define MSGBOX_TITLE LPGEN("Remove Personal Settings")

HINSTANCE hInst;
char gIniFile[MAX_PATH];
char gMirandaDir[MAX_PATH];

int hLangpack;


PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {60E94B84-A799-4021-9449-5B838FC06A7C}
	{ 0x60e94b84, 0xa799, 0x4021, { 0x94, 0x49, 0x5b, 0x83, 0x8f, 0xc0, 0x6a, 0x7c } }
};


INT_PTR RemoveAllService(WPARAM wParam,LPARAM lParam);
void SetProtocolsOffline();
void RemoveUsers();
void RemoveSettings();
void ExecuteServices();
void RemoveDirectories();
void DisablePlugins();

// Ini access functions
BOOL GetSettingBool(const char *section, const char *key, BOOL defaultValue);
BOOL GetSettings(const char *section, char *buffer, size_t bufferSize);


// Utils
void DeleteFileOrFolder(const char *name);
void DeleteSetting(const char *setting);
void DeleteSettingEx(const char *szModule, const char *szSetting);
BOOL isMetaContact(MCONTACT hContact);



BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}


extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_REMOVEPERSONALSETTINGS, MIID_LAST };


extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);

	CLISTMENUITEM mi;
	char *strTmp;

	CreateServiceFunction("RemovePersonalSettings/RemoveAll",RemoveAllService);
	memset(&mi, 0, sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName = LPGEN("Remove Personal Settings...");
	mi.pszService="RemovePersonalSettings/RemoveAll";
	Menu_AddMainMenuItem(&mi);

	// Get ini file name
	gMirandaDir[0] = '\0';
	GetModuleFileNameA(GetModuleHandle(NULL),gMirandaDir,sizeof(gMirandaDir));

	// Remove last name
	strTmp = strrchr(gMirandaDir,'\\');
	if(strTmp != NULL)
		*strTmp = '\0';

	// Set vars
	strcat(gMirandaDir, "\\");
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


extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}


INT_PTR RemoveAllService(WPARAM wParam,LPARAM lParam)
{
	if (gIniFile[0] == '\0') {
		MessageBox(NULL, TranslateT("Configuration file could not be found!"), TranslateT(MSGBOX_TITLE), MB_OK | MB_ICONERROR);
		return -1;
	}

	if (MessageBox(NULL, TranslateT(NOTICE_TEXT), TranslateT(MSGBOX_TITLE), MB_YESNO) == IDYES) {
		SetProtocolsOffline();
		RemoveUsers();
		RemoveSettings();
		ExecuteServices();
		RemoveDirectories();
		DisablePlugins();

		MessageBox(NULL, TranslateT("Settings are deleted now."), TranslateT(MSGBOX_TITLE), MB_OK | MB_ICONINFORMATION);
	}

	return 0;
}

void SetProtocolsOffline()
{
	if ( GetSettingBool("GlobalSettings", "SetProtocolsOffline", TRUE) ) {
		PROTOACCOUNT **accounts;
		int i,count;

		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&accounts);

		for (i = 0; i < count; i++) {
			/*if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;*/
			if (!accounts[i]->bIsEnabled)
				continue;

			if (accounts[i]->szModuleName == NULL || accounts[i]->szModuleName[0] == '\0')
				continue;

			CallProtoService(accounts[i]->szModuleName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
		}

		// Give some time to make it really offline
		Sleep(2000);
	}
}

void RemoveUsers()
{
	if ( GetSettingBool("GlobalSettings", "RemoveAllUsers", TRUE) ) {
		// To be faster, remove first all metacontacts (because it syncs histories)
		MCONTACT hContact = db_find_first();
		while(hContact != NULL) {
			MCONTACT hContactOld = hContact;
			hContact = db_find_next(hContact);

			if (isMetaContact(hContactOld) )
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContactOld, 0);
		}

		// Now delete all left-overs
		hContact = db_find_first();
		while(hContact != NULL) {
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);

			hContact = db_find_first();
		}

		// Delete events for contacts not in list
		MEVENT hDbEvent = db_event_first(0);

		while(hDbEvent != NULL) {
			int ret = db_event_delete(0, hDbEvent);
			hDbEvent = db_event_first(0);
		}

		// Now delete groups
		DeleteSettingEx("CListGroups", NULL);
	}
}

void RemoveProtocolSettings(const char * protocolName)
{
	char buffer[10000];

	// Remove protocol module settings
	if ( GetSettingBool("GlobalSettings", "RemoveWholeProtocolModule", FALSE) ) {
		DeleteSettingEx(protocolName, NULL);
	}
	else if ( GetSettings("ProtocolSettings", buffer, sizeof(buffer)) ) {
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				DeleteSettingEx(protocolName, name);

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}

	// Remove modules by protocol sufixes
	if ( GetSettings("ProtocolModuleSufixes", buffer, sizeof(buffer)) ) {
		char *name;
		char *value;
		char moduleName[256];

		name = buffer;
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
			}

			// Delete it
			if (name[0] != '\0') {
				mir_snprintf(moduleName, SIZEOF(moduleName), "%s%s", protocolName, name);
				DeleteSettingEx(moduleName, NULL);
			}

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}

void RemoveSettings()
{
	char buffer[10000];

	// Delete protocol settings
	if ( GetSettingBool("GlobalSettings", "RemoveProtocolSettings", TRUE) ) {
		PROTOACCOUNT **accounts;
		int i,count;

		// TODO MS_PROTO_ENUMACCOUNTS
		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&accounts);

		for (i = 0; i < count; i++) {
			/*if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;*/
			if (!accounts[i]->bIsEnabled)
				continue;

			if (accounts[i]->szModuleName == NULL || accounts[i]->szModuleName[0] == '\0')
				continue;

			RemoveProtocolSettings(accounts[i]->szModuleName);
		}

		// Get disabled protocols
		if ( GetSettings("DisabledProtocols", buffer, sizeof(buffer)) ) {
			char *name;
			char *value;

			name = buffer;
			while(name[0] != '\0') {
				value = strchr(name, '=');
				if (value == NULL)
					value = &name[mir_strlen(name)];

				// Has " ?
				if (*name == '"' && *(value-1) == '"') {
					name++;
					*(value-1) = '\0';
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
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
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

	if ( GetSettings("ExecuteServices", buffer, sizeof(buffer)) ) {
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0')
				if (ServiceExists(name))
					CallService(name,0,0);

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
		PROTOACCOUNT **accounts;
		int i,count;

		CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&count, (LPARAM)&accounts);

		for (i = 0; i < count; i++) {
			/*if (protos[i]->type != PROTOTYPE_PROTOCOL)
				continue;*/
			if (!accounts[i]->bIsEnabled)
				continue;

			if (accounts[i]->szModuleName == NULL || accounts[i]->szModuleName[0] == '\0')
				continue;

			mir_snprintf(dir, SIZEOF(dir), "%s%s", gMirandaDir, accounts[i]->szModuleName);
			DeleteFileOrFolder(dir);
		}
	}

	// Remove other folders
	if ( GetSettings("RemoveFilesOrFolders", buffer, sizeof(buffer)) ) {
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
			}

			// Delete it
			if (name[0] != '\0') {
				mir_snprintf(dir, SIZEOF(dir), "%s%s", gMirandaDir, name);
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

	if ( GetSettings("DisablePlugins", buffer, sizeof(buffer)) ) {
		char *name;
		char *value;

		name = buffer;
		while(name[0] != '\0') {
			value = strchr(name, '=');
			if (value == NULL)
				value = &name[mir_strlen(name)];

			// Has " ?
			if (*name == '"' && *(value-1) == '"') {
				name++;
				*(value-1) = '\0';
			}

			// Disable it
			if (name[0] != '\0') {
				CharLowerA(name);
				if (db_get_b(NULL, PLUGINDISABLELIST, name, 0) != 1)
				{
					db_set_b(NULL, PLUGINDISABLELIST, name, 1);
				}
			}

			// Get next one
			name = value + mir_strlen(value) + 1;
		}
	}
}



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
	return (BOOL)GetPrivateProfileSectionA(section, buffer, (DWORD)bufferSize, gIniFile) != 0;
}



// Utils

void DeleteFileOrFolder(const char *name)
{
	DWORD attibs = GetFileAttributesA(name);

	if (attibs == INVALID_FILE_ATTRIBUTES) { // Not exists
								// Try to find it
		char tmp[MAX_PATH];
		char *strTmp;

		// Delete files
		WIN32_FIND_DATAA findData;
		HANDLE hwnd = FindFirstFileA(name, &findData);
		if (hwnd != INVALID_HANDLE_VALUE) {
			strncpy(tmp, name, sizeof(tmp)-1);
			strTmp = strrchr(tmp,'\\');

			if(strTmp != NULL) {
				strTmp++;
				*strTmp = '\0';
			}
			else {
				strcat(tmp, "\\");
				strTmp = &tmp[mir_strlen(tmp)];
			}

			do {
				if (mir_strcmp(findData.cFileName, ".") && mir_strcmp(findData.cFileName, "..")) {
					mir_strcpy(strTmp, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			}
			while(FindNextFileA(hwnd, &findData) != 0);

			FindClose(hwnd);
		}
	}
	else if (attibs & FILE_ATTRIBUTE_DIRECTORY)	{ // Is a directory
												  // Get all files and delete then
		char tmp[MAX_PATH];
		mir_snprintf(tmp, SIZEOF(tmp), "%s\\*.*", name);

		// Delete files
		WIN32_FIND_DATAA findData;
		HANDLE hwnd = FindFirstFileA(tmp, &findData);
		if (hwnd != INVALID_HANDLE_VALUE) {
			do {
				if (mir_strcmp(findData.cFileName, ".") && mir_strcmp(findData.cFileName, "..")) {
					mir_snprintf(tmp, SIZEOF(tmp), "%s\\%s", name, findData.cFileName);
					DeleteFileOrFolder(tmp);
				}
			}
			while(FindNextFileA(hwnd, &findData) != 0);

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


BOOL isMetaContact(MCONTACT hContact)
{
	return mir_strcmp(GetContactProto(hContact), METACONTACTS_PROTOCOL_NAME) == 0;
}



typedef struct {
	char buffer[10000];
	size_t pos;
	const char *filter;
	size_t lenFilterMinusOne;
} DeleteModuleStruct;


int EnumProc(const char *szName, LPARAM lParam)
{
	DeleteModuleStruct *dms = (DeleteModuleStruct *) lParam;
	size_t len = mir_strlen(szName);

	if (dms->filter != NULL && dms->lenFilterMinusOne > 0) {
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

int ModuleEnumProc(const char *szName, DWORD ofsModuleName, LPARAM lParam)
{
	return EnumProc(szName, lParam);
}

void DeleteSettingEx(const char *szModule, const char *szSetting)
{
	size_t lenModule;

	if (szModule == NULL)
		return;

	lenModule = mir_strlen(szModule);
	if (szModule[0] == '*' || szModule[lenModule-1] == '*') {
		DeleteModuleStruct dms;
		memset(&dms, 0, sizeof(dms));

		dms.filter = szModule;
		dms.lenFilterMinusOne = lenModule-1;

		CallService(MS_DB_MODULES_ENUM, (WPARAM) &dms, (LPARAM) &ModuleEnumProc);

		// Delete then
		szModule = dms.buffer;
		while(szModule[0] != '\0') {
			DeleteSettingEx(szModule, szSetting);

			// Get next one
			szModule += mir_strlen(szModule) + 1;
		}
	}
	else {
		size_t lenSetting = szSetting == NULL ? 0 : mir_strlen(szSetting);
		if (szSetting == NULL || szSetting[0] == '*' || szSetting[lenSetting-1] == '*') {
			DeleteModuleStruct dms;
			DBCONTACTENUMSETTINGS dbces;

			memset(&dms, 0, sizeof(dms));

			dms.filter = szSetting;
			dms.lenFilterMinusOne = lenSetting-1;

			dbces.pfnEnumProc = EnumProc;
			dbces.lParam = (LPARAM) &dms;
			dbces.szModule = szModule;
			dbces.ofsSettings = 0;

			CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM) &dbces);

			// Delete then
			szSetting = dms.buffer;
			while(szSetting[0] != '\0') {
				db_unset(NULL, szModule, szSetting);

				// Get next one
				szSetting += mir_strlen(szSetting) + 1;
			}
		}
		else {
			db_unset(NULL, szModule, szSetting);
		}
	}
}

void DeleteSetting(const char *setting)
{
	char *szModule;
	char *szSetting;

	if (setting == NULL || setting[0] == '\0') {
		return;
	}

	// Split setting
	szModule = strdup(setting);
	szSetting = strrchr(szModule, '/');
	if (szSetting != NULL) {
		*szSetting = '\0';
		szSetting ++;
	}

	DeleteSettingEx(szModule, szSetting);

	free(szModule);
}
