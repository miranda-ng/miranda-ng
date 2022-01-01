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

#include "stdafx.h"

//loads of stuff that didn't really fit anywhere else

BOOL RectHitTest(RECT *rc, int testx, int testy)
{
	return testx >= rc->left && testx < rc->right && testy >= rc->top && testy < rc->bottom;
}

int cliHitTest(HWND hwnd, ClcData *dat, int testx, int testy, ClcContact **contact, ClcGroup **group, uint32_t *flags)
{
	ClcContact *hitcontact = nullptr;
	ClcGroup *hitgroup = nullptr;
	int hit = -1;
	RECT clRect;
	if (CLUI_TestCursorOnBorders() != 0) {
		if (flags) *flags = CLCHT_NOWHERE;
		return -1;
	}
	if (flags) *flags = 0;
	GetClientRect(hwnd, &clRect);
	if (testx < 0 || testy < 0 || testy >= clRect.bottom || testx >= clRect.right) {
		if (flags) {
			if (testx < 0) *flags |= CLCHT_TOLEFT;
			else if (testx >= clRect.right) *flags |= CLCHT_TORIGHT;
			if (testy < 0) *flags |= CLCHT_ABOVE;
			else if (testy >= clRect.bottom) *flags |= CLCHT_BELOW;
		}
		return -1;
	}
	if (testx < dat->leftMargin) {
		if (flags) *flags |= CLCHT_INLEFTMARGIN | CLCHT_NOWHERE;
		return -1;
	}

	// Get hit item
	hit = cliRowHitTest(dat, dat->yScroll + testy);

	if (hit != -1)
		hit = cliGetRowByIndex(dat, hit, &hitcontact, &hitgroup);

	if (hit == -1) {
		if (flags) *flags |= CLCHT_NOWHERE | CLCHT_BELOWITEMS;
		return -1;
	}

	if (contact) *contact = hitcontact;
	if (group) *group = hitgroup;

	if (((testx < hitcontact->pos_indent) && !dat->text_rtl) || ((testx>clRect.right - hitcontact->pos_indent) && dat->text_rtl)) {
		if (flags) *flags |= CLCHT_ONITEMINDENT;
		return hit;
	}

	if (RectHitTest(&hitcontact->pos_check, testx, testy)) {
		if (flags) *flags |= CLCHT_ONITEMCHECK;
		return hit;
	}

	if (RectHitTest(&hitcontact->pos_avatar, testx, testy)) {
		if (flags) *flags |= CLCHT_ONITEMICON;
		return hit;
	}

	if (RectHitTest(&hitcontact->pos_icon, testx, testy)) {
		if (flags) *flags |= CLCHT_ONITEMICON;
		return hit;
	}

	for (int i = 0; i < dat->extraColumnsCount; i++) {
		if (RectHitTest(&hitcontact->pos_extra[i], testx, testy)) {
			if (flags) *flags |= CLCHT_ONITEMEXTRA | (i << 24);
			return hit;
		}
	}

	if (dat->HiLightMode == 1) {
		if (flags) *flags |= CLCHT_ONITEMLABEL;
		return hit;
	}

	if (RectHitTest(&hitcontact->pos_label, testx, testy)) {
		if (flags) *flags |= CLCHT_ONITEMLABEL;
		return hit;
	}

	if (flags) *flags |= CLCHT_NOWHERE;
	return hit;
}

