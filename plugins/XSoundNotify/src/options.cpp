/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "Common.h"

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
		ProtoEnumAccounts(&count, &protos);
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

				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					char *szUniqueId = NULL;
					if (db_get_b(hContact, pa->szModuleName, "ChatRoom", 0))
						szUniqueId = "ChatRoomID";
					else
						szUniqueId = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
					if ((INT_PTR)szUniqueId != CALLSERVICE_NOTFOUND && szUniqueId != NULL) {
						DBVARIANT dbvuid = { 0 };
						if (!db_get(hContact, pa->szModuleName, szUniqueId, &dbvuid)) {
							TCHAR uid[MAX_PATH];
							switch (dbvuid.type) {
							case DBVT_DWORD:
								_itot(dbvuid.dVal, uid, 10);
								break;

							case DBVT_ASCIIZ:
								_tcscpy(uid, _A2T(dbvuid.pszVal));
								break;

							case DBVT_UTF8:
								_tcscpy(uid, ptrT(mir_utf8decodeT(dbvuid.pszVal)));
								break;
							}

							TCHAR *nick = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
							size_t value_max_len = (_tcslen(uid) + _tcslen(nick) + 4);
							TCHAR *value = (TCHAR *)mir_alloc(sizeof(TCHAR) * value_max_len);
							mir_sntprintf(value, value_max_len, _T("%s (%s)"), nick, uid);
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
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_CHOOSE_SOUND), TRUE);
				DBVARIANT dbv = { 0 };
				if (!db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
				{
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
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_IGNORE_SOUND), TRUE);
				CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, db_get_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, 0) ? BST_CHECKED : BST_UNCHECKED);
			}
			return 0;

		case IDC_OPT_BUTTON_CHOOSE_SOUND:
		{
			TCHAR FileName[MAX_PATH];
			TCHAR *tszMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));

			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(ofn);
			TCHAR tmp[MAX_PATH];
			if (GetModuleHandle(_T("bass_interface.dll")))
				mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.wav, *.mp3, *.ogg)%c*.wav;*.mp3;*.ogg%c%c"), TranslateT("Sound files"), 0, 0, 0);
			else
				mir_sntprintf(tmp, SIZEOF(tmp), _T("%s (*.wav)%c*.wav%c%c"), TranslateT("WAV files"), 0, 0, 0);
			ofn.lpstrFilter = tmp;
			ofn.hwndOwner = 0;
			ofn.lpstrFile = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir = tszMirDir;
			*FileName = '\0';
			ofn.lpstrDefExt = _T("");

			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, PathFindFileName(FileName));
				int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
				MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p == NULL)
					XSN_Users.insert(new XSN_Data(hContact, FileName, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
				else
				{
					_tcsncpy(p->path, FileName, SIZEOF(p->path));
					p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			mir_free(tszMirDir);
		}
		return 0;

		case IDC_OPT_BUTTON_TEST_PLAY:
		{
			int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
			MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
			XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
			isIgnoreSound = 0;
			if (p == NULL) {
				DBVARIANT dbv;
				if (!db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
					TCHAR longpath[MAX_PATH];
					PathToAbsoluteT(dbv.ptszVal, longpath);
					SkinPlaySoundFile(longpath);
					db_free(&dbv);
				}
			}
			else {
				TCHAR longpath[MAX_PATH] = { 0 };
				PathToAbsoluteT(p->path, longpath);
				SkinPlaySoundFile(longpath);
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
		return 0;

		case IDC_OPT_IGNORE_SOUND:
		{
			int cursel = SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETCURSEL, 0, 0);
			MCONTACT hContact = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
			XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
			if (p == NULL) {
				DBVARIANT dbv;
				if (!db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
					TCHAR longpath[MAX_PATH];
					PathToAbsoluteT(dbv.ptszVal, longpath);
					XSN_Users.insert(new XSN_Data(hContact, longpath, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
					db_free(&dbv);
				}
				else XSN_Users.insert(new XSN_Data(hContact, _T(""), IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
			}
			else p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		return 0;
		}
		break;

	case WM_NOTIFY:
		NMHDR *hdr = (NMHDR *)lParam;
		switch (hdr->code) {
		case PSN_APPLY:
			for (int i = 0; i < XSN_Users.getCount(); i++) {
				if (mir_tstrcmpi(XSN_Users[i]->path, _T(""))) {
					TCHAR shortpath[MAX_PATH];
					PathToRelativeT(XSN_Users[i]->path, shortpath);
					db_set_ts(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSKEY, shortpath);
				}
				db_set_b(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSIGNOREKEY, XSN_Users[i]->ignore);
			}
			break;
		}
	}
	return FALSE;
}

INT OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszGroup = LPGENT("Sounds");
	odp.ptszTitle = LPGENT("XSound Notify");
	odp.pfnDlgProc = OptsProc;
	Options_AddPage(wParam, &odp);
	return 0;
}
