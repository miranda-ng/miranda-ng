#include "headers.h"

void populateSettingsList(HWND hwnd2List)
{
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Send If My Status Is..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Send If They Change Status to..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)_T("----------------------------"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Reuse Pounce"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Give Up delay"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)TranslateT("Confirmation Window"));
}

void populateContacts(MCONTACT BPhContact, HWND hwnd2CB)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
			TCHAR name[300];
			mir_sntprintf(name, SIZEOF(name), _T("%s (%s)"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR), _A2T(szProto));
			int index = SendMessage(hwnd2CB, CB_ADDSTRING, 0, (LPARAM)name);
			SendMessage(hwnd2CB, CB_SETITEMDATA, index, hContact);
			if (BPhContact == hContact)
				SendMessage(hwnd2CB, CB_SETCURSEL, index, 0);
		}
	}
}

void saveLastSetting(MCONTACT hContact, HWND hwnd)
{
	TCHAR number[8];//, string[1024];//for sending file name
	switch (db_get_b(hContact, modname, "LastSetting", 2)) { // nothing to do
	case 0: // Send If My Status Is...
		break;
	case 1: // Send If They Change status to
		break;
	case 2: // ------, fall through
	case 6: // nothing to do...
		break;
	case 3: // Reuse Pounce
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
		db_set_b(hContact, modname, "Reuse", (BYTE)_ttoi(number));
		break;
	case 4: // Give Up delay
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
		db_set_b(hContact, modname, "GiveUpDays", (BYTE)_ttoi(number));
		{
			time_t today = time(NULL);
			db_set_dw(hContact, modname, "GiveUpDate", (DWORD)(_ttoi(number)*SECONDSINADAY));
		}
		break;
	case 5:	// confirm window
		GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
		db_set_w(hContact, modname, "ConfirmTimeout", (WORD)_ttoi(number));
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
	if (!db_get_ts(hContact, modname, "PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.ptszVal);
		db_free(&dbv);
	}
	else if (!db_get_ts(NULL, modname, "PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.ptszVal);
		db_free(&dbv);
	}
}

INT_PTR CALLBACK StatusModesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam); // windowInfo*
		TranslateDialogDefault(hwnd);

		return FALSE;
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			windowInfo *wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			TCHAR type[32];
			GetDlgItemText(hwnd, IDC_CHECK1, type, 32);

			WORD flag = (IsDlgButtonChecked(hwnd, IDC_CHECK1))
				|(IsDlgButtonChecked(hwnd, IDC_CHECK2)<<1)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK3)<<2)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK4)<<3)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK5)<<4)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK6)<<5)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK7)<<6)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK8)<<7)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK9)<<8)
				|(IsDlgButtonChecked(hwnd, IDC_CHECK10)<<9);

			if (!_tcscmp(type, _T("Any"))) {
				if (LOWORD(wParam) == IDOK)
					db_set_w(wi->hContact, modname, "SendIfMyStatusIsFLAG", flag);
				wi->SendIfMy = 0;
			}
			else {
				if (LOWORD(wParam) == IDOK)
					db_set_w(wi->hContact, modname, "SendIfTheirStatusIsFLAG", flag);
				wi->SendWhenThey = 0;
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
			hwnd = wi->SendIfMy = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_STATUSMODES), 0, StatusModesDlgProc, (LPARAM)wi);
			statusFlag = db_get_w(wi->hContact, modname, "SendIfMyStatusIsFLAG", 0);
			SetWindowText(hwnd, TranslateT("Send If My Status Is"));
			SetDlgItemText(hwnd, IDC_CHECK1, TranslateT("Any"));
			SetDlgItemText(hwnd, IDC_CHECK2, TranslateT("Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, TranslateT("Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, TranslateT("NA"));
			SetDlgItemText(hwnd, IDC_CHECK5, TranslateT("Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, TranslateT("DND"));
			SetDlgItemText(hwnd, IDC_CHECK7, TranslateT("Free For Chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, TranslateT("Invisible"));
			SetDlgItemText(hwnd, IDC_CHECK9, TranslateT("On The Phone"));
			SetDlgItemText(hwnd, IDC_CHECK10, TranslateT("Out To Lunch"));
		}
	}
	else {
		if (wi->SendWhenThey) {
			SetForegroundWindow(wi->SendWhenThey);
			return;
		}
		else {
			hwnd = wi->SendWhenThey = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_STATUSMODES),0,StatusModesDlgProc, (LPARAM)wi);
			statusFlag = db_get_w(wi->hContact, modname, "SendIfTheirStatusIsFLAG", 0);
			SetWindowText(hwnd, TranslateT("Send If Their Status changes"));
			SetDlgItemText(hwnd, IDC_CHECK1, TranslateT("From Offline"));
			SetDlgItemText(hwnd, IDC_CHECK2, TranslateT("To Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, TranslateT("To Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, TranslateT("To NA"));
			SetDlgItemText(hwnd, IDC_CHECK5, TranslateT("To Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, TranslateT("To DND"));
			SetDlgItemText(hwnd, IDC_CHECK7, TranslateT("To Free For Chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, TranslateT("To Invisible"));
			SetDlgItemText(hwnd, IDC_CHECK9, TranslateT("To On The Phone"));
			SetDlgItemText(hwnd, IDC_CHECK10, TranslateT("To Out To Lunch"));
		}
	}
	CheckDlgButton(hwnd, IDC_CHECK1, statusFlag&ANY);
	CheckDlgButton(hwnd, IDC_CHECK2, (statusFlag&ONLINE)>>1);
	CheckDlgButton(hwnd, IDC_CHECK3, (statusFlag&AWAY)>>2);
	CheckDlgButton(hwnd, IDC_CHECK4, (statusFlag&NA)>>3);
	CheckDlgButton(hwnd, IDC_CHECK5, (statusFlag&OCCUPIED)>>4);
	CheckDlgButton(hwnd, IDC_CHECK6, (statusFlag&DND)>>5);
	CheckDlgButton(hwnd, IDC_CHECK7, (statusFlag&FFC)>>6);
	CheckDlgButton(hwnd, IDC_CHECK8, (statusFlag&INVISIBLE)>>7);
	CheckDlgButton(hwnd, IDC_CHECK9, (statusFlag&PHONE)>>8);
	CheckDlgButton(hwnd, IDC_CHECK10, (statusFlag&LUNCH)>>9);
}

void deletePounce(MCONTACT hContact)
{
	db_unset(hContact,modname, "PounceMsg");
	db_unset(hContact,modname, "SendIfMyStatusIsFLAG");
	db_unset(hContact,modname, "SendIfTheirStatusIsFLAG");
	db_unset(hContact,modname, "Reuse");
	db_unset(hContact, modname, "GiveUpDays");
	db_unset(hContact, modname, "GiveUpDate");
	db_unset(hContact, modname, "ConfirmTimeout");
	db_unset(hContact, modname, "FileToSend");
}

INT_PTR CALLBACK BuddyPounceSimpleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		hContact = lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);

		getDefaultMessage(hwnd, IDC_MESSAGE, hContact);
		{
			TCHAR msg[1024];
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	
		}
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length;
				TCHAR msg[1024];
				length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), length);
				SetDlgItemText(hwnd, GRP_MSG, msg);
			}
			break;

		case IDC_ADVANCED:
			// fall through

		case IDOK:
			{
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)) + 1;
				if (length>1) {
					TCHAR *text = (TCHAR*)mir_alloc(length*sizeof(TCHAR));
					if (!text) {
						msg(TranslateT("Couldn't allocate enough memory"), _T(""));
						break;
					}
					GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
				}
				else db_unset(hContact, modname, "PounceMsg");
			}
			// fall through
		case IDCANCEL:
			if (LOWORD(wParam) == IDC_ADVANCED)
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_POUNCE), 0, BuddyPounceDlgProc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
			DestroyWindow(hwnd);
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	windowInfo *wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		wi = (windowInfo *)mir_alloc(sizeof(windowInfo));
		if (!wi) {
			msg(TranslateT("error......"), TranslateT("Buddy Pounce"));
			DestroyWindow(hwnd);
		}
		wi->hContact = lParam;
		wi->SendIfMy = 0;
		wi->SendWhenThey = 0;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wi);
		getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
		{
			TCHAR msg[1024];
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	
		}
		populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
		populateContacts(wi->hContact, GetDlgItem(hwnd, IDC_CONTACTS));
		SendMessage(GetDlgItem(hwnd,IDC_SPIN), UDM_SETRANGE, 0, (LPARAM) MAKELONG ((short) 1024, (short) 0));
		db_set_b(wi->hContact, modname, "LastSetting", 0);
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length;
				TCHAR msg[1024];
				length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), length);
				SetDlgItemText(hwnd, GRP_MSG, msg);
			}
			break;

		case IDC_SIMPLE:
		case IDOK:
			{
				MCONTACT hContact = (MCONTACT)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETITEMDATA, SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETCURSEL,0,0),0);
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE))+1;
				if (length>1) {
					TCHAR *text = (TCHAR*)mir_alloc(length*sizeof(TCHAR));
					if (!text) {
						msg(TranslateT("Couldn't allocate enough memory"), _T(""));
						break;
					}
					GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
					db_set_ws(hContact, modname, "PounceMsg", text);
					mir_free(text);
				}
				else db_unset(hContact, modname, "PounceMsg");
				saveLastSetting(hContact, hwnd);
			} // fall through
			if (LOWORD(wParam) == IDC_SIMPLE)
				CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_POUNCE_SIMPLE), 0, BuddyPounceSimpleDlgProc, (LPARAM)((windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact);
			// fall through

		case IDCANCEL:
			if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
			if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
			mir_free(wi);
			DestroyWindow(hwnd);
			break;

		case IDC_DELETE:
			deletePounce(wi->hContact);
			SetDlgItemText(hwnd, IDC_MESSAGE, _T(""));
			SetDlgItemText(hwnd, GRP_MSG, TranslateT("The Message    (0 Characters)"));
			break;

		case IDC_DEFAULT:
			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
			{
				TCHAR msg[1024];
				mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
				SetDlgItemText(hwnd, GRP_MSG, msg);
			}
			db_set_w(wi->hContact, modname, "SendIfMyStatusIsFLAG", (WORD)db_get_w(NULL, modname, "SendIfMyStatusIsFLAG",0));
			db_set_w(wi->hContact, modname, "SendIfTheirStatusIsFLAG", (WORD)db_get_w(NULL, modname, "SendIfTheirStatusIsFLAG",0));
			db_set_b(wi->hContact, modname, "Reuse",(BYTE)db_get_b(NULL, modname, "Reuse",0));
			db_set_b(wi->hContact, modname, "GiveUpDays", (BYTE)db_get_b(NULL, modname, "GiveUpDays",0));
			db_set_dw(wi->hContact, modname, "GiveUpDate", (DWORD)db_get_dw(NULL, modname, "GiveUpDate",0));
			db_set_w(wi->hContact, modname, "ConfirmTimeout", (WORD)db_get_w(NULL, modname, "ConfirmTimeout",0));
			break;

		case IDC_SETTINGS:
			if (HIWORD(wParam) == LBN_SELCHANGE) {	
				int item = SendMessage(GetDlgItem(hwnd, IDC_SETTINGS), LB_GETCURSEL, 0, 0);
				TCHAR temp[5];
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
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_b(wi->hContact, modname, "Reuse", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 4: // Give Up delay
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Give up after... (0 to not give up)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Days"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_b(wi->hContact, modname, "GiveUpDays", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 5:	// confirm window
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Show confirmation window? (0 to not Show)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Seconds to wait before sending"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_w(wi->hContact, modname, "ConfirmTimeout", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				}
				db_set_b(wi->hContact, modname, "LastSetting", (BYTE)item);
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			windowInfo *wi = (windowInfo *)mir_alloc(sizeof(windowInfo));
			TCHAR msg[1024];
			wi->hContact = 0;
			wi->SendIfMy = 0;
			wi->SendWhenThey = 0;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wi);

			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	
			populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
			SendMessage(GetDlgItem(hwnd,IDC_SPIN), UDM_SETRANGE, 0, (LPARAM) MAKELONG ((short) 1024, (short) 0));
			CheckDlgButton(hwnd, IDC_USEADVANCED, db_get_b(NULL, modname, "UseAdvanced", 0));
			CheckDlgButton(hwnd, IDC_SHOWDELIVERYMSGS, db_get_b(NULL, modname, "ShowDeliveryMessages", 1));
			db_set_b(wi->hContact, modname, "LastSetting", 0);
		}
		return FALSE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				windowInfo *wi = (windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				MCONTACT hContact = ((windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact;
				int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE))+1;
				if (length > 1) {
					TCHAR *text = (TCHAR*)mir_alloc(length*sizeof(TCHAR));
					if (!text) {
						msg(TranslateT("Couldn't allocate enough memory"), _T(""));
						break;
					}
					GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
					db_set_ws(hContact, modname, "PounceMsg", text);
					mir_free(text);
				}
				else db_unset(hContact, modname, "PounceMsg");
				db_set_b(NULL, modname, "UseAdvanced", (BYTE)IsDlgButtonChecked(hwnd, IDC_USEADVANCED));
				db_set_b(NULL, modname, "ShowDeliveryMessages", (BYTE)IsDlgButtonChecked(hwnd, IDC_SHOWDELIVERYMSGS));

				if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
				if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int length;
				TCHAR msg[1024];
				length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
				mir_sntprintf(msg, SIZEOF(msg), TranslateT("The Message    (%d Characters)"), length);
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
				int item = SendMessage(GetDlgItem(hwnd, IDC_SETTINGS), LB_GETCURSEL, 0, 0);
				TCHAR temp[5];
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
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_b(wi->hContact, modname, "Reuse", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 4: // Give Up delay
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Give up after... (0 to not give up)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Days"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_b(wi->hContact, modname, "GiveUpDays", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				case 5:	// confirm window
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG, TranslateT("Show confirmation window? (0 to not Show)"));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
					SetDlgItemText(hwnd, IDC_SETTINGMSG2, TranslateT("Seconds to wait before sending"));
					SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(db_get_w(wi->hContact, modname, "ConfirmTimeout", 0), temp, 10));
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
					break;
				}
				db_set_b(wi->hContact, modname, "LastSetting", (BYTE)item);
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK SendPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SendPounceDlgProcStruct *spdps = (SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		spdps = (SendPounceDlgProcStruct*)lParam;
		if (!spdps)
			DestroyWindow(hwnd);

		spdps->timer = db_get_w(spdps->hContact, modname, "ConfirmTimeout", 0);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)spdps);
		{
			DBVARIANT dbv;
			if (db_get_ts(spdps->hContact, modname, "PounceMsg", &dbv))
				DestroyWindow(hwnd);
			else {
				SetDlgItemText(hwnd,IDC_MESSAGE, dbv.ptszVal);
				db_free(&dbv);
			}
		}
		SetTimer(hwnd,1,1000,NULL);
		SendMessage(hwnd,WM_TIMER,0,0);
		break;

	case WM_TIMER:
		{
			TCHAR message[1024];
			mir_sntprintf(message, SIZEOF(message), TranslateT("Pounce being sent to %s in %d seconds"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)spdps->hContact, GCDNF_TCHAR), spdps->timer);
			SetDlgItemText(hwnd, LBL_CONTACT, message);
		}
		spdps->timer--;
		if (spdps->timer < 0) {
			KillTimer(hwnd,1);
			SendPounce(spdps->message, spdps->hContact);
			DestroyWindow(hwnd);
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			KillTimer(hwnd,1);
			SendPounce(spdps->message, spdps->hContact);
			// fall through
		case IDCANCEL:
			KillTimer(hwnd,1);
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

	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		TranslateDialogDefault(hwnd);
		hContact = lParam;
		{
			DBVARIANT dbv;
			if (db_get_ts(hContact, modname, "PounceMsg", &dbv))
				DestroyWindow(hwnd);
			else {
				SetDlgItemText(hwnd, IDC_MESSAGE, dbv.ptszVal);
				db_free(&dbv);
			}
		}
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			{
				TCHAR text[2048];
				GetWindowText(GetDlgItem(hwnd, IDOK), text, SIZEOF(text));
				if (!_tcscmp(text, TranslateT("Retry"))) {
					GetWindowText(GetDlgItem(hwnd,IDC_MESSAGE), text, SIZEOF(text));
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
	HWND hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONFIRMSEND), 0, PounceSentDlgProc, hContact);
	TCHAR msg[256];
	if (SentSuccess) {
		mir_sntprintf(msg, SIZEOF(msg), TranslateT("Message successfully sent to %s"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
		SetWindowText(GetDlgItem(hwnd, IDOK), TranslateT("OK"));
		ShowWindow(GetDlgItem(hwnd, IDCANCEL), 0);
	}
	else {
		mir_sntprintf(msg, SIZEOF(msg), TranslateT("Message failed to send to %s"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
		SetWindowText(GetDlgItem(hwnd, IDOK), TranslateT("Retry"));
	}
	SetWindowText(GetDlgItem(hwnd, LBL_CONTACT), msg);
	SetWindowText(hwnd, TranslateT(modFullname));
}
