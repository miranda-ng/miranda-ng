/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

/************************************************************************/
/*       Module responsible for working with contact list control       */
/************************************************************************/

#include "stdafx.h"
#include "modern_clcpaint.h"

/*
*	Private module variables
*/
static HANDLE	hShowInfoTipEvent;
static POINT	HitPoint;
static BOOL		fMouseUpped;
static uint8_t		IsDragToScrollMode = 0;
static int		StartDragPos = 0;
static int		StartScrollPos = 0;
static BOOL		g_bSortTimerIsSet = FALSE;
static ClcContact *hitcontact = nullptr;
HANDLE hSkinFolder;
wchar_t SkinsFolder[MAX_PATH];
void LoadFavoriteContactMenu();

int ReloadSkinFolder(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hSkinFolder, SkinsFolder, _countof(SkinsFolder), _A2W(DEFAULT_SKIN_FOLDER));
	return 0;
}

static int clcHookSmileyAddOptionsChanged(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;
	Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
	Clist_Broadcast(INTM_INVALIDATE, 0, 0);
	return 0;
}

static int clcHookProtoAck(WPARAM, LPARAM lParam)
{
	return ClcDoProtoAck((ACKDATA*)lParam);
}

static int clcHookIconsChanged(WPARAM, LPARAM)
{
	if (MirandaExiting())
		return 0;

	for (auto &it : g_pAvatarOverlayIcons)
		it.listID = -1;
	for (auto &it : g_pStatusOverlayIcons)
		it.listID = -1;

	if (hAvatarOverlays)
		ImageList_Destroy(hAvatarOverlays);
	hAvatarOverlays = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, _countof(g_pAvatarOverlayIcons) * 2, 1);

	for (auto &it : g_pAvatarOverlayIcons) {
		HICON hIcon = IcoLib_GetIcon(it.name);
		it.listID = ImageList_AddIcon(hAvatarOverlays, hIcon);
		IcoLib_Release(it.name);
	}

	for (auto &it : g_pStatusOverlayIcons) {
		HICON hIcon = IcoLib_GetIcon(it.name);
		it.listID = ImageList_AddIcon(hAvatarOverlays, hIcon);
		IcoLib_Release(it.name);
	}

	g_hListeningToIcon = IcoLib_GetIcon("LISTENING_TO_ICON");

	Clist_Broadcast(INTM_INVALIDATE, 0, 0);
	AniAva_UpdateOptions();
	return 0;
}

static int clcMetaModeChanged(WPARAM, LPARAM)
{
	Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
	return 0;
}

static int clcMetacontactChanged(WPARAM, LPARAM)
{
	Clist_Broadcast(INTM_NAMEORDERCHANGED, 0, 0);
	return 0;
}

static int clcHookSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (MirandaExiting())
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == 0) {
		if (!strcmp(cws->szSetting, "XStatusId") || !strcmp(cws->szSetting, "XStatusName"))
			cliCluiProtocolStatusChanged(0, cws->szModule);
		else if (!strcmp(cws->szModule, "CList")) {
			if (!strcmp(cws->szSetting, "OnTop"))
				SetWindowPos(g_clistApi.hwndContactList, cws->value.bVal ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
	else {
		if (!strcmp(cws->szModule, "UserInfo")) {
			if (!strcmp(cws->szSetting, "Timezone"))
				Clist_Broadcast(INTM_TIMEZONECHANGED, hContact, 0);
		}
		else if (!strcmp(cws->szModule, "CList")) {
			if (!strcmp(cws->szSetting, "StatusMsg"))
				Clist_Broadcast(INTM_STATUSMSGCHANGED, hContact, 0);
		}
		else if (!strcmp(cws->szModule, "ContactPhoto")) {
			if (!strcmp(cws->szSetting, "File"))
				Clist_Broadcast(INTM_AVATARCHANGED, hContact, 0);
		}
		else {
			if ((!strcmp(cws->szSetting, "XStatusName") || !strcmp(cws->szSetting, "XStatusMsg")))
				Clist_Broadcast(INTM_STATUSMSGCHANGED, hContact, 0);
			else if (!strcmp(cws->szSetting, "XStatusId"))
				Clist_Broadcast(INTM_STATUSCHANGED, hContact, 0);
			else if (!strcmp(cws->szSetting, "Timezone"))
				Clist_Broadcast(INTM_TIMEZONECHANGED, hContact, 0);
			else if (!strcmp(cws->szSetting, "ListeningTo"))
				Clist_Broadcast(INTM_STATUSMSGCHANGED, hContact, 0);
			else if (!strcmp(cws->szSetting, "Transport") || !strcmp(cws->szSetting, "IsTransported")) {
				g_clistApi.pfnInvalidateDisplayNameCacheEntry(hContact);
				Clist_Broadcast(CLM_AUTOREBUILD, hContact, 0);
			}
		}
	}
	return 0;
}

static int clcHookDbEventAdded(WPARAM hContact, LPARAM hDbEvent)
{
	g_CluiData.t_now = time(0);
	if (hContact && hDbEvent) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if ((dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_FILE) && !(dbei.flags & DBEF_SENT)) {
			g_plugin.setDword(hContact, "mf_lastmsg", dbei.timestamp);
			ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
			if (pdnce) {
				pdnce->dwLastMsgTime = dbei.timestamp;
				if (g_CluiData.hasSort(SORTBY_LASTMSG))
					Clist_Broadcast(CLM_AUTOREBUILD, hContact, 0);
			}
		}
	}
	return 0;
}

static int clcHookBkgndConfigChanged(WPARAM, LPARAM)
{
	Clist_ClcOptionsChanged();
	return 0;
}

static int clcHookAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (MirandaExiting()) return 0;
	Clist_Broadcast(INTM_AVATARCHANGED, wParam, lParam);
	return 0;
}

static int clcExitDragToScroll()
{
	if (!IsDragToScrollMode) return 0;
	IsDragToScrollMode = 0;
	ReleaseCapture();
	return 1;
}

static int clcProceedDragToScroll(HWND hwnd, int Y)
{
	if (!IsDragToScrollMode) return 0;
	if (GetCapture() != hwnd) clcExitDragToScroll();
	int dy = StartDragPos - Y;
	int pos = StartScrollPos + dy;
	if (pos < 0)
		pos = 0;
	SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, pos), 0);
	return 1;
}

static int clcSearchNextContact(HWND hwnd, ClcData *dat, int index, const wchar_t *text, int prefixOk, BOOL fSearchUp)
{
	ClcGroup *group = &dat->list;
	int testlen = (int)mir_wstrlen(text);
	BOOL fReturnAsFound = FALSE;
	int	 nLastFound = -1;
	if (index == -1) fReturnAsFound = TRUE;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if ((group = group->parent) == nullptr)
				break;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type != CLCIT_DIVIDER) {
			bool found;
			if (cc->type == CLCIT_GROUP) {
				found = true;
			}
			else if (dat->bFilterSearch) {
				wchar_t *lowered_szText = CharLowerW(NEWWSTR_ALLOCA(cc->szText));
				wchar_t *lowered_search = CharLowerW(NEWWSTR_ALLOCA(dat->szQuickSearch));
				found = wcsstr(lowered_szText, lowered_search) != nullptr;
			}
			else found = ((prefixOk && CSTR_EQUAL == CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, text, -1, cc->szText, testlen)) || (!prefixOk && !mir_wstrcmpi(text, cc->szText)));

			if (found) {
				ClcGroup *contactGroup = group;
				int contactScanIndex = group->scanIndex;
				int foundindex;
				for (; group; group = group->parent)
					g_clistApi.pfnSetGroupExpand(hwnd, dat, group, 1);
				foundindex = g_clistApi.pfnGetRowsPriorTo(&dat->list, contactGroup, contactScanIndex);
				if (fReturnAsFound)
					return foundindex;
				else if (nLastFound != -1 && fSearchUp && foundindex == index)
					return nLastFound;
				else if (!fSearchUp && foundindex == index)
					fReturnAsFound = TRUE;
				else
					nLastFound = foundindex;
				group = contactGroup;
				group->scanIndex = contactScanIndex;
			}
			if (cc->type == CLCIT_GROUP) {
				if (!(dat->exStyle & CLS_EX_QUICKSEARCHVISONLY) || cc->group->expanded) {
					group = cc->group;
					group->scanIndex = 0;
					continue;
				}
			}
		}
		group->scanIndex++;
	}
	return -1;
}

