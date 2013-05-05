#include "skype_proto.h"

INT_PTR CALLBACK CSkypeProto::SkypeMainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *proto = (CSkypeProto *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:
		{
			::TranslateDialogDefault(hwnd);

			proto = (CSkypeProto *)lParam;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
			{
				wchar_t *sid = ::db_get_wsa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_LOGIN);
				SetDlgItemText(hwnd, IDC_SL, sid);
				::mir_free(sid);
			}
			{
				char *pwd = ::db_get_sa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_PASSWORD);

				if (pwd)
				{
					::CallService(
						MS_DB_CRYPT_DECODESTRING,
						::strlen(pwd),
						reinterpret_cast<LPARAM>(pwd));
				}

				SetDlgItemTextA(hwnd, IDC_PW, pwd);
				::mir_free(pwd);
			}
			{
				int port = rand() % 10000 + 10000;
				SetDlgItemInt(hwnd, IDC_PORT, ::db_get_w(NULL, proto->m_szModuleName, "Port", port), FALSE);
				SendMessage(GetDlgItem(hwnd, IDC_PORT), EM_SETLIMITTEXT, 5, 0);
			}
			{
				CheckDlgButton(hwnd, IDC_USE_ALT_PORTS, ::db_get_b(NULL, proto->m_szModuleName, "UseAlternativePorts", 1));
			}
			if (proto->IsOnline())
			{
				SendMessage(GetDlgItem(hwnd, IDC_SL), EM_SETREADONLY, 1, 0);
				SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0); 
				SendMessage(GetDlgItem(hwnd, IDC_PORT), EM_SETREADONLY, 1, 0); 
				EnableWindow(GetDlgItem(hwnd, IDC_USE_ALT_PORTS), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), FALSE); 
				EnableWindow(GetDlgItem(hwnd, IDC_CHANGE_PWD), TRUE);
			}
			else if (::db_get_w(NULL, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) > ID_STATUS_OFFLINE)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), FALSE); 
			}

			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, SKYPE_GROUP_NAME_LIMIT, 0);

			wchar_t *defgroup = db_get_wsa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_DEF_GROUP);
			SetDlgItemText(hwnd, IDC_GROUP, defgroup);
			::mir_free(defgroup);
		}
		return TRUE;

	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDC_SL:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;

					if (!proto->IsOnline() && ::db_get_w(NULL, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) <= ID_STATUS_OFFLINE)
					{
						wchar_t sid[128];
						GetDlgItemText(hwnd, IDC_SL, sid, SIZEOF(sid));
						EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), ::wcslen(sid) != 0);
					}
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_PW:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;
					if (proto->IsOnline())
					{
						char pwd[128];
						GetDlgItemTextA(hwnd, IDC_SL, pwd, SIZEOF(pwd));
						EnableWindow(GetDlgItem(hwnd, IDC_CHANGE_PWD), ::strlen(pwd) != 0);
					}
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_GROUP:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
						return 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_PORT:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
						return 0;
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_USE_ALT_PORTS:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_REGISTER:
				{
					char sid[128], pwd[128];
					GetDlgItemTextA(hwnd, IDC_SL, sid, SIZEOF(sid));
					GetDlgItemTextA(hwnd, IDC_PW, pwd, SIZEOF(pwd));

					CSkype::VALIDATERESULT reason;
					g_skype->ValidatePassword(sid, pwd, reason);

					if (reason == CSkype::VALIDATED_OK)
					{
						CAccount::Ref account;
						g_skype->GetAccount(sid, proto->account);
						proto->account->SetStrProperty(CAccount::P_FULLNAME, sid);
						proto->account->SetOnAccountChangedCallback(
							(CAccount::OnAccountChanged)&CSkypeProto::OnAccountChanged, proto);
						proto->account->Register(pwd, false, false);
					}
					else
					{
						proto->ShowNotification(CSkypeProto::ValidationReasons[reason]);
					}
				}
				return FALSE;

			case IDC_CHANGE_PWD:
				{
					char sid[128], pwd[128];
					GetDlgItemTextA(hwnd, IDC_SL, sid, SIZEOF(sid));
					GetDlgItemTextA(hwnd, IDC_PW, pwd, SIZEOF(pwd));

					PasswordChangeBoxParam param;
					if (proto->ChangePassword(param))
					{
						proto->account->ChangePassword(param.password, param.password2);
					}
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY && !proto->IsOnline())
		{
			wchar_t sid[128];
			GetDlgItemText(hwnd, IDC_SL, sid, SIZEOF(sid));
			::db_set_ws(NULL, proto->m_szModuleName, SKYPE_SETTINGS_LOGIN, sid);
			::mir_free(proto->login);
			proto->login = ::mir_wstrdup(sid);

			char pwd[128];
			GetDlgItemTextA(hwnd, IDC_PW, pwd, SIZEOF(pwd));
			::CallService(MS_DB_CRYPT_ENCODESTRING, ::strlen(pwd), (LPARAM)&pwd);
			::db_set_s(NULL, proto->m_szModuleName, SKYPE_SETTINGS_PASSWORD, pwd);

			HWND item = GetDlgItem(hwnd, IDC_PORT);
			if (item)
			{
				BOOL error;
				int port = GetDlgItemInt(hwnd, IDC_PORT, &error, FALSE);
				::db_set_w(NULL, proto->m_szModuleName, "Port", port);
				::db_set_b(NULL, proto->m_szModuleName, "UseAlternativePorts", (BYTE)IsDlgButtonChecked(hwnd, IDC_USE_ALT_PORTS));
			}

			wchar_t tstr[128];
			GetDlgItemText(hwnd, IDC_GROUP, tstr, SIZEOF(tstr));
			if (lstrlen(tstr) > 0)
			{
				::db_set_ts(NULL, proto->m_szModuleName, SKYPE_SETTINGS_DEF_GROUP, tstr);
				::CallService(MS_CLIST_GROUPCREATE, 0, (LPARAM)tstr);
			}
			else
				::db_unset(NULL, proto->m_szModuleName, SKYPE_SETTINGS_DEF_GROUP);

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::SkypePasswordRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PasswordRequestBoxParam *param = reinterpret_cast<PasswordRequestBoxParam *>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		::TranslateDialogDefault(hwndDlg);

		param = (PasswordRequestBoxParam *)lParam;
		::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);

			wchar_t title[MAX_PATH];
			::mir_sntprintf(
				title, 
				MAX_PATH, 
				::TranslateT("Enter a password for Skype Name %s:"), 
				param->login);
			::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, title);

			::SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_LIMITTEXT, 128 - 1, 0);

			::CheckDlgButton(hwndDlg, IDC_SAVEPASSWORD, param->rememberPassword);
			::ShowWindow(::GetDlgItem(hwndDlg, IDC_SAVEPASSWORD), param->showRememberPasswordBox); 
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					param->rememberPassword = ::IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD) > 0;

					char password[SKYPE_PASSWORD_LIMIT];
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD, password, SIZEOF(password));
					param->password = ::mir_strdup(password);

					::EndDialog(hwndDlg, IDOK);
				}
				break;

			case IDCANCEL:
				::EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

