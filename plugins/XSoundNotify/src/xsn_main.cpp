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
LIST<XSN_Data> XSN_Users(10, LIST<XSN_Data>::FTSortFunc(HandleKeySort));

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

bool isReceiveMessage(LPARAM event)
{
	DBEVENTINFO info = { sizeof(info) };	
	CallService(MS_DB_EVENT_GET, event, (LPARAM)&info);
	// i don't understand why it works and how it works, but it works correctly - practice way (методом тыка)
	// so, i think correct condition would be : eventType == EVENTTYPE_MESSAGE && info.flags & DBEF_READ, but it really isn't
	return !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

INT ProcessEvent(WPARAM wParam, LPARAM lParam)
{
	if (!isReceiveMessage(lParam))
		return 0;
						
	DBVARIANT dbv;
	if ( !db_get_ts((HANDLE)wParam, SETTINGSNAME, SETTINGSKEY, &dbv))
	{
		TCHAR longpath[MAX_PATH] = {0};
		PathToAbsoluteT(dbv.ptszVal, longpath);
		SkinPlaySoundFile(longpath);
		db_free(&dbv);
	}

	return 0;
}

bool IsSuitableProto(PROTOACCOUNT* pa)
{
	if (pa == NULL)
		return false;

	if (pa->bDynDisabled || !pa->bIsEnabled)
		return false;

	if (CallProtoService(pa->szProtoName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
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
		for(int i = 0; i < count; i++) {
			if (IsSuitableProto(protos[i]))
				SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_PROTO), CB_SETITEMDATA, SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_PROTO), CB_ADDSTRING, 0, (LPARAM)protos[i]->tszAccountName), (LPARAM)protos[i]);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COMBO_PROTO:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE)) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_USERS), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_TEST_PLAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_RESET_SOUND), FALSE);
				SetDlgItemText(hwndDlg, IDC_LABEL_SOUND, TranslateT("Not set"));
				SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_RESETCONTENT, 0, 0);
				int cursel = SendDlgItemMessage(hwndDlg, IDC_COMBO_PROTO, CB_GETCURSEL, 0, 0);
				PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(hwndDlg, IDC_COMBO_PROTO, CB_GETITEMDATA, cursel, 0);
				HANDLE hContact = db_find_first();
				while (hContact != NULL) {
					char* szUniqueId = (char*)CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
					if ((INT_PTR) szUniqueId != CALLSERVICE_NOTFOUND && szUniqueId != NULL) {
						DBVARIANT dbvuid = {0};
						if ( !db_get(hContact, pa->szModuleName, szUniqueId, &dbvuid))
						{
							TCHAR uid[MAX_PATH];
							switch(dbvuid.type) {
							case DBVT_DWORD:
								_itot(dbvuid.dVal, uid, 10);
								break;

							case DBVT_ASCIIZ:
								_tcscpy(uid, _A2T(dbvuid.pszVal));
								break;

							case DBVT_UTF8:
								{
									TCHAR *tmpuid = mir_utf8decodeT(dbvuid.pszVal);
									_tcscpy(uid, tmpuid);
									mir_free(tmpuid);
								}
								break;
							}

							TCHAR *nick = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
							TCHAR *value = (TCHAR *)mir_alloc(sizeof(TCHAR) * (_tcslen(uid) + _tcslen(nick) + 4));
							mir_sntprintf(value, _tcslen(uid) + _tcslen(nick) + 4, _T("%s (%s)"), nick, uid);
							SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_USERS), CB_SETITEMDATA, SendMessage(GetDlgItem(hwndDlg, IDC_COMBO_USERS), CB_ADDSTRING, 0, (LPARAM)value), (LPARAM)hContact);
							mir_free(value);
							db_free(&dbvuid);
						}
					}
					hContact = db_find_next(hContact);
				}

			}
			return 0;

		case IDC_COMBO_USERS:
			if ((HIWORD(wParam) == CBN_SELCHANGE) || (HIWORD(wParam) == LBN_SELCHANGE)) {
				int cursel = SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETCURSEL, 0, 0);
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_CHOOSE_SOUND), TRUE);
				DBVARIANT dbv = {0};
				if ( !db_get_ts(hContact, SETTINGSNAME, SETTINGSKEY, &dbv))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_RESET_SOUND), TRUE);
					SetDlgItemText(hwndDlg, IDC_LABEL_SOUND, PathFindFileName(dbv.ptszVal));
					db_free(&dbv);
				} else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_TEST_PLAY), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_RESET_SOUND), FALSE);
					SetDlgItemText(hwndDlg, IDC_LABEL_SOUND, TranslateT("Not set"));
				}
			}
			return 0;

		case IDC_BUTTON_CHOOSE_SOUND:
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
					SetDlgItemText(hwndDlg, IDC_LABEL_SOUND, PathFindFileName(FileName));
					int cursel = SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETCURSEL, 0, 0);
					HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
					XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
					if (p == NULL)
						XSN_Users.insert( new XSN_Data(hContact, FileName));
					else
						_tcsncpy(p->path, FileName, SIZEOF(p->path));
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_TEST_PLAY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_RESET_SOUND), TRUE);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				mir_free(tszMirDir);
			}
			return 0;

		case IDC_BUTTON_TEST_PLAY:
			{
				int cursel = SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETCURSEL, 0, 0);
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
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
			return 0;

		case IDC_BUTTON_RESET_SOUND:
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_TEST_PLAY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_RESET_SOUND), FALSE);
				SetDlgItemText(hwndDlg, IDC_LABEL_SOUND, TranslateT("Not set"));
				int cursel = SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETCURSEL, 0, 0);
				HANDLE hContact = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_COMBO_USERS, CB_GETITEMDATA, cursel, 0);
				XSN_Data *p = XSN_Users.find((XSN_Data *)&hContact);
				if (p != NULL)
				{
					XSN_Users.remove(p);
					delete p;
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				db_unset(hContact, SETTINGSNAME, SETTINGSKEY);
			}
			return 0;
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR *)lParam;
			switch (hdr->code) {
			case PSN_APPLY:
				{
					for (int i = 0; i < XSN_Users.getCount(); i++)
					{
						TCHAR shortpath[MAX_PATH];
						PathToRelativeT(XSN_Users[i]->path, shortpath);
						db_set_ts(XSN_Users[i]->hContact, SETTINGSNAME, SETTINGSKEY, shortpath);
					}
					break;
				}
			}
		}
	}//end* switch (msg)
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

INT_PTR ShowDialog(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int OnLoadInit(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi); 
	mi.position = -0x7FFFFFFF; 
	mi.flags = CMIF_TCHAR; 
	mi.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA); 
	mi.ptszName = LPGENT("Custom contact sound"); 
	mi.pszService = "XSoundNotify/ContactMenuCommand"; 
	Menu_AddContactMenuItem(&mi); 

	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction("XSoundNotify/ContactMenuCommand", ShowDialog); 

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_DB_EVENT_ADDED, ProcessEvent);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnLoadInit);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

