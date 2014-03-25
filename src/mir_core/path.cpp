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
static char  szMirandaPathLower[MAX_PATH];
static TCHAR szMirandaPathW[MAX_PATH];
static TCHAR szMirandaPathWLower[MAX_PATH];

MIR_CORE_DLL(int) PathIsAbsolute(const char *path)
{
	if (strlen(path) <= 2)
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
		return 1;
	return 0;
}

MIR_CORE_DLL(int) PathToRelative(const char *pSrc, char *pOut)
{
	if ( !pSrc || !strlen(pSrc) || strlen(pSrc)>MAX_PATH) return 0;
	if ( !PathIsAbsolute(pSrc)) {
		mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
		return (int)strlen(pOut);
	}

	char szTmp[MAX_PATH];
	mir_snprintf(szTmp, SIZEOF(szTmp), "%s", pSrc);
	_strlwr(szTmp);
	if (strstr(szTmp, szMirandaPathLower)) {
		mir_snprintf(pOut, MAX_PATH, "%s", pSrc+strlen(szMirandaPathLower));
		return (int)strlen(pOut);
	}
	else {
		mir_snprintf(pOut, MAX_PATH, "%s", pSrc);
		return (int)strlen(pOut);
	}
}

MIR_CORE_DLL(int) PathToAbsolute(const char *pSrc, char *pOut, char* base)
{
	if ( !pSrc || !strlen(pSrc) || strlen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	char buf[MAX_PATH];
	if (pSrc[0] < ' ')
		return mir_snprintf(pOut, MAX_PATH, "%s", pSrc);

	if (PathIsAbsolute(pSrc))
		return GetFullPathNameA(pSrc, MAX_PATH, pOut, NULL);

	if (base == NULL)
		base = szMirandaPath;

	if (pSrc[0] != '\\')
		mir_snprintf(buf, MAX_PATH, "%s%s", base, pSrc);
	else
		mir_snprintf(buf, MAX_PATH, "%s%s", base, pSrc+1);

	return GetFullPathNameA(buf, MAX_PATH, pOut, NULL);
}

MIR_CORE_DLL(void) CreatePathToFile(char* szFilePath)
{
	char* pszLastBackslash = strrchr(szFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTree(szFilePath);
	*pszLastBackslash = '\\';
}

MIR_CORE_DLL(int) CreateDirectoryTree(const char *szDir)
{
	DWORD dwAttributes;
	char *pszLastBackslash, szTestDir[ MAX_PATH ];

	lstrcpynA(szTestDir, szDir, SIZEOF(szTestDir));
	if ((dwAttributes = GetFileAttributesA(szTestDir)) != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	pszLastBackslash = strrchr(szTestDir, '\\');
	if (pszLastBackslash == NULL)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTree(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryA(szTestDir, NULL) == 0) ? GetLastError() : 0;
}

///////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) PathIsAbsoluteW(const TCHAR *path)
{
	if (lstrlen(path) <= 2)
		return 0;
	if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
		return 1;
	return 0;
}

MIR_CORE_DLL(int) PathToRelativeW(const WCHAR *pSrc, WCHAR *pOut)
{
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
		return 0;

	if (!PathIsAbsoluteW(pSrc))
		mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
	else {
		TCHAR szTmp[MAX_PATH];

		mir_sntprintf(szTmp, SIZEOF(szTmp), _T("%s"), pSrc);
		_tcslwr(szTmp);
		if (_tcsstr(szTmp, szMirandaPathWLower))
			mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc + lstrlen(szMirandaPathWLower));
		else
			mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);
	}
	return lstrlen(pOut);
}

MIR_CORE_DLL(int) PathToAbsoluteW(const TCHAR *pSrc, TCHAR *pOut, TCHAR* base)
{
	if (!pSrc || !wcslen(pSrc) || wcslen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	TCHAR buf[MAX_PATH];
	if (pSrc[0] < ' ')
		return mir_sntprintf(pOut, MAX_PATH, _T("%s"), pSrc);

	if (PathIsAbsoluteW(pSrc))
		return GetFullPathName(pSrc, MAX_PATH, pOut, NULL);

	if (base == NULL)
		base = szMirandaPathW;

	if (pSrc[0] != '\\')
		mir_sntprintf(buf, MAX_PATH, _T("%s%s"), base, pSrc);
	else
		mir_sntprintf(buf, MAX_PATH, _T("%s%s"), base, pSrc + 1);
	return GetFullPathName(buf, MAX_PATH, pOut, NULL);
}

MIR_CORE_DLL(void) CreatePathToFileW(WCHAR* wszFilePath)
{
	WCHAR *pszLastBackslash = wcsrchr(wszFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(wszFilePath);
	*pszLastBackslash = '\\';
}

MIR_CORE_DLL(int) CreateDirectoryTreeW(const WCHAR* szDir)
{
	DWORD  dwAttributes;
	WCHAR *pszLastBackslash, szTestDir[MAX_PATH];

	lstrcpynW(szTestDir, szDir, SIZEOF(szTestDir));
	if ((dwAttributes = GetFileAttributesW(szTestDir)) != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == NULL)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryW(szTestDir, NULL) == 0) ? GetLastError() : 0;
}

int InitPathUtils(void)
{
	char *p = 0;
	GetModuleFileNameA(hInst, szMirandaPath, SIZEOF(szMirandaPath));
	p = strrchr(szMirandaPath, '\\');
	if (p)
		p[1] = 0;
	mir_snprintf(szMirandaPathLower, MAX_PATH, "%s", szMirandaPath);
	_strlwr(szMirandaPathLower);

	GetModuleFileName(hInst, szMirandaPathW, SIZEOF(szMirandaPathW));
	TCHAR *tp = _tcsrchr(szMirandaPathW, '\\');
	if (tp)
		tp[1] = 0;
	mir_sntprintf(szMirandaPathWLower, SIZEOF(szMirandaPathWLower), _T("%s"), szMirandaPathW);
	_tcslwr(szMirandaPathWLower);
	return 0;
}