static bool clcItemNotHiddenOffline(ClcGroup *group, ClcContact *contact)
{
	if (g_CluiData.bFilterEffective)
		return false;

	if (!contact)
		return false;

	if (contact->pce->m_bNoHiddenOffline)
		return true;

	if (!group)
		return false;
	if (group->hideOffline)
		return false;

	if (CLCItems_IsShowOfflineGroup(group))
		return true;

	return false;
}

static LRESULT clcOnCreate(ClcData*, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcData *dat = new ClcData();
	SetWindowLongPtr(hwnd, 0, (LONG_PTR)dat);
	dat->hCheckBoxTheme = xpt_AddThemeHandle(hwnd, L"BUTTON");
	dat->m_paintCouter = 0;
	dat->hWnd = hwnd;

	ImageArray_Initialize(&dat->avatar_cache, FALSE, 20); //this array will be used to keep small avatars too

	RowHeights_Initialize(dat);

	dat->bNeedsResort = true;

	dat->menuOwnerID = -1;
	dat->menuOwnerType = CLCIT_INVALID;

	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	if (dat->contact_time_show || g_plugin.secondLine.iType == TEXT_CONTACT_TIME || g_plugin.thirdLine.iType == TEXT_CONTACT_TIME)
		CLUI_SafeSetTimer(hwnd, TIMERID_INVALIDATE, 5000, nullptr);
	else
		KillTimer(hwnd, TIMERID_INVALIDATE);

	TRACE("Create New ClistControl TO END\r\n");
	return 0;
}

static LRESULT clcOnHitTest(ClcData *, HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
}

static LRESULT clcOnCommand(ClcData *dat, HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact;
	int hit = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr);
	if (hit != -1 && contact->type == CLCIT_GROUP) {
		switch (LOWORD(wParam)) {
		case POPUP_GROUPSHOWOFFLINE:
			Clist_GroupSetFlags(contact->groupId, MAKELPARAM(CLCItems_IsShowOfflineGroup(contact->group) ? 0 : GROUPF_SHOWOFFLINE, GROUPF_SHOWOFFLINE));
			Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
			return 0;
		}
	}

	return corecli.pfnContactListControlWndProc(hwnd, WM_COMMAND, wParam, lParam);
}

static LRESULT clcOnSize(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Clist_EndRename(dat, 1);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);
	cliRecalcScrollBar(hwnd, dat);
	if (g_CluiData.fDisableSkinEngine || dat->bForceInDialog) {
		RECT rc = { 0 };
		GetClientRect(hwnd, &rc);
		if (rc.right == 0)
			return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

		rc.bottom = max(dat->row_min_heigh, 1);

		HDC hdc = GetDC(hwnd);
		int depth = GetDeviceCaps(hdc, BITSPIXEL);
		if (depth < 16)
			depth = 16;
		HBITMAP hBmp = CreateBitmap(rc.right, rc.bottom, 1, depth, nullptr);
		HBITMAP hBmpMask = CreateBitmap(rc.right, rc.bottom, 1, 1, nullptr);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hoBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
		HBRUSH hBrush = CreateSolidBrush((dat->bUseWindowsColours || dat->bForceInDialog) ? GetSysColor(COLOR_HIGHLIGHT) : dat->selBkColour);
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
	else if (dat->himlHighlight) {
		ImageList_Destroy(dat->himlHighlight);
		dat->himlHighlight = nullptr;
	}
	return 0;
}

static LRESULT clcOnChar(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == 27 && dat->szQuickSearch[0] == '\0') { //escape and not quick search
		// minimize clist
		CListMod_HideWindow();
	}
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnPaint(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (IsWindowVisible(hwnd)) {
		if (!g_CluiData.fLayered || GetParent(hwnd) != g_clistApi.hwndContactList) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			g_clcPainter.cliPaintClc(hwnd, dat, hdc, &ps.rcPaint);
			EndPaint(hwnd, &ps);
		}
		else CallService(MS_SKINENG_INVALIDATEFRAMEIMAGE, (WPARAM)hwnd, 0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnEraseBkGround(ClcData *, HWND, UINT, WPARAM, LPARAM)
{
	return 1;
}

static LRESULT clcOnKeyDown(ClcData *dat, HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_CONTROL)
		return 0;

	Clist_HideInfoTip(dat);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);

	if (Clist_MenuProcessHotkey(wParam))
		return 0;

	RECT clRect;
	GetClientRect(hwnd, &clRect);
	int pageSize = (dat->rowHeight) ? clRect.bottom / dat->rowHeight : 0;
	int selMoved = 0;
	int changeGroupExpand = 0;

	switch (wParam) {
	case VK_DOWN:
	case VK_UP:
		if (dat->szQuickSearch[0] != '\0' && dat->selection != -1) { //get next contact
			//get next contact
			int index = clcSearchNextContact(hwnd, dat, dat->selection, dat->szQuickSearch, 1, (wParam == VK_UP));
			if (index == -1) {
				MessageBeep(MB_OK);
				return 0;
			}

			dat->selection = index;
			g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
			return 0;
		}

		if (wParam == VK_DOWN) dat->selection++;
		if (wParam == VK_UP) dat->selection--;
		selMoved = 1;
		break;

	case VK_PRIOR: dat->selection -= pageSize; selMoved = 1; break;
	case VK_NEXT: dat->selection += pageSize; selMoved = 1; break;
	case VK_HOME: dat->selection = 0; selMoved = 1; break;
	case VK_END: dat->selection = g_clistApi.pfnGetGroupContentsCount(&dat->list, 1) - 1; selMoved = 1; break;
	case VK_LEFT: changeGroupExpand = 1; break;
	case VK_RIGHT: changeGroupExpand = 2; break;
	case VK_RETURN:
		Clist_DoSelectionDefaultAction(hwnd, dat);
		SetCapture(hwnd);
		return 0;

	case VK_F2:
		cliBeginRenameSelection(hwnd, dat); 
		return 0;
	
	case VK_DELETE:
		Clist_DeleteFromContactList(hwnd, dat);
		SetCapture(hwnd);
		return 0;
	
	case VK_ESCAPE:
		if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
			dat->iDragItem = -1;
			dat->iInsertionMark = -1;
			dat->dragStage = 0;
			ReleaseCapture();
		}
		return 0;

	default:
		NMKEY nmkey;
		nmkey.hdr.hwndFrom = hwnd;
		nmkey.hdr.idFrom = GetDlgCtrlID(hwnd);
		nmkey.hdr.code = NM_KEYDOWN;
		nmkey.nVKey = wParam;
		nmkey.uFlags = HIWORD(lParam);

		if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nmkey)) {
			SetCapture(hwnd);
			return 0;
		}
	}

	if (changeGroupExpand) {
		ClcContact *contact;
		ClcGroup *group;
		int hit = cliGetRowByIndex(dat, dat->selection, &contact, &group);
		if (hit == -1) {
			SetCapture(hwnd);
			return 0;
		}

		if (contact->type == CLCIT_CONTACT && (contact->iSubNumber || contact->iSubAllocated > 0)) {
			if (contact->iSubNumber && changeGroupExpand == 1) {
				dat->selection -= contact->iSubNumber;
				selMoved = 1;
			}
			else if (!contact->iSubNumber && contact->iSubAllocated > 0) {
				if (changeGroupExpand == 1 && !contact->bSubExpanded) {
					dat->selection = cliGetRowsPriorTo(&dat->list, group, -1);
					selMoved = 1;
				}
				else if (changeGroupExpand == 1 && contact->bSubExpanded) {
					//Contract
					ClcContact *ht = nullptr;
					KillTimer(hwnd, TIMERID_SUBEXPAND);
					contact->bSubExpanded = false;
					g_plugin.setByte(contact->hContact, "Expanded", 0);
					ht = contact;
					dat->bNeedsResort = true;
					g_clistApi.pfnSortCLC(hwnd, dat, 1);
					cliRecalcScrollBar(hwnd, dat);
					hitcontact = nullptr;
				}
				else if (changeGroupExpand == 2 && contact->bSubExpanded) {
					dat->selection++;
					selMoved = 1;
				}
				else if (changeGroupExpand == 2 && !contact->bSubExpanded && dat->bMetaExpanding) {
					ClcContact *ht = nullptr;
					KillTimer(hwnd, TIMERID_SUBEXPAND);
					contact->bSubExpanded = true;
					g_plugin.setByte(contact->hContact, "Expanded", 1);
					ht = contact;
					dat->bNeedsResort = true;
					g_clistApi.pfnSortCLC(hwnd, dat, 1);
					cliRecalcScrollBar(hwnd, dat);
					if (ht) {
						ClcContact *contact2;
						ClcGroup *group2;
						if (Clist_FindItem(hwnd, dat, contact->hContact, &contact2, &group2)) {
							int i = cliGetRowsPriorTo(&dat->list, group2, group2->cl.indexOf(contact2));
							Clist_EnsureVisible(hwnd, dat, i + contact->iSubAllocated, 0);
						}
					}
					hitcontact = nullptr;
				}
			}
		}
		else {
			if (changeGroupExpand == 1 && contact->type == CLCIT_CONTACT) {
				if (group == &dat->list) { SetCapture(hwnd); return 0; }
				dat->selection = cliGetRowsPriorTo(&dat->list, group, -1);
				selMoved = 1;
			}
			else if (contact->type == CLCIT_GROUP) {
				if (changeGroupExpand == 1) {
					if (!contact->group->expanded) {
						dat->selection--;
						selMoved = 1;
					}
					else g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, 0);
				}
				else if (changeGroupExpand == 2) {
					g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, 1);
					dat->selection++;
					selMoved = 1;
				}
				else {
					SetCapture(hwnd);
					return 0;
				}
			}
		}
	}

	if (selMoved) {
		if (dat->selection >= g_clistApi.pfnGetGroupContentsCount(&dat->list, 1))
			dat->selection = g_clistApi.pfnGetGroupContentsCount(&dat->list, 1) - 1;
		if (dat->selection < 0) dat->selection = 0;
		if (dat->bCompactMode)
			SendMessage(hwnd, WM_SIZE, 0, 0);
		cliInvalidateRect(hwnd, nullptr, FALSE);
		Clist_EnsureVisible(hwnd, dat, dat->selection, 0);
		UpdateWindow(hwnd);
	}
	SetCapture(hwnd);
	return 0;
}

