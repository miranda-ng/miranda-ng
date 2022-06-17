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

//loads of stuff that didn't really fit anywhere else

extern HANDLE hHideInfoTipEvent;

MIR_APP_DLL(wchar_t*) Clist_GetGroupCountsText(ClcData *dat, ClcContact *contact)
{
	if (contact->type != CLCIT_GROUP || !(dat->exStyle & CLS_EX_SHOWGROUPCOUNTS))
		return L"";

	ClcGroup *group = contact->group, *topgroup = group;
	int onlineCount = 0;
	int totalCount = group->totalMembers;
	group->scanIndex = 0;
	for (;;) {
		if (group->scanIndex == group->cl.getCount()) {
			if (group == topgroup)
				break;
			group = group->parent;
			group->scanIndex++;
			continue;
		}

		ClcContact *cc = group->cl[group->scanIndex];
		if (cc->type == CLCIT_GROUP) {
			group = cc->group;
			group->scanIndex = 0;
			totalCount += group->totalMembers;
			continue;
		}
		else if (cc->type == CLCIT_CONTACT)
			if (cc->flags & CONTACTF_ONLINE)
				onlineCount++;
		group->scanIndex++;
	}
	if (onlineCount == 0 && dat->exStyle & CLS_EX_HIDECOUNTSWHENEMPTY)
		return L"";

	static wchar_t szName[32];
	mir_snwprintf(szName, L"(%u/%u)", onlineCount, totalCount);
	return szName;
}

