/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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

bool shouldAutoCreate(const MFilePath &szProfile)
{
	if (szProfile.IsEmpty())
		return false;

	wchar_t ac[32];
	Profile_GetSetting(L"Database/AutoCreate", ac);
	return mir_wstrcmpi(ac, L"yes") == 0;
}

static void getDefaultProfile(MFilePath &szProfile)
{
	wchar_t defaultProfile[MAX_PATH];
	Profile_GetSetting(L"Database/DefaultProfile", defaultProfile);

	if (defaultProfile[0] == 0)
		return;

	VARSW res(defaultProfile);
	if (res)
		szProfile.Format(L"%s\\%s\\%s%s", g_profileDir, (wchar_t*)res, (wchar_t*)res, isValidProfileName(res) ? L"" : L".dat");
	else
		szProfile.Empty();
}

// returns 1 if something that looks like a profile is there
static void loadProfileByShortName(const wchar_t *src, MFilePath &szProfile)
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

	mir_snwprintf(newProfileDir, MAX_PATH, L"%s\\%s\\", g_profileDir, profileName);
	szProfile.Truncate(MAX_PATH);
	PathToAbsoluteW(buf, szProfile.GetBuffer(), newProfileDir);

	if (wcschr(buf, '\\')) {
		wcsncpy_s(g_profileDir, szProfile, _TRUNCATE);
		if (profileName[0]) {
			p = wcsrchr(g_profileDir, '\\'); *p = 0;
			p = wcsrchr(g_profileDir, '\\');
			if (p && mir_wstrcmpi(p + 1, profileName) == 0)
				*p = 0;
		}
		else szProfile.Empty();
	}
}

void getProfileCmdLine(MFilePath &szProfile)
{
	LPCTSTR ptszProfileName = CmdLine_GetOption(L"profile");
	if (ptszProfileName != nullptr)
		loadProfileByShortName(ptszProfileName, szProfile);
}

void getProfileDefault(MFilePath &szProfile)
{
	if (g_defaultProfile != nullptr) {
		loadProfileByShortName(g_defaultProfile, szProfile);
		mir_free(g_defaultProfile);
	}
}

// move profile from profile subdir
static void moveProfileDirProfiles(const wchar_t *profiledir, bool isRootDir)
{
	MFilePath pfd, path, path2;
	if (isRootDir)
		pfd = VARSW(L"%miranda_path%\\*.dat").get();
	else
		pfd.Format(L"%s\\*.dat", profiledir);

	for (auto &it: pfd.search()) {
		if (it.isDir())
			continue;

		int idx = pfd.ReverseFind('\\');
		if (idx != -1)
			pfd.Trim(idx);

		auto *wszFileName = NEWWSTR_ALLOCA(it.getPath());
		auto *c = wcsrchr(wszFileName, '.'); if (c) *c = 0;

		path.Format(L"%s\\%s", pfd.c_str(), wszFileName);
		path2.Format(L"%s\\%s", profiledir, wszFileName);
		CreateDirectoryTreeW(path2);
		
		path2.AppendFormat(L"\\%s.dat", wszFileName);
		if (path2.isExist()) {
			wchar_t buf[512];
			mir_snwprintf(buf,
				TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s\nBecause profile with this name already exists. Please resolve the issue manually."),
				path.c_str(), path2.c_str());
			MessageBoxW(nullptr, buf, L"Miranda NG", MB_ICONERROR | MB_OK);
		}
		else if (!path.move(path2)) {
			wchar_t buf[512];
			mir_snwprintf(buf,
				TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s automatically\nMost likely this is due to insufficient privileges. Please move profile manually."),
				path.c_str(), path2.c_str());
			MessageBoxW(nullptr, buf, L"Miranda NG", MB_ICONERROR | MB_OK);
			break;
		}
	}
}

