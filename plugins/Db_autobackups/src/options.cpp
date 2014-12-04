#include "headers.h"

HWND	hPathTip;
Options	options;



HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle)
{
	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST,
		TOOLTIPS_CLASS, NULL,
		(WS_POPUP | TTS_NOPREFIX),
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hwndParent, NULL, g_hInstance, NULL);

	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE));

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
	ti.hwnd = hwndParent;
	ti.hinst = g_hInstance;
	ti.lpszText = ptszText;
	GetClientRect (hwndParent, &ti.rect);
	ti.rect.left = -80;

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	SendMessage(hwndTT, TTM_SETTITLE, 1, (LPARAM)ptszTitle);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, (LPARAM)650);

	return hwndTT;
}


int LoadOptions(void)
{
	options.backup_types = (BackupType)db_get_b(0, "AutoBackups", "BackupType", (BYTE)(BT_PERIODIC));
	options.period = (unsigned int)db_get_w(0, "AutoBackups", "Period", 1);
	options.period_type = (PeriodType)db_get_b(0, "AutoBackups", "PeriodType", (BYTE)PT_DAYS);

	if (!ServiceExists(MS_FOLDERS_GET_PATH)) {
		DBVARIANT dbv;

		if (!db_get_ts(0, "AutoBackups", "Folder", &dbv)) {
			TCHAR *tmp = Utils_ReplaceVarsT(dbv.ptszVal);

			if (_tcslen(tmp) >= 2 && tmp[1] == ':')
				_tcsncpy_s(options.folder, dbv.ptszVal, _TRUNCATE);
			else
				mir_sntprintf(options.folder, SIZEOF(options.folder), _T("%s\\%s"), profilePath, dbv.ptszVal);

			db_free(&dbv);
			mir_free(tmp);
		} else
			mir_sntprintf(options.folder, MAX_PATH, _T("%s%s"), DIR, SUB_DIR);
	}
	options.num_backups = (unsigned int)db_get_w(0, "AutoBackups", "NumBackups", 3);

	options.disable_progress = (BOOL)db_get_b(0, "AutoBackups", "NoProgress", 0);
	options.disable_popups = (BOOL)db_get_b(0, "AutoBackups", "NoPopups", 0);
	options.use_zip = (BOOL)db_get_b(0, "AutoBackups", "UseZip", 0);

	SetBackupTimer();
	return 0;
}

int SaveOptions(void)
{
	TCHAR prof_dir[MAX_PATH];

	db_set_b(0, "AutoBackups", "BackupType", (BYTE)options.backup_types);
	if (options.period < 1)
		options.period = 1;
	db_set_w(0, "AutoBackups", "Period", (WORD)options.period);
	db_set_b(0, "AutoBackups", "PeriodType", (BYTE)options.period_type);

	mir_sntprintf(prof_dir, SIZEOF(prof_dir), _T("%s\\"), profilePath);
	size_t prof_len = _tcslen(prof_dir);
	size_t opt_len = _tcslen(options.folder);

	if (opt_len > prof_len && _tcsncmp(options.folder, prof_dir, prof_len) == 0) {
		db_set_ts(0, "AutoBackups", "Folder", (options.folder + prof_len));
	} else
		db_set_ts(0, "AutoBackups", "Folder", options.folder);

	TCHAR *tmp = Utils_ReplaceVarsT(options.folder);
	if (_tcslen(tmp) < 2 || tmp[1] != ':') {
		_tcsncpy_s(prof_dir, options.folder, _TRUNCATE);
		mir_sntprintf(options.folder, SIZEOF(options.folder), _T("%s\\%s"), profilePath, prof_dir);
	}
	mir_free(tmp);
	db_set_w(0, "AutoBackups", "NumBackups", (WORD)options.num_backups);
	db_set_b(0, "AutoBackups", "NoProgress", (BYTE)options.disable_progress);
	db_set_b(0, "AutoBackups", "NoPopups", (BYTE)options.disable_popups);
	db_set_b(0, "AutoBackups", "UseZip", (BYTE)options.use_zip);

	SetBackupTimer();
	return 0;
}

Options new_options;

