/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
#include "stdafx.h"
#include "file.h"

#define VSCAN_MCAFEE      1
#define VSCAN_DRSOLOMON   2
#define VSCAN_NORTON      3
#define VSCAN_CA          4

struct virusscannerinfo {
	const wchar_t *szProductName;
	const wchar_t *szExeRegPath;
	const wchar_t *szExeRegValue;
	const wchar_t *szCommandLine;
} virusScanners[] = {
	{L"Network Associates/McAfee VirusScan", L"SOFTWARE\\McAfee\\VirusScan", L"Scan32EXE", L"\"%s\" %%f /nosplash /comp /autoscan /autoexit /noboot"},
	{L"Dr Solomon's VirusScan (Network Associates)", L"SOFTWARE\\Network Associates\\TVD\\VirusScan\\AVConsol\\General", L"szScannerExe", L"\"%s\" %%f /uinone /noboot /comp /prompt /autoexit"},
	{L"Norton AntiVirus", L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\Navw32.exe", NULL, L"\"%s\" %%f /b- /m- /s+ /noresults"},
	{L"Computer Associates/Inoculate IT", L"Software\\Antivirus", L"ImageFilename", L"\"%s\" %%f /display = progress /exit"},
	{L"Computer Associates eTrust", L"SOFTWARE\\ComputerAssociates\\Anti-Virus\\Resident", L"VetPath", L"\"%s\" %%f /display = progress /exit"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\Components\\101", L"EXEName", L"\"%s\" /S /Q %%f"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV8", L"\"%savp.exe\" SCAN %%f"},
	{L"Kaspersky Anti-Virus", L"SOFTWARE\\KasperskyLab\\SetupFolders", L"KAV9", L"\"%savp.exe\" SCAN %%f"},
	{L"AntiVir PersonalEdition Classic", L"SOFTWARE\\Avira\\AntiVir PersonalEdition Classic", L"Path", L"\"%savscan.exe\" /GUIMODE = 2 /PATH = \"%%f\""},
	{L"ESET NOD32 Antivirus", L"SOFTWARE\\ESET\\ESET Security\\CurrentVersion\\Info", L"InstallDir", L"\"%secls.exe\" /log-all /aind /no-boots /adware /sfx /unsafe /unwanted /heur /adv-heur /action = clean \"%%f\""},
};

#define M_UPDATEENABLING   (WM_USER+100)
#define M_SCANCMDLINESELCHANGE  (WM_USER+101)

#ifndef SHACF_FILESYS_DIRS
	#define SHACF_FILESYS_DIRS  0x00000020
#endif

