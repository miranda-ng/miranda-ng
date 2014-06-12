#include "common.h"

NASettings g_settings;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			char buf[10];
			CheckDlgButton(hwndDlg, NA_LOCAL_CHECK, g_settings.local_only ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, NA_DEBUG_MSG_CHECK, g_settings.debug_messages ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, NA_LOG_CHECK, g_settings.log_to_file ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemTextA(hwndDlg, NA_LOG_FILENAME, g_settings.log_filename.c_str());
			EnableWindow(GetDlgItem(hwndDlg, NA_DEBUG_MSG_CHECK), IsDlgButtonChecked(hwndDlg, NA_LOG_CHECK) ? 1 : 0);
			CheckDlgButton(hwndDlg, NA_PCSPEAKER_CHECK, g_settings.use_pcspeaker ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, NA_ALLOW_EXECUTE, g_settings.allow_execute ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemTextA(hwndDlg, NA_PORT, itoa(g_settings.port, buf, 10));
			SetDlgItemTextA(hwndDlg, NA_PASSWORD, g_settings.password.c_str());
			UINT state;
			switch (g_settings.sound) {
			case g_settings.always:
				state = BST_CHECKED; break;
			case g_settings.never:
				state = BST_UNCHECKED; break;
			case g_settings.request:
			default:
				state = BST_INDETERMINATE; break;
			}
			CheckDlgButton(hwndDlg, NA_SOUND_CHECK, state);
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case NA_SOUND_CHECK:
		case NA_PCSPEAKER_CHECK:
		case NA_LOCAL_CHECK:
		case NA_DEBUG_MSG_CHECK:
		case NA_LOG_CHECK:
		case NA_PORT:
		case NA_PASSWORD:
		case NA_LOG_FILENAME:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case NA_LOG_BROWSE:
				TCHAR szTemp[MAX_PATH+1], szTemp1[MAX_PATH+1], szProfileDir[MAX_PATH+1];
				GetDlgItemText(hwndDlg, NA_LOG_FILENAME, szTemp, MAX_PATH);
				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = szTemp;
				ofn.nMaxFile = MAX_PATH;
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = TranslateT("Log (*.log)\0*.log\0Text (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
				ofn.nFilterIndex = 1;
				// Use profile directory as default, if path is not specified
				CallService(MS_DB_GETPROFILEPATH, (WPARAM)MAX_PATH, (LPARAM) szProfileDir);
				ofn.lpstrInitialDir = szProfileDir;
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
				ofn.lpstrDefExt = _T("log");
				if ( GetOpenFileName(&ofn)) {
					PathToRelativeT(szTemp, szTemp1);
					SetDlgItemText(hwndDlg, NA_LOG_FILENAME, szTemp1);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				}

				return 0;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom != 0)
			break;

		switch (((LPNMHDR)lParam)->code) {
		case PSN_RESET:
			return TRUE;

		case PSN_APPLY: {
			UINT state = IsDlgButtonChecked(hwndDlg, NA_SOUND_CHECK);
			NASettings s;
			switch (state) {
			case BST_CHECKED:
				s.sound = s.always; break;
			case BST_INDETERMINATE:
				s.sound = s.request; break;
			case BST_UNCHECKED:
			default:
				s.sound = s.never; break;
			}
			s.local_only = IsDlgButtonChecked(hwndDlg, NA_LOCAL_CHECK) != BST_UNCHECKED;
			s.debug_messages = IsDlgButtonChecked(hwndDlg, NA_DEBUG_MSG_CHECK) != BST_UNCHECKED;
			s.log_to_file = IsDlgButtonChecked(hwndDlg, NA_LOG_CHECK) != BST_UNCHECKED;
			s.use_pcspeaker = IsDlgButtonChecked(hwndDlg, NA_PCSPEAKER_CHECK) != BST_UNCHECKED;
			s.allow_execute = IsDlgButtonChecked(hwndDlg, NA_ALLOW_EXECUTE) != BST_UNCHECKED;

			TCHAR buf[1000];
			if (!GetDlgItemText(hwndDlg, NA_PORT, buf, sizeof buf - 1))
				buf[0] = '\0';
			int port = _ttoi(buf);
			if (port <= 0 || port > 65535)
				MessageBox(0, TranslateT("Invalid port number"), TranslateT("NotifyAnything"), MB_ICONWARNING | MB_OK);
			else
				s.port = port;

			if (!GetDlgItemText(hwndDlg, NA_PASSWORD, buf, sizeof buf - 1))
				buf[0] = '\0';
			s.password = _T2A(buf);
			
			g_settings = s;
			save_settings();
			stop_threads();
			start_threads();
			return TRUE;
		}
		}
		break;
	}
	return FALSE;
}

void save_settings()
{
	db_set_b(NULL, PlugName, "local_only", g_settings.local_only);
	db_set_b(NULL, PlugName, "debug_messages", g_settings.debug_messages);
	db_set_b(NULL, PlugName, "log_to_file", g_settings.log_to_file);
	db_set_b(NULL, PlugName, "beep", g_settings.sound);
	db_set_b(NULL, PlugName, "use_pcspeaker", g_settings.use_pcspeaker);
	db_set_b(NULL, PlugName, "allow_execute", g_settings.allow_execute);
	db_set_dw(NULL, PlugName, "port", g_settings.port);
	db_set_s(NULL, PlugName, "password", g_settings.password.c_str());
	db_set_s(NULL, PlugName, "log_filename", g_settings.log_filename.c_str());
}

void load_settings()
{
	g_settings.local_only = db_get_b(NULL, PlugName, "local_only", 1) != 0;
	g_settings.debug_messages = db_get_b(NULL, PlugName, "debug_messages", 0) != 0;
	g_settings.log_to_file = db_get_b(NULL, PlugName, "log_to_file", 0) != 0;
	g_settings.sound = NASettings::sound_t(db_get_b(NULL, PlugName, "beep", g_settings.request));
	g_settings.use_pcspeaker = db_get_b(NULL, PlugName, "use_pcspeaker", 0) != 0;
	g_settings.allow_execute = db_get_b(NULL, PlugName, "allow_execute", 0) != 0;
	g_settings.port = db_get_dw(NULL, PlugName, "port", 12001);

	DBVARIANT dbv;
	if(!db_get(NULL, PlugName, "password", &dbv)) {
		g_settings.password = dbv.pszVal;
		db_free(&dbv);
	}

	if(!db_get(NULL, PlugName, "log_filename", &dbv)) {
		g_settings.log_filename = dbv.pszVal;
		db_free(&dbv);
	}
	else
        g_settings.log_filename = g_mirandaDir + "\\"+LOG_ID+".log";
}

int OptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.pszTemplate = MAKEINTRESOURCEA(NA_OPTIONS);
	odp.pfnDlgProc = DlgProcOpts;
	odp.pszTitle = "Notify Anything";
	odp.pszGroup = Translate("Plugins");
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.groupPosition = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