int fnHitTest(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, uint32_t * flags)
{
	ClcContact *hitcontact = nullptr;
	ClcGroup *hitgroup = nullptr;
	int indent, i;
	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (flags)
		*flags = 0;

	RECT clRect;
	GetClientRect(hwnd, &clRect);
	if (testx < 0 || testy < 0 || testy >= clRect.bottom || testx >= clRect.right) {
		if (flags) {
			if (testx < 0)
				*flags |= CLCHT_TOLEFT;
			else if (testx >= clRect.right)
				*flags |= CLCHT_TORIGHT;
			if (testy < 0)
				*flags |= CLCHT_ABOVE;
			else if (testy >= clRect.bottom)
				*flags |= CLCHT_BELOW;
		}
		return -1;
	}
	if (testx < dat->leftMargin) {
		if (flags)
			*flags |= CLCHT_INLEFTMARGIN | CLCHT_NOWHERE;
		return -1;
	}
	int hit = g_clistApi.pfnRowHitTest(dat, dat->yScroll + testy);
	if (hit != -1)
		hit = g_clistApi.pfnGetRowByIndex(dat, hit, &hitcontact, &hitgroup);
	if (hit == -1) {
		if (flags)
			*flags |= CLCHT_NOWHERE | CLCHT_BELOWITEMS;
		return -1;
	}
	if (contact)
		*contact = hitcontact;
	if (group)
		*group = hitgroup;
	for (indent = 0; hitgroup->parent; indent++, hitgroup = hitgroup->parent);
	if (testx < dat->leftMargin + indent * dat->groupIndent) {
		if (flags)
			*flags |= CLCHT_ONITEMINDENT;
		return hit;
	}
	int checkboxWidth = 0;
	if (style & CLS_CHECKBOXES && hitcontact->type == CLCIT_CONTACT)
		checkboxWidth = dat->checkboxSize + 2;
	if (style & CLS_GROUPCHECKBOXES && hitcontact->type == CLCIT_GROUP)
		checkboxWidth = dat->checkboxSize + 2;
	if (hitcontact->type == CLCIT_INFO && hitcontact->flags & CLCIIF_CHECKBOX)
		checkboxWidth = dat->checkboxSize + 2;
	if (testx < dat->leftMargin + indent * dat->groupIndent + checkboxWidth) {
		if (flags)
			*flags |= CLCHT_ONITEMCHECK;
		return hit;
	}
	if (testx < dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace) {
		if (flags)
			*flags |= CLCHT_ONITEMICON;
		return hit;
	}

	int eiOffset = 0;
	for (i = dat->extraColumnsCount-1; i >= 0; i--) {
		if (hitcontact->iExtraImage[i] == EMPTY_EXTRA_ICON)
			continue;

		eiOffset += dat->extraColumnSpacing;
		if (testx >= clRect.right - eiOffset && testx < clRect.right - eiOffset + g_IconWidth) {
			if (flags)
				*flags |= CLCHT_ONITEMEXTRA | (i << 24);
			return hit;
		}
	}

	HDC hdc = GetDC(hwnd);
	HFONT hFont = (HFONT)SelectObject(hdc, dat->fontInfo[hitcontact->type == CLCIT_GROUP ? FONTID_GROUPS : FONTID_CONTACTS].hFont);

	SIZE textSize;
	GetTextExtentPoint32(hdc, hitcontact->szText, (int)mir_wstrlen(hitcontact->szText), &textSize);
	int width = textSize.cx;
	if (hitcontact->type == CLCIT_GROUP) {
		wchar_t *szCounts = Clist_GetGroupCountsText(dat, hitcontact);
		if (szCounts[0]) {
			GetTextExtentPoint32(hdc, L" ", 1, &textSize);
			width += textSize.cx;
			SelectObject(hdc, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
			GetTextExtentPoint32(hdc, szCounts, (int)mir_wstrlen(szCounts), &textSize);
			width += textSize.cx;
		}
	}
	SelectObject(hdc, hFont);
	ReleaseDC(hwnd, hdc);
	if (testx < dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 4) {
		if (flags)
			*flags |= CLCHT_ONITEMLABEL;
		return hit;
	}
	if (flags)
		*flags |= CLCHT_NOWHERE;
	return -1;
}

void fnScrollTo(HWND hwnd, ClcData *dat, int desty, int noSmooth)
{
	int oldy = dat->yScroll;

	if (dat->iHotTrack != -1 && dat->yScroll != desty) {
		Clist_InvalidateItem(hwnd, dat, dat->iHotTrack);
		dat->iHotTrack = -1;
		ReleaseCapture();
	}

	RECT clRect;
	GetClientRect(hwnd, &clRect);
	
	int maxy = g_clistApi.pfnGetRowTotalHeight(dat) - clRect.bottom;
	if (desty > maxy)
		desty = maxy;
	if (desty < 0)
		desty = 0;
	if (abs(desty - dat->yScroll) < 4)
		noSmooth = 1;
	if (!noSmooth && dat->exStyle & CLS_EX_NOSMOOTHSCROLLING)
		noSmooth = 1;
	
	int previousy = dat->yScroll;
	if (!noSmooth) {
		uint32_t startTick = GetTickCount();
		for (;;) {
			uint32_t nowTick = GetTickCount();
			if (nowTick >= startTick + dat->scrollTime)
				break;
			
			dat->yScroll = oldy + (desty - oldy) * (int)(nowTick - startTick) / dat->scrollTime;
			if (dat->backgroundBmpUse & CLBF_SCROLL || dat->hBmpBackground == nullptr)
				ScrollWindowEx(hwnd, 0, previousy - dat->yScroll, nullptr, nullptr, nullptr, nullptr, SW_INVALIDATE);
			else
				g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
			previousy = dat->yScroll;
			SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
			UpdateWindow(hwnd);
		}
	}
	
	dat->yScroll = desty;
	if (dat->backgroundBmpUse & CLBF_SCROLL || dat->hBmpBackground == nullptr)
		ScrollWindowEx(hwnd, 0, previousy - dat->yScroll, nullptr, nullptr, nullptr, nullptr, SW_INVALIDATE);
	else
		g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
	SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
}

MIR_APP_DLL(void) Clist_EnsureVisible(HWND hwnd, ClcData *dat, int iItem, int partialOk)
{
	int itemy = g_clistApi.pfnGetRowTopY(dat, iItem), itemh = g_clistApi.pfnGetRowHeight(dat, iItem), newY = 0;
	int moved = 0;
	RECT clRect;

	GetClientRect(hwnd, &clRect);
	if (partialOk) {
		if (itemy + itemh - 1 < dat->yScroll) {
			newY = itemy;
			moved = 1;
		}
		else if (itemy >= dat->yScroll + clRect.bottom) {
			newY = itemy - clRect.bottom + itemh;
			moved = 1;
		}
	}
	else {
		if (itemy < dat->yScroll) {
			newY = itemy;
			moved = 1;
		}
		else if (itemy >= dat->yScroll + clRect.bottom - itemh) {
			newY = itemy - clRect.bottom + itemh;
			moved = 1;
		}
	}
	if (moved)
		g_clistApi.pfnScrollTo(hwnd, dat, newY, 0);
}

void fnRecalcScrollBar(HWND hwnd, ClcData *dat)
{
	if (dat->bLockScrollbar)
		return;

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = g_clistApi.pfnGetRowTotalHeight(dat)-1;
	si.nPage = clRect.bottom;
	si.nPos = dat->yScroll;

	if (GetWindowLongPtr(hwnd, GWL_STYLE) & CLS_CONTACTLIST) {
		if (!dat->bNoVScrollbar)
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	}
	else SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

	g_clistApi.pfnScrollTo(hwnd, dat, dat->yScroll, 1);

	NMCLISTCONTROL nm;
	nm.hdr.code = CLN_LISTSIZECHANGE;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	nm.pt.y = si.nMax;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
}

void fnSetGroupExpand(HWND hwnd, ClcData *dat, ClcGroup *group, int newState)
{
	int contentCount;
	int groupy;
	int newY, posY;
	RECT clRect;
	NMCLISTCONTROL nm;

	if (newState == -1)
		group->expanded ^= 1;
	else {
		if (group->expanded == (newState != 0))
			return;
		group->expanded = newState != 0;
	}
	g_clistApi.pfnInvalidateRect(hwnd, nullptr, FALSE);
	contentCount = g_clistApi.pfnGetGroupContentsCount(group, 1);
	groupy = g_clistApi.pfnGetRowsPriorTo(&dat->list, group, -1);
	if (dat->selection > groupy && dat->selection < groupy + contentCount)
		dat->selection = groupy;
	GetClientRect(hwnd, &clRect);
	newY = dat->yScroll;
	posY = g_clistApi.pfnGetRowBottomY(dat, groupy + contentCount);
	if (posY >= newY + clRect.bottom)
		newY = posY - clRect.bottom;
	posY = g_clistApi.pfnGetRowTopY(dat, groupy);
	if (newY > posY)
		newY = posY;
	g_clistApi.pfnRecalcScrollBar(hwnd, dat);
	if (group->expanded)
		g_clistApi.pfnScrollTo(hwnd, dat, newY, 0);
	nm.hdr.code = CLN_EXPANDED;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	nm.hItem = (HANDLE)group->groupId;
	nm.action = group->expanded;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
}

MIR_APP_DLL(void) Clist_DoSelectionDefaultAction(HWND hwnd, ClcData *dat)
{
	if (dat->selection == -1)
		return;

	bool bSearchExisted = dat->szQuickSearch[0] != 0;
	dat->szQuickSearch[0] = 0;

	ClcContact *contact;
	if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) == -1)
		return;

	if (contact->type == CLCIT_GROUP)
		g_clistApi.pfnSetGroupExpand(hwnd, dat, contact->group, -1);
	if (contact->type == CLCIT_CONTACT)
		Clist_ContactDoubleClicked(contact->hContact);

	if (dat->bFilterSearch && bSearchExisted)
		Clist_SaveStateAndRebuildList(hwnd, dat);
}

