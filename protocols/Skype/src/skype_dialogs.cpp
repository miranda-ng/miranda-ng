#include "skype_proto.h"

INT_PTR CALLBACK CSkypeProto::SkypeMainOptionsProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *proto = reinterpret_cast<CSkypeProto*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_INITDIALOG:
		{
			::TranslateDialogDefault(hwnd);

			proto = (CSkypeProto*)lParam;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
			{
				char *sid = ::db_get_sa(NULL, proto->m_szModuleName, SKYPE_SETTINGS_LOGIN);
				SetDlgItemTextA(hwnd, IDC_SL, sid);
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
				SetDlgItemInt(hwnd, IDC_PORT, proto->GetSettingWord("Port", port), FALSE);
				SendMessage(GetDlgItem(hwnd, IDC_PORT), EM_SETLIMITTEXT, 5, 0);
			}
			{
				CheckDlgButton(hwnd, IDC_USE_ALT_PORTS, proto->GetSettingByte("UseAlternativePorts", 1));
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
			else if (proto->GetSettingWord("Status") > ID_STATUS_OFFLINE)
			{
				EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), FALSE); 
			}
		}
		return TRUE;

	case WM_COMMAND: 
		{
			switch(LOWORD(wParam))
			{
			case IDC_SL:
				{
					if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0;

					if (!proto->IsOnline() && proto->GetSettingWord("Status") <= ID_STATUS_OFFLINE)
					{
						char sid[128];
						GetDlgItemTextA(hwnd, IDC_SL, sid, SIZEOF(sid));
						EnableWindow(GetDlgItem(hwnd, IDC_REGISTER), ::strlen(sid));
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
						EnableWindow(GetDlgItem(hwnd, IDC_CHANGE_PWD), ::strlen(pwd));
					}
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_PORT:
			case IDC_USE_ALT_PORTS:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;

			case IDC_REGISTER:
				{
					char sid[128], pwd[128];
					GetDlgItemTextA(hwnd, IDC_SL, sid, SIZEOF(sid));
					GetDlgItemTextA(hwnd, IDC_PW, pwd, SIZEOF(pwd));
						
					CSkype::VALIDATERESULT reason;
					proto->skype->ValidatePassword(sid, pwd, reason);
						
					if (reason == CSkype::VALIDATED_OK)
					{
						CAccount::Ref account;
						proto->skype->GetAccount(sid, proto->account);
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
			char data[128];
			GetDlgItemTextA(hwnd, IDC_SL, data, SIZEOF(data));
			::db_set_s(NULL, proto->m_szModuleName, "sid", data);
			::mir_free(proto->login);
			proto->login = ::mir_strdup(data);

			GetDlgItemTextA(hwnd, IDC_PW, data, sizeof(data));
			::CallService(MS_DB_CRYPT_ENCODESTRING, strlen(data), LPARAM((char*)data));
			::db_set_s(NULL, proto->m_szModuleName, SKYPE_SETTINGS_PASSWORD, data);

			HWND item = GetDlgItem(hwnd, IDC_PORT);
			if (item)
			{
				BOOL error;
				int port = GetDlgItemInt(hwnd, IDC_PORT, &error, FALSE);
				proto->SetSettingWord("Port", port);
				proto->SetSettingByte("UseAlternativePorts", (BYTE)IsDlgButtonChecked(hwnd, IDC_USE_ALT_PORTS));
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR __cdecl CSkypeProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
{
	return (int)::CreateDialogParam(
		g_hInstance, 
		MAKEINTRESOURCE(IDD_ACCMGR), 
		(HWND)lParam, 
		&CSkypeProto::SkypeMainOptionsProc, 
		(LPARAM)this);
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
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_MAIN);
	odp.pfnDlgProc = SkypeMainOptionsProc;
	::Options_AddPage(wParam, &odp);

	return 0;
}

INT_PTR CALLBACK CSkypeProto::SkypePasswordRequestProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PasswordRequestBoxParam *param = reinterpret_cast<PasswordRequestBoxParam*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

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
				::mir_a2u(param->login));
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
	PasswordChangeBoxParam *param = reinterpret_cast<PasswordChangeBoxParam*>(::GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

	switch (msg) 
	{
	case WM_INITDIALOG:
		::TranslateDialogDefault(hwndDlg);

		param = (PasswordChangeBoxParam *)lParam;
		::SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			::SendMessage(hwndDlg, WM_SETICON, ICON_BIG,	(LPARAM)CSkypeProto::IconList[0].Handle);
			::SendMessage(hwndDlg, WM_SETICON, ICON_SMALL,	(LPARAM)CSkypeProto::IconList[0].Handle);
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

INT_PTR CALLBACK CSkypeProto::InviteToChatProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InviteChatParam *param = (InviteChatParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) 
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			param = (InviteChatParam*)lParam;

			HWND hwndClist = GetDlgItem(hwndDlg, IDC_CCLIST);
			SetWindowLongPtr(hwndClist, GWL_STYLE, GetWindowLongPtr(hwndClist, GWL_STYLE) & ~CLS_HIDEOFFLINE);

	//		WindowSetIcon(hwndDlg, "msn");
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_NCDESTROY:
//		WindowFreeIcon(hwndDlg);
		delete param;
		break;

	case WM_NOTIFY:
	{
		NMCLISTCONTROL* nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == IDC_CCLIST)
		{
			switch (nmc->hdr.code) 
			{
			case CLN_NEWCONTACT:
				if (param && (nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0) 
				{
					char *contacts = NULL;
					if (param->id)
					{
						HANDLE hContact = param->ppro->GetChatRoomByID(param->id);
						if (hContact && ::db_get_w(hContact, param->ppro->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
						{
							contacts = param->ppro->GetChatUsers(param->id);
						}
					}
					param->ppro->ChatValidateContact(nmc->hItem, nmc->hdr.hwndFrom, contacts);
				}
				break;

			case CLN_LISTREBUILT:
				if (param) 
				{
					char *contacts = NULL;
					if (param->id)
					{
						HANDLE hContact = param->ppro->GetChatRoomByID(param->id);
						if (hContact && ::db_get_w(hContact, param->ppro->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
						{
							contacts = param->ppro->GetChatUsers(param->id);
						}
					}
					param->ppro->ChatPrepare(NULL, nmc->hdr.hwndFrom, contacts);
				}
				break; 
			}
		}
	}
	break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
			case IDC_ADDSCR:
				if (param->ppro->IsOnline())
				{
					wchar_t sid[SKYPE_SID_LIMIT];
					::GetDlgItemText(hwndDlg, IDC_EDITSCR, sid, SIZEOF(sid));

					CLCINFOITEM cii = {0};
					cii.cbSize = sizeof(cii);
					cii.flags = CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
					cii.pszText = ::wcslwr(sid);

					HANDLE hItem = (HANDLE)::SendDlgItemMessage(
						hwndDlg, 
						IDC_CCLIST, 
						CLM_ADDINFOITEM, 
						0, 
						(LPARAM)&cii);
					::SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_SETCHECKMARK, (LPARAM)hItem, 1);
				}
				break;

			case IDOK:
				{
					HWND hwndList = ::GetDlgItem(hwndDlg, IDC_CCLIST);

					SEStringList invitedContacts;
					param->ppro->GetInviteContacts(NULL, hwndList, invitedContacts);

					char *chatID = ::mir_strdup(param->id);

					if (chatID)
					{
						for (uint i = 0; i < invitedContacts.size(); i++)
						{
							CContact::Ref contact;
							CContact::AVAILABILITY status;
							param->ppro->skype->GetContact(invitedContacts[i], contact);
							contact->GetPropAvailability(status);

							//todo: fix rank
							param->ppro->AddChatContact(
								chatID, 
								invitedContacts[i], 
								CParticipant::GetRankName(CParticipant::SPEAKER),
								status);
						}

						CConversation::Ref conversation;
						param->ppro->skype->GetConversationByIdentity(chatID, conversation);
						conversation->AddConsumers(invitedContacts);
					}
					else
					{
						chatID = param->ppro->StartChat(NULL, invitedContacts);

						for (uint i = 0; i < invitedContacts.size(); i++)
						{
							CContact::Ref contact;
							CContact::AVAILABILITY status;
							param->ppro->skype->GetContact(invitedContacts[i], contact);
							contact->GetPropAvailability(status);

							//todo: fix rank
							param->ppro->AddChatContact(
								chatID, 
								invitedContacts[i], 
								CParticipant::GetRankName(CParticipant::SPEAKER),
								status);
						}
					}
				}

				EndDialog(hwndDlg, IDOK);
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}