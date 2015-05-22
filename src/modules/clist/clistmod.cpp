/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"
#include "clc.h"

INT_PTR ContactChangeGroup(WPARAM wParam, LPARAM lParam);
int InitCListEvents(void);
void UninitCListEvents(void);
int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
int ContactAdded(WPARAM wParam, LPARAM lParam);
int ContactDeleted(WPARAM wParam, LPARAM lParam);
INT_PTR GetContactDisplayName(WPARAM wParam, LPARAM lParam);
INT_PTR InvalidateDisplayName(WPARAM wParam, LPARAM lParam);
int InitGroupServices(void);
void LoadCluiServices();
INT_PTR Docking_IsDocked(WPARAM wParam, LPARAM lParam);
int LoadCLUIModule(void);
int InitClistHotKeys(void);

HANDLE hContactDoubleClicked, hContactIconChangedEvent;
HIMAGELIST hCListImages;

extern BYTE nameOrder[];

struct ProtoIconIndex
{
	char *szProto;
	int iIconBase;
};

OBJLIST<ProtoIconIndex> protoIconIndex(5);

TCHAR* fnGetStatusModeDescription(int mode, int flags)
{
	static TCHAR szMode[64];
	TCHAR* descr;
	int    noPrefixReqd = 0;
	switch (mode) {
	case ID_STATUS_OFFLINE:
		descr = LPGENT("Offline");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_CONNECTING:
		descr = LPGENT("Connecting");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_ONLINE:
		descr = LPGENT("Online");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_AWAY:
		descr = LPGENT("Away");
		break;
	case ID_STATUS_DND:
		descr = LPGENT("DND");
		break;
	case ID_STATUS_NA:
		descr = LPGENT("NA");
		break;
	case ID_STATUS_OCCUPIED:
		descr = LPGENT("Occupied");
		break;
	case ID_STATUS_FREECHAT:
		descr = LPGENT("Free for chat");
		break;
	case ID_STATUS_INVISIBLE:
		descr = LPGENT("Invisible");
		break;
	case ID_STATUS_OUTTOLUNCH:
		descr = LPGENT("Out to lunch");
		break;
	case ID_STATUS_ONTHEPHONE:
		descr = LPGENT("On the phone");
		break;
	case ID_STATUS_IDLE:
		descr = LPGENT("Idle");
		break;
	default:
		if (IsStatusConnecting(mode)) {
			const TCHAR* connFmt = LPGENT("Connecting (attempt %d)");
			mir_sntprintf(szMode, SIZEOF(szMode), (flags & GSMDF_UNTRANSLATED) ? connFmt : TranslateTS(connFmt), mode - ID_STATUS_CONNECTING + 1);
			return szMode;
		}
		return NULL;
	}

	return (flags & GSMDF_UNTRANSLATED) ? descr : TranslateTS(descr);
}

static INT_PTR GetStatusModeDescription(WPARAM wParam, LPARAM lParam)
{
	TCHAR *buf1 = cli.pfnGetStatusModeDescription(wParam, lParam);

	if (!(lParam & GSMDF_TCHAR)) {
		static char szMode[64];
		char *buf2 = mir_u2a(buf1);
		strncpy_s(szMode, buf2, _TRUNCATE);
		mir_free(buf2);
		return (INT_PTR)szMode;
	}

	return (INT_PTR)buf1;
}

static int ProtocolAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *) lParam;
	if (ack->type != ACKTYPE_STATUS)
		return 0;

	cli.pfnCluiProtocolStatusChanged(lParam, ack->szModule);

	if ((int)ack->hProcess < ID_STATUS_ONLINE && ack->lParam >= ID_STATUS_ONLINE) {
		DWORD caps = (DWORD)CallProtoServiceInt(NULL,ack->szModule, PS_GETCAPS, PFLAGNUM_1, 0);
		if (caps & PF1_SERVERCLIST) {
			for (MCONTACT hContact = db_find_first(ack->szModule); hContact; ) {
				MCONTACT hNext = db_find_next(hContact, ack->szModule);
				if (db_get_b(hContact, "CList", "Delete", 0))
					CallService(MS_DB_CONTACT_DELETE, hContact, 0);
				hContact = hNext;
			}
		}
	}

	cli.pfnTrayIconUpdateBase(ack->szModule);
	return 0;
}

HICON fnGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status)
{
	return ImageList_GetIcon(hCListImages, cli.pfnIconFromStatusMode(szProto, status, hContact), ILD_NORMAL);
}

int fnIconFromStatusMode(const char *szProto, int status, MCONTACT)
{
	int index, i;

	for (index = 0; index < SIZEOF(statusModeList); index++)
		if (status == statusModeList[index])
			break;

	if (index == SIZEOF(statusModeList))
		index = 0;
	if (szProto == NULL)
		return index + 1;
	for (i=0; i < protoIconIndex.getCount(); i++) {
		if (mir_strcmp(szProto, protoIconIndex[i].szProto) == 0)
			return protoIconIndex[i].iIconBase + index;
	}
	return 1;
}

