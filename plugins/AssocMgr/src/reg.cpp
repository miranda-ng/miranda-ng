/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#ifndef FILETYPEATTRIBUTEFLAGS
	#define FTA_Exclude               0x00000001
	#define FTA_Show                  0x00000002
	#define FTA_HasExtension          0x00000004
	#define FTA_NoEdit                0x00000008
	#define FTA_NoRemove              0x00000010
	#define FTA_NoNewVerb             0x00000020
	#define FTA_NoEditVerb            0x00000040
	#define FTA_NoRemoveVerb          0x00000080
	#define FTA_NoEditDesc            0x00000100
	#define FTA_NoEditIcon            0x00000200
	#define FTA_NoEditDflt            0x00000400
	#define FTA_NoEditVerbCmd         0x00000800
	#define FTA_NoEditVerbExe         0x00001000
	#define FTA_NoDDE                 0x00002000
	#define FTA_NoEditMIME            0x00008000
	#define FTA_OpenIsSafe            0x00010000
	#define FTA_AlwaysUnsafe          0x00020000
	#define FTA_AlwaysShowExt         0x00040000
	#define FTA_NoRecentDocs          0x00100000
	#define FTA_SafeForElevation      0x00200000
	#define FTA_AlwaysUseDirectInvoke 0x00400000
#endif  /* FTA_Exclude */

#ifdef _DEBUG
// Debug: Ensure all registry calls do succeed and have valid parameters.
// Shows a details message box otherwise.
static __inline LONG regchk(LONG res, const char *pszFunc, const void *pszInfo, BOOL fInfoUnicode, const char *pszFile, unsigned int nLine)
{
	if (res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND && res != ERROR_NO_MORE_ITEMS) {
		wchar_t szMsg[1024], *pszInfo2;
		char *pszErr;
		pszErr = GetWinErrorDescription(res);
		pszInfo2 = s2t(pszInfo, fInfoUnicode, FALSE);  // does NULL check
		mir_snwprintf(szMsg, TranslateT("Access failed:\n%.64hs(%.128s)\n%.250hs(%u)\n%.256hs (%u)"), pszFunc, pszInfo2, pszFile, nLine, pszErr, res);
		MessageBox(nullptr, szMsg, TranslateT("Registry warning"), MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL);
		if (pszErr != nullptr) LocalFree(pszErr);
		mir_free(pszInfo2);  // does NULL check
	}
	return res;
}
#undef  RegCloseKey
#define RegCloseKey(hKey) \
	regchk(RegCloseKey(hKey),"RegCloseKey",NULL,FALSE,__FILE__,__LINE__)
#undef  RegOpenKeyExA
#define RegOpenKeyExA(hKey,szSubkey,opt,rights,phKey) \
	regchk(RegOpenKeyExA(hKey,szSubkey,opt,rights,phKey),"RegOpenKeyExA",szSubkey,FALSE,__FILE__,__LINE__)
#undef  RegCreateKeyExA
#define RegCreateKeyExA(hKey,szSubkey,x,y,opt,rights,sec,phKey,pDisp) \
	regchk(RegCreateKeyExA(hKey,szSubkey,x,y,opt,rights,sec,phKey,pDisp),"RegCreateKeyExA",szSubkey,FALSE,__FILE__,__LINE__)
#undef  RegDeleteKeyA
#define RegDeleteKeyA(hKey,szName) \
	regchk(RegDeleteKeyA(hKey,szName),"RegDeleteKeyA",szName,FALSE,__FILE__,__LINE__)
#undef  RegSetValueExA
#define RegSetValueExA(hSubKey,szName,x,type,pVal,size) \
	regchk(RegSetValueExA(hSubKey,szName,x,type,pVal,size),"RegSetValueExA",szName,FALSE,__FILE__,__LINE__)
#undef  RegQueryValueExA
#define RegQueryValueExA(hKey,szName,x,pType,pVal,pSize) \
	regchk(RegQueryValueExA(hKey,szName,x,pType,pVal,pSize),"RegQueryValueExA",szName,FALSE,__FILE__,__LINE__)
#undef  RegQueryInfoKeyA
#define RegQueryInfoKeyA(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime) \
	regchk(RegQueryInfoKeyA(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime),"RegQueryInfoKeyA",NULL,FALSE,__FILE__,__LINE__)
#undef  RegEnumKeyExA
#define RegEnumKeyExA(hKey,idx,pName,pnName,x,y,z,pTime) \
	regchk(RegEnumKeyExA(hKey,idx,pName,pnName,x,y,z,pTime),"RegEnumKeyExA",NULL,FALSE,__FILE__,__LINE__)
#undef  RegDeleteValueA
#define RegDeleteValueA(hKey,szName) \
	regchk(RegDeleteValueA(hKey,szName),"RegDeleteValueA",szName,FALSE,__FILE__,__LINE__)
#undef  RegOpenKeyExW
#define RegOpenKeyExW(hKey,szSubkey,x,sam,phKey) \
	regchk(RegOpenKeyExW(hKey,szSubkey,x,sam,phKey),"RegOpenKeyExW",szSubkey,TRUE,__FILE__,__LINE__)
#undef  RegCreateKeyExW
#define RegCreateKeyExW(hKey,szSubkey,x,y,z,rights,p,phKey,q) \
	regchk(RegCreateKeyExW(hKey,szSubkey,x,y,z,rights,p,phKey,q),"RegCreateKeyExW",szSubkey,TRUE,__FILE__,__LINE__)
#undef  RegDeleteKeyW
#define RegDeleteKeyW(hKey,szName) \
	regchk(RegDeleteKeyW(hKey,szName),"RegDeleteKeyW",szName,TRUE,__FILE__,__LINE__)
#undef  RegSetValueExW
#define RegSetValueExW(hSubKey,szName,x,type,pVal,size) \
	regchk(RegSetValueExW(hSubKey,szName,x,type,pVal,size),"RegSetValueExW",szName,TRUE,__FILE__,__LINE__)
#undef  RegQueryValueExW
#define RegQueryValueExW(hKey,szName,x,pType,pVal,pSize) \
	regchk(RegQueryValueExW(hKey,szName,x,pType,pVal,pSize),"RegQueryValueExW",szName,TRUE,__FILE__,__LINE__)
#undef  RegQueryInfoKeyW
#define RegQueryInfoKeyW(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime) \
	regchk(RegQueryInfoKeyW(hKey,x,y,z,pnKeys,pnKeyLen,a,pnVals,pnNames,pnValLen,sec,pTime),"RegQueryInfoKeyW",NULL,TRUE,__FILE__,__LINE__)
#undef  RegEnumKeyExW
#define RegEnumKeyExW(hKey,idx,pName,pnName,x,y,z,pTime) \
	regchk(RegEnumKeyExW(hKey,idx,pName,pnName,x,y,z,pTime),"RegEnumKeyExW",NULL,TRUE,__FILE__,__LINE__)