// returns 1 if a single profile (full path) is found within the profile dir
static int getProfile1(MFilePath &szProfile, wchar_t *profiledir, bool *noProfiles)
{
	int found = 0;

	if (IsInsideRootDir(profiledir, false))
		moveProfileDirProfiles(profiledir, true);
	moveProfileDirProfiles(profiledir, false);

	bool bNoDefaultProfile = (*szProfile == 0);
	bool reqfd = !bNoDefaultProfile && (szProfile.isExist() || shouldAutoCreate(szProfile));
	bool bShowProfileManager = showProfileManager();

	if (reqfd)
		found++;

	if (bShowProfileManager || !reqfd) {
		MFilePath searchspec;
		searchspec.Format(L"%s\\*.*", profiledir);

		for (auto &it: searchspec.search()) {
			// make sure the first hit is actually a *.dat file
			if (!it.isDir() || !wcscmp(it.getPath(), L".") || !wcscmp(it.getPath(), L".."))
				continue;

			MFilePath newProfile;
			newProfile.Format(L"%s\\%s\\%s.dat", profiledir, it.getPath(), it.getPath());
			if (!newProfile.isExist())
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
					szProfile = newProfile;
				break;
			}
		}
		reqfd = (!bShowProfileManager && found == 1 && bNoDefaultProfile);
	}

	if (noProfiles)
		*noProfiles = (found == 0);

	if (bNoDefaultProfile && !reqfd)
		szProfile.Empty();

	return reqfd;
}

// returns 1 if a default profile should be selected instead of showing the manager.
static int getProfileAutoRun(MFilePath &szProfile)
{
	if (szProfile.IsEmpty())
		return false;

	wchar_t Mgr[32];
	Profile_GetSetting(L"Database/ShowProfileMgr", Mgr);
	if (mir_wstrcmpi(Mgr, L"never"))
		return 0;

	return fileExist(szProfile) || shouldAutoCreate(szProfile);
}

