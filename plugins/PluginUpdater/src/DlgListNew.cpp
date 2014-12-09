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

static HWND hwndDialog;
HANDLE hListThread;

static void SelectAll(HWND hDlg, bool bEnable)
{
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	for (int i=0; i < todo.getCount(); i++) {
		ListView_SetCheckState(hwndList, i, todo[i].bEnabled = bEnable);
	}
}

static void ApplyDownloads(void *param)
{
	HWND hDlg = (HWND)param;

	//////////////////////////////////////////////////////////////////////////////////////
	// if we need to escalate priviledges, launch a atub

	if (!PrepareEscalation()) {
		PostMessage(hDlg, WM_CLOSE, 0, 0);
		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// ok, let's unpack all zips

	AutoHandle pipe(hPipe);
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);
	OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	//create needed folders after escalating priviledges. Folders creates when we actually install updates
	TCHAR tszFileTemp[MAX_PATH], tszFileBack[MAX_PATH];

	mir_sntprintf(tszFileBack, SIZEOF(tszFileBack), _T("%s\\Backups"), tszRoot);
	SafeCreateDirectory(tszFileBack);

	mir_sntprintf(tszFileTemp, SIZEOF(tszFileTemp), _T("%s\\Temp"), tszRoot);
	SafeCreateDirectory(tszFileTemp);

	VARST tszMirandaPath(_T("%miranda_path%"));

	HANDLE nlc = NULL;
	for (int i=0; i < todo.getCount(); ++i) {
		ListView_EnsureVisible(hwndList, i, FALSE);
		if (todo[i].bEnabled) {
			// download update
			ListView_SetItemText(hwndList, i, 1, TranslateT("Downloading..."));

			if (DownloadFile(&todo[i].File, nlc)) {
				ListView_SetItemText(hwndList, i, 1, TranslateT("Succeeded."));
				if (unzip(todo[i].File.tszDiskPath, tszMirandaPath, tszFileBack,false))
					SafeDeleteFile(todo[i].File.tszDiskPath);  // remove .zip after successful update
			}
			else
				ListView_SetItemText(hwndList, i, 1, TranslateT("Failed!"));
		}
		else
			ListView_SetItemText(hwndList, i, 1, TranslateT("Skipped."));
	}
	Netlib_CloseHandle(nlc);

	ShowPopup(TranslateT("Plugin Updater"), TranslateT("Download complete"), POPUP_TYPE_INFO);

	int rc = MessageBox(hDlg, TranslateT("Download complete. Do you want to go to plugins option page?"), TranslateT("Plugin Updater"), MB_YESNO | MB_ICONQUESTION);
	if (rc == IDYES)
		CallFunctionAsync(OpenPluginOptions, 0);

	PostMessage(hDlg, WM_CLOSE, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

static WNDPROC oldWndProc = NULL;

static LRESULT CALLBACK PluginListWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN) {
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam); hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwnd, &hi);
		if ((hi.iSubItem == 0) && (hi.flags & LVHT_ONITEMICON)) {
			LVITEM lvi = {0};
			lvi.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_GROUPID;
			lvi.stateMask = -1;
			lvi.iItem = hi.iItem;
			if (ListView_GetItem(hwnd, &lvi) && lvi.iGroupId == 1) {
				FILEINFO *info = (FILEINFO *)lvi.lParam;

				TCHAR tszFileName[MAX_PATH];
				_tcscpy(tszFileName, _tcsrchr(info->tszNewName, L'\\') + 1);
				TCHAR *p = _tcschr(tszFileName, L'.'); *p = 0;

				TCHAR link[MAX_PATH];
				mir_sntprintf(link, SIZEOF(link), _T("http://miranda-ng.org/p/%s"), tszFileName);
				CallService(MS_UTILS_OPENURL, OUF_TCHAR, (LPARAM) link);
			}
		}
	}

	return CallWindowProc(oldWndProc, hwnd, msg, wParam, lParam);
}

static int ListDlg_Resize(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_SELNONE:
	case IDOK:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_UPDATETEXT:
		return RD_ANCHORX_CENTRE;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
}