#undef  RegDeleteValueW
#define RegDeleteValueW(hKey,szName) \
	regchk(RegDeleteValueW(hKey,szName),"RegDeleteValueW",szName,TRUE,__FILE__,__LINE__)
#endif // _DEBUG

/************************* Strings ********************************/

// mir_free() the return value
char* MakeFileClassName(const char *pszFileExt)
{
	size_t cbLen = mir_strlen(pszFileExt) + 12;
	char *pszClass = (char*)mir_alloc(cbLen);
	if (pszClass != nullptr)
		// using correctly formated PROGID
		mir_snprintf(pszClass, cbLen, "miranda%sfile", pszFileExt); // includes dot, buffer safe
	return pszClass;
}

// mir_free() the return value
char* MakeUrlClassName(const char *pszUrl)
{
	char *pszClass = mir_strdup(pszUrl);
	if (pszClass != nullptr)
		// remove trailing :
		pszClass[mir_strlen(pszClass) - 1] = 0;
	return pszClass;
}

static BOOL IsFileClassName(char *pszClassName, char **ppszFileExt)
{
	*ppszFileExt = strchr(pszClassName, '.');
	return *ppszFileExt != nullptr;
}

// mir_free() the return value
wchar_t* MakeRunCommand(BOOL fMirExe, BOOL fFixedDbProfile)
{
	wchar_t szDbFile[MAX_PATH], szExe[MAX_PATH], *pszFmt;
	if (fFixedDbProfile) {
		if (Profile_GetNameW(_countof(szDbFile), szDbFile))
			return nullptr;
		wchar_t *p = wcsrchr(szDbFile, '.');
		if (p)
			*p = 0;
	}
	else mir_wstrcpy(szDbFile, L"%1"); // buffer safe

	if (!GetModuleFileName(fMirExe ? nullptr : g_plugin.getInst(), szExe, _countof(szExe)))
		return nullptr;

	if (fMirExe)
		// run command for miranda32.exe
		pszFmt = L"\"%s\" \"/profile:%s\"";
	else {
		// run command for rundll32.exe calling WaitForDDE
		pszFmt = L"rundll32.exe %s,WaitForDDE \"/profile:%s\"";
		// ensure the command line is not too long
		GetShortPathName(szExe, szExe, _countof(szExe));
		// surround by quotes if failed
		size_t len = mir_wstrlen(szExe);
		if (wcschr(szExe, ' ') != nullptr && (len + 2) < _countof(szExe)) {
			memmove(szExe, szExe + 1, (len + 1) * sizeof(wchar_t));
			szExe[len + 2] = szExe[0] = '\"';
			szExe[len + 3] = 0;
		}
	}

	wchar_t tszBuffer[1024];
	mir_snwprintf(tszBuffer, pszFmt, szExe, szDbFile);
	return mir_wstrdup(tszBuffer);
}

static BOOL IsValidRunCommand(const wchar_t *pszRunCmd)
{
	wchar_t *pexe, *pargs;
	wchar_t szFullExe[MAX_PATH], *pszFilePart;
	wchar_t *buf = mir_wstrcpy((wchar_t*)_alloca((mir_wstrlen(pszRunCmd) + 1) * sizeof(wchar_t)), pszRunCmd);
	// split into executable path and arguments
	if (buf[0] == '\"') {
		pargs = wcschr(&buf[1], '\"');
		if (pargs != nullptr) *(pargs++) = 0;
		pexe = &buf[1];
		if (*pargs == ' ') ++pargs;
	}
	else {
		pargs = wcschr(buf, ' ');
		if (pargs != nullptr) *pargs = 0;
		pexe = buf;
	}
	if (SearchPath(nullptr, pexe, L".exe", _countof(szFullExe), szFullExe, &pszFilePart)) {
		if (pszFilePart != nullptr)
			if (!mir_wstrcmpi(pszFilePart, L"rundll32.exe") || !mir_wstrcmpi(pszFilePart, L"rundll.exe")) {
				// split into dll path and arguments
				if (pargs[0] == '\"') {
					++pargs;
					pexe = wcschr(&pargs[1], '\"');
					if (pexe != nullptr) *pexe = 0;
				}
				else {
					pexe = wcschr(pargs, ',');
					if (pexe != nullptr) *pexe = 0;
				}
				return SearchPath(nullptr, pargs, L".dll", 0, nullptr, nullptr) != 0;
			}
		return TRUE;
	}
	return FALSE;
}

// mir_free() the return value
wchar_t* MakeIconLocation(HMODULE hModule, uint16_t nIconResID)
{
	wchar_t szModule[MAX_PATH], *pszIconLoc = nullptr;
	int cch;
	if ((cch = GetModuleFileName(hModule, szModule, _countof(szModule))) != 0) {
		pszIconLoc = (wchar_t*)mir_alloc((cch += 8) * sizeof(wchar_t));
		if (pszIconLoc != nullptr)
			mir_snwprintf(pszIconLoc, cch, L"%s,%i", szModule, -(int)nIconResID); // id may be 0, buffer safe
	}
	return pszIconLoc;
}

// mir_free() the return value
wchar_t* MakeAppFileName(BOOL fMirExe)
{
	wchar_t szExe[MAX_PATH], *psz;
	if (GetModuleFileName(fMirExe ? nullptr : g_plugin.getInst(), szExe, _countof(szExe))) {
		psz = wcsrchr(szExe, '\\');
		if (psz != nullptr) ++psz;
		else psz = szExe;
		return mir_wstrdup(psz);
	}
	return nullptr;
}

/************************* Helpers ********************************/

