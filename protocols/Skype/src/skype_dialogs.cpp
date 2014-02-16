#include "skype.h"

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
				ptrW sid( ::db_get_wsa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_SID));
				SetDlgItemText(hwnd, IDC_SL, sid);
			}
			{
				ptrA pwd( ::db_get_sa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_PASSWORD));
				SetDlgItemTextA(hwnd, IDC_PW, pwd);
			}
			{
				int port = rand() % 10000 + 10000;
				SetDlgItemInt(hwnd, IDC_PORT, ::db_get_w(NULL, proto->m_szModuleName, "Port", port), FALSE);
				SendMessage(GetDlgItem(hwnd, IDC_PORT), EM_SETLIMITTEXT, 5, 0);
			}

			CheckDlgButton(hwnd, IDC_USE_ALT_PORTS, ::db_get_b(NULL, proto->m_szModuleName, "UseAlternativePorts", 1));
			
			if (proto->IsOnline())
			{
				SendMessage(GetDlgItem(hwnd, IDC_SL), EM_SETREADONLY, 1, 0);
				SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0);
				SendMessage(GetDlgItem(hwnd, IDC_PORT), EM_SETREADONLY, 1, 0);
				EnableWindow(GetDlgItem(hwnd, IDC_USE_ALT_PORTS), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), FALSE);
				EnableWindow(GetDlgItem(hwnd, IDC_CHANGE_PWD), TRUE);
				EnableWindow(GetDlgItem(hwnd, IDC_GROUP), FALSE);
			}
			else if (::db_get_w(NULL, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) > ID_STATUS_OFFLINE)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), FALSE);
			}

			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, SKYPE_GROUP_NAME_LIMIT, 0);

			ptrW defgroup( db_get_wsa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_DEF_GROUP));
			SetDlgItemText(hwnd, IDC_GROUP, defgroup);
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

					Skype::VALIDATERESULT reason;
					proto->ValidatePassword(sid, pwd, reason);

					if (reason == Skype::VALIDATED_OK)
					{
						CAccount::Ref account;
						proto->GetAccount(sid, proto->account);
						proto->account->SetStrProperty(CAccount::P_FULLNAME, sid);
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
					param.password = ::mir_strdup(pwd);

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
			::db_set_ws(NULL, proto->m_szModuleName, SKYPE_SETTINGS_SID, sid);
			::mir_free(proto->login);
			proto->login = ::mir_wstrdup(sid);

			char pwd[128];
			GetDlgItemTextA(hwnd, IDC_PW, pwd, SIZEOF(pwd));
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
				::Clist_CreateGroup(0, tstr);
			}
			else
				::db_unset(NULL, proto->m_szModuleName, SKYPE_SETTINGS_DEF_GROUP);

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::SkypePrivacyOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		if (lParam)
		{
			ppro = (CSkypeProto *)lParam;
			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			::TranslateDialogDefault(hwndDlg);
		}
		break;

	/*case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			}
		}
		break;*/

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY && !ppro->IsOnline())
		{
			return TRUE;
		}
		break;

		/*switch(LOWORD(wParam))
		{
		}
		break;*/
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
				::TranslateT("Enter a password for %s:"), 
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

		SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_LIMITTEXT, SKYPE_PASSWORD_LIMIT, 0);
		SendDlgItemMessage(hwndDlg, IDC_PASSWORD2, EM_LIMITTEXT, SKYPE_PASSWORD_LIMIT, 0);
		SendDlgItemMessage(hwndDlg, IDC_PASSWORD3, EM_LIMITTEXT, SKYPE_PASSWORD_LIMIT, 0);

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
					char pwd1[SKYPE_PASSWORD_LIMIT];
					char pwd2[SKYPE_PASSWORD_LIMIT];

					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD, oldPwd, SIZEOF(oldPwd));
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD2, pwd1, SIZEOF(pwd1));
					::GetDlgItemTextA(hwndDlg, IDC_PASSWORD3, pwd2, SIZEOF(pwd2));

					if (!::strlen(oldPwd) || !::strlen(pwd1)) {
						::MessageBox(NULL, TranslateT("Password can't be empty."), TranslateT("Change password"), MB_OK | MB_ICONERROR);
						break;
					}

					if (::strcmp(param->password, oldPwd)) {
						::MessageBox(NULL, TranslateT("Old password is not correct."), TranslateT("Change password"), MB_OK | MB_ICONERROR);
						break;
					}

					if (!::strcmp(oldPwd, pwd1)) {
						::MessageBox(NULL, TranslateT("New password is same as old password."), TranslateT("Change password"), MB_OK | MB_ICONERROR);
						break;
					}

					if (::strcmp(pwd1, pwd2)) {
						::MessageBox(NULL, TranslateT("New password and confirmation must be same."), TranslateT("Change password"), MB_OK | MB_ICONERROR);
						break;
					}
					
					param->password2 = ::mir_strdup(pwd1);

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
	return value == IDOK;
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
					MCONTACT hContact = (MCONTACT)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL)
						szProto = ppro->m_szModuleName;
					else
						szProto = (char *)::CallService(MS_PROTO_GETCONTACTBASEPROTO, hContact, 0);

					if (!szProto)
						break;

					::SetDlgItemText(hwndDlg, IDC_SID, ptrW(::db_get_wsa(hContact, ppro->m_szModuleName, SKYPE_SETTINGS_SID)));

					DBVARIANT dbv;
					if ( !::db_get_ts(hContact, ppro->m_szModuleName, "XStatusMsg", &dbv)) {
						::SetDlgItemText(hwndDlg, IDC_STATUSTEXT, dbv.ptszVal);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATUSTEXT), TRUE);
						::db_free(&dbv);
					} else {
						::SetDlgItemText(hwndDlg, IDC_STATUSTEXT, TranslateT("<not specified>"));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_STATUSTEXT), FALSE);
					}

					if (::db_get_dw(hContact, ppro->m_szModuleName, "OnlineSinceTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "OnlineSinceTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, date);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_ONLINESINCE), TRUE);
					} else {
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_ONLINESINCE), FALSE);
					}

					if (::db_get_dw(hContact, ppro->m_szModuleName, "LastEventDateTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						::CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "LastEventDateTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_LASTEVENTDATE, date);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_LASTEVENTDATE), TRUE);
					} else {
						::SetDlgItemText(hwndDlg, IDC_LASTEVENTDATE, TranslateT("<not specified>"));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_LASTEVENTDATE), FALSE);
					}

					if (::db_get_dw(hContact, ppro->m_szModuleName, "ProfileTS", 0)) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						::CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)::db_get_dw(hContact, ppro->m_szModuleName, "ProfileTS", 0), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_LASTPROFILECHANGE, date);
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_LASTPROFILECHANGE), TRUE);
					} else {
						::SetDlgItemText(hwndDlg, IDC_LASTPROFILECHANGE, TranslateT("<not specified>"));
						::EnableWindow(::GetDlgItem(hwndDlg, IDC_LASTPROFILECHANGE), FALSE);
					}
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

