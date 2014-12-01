
#include "RecentContacts.h"

extern int onOptInitialise(WPARAM wParam, LPARAM lParam);

using namespace std;
static const basic_string <char>::size_type npos = -1;

char *szProto;
HINSTANCE hInst = NULL;
int hLangpack = 0;

CLIST_INTERFACE *pcli;
HANDLE hTopToolbarButtonShowList;
HANDLE hMsgWndEvent;
HANDLE hWindowList;
HGENMENU hMenuItemRemove;

const INT_PTR boo = 0;

BOOL IsMessageAPI = FALSE;

LastUCOptions LastUCOpt = {0};

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {0E5F3B9D-EBCD-44D7-9374-D8E5D88DF4E3}
	{0x0e5f3b9d, 0xebcd, 0x44d7, {0x93, 0x74, 0xd8, 0xe5, 0xd8, 0x8d, 0xf4, 0xe3}}
};

static IconItem icon = { LPGEN("Main icon"), "recent_main", IDI_SHOWRECENT };

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadDBSettings()
{
	memset(&LastUCOpt, 0, sizeof(LastUCOpt));
	LastUCOpt.MaxShownContacts = (INT)db_get_b( NULL, dbLastUC_ModuleName, dbLastUC_MaxShownContacts, 0 );
	LastUCOpt.HideOffline = db_get_b( NULL, dbLastUC_ModuleName, dbLastUC_HideOfflineContacts, 0 );
	LastUCOpt.WindowAutoSize = db_get_b( NULL, dbLastUC_ModuleName, dbLastUC_WindowAutosize, 0 );

	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = NULL;
	if ( db_get(NULL, dbLastUC_ModuleName, dbLastUC_DateTimeFormat, &dbv) == 0 && dbv.pszVal[0]!=0 ) {
		LastUCOpt.DateTimeFormat = dbv.pszVal;
		db_free(&dbv);
	}
	else LastUCOpt.DateTimeFormat = "(%Y-%m-%d %H:%M)  ";
}

void ShowListMainDlgProc_AdjustListPos(HWND hDlg, LASTUC_DLG_DATA *DlgDat)
{
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_LIST);
	if (hList == NULL)
		return;

	RECT rc;
	SIZE cur;
	GetWindowRect(hDlg, &rc);
	cur.cx = rc.right - rc.left;
	cur.cy = rc.bottom - rc.top;
	rc.left = DlgDat->ListUCRect.left;
	rc.top = DlgDat->ListUCRect.top;
	rc.right = DlgDat->ListUCRect.right + cur.cx - DlgDat->WindowMinSize.cx;
	rc.bottom = DlgDat->ListUCRect.bottom + cur.cy - DlgDat->WindowMinSize.cy;
	MoveWindow(hList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_WIDTH;
	lvc.cx = rc.right - rc.left - GetSystemMetrics(SM_CYHSCROLL) - 4;
	if (lvc.cx < 10)
		lvc.cx = 10;
	ListView_SetColumn(hList, 0, &lvc);
}

BOOL ShowListMainDlgProc_OpenContact(HWND hList, int item)
{
	if (item != -1) {
		LVITEM lvi;
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask = LVIF_PARAM;
		lvi.lParam = NULL;
		lvi.iItem = item;
		lvi.iSubItem = 0;
		ListView_GetItem(hList, &lvi);
		if (lvi.lParam != NULL) {
			CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)lvi.lParam, NULL);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ShowListMainDlgProc_OpenContactMenu(HWND hDlg, HWND hList, int item, LASTUC_DLG_DATA *DlgDat)
{
	if (item != -1) {
		LVITEM lvi;
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask = LVIF_PARAM;
		lvi.lParam = NULL;
		lvi.iItem = item;
		lvi.iSubItem = 0;
		ListView_GetItem(hList, &lvi);
		if (lvi.lParam != NULL) {
			HMENU hCMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)lvi.lParam, NULL);
			if (hCMenu != NULL) {
				POINT p;
				GetCursorPos(&p);
				DlgDat->hContact = (MCONTACT) lvi.lParam;
				BOOL ret = TrackPopupMenu(hCMenu, 0, p.x, p.y, 0, hDlg, NULL);
				DestroyMenu(hCMenu);
				if (ret)
					return TRUE;
				DlgDat->hContact = NULL;
			}
		}
	}
	return FALSE;
}

void wSetData(char **Data, const char *Value)
{
	if (Value[0] != 0) {
		char *newData = (char*)mir_alloc(strlen(Value)+3);
		strcpy(newData, Value);
		*Data = newData;
	}
	else *Data = "";
}

