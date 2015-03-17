#include "common.h"

INT_PTR CSkypeProto::MainOptionsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSkypeProto *proto = (CSkypeProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CSkypeProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			ptrA login(proto->getStringA(SKYPE_SETTINGS_ID));
			SetDlgItemTextA(hwnd, IDC_LOGIN, login);
			SendDlgItemMessage(hwnd, IDC_LOGIN, EM_LIMITTEXT, 32, 0);

			ptrA password(proto->getStringA("Password"));
			SetDlgItemTextA(hwnd, IDC_PASSWORD, password);
			SendDlgItemMessage(hwnd, IDC_PASSWORD, EM_LIMITTEXT, 20, 0);

			ptrT group(proto->getTStringA(SKYPE_SETTINGS_GROUP));
			SetDlgItemText(hwnd, IDC_GROUP, group ? group : _T("Skype"));
			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 64, 0);
		}
		return TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_LOGIN:
		case IDC_GROUP:
		case IDC_PASSWORD:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;
		}
	}
	break;

	case WM_NOTIFY:
		if (((NMHDR*)lParam)->code == PSN_APPLY)
		{
			char login[32];
			GetDlgItemTextA(hwnd, IDC_LOGIN, login, SIZEOF(login));
			proto->setString(SKYPE_SETTINGS_ID, login);

			char password[20];
			GetDlgItemTextA(hwnd, IDC_PASSWORD, password, SIZEOF(password));
			proto->setString(SKYPE_SETTINGS_PASSWORD, password);

			TCHAR group[64];
			GetDlgItemText(hwnd, IDC_GROUP, group, SIZEOF(group));
			if (_tcslen(group) > 0)
			{
				proto->setTString(SKYPE_SETTINGS_GROUP, group);
				Clist_CreateGroup(0, group);
			}
			else
			{
				proto->delSetting(NULL, SKYPE_SETTINGS_GROUP);
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}

int CSkypeProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	char *title = mir_t2a(m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pfnDlgProc = MainOptionsProc;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}