#include "headers.h"

static BOOL Convert(MCONTACT hContact, char* module, char* setting, int value, int toType) // 0 = byte, 1 = word, 2 = dword, 3 = string
{
    int Result = 1;
	char temp[64];

	switch (toType) {
	case 0:
		if (value > 0xFF)
			Result = 0;
		else
			db_set_b(hContact, module, setting, (BYTE)value);
		break;
	case 1:
		if (value > 0xFFFF)
			Result = 0;
		else
			db_set_w(hContact, module, setting, (WORD)value);
		break;
	case 2:
		db_set_dw(hContact, module, setting, (DWORD)value);
		break;
	case 3:
		db_unset(hContact, module, setting);
		db_set_s(hContact, module, setting, itoa(value,temp,10));
		break;
	}
	return Result;
}


BOOL convertSetting(MCONTACT hContact, char* module, char* setting, int toType) // 0 = byte, 1 = word, 2 = dword, 3 = string, 4 = unicode
{
	DBVARIANT dbv = {0};
	BOOL Result = 0;

	if (!GetSetting(hContact, module, setting, &dbv)) {
		switch (dbv.type) {
		case DBVT_BYTE:
			Result = Convert(hContact, module, setting, dbv.bVal, toType);
			break;

		case DBVT_WORD:
			Result = Convert(hContact, module, setting, dbv.wVal, toType);
			break;

		case DBVT_DWORD:
			Result = Convert(hContact, module, setting, dbv.dVal, toType);
			break;

		case DBVT_ASCIIZ:
			if (toType == 4) // convert to UNICODE
			{
				int len = (int)strlen(dbv.pszVal) + 1;
				WCHAR *wc = (WCHAR*)_alloca(len*sizeof(WCHAR));
				MultiByteToWideChar(CP_ACP, 0, dbv.pszVal, -1, wc, len);
				Result = !db_set_ws(hContact, module, setting, wc);
			}
			else if (strlen(dbv.pszVal) < 11 && toType != 3) {
				int val = atoi(dbv.pszVal);
				if (val == 0 && dbv.pszVal[0] != '0')
					break;

				Result = Convert(hContact, module, setting, val, toType);
			}
			break;

		case DBVT_UTF8:
			if (toType == 3) { // convert to ANSI
				int len = (int)strlen(dbv.pszVal) + 1;
				char *sz = (char*)_alloca(len*3);
				WCHAR *wc = (WCHAR*)_alloca(len*sizeof(WCHAR));
				MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, wc, len);
				WideCharToMultiByte(CP_ACP, 0, wc, -1, sz, len, NULL, NULL);
				Result = !db_set_s(hContact, module, setting, sz);
			}
			break;
		}

		if (!Result)
			msg(Translate("Cannot Convert!"), modFullname);

		db_free(&dbv);
	}

	return Result;
}


int saveAsType(HWND hwnd)
{
	if(IsDlgButtonChecked(hwnd, CHK_BYTE))
		return 0;
	else if(IsDlgButtonChecked(hwnd, CHK_WORD))
		return 1;
	else if(IsDlgButtonChecked(hwnd, CHK_DWORD))
		return 2;
	else if(IsDlgButtonChecked(hwnd, CHK_STRING))
		return 3;
	return 3;
}