void wfree(char **Data)
{
	if (*Data && strlen(*Data) > 0)	mir_free(*Data);
	*Data = NULL;
}

HWND hwndContactTree = NULL;

INT_PTR CALLBACK ShowListMainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LASTUC_DLG_DATA *DlgDat;

	DlgDat = (LASTUC_DLG_DATA *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_LIST);
	if (hList == NULL)
		return FALSE;

	switch( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			DlgDat = new LASTUC_DLG_DATA;
			memset(DlgDat, 0, sizeof(LASTUC_DLG_DATA));
			DlgDat->Contacts = (cmultimap*)lParam;

			RECT rc;
			GetWindowRect(hDlg, &rc);
			DlgDat->WindowMinSize.cx = rc.right - rc.left;
			DlgDat->WindowMinSize.cy = rc.bottom - rc.top;
			GetWindowRect(hList, &DlgDat->ListUCRect);

			POINT p;
			p.x = DlgDat->ListUCRect.left;
			p.y = DlgDat->ListUCRect.top;
			ScreenToClient(hDlg, &p);
			DlgDat->ListUCRect.left = p.x;
			DlgDat->ListUCRect.top = p.y;
			p.x = DlgDat->ListUCRect.right;
			p.y = DlgDat->ListUCRect.bottom;
			ScreenToClient(hDlg, &p);
			DlgDat->ListUCRect.right = p.x;
			DlgDat->ListUCRect.bottom = p.y;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)DlgDat);

			//set listview styles
			ListView_SetExtendedListViewStyleEx(hList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP
				| LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT, -1);

			// add header columns to listview
			LVCOLUMN lvc;
			memset(&lvc, 0, sizeof(lvc));
			lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
			lvc.fmt = LVCFMT_LEFT;
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Contact");
			lvc.cx = 10;
			ListView_InsertColumn(hList, 0, &lvc);

			// add conacts to listview
			HIMAGELIST hImgList = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
			if (hImgList != NULL)
				ListView_SetImageList(hList, hImgList, LVSIL_SMALL);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

			int i=0;
			cmultimap::iterator curContact;
			std::tstring str;
			char strtim[256 + 16];

			string strtimformat;
			DBVARIANT dbv;
			dbv.type = DBVT_ASCIIZ;
			dbv.pszVal = NULL;
			if (db_get(NULL, dbLastUC_ModuleName, dbLastUC_DateTimeFormat, &dbv) == 0) {
				strtimformat = dbv.pszVal;
				db_free(&dbv);
			}
			else strtimformat = dbLastUC_DateTimeFormatDefault;

			for(curContact = DlgDat->Contacts->begin(); curContact != DlgDat->Contacts->end(); curContact++) {
				if (curContact->second != NULL && db_get_b(curContact->second, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0 ) {
					TCHAR *cname = ( TCHAR* )CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)curContact->second, GCDNF_TCHAR);
					if ( cname == NULL )
						continue;

					if (LastUCOpt.HideOffline == 1) {
						szProto = GetContactProto(curContact->second);
						if (szProto != NULL && db_get_w(curContact->second, szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
							continue;
					}

					lvi.iItem = i;
					lvi.iSubItem = 0;
					lvi.lParam = (LPARAM)curContact->second;
							
					strftime(strtim, 256, strtimformat.c_str(), _localtime64(&curContact->first));
					strtim[255] = 0;
					str = _A2T(strtim);
					str += cname;
					lvi.pszText = (LPTSTR)str.c_str();
					lvi.iImage = CallService(MS_CLIST_GETCONTACTICON, (WPARAM)curContact->second, 0);
					ListView_InsertItem(hList, &lvi);
					i++;

				}
				if (LastUCOpt.MaxShownContacts > 0 && i >= LastUCOpt.MaxShownContacts)
				break;
			}


			// window autosize/autopos - ike blaster
			bool restorePos = !LastUCOpt.WindowAutoSize;

			if (!restorePos) {
				RECT rect;
				if (GetWindowRect(pcli->hwndContactList, &rect)) {
					WINDOWPLACEMENT wp;

					wp.length = sizeof(wp);
					GetWindowPlacement(hDlg, &wp);

					char szSettingName[64];
					mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", dbLastUC_WindowPosPrefix);
					int width = db_get_dw(NULL, dbLastUC_ModuleName, szSettingName, -1);

					int right = rect.left - 6;
					if(!IsWindowVisible(pcli->hwndContactList)) right = rect.right;

					wp.rcNormalPosition.left = right - width;
					wp.rcNormalPosition.top = rect.top;
					wp.rcNormalPosition.right = right;
					wp.rcNormalPosition.bottom = rect.bottom;
					wp.flags = 0;

					SetWindowPlacement(hDlg, &wp);
				} else {
					restorePos = true;
				}
			}

			if (restorePos) {
				SAVEWINDOWPOS pos;
				pos.hContact = NULL;
				pos.hwnd = hDlg;
				pos.szModule = dbLastUC_ModuleName;
				pos.szNamePrefix = dbLastUC_WindowPosPrefix;

				CallService(MS_UTILS_RESTOREWINDOWPOSITION, 0, (LPARAM)(SAVEWINDOWPOS*)&pos);
			}

			SendMessage(hDlg, WM_SIZE, 0, 0);
			WindowList_Add(hWindowList, hDlg, NULL);
			return TRUE;
		}

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpNmhdr;
			lpNmhdr = (LPNMHDR)lParam;
			if (lpNmhdr->hwndFrom == hList) {
				if (lpNmhdr->code == NM_CLICK || lpNmhdr->code == NM_RCLICK) {
					RECT r;
					POINT p;
					GetCursorPos(&p);
					GetWindowRect(hList, &r);
					if (PtInRect(&r, p)) {
						LVHITTESTINFO lvh;
						memset(&lvh, 0, sizeof(lvh));
						lvh.pt = p;
						ScreenToClient(hList, &lvh.pt);
						ListView_HitTest(hList, &lvh);
						if ((lvh.flags & (LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON)) && lvh.iItem != -1) {
							if (lpNmhdr->code == NM_CLICK) {
								if (ShowListMainDlgProc_OpenContact(hList, lvh.iItem))
									SendMessage(hDlg, WM_CLOSE, 0, 0);
							}
							else ShowListMainDlgProc_OpenContactMenu(hDlg, hList, lvh.iItem, DlgDat);
						}
					}
				}
				else if (lpNmhdr->code == NM_RETURN) {
					if (ShowListMainDlgProc_OpenContact(hList, ListView_GetNextItem(hList, -1, LVIS_SELECTED)))
						SendMessage(hDlg, WM_CLOSE, 0, 0);
				}
			}
			break;
		}

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)DlgDat->hContact))
			break;

		switch(wParam) {
		case IDOK:
			ShowListMainDlgProc_OpenContact(hList, ListView_GetNextItem(hList, -1, LVIS_SELECTED));
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;
			mmi->ptMinTrackSize.x = 100;
			mmi->ptMinTrackSize.y = 150;
			return 0;
		}

	case WM_SIZE:
		ShowListMainDlgProc_AdjustListPos(hDlg, DlgDat);
		break;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_DESTROY:
		{
			// Save current window position.
			SAVEWINDOWPOS pos;
			pos.hContact = NULL;
			pos.hwnd = hDlg;
			pos.szModule = dbLastUC_ModuleName;
			pos.szNamePrefix = dbLastUC_WindowPosPrefix;
			CallService(MS_UTILS_SAVEWINDOWPOSITION, 0, (LPARAM)(SAVEWINDOWPOS*)&pos);
			delete DlgDat->Contacts;
			delete DlgDat;
			// Remove entry from Window list
			WindowList_Remove(hWindowList, hDlg);
			break;
		}
	}
	return FALSE;
}

