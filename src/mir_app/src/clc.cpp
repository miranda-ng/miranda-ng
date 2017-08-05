/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

#include "stdafx.h"
#include "clc.h"
#include "genmenu.h"

void InitCustomMenus(void);
void UninitCustomMenus(void);

void MTG_OnmodulesLoad(void);

static bool bModuleInitialized = false;
static HANDLE hShowInfoTipEvent;
HANDLE hHideInfoTipEvent;
static MWindowList hClcWindowList;

int g_IconWidth, g_IconHeight;

void FreeDisplayNameCache(void);

void fnInitAutoRebuild(HWND hWnd)
{
	if (!cli.bAutoRebuild && hWnd) {
		cli.bAutoRebuild = true;
		PostMessage(hWnd, CLM_AUTOREBUILD, 0, 0);
	}
}

MIR_APP_DLL(void) Clist_Broadcast(int msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_Broadcast(hClcWindowList, msg, wParam, lParam);
}

MIR_APP_DLL(void) Clist_BroadcastAsync(int msg, WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(hClcWindowList, msg, wParam, lParam);
}

void fnClcOptionsChanged(void)
{
	Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// standard CLC services

extern bool g_bGroupsLocked;

static int ClcSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (hContact == 0) {
		if (!strcmp(cws->szModule, "CListGroups")) {
			if (g_bGroupsLocked)
				Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
			else
				Clist_Broadcast(INTM_GROUPSCHANGED, hContact, lParam);
		}
		return 0;
	}

	if (!strcmp(cws->szModule, "CList")) {
		if (!strcmp(cws->szSetting, "MyHandle")) {
			cli.pfnInvalidateDisplayNameCacheEntry(hContact);
			Clist_Broadcast(INTM_NAMECHANGED, hContact, lParam);
		}
		else if (!strcmp(cws->szSetting, "Group"))
			Clist_Broadcast(INTM_GROUPCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "Hidden"))
			Clist_Broadcast(INTM_HIDDENCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "NotOnList"))
			Clist_Broadcast(INTM_NOTONLISTCHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "Status"))
			Clist_Broadcast(INTM_INVALIDATE, 0, 0);
		else if (!strcmp(cws->szSetting, "NameOrder"))
			Clist_Broadcast(INTM_NAMEORDERCHANGED, 0, 0);
	}
	else {
		char *szProto = GetContactProto(hContact);
		if (szProto != nullptr) {
			if (!strcmp(cws->szModule, "Protocol") && !strcmp(cws->szSetting, "p"))
				Clist_Broadcast(INTM_PROTOCHANGED, hContact, lParam);

			// something is being written to a protocol module
			if (!strcmp(szProto, cws->szModule)) {
				// was a unique setting key written?
				char *id = (char *)CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
				if ((INT_PTR)id != CALLSERVICE_NOTFOUND && id != nullptr && !strcmp(id, cws->szSetting))
					Clist_Broadcast(INTM_PROTOCHANGED, hContact, lParam);
			}
		}
		if (szProto == nullptr || strcmp(szProto, cws->szModule))
			return 0;
		if (!strcmp(cws->szSetting, "Nick") || !strcmp(cws->szSetting, "FirstName") || !strcmp(cws->szSetting, "e-mail")
			|| !strcmp(cws->szSetting, "LastName") || !strcmp(cws->szSetting, "UIN"))
			Clist_Broadcast(INTM_NAMECHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "ApparentMode"))
			Clist_Broadcast(INTM_APPARENTMODECHANGED, hContact, lParam);
		else if (!strcmp(cws->szSetting, "IdleTS"))
			Clist_Broadcast(INTM_IDLECHANGED, hContact, lParam);
	}
	return 0;
}

