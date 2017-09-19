/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

  This file contains some compatibility functions for Miranda IM
*/

#include "stdafx.h"

PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
LIST_INTERFACE li;
MD5_INTERFACE md5i;
UTF8_INTERFACE utfi;
PlugOptions opts;
POPUP_OPTIONS PopupOptions = {0};

static const MUUID interfaces[] = {MIID_UPDATER, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

static char szHexTable[] = "0123456789abcdef";

char *bin2hex(const void *pData, size_t len, char *dest)
{
	const BYTE *p = (const BYTE*)pData;
	char *d = dest;

	for (size_t i=0; i < len; i++, p++) {
		*d++ = szHexTable[*p >> 4];
		*d++ = szHexTable[*p & 0x0F];
	}
	*d = 0;

	return dest;
}

char *rtrim(char *str)
{
	if (str == NULL)
		return NULL;

	char *p = strchr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}

wchar_t* strdelw(wchar_t *str, size_t len)
{
	wchar_t* p;
	for (p = str + len; *p != 0; p++)
		*(p - len) = *p;

	*(p - len) = '\0';
	return str;
}

void CreatePathToFileW(WCHAR *tszFilePath)
{
	TCHAR *pszLastBackslash = _tcsrchr(tszFilePath, '\\');
	if (pszLastBackslash != NULL){
		*pszLastBackslash = '\0';
		CreateDirectoryTreeW(tszFilePath);
		*pszLastBackslash = '\\';
	}
}

int mir_snwprintf(WCHAR *buffer, size_t count, const WCHAR* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	int len = _vsntprintf(buffer, count-1, fmt, va);
	va_end(va);
	buffer[count-1] = 0;
	return len;
}

int mir_vsnwprintf(WCHAR *buffer, size_t count, const WCHAR* fmt, va_list va)
{
	int len = _vsntprintf(buffer, count-1, fmt, va);
	buffer[count-1] = 0;
	return len;
}

int mir_wstrcmpi(const wchar_t *p1, const wchar_t *p2)
{
	if (p1 == NULL)
		return (p2 == NULL) ? 0 : -1;
	if (p2 == NULL)
		return 1;
	return _wcsicmp(p1, p2);
}

int wildcmpiw(const WCHAR *name, const WCHAR *mask)
{
	if (name == NULL || mask == NULL)
		return false;

	const WCHAR* last = NULL;
	for (;; mask++, name++) {
		if (*mask != '?' && _qtoupper(*mask) != _qtoupper(*name)) break;
		if (*name == '\0') return ((BOOL)!*mask);
	}
	if (*mask != '*') return FALSE;
	for (;; mask++, name++) {
		while(*mask == '*') {
			last = mask++;
			if (*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if (*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if (*mask != '?' && _qtoupper(*mask)  != _qtoupper(*name)) name -= (size_t)(mask - last) - 1, mask = last;
	}
}


void __stdcall RestartMe(void*)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[MAX_PATH];
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));

	TCHAR *profilename = Utils_ReplaceVarsT(L"%miranda_profilename%");
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), L"\"%s\" /restart:%d /profile=%s", mirandaPath, GetCurrentProcessId(), profilename);
	mir_free(profilename);

	CallService("CloseAction", 0, 0);

	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

/////////////////////////////////////////////////////////////////////////////////////
// we don't use Icon_Register here because it should work under Miranda IM too

void InitIcoLib()
{
	TCHAR destfile[MAX_PATH];
	GetModuleFileName(hInst, destfile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_PATH_TCHAR;
	sid.ptszDefaultFile = destfile;
	sid.pszSection = MODULEA;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		sid.pszName = iconList[i].szIconName;
		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].IconID;
		IcoLib_AddIcon(&sid);
	}
}

TCHAR *GetDefaultUrl() {
	return mir_tstrdup(L"https://miranda-ng.org/distr/deprecated/0.94.9/x%platform%");
}