void cliScrollTo(HWND hwnd, ClcData *dat, int desty, int noSmooth)
{
	uint32_t startTick, nowTick;
	int oldy = dat->yScroll;
	RECT clRect, rcInvalidate;
	int maxy, previousy;

	if (dat->iHotTrack != -1 && dat->yScroll != desty) {
		Clist_InvalidateItem(hwnd, dat, dat->iHotTrack);
		dat->iHotTrack = -1;
		ReleaseCapture();
	}
	GetClientRect(hwnd, &clRect);
	rcInvalidate = clRect;
	//maxy = dat->rowHeight*GetGroupContentsCount(&dat->list,2)-clRect.bottom;
	maxy = cliGetRowTotalHeight(dat) - clRect.bottom;
	if (desty > maxy) desty = maxy;
	if (desty < 0) desty = 0;
	if (abs(desty - dat->yScroll) < 4) noSmooth = 1;
	if (!noSmooth && dat->exStyle&CLS_EX_NOSMOOTHSCROLLING) noSmooth = 1;
	previousy = dat->yScroll;

	BOOL keyDown = ((GetKeyState(VK_UP)
		| GetKeyState(VK_DOWN)
		| GetKeyState(VK_LEFT)
		| GetKeyState(VK_RIGHT)
		| GetKeyState(VK_PRIOR)
		| GetKeyState(VK_NEXT)
		| GetKeyState(VK_HOME)
		| GetKeyState(VK_END)) & 0x8000);

	if (!noSmooth && !keyDown) {
		startTick = GetTickCount();
		for (;;) {
			nowTick = GetTickCount();
			if (nowTick >= startTick + dat->scrollTime) break;
			dat->yScroll = oldy + (desty - oldy)*(int)(nowTick - startTick) / dat->scrollTime;
			if (/*dat->backgroundBmpUse&CLBF_SCROLL || dat->hBmpBackground == nullptr  && */FALSE)
				ScrollWindowEx(hwnd, 0, previousy - dat->yScroll, nullptr, nullptr, nullptr, nullptr, SW_INVALIDATE);
			else
				CallService(MS_SKINENG_UPTATEFRAMEIMAGE, (WPARAM)hwnd, 0);

			previousy = dat->yScroll;
			SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
			CallService(MS_SKINENG_UPTATEFRAMEIMAGE, (WPARAM)hwnd, 0);
			UpdateWindow(hwnd);
		}
	}
	dat->yScroll = desty;
	cliInvalidateRect(hwnd, nullptr, FALSE);
	SetScrollPos(hwnd, SB_VERT, dat->yScroll, TRUE);
}

void cliRecalcScrollBar(HWND hwnd, ClcData *dat)
{
	if (dat->bLockScrollbar)
		return;

	RowHeights_CalcRowHeights(dat, hwnd);

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	SCROLLINFO si = { 0 };
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = cliGetRowTotalHeight(dat) - 1;
	si.nPage = clRect.bottom;
	si.nPos = dat->yScroll;

	NMCLISTCONTROL nm;
	nm.hdr.code = CLN_LISTSIZECHANGE;
	nm.hdr.hwndFrom = hwnd;
	nm.hdr.idFrom = 0;//GetDlgCtrlID(hwnd);
	nm.pt.y = si.nMax;
	SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&nm);       //post

	GetClientRect(hwnd, &clRect);
	si.cbSize = sizeof(si);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = cliGetRowTotalHeight(dat) - 1;
	si.nPage = clRect.bottom;
	si.nPos = dat->yScroll;

	if (GetWindowLongPtr(hwnd, GWL_STYLE)&CLS_CONTACTLIST) {
		if (!dat->bNoVScrollbar)
			SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	}
	else SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	
	g_bSizing = true;
	cliScrollTo(hwnd, dat, dat->yScroll, 1);
	g_bSizing = false;
}


static LRESULT CALLBACK RenameEditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_RETURN:
			Clist_EndRename((ClcData*)GetWindowLongPtr(hwnd, GWLP_USERDATA), 1);
			return 0;
		case VK_ESCAPE:
			Clist_EndRename((ClcData*)GetWindowLongPtr(hwnd, GWLP_USERDATA), 0);
			return 0;
		}
		break;
	
	case WM_GETDLGCODE:
		if (lParam) {
			MSG *msg = (MSG*)lParam;
			if (msg->message == WM_KEYDOWN && msg->wParam == VK_TAB) return 0;
			if (msg->message == WM_CHAR && msg->wParam == '\t') return 0;
		}
		return DLGC_WANTMESSAGE;
	
	case WM_KILLFOCUS:
		Clist_EndRename((ClcData*)GetWindowLongPtr(hwnd, GWLP_USERDATA), 1);
		SendMessage(g_clistApi.hwndContactTree, WM_SIZE, 0, 0);
		return 0;
	}
	return mir_callNextSubclass(hwnd, RenameEditSubclassProc, uMsg, wParam, lParam);
}

