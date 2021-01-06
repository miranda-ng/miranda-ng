#include "stdafx.h"

void populateSettingsList(HWND hwnd2List)
{
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Send If My Status Is..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Send If They Change Status to..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)L"----------------------------");
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Reuse Pounce"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Give Up delay"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Confirmation Window"));
}

void populateContacts(MCONTACT BPhContact, HWND hwnd2CB)
{
	for (auto &hContact : Contacts()) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
			wchar_t name[300];
			mir_snwprintf(name, L"%s (%S)", Clist_GetContactDisplayName(hContact), szProto);
			int index = SendMessage(hwnd2CB, CB_ADDSTRING, 0, (LPARAM)name);
			SendMessage(hwnd2CB, CB_SETITEMDATA, index, hContact);
			if (BPhContact == hContact)
				SendMessage(hwnd2CB, CB_SETCURSEL, index, 0);
		}
	}
}

void saveLastSetting(MCONTACT hContact, HWND hwnd)
{
	wchar_t number[8];//, string[1024];//for sending file name
	switch (g_plugin.getByte(hContact, "LastSetting", 2)) { // nothing to do
	case 0: // Send If My Status Is...
		break;
	case 1: // Send If They Change status to
		break;
	case 2: // ------, fall through
	case 6: // nothing to do...
		break;
	case 3: // Reuse Pounce
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, _countof(number));
		g_plugin.setByte(hContact, "Reuse", (BYTE)_wtoi(number));
		break;
	case 4: // Give Up delay
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, _countof(number));
		g_plugin.setByte(hContact, "GiveUpDays", (BYTE)_wtoi(number));
		{
			g_plugin.setDword(hContact, "GiveUpDate", (DWORD)(_wtoi(number)*SECONDSINADAY));
		}
		break;
	case 5:	// confirm window
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, _countof(number));
		g_plugin.setWord(hContact, "ConfirmTimeout", (WORD)_wtoi(number));
		break;
	}
}

void hideAll(HWND hwnd)
{
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_HIDE);
}

