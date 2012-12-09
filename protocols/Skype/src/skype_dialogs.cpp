#include "skype_proto.h"

INT_PTR CALLBACK CSkypeProto::SkypeAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSkypeProto *proto;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<CSkypeProto*>(lparam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

		wchar_t* sid = proto->GetSettingString(SKYPE_SETTINGS_LOGIN);
		SetDlgItemText(hwnd, IDC_SL, sid);
		::mir_free(sid);

		char* pwd = proto->GetDecodeSettingString(NULL, SKYPE_SETTINGS_PASSWORD);
		SetDlgItemTextA(hwnd, IDC_PW, pwd);
		::mir_free(pwd);

		if ( proto->m_iStatus != ID_STATUS_OFFLINE) {
			SendMessage(GetDlgItem(hwnd, IDC_SL), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0); 
		}
	}
	return TRUE;

	case WM_COMMAND:
	{
		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
		{
			switch(LOWORD(wparam)) {
			case IDC_SL:
			case IDC_PW:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
	}
	break;

	case WM_NOTIFY:
	{
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY) {
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));			
			{
				TCHAR data[128];
				::mir_free(proto->login);
				GetDlgItemText(hwnd, IDC_SL, data, SIZEOF(data));
				proto->SetSettingString(SKYPE_SETTINGS_LOGIN, data);
				proto->login = ::mir_wstrdup(data);
			}
			{
				char data[128];
				GetDlgItemTextA(hwnd, IDC_PW, data, sizeof(data));
				proto->SetDecodeSettingString(NULL, SKYPE_SETTINGS_PASSWORD, data);
			}

			proto->SetSettingByte("RememberPassword", true);

			return TRUE;
		}
	}
	break;

	}

	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::SkypeOptionsProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSkypeProto *proto;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);

			proto = reinterpret_cast<CSkypeProto*>(lparam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);
			{
				wchar_t* data = proto->GetSettingString(SKYPE_SETTINGS_LOGIN, L"");
				SetDlgItemText(hwnd, IDC_SL, data);
				::mir_free(data);
			}
			{
				char *data = proto->GetDecodeSettingString(NULL, SKYPE_SETTINGS_PASSWORD, "");
				SetDlgItemTextA(hwnd, IDC_PW, data);
				::mir_free(data);
			}

			if (proto->m_iStatus != ID_STATUS_OFFLINE) {
				SendMessage(GetDlgItem(hwnd, IDC_SL), EM_SETREADONLY, 1, 0);
				SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0); 
			}
		}
		return TRUE;

	case WM_COMMAND: 
		{
			if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
			{
				switch(LOWORD(wparam))
				{
				case IDC_SL:
				case IDC_PW:
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			{
				wchar_t data[128];
				::mir_free(proto->login);
				GetDlgItemText(hwnd, IDC_SL, data, SIZEOF(data));
				proto->SetSettingString(SKYPE_SETTINGS_LOGIN, data);
				proto->login = ::mir_wstrdup(data);
			}
			{
				char data[128];
				GetDlgItemTextA(hwnd, IDC_PW, data, sizeof(data));
				proto->SetDecodeSettingString(NULL, SKYPE_SETTINGS_PASSWORD, data);
			}

			proto->SetSettingByte("RememberPassword", true);

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CSkypeProto::SkypePasswordProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto* ppro = (CSkypeProto*)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		::TranslateDialogDefault(hwndDlg);

		ppro = (CSkypeProto*)lParam;
		::SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );
		{
			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)CSkypeProto::iconList[0].Handle);
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)CSkypeProto::iconList[0].Handle);

			wchar_t skypeName[MAX_PATH];
			::mir_sntprintf(
				skypeName, 
				MAX_PATH, 
				TranslateT("Enter a password for Skype Name %s:"), 
				ppro->login);
			::SetDlgItemText(hwndDlg, IDC_INSTRUCTION, skypeName);

			::SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_LIMITTEXT, 128 - 1, 0);

			::CheckDlgButton(hwndDlg, IDC_SAVEPASSWORD, ppro->GetSettingByte(NULL, "RememberPassword", 0));
		}
		break;

	//case WM_DESTROY:
		//ppro->m_hIconProtocol->ReleaseIcon(true);
		//ppro->m_hIconProtocol->ReleaseIcon();
		//break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDOK:
				ppro->rememberPassword = ::IsDlgButtonChecked(hwndDlg, IDC_SAVEPASSWORD) > 0;
				ppro->SetSettingByte("RememberPassword", ppro->rememberPassword);

				::mir_free(ppro->password);
				char password[SKYPE_PASSWORD_LIMIT];
				::GetDlgItemTextA(hwndDlg, IDC_PASSWORD, password, sizeof(password));
				ppro->password = ::mir_strdup(password);

				ppro->SignIn(false);

				::EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				ppro->SetStatus(ID_STATUS_OFFLINE);
				::EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}

	return FALSE;
}

INT_PTR __cdecl CSkypeProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
{
	return (int)CreateDialogParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_SKYPEACCOUNT), 
		(HWND)lParam, 
		&CSkypeProto::SkypeAccountProc, (LPARAM)this);
}

