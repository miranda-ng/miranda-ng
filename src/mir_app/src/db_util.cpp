/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Profile_GetPathA(size_t cbLen, char *pszDest)
{
	if (!pszDest || !cbLen)
		return 1;

	strncpy_s(pszDest, cbLen, _T2A(g_profileDir), _TRUNCATE);
	return 0;
}

MIR_APP_DLL(int) Profile_GetPathW(size_t cbLen, wchar_t *pwszDest)
{
	if (!pwszDest || !cbLen)
		return 1;

	wcsncpy_s(pwszDest, cbLen, g_profileDir, _TRUNCATE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Profile_GetNameA(size_t cbLen, char *pszDest)
{
	if (!cbLen || !pszDest)
		return 1;

	strncpy_s(pszDest, cbLen, ptrA(makeFileName(g_profileName)), _TRUNCATE);
	return 0;
}

MIR_APP_DLL(int) Profile_GetNameW(size_t cbLen, wchar_t *pwszDest)
{
	if (!cbLen || !pwszDest)
		return 1;

	wcsncpy_s(pwszDest, cbLen, g_profileName, _TRUNCATE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Profile_GetSetting(const wchar_t *pwszSetting, wchar_t *pwszBuf, size_t cbLen, const wchar_t *pwszDefault)
{
	if (pwszSetting == nullptr) {
		*pwszBuf = 0;
		return false;
	}

	if (pwszDefault == nullptr)
		pwszDefault = L"";

	wchar_t *pBuf = NEWWSTR_ALLOCA(pwszSetting);

	wchar_t *p = wcschr(pBuf, '/');
	if (p) {
		*p = 0; p++;
		GetPrivateProfileStringW(pBuf, p, pwszDefault, pwszBuf, (uint32_t)cbLen, mirandabootini);
	}
	else GetPrivateProfileStringW(pBuf, L"", pwszDefault, pwszBuf, (uint32_t)cbLen, mirandabootini);

	return pwszBuf[0] != 0;
}

MIR_APP_DLL(int) Profile_GetSettingInt(const wchar_t *pwszSetting, int iDefault)
{
	if (pwszSetting == nullptr)
		return iDefault;

	wchar_t *pBuf = NEWWSTR_ALLOCA(pwszSetting);

	wchar_t *p = wcschr(pBuf, '/');
	if (p) {
		*p = 0; p++;
		return GetPrivateProfileIntW(pBuf, p, iDefault, mirandabootini);
	}
	
	return GetPrivateProfileIntW(pBuf, L"", iDefault, mirandabootini);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Profile_SetDefault(const wchar_t *pwszPath)
{
	extern wchar_t* g_defaultProfile;
	replaceStrW(g_defaultProfile, pwszPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Profile_CheckOpened(const wchar_t *pwszProfileName)
{
	CMStringW wszPhysName(pwszProfileName);
	wszPhysName.Replace(L"\\", L"_");
	wszPhysName.Insert(0, L"Global\\");

	HANDLE hMutex = ::OpenMutexW(SYNCHRONIZE, false, wszPhysName);
	if (hMutex == nullptr)
		return false;

	::CloseHandle(hMutex);
	return true;
}
