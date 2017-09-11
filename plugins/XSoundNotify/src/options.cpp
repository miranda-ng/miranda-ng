/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "stdafx.h"

bool IsSuitableProto(PROTOACCOUNT *pa)
{
	if (pa == NULL)
		return false;

	if (pa->bDynDisabled || !pa->bIsEnabled)
		return false;

	if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMRECV))
		return false;

	return true;
}

static INT_PTR CALLBACK OptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		int count;
		PROTOACCOUNT **protos;
		Proto_EnumAccounts(&count, &protos);
		for (int i = 0; i < count; i++)
			if (IsSuitableProto(protos[i]))
				SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_SETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_ADDSTRING, 0, (LPARAM)protos[i]->tszAccountName), (LPARAM)protos[i]);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_OPT_COMBO_PROTO:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_COMBO_USERS), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_IGNORE_SOUND), FALSE);
				CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, BST_UNCHECKED);
				SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, TranslateT("Not set"));
				SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_RESETCONTENT, 0, 0);
				int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETCURSEL, 0, 0);
				PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);

				SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_SETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_ADDSTRING, 0, (LPARAM)TranslateT("All contacts")), cursel);
				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					char *szUniqueId = NULL;
					if (db_get_b(hContact, pa->szModuleName, "ChatRoom", 0))
						szUniqueId = "ChatRoomID";
					else
						szUniqueId = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
					if ((INT_PTR)szUniqueId != CALLSERVICE_NOTFOUND && szUniqueId != NULL) {
						DBVARIANT dbvuid = { 0 };
						if (!db_get(hContact, pa->szModuleName, szUniqueId, &dbvuid)) {
							wchar_t uid[MAX_PATH];
							switch (dbvuid.type) {
							case DBVT_DWORD:
								_itow(dbvuid.dVal, uid, 10);
								break;

							case DBVT_ASCIIZ:
								mir_wstrcpy(uid, _A2T(dbvuid.pszVal));
								break;

							case DBVT_UTF8:
								mir_wstrcpy(uid, ptrW(mir_utf8decodeW(dbvuid.pszVal)));
								break;

							default:
								mir_wstrcpy(uid, TranslateT("(Unknown contact)"));
							}

							wchar_t *nick = (wchar_t *)pcli->pfnGetContactDisplayName(hContact, 0);
							size_t value_max_len = (mir_wstrlen(uid) + mir_wstrlen(nick) + 4);
							wchar_t *value = (wchar_t *)mir_alloc(sizeof(wchar_t) * value_max_len);
							mir_snwprintf(value, value_max_len, L"%s (%s)", nick, uid);
							SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_SETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_ADDSTRING, 0, (LPARAM)value), hContact);
							mir_free(value);
							db_free(&dbvuid);
						}
					}
				}
			}
			return 0;

		case IDC_OPT_COMBO_USERS:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE)) {
				int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_CHOOSE_SOUND), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_IGNORE_SOUND), TRUE);
				if (!cursel) {
					PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
					DBVARIANT dbv = { 0 };
					if (!db_get_ws(NULL, SETTINGSNAME, pa->szModuleName, &dbv)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
						SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, PathFindFileName(dbv.ptszVal));
						db_free(&dbv);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), FALSE);
						SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, TranslateT("Not set"));
					}
					size_t value_max_len = mir_strlen(pa->szModuleName) + 8;
					char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
					mir_snprintf(value, value_max_len, "%s_ignore", pa->szModuleName);
					CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, db_get_b(NULL, SETTINGSNAME, value, 0) ? BST_CHECKED : BST_UNCHECKED);
					mir_free(value);
				}
				else {
					MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
					DBVARIANT dbv = { 0 };
					if (!db_get_ws(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
						SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, PathFindFileName(dbv.ptszVal));
						db_free(&dbv);
					}
					else {
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), FALSE);
						SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, TranslateT("Not set"));
					}
					CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, db_get_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, 0) ? BST_CHECKED : BST_UNCHECKED);
				}
			}
			return 0;

		case IDC_OPT_BUTTON_CHOOSE_SOUND:
		{
			wchar_t FileName[MAX_PATH];
			wchar_t *tszMirDir = Utils_ReplaceVarsW(L"%miranda_path%");

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			wchar_t tmp[MAX_PATH];
			if (GetModuleHandle(L"bass_interface.dll"))
				mir_snwprintf(tmp, L"%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%c", TranslateT("Sound files"), 0, 0, 0);
			else
				mir_snwprintf(tmp, L"%s (*.wav)%c*.wav%c%c", TranslateT("WAV files"), 0, 0, 0);
			ofn.lpstrFilter = tmp;
			ofn.hwndOwner = 0;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir = tszMirDir;
			*FileName = '\0';
			ofn.lpstrDefExt = L"";

			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, PathFindFileName(FileName));
				int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
				if (!cursel) {
					PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
					XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
					if (p == NULL)
						XSN_Users.insert(new XSN_Data((LPARAM)pa->szModuleName, FileName, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 0));
					else
					{
						wcsncpy(p->path, FileName, _countof(p->path));
						p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				else {
					MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
					XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
					if (p == NULL)
						XSN_Users.insert(new XSN_Data(hContact, FileName, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 1));
					else
					{
						wcsncpy(p->path, FileName, _countof(p->path));
						p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			mir_free(tszMirDir);
		}
		return 0;

		case IDC_OPT_BUTTON_TEST_PLAY:
		{
			int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
			if (!cursel){
				PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
				isIgnoreSound = 0;
				if (p == NULL) {
					DBVARIANT dbv;
					if (!db_get_ws(NULL, SETTINGSNAME, pa->szModuleName, &dbv)) {
						wchar_t longpath[MAX_PATH];
						PathToAbsoluteW(dbv.ptszVal, longpath);
						Skin_PlaySoundFile(longpath);
						db_free(&dbv);
					}
				}
				else {
					wchar_t longpath[MAX_PATH] = { 0 };
					PathToAbsoluteW(p->path, longpath);
					Skin_PlaySoundFile(longpath);
				}
			}
			else {
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				isIgnoreSound = 0;
				if (p == NULL) {
					DBVARIANT dbv;
					if (!db_get_ws(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						wchar_t longpath[MAX_PATH];
						PathToAbsoluteW(dbv.ptszVal, longpath);
						Skin_PlaySoundFile(longpath);
						db_free(&dbv);
					}
				}
				else {
					wchar_t longpath[MAX_PATH] = { 0 };
					PathToAbsoluteW(p->path, longpath);
					Skin_PlaySoundFile(longpath);
				}
			}
		}
		return 0;

		case IDC_OPT_BUTTON_RESET_SOUND:
		{
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), FALSE);
			CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, BST_UNCHECKED);
			SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, TranslateT("Not set"));
			int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
			if (!cursel) {
				PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
				if (p != NULL) {
					XSN_Users.remove(p);
					delete p;
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				db_unset(NULL, SETTINGSNAME, pa->szModuleName);
				size_t value_max_len = mir_strlen(pa->szModuleName) + 8;
				char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
				mir_snprintf(value, value_max_len, "%s_ignore", pa->szModuleName);
				db_unset(NULL, SETTINGSNAME, value);
				mir_free(value);
			}
			else {
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p != NULL) {
					XSN_Users.remove(p);
					delete p;
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				db_unset(hContact, SETTINGSNAME, SETTINGSKEY);
				db_unset(hContact, SETTINGSNAME, SETTINGSIGNOREKEY);
			}
		}
		return 0;

		case IDC_OPT_IGNORE_SOUND:
		{
			int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
			if (!cursel) {
				PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&pa->szModuleName);
				if (p == NULL) {
					DBVARIANT dbv;
					if (!db_get_ws(NULL, SETTINGSNAME, pa->szModuleName, &dbv)) {
						wchar_t longpath[MAX_PATH];
						PathToAbsoluteW(dbv.ptszVal, longpath);
						XSN_Users.insert(new XSN_Data((LPARAM)pa->szModuleName, longpath, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 0));
						db_free(&dbv);
					}
					else XSN_Users.insert(new XSN_Data((LPARAM)pa->szModuleName, L"", IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 0));
				}
				else p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			else {
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p == NULL) {
					DBVARIANT dbv;
					if (!db_get_ws(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						wchar_t longpath[MAX_PATH];
						PathToAbsoluteW(dbv.ptszVal, longpath);
						XSN_Users.insert(new XSN_Data(hContact, longpath, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 1));
						db_free(&dbv);
					}
					else XSN_Users.insert(new XSN_Data(hContact, L"", IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0, 1));
				}
				else p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		return 0;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR *)lParam;
		switch (hdr->code) {
		case PSN_APPLY:
			for (int i = 0; i < XSN_Users.getCount(); i++) {
				if (mir_wstrcmpi(XSN_Users[i]->path, L"")) {
					wchar_t shortpath[MAX_PATH];
					PathToRelativeW(XSN_Users[i]->path, shortpath);
					if (XSN_Users[i]->iscontact)
						db_set_ws(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSKEY, shortpath);
					else
						db_set_ws(NULL, SETTINGSNAME, (LPCSTR)XSN_Users[i]->hContact, shortpath);
				}
				if (XSN_Users[i]->iscontact)
					db_set_b(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSIGNOREKEY, XSN_Users[i]->ignore);
				else {
					size_t value_max_len = mir_strlen((const char*)XSN_Users[i]->hContact) + 8;
					char *value = (char *)mir_alloc(sizeof(char) * value_max_len);
					mir_snprintf(value, value_max_len, "%s_ignore", (const char*)XSN_Users[i]->hContact);
					db_set_b(NULL, SETTINGSNAME, value, XSN_Users[i]->ignore);
					mir_free(value);
				}
			}
			break;
		}
	}
	return FALSE;
}

INT OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szGroup.w = LPGENW("Sounds");
	odp.szTitle.w = LPGENW("XSound Notify");
	odp.pfnDlgProc = OptsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
