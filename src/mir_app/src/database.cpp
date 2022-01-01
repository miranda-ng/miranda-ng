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
#include "encrypt.h"
#include "profilemanager.h"

bool g_bDbCreated;
wchar_t g_profileDir[MAX_PATH], g_profileName[MAX_PATH], g_shortProfileName[MAX_PATH];
wchar_t* g_defaultProfile;

static bool fileExist(const wchar_t *fname)
{
	if (*fname == 0)
		return false;

	FILE *fp = _wfopen(fname, L"r+");
	bool res = (fp != nullptr);
	if (fp) fclose(fp);
	return res;
}

static void fillProfileName(const wchar_t* ptszFileName)
{
	const wchar_t* p = wcsrchr(ptszFileName, '\\');
	if (p == nullptr)
		p = ptszFileName;
	else
		p++;

	wcsncpy_s(g_profileName, p, _TRUNCATE);

	wcsncpy_s(g_shortProfileName, p, _TRUNCATE);
	wchar_t *pos = wcsrchr(g_shortProfileName, '.');
	if (mir_wstrcmpi(pos, L".dat") == 0)
		*pos = 0;
}

bool IsInsideRootDir(wchar_t* profiledir, bool exact)
{
	VARSW pfd(L"%miranda_path%");
	if (exact)
		return mir_wstrcmpi(profiledir, pfd) == 0;

	return wcsnicmp(profiledir, pfd, mir_wstrlen(pfd)) == 0;
}

// returns 1 if the profile path was returned, without trailing slash
int getProfilePath(wchar_t *buf, size_t)
{
	wchar_t profiledir[MAX_PATH];
	Profile_GetSetting(L"Database/ProfileDir", profiledir);

	if (profiledir[0] == 0)
		mir_wstrcpy(profiledir, L"%miranda_path%\\Profiles");

	size_t len = PathToAbsoluteW(VARSW(profiledir), buf);

	if (buf[len - 1] == '/' || buf[len - 1] == '\\')
		buf[len - 1] = 0;

	return 0;
}

// returns 1 if *.dat spec is matched
int isValidProfileName(const wchar_t *name)
{
	size_t len = mir_wstrlen(name) - 4;
	return len > 0 && mir_wstrcmpi(&name[len], L".dat") == 0;
}

// returns 1 if the profile manager should be shown
static bool showProfileManager(void)
{
	// is control pressed?
	#ifdef _WINDOWS
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		return 1;
	#endif

	// wanna show it?
	wchar_t Mgr[32];
	Profile_GetSetting(L"Database/ShowProfileMgr", Mgr, L"never");
	return (mir_wstrcmpi(Mgr, L"yes") == 0);
}

bool shouldAutoCreate(wchar_t *szProfile)
{
	if (szProfile[0] == 0)
		return false;

	wchar_t ac[32];
	Profile_GetSetting(L"Database/AutoCreate", ac);
	return mir_wstrcmpi(ac, L"yes") == 0;
}

static void getDefaultProfile(wchar_t *szProfile, size_t cch)
{
	wchar_t defaultProfile[MAX_PATH];
	Profile_GetSetting(L"Database/DefaultProfile", defaultProfile);

	if (defaultProfile[0] == 0)
		return;

	VARSW res(defaultProfile);
	if (res)
		mir_snwprintf(szProfile, cch, L"%s\\%s\\%s%s", g_profileDir, (wchar_t*)res, (wchar_t*)res, isValidProfileName(res) ? L"" : L".dat");
	else
		szProfile[0] = 0;
}

// returns 1 if something that looks like a profile is there
static void loadProfileByShortName(const wchar_t* src, wchar_t *szProfile, size_t cch)
{
	wchar_t buf[MAX_PATH];
	wcsncpy_s(buf, src, _TRUNCATE);

	wchar_t *p = wcsrchr(buf, '\\'); if (p) ++p; else p = buf;
	if (!isValidProfileName(buf) && *p)
		mir_wstrcat(buf, L".dat");

	wchar_t profileName[MAX_PATH], newProfileDir[MAX_PATH];
	wcsncpy_s(profileName, p, _TRUNCATE);
	if (!isValidProfileName(profileName) && *p)
		mir_wstrcat(profileName, L".dat");

	wcsncpy_s(profileName, p, _TRUNCATE);
	p = wcsrchr(profileName, '.'); if (p) *p = 0;

	mir_snwprintf(newProfileDir, cch, L"%s\\%s\\", g_profileDir, profileName);
	PathToAbsoluteW(buf, szProfile, newProfileDir);

	if (wcschr(buf, '\\')) {
		wcsncpy_s(g_profileDir, szProfile, _TRUNCATE);
		if (profileName[0]) {
			p = wcsrchr(g_profileDir, '\\'); *p = 0;
			p = wcsrchr(g_profileDir, '\\');
			if (p && mir_wstrcmpi(p + 1, profileName) == 0)
				*p = 0;
		}
		else szProfile[0] = 0;
	}
}

