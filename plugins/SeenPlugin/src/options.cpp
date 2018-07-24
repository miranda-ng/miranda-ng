/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

struct helpstring
{
	wchar_t letter;
	const wchar_t *pwszHelp;
};

static helpstring section1[] = 
{
	{ 'Y', LPGENW("year (4 digits)") },
	{ 'y', LPGENW("year (2 digits)") },
	{ 'm', LPGENW("month") },
	{ 'E', LPGENW("name of month") },
	{ 'e', LPGENW("short name of month") },
	{ 'd', LPGENW("day") },
	{ 'W', LPGENW("weekday (full)") },
	{ 'w', LPGENW("weekday (abbreviated)") },
};

static helpstring section2[] =
{
	{ 'H', LPGENW("hours (24)") },
	{ 'h', LPGENW("hours (12)") },
	{ 'p', LPGENW("AM/PM") },
	{ 'M', LPGENW("minutes") },
	{ 'S', LPGENW("seconds") },
};

static helpstring section3[] =
{
	{ 'n', LPGENW("username") },
	{ 'N', LPGENW("nick") },
	{ 'u', LPGENW("UIN/handle") },
	{ 'G', LPGENW("group") },
	{ 's', LPGENW("status") },
	{ 'T', LPGENW("status message") },
	{ 'o', LPGENW("old status") },
	{ 'i', LPGENW("external IP") },
	{ 'r', LPGENW("internal IP") },
	{ 'C', LPGENW("client info") },
	{ 'P', LPGENW("protocol") },
	{ 'A', LPGENW("account") },
};

static helpstring section4[] =
{
	{ 't', LPGENW("tabulator") },
	{ 'b', LPGENW("line break") },
};

static void addSection(CMStringW &str, const wchar_t *pwszTitle, const helpstring *strings, int count)
{
	str.Append(TranslateW(pwszTitle)); str.AppendChar('\n');
	
	for (int i=0; i < count; i++)
		str.AppendFormat(L"%%%c: \t %s \n", strings[i].letter, TranslateW(strings[i].pwszHelp));

	str.AppendChar('\n');
}

