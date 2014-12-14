/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

Options *Options::instance = NULL;
Options &opt = Options::getInstance();

extern DeleteTimer &deleteTimer;
extern ServerList &ftpList;

extern void PrebuildMainMenu();

void Options::deinit()
{	
	delete this;
}

void Options::loadOptions()
{
	selected = db_get_b(0, MODULE, "Selected", 0);
	defaultFTP = db_get_b(0, MODULE, "Default", 0);
	bAutosend = db_get_b(0, MODULE, "Autosend", 0) ? true : false;
	bCloseDlg = db_get_b(0, MODULE, "CloseDlg", 0) ? true : false;
	bCopyLink = db_get_b(0, MODULE, "CopyLink", 1) ? true : false;
	bUseSubmenu = db_get_b(0, MODULE, "UseSubmenu", 1) ? true : false;
	bHideInactive = db_get_b(0, MODULE, "HideInactive", 1) ? true : false;
	bAutoDelete = db_get_b(0, MODULE, "DeleteTimer", 0) ? true : false;
	iDeleteTime = db_get_dw(0, MODULE, "AutoDeleteTime", 60);
	timeRange = (ETimeRange)db_get_b(0, MODULE, "TimeRange", TR_MINUTES);
	iCompressionLevel = db_get_b(0, MODULE, "CompressionLevel", 6);
	bSetZipName = db_get_b(0, MODULE, "SetZipName", 0) ? true : false;
}

void Options::saveOptions() const
{
	db_set_b(0, MODULE, "Autosend", bAutosend ? 1 : 0);
	db_set_b(0, MODULE, "CopyLink", bCopyLink ? 1 : 0);			
	db_set_b(0, MODULE, "UseSubmenu", bUseSubmenu ? 1 : 0);
	db_set_b(0, MODULE, "HideInactive", bHideInactive ? 1 : 0);
	db_set_b(0, MODULE, "CloseDlg", bCloseDlg ? 1 : 0);
	db_set_b(0, MODULE, "DeleteTimer", bAutoDelete ? 1 : 0);
	db_set_dw(0, MODULE, "AutoDeleteTime", iDeleteTime);
	db_set_b(0, MODULE, "TimeRange", (int)timeRange);
	db_set_b(0, MODULE, "CompressionLevel", iCompressionLevel);
	db_set_b(0, MODULE, "SetZipName", bSetZipName ? 1 : 0);
}

void Options::enableItems(HWND hwndDlg, bool state)
{
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULT), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOLIST), state);	
	EnableWindow(GetDlgItem(hwndDlg, IDC_SERVER), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_USER), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DIR), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_URL), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PORT), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHMOD), state);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PASSIVE), state);	
}