static LONG DeleteRegSubTree(HKEY hKey, const wchar_t *pszSubKey)
{
	LONG res;
	DWORD nMaxSubKeyLen, cchSubKey;
	wchar_t *pszSubKeyBuf;
	HKEY hSubKey;
	if ((res = RegOpenKeyEx(hKey, pszSubKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | DELETE, &hSubKey)) == ERROR_SUCCESS) {
		if ((res = RegQueryInfoKey(hSubKey, nullptr, nullptr, nullptr, nullptr, &nMaxSubKeyLen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) == ERROR_SUCCESS) {
			pszSubKeyBuf = (wchar_t*)mir_alloc((nMaxSubKeyLen + 1) * sizeof(wchar_t));
			if (pszSubKeyBuf == nullptr) res = ERROR_NOT_ENOUGH_MEMORY;
			while (!res) {
				cchSubKey = nMaxSubKeyLen + 1;
				if ((res = RegEnumKeyEx(hSubKey, 0, pszSubKeyBuf, &cchSubKey, nullptr, nullptr, nullptr, nullptr)) == ERROR_SUCCESS)
					res = DeleteRegSubTree(hSubKey, pszSubKeyBuf); // recursion
			}
			mir_free(pszSubKeyBuf); // does NULL check
			if (res == ERROR_NO_MORE_ITEMS) res = ERROR_SUCCESS;
		}
		RegCloseKey(hSubKey);
	}
	if (!res) res = RegDeleteKey(hKey, pszSubKey);
	return res;
}

// hMainKey must have been opened with KEY_CREATE_SUB_KEY access right
static LONG SetRegSubKeyStrDefValue(HKEY hMainKey, const wchar_t *pszSubKey, const wchar_t *pszVal)
{
	HKEY hSubKey;
	LONG res = RegCreateKeyEx(hMainKey, pszSubKey, 0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, nullptr, &hSubKey, nullptr);
	if (!res) {
		res = RegSetValueEx(hSubKey, nullptr, 0, REG_SZ, (uint8_t*)pszVal, (int)(mir_wstrlen(pszVal) + 1) * sizeof(wchar_t));
		RegCloseKey(hSubKey);
	}
	return res;
}

// hKey must have been opened with KEY_SET_VALUE access right
static void SetRegStrPrefixValue(HKEY hKey, const wchar_t *pszValPrefix, const wchar_t *pszVal)
{
	size_t dwSize = (mir_wstrlen(pszVal) + mir_wstrlen(pszValPrefix) + 1) * sizeof(wchar_t);
	wchar_t *pszStr = (wchar_t*)_alloca(dwSize);
	mir_wstrcat(mir_wstrcpy(pszStr, pszValPrefix), pszVal); // buffer safe
	RegSetValueEx(hKey, nullptr, 0, REG_SZ, (uint8_t*)pszStr, (int)dwSize);
}

// hKey must have been opened with KEY_QUERY_VALUE access right
// mir_free() the return value
static wchar_t* GetRegStrValue(HKEY hKey, const wchar_t *pszValName)
{
	// get size
	DWORD dwSize, dwType;
	if (!RegQueryValueEx(hKey, pszValName, nullptr, nullptr, nullptr, &dwSize) && dwSize > sizeof(wchar_t)) {
		wchar_t *pszVal = (wchar_t*)mir_alloc(dwSize + sizeof(wchar_t));
		if (pszVal != nullptr) {
			// get value
			if (!RegQueryValueEx(hKey, pszValName, nullptr, &dwType, (uint8_t*)pszVal, &dwSize)) {
				pszVal[dwSize / sizeof(wchar_t)] = 0;
				if (dwType == REG_EXPAND_SZ) {
					dwSize = MAX_PATH;
					wchar_t *pszVal2 = (wchar_t*)mir_alloc(dwSize * sizeof(wchar_t));
					if (ExpandEnvironmentStrings(pszVal, pszVal2, dwSize)) {
						mir_free(pszVal);
						return pszVal2;
					}
					mir_free(pszVal2);
				}
				else if (dwType == REG_SZ)
					return pszVal;
			}
			mir_free(pszVal);
		}
	}
	return nullptr;
}

// hKey must have been opened with KEY_QUERY_VALUE access right
static BOOL IsRegStrValue(HKEY hKey, const wchar_t *pszValName, const wchar_t *pszCmpVal)
{
	BOOL fSame = FALSE;
	wchar_t *pszVal = GetRegStrValue(hKey, pszValName);
	if (pszVal != nullptr) {
		fSame = !mir_wstrcmp(pszVal, pszCmpVal);
		mir_free(pszVal);
	}
	return fSame;
}

// hKey must have been opened with KEY_QUERY_VALUE access right
static BOOL IsRegStrValueA(HKEY hKey, const wchar_t *pszValName, const char *pszCmpVal)
{
	BOOL fSame = FALSE;
	wchar_t *pszVal = GetRegStrValue(hKey, pszValName);
	if (pszVal != nullptr) {
		fSame = !mir_strcmp(_T2A(pszVal), pszCmpVal);
		mir_free(pszVal);
	}
	return fSame;
}

/************************* Backup to DB ***************************/

#define REGF_ANSI  0x80000000 // this bit is set in dwType for ANSI registry data 

// pData must always be Unicode data, registry supports Unicode even on Win95
static void WriteDbBackupData(const char *pszSetting, uint32_t dwType, uint8_t *pData, uint32_t cbData)
{
	size_t cbLen = cbData + sizeof(uint32_t);
	uint8_t *buf = (uint8_t*)mir_alloc(cbLen);
	if (buf) {
		*(uint32_t*)buf = dwType;
		memcpy(buf + sizeof(uint32_t), pData, cbData);
		db_set_blob(0, MODULENAME, pszSetting, buf, (unsigned)cbLen);
		mir_free(buf);
	}
}

// mir_free() the value returned in ppData 
static BOOL ReadDbBackupData(const char *pszSetting, uint32_t *pdwType, uint8_t **ppData, uint32_t *pcbData)
{
	DBVARIANT dbv;
	if (!db_get(0, MODULENAME, pszSetting, &dbv)) {
		if (dbv.type == DBVT_BLOB && dbv.cpbVal >= sizeof(uint32_t)) {
			*pdwType = *(uint32_t*)dbv.pbVal;
			*ppData = dbv.pbVal;
			*pcbData = dbv.cpbVal - sizeof(uint32_t);
			memmove(*ppData, *ppData + sizeof(uint32_t), *pcbData);
			return TRUE;
		}
		db_free(&dbv);
	}
	return FALSE;
}

struct BackupRegTreeParam
{
	char **ppszDbPrefix;
	uint32_t *pdwDbPrefixSize;
	int level;
};

static void BackupRegTree_Worker(HKEY hKey, const char *pszSubKey, struct BackupRegTreeParam *param)
{
	LONG res;
	DWORD nMaxSubKeyLen, nMaxValNameLen, nMaxValSize;
	DWORD cchName, dwType, cbData;
	char *pszName;
	DWORD nDbPrefixLen;
	if ((res = RegOpenKeyExA(hKey, pszSubKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hKey)) == ERROR_SUCCESS) {
		if ((res = RegQueryInfoKey(hKey, nullptr, nullptr, nullptr, nullptr, &nMaxSubKeyLen, nullptr, nullptr, &nMaxValNameLen, &nMaxValSize, nullptr, nullptr)) == ERROR_SUCCESS) {
			if (nMaxSubKeyLen > nMaxValNameLen) nMaxValNameLen = nMaxSubKeyLen;
			// prepare buffer
			nDbPrefixLen = (uint32_t)(mir_strlen(*param->ppszDbPrefix) + mir_strlen(pszSubKey) + 1);
			cchName = nDbPrefixLen + nMaxValNameLen + 3;
			if (cchName > *param->pdwDbPrefixSize) {
				pszName = (char*)mir_realloc(*param->ppszDbPrefix, cchName);
				if (pszName == nullptr) return;
				*param->ppszDbPrefix = pszName;
				*param->pdwDbPrefixSize = cchName;
			}
			mir_strcat(mir_strcat(*param->ppszDbPrefix, pszSubKey), "\\"); // buffer safe
			// enum values
			pszName = (char*)mir_alloc(nMaxValNameLen + 1);
			if (nMaxValSize == 0) nMaxValSize = 1;
			uint8_t *pData = (uint8_t*)mir_alloc(nMaxValSize);
			if (pszName != nullptr && pData != nullptr) {
				uint32_t index = 0;
				while (!res) {
					cchName = nMaxValNameLen + 1;
					cbData = nMaxValSize;
					if ((res = RegEnumValueA(hKey, index++, pszName, &cchName, nullptr, nullptr, nullptr, nullptr)) == ERROR_SUCCESS) {
						(*param->ppszDbPrefix)[nDbPrefixLen] = 0;
						mir_strcat(*param->ppszDbPrefix, pszName); // buffer safe
						if (!RegQueryValueEx(hKey, _A2T(pszName), nullptr, &dwType, pData, &cbData))
							WriteDbBackupData(*param->ppszDbPrefix, dwType, pData, cbData);
					}
				}
				if (res == ERROR_NO_MORE_ITEMS)
					res = ERROR_SUCCESS;
			}
			mir_free(pData); // does NULL check
			// enum subkeys
			if (param->level < 32 && pszName != nullptr) {
				++param->level; // can be max 32 levels deep (after prefix), restriction of RegCreateKeyEx()
				uint32_t index = 0;
				while (!res) {
					cchName = nMaxSubKeyLen + 1;
					if ((res = RegEnumKeyExA(hKey, index++, pszName, &cchName, nullptr, nullptr, nullptr, nullptr)) == ERROR_SUCCESS) {
						(*param->ppszDbPrefix)[nDbPrefixLen] = 0;
						BackupRegTree_Worker(hKey, pszName, param); // recursion
					}
				}
			}
			if (res == ERROR_NO_MORE_ITEMS) res = ERROR_SUCCESS;
			mir_free(pszName); // does NULL check
		}
		RegCloseKey(hKey);
	}
}

static void BackupRegTree(HKEY hKey, const char *pszSubKey, const char *pszDbPrefix)
{
	char *prefix = mir_strdup(pszDbPrefix);
	struct BackupRegTreeParam param;
	uint32_t dwDbPrefixSize;
	param.level = 0;
	param.pdwDbPrefixSize = &dwDbPrefixSize;
	param.ppszDbPrefix = (char**)&prefix;
	dwDbPrefixSize = (int)mir_strlen(prefix) + 1;
	BackupRegTree_Worker(hKey, pszSubKey, &param);
	mir_free(prefix);
}

static LONG RestoreRegTree(HKEY hKey, const char *pszSubKey, const char *pszDbPrefix)
{
	int nDbPrefixLen = (int)mir_strlen(pszDbPrefix);
	int nPrefixWithSubKeyLen = nDbPrefixLen + (int)mir_strlen(pszSubKey) + 1;
	char *pszPrefixWithSubKey = (char*)mir_alloc(nPrefixWithSubKeyLen + 1);
	if (pszPrefixWithSubKey == nullptr)
		return ERROR_OUTOFMEMORY;

	mir_strcat(mir_strcat(mir_strcpy(pszPrefixWithSubKey, pszDbPrefix), pszSubKey), "\\"); // buffer safe
	LONG res = ERROR_NO_MORE_ITEMS;
	if (pszPrefixWithSubKey != nullptr) {
		int nSettingsCount;
		char **ppszSettings;
		if (EnumDbPrefixSettings(MODULENAME, pszPrefixWithSubKey, &ppszSettings, &nSettingsCount)) {
			for (int i = 0; i < nSettingsCount; ++i) {
				char *pszSuffix = &ppszSettings[i][nDbPrefixLen];
				// key hierachy
				char *pkeys = mir_strcpy((char*)_alloca(mir_strlen(pszSuffix) + 1), pszSuffix);
				char *pnext = pkeys, *pslash = nullptr;
				while ((pnext = strchr(pnext + 1, '\\')) != nullptr)
					pslash = pnext;
				if (pslash != nullptr) {
					// create subkey
					*(pslash++) = 0;
					HKEY hSubKey = hKey;
					if (pslash != pkeys + 1)
						if ((res = RegCreateKeyExA(hKey, pkeys, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &hSubKey, nullptr)) != ERROR_SUCCESS)
							break;
					char *pszValName = pslash;
					// read data
					uint8_t *pData;
					uint32_t dwType, cbData;
					if (ReadDbBackupData(ppszSettings[i], &dwType, &pData, &cbData)) {
						// set value
						if (!(dwType & REGF_ANSI)) {
							ptrW pwszValName(mir_a2u(pszValName));
							if (pwszValName != nullptr)
								res = RegSetValueExW(hSubKey, pwszValName, 0, dwType, pData, cbData);
							else
								res = ERROR_NOT_ENOUGH_MEMORY;
						}
						else res = RegSetValueExA(hSubKey, pszValName, 0, dwType&~REGF_ANSI, pData, cbData);
						mir_free(pData);
					}
					else res = ERROR_INVALID_DATA;
					if (res)
						break;

					g_plugin.delSetting(ppszSettings[i]);
					if (hSubKey != hKey)
						RegCloseKey(hSubKey);
				}
				mir_free(ppszSettings[i]);
			}
			mir_free(ppszSettings);
		}
		mir_free(pszPrefixWithSubKey);
	}
	return res;
}

static void DeleteRegTreeBackup(const char *pszSubKey, const char *pszDbPrefix)
{
	char **ppszSettings;
	int nSettingsCount, i;

	char *pszPrefixWithSubKey = (char*)mir_alloc(mir_strlen(pszDbPrefix) + mir_strlen(pszSubKey) + 2);
	if (pszPrefixWithSubKey == nullptr) return;
	mir_strcat(mir_strcat(mir_strcpy(pszPrefixWithSubKey, pszDbPrefix), pszSubKey), "\\"); // buffer safe
	if (pszPrefixWithSubKey != nullptr) {
		if (EnumDbPrefixSettings(MODULENAME, pszPrefixWithSubKey, &ppszSettings, &nSettingsCount)) {
			for (i = 0; i < nSettingsCount; ++i) {
				g_plugin.delSetting(ppszSettings[i]);
				mir_free(ppszSettings[i]);
			}
			mir_free(ppszSettings);
		}
		mir_free(pszPrefixWithSubKey);
	}
}

void CleanupRegTreeBackupSettings(void)
{
	// delete old bak_* settings and try to restore backups
	int nSettingsCount;
	char **ppszSettings;
	if (!EnumDbPrefixSettings(MODULENAME, "bak_", &ppszSettings, &nSettingsCount))
		return;

	for (int i = 0; i < nSettingsCount; ++i) {
		char *pszClassName = &ppszSettings[i][4];
		char *pszBuf = strchr(pszClassName, '\\');
		if (pszBuf != nullptr) {
			*pszBuf = '\0';

			// remove others in list with same class name
			if (i < nSettingsCount - 1) {
				for (int j = i + 1; j < nSettingsCount; ++j) {
					pszBuf = strchr(&ppszSettings[j][4], '\\');
					if (pszBuf != nullptr) *pszBuf = '\0';
					if (mir_strcmp(pszClassName, &ppszSettings[j][4])) {
						if (pszBuf != nullptr) *pszBuf = '\\';
						continue;
					}

					mir_free(ppszSettings[j]);
					memmove(&ppszSettings[j], &ppszSettings[j + 1], ((--nSettingsCount) - j) * sizeof(char*));
					--j; // reiterate current index
				}
			}

			// no longer registered?
			if (!IsRegisteredAssocItem(pszClassName)) {
				char *pszFileExt;
				if (IsFileClassName(pszClassName, &pszFileExt))
					RemoveRegFileExt(pszFileExt, pszClassName);
				else
					RemoveRegClass(pszClassName);
			}
		}
		mir_free(ppszSettings[i]);
	}
	mir_free(ppszSettings);
}

/************************* Class **********************************/

/*
* Add a new file class to the class list.
* This either represents a superclass for several file extensions or
* the the url object.
* Urls just need a class named after their prefix e.g. "http".
* File extensions should follow the rule "appname.extension".
*/

// pszIconLoc, pszVerbDesc and pszDdeCmd are allowed to be NULL
// call GetLastError() on error to get more error details
BOOL AddRegClass(const char *pszClassName, const wchar_t *pszTypeDescription, const wchar_t *pszIconLoc, const wchar_t *pszAppName, const wchar_t *pszRunCmd, const wchar_t *pszDdeCmd, const wchar_t *pszDdeApp, const wchar_t *pszDdeTopic, const wchar_t *pszVerbDesc, BOOL fBrowserAutoOpen, BOOL fUrlProto, BOOL fIsShortcut)
{
	LONG res;
	HKEY hRootKey, hClassKey, hShellKey, hVerbKey, hDdeKey;

	// some error checking for disallowed values (to avoid errors in registry)
	if (strchr(pszClassName, '\\') != nullptr || strchr(pszClassName, ' ') != nullptr) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	// try to open interactive user's classes key
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_CREATE_SUB_KEY, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT; // might be write protected by security settings

	// class
	if ((res = RegCreateKeyExA(hRootKey, pszClassName, 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY | DELETE | KEY_QUERY_VALUE, nullptr, &hClassKey, nullptr)) == ERROR_SUCCESS) {
		// backup class if shared
		if (fUrlProto) BackupRegTree(hRootKey, pszClassName, "bak_");
		// type description
		if (fUrlProto) SetRegStrPrefixValue(hClassKey, L"URL:", pszTypeDescription);
		else RegSetValueEx(hClassKey, nullptr, 0, REG_SZ, (uint8_t*)pszTypeDescription, (int)(mir_wstrlen(pszTypeDescription) + 1) * sizeof(wchar_t));
		// default icon
		if (pszIconLoc != nullptr) SetRegSubKeyStrDefValue(hClassKey, L"DefaultIcon", pszIconLoc);
		// url protocol
		if (!fUrlProto) RegDeleteValue(hClassKey, L"URL Protocol");
		else RegSetValueEx(hClassKey, L"URL Protocol", 0, REG_SZ, nullptr, 0);
		// moniker clsid
		RegDeleteKey(hClassKey, L"CLSID");
		// edit flags
		{
			DWORD dwFlags = 0, dwSize = sizeof(dwFlags);
			RegQueryValueEx(hClassKey, L"EditFlags", nullptr, nullptr, (uint8_t*)&dwFlags, &dwSize);
			if (fBrowserAutoOpen) dwFlags = (dwFlags & ~FTA_AlwaysUnsafe) | FTA_OpenIsSafe;
			if (!fUrlProto) dwFlags |= FTA_HasExtension;
			else dwFlags = (dwFlags & ~FTA_HasExtension) | FTA_Show; // show classes without extension
			RegSetValueEx(hClassKey, L"EditFlags", 0, REG_DWORD, (uint8_t*)&dwFlags, sizeof(dwFlags));
		}
		if (fIsShortcut) {
			RegSetValueExA(hClassKey, "IsShortcut", 0, REG_SZ, nullptr, 0);
			RegSetValueExA(hClassKey, "NeverShowExt", 0, REG_SZ, nullptr, 0);
		}
		// shell
		if ((res = RegCreateKeyEx(hClassKey, L"shell", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hShellKey, nullptr)) == ERROR_SUCCESS) {
			// default verb (when empty "open" is used)
			RegSetValueEx(hShellKey, nullptr, 0, REG_SZ, (uint8_t*)L"open", 5 * sizeof(wchar_t));
			// verb
			if ((res = RegCreateKeyEx(hShellKey, L"open", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY | DELETE, nullptr, &hVerbKey, nullptr)) == ERROR_SUCCESS) {
				// verb description
				if (pszVerbDesc == nullptr) RegDeleteValue(hVerbKey, nullptr);
				else RegSetValueEx(hVerbKey, nullptr, 0, REG_SZ, (uint8_t*)pszVerbDesc, (int)(mir_wstrlen(pszVerbDesc) + 1) * sizeof(wchar_t));
				// friendly appname (mui string)
				RegSetValueEx(hVerbKey, L"FriendlyAppName", 0, REG_SZ, (uint8_t*)pszAppName, (int)(mir_wstrlen(pszAppName) + 1) * sizeof(wchar_t));
				// command
				SetRegSubKeyStrDefValue(hVerbKey, L"command", pszRunCmd);
				// ddeexec
				if (pszDdeCmd != nullptr) {
					if (!RegCreateKeyEx(hVerbKey, L"ddeexec", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY | DELETE, nullptr, &hDdeKey, nullptr)) {
						// command
						RegSetValueEx(hDdeKey, nullptr, 0, REG_SZ, (uint8_t*)pszDdeCmd, (int)(mir_wstrlen(pszDdeCmd) + 1) * sizeof(wchar_t));
						// application
						SetRegSubKeyStrDefValue(hDdeKey, L"application", pszDdeApp);
						// topic
						SetRegSubKeyStrDefValue(hDdeKey, L"topic", pszDdeTopic);
						// ifexec
						RegDeleteKey(hDdeKey, L"ifexec");
						RegCloseKey(hDdeKey);
					}
				}
				else {
					if (!RegOpenKeyEx(hVerbKey, L"ddeexec", 0, DELETE, &hDdeKey)) {
						// application
						RegDeleteKey(hDdeKey, L"application");
						// topic
						RegDeleteKey(hDdeKey, L"topic");
						// ifexec
						RegDeleteKey(hDdeKey, L"ifexec");
						RegCloseKey(hDdeKey);
					}
					RegDeleteKey(hVerbKey, L"ddeexec");
				}
				// drop target (WinXP+)
				RegDeleteKey(hVerbKey, L"DropTarget");
				RegCloseKey(hVerbKey);
			}
			RegCloseKey(hShellKey);
		}
		RegCloseKey(hClassKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);

	if (res) SetLastError(res);
	return !res;
}

BOOL RemoveRegClass(const char *pszClassName)
{
	HKEY hRootKey, hClassKey, hShellKey, hVerbKey;
	wchar_t *ptszPrevRunCmd;

	if (pszClassName == nullptr)
		return ERROR_BAD_ARGUMENTS;

	// try to open interactive user's classes key
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, DELETE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// class name
	LONG res = DeleteRegSubTree(hRootKey, _A2T(pszClassName));

	// backup only saved/restored for fUrlProto
	if (!res) {
		if ((res = RestoreRegTree(hRootKey, pszClassName, "bak_")) == ERROR_SUCCESS)
			// class
			if (!RegOpenKeyExA(hRootKey, pszClassName, 0, KEY_QUERY_VALUE, &hClassKey)) {
				// shell
				if (!RegOpenKeyEx(hClassKey, L"shell", 0, KEY_QUERY_VALUE, &hShellKey)) {
					// verb
					if (!RegOpenKeyEx(hShellKey, L"open", 0, KEY_QUERY_VALUE, &hVerbKey)) {
						// command
						ptszPrevRunCmd = GetRegStrValue(hVerbKey, L"command");
						if (ptszPrevRunCmd != nullptr && !IsValidRunCommand(ptszPrevRunCmd))
							res = DeleteRegSubTree(hRootKey, _A2T(pszClassName)); // backup outdated, remove all
						mir_free(ptszPrevRunCmd); // does NULL check
						RegCloseKey(hVerbKey);
					}
					RegCloseKey(hShellKey);
				}
				RegCloseKey(hClassKey);
			}
	}
	else DeleteRegTreeBackup(pszClassName, "bak_");

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);

	if (res == ERROR_SUCCESS || res == ERROR_FILE_NOT_FOUND || res == ERROR_NO_MORE_ITEMS) return TRUE;
	SetLastError(res);
	return FALSE;
}

/*
* Test if a given class belongs to the current process
* specified via its run command.
* This is especially needed for Urls where the same class name "http" can be
* registered and thus be overwritten by multiple applications.
*/

BOOL IsRegClass(const char *pszClassName, const wchar_t *pszRunCmd)
{
	BOOL fSuccess = FALSE;
	HKEY hClassKey, hShellKey, hVerbKey, hCmdKey;

	// using the merged view classes key for reading
	// class
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT, pszClassName, 0, KEY_QUERY_VALUE, &hClassKey)) {
		// shell
		if (!RegOpenKeyEx(hClassKey, L"shell", 0, KEY_QUERY_VALUE, &hShellKey)) {
			// verb
			if (!RegOpenKeyEx(hShellKey, L"open", 0, KEY_QUERY_VALUE, &hVerbKey)) {
				// command
				if (!RegOpenKeyEx(hVerbKey, L"command", 0, KEY_QUERY_VALUE, &hCmdKey)) {
					// it is enough to check if the command is right
					fSuccess = IsRegStrValue(hCmdKey, nullptr, pszRunCmd);
					RegCloseKey(hCmdKey);
				}
				RegCloseKey(hVerbKey);
			}
			RegCloseKey(hShellKey);
		}
		RegCloseKey(hClassKey);
	}
	return fSuccess;
}