static int ClcAccountsChanged(WPARAM, LPARAM)
{
	int i, cnt;
	for (i = 0, cnt = 0; i < accounts.getCount(); i++)
		if (Proto_IsAccountEnabled(accounts[i]))
			cnt++;

	cli.hClcProtoCount = cnt;
	cli.clcProto = (ClcProtoStatus *)mir_realloc(cli.clcProto, sizeof(ClcProtoStatus) * cli.hClcProtoCount);

	for (i = 0, cnt = 0; i < accounts.getCount(); i++) {
		if (Proto_IsAccountEnabled(accounts[i])) {
			cli.clcProto[cnt].szProto = accounts[i]->szModuleName;
			cli.clcProto[cnt].dwStatus = CallProtoServiceInt(0, accounts[i]->szModuleName, PS_GETSTATUS, 0, 0);
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
	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->type == ACKTYPE_STATUS) {
		WindowList_BroadcastAsync(hClcWindowList, INTM_INVALIDATE, 0, 0);
		if (ack->result == ACKRESULT_SUCCESS) {
			for (int i = 0; i < cli.hClcProtoCount; i++) {
				if (!mir_strcmp(cli.clcProto[i].szProto, ack->szModule)) {
					cli.clcProto[i].dwStatus = (WORD)ack->lParam;
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

static int ClcIconsChanged(WPARAM, LPARAM)
{
	WindowList_BroadcastAsync(hClcWindowList, INTM_INVALIDATE, 0, 0);
	return 0;
}

static INT_PTR SetInfoTipHoverTime(WPARAM wParam, LPARAM)
{
	db_set_w(0, "CLC", "InfoTipHoverTime", (WORD)wParam);
	Clist_Broadcast(INTM_SETINFOTIPHOVERTIME, wParam, 0);
	return 0;
}

static INT_PTR GetInfoTipHoverTime(WPARAM, LPARAM)
{
	return db_get_w(0, "CLC", "InfoTipHoverTime", 750);
}

static void SortClcByTimer(HWND hwnd)
{
	SetTimer(hwnd, TIMERID_DELAYEDRESORTCLC, 200, nullptr);
}

int LoadCLCModule(void)
{
	bModuleInitialized = true;

	g_IconWidth = g_iIconSX;
	g_IconHeight = g_iIconSY;

	hClcWindowList = WindowList_Create();
	hShowInfoTipEvent = CreateHookableEvent(ME_CLC_SHOWINFOTIP);
	hHideInfoTipEvent = CreateHookableEvent(ME_CLC_HIDEINFOTIP);
	CreateServiceFunction(MS_CLC_SETINFOTIPHOVERTIME, SetInfoTipHoverTime);
	CreateServiceFunction(MS_CLC_GETINFOTIPHOVERTIME, GetInfoTipHoverTime);

	InitFileDropping();

	HookEvent(ME_SYSTEM_MODULESLOADED, ClcModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, ClcAccountsChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ClcSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, ClcContactAdded);
	HookEvent(ME_SKIN_ICONSCHANGED, ClcIconsChanged);
	HookEvent(ME_PROTO_ACK, ClcProtoAck);

	InitCustomMenus();
	return 0;
}

void UnloadClcModule()
{
	if (!bModuleInitialized)
		return;

	mir_free(cli.clcProto);
	WindowList_Destroy(hClcWindowList); hClcWindowList = nullptr;

	FreeDisplayNameCache();

	UninitCustomMenus();
	UninitGenMenu();
}

/////////////////////////////////////////////////////////////////////////////////////////
// default contact list control window procedure

LRESULT CALLBACK fnContactListControlWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ClcGroup *group;
	ClcContact *contact;
	DWORD hitFlags;
	int hit;

	ClcData *dat = (ClcData *) GetWindowLongPtr(hwnd, 0);
	if (uMsg >= CLM_FIRST && uMsg < CLM_LAST)
		return cli.pfnProcessExternalMessages(hwnd, dat, uMsg, wParam, lParam);

	switch (uMsg) {
	case WM_CREATE:
		WindowList_Add(hClcWindowList, hwnd, 0);
		cli.pfnRegisterFileDropping(hwnd);
		if (dat == nullptr) {
			dat = new ClcData();
			SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);
		}

		for (int i = 0; i <= FONTID_MAX; i++)
			dat->fontInfo[i].changed = 1;

		dat->selection = -1;
		dat->iconXSpace = 20;
		dat->checkboxSize = 13;
		dat->dragAutoScrollHeight = 30;
		dat->iDragItem = -1;
		dat->iInsertionMark = -1;
		dat->insertionMarkHitHeight = 5;
		dat->iHotTrack = -1;
		dat->infoTipTimeout = db_get_w(0, "CLC", "InfoTipHoverTime", 750);
		dat->extraColumnSpacing = 20;
		dat->bNeedsResort = true;
		cli.pfnLoadClcOptions(hwnd, dat, TRUE);
		if (!IsWindowVisible(hwnd))
			SetTimer(hwnd, TIMERID_REBUILDAFTER, 10, nullptr);
		else {
			cli.pfnRebuildEntireList(hwnd, dat);
			NMCLISTCONTROL nm;
			nm.hdr.code = CLN_LISTREBUILT;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
		}
		break;

	case INTM_SCROLLBARCHANGED:
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CONTACTLIST) {
			if (dat->bNoVScrollbar)
				ShowScrollBar(hwnd, SB_VERT, FALSE);
			else
				cli.pfnRecalcScrollBar(hwnd, dat);
		}
		break;

	case INTM_RELOADOPTIONS:
		cli.pfnLoadClcOptions(hwnd, dat, FALSE);
		cli.pfnSaveStateAndRebuildList(hwnd, dat);
		break;

	case WM_THEMECHANGED:
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
			HBITMAP hBmp = CreateBitmap(rc.right, rc.bottom, 1, depth, nullptr);
			HBITMAP hBmpMask = CreateBitmap(rc.right, rc.bottom, 1, 1, nullptr);
			HDC hdcMem = CreateCompatibleDC(hdc);
			HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
			HBRUSH hBrush = CreateSolidBrush(dat->bUseWindowsColours ? GetSysColor(COLOR_HIGHLIGHT) : dat->selBkColour);
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
			MSG *msg = (MSG *)lParam;
			if (msg->message == WM_KEYDOWN) {
				if (msg->wParam == VK_TAB)
					return 0;
				if (msg->wParam == VK_ESCAPE && dat->hwndRenameEdit == nullptr && dat->szQuickSearch[0] == 0)
					return 0;
			}
			if (msg->message == WM_CHAR) {
				if (msg->wParam == '\t')
					return 0;
				if (msg->wParam == 27 && dat->hwndRenameEdit == nullptr && dat->szQuickSearch[0] == 0)
					return 0;
			}
		}
		return DLGC_WANTMESSAGE;

	case WM_KILLFOCUS:
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
	case WM_SETFOCUS:
	case WM_ENABLE:
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_GETFONT:
		return (LRESULT)dat->fontInfo[FONTID_CONTACTS].hFont;

	case INTM_GROUPSCHANGED:
		{
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (dbcws->value.type == DBVT_ASCIIZ || dbcws->value.type == DBVT_UTF8) {
				int groupId = atoi(dbcws->szSetting) + 1;
				int i, eq;
				//check name of group and ignore message if just being expanded/collapsed
				if (Clist_FindItem(hwnd, dat, groupId | HCONTACT_ISGROUP, &contact, &group, nullptr)) {
					CMStringW szFullName(contact->szText);
					while (group->parent) {
						ClcContact *cc = nullptr;
						for (i = 0; i < group->parent->cl.getCount(); i++) {
							cc = group->parent->cl[i];
							if (cc->group == group)
								break;
						}
						if (i == group->parent->cl.getCount()) {
							szFullName.Empty();
							break;
						}
						szFullName = CMStringW(cc->szText) + L"\\" + szFullName;
						group = group->parent;
					}

					if (dbcws->value.type == DBVT_ASCIIZ)
						eq = !mir_wstrcmp(szFullName, _A2T(dbcws->value.pszVal+1));
					else
						eq = !mir_wstrcmp(szFullName, ptrW(mir_utf8decodeW(dbcws->value.pszVal + 1)));

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
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN)
				break;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0) {
				if (Clist_FindItem(hwnd, dat, wParam, nullptr, nullptr, nullptr))
					break;
				cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				cli.pfnNotifyNewContact(hwnd, wParam);
			}
			else cli.pfnDeleteItemFromTree(hwnd, wParam);

			dat->bNeedsResort = true;
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_GROUPCHANGED:
		{
			WORD iExtraImage[EXTRA_ICON_COUNT];
			BYTE flags = 0;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr))
				memset(iExtraImage, 0xFF, sizeof(iExtraImage));
			else {
				memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
				flags = contact->flags;
			}
			cli.pfnDeleteItemFromTree(hwnd, wParam);
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !db_get_b(wParam, "CList", "Hidden", 0)) {
				NMCLISTCONTROL nm;
				cli.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				if (Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr)) {
					memcpy(contact->iExtraImage, iExtraImage, sizeof(iExtraImage));
					if (flags & CONTACTF_CHECKED)
						contact->flags |= CONTACTF_CHECKED;
				}
				nm.hdr.code = CLN_CONTACTMOVED;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = (HANDLE)wParam;
				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
				dat->bNeedsResort = true;
			}
		}
		SetTimer(hwnd, TIMERID_REBUILDAFTER, 1, nullptr);
		break;

	case INTM_ICONCHANGED:
		{
			WORD status;
			MCONTACT hSelItem = 0;
			ClcContact *selcontact = nullptr;

			char *szProto = GetContactProto(wParam);
			if (szProto == nullptr)
				status = ID_STATUS_OFFLINE;
			else
				status = db_get_w(wParam, szProto, "Status", ID_STATUS_OFFLINE);

			// this means an offline msg is flashing, so the contact should be shown
			DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
			int shouldShow = (style & CLS_SHOWHIDDEN || !db_get_b(wParam, "CList", "Hidden", 0))
				&& (!cli.pfnIsHiddenMode(dat, status) || cli.pfnGetContactIcon(wParam) != lParam);

			contact = nullptr;
			group = nullptr;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group, nullptr)) {
				if (shouldShow && db_is_contact(wParam)) {
					if (dat->selection >= 0 && cli.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					cli.pfnAddContactToTree(hwnd, dat, wParam, (style & CLS_CONTACTLIST) == 0, 0);
					Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr);
					if (contact) {
						contact->iImage = (WORD)lParam;
						cli.pfnNotifyNewContact(hwnd, wParam);
						dat->bNeedsResort = true;
					}
				}
			}
			else { // item in list already
				if (contact->iImage == (WORD)lParam)
					break;
				if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					if (dat->selection >= 0 && cli.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					cli.pfnRemoveItemFromGroup(hwnd, group, contact, (style & CLS_CONTACTLIST) == 0);
				}
				else {
					contact->iImage = (WORD)lParam;
					if (!cli.pfnIsHiddenMode(dat, status))
						contact->flags |= CONTACTF_ONLINE;
					else
						contact->flags &= ~CONTACTF_ONLINE;
				}
				dat->bNeedsResort = true;
			}
			if (hSelItem) {
				ClcGroup *selgroup;
				if (Clist_FindItem(hwnd, dat, hSelItem, &selcontact, &selgroup, nullptr))
					dat->selection = cli.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
				else
					dat->selection = -1;
			}
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_NAMECHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr))
			break;

		mir_wstrncpy(contact->szText, cli.pfnGetContactDisplayName(wParam, 0), _countof(contact->szText));
		dat->bNeedsResort = true;
		SortClcByTimer(hwnd);
		break;

	case INTM_PROTOCHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr))
			break;

		contact->proto = GetContactProto(wParam);
		cli.pfnInvalidateDisplayNameCacheEntry(wParam);
		mir_wstrncpy(contact->szText, cli.pfnGetContactDisplayName(wParam, 0), _countof(contact->szText));
		SortClcByTimer(hwnd);
		break;

	case INTM_NOTONLISTCHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr))
			break;

		if (contact->type == CLCIT_CONTACT) {
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0)
				contact->flags &= ~CONTACTF_NOTONLIST;
			else
				contact->flags |= CONTACTF_NOTONLIST;
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case INTM_INVALIDATE:
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case INTM_APPARENTMODECHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr)) {
			char *szProto = GetContactProto(wParam);
			if (szProto == nullptr)
				break;

			WORD apparentMode = db_get_w(wParam, szProto, "ApparentMode", 0);
			contact->flags &= ~(CONTACTF_INVISTO | CONTACTF_VISTO);
			if (apparentMode == ID_STATUS_OFFLINE)
				contact->flags |= CONTACTF_INVISTO;
			else if (apparentMode == ID_STATUS_ONLINE)
				contact->flags |= CONTACTF_VISTO;
			else if (apparentMode)
				contact->flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case INTM_SETINFOTIPHOVERTIME:
		dat->infoTipTimeout = wParam;
		break;

	case INTM_IDLECHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact, nullptr, nullptr)) {
			char *szProto = GetContactProto(wParam);
			if (szProto == nullptr)
				break;
			contact->flags &= ~CONTACTF_IDLE;
			if (db_get_dw(wParam, szProto, "IdleTS", 0))
				contact->flags |= CONTACTF_IDLE;

			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case WM_PRINTCLIENT:
		cli.pfnPaintClc(hwnd, dat, (HDC)wParam, nullptr);
		break;

	case WM_NCPAINT:
		if (wParam != 1) {
			POINT ptTopLeft = { 0, 0 };
			HRGN hClientRgn;
			ClientToScreen(hwnd, &ptTopLeft);
			hClientRgn = CreateRectRgn(0, 0, 1, 1);
			CombineRgn(hClientRgn, (HRGN)wParam, nullptr, RGN_COPY);
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

		UINT scrollLines;
		if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
			scrollLines = 3;
		cli.pfnScrollTo(hwnd, dat, dat->yScroll - (short)HIWORD(wParam) * dat->rowHeight * (signed)scrollLines / WHEEL_DELTA, 0);
		return 0;

	case WM_KEYDOWN:
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		if (!Clist_MenuProcessHotkey(wParam)) {
			RECT clRect;
			GetClientRect(hwnd, &clRect);
			int pageSize = clRect.bottom / dat->rowHeight;
			int selMoved = 0, changeGroupExpand = 0;

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
				return 0;

			case VK_F2:     cli.pfnBeginRenameSelection(hwnd, dat); return 0;
			case VK_DELETE: cli.pfnDeleteFromContactList(hwnd, dat); return 0;

			default:
				NMKEY nmkey;
				nmkey.hdr.hwndFrom = hwnd;
				nmkey.hdr.idFrom = GetDlgCtrlID(hwnd);
				nmkey.hdr.code = NM_KEYDOWN;
				nmkey.nVKey = wParam;
				nmkey.uFlags = HIWORD(lParam);
				if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nmkey))
					return 0;
			}
			if (changeGroupExpand) {
				if (!dat->bFilterSearch)
					dat->szQuickSearch[0] = 0;
				hit = cli.pfnGetRowByIndex(dat, dat->selection, &contact, &group);
				if (hit == -1) 
					return 0;

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
			if (selMoved) {
				if (!dat->bFilterSearch)
					dat->szQuickSearch[0] = 0;
				if (dat->selection >= cli.pfnGetGroupContentsCount(&dat->list, 1))
					dat->selection = cli.pfnGetGroupContentsCount(&dat->list, 1) - 1;
				if (dat->selection < 0)
					dat->selection = 0;
				cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
			dat->szQuickSearch[mir_wstrlen(dat->szQuickSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else if (wParam == ' ' && dat->szQuickSearch[0] == '\0' && GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CHECKBOXES) {
			NMCLISTCONTROL nm;
			if (cli.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) == -1)
				break;
			if (contact->type != CLCIT_CONTACT)
				break;
			contact->flags ^= CONTACTF_CHECKED;
			if (contact->type == CLCIT_GROUP)
				cli.pfnSetGroupChildCheckboxes(contact->group, contact->flags & CONTACTF_CHECKED);
			cli.pfnRecalculateGroupCheckboxes(hwnd, dat);
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			nm.hdr.code = CLN_CHECKCHANGED;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
		}
		else {
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			if (mir_wstrlen(dat->szQuickSearch) >= _countof(dat->szQuickSearch) - 1) {
				MessageBeep(MB_OK);
				break;
			}
			mir_wstrcat(dat->szQuickSearch, szNew);
		}

		if (dat->bFilterSearch)
			cli.pfnSaveStateAndRebuildList(hwnd, dat);

		if (dat->szQuickSearch[0]) {
			int index;
			index = cli.pfnFindRowByText(hwnd, dat, dat->szQuickSearch, 1);
			if (index != -1)
				dat->selection = index;
			else {
				MessageBeep(MB_OK);
				dat->szQuickSearch[mir_wstrlen(dat->szQuickSearch) - 1] = '\0';
				cli.pfnSaveStateAndRebuildList(hwnd, dat);
			}
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
		}
		else
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_SYSKEYDOWN:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		dat->iHotTrack = -1;
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		ReleaseCapture();
		if (wParam == VK_F10 && GetKeyState(VK_SHIFT) & 0x8000)
			break;
		SendMessage(GetParent(hwnd), uMsg, wParam, lParam);
		return 0;

	case WM_TIMER:
		switch (wParam) {
		case TIMERID_RENAME:
			cli.pfnBeginRenameSelection(hwnd, dat);
			break;

		case TIMERID_DRAGAUTOSCROLL:
			cli.pfnScrollTo(hwnd, dat, dat->yScroll + dat->dragAutoScrolling * dat->rowHeight * 2, 0);
			break;

		case TIMERID_INFOTIP:
			KillTimer(hwnd, wParam);
			{
				CLCINFOTIP it;
				GetCursorPos(&it.ptCursor);
				ScreenToClient(hwnd, &it.ptCursor);
				if (it.ptCursor.x != dat->ptInfoTip.x || it.ptCursor.y != dat->ptInfoTip.y)
					break;

				RECT clRect;
				GetClientRect(hwnd, &clRect);

				it.rcItem.left = 0;
				it.rcItem.right = clRect.right;

				hit = cli.pfnHitTest(hwnd, dat, it.ptCursor.x, it.ptCursor.y, &contact, nullptr, nullptr);
				if (hit == -1)
					break;
				if (contact->type != CLCIT_GROUP && contact->type != CLCIT_CONTACT)
					break;

				ClientToScreen(hwnd, &it.ptCursor);

				POINT ptClientOffset = { 0 };
				ClientToScreen(hwnd, &ptClientOffset);
				it.isTreeFocused = GetFocus() == hwnd;
				it.rcItem.top = cli.pfnGetRowTopY(dat, hit) - dat->yScroll;
				it.rcItem.bottom = it.rcItem.top + cli.pfnGetRowHeight(dat, hit);
				OffsetRect(&it.rcItem, ptClientOffset.x, ptClientOffset.y);
				it.isGroup = contact->type == CLCIT_GROUP;
				it.hItem = (contact->type == CLCIT_GROUP) ? (HANDLE)contact->groupId : (HANDLE)contact->hContact;
				it.cbSize = sizeof(it);
				dat->hInfoTipItem = Clist_ContactToHItem(contact);
				NotifyEventHooks(hShowInfoTipEvent, 0, (LPARAM)&it);
			}
			break;

		case TIMERID_REBUILDAFTER:
			KillTimer(hwnd, TIMERID_REBUILDAFTER);
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			cli.pfnSaveStateAndRebuildList(hwnd, dat);
			cli.bAutoRebuild = false;
			break;

		case TIMERID_DELAYEDRESORTCLC:
			KillTimer(hwnd, TIMERID_DELAYEDRESORTCLC);
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
		dat->ptDragStart.x = (short)LOWORD(lParam);
		dat->ptDragStart.y = (short)HIWORD(lParam);
		if (!dat->bFilterSearch)
			dat->szQuickSearch[0] = 0;

		hit = cli.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, &group, &hitFlags);
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
						cli.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
					if (dat->selection == -1)
						dat->selection = cli.pfnGetRowsPriorTo(&dat->list, contact->group, -1);
				}
				cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			nm.hdr.code = CLN_CHECKCHANGED;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
		}
		if (!(hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL | CLCHT_ONITEMCHECK))) {
			NMCLISTCONTROL nm;
			nm.hdr.code = NM_CLICK;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			if (hit == -1)
				nm.hItem = nullptr;
			else
				nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
			nm.iColumn = hitFlags & CLCHT_ONITEMEXTRA ? HIBYTE(HIWORD(hitFlags)) : -1;
			nm.pt = dat->ptDragStart;
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
		}
		if (hitFlags & (CLCHT_ONITEMCHECK | CLCHT_ONITEMEXTRA))
			break;
		dat->selection = hit;
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		if (dat->selection != -1)
			cli.pfnEnsureVisible(hwnd, dat, hit, 0);
		UpdateWindow(hwnd);
		if (dat->selection != -1 && (contact->type == CLCIT_CONTACT || contact->type == CLCIT_GROUP) && !(hitFlags & (CLCHT_ONITEMEXTRA | CLCHT_ONITEMCHECK))) {
			SetCapture(hwnd);
			dat->iDragItem = dat->selection;
			dat->dragStage = DRAGSTAGE_NOTMOVED;
			dat->dragAutoScrolling = 0;
		}
		break;

	case WM_MOUSEMOVE:
		if (dat->iDragItem == -1) {
			int iOldHotTrack = dat->iHotTrack;
			if (dat->hwndRenameEdit != nullptr)
				break;
			if (GetKeyState(VK_MENU) & 0x8000 || GetKeyState(VK_F10) & 0x8000)
				break;
			dat->iHotTrack = cli.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, nullptr);
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
			if (wParam == 0 && dat->hInfoTipItem == 0) {
				dat->ptInfoTip.x = (short)LOWORD(lParam);
				dat->ptInfoTip.y = (short)HIWORD(lParam);
				SetTimer(hwnd, TIMERID_INFOTIP, dat->infoTipTimeout, nullptr);
			}
			break;
		}
		if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_NOTMOVED && !(dat->exStyle & CLS_EX_DISABLEDRAGDROP)) {
			if (abs((short)LOWORD(lParam) - dat->ptDragStart.x) >= GetSystemMetrics(SM_CXDRAG)
				|| abs((short)HIWORD(lParam) - dat->ptDragStart.y) >= GetSystemMetrics(SM_CYDRAG))
				dat->dragStage = (dat->dragStage & ~DRAGSTAGEM_STAGE) | DRAGSTAGE_ACTIVE;
		}
		if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
			RECT clRect;
			GetClientRect(hwnd, &clRect);

			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);

			HCURSOR hNewCursor = LoadCursor(nullptr, IDC_NO);
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			if (dat->dragAutoScrolling) {
				KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
				dat->dragAutoScrolling = 0;
			}
			int target = cli.pfnGetDropTargetInformation(hwnd, dat, pt);
			if (dat->dragStage & DRAGSTAGEF_OUTSIDE && target != DROPTARGET_OUTSIDE) {
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

				NMCLISTCONTROL nm;
				nm.hdr.code = CLN_DRAGSTOP;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
				SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
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
				if (pt.x >= 0 && pt.x < clRect.right
					&& ((pt.y < 0 && pt.y > -dat->dragAutoScrollHeight)
					|| (pt.y >= clRect.bottom && pt.y < clRect.bottom + dat->dragAutoScrollHeight))) {
					if (!dat->dragAutoScrolling) {
						if (pt.y < 0)
							dat->dragAutoScrolling = -1;
						else
							dat->dragAutoScrolling = 1;
						SetTimer(hwnd, TIMERID_DRAGAUTOSCROLL, dat->scrollTime, nullptr);
					}
					SendMessage(hwnd, WM_TIMER, TIMERID_DRAGAUTOSCROLL, 0);
				}

				dat->dragStage |= DRAGSTAGEF_OUTSIDE;
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

				NMCLISTCONTROL nm;
				nm.hdr.code = CLN_DRAGGING;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
				nm.pt = pt;
				if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm))
					return 0;
				break;

			default:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, nullptr, &group);
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

		SetCursor((HCURSOR)GetClassLongPtr(hwnd, GCLP_HCURSOR));
		if (dat->exStyle & CLS_EX_TRACKSELECT) {
			dat->iHotTrack = cli.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, nullptr);
			if (dat->iHotTrack == -1)
				ReleaseCapture();
		}
		else ReleaseCapture();
		KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
		if (dat->dragStage == (DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME))
			SetTimer(hwnd, TIMERID_RENAME, GetDoubleClickTime(), nullptr);
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
					cli.pfnGetRowByIndex(dat, dat->selection, &contactn, nullptr);
					cli.pfnGetRowByIndex(dat, dat->iDragItem, &contacto, nullptr);
					if (contacto->type == CLCIT_CONTACT) //dropee is a contact
						Clist_ContactChangeGroup(contacto->hContact, contactn->groupId);
					else if (contacto->type == CLCIT_GROUP) { //dropee is a group
						wchar_t szNewName[120];
						mir_snwprintf(szNewName, L"%s\\%s", Clist_GroupGetName(contactn->groupId, nullptr), contacto->szText);
						Clist_GroupRename(contacto->groupId, szNewName);
					}
				}
				break;

			case DROPTARGET_INSERTION:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);
				{
					ClcContact *destcontact;
					ClcGroup *destgroup;
					if (cli.pfnGetRowByIndex(dat, dat->iInsertionMark, &destcontact, &destgroup) == -1 || destgroup != contact->group->parent)
						Clist_GroupMoveBefore(contact->groupId, 0);
					else {
						if (destcontact->type == CLCIT_GROUP)
							destgroup = destcontact->group;
						Clist_GroupMoveBefore(contact->groupId, destgroup->groupId);
					}
				}
				break;
			case DROPTARGET_OUTSIDE:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);
				{
					NMCLISTCONTROL nm;
					nm.hdr.code = CLN_DROPPED;
					nm.hdr.hwndFrom = hwnd;
					nm.hdr.idFrom = GetDlgCtrlID(hwnd);
					nm.flags = 0;
					nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
					nm.pt = pt;
					SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)& nm);
				}
				break;
			default:
				cli.pfnGetRowByIndex(dat, dat->iDragItem, &contact, &group);
				if (!group->parent)
					break;
				if (contact->type == CLCIT_GROUP) { //dropee is a group
					wchar_t szNewName[120];
					mir_wstrncpy(szNewName, contact->szText, _countof(szNewName));
					Clist_GroupRename(contact->groupId, szNewName);
				}
				else if (contact->type == CLCIT_CONTACT) //dropee is a contact
					Clist_ContactChangeGroup(contact->hContact, 0);
			}
		}

		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		dat->iDragItem = -1;
		dat->iInsertionMark = -1;
		break;

	case WM_LBUTTONDBLCLK:
		ReleaseCapture();
		dat->iHotTrack = -1;
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);

		dat->selection = cli.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, nullptr, &hitFlags);
		cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
		if (dat->selection != -1)
			cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
		
		if (hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL)) {
			UpdateWindow(hwnd);
			cli.pfnDoSelectionDefaultAction(hwnd, dat);
		}
		break;

	case WM_CONTEXTMENU:
		cli.pfnEndRename(hwnd, dat, 1);
		cli.pfnHideInfoTip(hwnd, dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);
		if (GetFocus() != hwnd)
			SetFocus(hwnd);
		dat->iHotTrack = -1;
		if (!dat->bFilterSearch)
			dat->szQuickSearch[0] = 0;
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (pt.x == -1 && pt.y == -1) {
				dat->selection = cli.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
				if (dat->selection != -1)
					cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
				pt.x = dat->iconXSpace + 15;
				pt.y = cli.pfnGetRowTopY(dat, dat->selection) - dat->yScroll + (int)(cli.pfnGetRowHeight(dat, dat->selection) * .7);
				hitFlags = (dat->selection == -1) ? CLCHT_NOWHERE : CLCHT_ONITEMLABEL;
			}
			else {
				ScreenToClient(hwnd, &pt);
				dat->selection = cli.pfnHitTest(hwnd, dat, pt.x, pt.y, &contact, nullptr, &hitFlags);
			}
			cli.pfnInvalidateRect(hwnd, nullptr, FALSE);
			if (dat->selection != -1)
				cli.pfnEnsureVisible(hwnd, dat, dat->selection, 0);
			UpdateWindow(hwnd);

			if (dat->selection != -1 && hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMCHECK | CLCHT_ONITEMLABEL)) {
				HMENU hMenu;
				if (contact->type == CLCIT_GROUP)
					hMenu = Menu_BuildSubGroupMenu(contact->group);
				else if (contact->type == CLCIT_CONTACT)
					hMenu = Menu_BuildContactMenu(contact->hContact);
				else
					return 0;

				ClientToScreen(hwnd, &pt);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
				DestroyMenu(hMenu);
			}
			else  //call parent for new group/hide offline menu
				SendMessage(GetParent(hwnd), WM_CONTEXTMENU, wParam, lParam);
		}
		return 0;

	case WM_MEASUREITEM:
		return Menu_MeasureItem(lParam);

	case WM_DRAWITEM:
		return Menu_DrawItem(lParam);

	case WM_COMMAND:
		hit = cli.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
		if (hit == -1)
			break;
		
		if (contact->type == CLCIT_CONTACT)
			if (Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, contact->hContact))
				break;

		if (contact->type == CLCIT_GROUP) {
			switch (LOWORD(wParam)) {
			case POPUP_NEWSUBGROUP:
				SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~CLS_HIDEEMPTYGROUPS);
				SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | CLS_USEGROUPS);
				Clist_GroupCreate(contact->groupId, 0);
				break;
			case POPUP_RENAMEGROUP:
				cli.pfnBeginRenameSelection(hwnd, dat);
				break;
			case POPUP_DELETEGROUP:
				Clist_GroupDelete(contact->groupId);
				break;
			case POPUP_GROUPHIDEOFFLINE:
				Clist_GroupSetFlags(contact->groupId, MAKELPARAM(contact->group->hideOffline ? 0 : GROUPF_HIDEOFFLINE, GROUPF_HIDEOFFLINE));
				break;
			}

			Menu_ProcessCommandById(wParam, (LPARAM)hwnd);
		}
		break;

	case WM_DESTROY:
		cli.pfnHideInfoTip(hwnd, dat);

		for (int i = 0; i <= FONTID_MAX; i++)
			if (!dat->fontInfo[i].changed)
				DeleteObject(dat->fontInfo[i].hFont);

		if (dat->himlHighlight) {
			ImageList_Destroy(dat->himlHighlight);
			dat->himlHighlight = nullptr;
		}
		if (dat->hwndRenameEdit)
			DestroyWindow(dat->hwndRenameEdit);
		if (dat->hBmpBackground)
			DeleteObject(dat->hBmpBackground);
		cli.pfnFreeGroup(&dat->list);
		delete dat;
		cli.pfnUnregisterFileDropping(hwnd);
		WindowList_Remove(hClcWindowList, hwnd);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