int SetDlgState(HWND hwndDlg)
{
	TCHAR buff[10];

	if (new_options.backup_types == BT_DISABLED) {
		CheckDlgButton(hwndDlg, IDC_RAD_DISABLED, BST_CHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_RAD_DISABLED), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_NUMBACKUPS), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_FOLDER), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BUT_BROWSE), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LNK_FOLDERS), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_NOPROG), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_NOPOPUP), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEZIP), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PERIOD), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PT), FALSE);

		CheckDlgButton(hwndDlg, IDC_RAD_START, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_EXIT, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_PERIODIC, BST_UNCHECKED);
	} else {
		EnableWindow(GetDlgItem(hwndDlg, IDC_RAD_DISABLED), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_NUMBACKUPS), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_FOLDER), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_BUT_BROWSE), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LNK_FOLDERS), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_NOPROG), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_NOPOPUP), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_USEZIP), TRUE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PERIOD), new_options.backup_types & BT_PERIODIC);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PT), new_options.backup_types & BT_PERIODIC);

		CheckDlgButton(hwndDlg, IDC_RAD_DISABLED, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_START, new_options.backup_types & BT_START ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_EXIT, new_options.backup_types & BT_EXIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_PERIODIC, new_options.backup_types & BT_PERIODIC ? BST_CHECKED : BST_UNCHECKED);
	}

	SendDlgItemMessage(hwndDlg, SPIN_PERIOD, UDM_SETRANGE32, 1, (LPARAM)60);
	SetDlgItemText(hwndDlg, IDC_ED_PERIOD, _itot(new_options.period, buff, 10));

	SendDlgItemMessage(hwndDlg, SPIN_NUMBACKUPS, UDM_SETRANGE32, 1, (LPARAM)100);
	SetDlgItemText(hwndDlg, IDC_ED_NUMBACKUPS, _itot(new_options.num_backups, buff, 10));

	SetDlgItemText(hwndDlg, IDC_ED_FOLDER, new_options.folder);

	CheckDlgButton(hwndDlg, IDC_CHK_NOPROG, new_options.disable_progress ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_NOPOPUP, new_options.disable_popups ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_CHK_USEZIP, new_options.use_zip ? BST_CHECKED : BST_UNCHECKED);
	if ( !ServiceExists(MS_POPUP_ADDPOPUPT))
		ShowWindow(GetDlgItem(hwndDlg, IDC_CHK_NOPOPUP), SW_HIDE);

	return 0;
}

