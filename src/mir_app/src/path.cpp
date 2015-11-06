/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\..\plugins\ExternalAPI\m_folders.h"

extern TCHAR g_profileDir[MAX_PATH], g_shortProfileName[MAX_PATH];

static HANDLE hAvatarFolder;
static TCHAR tszAvatarRoot[MAX_PATH];

TCHAR* GetContactID(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 1) {
		return db_get_tsa(hContact, szProto, "ChatRoomID");
	}
	else {
		CONTACTINFO ci = {0};
		ci.cbSize = sizeof(ci);
		ci.hContact = hContact;
		ci.szProto = szProto;
		ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
		if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM) & ci)) {
			switch (ci.type) {
			case CNFT_ASCIIZ:
				return (TCHAR *)ci.pszVal;
			case CNFT_DWORD:
				return _itot(ci.dVal, (TCHAR *)mir_alloc(sizeof(TCHAR)*32), 10);
			}
		}
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Variables parser

#define XSTR(target, s) _xstrselect(target, s, _T(s))

static __forceinline int _xcscmp(const char *s1, const char *s2) { return strcmp(s1, s2); }
static __forceinline int _xcsncmp(const char *s1, const char *s2, size_t n) { return strncmp(s1, s2, n); }
static __forceinline size_t _xcslen(const char *s1) { return strlen(s1); }
static __forceinline char *_xcscpy(char *s1, const char *s2) { return strcpy(s1, s2); }
static __forceinline char *_xcsncpy(char *s1, const char *s2, size_t n) { return strncpy(s1, s2, n); }
static __forceinline const char *_xstrselect(const char*, const char *s1, TCHAR*) { return s1; }
static __forceinline char *_itox(char*, int a) { return itoa(a, (char *)mir_alloc(sizeof(char)*20), 10); }
static __forceinline char *mir_a2x(const char*, const char *s) { return mir_strdup(s); }

static __forceinline char *GetContactNickX(const char*, MCONTACT hContact)
{
	return mir_t2a(cli.pfnGetContactDisplayName(hContact, 0));
}

static __forceinline char *GetContactIDX(const char*, MCONTACT hContact)
{
	TCHAR *id = GetContactID(hContact);
	char* res = mir_t2a(id);
	mir_free(id);
	return res;
}

static __forceinline char *GetEnvironmentVariableX(const char *variable)
{
	char result[512];
	if (GetEnvironmentVariableA(variable, result, _countof(result)))
		return mir_strdup(result);
	return NULL;
}

static __forceinline char *GetProfileDirX(const char*)
{
	return mir_t2a(g_profileDir);
}

static __forceinline char *SHGetSpecialFolderPathX(int iCSIDL, char*)
{
	char result[512];
	if (SHGetSpecialFolderPathA(NULL, result, iCSIDL, FALSE))
		return mir_strdup(result);
	return NULL;
}

static __forceinline char *GetModulePathX(const char*, HMODULE hModule)
{
	char result[MAX_PATH];
	GetModuleFileNameA(hModule, result, sizeof(result));
	char* str = strrchr(result, '\\');
	if (str) *str = 0;
	return mir_strdup(result);
}

static __forceinline char *GetUserNameX(const char*)
{
	char result[128];
	DWORD size = _countof(result);
	if (GetUserNameA(result, &size))
		return mir_strdup(result);
	return NULL;
}

static __forceinline char *GetProfileNameX(const char*)
{
	return mir_t2a(g_shortProfileName);
}

static __forceinline char* GetPathVarX(const char*, int code)
{
	TCHAR szFullPath[MAX_PATH];

	switch(code) {
	case 1:
		if (hAvatarFolder != NULL)
			_tcsncpy_s(szFullPath, tszAvatarRoot, _TRUNCATE);
		else
			mir_sntprintf(szFullPath, _T("%s\\%s\\AvatarCache"), g_profileDir, g_shortProfileName);
		break;
	case 2:
		mir_sntprintf(szFullPath, _T("%s\\%s\\Logs"), g_profileDir, g_shortProfileName);
		break;
	case 3:
		mir_sntprintf(szFullPath, _T("%s\\%s"), g_profileDir, g_shortProfileName);
		break;
	}
	return makeFileName(szFullPath);
}

static __forceinline int _xcscmp(const TCHAR *s1, const TCHAR *s2) { return _tcscmp(s1, s2); }
static __forceinline int _xcsncmp(const TCHAR *s1, const TCHAR *s2, size_t n) { return _tcsncmp(s1, s2, n); }
static __forceinline size_t _xcslen(const TCHAR *s1) { return _tcslen(s1); }
static __forceinline TCHAR* _xcscpy(TCHAR *s1, const TCHAR *s2) { return _tcscpy(s1, s2); }
static __forceinline TCHAR* _xcsncpy(TCHAR *s1, const TCHAR *s2, size_t n) { return _tcsncpy(s1, s2, n); }
static __forceinline const TCHAR* _xstrselect(const TCHAR*, const char*, const TCHAR *s2) { return s2; }
static __forceinline TCHAR* _itox(TCHAR *, int a) { return _itot(a, (TCHAR *)mir_alloc(sizeof(TCHAR)*20), 10); }
static __forceinline TCHAR* mir_a2x(const TCHAR *, const char *s) { return mir_a2t(s); }

