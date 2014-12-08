/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "commonheaders.h"

static char  szMirandaPath[MAX_PATH];
static WCHAR szMirandaPathW[MAX_PATH];

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) PathIsAbsolute(const char *path)
{
	if (path && strlen(path) > 2)
		if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
			return 1;
	return 0;
}

MIR_CORE_DLL(int) PathToRelative(const char *pSrc, char *pOut, const char *pBase)
{
	if (!pSrc || !pSrc[0] || strlen(pSrc) > MAX_PATH)
		return 0;

	if (!PathIsAbsolute(pSrc))
		strncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	else {
		if (pBase == NULL)
			pBase = szMirandaPath;

		size_t cbBaseLen = strlen(pBase);
		if (!strnicmp(pSrc, pBase, cbBaseLen))
			strncpy_s(pOut, MAX_PATH, pSrc + cbBaseLen, _TRUNCATE);
		else
			strncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	}

	return (int)strlen(pOut);
}

MIR_CORE_DLL(int) PathToAbsolute(const char *pSrc, char *pOut, const char *base)
{
	if (!pSrc || !pSrc[0] || strlen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	char buf[MAX_PATH];
	if (pSrc[0] < ' ')
		strncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);

	if (PathIsAbsolute(pSrc))
		return GetFullPathNameA(pSrc, MAX_PATH, pOut, NULL);

	if (base == NULL)
		base = szMirandaPath;

	if (pSrc[0] == '\\')
		pSrc++;
	mir_snprintf(buf, SIZEOF(buf), "%s%s", base, pSrc);
	return GetFullPathNameA(buf, SIZEOF(buf), pOut, NULL);
}

MIR_CORE_DLL(void) CreatePathToFile(char *szFilePath)
{
	char *pszLastBackslash = strrchr(szFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTree(szFilePath);
	*pszLastBackslash = '\\';
}

MIR_CORE_DLL(int) CreateDirectoryTree(const char *szDir)
{
	char szTestDir[MAX_PATH];
	mir_strncpy(szTestDir, szDir, SIZEOF(szTestDir));

	DWORD dwAttributes = GetFileAttributesA(szTestDir);
	if (dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	char *pszLastBackslash = strrchr(szTestDir, '\\');
	if (pszLastBackslash == NULL)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTree(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryA(szTestDir, NULL) == 0) ? GetLastError() : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) PathIsAbsoluteW(const WCHAR *path)
{
	if (path && wcslen(path) > 2)
		if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
			return 1;
	return 0;
}

MIR_CORE_DLL(int) PathToRelativeW(const WCHAR *pSrc, WCHAR *pOut, const WCHAR *pBase)
{
	if (!pSrc || !pSrc[0] || wcslen(pSrc) > MAX_PATH)
		return 0;

	if (!PathIsAbsoluteW(pSrc))
		wcsncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	else {
		if (pBase == NULL)
			pBase = szMirandaPathW;

		size_t cbBaseLen = wcslen(pBase);
		if (!wcsnicmp(pSrc, pBase, cbBaseLen))
			wcsncpy_s(pOut, MAX_PATH, pSrc + cbBaseLen, _TRUNCATE);
		else
			wcsncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	}
	return (int)wcslen(pOut);
}

MIR_CORE_DLL(int) PathToAbsoluteW(const WCHAR *pSrc, WCHAR *pOut, const WCHAR *base)
{
	if (!pSrc || !pSrc[0] || wcslen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	WCHAR buf[MAX_PATH];
	if (pSrc[0] < ' ')
		return mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);

	if (PathIsAbsoluteW(pSrc))
		return GetFullPathName(pSrc, MAX_PATH, pOut, NULL);

	if (base == NULL)
		base = szMirandaPathW;

	if (pSrc[0] == '\\')
		pSrc++;

	mir_sntprintf(buf, MAX_PATH, _T("%s%s"), base, pSrc);
	return GetFullPathName(buf, MAX_PATH, pOut, NULL);
}

MIR_CORE_DLL(void) CreatePathToFileW(WCHAR *wszFilePath)
{
	WCHAR *pszLastBackslash = wcsrchr(wszFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(wszFilePath);
	*pszLastBackslash = '\\';
}

MIR_CORE_DLL(int) CreateDirectoryTreeW(const WCHAR *szDir)
{
	WCHAR szTestDir[MAX_PATH];
	mir_wstrncpy(szTestDir, szDir, SIZEOF(szTestDir));

	DWORD dwAttributes = GetFileAttributesW(szTestDir);
	if (dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	WCHAR *pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == NULL)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryW(szTestDir, NULL) == 0) ? GetLastError() : 0;
}

int InitPathUtils(void)
{
	GetModuleFileNameA(hInst, szMirandaPath, SIZEOF(szMirandaPath));
	char *p = strrchr(szMirandaPath, '\\');
	if (p)
		p[1] = 0;

	GetModuleFileNameW(hInst, szMirandaPathW, SIZEOF(szMirandaPathW));
	WCHAR *tp = wcsrchr(szMirandaPathW, '\\');
	if (tp)
		tp[1] = 0;
	return 0;
}