void getDefaultMessage(HWND hwnd, UINT control, MCONTACT hContact)
{
	DBVARIANT dbv;
	if (!g_plugin.getWString(hContact, "PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.pwszVal);
		db_free(&dbv);
	}
	else if (!g_plugin.getWString("PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.pwszVal);
		db_free(&dbv);
	}
}

INT_PTR CALLBACK StatusModesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam); // windowInfo*
		TranslateDialogDefault(hwnd);

		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			windowInfo * wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			wchar_t type[32];
			GetDlgItemText(hwnd, IDC_CHECK1, type, _countof(type));

			WORD flag = (IsDlgButtonChecked(hwnd, IDC_CHECK1))
				| (IsDlgButtonChecked(hwnd, IDC_CHECK2) << 1)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK3) << 2)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK4) << 3)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK5) << 4)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK6) << 5)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK7) << 6)
				| (IsDlgButtonChecked(hwnd, IDC_CHECK8) << 7);

			if (!mir_wstrcmp(type, L"Any")) {
				if (LOWORD(wParam) == IDOK)
					g_plugin.setWord(wi->hContact, "SendIfMyStatusIsFLAG", flag);
				wi->SendIfMy = nullptr;
			}
			else {
				if (LOWORD(wParam) == IDOK)
					g_plugin.setWord(wi->hContact, "SendIfTheirStatusIsFLAG", flag);
				wi->SendWhenThey = nullptr;
			}
			DestroyWindow(hwnd);
		}
		break;
	}
	return FALSE;
}
void statusModes(windowInfo *wi, int myStatusMode) // myStatusMode=1 sendIfMyStatusFlag
{
	int statusFlag;
	HWND hwnd;

	if (myStatusMode) {
		if (wi->SendIfMy) {
			SetForegroundWindow(wi->SendIfMy);
			return;
		}
		else {
			hwnd = wi->SendIfMy = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_STATUSMODES), nullptr, StatusModesDlgProc, (LPARAM)wi);
			statusFlag = g_plugin.getWord(wi->hContact, "SendIfMyStatusIsFLAG", 0);
			SetWindowText(hwnd, TranslateT("Send If My Status Is"));
			SetDlgItemText(hwnd, IDC_CHECK1, TranslateT("Any"));
			SetDlgItemText(hwnd, IDC_CHECK2, TranslateT("Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, TranslateT("Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, TranslateT("Not available"));
			SetDlgItemText(hwnd, IDC_CHECK5, TranslateT("Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, TranslateT("Do not disturb"));
			SetDlgItemText(hwnd, IDC_CHECK7, TranslateT("Free for chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, TranslateT("Invisible"));
		}
	}
	else {
		if (wi->SendWhenThey) {
			SetForegroundWindow(wi->SendWhenThey);
			return;
		}
		else {
			hwnd = wi->SendWhenThey = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_STATUSMODES), nullptr, StatusModesDlgProc, (LPARAM)wi);
			statusFlag = g_plugin.getWord(wi->hContact, "SendIfTheirStatusIsFLAG", 0);
			SetWindowText(hwnd, TranslateT("Send If Their Status changes"));
			SetDlgItemText(hwnd, IDC_CHECK1, TranslateT("From Offline"));
			SetDlgItemText(hwnd, IDC_CHECK2, TranslateT("To Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, TranslateT("To Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, TranslateT("To Not available"));
			SetDlgItemText(hwnd, IDC_CHECK5, TranslateT("To Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, TranslateT("To Do not disturb"));
			SetDlgItemText(hwnd, IDC_CHECK7, TranslateT("To Free for chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, TranslateT("To Invisible"));
		}
	}
	CheckDlgButton(hwnd, IDC_CHECK1, (statusFlag & ANY) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK2, (statusFlag & ONLINE) >> 1 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK3, (statusFlag & AWAY) >> 2 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK4, (statusFlag & NA) >> 3 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK5, (statusFlag & OCCUPIED) >> 4 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK6, (statusFlag & DND) >> 5 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK7, (statusFlag & FFC) >> 6 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK8, (statusFlag & INVISIBLE) >> 7 ? BST_CHECKED : BST_UNCHECKED);
}

void deletePounce(MCONTACT hContact)
{
	g_plugin.delSetting(hContact, "PounceMsg");
	g_plugin.delSetting(hContact, "SendIfMyStatusIsFLAG");
	g_plugin.delSetting(hContact, "SendIfTheirStatusIsFLAG");
	g_plugin.delSetting(hContact, "Reuse");
	g_plugin.delSetting(hContact, "GiveUpDays");
	g_plugin.delSetting(hContact, "GiveUpDate");
	g_plugin.delSetting(hContact, "ConfirmTimeout");
	g_plugin.delSetting(hContact, "FileToSend");
}

INT_PTR CALLBACK BuddyPounceSimpleDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	wchar_t msg[1024];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		hContact = lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

		getDefaultMessage(hwnd, IDC_MESSAGE, hContact);
		mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
		SetDlgItemText(hwnd, GRP_MSG, msg);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), length);
				SetDlgItemText(hwnd, GRP_MSG, msg);
			}
			break;

		case IDC_ADVANCED:
			// fall through

		case IDOK:
		{
			int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)) + 1;
			if (length > 1) {
				wchar_t *text = (wchar_t*)_alloca(length * sizeof(wchar_t));
				GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
				g_plugin.setWString(hContact, "PounceMsg", text);
			}
			else g_plugin.delSetting(hContact, "PounceMsg");
		}
		// fall through
		case IDCANCEL:
			if (LOWORD(wParam) == IDC_ADVANCED)
				CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POUNCE), nullptr, BuddyPounceDlgProc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
			DestroyWindow(hwnd);
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	windowInfo *wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	wchar_t msg[1024];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		wi = (windowInfo *)mir_alloc(sizeof(windowInfo));
		wi->hContact = lParam;
		wi->SendIfMy = nullptr;
		wi->SendWhenThey = nullptr;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wi);
		getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);

		mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
		SetDlgItemText(hwnd, GRP_MSG, msg);

		populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
		populateContacts(wi->hContact, GetDlgItem(hwnd, IDC_CONTACTS));
		SendDlgItemMessage(hwnd, IDC_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)1024, (short)0));
		g_plugin.setByte(wi->hContact, "LastSetting", 0);
		return FALSE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), length);
				SetDlgItemText(hwnd, GRP_MSG, msg);
			}
			break;

		case IDC_SIMPLE:
		case IDOK:
			{
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_GETITEMDATA, SendDlgItemMessage(hwnd, IDC_CONTACTS, CB_GETCURSEL, 0, 0), 0);
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)) + 1;
				if (length > 1) {
					wchar_t *text = (wchar_t *)mir_alloc(length * sizeof(wchar_t));
					if (!text) {
						msg(TranslateT("Couldn't allocate enough memory"), L"");
						break;
					}
					GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
					g_plugin.setWString(hContact, "PounceMsg", text);
					mir_free(text);
				}
				else g_plugin.delSetting(hContact, "PounceMsg");
				saveLastSetting(hContact, hwnd);
			}
			if (LOWORD(wParam) == IDC_SIMPLE)
				CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POUNCE_SIMPLE), nullptr, BuddyPounceSimpleDlgProc, (LPARAM)((windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact);
			__fallthrough;

		case IDCANCEL:
			if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
			if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
			mir_free(wi);
			DestroyWindow(hwnd);
			break;

		case IDC_DELETE:
			deletePounce(wi->hContact);
			SetDlgItemText(hwnd, IDC_MESSAGE, L"");
			SetDlgItemText(hwnd, GRP_MSG, TranslateT("The Message    (0 Characters)"));
			break;

		case IDC_DEFAULT:
			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);

			mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);

			g_plugin.setWord(wi->hContact, "SendIfMyStatusIsFLAG", g_plugin.getWord("SendIfMyStatusIsFLAG", 0));
			g_plugin.setWord(wi->hContact, "SendIfTheirStatusIsFLAG", g_plugin.getWord("SendIfTheirStatusIsFLAG", 0));
			g_plugin.setByte(wi->hContact, "Reuse", g_plugin.getByte("Reuse", 0));
			g_plugin.setByte(wi->hContact, "GiveUpDays", g_plugin.getByte("GiveUpDays", 0));
			g_plugin.setDword(wi->hContact, "GiveUpDate", g_plugin.getDword("GiveUpDate", 0));
			g_plugin.setWord(wi->hContact, "ConfirmTimeout", g_plugin.getWord("ConfirmTimeout", 0));
			break;

		case IDC_SETTINGS:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int item = SendDlgItemMessage(hwnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
				wchar_t temp[5];
				saveLastSetting(wi->hContact, hwnd);
				hideAll(hwnd);
				switch (item) {
				case 0: // Send If My Status Is...
					statusModes(wi, 1);
					break;
				case 1: // Send If They Change status to
					statusModes(wi, 0);
					break;
				case 3: // Reuse Pounce
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Reuse this message? (0 to use it once)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Times"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getByte(wi->hContact, "Reuse", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 4: // Give Up delay
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Give up after... (0 to not give up)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Days"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getByte(wi->hContact, "GiveUpDays", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 5:	// confirm window
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Show confirmation window? (0 to not Show)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Seconds to wait before sending"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getWord(wi->hContact, "ConfirmTimeout", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				}
				g_plugin.setByte(wi->hContact, "LastSetting", (BYTE)item);
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceOptionsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wchar_t msg[1024];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			windowInfo *wi = (windowInfo *)mir_alloc(sizeof(windowInfo));
			wi->hContact = 0;
			wi->SendIfMy = nullptr;
			wi->SendWhenThey = nullptr;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wi);

			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
			mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);
			populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
			SendDlgItemMessage(hwnd, IDC_SPIN, UDM_SETRANGE, 0, (LPARAM)MAKELONG((short)1024, (short)0));
			CheckDlgButton(hwnd, IDC_USEADVANCED, g_plugin.getByte("UseAdvanced", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, IDC_SHOWDELIVERYMSGS, g_plugin.getByte("ShowDeliveryMessages", 1) ? BST_CHECKED : BST_UNCHECKED);
			g_plugin.setByte(wi->hContact, "LastSetting", 0);
		}
		return FALSE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				windowInfo * wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				MCONTACT hContact = ((windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact;
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)) + 1;
				if (length > 1) {
					wchar_t *text = (wchar_t*)mir_alloc(length * sizeof(wchar_t));
					if (!text) {
						msg(TranslateT("Couldn't allocate enough memory"), L"");
						break;
					}
					GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
					g_plugin.setWString(hContact, "PounceMsg", text);
					mir_free(text);
				}
				else g_plugin.delSetting(hContact, "PounceMsg");
				g_plugin.setByte("UseAdvanced", (BYTE)IsDlgButtonChecked(hwnd, IDC_USEADVANCED));
				g_plugin.setByte("ShowDeliveryMessages", (BYTE)IsDlgButtonChecked(hwnd, IDC_SHOWDELIVERYMSGS));

				if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
				if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_snwprintf(msg, TranslateT("The Message    (%d Characters)"), length);
				SetDlgItemText(hwnd, GRP_MSG, msg);
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_SHOWDELIVERYMSGS:
		case IDC_USEADVANCED:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;

		case IDC_SETTINGS:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				windowInfo *wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				int item = SendDlgItemMessage(hwnd, IDC_SETTINGS, LB_GETCURSEL, 0, 0);
				wchar_t temp[5];
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				saveLastSetting(wi->hContact, hwnd);
				hideAll(hwnd);
				switch (item) {
				case 0: // Send If My Status Is...
					statusModes(wi, 1);
					break;
				case 1: // Send If They Change status to
					statusModes(wi, 0);
					break;
				case 3: // Reuse Pounce
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Reuse this message? (0 to use it once)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Times"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getByte(wi->hContact, "Reuse", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 4: // Give Up delay
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Give up after... (0 to not give up)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Days"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getByte(wi->hContact, "GiveUpDays", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 5:	// confirm window
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Show confirmation window? (0 to not Show)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Seconds to wait before sending"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itow(g_plugin.getWord(wi->hContact, "ConfirmTimeout", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				}
				g_plugin.setByte(wi->hContact, "LastSetting", (BYTE)item);
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK SendPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SendPounceDlgProcStruct *spdps = (SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		spdps = (SendPounceDlgProcStruct*)lParam;
		spdps->timer = g_plugin.getWord(spdps->hContact, "ConfirmTimeout");
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)spdps);
		{
			DBVARIANT dbv;
			if (g_plugin.getWString(spdps->hContact, "PounceMsg", &dbv))
				DestroyWindow(hwnd);
			else {
				SetDlgItemText(hwnd, IDC_MESSAGE, dbv.pwszVal);
				db_free(&dbv);
			}
		}
		SetTimer(hwnd, 1, 1000, nullptr);
		SendMessage(hwnd, WM_TIMER, 0, 0);
		break;

	case WM_TIMER:
	{
		wchar_t message[1024];
		mir_snwprintf(message, TranslateT("Pounce being sent to %s in %d seconds"), Clist_GetContactDisplayName(spdps->hContact), spdps->timer);
		SetDlgItemText(hwnd, LBL_CONTACT, message);
	}
	spdps->timer--;
	if (spdps->timer < 0) {
		KillTimer(hwnd, 1);
		SendPounce(spdps->message, spdps->hContact);
		DestroyWindow(hwnd);
	}
	break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			KillTimer(hwnd, 1);
			SendPounce(spdps->message, spdps->hContact);
			// fall through
		case IDCANCEL:
			KillTimer(hwnd, 1);
			DestroyWindow(hwnd);
			break;
		}
		break;

	case WM_DESTROY:
		mir_free(spdps->message);
		mir_free(spdps);
		break;
	}
	return 0;
}