int fnFindRowByText(HWND hwnd, ClcData *dat, const wchar_t *text, int prefixOk)
{
	ClcGroup *group = &dat->list;
	size_t testlen = mir_wstrlen(text);

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
			bool show;
			if (dat->bFilterSearch) {
				wchar_t *lowered_szText = CharLowerW(NEWWSTR_ALLOCA(cc->szText));
				wchar_t *lowered_text = CharLowerW(NEWWSTR_ALLOCA(text));
				show = wcsstr(lowered_szText, lowered_text) != nullptr;
			}
			else show = ((prefixOk && !wcsnicmp(text, cc->szText, testlen)) || (!prefixOk && !mir_wstrcmpi(text, cc->szText)));

			if (show) {
				ClcGroup *contactGroup = group;
				int contactScanIndex = group->scanIndex;
				for (; group; group = group->parent)
					g_clistApi.pfnSetGroupExpand(hwnd, dat, group, 1);
				return g_clistApi.pfnGetRowsPriorTo(&dat->list, contactGroup, contactScanIndex);
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

MIR_APP_DLL(void) Clist_EndRename(ClcData *dat, int save)
{
	HWND hwndEdit = dat->hwndRenameEdit;
	if (hwndEdit == nullptr)
		return;

	dat->hwndRenameEdit = nullptr;
	if (save) {
		wchar_t text[120]; text[0] = 0;
		GetWindowText(hwndEdit, text, _countof(text));

		ClcContact *contact;
		if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) != -1) {
			if (mir_wstrcmp(contact->szText, text) && !wcschr(text, '\\')) {
				if (contact->type == CLCIT_GROUP) {
					if (contact->group->parent && contact->group->parent->parent) {
						wchar_t szFullName[256];
						mir_snwprintf(szFullName, L"%s\\%s",
							Clist_GroupGetName(contact->group->parent->groupId, nullptr), text);
						Clist_GroupRename(contact->groupId, szFullName);
					}
					else
						Clist_GroupRename(contact->groupId, text);
				}
				else if (contact->type == CLCIT_CONTACT) {
					g_clistApi.pfnInvalidateDisplayNameCacheEntry(contact->hContact);
					wchar_t* otherName = Clist_GetContactDisplayName(contact->hContact, GCDNF_NOMYHANDLE);
					if (!text[0] || !mir_wstrcmp(otherName, text))
						db_unset(contact->hContact, "CList", "MyHandle");
					else
						db_set_ws(contact->hContact, "CList", "MyHandle", text);
					mir_free(otherName);
				}
			}
		}
	}
	DestroyWindow(hwndEdit);
}

