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

		wchar_t* pwd = proto->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD);
		SetDlgItemText(hwnd, IDC_PW, pwd);
		::mir_free(pwd);

		if ( proto->m_iStatus != ID_STATUS_OFFLINE)
		{
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
	{
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			TCHAR data[128];
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));			

			GetDlgItemText(hwnd, IDC_SL, data, sizeof(data));
			proto->SetSettingString(SKYPE_SETTINGS_LOGIN, data);

			GetDlgItemText(hwnd, IDC_PW, data, sizeof(data));
			proto->SetDecodeSettingString(SKYPE_SETTINGS_PASSWORD, data);

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

		wchar_t* data = proto->GetSettingString(SKYPE_SETTINGS_LOGIN, L"");
		SetDlgItemText(hwnd, IDC_SL, data);
		::mir_free(data);

		data = proto->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD, L"");
		SetDlgItemText(hwnd, IDC_PW, data);
		::mir_free(data);

		if (proto->m_iStatus != ID_STATUS_OFFLINE) 
		{
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
	{
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			wchar_t data[128];
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

			GetDlgItemText(hwnd, IDC_SL, data, sizeof(data));
			proto->SetSettingString(SKYPE_SETTINGS_LOGIN, data);

			GetDlgItemText(hwnd, IDC_PW, data, sizeof(data));
			proto->SetDecodeSettingString(SKYPE_SETTINGS_PASSWORD, data);

			return TRUE;
		}
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
				ppro->SetSettingByte(NULL, "RememberPassword", ppro->rememberPassword);

				::GetDlgItemText(hwndDlg, IDC_PASSWORD, ppro->password, sizeof(ppro->password));

				ppro->SignIn();

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

int __cdecl CSkypeProto::OnAccountManagerInit(WPARAM wParam, LPARAM lParam)
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
	odp.hInstance   = g_hInstance;
	odp.ptszTitle   = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags       = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.position    = 271828;
	odp.ptszGroup   = LPGENT("Network");
	odp.ptszTab     = LPGENT("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc  = SkypeOptionsProc;
	Options_AddPage(wParam, &odp);
	
	return 0;
}