INT_PTR OnMenuCommandShowList(WPARAM wParam, LPARAM lParam)
{
	cmultimap *contacts = new cmultimap;

	__time64_t curTime;
	//DWORD t;
	DBEVENTINFO dbe;
	memset(&dbe, 0, sizeof(dbe));
	dbe.cbSize = sizeof(dbe);
	BYTE buf[1];
	dbe.pBlob = buf;
	HANDLE curEvent;
	
	for (MCONTACT curContact = db_find_first(); curContact != NULL; curContact = db_find_next(curContact)) {
		curTime = ((__time64_t)db_get_dw(curContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeLo, -1)) |
					(((__time64_t)db_get_dw(curContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeHi, -1)) << 32);

		curEvent = db_event_last(curContact);
		if (curEvent != NULL) {
			for ( ; curEvent != NULL; curEvent = db_event_prev(curContact, curEvent)) {
				dbe.cbBlob = 1;
				if (db_event_get(curEvent, &dbe) != 0) {
					curEvent = NULL;
					break;
				}
				if ((dbe.flags & (DBEF_READ | DBEF_SENT)) && dbe.eventType < 2000)
					break;
			}
			if (curEvent != NULL)
				if (curTime == -1 || (__time64_t)dbe.timestamp > curTime)
					curTime = (__time64_t)dbe.timestamp;
		}

		if (curTime != -1)
			contacts->insert(cpair(curTime, curContact));
	}

	HWND hWndMain;
	if ((hWndMain = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LASTUC_DIALOG), NULL, ShowListMainDlgProc, (LPARAM)contacts)) == NULL)
		return -1;

	ShowWindow(hWndMain, SW_SHOW);

	if (hTopToolbarButtonShowList != NULL)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButtonShowList, 0);
	return 0;
}

