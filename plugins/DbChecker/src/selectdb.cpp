/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

static bool CheckBroken(const wchar_t *ptszFullPath)
{
	DATABASELINK *dblink = FindDatabasePlugin(ptszFullPath);
	if (dblink == NULL || dblink->CheckDB == NULL)
		return true;

	return dblink->grokHeader(ptszFullPath) != EGROKPRF_NOERROR;
}

int OpenDatabase(HWND hdlg, INT iNextPage)
{
	wchar_t tszMsg[1024];
	int error = 0;

	if (opts.dbChecker == NULL) {
		DATABASELINK* dblink = FindDatabasePlugin(opts.filename);
		if (dblink == NULL) {
			mir_snwprintf(tszMsg,
				TranslateT("Database Checker cannot find a suitable database plugin to open '%s'."),
				opts.filename);
		LBL_Error:
			MessageBox(hdlg, tszMsg, TranslateT("Error"), MB_OK | MB_ICONERROR);
			return false;
		}

		if (dblink->CheckDB == NULL) {
			mir_snwprintf(tszMsg,
				TranslateT("Database driver '%s' doesn't support checking."),
				TranslateW(dblink->szFullName));
			goto LBL_Error;
		}

		opts.dbChecker = dblink->CheckDB(opts.filename, &error);
		if (opts.dbChecker == NULL) {
			if ((opts.error = GetLastError()) == 0)
				opts.error = error;
			PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_OPENERROR, (LPARAM)OpenErrorDlgProc);
			return true;
		}
		opts.dblink = dblink;
	}

	// force check
	if (error == EGROKPRF_OBSOLETE) {
		opts.bAggressive = opts.bBackup = true;
		PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressDlgProc);
	}
	else if (iNextPage == IDD_FILEACCESS)
		PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_FILEACCESS, (LPARAM)FileAccessDlgProc);
	else
		PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressDlgProc);
	return true;
}

void GetProfileDirectory(wchar_t* szMirandaDir, wchar_t* szPath, int cbPath)
{
	wchar_t szProfileDir[MAX_PATH], szExpandedProfileDir[MAX_PATH], szMirandaBootIni[MAX_PATH];

	mir_wstrcpy(szMirandaBootIni, szMirandaDir);
	mir_wstrcat(szMirandaBootIni, L"\\mirandaboot.ini");
	GetPrivateProfileString(L"Database", L"ProfileDir", L"./Profiles", szProfileDir, _countof(szProfileDir), szMirandaBootIni);
	ExpandEnvironmentStrings(szProfileDir, szExpandedProfileDir, _countof(szExpandedProfileDir));
	_wchdir(szMirandaDir);
	if (!_wfullpath(szPath, szExpandedProfileDir, cbPath))
		mir_wstrncpy(szPath, szMirandaDir, cbPath);
	if (szPath[mir_wstrlen(szPath) - 1] == '\\')
		szPath[mir_wstrlen(szPath) - 1] = 0;
}

static int AddDatabaseToList(HWND hwndList, const wchar_t* filename, wchar_t* dir)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	for (lvi.iItem = ListView_GetItemCount(hwndList) - 1; lvi.iItem >= 0; lvi.iItem--) {
		ListView_GetItem(hwndList, &lvi);
		if (!mir_wstrcmpi((wchar_t*)lvi.lParam, filename))
			return lvi.iItem;
	}

	struct _stat st;
	if (_wstat(filename, &st) == -1)
		return -1;

	DWORD totalSize = st.st_size;

	bool isBroken = CheckBroken(filename);

	const wchar_t *pName = wcsrchr(filename, '\\');
	if (pName == NULL)
		pName = (LPTSTR)filename;
	else
		pName++;

	wchar_t szName[MAX_PATH];
	mir_snwprintf(szName, L"%s%s", dir, pName);

	wchar_t *pDot = wcsrchr(szName, '.');
	if (pDot != NULL && !mir_wstrcmpi(pDot, L".dat"))
		*pDot = 0;

	lvi.iItem = 0;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)wcsdup(filename);
	lvi.pszText = szName;
	lvi.iImage = (isBroken) ? 1 : 0;

	int iNewItem = ListView_InsertItem(hwndList, &lvi);
	wchar_t szSize[20];
	mir_snwprintf(szSize, L"%.2lf MB", totalSize / 1048576.0);
	ListView_SetItemText(hwndList, iNewItem, 1, szSize);
	return iNewItem;
}

