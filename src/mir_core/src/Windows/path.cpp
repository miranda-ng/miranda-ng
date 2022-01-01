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

#include "../stdafx.h"

static char  szMirandaPath[MAX_PATH];
static wchar_t szMirandaPathW[MAX_PATH];

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
	if (!pSrc || !pSrc[0] || strlen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	if (!PathIsAbsolute(pSrc))
		strncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	else {
		if (pBase == nullptr)
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
		return GetFullPathNameA(pSrc, MAX_PATH, pOut, nullptr);

	if (base == nullptr)
		base = szMirandaPath;

	if (pSrc[0] == '\\')
		pSrc++;
	mir_snprintf(buf, "%s%s", base, pSrc);
	return GetFullPathNameA(buf, _countof(buf), pOut, nullptr);
}

MIR_CORE_DLL(int) CreatePathToFile(const char *szFilePath)
{
	if (szFilePath == nullptr)
		return ERROR_INVALID_PARAMETER;

	char *buf = NEWSTR_ALLOCA(szFilePath);
	char *p = strrchr(buf, '\\');
	if (p == nullptr)
		return 0;

	*p = '\0';
	return CreateDirectoryTree(buf);
}

MIR_CORE_DLL(int) CreateDirectoryTree(const char *szDir)
{
	if (szDir == nullptr)
		return 1;

	uint32_t dwAttributes = GetFileAttributesA(szDir);
	if (dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	char szTestDir[MAX_PATH];
	mir_strncpy(szTestDir, szDir, _countof(szTestDir));
	char *pszLastBackslash = strrchr(szTestDir, '\\');
	if (pszLastBackslash == nullptr)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTree(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryA(szTestDir, nullptr) == 0) ? GetLastError() : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) PathIsAbsoluteW(const wchar_t *path)
{
	if (path && wcslen(path) > 2)
		if ((path[1] == ':' && path[2] == '\\') || (path[0] == '\\' && path[1] == '\\'))
			return 1;
	return 0;
}

MIR_CORE_DLL(int) PathToRelativeW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t *pBase)
{
	if (!pSrc || !pSrc[0] || wcslen(pSrc) > MAX_PATH)
		return 0;

	if (!PathIsAbsoluteW(pSrc))
		wcsncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	else {
		if (pBase == nullptr)
			pBase = szMirandaPathW;

		size_t cbBaseLen = wcslen(pBase);
		if (!wcsnicmp(pSrc, pBase, cbBaseLen))
			wcsncpy_s(pOut, MAX_PATH, pSrc + cbBaseLen, _TRUNCATE);
		else
			wcsncpy_s(pOut, MAX_PATH, pSrc, _TRUNCATE);
	}
	return (int)wcslen(pOut);
}

MIR_CORE_DLL(int) PathToAbsoluteW(const wchar_t *pSrc, wchar_t *pOut, const wchar_t *base)
{
	if (!pSrc || !pSrc[0] || wcslen(pSrc) > MAX_PATH) {
		*pOut = 0;
		return 0;
	}

	wchar_t buf[MAX_PATH];
	if (pSrc[0] < ' ')
		return mir_snwprintf(pOut, MAX_PATH, L"%s", pSrc);

	if (PathIsAbsoluteW(pSrc))
		return GetFullPathName(pSrc, MAX_PATH, pOut, nullptr);

	if (base == nullptr)
		base = szMirandaPathW;

	if (pSrc[0] == '\\')
		pSrc++;

	mir_snwprintf(buf, MAX_PATH, L"%s%s", base, pSrc);
	return GetFullPathName(buf, MAX_PATH, pOut, nullptr);
}

MIR_CORE_DLL(int) CreatePathToFileW(const wchar_t *wszFilePath)
{
	if (wszFilePath == nullptr)
		return ERROR_INVALID_PARAMETER;

	wchar_t *buf = NEWWSTR_ALLOCA(wszFilePath);
	wchar_t *p = wcsrchr(buf, '\\');
	if (p == nullptr)
		return 0;

	*p = '\0';
	return CreateDirectoryTreeW(buf);
}

MIR_CORE_DLL(int) CreateDirectoryTreeW(const wchar_t *szDir)
{
	if (szDir == nullptr)
		return 1;

	uint32_t dwAttributes = GetFileAttributesW(szDir);
	if (dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	wchar_t szTestDir[MAX_PATH];
	mir_wstrncpy(szTestDir, szDir, _countof(szTestDir));
	wchar_t *pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == nullptr)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryW(szTestDir, nullptr) == 0) ? GetLastError() : 0;
}

MIR_CORE_DLL(int) DeleteDirectoryTreeW(const wchar_t *pwszDir, bool bAllowUndo)
{
	if (pwszDir == nullptr)
		return ERROR_BAD_ARGUMENTS;

	CMStringW wszPath(pwszDir);
	wszPath.AppendChar(0);

	SHFILEOPSTRUCTW file_op = {
		nullptr,
		FO_DELETE,
		wszPath,
		L"",
		FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION,
		false,
		nullptr,
		L"" };

	if (bAllowUndo)
		file_op.fFlags |= FOF_ALLOWUNDO;

	return SHFileOperationW(&file_op);
}

int InitPathUtils(void)
{
	GetModuleFileNameA(nullptr, szMirandaPath, _countof(szMirandaPath));
	char *p = strrchr(szMirandaPath, '\\');
	if (p)
		p[1] = 0;

	GetModuleFileNameW(nullptr, szMirandaPathW, _countof(szMirandaPathW));
	wchar_t *tp = wcsrchr(szMirandaPathW, '\\');
	if (tp)
		tp[1] = 0;
	return 0;
}
