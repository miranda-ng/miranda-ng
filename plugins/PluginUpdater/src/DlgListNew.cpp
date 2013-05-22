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

static void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, bEnable);
		todo[i].bEnabled = bEnable;
	}
}

static void ApplyDownloads(void *param)
{
	HWND hDlg = (HWND)param;

	//////////////////////////////////////////////////////////////////////////////////////
	// if we need to escalate priviledges, launch a atub

	if ( !PrepareEscalation()) {
		DestroyWindow(hDlg);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ok, let's unpack all zips

	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	TCHAR tszBuff[2048], tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	SafeCreateDirectory(tszFileTemp);

	for (int i=0; i < todo.getCount(); ++i) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if ( !todo[i].bEnabled) {
			ListView_SetItemText(hwndList, i, 2, TranslateT("Skipped."));
			continue;
		}

		// download update
		ListView_SetItemText(hwndList, i, 2, TranslateT("Downloading..."));

		FILEURL *pFileUrl = &todo[i].File;
		if ( !DownloadFile(pFileUrl->tszDownloadURL, pFileUrl->tszDiskPath, pFileUrl->CRCsum)){
			ListView_SetItemText(hwndList, i, 2, TranslateT("Failed!"));
		}
		else
			ListView_SetItemText(hwndList, i, 2, TranslateT("Succeeded."));
	}

	if (todo.getCount() > 0) {
		ShowPopup(0, LPGENT("Plugin Updater"), TranslateT("Download complete"), 2, 0);

		TCHAR *tszMirandaPath = Utils_ReplaceVarsT(_T("%miranda_path%"));

		for (int i = 0; i < todo.getCount(); i++) {
			if ( !todo[i].bEnabled)
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

			if ( unzip(p.File.tszDiskPath, tszMirandaPath, tszFileBack))
				SafeDeleteFile(p.File.tszDiskPath);  // remove .zip after successful update
		}
	}

	PopupDataText temp;
	temp.Title = TranslateT("Plugin Updater");
	temp.Text = tszBuff;
	lstrcpyn(tszBuff, TranslateT("Download complete. Do you want go to plugins option page?"), SIZEOF(tszBuff));
	int rc = MessageBox(NULL, temp.Text, temp.Title, MB_YESNO | MB_ICONQUESTION);
	if (rc == IDYES)
		CallFunctionAsync(OpenPluginOptions, 0);

	if (hPipe)
		CloseHandle(hPipe);
	CloseWindow(hDlg);
	DestroyWindow(hDlg);
	hwndDialog = NULL;
	return;
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK PluginListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN) {
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam); hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		if (hi.iSubItem == 1) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_GROUPID;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			if (ListView_GetItem(hwnd, &lvi) && lvi.iGroupId == 1) {
				TCHAR link[MAX_PATH];
				FILEINFO *info = (FILEINFO *)lvi.lParam;

				TCHAR tszFileName[MAX_PATH];
				_tcscpy(tszFileName, _tcsrchr(info->tszNewName, L'\\') + 1);
				TCHAR *p = _tcschr(tszFileName, L'.'); *p = 0;

				mir_sntprintf(link, MAX_PATH, L"http://wiki.miranda-ng.org/index.php?title=Plugin:%s", tszFileName);
				CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM) link);
			}
		}
	}

	return mir_callNextSubclass(hwnd, PluginListWndProc, msg, wParam, lParam);
}

static int ListDlg_Resize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_SELALL:
	case IDC_SELNONE:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDOK:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
}

