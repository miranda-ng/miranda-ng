/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt. If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "common.h"

#define UM_ERROR (WM_USER+1)

static bool bShowDetails;
static HWND hwndDialog;

static void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, bEnable);
		db_set_b(NULL, MODNAME "Files", StrToLower(_T2A(todo[i].tszOldName)), todo[i].bEnabled = bEnable);
	}
}

static void SetStringText(HWND hWnd, size_t i, TCHAR *ptszText)
{
	ListView_SetItemText(hWnd, i, 1, ptszText);
}

static void ApplyUpdates(void *param)
{
	HWND hDlg = (HWND)param;

	//////////////////////////////////////////////////////////////////////////////////////
	// if we need to escalate priviledges, launch a atub

	if (!PrepareEscalation()) {
		EndDialog(hDlg, 0);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ok, let's unpack all zips

	AutoHandle pipe(hPipe);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	TCHAR tszBuff[2048], tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	SafeCreateDirectory(tszFileTemp);

	bool error = false;
	HANDLE nlc = NULL;
	for (int i=0; i < todo.getCount(); i++) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if (!todo[i].bEnabled) {
			SetStringText(hwndList, i, TranslateT("Skipped."));
			continue;
		}
		if (todo[i].bDeleteOnly) {
			SetStringText(hwndList, i, TranslateT("Will be deleted!"));
			continue;
		}

		// download update
		SetStringText(hwndList, i, TranslateT("Downloading..."));

		FILEURL *pFileUrl = &todo[i].File;
		if (!DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath, pFileUrl->CRCsum, nlc)) {
			SetStringText(hwndList, i, TranslateT("Failed!"));

			// interrupt update as we require all components to be updated
			error = true;
			break;
		}
		SetStringText(hwndList, i, TranslateT("Succeeded."));
	}
	Netlib_CloseHandle(nlc);

	if (error) {
		PostMessage(hDlg, UM_ERROR, 0, 0);
		return;
	}
	if (todo.getCount() > 0) {
		TCHAR *tszMirandaPath = Utils_ReplaceVarsT(_T("%miranda_path%"));

		for (int i = 0; i < todo.getCount(); i++) {
			if (!todo[i].bEnabled)
				continue;

			TCHAR tszBackFile[MAX_PATH];
			FILEINFO& p = todo[i];
			if (p.bDeleteOnly) { // we need only to backup the old file
				TCHAR *ptszRelPath = p.tszNewName + _tcslen(tszMirandaPath) + 1;
				mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, ptszRelPath);
				BackupFile(p.tszNewName, tszBackFile);
				continue;
			}

			// if file name differs, we also need to backup the old file here
			// otherwise it would be replaced by unzip
			if ( _tcsicmp(p.tszOldName, p.tszNewName)) {
				TCHAR tszSrcPath[MAX_PATH];
				mir_sntprintf(tszSrcPath, SIZEOF(tszSrcPath), _T("%s\\%s"), tszMirandaPath, p.tszOldName);
				mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, p.tszOldName);
				BackupFile(tszSrcPath, tszBackFile);
			}

			if ( unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack,true))
				SafeDeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
		}

		// Change title of clist
		#if MIRANDA_VER < 0x0A00
			ptrT title = db_get_tsa(NULL, "CList", "TitleText");
			if (!_tcsicmp(title, _T("Miranda IM")))
				db_set_ts(NULL, "CList", "TitleText", _T("Miranda NG"));
		#endif

		opts.bForceRedownload = false;
		db_unset(NULL, MODNAME, "ForceRedownload");

		db_set_b(NULL, MODNAME, "RestartCount", 5);

		PopupDataText temp;
		temp.Title = TranslateT("Plugin Updater");
		temp.Text = tszBuff;
		lstrcpyn(tszBuff, TranslateT("Update complete. Press Yes to restart Miranda now or No to postpone a restart until the exit."), SIZEOF(tszBuff));
		int rc = MessageBox(hDlg, temp.Text, temp.Title, MB_YESNO | MB_ICONQUESTION);
		if (rc == IDYES)
			CallFunctionAsync(RestartMe, 0);
	}
	EndDialog(hDlg, 0);
	return;
}

