/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "Common.h"

HINSTANCE hInst;
int hLangpack;
LIST<XSN_Data> XSN_Users(10, HandleKeySortT);
HGENMENU hChangeSound = NULL;
HANDLE hChangeSoundDlgList = NULL;
BYTE isIgnoreSound = 0, isOwnSound = 0;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {08B86253-EC6E-4D09-B7A9-64ACDF0627B8}
	{0x8b86253, 0xec6e, 0x4d09, {0xb7, 0xa9, 0x64, 0xac, 0xdf, 0x6, 0x27, 0xb8}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

bool isReceiveMessage(HANDLE hDbEvent)
{
	DBEVENTINFO info = { sizeof(info) };
	db_event_get(hDbEvent, &info);
	// i don't understand why it works and how it works, but it works correctly - practice way (методом тыка)
	// so, i think correct condition would be : eventType == EVENTTYPE_MESSAGE && info.flags & DBEF_READ, but it really isn't
	return !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

INT ProcessEvent(WPARAM wParam, LPARAM lParam)
{
	if (!isReceiveMessage(HANDLE(lParam)))
		return 0;

	isIgnoreSound = db_get_b((HANDLE)wParam, SETTINGSNAME, SETTINGSIGNOREKEY, 0);
	DBVARIANT dbv;
	if ( !isIgnoreSound && !db_get_ts((HANDLE)wParam, SETTINGSNAME, SETTINGSKEY, &dbv)) {
		TCHAR PlaySoundPath[MAX_PATH] = {0};
		PathToAbsoluteT(dbv.ptszVal, PlaySoundPath);
		SkinPlaySoundFile(PlaySoundPath);
		db_free(&dbv);
		isOwnSound = 1;
	}

	return 0;
}

int OnPlaySound(WPARAM wParam, LPARAM lParam)
{
	if (isIgnoreSound)
		return 1;
	if (isOwnSound) {
		isOwnSound = 0;
		return 1;
	}

	return 0;
}

bool IsSuitableProto(PROTOACCOUNT* pa)
{
	if (pa == NULL)
		return false;

	if (pa->bDynDisabled || !pa->bIsEnabled)
		return false;

	if (!(CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMRECV))
		return false;

	return true;
}

INT_PTR CALLBACK OptsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		int count;
		PROTOACCOUNT **protos;
		ProtoEnumAccounts(&count, &protos);
		for(int i = 0; i < count; i++)
			if (IsSuitableProto(protos[i]))
				SendMessage(GetDlgItem(hwndDlg, IDC_OPT_COMBO_PROTO), CB_SETITEMDATA, SendMessage(GetDlgItem(hwndDlg, IDC_OPT_COMBO_PROTO), CB_ADDSTRING, 0, (LPARAM)protos[i]->tszAccountName), (LPARAM)protos[i]);
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

				for (HANDLE hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
					char* szUniqueId = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
					if ((INT_PTR) szUniqueId != CALLSERVICE_NOTFOUND && szUniqueId != NULL) {
						DBVARIANT dbvuid = {0};
						if ( !db_get(hContact, pa->szModuleName, szUniqueId, &dbvuid)) {
							TCHAR uid[MAX_PATH];
							switch(dbvuid.type) {
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

							TCHAR *nick = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
							TCHAR *value = (TCHAR *)mir_alloc(sizeof(TCHAR) * (_tcslen(uid) + _tcslen(nick) + 4));
							mir_sntprintf(value, _tcslen(uid) + _tcslen(nick) + 4, _T("%s (%s)"), nick, uid);
							SendMessage(GetDlgItem(hwndDlg, IDC_OPT_COMBO_USERS), CB_SETITEMDATA, SendMessage(GetDlgItem(hwndDlg, IDC_OPT_COMBO_USERS), CB_ADDSTRING, 0, (LPARAM)value), (LPARAM)hContact);
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
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_CHOOSE_SOUND), TRUE);
				DBVARIANT dbv = {0};
				if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), TRUE);
					SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, PathFindFileName(dbv.ptszVal));
					db_free(&dbv);
				} else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_TEST_PLAY), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_BUTTON_RESET_SOUND), FALSE);
					SetDlgItemText(hwndDlg, IDC_OPT_LABEL_SOUND, TranslateT("Not set"));
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_OPT_IGNORE_SOUND), TRUE);
				CheckDlgButton(hwndDlg, IDC_OPT_IGNORE_SOUND, db_get_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, 0));
			}
			return 0;

		case IDC_OPT_BUTTON_CHOOSE_SOUND:
			{
				TCHAR FileName[MAX_PATH];
				TCHAR *tszMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));

				OPENFILENAME ofn = {0};
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
					HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
					XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
					if (p == NULL)
						XSN_Users.insert( new XSN_Data(hContact, FileName, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
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
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				isIgnoreSound = 0;
				if (p == NULL) {
					DBVARIANT dbv;
					if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						TCHAR longpath[MAX_PATH];
						PathToAbsoluteT(dbv.ptszVal, longpath);
						SkinPlaySoundFile(longpath);
						db_free(&dbv);
					}
				}
				else {
					TCHAR longpath[MAX_PATH] = {0};
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
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
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
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_OPT_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p == NULL)
				{
					DBVARIANT dbv;
					if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						TCHAR longpath[MAX_PATH];
						PathToAbsoluteT(dbv.ptszVal, longpath);
						XSN_Users.insert( new XSN_Data(hContact, longpath, IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
						db_free(&dbv);
					} else
						XSN_Users.insert( new XSN_Data(hContact, _T(""), IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0));
				}
				else
					p->ignore = IsDlgButtonChecked(hwndDlg, IDC_OPT_IGNORE_SOUND) ? 1 : 0;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			return 0;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			switch (hdr->code) {
			case PSN_APPLY:
				for (int i = 0; i < XSN_Users.getCount(); i++) {
					if (lstrcmpi(XSN_Users[i]->path, _T(""))) {
						TCHAR shortpath[MAX_PATH];
						PathToRelativeT(XSN_Users[i]->path, shortpath);
						db_set_ts(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSKEY, shortpath);
					}
					db_set_b(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSIGNOREKEY, XSN_Users[i]->ignore);
				}
				break;
			}
		}
	}
	return FALSE;
}