static INT_PTR CALLBACK DlgProcFileOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SHAutoComplete(GetDlgItem(hwndDlg, IDC_FILEDIR), SHACF_FILESYS_DIRS);

			wchar_t str[MAX_PATH];
			GetContactReceivedFilesDir(NULL, str, _countof(str), FALSE);
			SetDlgItemText(hwndDlg, IDC_FILEDIR, str);

			CheckDlgButton(hwndDlg, IDC_AUTOACCEPT, db_get_b(NULL, "SRFile", "AutoAccept", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOMIN, db_get_b(NULL, "SRFile", "AutoMin", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOCLOSE, db_get_b(NULL, "SRFile", "AutoClose", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_AUTOCLEAR, db_get_b(NULL, "SRFile", "AutoClear", 1) ? BST_CHECKED : BST_UNCHECKED);
			switch (db_get_b(NULL, "SRFile", "UseScanner", VIRUSSCAN_DISABLE)) {
				case VIRUSSCAN_AFTERDL: CheckDlgButton(hwndDlg, IDC_SCANAFTERDL, BST_CHECKED); break;
				case VIRUSSCAN_DURINGDL: CheckDlgButton(hwndDlg, IDC_SCANDURINGDL, BST_CHECKED); break;
				default: CheckDlgButton(hwndDlg, IDC_NOSCANNER, BST_CHECKED); break;
			}
			CheckDlgButton(hwndDlg, IDC_WARNBEFOREOPENING, db_get_b(NULL, "SRFile", "WarnBeforeOpening", 1) ? BST_CHECKED : BST_UNCHECKED);

			for (int i = 0; i < _countof(virusScanners); i++) {
				wchar_t szScanExe[MAX_PATH];
				if (SRFile_GetRegValue(HKEY_LOCAL_MACHINE, virusScanners[i].szExeRegPath, virusScanners[i].szExeRegValue, szScanExe, _countof(szScanExe))) {
					int iItem = SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_ADDSTRING, 0, (LPARAM)virusScanners[i].szProductName);
					SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_SETITEMDATA, iItem, i);
				}
			}
			if (SendDlgItemMessageA(hwndDlg, IDC_SCANCMDLINE, CB_GETCOUNT, 0, 0) == 0) {
				int iItem = SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_ADDSTRING, 0, (LPARAM)L"");
				SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_SETITEMDATA, iItem, (LPARAM)-1);
			}

			DBVARIANT dbv;
			if (db_get_ws(NULL, "SRFile", "ScanCmdLine", &dbv) == 0) {
				SetDlgItemText(hwndDlg, IDC_SCANCMDLINE, dbv.ptszVal);
				db_free(&dbv);
			}
			else if (SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_GETCOUNT, 0, 0)) {
				SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_SETCURSEL, 0, 0);
				PostMessage(hwndDlg, M_SCANCMDLINESELCHANGE, 0, 0);
			}

			switch (db_get_b(NULL, "SRFile", "IfExists", FILERESUME_ASK)) {
				case FILERESUME_RESUMEALL: CheckDlgButton(hwndDlg, IDC_RESUME, BST_CHECKED); break;
				case FILERESUME_OVERWRITEALL: CheckDlgButton(hwndDlg, IDC_OVERWRITE, BST_CHECKED); break;
				case FILERESUME_RENAMEALL: CheckDlgButton(hwndDlg, IDC_RENAME, BST_CHECKED); break;
				default: CheckDlgButton(hwndDlg, IDC_ASK, BST_CHECKED); break;
			}
			SendMessage(hwndDlg, M_UPDATEENABLING, 0, 0);
		}
		return TRUE;

	case M_UPDATEENABLING:
		{
			int on = BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_NOSCANNER);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ST_CMDLINE), on);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCANCMDLINE), on);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SCANCMDLINEBROWSE), on);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ST_CMDLINEHELP), on);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AUTOMIN), IsDlgButtonChecked(hwndDlg, IDC_AUTOACCEPT));
		}
		break;
	case M_SCANCMDLINESELCHANGE:
		{
			wchar_t str[512];
			wchar_t szScanExe[MAX_PATH];
			int iScanner = SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_SCANCMDLINE, CB_GETCURSEL, 0, 0), 0);
			if (iScanner >= _countof(virusScanners) || iScanner < 0) break;
			str[0] = '\0';
			if (SRFile_GetRegValue(HKEY_LOCAL_MACHINE, virusScanners[iScanner].szExeRegPath, virusScanners[iScanner].szExeRegValue, szScanExe, _countof(szScanExe)))
				mir_snwprintf(str, virusScanners[iScanner].szCommandLine, szScanExe);
			SetDlgItemText(hwndDlg, IDC_SCANCMDLINE, str);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FILEDIR:
			if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
			break;

		case IDC_FILEDIRBROWSE:
			{
				wchar_t str[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILEDIR, str, _countof(str));
				if (BrowseForFolder(hwndDlg, str))
					SetDlgItemText(hwndDlg, IDC_FILEDIR, str);
			}
			break;

		case IDC_AUTOACCEPT:
		case IDC_NOSCANNER:
		case IDC_SCANAFTERDL:
		case IDC_SCANDURINGDL:
			SendMessage(hwndDlg, M_UPDATEENABLING, 0, 0);
			break;

		case IDC_SCANCMDLINE:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				PostMessage(hwndDlg, M_SCANCMDLINESELCHANGE, 0, 0);
			else if (HIWORD(wParam) != CBN_EDITCHANGE)
				return 0;
			break;

		case IDC_SCANCMDLINEBROWSE:
			wchar_t str[MAX_PATH + 2];
			GetDlgItemText(hwndDlg, IDC_SCANCMDLINE, str, _countof(str));

			CMStringW tszFilter;
			tszFilter.AppendFormat(L"%s (*.exe)%c*.exe%c", TranslateT("Executable files"), 0, 0);
			tszFilter.AppendFormat(L"%s (*)%c*%c", TranslateT("All files"), 0, 0);

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
			ofn.lpstrFilter = tszFilter;
			ofn.lpstrFile = str;
			ofn.nMaxFile = _countof(str) - 2;
			if (str[0] == '"') {
				wchar_t *pszQuote = wcschr(str + 1, '"');
				if (pszQuote)
					*pszQuote = 0;
				memmove(str, str + 1, (mir_wstrlen(str) * sizeof(wchar_t)));
			}
			else {
				wchar_t *pszSpace = wcschr(str, ' ');
				if (pszSpace) *pszSpace = 0;
			}
			ofn.nMaxFileTitle = MAX_PATH;
			if (!GetOpenFileName(&ofn)) break;
			if (wcschr(str, ' ') != NULL) {
				memmove(str + 1, str, ((_countof(str) - 2) * sizeof(wchar_t)));
				str[0] = '"';
				mir_wstrcat(str, L"\"");
			}
			SetDlgItemText(hwndDlg, IDC_SCANCMDLINE, str);
			break;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			wchar_t str[512];
			GetDlgItemText(hwndDlg, IDC_FILEDIR, str, _countof(str));
			RemoveInvalidPathChars(str);
			db_set_ws(NULL, "SRFile", "RecvFilesDirAdv", str);
			db_set_b(NULL, "SRFile", "AutoAccept", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOACCEPT));
			db_set_b(NULL, "SRFile", "AutoMin", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOMIN));
			db_set_b(NULL, "SRFile", "AutoClose", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOCLOSE));
			db_set_b(NULL, "SRFile", "AutoClear", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_AUTOCLEAR));
			db_set_b(NULL, "SRFile", "UseScanner", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SCANAFTERDL) ? VIRUSSCAN_AFTERDL : (IsDlgButtonChecked(hwndDlg, IDC_SCANDURINGDL) ? VIRUSSCAN_DURINGDL : VIRUSSCAN_DISABLE)));
			GetDlgItemText(hwndDlg, IDC_SCANCMDLINE, str, _countof(str));
			db_set_ws(NULL, "SRFile", "ScanCmdLine", str);
			db_set_b(NULL, "SRFile", "WarnBeforeOpening", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_WARNBEFOREOPENING));
			db_set_b(NULL, "SRFile", "IfExists", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ASK) ? FILERESUME_ASK : (IsDlgButtonChecked(hwndDlg, IDC_RESUME) ? FILERESUME_RESUMEALL : (IsDlgButtonChecked(hwndDlg, IDC_OVERWRITE) ? FILERESUME_OVERWRITEALL : FILERESUME_RENAMEALL))));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int FileOptInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 900000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FILETRANSFER);
	odp.szTitle.a = LPGEN("File transfers");
	odp.szGroup.a = LPGEN("Events");
	odp.pfnDlgProc = DlgProcFileOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
