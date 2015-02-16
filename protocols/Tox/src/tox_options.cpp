#include "common.h"

static WNDPROC oldWndProc = NULL;

INT_PTR CToxProto::MainOptionsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CToxProto *proto = (CToxProto*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			proto = (CToxProto*)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

			ptrT nick(proto->getTStringA("Nick"));
			SetDlgItemText(hwnd, IDC_NAME, nick);

			ptrT pass(proto->getTStringA("Password"));
			SetDlgItemText(hwnd, IDC_PASSWORD, pass);

			ptrA address(proto->getStringA(TOX_SETTINGS_ID));
			if (address != NULL)
			{
				SetDlgItemTextA(hwnd, IDC_TOXID, address);
			}

			ptrT group(proto->getTStringA(TOX_SETTINGS_GROUP));
			SetDlgItemText(hwnd, IDC_GROUP, group);
			SendDlgItemMessage(hwnd, IDC_GROUP, EM_LIMITTEXT, 64, 0);

			CheckDlgButton(hwnd, IDC_DISABLE_UDP, proto->getBool("DisableUDP", 0));
			CheckDlgButton(hwnd, IDC_DISABLE_IPV6, proto->getBool("DisableIPv6", 0));
		}
		return TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_NAME:
		case IDC_GROUP:
		case IDC_PASSWORD:
			if ((HWND)lParam == GetFocus())
			{
				if (HIWORD(wParam) != EN_CHANGE) return 0;
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_DISABLE_UDP:
		case IDC_DISABLE_IPV6:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}

	case IDC_CLIPBOARD:
		{
			char toxId[TOX_FRIEND_ADDRESS_SIZE * 2 + 1];
			GetDlgItemTextA(hwnd, IDC_TOXID, toxId, SIZEOF(toxId));
			if (OpenClipboard(GetDlgItem(hwnd, IDC_TOXID)))
			{
				EmptyClipboard();
				HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, sizeof(toxId));
				memcpy(GlobalLock(hMem), toxId, sizeof(toxId));
				GlobalUnlock(hMem);
				SetClipboardData(CF_TEXT, hMem);
				CloseClipboard();
			}
		}
		break;
	}
	break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lParam)->code == PSN_APPLY)
		{
			TCHAR nick[TOX_MAX_NAME_LENGTH];
			GetDlgItemText(hwnd, IDC_NAME, nick, TOX_MAX_NAME_LENGTH);
			proto->setTString("Nick", nick);

			TCHAR password[MAX_PATH];
			GetDlgItemText(hwnd, IDC_PASSWORD, password, SIZEOF(password));
			proto->setTString("Password", password);
			if (proto->password != NULL)
			{
				mir_free(proto->password);
				proto->password = NULL;
			}
			proto->password = mir_utf8encodeW(password);

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

			proto->setByte("DisableUDP", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_UDP));
			proto->setByte("DisableIPv6", (BYTE)IsDlgButtonChecked(hwnd, IDC_DISABLE_IPV6));

			if (proto->IsOnline())
			{
				//proto->SaveToxProfile();
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK EditNodeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	LONG_PTR iItem = (LONG_PTR)GetWindowLongPtr(hwndDlg, DWLP_USER);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			hwndList = (HWND)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			iItem = ListView_GetHotItem(hwndList);
			if (iItem == -1)
			{
				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)-1);
				SetWindowText(hwndDlg, TranslateT("Add node"));
			}
			else
			{
				SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)iItem);
				SetWindowText(hwndDlg, TranslateT("Change node"));

				LVITEMA lvi = { 0 };
				lvi.mask = LVIF_TEXT;
				lvi.iItem = iItem;
				lvi.cchTextMax = MAX_PATH;
				lvi.pszText = (char*)mir_alloc(MAX_PATH);

				lvi.iSubItem = 0;
				SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
				SetDlgItemTextA(hwndDlg, IDC_IPV4, lvi.pszText);

				lvi.iSubItem = 1;
				SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
				SetDlgItemTextA(hwndDlg, IDC_IPV6, lvi.pszText);

				lvi.iSubItem = 2;
				SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
				SetDlgItemTextA(hwndDlg, IDC_PORT, lvi.pszText);

				lvi.iSubItem = 3;
				SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
				SetDlgItemTextA(hwndDlg, IDC_PKEY, lvi.pszText);

				mir_free(lvi.pszText);
			}

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "EditNodeDlg");
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			char value[MAX_PATH];
			if (!GetDlgItemTextA(hwndDlg, IDC_IPV4, value, SIZEOF(value)))
			{
				MessageBox(hwndDlg, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
				break;
			}
			if (!GetDlgItemTextA(hwndDlg, IDC_PKEY, value, SIZEOF(value)))
			{
				MessageBox(hwndDlg, TranslateT("Enter public key"), TranslateT("Error"), MB_OK);
				break;
			}

			LVITEMA lvi = { 0 };
			lvi.mask = 0;
			lvi.iImage = -1;
			lvi.iItem = iItem;
			if (lvi.iItem == -1)
			{
				lvi.iItem = ListView_GetItemCount(hwndList);
				SendMessage(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&lvi);
				ListView_SetItemState(hwndList, lvi.iItem, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
				ListView_EnsureVisible(hwndList, lvi.iItem, TRUE);
			}
			lvi.cchTextMax = MAX_PATH;
			lvi.mask = LVIF_TEXT | LVIF_IMAGE;

			GetDlgItemTextA(hwndDlg, IDC_IPV4, value, SIZEOF(value));
			lvi.iSubItem = 0;
			lvi.pszText = mir_strdup(value);
			SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

			GetDlgItemTextA(hwndDlg, IDC_IPV6, value, SIZEOF(value));
			lvi.iSubItem = 1;
			lvi.pszText = mir_strdup(value);
			SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

			GetDlgItemTextA(hwndDlg, IDC_PORT, value, SIZEOF(value));
			lvi.iSubItem = 2;
			lvi.pszText = mir_strdup(value);
			SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

			GetDlgItemTextA(hwndDlg, IDC_PKEY, value, SIZEOF(value));
			lvi.iSubItem = 3;
			lvi.pszText = mir_strdup(value);
			SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

			lvi.mask = LVIF_IMAGE;
			lvi.iSubItem = 4;
			lvi.iImage = 0;
			ListView_SetItem(hwndList, &lvi);

			lvi.iSubItem = 5;
			lvi.iImage = 1;
			ListView_SetItem(hwndList, &lvi);

			EndDialog(hwndDlg, IDOK);
		}
		break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
		break;

	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "EditNodeDlg");
		break;
	}

	return FALSE;
}