int CALLBACK BrowseProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg)
	{
		case BFFM_INITIALIZED:
			TCHAR *folder = Utils_ReplaceVarsT(options.folder);
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)folder);
			mir_free(folder);
			break;
	}
	return 0;
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR folder_buff[MAX_PATH] = {0};

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		memcpy(&new_options, &options, sizeof(Options));

		if ( ServiceExists(MS_FOLDERS_GET_PATH)) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_ED_FOLDER), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BUT_BROWSE), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_LNK_FOLDERS), SW_SHOW);
		}
		else {
			TCHAR tszTooltipText[4096];
			mir_sntprintf(tszTooltipText, SIZEOF(tszTooltipText), _T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
				_T("%miranda_path%"),		TranslateT("path to Miranda root folder"),
				_T("%miranda_profilesdir%"),	TranslateT("path to folder containing Miranda profiles"),
				_T("%miranda_profilename%"),	TranslateT("name of current Miranda profile (filename, without extension)"),
				_T("%miranda_userdata%"),	TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
				_T("%appdata%"),		TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
				_T("%username%"),		TranslateT("username for currently logged-on Windows user"),
				_T("%mydocuments%"),		TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
				_T("%desktop%"),		TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
				_T("%xxxxxxx%"),		TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)")
				);
			hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_ED_FOLDER), tszTooltipText, TranslateT("Variables"));
		}

		SetDlgState(hwndDlg);

		SendMessage(GetDlgItem(hwndDlg, IDC_PT), CB_ADDSTRING, 0, (LPARAM) TranslateT("days"));
		SendMessage(GetDlgItem(hwndDlg, IDC_PT), CB_ADDSTRING, 0, (LPARAM) TranslateT("hours"));
		SendMessage(GetDlgItem(hwndDlg, IDC_PT), CB_ADDSTRING, 0, (LPARAM) TranslateT("minutes"));
		switch(new_options.period_type){
			case PT_DAYS: SendDlgItemMessage(hwndDlg, IDC_PT, CB_SETCURSEL, 0, 0); break;
			case PT_HOURS: SendDlgItemMessage(hwndDlg, IDC_PT, CB_SETCURSEL, 1, 0); break;
			case PT_MINUTES: SendDlgItemMessage(hwndDlg, IDC_PT, CB_SETCURSEL, 2, 0); break;
		}

		if (hPathTip)
			SetTimer(hwndDlg, 0, 3000, NULL);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_ED_PERIOD:
			case IDC_ED_FOLDER:
			case IDC_ED_NUMBACKUPS:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		if ( HIWORD( wParam ) == CBN_SELCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_RAD_DISABLED:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_DISABLED))
					new_options.backup_types = BT_DISABLED;

				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_RAD_START:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_START))
					new_options.backup_types |= BT_START;
				else
					new_options.backup_types &= ~BT_START;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_RAD_EXIT:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_EXIT))
					new_options.backup_types |= BT_EXIT;
				else
					new_options.backup_types &= ~BT_EXIT;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_RAD_PERIODIC:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_PERIODIC))
					new_options.backup_types |= BT_PERIODIC;
				else
					new_options.backup_types &= ~BT_PERIODIC;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_BUT_BROWSE:
				{
					BROWSEINFO bi;
					bi.hwndOwner = hwndDlg;
					bi.pidlRoot = 0;
					bi.pszDisplayName = folder_buff;
					bi.lpszTitle = TranslateT("Select backup folder");
					bi.ulFlags = BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseProc;
					bi.lParam = 0;
					bi.iImage = 0;

					LPCITEMIDLIST pidl = SHBrowseForFolder(&bi);
					if (pidl != 0) {
						SHGetPathFromIDList(pidl, folder_buff);

						SetDlgItemText(hwndDlg, IDC_ED_FOLDER, folder_buff);

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );

						CoTaskMemFree((void *)pidl);
					}
					break;
				}
			case IDC_BUT_NOW:
				BackupStart(NULL);
				break;
			case IDC_CHK_NOPROG:
				new_options.disable_progress = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOPROG);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_NOPOPUP:
				new_options.disable_popups = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOPOPUP);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_USEZIP:
				new_options.use_zip = IsDlgButtonChecked(hwndDlg, IDC_CHK_USEZIP);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_LNK_FOLDERS:
				{
					OPENOPTIONSDIALOG ood = {0};
					ood.cbSize = sizeof(ood);
					ood.pszGroup = "Customize";
					ood.pszPage = "Folders";
					Options_Open(&ood);
					break;
				}
			}
		}
		break;

	case WM_TIMER:
		if (IsWindow(hPathTip))
			KillTimer(hPathTip, 4); // It will prevent tooltip autoclosing
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			TCHAR buff[10];
			GetDlgItemText(hwndDlg, IDC_ED_PERIOD, buff, SIZEOF(buff));
			new_options.period = _ttoi(buff);
			GetDlgItemText(hwndDlg, IDC_ED_NUMBACKUPS, buff, SIZEOF(buff));
			new_options.num_backups = _ttoi(buff);

			switch(SendDlgItemMessage(hwndDlg, IDC_PT, CB_GETCURSEL, 0, 0)) {
				case 0: new_options.period_type = PT_DAYS; break;
				case 1: new_options.period_type = PT_HOURS; break;
				case 2: new_options.period_type = PT_MINUTES; break;
			}

			GetDlgItemText(hwndDlg, IDC_ED_FOLDER, folder_buff, SIZEOF(folder_buff));
			{
				TCHAR backupfolder[MAX_PATH] = {0};
				BOOL folder_ok = TRUE;
				TCHAR *tmp = Utils_ReplaceVarsT(folder_buff);

				if (_tcslen(tmp) >= 2 && tmp[1] == ':')
					_tcsncpy_s(backupfolder, tmp, _TRUNCATE);
				else
					mir_sntprintf(backupfolder, SIZEOF(backupfolder), _T("%s\\%s"), profilePath, tmp);
				mir_free(tmp);

				int err = CreateDirectoryTreeT(backupfolder);
				if (err != ERROR_ALREADY_EXISTS && err != 0) {
					TCHAR msg_buff[512];
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, msg_buff, 512, 0);
					MessageBox(0, msg_buff, TranslateT("Error creating backup folder"), MB_OK | MB_ICONERROR);
					folder_ok = FALSE;
				}

				if (folder_ok) {
					_tcsncpy_s(new_options.folder, folder_buff, _TRUNCATE);
					memcpy(&options, &new_options, sizeof(Options));
					SaveOptions();
				} else {
					memcpy(&new_options, &options, sizeof(Options));
					SetDlgState(hwndDlg);
				}
			}
			return TRUE;

		}
		break;

	case WM_DESTROY:
		if (hPathTip) {
			KillTimer(hwndDlg, 0);
			DestroyWindow(hPathTip);
			hPathTip = 0;
		}
		return FALSE;
	}

	return FALSE;
}

int OptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };

	odp.position = -790000000;
	odp.hInstance = g_hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle = LPGEN("Automatic backups");
	odp.pszGroup = LPGEN("Database");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);

	return 0;
}
