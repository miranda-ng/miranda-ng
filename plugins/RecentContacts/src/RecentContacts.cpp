
#include "stdafx.h"

extern int onOptInitialise(WPARAM wParam, LPARAM lParam);

using namespace std;
static const basic_string <char>::size_type npos = -1;

char *szProto;
CMPlugin g_plugin;

HANDLE hTopToolbarButtonShowList;
HANDLE hMsgWndEvent;
HGENMENU hMenuItemRemove;

const INT_PTR boo = 0;

LastUCOptions LastUCOpt = {0};

static IconItem iconList[] =
{
	{ LPGEN("Main icon"), "recent_main", IDI_SHOWRECENT }
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {0E5F3B9D-EBCD-44D7-9374-D8E5D88DF4E3}
	{0x0e5f3b9d, 0xebcd, 0x44d7, {0x93, 0x74, 0xd8, 0xe5, 0xd8, 0x8d, 0xf4, 0xe3}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

void LoadDBSettings()
{
	memset(&LastUCOpt, 0, sizeof(LastUCOpt));
	LastUCOpt.MaxShownContacts = (INT)g_plugin.getByte(dbLastUC_MaxShownContacts, 0);
	LastUCOpt.HideOffline = g_plugin.getByte(dbLastUC_HideOfflineContacts, 0);
	LastUCOpt.WindowAutoSize = g_plugin.getByte(dbLastUC_WindowAutosize, 0);

	ptrA szFormat(g_plugin.getStringA(dbLastUC_DateTimeFormat));
	if (szFormat)
		LastUCOpt.DateTimeFormat = szFormat;
	else
		LastUCOpt.DateTimeFormat = "(%Y-%m-%d %H:%M)  ";
}

void ShowListMainDlgProc_AdjustListPos(HWND hDlg, LASTUC_DLG_DATA *DlgDat)
{
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_LIST);
	if (hList == nullptr)
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
			Clist_ContactDoubleClicked(lvi.lParam);
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
			HMENU hCMenu = Menu_BuildContactMenu(lvi.lParam);
			if (hCMenu != nullptr) {
				POINT p;
				GetCursorPos(&p);
				DlgDat->hContact = (MCONTACT)lvi.lParam;
				BOOL ret = TrackPopupMenu(hCMenu, 0, p.x, p.y, 0, hDlg, nullptr);
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
		char *newData = (char*)mir_alloc(mir_strlen(Value) + 3);
		mir_strcpy(newData, Value);
		*Data = newData;
	}
	else *Data = "";
}

void wfree(char **Data)
{
	if (*Data && mir_strlen(*Data) > 0)	mir_free(*Data);
	*Data = nullptr;
}

HWND hwndContactTree = nullptr;

INT_PTR CALLBACK ShowListMainDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LASTUC_DLG_DATA *DlgDat = (LASTUC_DLG_DATA *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	HWND hList = GetDlgItem(hDlg, IDC_CONTACTS_LIST);
	if (hList == nullptr)
		return FALSE;

	switch (msg) {
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

			// set listview styles
			ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_ONECLICKACTIVATE | LVS_EX_UNDERLINEHOT, -1);

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
			HIMAGELIST hImgList = Clist_GetImageList();
			if (hImgList != nullptr)
				ListView_SetImageList(hList, hImgList, LVSIL_SMALL);

			LVITEM lvi = { 0 };
			lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

			int i = 0;
			std::wstring str;
			char strtim[256 + 16];

			ptrA szFormat(g_plugin.getStringA(dbLastUC_DateTimeFormat));
			if (!szFormat)
				szFormat = mir_strdup(dbLastUC_DateTimeFormatDefault);

			for (auto curContact = DlgDat->Contacts->begin(); curContact != DlgDat->Contacts->end(); curContact++) {
				if (curContact->second != NULL && g_plugin.getByte(curContact->second, dbLastUC_IgnoreContact) == 0) {
					wchar_t *cname = Clist_GetContactDisplayName(curContact->second);
					if (cname == nullptr)
						continue;

					if (LastUCOpt.HideOffline == 1) {
						szProto = Proto_GetBaseAccountName(curContact->second);
						if (szProto != nullptr && db_get_w(curContact->second, szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
							continue;
					}

					lvi.iItem = i;
					lvi.iSubItem = 0;
					lvi.lParam = (LPARAM)curContact->second;

					strftime(strtim, 256, szFormat, _localtime64(&curContact->first));
					strtim[255] = 0;
					str = _A2T(strtim);
					str += cname;
					lvi.pszText = (LPTSTR)str.c_str();
					lvi.iImage = Clist_GetContactIcon(curContact->second);
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
				if (GetWindowRect(g_clistApi.hwndContactList, &rect)) {
					WINDOWPLACEMENT wp;

					wp.length = sizeof(wp);
					GetWindowPlacement(hDlg, &wp);

					char szSettingName[64];
					mir_snprintf(szSettingName, "%swidth", dbLastUC_WindowPosPrefix);
					int width = g_plugin.getDword(szSettingName, -1);

					int right = rect.left - 6;
					if (!IsWindowVisible(g_clistApi.hwndContactList)) right = rect.right;

					wp.rcNormalPosition.left = right - width;
					wp.rcNormalPosition.top = rect.top;
					wp.rcNormalPosition.right = right;
					wp.rcNormalPosition.bottom = rect.bottom;
					wp.flags = 0;

					SetWindowPlacement(hDlg, &wp);
				}
				else restorePos = true;
			}

			if (restorePos)
				Utils_RestoreWindowPosition(hDlg, NULL, MODULENAME, dbLastUC_WindowPosPrefix);

			SendMessage(hDlg, WM_SIZE, 0, 0);
			return TRUE;
		}

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
			SendMessage(hDlg, WM_CLOSE, 0, 0);
		break;

	case WM_NOTIFY:
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

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);

	case WM_COMMAND:
		if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, DlgDat->hContact))
			break;

		switch (wParam) {
		case IDOK:
			ShowListMainDlgProc_OpenContact(hList, ListView_GetNextItem(hList, -1, LVIS_SELECTED));
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
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
		Utils_SaveWindowPosition(hDlg, NULL, MODULENAME, dbLastUC_WindowPosPrefix);

		delete DlgDat->Contacts;
		delete DlgDat;
		break;
	}
	return FALSE;
}