MIR_APP_DLL(void) Clist_DeleteFromContactList(HWND hwnd, ClcData *dat)
{
	ClcContact *contact;
	if (dat->selection == -1)
		return;
	
	dat->szQuickSearch[0] = 0;
	if (g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, nullptr) == -1)
		return;
	
	switch (contact->type) {
	case CLCIT_GROUP:
		Clist_GroupDelete(contact->groupId);
		break;
	case CLCIT_CONTACT:
		CallService(MS_CLIST_DELETECONTACT, (WPARAM)contact->hContact, (LPARAM)hwnd);
		break;
	}
}

static LRESULT CALLBACK RenameEditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			Clist_EndRename((ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 1);
			return 0;
		case VK_ESCAPE:
			Clist_EndRename((ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 0);
			return 0;
		}
		break;
	
	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg = (MSG*)lParam;
			if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB)
				return 0;
			if (msg->message == WM_CHAR && msg->wParam == '\t')
				return 0;
		}
		return DLGC_WANTMESSAGE;
	
	case WM_KILLFOCUS:
		Clist_EndRename((ClcData *) GetWindowLongPtr(GetParent(hwnd), 0), 1);
		return 0;
	}
	return mir_callNextSubclass(hwnd, RenameEditSubclassProc, uMsg, wParam, lParam);
}

void fnBeginRenameSelection(HWND hwnd, ClcData *dat)
{
	KillTimer(hwnd, TIMERID_RENAME);
	ReleaseCapture();
	dat->iHotTrack = -1;

	ClcGroup *group;
	ClcContact *contact;
	dat->selection = g_clistApi.pfnGetRowByIndex(dat, dat->selection, &contact, &group);
	if (dat->selection == -1 || (contact->type != CLCIT_CONTACT && contact->type != CLCIT_GROUP))
		return;

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	POINT pt;
	Clist_CalcEipPosition(dat, contact, group, &pt);
	int h = g_clistApi.pfnGetRowHeight(dat, dat->selection);
	dat->hwndRenameEdit = CreateWindow(L"EDIT", contact->szText, WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, pt.x, pt.y, clRect.right - pt.x, h, hwnd, nullptr, g_clistApi.hInst, nullptr);
	mir_subclassWindow(dat->hwndRenameEdit, RenameEditSubclassProc);
	SendMessage(dat->hwndRenameEdit, WM_SETFONT, (WPARAM)(contact->type == CLCIT_GROUP ? dat->fontInfo[FONTID_GROUPS].hFont : dat->fontInfo[FONTID_CONTACTS].hFont), 0);
	SendMessage(dat->hwndRenameEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN | EC_USEFONTINFO, 0);
	SendMessage(dat->hwndRenameEdit, EM_SETSEL, 0, (LPARAM)(-1));
	ShowWindow(dat->hwndRenameEdit, SW_SHOW);
	SetFocus(dat->hwndRenameEdit);
}