int __cdecl CSkypeProto::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInstance;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;

	odp.position = 271828;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTab = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc = SkypeOptionsProc;
	Options_AddPage(wParam, &odp);

	return 0;
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
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (( PSHNOTIFY* )lParam )->lParam );
				break;

			case PSN_INFOCHANGED:
				{
					CSkypeProto* ppro = (CSkypeProto*)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if (!ppro)
						break;

					char* szProto;
					HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lParam)->lParam;

					if (hContact == NULL)
						szProto = ppro->m_szModuleName;
					else
						szProto = (char*)::CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

					if (!szProto)
						break;

					::SetDlgItemText(hwndDlg, IDC_SID, ppro->GetSettingString(hContact, "sid"));
					::SetDlgItemText(hwndDlg, IDC_STATUSTEXT, ppro->GetSettingString(hContact, "XStatusMsg") ? ppro->GetSettingString(hContact, "XStatusMsg") : TranslateT("<not specified>"));

					if (ppro->GetSettingDword(hContact, "OnlineSinceTS")) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ppro->GetSettingDword(hContact, "OnlineSinceTS"), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, date);
					} else
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));

					if (ppro->GetSettingDword(hContact, "LastEventDateTS")) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ppro->GetSettingDword(hContact, "LastEventDateTS"), (LPARAM)&tts);
						::SetDlgItemText(hwndDlg, IDC_LASTEVENTDATE, date);
					} else
						::SetDlgItemText(hwndDlg, IDC_ONLINESINCE, TranslateT("<not specified>"));

					if (ppro->GetSettingDword(hContact, "ProfileTS")) {
						TCHAR date[64];
						DBTIMETOSTRINGT tts = {0};
						tts.szFormat = _T("d s");
						tts.szDest = date;
						tts.cbDest = sizeof(date);
						CallService(MS_DB_TIME_TIMESTAMPTOSTRINGT, (WPARAM)ppro->GetSettingDword(hContact, "ProfileTS"), (LPARAM)&tts);
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

INT_PTR CALLBACK CSkypeProto::OwnSkypeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_PARAMCHANGED:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (( PSHNOTIFY* )lParam )->lParam );
				break;

			case PSN_INFOCHANGED:
				{
					CSkypeProto* ppro = (CSkypeProto*)::GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if (!ppro)
						break;

					HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);
					ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);
					// Prepare ListView Columns
					LV_COLUMN lvc = {0};
					RECT rc;
					GetClientRect(hwndList, &rc);
					rc.right -= GetSystemMetrics(SM_CXVSCROLL);
					lvc.mask = LVCF_WIDTH;
					lvc.cx = rc.right / 3;
					ListView_InsertColumn(hwndList, 0, &lvc);
					lvc.cx = rc.right - lvc.cx;
					ListView_InsertColumn(hwndList, 1, &lvc);
					// Prepare Setting Items
					LV_ITEM lvi = {0};
					lvi.mask = LVIF_PARAM | LVIF_TEXT;

					for (lvi.iItem = 0; lvi.iItem < SIZEOF(setting); lvi.iItem++) 
					{
						lvi.lParam = lvi.iItem;
						lvi.pszText = (LPTSTR)setting[lvi.iItem].szDescription;
						ListView_InsertItem(hwndList, &lvi);
						wchar_t *text = L"";
						switch(setting[lvi.iItem].dbType) {
							case DBVT_WCHAR:
								text = ppro->GetSettingString(setting[lvi.iItem].szDbSetting);
								break;
							case DBVT_BYTE:
							{
								if (!strcmp(setting[lvi.iItem].szDbSetting, "Gender")) {
									switch(ppro->GetSettingByte(setting[lvi.iItem].szDbSetting)) {
									case 'M':
										text = L"Male";
										break;
									case 'F':
										text = L"Female";
										break;
									}
								} else if (!strcmp(setting[lvi.iItem].szDbSetting, "Timezone")) {
									HANDLE hTimeZone = tmi.createByContact ? tmi.createByContact(NULL, 0) : 0;
									LPCTSTR TzDescr = tmi.getTzDescription(tmi.getTzName(hTimeZone));
									text = mir_tstrdup(TzDescr);
								} else {
									wchar_t tmp[10];
									_ltot(ppro->GetSettingByte(setting[lvi.iItem].szDbSetting), tmp, 10);
									text = mir_tstrdup(tmp);
								}
								break;
							}
							case DBVT_WORD:
							{
								wchar_t tmp[10];
								_ltot(ppro->GetSettingWord(setting[lvi.iItem].szDbSetting), tmp, 10);
								text = mir_tstrdup(tmp);
								//text = (wchar_t*)ppro->GetSettingWord(setting[lvi.iItem].szDbSetting);
								break;
							}
						}
						ListView_SetItemText(hwndList, lvi.iItem, 1, text);
						if (setting[lvi.iItem].dbType == DBVT_WORD || (strcmp(setting[lvi.iItem].szDbSetting, "Gender") && setting[lvi.iItem].dbType == DBVT_BYTE))
							mir_free(text);
					}
				}
				break;
			}
			break;
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
	odp.flags = ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.hInstance = g_hInstance;
	odp.dwInitParam = LPARAM(this);
	odp.position = -1900000000;
	odp.ptszTitle = m_tszUserName;

	HANDLE hContact = (HANDLE)lParam;
	if (hContact) {
		char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto != NULL && !strcmp(szProto, m_szModuleName)) {
			odp.pfnDlgProc = SkypeDlgProc;
			odp.pszTemplate = MAKEINTRESOURCEA(IDD_INFO_SKYPE);
			UserInfo_AddPage(wParam, &odp);
		}
	} else {
		odp.pfnDlgProc = OwnSkypeDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OWNINFO_SKYPE);
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}