void getProfileCmdLine(wchar_t *szProfile, size_t cch)
{
	LPCTSTR ptszProfileName = CmdLine_GetOption(L"profile");
	if (ptszProfileName != nullptr)
		loadProfileByShortName(ptszProfileName, szProfile, cch);
}

void getProfileDefault(wchar_t *szProfile, size_t cch)
{
	if (g_defaultProfile != nullptr) {
		loadProfileByShortName(g_defaultProfile, szProfile, cch);
		mir_free(g_defaultProfile);
	}
}

// move profile from profile subdir
static void moveProfileDirProfiles(wchar_t *profiledir, BOOL isRootDir = TRUE)
{
	wchar_t pfd[MAX_PATH];
	if (isRootDir)
		wcsncpy_s(pfd, VARSW(L"%miranda_path%\\*.dat"), _TRUNCATE);
	else
		mir_snwprintf(pfd, L"%s\\*.dat", profiledir);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(pfd, &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		wchar_t *c = wcsrchr(pfd, '\\'); if (c) *c = 0;
		do {
			wchar_t path[MAX_PATH], path2[MAX_PATH];
			wchar_t* profile = mir_wstrdup(ffd.cFileName);
			c = wcsrchr(profile, '.'); if (c) *c = 0;
			mir_snwprintf(path, L"%s\\%s", pfd, ffd.cFileName);
			mir_snwprintf(path2, L"%s\\%s", profiledir, profile);
			CreateDirectoryTreeW(path2);
			mir_snwprintf(path2, L"%s\\%s\\%s", profiledir, profile, ffd.cFileName);
			if (_waccess(path2, 0) == 0) {
				wchar_t buf[512];
				mir_snwprintf(buf,
								  TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s\nBecause profile with this name already exists. Please resolve the issue manually."),
								  path, path2);
				MessageBox(nullptr, buf, L"Miranda NG", MB_ICONERROR | MB_OK);
			}
			else if (MoveFile(path, path2) == 0) {
				wchar_t buf[512];
				mir_snwprintf(buf,
								  TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s automatically\nMost likely this is due to insufficient privileges. Please move profile manually."),
								  path, path2);
				MessageBox(nullptr, buf, L"Miranda NG", MB_ICONERROR | MB_OK);
				mir_free(profile);
				break;
			}
			mir_free(profile);
		} while (FindNextFile(hFind, &ffd));
	}
	FindClose(hFind);
}

// returns 1 if a single profile (full path) is found within the profile dir
static int getProfile1(wchar_t *szProfile, size_t cch, wchar_t *profiledir, BOOL * noProfiles)
{
	int found = 0;

	if (IsInsideRootDir(profiledir, false))
		moveProfileDirProfiles(profiledir);
	moveProfileDirProfiles(profiledir, FALSE);

	bool bNoDefaultProfile = (*szProfile == 0);
	bool reqfd = !bNoDefaultProfile && (_waccess(szProfile, 0) == 0 || shouldAutoCreate(szProfile));
	bool bShowProfileManager = showProfileManager();

	if (reqfd)
		found++;

	if (bShowProfileManager || !reqfd) {
		wchar_t searchspec[MAX_PATH];
		mir_snwprintf(searchspec, L"%s\\*.*", profiledir);

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchspec, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// make sure the first hit is actually a *.dat file
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !mir_wstrcmp(ffd.cFileName, L".") || !mir_wstrcmp(ffd.cFileName, L".."))
					continue;

				wchar_t newProfile[MAX_PATH];
				mir_snwprintf(newProfile, L"%s\\%s\\%s.dat", profiledir, ffd.cFileName, ffd.cFileName);
				if (_waccess(newProfile, 0) != 0)
					continue;

				switch (touchDatabase(newProfile, nullptr)) {
				case EGROKPRF_CANTREAD:
					// profile might be locked by another Miranda
					if (!Profile_CheckOpened(newProfile))
						break;
					__fallthrough;

				case 0:
				case EGROKPRF_OBSOLETE:
					if (++found == 1 && bNoDefaultProfile)
						wcsncpy_s(szProfile, cch, newProfile, _TRUNCATE);
					break;
				}
			} while (FindNextFile(hFind, &ffd));

			FindClose(hFind);
		}
		reqfd = (!bShowProfileManager && found == 1 && bNoDefaultProfile);
	}

	if (noProfiles)
		*noProfiles = (found == 0);

	if (bNoDefaultProfile && !reqfd)
		szProfile[0] = 0;

	return reqfd;
}