MIR_APP_DLL(void) Clist_CalcEipPosition(ClcData *dat, ClcContact *, ClcGroup *group, POINT *result)
{
	int indent;
	for (indent = 0; group->parent; indent++, group = group->parent);
	result->x = indent * dat->groupIndent + dat->iconXSpace - 2;
	result->y = g_clistApi.pfnGetRowTopY(dat, dat->selection) - dat->yScroll;
}

int GetDropTargetInformation(HWND hwnd, ClcData *dat, POINT pt)
{
	RECT clRect;
	GetClientRect(hwnd, &clRect);
	dat->selection = dat->iDragItem;
	dat->iInsertionMark = -1;
	if (!PtInRect(&clRect, pt))
		return DROPTARGET_OUTSIDE;

	ClcContact *contact, *movecontact;
	ClcGroup *group, *movegroup;
	uint32_t hitFlags;
	int hit = g_clistApi.pfnHitTest(hwnd, dat, pt.x, pt.y, &contact, &group, &hitFlags);
	g_clistApi.pfnGetRowByIndex(dat, dat->iDragItem, &movecontact, &movegroup);
	if (hit == dat->iDragItem)
		return DROPTARGET_ONSELF;
	if (hit == -1 || movecontact == nullptr || (hitFlags & CLCHT_ONITEMEXTRA))
		return DROPTARGET_ONNOTHING;

	if (movecontact->type == CLCIT_GROUP) {
		ClcContact *bottomcontact = nullptr, *topcontact = nullptr;
		ClcGroup *topgroup = nullptr;
		int topItem = -1, bottomItem = -1;
		int ok = 0;
		if (pt.y + dat->yScroll < g_clistApi.pfnGetRowTopY(dat, hit) + dat->insertionMarkHitHeight) {
			//could be insertion mark (above)
			topItem = hit - 1;
			bottomItem = hit;
			bottomcontact = contact;
			topItem = g_clistApi.pfnGetRowByIndex(dat, topItem, &topcontact, &topgroup);
			ok = 1;
		}
		if (pt.y + dat->yScroll >= g_clistApi.pfnGetRowBottomY(dat, hit+1) - dat->insertionMarkHitHeight) {
			//could be insertion mark (below)
			topItem = hit;
			bottomItem = hit + 1;
			topcontact = contact;
			topgroup = group;
			bottomItem = g_clistApi.pfnGetRowByIndex(dat, bottomItem, &bottomcontact, nullptr);
			ok = 1;
		}
		if (ok) {
			ok = 0;
			if (bottomItem == -1 || bottomcontact->type != CLCIT_GROUP) { //need to special-case moving to end
				if (topItem != dat->iDragItem) {
					for (; topgroup; topgroup = topgroup->parent) {
						if (topgroup == movecontact->group)
							break;
						if (topgroup == movecontact->group->parent) {
							ok = 1;
							break;
						}
					}
					if (ok)
						bottomItem = topItem + 1;
				}
			}
			else if (bottomItem != dat->iDragItem && bottomcontact->type == CLCIT_GROUP && bottomcontact->group->parent == movecontact->group->parent) {
				if (bottomcontact != movecontact + 1)
					ok = 1;
			}
			if (ok) {
				dat->iInsertionMark = bottomItem;
				dat->selection = -1;
				return DROPTARGET_INSERTION;
			}
		}
	}
	if (contact->type == CLCIT_GROUP) {
		if (dat->iInsertionMark == -1) {
			if (movecontact->type == CLCIT_GROUP) { //check not moving onto its own subgroup
				for (; group; group = group->parent)
					if (group == movecontact->group)
						return DROPTARGET_ONSELF;
			}
			dat->selection = hit;
			return DROPTARGET_ONGROUP;
		}
	}
	return DROPTARGET_ONCONTACT;
}

