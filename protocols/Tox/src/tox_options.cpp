#include "common.h"

HWND hAddNodeDlg, hChangeNodeDlg;
bool UpdateListFlag = false;

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

void CreateList(HWND hwndList)
{
	LVCOLUMNA lvc = { 0 };
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
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
	lvc.cx = 150;
	SendMessage(hwndList, LVM_INSERTCOLUMNA, 3, (LPARAM)&lvc);

	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
}

void UpdateList(HWND hwndList)
{
	LVITEMA lvI = { 0 };
	char setting[MAX_PATH];

	int nodeCount = db_get_w(NULL, "TOX", TOX_SETTINGS_NODE_COUNT, 0);
	for (int i = 0; i < nodeCount; i++)
	{
		UpdateListFlag = true;
		lvI.mask = LVIF_TEXT;

		mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, i + 1);
		lvI.pszText = db_get_sa(NULL, "TOX", setting);
		lvI.iSubItem = 0;
		lvI.iItem = i;
		SendMessage(hwndList, LVM_INSERTITEMA, 0, (LPARAM)&lvI);

		mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, i + 1);
		lvI.iSubItem = 1;
		lvI.pszText = db_get_sa(NULL, "TOX", setting);
		SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvI);

		mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, i + 1);
		int port = db_get_w(NULL, "TOX", setting, 0);
		char portNum[10];
		_itoa(port, portNum, 10);
		lvI.iSubItem = 2;
		lvI.pszText = portNum;
		SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvI);

		mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, i + 1);
		lvI.iSubItem = 3;
		lvI.pszText = db_get_sa(NULL, "TOX", setting);
		SendMessage(hwndList, LVM_SETITEMA, 0, (LPARAM)&lvI);
	}
	UpdateListFlag = false;
}

void DeleteAllItems(HWND hwndList)
{
	ListView_DeleteAllItems(hwndList);
}

INT_PTR CALLBACK AddNodeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		SetWindowText(hwndDlg, TranslateT("Add node"));
		SetDlgItemInt(hwndDlg, IDC_PORT, 33445, TRUE);
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "AddNodeDlg");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				char value[MAX_PATH];

				if (!GetDlgItemTextA(hwndDlg, IDC_IPV4, value, SIZEOF(value))) {
					MessageBox(hwndDlg, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemTextA(hwndDlg, IDC_CLIENTID, value, SIZEOF(value))) {
					MessageBox(hwndDlg, TranslateT("Enter public key"), TranslateT("Error"), MB_OK);
					break;
				}

				char setting[MAX_PATH];
				int nodeCount = db_get_b(NULL, "TOX", "NodeCount", 0);

				GetDlgItemTextA(hwndDlg, IDC_IPV4, value, SIZEOF(value));
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, nodeCount + 1);
				db_set_s(NULL, "TOX", setting, value);

				GetDlgItemTextA(hwndDlg, IDC_IPV6, value, SIZEOF(value));
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, nodeCount + 1);
				db_set_s(NULL, "TOX", setting, value);

				GetDlgItemTextA(hwndDlg, IDC_CLIENTID, value, SIZEOF(value));
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, nodeCount + 1);
				db_set_s(NULL, "TOX", setting, value);

				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, nodeCount + 1);
				db_set_w(NULL, "TOX", setting, GetDlgItemInt(hwndDlg, IDC_PORT, NULL, false));

				db_set_w(NULL, "TOX", TOX_SETTINGS_NODE_COUNT, nodeCount + 1);

				HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				DeleteAllItems(hwndList);
				UpdateList(hwndList);
			}
			// fall through

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		hAddNodeDlg = 0;
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "AddNodeDlg");
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK ChangeNodeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			ItemInfo &SelItem = *(ItemInfo*)lParam;
			ItemInfo *nSelItem = new ItemInfo(SelItem);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)nSelItem);
			SetWindowText(hwndDlg, TranslateT("Change node"));

			char setting[MAX_PATH];
			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, SelItem.SelNumber + 1);
			ptrA addressIPv4(db_get_sa(NULL, "TOX", setting));

			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, SelItem.SelNumber + 1);
			ptrA addressIPv6(db_get_sa(NULL, "TOX", setting));

			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, SelItem.SelNumber + 1);
			int port = db_get_dw(NULL, "TOX", setting, 33445);

			mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, SelItem.SelNumber + 1);
			ptrA pubKey(db_get_sa(NULL, "TOX", setting));

			SetDlgItemTextA(hwndDlg, IDC_IPV4, addressIPv4);
			SetDlgItemTextA(hwndDlg, IDC_IPV6, addressIPv6);
			SetDlgItemInt(hwndDlg, IDC_PORT, port, TRUE);
			SetDlgItemTextA(hwndDlg, IDC_CLIENTID, pubKey);

			Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "ChangeNodeDlg");
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			TCHAR str[MAX_PATH];
			{
				ItemInfo *SelItem = (ItemInfo*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				if (!GetDlgItemText(hwndDlg, IDC_IPV4, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter public key"), TranslateT("Error"), MB_OK);
					break;
				}

				char buff[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_IPV4, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), TOX_SETTINGS_NODE_IPV4, SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_IPV6, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), TOX_SETTINGS_NODE_IPV6, SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), TOX_SETTINGS_NODE_PKEY, SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				mir_snprintf(buff, SIZEOF(buff), TOX_SETTINGS_NODE_PORT, SelItem->SelNumber + 1);
				db_set_dw(NULL, "TOX", buff, (DWORD)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, false));

				DeleteAllItems(SelItem->hwndList);
				UpdateList(SelItem->hwndList);
			}
			// fall through

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		ItemInfo *SelItem = (ItemInfo *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		hChangeNodeDlg = 0;
		Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ChangeNodeDlg");
		delete SelItem;
		break;
	}

	return FALSE;
}