// returns 1 if a default profile should be selected instead of showing the manager.
static int getProfileAutoRun(wchar_t *szProfile)
{
	if (*szProfile == 0)
		return false;

	wchar_t Mgr[32];
	Profile_GetSetting(L"Database/ShowProfileMgr", Mgr);
	if (mir_wstrcmpi(Mgr, L"never"))
		return 0;

	return fileExist(szProfile) || shouldAutoCreate(szProfile);
}

// returns 1 if a profile was selected
static int getProfile(wchar_t *szProfile, size_t cch)
{
	getProfilePath(g_profileDir, _countof(g_profileDir));
	if (IsInsideRootDir(g_profileDir, true))
		if (WritePrivateProfileString(L"Database", L"ProfileDir", L"", mirandabootini))
			getProfilePath(g_profileDir, _countof(g_profileDir));

	getDefaultProfile(szProfile, cch);
	getProfileCmdLine(szProfile, cch);
	getProfileDefault(szProfile, cch);

	if (IsInsideRootDir(g_profileDir, true)) {
		MessageBox(nullptr,
					  TranslateT("Profile cannot be placed into Miranda root folder.\nPlease move Miranda profile to some other location."),
					  LPGENW("Miranda NG"), MB_ICONERROR | MB_OK);
		return 0;
	}

	PROFILEMANAGERDATA pd = {};
	if (CmdLine_GetOption(L"ForceShowPM")) {
LBL_Show:
		pd.ptszProfile = szProfile;
		pd.ptszProfileDir = g_profileDir;
		if (!getProfileManager(&pd))
			return 0;

		return 1;
	}

	if (getProfileAutoRun(szProfile))
		return 1;

	if (getProfile1(szProfile, cch, g_profileDir, &pd.noProfiles))
		return 1;

	goto LBL_Show;
}

// carefully converts a file name from wchar_t* to char*
char* makeFileName(const wchar_t* tszOriginalName)
{
	char *szResult = nullptr;
	char *szFileName = mir_u2a(tszOriginalName);
	wchar_t *tszFileName = mir_a2u(szFileName);
	if (mir_wstrcmp(tszOriginalName, tszFileName)) {
		wchar_t tszProfile[MAX_PATH];
		if (GetShortPathName(tszOriginalName, tszProfile, MAX_PATH) != 0)
			szResult = mir_u2a(tszProfile);
	}

	if (!szResult)
		szResult = szFileName;
	else
		mir_free(szFileName);
	mir_free(tszFileName);

	return szResult;
}

int touchDatabase(const wchar_t *tszProfile, DATABASELINK **dblink)
{
	for (auto &it : arDbPlugins) {
		int iErrorCode = it->grokHeader(tszProfile);
		if (iErrorCode == 0) {
			if (dblink)
				*dblink = it;
			return 0;
		}
		if (iErrorCode == EGROKPRF_OBSOLETE) {
			if (dblink)
				*dblink = it;
			return EGROKPRF_OBSOLETE;
		}
	}

	if (dblink)
		*dblink = nullptr;
	return EGROKPRF_CANTREAD;
}

// enumerate all database plugins
int tryOpenDatabase(const wchar_t *tszProfile)
{
	for (auto &it : arDbPlugins) {
		// liked the profile?
		int err = it->grokHeader(tszProfile);
		if (err != ERROR_SUCCESS) { // smth went wrong
			switch (err) {
			case EGROKPRF_CANTREAD:
			case EGROKPRF_UNKHEADER:
				// just not supported.
				continue;

			case EGROKPRF_OBSOLETE:
				break;

			default:
				return err;
			}
		}

		// try to load database
		MDatabaseCommon *pDb = it->Load(tszProfile, FALSE);
		if (pDb == nullptr)
			return EGROKPRF_CANTREAD;

		fillProfileName(tszProfile);
		db_setCurrent(pDb);
		return 0;
	}

	return -1; // no suitable driver found
}

// enumerate all database plugins
static int tryCreateDatabase(const wchar_t *ptszProfile)
{
	CreatePathToFileW(ptszProfile);

	for (auto &p : arDbPlugins) {
		if (!(p->capabilities & MDB_CAPS_CREATE))
			continue;

		int err = p->makeDatabase(ptszProfile);
		if (err == ERROR_SUCCESS) {
			g_bDbCreated = true;
			MDatabaseCommon *pDb = p->Load(ptszProfile, FALSE);
			if (pDb == nullptr) // driver was found but smth went wrong
				return EGROKPRF_CANTREAD;

			fillProfileName(ptszProfile);
			db_setCurrent(pDb);
			return 0;
		}
	}
	return -1; // no suitable driver found
}