MIR_APP_DLL(int) Clist_ClcStatusToPf2(int status)
{
	switch(status) {
		case ID_STATUS_ONLINE: return PF2_ONLINE;
		case ID_STATUS_AWAY: return PF2_SHORTAWAY;
		case ID_STATUS_DND: return PF2_HEAVYDND;
		case ID_STATUS_NA: return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED: return PF2_LIGHTDND;
		case ID_STATUS_FREECHAT: return PF2_FREECHAT;
		case ID_STATUS_INVISIBLE: return PF2_INVISIBLE;
		case ID_STATUS_OFFLINE: return MODEF_OFFLINE;
	}
	return 0;
}

MIR_APP_DLL(int) Clist_IsHiddenMode(ClcData *dat, int status)
{
	return dat->offlineModes & Clist_ClcStatusToPf2(status);
}

MIR_APP_DLL(void) Clist_HideInfoTip(ClcData *dat)
{
	if (dat->hInfoTipItem == 0)
		return;

	CLCINFOTIP it = { 0 };
	it.isGroup = IsHContactGroup(dat->hInfoTipItem);
	it.hItem = (HANDLE)(dat->hInfoTipItem & ~HCONTACT_ISGROUP);
	it.cbSize = sizeof(it);
	dat->hInfoTipItem = 0;
	NotifyEventHooks(hHideInfoTipEvent, 0, (LPARAM)&it);
}

MIR_APP_DLL(void) Clist_NotifyNewContact(HWND hwnd, MCONTACT hContact)
{
	NMCLISTCONTROL nm;
	nm.hdr.code = CLN_NEWCONTACT;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = GetDlgCtrlID(hwnd);
	nm.flags = 0;
	nm.hItem = (HANDLE)hContact;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);
}

MIR_APP_DLL(uint32_t) Clist_GetDefaultExStyle(void)
{
	BOOL param;
	uint32_t ret = CLCDEFAULT_EXSTYLE;
	if (SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING, 0, &param, FALSE) && !param)
		ret |= CLS_EX_NOSMOOTHSCROLLING;
	if (SystemParametersInfo(SPI_GETHOTTRACKING, 0, &param, FALSE) && !param)
		ret &= ~CLS_EX_TRACKSELECT;
	return ret;
}

#define DBFONTF_BOLD       1
#define DBFONTF_ITALIC     2
#define DBFONTF_UNDERLINE  4

void fnGetDefaultFontSetting(int i, LOGFONT *lf, COLORREF *colour)
{
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), lf, FALSE);
	*colour = GetSysColor(COLOR_WINDOWTEXT);

	HDC hdc = GetDC(nullptr);
	lf->lfHeight = -MulDiv(lf->lfHeight, 72, GetDeviceCaps(hdc, LOGPIXELSY));
	ReleaseDC(nullptr, hdc);

	switch (i) {
	case FONTID_GROUPS:
		lf->lfWeight = FW_BOLD;
		break;
	case FONTID_GROUPCOUNTS:
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	case FONTID_OFFINVIS:
	case FONTID_INVIS:
		lf->lfItalic = !lf->lfItalic;
		break;
	case FONTID_DIVIDERS:
		break;
	case FONTID_NOTONLIST:
		*colour = GetSysColor(COLOR_3DSHADOW);
		break;
	}
}

MIR_APP_DLL(void) Clist_GetFontSetting(int i, LOGFONT *lf, COLORREF *colour)
{
	g_clistApi.pfnGetDefaultFontSetting(i, lf, colour);

	char idstr[20];
	mir_snprintf(idstr, "Font%dName", i);
	ptrW tszFace(db_get_wsa(0, "CLC", idstr));
	if (tszFace)
		mir_wstrcpy(lf->lfFaceName, tszFace);

	mir_snprintf(idstr, "Font%dCol", i);
	*colour = db_get_dw(0, "CLC", idstr, *colour);

	mir_snprintf(idstr, "Font%dSize", i);
	lf->lfHeight = (char)db_get_b(0, "CLC", idstr, lf->lfHeight);

	mir_snprintf(idstr, "Font%dSty", i);
	uint8_t style = (uint8_t)db_get_b(0, "CLC", idstr, (lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf->lfItalic ? DBFONTF_ITALIC : 0) | (lf->lfUnderline ? DBFONTF_UNDERLINE : 0));
	lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
	lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
	lf->lfItalic = (style & DBFONTF_ITALIC) != 0;
	lf->lfUnderline = (style & DBFONTF_UNDERLINE) != 0;
	lf->lfStrikeOut = 0;

	mir_snprintf(idstr, "Font%dSet", i);
	lf->lfCharSet = db_get_b(0, "CLC", idstr, lf->lfCharSet);
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf->lfQuality = DEFAULT_QUALITY;
	lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
}