INT_PTR CALLBACK PounceSentDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		TranslateDialogDefault(hwnd);
		hContact = lParam;
		{
			DBVARIANT dbv;
			if (g_plugin.getWString(hContact, "PounceMsg", &dbv))
				DestroyWindow(hwnd);
			else {
				SetDlgItemText(hwnd, IDC_MESSAGE, dbv.pwszVal);
				db_free(&dbv);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			wchar_t text[2048];
			GetDlgItemText(hwnd, IDOK, text, _countof(text));
			if (!mir_wstrcmp(text, TranslateT("Retry"))) {
				GetDlgItemText(hwnd, IDC_MESSAGE, text, _countof(text));
				SendPounce(text, hContact);
			}
		}
		// fall through
		case IDCANCEL:
			DestroyWindow(hwnd);
		}
		break;
	}
	return 0;
}

void CreateMessageAcknowlegedWindow(MCONTACT hContact, int SentSuccess)
{
	HWND hwnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONFIRMSEND), nullptr, PounceSentDlgProc, hContact);
	wchar_t msg[256];
	if (SentSuccess) {
		mir_snwprintf(msg, TranslateT("Message successfully sent to %s"), Clist_GetContactDisplayName(hContact));
		SetDlgItemText(hwnd, IDOK, TranslateT("OK"));
		ShowWindow(GetDlgItem(hwnd, IDCANCEL), 0);
	}
	else {
		mir_snwprintf(msg, TranslateT("Message failed to send to %s"), Clist_GetContactDisplayName(hContact));
		SetDlgItemText(hwnd, IDOK, TranslateT("Retry"));
	}
	SetDlgItemText(hwnd, LBL_CONTACT, msg);
	SetWindowText(hwnd, TranslateT(modFullname));
}