/////////////////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK EnumMirandaWindows(HWND hwnd, LPARAM lParam)
{
	wchar_t classname[256];
	if (GetClassName(hwnd, classname, _countof(classname)) && mir_wstrcmp(L"Miranda", classname) == 0) {
		DWORD_PTR res = 0;
		if (SendMessageTimeout(hwnd, uMsgProcessProfile, lParam, 0, SMTO_ABORTIFHUNG, 100, &res) && res)
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Default menu items

static INT_PTR CompactMe(void* obj, WPARAM, LPARAM)
{
	auto *db = (MDatabaseCommon *)obj;
	if (!db->Compact())
		MessageBox(0, TranslateT("Database was compacted successfully"), TranslateT("Database"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(0, TranslateT("Database compaction failed"), TranslateT("Database"), MB_OK | MB_ICONERROR);
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	auto *pDb = db_get_current();

	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000, g_plugin.getIconHandle(IDI_DATABASE));
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "F7C5567C-D1EE-484B-B4F6-24677A5AAAEF");

	if (pDb->GetDriver()->capabilities & MDB_CAPS_COMPACT) {
		SET_UID(mi, 0x98c0caf3, 0xBfe5, 0x4e31, 0xac, 0xf0, 0xab, 0x95, 0xb2, 0x9b, 0x9f, 0x73);
		mi.position++;
		mi.hIcolibItem = g_plugin.getIconHandle(IDI_DATABASE);
		mi.name.a = LPGEN("Compact");
		mi.pszService = "DB/UI/Compact";
		Menu_AddMainMenuItem(&mi);

		CreateServiceFunctionObj(mi.pszService, CompactMe, pDb);
	}

	InitCryptMenuItem(mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t tszNoDrivers[] = LPGENW("Miranda is unable to open '%s' because you do not have any profile plugins installed.\nYou need to install dbx_mdbx.dll");
static wchar_t tszUnknownFormat[] = LPGENW("Miranda was unable to open '%s', it's in an unknown format.");
static wchar_t tszProfileLocked[] = LPGENW("Miranda was unable to open '%s'\nIt's inaccessible or used by other application or Miranda instance");
static wchar_t tszNoSuitableDriver[] = LPGENW("Miranda was unable to open '%s'\nThere is no suitable database driver installed");

int LoadDatabaseModule(void)
{
	wchar_t szProfile[MAX_PATH];
	PathToAbsoluteW(L".", szProfile);
	_wchdir(szProfile);
	szProfile[0] = 0;

	// find out which profile to load
	if (!getProfile(szProfile, _countof(szProfile)))
		return 1;

	wchar_t *ptszFileName = wcsrchr(szProfile, '\\');
	ptszFileName = (ptszFileName) ? ptszFileName + 1 : szProfile;

	if (arDbPlugins.getCount() == 0) {
		MessageBox(nullptr, CMStringW(FORMAT, TranslateW(tszNoDrivers), ptszFileName), TranslateT("No profile support installed!"), MB_OK | MB_ICONERROR);
		return 1;
	}

	// if this profile is already opened in another miranda, silently return
	if (Profile_CheckOpened(szProfile)) {
		uMsgProcessProfile = RegisterWindowMessage(L"Miranda::ProcessProfile");
		ATOM aPath = GlobalAddAtom(szProfile);
		EnumWindows(EnumMirandaWindows, (LPARAM)aPath);
		GlobalDeleteAtom(aPath);
		return 1;
	}

	// find a driver to support the given profile
	bool retry;
	int rc;
	do {
		retry = false;
		if (_waccess(szProfile, 0) && shouldAutoCreate(szProfile))
			rc = tryCreateDatabase(szProfile);
		else
			rc = tryOpenDatabase(szProfile);

		// there were no suitable driver installed
		if (rc == -1) {
			MessageBox(nullptr,
				CMStringW(FORMAT, TranslateW(tszNoSuitableDriver), ptszFileName),
				TranslateT("Miranda can't open that profile"), MB_OK | MB_ICONERROR);
		}
		else if (rc > 0) {
			if (fileExist(szProfile)) {
				// file isn't locked, just no driver could open it.
				MessageBox(nullptr,
					CMStringW(FORMAT, TranslateW(tszUnknownFormat), ptszFileName),
					TranslateT("Miranda can't understand that profile"), MB_OK | MB_ICONERROR);
			}
			else
				retry = IDRETRY == MessageBox(nullptr,
					CMStringW(FORMAT, TranslateW(tszProfileLocked), ptszFileName),
					TranslateT("Miranda can't open that profile"), MB_RETRYCANCEL | MB_ICONERROR);
		}
	}
		while (retry);

	if (rc == 0)
		HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return rc;
}