static __forceinline TCHAR* GetContactNickX(const TCHAR*, MCONTACT hContact)
{
	return mir_tstrdup(cli.pfnGetContactDisplayName(hContact, 0));
}

static __forceinline TCHAR* GetContactIDX(const TCHAR*, MCONTACT hContact)
{
	return GetContactID(hContact);
}

static __forceinline TCHAR* GetEnvironmentVariableX(const TCHAR *variable)
{
	TCHAR result[512];
	if (GetEnvironmentVariable(variable, result, _countof(result)))
		return mir_tstrdup(result);
	return NULL;
}

static __forceinline TCHAR* SHGetSpecialFolderPathX(int iCSIDL, TCHAR*)
{
	TCHAR result[512];
	if (SHGetSpecialFolderPath(NULL, result, iCSIDL, FALSE))
		return mir_tstrdup(result);
	return NULL;
}

static __forceinline TCHAR* GetProfileDirX(const TCHAR*)
{
	return mir_tstrdup(g_profileDir);
}

static __forceinline TCHAR* GetModulePathX(const TCHAR*, HMODULE hModule)
{
	TCHAR result[MAX_PATH];
	GetModuleFileName(hModule, result, _countof(result));
	TCHAR* str = _tcsrchr(result, '\\');
	if (str) *str = 0;
	return mir_tstrdup(result);
}

static __forceinline TCHAR* GetUserNameX(const TCHAR*)
{
	TCHAR result[128];
	DWORD size = _countof(result);
	if (GetUserName(result, &size))
		return mir_tstrdup(result);
	return NULL;
}

static __forceinline TCHAR* GetProfileNameX(const TCHAR*)
{
	return mir_tstrdup(g_shortProfileName);
}

static __forceinline TCHAR* GetPathVarX(const TCHAR*, int code)
{
	TCHAR szFullPath[MAX_PATH];

	switch(code) {
	case 1:
		if (hAvatarFolder != NULL)
			_tcsncpy_s(szFullPath, tszAvatarRoot, _TRUNCATE);
		else
			mir_sntprintf(szFullPath, _T("%s\\%s\\AvatarCache"), g_profileDir, g_shortProfileName);
		break;
	case 2:
		mir_sntprintf(szFullPath, _T("%s\\%s\\Logs"), g_profileDir, g_shortProfileName);
		break;
	case 3:
		mir_sntprintf(szFullPath, _T("%s\\%s"), g_profileDir, g_shortProfileName);
		break;
	}
	return mir_tstrdup(szFullPath);
}

template<typename XCHAR>
XCHAR *GetInternalVariable(const XCHAR *key, size_t keyLength, MCONTACT hContact)
{
	XCHAR *theValue = NULL;
	XCHAR *theKey = (XCHAR *)_alloca(sizeof(XCHAR) * (keyLength + 1));
	_xcsncpy(theKey, key, keyLength);
	theKey[keyLength] = 0;

	if (hContact) {
		if (!_xcscmp(theKey, XSTR(key, "nick")))
			theValue = GetContactNickX(key, hContact);
		else if (!_xcscmp(theKey, XSTR(key, "proto")))
			theValue = mir_a2x(key, GetContactProto(hContact));
		else if (!_xcscmp(theKey, XSTR(key, "accountname"))) {
			PROTOACCOUNT *acc = Proto_GetAccount(GetContactProto(hContact));
			if (acc != NULL)
				theValue = mir_a2x(key, _T2A(acc->tszAccountName));
		}
		else if (!_xcscmp(theKey, XSTR(key, "userid")))
			theValue = GetContactIDX(key, hContact);
	}

	if (!theValue) {
		if (!_xcscmp(theKey, XSTR(key, "miranda_path")))
			theValue = GetModulePathX(key, NULL);
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
		return NULL;

	const XCHAR *varStart = 0, *p;
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

				varStart = 0;
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
	varStart = NULL;

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
				varStart = 0;
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
	return ReplaceVariables<WCHAR>(wszData, hContact, vars);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnFoldersChanged(WPARAM, LPARAM)
{
	mir_sntprintf(tszAvatarRoot, _T("%s\\%s\\AvatarCache"), g_profileDir, g_shortProfileName);

	TCHAR tmpVar[MAX_PATH];
	if (!FoldersGetCustomPathT(hAvatarFolder, tmpVar, _countof(tmpVar), tszAvatarRoot))
		_tcsncpy_s(tszAvatarRoot, tmpVar, _TRUNCATE);
	return 0;
}

void InitPathVar()
{
	mir_sntprintf(tszAvatarRoot, _T("%s\\%s\\AvatarCache"), g_profileDir, g_shortProfileName);
	if (hAvatarFolder = FoldersRegisterCustomPathT( LPGEN("Avatars"), LPGEN("Avatars root folder"), tszAvatarRoot)) {
		TCHAR tmpVar[MAX_PATH];
		if (!FoldersGetCustomPathT(hAvatarFolder, tmpVar, _countof(tmpVar), tszAvatarRoot))
			_tcsncpy_s(tszAvatarRoot, tmpVar, _TRUNCATE);
		HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
	}
}