bool CSkypeProto::RequestPassword(PasswordRequestBoxParam &param)
{
	int value = ::DialogBoxParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_PASSWORD_REQUEST), 
		NULL, 
		CSkypeProto::SkypePasswordRequestProc, 
		(LPARAM)&param);
	return value == 1;
}

INT_PTR CALLBACK CSkypeProto::SkypePasswordChangeProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PasswordChangeBoxParam *param = reinterpret_cast<PasswordChangeBoxParam *>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg)
	{
	case WM_INITDIALOG:
		::TranslateDialogDefault(hwndDlg);

		param = (PasswordChangeBoxParam *)lParam;
		::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)CSkypeProto::IconList[0].Handle);
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)CSkypeProto::IconList[0].Handle);
		}
		break;

	case WM_CLOSE:
		::EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					char oldPwd[SKYPE_PASSWORD_LIMIT];
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD, oldPwd, SIZEOF(oldPwd));
					param->password = ::mir_strdup(oldPwd);

					char pwd1[SKYPE_PASSWORD_LIMIT];
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD2, pwd1, SIZEOF(pwd1));
					param->password2 = ::mir_strdup(pwd1);

					char pwd2[SKYPE_PASSWORD_LIMIT];
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD3, pwd2, SIZEOF(pwd2));

					::EndDialog(hwndDlg, IDOK);
				}
				break;

			case IDCANCEL:
				::EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