static void ResizeVert(HWND hDlg, int yy)
{
	RECT r = { 0, 0, 244, yy };
	MapDialogRect(hDlg, &r);
	r.bottom += GetSystemMetrics(SM_CYSMCAPTION);
	SetWindowPos(hDlg, 0, 0, 0, r.right, r.bottom, SWP_NOMOVE | SWP_NOZORDER);
}

static INT_PTR CALLBACK DlgUpdate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		hwndDialog = hDlg;
		TranslateDialogDefault(hDlg);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon("check_update"));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon("check_update", 1));
		{
			OSVERSIONINFO osver = { sizeof(osver) };
			if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6)
			{
				wchar_t szPath[MAX_PATH];
				GetModuleFileName(NULL, szPath, SIZEOF(szPath));
				TCHAR *ext = _tcsrchr(szPath, '.');
				if (ext != NULL)
					*ext = '\0';
				_tcscat(szPath, _T(".test"));
				HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE)
					// Running Windows Vista or later (major version >= 6).
					Button_SetElevationRequiredState(GetDlgItem(hDlg, IDOK), !IsProcessElevated());
				else
				{
					CloseHandle(hFile);
					DeleteFile(szPath);
				}
			}
			RECT r;
			GetClientRect(hwndList, &r);

			LVCOLUMN lvc = {0};
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");
			lvc.cx = 220; // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT("State");
			lvc.cx = 120 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			//enumerate plugins, fill in list
			//bool one_enabled = false;
			ListView_DeleteAllItems(hwndList);

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all items.
			LVITEM lvI = {0};
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			bool enableOk = false;
			OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) {
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_IMAGE;
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)&todo[i];
				lvI.pszText = todo[i].tszOldName;
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);

				// remember whether the user has decided not to update this component with this particular new version
				todo[i].bEnabled = db_get_b(NULL, MODNAME "Files", StrToLower(_T2A(todo[i].tszOldName)), true);
				ListView_SetCheckState(hwndList, lvI.iItem, todo[i].bEnabled);
				if (todo[i].bEnabled)
					enableOk = true;
			}
			HWND hwOk = GetDlgItem(hDlg, IDOK);
			EnableWindow(hwOk, enableOk);
		}

		bShowDetails = false;
		ResizeVert(hDlg, 60);

		// do this after filling list - enables 'ITEMCHANGED' below
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Utils_RestoreWindowPositionNoSize(hDlg, 0, MODNAME, "ConfirmWindow");
		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->hwndFrom == hwndList) {
			switch (((LPNMHDR) lParam)->code) {
			case LVN_ITEMCHANGED:
				if (GetWindowLongPtr(hDlg, GWLP_USERDATA)) {
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					LVITEM lvI = {0};
					lvI.iItem = nmlv->iItem;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_PARAM;
					ListView_GetItem(hwndList, &lvI);

					OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						todo[lvI.iItem].bEnabled = ListView_GetCheckState(hwndList, nmlv->iItem);
						db_set_b(NULL, MODNAME "Files", StrToLower(_T2A(todo[lvI.iItem].tszOldName)), todo[lvI.iItem].bEnabled);

						bool enableOk = false;
						for (int i=0; i < todo.getCount(); ++i) {
							if (todo[i].bEnabled) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		if (HIWORD( wParam ) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
			case IDOK:
				EnableWindow( GetDlgItem(hDlg, IDOK), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELALL), FALSE);
				EnableWindow( GetDlgItem(hDlg, IDC_SELNONE), FALSE);

				mir_forkthread(ApplyUpdates, hDlg);
				return TRUE;

			case IDC_DETAILS:
				bShowDetails = !bShowDetails;
				ResizeVert(hDlg, bShowDetails ? 242 : 60);
				SetDlgItemText(hDlg, IDC_DETAILS, (bShowDetails ? TranslateT("<< Details") : TranslateT("Details >>")));
				break;

			case IDC_SELALL:
				SelectAll(hDlg, true);
				break;

			case IDC_SELNONE:
				SelectAll(hDlg, false);
				break;

			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			}
		}
		break;

	case UM_ERROR:
		MessageBox(hDlg, TranslateT("Update failed! One of the components wasn't downloaded correctly. Try it again later."), TranslateT("Plugin Updater"), MB_OK | MB_ICONERROR);
		DestroyWindow(hDlg);
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hDlg, WM_SETICON, ICON_SMALL, 0));
		Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ConfirmWindow");
		hwndDialog = NULL;
		delete (OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

static void DlgUpdateSilent(void *lParam)
{
	OBJLIST<FILEINFO> &UpdateFiles = *(OBJLIST<FILEINFO> *)lParam;

	//////////////////////////////////////////////////////////////////////////////////////
	// if we need to escalate priviledges, launch a atub

	if (!PrepareEscalation()) {
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ok, let's unpack all zips

	AutoHandle pipe(hPipe);
	TCHAR tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	SafeCreateDirectory(tszFileTemp);

	bool error = false;
	HANDLE nlc = NULL;
	for (int i = 0; i < UpdateFiles.getCount(); i++) {
		if (!db_get_b(NULL, MODNAME "Files", StrToLower(_T2A(UpdateFiles[i].tszOldName)), true)) {
			continue;
		}
		if (UpdateFiles[i].bDeleteOnly) {
			continue;
		}

		// download update
		FILEURL *pFileUrl = &UpdateFiles[i].File;
		if (!DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath, pFileUrl->CRCsum, nlc)) {
			// interrupt update as we require all components to be updated
			error = true;
			break;
		}
	}
	Netlib_CloseHandle(nlc);

	if (error) {
		return;
	}
	if (UpdateFiles.getCount() > 0) {
		TCHAR *tszMirandaPath = Utils_ReplaceVarsT(_T("%miranda_path%"));

		for (int i = 0; i < UpdateFiles.getCount(); i++) {
			if (!db_get_b(NULL, MODNAME "Files", StrToLower(_T2A(UpdateFiles[i].tszOldName)), true))
				continue;

			TCHAR tszBackFile[MAX_PATH];
			FILEINFO& p = UpdateFiles[i];
			if (p.bDeleteOnly) { // we need only to backup the old file
				TCHAR *ptszRelPath = p.tszNewName + _tcslen(tszMirandaPath) + 1;
				mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, ptszRelPath);
				BackupFile(p.tszNewName, tszBackFile);
				continue;
			}

			// if file name differs, we also need to backup the old file here
			// otherwise it would be replaced by unzip
			if (_tcsicmp(p.tszOldName, p.tszNewName)) {
				TCHAR tszSrcPath[MAX_PATH];
				mir_sntprintf(tszSrcPath, SIZEOF(tszSrcPath), _T("%s\\%s"), tszMirandaPath, p.tszOldName);
				mir_sntprintf(tszBackFile, SIZEOF(tszBackFile), _T("%s\\%s"), tszFileBack, p.tszOldName);
				BackupFile(tszSrcPath, tszBackFile);
			}

			if (unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack, true))
				SafeDeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
		}

		// Change title of clist
#if MIRANDA_VER < 0x0A00
		ptrT title = db_get_tsa(NULL, "CList", "TitleText");
		if (!_tcsicmp(title, _T("Miranda IM")))
			db_set_ts(NULL, "CList", "TitleText", _T("Miranda NG"));
#endif

		opts.bForceRedownload = false;
		db_unset(NULL, MODNAME, "ForceRedownload");

		db_set_b(NULL, MODNAME, "RestartCount", 5);
		db_set_b(NULL, MODNAME, "NeedRestart", 1);

		if (!opts.bSilent)
			ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("You need restart your Miranda to apply installed updates"), 2, 0, 1);
	}
}

