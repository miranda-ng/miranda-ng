#include "options.h"

HWND hWndOptions = 0;

Options options = {0};

#define DEFAULT_ID		(0x1800)

#define NUM_IDS		20
TCHAR *client_names[NUM_IDS] = {_T("Official Binary Library"), _T("Official Java Applet"), _T("Official Binary Application"), 
	_T("Official Java Application"), _T("Notes v6.5"), _T("Notes v7.0"), _T("ICT"), _T("NotesBuddy"), _T("NotesBuddy v4.15"), _T("Sanity"),
	_T("Perl"), _T("PMR Alert"), _T("Trillian (SourceForge)"), _T("Trillian (IBM)"), _T("Meanwhile Library"), _T("Meanwhile (Python)"),
	_T("Meanwhile (Gaim)"), _T("Meanwhile (Adium)"), _T("Meanwhile (Kopete)"), _T("Custom")};
int client_ids[NUM_IDS] = {0x1000, 0x1001, 0x1002, 0x1003, 0x1200, 0x1210, 0x1300, 0x1400, 0x1405, 0x1600, 0x1625,
	0x1650, 0x16aa, 0x16bb, 0x1700, 0x1701, 0x1702, 0x1703, 0x1704, 0xFFFF};

static BOOL CALLBACK DlgProcOptNet(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );
		
		{
			TCHAR verbuf[100];
			WORD client_ver = GetClientVersion(), server_ver = GetServerVersion();
			if(client_ver)
				_stprintf(verbuf, _T("Client proto version: %03d.%03d"), (client_ver & 0xFF00) >> 8, client_ver & 0xFF);
			else
				_stprintf(verbuf, _T("Disconnected"));
			SetDlgItemText(hwndDlg, IDC_ST_CLIENTVER, verbuf);
			if(server_ver)
				_stprintf(verbuf, _T("Server proto version: %03d.%03d"), (server_ver & 0xFF00) >> 8, server_ver & 0xFF);
			else
				_stprintf(verbuf, _T("Disconnected"));
			SetDlgItemText(hwndDlg, IDC_ST_SERVERVER, verbuf);
		}

		hWndOptions = hwndDlg;
		
		wchar_t *s;
		s = u2w(options.server_name); SetDlgItemTextW(hwndDlg, IDC_ED_SNAME, s); free(s);
		s = u2w(options.id); SetDlgItemTextW(hwndDlg, IDC_ED_NAME, s); free(s);
		s = u2w(options.pword); SetDlgItemTextW(hwndDlg, IDC_ED_PWORD, s); free(s);

		SetDlgItemInt(hwndDlg, IDC_ED_PORT, options.port, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_GETSERVERCONTACTS, options.get_server_contacts ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ADDCONTACTS, options.add_contacts ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_IDLEAWAY, options.idle_as_away ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_OLDDEFAULTVER, options.use_old_default_client_ver ? TRUE : FALSE);
		{
			SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_RESETCONTENT, 0, 0);
			int pos = 0;
			bool found = false;
			
			for(int i = 0; i < NUM_IDS; i++) {
				pos = SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_ADDSTRING, -1, (LPARAM)client_names[i]);
				SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_SETITEMDATA, pos, client_ids[i]);
				if(client_ids[i] == options.client_id) {
					found = true;
					SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_SETCURSEL, pos, 0);
					SetDlgItemInt(hwndDlg, IDC_ED_CLIENTID, client_ids[i], FALSE);
					if(i != sizeof(client_ids) / sizeof(int) - 1) {
						HWND hw = GetDlgItem(hwndDlg, IDC_ED_CLIENTID);
						EnableWindow(hw, false);
					}
				}
			}
			
			if(!found) {
				SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_SETCURSEL, pos, 0); // pos is last item, i.e. custom
				SetDlgItemInt(hwndDlg, IDC_ED_CLIENTID, options.client_id, FALSE);
			}
		}

		if(!ServiceExists(MS_POPUP_ADDPOPUP)) {
			HWND hw = GetDlgItem(hwndDlg, IDC_RAD_ERRPOP);
			EnableWindow(hw, FALSE);
		}

		if(!ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) {
			HWND hw = GetDlgItem(hwndDlg, IDC_RAD_ERRBAL);
			EnableWindow(hw, FALSE);
		}

		switch(options.err_method) {
		case ED_POP: CheckDlgButton(hwndDlg, IDC_RAD_ERRPOP, TRUE); break;
		case ED_MB: CheckDlgButton(hwndDlg, IDC_RAD_ERRMB, TRUE); break;
		case ED_BAL: CheckDlgButton(hwndDlg, IDC_RAD_ERRBAL, TRUE); break;
		}

		if(options.encrypt_session)
			CheckDlgButton(hwndDlg, IDC_RAD_ENC, TRUE);
		else
			CheckDlgButton(hwndDlg, IDC_RAD_NOENC, TRUE);
		
		return FALSE;
	}
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_GETCURSEL, 0, 0);
			int id = SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_GETITEMDATA, sel, 0);
			bool custom = (id == client_ids[sizeof(client_ids) / sizeof(int) - 1]);

			if(!custom)
				SetDlgItemInt(hwndDlg, IDC_ED_CLIENTID, id, FALSE);
			else
				SetDlgItemInt(hwndDlg, IDC_ED_CLIENTID, DEFAULT_ID, FALSE);

			HWND hw = GetDlgItem(hwndDlg, IDC_ED_CLIENTID);
			EnableWindow(hw, custom);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_UPLOADCONTACTS:
				{
					HWND hBut = GetDlgItem(hwndDlg, IDC_BTN_UPLOADCONTACTS);
					EnableWindow(hBut, FALSE);
					hBut = GetDlgItem(hwndDlg, IDC_BTN_IMPORTCONTACTS);
					EnableWindow(hBut, FALSE);
					
					ExportContactsToServer();

					SendMessage(hwndDlg, WMU_STORECOMPLETE, 0, 0);
				}
				return TRUE;
			case IDC_BTN_IMPORTCONTACTS:
				{
					OPENFILENAME ofn = {0};
					TCHAR import_filename[MAX_PATH];
					import_filename[0] = 0;

					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = import_filename;
					ofn.hwndOwner = hwndDlg;
					ofn.Flags = CC_FULLOPEN;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFilter = _T("All\0*.*\0");
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST;

					if(GetOpenFileName(&ofn) == TRUE) {
						HWND hBut = GetDlgItem(hwndDlg, IDC_BTN_UPLOADCONTACTS);
						EnableWindow(hBut, FALSE);
						hBut = GetDlgItem(hwndDlg, IDC_BTN_IMPORTCONTACTS);
						EnableWindow(hBut, FALSE);

						ImportContactsFromFile(ofn.lpstrFile);

						SendMessage(hwndDlg, WMU_STORECOMPLETE, 0, 0);
					}
				}
				return TRUE;
			case IDC_CHK_GETSERVERCONTACTS:
			case IDC_CHK_ENCMESSAGES:
			case IDC_RAD_ERRMB:
			case IDC_RAD_ERRBAL:
			case IDC_RAD_ERRPOP:
			case IDC_CHK_USERCP:
			case IDC_CHK_ADDCONTACTS:
			case IDC_CHK_IDLEAWAY:
			case IDC_CHK_OLDDEFAULTVER:
			case IDC_RAD_ENC:
			case IDC_RAD_NOENC:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_RAD_ANSI:
			case IDC_RAD_UTF8:
			case IDC_RAD_OEM:
			case IDC_RAD_UTF7:
			case IDC_RAD_USERCP:
				{
					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_USERCP);
					EnableWindow(hw, !IsDlgButtonChecked(hwndDlg, IDC_RAD_USERCP));
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
		}
		break;

	case WMU_STORECOMPLETE:
		{
			HWND hBut = GetDlgItem(hwndDlg, IDC_BTN_UPLOADCONTACTS);
			EnableWindow(hBut, TRUE);
			hBut = GetDlgItem(hwndDlg, IDC_BTN_IMPORTCONTACTS);
			EnableWindow(hBut, TRUE);
		}
		return TRUE;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			wchar_t ws[2048];
			char *utf;

			GetDlgItemTextW(hwndDlg, IDC_ED_SNAME, ws, LSTRINGLEN);
			strcpy(options.server_name, utf = w2u(ws)); free(utf);
			GetDlgItemTextW(hwndDlg, IDC_ED_NAME, ws, LSTRINGLEN);
			strcpy(options.id, utf = w2u(ws)); free(utf);
			GetDlgItemTextW(hwndDlg, IDC_ED_PWORD, ws, LSTRINGLEN);
			strcpy(options.pword, utf = w2u(ws)); free(utf);

			BOOL translated;
			int port = GetDlgItemInt(hwndDlg, IDC_ED_PORT, &translated, FALSE);
			if(translated)
				options.port = port;

			options.get_server_contacts = (IsDlgButtonChecked(hwndDlg, IDC_CHK_GETSERVERCONTACTS) != FALSE);

			int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_GETCURSEL, 0, 0);
			int id = SendDlgItemMessage(hwndDlg, IDC_CMB_CLIENT, CB_GETITEMDATA, sel, 0);

			if(id == client_ids[sizeof(client_ids) / sizeof(int) - 1]) {
				BOOL trans;
				id = GetDlgItemInt(hwndDlg, IDC_ED_CLIENTID, &trans, FALSE);
				if(trans)
					options.client_id = id;
			} else
				options.client_id = id;

			if(IsDlgButtonChecked(hwndDlg, IDC_RAD_ERRMB)) options.err_method = ED_MB;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_ERRBAL)) options.err_method = ED_BAL;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_ERRPOP)) options.err_method = ED_POP;

			options.add_contacts = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ADDCONTACTS) != FALSE);
			options.encrypt_session = (IsDlgButtonChecked(hwndDlg, IDC_RAD_ENC) != FALSE);
			options.idle_as_away = (IsDlgButtonChecked(hwndDlg, IDC_CHK_IDLEAWAY) != FALSE);

			options.use_old_default_client_ver = (IsDlgButtonChecked(hwndDlg, IDC_CHK_OLDDEFAULTVER) != FALSE);

			SaveOptions();

			return TRUE;
		}
		break;
	case WM_DESTROY:
		hWndOptions = 0;
		break;
	}

	return FALSE;
}

