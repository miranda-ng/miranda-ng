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

static bool CheckBroken(const TCHAR *ptszFullPath)
{
	DATABASELINK *dblink = FindDatabasePlugin(ptszFullPath);
	if (dblink == NULL || dblink->CheckDB == NULL)
		return true;

	return dblink->grokHeader(ptszFullPath) != EGROKPRF_NOERROR;
}

int OpenDatabase(HWND hdlg, INT iNextPage)
{
	TCHAR tszMsg[1024];
	int error = 0;

	if (opts.dbChecker == NULL) {
		DATABASELINK* dblink = FindDatabasePlugin(opts.filename);
		if (dblink == NULL) {
			mir_sntprintf(tszMsg, SIZEOF(tszMsg),
				TranslateT("Database Checker cannot find a suitable database plugin to open '%s'."),
				opts.filename);
		LBL_Error:
			MessageBox(hdlg, tszMsg, TranslateT("Error"), MB_OK | MB_ICONERROR);
			return false;
		}

		if (dblink->CheckDB == NULL) {
			mir_sntprintf(tszMsg, SIZEOF(tszMsg),
				TranslateT("Database driver '%s' doesn't support checking."),
				TranslateTS(dblink->szFullName));
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

void GetProfileDirectory(TCHAR* szMirandaDir, TCHAR* szPath, int cbPath)
{
	TCHAR szProfileDir[MAX_PATH], szExpandedProfileDir[MAX_PATH], szMirandaBootIni[MAX_PATH];

	mir_tstrcpy(szMirandaBootIni, szMirandaDir);
	mir_tstrcat(szMirandaBootIni, _T("\\mirandaboot.ini"));
	GetPrivateProfileString(_T("Database"), _T("ProfileDir"), _T("./Profiles"), szProfileDir, SIZEOF(szProfileDir), szMirandaBootIni);
	ExpandEnvironmentStrings(szProfileDir, szExpandedProfileDir, SIZEOF(szExpandedProfileDir));
	_tchdir(szMirandaDir);
	if (!_tfullpath(szPath, szExpandedProfileDir, cbPath))
		mir_tstrncpy(szPath, szMirandaDir, cbPath);
	if (szPath[mir_tstrlen(szPath) - 1] == '\\')
		szPath[mir_tstrlen(szPath) - 1] = 0;
}

static int AddDatabaseToList(HWND hwndList, const TCHAR* filename, TCHAR* dir)
{
	LV_ITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	for (lvi.iItem = ListView_GetItemCount(hwndList) - 1; lvi.iItem >= 0; lvi.iItem--) {
		ListView_GetItem(hwndList, &lvi);
		if (!_tcsicmp((TCHAR*)lvi.lParam, filename))
			return lvi.iItem;
	}

	_stat st;
	if (_tstat(filename, &st) == -1)
		return -1;

	DWORD totalSize = st.st_size;

	bool isBroken = CheckBroken(filename);

	const TCHAR *pName = _tcsrchr(filename, '\\');
	if (pName == NULL)
		pName = (LPTSTR)filename;
	else
		pName++;

	TCHAR szName[MAX_PATH];
	mir_sntprintf(szName, SIZEOF(szName), _T("%s%s"), dir, pName);

	TCHAR *pDot = _tcsrchr(szName, '.');
	if (pDot != NULL && !_tcsicmp(pDot, _T(".dat")))
		*pDot = 0;

	lvi.iItem = 0;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iSubItem = 0;
	lvi.lParam = (LPARAM)_tcsdup(filename);
	lvi.pszText = szName;
	lvi.iImage = (isBroken) ? 1 : 0;

	int iNewItem = ListView_InsertItem(hwndList, &lvi);
	TCHAR szSize[20];
	mir_sntprintf(szSize, SIZEOF(szSize), _T("%.2lf MB"), totalSize / 1048576.0);
	ListView_SetItemText(hwndList, iNewItem, 1, szSize);
	return iNewItem;
}

void FindAdd(HWND hdlg, TCHAR *szProfileDir, TCHAR *szPrefix)
{
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	TCHAR szSearchPath[MAX_PATH], szFilename[MAX_PATH];

	mir_tstrcpy(szSearchPath, szProfileDir);
	mir_tstrcat(szSearchPath, _T("\\*.*"));

	hFind = FindFirstFile(szSearchPath, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !_tcscmp(fd.cFileName, _T(".")) || !_tcscmp(fd.cFileName, _T("..")))
				continue;

			mir_sntprintf(szFilename, SIZEOF(szFilename), _T("%s\\%s\\%s.dat"), szProfileDir, fd.cFileName, fd.cFileName);
			if (_taccess(szFilename, 0) == 0)
				AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), szFilename, szPrefix);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

TCHAR *addstring(TCHAR *str, TCHAR *add)
{
	_tcscpy(str, add);
	return str + _tcslen(add) + 1;
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

			TCHAR szMirandaPath[MAX_PATH];
			GetModuleFileName(NULL, szMirandaPath, SIZEOF(szMirandaPath));
			TCHAR *str2 = _tcsrchr(szMirandaPath, '\\');
			if (str2 != NULL)
				*str2 = 0;

			int i = 0;
			HKEY hKey;
			TCHAR szProfileDir[MAX_PATH];
			TCHAR szMirandaProfiles[MAX_PATH];
			DWORD cbData = SIZEOF(szMirandaPath);

			_tcscpy(szMirandaProfiles, szMirandaPath);
			_tcscat(szMirandaProfiles, _T("\\Profiles"));
			GetProfileDirectory(szMirandaPath, szProfileDir, SIZEOF(szProfileDir));

			// search in profile dir (using ini file)
			if (mir_tstrcmpi(szProfileDir, szMirandaProfiles))
				FindAdd(hdlg, szProfileDir, _T("[ini]\\"));

			FindAdd(hdlg, szMirandaProfiles, _T("[prf]\\"));
			// search in current dir (as DBTOOL)
			FindAdd(hdlg, szMirandaPath, _T("[.]\\"));

			// search in profile dir (using registry path + ini file)
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\miranda32.exe"), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
				if (RegQueryValueEx(hKey, _T("Path"), NULL, NULL, (PBYTE)szMirandaPath, &cbData) == ERROR_SUCCESS) {
					if (mir_tstrcmp(szProfileDir, szMirandaPath)) {
						GetProfileDirectory(szMirandaPath, szProfileDir, SIZEOF(szProfileDir));
						FindAdd(hdlg, szProfileDir, _T("[reg]\\"));
					}
				}
				RegCloseKey(hKey);
			}
			// select
			if (opts.filename[0])
				i = AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), opts.filename, _T(""));
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
		GetDlgItemText(hdlg, IDC_FILE, opts.filename, SIZEOF(opts.filename));
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
			TCHAR str[MAX_PATH];

			// _T("Miranda Databases (*.dat)\0*.DAT\0All Files (*)\0*\0");
			TCHAR *filter, *tmp, *tmp1, *tmp2;
			tmp1 = TranslateT("Miranda Databases (*.dat)");
			tmp2 = TranslateT("All Files");
			filter = tmp = (TCHAR*)_alloca((_tcslen(tmp1) + _tcslen(tmp2) + 11)*sizeof(TCHAR));
			tmp = addstring(tmp, tmp1);
			tmp = addstring(tmp, _T("*.DAT"));
			tmp = addstring(tmp, tmp2);
			tmp = addstring(tmp, _T("*"));
			*tmp = 0;

			GetDlgItemText(hdlg, IDC_FILE, str, SIZEOF(str));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hdlg;
			ofn.hInstance = NULL;
			ofn.lpstrFilter = filter;
			ofn.lpstrDefExt = _T("dat");
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = SIZEOF(str);
			ofn.nMaxFileTitle = MAX_PATH;
			if (GetOpenFileName(&ofn)) {
				int i = AddDatabaseToList(GetDlgItem(hdlg, IDC_DBLIST), str, _T(""));
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
				SetDlgItemText(hdlg, IDC_FILE, (TCHAR*)lvi.lParam);
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
