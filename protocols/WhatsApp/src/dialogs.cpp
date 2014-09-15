#include "common.h"

INT_PTR CALLBACK WhatsAppAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	WhatsAppProto *proto;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<WhatsAppProto*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);
		SendDlgItemMessage(hwnd, IDC_REG_CODE_1, EM_LIMITTEXT, 3, 0);
		SendDlgItemMessage(hwnd, IDC_REG_CODE_2, EM_LIMITTEXT, 3, 0);
		CheckDlgButton(hwnd, IDC_SSL, db_get_b(NULL, proto->ModuleName(), WHATSAPP_KEY_SSL, 0));
		DBVARIANT dbv;

		if ( !db_get_s(0,proto->ModuleName(),WHATSAPP_KEY_CC,&dbv,DBVT_ASCIIZ))
		{
			SetDlgItemTextA(hwnd,IDC_CC,dbv.pszVal);
			db_free(&dbv);
		}

		if ( !db_get_s(0,proto->ModuleName(),WHATSAPP_KEY_LOGIN,&dbv,DBVT_ASCIIZ))
		{
			SetDlgItemTextA(hwnd,IDC_LOGIN,dbv.pszVal);
			db_free(&dbv);
		}

		if ( !db_get_s(0,proto->ModuleName(),WHATSAPP_KEY_NICK,&dbv,DBVT_ASCIIZ))
		{
			SetDlgItemTextA(hwnd,IDC_NICK,dbv.pszVal);
			db_free(&dbv);
		}

		if ( !db_get_s(0,proto->ModuleName(),WHATSAPP_KEY_PASS,&dbv,DBVT_ASCIIZ))
		{
			SetDlgItemTextA(hwnd,IDC_PW,dbv.pszVal);
			db_free(&dbv);
		}

		if (!proto->isOffline()) {
			SendMessage(GetDlgItem(hwnd, IDC_CC), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_LOGIN), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_NICK), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0);
			EnableWindow(GetDlgItem(hwnd, IDC_SSL), FALSE);
		}

		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDC_BUTTON_REQUEST_CODE || LOWORD(wparam) == IDC_BUTTON_REGISTER)
		{
			proto = reinterpret_cast<WhatsAppProto*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));

			char cc[5];
			GetDlgItemTextA(hwnd, IDC_CC, cc, sizeof(cc));
			char number[128];
			GetDlgItemTextA(hwnd, IDC_LOGIN, number, sizeof(number));

			if (LOWORD(wparam) == IDC_BUTTON_REQUEST_CODE)
			{
				if (MessageBox(NULL, TranslateT("An SMS with registration-code will be sent to your mobile phone.\nNotice that you are not able to use the real WhatsApp and this plugin simultaneously!\nContinue?"),
					PRODUCT_NAME, MB_YESNO) == IDYES)
				{
					proto->Register(REG_STATE_REQ_CODE, string(cc), string(number), string());
				}
			}
			else if (LOWORD(wparam) == IDC_BUTTON_REGISTER)
			{
				char code[7];
				if (SendDlgItemMessage(hwnd, IDC_REG_CODE_1, WM_GETTEXTLENGTH, 0, 0) == 3 &&
					SendDlgItemMessage(hwnd, IDC_REG_CODE_2, WM_GETTEXTLENGTH, 0, 0) == 3)
				{
					GetDlgItemTextA(hwnd, IDC_REG_CODE_1, code, 4);
					GetDlgItemTextA(hwnd, IDC_REG_CODE_2, &(code[3]), 4);
				}
				else
				{
					MessageBox(NULL, TranslateT("Please correctly specify your registration code received by SMS"), 
						PRODUCT_NAME, MB_ICONEXCLAMATION);
					return TRUE;
				}
				string pw = proto->Register(REG_STATE_REG_CODE, string(cc), string(number), string(code));
				if (!pw.empty())
				{
					proto->setString(WHATSAPP_KEY_PASS, pw.c_str());
					MessageBox(NULL, TranslateT("Your password has been set automatically.\nIf you change your password manually, you may lose it and need to request a new code!"), PRODUCT_NAME, MB_ICONWARNING);
				}
			}
		}

		if ( HIWORD( wparam ) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
		{
			switch(LOWORD(wparam))
			{
			case IDC_CC:
			case IDC_LOGIN:
			case IDC_NICK:
			case IDC_SSL:
			case IDC_PW:
				SendMessage(GetParent(hwnd) ,PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR *>(lparam)->code == PSN_APPLY)
		{
			proto = reinterpret_cast<WhatsAppProto *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			char str[128];

			GetDlgItemTextA(hwnd, IDC_CC, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_CC, str);

			GetDlgItemTextA(hwnd, IDC_LOGIN, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_LOGIN, str);

			GetDlgItemTextA(hwnd, IDC_NICK, str, sizeof(str));
			db_set_s(0, proto->ModuleName(), WHATSAPP_KEY_NICK, str);

			db_set_b(0, proto->ModuleName(), WHATSAPP_KEY_SSL, IsDlgButtonChecked(hwnd, IDC_SSL));

			GetDlgItemTextA(hwnd,IDC_PW,str,sizeof(str));
			db_set_s(0,proto->ModuleName(),WHATSAPP_KEY_PASS,str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}

INT_PTR CALLBACK WhatsAppInputBoxProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
{
	input_box* ib;

	switch(message)
	{

	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwnd);

		ib = reinterpret_cast<input_box*>(lparam);
		SetWindowLongPtr(hwnd,GWLP_USERDATA,lparam);
		SendDlgItemMessage(hwnd,IDC_VALUE,EM_LIMITTEXT,ib->limit,0);
		SetDlgItemText(hwnd, IDC_VALUE, ib->defaultValue.c_str());
		SetDlgItemText(hwnd, IDC_TEXT, ib->text.c_str());

		/*
		DBVARIANT dbv = { DBVT_TCHAR };
		if (!DBGetContactSettingTString(NULL,ib->proto->m_szModuleName,WHATSAPP_KEY_PUSH_NAME,&dbv))
		{
			SetWindowText( hwnd, dbv.ptszVal );
			db_free( &dbv );
		}
		*/
		SetWindowText(hwnd, ib->title.c_str());
	}

	EnableWindow(GetDlgItem( hwnd, IDC_OK ), FALSE);
	return TRUE;

	case WM_COMMAND:
		if ( LOWORD( wparam ) == IDC_VALUE && HIWORD( wparam ) == EN_CHANGE )
		{
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
			size_t len = SendDlgItemMessage(hwnd,IDC_VALUE,WM_GETTEXTLENGTH,0,0);
			TCHAR str[4];
			mir_sntprintf(str, 4, TEXT("%d"), ib->limit - len);
			//SetDlgItemText(hwnd,IDC_CHARACTERS,str);

			EnableWindow(GetDlgItem( hwnd, IDC_OK ), len > 0);
			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDC_OK )
		{
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
			TCHAR* value = new TCHAR[ib->limit+1];

			GetDlgItemText(hwnd,IDC_VALUE,value, ib->limit + 1);
			ShowWindow(hwnd,SW_HIDE);

			input_box_ret* ret = new input_box_ret;
			ret->userData = ib->userData;
			ret->value = mir_utf8encodeT(value);
			delete value;
			ib->proto->ForkThread(ib->thread, ret);
			EndDialog(hwnd, wparam);
			delete ib;
			return TRUE;
		}
		else if ( LOWORD( wparam ) == IDC_CANCEL )
		{
			EndDialog(hwnd, wparam);
			ib = reinterpret_cast<input_box*>(GetWindowLongPtr(hwnd,GWLP_USERDATA));
			delete ib;
			return TRUE;
		}
		break;

	}

	return FALSE;
}