void fnLoadClcOptions(HWND hwnd, ClcData *dat, BOOL bFirst)
{
	dat->rowHeight = db_get_b(0, "CLC", "RowHeight", CLCDEFAULT_ROWHEIGHT);
	dat->leftMargin = db_get_b(0, "CLC", "LeftMargin", CLCDEFAULT_LEFTMARGIN);
	dat->exStyle = db_get_dw(0, "CLC", "ExStyle", Clist_GetDefaultExStyle());
	dat->scrollTime = db_get_w(0, "CLC", "ScrollTime", CLCDEFAULT_SCROLLTIME);
	dat->groupIndent = db_get_b(0, "CLC", "GroupIndent", CLCDEFAULT_GROUPINDENT);
	dat->gammaCorrection = db_get_b(0, "CLC", "GammaCorrect", CLCDEFAULT_GAMMACORRECT);
	dat->bShowIdle = db_get_b(0, "CLC", "ShowIdle", CLCDEFAULT_SHOWIDLE) != 0;
	dat->bNoVScrollbar = db_get_b(0, "CLC", "NoVScrollBar", false) != 0;
	dat->bFilterSearch = Clist::FilterSearch;
	SendMessage(hwnd, INTM_SCROLLBARCHANGED, 0, 0);

	dat->greyoutFlags = db_get_dw(0, "CLC", "GreyoutFlags", CLCDEFAULT_GREYOUTFLAGS);
	dat->offlineModes = Clist::OfflineModes;
	dat->selBkColour = db_get_dw(0, "CLC", "SelBkColour", CLCDEFAULT_SELBKCOLOUR);
	dat->selTextColour = db_get_dw(0, "CLC", "SelTextColour", CLCDEFAULT_SELTEXTCOLOUR);
	dat->hotTextColour = db_get_dw(0, "CLC", "HotTextColour", CLCDEFAULT_HOTTEXTCOLOUR);
	dat->quickSearchColour = db_get_dw(0, "CLC", "QuickSearchColour", CLCDEFAULT_QUICKSEARCHCOLOUR);
	dat->bUseWindowsColours = db_get_b(0, "CLC", "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS) != 0;

	if (g_clistApi.hwndContactTree != nullptr && hwnd != g_clistApi.hwndContactTree) {
		dat->bkChanged = true; // block custom background
		dat->bkColour = GetSysColor(COLOR_WINDOW);
		if (dat->hBmpBackground) {
			DeleteObject(dat->hBmpBackground);
			dat->hBmpBackground = nullptr;
		}

		dat->greyoutFlags = 0;
		dat->leftMargin = 4;
		dat->groupIndent = 10;

		LPARAM dwColor = GetSysColor(COLOR_WINDOWTEXT);
		for (int i = 0; i <= FONTID_MAX; i++)
			SendMessage(hwnd, CLM_SETTEXTCOLOR, i, dwColor);
	}

	if (!dat->bkChanged) {
		dat->bkColour = db_get_dw(0, "CLC", "BkColour", CLCDEFAULT_BKCOLOUR);
		if (dat->hBmpBackground) {
			DeleteObject(dat->hBmpBackground);
			dat->hBmpBackground = nullptr;
		}
		if (db_get_b(0, "CLC", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmap(db_get_wsa(0, "CLC", "BkBitmap"));
			if (tszBitmap)
				dat->hBmpBackground = Bitmap_Load(tszBitmap);
		}
		dat->backgroundBmpUse = db_get_w(0, "CLC", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}

	NMHDR hdr;
	hdr.code = CLN_OPTIONSCHANGED;
	hdr.hwndFrom = hwnd;
	hdr.idFrom = (bFirst) ? 0 : GetDlgCtrlID(hwnd);
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&hdr);

	SendMessage(hwnd, WM_SIZE, 0, 0);
}

#define GSIF_HASMEMBERS   0x80000000
#define GSIF_ALLCHECKED   0x40000000
#define GSIF_INDEXMASK    0x3FFFFFFF

MIR_APP_DLL(void) Clist_RecalculateGroupCheckboxes(ClcData *dat)
{
	ClcGroup *group = &dat->list;
	group->scanIndex = GSIF_ALLCHECKED;
	for (;;) {
		if ((group->scanIndex & GSIF_INDEXMASK) == group->cl.getCount()) {
			int check = (group->scanIndex & (GSIF_HASMEMBERS | GSIF_ALLCHECKED)) == (GSIF_HASMEMBERS | GSIF_ALLCHECKED);
			if (group->parent == nullptr)
				break;
			group->parent->scanIndex |= group->scanIndex & GSIF_HASMEMBERS;
			group = group->parent;
			if (check)
				group->cl[(group->scanIndex & GSIF_INDEXMASK)]->flags |= CONTACTF_CHECKED;
			else {
				group->cl[(group->scanIndex & GSIF_INDEXMASK)]->flags &= ~CONTACTF_CHECKED;
				group->scanIndex &= ~GSIF_ALLCHECKED;
			}
		}
		else if (group->cl[(group->scanIndex & GSIF_INDEXMASK)]->type == CLCIT_GROUP) {
			group = group->cl[(group->scanIndex & GSIF_INDEXMASK)]->group;
			group->scanIndex = GSIF_ALLCHECKED;
			continue;
		}
		else if (group->cl[(group->scanIndex & GSIF_INDEXMASK)]->type == CLCIT_CONTACT) {
			group->scanIndex |= GSIF_HASMEMBERS;
			if (!(group->cl[(group->scanIndex & GSIF_INDEXMASK)]->flags & CONTACTF_CHECKED))
				group->scanIndex &= ~GSIF_ALLCHECKED;
		}
		group->scanIndex++;
	}
}

void fnSetContactCheckboxes(ClcContact *cc, int checked)
{
	if (checked)
		cc->flags |= CONTACTF_CHECKED;
	else
		cc->flags &= ~CONTACTF_CHECKED;
}

MIR_APP_DLL(void) Clist_SetGroupChildCheckboxes(ClcGroup *group, int checked)
{
	for (auto &cc : group->cl) {
		if (cc->type == CLCIT_GROUP) {
			Clist_SetGroupChildCheckboxes(cc->group, checked);
			g_clistApi.pfnSetContactCheckboxes(cc, checked);
		}
		else if (cc->type == CLCIT_CONTACT)
			g_clistApi.pfnSetContactCheckboxes(cc, checked);
	}
}

MIR_APP_DLL(void) Clist_InvalidateItem(HWND hwnd, ClcData *dat, int iItem)
{
	if (iItem == -1)
		return;

	RECT rc;
	GetClientRect(hwnd, &rc);
	rc.top = g_clistApi.pfnGetRowTopY(dat, iItem) - dat->yScroll;
	rc.bottom = rc.top + g_clistApi.pfnGetRowHeight(dat, iItem);
	g_clistApi.pfnInvalidateRect(hwnd, &rc, FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// row coord functions

int fnGetRowTopY(ClcData *dat, int item)
{
	return item * dat->rowHeight;
}

int fnGetRowBottomY(ClcData *dat, int item)
{
	return (item+1) * dat->rowHeight;
}

int fnGetRowTotalHeight(ClcData *dat)
{
	return dat->rowHeight * g_clistApi.pfnGetGroupContentsCount(&dat->list, 1);
}

int fnGetRowHeight(ClcData *dat, int)
{
	return dat->rowHeight;
}

int fnRowHitTest(ClcData *dat, int y)
{
	if (!dat->rowHeight)
		return y;
	return y / dat->rowHeight;
}
