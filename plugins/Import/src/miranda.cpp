/*

Import plugin for Miranda NG

Copyright (C) 2012-14 George Hazan

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

// =====================
// == LOCAL VARIABLES ==
// =====================

TCHAR importFile[MAX_PATH];

// ====================
// ====================
// == IMPLEMENTATION ==
// ====================
// ====================

static void SearchForLists(HWND hdlg, const TCHAR *mirandaPath, const TCHAR *mirandaProf)
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
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !_tcscmp(fd.cFileName, _T(".")) || !_tcscmp(fd.cFileName, _T("..")))
			continue;

		// skip the current profile too
		if (mirandaProf != NULL && !_tcsicmp(mirandaProf, fd.cFileName))
			continue;

		TCHAR buf[MAX_PATH], profile[MAX_PATH];
		mir_sntprintf(buf, SIZEOF(buf), _T("%s\\%s\\%s.dat"), mirandaPath, fd.cFileName, fd.cFileName);
		if (_taccess(buf, 0) == 0) {
			mir_sntprintf(profile, SIZEOF(profile), _T("%s.dat"), fd.cFileName);

			int i = SendDlgItemMessage(hdlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)profile);
			SendDlgItemMessage(hdlg, IDC_LIST, LB_SETITEMDATA, i, (LPARAM)mir_tstrdup(buf));
		}
	}
		while (FindNextFile(hFind, &fd));

	FindClose(hFind);
}

INT_PTR CALLBACK MirandaPageProc(HWND hdlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			ptrT pfd(VARST(_T("%miranda_path%\\Profiles")));
			ptrT pfd1(VARST(_T("%miranda_path%")));
			ptrT pfd2(VARST(_T("%miranda_profile%")));
			ptrT pfn(VARST(_T("%miranda_profilename%")));

			SearchForLists(hdlg, pfd2, pfn);
			SearchForLists(hdlg, pfd1, NULL);
			if (lstrcmpi(pfd, pfd2))
				SearchForLists(hdlg, pfd, NULL);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_WIZARDINTRO, (LPARAM)WizardIntroPageProc);
			break;

		case IDOK:
			{
				TCHAR filename[MAX_PATH];
				GetDlgItemText(hdlg, IDC_FILENAME, filename, SIZEOF(filename));
				if (_taccess(filename, 4)) {
					MessageBox(hdlg, TranslateT("The given file does not exist. Please check that you have entered the name correctly."), TranslateT("Miranda Import"), MB_OK);
					break;
				}
				lstrcpy(importFile, filename);
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_OPTIONS, (LPARAM)MirandaOptionsPageProc);
			}
			break;

		case IDCANCEL:
			PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
			break;

		case IDC_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int sel = SendDlgItemMessage(hdlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) break;
				SetDlgItemText(hdlg, IDC_FILENAME, (TCHAR*)SendDlgItemMessage(hdlg, IDC_LIST, LB_GETITEMDATA, sel, 0));
			}
			break;

		case IDC_OTHER:
			ptrT pfd(Utils_ReplaceVarsT(_T("%miranda_profile%")));

			TCHAR str[MAX_PATH], text[256];
			GetDlgItemText(hdlg, IDC_FILENAME, str, SIZEOF(str));
			mir_sntprintf(text, SIZEOF(text), _T("%s (*.dat, *.bak)%c*.dat;*.bak%c%s (*.*)%c*.*%c%c"), TranslateT("Miranda NG database"), 0, 0, TranslateT("All Files"), 0, 0, 0);

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hdlg;
			ofn.lpstrFilter = text;
			ofn.lpstrDefExt = _T("dat");
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_DONTADDTORECENT;
			ofn.nMaxFile = SIZEOF(str);
			ofn.lpstrInitialDir = pfd;
			if (GetOpenFileName(&ofn))
				SetDlgItemText(hdlg, IDC_FILENAME, str);
		}
		break;

	case WM_DESTROY:
		for (int i = SendDlgItemMessage(hdlg, IDC_LIST, LB_GETCOUNT, 0, 0) - 1; i >= 0; i--)
			mir_free((char*)SendDlgItemMessage(hdlg, IDC_LIST, LB_GETITEMDATA, i, 0));
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK MirandaOptionsPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		EnableWindow(GetDlgItem(hdlg, IDC_RADIO_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_STATIC_ALL), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_RADIO_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_STATIC_CONTACTS), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_RADIO_CUSTOM), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDC_STATIC_CUSTOM), TRUE);
		CheckDlgButton(hdlg, IDC_RADIO_ALL, BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_MIRANDADB, (LPARAM)MirandaPageProc);
			break;

		case IDOK:
			if (IsDlgButtonChecked(hdlg, IDC_RADIO_ALL)) {
				nImportOption = IMPORT_ALL;
				nCustomOptions = 0;//IOPT_MSGSENT|IOPT_MSGRECV|IOPT_URLSENT|IOPT_URLRECV;
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}

			if (IsDlgButtonChecked(hdlg, IDC_RADIO_CONTACTS)) {
				nImportOption = IMPORT_CONTACTS;
				nCustomOptions = 0;
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
				break;
			}

			if (IsDlgButtonChecked(hdlg, IDC_RADIO_CUSTOM)) {
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_ADVOPTIONS, (LPARAM)MirandaAdvOptionsPageProc);
				break;
			}
			break;

		case IDCANCEL:
			PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
			break;
		}
		break;
	}
	return FALSE;
}

static const UINT InControls[] = { IDC_IN_MSG, IDC_IN_URL, IDC_IN_FT, IDC_IN_OTHER };
static const UINT OutControls[] = { IDC_OUT_MSG, IDC_OUT_URL, IDC_OUT_FT, IDC_OUT_OTHER };
static const UINT SysControls[] = { IDC_CONTACTS, IDC_SYSTEM };

INT_PTR CALLBACK MirandaAdvOptionsPageProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			struct tm *TM = NULL;
			struct _SYSTEMTIME ST = { 0 };

			dwSinceDate = db_get_dw(NULL, IMPORT_MODULE, "ImportSinceTS", time(NULL));

			TM = localtime(&dwSinceDate);

			ST.wYear = TM->tm_year + 1900;
			ST.wMonth = TM->tm_mon + 1;
			ST.wDay = TM->tm_mday;

			DateTime_SetSystemtime(GetDlgItem(hdlg, IDC_DATETIMEPICKER), GDT_VALID, &ST);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BACK:
			PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_OPTIONS, (LPARAM)MirandaOptionsPageProc);
			break;

		case IDOK:
			nImportOption = IMPORT_CUSTOM;
			nCustomOptions = 0;

			if (IsDlgButtonChecked(hdlg, IDC_CONTACTS))
				nCustomOptions |= IOPT_CONTACTS | IOPT_GROUPS;
			if (IsDlgButtonChecked(hdlg, IDC_SYSTEM))
				nCustomOptions |= IOPT_SYSTEM;

			// incoming
			if (IsDlgButtonChecked(hdlg, IDC_IN_MSG))
				nCustomOptions |= IOPT_MSGRECV;
			if (IsDlgButtonChecked(hdlg, IDC_IN_URL))
				nCustomOptions |= IOPT_URLRECV;
			if (IsDlgButtonChecked(hdlg, IDC_IN_FT))
				nCustomOptions |= IOPT_FILERECV;
			if (IsDlgButtonChecked(hdlg, IDC_IN_OTHER))
				nCustomOptions |= IOPT_OTHERRECV;

			// outgoing
			if (IsDlgButtonChecked(hdlg, IDC_OUT_MSG))
				nCustomOptions |= IOPT_MSGSENT;
			if (IsDlgButtonChecked(hdlg, IDC_OUT_URL))
				nCustomOptions |= IOPT_URLSENT;
			if (IsDlgButtonChecked(hdlg, IDC_OUT_FT))
				nCustomOptions |= IOPT_FILESENT;
			if (IsDlgButtonChecked(hdlg, IDC_OUT_OTHER))
				nCustomOptions |= IOPT_OTHERSENT;

			// since date
			dwSinceDate = 0;

			if (IsDlgButtonChecked(hdlg, IDC_SINCE)) {
				struct _SYSTEMTIME ST = { 0 };

				if (DateTime_GetSystemtime(GetDlgItem(hdlg, IDC_DATETIMEPICKER), &ST) == GDT_VALID) {
					struct tm TM = { 0 };

					TM.tm_mday = ST.wDay;
					TM.tm_mon = ST.wMonth - 1;
					TM.tm_year = ST.wYear - 1900;

					dwSinceDate = mktime(&TM);

					db_set_dw(NULL, IMPORT_MODULE, "ImportSinceTS", dwSinceDate);
				}
			}

			if (nCustomOptions)
				PostMessage(GetParent(hdlg), WIZM_GOTOPAGE, IDD_PROGRESS, (LPARAM)ProgressPageProc);
			break;

		case IDCANCEL:
			PostMessage(GetParent(hdlg), WM_CLOSE, 0, 0);
			break;

		case IDC_SINCE:
			EnableWindow(GetDlgItem(hdlg, IDC_DATETIMEPICKER), IsDlgButtonChecked(hdlg, IDC_SINCE));
			break;

			if (HIWORD(wParam) != STN_CLICKED)
				break;

		case IDC_ALL:
		case IDC_INCOMING:
		case IDC_OUTGOING:
			if (LOWORD(wParam) == IDC_ALL)
				for (int i = 0; i < sizeof(SysControls) / sizeof(SysControls[0]); i++)
					CheckDlgButton(hdlg, SysControls[i], !IsDlgButtonChecked(hdlg, SysControls[i]));

			if (LOWORD(wParam) != IDC_OUTGOING)
				for (int i = 0; i < sizeof(InControls) / sizeof(InControls[0]); i++)
					CheckDlgButton(hdlg, InControls[i], !IsDlgButtonChecked(hdlg, InControls[i]));

			if (LOWORD(wParam) != IDC_INCOMING)
				for (int i = 0; i < sizeof(OutControls) / sizeof(OutControls[0]); i++)
					CheckDlgButton(hdlg, OutControls[i], !IsDlgButtonChecked(hdlg, OutControls[i]));
			break;

		case IDC_MSG:
			CheckDlgButton(hdlg, IDC_IN_MSG, !IsDlgButtonChecked(hdlg, IDC_IN_MSG));
			CheckDlgButton(hdlg, IDC_OUT_MSG, !IsDlgButtonChecked(hdlg, IDC_OUT_MSG));
			break;

		case IDC_URL:
			CheckDlgButton(hdlg, IDC_IN_URL, !IsDlgButtonChecked(hdlg, IDC_IN_URL));
			CheckDlgButton(hdlg, IDC_OUT_URL, !IsDlgButtonChecked(hdlg, IDC_OUT_URL));
			break;

		case IDC_FT:
			CheckDlgButton(hdlg, IDC_IN_FT, !IsDlgButtonChecked(hdlg, IDC_IN_FT));
			CheckDlgButton(hdlg, IDC_OUT_FT, !IsDlgButtonChecked(hdlg, IDC_OUT_FT));
			break;

		case IDC_OTHER:
			CheckDlgButton(hdlg, IDC_IN_OTHER, !IsDlgButtonChecked(hdlg, IDC_IN_OTHER));
			CheckDlgButton(hdlg, IDC_OUT_OTHER, !IsDlgButtonChecked(hdlg, IDC_OUT_OTHER));
			break;
		}
		break;
	}
	return FALSE;
}
