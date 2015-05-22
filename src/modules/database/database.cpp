/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "profilemanager.h"

// contains the location of mirandaboot.ini
bool g_bDbCreated;
TCHAR g_profileDir[MAX_PATH], g_profileName[MAX_PATH], g_shortProfileName[MAX_PATH];
TCHAR* g_defaultProfile;
void EnsureCheckerLoaded(bool);

void LoadDatabaseServices();

bool fileExist(const TCHAR *fname)
{
	if (*fname == 0)
		return false;

	FILE *fp = _tfopen(fname, _T("r+"));
	bool res = (fp != NULL);
	if (fp) fclose(fp);
	return res;
}

static void fillProfileName(const TCHAR* ptszFileName)
{
	const TCHAR* p = _tcsrchr(ptszFileName, '\\');
	if (p == NULL)
		p = ptszFileName;
	else
		p++;

	_tcsncpy_s(g_profileName, p, _TRUNCATE);

	_tcsncpy_s(g_shortProfileName, p, _TRUNCATE);
	TCHAR *pos = _tcsrchr(g_shortProfileName, '.');
	if (mir_tstrcmpi(pos, _T(".dat")) == 0)
		*pos = 0;
}

bool IsInsideRootDir(TCHAR* profiledir, bool exact)
{
	VARST pfd( _T("%miranda_path%"));
	if (exact)
		return mir_tstrcmpi(profiledir, pfd) == 0;

	return _tcsnicmp(profiledir, pfd, mir_tstrlen(pfd)) == 0;
}

// returns 1 if the profile path was returned, without trailing slash
int getProfilePath(TCHAR *buf, size_t cch)
{
	TCHAR profiledir[MAX_PATH];
	GetPrivateProfileString(_T("Database"), _T("ProfileDir"), _T(""), profiledir, SIZEOF(profiledir), mirandabootini);

	if (profiledir[0] == 0)
		mir_tstrcpy(profiledir, _T("%miranda_path%\\Profiles"));

	size_t len = PathToAbsoluteT( VARST(profiledir), buf);

	if (buf[len-1] == '/' || buf[len-1] == '\\')
		buf[len-1] = 0;

	return 0;
}

// returns 1 if *.dat spec is matched
int isValidProfileName(const TCHAR *name)
{
	size_t len = mir_tstrlen(name) - 4;
	return len > 0 && mir_tstrcmpi(&name[len], _T(".dat")) == 0;
}

// returns 1 if the profile manager should be shown
static bool showProfileManager(void)
{
	TCHAR Mgr[32];
	// is control pressed?
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		return 1;

	// wanna show it?
	GetPrivateProfileString(_T("Database"), _T("ShowProfileMgr"), _T("never"), Mgr, SIZEOF(Mgr), mirandabootini);
	return (mir_tstrcmpi(Mgr, _T("yes")) == 0);
}

bool shouldAutoCreate(TCHAR *szProfile)
{
	if (szProfile[0] == 0)
		return false;

	TCHAR ac[32];
	GetPrivateProfileString(_T("Database"), _T("AutoCreate"), _T(""), ac, SIZEOF(ac), mirandabootini);
	return mir_tstrcmpi(ac, _T("yes")) == 0;
}

static void getDefaultProfile(TCHAR *szProfile, size_t cch)
{
	TCHAR defaultProfile[MAX_PATH];
	GetPrivateProfileString(_T("Database"), _T("DefaultProfile"), _T(""), defaultProfile, SIZEOF(defaultProfile), mirandabootini);

	if (defaultProfile[0] == 0)
		return;

	VARST res(defaultProfile);
	if (res)
		mir_sntprintf(szProfile, cch, _T("%s\\%s\\%s%s"), g_profileDir, (TCHAR*)res, (TCHAR*)res, isValidProfileName(res) ? _T("") : _T(".dat"));
	else
		szProfile[0] = 0;
}

// returns 1 if something that looks like a profile is there
static void loadProfileByShortName(const TCHAR* src, TCHAR *szProfile, size_t cch)
{
	TCHAR buf[MAX_PATH];
	_tcsncpy_s(buf, src, _TRUNCATE);

	TCHAR *p = _tcsrchr(buf, '\\'); if (p) ++p; else p = buf;
	if (!isValidProfileName(buf) && *p)
		_tcscat(buf, _T(".dat"));

	TCHAR profileName[MAX_PATH], newProfileDir[MAX_PATH];
	_tcsncpy_s(profileName, p, _TRUNCATE);
	if (!isValidProfileName(profileName) && *p)
		_tcscat(profileName, _T(".dat"));

	_tcsncpy_s(profileName, p, _TRUNCATE);
	p = _tcsrchr(profileName, '.'); if (p) *p = 0;

	mir_sntprintf(newProfileDir, cch, _T("%s\\%s\\"), g_profileDir, profileName);
	PathToAbsoluteT(buf, szProfile, newProfileDir);

	if ( _tcschr(buf, '\\')) {
		_tcsncpy_s(g_profileDir, szProfile, _TRUNCATE);
		if (profileName[0]) {
			p = _tcsrchr(g_profileDir, '\\'); *p = 0;
			p = _tcsrchr(g_profileDir, '\\');
			if (p && mir_tstrcmpi(p + 1, profileName) == 0)
				*p = 0;
		}
		else szProfile[0] = 0;
	}
}