INT_PTR CALLBACK EditSettingDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			char tmp[32];
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)lParam);
			switch (((struct DBsetting*)lParam)->dbv.type)
			{
				case DBVT_BYTE:
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_HIDE);
					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New BYTE value"));
					}
					else
					{
						SetWindowText(hwnd, Translate("Edit BYTE value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
						SetDlgItemText(hwnd, IDC_SETTINGVALUE, itoa(((struct DBsetting*)lParam)->dbv.bVal, tmp, 10));
					}
					CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, CHK_DECIMAL);
					CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_BYTE);
				break;
				case DBVT_WORD:
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_HIDE);
					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New WORD value"));
					}
					else
					{
						SetWindowText(hwnd, Translate("Edit WORD value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
						SetDlgItemText(hwnd, IDC_SETTINGVALUE, itoa(((struct DBsetting*)lParam)->dbv.wVal, tmp, 10));
					}
					CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, CHK_DECIMAL);
					CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_WORD);
				break;
				case DBVT_DWORD:
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_HIDE);
					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New DWORD value"));
					}
					else
					{
						char text[32];
						SetWindowText(hwnd, Translate("Edit DWORD value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
						mir_snprintf(text, SIZEOF(text), "%X", ((struct DBsetting*)lParam)->dbv.dVal);
						SetDlgItemText(hwnd, IDC_SETTINGVALUE, text);
					}
					CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, CHK_HEX);
					CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_DWORD);
				break;
				case DBVT_ASCIIZ:
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGVALUE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_HEX),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DECIMAL),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_BASE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_TYPE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_BYTE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_WORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DWORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_STRING),SW_HIDE);
					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New STRING value"));
					}
					else
					{
						SetWindowText(hwnd, Translate("Edit STRING value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
						SetDlgItemText(hwnd, IDC_STRING, ((struct DBsetting*)lParam)->dbv.pszVal);
					}
				break;
				case DBVT_UTF8:
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_SHOW);
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGVALUE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_HEX),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DECIMAL),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_BASE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_TYPE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_BYTE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_WORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DWORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_STRING),SW_HIDE);
					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New UNICODE value"));
					}
					else
					{
						char *tmp = (((struct DBsetting*)lParam)->dbv.pszVal);
						int length = (int)strlen(tmp) + 1;
						WCHAR *wc = (WCHAR*)_alloca(length*sizeof(WCHAR));
						MultiByteToWideChar(CP_UTF8, 0, tmp, -1, wc, length);
						SetDlgItemTextW(hwnd, IDC_STRING, wc);

						SetWindowText(hwnd, Translate("Edit UNICODE value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
					}
				break;
				case DBVT_BLOB:
				{
					ShowWindow(GetDlgItem(hwnd, IDC_STRING),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_SETTINGVALUE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BLOB),SW_SHOW);

					if (!((struct DBsetting*)lParam)->setting[0])
					{
						SetWindowText(hwnd, Translate("New BLOB value"));
					}
					else
					{
						int j;
						char tmp[16];
						int len = ((struct DBsetting*)lParam)->dbv.cpbVal;
						char *data = (char*)_alloca(3*(len+1)+10);
						BYTE *p = ((struct DBsetting*)lParam)->dbv.pbVal;

						if (!data) return TRUE;
						data[0] = '\0';

						for(j=0; j<len; j++)
						{
							mir_snprintf(tmp, SIZEOF(tmp), "%02X ", (BYTE)p[j]);
							strcat(data, tmp);
						}

						SetWindowText(hwnd, Translate("Edit BLOB value"));
						SetDlgItemText(hwnd, IDC_SETTINGNAME, ((struct DBsetting*)lParam)->setting);
						SetDlgItemText(hwnd, IDC_BLOB, data);
					}
					ShowWindow(GetDlgItem(hwnd, CHK_HEX),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DECIMAL),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_BASE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, GRP_TYPE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_BYTE),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_WORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_DWORD),SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, CHK_STRING),SW_HIDE);
				}
				break;
				default: return TRUE;
			}
			TranslateDialogDefault(hwnd);
		}
		return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case CHK_BYTE:
				case CHK_WORD:
				case CHK_DWORD:
					EnableWindow(GetDlgItem(hwnd, CHK_HEX),1);
					EnableWindow(GetDlgItem(hwnd, CHK_DECIMAL),1);
					CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, LOWORD(wParam));
				break;
				case CHK_STRING:
					EnableWindow(GetDlgItem(hwnd, CHK_HEX),0);
					EnableWindow(GetDlgItem(hwnd, CHK_DECIMAL),0);
					CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, LOWORD(wParam));
				break;

				case CHK_HEX:
				case CHK_DECIMAL:
					CheckRadioButton(hwnd,CHK_HEX, CHK_DECIMAL, LOWORD(wParam));
					{
						char *setting, temp[32];
						int settingLength, tmp;
						settingLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_SETTINGVALUE));
						if (settingLength)
						{
							setting = (char*)_alloca(settingLength + 1);
							if (setting)
							{
								// havta convert it with mir_snprintf()
								GetWindowText(GetDlgItem(hwnd, IDC_SETTINGVALUE), setting, settingLength+1);
								if (LOWORD(wParam) == CHK_DECIMAL && IsDlgButtonChecked(hwnd, CHK_DECIMAL))
								{
									sscanf(setting, "%X", &tmp);
									mir_snprintf(temp, SIZEOF(temp), "%ld", tmp);
								}
								else
								{
									sscanf(setting, "%d", &tmp);
									mir_snprintf(temp, SIZEOF(temp), "%X", tmp);
								}
								SetWindowText(GetDlgItem(hwnd, IDC_SETTINGVALUE), temp);
							}
						}
					}
				break;
				case IDOK:
				{
					struct DBsetting *dbsetting = (struct DBsetting*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
					char *setting, *value;
					int settingLength, valueLength, valueID = IDC_SETTINGVALUE;
					settingLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_SETTINGNAME));

					if (IsWindowVisible(GetDlgItem(hwnd,IDC_STRING)))
						valueID = IDC_STRING;
					else
					if (IsWindowVisible(GetDlgItem(hwnd,IDC_SETTINGVALUE)))
						valueID = IDC_SETTINGVALUE;
					else
					if (IsWindowVisible(GetDlgItem(hwnd,IDC_BLOB)))
						valueID = IDC_BLOB;
					else
						break;

					valueLength = GetWindowTextLength(GetDlgItem(hwnd, valueID));

					if (dbsetting->dbv.type == DBVT_UTF8)
						valueLength *= sizeof(WCHAR);

					if (settingLength)
					{
						int settingValue;
						setting = (char*)_alloca(settingLength + 1);

						if (valueLength)
							value = (char*)_alloca(valueLength + 2);
						else
							value = (char*)_alloca(2);

						if (!setting || !value)
						{
							msg(Translate("Couldn't allocate enough memory!"), modFullname);
							DestroyWindow(hwnd);
							break;
						}

						GetWindowText(GetDlgItem(hwnd, IDC_SETTINGNAME), setting, settingLength+1);

						if (valueLength)
						{
							if (dbsetting->dbv.type == DBVT_UTF8)
								SendDlgItemMessageW(hwnd, valueID, WM_GETTEXT, valueLength+2, (LPARAM)value);
							else
								GetWindowText(GetDlgItem(hwnd, valueID), value, valueLength+1);
						}
						else
						if (IsWindowVisible(GetDlgItem(hwnd,IDC_STRING)) || (saveAsType(hwnd)==3))
							memcpy(value,"\0\0",2);
						else
							strcpy(value,"0");

						// delete the old setting
						if (mir_strcmp(setting, dbsetting->setting) && dbsetting->setting && (dbsetting->setting)[0] != 0)
							db_unset(dbsetting->hContact, dbsetting->module, dbsetting->setting);

						// delete the setting if we are saving as a different type
						switch (dbsetting->dbv.type)
						{
							case DBVT_BYTE:
								if (saveAsType(hwnd) != 0) db_unset(dbsetting->hContact, dbsetting->module, setting);
							break;
							case DBVT_WORD:
								if (saveAsType(hwnd) != 1) db_unset(dbsetting->hContact, dbsetting->module, setting);
							break;
							case DBVT_DWORD:
								if (saveAsType(hwnd) != 2) db_unset(dbsetting->hContact, dbsetting->module, setting);
							break;
							//case DBVT_ASCIIZ:
								//db_set_s(dbsetting->hContact, dbsetting->module, setting, value);
							//break;
						}
						// write the setting
						switch (saveAsType(hwnd))
						{
							case 0:
								if (IsDlgButtonChecked(hwnd, CHK_HEX)) sscanf(value, "%x", &settingValue);
								else sscanf(value, "%d", &settingValue);
								db_set_b(dbsetting->hContact, dbsetting->module, setting, (BYTE)settingValue);
							break;
							case 1:
								if (IsDlgButtonChecked(hwnd, CHK_HEX)) sscanf(value, "%x", &settingValue);
								else sscanf(value, "%d", &settingValue);
								db_set_w(dbsetting->hContact, dbsetting->module, setting, (WORD)settingValue);
							break;
							case 2:
								if (IsDlgButtonChecked(hwnd, CHK_HEX)) sscanf(value, "%x", &settingValue);
								else sscanf(value, "%d", &settingValue);
								db_set_dw(dbsetting->hContact, dbsetting->module, setting, (DWORD)settingValue);
							break;
							case 3:
								if (dbsetting->dbv.type == DBVT_UTF8)
									db_set_ws(dbsetting->hContact, dbsetting->module, setting, (WCHAR*)value);
								else if (dbsetting->dbv.type == DBVT_BLOB)
									WriteBlobFromString(dbsetting->hContact,dbsetting->module,setting,value,valueLength);
								else if (dbsetting->dbv.type == DBVT_ASCIIZ)
									db_set_s(dbsetting->hContact, dbsetting->module, setting, value);
							break;
						}

					}
				} // fall through
				case IDCANCEL:
				{
					struct DBsetting *dbsetting = (struct DBsetting*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
					mir_free(dbsetting->module);
					mir_free(dbsetting->setting);
					mir_free(dbsetting);
					DestroyWindow(hwnd);
				}
				break;
			}
		break;
		}
	return 0;
}

void editSetting(MCONTACT hContact, char* module, char* setting)
{
	DBVARIANT dbv = {0}; // freed in the dialog
	if (!GetSetting(hContact,module, setting, &dbv))
	{
		struct DBsetting *dbsetting = (struct DBsetting *)mir_alloc(sizeof(struct DBsetting)); // gets free()ed in the window proc

		dbsetting->dbv = dbv; // freed in the dialog
		dbsetting->hContact = hContact;
		dbsetting->module = mir_tstrdup(module);
		dbsetting->setting = mir_tstrdup(setting);

		if (dbv.type == DBVT_UTF8)
			CreateDialogParamW(hInst, MAKEINTRESOURCEW(IDD_EDIT_SETTING), hwnd2mainWindow, EditSettingDlgProc, (LPARAM)dbsetting);
		else
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_EDIT_SETTING), hwnd2mainWindow, EditSettingDlgProc, (LPARAM)dbsetting);
	}
}