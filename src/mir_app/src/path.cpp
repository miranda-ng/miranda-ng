/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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
*/

#include "stdafx.h"
#include "profilemanager.h"

#include "../../../plugins/ExternalAPI/m_folders.h"

extern wchar_t g_profileDir[MAX_PATH], g_shortProfileName[MAX_PATH];

static HANDLE hAvatarFolder;
static wchar_t tszAvatarRoot[MAX_PATH];

wchar_t* GetContactID(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (Contact::IsGroupChat(hContact, szProto))
		return db_get_wsa(hContact, szProto, "ChatRoomID");

	return Contact::GetInfo(CNF_UNIQUEID, hContact, szProto);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Variables parser

#define XSTR(target, s) _xstrselect(target, s, _A2W(s))

static __forceinline int    _xcscmp(const char *s1, const char *s2) { return strcmp(s1, s2); }
static __forceinline int    _xcsncmp(const char *s1, const char *s2, size_t n) { return strncmp(s1, s2, n); }
static __forceinline size_t _xcslen(const char *s1) { return strlen(s1); }
static __forceinline char*  _xcscpy(char *s1, const char *s2) { return strcpy(s1, s2); }
static __forceinline char*  _xcsncpy(char *s1, const char *s2, size_t n) { return strncpy(s1, s2, n); }
static __forceinline const char* _xstrselect(const char*, const char *s1, wchar_t*) { return s1; }
static __forceinline char*  _itox(char*, int a) { return itoa(a, (char *)mir_alloc(sizeof(char)*20), 10); }
static __forceinline char*  mir_a2x(const char *, const char *s) { return mir_strdup(s); }
static __forceinline char*  mir_w2x(const char *, const wchar_t *s) { return mir_u2a(s); }

static __forceinline char *GetContactNickX(const char*, MCONTACT hContact)
{
	return mir_u2a(Clist_GetContactDisplayName(hContact));
}

static __forceinline char* GetContactIDX(const char*, MCONTACT hContact)
{
	wchar_t *id = GetContactID(hContact);
	char* res = mir_u2a(id);
	mir_free(id);
	return res;
}

static __forceinline char* GetEnvironmentVariableX(const char *variable)
{
	char result[512];
	if (GetEnvironmentVariableA(variable, result, _countof(result)))
		return mir_strdup(result);
	return nullptr;
}

static __forceinline char* GetProfileDirX(const char*)
{
	return mir_u2a(g_profileDir);
}

static __forceinline char* SHGetSpecialFolderPathX(int iCSIDL, char*)
{
	char result[512];
	if (SHGetSpecialFolderPathA(nullptr, result, iCSIDL, FALSE))
		return mir_strdup(result);
	return nullptr;
}

static __forceinline char* GetModulePathX(const char*, HMODULE hModule)
{
	char result[MAX_PATH];
	GetModuleFileNameA(hModule, result, sizeof(result));
	char* str = strrchr(result, '\\');
	if (str) *str = 0;
	return mir_strdup(result);
}

static __forceinline char* GetUserNameX(const char*)
{
	char result[128];
	DWORD size = _countof(result);
	if (GetUserNameA(result, &size))
		return mir_strdup(result);
	return nullptr;
}

static __forceinline char* GetProfileNameX(const char*)
{
	return mir_u2a(g_shortProfileName);
}

static __forceinline char* GetPathVarX(const char*, int code)
{
	wchar_t szFullPath[MAX_PATH];

	switch(code) {
	case 1:
		if (hAvatarFolder != nullptr)
			wcsncpy_s(szFullPath, tszAvatarRoot, _TRUNCATE);
		else
			mir_snwprintf(szFullPath, L"%s\\%s\\AvatarCache", g_profileDir, g_shortProfileName);
		break;
	case 2:
		mir_snwprintf(szFullPath, L"%s\\%s\\Logs", g_profileDir, g_shortProfileName);
		break;
	case 3:
		mir_snwprintf(szFullPath, L"%s\\%s", g_profileDir, g_shortProfileName);
		break;
	}
	return makeFileName(szFullPath);
}

static __forceinline int _xcscmp(const wchar_t *s1, const wchar_t *s2) { return wcscmp(s1, s2); }
static __forceinline int _xcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n) { return wcsncmp(s1, s2, n); }
static __forceinline size_t _xcslen(const wchar_t *s1) { return wcslen(s1); }
static __forceinline wchar_t* _xcscpy(wchar_t *s1, const wchar_t *s2) { return wcscpy(s1, s2); }
static __forceinline wchar_t* _xcsncpy(wchar_t *s1, const wchar_t *s2, size_t n) { return wcsncpy(s1, s2, n); }
static __forceinline const wchar_t* _xstrselect(const wchar_t *, const char *, const wchar_t *s2) { return s2; }
static __forceinline wchar_t* _itox(wchar_t *, int a) { return _itow(a, (wchar_t *)mir_alloc(sizeof(wchar_t) * 20), 10); }
static __forceinline wchar_t* mir_a2x(const wchar_t *, const char *s) { return mir_a2u(s); }
static __forceinline wchar_t* mir_w2x(const wchar_t *, const wchar_t *s) { return mir_wstrdup(s); }