void FindAdd(HWND hdlg, wchar_t *szProfileDir, wchar_t *szPrefix)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	wchar_t szSearchPath[MAX_PATH], szFilename[MAX_PATH];

	mir_wstrcpy(szSearchPath, szProfileDir);
	mir_wstrcat(szSearchPath, L"\\*.*");

	hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
				continue;

			mir_snwprintf(szFilename, L"%s\\%s\\%s.dat", szProfileDir, fd.cFileName, fd.cFileName);
			if (_waccess(szFilename, 0) == 0)
				AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), szFilename, szPrefix);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

wchar_t *addstring(wchar_t *str, wchar_t *add)
{
	mir_wstrcpy(str, add);
	return str + mir_wstrlen(add) + 1;
}

INT_PTR CALLBACK SelectDbDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	INT_PTR bReturn;
	if (DoMyControlProcessing(hdlg, message, wParam, lParam, &bReturn))
		return bReturn;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
			ImageList_AddIcon(hIml, LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROFILEGREEN)));
			ImageList_AddIcon(hIml, LoadIcon(hInst, MAKEINTRESOURCE(IDI_BAD)));
			ListView_SetImageList(GetDlgItem(hdlg, IDC_DBLIST), hIml, LVSIL_SMALL);
		}
		ListView_SetExtendedListViewStyleEx(GetDlgItem(hdlg, IDC_DBLIST), LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		{
			LV_COLUMN lvc;
			lvc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;
			lvc.cx = 290;
			lvc.fmt = LVCFMT_LEFT;
			lvc.pszText = TranslateT("Database");
			ListView_InsertColumn(GetDlgItem(hdlg, IDC_DBLIST), 0, &lvc);
			lvc.cx = 68;
			lvc.fmt = LVCFMT_RIGHT;
			lvc.pszText = TranslateT("Total size");
			ListView_InsertColumn(GetDlgItem(hdlg, IDC_DBLIST), 1, &lvc);

			wchar_t szMirandaPath[MAX_PATH];
			GetModuleFileName(NULL, szMirandaPath, _countof(szMirandaPath));
			wchar_t *str2 = wcsrchr(szMirandaPath, '\\');
			if (str2 != NULL)
				*str2 = 0;

			int i = 0;
			HKEY hKey;
			wchar_t szProfileDir[MAX_PATH];
			wchar_t szMirandaProfiles[MAX_PATH];
			DWORD cbData = _countof(szMirandaPath);

			mir_wstrcpy(szMirandaProfiles, szMirandaPath);
			mir_wstrcat(szMirandaProfiles, L"\\Profiles");
			GetProfileDirectory(szMirandaPath, szProfileDir, _countof(szProfileDir));

			// search in profile dir (using ini file)
			if (mir_wstrcmpi(szProfileDir, szMirandaProfiles))
				FindAdd(hdlg, szProfileDir, L"[ini]\\");

			FindAdd(hdlg, szMirandaProfiles, L"[prf]\\");
			// search in current dir (as DBTOOL)
			FindAdd(hdlg, szMirandaPath, L"[.]\\");

			// search in profile dir (using registry path + ini file)
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\miranda32.exe", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
				if (RegQueryValueEx(hKey, L"Path", NULL, NULL, (PBYTE)szMirandaPath, &cbData) == ERROR_SUCCESS) {
					if (mir_wstrcmp(szProfileDir, szMirandaPath)) {
						GetProfileDirectory(szMirandaPath, szProfileDir, _countof(szProfileDir));
						FindAdd(hdlg, szProfileDir, L"[reg]\\");
					}
				}
				RegCloseKey(hKey);
			}
			// select
			if (opts.filename[0])
				i = AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), opts.filename, L"");
			if (i == -1)
				i = 0;
			ListView_SetItemState(GetDlgItem(hdlg, IDC_DBLIST), i, LVIS_SELECTED, LVIS_SELECTED);
		}

		if (opts.dbChecker != NULL) {
			opts.dbChecker->Destroy();
			opts.dbChecker = NULL;
		}

		if (bShortMode)
			if (!OpenDatabase(hdlg, IDD_FILEACCESS)) {
				EndDialog(GetParent(hdlg), 1);
				return FALSE;
			}
		return TRUE;

	case WZN_PAGECHANGING:
		GetDlgItemText(hdlg, IDC_FILE, opts.filename, _countof(opts.filename));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FILE:
			if (HIWORD(wParam) == EN_CHANGE)
				EnableWindow(GetDlgItem(GetParent(hdlg), IDOK), GetWindowTextLength(GetDlgItem(hdlg, IDC_FILE)));
			break;

		case IDC_OTHER:
		{
			OPENFILENAME ofn = { 0 };
			wchar_t str[MAX_PATH];

			// L"Miranda Databases (*.dat)\0*.DAT\0All Files (*)\0*\0";
			wchar_t *filter, *tmp, *tmp1, *tmp2;
			tmp1 = TranslateT("Miranda Databases (*.dat)");
			tmp2 = TranslateT("All Files");
			filter = tmp = (wchar_t*)_alloca((mir_wstrlen(tmp1) + mir_wstrlen(tmp2) + 11)*sizeof(wchar_t));
			tmp = addstring(tmp, tmp1);
			tmp = addstring(tmp, L"*.DAT");
			tmp = addstring(tmp, tmp2);
			tmp = addstring(tmp, L"*");
			*tmp = 0;

			GetDlgItemText(hdlg, IDC_FILE, str, _countof(str));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hdlg;
			ofn.hInstance = NULL;
			ofn.lpstrFilter = filter;
			ofn.lpstrDefExt = L"dat";
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = _countof(str);
			ofn.nMaxFileTitle = MAX_PATH;
			if (GetOpenFileName(&ofn)) {
				int i = AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), str, L"");
				if (i == -1)
					i = 0;
				ListView_SetItemState(GetDlgItem(hdlg, IDC_DBLIST), i, LVIS_SELECTED, LVIS_SELECTED);
			}
		}
		break;

		case IDC_BACK:
			if (!bShortMode)
				PostMessage(GetParent(hdlg), WZM_GOTOPAGE, IDD_WELCOME, (LPARAM)WelcomeDlgProc);
			break;

		case IDOK:
			OpenDatabase(hdlg, IDD_FILEACCESS);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_DBLIST:
			switch (((LPNMLISTVIEW)lParam)->hdr.code) {
			case LVN_ITEMCHANGED:
				LV_ITEM lvi;
				lvi.iItem = ListView_GetNextItem(GetDlgItem(hdlg, IDC_DBLIST), -1, LVNI_SELECTED);
				if (lvi.iItem == -1) break;
				lvi.mask = LVIF_PARAM;
				ListView_GetItem(GetDlgItem(hdlg, IDC_DBLIST), &lvi);
				SetDlgItemText(hdlg, IDC_FILE, (wchar_t*)lvi.lParam);
				SendMessage(hdlg, WM_COMMAND, MAKEWPARAM(IDC_FILE, EN_CHANGE), (LPARAM)GetDlgItem(hdlg, IDC_FILE));
			}
			break;
		}
		break;

	case WM_DESTROY:
	{
		LV_ITEM lvi;
		lvi.mask = LVIF_PARAM;
		for (lvi.iItem = ListView_GetItemCount(GetDlgItem(hdlg, IDC_DBLIST)) - 1; lvi.iItem >= 0; lvi.iItem--) {
			ListView_GetItem(GetDlgItem(hdlg, IDC_DBLIST), &lvi);
			free((char*)lvi.lParam);
		}
	}
	break;
	}
	return FALSE;
}
