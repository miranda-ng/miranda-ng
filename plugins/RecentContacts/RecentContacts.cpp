
#include "RecentContacts.h"

extern int onOptInitialise(WPARAM wParam, LPARAM lParam);

using namespace std;
static const basic_string <char>::size_type npos = -1;

char *szProto;
HINSTANCE hInst = NULL;
PLUGINLINK *pluginLink = NULL;
int hLangpack = 0;

MM_INTERFACE mmi;
LIST_INTERFACE li;

HANDLE hTopToolbarButtonShowList;
HANDLE hMsgWndEvent;
HANDLE hWindowList;
HANDLE hMenuItemRemove;
HANDLE hIcon;

const INT_PTR boo = 0;
LIST<void> ServiceList(10,boo), HookList(10,boo);

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
	"Recent Contacts",
	PLUGIN_MAKE_VERSION(0,0,2,0),
	"Adds a menu item in main menu, which open the window with list of last used contacts names,"
		" sorted in order from most recent to least.",
	"ValeraVi, Kildor",
	"kostia@ngs.ru",
	"© 2005 ValeraVi, © 2009 Kildor",
	"http://kildor.miranda.im",
	UNICODE_AWARE,
	0,  	//doesn't replace anything built-in
	{ 0x0e5f3b9d, 0xebcd, 0x44d7, {0x93, 0x74, 0xd8, 0xe5, 0xd8, 0x8d, 0xf4, 0xe3}}
	/* 0e5f3b9d-ebcd-44d7-9374-d8e5d88df4e3 */
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

static const MUUID interfaces[] = {MIID_RECENTCONTACTS, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadDBSettings()
{
	ZeroMemory(&LastUCOpt, sizeof(LastUCOpt));
	LastUCOpt.MaxShownContacts = (INT)DBGetContactSettingByte( NULL, dbLastUC_ModuleName, dbLastUC_MaxShownContacts, 0 );
	LastUCOpt.HideOffline = DBGetContactSettingByte( NULL, dbLastUC_ModuleName, dbLastUC_HideOfflineContacts, 0 );

	DBVARIANT dbv;
	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = NULL;
	if ( db_get(NULL, dbLastUC_ModuleName, dbLastUC_DateTimeFormat, &dbv) == 0 && dbv.pszVal[0]!=0 ) {
		LastUCOpt.DateTimeFormat = dbv.pszVal;
		DBFreeVariant(&dbv);
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
		ZeroMemory(&lvi, sizeof(lvi));
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
		ZeroMemory(&lvi, sizeof(lvi));
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
				DlgDat->hContact = (HANDLE) lvi.lParam;
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
			ZeroMemory(DlgDat, sizeof(LASTUC_DLG_DATA));
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
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG)DlgDat);

			//set listview styles
			ListView_SetExtendedListViewStyleEx(hList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP
				| LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT, -1);

			// add header columns to listview
			LVCOLUMN lvc;
			ZeroMemory(&lvc, sizeof(lvc));
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
				DBFreeVariant(&dbv);
			}
			else strtimformat = dbLastUC_DateTimeFormatDefault;

			for(curContact = DlgDat->Contacts->begin(); curContact != DlgDat->Contacts->end(); curContact++) {
				if (curContact->second != NULL && db_byte_get(curContact->second, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0 ) {
					TCHAR *cname = ( TCHAR* )CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)curContact->second, GCDNF_TCHAR);
					if ( cname == NULL )
						continue;

					if (LastUCOpt.HideOffline == 1) {
						szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)curContact->second, 0);
						if (szProto != NULL && DBGetContactSettingWord((HANDLE)curContact->second, szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
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

			SAVEWINDOWPOS pos;
			pos.hContact = NULL;
			pos.hwnd = hDlg;
			pos.szModule = dbLastUC_ModuleName;
			pos.szNamePrefix = dbLastUC_WindowPosPrefix;
			CallService(MS_UTILS_RESTOREWINDOWPOSITION, 0, (LPARAM)(SAVEWINDOWPOS*)&pos);
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
						ZeroMemory(&lvh, sizeof(lvh));
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
	ZeroMemory(&dbe, sizeof(dbe));
	dbe.cbSize = sizeof(dbe);
	BYTE buf[1];
	dbe.pBlob = buf;
	HANDLE curEvent;
	HANDLE curContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	for (; curContact != NULL; curContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)curContact, 0))
	{
//		if (IsMessageAPI)
		{
			curTime = ((__time64_t)db_dword_get(curContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeLo, -1)) |
				(((__time64_t)db_dword_get(curContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeHi, -1)) << 32);
			//use TabSRMM last used time.  ! NOT used, because bug: TabSRMM reset last used time to time when miranda started at miranda start!
			//t = ((DWORD)db_dword_get(curContact, "Tab_SRMsg", "isRecent", -1));
			//if (t != -1)
			//{
			//	if (curTime == -1 || (__time64_t)t > curTime)
			//		curTime = (__time64_t)t;
			//}
		}
//		else
		{
			curEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)curContact, 0);
			if (curEvent != NULL)
			{
				for ( ; curEvent != NULL; curEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)curEvent, 0))
				{
					dbe.cbBlob = 1;
					if (CallService(MS_DB_EVENT_GET, (WPARAM)curEvent, (LPARAM)&dbe) != 0)
					{
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
		}
		if (curTime != -1)
			contacts->insert(cpair(curTime, curContact));
	}

	HWND hWndMain;
	if ((hWndMain = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_LASTUC_DIALOG), NULL, ShowListMainDlgProc, (LPARAM)contacts)) == NULL)
		return -1;

	ShowWindow(hWndMain, SW_SHOW);

	if (hTopToolbarButtonShowList != NULL)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButtonShowList, TTBST_RELEASED);
	return 0;
}