INT_PTR CALLBACK CSkypeProto::PersonalSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 0;
	CSkypeProto *ppro = (CSkypeProto *)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		if (lParam)
		{
			ppro = (CSkypeProto *)lParam;
			::TranslateDialogDefault(hwndDlg);

			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			// gender
			::SendMessage(::GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)L"");
			::SendMessage(::GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)::TranslateT("Male"));
			::SendMessage(::GetDlgItem(hwndDlg, IDC_GENDER), CB_ADDSTRING, 0, (LPARAM)::TranslateT("Female"));
			
			BYTE b = ::db_get_b(NULL, ppro->m_szModuleName, "Gender", 0);
			switch (b)
			{
			case 0:
				::SetDlgItemText(hwndDlg, IDC_GENDER, _T(""));
				break;
			case 'M':
				::SetDlgItemText(hwndDlg, IDC_GENDER, TranslateT("Male"));
				break;
			case 'F':
				SetDlgItemText(hwndDlg, IDC_GENDER, TranslateT("Female"));
				break;
			}				

			// birthday
			wchar_t date[5];

			for (int i = 1; i < 32; i++)
			{
				::mir_sntprintf(date, 3, L"%02d", i);
				::SendMessage(GetDlgItem(hwndDlg, IDC_BIRTH_DAY), CB_ADDSTRING, 0, (LPARAM)date);
			}
			BYTE bday = ::db_get_b(NULL, ppro->m_szModuleName, "BirthDay", 0);
			if (bday > 0 && bday < 32)
			{
				::mir_sntprintf(date, 3, L"%02d", bday);
				::SetDlgItemText(hwndDlg, IDC_BIRTH_DAY, date);
			}

			for (int i = 1; i < 13; i++)
			{
				::mir_sntprintf(date, 3, L"%02d", i);
				::SendMessage(::GetDlgItem(hwndDlg, IDC_BIRTH_MONTH), CB_ADDSTRING, 0, (LPARAM)date);
			}
			BYTE bmon = ::db_get_b(NULL, ppro->m_szModuleName, "BirthMonth", 0);
			if (bmon > 0 && bmon < 13)
			{
				::mir_sntprintf(date, 3, L"%02d", bmon);
				::SetDlgItemText(hwndDlg, IDC_BIRTH_MONTH, date);
			}
			SYSTEMTIME sToday = {0};
			::GetLocalTime(&sToday);
			// ages from 10 to 50 is need more?
			for (WORD i = sToday.wYear - 50; i < sToday.wYear - 10; i++)
			{
				::_itow(i, date, 10);
				::SendMessage(::GetDlgItem(hwndDlg, IDC_BIRTH_YEAR), CB_ADDSTRING, 0, (LPARAM)date);
			}
			WORD byear = ::db_get_w(NULL, ppro->m_szModuleName, "BirthYear", 0);
			if (byear > 1900 && bmon < 2214)
				::SetDlgItemInt(hwndDlg, IDC_BIRTH_YEAR, byear, false);

			// language
			int i = 0;
			ptrW lang( ::db_get_wsa(NULL, ppro->m_szModuleName, "Language1"));
			for (auto it = CSkypeProto::languages.begin(); it != CSkypeProto::languages.end(); ++it)
			{
				::SendMessage(
					::GetDlgItem(hwndDlg, IDC_LANGUAGE), 
					CB_ADDSTRING, 
					0, 
					(LPARAM)::TranslateTS(it->second.c_str()));
				
				::SendMessage(
					::GetDlgItem(hwndDlg, IDC_LANGUAGE), 
					CB_SETITEMDATA, 
					i, 
					(LPARAM)&it->first);

				if (lang && it->second.compare(lang) == 0)
					::SendMessage(GetDlgItem(hwndDlg, IDC_LANGUAGE), CB_SETCURSEL, i, 0);
				i++;
			}
			
			// nick
			DBVARIANT dbv;
			if ( !ppro->getTString("Nick", &dbv))
			{
				::SetDlgItemText(hwndDlg, IDC_FULLNAME, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_FULLNAME, _T(""));

			// homepage
			if ( !ppro->getTString("Homepage", &dbv))
			{
				::SetDlgItemText(hwndDlg, IDC_HOMEPAGE, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_HOMEPAGE, _T(""));

			// about
			if ( !ppro->getTString("About", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_ABOUT, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_ABOUT, _T(""));

			// mood
			if ( !ppro->getTString("XStatusMsg", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_MOOD, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_MOOD, _T(""));
		}
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE) ||
			(((HWND)lParam == GetDlgItem(hwndDlg, IDC_GENDER) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_DAY) ||
			(HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_MONTH) || (HWND)lParam == GetDlgItem(hwndDlg, IDC_BIRTH_YEAR) ||
			(HWND)lParam == GetDlgItem(hwndDlg, IDC_LANGUAGE)) && (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_SELCHANGE)))
		{
			ppro->NeedUpdate = 1;
			::SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				::SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY *)lParam)->lParam);
				break;
			case PSN_APPLY:
				if (ppro->IsOnline() && ppro->NeedUpdate)
					ppro->SaveOwnInfoToServer(hwndDlg, iPageId);
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::ContactSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 1;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			::TranslateDialogDefault(hwndDlg);

			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			DBVARIANT dbv;
			if ( !ppro->getTString("Cellular", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_MOBPHONE, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_MOBPHONE, _T(""));

			if ( !ppro->getTString("Phone", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_HOMEPHONE, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_HOMEPHONE, _T(""));

			if ( !ppro->getTString("CompanyPhone", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_OFFICEPHONE, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_OFFICEPHONE, _T(""));

			if ( !ppro->getTString("e-mail0", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_EMAIL1, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_EMAIL1, _T(""));

			if ( !ppro->getTString("e-mail1", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_EMAIL2, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_EMAIL2, _T(""));

			if ( !ppro->getTString("e-mail2", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_EMAIL3, dbv.ptszVal);
				::db_free(&dbv);
			}
			else
				::SetDlgItemText(hwndDlg, IDC_EMAIL3, _T(""));
		}
		break;

	case WM_COMMAND:
		if ((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE)
		{
			ppro->NeedUpdate = 1;
			::SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				::SendMessage(hwndDlg, WM_INITDIALOG, 0, ((PSHNOTIFY *)lParam)->lParam);
				break;
			case PSN_APPLY:
				if (ppro->IsOnline() && ppro->NeedUpdate)
					ppro->SaveOwnInfoToServer(hwndDlg, iPageId);
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::HomeSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 2;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			::TranslateDialogDefault(hwndDlg);

			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			DBVARIANT dbv;
			if ( !ppro->getTString("City", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_CITY, dbv.ptszVal);
				::db_free(&dbv);
			}
			else ::SetDlgItemText(hwndDlg, IDC_CITY, _T(""));

			if ( !ppro->getTString("State", &dbv)) {
				::SetDlgItemText(hwndDlg, IDC_STATE, dbv.ptszVal);
				::db_free(&dbv);
			}
			else ::SetDlgItemText(hwndDlg, IDC_STATE, _T(""));

			ptrW countr(::db_get_wsa(NULL, ppro->m_szModuleName, "Country"));
			for (int i = 0; i < g_cbCountries; i++)
			{
				if (g_countries[i].id != 0xFFFF && g_countries[i].id != 0)
				{
					ptrT country( mir_a2t(g_countries[i].szName));
					int nItem = ::SendMessage(
						::GetDlgItem(hwndDlg, IDC_COUNTRY), 
						CB_ADDSTRING, 
						0, 
						(LPARAM)::TranslateTS(country));
					
					::SendMessage(
						::GetDlgItem(hwndDlg, IDC_COUNTRY), 
						CB_SETITEMDATA, 
						nItem, 
						(LPARAM)&g_countries[i].ISOcode);

					if (countr && ::wcscmp(country, countr) == 0)
						::SendMessage(GetDlgItem(hwndDlg, IDC_COUNTRY), CB_SETCURSEL, nItem, 0);
				}
			}

			tmi.prepareList(lParam, NULL, ::GetDlgItem(hwndDlg, IDC_TIMEZONE), TZF_PLF_CB);
			HANDLE hTimeZone = tmi.createByContact(NULL, 0, 0);
			LPCTSTR TzDescr = tmi.getTzDescription(tmi.getTzName(hTimeZone));
			::SetDlgItemText(hwndDlg, IDC_TIMEZONE, TzDescr);
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
					ppro->SaveOwnInfoToServer(hwndDlg, iPageId);
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::AccountSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	const unsigned long iPageId = 3;
	CSkypeProto *ppro = (CSkypeProto *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		if (lParam) {
			ppro = (CSkypeProto *)lParam;
			::TranslateDialogDefault(hwndDlg);

			::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		}
		break;

	case WM_COMMAND:
		if (((HWND)lParam == GetFocus() && HIWORD(wParam) == EN_CHANGE))
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
					ppro->SaveOwnInfoToServer(hwndDlg, iPageId);
				else if ( !ppro->IsOnline())
					ppro->ShowNotification(::TranslateT("You are not currently connected to the Skype network. You must be online in order to update your information on the server."));
				break;
			}
		}
		break;
	}
	return FALSE;
}