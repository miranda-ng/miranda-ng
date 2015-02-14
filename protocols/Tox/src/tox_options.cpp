#include "common.h"

HWND hAddNodeDlg;

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
	SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

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
			char passw[MAX_PATH];
			/*{
				if (!GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter Feed name"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str)) || mir_tstrcmp(str, _T("http://")) == 0) {
					MessageBox(hwndDlg, TranslateT("Enter Feed URL"), TranslateT("Error"), MB_OK);
					break;
				}
				if (!GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str))) {
					MessageBox(hwndDlg, TranslateT("Enter message format"), TranslateT("Error"), MB_OK);
					break;
				}

				MCONTACT hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)MODULE);
				GetDlgItemText(hwndDlg, IDC_FEEDTITLE, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "Nick", str);

				HWND hwndList = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				GetDlgItemText(hwndDlg, IDC_FEEDURL, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "URL", str);
				db_set_b(hContact, MODULE, "CheckState", 1);
				db_set_dw(hContact, MODULE, "UpdateTime", (DWORD)GetDlgItemInt(hwndDlg, IDC_CHECKTIME, NULL, false));
				GetDlgItemText(hwndDlg, IDC_TAGSEDIT, str, SIZEOF(str));
				db_set_ts(hContact, MODULE, "MsgFormat", str);
				db_set_w(hContact, MODULE, "Status", CallProtoService(MODULE, PS_GETSTATUS, 0, 0));
				if (IsDlgButtonChecked(hwndDlg, IDC_USEAUTH)) {
					db_set_b(hContact, MODULE, "UseAuth", 1);
					GetDlgItemText(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
					db_set_ts(hContact, MODULE, "Login", str);
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, passw, SIZEOF(passw));
					db_set_s(hContact, MODULE, "Password", passw);
				}
				DeleteAllItems(hwndList);
				UpdateList(hwndList);
			}*/
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

INT_PTR CALLBACK ToxNodesOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_NODESLIST);
	int sel;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		CreateList(hwndList);
		//UpdateList(hwndList);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADDNODE:
			if (hAddNodeDlg == 0)
				hAddNodeDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(IDD_ADDNODE), hwndDlg, AddNodeDlgProc, (LPARAM)hwndList);
			return FALSE;

		case IDC_CHANGE:
			sel = ListView_GetSelectionMark(hwndList);
			/*{
				ItemInfo SelItem = { 0 };
				ListView_GetItemText(hwndList, sel, 0, SelItem.nick, SIZEOF(SelItem.nick));
				ListView_GetItemText(hwndList, sel, 1, SelItem.url, SIZEOF(SelItem.url));
				SelItem.hwndList = hwndList;
				SelItem.SelNumber = sel;
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), hwndDlg, DlgProcChangeFeedOpts, (LPARAM)&SelItem);
			}*/
			return FALSE;

		case IDC_REMOVE:
			if (MessageBox(hwndDlg, TranslateT("Are you sure?"), TranslateT("Contact deleting"), MB_YESNO | MB_ICONWARNING) == IDYES) {
				TCHAR nick[MAX_PATH], url[MAX_PATH];
				int sel = ListView_GetSelectionMark(hwndList);
				ListView_GetItemText(hwndList, sel, 0, nick, SIZEOF(nick));
				ListView_GetItemText(hwndList, sel, 1, url, SIZEOF(url));

				for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
					ptrT dbNick(db_get_tsa(hContact, MODULE, "Nick"));
					if (dbNick == NULL)
						break;
					if (mir_tstrcmp(dbNick, nick))
						continue;

					ptrT dbURL(db_get_tsa(hContact, MODULE, "URL"));
					if (dbURL == NULL)
						break;
					if (mir_tstrcmp(dbURL, url))
						continue;

					CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
					ListView_DeleteItem(hwndList, sel);
					break;
				}
			}
			return FALSE;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR *)lParam;
		switch (hdr->code) {
		case PSN_APPLY:
			//db_set_b(NULL, MODULE, "StartupRetrieve", IsDlgButtonChecked(hwndDlg, IDC_STARTUPRETRIEVE));
			{
				int i = 0;
				for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
					db_set_b(hContact, MODULE, "CheckState", ListView_GetCheckState(hwndList, i));
					if (!ListView_GetCheckState(hwndList, i))
						db_set_b(hContact, "CList", "Hidden", 1);
					else
						db_unset(hContact, "CList", "Hidden");
					i++;
				}
			}
			break;

		case NM_DBLCLK:
			sel = ListView_GetHotItem(hwndList);
			/*if (sel != -1) {
				ItemInfo SelItem = { 0 };
				ListView_GetItemText(hwndList, sel, 0, SelItem.nick, SIZEOF(SelItem.nick));
				ListView_GetItemText(hwndList, sel, 1, SelItem.url, SIZEOF(SelItem.url));
				SelItem.hwndList = hwndList;
				SelItem.SelNumber = sel;
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDFEED), hwndDlg, DlgProcChangeFeedOpts, (LPARAM)&SelItem);
			}*/
			break;

		case LVN_ITEMCHANGED:
			NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
			//if (((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) && !UpdateListFlag)
			//	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
	}
	return FALSE;
}