void getProfileCmdLine(TCHAR *szProfile, size_t cch)
{
	LPCTSTR ptszProfileName = CmdLine_GetOption( _T("profile"));
	if (ptszProfileName != NULL)
		loadProfileByShortName(ptszProfileName, szProfile, cch);
}

void getProfileDefault(TCHAR *szProfile, size_t cch)
{
	if (g_defaultProfile != NULL) {
		loadProfileByShortName(g_defaultProfile, szProfile, cch);
		mir_free(g_defaultProfile);
	}
}

// move profile from profile subdir
static void moveProfileDirProfiles(TCHAR *profiledir, BOOL isRootDir = TRUE)
{
	TCHAR pfd[MAX_PATH];
	if (isRootDir)
		_tcsncpy_s(pfd, VARST(_T("%miranda_path%\\*.dat")), _TRUNCATE);
	else
		mir_sntprintf(pfd, SIZEOF(pfd), _T("%s\\*.dat"), profiledir);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(pfd, &ffd);
	if (hFind != INVALID_HANDLE_VALUE) {
		TCHAR *c = _tcsrchr(pfd, '\\'); if (c) *c = 0;
		do {
			TCHAR path[MAX_PATH], path2[MAX_PATH];
			TCHAR* profile = mir_tstrdup(ffd.cFileName);
			TCHAR *c = _tcsrchr(profile, '.'); if (c) *c = 0;
			mir_sntprintf(path, SIZEOF(path), _T("%s\\%s"), pfd, ffd.cFileName);
			mir_sntprintf(path2, SIZEOF(path2), _T("%s\\%s"), profiledir, profile);
			CreateDirectoryTreeT(path2);
			mir_sntprintf(path2, SIZEOF(path2), _T("%s\\%s\\%s"), profiledir, profile, ffd.cFileName);
			if (_taccess(path2, 0) == 0) {
				TCHAR buf[512];
				mir_sntprintf(buf, SIZEOF(buf),
					TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s\nBecause profile with this name already exists. Please resolve the issue manually."),
					path, path2);
				MessageBox(NULL, buf, _T("Miranda NG"), MB_ICONERROR | MB_OK);
			}
			else if (MoveFile(path, path2) == 0) {
				TCHAR buf[512];
				mir_sntprintf(buf, SIZEOF(buf),
					TranslateT("Miranda is trying to upgrade your profile structure.\nIt cannot move profile %s to the new location %s automatically\nMost likely this is due to insufficient privileges. Please move profile manually."),
					path, path2);
				MessageBox(NULL, buf, _T("Miranda NG"), MB_ICONERROR | MB_OK);
				mir_free(profile);
				break;
			}
			mir_free(profile);
		}
			while (FindNextFile(hFind, &ffd));
	}
	FindClose(hFind);
}

