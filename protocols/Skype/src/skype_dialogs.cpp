#include "skype_proto.h"

INT_PTR CALLBACK CSkypeProto::SkypeAccountProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	CSkypeProto *proto;

	switch ( message )
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);

		proto = reinterpret_cast<CSkypeProto*>(lparam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lparam);

		DBVARIANT dbv;
		if ( !DBGetContactSettingString(0, proto->ModuleName(), "SkypeName", &dbv))
		{
			SetDlgItemText(hwnd, IDC_SN, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		if ( !DBGetContactSettingString(0, proto->ModuleName(), "Password", &dbv))
		{
			CallService(
				MS_DB_CRYPT_DECODESTRING,
				wcslen(dbv.ptszVal) + 1,
				reinterpret_cast<LPARAM>(dbv.ptszVal));
			SetDlgItemText(hwnd, IDC_PW, dbv.ptszVal);
			DBFreeVariant(&dbv);
		}

		if ( !proto->IsOffline()) 
		{
			SendMessage(GetDlgItem(hwnd, IDC_SN), EM_SETREADONLY, 1, 0);
			SendMessage(GetDlgItem(hwnd, IDC_PW), EM_SETREADONLY, 1, 0); 
		}

		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wparam) == EN_CHANGE && reinterpret_cast<HWND>(lparam) == GetFocus())
		{
			switch(LOWORD(wparam))
			{
			case IDC_SN:
			case IDC_PW:
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (reinterpret_cast<NMHDR*>(lparam)->code == PSN_APPLY)
		{
			proto = reinterpret_cast<CSkypeProto*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			TCHAR str[128];

			GetDlgItemText(hwnd, IDC_SN, str, sizeof(str));
			DBWriteContactSettingTString(0, proto->ModuleName(), "SkypeName", str);

			GetDlgItemText(hwnd, IDC_PW, str, sizeof(str));
			CallService(MS_DB_CRYPT_ENCODESTRING, sizeof(str), reinterpret_cast<LPARAM>(str));
			DBWriteContactSettingTString(0, proto->ModuleName(), "Password", str);

			return TRUE;
		}
		break;

	}

	return FALSE;
}