int fnGetContactIcon(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	return cli.pfnIconFromStatusMode(szProto,
		szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact);
}

static INT_PTR GetContactIcon(WPARAM wParam, LPARAM)
{
	return cli.pfnGetContactIcon(wParam);
}

static void AddProtoIconIndex(PROTOACCOUNT *pa)
{
	ProtoIconIndex *pii = new ProtoIconIndex;
	pii->szProto = pa->szModuleName;
	for (int i=0; i < SIZEOF(statusModeList); i++) {
		int iImg = ImageList_AddIcon_ProtoIconLibLoaded(hCListImages, pa->szModuleName, statusModeList[i]);
		if (i == 0)
			pii->iIconBase = iImg;
	}
	protoIconIndex.insert(pii);
}

static void RemoveProtoIconIndex(PROTOACCOUNT *pa)
{
	for (int i=0; i < protoIconIndex.getCount(); i++)
		if (mir_strcmp(protoIconIndex[i].szProto, pa->szModuleName) == 0) {
			protoIconIndex.remove(i);
			break;
		}
}

static int ContactListModulesLoaded(WPARAM, LPARAM)
{
	RebuildMenuOrder();
	for (int i=0; i < accounts.getCount(); i++)
		AddProtoIconIndex(accounts[i]);

	cli.pfnLoadContactTree();

	LoadCLUIModule();

	InitClistHotKeys();

	return 0;
}

static int ContactListAccountsChanged(WPARAM eventCode, LPARAM lParam)
{
	switch (eventCode) {
	case PRAC_ADDED:
		AddProtoIconIndex((PROTOACCOUNT*)lParam);
		break;

	case PRAC_REMOVED:
		RemoveProtoIconIndex((PROTOACCOUNT*)lParam);
		break;
	}
	cli.pfnReloadProtoMenus();
	cli.pfnTrayIconIconsChanged();
	cli.pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
	cli.pfnClcBroadcast(INTM_INVALIDATE, 0, 0);
	return 0;
}

static INT_PTR ContactDoubleClicked(WPARAM wParam, LPARAM)
{
	// Try to process event myself
	if (cli.pfnEventsProcessContactDoubleClick(wParam) == 0)
		return 0;

	// Allow third-party plugins to process a dblclick
	if (NotifyEventHooks(hContactDoubleClicked, wParam, 0))
		return 0;

	// Otherwise try to execute the default action
	TryProcessDoubleClick(wParam);
	return 0;
}

static INT_PTR GetIconsImageList(WPARAM, LPARAM)
{
	return (INT_PTR)hCListImages;
}

static INT_PTR ContactFilesDropped(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_FILE_SENDSPECIFICFILES, wParam, lParam);
	return 0;
}

static int CListIconsChanged(WPARAM, LPARAM)
{
	int i, j;

	for (i=0; i < SIZEOF(statusModeList); i++)
		ImageList_ReplaceIcon_IconLibLoaded(hCListImages, i + 1, LoadSkinIcon(skinIconStatusList[i]));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPOPEN, LoadSkinIcon(SKINICON_OTHER_GROUPOPEN));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPSHUT, LoadSkinIcon(SKINICON_OTHER_GROUPSHUT));
	for (i=0; i < protoIconIndex.getCount(); i++)
		for (j = 0; j < SIZEOF(statusModeList); j++)
			ImageList_ReplaceIcon_IconLibLoaded(hCListImages, protoIconIndex[i].iIconBase + j, LoadSkinProtoIcon(protoIconIndex[i].szProto, statusModeList[j]));
	cli.pfnTrayIconIconsChanged();
	cli.pfnInvalidateRect(cli.hwndContactList, NULL, TRUE);
	return 0;
}

/*
Begin of Hrk's code for bug
*/
#define GWVS_HIDDEN 1
#define GWVS_VISIBLE 2
#define GWVS_COVERED 3
#define GWVS_PARTIALLY_COVERED 4

