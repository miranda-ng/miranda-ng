/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"
#include "clc.h"

int InitGenMenu(void);
int UnitGenMenu(void);

void InitCustomMenus(void);
void UninitCustomMenus(void);

void MTG_OnmodulesLoad(void);

static bool bModuleInitialized = false;
static HANDLE hClcWindowList;
static HANDLE hShowInfoTipEvent;
HANDLE hHideInfoTipEvent;
static LIST<void> arEvents(10);

int g_IconWidth, g_IconHeight;

void FreeDisplayNameCache(void);

void fnInitAutoRebuild(HWND hWnd)
{
	if (!cli.bAutoRebuild && hWnd) {
		cli.bAutoRebuild = true;
		SendMessage(hWnd, CLM_AUTOREBUILD, 0, 0);
	}
}

void fnClcBroadcast(int msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hClcWindowList, msg, wParam, lParam);
}

void fnClcOptionsChanged(void)
{
	cli.pfnClcBroadcast(INTM_RELOADOPTIONS, 0, 0);
}

HMENU fnBuildGroupPopupMenu(ClcGroup* group)
{
	HMENU hMenu = LoadMenu(cli.hInst, MAKEINTRESOURCE(IDR_CONTEXT));
	HMENU hGroupMenu = GetSubMenu(hMenu, 2);
	RemoveMenu(hMenu, 2, MF_BYPOSITION);
	DestroyMenu(hMenu);
	TranslateMenu(hGroupMenu);

	CheckMenuItem(hGroupMenu, POPUP_GROUPHIDEOFFLINE, group->hideOffline ? MF_CHECKED : MF_UNCHECKED);
	return hGroupMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////
// standard CLC services

static int ClcSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	if (hContact == NULL) {
		if (!strcmp(cws->szModule, "CListGroups"))
			cli.pfnClcBroadcast(INTM_GROUPSCHANGED, hContact, lParam);
		return 0;
	}

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "MyHandle")) {
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
			cli.pfnClcBroadcast(INTM_NAMECHANGED, hContact, lParam);
		}
		else if (!strcmp(cws->szSetting, "Group"))
			cli.pfnClcBroadcast(INTM_GROUPCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "Hidden"))
			cli.pfnClcBroadcast(INTM_HIDDENCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "NotOnList"))
			cli.pfnClcBroadcast(INTM_NOTONLISTCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "Status"))
			cli.pfnClcBroadcast(INTM_INVALIDATE, 0, 0);
		else if (!strcmp(cws->szSetting, "NameOrder"))
			cli.pfnClcBroadcast(INTM_NAMEORDERCHANGED, 0, 0);
	}
	else {
		char *szProto = GetContactProto(hContact);
		if (szProto != NULL) {
			if (!strcmp(cws->szModule, "Protocol") && !strcmp(cws->szSetting, "p"))
				cli.pfnClcBroadcast(INTM_PROTOCHANGED, hContact, lParam);

			// something is being written to a protocol module
			if (!strcmp(szProto, cws->szModule)) {
				// was a unique setting key written?
				char *id = (char *) CallProtoServiceInt(NULL,szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
				if ((INT_PTR)id != CALLSERVICE_NOTFOUND && id != NULL && !strcmp(id, cws->szSetting))
					cli.pfnClcBroadcast(INTM_PROTOCHANGED, hContact, lParam);
			}
		}
		if (szProto == NULL || strcmp(szProto, cws->szModule))
			return 0;
		if (!strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName") || !strcmp(cws->szSetting, "e-mail")
			 ||  !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "UIN"))
			cli.pfnClcBroadcast(INTM_NAMECHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "ApparentMode"))
			cli.pfnClcBroadcast(INTM_APPARENTMODECHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "IdleTS"))
			cli.pfnClcBroadcast(INTM_IDLECHANGED, hContact, lParam);
	}
	return 0;
}

static int ClcAccountsChanged(WPARAM, LPARAM)
{
	int i, cnt;
	for (i=0, cnt=0; i < accounts.getCount(); i++)
		if (Proto_IsAccountEnabled(accounts[i]))
			cnt++;

	cli.hClcProtoCount = cnt;
	cli.clcProto = (ClcProtoStatus *) mir_realloc(cli.clcProto, sizeof(ClcProtoStatus) * cli.hClcProtoCount);

	for (i=0, cnt=0; i < accounts.getCount(); i++) {
		if (Proto_IsAccountEnabled(accounts[i])) {
			cli.clcProto[cnt].szProto = accounts[i]->szModuleName;
			cli.clcProto[cnt].dwStatus = CallProtoServiceInt(NULL,accounts[i]->szModuleName, PS_GETSTATUS, 0, 0);
			++cnt;
		}
	}
	return 0;
}

static int ClcModulesLoaded(WPARAM, LPARAM)
{
	ClcAccountsChanged(0, 0);
	MTG_OnmodulesLoad();
	return 0;
}

static int ClcProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *) lParam;
	if (ack->type == ACKTYPE_STATUS) {
		WindowList_BroadcastAsync(hClcWindowList, INTM_INVALIDATE, 0, 0);
		if (ack->result == ACKRESULT_SUCCESS) {
			for (int i=0; i < cli.hClcProtoCount; i++) {
				if (!lstrcmpA(cli.clcProto[i].szProto, ack->szModule)) {
					cli.clcProto[i].dwStatus = (WORD) ack->lParam;
					break;
				}
			}
		}
	}
	return 0;
}

static int ClcContactAdded(WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(hClcWindowList, INTM_CONTACTADDED, wParam, lParam);
	return 0;
}

static int ClcContactDeleted(WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(hClcWindowList, INTM_CONTACTDELETED, wParam, lParam);
	return 0;
}

static int ClcContactIconChanged(WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(hClcWindowList, INTM_ICONCHANGED, wParam, lParam);
	return 0;
}

static int ClcIconsChanged(WPARAM, LPARAM)
{
	WindowList_BroadcastAsync(hClcWindowList, INTM_INVALIDATE, 0, 0);
	return 0;
}