void clcSetDelayTimer(UINT_PTR uIDEvent, HWND hwnd, int nDelay)
{
	KillTimer(hwnd, uIDEvent);
	int delay = nDelay;
	if (delay == -1) {
		switch (uIDEvent) {
		case TIMERID_DELAYEDRESORTCLC: delay = 200; break;
		case TIMERID_RECALCSCROLLBAR:  delay = 10;  break;
		case TIMERID_REBUILDAFTER:     delay = 50;  break;
		default:                       delay = 100; break;
		}
	}
	CLUI_SafeSetTimer(hwnd, uIDEvent, delay, nullptr);
}

static LRESULT clcOnTimer(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case TIMERID_INVALIDATE:
		{
			time_t cur_time = (time(0) / 60);
			if (cur_time != dat->last_tick_time) {
				cliInvalidateRect(hwnd, nullptr, FALSE);
				dat->last_tick_time = cur_time;
			}
		}
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	case TIMERID_SUBEXPAND:
		KillTimer(hwnd, TIMERID_SUBEXPAND);
		{
			ClcContact *ht = nullptr;
			if (hitcontact && dat->bMetaExpanding) {
				hitcontact->bSubExpanded = !hitcontact->bSubExpanded;
				g_plugin.setByte(hitcontact->hContact, "Expanded", hitcontact->bSubExpanded);
				if (hitcontact->bSubExpanded)
					ht = &(hitcontact->subcontacts[hitcontact->iSubAllocated - 1]);
			}

			dat->bNeedsResort = true;
			g_clistApi.pfnSortCLC(hwnd, dat, 1);
			cliRecalcScrollBar(hwnd, dat);
			if (ht) {
				ClcContact *contact;
				ClcGroup *group;
				if (Clist_FindItem(hwnd, dat, hitcontact->hContact, &contact, &group)) {
					int i = cliGetRowsPriorTo(&dat->list, group, group->cl.indexOf(contact));
					Clist_EnsureVisible(hwnd, dat, i + hitcontact->iSubAllocated, 0);
				}
			}
			hitcontact = nullptr;
		}
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	case TIMERID_DELAYEDRESORTCLC:
		TRACE("Do sort on Timer\n");
		KillTimer(hwnd, TIMERID_DELAYEDRESORTCLC);
		g_clistApi.pfnSortCLC(hwnd, dat, 1);
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
		return 0;

	case TIMERID_RECALCSCROLLBAR:
		KillTimer(hwnd, TIMERID_RECALCSCROLLBAR);
		g_clistApi.pfnRecalcScrollBar(hwnd, dat);
		return 0;

	default:
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	}
}