INT_PTR CALLBACK DlgList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		hwndDialog = hDlg;
		TranslateDialogDefault( hDlg );
		mir_subclassWindow(hwndList, PluginListWndProc);

		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon("plg_list", 1));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIcon("plg_list"));
		{
			HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 4, 0);
			ImageList_AddIconFromIconLib(hIml, "info");
			ListView_SetImageList(hwndList, hIml, LVSIL_SMALL);

			OSVERSIONINFO osver = { sizeof(osver) };
			if (GetVersionEx(&osver) && osver.dwMajorVersion >= 6) {
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
				else {
					CloseHandle(hFile);
					DeleteFile(szPath);
				}
			}
			RECT r;
			GetClientRect(hwndList, &r);

			///
			LVCOLUMN lvc = {0};
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			//lvc.fmt = LVCFMT_LEFT;

			lvc.pszText = TranslateT("Component Name");
			lvc.cx = 220; // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.pszText = L"";
			lvc.cx = 32 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.pszText = TranslateT("State");
			lvc.cx = 100 - GetSystemMetrics(SM_CXVSCROLL); // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);

			///
			LVGROUP lvg;
			lvg.cbSize = sizeof(LVGROUP);
			lvg.mask = LVGF_HEADER | LVGF_GROUPID;

			lvg.pszHeader = TranslateT("Plugins");
			lvg.iGroupId = 1;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Icons");
			lvg.iGroupId = 2;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Other");
			lvg.iGroupId = 3;
			ListView_InsertGroup(hwndList, 0, &lvg);

			ListView_EnableGroupView(hwndList, TRUE);

			///
			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);
			ListView_DeleteAllItems(hwndList);			

			///
			OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) {
				int groupId = 3;
				LVITEM lvi = {0};				
				lvi.mask = LVIF_PARAM | LVIF_GROUPID | LVIF_TEXT | LVIF_IMAGE;
				
				if (_tcschr(todo[i].tszOldName, L'\\') != NULL)
					groupId = _tcsstr(todo[i].tszOldName, L"Plugins") != NULL ? 1 : 2;

				lvi.iItem = i;
				lvi.lParam = (LPARAM)&todo[i];
				lvi.iGroupId = groupId;
				lvi.iImage   = -1;
				lvi.pszText = todo[i].tszOldName;
				int iRow = ListView_InsertItem(hwndList, &lvi);


				if (iRow != -1) {
					lvi.iItem = iRow;
					if (groupId == 1) {
						lvi.mask = LVIF_IMAGE;
						lvi.iSubItem = 1;
						lvi.iImage = 0;
						ListView_SetItem(hwndList, &lvi);
					}
				}
				todo[i].bEnabled = false;
			}
			HWND hwOk = GetDlgItem(hDlg, IDOK);
			EnableWindow(hwOk, false);
		}

		// do this after filling list - enables 'ITEMCHANGED' below
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		Utils_RestoreWindowPosition(hDlg, 0, MODNAME, "ListWindow");
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

				mir_forkthread(ApplyDownloads, hDlg);
				return TRUE;

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

	case WM_SIZE: // make the dlg resizeable
		if ( !IsIconic(hDlg)) {
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = hInst;
			urd.hwndDlg = hDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_LIST);
			urd.pfnResizer = ListDlg_Resize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hDlg, NULL, MODNAME, "ListWindow");
		Skin_ReleaseIcon((HICON)SendMessage(hDlg, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hDlg, WM_SETICON, ICON_SMALL, 0));
		hwndDialog = NULL;
		delete (OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, 0);
		break;
	}

	return FALSE;
}

static void __stdcall LaunchListDialog(void *param)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LIST), GetDesktopWindow(), DlgList, (LPARAM)param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// building file list in the separate thread