bool CSkypeProto::ChangePassword(PasswordChangeBoxParam &param)
{
	int value = ::DialogBoxParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_PASSWORD_CHANGE), 
		NULL, 
		CSkypeProto::SkypePasswordChangeProc, 
		(LPARAM)&param);
	return value == 1;
}

INT_PTR CALLBACK CSkypeProto::SkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, ((PSHNOTIFY *)lParam )->lParam);
				break;

			case PSN_INFOCHANGED:
				{
					CSkypeProto *ppro = (CSkypeProto *)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if (!ppro)
						break;

					char *szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL)
						szProto = ppro->m_szModuleName;
					else
						szProto = (char *)::CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

					if (!szProto)
						break;

					::SetDlgItemText(hwndDlg, IDC_SID, ::db_get_wsa(hContact, ppro->m_szModuleName, SKYPE_SETTINGS_LOGIN));
					::SetDlgItemText(hwndDlg, IDC_STATUSTEXT, ::db_get_wsa(hContact, ppro->m_szModuleName, "XStatusMsg") ? ::db_get_wsa(hContact, ppro->m_szModuleName, "XStatusMsg") : TranslateT("<not specified>"));

					if (::db_get_dw(hContact, ppro->m_szModuleName, "OnlineSinceTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "OnlineSinceTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, date);
					} else
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));

					if (::db_get_dw(hContact, ppro->m_szModuleName, "LastEventDateTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						::CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "LastEventDateTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_LASTEVENTDATE, date);
					} else
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));

					if (::db_get_dw(hContact, ppro->m_szModuleName, "ProfileTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						::CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "ProfileTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_LASTPROFILECHANGE, date);
					} else
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));
				}
				break;
			}
			break;
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDCANCEL:
			SendMessage(GetParent(hwndDlg), msg, wParam, lParam);
			break;
		}
		break;
	}

	return FALSE;
}