void cliBeginRenameSelection(HWND hwnd, ClcData *dat)
{
	KillTimer(hwnd, TIMERID_RENAME);
	ReleaseCapture();
	dat->iHotTrack = -1;

	ClcGroup *group;
	ClcContact *contact;
	dat->selection = cliGetRowByIndex(dat, dat->selection, &contact, &group);
	if (dat->selection == -1 || (contact->type != CLCIT_CONTACT && contact->type != CLCIT_GROUP))
		return;

	int subindent = (contact->type == CLCIT_CONTACT && contact->iSubNumber) ? dat->subIndent : 0;

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	POINT pt;
	Clist_CalcEipPosition(dat, contact, group, &pt);

	int x = pt.x + subindent, y = pt.y;
	int w = clRect.right - x;
	int h = dat->getRowHeight(dat->selection);

	for (int i = 0; i <= FONTID_MODERN_MAX; i++)
		if (h < dat->fontModernInfo[i].fontHeight + 4)
			h = dat->fontModernInfo[i].fontHeight + 4;

	RECT rectW;
	GetWindowRect(hwnd, &rectW);
	x += rectW.left;
	y += rectW.top;

	int a = 0;
	if (contact->type == CLCIT_GROUP) {
		if (dat->row_align_group_mode == 1)
			a |= ES_CENTER;
		else if (dat->row_align_group_mode == 2)
			a |= ES_RIGHT;
	}
	if (dat->text_rtl)
		a |= EN_ALIGN_RTL_EC;
	if (contact->type == CLCIT_GROUP)
		dat->hwndRenameEdit = CreateWindow(L"EDIT", contact->szText, WS_POPUP | WS_BORDER | ES_AUTOHSCROLL | a, x, y, w, h, hwnd, nullptr, g_plugin.getInst(), nullptr);
	else
		dat->hwndRenameEdit = CreateWindow(L"EDIT", Clist_GetContactDisplayName(contact->hContact), WS_POPUP | WS_BORDER | ES_AUTOHSCROLL | a, x, y, w, h, hwnd, nullptr, g_plugin.getInst(), nullptr);

	SetWindowLongPtr(dat->hwndRenameEdit, GWL_STYLE, GetWindowLongPtr(dat->hwndRenameEdit, GWL_STYLE)&(~WS_CAPTION) | WS_BORDER);
	SetWindowLongPtr(dat->hwndRenameEdit, GWLP_USERDATA, (LONG_PTR)dat);
	mir_subclassWindow(dat->hwndRenameEdit, RenameEditSubclassProc);
	SendMessage(dat->hwndRenameEdit, WM_SETFONT, (WPARAM)(contact->type == CLCIT_GROUP ? dat->fontModernInfo[FONTID_OPENGROUPS].hFont : dat->fontModernInfo[FONTID_CONTACTS].hFont), 0);
	SendMessage(dat->hwndRenameEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN | EC_USEFONTINFO, 0);
	SendMessage(dat->hwndRenameEdit, EM_SETSEL, 0, -1);

	RECT r;
	r.top = 1;
	r.bottom = h - 1;
	r.left = 0;
	r.right = w;
	SendMessage(dat->hwndRenameEdit, EM_SETRECT, 0, (LPARAM)&r);

	CLUI_ShowWindowMod(dat->hwndRenameEdit, SW_SHOW);
	SetWindowPos(dat->hwndRenameEdit, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetFocus(dat->hwndRenameEdit);
}

int GetDropTargetInformation(HWND hwnd, ClcData *dat, POINT pt)
{
	ClcContact *contact = nullptr, *movecontact = nullptr;
	ClcGroup *group, *movegroup;

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	dat->selection = dat->iDragItem;
	dat->iInsertionMark = -1;
	dat->nInsertionLevel = 0;
	if (!PtInRect(&clRect, pt))
		return DROPTARGET_OUTSIDE;

	uint32_t hitFlags;
	int hit = cliHitTest(hwnd, dat, pt.x, pt.y, &contact, &group, &hitFlags);
	cliGetRowByIndex(dat, dat->iDragItem, &movecontact, &movegroup);
	if (hit == dat->iDragItem) return DROPTARGET_ONSELF;
	if (hit == -1 || hitFlags & CLCHT_ONITEMEXTRA || !movecontact)
		return DROPTARGET_ONNOTHING;

	int nSetSelection = -1;
	if (movecontact->type == CLCIT_GROUP) {
		ClcContact *bottomcontact = nullptr, *topcontact = nullptr;
		ClcGroup *topgroup = nullptr, *bottomgroup = nullptr;
		int topItem = -1, bottomItem = -1;
		int ok = 0;
		if (pt.y + dat->yScroll < cliGetRowTopY(dat, hit) + dat->insertionMarkHitHeight || contact->type != CLCIT_GROUP) {
			//could be insertion mark (above)
			topItem = hit - 1; bottomItem = hit;
			bottomcontact = contact;
			bottomgroup = group;
			topItem = cliGetRowByIndex(dat, topItem, &topcontact, &topgroup);
			ok = 1;
		}
		else if ((pt.y + dat->yScroll >= cliGetRowTopY(dat, hit + 1) - dat->insertionMarkHitHeight)
			|| (contact->type == CLCIT_GROUP && contact->group->expanded && contact->group->cl.getCount() > 0)) {
			//could be insertion mark (below)
			topItem = hit; bottomItem = hit + 1;
			topcontact = contact; topgroup = group;
			bottomItem = cliGetRowByIndex(dat, bottomItem, &bottomcontact, &bottomgroup);
			ok = 1;
		}
		if (ok) {
			if (bottomItem == -1 && contact->type == CLCIT_GROUP) {
				bottomItem = topItem + 1;
			}
			else {
				if (bottomItem == -1 && contact->type != CLCIT_GROUP && contact->groupId == 0) {
					bottomItem = topItem;
					cliGetRowByIndex(dat, bottomItem, &bottomcontact, &bottomgroup);
				}
				if (bottomItem != -1 && bottomcontact->type != CLCIT_GROUP) {
					ClcGroup *gr = bottomgroup;
					do {
						bottomItem = cliGetRowByIndex(dat, bottomItem - 1, &bottomcontact, &bottomgroup);
					} while (bottomItem >= 0 && bottomcontact->type != CLCIT_GROUP && bottomgroup == gr);
					nSetSelection = bottomItem;
					bottomItem = cliGetRowByIndex(dat, bottomItem + 1, &bottomcontact, &bottomgroup);
				}
			}

			if (bottomItem == -1)	
				bottomItem = topItem + 1;

			int bi = cliGetRowByIndex(dat, bottomItem, &bottomcontact, &bottomgroup);
			if (bi != -1) {
				group = bottomgroup;
				if (bottomcontact == movecontact || group == movecontact->group)
					return DROPTARGET_ONSELF;
				
				dat->nInsertionLevel = -1; // decreasing here
				for (; group; group = group->parent) {
					dat->nInsertionLevel++;
					if (group == movecontact->group)
						return DROPTARGET_ONSELF;
				}
			}

			dat->iInsertionMark = bottomItem;
			dat->selection = nSetSelection;
			return DROPTARGET_INSERTION;
		}
	}

	if (contact->type == CLCIT_GROUP) {
		if (dat->iInsertionMark == -1) {
			if (movecontact->type == CLCIT_GROUP) {	 //check not moving onto its own subgroup
				dat->iInsertionMark = hit + 1;
				for (; group; group = group->parent) {
					dat->nInsertionLevel++;
					if (group == movecontact->group)
						return DROPTARGET_ONSELF;
				}
			}
			dat->selection = hit;
			return DROPTARGET_ONGROUP;
		}
	}
	dat->selection = hit;

	if (contact->pce)
		if (!mir_strcmp(contact->pce->szProto, META_PROTO))
			return DROPTARGET_ONMETACONTACT;

	if (contact->iSubNumber)
		return DROPTARGET_ONSUBCONTACT;

	return DROPTARGET_ONCONTACT;
}

COLORREF cliGetColor(char *module, char *color, COLORREF defColor)
{
	BOOL useWinColor = db_get_b(0, module, "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);
	if (useWinColor) return defColor;
	else return db_get_dw(0, module, color, defColor);
}

void RegisterCLUIFonts(void);

void LoadCLCFonts(HWND hwnd, ClcData *dat)
{
	RegisterCLUIFonts();

	HDC hdc = GetDC(hwnd);
	HFONT holdfont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);

	for (int i = 0; i <= FONTID_MODERN_MAX; i++) {
		if (!dat->fontModernInfo[i].changed && dat->fontModernInfo[i].hFont)
			DeleteObject(dat->fontModernInfo[i].hFont);

		// Issue 40: Do not reload font colors for embedded clists
		// Parent window is responsible to re-set fonts colors if needed
		LOGFONT lf;
		GetFontSetting(i, &lf, dat->bForceInDialog ? nullptr : &dat->fontModernInfo[i].colour, &dat->fontModernInfo[i].effect, &dat->fontModernInfo[i].effectColour1, &dat->fontModernInfo[i].effectColour2);
		dat->fontModernInfo[i].hFont = CreateFontIndirect(&lf);
		dat->fontModernInfo[i].changed = 0;

		SelectObject(hdc, dat->fontModernInfo[i].hFont);
		SIZE fontSize;
		GetTextExtentPoint32A(hdc, "x", 1, &fontSize);
		dat->fontModernInfo[i].fontHeight = fontSize.cy;
	}
	SelectObject(hdc, holdfont);
	ReleaseDC(hwnd, hdc);
}

void cli_LoadCLCOptions(HWND hwnd, ClcData *dat, BOOL bFirst)
{
	g_CluiData.fDisableSkinEngine = db_get_b(0, "ModernData", "DisableEngine", SETTING_DISABLESKIN_DEFAULT) != 0;

	LoadCLCFonts(hwnd, dat);

	g_CluiData.bSortByOrder[0] = g_plugin.getByte("SortBy1", SETTING_SORTBY1_DEFAULT);
	g_CluiData.bSortByOrder[1] = g_plugin.getByte("SortBy2", SETTING_SORTBY2_DEFAULT);
	g_CluiData.bSortByOrder[2] = g_plugin.getByte("SortBy3", SETTING_SORTBY3_DEFAULT);
	g_CluiData.fSortNoOfflineBottom = g_plugin.getByte("NoOfflineBottom", SETTING_NOOFFLINEBOTTOM_DEFAULT) != 0;

	// Row
	dat->row_min_heigh = g_plugin.getWord("MinRowHeight", CLCDEFAULT_ROWHEIGHT);
	dat->row_border = g_plugin.getWord("RowBorder", SETTING_ROWBORDER_DEFAULT);
	dat->row_before_group_space = ((hwnd != g_clistApi.hwndContactTree && g_clistApi.hwndContactTree != nullptr)
		|| !db_get_b(0, "ModernData", "UseAdvancedRowLayout", SETTING_ROW_ADVANCEDLAYOUT_DEFAULT)) ? 0 : db_get_w(0, "ModernSkin", "SpaceBeforeGroup", SKIN_SPACEBEFOREGROUP_DEFAULT);
	dat->row_variable_height = g_plugin.getByte("VariableRowHeight", SETTING_VARIABLEROWHEIGHT_DEFAULT);
	dat->row_align_left_items_to_left = g_plugin.getByte("AlignLeftItemsToLeft", SETTING_ALIGNLEFTTOLEFT_DEFAULT);
	dat->row_hide_group_icon = g_plugin.getByte("HideGroupsIcon", SETTING_HIDEGROUPSICON_DEFAULT);
	dat->row_align_right_items_to_right = g_plugin.getByte("AlignRightItemsToRight", SETTING_ALIGNRIGHTORIGHT_DEFAULT);
	//TODO: Add to settings
	dat->row_align_group_mode = g_plugin.getByte("AlignGroupCaptions", SETTING_ALIGNGROPCAPTION_DEFAULT);
	if (g_clistApi.hwndContactTree == nullptr || dat->hWnd == g_clistApi.hwndContactTree) {
		int defItemsOrder[NUM_ITEM_TYPE] = { ITEM_AVATAR, ITEM_ICON, ITEM_TEXT, ITEM_CONTACT_TIME, ITEM_EXTRA_ICONS };
		for (int i = 0; i < NUM_ITEM_TYPE; i++) {
			char tmp[128];
			mir_snprintf(tmp, "RowPos%d", i);
			dat->row_items[i] = g_plugin.getWord(tmp, defItemsOrder[i]);
		}
	}
	else {
		int defItems[NUM_ITEM_TYPE] = { ITEM_ICON, ITEM_TEXT, ITEM_EXTRA_ICONS, -1, -1 };
		memcpy(dat->row_items, defItems, sizeof(defItems));
	}

	// Avatar
	if (g_clistApi.hwndContactTree == hwnd || g_clistApi.hwndContactTree == nullptr) {
		dat->avatars_show = ServiceExists(MS_AV_GETAVATARBITMAP) && g_plugin.getByte("AvatarsShow", SETTINGS_SHOWAVATARS_DEFAULT);
		dat->avatars_draw_border = g_plugin.getByte("AvatarsDrawBorders", SETTINGS_AVATARDRAWBORDER_DEFAULT);
		dat->avatars_border_color = (COLORREF)g_plugin.getDword("AvatarsBorderColor", SETTINGS_AVATARBORDERCOLOR_DEFAULT);
		dat->avatars_round_corners = g_plugin.getByte("AvatarsRoundCorners", SETTINGS_AVATARROUNDCORNERS_DEFAULT);
		dat->avatars_use_custom_corner_size = g_plugin.getByte("AvatarsUseCustomCornerSize", SETTINGS_AVATARUSECUTOMCORNERSIZE_DEFAULT);
		dat->avatars_custom_corner_size = g_plugin.getWord("AvatarsCustomCornerSize", SETTINGS_AVATARCORNERSIZE_DEFAULT);
		dat->avatars_ignore_size_for_row_height = g_plugin.getByte("AvatarsIgnoreSizeForRow", SETTINGS_AVATARIGNORESIZEFORROW_DEFAULT);
		dat->avatars_draw_overlay = g_plugin.getByte("AvatarsDrawOverlay", SETTINGS_AVATARDRAWOVERLAY_DEFAULT);
		dat->avatars_overlay_type = g_plugin.getByte("AvatarsOverlayType", SETTINGS_AVATAROVERLAYTYPE_DEFAULT);
		dat->avatars_maxheight_size = g_plugin.getWord("AvatarsSize", SETTING_AVATARHEIGHT_DEFAULT);
		dat->avatars_maxwidth_size = g_plugin.getWord("AvatarsWidth", SETTING_AVATARWIDTH_DEFAULT);
	}
	else {
		dat->avatars_show = 0;
		dat->avatars_draw_border = 0;
		dat->avatars_border_color = 0;
		dat->avatars_round_corners = 0;
		dat->avatars_use_custom_corner_size = 0;
		dat->avatars_custom_corner_size = 4;
		dat->avatars_ignore_size_for_row_height = 0;
		dat->avatars_draw_overlay = 0;
		dat->avatars_overlay_type = SETTING_AVATAR_OVERLAY_TYPE_NORMAL;
		dat->avatars_maxheight_size = 30;
		dat->avatars_maxwidth_size = 0;
	}

	// Icon
	if (g_clistApi.hwndContactTree == hwnd || g_clistApi.hwndContactTree == nullptr) {
		dat->icon_hide_on_avatar = g_plugin.getByte("IconHideOnAvatar", SETTING_HIDEICONONAVATAR_DEFAULT);
		dat->icon_draw_on_avatar_space = g_plugin.getByte("IconDrawOnAvatarSpace", SETTING_ICONONAVATARPLACE_DEFAULT);
		dat->icon_ignore_size_for_row_height = g_plugin.getByte("IconIgnoreSizeForRownHeight", SETTING_ICONIGNORESIZE_DEFAULT);
	}
	else {
		dat->icon_hide_on_avatar = 0;
		dat->icon_draw_on_avatar_space = 0;
		dat->icon_ignore_size_for_row_height = 0;
	}

	// Contact time
	if (g_clistApi.hwndContactTree == hwnd || g_clistApi.hwndContactTree == nullptr) {
		dat->contact_time_show = g_plugin.getByte("ContactTimeShow", SETTING_SHOWTIME_DEFAULT);
		dat->contact_time_show_only_if_different = g_plugin.getByte("ContactTimeShowOnlyIfDifferent", SETTING_SHOWTIMEIFDIFF_DEFAULT);
	}
	else {
		dat->contact_time_show = 0;
		dat->contact_time_show_only_if_different = 0;
	}

	// Text
	dat->text_rtl = g_plugin.getByte("TextRTL", SETTING_TEXT_RTL_DEFAULT);
	dat->text_align_right = g_plugin.getByte("TextAlignToRight", SETTING_TEXT_RIGHTALIGN_DEFAULT);
	dat->text_replace_smileys = g_plugin.getByte("TextReplaceSmileys", SETTING_TEXT_SMILEY_DEFAULT);
	dat->text_resize_smileys = g_plugin.getByte("TextResizeSmileys", SETTING_TEXT_RESIZESMILEY_DEFAULT);
	dat->text_smiley_height = 0;
	dat->text_use_protocol_smileys = g_plugin.getByte("TextUseProtocolSmileys", SETTING_TEXT_PROTOSMILEY_DEFAULT);

	if (g_clistApi.hwndContactTree == hwnd || g_clistApi.hwndContactTree == nullptr)
		dat->text_ignore_size_for_row_height = g_plugin.getByte("TextIgnoreSizeForRownHeight", SETTING_TEXT_IGNORESIZE_DEFAULT);
	else
		dat->text_ignore_size_for_row_height = 0;

	// First line
	dat->first_line_draw_smileys = g_plugin.getByte("FirstLineDrawSmileys", SETTING_FIRSTLINE_SMILEYS_DEFAULT);
	dat->first_line_append_nick = g_plugin.getByte("FirstLineAppendNick", SETTING_FIRSTLINE_APPENDNICK_DEFAULT);
	g_bTrimText = g_plugin.getByte("TrimText", SETTING_FIRSTLINE_TRIMTEXT_DEFAULT) != 0;

	dat->rightMargin = db_get_b(0, "CLC", "RightMargin", CLCDEFAULT_RIGHTMARGIN);
	dat->topMargin = db_get_b(0, "CLC", "TopMargin", 0);
	dat->bottomMargin = db_get_b(0, "CLC", "BottomMargin", 0);
	dat->bForceInDialog = (g_clistApi.hwndContactTree) ? (hwnd != g_clistApi.hwndContactTree) : 0;
	dat->subIndent = db_get_b(0, "CLC", "SubIndent", CLCDEFAULT_GROUPINDENT);

	if (dat->hBmpBackground) { DeleteObject(dat->hBmpBackground); dat->hBmpBackground = nullptr; }
	if (dat->hMenuBackground) { DeleteObject(dat->hMenuBackground); dat->hMenuBackground = nullptr; }

	if (g_CluiData.fDisableSkinEngine) {
		dat->MenuBkColor = cliGetColor("Menu", "BkColour", CLCDEFAULT_BKCOLOUR);
		dat->MenuBkHiColor = cliGetColor("Menu", "SelBkColour", CLCDEFAULT_SELBKCOLOUR);

		dat->MenuTextColor = cliGetColor("Menu", "TextColour", CLCDEFAULT_TEXTCOLOUR);
		dat->MenuTextHiColor = cliGetColor("Menu", "SelTextColour", CLCDEFAULT_MODERN_SELTEXTCOLOUR);

		if (db_get_b(0, "Menu", "UseBitmap", CLCDEFAULT_USEBITMAP)) {
			ptrW tszBitmap(db_get_wsa(0, "Menu", "BkBitmap"));
			if (tszBitmap != nullptr)
				dat->hMenuBackground = Bitmap_Load(tszBitmap);
		}
		dat->MenuBmpUse = db_get_w(0, "Menu", "BkBmpUse", CLCDEFAULT_BKBMPUSE);
	}

	dat->IsMetaContactsEnabled = (!(GetWindowLongPtr(hwnd, GWL_STYLE)&CLS_MANUALUPDATE)) && db_get_b(0, META_PROTO, "Enabled", 1);

	if (g_clistApi.hwndContactTree == nullptr || dat->hWnd == g_clistApi.hwndContactTree)
		dat->bMetaIgnoreEmptyExtra = db_get_b(0, "CLC", "MetaIgnoreEmptyExtra", SETTING_METAIGNOREEMPTYEXTRA_DEFAULT) != 0;
	else
		dat->bMetaIgnoreEmptyExtra = false;

	dat->bMetaExpanding = db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT) != 0;

	dat->bPlaceOfflineToRoot = g_plugin.getByte("PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT) != 0;
	dat->drawOverlayedStatus = db_get_b(0, "CLC", "DrawOverlayedStatus", SETTING_DRAWOVERLAYEDSTATUS_DEFAULT);

	dat->dbbMetaHideExtra = db_get_b(0, "CLC", "MetaHideExtra", SETTING_METAHIDEEXTRA_DEFAULT);
	dat->dbbBlendInActiveState = db_get_b(0, "CLC", "BlendInActiveState", SETTING_BLENDINACTIVESTATE_DEFAULT);
	dat->dbbBlend25 = db_get_b(0, "CLC", "Blend25%", SETTING_BLENDINACTIVESTATE_DEFAULT);
	dat->bCompactMode = db_get_b(0, "CLC", "CompactMode", SETTING_COMPACTMODE_DEFAULT);

	corecli.pfnLoadClcOptions(hwnd, dat, bFirst);

	dat->selTextColour = db_get_dw(0, "CLC", "SelTextColour", CLCDEFAULT_MODERN_SELTEXTCOLOUR);
	dat->hotTextColour = db_get_dw(0, "CLC", "HotTextColour", CLCDEFAULT_MODERN_HOTTEXTCOLOUR);
	dat->quickSearchColour = db_get_dw(0, "CLC", "QuickSearchColour", CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR);
	dat->bUseWindowsColours = false; // because it's missing in the options
}

int ExpandMetaContact(HWND hwnd, ClcContact *contact, ClcData *dat)
{
	KillTimer(hwnd, TIMERID_SUBEXPAND);
	if (contact->type != CLCIT_CONTACT || contact->iSubAllocated == 0 || contact->bSubExpanded || !db_get_b(0, "CLC", "MetaExpanding", SETTING_METAEXPANDING_DEFAULT))
		return 0;

	contact->bSubExpanded = true;
	g_plugin.setByte(contact->hContact, "Expanded", contact->bSubExpanded);
	dat->bNeedsResort = true;
	g_clistApi.pfnSortCLC(hwnd, dat, 1);
	cliRecalcScrollBar(hwnd, dat);
	return contact->bSubExpanded;
}

int cliFindRowByText(HWND hwnd, ClcData *dat, const wchar_t *text, int prefixOk)
{
	ClcGroup *group = &dat->list;
	int testlen = (int)mir_wstrlen(text);
	int SubCount = 0;

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
			if (dat->bFilterSearch) {
				wchar_t *lowered_szText = CharLowerW(NEWWSTR_ALLOCA(cc->szText));
				wchar_t *lowered_text = CharLowerW(NEWWSTR_ALLOCA(text));
				found = wcsstr(lowered_szText, lowered_text) != nullptr;
			}
			else found = (prefixOk && !wcsnicmp(text, cc->szText, testlen)) || (!prefixOk && !mir_wstrcmpi(text, cc->szText));

			if (found) {
				ClcGroup *ccGroup = group;
				int ccScanIndex = group->scanIndex;
				for (; group; group = group->parent)
					g_clistApi.pfnSetGroupExpand(hwnd, dat, group, 1);
				return g_clistApi.pfnGetRowsPriorTo(&dat->list, ccGroup, ccScanIndex + SubCount);
			}
			
			if (cc->type == CLCIT_GROUP) {
				if (!(dat->exStyle & CLS_EX_QUICKSEARCHVISONLY) || cc->group->expanded) {
					group = cc->group;
					group->scanIndex = 0;
					SubCount = 0;
					continue;
				}
			}
		}

		if (cc->type == CLCIT_CONTACT && cc->iSubAllocated) {
			if (!(dat->exStyle & CLS_EX_QUICKSEARCHVISONLY) || cc->bSubExpanded) {
				for (int i = 0; i < cc->iSubAllocated; i++) {
					const ClcContact &ccSub = cc->subcontacts[i];

					bool found;
					if (dat->bFilterSearch) {
						wchar_t *lowered_szText = CharLowerW(NEWWSTR_ALLOCA(ccSub.szText));
						wchar_t *lowered_text = CharLowerW(NEWWSTR_ALLOCA(text));
						found = wcsstr(lowered_szText, lowered_text) != nullptr;
					}
					else found = (prefixOk && !wcsnicmp(text, ccSub.szText, testlen)) || (!prefixOk && !mir_wstrcmpi(text, ccSub.szText));

					if (found) {
						ClcGroup *ccGroup = group;
						int ccScanIndex = group->scanIndex;
						for (; group; group = group->parent)
							g_clistApi.pfnSetGroupExpand(hwnd, dat, group, 1);
						if (!cc->bSubExpanded)
							ExpandMetaContact(hwnd, cc, dat);
						return g_clistApi.pfnGetRowsPriorTo(&dat->list, ccGroup, ccScanIndex + SubCount + i + 1);
					}
				}
			}
		}

		if (cc->type == CLCIT_CONTACT && cc->iSubAllocated && cc->bSubExpanded)
			SubCount += cc->iSubAllocated;
		group->scanIndex++;
	}
	return -1;
}