int fnGetWindowVisibleState(HWND hWnd, int iStepX, int iStepY)
{
	RECT rc, rcWin, rcWorkArea;
	POINT pt;
	register int i, j, width, height, iCountedDots = 0, iNotCoveredDots = 0;
	BOOL bPartiallyCovered = FALSE;
	HWND hAux = 0;

	if (hWnd == NULL) {
		SetLastError(0x00000006);       //Wrong handle
		return -1;
	}

	//Some defaults now. The routine is designed for thin and tall windows.
	if (iStepX <= 0)
		iStepX = 4;
	if (iStepY <= 0)
		iStepY = 16;

	if (IsIconic(hWnd) || !IsWindowVisible(hWnd))
		return GWVS_HIDDEN;

	if (CallService(MS_CLIST_DOCKINGISDOCKED, 0, 0))
		return GWVS_VISIBLE;

	GetWindowRect(hWnd, &rcWin);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMon, &mi))
		rcWorkArea = mi.rcWork;

	IntersectRect(&rc, &rcWin, &rcWorkArea);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	for (i = rc.top; i < rc.bottom; i += (height / iStepY)) {
		pt.y = i;
		for (j = rc.left; j < rc.right; j += (width / iStepX)) {
			pt.x = j;
			hAux = WindowFromPoint(pt);
			while (GetParent(hAux) != NULL)
				hAux = GetParent(hAux);
			if (hAux != hWnd && hAux != NULL)       //There's another window!
				bPartiallyCovered = TRUE;
			else
				iNotCoveredDots++;  //Let's count the not covered dots.
			iCountedDots++; //Let's keep track of how many dots we checked.
		}
	}

	if (iNotCoveredDots == iCountedDots)    //Every dot was not covered: the window is visible.
		return GWVS_VISIBLE;

	if (iNotCoveredDots == 0)  //They're all covered!
		return GWVS_COVERED;

	//There are dots which are visible, but they are not as many as the ones we counted: it's partially covered.
	return GWVS_PARTIALLY_COVERED;
}

