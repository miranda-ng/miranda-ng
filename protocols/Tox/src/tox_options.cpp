#include "common.h"

INT_PTR CALLBACK CToxProto::MainOptionsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			ptrA username(proto->getStringA("Username"));
			SetDlgItemTextA(hwnd, IDC_USERNAME, username);

			std::string toxProfilePath = proto->GetToxProfilePath();
			SetDlgItemTextA(hwnd, IDC_DATAPATH, toxProfilePath.c_str());

			ptrW groupName(proto->getTStringA(TOX_SETTINGS_GROUP));
			SetDlgItemText(hwnd, IDC_GROUP, groupName);
			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 64, 0);

			CheckDlgButton(hwnd, IDC_DISABLE_UDP, proto->getByte("DisableUDP", 0));
			CheckDlgButton(hwnd, IDC_DISABLE_IPV6, proto->getByte("DisableIPv6", 0));
		}
		return TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_USERNAME:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				char username[128];
				GetDlgItemTextA(hwnd, IDC_USERNAME, username, SIZEOF(username));
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_DATAPATH:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				char dataPath[128];
				GetDlgItemTextA(hwnd, IDC_DATAPATH, dataPath, SIZEOF(dataPath));
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_BROWSE:
		{
			char dataPath[MAX_PATH];
			GetDlgItemTextA(hwnd, IDC_DATAPATH, dataPath, SIZEOF(dataPath));

			char filter[MAX_PATH] = "";
			mir_snprintf(filter, MAX_PATH, "%s\0*.*\0", Translate("All Files (*.*)"));

			OPENFILENAMEA ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = 0;
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = dataPath;
			ofn.lpstrTitle = Translate("Select data file");
			ofn.nMaxFile = SIZEOF(dataPath);
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
			if (GetOpenFileNameA(&ofn) && dataPath[0])
			{
				SetDlgItemTextA(hwnd, IDC_DATAPATH, dataPath);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
			break;

		case IDC_GROUP:
		{
			if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
			break;

		case IDC_DISABLE_UDP:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_DISABLE_IPV6:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
	}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
		{
			char username[128];
			GetDlgItemTextA(hwnd, IDC_USERNAME, username, SIZEOF(username));
			proto->setString("Username", username);
			tox_set_name(proto->tox, (uint8_t*)&username[0], strlen(username));

			proto->setByte("DisableUDP", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_UDP));
			proto->setByte("DisableIPv6", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_IPV6));

			wchar_t groupName[128];
			GetDlgItemText(hwnd, IDC_GROUP, groupName, SIZEOF(groupName));
			if (lstrlen(groupName) > 0)
			{
				proto->setWString(NULL, TOX_SETTINGS_GROUP, groupName);
				Clist_CreateGroup(0, groupName);
			}
			else
				proto->delSetting(NULL, TOX_SETTINGS_GROUP);

			char dataPath[128];
			GetDlgItemTextA(hwnd, IDC_DATAPATH, dataPath, SIZEOF(dataPath));
			if (proto->GetToxProfilePath().compare(dataPath) != 0)
			{
				proto->UninitToxCore();
				proto->setString("DataPath", dataPath);
				proto->InitToxCore();
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}