static INT_PTR SetInfoTipHoverTime(WPARAM wParam, LPARAM)
{
	db_set_w(NULL, "CLC", "InfoTipHoverTime", (WORD) wParam);
	cli.pfnClcBroadcast(INTM_SETINFOTIPHOVERTIME, wParam, 0);
	return 0;
}

static INT_PTR GetInfoTipHoverTime(WPARAM, LPARAM)
{
	return db_get_w(NULL, "CLC", "InfoTipHoverTime", 750);
}

static void SortClcByTimer(HWND hwnd)
{
	KillTimer(hwnd, TIMERID_DELAYEDRESORTCLC);
	SetTimer(hwnd, TIMERID_DELAYEDRESORTCLC, 200, NULL);
}

int LoadCLCModule(void)
{
	bModuleInitialized = true;

	g_IconWidth = GetSystemMetrics(SM_CXSMICON);
	g_IconHeight = GetSystemMetrics(SM_CYSMICON);

	hClcWindowList = WindowList_Create();
	hShowInfoTipEvent = CreateHookableEvent(ME_CLC_SHOWINFOTIP);
	hHideInfoTipEvent = CreateHookableEvent(ME_CLC_HIDEINFOTIP);
	CreateServiceFunction(MS_CLC_SETINFOTIPHOVERTIME, SetInfoTipHoverTime);
	CreateServiceFunction(MS_CLC_GETINFOTIPHOVERTIME, GetInfoTipHoverTime);

	InitFileDropping();

	arEvents.insert(HookEvent(ME_SYSTEM_MODULESLOADED, ClcModulesLoaded));
	arEvents.insert(HookEvent(ME_PROTO_ACCLISTCHANGED, ClcAccountsChanged));
	arEvents.insert(HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ClcSettingChanged));
	arEvents.insert(HookEvent(ME_DB_CONTACT_ADDED, ClcContactAdded));
	arEvents.insert(HookEvent(ME_DB_CONTACT_DELETED, ClcContactDeleted));
	arEvents.insert(HookEvent(ME_CLIST_CONTACTICONCHANGED, ClcContactIconChanged));
	arEvents.insert(HookEvent(ME_SKIN_ICONSCHANGED, ClcIconsChanged));
	arEvents.insert(HookEvent(ME_PROTO_ACK, ClcProtoAck));

	InitCustomMenus();
	return 0;
}

void UnloadClcModule()
{
	if (!bModuleInitialized)
		return;

	for (int i = 0; i < arEvents.getCount(); i++)
		UnhookEvent(arEvents[i]);

	mir_free(cli.clcProto);
	WindowList_Destroy(hClcWindowList); hClcWindowList = NULL;

	FreeDisplayNameCache();

	UninitCustomMenus();
	UnitGenMenu();
}

/////////////////////////////////////////////////////////////////////////////////////////
// default contact list control window procedure

