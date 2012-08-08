/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "Common.h"

int CalculateModuleHash(const TCHAR* tszFileName, char* dest);

static BYTE IsPluginDisabled(const char *filename)
{
	return DBGetContactSettingByte(NULL, "PluginDisable", filename, 0);
}

static bool Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef map<string, string> hashMap;
typedef pair<string, string> hashItem;

static void ScanFolder(const TCHAR* tszFolder, const TCHAR* tszBaseUrl, hashMap& hashes, vector<FILEINFO>* UpdateFiles)
{
	TCHAR tszMask[MAX_PATH], tszFileTemp[MAX_PATH];
	mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\*"), tszFolder);
	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszMask, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ( !_tcscmp(ffd.cFileName, _T(".")) || !_tcscmp(ffd.cFileName, _T("..")))
				continue;
			if ( !_tcsicmp(ffd.cFileName, _T("Profiles")))
				continue;

			mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\%s"), tszFolder, ffd.cFileName);
			ScanFolder(tszMask, tszBaseUrl, hashes, UpdateFiles);
			continue;
		}

		TCHAR *p = _tcsrchr(ffd.cFileName, '.');
		if (!p) continue;
		if ( _tcsicmp(p, _T(".dll")) && _tcsicmp(p, _T(".exe")))
			continue;

		char szFileName[MAX_PATH];
		strncpy(szFileName, _T2A(ffd.cFileName), SIZEOF(szFileName));
		_strlwr(szFileName);
		if ( IsPluginDisabled(szFileName)) //check if plugin disabled
			continue;

		// Read version info
		hashMap::iterator boo = hashes.find(szFileName);
		if (boo == hashes.end())
			continue;

		TCHAR *plugname = ffd.cFileName;
		FILEINFO FileInfo = { 0 };

		mir_sntprintf(tszMask, SIZEOF(tszMask), _T("%s\\%s"), tszFolder, ffd.cFileName);
		CalculateModuleHash(tszMask, FileInfo.curhash);

		strncpy(FileInfo.newhash, boo->second.c_str(), SIZEOF(FileInfo.newhash));

		// Compare versions
		if ( strcmp(FileInfo.curhash, FileInfo.newhash)) { // Yeah, we've got new version.
			_tcscpy(FileInfo.tszDescr, ffd.cFileName);

			*p = 0;
			mir_sntprintf(FileInfo.File.tszDownloadURL, SIZEOF(FileInfo.File.tszDownloadURL), _T("%s/%s.zip"), tszBaseUrl, ffd.cFileName);
			_tcslwr(FileInfo.File.tszDownloadURL);

			mir_sntprintf(FileInfo.File.tszDiskPath, SIZEOF(FileInfo.File.tszDiskPath), _T("%s\\%s.zip"), tszFileTemp, ffd.cFileName);

			UpdateFiles->push_back(FileInfo);
		} // end compare versions
	}
		while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

static void __stdcall LaunchDialog(void* param)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)param);
}

static void CheckUpdates(void *)
{
	char szKey[64] = {0};
	DBVARIANT dbVar = {0};

	if (!Exists(tszRoot))
		CreateDirectoryTreeT(tszRoot);

	// Load files info
	if (DBGetContactSettingTString(NULL, MODNAME, "UpdateURL", &dbVar)) { // URL is not set
		DBWriteContactSettingTString(NULL, MODNAME, "UpdateURL", _T(DEFAULT_UPDATE_URL));
		DBGetContactSettingTString(NULL, MODNAME, "UpdateURL", &dbVar);
	}
	TCHAR* tszBaseUrl = NEWTSTR_ALLOCA(dbVar.ptszVal);
	DBFreeVariant(&dbVar);

	// Download version info
	ShowPopup(NULL, TranslateT("Plugin Updater"), TranslateT("Downloading version info..."), 3, 0);

	FILEURL pFileUrl;
	mir_sntprintf(pFileUrl.tszDownloadURL, SIZEOF(pFileUrl.tszDownloadURL), _T("%s/hashes.zip"), tszBaseUrl);
	mir_sntprintf(pFileUrl.tszDiskPath, SIZEOF(pFileUrl.tszDiskPath), _T("%s\\hashes.zip"), tszRoot);
	if (!DownloadFile(pFileUrl.tszDownloadURL, pFileUrl.tszDiskPath)) {
		ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("An error occured while downloading the update."), 1, 0);
		CheckThread = NULL;
		return;
	}

	unzip(pFileUrl.tszDiskPath, tszRoot, tszRoot);
	DeleteFile(pFileUrl.tszDiskPath);

	TCHAR tszTmpIni[MAX_PATH] = {0};
	mir_sntprintf(tszTmpIni, SIZEOF(tszTmpIni), _T("%s\\hashes.txt"), tszRoot);
	FILE* fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp)
		return;

	hashMap hashes;
	char str[200];
	while(fgets(str, SIZEOF(str), fp) != NULL) {
		rtrim(str);
		char* p = strchr(str, ' ');
		if (p == NULL)
			continue;

		*p++ = 0;
		_strlwr(str);
		if ( !opts.bUpdateIcons && !strncmp(str, "icons\\", 6))
			continue;

		char* szName = strrchr(str, '\\');
		if (szName == NULL)
			szName = str;
		else 
			szName++;

		_strlwr(p);
		hashes[szName] = p;
	}
	fclose(fp);
	DeleteFile(tszTmpIni);

	vector<FILEINFO>* UpdateFiles = new vector<FILEINFO>;
	TCHAR *dirname = Utils_ReplaceVarsT(_T("%miranda_path%"));
	ScanFolder(dirname, tszBaseUrl, hashes, UpdateFiles);
	mir_free(dirname);

	// Show dialog
	if (UpdateFiles->size() == 0) {
		if ( !opts.bSilent)
			ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("No updates found."), 2, 0);
	}
	else CallFunctionAsync(LaunchDialog, UpdateFiles);

	CheckThread = NULL;
}

void DoCheck(int iFlag)
{
	if (CheckThread)
		ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("Update checking already started!"), 2, 0);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	}
	else if (iFlag) {
		CheckThread = mir_forkthread(CheckUpdates, 0);
		DBWriteContactSettingDword(NULL, MODNAME, "LastUpdate", time(NULL));
	}
}