static LRESULT clcOnActivate(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TRACE("clcOnActivate\n");
	if (dat->bCompactMode) {
		cliRecalcScrollBar(hwnd, dat);
		if (dat->hwndRenameEdit == nullptr)
			PostMessage(hwnd, WM_SIZE, 0, 0);
	}
	dat->dragStage |= DRAGSTAGEF_SKIPRENAME;
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnSetCursor(ClcData *, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!CLUI_IsInMainWindow(hwnd))
		return DefWindowProc(hwnd, msg, wParam, lParam);

	if (g_CluiData.nBehindEdgeState > 0)
		CLUI_ShowFromBehindEdge();

	if (g_CluiData.bBehindEdgeSettings)
		CLUI_UpdateTimer();

	int lResult = CLUI_TestCursorOnBorders();
	return lResult ? lResult : DefWindowProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnLButtonDown(ClcData *dat, HWND hwnd, UINT, WPARAM, LPARAM lParam)
{
	POINT pt = { LOWORD(lParam), HIWORD(lParam) };
	ClientToScreen(hwnd, &pt);
	int k = CLUI_SizingOnBorder(pt, 0);
	if (k) {
		int io = dat->iHotTrack;
		dat->iHotTrack = 0;
		if (dat->exStyle & CLS_EX_TRACKSELECT)
			Clist_InvalidateItem(hwnd, dat, io);

		if (k && GetCapture() == hwnd)
			SendMessage(GetParent(hwnd), WM_PARENTNOTIFY, WM_LBUTTONDOWN, lParam);
		return FALSE;
	}

	fMouseUpped = FALSE;
	Clist_HideInfoTip(dat);
	KillTimer(hwnd, TIMERID_INFOTIP);
	KillTimer(hwnd, TIMERID_RENAME);
	KillTimer(hwnd, TIMERID_SUBEXPAND);

	Clist_EndRename(dat, 1);
	dat->ptDragStart.x = (short)LOWORD(lParam);
	dat->ptDragStart.y = (short)HIWORD(lParam);

	ClcContact *contact;
	ClcGroup *group;
	uint32_t hitFlags;
	int hit = cliHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), &contact, &group, &hitFlags);
	if (GetFocus() != hwnd)
		SetFocus(hwnd);

	if (hit != -1 && !(hitFlags & CLCHT_NOWHERE)) {
		if (hit == dat->selection && hitFlags & CLCHT_ONITEMLABEL && dat->exStyle & CLS_EX_EDITLABELS) {
			if (!(dat->dragStage & DRAGSTAGEF_SKIPRENAME)) {
				SetCapture(hwnd);
				dat->iDragItem = dat->selection;
				dat->dragStage = DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME;
				dat->dragAutoScrolling = 0;
				return TRUE;
			}
			else {
				dat->dragStage &= ~DRAGSTAGEF_SKIPRENAME;
				return TRUE;
			}
		}
	}

	if (hit != -1 && !(hitFlags & CLCHT_NOWHERE) && contact->type == CLCIT_CONTACT && contact->iSubAllocated && !contact->iSubNumber)
		if (hitFlags & CLCHT_ONITEMICON && dat->bMetaExpanding) {
			hitcontact = contact;
			HitPoint.x = (short)LOWORD(lParam);
			HitPoint.y = (short)HIWORD(lParam);
			fMouseUpped = FALSE;
			if ((GetKeyState(VK_SHIFT) & 0x8000) || (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000)) {
				fMouseUpped = TRUE;
				hitcontact = contact;
				KillTimer(hwnd, TIMERID_SUBEXPAND);
				CLUI_SafeSetTimer(hwnd, TIMERID_SUBEXPAND, 0, nullptr);
			}
		}
		else hitcontact = nullptr;

		if (hit != -1 && !(hitFlags & CLCHT_NOWHERE) && contact->type == CLCIT_GROUP && (hitFlags & CLCHT_ONITEMICON)) {
			ClcGroup *selgroup;
			ClcContact *selcontact;
			dat->selection = cliGetRowByIndex(dat, dat->selection, &selcontact, &selgroup);
			g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, -1);
			if (dat->selection != -1) {
				dat->selection = cliGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
				if (dat->selection == -1)
					dat->selection = cliGetRowsPriorTo(&dat->list, contact->group, -1);
			}

			if (dat->bCompactMode)
				SendMessage(hwnd, WM_SIZE, 0, 0);
			else {
				cliInvalidateRect(hwnd, nullptr, FALSE);
				UpdateWindow(hwnd);
			}
			return TRUE;
		}

		if (hit != -1 && !(hitFlags & CLCHT_NOWHERE) && (hitFlags & CLCHT_ONITEMCHECK)) {
			int bNewState = (contact->flags & CONTACTF_CHECKED) == 0; // inversion

			if (contact->type == CLCIT_GROUP)
				Clist_SetGroupChildCheckboxes(contact->group, bNewState);
			else
				g_clistApi.pfnSetContactCheckboxes(contact, bNewState);
			Clist_RecalculateGroupCheckboxes(dat);
			cliInvalidateRect(hwnd, nullptr, FALSE);

			NMCLISTCONTROL nm;
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
			nm.hItem = (hit == -1 || hitFlags & CLCHT_NOWHERE) ? nullptr : Clist_ContactToItemHandle(contact, &nm.flags);
			nm.iColumn = hitFlags & CLCHT_ONITEMEXTRA ? HIBYTE(HIWORD(hitFlags)) : -1;
			nm.pt = dat->ptDragStart;
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
		}

		if (hitFlags & (CLCHT_ONITEMCHECK | CLCHT_ONITEMEXTRA))
			return FALSE;

		dat->selection = (hitFlags & CLCHT_NOWHERE) ? -1 : hit;
		cliInvalidateRect(hwnd, nullptr, FALSE);

		UpdateWindow(hwnd);
		if (dat->selection != -1 && (contact->type == CLCIT_CONTACT || contact->type == CLCIT_GROUP) && !(hitFlags & (CLCHT_ONITEMEXTRA | CLCHT_ONITEMCHECK | CLCHT_NOWHERE))) {
			SetCapture(hwnd);
			dat->iDragItem = dat->selection;
			dat->dragStage = DRAGSTAGE_NOTMOVED;
			dat->dragAutoScrolling = 0;
		}

		if (dat->bCompactMode)
			SendMessage(hwnd, WM_SIZE, 0, 0);

		if (dat->selection != -1)
			Clist_EnsureVisible(hwnd, dat, hit, 0);
		return TRUE;
}

static LRESULT clcOnCaptureChanged(ClcData *dat, HWND hwnd, UINT, WPARAM, LPARAM lParam)
{
	if ((HWND)lParam != hwnd) {
		if (dat->iHotTrack != -1) {
			int i;
			i = dat->iHotTrack;
			dat->iHotTrack = -1;
			Clist_InvalidateItem(hwnd, dat, i);
			Clist_HideInfoTip(dat);
		}
	}
	return 0;
}

static LRESULT clcOnMouseMove(ClcData *dat, HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	BOOL isOutside = FALSE;
	if (CLUI_IsInMainWindow(hwnd)) {
		if (g_CluiData.bBehindEdgeSettings)
			CLUI_UpdateTimer();
		CLUI_TestCursorOnBorders();
	}

	if (clcProceedDragToScroll(hwnd, (short)HIWORD(lParam)))
		return 0;

	if (dat->dragStage & DRAGSTAGEF_MAYBERENAME) {
		POINT pt = UNPACK_POINT(lParam);
		if (abs(pt.x - dat->ptDragStart.x) > GetSystemMetrics(SM_CXDOUBLECLK) || abs(pt.y - dat->ptDragStart.y) > GetSystemMetrics(SM_CYDOUBLECLK)) {
			KillTimer(hwnd, TIMERID_RENAME);
			dat->dragStage &= (~DRAGSTAGEF_MAYBERENAME);
		}
	}

	if (dat->iDragItem == -1) {
		POINT pt = UNPACK_POINT(lParam);
		ClientToScreen(hwnd, &pt);
		HWND window = WindowFromPoint(pt);
		if (window != hwnd)
			isOutside = TRUE;
	}

	if (hitcontact != nullptr) {
		int x = (short)LOWORD(lParam);
		int y = (short)HIWORD(lParam);
		int xm = GetSystemMetrics(SM_CXDOUBLECLK);
		int ym = GetSystemMetrics(SM_CYDOUBLECLK);
		if (abs(HitPoint.x - x) > xm || abs(HitPoint.y - y) > ym) {
			if (fMouseUpped) {
				KillTimer(hwnd, TIMERID_SUBEXPAND);
				CLUI_SafeSetTimer(hwnd, TIMERID_SUBEXPAND, 0, nullptr);
				fMouseUpped = FALSE;
			}
			else {
				KillTimer(hwnd, TIMERID_SUBEXPAND);
				hitcontact = nullptr;
				fMouseUpped = FALSE;
			}
		}
	}

	if (dat->iDragItem == -1) {
		uint32_t flag = 0;
		int iOldHotTrack = dat->iHotTrack;

		if (dat->hwndRenameEdit != nullptr || GetKeyState(VK_MENU) & 0x8000 || GetKeyState(VK_F10) & 0x8000)
			return 0;

		dat->iHotTrack = isOutside ? -1 : cliHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, &flag);

		if (flag & CLCHT_NOWHERE)
			dat->iHotTrack = -1;

		if (iOldHotTrack != dat->iHotTrack || isOutside) {
			if (iOldHotTrack == -1 && !isOutside)
				SetCapture(hwnd);

			if (dat->iHotTrack == -1 || isOutside)
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
			CLUI_SafeSetTimer(hwnd, TIMERID_INFOTIP, dat->infoTipTimeout, nullptr);
		}
		return 0;
	}

	if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_NOTMOVED && !(dat->exStyle & CLS_EX_DISABLEDRAGDROP))
		if (abs((short)LOWORD(lParam) - dat->ptDragStart.x) >= GetSystemMetrics(SM_CXDRAG) || abs((short)HIWORD(lParam) - dat->ptDragStart.y) >= GetSystemMetrics(SM_CYDRAG))
			dat->dragStage = (dat->dragStage & ~DRAGSTAGEM_STAGE) | DRAGSTAGE_ACTIVE;

	if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
		RECT clRect;
		GetClientRect(hwnd, &clRect);

		POINT pt = UNPACK_POINT(lParam);
		HCURSOR hNewCursor = LoadCursor(nullptr, IDC_NO);
		cliInvalidateRect(hwnd, nullptr, FALSE);
		if (dat->dragAutoScrolling) {
			KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
			dat->dragAutoScrolling = 0;
		}
		int target = GetDropTargetInformation(hwnd, dat, pt);
		if ((dat->dragStage & DRAGSTAGEF_OUTSIDE) && target != DROPTARGET_OUTSIDE) {
			ClcContact *contact;
			cliGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

			NMCLISTCONTROL nm;
			nm.hdr.code = CLN_DRAGSTOP;
			nm.hdr.hwndFrom = hwnd;
			nm.hdr.idFrom = GetDlgCtrlID(hwnd);
			nm.flags = 0;
			nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
			SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
			dat->dragStage &= ~DRAGSTAGEF_OUTSIDE;
		}

		ClcContact *contSour = nullptr, *contDest = nullptr;
		cliGetRowByIndex(dat, dat->selection, &contDest, nullptr);
		cliGetRowByIndex(dat, dat->iDragItem, &contSour, nullptr);

		if (contDest->getType() != CLCIT_INFO && contSour->getType() != CLCIT_INFO)
		switch (target) {
		case DROPTARGET_ONSELF:
			break;

		case DROPTARGET_ONCONTACT:
			if (contSour->isChat())
				break;
			if (contSour->type == CLCIT_CONTACT && mir_strcmp(contSour->pce->szProto, META_PROTO)) {
				if (!contSour->iSubNumber)
					hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
				else
					hNewCursor = LoadCursor(g_plugin.getInst(), MAKEINTRESOURCE(IDC_DROPMETA));
			}
			break;

		case DROPTARGET_ONMETACONTACT:
			if (contSour->isChat() || contDest->isChat())
				break;
			if (contSour->type == CLCIT_CONTACT && mir_strcmp(contSour->pce->szProto, META_PROTO)) {
				if (!contSour->iSubNumber)
					hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
				else if (contSour->subcontacts == contDest)
					hNewCursor = LoadCursor(g_plugin.getInst(), MAKEINTRESOURCE(IDC_DEFAULTSUB)); ///MakeDefault
				else
					hNewCursor = LoadCursor(g_plugin.getInst(), MAKEINTRESOURCE(IDC_REGROUP));
			}
			break;

		case DROPTARGET_ONSUBCONTACT:
			if (contSour->isChat() || contDest->isChat())
				break;
			if (contSour->type == CLCIT_CONTACT && mir_strcmp(contSour->pce->szProto, META_PROTO)) {
				if (!contSour->iSubNumber)
					hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROPUSER));  /// Add to meta
				else if (contDest->subcontacts == contSour->subcontacts)
					break;
				else
					hNewCursor = LoadCursor(g_plugin.getInst(), MAKEINTRESOURCE(IDC_REGROUP));
			}
			break;

		case DROPTARGET_ONGROUP:
			hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROPUSER));
			break;

		case DROPTARGET_INSERTION:
			hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROP));
			break;

		case DROPTARGET_OUTSIDE:
			if (pt.x >= 0 && pt.x < clRect.right && ((pt.y < 0 && pt.y>-dat->dragAutoScrollHeight) || (pt.y >= clRect.bottom && pt.y < clRect.bottom + dat->dragAutoScrollHeight))) {
				if (!dat->dragAutoScrolling) {
					dat->dragAutoScrolling = (pt.y < 0) ? -1 : 1;
					CLUI_SafeSetTimer(hwnd, TIMERID_DRAGAUTOSCROLL, dat->scrollTime, nullptr);
				}
				SendMessage(hwnd, WM_TIMER, TIMERID_DRAGAUTOSCROLL, 0);
			}

			dat->dragStage |= DRAGSTAGEF_OUTSIDE;
			{
				ClcContact *contact;
				cliGetRowByIndex(dat, dat->iDragItem, &contact, nullptr);

				NMCLISTCONTROL nm;
				nm.hdr.code = CLN_DRAGGING;
				nm.hdr.hwndFrom = hwnd;
				nm.hdr.idFrom = GetDlgCtrlID(hwnd);
				nm.flags = 0;
				nm.hItem = Clist_ContactToItemHandle(contact, &nm.flags);
				nm.pt = pt;
				if (SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm))
					return 0;
			}
			break;

		default:
			ClcGroup *group = nullptr;
			cliGetRowByIndex(dat, dat->iDragItem, nullptr, &group);
			if (group && group->parent) {
				cliGetRowByIndex(dat, dat->iDragItem, &contSour, nullptr);
				if (!contSour->iSubNumber)
					hNewCursor = LoadCursor(g_hMirApp, MAKEINTRESOURCE(IDC_DROPUSER));
			}
			break;
		}
		SetCursor(hNewCursor);
	}
	return 0;
}