static void GetList(void *)
{
	char szKey[64] = {0};
	DBVARIANT dbVar = {0};
	
	TCHAR tszTempPath[MAX_PATH];
	DWORD dwLen = GetTempPath(SIZEOF(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen-1] == '\\')
		tszTempPath[dwLen-1] = 0;

	// Load files info
	if (db_get_ts(NULL, MODNAME, "UpdateURL", &dbVar)) { // URL is not set
		db_set_ts(NULL, MODNAME, "UpdateURL", _T(DEFAULT_UPDATE_URL));
		db_get_ts(NULL, MODNAME, "UpdateURL", &dbVar);
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
	db_free(&dbVar);

	// Download version info
	ShowPopup(NULL, TranslateT("Plugin Updater"), TranslateT("Downloading version info..."), 4, 0);

	FILEURL pFileUrl;
	mir_sntprintf(pFileUrl.tszDownloadURL, SIZEOF(pFileUrl.tszDownloadURL), _T("%s/hashes.zip"), (TCHAR*)tszBaseUrl);
	mir_sntprintf(pFileUrl.tszDiskPath, SIZEOF(pFileUrl.tszDiskPath), _T("%s\\hashes.zip"), tszTempPath);
	if (!DownloadFile(pFileUrl.tszDownloadURL, pFileUrl.tszDiskPath, 0)) {
		ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("An error occured while downloading the update."), 1, 0);
		hListThread = NULL;
		return;
	}

	unzip(pFileUrl.tszDiskPath, tszTempPath, NULL);
	DeleteFile(pFileUrl.tszDiskPath);

	TCHAR tszTmpIni[MAX_PATH] = {0};
	mir_sntprintf(tszTmpIni, SIZEOF(tszTmpIni), _T("%s\\hashes.txt"), tszTempPath);
	FILE *fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp) {
		hListThread = NULL;
		return;
	}

	FILELIST *UpdateFiles = new FILELIST(20);
	char str[200];
	TCHAR *dirname = Utils_ReplaceVarsT(_T("%miranda_path%"));
	while(fgets(str, SIZEOF(str), fp) != NULL) {
		rtrim(str);
		char *p = strchr(str, ' ');
		if (p == NULL)
			continue;

		*p++ = 0;
		_strlwr(p);

		int dwCrc32;
		char *p1 = strchr(p, ' ');
		if (p1 == NULL)
			dwCrc32 = 0;
		else {
			*p1++ = 0;
			sscanf(p1, "%08x", &dwCrc32);
		}
		ServListEntry hash(str, p, dwCrc32);

		TCHAR tszPath[MAX_PATH];
		mir_sntprintf(tszPath, SIZEOF(tszPath), _T("%s\\%s"), dirname, hash.m_name);
		
		if (GetFileAttributes(tszPath) != DWORD(-1))
			continue;

		FILEINFO *FileInfo = new FILEINFO;
		FileInfo->bDeleteOnly = FALSE;
		_tcscpy(FileInfo->tszOldName, hash.m_name); // copy the relative old name
		_tcscpy(FileInfo->tszNewName, hash.m_name);

		TCHAR tszFileName[MAX_PATH];
		_tcscpy(tszFileName, _tcsrchr(tszPath, L'\\') + 1);
		TCHAR *tp = _tcschr(tszFileName, L'.'); *tp = 0;

		TCHAR tszRelFileName[MAX_PATH];
		_tcscpy(tszRelFileName, hash.m_name);
		tp = _tcsrchr(tszRelFileName, L'.'); if (tp) *tp = 0;
		tp = _tcschr(tszRelFileName, L'\\'); if (tp) tp++; else tp = tszRelFileName;
		_tcslwr(tp);

		mir_sntprintf(FileInfo->File.tszDiskPath, SIZEOF(FileInfo->File.tszDiskPath), _T("%s\\Temp\\%s.zip"), tszRoot, tszFileName);
		mir_sntprintf(FileInfo->File.tszDownloadURL, SIZEOF(FileInfo->File.tszDownloadURL), _T("%s/%s.zip"), tszBaseUrl, tszRelFileName);
		for (tp = _tcschr(FileInfo->File.tszDownloadURL, '\\'); tp != 0; tp = _tcschr(tp, '\\'))
			*tp++ = '/';
		FileInfo->File.CRCsum = hash.m_crc;
		UpdateFiles->insert(FileInfo);
	}
	fclose(fp);
	DeleteFile(tszTmpIni);

	mir_free(dirname);

	// Show dialog
	if (UpdateFiles->getCount() == 0) {
		if ( !opts.bSilent)
			ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("List is empty."), 2, 0);
	}
	else CallFunctionAsync(LaunchListDialog, UpdateFiles);

	hListThread = NULL;
}

void DoGetList(int iFlag)
{
	if (hListThread)
		ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("List loading already started!"), 2, 0);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	}
	else if (iFlag)
		hListThread = mir_forkthread(GetList, 0);
}
