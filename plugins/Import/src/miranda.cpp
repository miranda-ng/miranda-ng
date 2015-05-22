/*

Import plugin for Miranda NG

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)

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

#include "import.h"

time_t dwSinceDate = 0;

TCHAR importFile[MAX_PATH];

//=======================================================================================
// Profile selection dialog

static void SearchForLists(HWND hwndDlg, const TCHAR *mirandaPath, const TCHAR *mirandaProf)
{
	// find in Miranda profile subfolders
	TCHAR searchspec[MAX_PATH];
	mir_sntprintf(searchspec, SIZEOF(searchspec), _T("%s\\*.*"), mirandaPath);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchspec, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		// find all subfolders except "." and ".."
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !mir_tstrcmp(fd.cFileName, _T(".")) || !mir_tstrcmp(fd.cFileName, _T("..")))
			continue;

		// skip the current profile too
		if (mirandaProf != NULL && !_tcsicmp(mirandaProf, fd.cFileName))
			continue;

		TCHAR buf[MAX_PATH], profile[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\%s\\%s.dat"), mirandaPath, fd.cFileName, fd.cFileName);
		if (_taccess(buf, 0) == 0) {
			mir_sntprintf(profile, SIZEOF(profile), _T("%s.dat"), fd.cFileName);

			int i = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)profile);
			SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETITEMDATA, i, (LPARAM)mir_tstrdup(buf));
		}
	}
		while (FindNextFile(hFind, &fd));

	FindClose(hFind);
}

INT_PTR CALLBACK MirandaPageProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			VARST pfd(_T("%miranda_path%\\Profiles"));
			VARST pfd1(_T("%miranda_path%"));
			VARST pfd2(_T("%miranda_profilesdir%"));
			VARST pfn(_T("%miranda_profilename%"));

			SearchForLists(hwndDlg, pfd2, pfn);
			SearchForLists(hwndDlg, pfd1, NULL);
			if (mir_tstrcmpi(pfd, pfd2))
				SearchForLists(hwndDlg, pfd, NULL);
		}
		SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETCURSEL, 0, 0);
		SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LIST, LBN_SELCHANGE), 0);
		TCHAR filename[MAX_PATH];
		GetDlgItemText(hwndDlg, IDC_FILENAME, filename, SIZEOF(filename));
		if (_taccess(filename, 4))
			SendMessage(GetParent(hwndDlg), WIZM_DISABLEBUTTON, 1, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_WIZARDINTRO, (LPARAM)WizardIntroPageProc);
			break;

		case IDOK:
			TCHAR filename[MAX_PATH];
			GetDlgItemText(hwndDlg, IDC_FILENAME, filename, SIZEOF(filename));
			if (_taccess(filename, 4)) {
				MessageBox(hwndDlg, TranslateT("The given file does not exist. Please check that you have entered the name correctly."), TranslateT("Miranda Import"), MB_OK);
				break;
			}
			mir_tstrcpy(importFile, filename);
			PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_OPTIONS, (LPARAM)MirandaOptionsPageProc);
			break;

		case IDCANCEL:
			PostMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
			break;

		case IDC_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int sel = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR) {
					SetDlgItemText(hwndDlg, IDC_FILENAME, (TCHAR*)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, sel, 0));
					SendMessage(GetParent(hwndDlg), WIZM_ENABLEBUTTON, 1, 0);
				}
			}
			break;

		case IDC_OTHER:
			ptrT pfd(Utils_ReplaceVarsT(_T("%miranda_profilesdir%")));

			TCHAR str[MAX_PATH], text[256];
			GetDlgItemText(hwndDlg, IDC_FILENAME, str, SIZEOF(str));
			mir_sntprintf(text, SIZEOF(text), _T("%s (*.dat, *.bak)%c*.dat;*.bak%c%s (*.*)%c*.*%c%c"), TranslateT("Miranda NG database"), 0, 0, TranslateT("All Files"), 0, 0, 0);

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.lpstrFilter = text;
			ofn.lpstrDefExt = _T("dat");
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
			ofn.nMaxFile = SIZEOF(str);
			ofn.lpstrInitialDir = pfd;
			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(hwndDlg, IDC_FILENAME, str);
				SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETCURSEL, -1, 0);
				SendMessage(GetParent(hwndDlg), WIZM_ENABLEBUTTON, 1, 0);
			}
		}
		break;

	case WM_DESTROY:
		for (int i = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
			mir_free((char*)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, i, 0));
		break;
	}

	return FALSE;
}

//=======================================================================================
// Import options dialog

INT_PTR CALLBACK MirandaOptionsPageProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_COMPLETE), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_ALL), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_ALL), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO_CUSTOM), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_STATIC_CUSTOM), TRUE);
		CheckDlgButton(hwndDlg, IDC_RADIO_ALL, BST_CHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_MIRANDADB, (LPARAM)MirandaPageProc);
			break;

		case IDOK:
			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_COMPLETE)) {
				nImportOptions = INT32_MAX;
				PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}

			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_ALL)) {
				nImportOptions = IOPT_HISTORY | IOPT_SYSTEM | IOPT_GROUPS | IOPT_CONTACTS;
				PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}

			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_CONTACTS)) {
				nImportOptions = IOPT_CONTACTS;
				PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}

			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_CUSTOM)) {
				PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_ADVOPTIONS, (LPARAM)MirandaAdvOptionsPageProc);
				break;
			}
			break;

		case IDCANCEL:
			PostMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
			break;
		}
		break;
	}
	return FALSE;
}

//=======================================================================================
// Advanced options dialog

static const UINT InControls[] = { IDC_IN_MSG, IDC_IN_URL, IDC_IN_FT, IDC_IN_OTHER };
static const UINT OutControls[] = { IDC_OUT_MSG, IDC_OUT_URL, IDC_OUT_FT, IDC_OUT_OTHER };
static const UINT SysControls[] = { IDC_CONTACTS, IDC_SYSTEM };

INT_PTR CALLBACK MirandaAdvOptionsPageProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dwSinceDate = db_get_dw(NULL, IMPORT_MODULE, "ImportSinceTS", time(NULL));
			struct tm *TM = localtime(&dwSinceDate);

			struct _SYSTEMTIME ST = { 0 };
			ST.wYear = TM->tm_year + 1900;
			ST.wMonth = TM->tm_mon + 1;
			ST.wDay = TM->tm_mday;

			DateTime_SetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), GDT_VALID, &ST);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_OPTIONS, (LPARAM)MirandaOptionsPageProc);
			break;

		case IDOK:
			nImportOptions = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_CONTACTS))
				nImportOptions |= IOPT_CONTACTS | IOPT_GROUPS;
			if (IsDlgButtonChecked(hwndDlg, IDC_SYSTEM))
				nImportOptions |= IOPT_SYSTEM;

			// incoming
			if (IsDlgButtonChecked(hwndDlg, IDC_IN_MSG))
				nImportOptions |= IOPT_MSGRECV;
			if (IsDlgButtonChecked(hwndDlg, IDC_IN_URL))
				nImportOptions |= IOPT_URLRECV;
			if (IsDlgButtonChecked(hwndDlg, IDC_IN_FT))
				nImportOptions |= IOPT_FILERECV;
			if (IsDlgButtonChecked(hwndDlg, IDC_IN_OTHER))
				nImportOptions |= IOPT_OTHERRECV;

			// outgoing
			if (IsDlgButtonChecked(hwndDlg, IDC_OUT_MSG))
				nImportOptions |= IOPT_MSGSENT;
			if (IsDlgButtonChecked(hwndDlg, IDC_OUT_URL))
				nImportOptions |= IOPT_URLSENT;
			if (IsDlgButtonChecked(hwndDlg, IDC_OUT_FT))
				nImportOptions |= IOPT_FILESENT;
			if (IsDlgButtonChecked(hwndDlg, IDC_OUT_OTHER))
				nImportOptions |= IOPT_OTHERSENT;

			// since date
			dwSinceDate = 0;

			if (IsDlgButtonChecked(hwndDlg, IDC_SINCE)) {
				struct _SYSTEMTIME ST = { 0 };

				if (DateTime_GetSystemtime(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), &ST) == GDT_VALID) {
					struct tm TM = { 0 };

					TM.tm_mday = ST.wDay;
					TM.tm_mon = ST.wMonth - 1;
					TM.tm_year = ST.wYear - 1900;

					dwSinceDate = mktime(&TM);

					db_set_dw(NULL, IMPORT_MODULE, "ImportSinceTS", dwSinceDate);
				}
			}

			if (nImportOptions)
				PostMessage(GetParent(hwndDlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
			break;

		case IDCANCEL:
			PostMessage(GetParent(hwndDlg), WM_CLOSE, 0, 0);
			break;

		case IDC_SINCE:
			EnableWindow(GetDlgItem(hwndDlg, IDC_DATETIMEPICKER), IsDlgButtonChecked(hwndDlg, IDC_SINCE));
			break;

			if (HIWORD(wParam) != STN_CLICKED)
				break;

		case IDC_ALL:
		case IDC_INCOMING:
		case IDC_OUTGOING:
			if (LOWORD(wParam) == IDC_ALL)
				for (int i = 0; i < SIZEOF(SysControls); i++)
					CheckDlgButton(hwndDlg, SysControls[i], IsDlgButtonChecked(hwndDlg, SysControls[i]) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

			if (LOWORD(wParam) != IDC_OUTGOING)
				for (int i = 0; i < SIZEOF(InControls); i++)
					CheckDlgButton(hwndDlg, InControls[i], IsDlgButtonChecked(hwndDlg, InControls[i]) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

			if (LOWORD(wParam) != IDC_INCOMING)
				for (int i = 0; i < SIZEOF(OutControls); i++)
					CheckDlgButton(hwndDlg, OutControls[i], IsDlgButtonChecked(hwndDlg, OutControls[i]) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			break;

		case IDC_MSG:
			CheckDlgButton(hwndDlg, IDC_IN_MSG, IsDlgButtonChecked(hwndDlg, IDC_IN_MSG) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OUT_MSG, IsDlgButtonChecked(hwndDlg, IDC_OUT_MSG) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			break;

		case IDC_URL:
			CheckDlgButton(hwndDlg, IDC_IN_URL, IsDlgButtonChecked(hwndDlg, IDC_IN_URL) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OUT_URL, IsDlgButtonChecked(hwndDlg, IDC_OUT_URL) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			break;

		case IDC_FT:
			CheckDlgButton(hwndDlg, IDC_IN_FT, IsDlgButtonChecked(hwndDlg, IDC_IN_FT) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OUT_FT, IsDlgButtonChecked(hwndDlg, IDC_OUT_FT) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			break;

		case IDC_OTHER:
			CheckDlgButton(hwndDlg, IDC_IN_OTHER, IsDlgButtonChecked(hwndDlg, IDC_IN_OTHER) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_OUT_OTHER, IsDlgButtonChecked(hwndDlg, IDC_OUT_OTHER) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
			break;
		}
		break;
	}
	return FALSE;
}