INT_PTR CALLBACK OptsPopupsDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	DBVARIANT dbv;
	wchar_t szstamp[256];
	BOOL hasPopups;
	BYTE bchecked;

	switch (msg) {
	case WM_INITDIALOG:
		if (hasPopups = (ServiceExists(MS_POPUP_QUERY)) != 0)
			hasPopups = CallService(MS_POPUP_QUERY, PUQS_GETSTATUS, 0);

		TranslateDialogDefault(hdlg);
		ShowWindow(GetDlgItem(hdlg, IDC_POPUPS), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_POPUPSTAMP), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_LABTEXT), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_LABTTITLE), hasPopups ? SW_SHOW : SW_HIDE);
		ShowWindow(GetDlgItem(hdlg, IDC_POPUPSTAMPTEXT), hasPopups ? SW_SHOW : SW_HIDE);
		CheckDlgButton(hdlg, IDC_POPUPS, (db_get_b(NULL, S_MOD, "UsePopups", 0) & hasPopups) ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hdlg, IDC_POPUPS), hasPopups);
		hasPopups = IsDlgButtonChecked(hdlg, IDC_POPUPS);
		EnableWindow(GetDlgItem(hdlg, IDC_POPUPSTAMP), hasPopups);
		EnableWindow(GetDlgItem(hdlg, IDC_POPUPSTAMPTEXT), hasPopups);
		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
			char szSetting[100];
			mir_snprintf(szSetting, "Col_%d", i - ID_STATUS_OFFLINE);
			DWORD sett = db_get_dw(NULL, S_MOD, szSetting, StatusColors15bits[i - ID_STATUS_OFFLINE]);

			COLORREF back, text;
			GetColorsFromDWord(&back, &text, sett);
			SendDlgItemMessage(hdlg, i, CPM_SETCOLOUR, 0, back);
			SendDlgItemMessage(hdlg, i + 20, CPM_SETCOLOUR, 0, text);
			EnableWindow(GetDlgItem(hdlg, i), hasPopups);
			EnableWindow(GetDlgItem(hdlg, i + 20), hasPopups);
		}

		if (!db_get_ws(NULL, S_MOD, "PopupStamp", &dbv)) {
			SetDlgItemText(hdlg, IDC_POPUPSTAMP, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_POPUPSTAMP, DEFAULT_POPUPSTAMP);

		if (!db_get_ws(NULL, S_MOD, "PopupStampText", &dbv)) {
			SetDlgItemText(hdlg, IDC_POPUPSTAMPTEXT, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_POPUPSTAMPTEXT, DEFAULT_POPUPSTAMPTEXT);
		break;

	case WM_COMMAND:
		if ((HIWORD(wparam) == BN_CLICKED || HIWORD(wparam) == EN_CHANGE) && GetFocus() == (HWND)lparam)
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		else if (HIWORD(wparam) == CPN_COLOURCHANGED) {
			WORD idText, idBack;
			if (LOWORD(wparam) > ID_STATUS_OUTTOLUNCH) // we have clicked a text color
				idText = wparam, idBack = wparam - 20;
			else
				idText = wparam + 20, idBack = wparam;

			POPUPDATAT ppd = { 0 };
			ppd.colorBack = SendDlgItemMessage(hdlg, idBack, CPM_GETCOLOUR, 0, 0);
			ppd.colorText = SendDlgItemMessage(hdlg, idText, CPM_GETCOLOUR, 0, 0);
			DWORD temp = GetDWordFromColors(ppd.colorBack, ppd.colorText);
			GetColorsFromDWord(&ppd.colorBack, &ppd.colorText, temp);
			SendDlgItemMessage(hdlg, idBack, CPM_SETCOLOUR, 0, ppd.colorBack);
			SendDlgItemMessage(hdlg, idText, CPM_SETCOLOUR, 0, ppd.colorText);
			ppd.lchIcon = Skin_LoadProtoIcon(nullptr, idBack);

			GetDlgItemText(hdlg, IDC_POPUPSTAMP, szstamp, _countof(szstamp));
			wcsncpy(ppd.lptzContactName, ParseString(szstamp, NULL), MAX_CONTACTNAME);

			GetDlgItemText(hdlg, IDC_POPUPSTAMPTEXT, szstamp, _countof(szstamp));
			wcsncpy(ppd.lptzText, ParseString(szstamp, NULL), MAX_SECONDLINE);

			PUAddPopupT(&ppd);
			SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
		}

		if (HIWORD(wparam) == BN_CLICKED) {
			switch (LOWORD(wparam)) {
			case IDC_POPUPS:
				hasPopups = IsDlgButtonChecked(hdlg, IDC_POPUPS);
				EnableWindow(GetDlgItem(hdlg, IDC_POPUPSTAMP), hasPopups);
				EnableWindow(GetDlgItem(hdlg, IDC_POPUPSTAMPTEXT), hasPopups);
				for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
					EnableWindow(GetDlgItem(hdlg, i), hasPopups);
					EnableWindow(GetDlgItem(hdlg, i + 20), hasPopups);
				}
				break;
			case IDC_DEFAULTCOL:
				for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
					DWORD sett = StatusColors15bits[i - ID_STATUS_OFFLINE];
					COLORREF back, text;
					GetColorsFromDWord(&back, &text, sett);
					SendDlgItemMessage(hdlg, i, CPM_SETCOLOUR, 0, back);
					SendDlgItemMessage(hdlg, i + 20, CPM_SETCOLOUR, 0, text);
				}
				break;
			}
		}
		break; //case WM_COMMAND

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lparam)->code) {
			case PSN_APPLY:
				GetDlgItemText(hdlg, IDC_POPUPSTAMP, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "PopupStamp", szstamp);

				GetDlgItemText(hdlg, IDC_POPUPSTAMPTEXT, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "PopupStampText", szstamp);

				bchecked = (BYTE)IsDlgButtonChecked(hdlg, IDC_POPUPS);
				if (db_get_b(NULL, S_MOD, "UsePopups", 0) != bchecked)
					db_set_b(NULL, S_MOD, "UsePopups", bchecked);

				for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
					COLORREF back = SendDlgItemMessage(hdlg, i, CPM_GETCOLOUR, 0, 0);
					COLORREF text = SendDlgItemMessage(hdlg, i + 20, CPM_GETCOLOUR, 0, 0);
					DWORD sett = GetDWordFromColors(back, text);

					char szSetting[100];
					mir_snprintf(szSetting, "Col_%d", i - ID_STATUS_OFFLINE);
					if (sett != StatusColors15bits[i - ID_STATUS_OFFLINE])
						db_set_dw(NULL, S_MOD, szSetting, sett);
					else
						db_unset(NULL, S_MOD, szSetting);
				}
				break; //case PSN_APPLY
			}
			break; //case 0
		}
		break;//case WM_NOTIFY
	}

	return 0;
}