static INT_PTR CALLBACK PersonalSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 0;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			SendMessage(GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)_T(""));
			SendMessage(GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)TranslateT("Male"));
			SendMessage(GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)TranslateT("Female"));
			for (int i = 1; i < 32; i++)
			{
				TCHAR day[3];
				_itot(i, day, 10);
				SendMessage(GetDlgItem(hwndDlg, IDC_BIRTH_DAY), CB_ADDSTRING, 0, (LPARAM)day);
			}
			for (int i = 1; i < 13; i++)
			{
				TCHAR mon[3];
				_itot(i, mon, 10);
				SendMessage(GetDlgItem(hwndDlg, IDC_BIRTH_MONTH), CB_ADDSTRING, 0, (LPARAM)mon);
			}
			for (int i = 1900; i < 2214; i++)
			{
				TCHAR year[5];
				_itot(i, year, 10);
				SendMessage(GetDlgItem(hwndDlg, IDC_BIRTH_YEAR), CB_ADDSTRING, 0, (LPARAM)year);

			}

			wchar_t *lang = ::db_get_wsa(NULL, ppro->m_szModuleName, "Language1");
			for (std::map<std::wstring, std::wstring>::iterator it = CSkypeProto::languages.begin(); it != CSkypeProto::languages.end(); ++it)
			{
				int nItem = SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_ADDSTRING, 0, (LPARAM)TranslateTS(it->second.c_str()));
				SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_SETITEMDATA, nItem, (LPARAM)it->first.c_str());
				int ii = it->second.compare(lang);
				if(it->second.compare(lang) == 0)
					SetDlgItemText(hwndDlg, IDC_LANGUAGE, TranslateTS(it->second.c_str()));
			}
			DBVARIANT dbv;
			if ( !db_get_ts(NULL, ppro->m_szModuleName, "Nick", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_FULLNAME, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_FULLNAME, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "Homepage", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOMEPAGE, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_HOMEPAGE, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "About", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_ABOUT, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_ABOUT, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "XStatusMsg", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_MOOD, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_ABOUT, _T(""));

			BYTE b = db_get_b(NULL, ppro->m_szModuleName, "Gender", 0);
			if (b == 'M')
				SetDlgItemText(hwndDlg, IDC_GENDER, TranslateT("Male"));
			else if (b == 'F')
				SetDlgItemText(hwndDlg, IDC_GENDER, TranslateT("Female"));
			else
				SetDlgItemText(hwndDlg, IDC_GENDER, _T(""));
			BYTE bday = db_get_b(NULL, ppro->m_szModuleName, "BirthDay", 0);
			if (bday > 1 && bday < 32)
				SetDlgItemInt(hwndDlg, IDC_BIRTH_DAY, bday, false);
			BYTE bmon = db_get_b(NULL, ppro->m_szModuleName, "BirthMonth", 0);
			if (bmon > 1 && bmon < 13)
				SetDlgItemInt(hwndDlg, IDC_BIRTH_MONTH, bmon, false);
			WORD byear = db_get_w(NULL, ppro->m_szModuleName, "BirthYear", 0);
			if (byear > 1900 && bmon < 2214)
				SetDlgItemInt(hwndDlg, IDC_BIRTH_YEAR, byear, false);
		}
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			(((HWND)lParam == GetDlgItem(hwndDlg, IDC_GENDER) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_DAY) ||
			(HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_MONTH) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_YEAR) ||
			(HWND)lParam == GetDlgItem(hwndDlg, IDC_LANGUAGE)) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE)))
		{
			ppro->NeedUpdate = 1;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY *)lParam)->lParam);
				break;
			case PSN_APPLY:
				if (ppro->IsOnline() && ppro->NeedUpdate)
				{
					ppro->SaveToDB(hwndDlg, iPageId);
					ppro->SaveToServer();
				}
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK ContactSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 1;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			DBVARIANT dbv;
			if ( !db_get_ts(NULL, ppro->m_szModuleName, "Cellular", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_MOBPHONE, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_MOBPHONE, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "Phone", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_HOMEPHONE, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_HOMEPHONE, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "CompanyPhone", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_OFFICEPHONE, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_OFFICEPHONE, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "e-mail0", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EMAIL1, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_EMAIL1, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "e-mail1", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EMAIL2, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_EMAIL2, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "e-mail2", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_EMAIL3, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_EMAIL3, _T(""));
		}
		break;

	case WM_COMMAND:
		if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
		{
			ppro->NeedUpdate = 1;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY *)lParam)->lParam);
				break;
			case PSN_APPLY:
				if (ppro->IsOnline() && ppro->NeedUpdate)
				{
					ppro->SaveToDB(hwndDlg, iPageId);
					ppro->SaveToServer();
				}
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK HomeSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 2;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			DBVARIANT dbv;
			if ( !db_get_ts(NULL, ppro->m_szModuleName, "City", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_CITY, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_MOBPHONE, _T(""));

			if ( !db_get_ts(NULL, ppro->m_szModuleName, "State", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_STATE, dbv.ptszVal);
				db_free(&dbv);
			}
			else
				SetDlgItemText(hwndDlg, IDC_STATE, _T(""));

			wchar_t *countr = ::db_get_wsa(NULL, ppro->m_szModuleName, "Country");
			for (int i = 0; i < g_cbCountries; i++)	{
				if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0) {
					TCHAR *country = mir_a2t(g_countries[i].szName);
					int nItem = SendMessage(GetDlgItem(hwndDlg, IDC_COUNTRY), CB_ADDSTRING, 0, (LPARAM)TranslateTS(country));
					SendMessage(GetDlgItem(hwndDlg, IDC_COUNTRY), CB_SETITEMDATA, nItem, (LPARAM)g_countries[i].id);
					if (_tcscmp(country, countr) == 0)
						SetDlgItemText(hwndDlg, IDC_COUNTRY, TranslateTS(country));
					mir_free(country);
				}
			}

			tmi.prepareList((HANDLE)lParam, GetDlgItem(hwndDlg, IDC_TIMEZONE), TZF_PLF_CB);
			//HANDLE hTimeZone = tmi.createByContact ? tmi.createByContact(NULL, 0) : 0;
			//LPCTSTR TzDescr = tmi.getTzDescription(tmi.getTzName(hTimeZone));
			//SetDlgItemText(hwndDlg, IDC_TIMEZONE, TzDescr);
		}
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			(((HWND)lParam == GetDlgItem(hwndDlg, IDC_COUNTRY) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_TIMEZONE)) &&
			(HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE)))
		{
			ppro->NeedUpdate = 1;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY *)lParam)->lParam);
				break;
			case PSN_APPLY:
				if (ppro->IsOnline() && ppro->NeedUpdate)
				{
					ppro->SaveToDB(hwndDlg, iPageId);
					ppro->SaveToServer();
				}
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

