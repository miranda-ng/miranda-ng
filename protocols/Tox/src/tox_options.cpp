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
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc = { 0 };
	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text, and
	// subitem members of the structure are valid.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;

	lvc.iSubItem = 0;
	lvc.pszText = TranslateT("IPv4");
	lvc.cx = 100;     // width of column in pixels
	ListView_InsertColumn(hwndList, 0, &lvc);

	lvc.iSubItem = 1;
	lvc.pszText = TranslateT("IPv6");
	lvc.cx = 100;     // width of column in pixels
	ListView_InsertColumn(hwndList, 1, &lvc);

	lvc.iSubItem = 2;
	lvc.pszText = TranslateT("Port");
	lvc.cx = 100;     // width of column in pixels
	ListView_InsertColumn(hwndList, 2, &lvc);

	lvc.iSubItem = 3;
	lvc.pszText = TranslateT("Client ID");
	lvc.cx = 100;     // width of column in pixels
	ListView_InsertColumn(hwndList, 3, &lvc);
}

void UpdateList(HWND hwndList)
{
	LVITEM lvI = { 0 };

	int NodeCount = db_get_b(NULL, "TOX", "NodeCount", 0);

	for (int i = 0; i < NodeCount; i++) {
		UpdateListFlag = true;
		lvI.mask = LVIF_TEXT;
		lvI.iSubItem = 0;
		char buff[MAX_PATH];
		mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", i + 1);
		TCHAR *ptszIPv4 = db_get_tsa(NULL, "TOX", buff);
		mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", i + 1);
		TCHAR *ptszIPv6 = db_get_tsa(NULL, "TOX", buff);
		mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", i + 1);
		TCHAR *ptszClientID = db_get_tsa(NULL, "TOX", buff);
		mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", i + 1);
		DWORD PortNum = db_get_dw(NULL, "TOX", buff, 0);
		TCHAR ptszPort[10];
		_itot(PortNum, ptszPort, 10);
		if (ptszIPv4 && ptszIPv6 && ptszClientID && ptszPort) {
			lvI.pszText = ptszIPv4;
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);

			lvI.iSubItem = 1;
			lvI.pszText = ptszIPv6;
			ListView_SetItem(hwndList, &lvI);

			lvI.iSubItem = 2;
			lvI.pszText = ptszPort;
			ListView_SetItem(hwndList, &lvI);

			lvI.iSubItem = 3;
			lvI.pszText = ptszClientID;
			ListView_SetItem(hwndList, &lvI);

			mir_free(ptszIPv4);
			mir_free(ptszIPv6);
			mir_free(ptszClientID);
		}
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
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "AddNodeDlg");
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			TCHAR str[MAX_PATH];
			{
				if (!GetDlgItemText(hwndDlg, IDC_IPV4, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter IPv4"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_IPV6, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter IPv6"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemInt(hwndDlg, IDC_PORT, NULL, false)) {
					MessageBox(hwndDlg, TranslateT("Enter port"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter client ID"), TranslateT("Error"), MB_OK);
					break;
				}

				int NodeCount = db_get_b(NULL, "TOX", "NodeCount", 0);
				char buff[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_IPV4, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", NodeCount + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_IPV6, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", NodeCount + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", NodeCount + 1);
				db_set_ts(NULL, "TOX", buff, str);

				mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", NodeCount + 1);
				db_set_dw(NULL, "TOX", buff, (DWORD)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, false));

				db_set_b(NULL, "TOX", "NodeCount", NodeCount + 1);

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

			char buff[MAX_PATH];
			mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", SelItem.SelNumber + 1);
			ptrT dbIPv4(db_get_tsa(NULL, "TOX", buff));
			if (dbIPv4 == NULL)
				break;

			mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", SelItem.SelNumber + 1);
			ptrT dbIPv6(db_get_tsa(NULL, "TOX", buff));
			if (dbIPv6 == NULL)
				break;

			mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", SelItem.SelNumber + 1);
			ptrT dbClientID(db_get_tsa(NULL, "TOX", buff));
			if (dbClientID == NULL)
				break;

			mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", SelItem.SelNumber + 1);
			DWORD Port = db_get_dw(NULL, "TOX", buff, 0);
			if (Port == 0)
				break;
			SetDlgItemText(hwndDlg, IDC_IPV4, dbIPv4);
			SetDlgItemText(hwndDlg, IDC_IPV6, dbIPv6);
			SetDlgItemText(hwndDlg, IDC_CLIENTID, dbClientID);
			SetDlgItemInt(hwndDlg, IDC_PORT, Port, TRUE);

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
				if (!GetDlgItemText(hwndDlg, IDC_IPV6, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter IPv6"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemInt(hwndDlg, IDC_PORT, NULL, false)) {
					MessageBox(hwndDlg, TranslateT("Enter port"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter client ID"), TranslateT("Error"), MB_OK);
					break;
				}

				char buff[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_IPV4, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_IPV6, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				GetDlgItemText(hwndDlg, IDC_CLIENTID, str, SIZEOF(str));
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", SelItem->SelNumber + 1);
				db_set_ts(NULL, "TOX", buff, str);

				mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", SelItem->SelNumber + 1);
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
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Node deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
				int sel = ListView_GetSelectionMark(hwndList);
				char buff[MAX_PATH];
				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", sel + 1);
				db_unset(NULL, "TOX", buff);

				mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", sel + 1);
				db_unset(NULL, "TOX", buff);

				mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", sel + 1);
				db_unset(NULL, "TOX", buff);

				mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", sel + 1);
				db_unset(NULL, "TOX", buff);

				int NodeCount = db_get_b(NULL, "TOX", "NodeCount", 0);

				for (int i = sel + 1; i < NodeCount; i++) {
					mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", i + 1);
					TCHAR *ptszIPv4 = db_get_tsa(NULL, "TOX", buff);
					db_unset(NULL, "TOX", buff);
					mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv4", i);
					db_set_ts(NULL, "TOX", buff, ptszIPv4);

					mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", i + 1);
					TCHAR *ptszIPv6 = db_get_tsa(NULL, "TOX", buff);
					db_unset(NULL, "TOX", buff);
					mir_snprintf(buff, SIZEOF(buff), "Node_%d_IPv6", i);
					db_set_ts(NULL, "TOX", buff, ptszIPv6);

					mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", i + 1);
					TCHAR *ptszClientID = db_get_tsa(NULL, "TOX", buff);
					db_unset(NULL, "TOX", buff);
					mir_snprintf(buff, SIZEOF(buff), "Node_%d_ClientID", i);
					db_set_ts(NULL, "TOX", buff, ptszClientID);

					mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", i + 1);
					DWORD Port = db_get_dw(NULL, "TOX", buff, 0);
					db_unset(NULL, "TOX", buff);
					mir_snprintf(buff, SIZEOF(buff), "Node_%d_Port", i);
					db_set_dw(NULL, "TOX", buff, Port);
				}

				db_set_b(NULL, "TOX", "NodeCount", NodeCount - 1);
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