static void __stdcall LaunchDialog(void *param)
{
	if (opts.bSilentMode)
		DlgUpdateSilent(param);
	else
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_UPDATE), GetDesktopWindow(), DlgUpdate, (LPARAM)param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// building file list in the separate thread

struct
{
	TCHAR *oldName, *newName;
}
static renameTable[] =
{
	{ _T("svc_dbepp.dll"),                  _T("Plugins\\dbeditorpp.dll") },
	{ _T("svc_crshdmp.dll"),                _T("Plugins\\crashdumper.dll") },
	{ _T("svc_vi.dll"),                     _T("Plugins\\versioninfo.dll") },
	{ _T("crashrpt.dll"),                   _T("Plugins\\crashdumper.dll") },
	{ _T("advsplashscreen.dll"),            _T("Plugins\\splashscreen.dll") },
	{ _T("import_sa.dll"),                  _T("Plugins\\import.dll") },
	{ _T("newnr.dll"),                      _T("Plugins\\notesreminders.dll") },
	{ _T("dbtool.exe"),                     _T("Plugins\\dbchecker.dll") },
	{ _T("dbtool_sa.exe"),                  _T("Plugins\\dbchecker.dll") },
	{ _T("bclist.dll"),                     _T("Plugins\\clist_blind.dll") },
	{ _T("otr.dll"),                        _T("Plugins\\mirotr.dll") },
	{ _T("ttnotify.dll"),                   _T("Plugins\\tooltipnotify.dll") },
	{ _T("newstatusnotify.dll"),            _T("Plugins\\newxstatusnotify.dll") },
	{ _T("rss.dll"),                        _T("Plugins\\newsaggregator.dll") },
	{ _T("dbx_3x.dll"),                     _T("Plugins\\dbx_mmap.dll") },
	{ _T("dbx_mmap_sa.dll"),                _T("Plugins\\dbx_mmap.dll") },

	{ _T("proto_newsaggr.dll"),             _T("Icons\\proto_newsaggregator.dll") },
	{ _T("clienticons_*.dll"),              _T("Icons\\fp_icons.dll") },
	{ _T("fp_*.dll"),                       _T("Icons\\fp_icons.dll") },
	
	{ _T("langpack_*.txt"),                 _T("Languages\\*") },

	{ _T("clist_classic.dll"),              NULL },
	{ _T("chat.dll"),                       NULL },
	{ _T("gender.dll"),                     NULL },
	{ _T("srmm.dll"),                       NULL },
	{ _T("extraicons.dll"),                 NULL },
	{ _T("langman.dll"),                    NULL },
	{ _T("metacontacts.dll"),               NULL },
};