static __time64_t GetLastUsedTimeStamp(MCONTACT hContact)
{
	return
		((__time64_t)g_plugin.getDword(hContact, dbLastUC_LastUsedTimeLo, -1)) |
		(((__time64_t)g_plugin.getDword(hContact, dbLastUC_LastUsedTimeHi, -1)) << 32);
}

INT_PTR OnMenuCommandShowList(WPARAM, LPARAM)
{
	cmultimap *contacts = new cmultimap;

	DBEVENTINFO dbe = {};
	uint8_t buf[1];
	dbe.pBlob = buf;

	for (auto &curContact : Contacts()) {
		__time64_t curTime = GetLastUsedTimeStamp(curContact);

		MEVENT curEvent = db_event_last(curContact);
		if (curEvent != NULL) {
			for (; curEvent != NULL; curEvent = db_event_prev(curContact, curEvent)) {
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

	HWND hWndMain = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_LASTUC_DIALOG), nullptr, ShowListMainDlgProc, (LPARAM)contacts);
	if (hWndMain == nullptr)
		return -1;

	ShowWindow(hWndMain, SW_SHOW);

	if (hTopToolbarButtonShowList != nullptr)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTopToolbarButtonShowList, 0);
	return 0;
}

static int OnContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *pdbcws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL)
		if (!strcmp(pdbcws->szModule, MODULENAME))
			LoadDBSettings();

	return 0;
}

int Create_TopToolbarShowList(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.hIconHandleUp = iconList[0].hIcolib;
	ttb.pszService = msLastUC_ShowList;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.name = ttb.pszTooltipUp = LPGEN("Recent Contacts");
	hTopToolbarButtonShowList = g_plugin.addTTB(&ttb);
	return 0;
}

