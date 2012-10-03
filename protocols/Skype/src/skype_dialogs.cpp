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

		SetDlgItemText(hwnd, IDC_SL, proto->GetSettingString(SKYPE_SETTINGS_LOGIN, L""));
		SetDlgItemText(hwnd, IDC_SL, proto->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD, L""));

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

		SetDlgItemText(hwnd, IDC_SL, proto->GetSettingString(SKYPE_SETTINGS_LOGIN, L""));
		SetDlgItemText(hwnd, IDC_PW, proto->GetDecodeSettingString(SKYPE_SETTINGS_PASSWORD, L""));

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