static __forceinline wchar_t* GetContactNickX(const wchar_t*, MCONTACT hContact)
{
	return mir_wstrdup(Clist_GetContactDisplayName(hContact));
}

static __forceinline wchar_t* GetContactIDX(const wchar_t*, MCONTACT hContact)
{
	return GetContactID(hContact);
}

static __forceinline wchar_t* GetEnvironmentVariableX(const wchar_t *variable)
{
	wchar_t result[512];
	if (GetEnvironmentVariable(variable, result, _countof(result)))
		return mir_wstrdup(result);
	return nullptr;
}

static __forceinline wchar_t* SHGetSpecialFolderPathX(int iCSIDL, wchar_t*)
{
	wchar_t result[512];
	if (SHGetSpecialFolderPath(nullptr, result, iCSIDL, FALSE))
		return mir_wstrdup(result);
	return nullptr;
}

static __forceinline wchar_t* GetProfileDirX(const wchar_t*)
{
	return mir_wstrdup(g_profileDir);
}

static __forceinline wchar_t* GetModulePathX(const wchar_t*, HMODULE hModule)
{
	wchar_t result[MAX_PATH];
	GetModuleFileName(hModule, result, _countof(result));
	wchar_t* str = wcsrchr(result, '\\');
	if (str) *str = 0;
	return mir_wstrdup(result);
}

static __forceinline wchar_t* GetUserNameX(const wchar_t*)
{
	wchar_t result[128];
	DWORD size = _countof(result);
	if (GetUserName(result, &size))
		return mir_wstrdup(result);
	return nullptr;
}

static __forceinline wchar_t* GetProfileNameX(const wchar_t*)
{
	return mir_wstrdup(g_shortProfileName);
}

static __forceinline wchar_t* GetPathVarX(const wchar_t*, int code)
{
	wchar_t szFullPath[MAX_PATH];

	switch(code) {
	case 1:
		if (hAvatarFolder != nullptr)
			wcsncpy_s(szFullPath, tszAvatarRoot, _TRUNCATE);
		else
			mir_snwprintf(szFullPath, L"%s\\%s\\AvatarCache", g_profileDir, g_shortProfileName);
		break;
	case 2:
		mir_snwprintf(szFullPath, L"%s\\%s\\Logs", g_profileDir, g_shortProfileName);
		break;
	case 3:
		mir_snwprintf(szFullPath, L"%s\\%s", g_profileDir, g_shortProfileName);
		break;
	}
	return mir_wstrdup(szFullPath);
}

template<typename XCHAR>
XCHAR *GetInternalVariable(const XCHAR *key, size_t keyLength, MCONTACT hContact)
{
	XCHAR *theValue = nullptr;
	XCHAR *theKey = (XCHAR *)_alloca(sizeof(XCHAR) * (keyLength + 1));
	_xcsncpy(theKey, key, keyLength);
	theKey[keyLength] = 0;

	if (hContact) {
		if (!_xcscmp(theKey, XSTR(key, "nick")))
			theValue = GetContactNickX(key, hContact);
		else if (!_xcscmp(theKey, XSTR(key, "proto")))
			theValue = mir_a2x(key, Proto_GetBaseAccountName(hContact));
		else if (!_xcscmp(theKey, XSTR(key, "accountname"))) {
			PROTOACCOUNT *acc = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
			if (acc != nullptr)
				theValue = mir_w2x(key, acc->tszAccountName);
		}
		else if (!_xcscmp(theKey, XSTR(key, "group"))) {
			ptrW wszGroup(Clist_GetGroup(hContact));
			if (wszGroup)
				theValue = mir_w2x(key, wszGroup);
			else
				theValue = mir_w2x(key, L"");
		}
		else if (!_xcscmp(theKey, XSTR(key, "userid")))
			theValue = GetContactIDX(key, hContact);
	}

	if (!theValue) {
		if (!_xcscmp(theKey, XSTR(key, "miranda_path")))
			theValue = GetModulePathX(key, nullptr);
		else if (!_xcscmp(theKey, XSTR(key, "appdata")))
			theValue = SHGetSpecialFolderPathX(CSIDL_APPDATA, theKey);
		else if (!_xcscmp(theKey, XSTR(key, "mydocuments")))
			theValue = SHGetSpecialFolderPathX(CSIDL_PERSONAL, theKey);
		else if (!_xcscmp(theKey, XSTR(key, "desktop")))
			theValue = SHGetSpecialFolderPathX(CSIDL_DESKTOPDIRECTORY, theKey);
		else if (!_xcscmp(theKey, XSTR(key, "miranda_profilesdir")))
			theValue = GetProfileDirX(key);
		else if (!_xcscmp(theKey, XSTR(key, "miranda_profilename")))
			theValue = GetProfileNameX(key);
		else if (!_xcscmp(theKey, XSTR(key, "username")))
			theValue = GetUserNameX(key);
		else if (!_xcscmp(theKey, XSTR(key, "miranda_avatarcache")))
			theValue = GetPathVarX(key, 1);
		else if (!_xcscmp(theKey, XSTR(key, "miranda_logpath")))
			theValue = GetPathVarX(key, 2);
		else if (!_xcscmp(theKey, XSTR(key, "miranda_userdata")))
			theValue = GetPathVarX(key, 3);
	}

	if (!theValue)
		theValue = GetEnvironmentVariableX(theKey);

	return theValue;
}