int __cdecl CSkypeProto::OnUserInfoInit(WPARAM wParam, LPARAM lParam)
{
	if ((!this->IsProtoContact((HANDLE)lParam)) && lParam)
		return 0;

	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.flags = ODPF_TCHAR | ODPF_USERINFOTAB | ODPF_DONTTRANSLATE;
	odp.hInstance = g_hInstance;
	odp.dwInitParam = LPARAM(this);
	odp.position = -1900000000;
	odp.ptszTitle = m_tszUserName;

	HANDLE hContact = (HANDLE)lParam;
	if (hContact) {
		char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto != NULL && !strcmp(szProto, m_szModuleName)) {
			odp.pfnDlgProc = SkypeDlgProc;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SKYPE);
			UserInfo_AddPage(wParam, &odp);
		}
	} else {
		NeedUpdate = 0;
		odp.pfnDlgProc = ContactSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_CONTACT);
		odp.ptszTab = LPGENT("Contacts");
		UserInfo_AddPage(wParam, &odp);

		odp.pfnDlgProc = HomeSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_HOME);
		odp.ptszTab = LPGENT("Home");
		UserInfo_AddPage(wParam, &odp);

		odp.pfnDlgProc = PersonalSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_PERSONAL);
		odp.ptszTab = LPGENT("General");
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}

