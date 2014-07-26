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

void CreatePathToFileT(TCHAR *tszFilePath)
{
	TCHAR *pszLastBackslash = _tcsrchr(tszFilePath, '\\');
	if (pszLastBackslash != NULL){
		*pszLastBackslash = '\0';
		CreateDirectoryTreeT(tszFilePath);
		*pszLastBackslash = '\\';
	}
}

int wildcmpit(const WCHAR *name, const WCHAR *mask)
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

	TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
	mir_free(profilename);

	CallService("CloseAction", 0, 0);

	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}