int Create_MenuitemShowList(void)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xe22ce213, 0x362a, 0x444a, 0xa5, 0x82, 0xc, 0xcf, 0xf5, 0x4b, 0xd1, 0x8e);
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.a = LPGEN("Recent Contacts");
	mi.pszService = msLastUC_ShowList;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xe22ce213, 0x362a, 0x444a, 0xa5, 0x82, 0xc, 0xcf, 0xf5, 0x4b, 0xd1, 0x8e);
	mi.position = 0xFFFFF;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.a = LPGEN("Toggle Ignore");
	mi.pszService = V_RECENTCONTACTS_TOGGLE_IGNORE;
	hMenuItemRemove = Menu_AddContactMenuItem(&mi);
	return 0;
}

static void SaveLastUsedTimeStamp(MCONTACT hContact)
{
	__time64_t ct = _time64(nullptr);
	g_plugin.setDword(hContact, dbLastUC_LastUsedTimeLo, (uint32_t)ct);
	g_plugin.setDword(hContact, dbLastUC_LastUsedTimeHi, (uint32_t)(ct >> 32));
}

static int OnGCInEvent(WPARAM, LPARAM lParam)
{
	GCEVENT *gce = (GCEVENT*)lParam;
	if (gce->iType == GC_EVENT_MESSAGE) {
		SESSION_INFO *si = g_chatApi.SM_FindSession(gce->pszID.w, gce->pszModule);
		if (si && si->hContact) {
			// skip old events
			if (gce->time && gce->time <= GetLastUsedTimeStamp(si->hContact))
				return 0;
			SaveLastUsedTimeStamp(si->hContact);
		}
	}
	return 0;
}

static int OnGCOutEvent(WPARAM, LPARAM lParam)
{
	GCEVENT *gce = (GCEVENT*)lParam;
	if (gce->iType == GC_USER_MESSAGE) {
		SESSION_INFO *si = g_chatApi.SM_FindSession(gce->pszID.w, gce->pszModule);
		if (si && si->hContact)
			SaveLastUsedTimeStamp(si->hContact);
	}
	return 0;
}

static int OnMsgEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *ed = (MessageWindowEventData *)lParam;
	if (ed->hContact != 0 && ed->uType == MSG_WINDOW_EVT_OPEN)
		SaveLastUsedTimeStamp(ed->hContact);
	return 0;
}

static int OnProtoBroadcast(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type == ACKTYPE_MESSAGE && ack->result == ACKRESULT_SUCCESS)
		SaveLastUsedTimeStamp(ack->hContact);
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (g_plugin.getByte(hContact, dbLastUC_IgnoreContact) == 0)
		Menu_ModifyItem(hMenuItemRemove, LPGENW("Ignore Contact"));
	else
		Menu_ModifyItem(hMenuItemRemove, LPGENW("Show Contact"));
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_TTB_MODULELOADED, Create_TopToolbarShowList);

	Create_MenuitemShowList();
	LoadDBSettings();

	// hotkeys
	HOTKEYDESC hk = {};
	hk.pszName = msLastUC_ShowList;
	hk.szSection.a = "Contacts";
	hk.szDescription.a = LPGEN("Show Recent Contacts");
	hk.pszService = msLastUC_ShowList;
	hk.DefHotKey = MAKEWORD('R', HOTKEYF_CONTROL | HOTKEYF_SHIFT);
	g_plugin.addHotkey(&hk);
	return 0;
}

static INT_PTR ToggleIgnore(WPARAM hContact, LPARAM)
{
	if (hContact != NULL) {
		int state = !g_plugin.getByte(hContact, dbLastUC_IgnoreContact);
		g_plugin.setByte(hContact, dbLastUC_IgnoreContact, state);
		return state;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	CoInitialize(nullptr);

	g_plugin.registerIcon("Recent Contacts", iconList);

	CreateServiceFunction(msLastUC_ShowList, OnMenuCommandShowList);
	CreateServiceFunction(V_RECENTCONTACTS_TOGGLE_IGNORE, ToggleIgnore);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	HookEvent(ME_MSG_WINDOWEVENT, OnMsgEvent);
	HookEvent(ME_GC_HOOK_EVENT, OnGCInEvent);
	HookEvent(ME_GC_EVENT, OnGCOutEvent);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, OnContactSettingChanged);
	HookEvent(ME_OPT_INITIALISE, onOptInitialise);
	HookEvent(ME_PROTO_ACK, OnProtoBroadcast);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	CoUninitialize();
	return 0;
}