INT OptInit(WPARAM wParam, LPARAM lParam)
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

INT_PTR CALLBACK DlgProcContactsOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			HANDLE hContact = (HANDLE)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			WindowList_Add(hChangeSoundDlgList, hwndDlg, hContact);
			Utils_RestoreWindowPositionNoSize(hwndDlg, hContact, SETTINGSNAME, "ChangeSoundDlg");
			char* szProto = GetContactProto(hContact);
			PROTOACCOUNT *pa = ProtoGetAccount(szProto);
			char* szUniqueId = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
			if ((INT_PTR) szUniqueId != CALLSERVICE_NOTFOUND && szUniqueId != NULL) {
				DBVARIANT dbvuid = {0};
				if ( !db_get(hContact, pa->szModuleName, szUniqueId, &dbvuid)) {
					TCHAR uid[MAX_PATH];
					switch(dbvuid.type) {
					case DBVT_DWORD:
						_itot(dbvuid.dVal, uid, 10);
						break;

					case DBVT_ASCIIZ:
						_tcscpy(uid, _A2T(dbvuid.pszVal));
						break;

					case DBVT_UTF8:
						_tcscpy(uid, ptrT( mir_utf8decodeT(dbvuid.pszVal)));
						break;
					}

					TCHAR *nick = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
					TCHAR value[100];
					mir_sntprintf(value, SIZEOF(value), TranslateT("Custom sound for %s (%s)"), nick, uid);
					SetWindowText(hwndDlg, value);
					db_free(&dbvuid);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_CHOOSE_SOUND), TRUE);
			DBVARIANT dbv = {0};
			if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_TEST_PLAY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_RESET_SOUND), TRUE);
				SetDlgItemText(hwndDlg, IDC_CONT_LABEL_SOUND, PathFindFileName(dbv.ptszVal));
				db_free(&dbv);
			} else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_TEST_PLAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_RESET_SOUND), FALSE);
				SetDlgItemText(hwndDlg, IDC_CONT_LABEL_SOUND, TranslateT("Not set"));
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_IGNORE_SOUND), TRUE);
			CheckDlgButton(hwndDlg, IDC_CONT_IGNORE_SOUND, db_get_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, 0));
			XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
			if (p == NULL)
			{
				DBVARIANT dbv;
				if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
				{
					XSN_Users.insert( new XSN_Data(hContact, dbv.ptszVal, IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0));
					db_free(&dbv);
				}
			}
		}
		return TRUE;
	
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p != NULL)
				{
					if (lstrcmpi(p->path, _T(""))) {
						TCHAR shortpath[MAX_PATH];
						PathToRelativeT(p->path, shortpath);
						db_set_ts(hContact, SETTINGSNAME, SETTINGSKEY, shortpath);
					}
					db_set_b(hContact, SETTINGSNAME, SETTINGSIGNOREKEY, p->ignore);
				}
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_CONT_BUTTON_CHOOSE_SOUND:
			{
				TCHAR FileName[MAX_PATH];
				TCHAR *tszMirDir = Utils_ReplaceVarsT(_T("%miranda_path%"));

				OPENFILENAME ofn = {0};
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
					HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					SetDlgItemText(hwndDlg, IDC_CONT_LABEL_SOUND, PathFindFileName(FileName));
					XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
					if (p == NULL)
						XSN_Users.insert( new XSN_Data(hContact, FileName, IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0));
					else
					{
						_tcsncpy(p->path, FileName, SIZEOF(p->path));
						p->ignore = IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0;
					}
					EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_RESET_SOUND), TRUE);
				}
				mir_free(tszMirDir);
			}
			break;

		case IDC_CONT_BUTTON_TEST_PLAY:
			{
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				isIgnoreSound = 0;
				if (p == NULL)
				{
					DBVARIANT dbv;
					if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
					{
						TCHAR longpath[MAX_PATH] = {0};
						PathToAbsoluteT(dbv.ptszVal, longpath);
						SkinPlaySoundFile(longpath);
						db_free(&dbv);
					}
				}
				else
				{
					TCHAR longpath[MAX_PATH] = {0};
					PathToAbsoluteT(p->path, longpath);
					SkinPlaySoundFile(longpath);
				}
			}
			break;

		case IDC_CONT_BUTTON_RESET_SOUND:
			{
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_TEST_PLAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CONT_BUTTON_RESET_SOUND), FALSE);
				CheckDlgButton(hwndDlg, IDC_CONT_IGNORE_SOUND, BST_UNCHECKED);
				SetDlgItemText(hwndDlg, IDC_CONT_LABEL_SOUND, TranslateT("Not set"));
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p != NULL)
				{
					XSN_Users.remove(p);
					delete p;
				}
				db_unset(hContact, SETTINGSNAME, SETTINGSKEY);
				db_unset(hContact, SETTINGSNAME, SETTINGSIGNOREKEY);
			}
			break;

		case IDC_CONT_IGNORE_SOUND:
			{
				HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p == NULL)
				{
					DBVARIANT dbv;
					if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv)) {
						TCHAR longpath[MAX_PATH];
						PathToAbsoluteT(dbv.ptszVal, longpath);
						XSN_Users.insert( new XSN_Data(hContact, longpath, IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0));
						db_free(&dbv);
					} else
						XSN_Users.insert( new XSN_Data(hContact, _T(""), IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0));
				}
				else
					p->ignore = IsDlgButtonChecked(hwndDlg, IDC_CONT_IGNORE_SOUND) ? 1 : 0;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		Utils_SaveWindowPosition(hwndDlg, hContact, SETTINGSNAME, "ChangeSoundDlg");
		WindowList_Remove(hChangeSoundDlgList, hwndDlg);
	}
	return FALSE;
}

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam)
{
	HWND hChangeSoundDlg = WindowList_Find(hChangeSoundDlgList, (HANDLE)wParam);
	if (!hChangeSoundDlg) {
		hChangeSoundDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONTACTS), 0, DlgProcContactsOptions, (LPARAM)wParam);
		ShowWindow(hChangeSoundDlg, SW_SHOW);
	}
	else {
		SetForegroundWindow(hChangeSoundDlg);
		SetFocus(hChangeSoundDlg);
	}
	return 0;
}

int OnLoadInit(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF; 
	mi.flags = CMIF_TCHAR; 
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA); 
	mi.ptszName = LPGENT("Custom contact sound"); 
	mi.pszService = "XSoundNotify/ContactMenuCommand"; 
	hChangeSound = Menu_AddContactMenuItem(&mi); 
	return 0;
}

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (hContact) {
		char* szProto = GetContactProto(hContact);
		PROTOACCOUNT *pa = ProtoGetAccount(szProto);
		Menu_ShowItem(hChangeSound, IsSuitableProto(pa));
	}
	return 0;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hChangeSoundDlgList, WM_CLOSE, 0, 0);
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction("XSoundNotify/ContactMenuCommand", ShowDialog);

	hChangeSoundDlgList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, ProcessEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnLoadInit);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_SKIN_PLAYINGSOUND, OnPlaySound);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
