#include "stdafx.h"

int saveAsType(HWND hwnd, int original)
{
	if (!IsWindowVisible(GetDlgItem(hwnd, GRP_TYPE)))
		return original;

	if (IsDlgButtonChecked(hwnd, CHK_BYTE))
		return DBVT_BYTE;
	else if (IsDlgButtonChecked(hwnd, CHK_WORD))
		return DBVT_WORD;
	else if (IsDlgButtonChecked(hwnd, CHK_DWORD))
		return DBVT_DWORD;
	else if (IsDlgButtonChecked(hwnd, CHK_STRING))
		return DBVT_ASCIIZ;

	return original;
}

INT_PTR CALLBACK EditSettingDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		{
			struct DBsetting *dbsetting = (struct DBsetting*)lParam;

			char val[16] = {};
			int convert = 0;

			switch (dbsetting->dbv.type) {
			case DBVT_BYTE:
				CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_BYTE);
				ShowWindow(GetDlgItem(hwnd, IDC_STRING), SW_HIDE);
				CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, (g_Hex & HEX_BYTE) ? CHK_HEX : CHK_DECIMAL);
				if (dbsetting->setting) mir_snprintf(val, (g_Hex & HEX_BYTE) ? "0x%02X" : "%u", dbsetting->dbv.bVal);
				break;
			case DBVT_WORD:
				CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_WORD);
				ShowWindow(GetDlgItem(hwnd, IDC_STRING), SW_HIDE);
				CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, (g_Hex & HEX_WORD) ? CHK_HEX : CHK_DECIMAL);
				if (dbsetting->setting) mir_snprintf(val, (g_Hex & HEX_WORD) ? "0x%04X" : "%u", dbsetting->dbv.wVal);
				break;
			case DBVT_DWORD:
				CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_DWORD);
				ShowWindow(GetDlgItem(hwnd, IDC_STRING), SW_HIDE);
				CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, (g_Hex & HEX_DWORD) ? CHK_HEX : CHK_DECIMAL);
				if (dbsetting->setting) mir_snprintf(val, (g_Hex & HEX_DWORD) ? "0x%08X" : "%u", dbsetting->dbv.dVal);
				break;

			case DBVT_ASCIIZ:
			case DBVT_UTF8:
			case DBVT_WCHAR:
				ShowWindow(GetDlgItem(hwnd, IDC_STRING), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, IDC_SETTINGVALUE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_HEX), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_DECIMAL), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, GRP_BASE), SW_HIDE);
				break;

			case DBVT_BLOB:
				ShowWindow(GetDlgItem(hwnd, IDC_STRING), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, IDC_SETTINGVALUE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, IDC_BLOB), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, CHK_HEX), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_DECIMAL), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, GRP_BASE), SW_HIDE);
				break;

			case DBVT_DELETED: // resident
				CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, CHK_STRING);
				CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, CHK_DECIMAL);
				convert = 1;
				break;

			default:
				g_pMainWindow->msg(TranslateT("Unknown DBVariant type!"));
				DestroyWindow(hwnd);
				return TRUE;
			}


			if (dbsetting->setting) {
				SetDlgItemTextA(hwnd, IDC_SETTINGNAME, dbsetting->setting);

				switch (dbsetting->dbv.type) {
				case DBVT_BYTE:
				case DBVT_WORD:
				case DBVT_DWORD:
					SetDlgItemTextA(hwnd, IDC_SETTINGVALUE, val);
					convert = 1;
					break;

				case DBVT_ASCIIZ:
					SetDlgItemTextA(hwnd, IDC_STRING, dbsetting->dbv.pszVal);
					break;

				case DBVT_WCHAR:
					SetDlgItemTextW(hwnd, IDC_STRING, dbsetting->dbv.pwszVal);
					break;

				case DBVT_UTF8:
				{
					ptrW tmp(mir_utf8decodeW(dbsetting->dbv.pszVal));
					SetDlgItemTextW(hwnd, IDC_STRING, tmp);
					break;
				}

				case DBVT_BLOB:
				{
					ptrA tmp(StringFromBlob(dbsetting->dbv.pbVal, dbsetting->dbv.cpbVal));
					SetDlgItemTextA(hwnd, IDC_BLOB, tmp);
					break;
				}
				}
			}

			if (!convert) {
				ShowWindow(GetDlgItem(hwnd, GRP_TYPE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_BYTE), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_WORD), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_DWORD), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, CHK_STRING), SW_HIDE);
			}

			TranslateDialogDefault(hwnd);
			{
				wchar_t text[MSG_SIZE];
				mir_snwprintf(text, dbsetting->setting ? TranslateT("Edit setting (%s)") : TranslateT("New setting (%s)"), DBVType(dbsetting->dbv.type));
				SetWindowText(hwnd, text);
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CHK_BYTE:
		case CHK_WORD:
		case CHK_DWORD:
			EnableWindow(GetDlgItem(hwnd, CHK_HEX), 1);
			EnableWindow(GetDlgItem(hwnd, CHK_DECIMAL), 1);
			CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, LOWORD(wParam));
			break;
		case CHK_STRING:
			EnableWindow(GetDlgItem(hwnd, CHK_HEX), 0);
			EnableWindow(GetDlgItem(hwnd, CHK_DECIMAL), 0);
			CheckRadioButton(hwnd, CHK_BYTE, CHK_STRING, LOWORD(wParam));
			break;

		case CHK_HEX:
		case CHK_DECIMAL:
			CheckRadioButton(hwnd, CHK_HEX, CHK_DECIMAL, LOWORD(wParam));
			{
				wchar_t *setting, text[32];
				int settingLength, tmp;
				settingLength = GetWindowTextLength(GetDlgItem(hwnd, IDC_SETTINGVALUE));
				if (settingLength) {
					setting = (wchar_t*)mir_alloc((settingLength + 1) * sizeof(wchar_t));
					GetDlgItemText(hwnd, IDC_SETTINGVALUE, setting, settingLength + 1);
					if (LOWORD(wParam) == CHK_DECIMAL && IsDlgButtonChecked(hwnd, CHK_DECIMAL)) {
						swscanf(setting, L"%X", &tmp);
						mir_snwprintf(text, L"%u", tmp);
					}
					else {
						swscanf(setting, L"%u", &tmp);
						mir_snwprintf(text, L"%X", tmp);
					}
					SetDlgItemText(hwnd, IDC_SETTINGVALUE, text);
					mir_free(setting);
				}
			}
			break;

		case IDC_SETTINGNAME:
			EnableWindow(GetDlgItem(hwnd, IDOK), GetWindowTextLength(GetDlgItem(hwnd, IDC_SETTINGNAME)));
			break;

		case IDOK:
		{
			struct DBsetting *dbsetting = (struct DBsetting*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			wchar_t settingname[FLD_SIZE];
			GetDlgItemText(hwnd, IDC_SETTINGNAME, settingname, _countof(settingname));

			if (settingname[0]) {
				int valueID = 0;

				switch (dbsetting->dbv.type) {
				case DBVT_BYTE:
				case DBVT_WORD:
				case DBVT_DWORD:
				case DBVT_DELETED:
					valueID = IDC_SETTINGVALUE;
					break;

				case DBVT_ASCIIZ:
				case DBVT_UTF8:
				case DBVT_WCHAR:
					valueID = IDC_STRING;
					break;

				case DBVT_BLOB:
					valueID = IDC_BLOB;
					break;
				default:
					break;
				}

				if (!valueID)
					break;

				int len = GetWindowTextLength(GetDlgItem(hwnd, valueID)) + 1;
				wchar_t *value = (wchar_t*)mir_alloc(len * sizeof(wchar_t));

				GetDlgItemText(hwnd, valueID, value, len);
				_T2A setting(settingname);

				int type = saveAsType(hwnd, dbsetting->dbv.type);
				int res = 0;

				// write the setting
				switch (type) {
				case DBVT_BYTE:
				case DBVT_WORD:
				case DBVT_DWORD:
					res = setNumericValue(dbsetting->hContact, dbsetting->module, setting, wcstoul(value, nullptr, IsDlgButtonChecked(hwnd, CHK_HEX) ? 16 : 10), type);
					break;
				case DBVT_ASCIIZ:
				case DBVT_UTF8:
				case DBVT_WCHAR:
					res = setTextValue(dbsetting->hContact, dbsetting->module, setting, value, type);
					break;
				case DBVT_BLOB:
					res = WriteBlobFromString(dbsetting->hContact, dbsetting->module, setting, _T2A(value), len);
					break;
				}

				mir_free(value);

				if (!res) {
					g_pMainWindow->msg(TranslateT("Unable to store value in this data type!"));
					break;
				}

				// delete old setting
				if (dbsetting->setting && mir_strcmp(setting, dbsetting->setting))
					db_unset(dbsetting->hContact, dbsetting->module, dbsetting->setting);
			}

		}
		__fallthrough;

		case IDCANCEL:
			struct DBsetting *dbsetting = (struct DBsetting*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			mir_free(dbsetting->module);
			mir_free(dbsetting->setting);
			db_free(&dbsetting->dbv);
			mir_free(dbsetting);
			DestroyWindow(hwnd);
			break;
		}
	}
	return 0;
}