static LRESULT clcOnLButtonUp(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (clcExitDragToScroll())
		return 0;

	fMouseUpped = TRUE;

	if (hitcontact != nullptr && dat->bMetaExpanding) {
		uint8_t doubleClickExpand = db_get_b(0, "CLC", "MetaDoubleClick", SETTING_METAAVOIDDBLCLICK_DEFAULT);
		CLUI_SafeSetTimer(hwnd, TIMERID_SUBEXPAND, GetDoubleClickTime()*doubleClickExpand, nullptr);
	}
	else if (dat->iHotTrack == -1 && dat->iDragItem == -1)
		ReleaseCapture();

	if (dat->iDragItem == -1)
		return 0;

	SetCursor((HCURSOR)GetClassLongPtr(hwnd, GCLP_HCURSOR));
	if (dat->exStyle & CLS_EX_TRACKSELECT) {
		uint32_t flags;
		dat->iHotTrack = cliHitTest(hwnd, dat, (short)LOWORD(lParam), (short)HIWORD(lParam), nullptr, nullptr, &flags);
		if (dat->iHotTrack == -1)
			ReleaseCapture();
	}
	else if (hitcontact == nullptr)
		ReleaseCapture();
	
	KillTimer(hwnd, TIMERID_DRAGAUTOSCROLL);
	if (dat->dragStage == (DRAGSTAGE_NOTMOVED | DRAGSTAGEF_MAYBERENAME))
		CLUI_SafeSetTimer(hwnd, TIMERID_RENAME, GetDoubleClickTime(), nullptr);
	else if ((dat->dragStage & DRAGSTAGEM_STAGE) == DRAGSTAGE_ACTIVE) {
		POINT pt = UNPACK_POINT(lParam);
		int target = GetDropTargetInformation(hwnd, dat, pt);

		ClcContact *contDest = nullptr, *contSour = nullptr;
		cliGetRowByIndex(dat, dat->iDragItem, &contSour, nullptr);
		cliGetRowByIndex(dat, dat->selection, &contDest, nullptr);

		if (contDest->getType() != CLCIT_INFO && contSour->getType() != CLCIT_INFO)
		switch (target) {
		case DROPTARGET_ONSELF:
			break;

		case DROPTARGET_ONCONTACT:
			if (contSour->isChat() || contDest->isChat())
				break;

			if (contSour->type == CLCIT_CONTACT && db_mc_isEnabled()) {
				MCONTACT hcontact = contSour->hContact;
				if (mir_strcmp(contSour->pce->szProto, META_PROTO)) {
					wchar_t Wording[500];
					if (!contSour->iSubNumber) {
						MCONTACT hDest = contDest->hContact;
						mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be converted to metacontact and '%s' be added to it?"), contDest->szText, contSour->szText);
						int res = MessageBox(hwnd, Wording, TranslateT("Converting to metacontact"), MB_OKCANCEL | MB_ICONQUESTION);
						if (res == 1) {
							MCONTACT handle = db_mc_convertToMeta(hDest);
							if (!handle)
								return 0;

							db_mc_addToMeta(hcontact, handle);
						}
					}
					else {
						hcontact = contSour->hContact;
						MCONTACT hdest = contDest->hContact;
						mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be converted to metacontact and '%s' be added to it (remove it from '%s')?"), contDest->szText, contSour->szText, contSour->subcontacts->szText);
						int res = MessageBox(hwnd, Wording, TranslateT("Converting to metacontact (moving)"), MB_OKCANCEL | MB_ICONQUESTION);
						if (res == 1) {
							MCONTACT handle = db_mc_convertToMeta(hdest);
							if (!handle)
								return 0;

							db_mc_removeFromMeta(hcontact);
							db_mc_addToMeta(hcontact, handle);
						}
					}
				}
			}
			break;

		case DROPTARGET_ONMETACONTACT:
			if (contSour->isChat() || contDest->isChat())
				break;
			if (contSour->type == CLCIT_CONTACT) {
				if (!mir_strcmp(contSour->pce->szProto, META_PROTO))
					break;

				wchar_t Wording[500];
				if (!contSour->iSubNumber) {
					MCONTACT hcontact = contSour->hContact;
					MCONTACT handle = contDest->hContact;
					mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be added to metacontact '%s'?"), contSour->szText, contDest->szText);
					int res = MessageBox(hwnd, Wording, TranslateT("Adding contact to metacontact"), MB_OKCANCEL | MB_ICONQUESTION);
					if (res == 1) {
						if (!handle)
							return 0;
						db_mc_addToMeta(hcontact, handle);
					}
				}
				else if (contSour->subcontacts == contDest) {
					MCONTACT hsour = contSour->hContact;
					mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be default?"), contSour->szText);
					int res = MessageBox(hwnd, Wording, TranslateT("Set default contact"), MB_OKCANCEL | MB_ICONQUESTION);
					if (res == 1)
						db_mc_setDefault(contDest->hContact, hsour, true);
				}
				else {
					MCONTACT hcontact = contSour->hContact;
					MCONTACT handle = contDest->hContact;
					mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be removed from metacontact '%s' and added to '%s'?"), contSour->szText, contSour->subcontacts->szText, contDest->szText);
					int res = MessageBox(hwnd, Wording, TranslateT("Changing metacontacts (moving)"), MB_OKCANCEL | MB_ICONQUESTION);
					if (res == 1) {
						if (!handle)
							return 0;

						db_mc_removeFromMeta(hcontact);
						db_mc_addToMeta(hcontact, handle);
					}
				}
			}
			break;

		case DROPTARGET_ONSUBCONTACT:
			if (contSour->isChat() || contDest->isChat())
				break;
			if (contSour->type == CLCIT_CONTACT) {
				if (!mir_strcmp(contSour->pce->szProto, META_PROTO))
					break;

				wchar_t Wording[500];
				if (!contSour->iSubNumber) {
					MCONTACT hcontact = contSour->hContact;
					MCONTACT handle = contDest->subcontacts->hContact;
					mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be added to metacontact '%s'?"), contSour->szText, contDest->subcontacts->szText);
					int res = MessageBox(hwnd, Wording, TranslateT("Changing metacontacts (moving)"), MB_OKCANCEL | MB_ICONQUESTION);
					if (res == 1) {
						if (!handle)
							return 0;

						db_mc_addToMeta(hcontact, handle);
					}
				}
				else if (contSour->subcontacts != contDest->subcontacts) {
					MCONTACT hcontact = contSour->hContact;
					MCONTACT handle = contDest->subcontacts->hContact;
					mir_snwprintf(Wording, TranslateT("Do you want contact '%s' to be removed from metacontact '%s' and added to '%s'?"), contSour->szText, contSour->subcontacts->szText, contDest->subcontacts->szText);
					int res = MessageBox(hwnd, Wording, TranslateT("Changing metacontacts (moving)"), MB_OKCANCEL | MB_ICONQUESTION);
					if (res == 1) {
						if (!handle)
							return 0;

						db_mc_removeFromMeta(hcontact);
						db_mc_addToMeta(hcontact, handle);
					}
				}
			}
			break;

		case DROPTARGET_INSERTION:
			ClcContact *contact, *destcontact;
			ClcGroup *group, *destgroup;
			{
				BOOL NeedRename = FALSE;
				wchar_t newName[128] = { 0 };
				g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &contact, &group);
				int i = g_clistApi.pfnGetRowByIndex(dat, dat->iInsertionMark, &destcontact, &destgroup);
				if (i != -1 && group->groupId != destgroup->groupId) {
					wchar_t *groupName = mir_wstrdup(Clist_GroupGetName(contact->groupId, nullptr));
					wchar_t *shortGroup = nullptr;
					wchar_t *sourceGrName = mir_wstrdup(Clist_GroupGetName(destgroup->groupId, nullptr));
					if (groupName) {
						int len = (int)mir_wstrlen(groupName);
						do { len--; } while (len >= 0 && groupName[len] != '\\');
						if (len >= 0)
							shortGroup = groupName + len + 1;
						else
							shortGroup = groupName;
					}
					if (shortGroup) {
						NeedRename = TRUE;
						if (sourceGrName)
							mir_snwprintf(newName, L"%s\\%s", sourceGrName, shortGroup);
						else
							wcsncpy_s(newName, shortGroup, _TRUNCATE);
					}
					mir_free(groupName);
					mir_free(sourceGrName);
				}

				int newIndex = Clist_GroupMoveBefore(contact->groupId, (destcontact && i != -1) ? destcontact->groupId : 0);
				newIndex = newIndex ? newIndex : contact->groupId;
				if (NeedRename)
					Clist_GroupRename(newIndex, newName);
			}
			break;

		default:
			corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
			break;
		}
	}

	cliInvalidateRect(hwnd, nullptr, FALSE);
	dat->iDragItem = -1;
	dat->iInsertionMark = -1;
	return 0;
}