// returns 1 if a profile was selected
static int getProfile(MFilePath &szProfile)
{
	getProfilePath(g_profileDir, _countof(g_profileDir));
	if (IsInsideRootDir(g_profileDir, true))
		if (WritePrivateProfileString(L"Database", L"ProfileDir", L"", mirandabootini))
			getProfilePath(g_profileDir, _countof(g_profileDir));

	getDefaultProfile(szProfile);
	getProfileCmdLine(szProfile);
	getProfileDefault(szProfile);

	if (IsInsideRootDir(g_profileDir, true)) {
		MessageBoxW(nullptr,
			TranslateT("Profile cannot be placed into Miranda root folder.\nPlease move Miranda profile to some other location."),
			LPGENW("Miranda NG"), MB_ICONERROR | MB_OK);
		return 0;
	}

	PROFILEMANAGERDATA pd(szProfile);
	if (CmdLine_GetOption(L"ForceShowPM")) {
LBL_Show:
		if (!getProfileManager(&pd))
			return 0;

		szProfile = pd.m_profile;
		return 1;
	}

	if (getProfileAutoRun(szProfile))
		return 1;

	if (getProfile1(szProfile, g_profileDir, &pd.noProfiles))
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
		if (GetShortPathNameW(tszOriginalName, tszProfile, MAX_PATH) != 0)
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
		if (err != EGROKPRF_NOERROR) { // smth went wrong
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
		if (err == EGROKPRF_NOERROR) {
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
	#ifdef _WINDOWS
		wchar_t classname[256];
		if (GetClassName(hwnd, classname, _countof(classname)) && mir_wstrcmp(L"Miranda", classname) == 0) {
			DWORD_PTR res = 0;
			if (SendMessageTimeout(hwnd, uMsgProcessProfile, lParam, 0, SMTO_ABORTIFHUNG, 100, &res) && res)
				return FALSE;
		}
	#endif

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Default menu items

static INT_PTR CompactMe(void* obj, WPARAM, LPARAM)
{
	auto *db = (MDatabaseCommon *)obj;
	if (!db->Compact())
		MessageBoxW(0, TranslateT("Database was compacted successfully"), TranslateT("Database"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBoxW(0, TranslateT("Database compaction failed"), TranslateT("Database"), MB_OK | MB_ICONERROR);
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
		mi.name.a = LPGEN("Compact database");
		mi.pszService = "DB/UI/Compact";
		Menu_AddMainMenuItem(&mi);

		CreateServiceFunctionObj(mi.pszService, CompactMe, pDb);
	}

	InitCryptMenuItem(mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t tszNoDrivers[] = LPGENW("Miranda is unable to open '%s' because you do not have any profile plugins installed.\nPress Yes to install Dbx_sqlite or Cancel to exit");
static wchar_t tszUnknownFormat[] = LPGENW("Miranda was unable to open '%s', it's in an unknown format.");
static wchar_t tszProfileLocked[] = LPGENW("Miranda was unable to open '%s'\nIt's inaccessible or used by other application or Miranda instance");
static wchar_t tszNoSuitableDriver[] = LPGENW("Miranda was unable to open '%s'\nThere is no suitable database driver installed");

static IconItem iconList[] =
{
	{ LPGEN("Show password"), "showpass", IDI_EYE },
};

int LoadDatabaseModule(void)
{
	wchar_t tmp[MAX_PATH];
	PathToAbsoluteW(L".", tmp);
	_wchdir(tmp);

	// find out which profile to load
	MFilePath szProfile;
	if (!getProfile(szProfile))
		return 1;

	auto *ptszFileName = wcsrchr(szProfile, '\\');
	ptszFileName = (ptszFileName) ? ptszFileName + 1 : szProfile.c_str();

	if (arDbPlugins.getCount() == 0) {
		CMStringW wszError(FORMAT, TranslateW(tszNoDrivers), ptszFileName);
		if (IDYES == MessageBoxW(nullptr, wszError, TranslateT("No profile support installed!"), MB_YESNOCANCEL | MB_ICONERROR))
			Utils_OpenUrl("https://miranda-ng.org/p/Dbx_sqlite");
		return 1;
	}

	// if this profile is already opened in another miranda, silently return
	if (Profile_CheckOpened(szProfile)) {
		#ifdef _WINDOWS
			uMsgProcessProfile = RegisterWindowMessage(L"Miranda::ProcessProfile");
			ATOM aPath = GlobalAddAtom(szProfile);
			EnumWindows(EnumMirandaWindows, (LPARAM)aPath);
			GlobalDeleteAtom(aPath);
		#endif
		return 1;
	}

	// find a driver to support the given profile
	bool retry;
	int rc;
	do {
		retry = false;
		if (!szProfile.isExist() && shouldAutoCreate(szProfile))
			rc = tryCreateDatabase(szProfile);
		else
			rc = tryOpenDatabase(szProfile);

		// there were no suitable driver installed
		if (rc == -1) {
			MessageBoxW(nullptr,
				CMStringW(FORMAT, TranslateW(tszNoSuitableDriver), ptszFileName),
				TranslateT("Miranda can't open that profile"), MB_OK | MB_ICONERROR);
		}
		else if (rc > 0) {
			if (fileExist(szProfile)) {
				// file isn't locked, just no driver could open it.
				MessageBoxW(nullptr,
					CMStringW(FORMAT, TranslateW(tszUnknownFormat), ptszFileName),
					TranslateT("Miranda can't understand that profile"), MB_OK | MB_ICONERROR);
			}
			else
				retry = IDRETRY == MessageBoxW(nullptr,
					CMStringW(FORMAT, TranslateW(tszProfileLocked), ptszFileName),
					TranslateT("Miranda can't open that profile"), MB_RETRYCANCEL | MB_ICONERROR);
		}
	}
		while (retry);

	if (rc == 0)
		HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	g_plugin.registerIcon(LPGEN("Database"), iconList, "database");
	return rc;
}