LRESULT CALLBACK RowItemsSubProc(HWND hwndList, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_LBUTTONDOWN)
	{
		LVHITTESTINFO hi;
		hi.pt.x = LOWORD(lParam);
		hi.pt.y = HIWORD(lParam);
		ListView_SubItemHitTest(hwndList, &hi);
		if (hi.iSubItem == 4)
		{
			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_ADDNODE),
				GetParent(hwndList), EditNodeDlgProc,
				(LPARAM)hwndList) == IDOK)
			{
				SendMessage(GetParent(GetParent(hwndList)), PSM_CHANGED, 0, 0);
			}
		}
		else if (hi.iSubItem == 5)
		{
			if (MessageBox(hwndList, TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				ListView_DeleteItem(hwndList, hi.iItem);
				SendMessage(GetParent(GetParent(hwndList)), PSM_CHANGED, 0, 0);
			}
		}
	}

	return CallWindowProc(oldWndProc, hwndList, msg, wParam, lParam);
}

INT_PTR CALLBACK ToxNodesOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_NODESLIST);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			oldWndProc = (WNDPROC)SetWindowLongPtr(hwndList, GWLP_WNDPROC, (LONG_PTR)RowItemsSubProc);

			HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 0);
			HICON icon = LoadSkinnedIcon(SKINICON_OTHER_TYPING);
			ImageList_AddIcon(hImageList, icon); Skin_ReleaseIcon(icon);
			icon = LoadSkinnedIcon(SKINICON_OTHER_DELETE);
			ImageList_AddIcon(hImageList, icon); Skin_ReleaseIcon(icon);
			ListView_SetImageList(hwndList, hImageList, LVSIL_SMALL);

			ListView_SetExtendedListViewStyle(hwndList, LVS_EX_SUBITEMIMAGES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

			LVCOLUMNA lvc = { 0 };
			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = Translate("IPv4");
			lvc.cx = 100;
			SendMessage(hwndList, LVM_INSERTCOLUMNA, 0, (LPARAM)&lvc);

			lvc.iSubItem = 1;
			lvc.pszText = Translate("IPv6");
			lvc.cx = 100;
			SendMessage(hwndList, LVM_INSERTCOLUMNA, 1, (LPARAM)&lvc);

			lvc.iSubItem = 2;
			lvc.pszText = Translate("Port");
			lvc.cx = 50;
			SendMessage(hwndList, LVM_INSERTCOLUMNA, 2, (LPARAM)&lvc);

			lvc.iSubItem = 3;
			lvc.pszText = Translate("Public key");
			lvc.cx = 130;
			SendMessage(hwndList, LVM_INSERTCOLUMNA, 3, (LPARAM)&lvc);

			lvc.iSubItem = 4;
			lvc.pszText = "";
			lvc.cx = 32 - GetSystemMetrics(SM_CXVSCROLL);
			ListView_InsertColumn(hwndList, 4, &lvc);

			lvc.iSubItem = 5;
			lvc.cx = 32 - GetSystemMetrics(SM_CXVSCROLL);
			ListView_InsertColumn(hwndList, 5, &lvc);

			char setting[MAX_PATH];

			LVITEMA lvi = { 0 };
			lvi.cchTextMax = MAX_PATH;

			int nodeCount = db_get_w(NULL, MODULE, TOX_SETTINGS_NODE_COUNT, 0);
			for (lvi.iItem = 0; lvi.iItem < nodeCount; lvi.iItem++)
			{
				lvi.iImage = -1;
				lvi.mask = LVIF_TEXT | LVIF_IMAGE;

				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, lvi.iItem + 1);
				lvi.iSubItem = 0;
				lvi.pszText = db_get_sa(NULL, MODULE, setting);
				SendMessage(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&lvi);

				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, lvi.iItem + 1);
				lvi.iSubItem = 1;
				lvi.pszText = db_get_sa(NULL, MODULE, setting);
				SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, lvi.iItem + 1);
				int port = db_get_w(NULL, MODULE, setting, 33445);
				char portNum[10];
				itoa(port, portNum, 10);
				lvi.iSubItem = 2;
				lvi.pszText = mir_strdup(portNum);
				SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, lvi.iItem + 1);
				lvi.iSubItem = 3;
				lvi.pszText = db_get_sa(NULL, MODULE, setting);
				SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvi);

				lvi.mask = LVIF_IMAGE;
				lvi.iSubItem = 4;
				lvi.iImage = 0;
				ListView_SetItem(hwndList, &lvi);

				lvi.iSubItem = 5;
				lvi.iImage = 1;
				ListView_SetItem(hwndList, &lvi);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADDNODE:
			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_ADDNODE),
				hwndDlg, EditNodeDlgProc,
				(LPARAM)hwndList) == IDOK)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			return FALSE;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR*)lParam;
		switch (hdr->code)
		{
		case NM_DBLCLK:
		{
			if (DialogBoxParam(
				g_hInstance,
				MAKEINTRESOURCE(IDD_ADDNODE),
				hwndDlg, EditNodeDlgProc,
				(LPARAM)hwndList) == IDOK)
			{
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

		case PSN_APPLY:
			{
				char setting[MAX_PATH];

				LVITEMA lvi = { 0 };
				lvi.mask = LVIF_TEXT;
				lvi.cchTextMax = MAX_PATH;
				lvi.pszText = (char*)mir_alloc(MAX_PATH);

				int itemCount = ListView_GetItemCount(hwndList);
				for (lvi.iItem = 0; lvi.iItem < itemCount; lvi.iItem++)
				{
					if (itemCount)
					lvi.iSubItem = 0;
					SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, lvi.iItem + 1);
					db_set_s(NULL, MODULE, setting, lvi.pszText);

					lvi.iSubItem = 1;
					SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, lvi.iItem + 1);
					db_set_s(NULL, MODULE, setting, lvi.pszText);

					lvi.iSubItem = 2;
					SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, lvi.iItem + 1);
					db_set_w(NULL, MODULE, setting, atoi(lvi.pszText));

					lvi.iSubItem = 3;
					SendMessage(hwndList, LVM_GETITEMA, 0, (LPARAM)&lvi);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, lvi.iItem + 1);
					db_set_s(NULL, MODULE, setting, lvi.pszText);
				}

				int nodeCount = db_get_b(NULL, MODULE, TOX_SETTINGS_NODE_COUNT, 0);
				for (lvi.iItem = itemCount; lvi.iItem < nodeCount; lvi.iItem++)
				{
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, lvi.iItem + 1);
					db_unset(NULL, MODULE, setting);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, lvi.iItem + 1);
					db_unset(NULL, MODULE, setting);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, lvi.iItem + 1);
					db_unset(NULL, MODULE, setting);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, lvi.iItem + 1);
					db_unset(NULL, MODULE, setting);
				}

				db_set_b(NULL, MODULE, TOX_SETTINGS_NODE_COUNT, itemCount);
			}
			return TRUE;
		}
	}
	return FALSE;
}