void CSkypeProto::SaveToDB(HWND hwndPage, int iPage)
{
	TCHAR text[2048];

	switch (iPage) {
	// Page 0: Personal
	case 0:
		{
			GetDlgItemText(hwndPage, IDC_FULLNAME, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_ws(NULL, this->m_szModuleName, "Nick", text);
			else
				db_unset(NULL, this->m_szModuleName, "Nick");
			GetDlgItemText(hwndPage, IDC_MOOD, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_ws(NULL, this->m_szModuleName, "XStatusMsg", text);
			else
				db_unset(NULL, this->m_szModuleName, "XStatusMsg");
			GetDlgItemText(hwndPage, IDC_ABOUT, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_ws(NULL, this->m_szModuleName, "About", text);
			else
				db_unset(NULL, this->m_szModuleName, "About");
			GetDlgItemText(hwndPage, IDC_HOMEPAGE, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_ws(NULL, this->m_szModuleName, "Homepage", text);
			else
				db_unset(NULL, this->m_szModuleName, "Homepage");
			switch (SendMessage(GetDlgItem(hwndPage, IDC_GENDER), CB_GETCURSEL, 0, 0)) {
			case 0:
				db_unset(NULL, this->m_szModuleName, "Gender");
				break;
			case 1:
				db_set_b(NULL, this->m_szModuleName, "Gender", 'M');
				break;
			case 2:
				db_set_b(NULL, this->m_szModuleName, "Gender", 'F');
				break;
			}
			GetDlgItemText(hwndPage, IDC_BIRTH_DAY, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_b(NULL, this->m_szModuleName, "BirthDay", _ttoi(text));
			else
				db_unset(NULL, this->m_szModuleName, "BirthDay");
			GetDlgItemText(hwndPage, IDC_BIRTH_MONTH, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_b(NULL, this->m_szModuleName, "BirthMonth", _ttoi(text));
			else
				db_unset(NULL, this->m_szModuleName, "BirthMonth");
			GetDlgItemText(hwndPage, IDC_BIRTH_YEAR, text, SIZEOF(text));
			if (text && _tcslen(text) > 0)
				db_set_w(NULL, this->m_szModuleName, "BirthYear", _ttoi(text));
			else
				db_unset(NULL, this->m_szModuleName, "BirthYear");

			int lang = SendMessage(GetDlgItem(hwndPage, IDC_LANGUAGE), CB_GETCURSEL, 0, 0);
			wchar_t *key = (wchar_t *)SendMessage(GetDlgItem(hwndPage, IDC_LANGUAGE), CB_GETITEMDATA, lang, 0);
			std::wstring value = CSkypeProto::languages[key];
			db_set_ws(NULL, this->m_szModuleName, "Language1", value.c_str());
		}
		break;

	// Page 1: Contacts
	case 1:
		GetDlgItemText(hwndPage, IDC_EMAIL1, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "e-mail0", text);
		else
			db_unset(NULL, this->m_szModuleName, "e-mail0");
		GetDlgItemText(hwndPage, IDC_EMAIL2, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "e-mail1", text);
		else
			db_unset(NULL, this->m_szModuleName, "e-mail1");
		GetDlgItemText(hwndPage, IDC_EMAIL3, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "e-mail2", text);
		else
			db_unset(NULL, this->m_szModuleName, "e-mail2");
		GetDlgItemText(hwndPage, IDC_MOBPHONE, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "Cellular", text);
		else
			db_unset(NULL, this->m_szModuleName, "Cellular");
		GetDlgItemText(hwndPage, IDC_HOMEPHONE, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "Phone", text);
		else
			db_unset(NULL, this->m_szModuleName, "Phone");
		GetDlgItemText(hwndPage, IDC_OFFICEPHONE, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "CompanyPhone", text);
		else
			db_unset(NULL, this->m_szModuleName, "CompanyPhone");
		break;

	// Page 2: Home
	case 2:
		GetDlgItemText(hwndPage, IDC_CITY, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "City", text);
		else
			db_unset(NULL, this->m_szModuleName, "City");
		GetDlgItemText(hwndPage, IDC_STATE, text, SIZEOF(text));
		if (text && _tcslen(text) > 0)
			db_set_ws(NULL, this->m_szModuleName, "State", text);
		else
			db_unset(NULL, this->m_szModuleName, "State");
		int i = SendMessage(GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETCURSEL, 0, 0);
		int id = SendMessage(GetDlgItem(hwndPage, IDC_COUNTRY), CB_GETITEMDATA, i, 0);
		char *countrystr = (char *)CallService(MS_UTILS_GETCOUNTRYBYNUMBER, (WPARAM)id, 0);
		TCHAR *country = mir_a2t(countrystr);
		db_set_ws(NULL, this->m_szModuleName, "Country", country);
		mir_free(country);
		break;
	}
}

void CSkypeProto::SaveToServer()
{
}