int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	TCHAR *temp_proto = a2t(PROTO);

	odp.flags						= ODPF_BOLDGROUPS;
#ifdef _UNICODE
	odp.flags |= ODPF_UNICODE;
#endif
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTNET);
	odp.ptszTitle					= TranslateTS(temp_proto);
	odp.ptszGroup					= TranslateT("Network");
	odp.pfnDlgProc					= DlgProcOptNet;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	free(temp_proto);

	return 0;
}

void LoadOptions() {
	DBVARIANT dbv;

	if(!DBGetContactSettingUtf(0, PROTO, "ServerName", &dbv)) {
		strncpy(options.server_name, dbv.pszVal, LSTRINGLEN);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingUtf(0, PROTO, "stid", &dbv)) {
		strncpy(options.id, dbv.pszVal, LSTRINGLEN);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingUtf(0, PROTO, "PWord", &dbv)) {
		strncpy(options.pword, dbv.pszVal, LSTRINGLEN);
		DBFreeVariant(&dbv);

		CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)LSTRINGLEN, (LPARAM)options.pword);
	}

	options.port = DBGetContactSettingDword(0, PROTO, "ServerPort", DEFAULT_PORT);
	options.get_server_contacts = (DBGetContactSettingByte(0, PROTO, "GetServerContacts", 1) == 1);
	options.client_id = DBGetContactSettingDword(0, PROTO, "ClientID", DEFAULT_ID);

	// if popups not installed, will be changed to 'ED_BAL' (balloons) in main.cpp, modules loaded
	options.err_method = (ErrorDisplay)DBGetContactSettingByte(0, PROTO, "ErrorDisplay", ED_POP); 

	options.add_contacts = (DBGetContactSettingByte(0, PROTO, "AutoAddContacts", 0) == 1); 
	options.encrypt_session = (DBGetContactSettingByte(0, PROTO, "EncryptSession", 0) == 1); 
	options.idle_as_away = (DBGetContactSettingByte(0, PROTO, "IdleAsAway", 1) == 1); 

	options.use_old_default_client_ver = (DBGetContactSettingByte(0, PROTO, "UseOldClientVer", 0) == 1); 
}