/*
* Extract the icon name of the class from the registry and load it.
* For uses especially with url classes.
*/

// DestroyIcon() the return value
HICON LoadRegClassSmallIcon(ASSOCDATA *assoc)
{
	HICON hIcon = LoadIcon(assoc->hInstance, MAKEINTRESOURCE(assoc->nIconResID));
	if (hIcon != nullptr)
		return hIcon;

	// using the merged view classes key for reading class
	HKEY hClassKey, hIconKey;
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT, assoc->pszClassName, 0, KEY_QUERY_VALUE, &hClassKey)) {
		// default icon
		if (!RegOpenKeyEx(hClassKey, L"DefaultIcon", 0, KEY_QUERY_VALUE, &hIconKey)) {
			// extract icon
			wchar_t *pszIconLoc = GetRegStrValue(hIconKey, nullptr);
			if (pszIconLoc != nullptr) {
				wchar_t *p = wcsrchr(pszIconLoc, ',');
				if (p != nullptr) {
					*(p++) = 0;
					ExtractIconEx(pszIconLoc, _wtoi(p), nullptr, &hIcon, 1);
				}
				mir_free(pszIconLoc);
			}
			RegCloseKey(hIconKey);
		}
		RegCloseKey(hClassKey);
	}

	return hIcon;
}