INT_PTR CALLBACK DlgList(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hDlg, IDC_LIST_UPDATES);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hDlg );
		oldWndProc = (WNDPROC)SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)PluginListWndProc);

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

			lvc.pszText = TranslateT("State");
			lvc.cx = 100; // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

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
			
			lvg.pszHeader = TranslateT("Languages");
			lvg.iGroupId = 3;
			ListView_InsertGroup(hwndList, 0, &lvg);

			lvg.pszHeader = TranslateT("Other");
			lvg.iGroupId = 4;
			ListView_InsertGroup(hwndList, 0, &lvg);

			ListView_EnableGroupView(hwndList, TRUE);

			///
			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);
			ListView_DeleteAllItems(hwndList);

			///
			OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) {
				LVITEM lvi = {0};
				lvi.mask = LVIF_PARAM | LVIF_GROUPID | LVIF_TEXT | LVIF_IMAGE;
				
				int groupId = 4;
				if (_tcschr(todo[i].tszOldName, L'\\') != NULL)
					groupId = (_tcsstr(todo[i].tszOldName, _T("Plugins")) != NULL) ? 1 : ((_tcsstr(todo[i].tszOldName, _T("Languages")) != NULL) ? 3 : 2);

				lvi.iItem = i;
				lvi.lParam = (LPARAM)&todo[i];
				lvi.iGroupId = groupId;
				lvi.iImage = ((groupId ==1) ? 0 : -1);
				lvi.pszText = todo[i].tszOldName;
				ListView_InsertItem(hwndList, &lvi);
			}
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
					if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						LVITEM lvI = {0};
						lvI.iItem = nmlv->iItem;
						lvI.iSubItem = 0;
						lvI.mask = LVIF_PARAM;
						ListView_GetItem(hwndList, &lvI);

						OBJLIST<FILEINFO> &todo = *(OBJLIST<FILEINFO> *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
						FILEINFO *p = (FILEINFO*)lvI.lParam;
						p->bEnabled = ListView_GetCheckState(hwndList, nmlv->iItem);

						bool enableOk = false;
						for (int i=0; i < todo.getCount(); ++i) {
							if (todo[i].bEnabled) {
								enableOk = true;
								break;
							}
						}
						EnableWindow(GetDlgItem(hDlg, IDOK), enableOk ? TRUE : FALSE);
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
				EnableWindow( GetDlgItem(hDlg, IDC_SELNONE), FALSE);

				mir_forkthread(ApplyDownloads, hDlg);
				return TRUE;

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
		if (!IsIconic(hDlg)) {
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = hInst;
			urd.hwndDlg = hDlg;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_LIST);
			urd.pfnResizer = ListDlg_Resize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		break;

	case WM_GETMINMAXINFO: 
		{
			LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;

			// The minimum width in points
			mmi->ptMinTrackSize.x = 370;
			// The minimum height in points
			mmi->ptMinTrackSize.y = 300;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
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
	hwndDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LIST), GetDesktopWindow(), DlgList, (LPARAM)param);
}

/////////////////////////////////////////////////////////////////////////////////////////
// building file list in the separate thread

static void GetList(void *)
{
	TCHAR tszTempPath[MAX_PATH];
	DWORD dwLen = GetTempPath(SIZEOF(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen-1] == '\\')
		tszTempPath[dwLen-1] = 0;

	ptrT updateUrl( GetDefaultUrl()), baseUrl;
	SERVLIST hashes(50, CompareHashes);
	if (!ParseHashes(updateUrl, baseUrl, hashes)) {
		hListThread = NULL;
		return;
	}

	FILELIST *UpdateFiles = new FILELIST(20);
	VARST dirname(_T("%miranda_path%"));

	for (int i=0; i < hashes.getCount(); i++) {
		ServListEntry &hash = hashes[i];

		TCHAR tszPath[MAX_PATH];
		mir_sntprintf(tszPath, SIZEOF(tszPath), _T("%s\\%s"), dirname, hash.m_name);

		if (GetFileAttributes(tszPath) == INVALID_FILE_ATTRIBUTES) {
			FILEINFO *FileInfo = new FILEINFO;
			FileInfo->bDeleteOnly = FALSE;
			// copy the relative old name
			_tcsncpy(FileInfo->tszOldName, hash.m_name, SIZEOF(FileInfo->tszOldName));
			_tcsncpy(FileInfo->tszNewName, hash.m_name, SIZEOF(FileInfo->tszNewName));

			TCHAR tszFileName[MAX_PATH];
			_tcsncpy(tszFileName, _tcsrchr(tszPath, L'\\') + 1, SIZEOF(tszFileName));
			TCHAR *tp = _tcschr(tszFileName, L'.'); *tp = 0;

			TCHAR tszRelFileName[MAX_PATH];
			_tcsncpy(tszRelFileName, hash.m_name, MAX_PATH);
			tp = _tcsrchr(tszRelFileName, L'.'); if (tp) *tp = 0;
			tp = _tcschr(tszRelFileName, L'\\'); if (tp) tp++; else tp = tszRelFileName;
			_tcslwr(tp);

			mir_sntprintf(FileInfo->File.tszDiskPath, SIZEOF(FileInfo->File.tszDiskPath), _T("%s\\Temp\\%s.zip"), tszRoot, tszFileName);
			mir_sntprintf(FileInfo->File.tszDownloadURL, SIZEOF(FileInfo->File.tszDownloadURL), _T("%s/%s.zip"), baseUrl, tszRelFileName);
			for (tp = _tcschr(FileInfo->File.tszDownloadURL, '\\'); tp != 0; tp = _tcschr(tp, '\\'))
				*tp++ = '/';
			FileInfo->File.CRCsum = hash.m_crc;
			// Deselect all plugins by default
			FileInfo->bEnabled = false;
			UpdateFiles->insert(FileInfo);
		}
	}

	// Show dialog
	if (UpdateFiles->getCount() == 0) {
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("List is empty."), POPUP_TYPE_INFO);
		delete UpdateFiles;
	}
	else CallFunctionAsync(LaunchListDialog, UpdateFiles);

	hListThread = NULL;
}

void DoGetList()
{
	if (hListThread)
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("List loading already started!"), POPUP_TYPE_INFO);
	else if (hwndDialog) {
		ShowWindow(hwndDialog, SW_SHOW);
		SetForegroundWindow(hwndDialog);
		SetFocus(hwndDialog);
	}
	else
		hListThread = mir_forkthread(GetList, 0);
}

void UninitListNew()
{
	if (hwndDialog != NULL)
		DestroyWindow(hwndDialog);
}

INT_PTR ShowListCommand(WPARAM,LPARAM)
{
	DoGetList();
	return 0;
}

void InitListNew()
{
	CreateServiceFunction(MODNAME"/ShowList", ShowListCommand);
}

void UnloadListNew()
{
	if (hListThread)
		hListThread = NULL;
}