int fnShowHide(WPARAM, LPARAM)
{
	BOOL bShow = FALSE;

	int iVisibleState = cli.pfnGetWindowVisibleState(cli.hwndContactList, 0, 0);

	//bShow is FALSE when we enter the switch.
	switch (iVisibleState) {
	case GWVS_PARTIALLY_COVERED:
		//If we don't want to bring it to top, we can use a simple break. This goes against readability ;-) but the comment explains it.
		if (!db_get_b(NULL, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT))
			break;
	case GWVS_COVERED:     //Fall through (and we're already falling)
	case GWVS_HIDDEN:
		bShow = TRUE;
		break;
	case GWVS_VISIBLE:     //This is not needed, but goes for readability.
		bShow = FALSE;
		break;
	case -1:               //We can't get here, both cli.hwndContactList and iStepX and iStepY are right.
		return 0;
	}

	if (bShow == TRUE) {
		RECT rcWindow;

		ShowWindow(cli.hwndContactList, SW_RESTORE);
		if (!db_get_b(NULL, "CList", "OnTop", SETTING_ONTOP_DEFAULT))
			SetWindowPos(cli.hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		else
			SetWindowPos(cli.hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		SetForegroundWindow(cli.hwndContactList);
		db_set_b(NULL, "CList", "State", SETTING_STATE_NORMAL);

		//this forces the window onto the visible screen
		GetWindowRect(cli.hwndContactList, &rcWindow);
		if (AssertInsideScreen(rcWindow) == 1) {
			MoveWindow(cli.hwndContactList, rcWindow.left, rcWindow.top,
				rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
		}
	}
	else {                      //It needs to be hidden
		if (db_get_b(NULL, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ||
			db_get_b(NULL, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
			ShowWindow(cli.hwndContactList, SW_HIDE);
			db_set_b(NULL, "CList", "State", SETTING_STATE_HIDDEN);
		}
		else {
			ShowWindow(cli.hwndContactList, SW_MINIMIZE);
			db_set_b(NULL, "CList", "State", SETTING_STATE_MINIMIZED);
		}

		if (db_get_b(NULL, "CList", "DisableWorkingSet", 1))
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// old evil code. hopefully it will be deleted soon, cause nobody uses it now

#define SAFESTRING(a) a?a:""

int GetStatusModeOrdering(int statusMode);
extern int sortByStatus, sortByProto;

static INT_PTR CompareContacts(WPARAM wParam, LPARAM lParam)
{
	MCONTACT a = wParam, b = lParam;
	TCHAR namea[128], *nameb;
	int statusa, statusb;
	char *szProto1, *szProto2;
	int rc;

	szProto1 = GetContactProto(a);
	szProto2 = GetContactProto(b);
	statusa = db_get_w(a, SAFESTRING(szProto1), "Status", ID_STATUS_OFFLINE);
	statusb = db_get_w(b, SAFESTRING(szProto2), "Status", ID_STATUS_OFFLINE);

	if (sortByProto) {
		/* deal with statuses, online contacts have to go above offline */
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)) {
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}
		/* both are online, now check protocols */
		rc = mir_strcmp(SAFESTRING(szProto1), SAFESTRING(szProto2));        /* mir_strcmp() doesn't like NULL so feed in "" as needed */
		if (rc != 0 && (szProto1 != NULL && szProto2 != NULL))
			return rc;
		/* protocols are the same, order by display name */
	}

	if (sortByStatus) {
		int ordera, orderb;
		ordera = GetStatusModeOrdering(statusa);
		orderb = GetStatusModeOrdering(statusb);
		if (ordera != orderb)
			return ordera - orderb;
	}
	else {
		//one is offline: offline goes below online
		if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)) {
			return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}
	}

	nameb = cli.pfnGetContactDisplayName(a, 0);
	_tcsncpy_s(namea, nameb, _TRUNCATE);
	namea[ SIZEOF(namea)-1 ] = 0;
	nameb = cli.pfnGetContactDisplayName(b, 0);

	//otherwise just compare names
	return _tcsicmp(namea, nameb);
}

/***************************************************************************************/

static INT_PTR ShowHideStub(WPARAM wParam, LPARAM lParam) { return cli.pfnShowHide(wParam, lParam); }
static INT_PTR SetHideOfflineStub(WPARAM wParam, LPARAM lParam) { return cli.pfnSetHideOffline(wParam, lParam); }
static INT_PTR Docking_ProcessWindowMessageStub(WPARAM wParam, LPARAM lParam) { return cli.pfnDocking_ProcessWindowMessage(wParam, lParam); }
static INT_PTR HotkeysProcessMessageStub(WPARAM wParam, LPARAM lParam) { return cli.pfnHotkeysProcessMessage(wParam, lParam); }

int LoadContactListModule2(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ContactListModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, ContactListAccountsChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_PROTO_ACK, ProtocolAck);

	hContactDoubleClicked = CreateHookableEvent(ME_CLIST_DOUBLECLICKED);
	hContactIconChangedEvent = CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);

	LoadCluiServices();

	CreateServiceFunction(MS_CLIST_CONTACTDOUBLECLICKED, ContactDoubleClicked);
	CreateServiceFunction(MS_CLIST_CONTACTFILESDROPPED, ContactFilesDropped);
	CreateServiceFunction(MS_CLIST_GETSTATUSMODEDESCRIPTION, GetStatusModeDescription);
	CreateServiceFunction(MS_CLIST_GETCONTACTDISPLAYNAME, GetContactDisplayName);
	CreateServiceFunction(MS_CLIST_INVALIDATEDISPLAYNAME, InvalidateDisplayName);
	CreateServiceFunction(MS_CLIST_CONTACTSCOMPARE, CompareContacts);
	CreateServiceFunction(MS_CLIST_CONTACTCHANGEGROUP, ContactChangeGroup);
	CreateServiceFunction(MS_CLIST_SHOWHIDE, ShowHideStub);
	CreateServiceFunction(MS_CLIST_SETHIDEOFFLINE, SetHideOfflineStub);
	CreateServiceFunction(MS_CLIST_DOCKINGPROCESSMESSAGE, Docking_ProcessWindowMessageStub);
	CreateServiceFunction(MS_CLIST_DOCKINGISDOCKED, Docking_IsDocked);
	CreateServiceFunction(MS_CLIST_HOTKEYSPROCESSMESSAGE, HotkeysProcessMessageStub);
	CreateServiceFunction(MS_CLIST_GETCONTACTICON, GetContactIcon);

	InitCListEvents();
	InitGroupServices();
	cli.pfnInitTray();

	hCListImages = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 13, 0);
	HookEvent(ME_SKIN_ICONSCHANGED, CListIconsChanged);
	CreateServiceFunction(MS_CLIST_GETICONSIMAGELIST, GetIconsImageList);

	ImageList_AddIcon_NotShared(hCListImages, MAKEINTRESOURCE(IDI_BLANK));

	//now all core skin icons are loaded via icon lib. so lets release them
	for (int i=0; i < SIZEOF(statusModeList); i++)
		ImageList_AddIcon_IconLibLoaded(hCListImages, skinIconStatusList[i]);

	//see IMAGE_GROUP... in clist.h if you add more images above here
	ImageList_AddIcon_IconLibLoaded(hCListImages, SKINICON_OTHER_GROUPOPEN);
	ImageList_AddIcon_IconLibLoaded(hCListImages, SKINICON_OTHER_GROUPSHUT);
	return 0;
}

void UnloadContactListModule()
{
	if (!hCListImages)
		return;

	//remove transitory contacts
	for (MCONTACT hContact = db_find_first(); hContact != NULL; ) {
		MCONTACT hNext = db_find_next(hContact);
		if (db_get_b(hContact, "CList", "NotOnList", 0))
			CallService(MS_DB_CONTACT_DELETE, hContact, 0);
		hContact = hNext;
	}
	ImageList_Destroy(hCListImages);
	UninitCListEvents();
	DestroyHookableEvent(hContactDoubleClicked);
}