static LRESULT clcOnLButtonDblClick(ClcData *, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hwnd, TIMERID_SUBEXPAND);
	hitcontact = nullptr;
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnDestroy(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i <= FONTID_MODERN_MAX; i++) {
		if (dat->fontModernInfo[i].hFont)
			DeleteObject(dat->fontModernInfo[i].hFont);
		dat->fontModernInfo[i].hFont = nullptr;
	}
	if (dat->hMenuBackground) {
		DeleteObject(dat->hMenuBackground);
		dat->hMenuBackground = nullptr;
	}
	if (dat->hBmpBackground) {
		DeleteObject(dat->hBmpBackground);
		dat->hBmpBackground = nullptr;
	}

	ImageArray_Clear(&dat->avatar_cache);
	DeleteDC(dat->avatar_cache.hdc);
	ImageArray_Free(&dat->avatar_cache, FALSE);
	if (dat->himlHighlight) {
		ImageList_Destroy(dat->himlHighlight);
		dat->himlHighlight = nullptr;
	}

	RowHeights_Free(dat);
	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	xpt_FreeThemeForWindow(hwnd);
	return 0;
}

static LRESULT clcOnIntmGroupChanged(ClcData *dat, HWND hwnd, UINT, WPARAM wParam, LPARAM)
{
	uint16_t iExtraImage[EXTRA_ICON_COUNT];
	uint8_t flags = 0;

	ClcContact *contact;
	if (!Clist_FindItem(hwnd, dat, wParam, &contact))
		memset(iExtraImage, 0xFF, sizeof(iExtraImage));
	else {
		memcpy(iExtraImage, contact->iExtraImage, sizeof(iExtraImage));
		flags = contact->flags;
	}
	Clist_DeleteItemFromTree(hwnd, wParam);
	if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_SHOWHIDDEN || !Contact::IsHidden(wParam)) {
		g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, 1, 1);
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			memcpy(contact->iExtraImage, iExtraImage, sizeof(iExtraImage));
			if (flags & CONTACTF_CHECKED)
				contact->flags |= CONTACTF_CHECKED;
		}

		NMCLISTCONTROL nm;
		nm.hdr.code = CLN_CONTACTMOVED;
		nm.hdr.hwndFrom = hwnd;
		nm.hdr.idFrom = GetDlgCtrlID(hwnd);
		nm.flags = 0;
		nm.hItem = (HANDLE)wParam;
		SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
		dat->bNeedsResort = true;
	}
	Clist_InitAutoRebuild(hwnd);
	return 0;
}

