#include "stdafx.h"

struct PasswordData
{
	HANDLE hContact;
	int attempt;
};

bool CheckPassword(HANDLE hContact, char *password)
{
	md5_state_t ms;
	md5_byte_t digest[16];
	md5_init(&ms);
	md5_append(&ms, (const unsigned char *)password, lstrlenA(password));
	md5_finish(&ms, digest);

	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	dbcgs.szModule = MODULENAME;
	dbcgs.szSetting = "password";
	dbcgs.pValue = &dbv;
	CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&dbcgs);
	if (dbv.type != DBVT_BLOB)
	{
		CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
		return true;
	} else
	{
		if (dbv.cpbVal != 16)
		{
			CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
			return false;
		}
		for (int i = 0; i < 16; i++)
			if (dbv.pbVal[i] != digest[i])
			{
				CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
				return false;
			}
	
		CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
		return true;
	}
}

void SetPassword(HANDLE hContact, char *password)
{
	if (!password || !*password)
	{
		DBCONTACTGETSETTING dbcgs;
		dbcgs.szModule = MODULENAME;
		dbcgs.szSetting = "password";
		CallService(MS_DB_CONTACT_DELETESETTING, (WPARAM)hContact, (LPARAM)&dbcgs);
		return;
	}

	md5_state_t ms;
	md5_byte_t digest[16];
	md5_init(&ms);
	md5_append(&ms, (const unsigned char *)password, lstrlenA(password));
	md5_finish(&ms, digest);

	DBCONTACTWRITESETTING dbcws;
	dbcws.szModule = MODULENAME;
	dbcws.szSetting = "password";
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.cpbVal = 16;
	dbcws.value.pbVal = (PBYTE)digest;
	CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&dbcws);
}

int CALLBACK PasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PasswordData *data = (PasswordData *)GetWindowLong(hwnd, GWL_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			data = new PasswordData;
			data->attempt = 0;
			data->hContact = (HANDLE)lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)data);

			RECT rc, rcMe;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
			GetWindowRect(hwnd, &rcMe);
			SetWindowPos(hwnd, hwnd,
				rc.left+((rc.right-rc.left) - (rcMe.right-rcMe.left))/2, 
				rc.top+((rc.bottom-rc.top) - (rcMe.bottom-rcMe.top))/2, 
				0, 0, SWP_NOSIZE|SWP_NOZORDER);

			SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					data->attempt++;
					int length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD));
					char *input = new char[length+1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), input, length+1);
					if (CheckPassword(data->hContact, input))
					{
						MessageBeep(MB_OK);
						EndDialog(hwnd, 1);
					} else
					{
						if (data->attempt >= 3)
						{
							MessageBoxA(hwnd, "Sorry, but you've not managed to type\r\nthe correct password in 3 attempts.", "Newstory", MB_ICONHAND|MB_OK);
							EndDialog(hwnd, 0);
						} else
						{
							MessageBeep(MB_ICONHAND);
							SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), "");
						}
					}
					delete [] input;
					return TRUE;
				}

				case IDCANCEL:
				{
					EndDialog(hwnd, 0);
					return TRUE;
				}
			}
			break;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			return TRUE;
		}

		case WM_DESTROY:
		{
			delete data;
			data = 0;
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}
	return FALSE;
}

bool AskPassword(HANDLE hContact)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	dbcgs.szModule = MODULENAME;
	dbcgs.szSetting = "password";
	dbcgs.pValue = &dbv;
	CallService(MS_DB_CONTACT_GETSETTING, (WPARAM)hContact, (LPARAM)&dbcgs);
	if (dbv.type != DBVT_BLOB)
	{
		CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
		return true;
	}
	CallService(MS_DB_CONTACT_FREEVARIANT, 0, (LPARAM)&dbv);
	
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PASSWORD), 0, PasswordDlgProc, (LPARAM)hContact))
		return true;
	return false;
}


int CALLBACK ChangePasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			hContact = (HANDLE)lParam;
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)hContact);

//			RECT rc, rcMe;
//			SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
//			GetWindowRect(hwnd, &rcMe);
//			SetWindowPos(hwnd, hwnd,
//				rc.left+((rc.right-rc.left) - (rcMe.right-rcMe.left))/2, 
//				rc.top+((rc.bottom-rc.top) - (rcMe.bottom-rcMe.top))/2, 
//				0, 0, SWP_NOSIZE|SWP_NOZORDER);

			SetFocus(GetDlgItem(hwnd, IDC_PASSWORD));
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDOK:
				{
					int length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD));
					char *input = new char[length+1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), input, length+1);

					length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD2));
					char *input2 = new char[length+1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD2), input2, length+1);

					length = GetWindowTextLengthA(GetDlgItem(hwnd, IDC_PASSWORD3));
					char *input3 = new char[length+1];
					GetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD3), input3, length+1);

					if (lstrcmpA(input2, input3) || !CheckPassword(hContact, input))
					{
						MessageBeep(MB_ICONHAND);
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD), "");
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD2), "");
						SetWindowTextA(GetDlgItem(hwnd, IDC_PASSWORD3), "");
					} else
					{
						SetPassword(hContact, input2);
						EndDialog(hwnd, 0);
					}

					delete [] input;
					delete [] input2;
					delete [] input3;

					return TRUE;
				}

				case IDCANCEL:
				{
					EndDialog(hwnd, 0);
					return TRUE;
				}
			}
			break;
		}

		case WM_CLOSE:
		{
			EndDialog(hwnd, 0);
			DestroyWindow(hwnd);
			return TRUE;
		}

		case WM_DESTROY:
		{
			SetWindowLong(hwnd, GWL_USERDATA, 0);
		}
	}
	return FALSE;
}

void ChangePassword(HWND hwnd, HANDLE hContact)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHANGEPASSWORD), hwnd, ChangePasswordDlgProc, (LPARAM)hContact);
}
