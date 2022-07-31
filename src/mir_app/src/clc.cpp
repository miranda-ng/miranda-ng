/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

static bool bModuleInitialized = false;
static HANDLE hShowInfoTipEvent;
HANDLE hHideInfoTipEvent;
static MWindowList hClcWindowList;

int g_IconWidth, g_IconHeight;

void FreeDisplayNameCache(void);

MIR_APP_DLL(void) Clist_InitAutoRebuild(HWND hWnd)
{
	if (!g_clistApi.bAutoRebuild && hWnd) {
		g_clistApi.bAutoRebuild = true;
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

MIR_APP_DLL(void) Clist_ClcOptionsChanged(void)
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
			g_clistApi.pfnInvalidateDisplayNameCacheEntry(hContact);
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
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (szProto != nullptr) {
			if (!strcmp(cws->szModule, "Protocol") && !strcmp(cws->szSetting, "p"))
				Clist_Broadcast(INTM_PROTOCHANGED, hContact, lParam);

			// something is being written to a protocol module
			if (!strcmp(szProto, cws->szModule)) {
				// was a unique setting key written?
				const char *id = Proto_GetUniqueId(szProto);
				if (id != nullptr && !strcmp(id, cws->szSetting))
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

static int ClcProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *)lParam;
	if (ack->type == ACKTYPE_STATUS) {
		if (ack->result == ACKRESULT_SUCCESS) {
			PROTOACCOUNT *pa = Proto_GetAccount(ack->szModule);
			if (pa)
				pa->iRealStatus = ack->lParam;
		}
		
		g_clistApi.pfnCluiProtocolStatusChanged(lParam, ack->szModule);

		if (ack->result == ACKRESULT_SUCCESS)
			Clist_TrayIconUpdateBase(ack->szModule);

		if ((INT_PTR)ack->hProcess < ID_STATUS_ONLINE && ack->lParam >= ID_STATUS_ONLINE) {
			// if we're going offline, kill all contacts scheduled for deletion
			uint32_t caps = (uint32_t)CallProtoServiceInt(0, ack->szModule, PS_GETCAPS, PFLAGNUM_1, 0);
			if (caps & PF1_SERVERCLIST) {
				for (MCONTACT hContact = db_find_first(ack->szModule); hContact; ) {
					MCONTACT hNext = db_find_next(hContact, ack->szModule);
					if (db_get_b(hContact, "CList", "Delete", 0))
						db_delete_contact(hContact);
					hContact = hNext;
				}
			}
		}

		WindowList_BroadcastAsync(hClcWindowList, INTM_INVALIDATE, 0, 0);
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
	db_set_w(0, "CLC", "InfoTipHoverTime", (uint16_t)wParam);
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
	uint32_t hitFlags;
	int hit;

	ClcData *dat = (ClcData *)GetWindowLongPtr(hwnd, 0);
	if (uMsg >= CLM_FIRST && uMsg < CLM_LAST)
		return g_clistApi.pfnProcessExternalMessages(hwnd, dat, uMsg, wParam, lParam);

	switch (uMsg) {
	case WM_CREATE:
		WindowList_Add(hClcWindowList, hwnd);
		RegisterFileDropping(hwnd);
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
		g_clistApi.pfnLoadClcOptions(hwnd, dat, TRUE);
		if (!IsWindowVisible(hwnd))
			SetTimer(hwnd, TIMERID_REBUILDAFTER, 10, nullptr);
		else {
			g_clistApi.pfnRebuildEntireList(hwnd, dat);
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
				g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		}
		break;

	case INTM_RELOADOPTIONS:
		g_clistApi.pfnLoadClcOptions(hwnd, dat, FALSE);
		Clist_SaveStateAndRebuildList(hwnd, dat);
		break;

	case WM_THEMECHANGED:
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_SIZE:
		Clist_EndRename(dat, 1);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
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
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_GETFONT:
		return (LRESULT)dat->fontInfo[FONTID_CONTACTS].hFont;

	case INTM_GROUPSCHANGED:
		{
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (dbcws->value.type == DBVT_ASCIIZ || dbcws->value.type == DBVT_UTF8) {
				int groupId = atoi(dbcws->szSetting) + 1;

				// check name of group and ignore message if just being expanded/collapsed
				if (Clist_FindItem(hwnd, dat, groupId | HCONTACT_ISGROUP, &contact, &group)) {
					CMStringW szFullName(contact->szText);
					while (group->parent) {
						ClcContact *cc = nullptr;
						for (auto &it : group->parent->cl)
							if (it->group == group) {
								cc = it;
								break;
							}

						if (cc == nullptr) {
							szFullName.Empty();
							break;
						}
						szFullName = CMStringW(cc->szText) + L"\\" + szFullName;
						group = group->parent;
					}

					int eq;
					if (dbcws->value.type == DBVT_ASCIIZ)
						eq = !mir_wstrcmp(szFullName, _A2T(dbcws->value.pszVal + 1));
					else
						eq = !mir_wstrcmp(szFullName, ptrW(mir_utf8decodeW(dbcws->value.pszVal + 1)));

					if (eq && (contact->group->hideOffline != 0) == ((dbcws->value.pszVal[0] & GROUPF_HIDEOFFLINE) != 0))
						break;  //only expanded has changed: no action reqd
				}
			}
			Clist_SaveStateAndRebuildList(hwnd, dat);
		}
		break;

	case INTM_NAMEORDERCHANGED:
		Clist_InitAutoRebuild(hwnd);
		break;

	case INTM_CONTACTADDED:
		g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
		Clist_NotifyNewContact(hwnd, wParam);
		SortClcByTimer(hwnd);
		break;

	case INTM_CONTACTDELETED:
		Clist_DeleteItemFromTree(hwnd, wParam);
		SortClcByTimer(hwnd);
		break;

	case INTM_HIDDENCHANGED:
		{
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN)
				break;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0) {
				if (Clist_FindItem(hwnd, dat, wParam, nullptr))
					break;
				g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				Clist_NotifyNewContact(hwnd, wParam);
			}
			else Clist_DeleteItemFromTree(hwnd, wParam);

			dat->bNeedsResort = true;
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_GROUPCHANGED:
		{
			uint16_t iExtraImage[EXTRA_ICON_COUNT];
			uint8_t flags = 0;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact))
				memset(iExtraImage, 0xFF, sizeof(iExtraImage));
			else {
				memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
				flags = contact->flags;
			}
			Clist_DeleteItemFromTree(hwnd, wParam);
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !Contact::IsHidden(wParam)) {
				NMCLISTCONTROL nm;
				g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
				if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
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
			uint16_t status;
			MCONTACT hSelItem = 0;
			ClcContact *selcontact = nullptr;

			char *szProto = Proto_GetBaseAccountName(wParam);
			if (szProto == nullptr)
				status = ID_STATUS_OFFLINE;
			else
				status = db_get_w(wParam, szProto, "Status", ID_STATUS_OFFLINE);

			// this means an offline msg is flashing, so the contact should be shown
			uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
			int shouldShow = (style & CLS_SHOWHIDDEN || !Contact::IsHidden(wParam))
				&& (!Clist_IsHiddenMode(dat, status) || Clist_GetContactIcon(wParam) != lParam);

			contact = nullptr;
			group = nullptr;
			if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group)) {
				if (shouldShow && db_is_contact(wParam)) {
					if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, (style & CLS_CONTACTLIST) == 0, 0);
					Clist_FindItem(hwnd, dat, wParam, &contact);
					if (contact) {
						contact->iImage = (uint16_t)lParam;
						Clist_NotifyNewContact(hwnd, wParam);
						dat->bNeedsResort = true;
					}
				}
			}
			else { // item in list already
				if (contact->iImage == (uint16_t)lParam)
					break;
				if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
						hSelItem = Clist_ContactToHItem(selcontact);
					Clist_RemoveItemFromGroup(hwnd, group, contact, (style & CLS_CONTACTLIST) == 0);
				}
				else {
					contact->iImage = (uint16_t)lParam;
					if (!Clist_IsHiddenMode(dat, status))
						contact->flags |= CONTACTF_ONLINE;
					else
						contact->flags &= ~CONTACTF_ONLINE;
				}
				dat->bNeedsResort = true;
			}
			if (hSelItem) {
				ClcGroup *selgroup;
				if (Clist_FindItem(hwnd, dat, hSelItem, &selcontact, &selgroup))
					dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
				else
					dat->selection = -1;
			}
			SortClcByTimer(hwnd);
		}
		break;

	case INTM_NAMECHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		mir_wstrncpy(contact->szText, Clist_GetContactDisplayName(wParam), _countof(contact->szText));
		dat->bNeedsResort = true;
		SortClcByTimer(hwnd);
		break;

	case INTM_PROTOCHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		contact->pce->szProto = Proto_GetBaseAccountName(wParam);
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(wParam);
		mir_wstrncpy(contact->szText, Clist_GetContactDisplayName(wParam), _countof(contact->szText));
		SortClcByTimer(hwnd);
		break;

	case INTM_NOTONLISTCHANGED:
		if (!Clist_FindItem(hwnd, dat, wParam, &contact))
			break;

		if (contact->type == CLCIT_CONTACT) {
			DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING *)lParam;
			if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0)
				contact->flags &= ~CONTACTF_NOTONLIST;
			else
				contact->flags |= CONTACTF_NOTONLIST;
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case INTM_INVALIDATE:
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case INTM_APPARENTMODECHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			char *szProto = Proto_GetBaseAccountName(wParam);
			if (szProto == nullptr)
				break;

			uint16_t apparentMode = db_get_w(wParam, szProto, "ApparentMode", 0);
			contact->flags &= ~(CONTACTF_INVISTO | CONTACTF_VISTO);
			if (apparentMode == ID_STATUS_OFFLINE)
				contact->flags |= CONTACTF_INVISTO;
			else if (apparentMode == ID_STATUS_ONLINE)
				contact->flags |= CONTACTF_VISTO;
			else if (apparentMode)
				contact->flags |= CONTACTF_VISTO | CONTACTF_INVISTO;
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case INTM_SETINFOTIPHOVERTIME:
		dat->infoTipTimeout = wParam;
		break;

	case INTM_IDLECHANGED:
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			char *szProto = Proto_GetBaseAccountName(wParam);
			if (szProto == nullptr)
				break;
			contact->flags &= ~CONTACTF_IDLE;
			if (db_get_dw(wParam, szProto, "IdleTS", 0))
				contact->flags |= CONTACTF_IDLE;

			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		}
		break;

	case WM_PRINTCLIENT:
		g_clistApi.pfnPaintClc(hwnd, dat, (HDC)wParam, nullptr);
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
			/* we get so many g_clistApi.pfnInvalidateRect()'s that there is no point painting,
			Windows in theory shouldn't queue up WM_PAINTs in this case but it does so
			we'll just ignore them */
			if (IsWindowVisible(hwnd))
				g_clistApi.pfnPaintClc(hwnd, dat, hdc, &ps.rcPaint);
			EndPaint(hwnd, &ps);
		}
		break;

	case WM_VSCROLL:
		Clist_EndRename(dat, 1);
		Clist_HideInfoTip(dat);
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
			g_clistApi.pfnScrollTo(hwnd, dat, desty, noSmooth);
		}
		break;

	case WM_MOUSEWHEEL:
		Clist_EndRename(dat, 1);
		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);

		UINT scrollLines;
		if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scrollLines, FALSE))
			scrollLines = 3;
		g_clistApi.pfnScrollTo(hwnd, dat, dat->yScroll - (short)HIWORD(wParam) * dat->rowHeight * (signed)scrollLines / WHEEL_DELTA, 0);
		return 0;

	case WM_KEYDOWN:
		Clist_HideInfoTip(dat);
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
			case VK_END:    dat->selection = g_clistApi.pfnGetGroupContentsCount(&dat->list, 1) - 1; selMoved = 1; break;
			case VK_LEFT:   changeGroupExpand = 1; break;
			case VK_RIGHT:  changeGroupExpand = 2; break;

			case VK_RETURN:
				Clist_DoSelectionDefaultAction(hwnd, dat);
				return 0;

			case VK_F2:     g_clistApi.pfnBeginRenameSelection(hwnd, dat); return 0;
			case VK_DELETE: Clist_DeleteFromContactList(hwnd, dat); return 0;

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
				hit = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, &group);
				if (hit == -1)
					return 0;

				if (changeGroupExpand == 1) {
					switch (contact->type) {
					case CLCIT_GROUP:
						if (contact->group->expanded)
							break;
						__fallthrough;
					case CLCIT_CONTACT:
						if (group == &dat->list)
							return 0;
						dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, group, -1);
						goto LBL_MoveSelection;
					}
				}

				if (contact->type == CLCIT_GROUP)
					g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, changeGroupExpand == 2);
				return 0;
			}
			
			if (selMoved) {
LBL_MoveSelection:
				if (!dat->bFilterSearch)
					dat->szQuickSearch[0] = 0;
				if (dat->selection >= g_clistApi.pfnGetGroupContentsCount(&dat->list, 1))
					dat->selection = g_clistApi.pfnGetGroupContentsCount(&dat->list, 1) - 1;
				if (dat->selection < 0)
					dat->selection = 0;
				g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
				Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
				UpdateWindow(hwnd);
				return 0;
			}
		}
		break;

	case WM_CHAR:
		Clist_HideInfoTip(dat);
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
			if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) == -1)
				break;
			if (contact->type != CLCIT_CONTACT)
				break;
			contact->flags ^= CONTACTF_CHECKED;
			Clist_RecalculateGroupCheckboxes(dat);
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
			Clist_SaveStateAndRebuildList(hwnd, dat);

		if (dat->szQuickSearch[0]) {
			int index;
			index = g_clistApi.pfnFindRowByText(hwnd, dat, dat->szQuickSearch, 1);
			if (index != -1)
				dat->selection = index;
			else {
				MessageBeep(MB_OK);
				dat->szQuickSearch[mir_wstrlen(dat->szQuickSearch) - 1] = '\0';
				Clist_SaveStateAndRebuildList(hwnd, dat);
			}
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
		}
		else
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_SYSKEYDOWN:
		Clist_EndRename(dat, 1);
		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		dat->iHotTrack = -1;
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		ReleaseCapture();
		if (wParam == VK_F10 && GetKeyState(VK_SHIFT) & 0x8000)
			break;
		SendMessage(GetParent(hwnd), uMsg, wParam, lParam);
		return 0;

	case WM_TIMER:
		switch (wParam) {
		case TIMERID_RENAME:
			g_clistApi.pfnBeginRenameSelection(hwnd, dat);
			break;

		case TIMERID_DRAGAUTOSCROLL:
			g_clistApi.pfnScrollTo(hwnd, dat, dat->yScroll + dat->dragAutoScrolling * dat->rowHeight * 2, 0);
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

				hit = g_clistApi.pfnHitTest(hwnd, dat, it.ptCursor.x, it.ptCursor.y, &contact, nullptr, nullptr);
				if (hit == -1)
					break;
				if (contact->type != CLCIT_GROUP && contact->type != CLCIT_CONTACT)
					break;

				ClientToScreen(hwnd, &it.ptCursor);

				POINT ptClientOffset = { 0 };
				ClientToScreen(hwnd, &ptClientOffset);
				it.isTreeFocused = GetFocus() == hwnd;
				it.rcItem.top = g_clistApi.pfnGetRowTopY(dat, hit) - dat->yScroll;
				it.rcItem.bottom = it.rcItem.top + g_clistApi.pfnGetRowHeight(dat, hit);
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
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			Clist_SaveStateAndRebuildList(hwnd, dat);
			g_clistApi.bAutoRebuild = false;
			break;

		case TIMERID_DELAYEDRESORTCLC:
			KillTimer(hwnd, TIMERID_DELAYEDRESORTCLC);
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			g_clistApi.pfnSortCLC(hwnd, dat, 1);
			g_clistApi.pfnRecalcScrollBar(hwnd, dat);
			break;
		}
		break;

	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
		if (GetFocus() != hwnd)
			SetFocus(hwnd);

		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_INFOTIP);
		KillTimer(hwnd, TIMERID_RENAME);
		Clist_EndRename(dat, 1);
		dat->ptDragStart.x = (short)LOWORD(lParam);
		dat->ptDragStart.y = (short)HIWORD(lParam);
		if (!dat->bFilterSearch)
			dat->szQuickSearch[0] = 0;

		hit = g_clistApi.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, &group, &hitFlags);
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
				dat->selection = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, &selgroup);
				g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, -1);
				if (dat->selection != -1) {
					dat->selection =
						g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
					if (dat->selection == -1)
						dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, contact->group, -1);
				}
				g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
				UpdateWindow(hwnd);
				break;
			}
		}
		if (hit != -1 && hitFlags & CLCHT_ONITEMCHECK) {
			NMCLISTCONTROL nm;
			contact->flags ^= CONTACTF_CHECKED;
			if (contact->type == CLCIT_GROUP)
				Clist_SetGroupChildCheckboxes(contact->group, contact->flags & CONTACTF_CHECKED);
			Clist_RecalculateGroupCheckboxes(dat);
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
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
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		if (dat->selection != -1)
			Clist_EnsureVisible(hwnd, dat, hit, 0);
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
			dat->iHotTrack = g_clistApi.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, nullptr);
			if (iOldHotTrack != dat->iHotTrack) {
				if (iOldHotTrack == -1)
					SetCapture(hwnd);
				else if (dat->iHotTrack == -1)
					ReleaseCapture();
				if (dat->exStyle & CLS_EX_TRACKSELECT) {
					Clist_InvalidateItem(hwnd, dat, iOldHotTrack);
					Clist_InvalidateItem(hwnd, dat, dat->iHotTrack);
				}
				Clist_HideInfoTip(dat);
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
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			if (dat->dragAutoScrolling) {
				KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
				dat->dragAutoScrolling = 0;
			}
			int target = GetDropTargetInformation(hwnd, dat, pt);
			if (dat->dragStage & DRAGSTAGEF_OUTSIDE && target != DROPTARGET_OUTSIDE) {
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

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
			case DROPTARGET_INSERTION:
				hNewCursor = LoadCursor(g_clistApi.hInst, MAKEINTRESOURCE(IDC_DROPUSER));
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
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

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
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, nullptr, &group);
				if (group->parent)
					hNewCursor = LoadCursor(g_clistApi.hInst, MAKEINTRESOURCE(IDC_DROPUSER));
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
			dat->iHotTrack = g_clistApi.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, nullptr);
			if (dat->iHotTrack == -1)
				ReleaseCapture();
		}
		else ReleaseCapture();
		KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
		if (dat->dragStage == (DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME))
			SetTimer(hwnd, TIMERID_RENAME, GetDoubleClickTime(), nullptr);
		else if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			int target = GetDropTargetInformation(hwnd, dat, pt);
			switch (target) {
			case DROPTARGET_ONSELF:
			case DROPTARGET_ONCONTACT:
				break;

			case DROPTARGET_ONGROUP:
				{
					ClcContact *contactn, *contacto;
					g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contactn, nullptr);
					g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contacto, nullptr);
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
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);
				{
					ClcContact *destcontact;
					ClcGroup *destgroup;
					if (g_clistApi.pfnGetRowByIndex(dat, dat->iInsertionMark, &destcontact, &destgroup) == -1 || destgroup != contact->group->parent)
						Clist_GroupMoveBefore(contact->groupId, 0);
					else {
						if (destcontact->type == CLCIT_GROUP)
							destgroup = destcontact->group;
						Clist_GroupMoveBefore(contact->groupId, destgroup->groupId);
					}
				}
				break;
			
			case DROPTARGET_OUTSIDE:
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);
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
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, &group);
				if (group && !group->parent)
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

		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		dat->iDragItem = -1;
		dat->iInsertionMark = -1;
		break;

	case WM_LBUTTONDBLCLK:
		ReleaseCapture();
		dat->iHotTrack = -1;
		Clist_HideInfoTip(dat);
		KillTimer(hwnd, TIMERID_RENAME);
		KillTimer(hwnd, TIMERID_INFOTIP);

		dat->selection = g_clistApi.pfnHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, nullptr, &hitFlags);
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		if (dat->selection != -1)
			Clist_EnsureVisible(hwnd, dat, dat->selection, 0);

		if (hitFlags & (CLCHT_ONITEMICON | CLCHT_ONITEMLABEL)) {
			UpdateWindow(hwnd);
			Clist_DoSelectionDefaultAction(hwnd, dat);
		}
		break;

	case WM_CONTEXTMENU:
		Clist_EndRename(dat, 1);
		Clist_HideInfoTip(dat);
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
				dat->selection = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
				if (dat->selection != -1)
					Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
				pt.x = dat->iconXSpace + 15;
				pt.y = g_clistApi.pfnGetRowTopY(dat, dat->selection) - dat->yScroll + (int)(g_clistApi.pfnGetRowHeight(dat, dat->selection) * .7);
				hitFlags = (dat->selection == -1) ? CLCHT_NOWHERE : CLCHT_ONITEMLABEL;
			}
			else {
				ScreenToClient(hwnd, &pt);
				dat->selection = g_clistApi.pfnHitTest(hwnd, dat, pt.x, pt.y, &contact, nullptr, &hitFlags);
			}
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			if (dat->selection != -1)
				Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
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
		hit = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
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
				Clist_GroupCreate(contact->groupId, nullptr);
				break;
			case POPUP_RENAMEGROUP:
				g_clistApi.pfnBeginRenameSelection(hwnd, dat);
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
		Clist_HideInfoTip(dat);

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
		FreeGroup(&dat->list);
		delete dat;
		UnregisterFileDropping(hwnd);
		WindowList_Remove(hClcWindowList, hwnd);
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