static bool CheckFileRename(const TCHAR *ptszOldName, TCHAR *pNewName)
{
	for (int i = 0; i < SIZEOF(renameTable); i++) {
		if (!wildcmpit(ptszOldName, renameTable[i].oldName))
			continue;

		TCHAR *ptszDest = renameTable[i].newName;
		if (ptszDest == NULL)
			*pNewName = 0;
		else {
			_tcsncpy_s(pNewName, MAX_PATH, ptszDest, _TRUNCATE);
			size_t cbLen = _tcslen(ptszDest) - 1;
			if (pNewName[cbLen] == '*')
				_tcsncpy_s(pNewName + cbLen, MAX_PATH - cbLen, ptszOldName, _TRUNCATE);
		}
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool isValidExtension(const TCHAR *ptszFileName)
{
	const TCHAR *pExt = _tcsrchr(ptszFileName, '.');

	return (pExt != NULL) && (!_tcsicmp(pExt, _T(".dll")) || !_tcsicmp(pExt, _T(".exe")) || !_tcsicmp(pExt, _T(".txt")));
}

static int ScanFolder(const TCHAR *tszFolder, size_t cbBaseLen, int level, const TCHAR *tszBaseUrl, SERVLIST& hashes, OBJLIST<FILEINFO> *UpdateFiles)
{
	int count = 0;

	// skip updater's own folder
	if (!_tcsicmp(tszFolder, tszRoot))
		return count;

	TCHAR tszBuf[MAX_PATH];
	mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\*"), tszFolder);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(tszBuf, &ffd);
	if (hFind == INVALID_HANDLE_VALUE)
		return count;

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (!_tcscmp(ffd.cFileName, _T(".")) || !_tcscmp(ffd.cFileName, _T("..")))
				continue;

			// we need to skip profile folder
			TCHAR tszProfilePath[MAX_PATH];
			CallService(MS_DB_GETPROFILEPATHT, SIZEOF(tszProfilePath), (LPARAM)tszProfilePath);

			mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\%s"), tszFolder, ffd.cFileName);
			if (0 != _tcsicmp(tszBuf, tszProfilePath))
				count += ScanFolder(tszBuf, cbBaseLen, level+1, tszBaseUrl, hashes, UpdateFiles);
			continue;
		}

		if (!isValidExtension(ffd.cFileName))
			continue;

		// calculate the current file's relative name and store it into tszNewName
		TCHAR tszNewName[MAX_PATH];
		if (!CheckFileRename(ffd.cFileName, tszNewName)) {
			if (level == 0)
				_tcsncpy(tszNewName, ffd.cFileName, MAX_PATH);
			else
				mir_sntprintf(tszNewName, SIZEOF(tszNewName), _T("%s\\%s"), tszFolder+cbBaseLen, ffd.cFileName);
		}

		bool bHasNewVersion = true;
		TCHAR *ptszUrl;
		int MyCRC = 0;
		mir_sntprintf(tszBuf, SIZEOF(tszBuf), _T("%s\\%s"), tszFolder, ffd.cFileName);

		// this file is not marked for deletion
		if (tszNewName[0]) {
			TCHAR *pName = tszNewName;
			ServListEntry *item = hashes.find((ServListEntry*)&pName);
			if (item == NULL) {
				TCHAR *p = _tcsrchr(tszNewName, '.');
				if (p[-1] != 'w' && p[-1] != 'W')
					continue;

				int iPos = int(p - tszNewName)-1;
				strdel(p-1, 1);
				if ((item = hashes.find((ServListEntry*)&pName)) == NULL)
					continue;

				strdel(tszNewName+iPos, 1);
			}

			ptszUrl = item->m_name;
			// No need to hash a file if we are forcing a redownload anyway
			if (!opts.bForceRedownload) {
				// try to hash the file
				char szMyHash[33];
				__try {
					CalculateModuleHash(tszBuf, szMyHash);
					bHasNewVersion = strcmp(szMyHash, item->m_szHash) != 0;
				}
				__except(EXCEPTION_EXECUTE_HANDLER) {
					ZeroMemory(szMyHash, 0);
					// smth went wrong, reload a file from scratch
				}
			}

			MyCRC = item->m_crc;
		}
		else // file was marked for deletion, add it to the list anyway
			ptszUrl = _T("");

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
				_tcsncpy(FileInfo->tszNewName, ptszUrl, SIZEOF(FileInfo->tszNewName));
			}

			_tcscpy(tszBuf, ptszUrl);
			TCHAR *p = _tcsrchr(tszBuf, '.');
			if (p) *p = 0;
			p = _tcsrchr(tszBuf, '\\');
			p = (p) ? p+1 : tszBuf;
			_tcslwr(p);

			mir_sntprintf(FileInfo->File.tszDiskPath, SIZEOF(FileInfo->File.tszDiskPath), _T("%s\\Temp\\%s.zip"), tszRoot, p);
			mir_sntprintf(FileInfo->File.tszDownloadURL, SIZEOF(FileInfo->File.tszDownloadURL), _T("%s/%s.zip"), tszBaseUrl, tszBuf);
			for (p = _tcschr(FileInfo->File.tszDownloadURL, '\\'); p != 0; p = _tcschr(p, '\\'))
				*p++ = '/';

			FileInfo->File.CRCsum = MyCRC;
			UpdateFiles->insert(FileInfo);
			
			if (!opts.bSilent || db_get_b(NULL, MODNAME "Files", StrToLower(_T2A(FileInfo->tszNewName)), true))
				count++;
		} // end compare versions
	}
		while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
	return count;
}

