#include "common.h"

#define szAskSendSms  LPGEN("An SMS with registration code will be sent to your mobile phone.\nNotice that you are not able to use the real WhatsApp and this plugin simultaneously!\nContinue?")
#define szPasswordSet LPGEN("Your password has been set automatically. You can proceed with login now")

INT_PTR CALLBACK WhatsAppAccountProc(HWND hwndDlg, UINT message, WPARAM wparam, LPARAM lparam)
{
	WhatsAppProto *proto;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = reinterpret_cast<WhatsAppProto*>(lparam);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam);
		SendDlgItemMessage(hwndDlg, IDC_PW, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hwndDlg, IDC_PW2, EM_LIMITTEXT, 3, 0);
		CheckDlgButton(hwndDlg, IDC_SSL, db_get_b(NULL, proto->ModuleName(), WHATSAPP_KEY_SSL, 0));
		DBVARIANT dbv;

		if (!db_get_s(0, proto->ModuleName(), WHATSAPP_KEY_CC, &dbv, DBVT_ASCIIZ)) {
			SetDlgItemTextA(hwndDlg, IDC_CC, dbv.pszVal);
			db_free(&dbv);
		}

		if (!db_get_s(0, proto->ModuleName(), WHATSAPP_KEY_LOGIN, &dbv, DBVT_ASCIIZ)) {
			SetDlgItemTextA(hwndDlg, IDC_LOGIN, dbv.pszVal);
			db_free(&dbv);
		}

		if (!db_get_s(0, proto->ModuleName(), WHATSAPP_KEY_NICK, &dbv, DBVT_ASCIIZ)) {
			SetDlgItemTextA(hwndDlg, IDC_NICK, dbv.pszVal);
			db_free(&dbv);
		}

		EnableWindow(GetDlgItem(hwndDlg, IDC_PW), FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PW2), FALSE);

		if (!proto->isOffline()) {
			SendDlgItemMessage(hwndDlg, IDC_CC, EM_SETREADONLY, 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_LOGIN, EM_SETREADONLY, 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_NICK, EM_SETREADONLY, 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_PW, EM_SETREADONLY, 1, 0);
			SendDlgItemMessage(hwndDlg, IDC_PW2, EM_SETREADONLY, 1, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SSL), FALSE);
		}

		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_BUTTON_REQUEST_CODE || LOWORD(wparam) == IDC_BUTTON_REGISTER) {
			proto = reinterpret_cast<WhatsAppProto*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));

			string password;
			char cc[5];
			GetDlgItemTextA(hwndDlg, IDC_CC, cc, sizeof(cc));
			char number[128];
			GetDlgItemTextA(hwndDlg, IDC_LOGIN, number, sizeof(number));

			if (LOWORD(wparam) == IDC_BUTTON_REQUEST_CODE) {
				if (IDYES == MessageBox(NULL, TranslateT(szAskSendSms), PRODUCT_NAME, MB_YESNO)) {
					if (proto->Register(REG_STATE_REQ_CODE, string(cc), string(number), string(), password)) {
						if (!password.empty()) {
							MessageBox(NULL, TranslateT(szPasswordSet), PRODUCT_NAME, MB_ICONWARNING);
							proto->setString(WHATSAPP_KEY_PASS, password.c_str());
						}
						else {
							EnableWindow(GetDlgItem(hwndDlg, IDC_PW), TRUE); // unblock sms code entry field
							EnableWindow(GetDlgItem(hwndDlg, IDC_PW2), TRUE);
						}
					}
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_REGISTER) {
				HWND hwnd1 = GetDlgItem(hwndDlg, IDC_PW), hwnd2 = GetDlgItem(hwndDlg, IDC_PW2);
				if (GetWindowTextLength(hwnd1) != 3 || GetWindowTextLength(hwnd2) != 3) {
					MessageBox(NULL, TranslateT("Please correctly specify your registration code received by SMS"), PRODUCT_NAME, MB_ICONEXCLAMATION);
					return TRUE;
				}

				char code[10];
				GetWindowTextA(hwnd1, code, 4);
				GetWindowTextA(hwnd2, code + 3, 4);

				if (proto->Register(REG_STATE_REG_CODE, string(cc), string(number), string(code), password)) {
					proto->setString(WHATSAPP_KEY_PASS, password.c_str());
					MessageBox(NULL, TranslateT(szPasswordSet), PRODUCT_NAME, MB_ICONWARNING);
				}
			}
		}

		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus()) {
			switch (LOWORD(wparam)) {
			case IDC_CC:
			case IDC_LOGIN:
			case IDC_NICK:
			case IDC_SSL:
			case IDC_PW:
			case IDC_PW2:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR *>(lparam)->code == PSN_APPLY) {
			proto = reinterpret_cast<WhatsAppProto *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			char str[128];

			GetDlgItemTextA(hwndDlg, IDC_CC, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_CC, str);

			GetDlgItemTextA(hwndDlg, IDC_LOGIN, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_LOGIN, str);

			GetDlgItemTextA(hwndDlg, IDC_NICK, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_NICK, str);

			db_set_b(0, proto->ModuleName(), WHATSAPP_KEY_SSL, IsDlgButtonChecked(hwndDlg, IDC_SSL));
			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK WhatsAppInputBoxProc(HWND hwndDlg, UINT message, WPARAM wparam, LPARAM lparam)
{
	input_box* ib;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		ib = reinterpret_cast<input_box*>(lparam);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lparam);
		SendDlgItemMessage(hwndDlg, IDC_VALUE, EM_LIMITTEXT, ib->limit, 0);
		SetDlgItemText(hwndDlg, IDC_VALUE, ib->defaultValue.c_str());
		SetDlgItemText(hwndDlg, IDC_TEXT, ib->text.c_str());

		SetWindowText(hwndDlg, ib->title.c_str());

		EnableWindow(GetDlgItem(hwndDlg, IDC_OK), FALSE);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_VALUE && HIWORD(wparam) == EN_CHANGE) {
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			size_t len = SendDlgItemMessage(hwndDlg, IDC_VALUE, WM_GETTEXTLENGTH, 0, 0);
			TCHAR str[4];
			mir_sntprintf(str, 4, TEXT("%d"), ib->limit - len);
			//SetDlgItemText(hwndDlg,IDC_CHARACTERS,str);

			EnableWindow(GetDlgItem(hwndDlg, IDC_OK), len > 0);
			return TRUE;
		}

		if (LOWORD(wparam) == IDC_OK) {
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			TCHAR* value = new TCHAR[ib->limit + 1];

			GetDlgItemText(hwndDlg, IDC_VALUE, value, ib->limit + 1);
			ShowWindow(hwndDlg, SW_HIDE);

			input_box_ret* ret = new input_box_ret;
			ret->userData = ib->userData;
			ret->value = mir_utf8encodeT(value);
			delete value;
			ib->proto->ForkThread(ib->thread, ret);
			EndDialog(hwndDlg, wparam);
			delete ib;
			return TRUE;
		}

		if (LOWORD(wparam) == IDC_CANCEL) {
			EndDialog(hwndDlg, wparam);
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
			delete ib;
			return TRUE;
		}
		break;
	}

	return FALSE;
}