// returns 1 if a single profile (full path) is found within the profile dir
static int getProfile1(TCHAR *szProfile, size_t cch, TCHAR *profiledir, BOOL * noProfiles)
{
	int found = 0;

	if (IsInsideRootDir(profiledir, false))
		moveProfileDirProfiles(profiledir);
	moveProfileDirProfiles(profiledir, FALSE);

	bool bNoDefaultProfile = (*szProfile == 0);
	bool reqfd = !bNoDefaultProfile && (_taccess(szProfile, 0) == 0 || shouldAutoCreate(szProfile));
	bool bShowProfileManager = showProfileManager();

	if (reqfd)
		found++;

	if (bShowProfileManager || !reqfd) {
		TCHAR searchspec[MAX_PATH];
		mir_sntprintf(searchspec, SIZEOF(searchspec), _T("%s\\*.*"), profiledir);

		WIN32_FIND_DATA ffd;
		HANDLE hFind = FindFirstFile(searchspec, &ffd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// make sure the first hit is actually a *.dat file
				if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !mir_tstrcmp(ffd.cFileName, _T(".")) || !mir_tstrcmp(ffd.cFileName, _T("..")))
					continue;

				TCHAR newProfile[MAX_PATH];
				mir_sntprintf(newProfile, SIZEOF(newProfile), _T("%s\\%s\\%s.dat"), profiledir, ffd.cFileName, ffd.cFileName);
				if (_taccess(newProfile, 0) != 0)
					continue;

				switch (touchDatabase(newProfile, NULL)) {
				case 0:
				case EGROKPRF_OBSOLETE:
					if (++found == 1 && bNoDefaultProfile)
						_tcsncpy_s(szProfile, cch, newProfile, _TRUNCATE);
					break;
				}
			}
				while (FindNextFile(hFind, &ffd));

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
static int getProfileAutoRun(TCHAR *szProfile)
{
	if (*szProfile == 0)
		return false;

	TCHAR Mgr[32];
	GetPrivateProfileString(_T("Database"), _T("ShowProfileMgr"), _T(""), Mgr, SIZEOF(Mgr), mirandabootini);
	if (mir_tstrcmpi(Mgr, _T("never")))
		return 0;

	return fileExist(szProfile) || shouldAutoCreate(szProfile);
}

// returns 1 if a profile was selected
static int getProfile(TCHAR *szProfile, size_t cch)
{
	getProfilePath(g_profileDir, SIZEOF(g_profileDir));
	if (IsInsideRootDir(g_profileDir, true))
		if (WritePrivateProfileString(_T("Database"), _T("ProfileDir"), _T(""), mirandabootini))
			getProfilePath(g_profileDir, SIZEOF(g_profileDir));

	getDefaultProfile(szProfile, cch);
	getProfileCmdLine(szProfile, cch);
	getProfileDefault(szProfile, cch);

	if (IsInsideRootDir(g_profileDir, true)) {
		MessageBox(NULL,
			TranslateT("Profile cannot be placed into Miranda root folder.\nPlease move Miranda profile to some other location."),
			LPGENT("Miranda NG"), MB_ICONERROR | MB_OK);
		return 0;
	}

	PROFILEMANAGERDATA pd = { 0 };
	if (CmdLine_GetOption(_T("ForceShowPM"))) {
LBL_Show:
		pd.ptszProfile = szProfile;
		pd.ptszProfileDir = g_profileDir;
		if (!getProfileManager(&pd))
			return 0;

		if (!pd.bRun)
			return CallService(MS_DB_CHECKPROFILE, WPARAM(szProfile), TRUE);

		return 1;
	}

	if (getProfileAutoRun(szProfile))
		return 1;

	if (getProfile1(szProfile, cch, g_profileDir, &pd.noProfiles))
		return 1;

	goto LBL_Show;
}

// carefully converts a file name from TCHAR* to char*
char* makeFileName(const TCHAR* tszOriginalName)
{
	char *szResult = NULL;
	char *szFileName = mir_t2a(tszOriginalName);
	TCHAR *tszFileName = mir_a2t(szFileName);
	if (mir_tstrcmp(tszOriginalName, tszFileName)) {
		TCHAR tszProfile[MAX_PATH];
		if (GetShortPathName(tszOriginalName, tszProfile, MAX_PATH) != 0)
			szResult = mir_t2a(tszProfile);
	}

	if (!szResult)
		szResult = szFileName;
	else
		mir_free(szFileName);
	mir_free(tszFileName);

	return szResult;
}

int touchDatabase(const TCHAR *tszProfile, DATABASELINK **dblink)
{
	for (int i = arDbPlugins.getCount() - 1; i >= 0; i--) {
		DATABASELINK *p = arDbPlugins[i];
		int iErrorCode = p->grokHeader(tszProfile);
		if (iErrorCode == 0) {
			if (dblink)
				*dblink = p;
			return 0;
		}
		if (iErrorCode == EGROKPRF_OBSOLETE) {
			if (dblink)
				*dblink = p;
			return EGROKPRF_OBSOLETE;
		}
	}
	
	if (dblink)
		*dblink = NULL;
	return EGROKPRF_CANTREAD;
}

// enumerate all plugins that had valid DatabasePluginInfo()
int tryOpenDatabase(const TCHAR *tszProfile)
{
	bool bWasOpened = false;

	for (int i = arDbPlugins.getCount() - 1; i >= 0; i--) {
		DATABASELINK *p = arDbPlugins[i];

		// liked the profile?
		int err = p->grokHeader(tszProfile);
		if (err != ERROR_SUCCESS) { // smth went wrong
			switch (err) {
			case EGROKPRF_CANTREAD:
			case EGROKPRF_UNKHEADER:
				// just not supported.
				continue;

			case EGROKPRF_OBSOLETE:
				EnsureCheckerLoaded(true);
				CallService(MS_DB_CHECKPROFILE, (WPARAM)tszProfile, 2);
				break;

			default:
				return err;
			}
		}

		bWasOpened = true;

		// try to load database
		MIDatabase *pDb = p->Load(tszProfile, FALSE);
		if (pDb) {
			fillProfileName(tszProfile);
			currDblink = p;
			db_setCurrent(currDb = pDb);
			return 0;
		}
	}

	return (bWasOpened) ? -1 : EGROKPRF_CANTREAD;
}

// enumerate all plugins that had valid DatabasePluginInfo()
static int tryCreateDatabase(const TCHAR *ptszProfile)
{
	TCHAR *tszProfile = NEWTSTR_ALLOCA(ptszProfile);
	CreatePathToFileT(tszProfile);

	for (int i = 0; i < arDbPlugins.getCount(); i++) {
		DATABASELINK* p = arDbPlugins[i];

		int err = p->makeDatabase(tszProfile);
		if (err == ERROR_SUCCESS) {
			g_bDbCreated = true;
			MIDatabase *pDb = p->Load(tszProfile, FALSE);
			if (pDb != NULL) {
				fillProfileName(tszProfile);
				currDblink = p;
				db_setCurrent(currDb = pDb);
				return 0;
			}
			return 1;
		}
	}
	return 1;
}

typedef struct {
	TCHAR *profile;
	UINT msg;
	ATOM aPath;
	int found;
} ENUMMIRANDAWINDOW;

static BOOL CALLBACK EnumMirandaWindows(HWND hwnd, LPARAM lParam)
{
	TCHAR classname[256];
	ENUMMIRANDAWINDOW *x = (ENUMMIRANDAWINDOW *)lParam;
	DWORD_PTR res = 0;
	if (GetClassName(hwnd, classname, SIZEOF(classname)) && mir_tstrcmp(_T("Miranda"), classname) == 0) {
		if (SendMessageTimeout(hwnd, x->msg, (WPARAM)x->aPath, 0, SMTO_ABORTIFHUNG, 100, &res) && res) {
			x->found++;
			return FALSE;
		}
	}
	return TRUE;
}

static int FindMirandaForProfile(TCHAR *szProfile)
{
	ENUMMIRANDAWINDOW x = { 0 };
	x.profile = szProfile;
	x.msg = RegisterWindowMessage(_T("Miranda::ProcessProfile"));
	x.aPath = GlobalAddAtom(szProfile);
	EnumWindows(EnumMirandaWindows, (LPARAM)&x);
	GlobalDeleteAtom(x.aPath);
	return x.found;
}

int LoadDatabaseModule(void)
{
	TCHAR szProfile[MAX_PATH];
	PathToAbsoluteT(_T("."), szProfile);
	_tchdir(szProfile);
	szProfile[0] = 0;

	LoadDatabaseServices();

	// find out which profile to load
	if (!getProfile(szProfile, SIZEOF(szProfile)))
		return 1;

	if (arDbPlugins.getCount() == 0) {
		TCHAR buf[256];
		TCHAR *p = _tcsrchr(szProfile, '\\');
		mir_sntprintf(buf, SIZEOF(buf), TranslateT("Miranda is unable to open '%s' because you do not have any profile plugins installed.\nYou need to install dbx_mmap.dll"), p ? ++p : szProfile);
		MessageBox(0, buf, TranslateT("No profile support installed!"), MB_OK | MB_ICONERROR);
	}

	// find a driver to support the given profile
	bool retry;
	int rc;
	do {
		retry = false;
		if (_taccess(szProfile, 0) && shouldAutoCreate(szProfile))
			rc = tryCreateDatabase(szProfile);
		else
			rc = tryOpenDatabase(szProfile);

		if (rc > 0) {
			// if there were drivers but they all failed cos the file is locked, try and find the miranda which locked it
			if (fileExist(szProfile)) {
				// file isn't locked, just no driver could open it.
				TCHAR buf[256];
				TCHAR *p = _tcsrchr(szProfile, '\\');
				mir_sntprintf(buf, SIZEOF(buf), TranslateT("Miranda was unable to open '%s', it's in an unknown format.\nThis profile might also be damaged, please run DbChecker which should be installed."), p ? ++p : szProfile);
				MessageBox(0, buf, TranslateT("Miranda can't understand that profile"), MB_OK | MB_ICONERROR);
			}
			else if (!FindMirandaForProfile(szProfile)) {
				TCHAR buf[256];
				TCHAR *p = _tcsrchr(szProfile, '\\');
				mir_sntprintf(buf, SIZEOF(buf), TranslateT("Miranda was unable to open '%s'\nIt's inaccessible or used by other application or Miranda instance"), p ? ++p : szProfile);
				retry = MessageBox(0, buf, TranslateT("Miranda can't open that profile"), MB_RETRYCANCEL | MB_ICONERROR) == IDRETRY;
			}
		}
	}
		while (retry);

	EnsureCheckerLoaded(false); // unload dbchecker

	if (rc == ERROR_SUCCESS) {
		InitIni();
		return 0;
	}

	return rc;
}