INT_PTR CALLBACK Options::DlgProcOptsAccounts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{		
			for (int i = 0; i < ServerList::FTP_COUNT; i++) 
				ComboBox_AddString(GetDlgItem(hwndDlg, IDC_FTPLIST), ftpList[i]->stzName);

			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_FTPLIST), opt.selected);
			CheckDlgButton(hwndDlg, IDC_DEFAULT, opt.selected == opt.defaultFTP ? BST_CHECKED : BST_UNCHECKED);	

			ServerList::FTP *ftp = ftpList.getSelected();
			ComboBox_AddString(GetDlgItem(hwndDlg, IDC_PROTOLIST), TranslateT("FTP (Standard)"));
			ComboBox_AddString(GetDlgItem(hwndDlg, IDC_PROTOLIST), TranslateT("FTP+SSL (Explicit)"));
			ComboBox_AddString(GetDlgItem(hwndDlg, IDC_PROTOLIST), TranslateT("FTP+SSL (Implicit)"));
			ComboBox_AddString(GetDlgItem(hwndDlg, IDC_PROTOLIST), TranslateT("SFTP (Secure FTP over SSH)"));
			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PROTOLIST), ftp->ftpProto);		

			SetDlgItemTextA(hwndDlg, IDC_SERVER, ftp->szServer);
			SetDlgItemTextA(hwndDlg, IDC_USER, ftp->szUser);
			SetDlgItemTextA(hwndDlg, IDC_PASSWORD, ftp->szPass);
			SetDlgItemTextA(hwndDlg, IDC_DIR, ftp->szDir);
			SetDlgItemTextA(hwndDlg, IDC_URL, ftp->szUrl);
			SetDlgItemTextA(hwndDlg, IDC_CHMOD, ftp->szChmod);
			SetDlgItemInt (hwndDlg, IDC_PORT, ftp->iPort, FALSE);
			CheckDlgButton(hwndDlg, IDC_PASSIVE, ftp->bPassive ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ENABLED, ftp->bEnabled ? BST_CHECKED : BST_UNCHECKED);
			enableItems(hwndDlg, ftp->bEnabled);
		}
		return TRUE;

	case WM_COMMAND:

		if (HIWORD(wParam) == BN_CLICKED) {
			if (LOWORD(wParam) == IDC_ENABLED)
				enableItems(hwndDlg, IsDlgButtonChecked(hwndDlg, IDC_ENABLED) == BST_CHECKED ? true : false);
		}
		else if (HIWORD(wParam) == CBN_SELCHANGE) {
			if (LOWORD(wParam) == IDC_FTPLIST) {
				opt.selected = (BYTE)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_FTPLIST));
				CheckDlgButton(hwndDlg, IDC_DEFAULT, opt.selected == opt.defaultFTP ? BST_CHECKED : BST_UNCHECKED);	

				ServerList::FTP *ftp = ftpList.getSelected();
				SendDlgItemMessage(hwndDlg, IDC_PROTOLIST, CB_SETCURSEL, ftp->ftpProto, 0);
				SetDlgItemTextA(hwndDlg, IDC_SERVER, ftp->szServer);
				SetDlgItemTextA(hwndDlg, IDC_USER, ftp->szUser);
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, ftp->szPass);
				SetDlgItemTextA(hwndDlg, IDC_DIR, ftp->szDir);
				SetDlgItemTextA(hwndDlg, IDC_URL, ftp->szUrl);
				SetDlgItemTextA(hwndDlg, IDC_CHMOD, ftp->szChmod);
				SetDlgItemInt (hwndDlg, IDC_PORT, ftp->iPort, FALSE);
				CheckDlgButton(hwndDlg, IDC_PASSIVE, ftp->bPassive ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_ENABLED, ftp->bEnabled ? BST_CHECKED : BST_UNCHECKED);	
				enableItems(hwndDlg, ftp->bEnabled);
			}
			else if (LOWORD(wParam) == IDC_PROTOLIST) {
				int sel = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PROTOLIST));
				switch (sel) {
					case ServerList::FTP::FT_STANDARD:
					case ServerList::FTP::FT_SSL_EXPLICIT: SetDlgItemInt(hwndDlg, IDC_PORT, 21, FALSE); break;
					case ServerList::FTP::FT_SSL_IMPLICIT: SetDlgItemInt(hwndDlg, IDC_PORT, 990, FALSE); break;
					case ServerList::FTP::FT_SSH: SetDlgItemInt(hwndDlg, IDC_PORT, 22, FALSE); break;
				}
			}
		}

		if (HIWORD(wParam)==BN_CLICKED || HIWORD(wParam)==EN_CHANGE || HIWORD(wParam)==CBN_SELCHANGE || HIWORD(wParam)==CBN_EDITCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		return TRUE;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			if (IsDlgButtonChecked(hwndDlg, IDC_DEFAULT))
				opt.defaultFTP = opt.selected;

			ServerList::FTP *ftp = ftpList.getSelected();
			GetDlgItemText(hwndDlg, IDC_FTPLIST, ftp->stzName, SIZEOF(ftp->stzName));
			GetDlgItemTextA(hwndDlg, IDC_SERVER, ftp->szServer, SIZEOF(ftp->szServer));
			GetDlgItemTextA(hwndDlg, IDC_USER, ftp->szUser, SIZEOF(ftp->szUser));
			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, ftp->szPass, SIZEOF(ftp->szPass));
			GetDlgItemTextA(hwndDlg, IDC_DIR, ftp->szDir, SIZEOF(ftp->szDir));
			GetDlgItemTextA(hwndDlg, IDC_URL, ftp->szUrl, SIZEOF(ftp->szUrl));
			GetDlgItemTextA(hwndDlg, IDC_CHMOD, ftp->szChmod, SIZEOF(ftp->szChmod));

			ftp->ftpProto = (ServerList::FTP::EProtoType)ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_PROTOLIST));
			ftp->iPort = GetDlgItemInt(hwndDlg, IDC_PORT, 0, 0);
			ftp->bPassive = IsDlgButtonChecked(hwndDlg, IDC_PASSIVE) ? true : false;
			ftp->bEnabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLED) ? true : false;

			ComboBox_DeleteString(GetDlgItem(hwndDlg, IDC_FTPLIST), opt.selected);
			ComboBox_InsertString(GetDlgItem(hwndDlg, IDC_FTPLIST), opt.selected, ftp->stzName);
			ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_FTPLIST), opt.selected);

			ftpList.saveToDb();
			PrebuildMainMenu();
		}
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK Options::DlgProcOptsAdvanced(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_URL_AUTOSEND, opt.bAutosend ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_URL_COPYTOML, opt.bCopyLink ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_USESUBMENU, opt.bUseSubmenu ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_HIDEINACTIVE, opt.bHideInactive ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CLOSEDLG, opt.bCloseDlg ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTODELETE, opt.bAutoDelete ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_LEVEL_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(9, 0));
		SetDlgItemInt(hwndDlg, IDC_LEVEL, opt.iCompressionLevel, FALSE);
		CheckDlgButton(hwndDlg, IDC_SETZIPNAME, opt.bSetZipName ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_DELETETIME_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG(UD_MAXVAL, 1));
		SetDlgItemInt(hwndDlg, IDC_DELETETIME, opt.iDeleteTime, FALSE);

		SendDlgItemMessage(hwndDlg, IDC_RANGE, CB_ADDSTRING, 0, (LPARAM)TranslateT("minutes"));
		SendDlgItemMessage(hwndDlg, IDC_RANGE, CB_ADDSTRING, 0, (LPARAM)TranslateT("hours"));
		SendDlgItemMessage(hwndDlg, IDC_RANGE, CB_ADDSTRING, 0, (LPARAM)TranslateT("days"));
		SendDlgItemMessage(hwndDlg, IDC_RANGE, CB_SETCURSEL, (int)opt.timeRange, 0);

		EnableWindow(GetDlgItem(hwndDlg, IDC_DELETETIME), opt.bAutoDelete);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RANGE), opt.bAutoDelete);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_AUTODELETE) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETETIME), IsDlgButtonChecked(hwndDlg, IDC_AUTODELETE));
			EnableWindow(GetDlgItem(hwndDlg, IDC_RANGE), IsDlgButtonChecked(hwndDlg, IDC_AUTODELETE));
		}

		if (HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		return TRUE;

	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == PSN_APPLY) {
			opt.bAutosend = IsDlgButtonChecked(hwndDlg, IDC_URL_AUTOSEND) ? true : false;
			opt.bCopyLink = IsDlgButtonChecked(hwndDlg, IDC_URL_COPYTOML) ? true : false;
			opt.bUseSubmenu = IsDlgButtonChecked(hwndDlg, IDC_USESUBMENU) ? true : false;
			opt.bHideInactive = IsDlgButtonChecked(hwndDlg, IDC_HIDEINACTIVE) ? true : false;
			opt.bCloseDlg = IsDlgButtonChecked(hwndDlg, IDC_CLOSEDLG) ? true : false;
			opt.bAutoDelete = IsDlgButtonChecked(hwndDlg, IDC_AUTODELETE) ? true : false;
			opt.iCompressionLevel = GetDlgItemInt(hwndDlg, IDC_LEVEL, 0, FALSE);
			opt.bSetZipName = IsDlgButtonChecked(hwndDlg, IDC_SETZIPNAME) ? true : false;
			opt.iDeleteTime = GetDlgItemInt(hwndDlg, IDC_DELETETIME, 0, FALSE);
			opt.timeRange = (Options::ETimeRange)SendDlgItemMessage(hwndDlg, IDC_RANGE, CB_GETCURSEL, 0, 0);
			opt.saveOptions();

			if (opt.bAutoDelete) deleteTimer.start();
			else deleteTimer.stop();

			PrebuildMainMenu();
		}
		return TRUE;
	}

	return FALSE;
}

int Options::InitOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszTitle = LPGENT("FTP File");
	odp.ptszGroup = LPGENT("Services");

	odp.ptszTab = LPGENT("Accounts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FTPFILE);
	odp.pfnDlgProc = Options::DlgProcOptsAccounts;
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ADVANCED);
	odp.pfnDlgProc = Options::DlgProcOptsAdvanced;
	Options_AddPage(wParam, &odp);
	return 0;
}