void CMainDlg::editSetting(MCONTACT hContact, const char *module, const char *setting)
{
	DBVARIANT dbv = {};
	if (db_get_s(hContact, module, setting, &dbv, 0) || IsResidentSetting(module, setting))
		return;

	struct DBsetting *dbsetting = (struct DBsetting *)mir_calloc(sizeof(struct DBsetting));
	dbsetting->dbv = dbv;
	dbsetting->hContact = hContact;
	dbsetting->module = mir_strdup(module);
	dbsetting->setting = mir_strdup(setting);
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EDIT_SETTING), m_hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
}

void CMainDlg::copySetting(MCONTACT hContact, const char *module, const char *setting)
{
	DBVARIANT dbv = {}, dbv2;
	if (db_get_s(hContact, module, setting, &dbv, 0))
		return;

	char tmp[FLD_SIZE];

	for (int i = 1; i < 10; i++) {
		mir_snprintf(tmp, "%s (%d)", setting, i);
		if (!db_get_s(hContact, module, tmp, &dbv2, 0))
			db_free(&dbv2);
		else {
			struct DBsetting *dbsetting = (struct DBsetting *)mir_calloc(sizeof(struct DBsetting));
			dbsetting->dbv = dbv;
			dbsetting->hContact = hContact;
			dbsetting->module = mir_strdup(module);
			dbsetting->setting = mir_strdup(tmp);
			CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EDIT_SETTING), m_hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
			return;
		}
	}
	db_free(&dbv);
}

void CMainDlg::newSetting(MCONTACT hContact, const char *module, int type)
{
	// gets safe_free()ed in the window proc
	DBsetting *dbsetting = (DBsetting*)mir_calloc(sizeof(DBsetting));
	dbsetting->dbv.type = type;
	dbsetting->hContact = hContact;
	dbsetting->module = mir_strdup(module);
	dbsetting->setting = nullptr;
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EDIT_SETTING), m_hwnd, EditSettingDlgProc, (LPARAM)dbsetting);
}
