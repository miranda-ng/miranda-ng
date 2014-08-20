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

			ptrW nick(proto->getTStringA("Nick"));
			SetDlgItemText(hwnd, IDC_NAME, nick);

			ptrA toxId(proto->getStringA(TOX_SETTINGS_ID));
			SetDlgItemTextA(hwnd, IDC_TOXID, toxId);

			ptrW group(proto->getTStringA(TOX_SETTINGS_GROUP));
			SetDlgItemText(hwnd, IDC_GROUP, group);
			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 64, 0);

			CheckDlgButton(hwnd, IDC_DISABLE_UDP, proto->getByte("DisableUDP", 0));
			CheckDlgButton(hwnd, IDC_DISABLE_IPV6, proto->getByte("DisableIPv6", 1));
		}
		return TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_NAME:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_CLIPBOARD:
			{
				char toxId[TOX_FRIEND_ADDRESS_SIZE * 2 + 1];
				GetDlgItemTextA(hwnd, IDC_TOXID, toxId, SIZEOF(toxId));
				if (OpenClipboard(GetDlgItem(hwnd, IDC_TOXID)))
				{
					EmptyClipboard();
					HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, SIZEOF(toxId));
					memcpy(GlobalLock(hMem), toxId, SIZEOF(toxId));
					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT, hMem);
					CloseClipboard();
				}
			}
			break;

		case IDC_GROUP:
			if ((HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
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
			TCHAR nick[TOX_MAX_NAME_LENGTH];
			GetDlgItemText(hwnd, IDC_NAME, nick, TOX_MAX_NAME_LENGTH);
			proto->setTString("Nick", nick);

			proto->setByte("DisableUDP", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_UDP));
			proto->setByte("DisableIPv6", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_IPV6));

			TCHAR group[64];
			GetDlgItemText(hwnd, IDC_GROUP, group, SIZEOF(group));
			if (_tcslen(group) > 0)
			{
				proto->setTString(NULL, TOX_SETTINGS_GROUP, group);
				Clist_CreateGroup(0, group);
			}
			else
			{
				proto->delSetting(NULL, TOX_SETTINGS_GROUP);
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK CToxProto::ToxProfileManagerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TCHAR *profilePath = (TCHAR*)GetWindowLongPtr(hwnd, DWLP_USER);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			profilePath = (TCHAR*)mir_calloc(sizeof(TCHAR) * MAX_PATH);
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)profilePath);

			CheckDlgButton(hwnd, IDC_CREATE_NEW, TRUE);
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;

	case WM_DESTROY:
		mir_free(profilePath);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_USE_EXISTING:
			EnableWindow(GetDlgItem(hwnd, IDC_PROFILE_PATH), IsDlgButtonChecked(hwnd, IDC_USE_EXISTING));
			EnableWindow(GetDlgItem(hwnd, IDC_BROWSE_PROFILE), IsDlgButtonChecked(hwnd, IDC_USE_EXISTING));
			break;

		case IDC_BROWSE_PROFILE:
			{
				TCHAR filter[MAX_PATH] = { 0 };
				mir_sntprintf(filter, MAX_PATH, _T("%s\0*.*"), TranslateT("All files (*.*)"));

				OPENFILENAME ofn = { sizeof(ofn) };
				ofn.hwndOwner = hwnd;
				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = profilePath;
				ofn.lpstrTitle = TranslateT("Select tox profile");
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER;

				if (GetOpenFileName(&ofn) && profilePath)
				{
					SetDlgItemText(hwnd, IDC_PROFILE_PATH, profilePath);
				}
			}
			break;

		case IDOK:
			{
				if (IsDlgButtonChecked(hwnd, IDC_USE_EXISTING))
				{
					if (profilePath != NULL)
					{
						std::tstring toxProfilePath = proto->GetToxProfilePath();
						CopyFile(profilePath, toxProfilePath.c_str(), FALSE);
					}
				}
				EndDialog(hwnd, 1);
			}
			break;

		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		}
		break;
	}

	return FALSE;
}