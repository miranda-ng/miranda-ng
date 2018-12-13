#include "stdafx.h"

struct PasswordData
{
	MCONTACT hContact;
	int attempt;
};

bool CheckPassword(MCONTACT hContact, char *password)
{
	mir_md5_state_t ms;
	BYTE digest[16];
	mir_md5_init(&ms);
	mir_md5_append(&ms, (const unsigned char *)password, lstrlenA(password));
	mir_md5_finish(&ms, digest);

	DBVARIANT dbv;
	db_get(hContact, MODULENAME, "password", &dbv);
	if (dbv.type != DBVT_BLOB) {
		db_free(&dbv);
		return true;
	}
	else {
		if (dbv.cpbVal != 16) {
			db_free(&dbv);
			return false;
		}
		for (int i = 0; i < 16; i++) {
			if (dbv.pbVal[i] != digest[i]) {
				db_free(&dbv);
				return false;
			}
		}

		db_free(&dbv);
		return true;
	}
}

void SetPassword(MCONTACT hContact, char *password)
{
	if (!password || !*password) {
		db_unset(hContact, MODULENAME, "password");
		return;
	}

	mir_md5_state_t ms;
	BYTE digest[16];
	mir_md5_init(&ms);
	mir_md5_append(&ms, (const unsigned char *)password, lstrlenA(password));
	mir_md5_finish(&ms, digest);

	db_set_blob(hContact, MODULENAME, "password", (PBYTE)digest, 16);
}

INT_PTR CALLBACK PasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PasswordData *data = (PasswordData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		data = new PasswordData;
		data->attempt = 0;
		data->hContact = (MCONTACT)lParam;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);

		RECT rc, rcMe;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
		GetWindowRect(hwnd, &rcMe);
		SetWindowPos(hwnd, hwnd,
			rc.left + ((rc.right - rc.left) - (rcMe.right - rcMe.left)) / 2,
			rc.top + ((rc.bottom - rc.top) - (rcMe.bottom - rcMe.top)) / 2,
			0, 0, SWP_NOSIZE | SWP_NOZORDER);

		SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
		return TRUE;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDOK:
				data->attempt++;
				{
					int length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD));
					char *input = new char[length + 1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), input, length + 1);
					if (CheckPassword(data->hContact, input)) {
						MessageBeep(MB_OK);
						EndDialog(hwnd, 1);
					}
					else {
						if (data->attempt >= 3) {
							MessageBoxA(hwnd, "Sorry, but you've not managed to type\r\nthe correct password in 3 attempts.", "Newstory", MB_ICONHAND | MB_OK);
							EndDialog(hwnd, 0);
						}
						else {
							MessageBeep(MB_ICONHAND);
							SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), "");
						}
					}
					delete[] input;
				}
				return TRUE;

			case IDCANCEL:
				EndDialog(hwnd, 0);
				return TRUE;
			}
			break;
		}

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return TRUE;

	case WM_DESTROY:
		delete data;
		data = 0;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
	}
	return FALSE;
}

bool AskPassword(MCONTACT hContact)
{
	DBVARIANT dbv;
	db_get(hContact, MODULENAME, "password", &dbv);
	if (dbv.type != DBVT_BLOB) {
		db_free(&dbv);
		return true;
	}
	db_free(&dbv);

	if (DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_PASSWORD), 0, PasswordDlgProc, (LPARAM)hContact))
		return true;
	return false;
}


INT_PTR CALLBACK ChangePasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		{
			hContact = (MCONTACT)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);

			// RECT rc, rcMe;
			// SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
			// GetWindowRect(hwnd, &rcMe);
			// SetWindowPos(hwnd, hwnd,
			// rc.left+((rc.right-rc.left) - (rcMe.right-rcMe.left))/2, 
			// rc.top+((rc.bottom-rc.top) - (rcMe.bottom-rcMe.top))/2, 
			// 0, 0, SWP_NOSIZE|SWP_NOZORDER);

			SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
			return TRUE;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
			case IDOK:
				{
					int length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD));
					char *input = new char[length + 1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), input, length + 1);

					length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD2));
					char *input2 = new char[length + 1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD2), input2, length + 1);

					length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD3));
					char *input3 = new char[length + 1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD3), input3, length + 1);

					if (lstrcmpA(input2, input3) || !CheckPassword(hContact, input)) {
						MessageBeep(MB_ICONHAND);
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), "");
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD2), "");
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD3), "");
					}
					else {
						SetPassword(hContact, input2);
						EndDialog(hwnd, 0);
					}

					delete[] input;
					delete[] input2;
					delete[] input3;
				}
				return TRUE;

			case IDCANCEL:
				EndDialog(hwnd, 0);
				return TRUE;
			}
			break;
		}

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		DestroyWindow(hwnd);
		return TRUE;

	case WM_DESTROY:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
	}
	return FALSE;
}

void ChangePassword(HWND hwnd, MCONTACT hContact)
{
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CHANGEPASSWORD), hwnd, ChangePasswordDlgProc, (LPARAM)hContact);
}