static int OnContactSettingChanged( WPARAM wParam, LPARAM lParam )
{
	HANDLE		hContact	= ( HANDLE )wParam;
	DBCONTACTWRITESETTING* pdbcws = ( DBCONTACTWRITESETTING* )lParam;

	if ( hContact == NULL )
		if ( !stricmp( pdbcws->szModule, dbLastUC_ModuleName))
			LoadDBSettings();

	return 0;
}

int Create_TopToolbarShowList(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_TTB_ADDBUTTON)) {
		TTBButton ttbb = { 0 };
		ttbb.cbSize = sizeof(ttbb);
		ttbb.hIconHandleDn = ttbb.hIconHandleDn = hIcon;
		ttbb.pszService = msLastUC_ShowList;
		ttbb.dwFlags = TTBBF_VISIBLE|TTBBF_SHOWTOOLTIP|TTBBF_DRAWBORDER;
		ttbb.name = Translate(msLastUC_ShowListName);

		hTopToolbarButtonShowList = (HANDLE)CallService(MS_TTB_ADDBUTTON, (WPARAM)&ttbb, 0);
		if ((int)hTopToolbarButtonShowList == -1)
			hTopToolbarButtonShowList = NULL;
	}

	return 0;
}

int Create_MenuitemShowList(void)
{
	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.hIcon = ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )"recent_main" );
	mi.pszName = msLastUC_ShowListName;
	mi.pszService = msLastUC_ShowList;
	Menu_AddMainMenuItem(&mi);

	ZeroMemory( &mi, sizeof( mi ));
	mi.cbSize = sizeof( mi );
	mi.position = 0xFFFFF;
	mi.hIcon = ( HICON )CallService( MS_SKIN2_GETICON, 0, ( LPARAM )"recent_main" );
	mi.ptszName = _T("Toggle Ignore");
	mi.pszService	= V_RECENTCONTACTS_TOGGLE_IGNORE;
	hMenuItemRemove = Menu_AddContactMenuItem(&mi);
	return 0;
}

BOOL SaveLastUsedTimeStamp(HANDLE hContact)
{
	__time64_t ct = _time64(NULL);
	db_dword_set(hContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeLo, (DWORD)ct);
	db_dword_set(hContact, dbLastUC_ModuleName, dbLastUC_LastUsedTimeHi, (DWORD)(ct >> 32));
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

static void iconsInit(void)
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_PATH_UNICODE;
	sid.pszSection = Translate(msLastUC_ShowListName);
	sid.ptszDefaultFile = szFile;

	sid.pszDescription = LPGEN("Main icon");
	sid.pszName = "recent_main";
	sid.iDefaultIndex = -IDI_SHOWRECENT;
	hIcon = Skin_AddIcon(&sid);
}