static LRESULT clcOnIntmIconChanged(ClcData *dat, HWND hwnd, UINT, WPARAM wParam, LPARAM lParam)
{
	bool needRepaint = false;
	int contacticon = Clist_GetContactIcon(wParam);
	MCONTACT hSelItem = 0;

	ClcGroup *selgroup;
	ClcContact *selcontact = nullptr;

	char *szProto = Proto_GetBaseAccountName(wParam);
	uint16_t status = (szProto == nullptr) ? ID_STATUS_OFFLINE : GetContactCachedStatus(wParam);
	bool bImageIsSpecial = (LOWORD(contacticon) != (LOWORD(lParam))); //check only base icons

	int nHiddenStatus = CLVM_GetContactHiddenStatus(wParam, szProto, dat);

	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
	bool isVisiblebyFilter = (((style & CLS_SHOWHIDDEN) && nHiddenStatus != -1) || !nHiddenStatus);
	bool ifVisibleByClui = !Clist_IsHiddenMode(dat, status);
	bool isVisible = (g_CluiData.bFilterEffective & CLVM_FILTER_STATUS) ? true : ifVisibleByClui;
	bool isIconChanged = Clist_GetContactIcon(wParam) != LOWORD(lParam);

	int shouldShow = isVisiblebyFilter && (isVisible || isIconChanged);

	// XXX CLVM changed - this means an offline msg is flashing, so the contact should be shown

	ClcGroup *group = nullptr;
	ClcContact *contact = nullptr;
	if (!Clist_FindItem(hwnd, dat, wParam, &contact, &group)) {
		if (shouldShow && db_is_contact(wParam)) {
			if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
				hSelItem = Clist_ContactToHItem(selcontact);
			g_clistApi.pfnAddContactToTree(hwnd, dat, wParam, (style & CLS_CONTACTLIST) == 0, 0);
			needRepaint = TRUE;
			Clist_FindItem(hwnd, dat, wParam, &contact);
			if (contact) {
				contact->iImage = lParam;
				contact->bImageIsSpecial = bImageIsSpecial;
				Clist_NotifyNewContact(hwnd, wParam);
				dat->bNeedsResort = true;
			}
		}
	}
	else {
		// item in list already
		if (contact && contact->iImage == lParam)
			return 0;

		if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline) && clcItemNotHiddenOffline(group, contact))
			shouldShow = TRUE;

		if (!shouldShow && !(style & CLS_NOHIDEOFFLINE) && ((style & CLS_HIDEOFFLINE) || group->hideOffline)) { // CLVM changed
			if (dat->selection >= 0 && g_clistApi.pfnGetRowByIndex(dat, dat->selection, &selcontact, nullptr) != -1)
				hSelItem = Clist_ContactToHItem(selcontact);
			Clist_RemoveItemFromGroup(hwnd, group, contact, (style & CLS_CONTACTLIST) == 0);
			needRepaint = TRUE;
			dat->bNeedsResort = true;
		}
		else if (contact) {
			contact->iImage = lParam;
			if (!Clist_IsHiddenMode(dat, status))
				contact->flags |= CONTACTF_ONLINE;
			else
				contact->flags &= ~CONTACTF_ONLINE;
			contact->bImageIsSpecial = bImageIsSpecial;
			if (!bImageIsSpecial) { //Only if it is status changing
				dat->bNeedsResort = true;
				needRepaint = true;
			}
			else if (dat->m_paintCouter == contact->lastPaintCounter) //if contacts is visible
				needRepaint = true;
		}
	}

	if (hSelItem) {
		if (Clist_FindItem(hwnd, dat, hSelItem, &selcontact, &selgroup))
			dat->selection = g_clistApi.pfnGetRowsPriorTo(&dat->list, selgroup, selgroup->cl.indexOf(selcontact));
		else
			dat->selection = -1;
	}

	if (dat->bNeedsResort) {
		TRACE("Sort required\n");
		clcSetDelayTimer(TIMERID_DELAYEDRESORTCLC, hwnd);
	}
	else if (needRepaint) {
		if (contact && contact->pos_icon.bottom != 0 && contact->pos_icon.right != 0)
			cliInvalidateRect(hwnd, &contact->pos_icon, FALSE);
		else
			cliInvalidateRect(hwnd, nullptr, FALSE);
		//try only needed rectangle
	}

	return 0;
}

static LRESULT clcOnIntmAvatarChanged(ClcData *dat, HWND hwnd, UINT, WPARAM hContact, LPARAM)
{
	ClcContact *contact;
	if (Clist_FindItem(hwnd, dat, hContact, &contact))
		Cache_GetAvatar(dat, contact);
	else if (hContact == 0)
		UpdateAllAvatars(dat);

	cliInvalidateRect(hwnd, nullptr, FALSE);
	return 0;
}

static LRESULT clcOnIntmTimeZoneChanged(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ClcContact *contact;
	if (!Clist_FindItem(hwnd, dat, wParam, &contact))
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	if (contact) {
		Cache_GetTimezone(dat, contact->hContact);
		Cache_GetText(dat, contact);
		cliRecalcScrollBar(hwnd, dat);
	}
	return 0;
}

static LRESULT clcOnIntmNameChanged(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	ClcContact *contact;
	if (Clist_FindItem(hwnd, dat, wParam, &contact))
		Cache_GetText(dat, contact);

	return ret;
}

static LRESULT clcOnIntmApparentModeChanged(ClcData *, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnIntmStatusMsgChanged(ClcData *dat, HWND hwnd, UINT msg, WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0 || IsHContactInfo(hContact) || IsHContactGroup(hContact))
		return corecli.pfnContactListControlWndProc(hwnd, msg, hContact, lParam);

	ClcContact *contact;
	if (!Clist_FindItem(hwnd, dat, hContact, &contact))
		return corecli.pfnContactListControlWndProc(hwnd, msg, hContact, lParam);

	if (contact) {
		Cache_GetText(dat, contact);
		// cliRecalcScrollBar(hwnd, dat);
		PostMessage(hwnd, INTM_INVALIDATE, 0, 0);
	}
	return corecli.pfnContactListControlWndProc(hwnd, msg, hContact, lParam);
}

static LRESULT clcOnIntmNotOnListChanged(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *dbcws = (DBCONTACTWRITESETTING*)lParam;

	ClcContact *contact;
	if (!Clist_FindItem(hwnd, dat, wParam, &contact))
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	if (contact->type != CLCIT_CONTACT)
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);

	if (dbcws->value.type == DBVT_DELETED || dbcws->value.bVal == 0)
		contact->flags &= ~CONTACTF_NOTONLIST;
	else
		contact->flags |= CONTACTF_NOTONLIST;

	cliInvalidateRect(hwnd, nullptr, FALSE);
	return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
}

static LRESULT clcOnIntmScrollBarChanged(ClcData *dat, HWND hwnd, UINT, WPARAM, LPARAM)
{
	if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CONTACTLIST) {
		if (dat->bNoVScrollbar)
			ShowScrollBar(hwnd, SB_VERT, FALSE);
		else
			g_clistApi.pfnRecalcScrollBar(hwnd, dat);
	}
	return 0;
}

static LRESULT clcOnIntmStatusChanged(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	
	if (wParam != 0) {
		ClcContact *contact;
		if (Clist_FindItem(hwnd, dat, wParam, &contact)) {
			ClcCacheEntry *pdnce = contact->pce;
			if (pdnce && pdnce->szProto) {
				if (!dat->bForceInDialog) {
					Cache_GetNthLineText(dat, pdnce, 2);
					Cache_GetNthLineText(dat, pdnce, 3);
				}

				clcOnIntmIconChanged(dat, hwnd, msg, wParam, Clist_GetContactIcon(wParam));

				if (contact->type == CLCIT_CONTACT) {
					if (!contact->bImageIsSpecial && pdnce->getStatus() > ID_STATUS_OFFLINE)
						contact->iImage = Clist_GetContactIcon(wParam);
					if (contact->iSubNumber && contact->subcontacts && contact->subcontacts->type == CLCIT_CONTACT)
						Clist_Broadcast(INTM_STATUSCHANGED, contact->subcontacts->hContact, 0); //forward status changing to host meta contact
				}
			}
		}
	}

	return ret;
}

static LRESULT clcOnIntmReloadOptions(ClcData *dat, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	g_clistApi.pfnLoadClcOptions(hwnd, dat, FALSE);
	Clist_SaveStateAndRebuildList(hwnd, dat);
	g_clistApi.pfnSortCLC(hwnd, dat, 1);
	if (IsWindowVisible(hwnd))
		g_clistApi.pfnInvalidateRect(GetParent(hwnd), nullptr, FALSE);
	return TRUE;
}

