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

int CalculateModuleHash(const TCHAR *tszFileName, char *dest);

static BYTE IsPluginDisabled(const char *filename)
{
	return DBGetContactSettingByte(NULL, "PluginDisable", filename, 0);
}

static bool Exists(LPCTSTR strName)
{
	return GetFileAttributes(strName) != INVALID_FILE_ATTRIBUTES;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct ServListEntry
{
	~ServListEntry()
	{	mir_free(m_name);
	}

	TCHAR *m_name, *m_searchName;
	char   m_szHash[32+1];
};

static int CompareHashes(const ServListEntry *p1, const ServListEntry *p2)
{
	return _tcscmp(p1->m_searchName, p2->m_searchName);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct
{
	TCHAR *oldName, *newName;
}
static renameTable[] =
{
	{ _T("svc_dbepp.dll"),   _T("Plugins\\dbeditorpp.dll") },
	{ _T("svc_crshdmp.dll"), _T("Plugins\\crashdumper.dll") },
	{ _T("svc_vi.dll"),      _T("Plugins\\versioninfo.dll") },
	{ _T("import_sa.dll"),   _T("Plugins\\import.dll") },
	{ _T("dbtool.exe"),      _T("Plugins\\dbchecker.dll") },
	{ _T("dbtool_sa.exe"),   _T("Plugins\\dbchecker.dll") },

	{ _T("clist_classic.dll"), _T("") },
	{ _T("chat.dll"),          _T("") },
	{ _T("srmm.dll"),          _T("") },
};

static bool CheckFileRename(const TCHAR *ptszOldName, TCHAR *pNewName)
{
	for (int i=0; i < SIZEOF(renameTable); i++)
		if ( !_tcsicmp(ptszOldName, renameTable[i].oldName)) {
			_tcscpy(pNewName, renameTable[i].newName);
			return true;
		}

	return false;
}

typedef OBJLIST<ServListEntry> SERVLIST;

static void ScanFolder(const TCHAR *tszFolder, size_t cbBaseLen, int level, const TCHAR *tszBaseUrl, SERVLIST& hashes, OBJLIST<FILEINFO> *UpdateFiles)
{
	// skip updater's own folder
	if ( !_tcsicmp(tszFolder, tszRoot))
		return;

	TCHAR tszBuf[MAX_PATH];
	mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\*"), tszFolder);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszBuf, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if ( !_tcscmp(ffd.cFileName, _T(".")) || !_tcscmp(ffd.cFileName, _T("..")))
				continue;
			if ( !_tcsicmp(ffd.cFileName, _T("Profiles")))
				continue;

			mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\%s"), tszFolder, ffd.cFileName);
			ScanFolder(tszBuf, cbBaseLen, level+1, tszBaseUrl, hashes, UpdateFiles);
			continue;
		}

		TCHAR *pExt = _tcsrchr(ffd.cFileName, '.');
		if (pExt == NULL) continue;
		if ( _tcsicmp(pExt, _T(".dll")) && _tcsicmp(pExt, _T(".exe")))
			continue;

		// calculate the current file's relative name and store it into tszNewName
		TCHAR tszNewName[MAX_PATH], key[MAX_PATH];
		if ( !CheckFileRename(ffd.cFileName, tszNewName)) {
			if (level == 0)
				_tcscpy(tszNewName, ffd.cFileName);
			else
				mir_sntprintf(tszNewName, SIZEOF(tszNewName), _T("%s\\%s"), tszFolder+cbBaseLen, ffd.cFileName);
		}

		bool bHasNewVersion = false;
		TCHAR *ptszUrl;
		mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\%s"), tszFolder, ffd.cFileName);

		// this file is not marked for deletion
		if (tszNewName[0]) { 
			// parse a relative name and extract a key for hashtable lookup
			TCHAR *p = _tcschr(tszNewName, '\\');
			_tcscpy(key, (p != NULL) ? p+1 : tszNewName);
			_tcslwr(key);
			ServListEntry tmp = {NULL, key};
			ServListEntry *item = hashes.find(&tmp);
			if (item == NULL) {
				TCHAR *p = _tcschr(key, '.');
				if (p[-1] != 'w')
					continue;
	
				int iPos = int(p - key)-1;
				strdel(p-1, 1);
				if ((item = hashes.find(&tmp)) == NULL)
					continue;

				strdel(tszNewName+iPos, 1);
			}

			PrepareFileName(key, SIZEOF(key), NULL, item->m_name);
			if ( _tcsicmp(tszNewName, key)) // skip files with the same names from another folders
				continue;

			ptszUrl = item->m_name;

			char szMyHash[33];
			CalculateModuleHash(tszBuf, szMyHash);

			bHasNewVersion = strcmp(szMyHash, item->m_szHash) != 0;
		}
		else { // file was marked for deletion, add it to the list anyway
			bHasNewVersion = true;
			ptszUrl = _T("");
		}

		// Compare versions
		if (bHasNewVersion) { // Yeah, we've got new version.
			FILEINFO *FileInfo = new FILEINFO;
			_tcscpy(FileInfo->tszOldName, tszBuf+cbBaseLen); // copy the relative old name
			if (tszNewName[0] == 0) {
				FileInfo->bDeleteOnly = TRUE;
				_tcscpy(FileInfo->tszNewName, tszBuf);  // save the full old name for deletion
			}
			else {
				FileInfo->bDeleteOnly = FALSE;
				PrepareFileName(FileInfo->tszNewName, SIZEOF(FileInfo->tszNewName), NULL, ptszUrl);
			}

			_tcscpy(tszBuf, ptszUrl);
			TCHAR *p = _tcsrchr(tszBuf, '.');
			if (p) *p = 0;
			p = _tcsrchr(tszBuf, '/');

			mir_sntprintf(FileInfo->File.tszDiskPath, SIZEOF(FileInfo->File.tszDiskPath), _T("%s\\Temp\\%s.zip"), tszRoot, (p) ? p+1 : tszBuf);
			mir_sntprintf(FileInfo->File.tszDownloadURL, SIZEOF(FileInfo->File.tszDownloadURL), _T("%s/%s"), tszBaseUrl, ptszUrl);
			if ((pExt = _tcsrchr(FileInfo->File.tszDownloadURL, '.')) != NULL)
				_tcscpy(pExt, _T(".zip"));

			UpdateFiles->insert(FileInfo);
		} // end compare versions
	}
		while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

static void __stdcall LaunchDialog(void *param)
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

	REPLACEVARSARRAY vars[2];
	vars[0].lptzKey = _T("platform");
	#ifdef WIN64
		vars[0].lptzValue = _T("64");
	#else
		vars[0].lptzValue = _T("32");
	#endif
	vars[1].lptzKey = vars[1].lptzValue = 0;

	REPLACEVARSDATA dat = { sizeof(REPLACEVARSDATA) };
	dat.dwFlags = RVF_TCHAR;
	dat.variables = vars;
	mir_ptr<TCHAR> tszBaseUrl((TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)dbVar.ptszVal, (LPARAM)&dat));
	DBFreeVariant(&dbVar);

	// Download version info
	ShowPopup(NULL, TranslateT("Plugin Updater"), TranslateT("Downloading version info..."), 4, 0);

	FILEURL pFileUrl;
	mir_sntprintf(pFileUrl.tszDownloadURL, SIZEOF(pFileUrl.tszDownloadURL), _T("%s/hashes.zip"), (TCHAR*)tszBaseUrl);
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
	FILE *fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp)
		return;

	SERVLIST hashes(50, CompareHashes);
	char str[200];
	while(fgets(str, SIZEOF(str), fp) != NULL) {
		rtrim(str);
		char *p = strchr(str, ' ');
		if (p == NULL)
			continue;

		*p++ = 0;
		if ( !opts.bUpdateIcons && !_strnicmp(str, "icons\\", 6))
			continue;

		ServListEntry *newItem = new ServListEntry;
		_strlwr(p);
		strncpy(newItem->m_szHash, p, sizeof(newItem->m_szHash));

		for (p = strchr(str, '\\'); p != NULL; p = strchr(p+1, '\\'))
			*p = '/';

		newItem->m_name = mir_a2t(str);

		TCHAR *szName = _tcsrchr(newItem->m_name, '/');
		newItem->m_searchName = (szName == NULL) ? newItem->m_name : szName+1;
		_tcslwr(newItem->m_searchName);
		hashes.insert(newItem);
	}
	fclose(fp);
	DeleteFile(tszTmpIni);

	FILELIST *UpdateFiles = new FILELIST(20);
	TCHAR *dirname = Utils_ReplaceVarsT(_T("%miranda_path%"));
	ScanFolder(dirname, lstrlen(dirname)+1, 0, tszBaseUrl, hashes, UpdateFiles);
	mir_free(dirname);

	// Show dialog
	if (UpdateFiles->getCount() == 0) {
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
