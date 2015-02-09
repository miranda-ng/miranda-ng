#include "common.h"

#define szAskSendSms  LPGEN("An SMS with registration code will be sent to your mobile phone.\nNotice that you are not able to use the real WhatsApp and this plugin simultaneously!\nContinue?")
#define szPasswordSet LPGEN("Your password has been set automatically. You can proceed with login now")

INT_PTR CALLBACK WhatsAppAccountProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WhatsAppProto *proto = (WhatsAppProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = (WhatsAppProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwndDlg, IDC_PW, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hwndDlg, IDC_PW2, EM_LIMITTEXT, 3, 0);
		CheckDlgButton(hwndDlg, IDC_SSL, db_get_b(NULL, proto->m_szModuleName, WHATSAPP_KEY_SSL, 0) ? BST_CHECKED : BST_UNCHECKED);
		{
			ptrA szStr(proto->getStringA(WHATSAPP_KEY_CC));
			if (szStr != NULL)
				SetDlgItemTextA(hwndDlg, IDC_CC, szStr);

			if ((szStr = proto->getStringA(WHATSAPP_KEY_LOGIN)) != NULL)
				SetDlgItemTextA(hwndDlg, IDC_LOGIN, szStr);

			if ((szStr = proto->getStringA(WHATSAPP_KEY_NICK)) != NULL)
				SetDlgItemTextA(hwndDlg, IDC_NICK, szStr);
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
		if (LOWORD(wParam) == IDC_BUTTON_REQUEST_CODE || LOWORD(wParam) == IDC_BUTTON_REGISTER) {
			string password;
			char cc[5];
			char number[128];
			GetDlgItemTextA(hwndDlg, IDC_CC, cc, SIZEOF(cc));
			GetDlgItemTextA(hwndDlg, IDC_LOGIN, number, SIZEOF(number));

			if (LOWORD(wParam) == IDC_BUTTON_REQUEST_CODE) {
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
			else if (LOWORD(wParam) == IDC_BUTTON_REGISTER) {
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

		if (HIWORD(wParam) == EN_CHANGE && reinterpret_cast<HWND>(lParam) == GetFocus()) {
			switch (LOWORD(wParam)) {
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
		if (reinterpret_cast<NMHDR *>(lParam)->code == PSN_APPLY) {
			char str[128];

			GetDlgItemTextA(hwndDlg, IDC_CC, str, SIZEOF(str));
			proto->setString(WHATSAPP_KEY_CC, str);

			GetDlgItemTextA(hwndDlg, IDC_LOGIN, str, SIZEOF(str));
			proto->setString(WHATSAPP_KEY_LOGIN, str);

			GetDlgItemTextA(hwndDlg, IDC_NICK, str, SIZEOF(str));
			proto->setString(WHATSAPP_KEY_NICK, str);

			proto->setByte(WHATSAPP_KEY_SSL, IsDlgButtonChecked(hwndDlg, IDC_SSL));
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invite dialog

static void FilterList(HWND hwndClist, WhatsAppProto *ppro)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (mir_strcmp(proto, ppro->m_szModuleName) || ppro->isChatRoom(hContact))
			if (MCONTACT hItem = SendMessage(hwndClist, CLM_FINDCONTACT, hContact, 0))
				SendMessage(hwndClist, CLM_DELETEITEM, hItem, 0);
	}
}

static void InitList(HWND hwndClist, WhatsAppProto *ppro)
{
	SetWindowLongPtr(hwndClist, GWL_STYLE,
		GetWindowLongPtr(hwndClist, GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	SendMessage(hwndClist, CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);
	SendMessage(hwndClist, CLM_SETBKBITMAP, 0, 0);
	SendMessage(hwndClist, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(hwndClist, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(hwndClist, CLM_SETLEFTMARGIN, 4, 0);
	SendMessage(hwndClist, CLM_SETINDENT, 10, 0);
	SendMessage(hwndClist, CLM_SETBKBITMAP, 0, 0);
	SendMessage(hwndClist, CLM_SETHIDEEMPTYGROUPS, TRUE, 0);
	SendMessage(hwndClist, CLM_SETHIDEOFFLINEROOT, TRUE, 0);

	for (int i = 0; i <= FONTID_MAX; i++)
		SendMessage(hwndClist, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));
	
	FilterList(hwndClist, ppro);
}

INT_PTR CALLBACK InviteDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WhatsAppProto *proto = (WhatsAppProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		proto = (WhatsAppProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		InitList(GetDlgItem(hwndDlg, IDC_CLIST), proto);
		return 1;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_CLIST) {
			switch (((LPNMHDR)lParam)->code) {
			case CLN_LISTREBUILT:
			case CLN_NEWCONTACT:
				FilterList(((LPNMHDR)lParam)->hwndFrom, proto);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;

		case IDOK:
			proto->m_szInviteJids.clear();

			// invite users from clist
			for (MCONTACT hContact = db_find_first(proto->m_szModuleName); hContact; hContact = db_find_next(hContact, proto->m_szModuleName)) {
				if (proto->isChatRoom(hContact))
					continue;

				HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);
				if (int hItem = SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0)) {
					if (SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0)) {
						ptrA jid(proto->getStringA(hContact, "ID"));
						if (jid != NULL)
							proto->m_szInviteJids.push_back((char*)jid);
					}
				}
			}

			HWND hwndEntry = GetDlgItem(hwndDlg, IDC_NEWJID);
			int len = GetWindowTextLength(hwndEntry);
			if (len > 0) {
				std::string szOther; szOther.resize(len + 1);
				GetWindowTextA(hwndEntry, (char*)szOther.data(), len);
				proto->m_szInviteJids.push_back(szOther);
			}

			EndDialog(hwndDlg, 1);
		}
	}

	return 0;
}