INT_PTR CALLBACK ToxNodesOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_NODESLIST);
	int sel;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		CreateList(hwndList);
		UpdateList(hwndList);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADDNODE:
			if (hAddNodeDlg == 0)
				hAddNodeDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_ADDNODE), hwndDlg, AddNodeDlgProc, (LPARAM)hwndList);
			return FALSE;

		case IDC_CHANGE:
			if (hChangeNodeDlg == 0) {
				ItemInfo SelItem = { 0 };
				SelItem.hwndList = hwndList;
				SelItem.SelNumber = ListView_GetSelectionMark(hwndList);
				hChangeNodeDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_ADDNODE), hwndDlg, ChangeNodeDlgProc, (LPARAM)&SelItem);
			}
			return FALSE;

		case IDC_REMOVE:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES)
			{
				char setting[MAX_PATH];
				int sel = ListView_GetSelectionMark(hwndList);
				int nodeCount = db_get_b(NULL, "TOX", "NodeCount", 0);
				for (int i = sel + 1; i < nodeCount; i++)
				{
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, i + 1);
					ptrA addressIPv4(db_get_sa(NULL, "TOX", setting));
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, i);
					db_set_s(NULL, "TOX", setting, addressIPv4);

					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, i + 1);
					ptrA addressIPv6(db_get_sa(NULL, "TOX", setting));
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, i);
					db_set_s(NULL, "TOX", setting, addressIPv6);

					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, i + 1);
					int port = db_get_w(NULL, "TOX", setting, 0);
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, i);
					db_set_w(NULL, "TOX", setting, port);

					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, i + 1);
					ptrA pubKey(db_get_sa(NULL, "TOX", setting));
					mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, i);
					db_set_s(NULL, "TOX", setting, setting);
				}
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV4, nodeCount);
				db_unset(NULL, "TOX", setting);
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_IPV6, nodeCount);
				db_unset(NULL, "TOX", setting);
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PORT, nodeCount);
				db_unset(NULL, "TOX", setting);
				mir_snprintf(setting, SIZEOF(setting), TOX_SETTINGS_NODE_PKEY, nodeCount);
				db_unset(NULL, "TOX", setting);

				db_set_b(NULL, "TOX", TOX_SETTINGS_NODE_COUNT, nodeCount - 1);
				ListView_DeleteItem(hwndList, sel);

			}
			return FALSE;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR *)lParam;
		switch (hdr->code) {
		case NM_DBLCLK:
			sel = ListView_GetHotItem(hwndList);
			if (sel != -1) {
				ItemInfo SelItem = { 0 };
				SelItem.hwndList = hwndList;
				SelItem.SelNumber = sel;
				CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_ADDNODE), hwndDlg, ChangeNodeDlgProc, (LPARAM)&SelItem);
			}
			break;

		case LVN_ITEMCHANGED:
			NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
			if (((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) && !UpdateListFlag)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	}
	return FALSE;
}