void SaveOptions() {
	DBWriteContactSettingStringUtf(0, PROTO, "ServerName", options.server_name);

	DBWriteContactSettingStringUtf(0, PROTO, "stid", options.id);
	//DBWriteContactSettingString(0, PROTO, "Nick", options.id);

	char buff[LSTRINGLEN];
	strcpy(buff, options.pword);
	CallService(MS_DB_CRYPT_ENCODESTRING, (WPARAM)LSTRINGLEN, (LPARAM)buff);
	DBWriteContactSettingStringUtf(0, PROTO, "PWord", buff);

	DBWriteContactSettingDword(0, PROTO, "ServerPort", options.port);
	DBWriteContactSettingByte(0, PROTO, "GetServerContacts", options.get_server_contacts ? 1 : 0);
	DBWriteContactSettingDword(0, PROTO, "ClientID", options.client_id);
	DBWriteContactSettingByte(0, PROTO, "ErrorDisplay", options.err_method);

	DBWriteContactSettingByte(0, PROTO, "AutoAddContacts", options.add_contacts ? 1 : 0);
	DBWriteContactSettingByte(0, PROTO, "EncryptSession", options.encrypt_session ? 1 : 0);
	DBWriteContactSettingByte(0, PROTO, "IdleAsAway", options.idle_as_away ? 1 : 0);

	DBWriteContactSettingByte(0, PROTO, "UseOldClientVer", options.use_old_default_client_ver ? 1 : 0);
}