static void CheckUpdates(void *)
{
	char szKey[64] = {0};

	TCHAR tszTempPath[MAX_PATH];
	DWORD dwLen = GetTempPath(SIZEOF(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen-1] == '\\')
		tszTempPath[dwLen-1] = 0;

	ptrT updateUrl( GetDefaultUrl()), baseUrl;
	
	SERVLIST hashes(50, CompareHashes);
	bool success = ParseHashes(updateUrl, baseUrl, hashes);
	if (success) {
		FILELIST *UpdateFiles = new FILELIST(20);
		VARST dirname( _T("%miranda_path%"));
		int count = ScanFolder(dirname, lstrlen(dirname)+1, 0, baseUrl, hashes, UpdateFiles);

		// Show dialog
		if (count == 0) {
			if (!opts.bSilent)
				ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("No updates found."), 2, 0);
			delete UpdateFiles;
		}
		else CallFunctionAsync(LaunchDialog, UpdateFiles);
	}
	InitTimer(success ? 0 : 2);	
	
	hashes.destroy();
	hCheckThread = NULL;
}

void DoCheck(int iFlag)
{
	if (hCheckThread)
		ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("Update checking already started!"), 2, 0);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	} else if (iFlag) {
		db_set_dw(NULL, MODNAME, "LastUpdate", time(NULL));
		hCheckThread = mir_forkthread(CheckUpdates, 0);		
	}
}

void UninitCheck()
{
	if (hwndDialog != NULL)
		DestroyWindow(hwndDialog);
}