static int OnContactSettingChanged( WPARAM hContact, LPARAM lParam )
{
	DBCONTACTWRITESETTING* pdbcws = ( DBCONTACTWRITESETTING* )lParam;
	if ( hContact == NULL )
		if ( !stricmp( pdbcws->szModule, dbLastUC_ModuleName))
			LoadDBSettings();

	return 0;
}

int Create_TopToolbarShowList(WPARAM wParam, LPARAM lParam)
{
	TTBButton ttb = { sizeof(ttb) };
	ttb.hIconHandleUp = icon.hIcolib;
	ttb.pszService = msLastUC_ShowList;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.name = ttb.pszTooltipUp = msLastUC_ShowListName;
	hTopToolbarButtonShowList = TopToolbar_AddButton(&ttb);
	return 0;
}

int Create_MenuitemShowList(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = icon.hIcolib;
	mi.pszName = msLastUC_ShowListName;
	mi.pszService = msLastUC_ShowList;
	Menu_AddMainMenuItem(&mi);

	mi.position = 0xFFFFF;
	mi.icolibItem = icon.hIcolib;
	mi.pszName = LPGEN("Toggle Ignore");
	mi.pszService = V_RECENTCONTACTS_TOGGLE_IGNORE;
	hMenuItemRemove = Menu_AddContactMenuItem(&mi);
	return 0;
}

BOOL SaveLastUsedTimeStamp(MCONTACT hContact)
{
	__time64_t ct = _time64(NULL);
	db_set_dw(hContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeLo, (DWORD)ct);
	db_set_dw(hContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeHi, (DWORD)(ct >> 32));
	return TRUE;
}

int OnMsgEvent(WPARAM wParam, LPARAM lParam)
{
	MessageWindowEventData *ed = (MessageWindowEventData *)lParam;
	if (ed->hContact == NULL)
		return 0;

	if (ed->uType == MSG_WINDOW_EVT_OPEN)
		SaveLastUsedTimeStamp(ed->hContact);
	else if (ed->uType == MSG_WINDOW_EVT_CUSTOM) {
		struct TABSRMM_SessionInfo *si = (struct TABSRMM_SessionInfo*) ed->local;
		if (si != NULL)
			if (si->evtCode == tabMSG_WINDOW_EVT_CUSTOM_BEFORESEND)
				SaveLastUsedTimeStamp(ed->hContact);
	}
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	CLISTMENUITEM clmi = { sizeof(clmi) };
	clmi.flags = CMIM_NAME | CMIF_TCHAR;

	if (db_get_b(hContact, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0)
		clmi.ptszName = TranslateT("Ignore Contact");
	else
		clmi.ptszName = TranslateT("Show Contact");

	Menu_ModifyItem(hMenuItemRemove, &clmi);
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	HookEvent(ME_TTB_MODULELOADED, Create_TopToolbarShowList);

	Create_MenuitemShowList();
	IsMessageAPI = (CallService(MS_MSG_GETWINDOWAPI, 0, 0) != CALLSERVICE_NOTFOUND);
	LoadDBSettings();

	// hotkeys
	HOTKEYDESC hk = { sizeof(hk) };
	hk.pszName = msLastUC_ShowList;
	hk.pszDescription = LPGEN("Show Recent Contacts");
	hk.pszSection = "Contacts";
	hk.pszService = msLastUC_ShowList;
	hk.DefHotKey = MAKEWORD('R', HOTKEYF_CONTROL | HOTKEYF_SHIFT);
	Hotkey_Register(&hk);
	return 0;
}

INT_PTR ToggleIgnore (WPARAM hContact, LPARAM lParam)
{
	if (hContact != NULL) {
		int state = db_get_b(hContact, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0 ? 1 : 0 ;
		db_set_b(hContact, dbLastUC_ModuleName, dbLastUC_IgnoreContact, state);
		return state;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP( &pluginInfo );
	mir_getCLI();

	CoInitialize(NULL);
	hWindowList = WindowList_Create();

	Icon_Register(hInst, msLastUC_ShowListName, &icon, 1);

	CreateServiceFunction(msLastUC_ShowList, OnMenuCommandShowList);
	CreateServiceFunction(V_RECENTCONTACTS_TOGGLE_IGNORE, ToggleIgnore);
	
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	HookEvent(ME_MSG_WINDOWEVENT, OnMsgEvent);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged );
	HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hWindowList);
	CoUninitialize();
	return 0;
}