LRESULT CALLBACK fnContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcGroup *group;
	ClcContact *contact;
	DWORD hitFlags;
	int hit;

	ClcData *dat = (struct ClcData *) GetWindowLongPtr(hwnd, 0);
	if (msg >= CLM_FIRST && msg < CLM_LAST)
		return cli.pfnProcessExternalMessages(hwnd, dat, msg, wParam, lParam);

	switch (msg) {
	case WM_CREATE:
		WindowList_Add(hClcWindowList, hwnd, NULL);
		cli.pfnRegisterFileDropping(hwnd);
		if (dat == NULL) {
			dat = (struct ClcData *) mir_calloc(sizeof(struct ClcData));
			SetWindowLongPtr(hwnd, 0, (LONG_PTR) dat);
		}
		{
			for (int i=0; i <= FONTID_MAX; i++)
				dat->fontInfo[i].changed = 1;
		}
		dat->selection = -1;
		dat->iconXSpace = 20;
		dat->checkboxSize = 13;
		dat->dragAutoScrollHeight = 30;
		dat->iDragItem = -1;
		dat->iInsertionMark = -1;
		dat->insertionMarkHitHeight = 5;
		dat->iHotTrack = -1;
		dat->infoTipTimeout = db_get_w(NULL, "CLC", "InfoTipHoverTime", 750);
		dat->extraColumnSpacing = 20;
		dat->list.cl.increment = 30;
		dat->needsResort = 1;
		cli.pfnLoadClcOptions(hwnd, dat);
		if (!IsWindowVisible(hwnd))
			SetTimer(hwnd, TIMERID_REBUILDAFTER, 10, NULL);
		else {
			cli.pfnRebuildEntireList(hwnd, dat);
			NMCLISTCONTROL nm;
			nm.hdr.code = CLN_LISTREBUILT;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
		}
		break;

	case INTM_SCROLLBARCHANGED:
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CONTACTLIST) {
			if (dat->noVScrollbar)
				ShowScrollBar(hwnd, SB_VERT, FALSE);
			else
				cli.pfnRecalcScrollBar(hwnd, dat);
		}
		break;

	case INTM_RELOADOPTIONS:
		cli.pfnLoadClcOptions(hwnd, dat);
		cli.pfnSaveStateAndRebuildList(hwnd, dat);
		break;

	case WM_THEMECHANGED:
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_SIZE:
		cli.pfnEndRename(hwnd, dat, 1);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		cli.pfnRecalcScrollBar(hwnd, dat);
		{
			// creating imagelist containing blue line for highlight
			RECT rc;
			GetClientRect(hwnd, &rc);
			if (rc.right == 0)
				break;

			rc.bottom = dat->rowHeight;
			HDC hdc = GetDC(hwnd);
			int depth = GetDeviceCaps(hdc, BITSPIXEL);
			if (depth < 16)
				depth = 16;
			HBITMAP hBmp = CreateBitmap(rc.right, rc.bottom, 1, depth, NULL);
			HBITMAP hBmpMask = CreateBitmap(rc.right, rc.bottom, 1, 1, NULL);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hoBmp = (HBITMAP) SelectObject(hdcMem, hBmp);
			HBRUSH hBrush = CreateSolidBrush(dat->useWindowsColours ? GetSysColor(COLOR_HIGHLIGHT) : dat->selBkColour);
			FillRect(hdcMem, &rc, hBrush);
			DeleteObject(hBrush);

			HBITMAP hoMaskBmp = (HBITMAP)SelectObject(hdcMem, hBmpMask);
			FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
				SelectObject(hdcMem, hoMaskBmp);
			SelectObject(hdcMem, hoBmp);
			DeleteDC(hdcMem);
			ReleaseDC(hwnd, hdc);
			if (dat->himlHighlight)
				ImageList_Destroy(dat->himlHighlight);
			dat->himlHighlight = ImageList_Create(rc.right, rc.bottom, ILC_COLOR32 | ILC_MASK, 1, 1);
			ImageList_Add(dat->himlHighlight, hBmp, hBmpMask);
			DeleteObject(hBmpMask);
			DeleteObject(hBmp);
		}
		break;

	case WM_SYSCOLORCHANGE:
		SendMessage(hwnd, WM_SIZE, 0, 0);
		break;

	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg = (MSG *) lParam;
			if (msg->message == WM_KEYDOWN) {
				if (msg->wParam == VK_TAB)
					return 0;
				if (msg->wParam == VK_ESCAPE && dat->hwndRenameEdit == NULL && dat->szQuickSearch[0] == 0)
					return 0;
			}
			if (msg->message == WM_CHAR) {
				if (msg->wParam == '\t')
					return 0;
				if (msg->wParam == 27 && dat->hwndRenameEdit == NULL && dat->szQuickSearch[0] == 0)
					return 0;
			}
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
	case WM_SETFOCUS:
	case WM_ENABLE:
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_GETFONT:
		return (LRESULT)dat->fontInfo[FONTID_CONTACTS].hFont;

	case INTM_GROUPSCHANGED:
		{
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *) lParam;
			if (dbcws->value.type == DBVT_ASCIIZ || dbcws->value.type == DBVT_UTF8) {
				int groupId = atoi(dbcws->szSetting) + 1;
				TCHAR szFullName[512];
				int i, nameLen, eq;
				//check name of group and ignore message if just being expanded/collapsed
				if (cli.pfnFindItem(hwnd, dat, groupId | HCONTACT_ISGROUP, &contact, &group, NULL)) {
					lstrcpy(szFullName, contact->szText);
					while (group->parent) {
						for (i=0; i < group->parent->cl.count; i++)
							if (group->parent->cl.items[i]->group == group)
								break;
						if (i == group->parent->cl.count) {
							szFullName[0] = '\0';
							break;
						}
						group = group->parent;
						nameLen = lstrlen(group->cl.items[i]->szText);
						if (lstrlen(szFullName) + 1 + nameLen > SIZEOF(szFullName)) {
							szFullName[0] = '\0';
							break;
						}
						memmove(szFullName + 1 + nameLen, szFullName, sizeof(TCHAR)*(lstrlen(szFullName) + 1));
						memcpy(szFullName, group->cl.items[i]->szText, sizeof(TCHAR)*nameLen);
						szFullName[nameLen] = '\\';
					}

					if (dbcws->value.type == DBVT_ASCIIZ) {
						WCHAR* wszGrpName = mir_a2u(dbcws->value.pszVal+1);
						eq = !lstrcmp(szFullName, wszGrpName);
						mir_free(wszGrpName);
					}
					else {
						char* szGrpName = NEWSTR_ALLOCA(dbcws->value.pszVal+1);
						WCHAR* wszGrpName;
						Utf8Decode(szGrpName, &wszGrpName);
						eq = !lstrcmp(szFullName, wszGrpName);
						mir_free(wszGrpName);
					}
					if (eq && (contact->group->hideOffline != 0) == ((dbcws->value.pszVal[0] & GROUPF_HIDEOFFLINE) != 0))
						break;  //only expanded has changed: no action reqd
				}
			}
			cli.pfnSaveStateAndRebuildList(hwnd, dat);
		}
		break;

	case INTM_NAMEORDERCHANGED:
		cli.pfnInitAutoRebuild(hwnd);
		break;

	case INTM_CONTACTADDED:
		cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
		cli.pfnNotifyNewContact(hwnd, wParam);
		SortClcByTimer(hwnd);
		break;

	case INTM_CONTACTDELETED:
		cli.pfnDeleteItemFromTree(hwnd, wParam);
		SortClcByTimer(hwnd);
		break;

	case INTM_HIDDENCHANGED:
		{
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *) lParam;
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN)
				break;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0) {
				if (cli.pfnFindItem(hwnd, dat, wParam, NULL, NULL, NULL))
					break;
				cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				cli.pfnNotifyNewContact(hwnd, wParam);
			}
			else cli.pfnDeleteItemFromTree(hwnd, wParam);

			dat->needsResort = 1;
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_GROUPCHANGED:
		{
			WORD iExtraImage[EXTRA_ICON_COUNT];
			BYTE flags = 0;
			if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
				memset(iExtraImage, 0xFF, sizeof(iExtraImage));
			else {
				memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
				flags = contact->flags;
			}
			cli.pfnDeleteItemFromTree(hwnd, wParam);
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !db_get_b(wParam, "CList", "Hidden", 0)) {
				NMCLISTCONTROL nm;
				cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				if (cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL)) {
					memcpy(contact->iExtraImage, iExtraImage, sizeof(iExtraImage));
					if (flags & CONTACTF_CHECKED)
						contact->flags |= CONTACTF_CHECKED;
				}
				nm.hdr.code = CLN_CONTACTMOVED;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = (HANDLE)wParam;
				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
				dat->needsResort = 1;
			}
		}
		SetTimer(hwnd, TIMERID_REBUILDAFTER, 1, NULL);
		break;

	case INTM_ICONCHANGED:
		{
			int recalcScrollBar = 0, shouldShow;
			WORD status;
			MCONTACT hSelItem = NULL;
			ClcContact *selcontact = NULL;

			char *szProto = GetContactProto(wParam);
			if (szProto == NULL)
				status = ID_STATUS_OFFLINE;
			else
				status = db_get_w(wParam, szProto, "Status", ID_STATUS_OFFLINE);

			// this means an offline msg is flashing, so the contact should be shown
			DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
			shouldShow = (style & CLS_SHOWHIDDEN || !db_get_b(wParam, "CList", "Hidden", 0))
				&& (!cli.pfnIsHiddenMode(dat, status) || CallService(MS_CLIST_GETCONTACTICON, wParam, 0) != lParam);

			contact = NULL;
			group = NULL;
			if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, &group, NULL)) {
				if (shouldShow && CallService(MS_DB_CONTACT_IS, wParam, 0)) {
					if (dat->selection >= 0 && cli.pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
						hSelItem = (MCONTACT)cli.pfnContactToHItem(selcontact);
					cli.pfnAddContactToTree(hwnd, dat, wParam, (style & CLS_CONTACTLIST) == 0, 0);
					recalcScrollBar = 1;
					cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL);
					if (contact) {
						contact->iImage = (WORD) lParam;
						cli.pfnNotifyNewContact(hwnd, wParam);
						dat->needsResort = 1;
					}
				}
			}
			else { // item in list already
				if (contact->iImage == (WORD) lParam)
					break;
				if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					if (dat->selection >= 0 && cli.pfnGetRowByIndex(dat, dat->selection, &selcontact, NULL) != -1)
						hSelItem = (MCONTACT)cli.pfnContactToHItem(selcontact);
					cli.pfnRemoveItemFromGroup(hwnd, group, contact, (style & CLS_CONTACTLIST) == 0);
					recalcScrollBar = 1;
				}
				else {
					contact->iImage = (WORD) lParam;
					if (!cli.pfnIsHiddenMode(dat, status))
						contact->flags |= CONTACTF_ONLINE;
					else
						contact->flags &= ~CONTACTF_ONLINE;
				}
				dat->needsResort = 1;
			}
			if (hSelItem) {
				ClcGroup *selgroup;
				if (cli.pfnFindItem(hwnd, dat, hSelItem, &selcontact, &selgroup, NULL))
					dat->selection = cli.pfnGetRowsPriorTo(&dat->list, selgroup, List_IndexOf((SortedList*)&selgroup->cl, selcontact));
				else
					dat->selection = -1;
			}
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_NAMECHANGED:
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;

		lstrcpyn(contact->szText, cli.pfnGetContactDisplayName(wParam, 0), SIZEOF(contact->szText));
		dat->needsResort = 1;
		SortClcByTimer(hwnd);
		break;

	case INTM_PROTOCHANGED:
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;

		contact->proto = GetContactProto(wParam);
		cli.pfnInvalidateDisplayNameCacheEntry(wParam);
		lstrcpyn(contact->szText, cli.pfnGetContactDisplayName(wParam, 0), SIZEOF(contact->szText));
		SortClcByTimer(hwnd);
		break;

	case INTM_NOTONLISTCHANGED:
		if (!cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL))
			break;

		if (contact->type == CLCIT_CONTACT) {
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *) lParam;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0)
				contact->flags &= ~CONTACTF_NOTONLIST;
			else
				contact->flags |= CONTACTF_NOTONLIST;
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case INTM_INVALIDATE:
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case INTM_APPARENTMODECHANGED:
		if (cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL)) {
			char *szProto = GetContactProto(wParam);
			if (szProto == NULL)
				break;

			WORD apparentMode = db_get_w(wParam, szProto, "ApparentMode", 0);
			contact->flags &= ~(CONTACTF_INVISTO | CONTACTF_VISTO);
			if (apparentMode == ID_STATUS_OFFLINE)
				contact->flags |= CONTACTF_INVISTO;
			else if (apparentMode == ID_STATUS_ONLINE)
				contact->flags |= CONTACTF_VISTO;
			else if (apparentMode)
				contact->flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case INTM_SETINFOTIPHOVERTIME:
		dat->infoTipTimeout = wParam;
		break;

	case INTM_IDLECHANGED:
		if (cli.pfnFindItem(hwnd, dat, wParam, &contact, NULL, NULL)) {
			char *szProto = GetContactProto(wParam);
			if (szProto == NULL)
				break;
			contact->flags &= ~CONTACTF_IDLE;
			if (db_get_dw(wParam, szProto, "IdleTS", 0))
				contact->flags |= CONTACTF_IDLE;

			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		}
		break;

	case WM_PRINTCLIENT:
		cli.pfnPaintClc(hwnd, dat, (HDC) wParam, NULL);
		break;

	case WM_NCPAINT:
		if (wParam != 1) {
			POINT ptTopLeft = { 0, 0 };
			HRGN hClientRgn;
			ClientToScreen(hwnd, &ptTopLeft);
			hClientRgn = CreateRectRgn(0, 0, 1, 1);
			CombineRgn(hClientRgn, (HRGN) wParam, NULL, RGN_COPY);
			OffsetRgn(hClientRgn, -ptTopLeft.x, -ptTopLeft.y);
			InvalidateRgn(hwnd, hClientRgn, FALSE);
			DeleteObject(hClientRgn);
			UpdateWindow(hwnd);
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			/* we get so many cli.pfnInvalidateRect()'s that there is no point painting,
			Windows in theory shouldn't queue up WM_PAINTs in this case but it does so
			we'll just ignore them */
			if (IsWindowVisible(hwnd))
				cli.pfnPaintClc(hwnd, dat, hdc, &ps.rcPaint);
			EndPaint(hwnd, &ps);
		}
		break;

	case WM_VSCROLL:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		{
			int desty = dat->yScroll, noSmooth = 0;

			RECT clRect;
			GetClientRect(hwnd, &clRect);
			switch (LOWORD(wParam)) {
				case SB_LINEUP:     desty -= dat->rowHeight;   break;
				case SB_LINEDOWN:   desty += dat->rowHeight;   break;
				case SB_PAGEUP:     desty -= clRect.bottom - dat->rowHeight; break;
				case SB_PAGEDOWN:   desty += clRect.bottom - dat->rowHeight; break;
				case SB_BOTTOM:     desty = 0x7FFFFFFF;        break;
				case SB_TOP:        desty = 0;                 break;
				case SB_THUMBTRACK: desty = HIWORD(wParam); noSmooth = 1; break; //noone has more than 4000 contacts, right?
				default:            return 0;
			}
			cli.pfnScrollTo(hwnd, dat, desty, noSmooth);
		}
		break;

	case WM_MOUSEWHEEL:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		{
			UINT scrollLines;
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
				scrollLines = 3;
			cli.pfnScrollTo(hwnd, dat, dat->yScroll - (short) HIWORD(wParam) * dat->rowHeight * (signed) scrollLines / WHEEL_DELTA, 0);
		}
		return 0;

	case WM_KEYDOWN:
		{
			int selMoved = 0;
			int changeGroupExpand = 0;
			int pageSize;
			cli.pfnHideInfoTip(hwnd, dat);
			KillTimer(hwnd, TIMERID_INFOTIP);
			KillTimer(hwnd, TIMERID_RENAME);
			if (CallService(MS_CLIST_MENUPROCESSHOTKEY, wParam, MPCF_CONTACTMENU))
				break;
			{
				RECT clRect;
				GetClientRect(hwnd, &clRect);
				pageSize = clRect.bottom / dat->rowHeight;
			}
			switch (wParam) {
			case VK_DOWN:   dat->selection++; selMoved = 1; break;
			case VK_UP:     dat->selection--; selMoved = 1; break;
			case VK_PRIOR:  dat->selection -= pageSize; selMoved = 1; break;
			case VK_NEXT:   dat->selection += pageSize; selMoved = 1; break;
			case VK_HOME:   dat->selection = 0; selMoved = 1; break;
			case VK_END:    dat->selection = cli.pfnGetGroupContentsCount(&dat->list, 1) - 1; selMoved = 1; break;
			case VK_LEFT:   changeGroupExpand = 1; break;
			case VK_RIGHT:  changeGroupExpand = 2; break;
			case VK_RETURN:
				cli.pfnDoSelectionDefaultAction(hwnd, dat);
				dat->szQuickSearch[0] = 0;
				if (dat->filterSearch)
					cli.pfnSaveStateAndRebuildList(hwnd, dat);
				return 0;
			case VK_F2:     cli.pfnBeginRenameSelection(hwnd, dat); return 0;
			case VK_DELETE: cli.pfnDeleteFromContactList(hwnd, dat); return 0;
			default:
				{
					NMKEY nmkey;
					nmkey.hdr.hwndFrom = hwnd;
					nmkey.hdr.idFrom = GetDlgCtrlID(hwnd);
					nmkey.hdr.code = NM_KEYDOWN;
					nmkey.nVKey = wParam;
					nmkey.uFlags = HIWORD(lParam);
					if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nmkey))
						return 0;
				}
			}
			if (changeGroupExpand) {
				if (!dat->filterSearch)
					dat->szQuickSearch[0] = 0;
				hit = cli.pfnGetRowByIndex(dat, dat->selection, &contact, &group);
				if (hit != -1) {
					if (changeGroupExpand == 1 && contact->type == CLCIT_CONTACT) {
						if (group == &dat->list)
							return 0;
						dat->selection = cli.pfnGetRowsPriorTo(&dat->list, group, -1);
						selMoved = 1;
					}
					else {
						if (contact->type == CLCIT_GROUP)
							cli.pfnSetGroupExpand(hwnd, dat, contact->group, changeGroupExpand == 2);
						return 0;
					}
				}
				else
					return 0;
			}
			if (selMoved) {
				if (!dat->filterSearch)
					dat->szQuickSearch[0] = 0;
				if (dat->selection >= cli.pfnGetGroupContentsCount(&dat->list, 1))
					dat->selection = cli.pfnGetGroupContentsCount(&dat->list, 1) - 1;
				if (dat->selection < 0)
					dat->selection = 0;
				cli.pfnInvalidateRect(hwnd, NULL, FALSE);
				cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
				UpdateWindow(hwnd);
				return 0;
			}
		}
		break;

	case WM_CHAR:
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		if (wParam == 27) //escape
			dat->szQuickSearch[0] = 0;
		else if (wParam == '\b' && dat->szQuickSearch[0])
			dat->szQuickSearch[lstrlen(dat->szQuickSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else if (wParam == ' ' && dat->szQuickSearch[0] == '\0' && GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CHECKBOXES) {
			NMCLISTCONTROL nm;
			if (cli.pfnGetRowByIndex(dat, dat->selection, &contact, NULL) == -1)
				break;
			if (contact->type != CLCIT_CONTACT)
				break;
			contact->flags ^= CONTACTF_CHECKED;
			if (contact->type == CLCIT_GROUP)
				cli.pfnSetGroupChildCheckboxes(contact->group, contact->flags & CONTACTF_CHECKED);
			cli.pfnRecalculateGroupCheckboxes(hwnd, dat);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			nm.hdr.code = CLN_CHECKCHANGED;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
		}
		else {
			TCHAR szNew[2];
			szNew[0] = (TCHAR) wParam;
			szNew[1] = '\0';
			if (lstrlen(dat->szQuickSearch) >= SIZEOF(dat->szQuickSearch) - 1) {
				MessageBeep(MB_OK);
				break;
			}
			_tcscat(dat->szQuickSearch, szNew);
		}

		if (dat->filterSearch)
			cli.pfnSaveStateAndRebuildList(hwnd, dat);

		if (dat->szQuickSearch[0]) {
			int index;
			index = cli.pfnFindRowByText(hwnd, dat, dat->szQuickSearch, 1);
			if (index != -1)
				dat->selection = index;
			else {
				MessageBeep(MB_OK);
				dat->szQuickSearch[ lstrlen(dat->szQuickSearch) - 1] = '\0';
				cli.pfnSaveStateAndRebuildList(hwnd, dat);
			}
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
		}
		else
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		break;

	case WM_SYSKEYDOWN:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		dat->iHotTrack = -1;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		ReleaseCapture();
		if (wParam == VK_F10 && GetKeyState(VK_SHIFT) & 0x8000)
			break;
		SendMessage(GetParent(hwnd), msg, wParam, lParam);
		return 0;

	case WM_TIMER:
		switch(wParam) {
		case TIMERID_RENAME:
			cli.pfnBeginRenameSelection(hwnd, dat);
			break;
		case TIMERID_DRAGAUTOSCROLL:
			cli.pfnScrollTo(hwnd, dat, dat->yScroll + dat->dragAutoScrolling * dat->rowHeight * 2, 0);
			break;
		case TIMERID_INFOTIP:
			{
				CLCINFOTIP it;
				RECT clRect;
				POINT ptClientOffset = { 0 };

				KillTimer(hwnd, wParam);
				GetCursorPos(&it.ptCursor);
				ScreenToClient(hwnd, &it.ptCursor);
				if (it.ptCursor.x != dat->ptInfoTip.x || it.ptCursor.y != dat->ptInfoTip.y)
					break;
				GetClientRect(hwnd, &clRect);
				it.rcItem.left = 0;
				it.rcItem.right = clRect.right;
				hit = cli.pfnHitTest(hwnd, dat, it.ptCursor.x, it.ptCursor.y, &contact, NULL, NULL);
				if (hit == -1)
					break;
				if (contact->type != CLCIT_GROUP && contact->type != CLCIT_CONTACT)
					break;
				ClientToScreen(hwnd, &it.ptCursor);
				ClientToScreen(hwnd, &ptClientOffset);
				it.isTreeFocused = GetFocus() == hwnd;
				it.rcItem.top = cli.pfnGetRowTopY(dat, hit) - dat->yScroll;
				it.rcItem.bottom = it.rcItem.top + cli.pfnGetRowHeight(dat, hit);
				OffsetRect(&it.rcItem, ptClientOffset.x, ptClientOffset.y);
				it.isGroup = contact->type == CLCIT_GROUP;
				it.hItem = (contact->type == CLCIT_GROUP) ? (HANDLE)contact->groupId : (HANDLE)contact->hContact;
				it.cbSize = sizeof(it);
				dat->hInfoTipItem = cli.pfnContactToHItem(contact);
				NotifyEventHooks(hShowInfoTipEvent, 0, (LPARAM) & it);
				break;
			}
		case TIMERID_REBUILDAFTER:
			KillTimer(hwnd, TIMERID_REBUILDAFTER);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			cli.pfnSaveStateAndRebuildList(hwnd, dat);
			break;

		case TIMERID_DELAYEDRESORTCLC:
			KillTimer(hwnd, TIMERID_DELAYEDRESORTCLC);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			cli.pfnSortCLC(hwnd, dat, 1);
			cli.pfnRecalcScrollBar(hwnd, dat);
			break;
		}
		break;

	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (GetFocus() != hwnd)
			SetFocus(hwnd);

		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		cli.pfnEndRename(hwnd, dat, 1);
		dat->ptDragStart.x = (short) LOWORD(lParam);
		dat->ptDragStart.y = (short) HIWORD(lParam);
		if (!dat->filterSearch)
			dat->szQuickSearch[0] = 0;

		hit = cli.pfnHitTest(hwnd, dat, (short) LOWORD(lParam), (short) HIWORD(lParam), &contact, &group, &hitFlags);
		if (hit != -1) {
			if (hit == dat->selection && hitFlags & CLCHT_ONITEMLABEL && dat->exStyle & CLS_EX_EDITLABELS) {
				SetCapture(hwnd);
				dat->iDragItem = dat->selection;
				dat->dragStage = DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME;
				dat->dragAutoScrolling = 0;
				break;
			}
		}

		if (hit != -1 && contact->type == CLCIT_GROUP) {
			if (hitFlags & CLCHT_ONITEMICON) {
				ClcGroup *selgroup;
				ClcContact *selcontact;
				dat->selection = cli.pfnGetRowByIndex(dat, dat->selection, &selcontact, &selgroup);
				cli.pfnSetGroupExpand(hwnd, dat, contact->group, -1);
				if (dat->selection != -1) {
					dat->selection =
						cli.pfnGetRowsPriorTo(&dat->list, selgroup, List_IndexOf((SortedList*)&selgroup->cl, selcontact));
					if (dat->selection == -1)
						dat->selection = cli.pfnGetRowsPriorTo(&dat->list, contact->group, -1);
				}
				cli.pfnInvalidateRect(hwnd, NULL, FALSE);
				UpdateWindow(hwnd);
				break;
			}
		}
		if (hit != -1 && hitFlags & CLCHT_ONITEMCHECK) {
			NMCLISTCONTROL nm;
			contact->flags ^= CONTACTF_CHECKED;
			if (contact->type == CLCIT_GROUP)
				cli.pfnSetGroupChildCheckboxes(contact->group, contact->flags & CONTACTF_CHECKED);
			cli.pfnRecalculateGroupCheckboxes(hwnd, dat);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			nm.hdr.code = CLN_CHECKCHANGED;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
		}
		if (!(hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL | CLCHT_ONITEMCHECK))) {
			NMCLISTCONTROL nm;
			nm.hdr.code = NM_CLICK;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			if (hit == -1)
				nm.hItem = NULL;
			else
				nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
			nm.iColumn = hitFlags & CLCHT_ONITEMEXTRA ? HIBYTE(HIWORD(hitFlags)) : -1;
			nm.pt = dat->ptDragStart;
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
		}
		if (hitFlags & (CLCHT_ONITEMCHECK | CLCHT_ONITEMEXTRA))
			break;
		dat->selection = hit;
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		if (dat->selection != -1)
			cli.pfnEnsureVisible(hwnd, dat, hit, 0);
		UpdateWindow(hwnd);
		if (dat->selection != -1 && (contact->type == CLCIT_CONTACT || contact->type == CLCIT_GROUP)
			&& !(hitFlags & (CLCHT_ONITEMEXTRA | CLCHT_ONITEMCHECK))) {
				SetCapture(hwnd);
				dat->iDragItem = dat->selection;
				dat->dragStage = DRAGSTAGE_NOTMOVED;
				dat->dragAutoScrolling = 0;
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->iDragItem == -1) {
			int iOldHotTrack = dat->iHotTrack;
			if (dat->hwndRenameEdit != NULL)
				break;
			if (GetKeyState(VK_MENU) & 0x8000 || GetKeyState(VK_F10) & 0x8000)
				break;
			dat->iHotTrack = cli.pfnHitTest(hwnd, dat, (short) LOWORD(lParam), (short) HIWORD(lParam), NULL, NULL, NULL);
			if (iOldHotTrack != dat->iHotTrack) {
				if (iOldHotTrack == -1)
					SetCapture(hwnd);
				else if (dat->iHotTrack == -1)
					ReleaseCapture();
				if (dat->exStyle & CLS_EX_TRACKSELECT) {
					cli.pfnInvalidateItem(hwnd, dat, iOldHotTrack);
					cli.pfnInvalidateItem(hwnd, dat, dat->iHotTrack);
				}
				cli.pfnHideInfoTip(hwnd, dat);
			}
			KillTimer(hwnd, TIMERID_INFOTIP);
			if (wParam == 0 && dat->hInfoTipItem == NULL) {
				dat->ptInfoTip.x = (short) LOWORD(lParam);
				dat->ptInfoTip.y = (short) HIWORD(lParam);
				SetTimer(hwnd, TIMERID_INFOTIP, dat->infoTipTimeout, NULL);
			}
			break;
		}
		if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_NOTMOVED && !(dat->exStyle & CLS_EX_DISABLEDRAGDROP)) {
			if (abs((short) LOWORD(lParam) - dat->ptDragStart.x) >= GetSystemMetrics(SM_CXDRAG)
				 ||  abs((short) HIWORD(lParam) - dat->ptDragStart.y) >= GetSystemMetrics(SM_CYDRAG))
				dat->dragStage = (dat->dragStage & ~DRAGSTAGEM_STAGE) | DRAGSTAGE_ACTIVE;
		}
		if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
			HCURSOR hNewCursor;
			RECT clRect;
			POINT pt;
			int target;

			GetClientRect(hwnd, &clRect);
			pt.x = (short) LOWORD(lParam);
			pt.y = (short) HIWORD(lParam);
			hNewCursor = LoadCursor(NULL, IDC_NO);
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			if (dat->dragAutoScrolling) {
				KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
				dat->dragAutoScrolling = 0;
			}
			target = cli.pfnGetDropTargetInformation(hwnd, dat, pt);
			if (dat->dragStage & DRAGSTAGEF_OUTSIDE && target != DROPTARGET_OUTSIDE) {

				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, NULL);
				NMCLISTCONTROL nm;
				nm.hdr.code = CLN_DRAGSTOP;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
				dat->dragStage &= ~DRAGSTAGEF_OUTSIDE;
			}
			switch (target) {
			case DROPTARGET_ONSELF:
			case DROPTARGET_ONCONTACT:
				break;
			case DROPTARGET_ONGROUP:
				hNewCursor = LoadCursor(cli.hInst, MAKEINTRESOURCE(IDC_DROPUSER));
				break;
			case DROPTARGET_INSERTION:
				hNewCursor = LoadCursor(cli.hInst, MAKEINTRESOURCE(IDC_DROPUSER));
				break;
			case DROPTARGET_OUTSIDE:
				{
					NMCLISTCONTROL nm;

					if (pt.x >= 0 && pt.x < clRect.right
						&& ((pt.y < 0 && pt.y > -dat->dragAutoScrollHeight)
						||  (pt.y >= clRect.bottom && pt.y < clRect.bottom + dat->dragAutoScrollHeight))) {
							if (!dat->dragAutoScrolling) {
								if (pt.y < 0)
									dat->dragAutoScrolling = -1;
								else
									dat->dragAutoScrolling = 1;
								SetTimer(hwnd, TIMERID_DRAGAUTOSCROLL, dat->scrollTime, NULL);
							}
							SendMessage(hwnd, WM_TIMER, TIMERID_DRAGAUTOSCROLL, 0);
					}

					dat->dragStage |= DRAGSTAGEF_OUTSIDE;
					cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, NULL);
					nm.hdr.code = CLN_DRAGGING;
					nm.hdr.hwndFrom = hwnd;
					nm.hdr.idFrom = GetDlgCtrlID(hwnd);
					nm.flags = 0;
					nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
					nm.pt = pt;
					if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm))
						return 0;
				}
				break;

			default:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, NULL, &group);
				if (group->parent)
					hNewCursor = LoadCursor(cli.hInst, MAKEINTRESOURCE(IDC_DROPUSER));
				break;
			}
			SetCursor(hNewCursor);
		}
		break;

	case WM_LBUTTONUP:
		if (dat->iDragItem == -1)
			break;

		SetCursor((HCURSOR) GetClassLongPtr(hwnd, GCLP_HCURSOR));
		if (dat->exStyle & CLS_EX_TRACKSELECT) {
			dat->iHotTrack = cli.pfnHitTest(hwnd, dat, (short) LOWORD(lParam), (short) HIWORD(lParam), NULL, NULL, NULL);
			if (dat->iHotTrack == -1)
				ReleaseCapture();
		}
		else ReleaseCapture();
		KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
		if (dat->dragStage == (DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME))
			SetTimer(hwnd, TIMERID_RENAME, GetDoubleClickTime(), NULL);
		else if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			int target = cli.pfnGetDropTargetInformation(hwnd, dat, pt);
			switch (target) {
			case DROPTARGET_ONSELF:
			case DROPTARGET_ONCONTACT:
				break;

			case DROPTARGET_ONGROUP:
				{
					ClcContact *contactn, *contacto;
					cli.pfnGetRowByIndex(dat, dat->selection, &contactn, NULL);
					cli.pfnGetRowByIndex(dat, dat->iDragItem, &contacto, NULL);
					if (contacto->type == CLCIT_CONTACT) //dropee is a contact
						CallService(MS_CLIST_CONTACTCHANGEGROUP, (WPARAM)contacto->hContact, contactn->groupId);
					else if (contacto->type == CLCIT_GROUP) { //dropee is a group
						TCHAR szNewName[120];
						mir_sntprintf(szNewName, SIZEOF(szNewName), _T("%s\\%s"), cli.pfnGetGroupName(contactn->groupId, NULL), contacto->szText);
						cli.pfnRenameGroup(contacto->groupId, szNewName);
					}
				}
				break;

			case DROPTARGET_INSERTION:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, NULL);
				{
					ClcContact *destcontact;
					ClcGroup *destgroup;
					if (cli.pfnGetRowByIndex(dat, dat->iInsertionMark, &destcontact, &destgroup) == -1 || destgroup != contact->group->parent)
						CallService(MS_CLIST_GROUPMOVEBEFORE, contact->groupId, 0);
					else {
						if (destcontact->type == CLCIT_GROUP)
							destgroup = destcontact->group;
						CallService(MS_CLIST_GROUPMOVEBEFORE, contact->groupId, destgroup->groupId);
					}
				}
				break;
			case DROPTARGET_OUTSIDE:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, NULL);
				{
					NMCLISTCONTROL nm;
					nm.hdr.code = CLN_DROPPED;
					nm.hdr.hwndFrom = hwnd;
					nm.hdr.idFrom = GetDlgCtrlID(hwnd);
					nm.flags = 0;
					nm.hItem = cli.pfnContactToItemHandle(contact, &nm.flags);
					nm.pt = pt;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM) & nm);
				}
				break;
			default:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, &group);
				if (!group->parent)
					break;
				if (contact->type == CLCIT_GROUP) { //dropee is a group
					TCHAR szNewName[120];
					lstrcpyn(szNewName, contact->szText, SIZEOF(szNewName));
					cli.pfnRenameGroup(contact->groupId, szNewName);
				}
				else if (contact->type == CLCIT_CONTACT) //dropee is a contact
					CallService(MS_CLIST_CONTACTCHANGEGROUP, (WPARAM)contact->hContact, 0);
			}
		}

		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		dat->iDragItem = -1;
		dat->iInsertionMark = -1;
		break;

	case WM_LBUTTONDBLCLK:
		ReleaseCapture();
		dat->iHotTrack = -1;
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);

		dat->selection = cli.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, NULL, &hitFlags);
		cli.pfnInvalidateRect(hwnd, NULL, FALSE);
		if (dat->selection != -1)
			cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
		if (!(hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL)))
			break;

		UpdateWindow(hwnd);
		cli.pfnDoSelectionDefaultAction(hwnd, dat);
		dat->szQuickSearch[0] = 0;
		if (dat->filterSearch)
			cli.pfnSaveStateAndRebuildList(hwnd, dat);
		break;

	case WM_CONTEXTMENU:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);
		if (GetFocus() != hwnd)
			SetFocus(hwnd);
		dat->iHotTrack = -1;
		if (!dat->filterSearch)
			dat->szQuickSearch[0] = 0;
		{
			POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
			if (pt.x == -1 && pt.y == -1) {
				dat->selection = cli.pfnGetRowByIndex(dat, dat->selection, &contact, NULL);
				if (dat->selection != -1)
					cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
				pt.x = dat->iconXSpace + 15;
				pt.y = cli.pfnGetRowTopY(dat, dat->selection) - dat->yScroll + (int)(cli.pfnGetRowHeight(dat, dat->selection) * .7);
				hitFlags = (dat->selection == -1) ? CLCHT_NOWHERE : CLCHT_ONITEMLABEL;
			}
			else {
				ScreenToClient(hwnd, &pt);
				dat->selection = cli.pfnHitTest(hwnd, dat, pt.x, pt.y, &contact, NULL, &hitFlags);
			}
			cli.pfnInvalidateRect(hwnd, NULL, FALSE);
			if (dat->selection != -1)
				cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
			UpdateWindow(hwnd);

			HMENU hMenu = NULL;
			if (dat->selection != -1 && hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMCHECK | CLCHT_ONITEMLABEL)) {
				if (contact->type == CLCIT_GROUP) {
					hMenu = cli.pfnBuildGroupPopupMenu(contact->group);
					ClientToScreen(hwnd, &pt);
					TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
					DestroyMenu(hMenu);
					return 0;
				}
				if (contact->type == CLCIT_CONTACT)
					hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)contact->hContact, 0);
			}
			else {
				//call parent for new group/hide offline menu
				SendMessage(GetParent(hwnd), WM_CONTEXTMENU, wParam, lParam);
			}
			if (hMenu != NULL) {
				ClientToScreen(hwnd, &pt);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
				DestroyMenu(hMenu);
			}
		}
		return 0;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case WM_COMMAND:
		hit = cli.pfnGetRowByIndex(dat, dat->selection, &contact, NULL);
		if (hit == -1)
			break;
		if (contact->type == CLCIT_CONTACT)
			if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)contact->hContact))
				break;
		switch (LOWORD(wParam)) {
		case POPUP_NEWSUBGROUP:
			if (contact->type != CLCIT_GROUP)
				break;
			SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
			CallService(MS_CLIST_GROUPCREATE, contact->groupId, 0);
			break;
		case POPUP_RENAMEGROUP:
			cli.pfnBeginRenameSelection(hwnd, dat);
			break;
		case POPUP_DELETEGROUP:
			if (contact->type != CLCIT_GROUP)
				break;
			CallService(MS_CLIST_GROUPDELETE, contact->groupId, 0);
			break;
		case POPUP_GROUPHIDEOFFLINE:
			if (contact->type != CLCIT_GROUP)
				break;
			CallService(MS_CLIST_GROUPSETFLAGS, contact->groupId, MAKELPARAM(contact->group->hideOffline ? 0 : GROUPF_HIDEOFFLINE, GROUPF_HIDEOFFLINE));
			break;
		}
		break;

	case WM_DESTROY:
		cli.pfnHideInfoTip(hwnd, dat);

		for (int i = 0; i <= FONTID_MAX; i++)
			if (!dat->fontInfo[i].changed)
				DeleteObject(dat->fontInfo[i].hFont);

		if (dat->himlHighlight)
			ImageList_Destroy(dat->himlHighlight);
		if (dat->hwndRenameEdit)
			DestroyWindow(dat->hwndRenameEdit);
		if (!dat->bkChanged && dat->hBmpBackground)
			DeleteObject(dat->hBmpBackground);
		cli.pfnFreeGroup(&dat->list);
		mir_free(dat);
		cli.pfnUnregisterFileDropping(hwnd);
		WindowList_Remove(hClcWindowList, hwnd);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