static int clcHookModulesLoaded(WPARAM, LPARAM)
{
	if (MirandaExiting())
		return 0;

	HookEvent(ME_AV_AVATARCHANGED, clcHookAvatarChanged);

	HookEvent(ME_FOLDERS_PATH_CHANGED, ReloadSkinFolder);
	hSkinFolder = FoldersRegisterCustomPathW(LPGEN("Skins"), LPGEN("Modern contact list"), MIRANDA_PATHW L"\\" _A2W(DEFAULT_SKIN_FOLDER));
	FoldersGetCustomPathW(hSkinFolder, SkinsFolder, _countof(SkinsFolder), _A2W(DEFAULT_SKIN_FOLDER));

	// Get icons
	wchar_t szMyPath[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), szMyPath, _countof(szMyPath));

	// Menus
	LoadFavoriteContactMenu();

	// Icons
	SKINICONDESC sid = {};
	sid.defaultFile.w = szMyPath;
	sid.flags = SIDF_PATH_UNICODE;

	sid.section.a = LPGEN("Contact list");
	sid.description.a = LPGEN("Listening to");
	sid.pszName = "LISTENING_TO_ICON";
	sid.iDefaultIndex = -IDI_LISTENING_TO;
	g_plugin.addIcon(&sid);

	sid.section.a = LPGEN("Contact list") "/" LPGEN("Avatar overlay");
	for (auto &it : g_pAvatarOverlayIcons) {
		sid.description.a = it.description;
		sid.pszName = it.name;
		sid.iDefaultIndex = -it.id;
		g_plugin.addIcon(&sid);
	}

	sid.section.a = LPGEN("Contact list") "/" LPGEN("Status overlay");
	for (auto &it : g_pStatusOverlayIcons) {
		sid.description.a = it.description;
		sid.pszName = it.name;
		sid.iDefaultIndex = -it.id;
		g_plugin.addIcon(&sid);
	}

	clcHookIconsChanged(0, 0);

	HookEvent(ME_SKIN_ICONSCHANGED, clcHookIconsChanged);

	// Register smiley category
	if (ServiceExists(MS_SMILEYADD_REGISTERCATEGORY)) {
		SMADD_REGCAT rc;
		rc.cbSize = sizeof(rc);
		rc.name = "clist";
		rc.dispname = Translate("Contact list smileys");

		CallService(MS_SMILEYADD_REGISTERCATEGORY, 0, (LPARAM)&rc);

		HookEvent(ME_SMILEYADD_OPTIONSCHANGED, clcHookSmileyAddOptionsChanged);
	}

	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("List background")"/CLC"), 0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Menu background")"/Menu"), 0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Status bar background")"/StatusBar"), 0);
	CallService(MS_BACKGROUNDCONFIG_REGISTER, (WPARAM)(LPGEN("Frames title bar background")"/FrameTitleBar"), 0);

	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, clcHookBkgndConfigChanged);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, BgStatusBarChange);
	HookEvent(ME_BACKGROUNDCONFIG_CHANGED, OnFrameTitleBarBackgroundChange);
	HookEvent(ME_COLOUR_RELOAD, OnFrameTitleBarBackgroundChange);

	AniAva_UpdateOptions();
	return 0;
}

HRESULT ClcLoadModule()
{
	g_himlCListClc = Clist_GetImageList();

	HookEvent(ME_MC_SUBCONTACTSCHANGED, clcMetacontactChanged);
	HookEvent(ME_MC_ENABLED, clcMetaModeChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, clcHookSettingChanged);
	HookEvent(ME_OPT_INITIALISE, ClcOptInit);
	HookEvent(ME_PROTO_ACK, clcHookProtoAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, clcHookModulesLoaded);
	HookEvent(ME_DB_EVENT_ADDED, clcHookDbEventAdded);
	return S_OK;
}

int ClcUnloadModule()
{
	if (g_CluiData.bOldUseGroups != (uint8_t)-1)
		Clist::UseGroups = g_CluiData.bOldUseGroups;
	if (g_CluiData.boldHideOffline != (uint8_t)-1)
		Clist::HideOffline = g_CluiData.boldHideOffline;

	return 0;
}

int ClcDoProtoAck(ACKDATA *ack)
{
	if (MirandaExiting()) return 0;

	if (ack->type == ACKTYPE_AWAYMSG) {
		if (ack->result == ACKRESULT_SUCCESS && ack->lParam) {
			if (ack->szModule != nullptr)
				if (Contact::IsGroupChat(ack->hContact, ack->szModule))
					return 0;

			g_plugin.setWString(ack->hContact, "StatusMsg", (const wchar_t *)ack->lParam);
			Clist_Broadcast(INTM_STATUSCHANGED, ack->hContact, 0);
		}
		else {
			if (ack->szModule != nullptr)
				if (Contact::IsGroupChat(ack->hContact, ack->szModule))
					return 0;

			if (ack->hContact) {
				char *val = g_plugin.getStringA(ack->hContact, "StatusMsg");
				if (val) {
					if (mir_strcmp(val, ""))
						g_plugin.setString(ack->hContact, "StatusMsg", "");
					else
						Clist_Broadcast(INTM_STATUSCHANGED, ack->hContact, 0);
					mir_free(val);
				}
			}
		}
	}
	else if (ack->type == ACKTYPE_EMAIL) {
		CLUIUnreadEmailCountChanged(0, 0);
	}
	return 0;
}

int ClcEnterDragToScroll(HWND hwnd, int Y)
{
	if (IsDragToScrollMode)
		return 0;

	ClcData *dat = (ClcData*)GetWindowLongPtr(hwnd, 0);
	if (!dat)
		return 0;

	StartDragPos = Y;
	StartScrollPos = dat->yScroll;
	IsDragToScrollMode = 1;
	SetCapture(hwnd);
	return 1;
}


/*
*	Contact list control window procedure
*/
LRESULT CALLBACK cli_ContactListControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#define CASE_MSG_RET(msg, handler) case msg: return handler(dat, hwnd, msg, wParam, lParam);

	ClcData *dat = (ClcData*)GetWindowLongPtr(hwnd, 0);

	if (msg >= CLM_FIRST && msg < CLM_LAST)
		return cli_ProcessExternalMessages(hwnd, dat, msg, wParam, lParam);

	switch (msg) {
		CASE_MSG_RET(INTM_GROUPCHANGED, clcOnIntmGroupChanged);
		CASE_MSG_RET(INTM_ICONCHANGED, clcOnIntmIconChanged);
		CASE_MSG_RET(INTM_AVATARCHANGED, clcOnIntmAvatarChanged);
		CASE_MSG_RET(INTM_TIMEZONECHANGED, clcOnIntmTimeZoneChanged);
		CASE_MSG_RET(INTM_NAMECHANGED, clcOnIntmNameChanged);
		CASE_MSG_RET(INTM_APPARENTMODECHANGED, clcOnIntmApparentModeChanged);
		CASE_MSG_RET(INTM_STATUSMSGCHANGED, clcOnIntmStatusMsgChanged);
		CASE_MSG_RET(INTM_NOTONLISTCHANGED, clcOnIntmNotOnListChanged);
		CASE_MSG_RET(INTM_SCROLLBARCHANGED, clcOnIntmScrollBarChanged);
		CASE_MSG_RET(INTM_STATUSCHANGED, clcOnIntmStatusChanged);
		CASE_MSG_RET(INTM_RELOADOPTIONS, clcOnIntmReloadOptions);

		CASE_MSG_RET(WM_CREATE, clcOnCreate);
		CASE_MSG_RET(WM_NCHITTEST, clcOnHitTest);
		CASE_MSG_RET(WM_COMMAND, clcOnCommand);
		CASE_MSG_RET(WM_SIZE, clcOnSize);
		CASE_MSG_RET(WM_CHAR, clcOnChar);
		CASE_MSG_RET(WM_PAINT, clcOnPaint);
		CASE_MSG_RET(WM_ERASEBKGND, clcOnEraseBkGround);
		CASE_MSG_RET(WM_KEYDOWN, clcOnKeyDown);
		CASE_MSG_RET(WM_TIMER, clcOnTimer);
		CASE_MSG_RET(WM_ACTIVATE, clcOnActivate);
		CASE_MSG_RET(WM_SETCURSOR, clcOnSetCursor);
		CASE_MSG_RET(WM_LBUTTONDOWN, clcOnLButtonDown);
		CASE_MSG_RET(WM_CAPTURECHANGED, clcOnCaptureChanged);
		CASE_MSG_RET(WM_MOUSEMOVE, clcOnMouseMove);
		CASE_MSG_RET(WM_LBUTTONUP, clcOnLButtonUp);
		CASE_MSG_RET(WM_LBUTTONDBLCLK, clcOnLButtonDblClick);
		CASE_MSG_RET(WM_DESTROY, clcOnDestroy);

	default:
		return corecli.pfnContactListControlWndProc(hwnd, msg, wParam, lParam);
	}
}