/************************* Extension ******************************/

/*
* Add a new file extension to the class list.
* The file extension needs to be associated with a class
* that has been registered previously.
* Multiple file extensions can be assigned to the same class.
* The class contains most settings as the run command etc.
*/

// pszMimeType is allowed to be NULL
BOOL AddRegFileExt(const char *pszFileExt, const char *pszClassName, const char *pszMimeType, BOOL fIsText)
{
	BOOL fSuccess = FALSE;

	// some error checking for disallowed values (to avoid errors in registry)
	if (strchr(pszFileExt, '\\') != nullptr || strchr(pszFileExt, ' ') != nullptr)
		return FALSE;

	// try to open interactive user's classes key
	HKEY hRootKey, hExtKey, hOpenWithKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_CREATE_SUB_KEY, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// file ext
	if (!RegCreateKeyExA(hRootKey, pszFileExt, 0, nullptr, 0, KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hExtKey, nullptr)) {
		// backup previous app
		BackupRegTree(hRootKey, pszFileExt, "bak_");
		// remove any no-open flag
		RegDeleteValue(hExtKey, L"NoOpen");
		// open with progids
		wchar_t *pszPrevClass = GetRegStrValue(hExtKey, nullptr);
		if (pszPrevClass != nullptr && !IsRegStrValueA(hExtKey, nullptr, pszClassName))
			if (!RegCreateKeyEx(hExtKey, L"OpenWithProgids", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &hOpenWithKey, nullptr)) {
				// previous class (backup)
				RegSetValueEx(hOpenWithKey, pszPrevClass, 0, REG_NONE, nullptr, 0);
				RegCloseKey(hOpenWithKey);
			}
		mir_free(pszPrevClass); // does NULL check
		// class name
		fSuccess = !RegSetValueExA(hExtKey, nullptr, 0, REG_SZ, (uint8_t*)pszClassName, (int)mir_strlen(pszClassName) + 1);
		// mime type e.g. "application/x-icq"
		if (pszMimeType != nullptr) RegSetValueExA(hExtKey, "Content Type", 0, REG_SZ, (uint8_t*)pszMimeType, (int)mir_strlen(pszMimeType) + 1);
		// perceived type e.g. text (WinXP+)
		if (fIsText) RegSetValueEx(hExtKey, L"PerceivedType", 0, REG_SZ, (uint8_t*)L"text", 5 * sizeof(wchar_t));
		RegCloseKey(hExtKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
	return fSuccess;
}

void RemoveRegFileExt(const char *pszFileExt, const char *pszClassName)
{
	BOOL fRestored = FALSE;

	// try to open interactive user's classes key
	HKEY hRootKey, hExtKey, hSubKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, DELETE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// file ext
	if (!RegOpenKeyExA(hRootKey, pszFileExt, 0, KEY_QUERY_VALUE | KEY_SET_VALUE | DELETE, &hExtKey)) {
		// class name (the important part)
		wchar_t *pszPrevClassName = nullptr;
		if (!RestoreRegTree(hRootKey, pszFileExt, "bak_")) {
			pszPrevClassName = GetRegStrValue(hExtKey, nullptr);
			if (pszPrevClassName != nullptr) {
				// previous class name still exists?
				// using the merged view classes key for reading
				if (!RegOpenKeyEx(HKEY_CLASSES_ROOT, pszPrevClassName, 0, KEY_QUERY_VALUE, &hSubKey)) {
					fRestored = TRUE;
					RegCloseKey(hSubKey);
				}
				else RegDeleteValue(hExtKey, nullptr);
				mir_free(pszPrevClassName);
			}
		}

		if (pszPrevClassName == nullptr)
			RegDeleteValue(hExtKey, nullptr);

		// open with progids (remove if empty)
		DWORD nOpenWithCount = 0;
		if (!RegOpenKeyEx(hExtKey, L"OpenWithProgids", 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hSubKey)) {
			// remove current class (if set by another app)
			RegDeleteValueA(hSubKey, pszClassName);
			RegQueryInfoKey(hSubKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &nOpenWithCount, nullptr, nullptr, nullptr);
			RegCloseKey(hSubKey);
		}
		if (!nOpenWithCount)
			RegDeleteKey(hExtKey, L"OpenWithProgids"); // delete if no values
		RegCloseKey(hExtKey);
	}
	else DeleteRegTreeBackup(pszFileExt, "bak_");
	if (!fRestored)
		RegDeleteKeyA(hRootKey, pszFileExt); // try to remove it all

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/*
* Test if a given file extension belongs to the given class name.
* If it does not belong to the class name, it got reassigned and thus
* overwritten by another application.
*/

BOOL IsRegFileExt(const char *pszFileExt, const char *pszClassName)
{
	BOOL fSuccess = FALSE;

	// using the merged view classes key for reading file ext
	HKEY hExtKey;
	if (!RegOpenKeyExA(HKEY_CLASSES_ROOT, pszFileExt, 0, KEY_QUERY_VALUE, &hExtKey)) {
		// class name
		// it is enough to check if the class is right
		fSuccess = IsRegStrValueA(hExtKey, nullptr, pszClassName);
		RegCloseKey(hExtKey);
	}
	return fSuccess;
}

/************************* Mime Type ******************************/

/*
* Add a given mime type to the global mime database.
*/

// returns TRUE if the mime type was not yet registered on the system,
// it needs to be removed when the file extension gets removed
BOOL AddRegMimeType(const char *pszMimeType, const char *pszFileExt)
{
	BOOL fSuccess = FALSE;

	// some error checking for disallowed values (to avoid errors in registry)
	HKEY hRootKey, hDbKey, hTypeKey;
	if (strchr(pszMimeType, '\\') != nullptr || strchr(pszMimeType, ' ') != nullptr)
		return FALSE;

	// try to open interactive user's classes key
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// database
	if (!RegOpenKeyEx(hRootKey, L"MIME\\Database\\Content Type", 0, KEY_CREATE_SUB_KEY, &hDbKey)) {
		// mime type
		if (!RegCreateKeyExA(hDbKey, pszMimeType, 0, nullptr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &hTypeKey, nullptr)) {
			// file ext
			if (RegQueryValueExA(hTypeKey, "Extension", nullptr, nullptr, nullptr, nullptr)) // only set if not present
				fSuccess = !RegSetValueExA(hTypeKey, "Extension", 0, REG_SZ, (uint8_t*)pszFileExt, (int)mir_strlen(pszFileExt) + 1);
			RegCloseKey(hTypeKey);
		}
		RegCloseKey(hDbKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
	return fSuccess;
}

void RemoveRegMimeType(const char *pszMimeType, const char *pszFileExt)
{
	// try to open interactive user's classes key
	HKEY hRootKey, hDbKey, hTypeKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// database
	if (!RegOpenKeyEx(hRootKey, L"MIME\\Database\\Content Type", 0, DELETE, &hDbKey)) {
		BOOL fDelete = TRUE;

		// mime type
		if (!RegOpenKeyExA(hDbKey, pszMimeType, 0, KEY_QUERY_VALUE, &hTypeKey)) {
			// file ext
			fDelete = IsRegStrValueA(hTypeKey, L"Extension", pszFileExt);
			RegCloseKey(hTypeKey);
		}
		if (fDelete)
			RegDeleteKeyA(hDbKey, pszMimeType);
		RegCloseKey(hDbKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/************************* Open-With App **************************/

/*
* Add Miranda as an option to the advanced "Open With..." dialog.
*/

// pszDdeCmd is allowed to be NULL
void AddRegOpenWith(const wchar_t *pszAppFileName, BOOL fAllowOpenWith, const wchar_t *pszAppName, const wchar_t *pszIconLoc, const wchar_t *pszRunCmd, const wchar_t *pszDdeCmd, const wchar_t *pszDdeApp, const wchar_t *pszDdeTopic)
{
	// try to open interactive user's classes key
	HKEY hRootKey, hAppsKey, hExeKey, hShellKey, hVerbKey, hDdeKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// database
	if (!RegCreateKeyEx(hRootKey, L"Applications", 0, nullptr, 0, KEY_CREATE_SUB_KEY, nullptr, &hAppsKey, nullptr)) {
		// filename
		if (!RegCreateKeyEx(hAppsKey, pszAppFileName, 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hExeKey, nullptr)) {
			// appname
			RegSetValueEx(hExeKey, nullptr, 0, REG_SZ, (uint8_t*)pszAppName, (int)(mir_wstrlen(pszAppName) + 1) * sizeof(wchar_t));
			// no open-with flag
			if (fAllowOpenWith) RegDeleteValue(hExeKey, L"NoOpenWith");
			else RegSetValueEx(hExeKey, L"NoOpenWith", 0, REG_SZ, nullptr, 0);
			// default icon
			if (pszIconLoc != nullptr) SetRegSubKeyStrDefValue(hExeKey, L"DefaultIcon", pszIconLoc);
			// shell
			if (!RegCreateKeyEx(hExeKey, L"shell", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hShellKey, nullptr)) {
				// default verb (when empty "open" is used)
				RegSetValueEx(hShellKey, nullptr, 0, REG_SZ, (uint8_t*)L"open", 5 * sizeof(wchar_t));
				// verb
				if (!RegCreateKeyEx(hShellKey, L"open", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hVerbKey, nullptr)) {
					// friendly appname (mui string)
					RegSetValueEx(hVerbKey, L"FriendlyAppName", 0, REG_SZ, (uint8_t*)pszAppName, (int)(mir_wstrlen(pszAppName) + 1) * sizeof(wchar_t));
					// command
					SetRegSubKeyStrDefValue(hVerbKey, L"command", pszRunCmd);
					// ddeexec
					if (pszDdeCmd != nullptr)
						if (!RegCreateKeyEx(hVerbKey, L"ddeexec", 0, nullptr, 0, KEY_SET_VALUE | KEY_CREATE_SUB_KEY, nullptr, &hDdeKey, nullptr)) {
							// command
							RegSetValueEx(hDdeKey, nullptr, 0, REG_SZ, (uint8_t*)pszDdeCmd, (int)(mir_wstrlen(pszDdeCmd) + 1) * sizeof(wchar_t));
							// application
							SetRegSubKeyStrDefValue(hDdeKey, L"application", pszDdeApp);
							// topic
							SetRegSubKeyStrDefValue(hDdeKey, L"topic", pszDdeTopic);
							RegCloseKey(hDdeKey);
						}
					RegCloseKey(hVerbKey);
				}
				RegCloseKey(hShellKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

void RemoveRegOpenWith(const wchar_t *pszAppFileName)
{
	// try to open interactive user's classes key
	HKEY hRootKey, hAppsKey, hExeKey, hShellKey, hVerbKey, hDdeKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// applications
	if (!RegOpenKeyEx(hRootKey, L"Applications", 0, DELETE, &hAppsKey)) {
		// filename
		if (!RegOpenKeyEx(hAppsKey, pszAppFileName, 0, DELETE, &hExeKey)) {
			// default icon
			RegDeleteKey(hExeKey, L"DefaultIcon");
			// shell
			if (!RegOpenKeyEx(hExeKey, L"shell", 0, DELETE, &hShellKey)) {
				// verb
				if (!RegOpenKeyEx(hShellKey, L"open", 0, DELETE, &hVerbKey)) {
					// command
					RegDeleteKey(hVerbKey, L"command");
					// ddeexec
					if (!RegOpenKeyEx(hVerbKey, L"ddeexec", 0, DELETE, &hDdeKey)) {
						// application
						RegDeleteKey(hDdeKey, L"application");
						// topic
						RegDeleteKey(hDdeKey, L"topic");
						RegCloseKey(hDdeKey);
					}
					RegDeleteKey(hVerbKey, L"ddeexec");
					RegCloseKey(hVerbKey);
				}
				RegDeleteKey(hShellKey, L"open");
				RegCloseKey(hShellKey);
			}
			RegDeleteKey(hExeKey, L"shell");
			// supported types
			RegDeleteKey(hExeKey, L"SupportedTypes");
			RegCloseKey(hExeKey);
		}
		RegDeleteKey(hAppsKey, pszAppFileName);
		RegCloseKey(hAppsKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/*
* Tell the "Open With..." dialog we support a given file extension.
*/

void AddRegOpenWithExtEntry(const wchar_t *pszAppFileName, const char *pszFileExt, const wchar_t *pszFileDesc)
{
	// try to open interactive user's classes key
	HKEY hRootKey, hAppsKey, hExeKey, hTypesKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// applications
	if (!RegOpenKeyEx(hRootKey, L"Applications", 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hAppsKey)) {
		// filename
		if (!RegOpenKeyEx(hAppsKey, pszAppFileName, 0, KEY_CREATE_SUB_KEY, &hExeKey)) {
			// supported types
			if (!RegCreateKeyEx(hExeKey, L"SupportedTypes", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &hTypesKey, nullptr)) {
				ptrW ptszFileExt(mir_a2u(pszFileExt));
				if (ptszFileExt != nullptr)
					RegSetValueEx(hTypesKey, ptszFileExt, 0, REG_SZ, (uint8_t*)ptszFileExt.get(), (int)(mir_wstrlen(pszFileDesc) + 1) * sizeof(wchar_t));
				mir_free(ptszFileExt); // does NULL check
				RegCloseKey(hTypesKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

void RemoveRegOpenWithExtEntry(const wchar_t *pszAppFileName, const char *pszFileExt)
{
	// try to open interactive user's classes key
	HKEY hRootKey, hAppsKey, hExeKey, hTypesKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Classes", 0, KEY_QUERY_VALUE, &hRootKey))
		hRootKey = HKEY_CLASSES_ROOT;

	// applications
	if (!RegOpenKeyEx(hRootKey, L"Applications", 0, KEY_QUERY_VALUE, &hAppsKey)) {
		// filename
		if (!RegOpenKeyEx(hAppsKey, pszAppFileName, 0, KEY_QUERY_VALUE, &hExeKey)) {
			// supported types
			if (!RegOpenKeyEx(hExeKey, L"SupportedTypes", 0, KEY_SET_VALUE, &hTypesKey)) {
				RegDeleteValueA(hTypesKey, pszFileExt);
				RegCloseKey(hTypesKey);
			}
			RegCloseKey(hExeKey);
		}
		RegCloseKey(hAppsKey);
	}

	if (hRootKey != HKEY_CLASSES_ROOT)
		RegCloseKey(hRootKey);
}

/************************* Autostart ******************************/

/*
* Add Miranda to the autostart list in the registry.
*/

BOOL AddRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd)
{
	BOOL fSuccess = FALSE;

	// run
	HKEY hRunKey;
	if (!RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, nullptr, 0, KEY_SET_VALUE, nullptr, &hRunKey, nullptr)) {
		// appname
		fSuccess = !RegSetValueEx(hRunKey, pszAppName, 0, REG_SZ, (uint8_t*)pszRunCmd, (int)(mir_wstrlen(pszRunCmd) + 1) * sizeof(wchar_t));
		RegCloseKey(hRunKey);
	}
	return fSuccess;
}

BOOL RemoveRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd)
{
	// run
	HKEY hRunKey;
	LONG res = RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hRunKey);
	if (!res) {
		// appname
		if (IsRegStrValue(hRunKey, pszAppName, pszRunCmd))
			res = RegDeleteValue(hRunKey, pszAppName); // only remove if current
		RegCloseKey(hRunKey);
	}
	return res == ERROR_SUCCESS || res == ERROR_FILE_NOT_FOUND;
}

/*
* Check if the autostart item belongs to the current instance of Miranda.
*/

BOOL IsRegRunEntry(const wchar_t *pszAppName, const wchar_t *pszRunCmd)
{
	BOOL fState = FALSE;

	// Run
	HKEY hRunKey;
	if (!RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_QUERY_VALUE, &hRunKey)) {
		// appname
		fState = IsRegStrValue(hRunKey, pszAppName, pszRunCmd);
		RegCloseKey(hRunKey);
	}
	return fState;
}