template<typename XCHAR>
XCHAR *GetVariableFromArray(REPLACEVARSARRAY *vars, const XCHAR *key, size_t keyLength, MCONTACT hContact, bool *bFree)
{
	*bFree = false;
	for (REPLACEVARSARRAY *var = vars; var && var->key.a; ++var)
		if ((_xcslen((XCHAR *)var->key.a) == keyLength) && !_xcsncmp(key, (XCHAR *)var->key.a, keyLength))
			return (XCHAR *)var->value.a;

	*bFree = true;
	return GetInternalVariable(key, keyLength, hContact);
}

template<typename XCHAR>
XCHAR *ReplaceVariables(const XCHAR *str, MCONTACT hContact, REPLACEVARSARRAY *variables)
{
	if (!str)
		return nullptr;

	const XCHAR *varStart = nullptr, *p;
	size_t length = 0;
	bool bFree;

	for (p = str; *p; ++p) {
		if (*p == '%') {
			if (varStart) {
				if (p == varStart)
					length++;
				else if (XCHAR *value = GetVariableFromArray(variables, varStart, p-varStart, hContact, &bFree)) {
					length += _xcslen(value);
					if (bFree) mir_free(value);
				}
				else // variable not found
					length += p-varStart+2;

				varStart = nullptr;
			}
			else varStart = p+1;
		}
		else if (!varStart)
			length++;
	}
	if (varStart)
		length += (p - varStart)+1;

	XCHAR *result = (XCHAR *)mir_alloc(sizeof(XCHAR) * (length + 1));
	XCHAR *q = result;
	varStart = nullptr;

	for (p = str; *p; ++p) {
		if (*p == '%') {
			if (varStart) {
				if (p == varStart)
					*q++='%';
				else if (XCHAR *value = GetVariableFromArray(variables, varStart, p-varStart, hContact, &bFree)) {
					_xcscpy(q, value);
					q += _xcslen(value);
					if (bFree) mir_free(value);
				}
				else {
					// variable not found
					_xcsncpy(q, varStart-1, p-varStart+2);
					q += p-varStart+2;
				}
				varStart = nullptr;
			}
			else varStart = p+1;
		}
		else if (!varStart)
			*q++=*p;
	}

	if (varStart) {
		size_t len = p - varStart + 1;
		_xcsncpy(q, varStart-1, len);
		q += len;
	}

	*q = 0;

	return result;
}

MIR_APP_DLL(char*) Utils_ReplaceVars(const char *szData, MCONTACT hContact, REPLACEVARSARRAY *vars)
{
	return ReplaceVariables<CHAR>(szData, hContact, vars);
}

MIR_APP_DLL(wchar_t*) Utils_ReplaceVarsW(const wchar_t *wszData, MCONTACT hContact, REPLACEVARSARRAY *vars)
{
	return ReplaceVariables<wchar_t>(wszData, hContact, vars);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnFoldersChanged(WPARAM, LPARAM)
{
	mir_snwprintf(tszAvatarRoot, L"%s\\%s\\AvatarCache", g_profileDir, g_shortProfileName);

	wchar_t tmpVar[MAX_PATH];
	if (!FoldersGetCustomPathW(hAvatarFolder, tmpVar, _countof(tmpVar), tszAvatarRoot))
		wcsncpy_s(tszAvatarRoot, tmpVar, _TRUNCATE);
	return 0;
}

void InitPathVar()
{
	mir_snwprintf(tszAvatarRoot, L"%s\\%s\\AvatarCache", g_profileDir, g_shortProfileName);
	if (hAvatarFolder = FoldersRegisterCustomPathW(LPGEN("Avatars"), LPGEN("Avatars root folder"), tszAvatarRoot)) {
		wchar_t tmpVar[MAX_PATH];
		if (!FoldersGetCustomPathW(hAvatarFolder, tmpVar, _countof(tmpVar), tszAvatarRoot))
			wcsncpy_s(tszAvatarRoot, tmpVar, _TRUNCATE);
		HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
	}
}