static int OnPrebuildContactMenu (WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( clmi );
	clmi.flags = CMIM_NAME | CMIF_TCHAR;

	if ( DBGetContactSettingByte((HANDLE)wParam, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0)
		clmi.ptszName = TranslateT("Ignore Contact");
	else
		clmi.ptszName = TranslateT("Show Contact");

	CallService( MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItemRemove, (LPARAM)&clmi );
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	iconsInit();
	Create_MenuitemShowList();
	IsMessageAPI = (CallService(MS_MSG_GETWINDOWAPI, 0, 0) != CALLSERVICE_NOTFOUND);
	//maxShownContacts = ;
	LoadDBSettings();

   if ( ServiceExists(MS_UPDATE_REGISTER)) {
      char szVersion[16];

      Update update = {0};
      update.cbSize = sizeof(Update);
      update.szComponentName = pluginInfo.shortName;
      update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
      update.cpbVersion = (int)strlen((char *)update.pbVersion);

      update.szUpdateURL = UPDATER_AUTOREGISTER;

      // these are the three lines that matter - the archive, the page containing the version string, and the text (or data)
      // before the version that we use to locate it on the page
      // (note that if the update URL and the version URL point to standard file listing entries, the backend xml
      // data will be used to check for updates rather than the actual web page - this is not true for beta urls)
      update.szBetaUpdateURL      = "http://kildor.miranda.im/miranda/recentcontacts.zip";
      update.szBetaVersionURL     = "http://kildor.miranda.im/miranda/recentcontacts.txt";
      update.szBetaChangelogURL   = "http://kildor.miranda.im/miranda/recentcontacts_changes.txt";
      update.pbBetaVersionPrefix  = (BYTE *)"RecentContacts ";
      update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

      CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
   }

	/*
  	if (ServiceExists(MS_FONT_REGISTERT)) {
		FontIDT fid={0};

		fid.cbSize=sizeof(fid);
		fid.group = _T("Console");
		fid.name,TranslateT = "Text";
		fid.dbSettingsGroup = "Console";
		fid.prefix = "ConsoleFont";

		fid.backgroundGroup = _T("Console");
		fid.backgroundName = _T("Background");

		fid.flags = FIDF_DEFAULTVALID;

		fid.deffontsettings.charset = DEFAULT_CHARSET;
		fid.deffontsettings.colour = RGB(0, 0, 0);
		fid.deffontsettings.size = 10;
		fid.deffontsettings.style = 0;
		_tcsncpy(fid.deffontsettings.szFace, _T("Tahoma"), LF_FACESIZE);

		CallService(MS_FONT_REGISTERT,(WPARAM)&fid,0);

		hHooks[i++] = HookEvent(ME_FONT_RELOAD,OnFontChange);
	}
	*/

	if (ServiceExists(MS_HOTKEY_REGISTER)) {
		// hotkeys
		HOTKEYDESC hotkey = {0};
		hotkey.cbSize = sizeof(hotkey);
		hotkey.pszName =    msLastUC_ShowList;
		hotkey.pszDescription = LPGEN("Show Recent Contacts");
		hotkey.pszSection = "Contacts";
		hotkey.pszService = msLastUC_ShowList;
		hotkey.DefHotKey = MAKEWORD('R', HOTKEYF_CONTROL | HOTKEYF_SHIFT);
		CallService(MS_HOTKEY_REGISTER, 0, (LPARAM)&hotkey);
	}

	return 0;
}

INT_PTR ToggleIgnore (WPARAM wParam, LPARAM lParam)
{
	if (wParam != NULL) {
		HANDLE hContact = ( HANDLE )wParam;
		int state = DBGetContactSettingByte(hContact, dbLastUC_ModuleName, dbLastUC_IgnoreContact, 0) == 0 ? 1 : 0 ;
		DBWriteContactSettingByte(hContact, dbLastUC_ModuleName, dbLastUC_IgnoreContact, state);
		return state;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getLP( &pluginInfo );
	mir_getMMI( &mmi );
	mir_getLI( &li );

	CoInitialize(NULL);
	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);

	ServiceList.insert( CreateServiceFunction(msLastUC_ShowList, OnMenuCommandShowList));
	ServiceList.insert( CreateServiceFunction(V_RECENTCONTACTS_TOGGLE_IGNORE, ToggleIgnore));
	
	HookList.insert( HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded));
	HookList.insert( HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu ));
	HookList.insert( HookEvent(ME_TTB_MODULELOADED, Create_TopToolbarShowList));
	HookList.insert( HookEvent(ME_MSG_WINDOWEVENT, OnMsgEvent));
	HookList.insert( HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged ));
	HookList.insert( HookEvent(ME_OPT_INITIALISE, onOptInitialise));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) int Unload(void)
{
	int i;

	for (i=0; i < ServiceList.getCount(); ++i)
		DestroyServiceFunction( ServiceList[i] );

	for (i=0; i < HookList.getCount(); ++i)
		UnhookEvent( HookList[i] );

	CoUninitialize();
	return 0;
}