INT_PTR CALLBACK OptsSettingsDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	DBVARIANT dbv;
	wchar_t szstamp[256];

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		CheckDlgButton(hdlg, IDC_MENUITEM, db_get_b(NULL, S_MOD, "MenuItem", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_USERINFO, db_get_b(NULL, S_MOD, "UserinfoTab", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_FILE, g_bFileActive ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_HISTORY, db_get_b(NULL, S_MOD, "KeepHistory", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_IGNOREOFFLINE, db_get_b(NULL, S_MOD, "IgnoreOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_MISSEDONES, db_get_b(NULL, S_MOD, "MissedOnes", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_SHOWICON, db_get_b(NULL, S_MOD, "ShowIcon", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_COUNT, db_get_b(NULL, S_MOD, "MissedOnes_Count", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_IDLESUPPORT, db_get_b(NULL, S_MOD, "IdleSupport", 1) ? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hdlg, IDC_MENUSTAMP), IsDlgButtonChecked(hdlg, IDC_MENUITEM));
		EnableWindow(GetDlgItem(hdlg, IDC_SHOWICON), IsDlgButtonChecked(hdlg, IDC_MENUITEM));
		EnableWindow(GetDlgItem(hdlg, IDC_USERSTAMP), IsDlgButtonChecked(hdlg, IDC_USERINFO));
		EnableWindow(GetDlgItem(hdlg, IDC_FILESTAMP), IsDlgButtonChecked(hdlg, IDC_FILE));
		EnableWindow(GetDlgItem(hdlg, IDC_FILENAME), IsDlgButtonChecked(hdlg, IDC_FILE));
		EnableWindow(GetDlgItem(hdlg, IDC_HISTORYSIZE), IsDlgButtonChecked(hdlg, IDC_HISTORY));
		EnableWindow(GetDlgItem(hdlg, IDC_HISTORYSTAMP), IsDlgButtonChecked(hdlg, IDC_HISTORY));
		EnableWindow(GetDlgItem(hdlg, IDC_COUNT), IsDlgButtonChecked(hdlg, IDC_MISSEDONES));

		if (!db_get_ws(NULL, S_MOD, "MenuStamp", &dbv)) {
			SetDlgItemText(hdlg, IDC_MENUSTAMP, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_MENUSTAMP, DEFAULT_MENUSTAMP);

		if (!db_get_ws(NULL, S_MOD, "UserStamp", &dbv)) {
			SetDlgItemText(hdlg, IDC_USERSTAMP, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_USERSTAMP, DEFAULT_USERSTAMP);

		if (!db_get_ws(NULL, S_MOD, "FileStamp", &dbv)) {
			SetDlgItemText(hdlg, IDC_FILESTAMP, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_FILESTAMP, DEFAULT_FILESTAMP);

		if (!db_get_ws(NULL, S_MOD, "FileName", &dbv)) {
			SetDlgItemText(hdlg, IDC_FILENAME, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_FILENAME, DEFAULT_FILENAME);

		if (!db_get_ws(NULL, S_MOD, "HistoryStamp", &dbv)) {
			SetDlgItemText(hdlg, IDC_HISTORYSTAMP, dbv.pwszVal);
			db_free(&dbv);
		}
		else SetDlgItemText(hdlg, IDC_HISTORYSTAMP, DEFAULT_HISTORYSTAMP);

		SetDlgItemInt(hdlg, IDC_HISTORYSIZE, db_get_w(NULL, S_MOD, "HistoryMax", 10 - 1) - 1, FALSE);

		// load protocol list
		SetWindowLongPtr(GetDlgItem(hdlg, IDC_PROTOCOLLIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hdlg, IDC_PROTOCOLLIST), GWL_STYLE) | TVS_CHECKBOXES);
		{
			TVINSERTSTRUCT tvis;
			tvis.hParent = nullptr;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;

			for (auto &pa : Accounts()) {
				if (CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
					continue;

				tvis.item.pszText = pa->tszAccountName;
				tvis.item.lParam = (LPARAM)mir_strdup(pa->szModuleName);
				tvis.item.state = INDEXTOSTATEIMAGEMASK(IsWatchedProtocol(pa->szModuleName) + 1);
				TreeView_InsertItem(GetDlgItem(hdlg, IDC_PROTOCOLLIST), &tvis);
			}
		}
		break;

	case WM_COMMAND:
		if ((HIWORD(wparam) == BN_CLICKED || HIWORD(wparam) == EN_CHANGE) && GetFocus() == (HWND)lparam)
			if (LOWORD(wparam) != IDC_VARIABLES)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);

		if (HIWORD(wparam) == BN_CLICKED) {
			switch (LOWORD(wparam)) {
			case IDC_MENUITEM:
				EnableWindow(GetDlgItem(hdlg, IDC_MENUSTAMP), IsDlgButtonChecked(hdlg, IDC_MENUITEM));
				EnableWindow(GetDlgItem(hdlg, IDC_SHOWICON), IsDlgButtonChecked(hdlg, IDC_MENUITEM));
				break;
			case IDC_USERINFO:
				EnableWindow(GetDlgItem(hdlg, IDC_USERSTAMP), IsDlgButtonChecked(hdlg, IDC_USERINFO));
				break;
			case IDC_FILE:
				EnableWindow(GetDlgItem(hdlg, IDC_FILESTAMP), IsDlgButtonChecked(hdlg, IDC_FILE));
				EnableWindow(GetDlgItem(hdlg, IDC_FILENAME), IsDlgButtonChecked(hdlg, IDC_FILE));
				break;
			case IDC_HISTORY:
				EnableWindow(GetDlgItem(hdlg, IDC_HISTORYSTAMP), IsDlgButtonChecked(hdlg, IDC_HISTORY));
				EnableWindow(GetDlgItem(hdlg, IDC_HISTORYSIZE), IsDlgButtonChecked(hdlg, IDC_HISTORY));
				break;
			case IDC_MISSEDONES:
				EnableWindow(GetDlgItem(hdlg, IDC_COUNT), IsDlgButtonChecked(hdlg, IDC_MISSEDONES));
				break;
			}
		}

		if (LOWORD(wparam) == IDC_VARIABLES) {
			CMStringW str;
			addSection(str, LPGENW("-- Date --"), section1, _countof(section1));
			addSection(str, LPGENW("-- Time --"), section2, _countof(section2));
			addSection(str, LPGENW("-- User --"), section3, _countof(section3));
			addSection(str, LPGENW("-- Format --"), section4, _countof(section4));
			str.AppendFormat(L"%s\t%s\n\t%s", TranslateT("Note:"), TranslateT("Use # for empty string"), TranslateT("instead of <unknown>"));
			MessageBoxW(hdlg, str, TranslateT("Last Seen variables"), MB_OK | MB_TOPMOST);
		}
		break; //case WM_COMMAND

	case WM_NOTIFY:
		switch (((LPNMHDR)lparam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lparam)->code) {
			case PSN_APPLY:
				GetDlgItemText(hdlg, IDC_MENUSTAMP, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "MenuStamp", szstamp);

				GetDlgItemText(hdlg, IDC_USERSTAMP, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "UserStamp", szstamp);

				GetDlgItemText(hdlg, IDC_FILESTAMP, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "FileStamp", szstamp);

				GetDlgItemText(hdlg, IDC_FILENAME, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "FileName", szstamp);

				GetDlgItemText(hdlg, IDC_HISTORYSTAMP, szstamp, _countof(szstamp));
				db_set_ws(NULL, S_MOD, "HistoryStamp", szstamp);

				db_set_w(NULL, S_MOD, "HistoryMax", (WORD)(GetDlgItemInt(hdlg, IDC_HISTORYSIZE, nullptr, FALSE) + 1));

				BOOL bchecked = IsDlgButtonChecked(hdlg, IDC_MENUITEM);
				if (db_get_b(NULL, S_MOD, "MenuItem", 1) != bchecked) {
					db_set_b(NULL, S_MOD, "MenuItem", bchecked);
					if (hmenuitem == nullptr && bchecked)
						InitMenuitem();
				}

				bchecked = IsDlgButtonChecked(hdlg, IDC_USERINFO);
				if (db_get_b(NULL, S_MOD, "UserinfoTab", 1) != bchecked) {
					db_set_b(NULL, S_MOD, "UserinfoTab", bchecked);
					if (bchecked)
						ehuserinfo = HookEvent(ME_USERINFO_INITIALISE, UserinfoInit);
					else
						UnhookEvent(ehuserinfo);
				}

				bchecked = IsDlgButtonChecked(hdlg, IDC_FILE);
				if (g_bFileActive != bchecked) {
					g_bFileActive = bchecked;
					db_set_b(NULL, S_MOD, "FileOutput", bchecked);
					if (bchecked)
						InitFileOutput();
					else
						UninitFileOutput();
				}

				bchecked = IsDlgButtonChecked(hdlg, IDC_HISTORY);
				if (db_get_b(NULL, S_MOD, "KeepHistory", 0) != bchecked)
					db_set_b(NULL, S_MOD, "KeepHistory", bchecked);

				bchecked = IsDlgButtonChecked(hdlg, IDC_IGNOREOFFLINE);
				if (db_get_b(NULL, S_MOD, "IgnoreOffline", 1) != bchecked)
					db_set_b(NULL, S_MOD, "IgnoreOffline", bchecked);

				bchecked = IsDlgButtonChecked(hdlg, IDC_MISSEDONES);
				if (db_get_b(NULL, S_MOD, "MissedOnes", 0) != bchecked) {
					db_set_b(NULL, S_MOD, "MissedOnes", bchecked);
					if (bchecked)
						ehmissed_proto = HookEvent(ME_PROTO_ACK, ModeChange_mo);
					else
						UnhookEvent(ehmissed_proto);
				}

				bchecked = IsDlgButtonChecked(hdlg, IDC_SHOWICON);
				if (db_get_b(NULL, S_MOD, "ShowIcon", 1) != bchecked)
					db_set_b(NULL, S_MOD, "ShowIcon", bchecked);

				bchecked = IsDlgButtonChecked(hdlg, IDC_COUNT);
				if (db_get_b(NULL, S_MOD, "MissedOnes_Count", 0) != bchecked)
					db_set_b(NULL, S_MOD, "MissedOnes_Count", bchecked);

				includeIdle = IsDlgButtonChecked(hdlg, IDC_IDLESUPPORT);
				if (db_get_b(NULL, S_MOD, "IdleSupport", 1) != includeIdle)
					db_set_b(NULL, S_MOD, "IdleSupport", (BYTE)includeIdle);

				// save protocol list
				HWND hwndTreeView = GetDlgItem(hdlg, IDC_PROTOCOLLIST);
				char *protocol;
				int size = 1;

				CMStringA watchedProtocols;
				HTREEITEM hItem = TreeView_GetRoot(hwndTreeView);

				TVITEM tvItem;
				tvItem.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
				tvItem.stateMask = TVIS_STATEIMAGEMASK;

				while (hItem != nullptr) {
					tvItem.hItem = hItem;
					TreeView_GetItem(hwndTreeView, &tvItem);
					protocol = (char*)tvItem.lParam;
					if ((BOOL)(tvItem.state >> 12) - 1) {
						size += (int)mir_strlen(protocol) + 2;
						if (!watchedProtocols.IsEmpty())
							watchedProtocols.AppendChar('\n');
						watchedProtocols.Append(protocol);
					}
					hItem = TreeView_GetNextSibling(hwndTreeView, hItem);
				}
				db_set_s(NULL, S_MOD, "WatchedAccounts", watchedProtocols);

				UnloadWatchedProtos();
				LoadWatchedProtos();
			}
			break; //case 0

		case IDC_PROTOCOLLIST:
			if (((LPNMHDR)lparam)->code == NM_CLICK) {
				HWND hTree = ((LPNMHDR)lparam)->hwndFrom;
				HTREEITEM hItem;

				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(hTree, &hti.pt);
				if (hItem = TreeView_HitTest(hTree, &hti)) {
					if (hti.flags & TVHT_ONITEM)
						TreeView_SelectItem(hTree, hItem);
					if (hti.flags & TVHT_ONITEMSTATEICON)
						SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		break;//case WM_NOTIFY

	case WM_DESTROY:
		// free protocol list 
		HWND hwndTreeView = GetDlgItem(hdlg, IDC_PROTOCOLLIST);
		HTREEITEM hItem = TreeView_GetRoot(hwndTreeView);
		TVITEM tvItem;
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM;

		while (hItem != nullptr) {
			tvItem.hItem = hItem;
			TreeView_GetItem(hwndTreeView, &tvItem);
			mir_free((void *)tvItem.lParam);
			hItem = TreeView_GetNextSibling(hwndTreeView, hItem);
		}
		break;
	}

	return 0;
}

int OptionsInit(WPARAM wparam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETTINGS);
	odp.szGroup.w = LPGENW("Contacts");
	odp.szTitle.w = LPGENW("Last seen");
	odp.pfnDlgProc = OptsSettingsDlgProc;
	g_plugin.addOptions(wparam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS);
		odp.szGroup.w = LPGENW("Popups");
		odp.szTitle.w = LPGENW("Last seen");
		odp.pfnDlgProc = OptsPopupsDlgProc;
		g_plugin.addOptions(wparam, &odp);
	}
	return 0;
}
