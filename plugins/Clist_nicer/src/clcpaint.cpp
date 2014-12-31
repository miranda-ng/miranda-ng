/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "commonheaders.h"

extern struct avatarCache *g_avatarCache;
extern int g_curAvatar;

extern ImageItem *g_glyphItem;

extern int hClcProtoCount;

extern HIMAGELIST hCListImages;
static BYTE divide3[765] = {255};
extern char *im_clients[];
extern HICON im_clienthIcons[];
extern HICON overlayicons[];

extern TCHAR *statusNames[];

extern LONG g_cxsmIcon, g_cysmIcon;

int g_hottrack, g_center, g_ignoreselforgroups, g_selectiveIcon, g_hottrack_done;
HWND g_focusWnd;
BYTE selBlend;
BYTE saved_alpha;
int my_status;

BOOL g_inCLCpaint = FALSE;
int g_list_avatars = 0;

HFONT __fastcall ChangeToFont(HDC hdc, struct ClcData *dat, int id, int *fontHeight)
{
	HFONT hOldFont = reinterpret_cast<HFONT>(SelectObject(hdc, cfg::clcdat->fontInfo[id].hFont));
	SetTextColor(hdc, cfg::clcdat->fontInfo[id].colour);
	if (fontHeight)
		*fontHeight = cfg::clcdat->fontInfo[id].fontHeight;

	dat->currentFontID = id;
	return hOldFont;
}

static void __inline SetHotTrackColour(HDC hdc, struct ClcData *dat)
{
	if (dat->gammaCorrection) {
		COLORREF newCol;

		COLORREF oldCol = GetTextColor(hdc);
		int oldLum = (GetRValue(oldCol) * 30 + GetGValue(oldCol) * 59 + GetBValue(oldCol) * 11) / 100;
		int newLum = (GetRValue(dat->hotTextColour) * 30 + GetGValue(dat->hotTextColour) * 59 + GetBValue(dat->hotTextColour) * 11) / 100;
		if (newLum == 0) {
			SetTextColor(hdc, dat->hotTextColour);
			return;
		}

		if (newLum >= oldLum + 20) {
			oldLum += 20;
			newCol = RGB(GetRValue(dat->hotTextColour) * oldLum / newLum, GetGValue(dat->hotTextColour) * oldLum / newLum, GetBValue(dat->hotTextColour) * oldLum / newLum);
		}
		else if (newLum <= oldLum) {
			int r, g, b;
			r = GetRValue(dat->hotTextColour) * oldLum / newLum;
			g = GetGValue(dat->hotTextColour) * oldLum / newLum;
			b = GetBValue(dat->hotTextColour) * oldLum / newLum;
			if (r > 255) {
				g += (r - 255) * 3 / 7;
				b += (r - 255) * 3 / 7;
				r = 255;
			}
			if (g > 255) {
				r += (g - 255) * 59 / 41;
				if (r > 255)
					r = 255;
				b += (g - 255) * 59 / 41;
				g = 255;
			}
			if (b > 255) {
				r += (b - 255) * 11 / 89;
				if (r > 255)
					r = 255;
				g += (b - 255) * 11 / 89;
				if (g > 255)
					g = 255;
				b = 255;
			}
			newCol = RGB(r, g, b);
		}
		else newCol = dat->hotTextColour;
		SetTextColor(hdc, newCol);
	}
	else SetTextColor(hdc, dat->hotTextColour);
}

int __fastcall GetStatusOnlineness(int status)
{
	if (status >= ID_STATUS_CONNECTING && status < ID_STATUS_OFFLINE)
		return 120;

	switch (status) {
	case ID_STATUS_FREECHAT:
		return 110;
	case ID_STATUS_ONLINE:
		return 100;
	case ID_STATUS_OCCUPIED:
		return 60;
	case ID_STATUS_ONTHEPHONE:
		return 50;
	case ID_STATUS_DND:
		return 40;
	case ID_STATUS_AWAY:
		return 30;
	case ID_STATUS_OUTTOLUNCH:
		return 20;
	case ID_STATUS_NA:
		return 10;
	case ID_STATUS_INVISIBLE:
		return 5;
	}
	return 0;
}

static int __fastcall GetGeneralisedStatus(void)
{
	int status = ID_STATUS_OFFLINE, statusOnlineness = 0;

	for (int i = 0; i < pcli->hClcProtoCount; i++) {
		int thisStatus = pcli->clcProto[i].dwStatus;
		if (thisStatus == ID_STATUS_INVISIBLE)
			return ID_STATUS_INVISIBLE;

		int thisOnlineness = GetStatusOnlineness(thisStatus);
		if (thisOnlineness > statusOnlineness) {
			status = thisStatus;
			statusOnlineness = thisOnlineness;
		}
	}
	return status;
}

static int __fastcall GetRealStatus(ClcContact *contact, int status)
{
	char *szProto = contact->proto;
	if (!szProto)
		return status;

	for (int i = 0; i < pcli->hClcProtoCount; i++)
		if (!mir_strcmp(pcli->clcProto[i].szProto, szProto))
			return pcli->clcProto[i].dwStatus;

	return status;
}

int GetBasicFontID(ClcContact * contact)
{
	switch (contact->type) {
	case CLCIT_GROUP:
		return FONTID_GROUPS;

	case CLCIT_INFO:
		return (contact->flags & CLCIIF_GROUPFONT) ? FONTID_GROUPS : FONTID_CONTACTS;

	case CLCIT_DIVIDER:
		return FONTID_DIVIDERS;

	case CLCIT_CONTACT:
		if (contact->flags & CONTACTF_NOTONLIST)
			return FONTID_NOTONLIST;

		if ((contact->flags&CONTACTF_INVISTO && GetRealStatus(contact, ID_STATUS_OFFLINE) != ID_STATUS_INVISIBLE)
			|| (contact->flags&CONTACTF_VISTO && GetRealStatus(contact, ID_STATUS_OFFLINE) == ID_STATUS_INVISIBLE))
			return contact->flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS;

		return contact->flags & CONTACTF_ONLINE ? FONTID_CONTACTS : FONTID_OFFLINE;
	}
	return FONTID_CONTACTS;
}

void PaintNotifyArea(HDC hDC, RECT *rc)
{
	static int ev_lastIcon = 0;

	rc->left += 26;
	int iCount = GetMenuItemCount(cfg::dat.hMenuNotify);
	if (cfg::dat.hUpdateContact != 0) {
		TCHAR *szName = pcli->pfnGetContactDisplayName(cfg::dat.hUpdateContact, 0);
		int iIcon = pcli->pfnGetContactIcon(cfg::dat.hUpdateContact);

		ImageList_DrawEx(hCListImages, iIcon, hDC, rc->left, (rc->bottom + rc->top - g_cysmIcon) / 2, g_cxsmIcon, g_cysmIcon, CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc->left += 18;
		DrawText(hDC, szName, -1, rc, DT_VCENTER | DT_SINGLELINE);
		ImageList_DrawEx(hCListImages, (int)cfg::dat.hIconNotify, hDC, 4, (rc->bottom + rc->top - 16) / 2, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		ev_lastIcon = cfg::dat.hIconNotify;
	}
	else if (iCount > 0) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_DATA;
		GetMenuItemInfo(cfg::dat.hMenuNotify, iCount - 1, TRUE, &mii);

		NotifyMenuItemExData *nmi = (struct NotifyMenuItemExData *) mii.dwItemData;
		TCHAR *szName = pcli->pfnGetContactDisplayName(nmi->hContact, 0);
		int iIcon = pcli->pfnGetContactIcon(nmi->hContact);
		ImageList_DrawEx(hCListImages, iIcon, hDC, rc->left, (rc->bottom + rc->top - g_cysmIcon) / 2, g_cxsmIcon, g_cysmIcon, CLR_NONE, CLR_NONE, ILD_NORMAL);
		rc->left += 18;
		ImageList_DrawEx(hCListImages, nmi->iIcon, hDC, 4, (rc->bottom + rc->top) / 2 - 8, 16, 16, CLR_NONE, CLR_NONE, ILD_NORMAL);
		DrawText(hDC, szName, -1, rc, DT_VCENTER | DT_SINGLELINE);
		ev_lastIcon = (int)nmi->hIcon;
	}
	else {
		HICON hIcon = reinterpret_cast<HICON>(LoadImage(g_hInst, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 16, 16, 0));
		DrawText(hDC, TranslateT("No events..."), -1, rc, DT_VCENTER | DT_SINGLELINE);
		DrawIconEx(hDC, 4, (rc->bottom + rc->top - 16) / 2, hIcon, 16, 16, 0, 0, DI_NORMAL | DI_COMPAT);
		DestroyIcon(hIcon);
	}
}

static BLENDFUNCTION bf = { 0, 0, AC_SRC_OVER, 0 };
static BOOL avatar_done = FALSE;
HDC g_HDC;
static BOOL g_RTL;
HDC hdcTempAV;
HBITMAP hbmTempAV, hbmTempOldAV;

HDC hdcAV;

LONG g_maxAV_X = 200, g_maxAV_Y = 200;

static int __fastcall DrawAvatar(HDC hdcMem, RECT *rc, ClcContact *contact, int y, struct ClcData *dat, WORD cstatus, int rowHeight, DWORD dwFlags)
{
	float dScale = 0.;
	float newHeight, newWidth;
	HDC hdcAvatar = hdcAV;
	//HBITMAP hbmMem;
	DWORD topoffset = 0, leftoffset = 0;
	LONG bmWidth, bmHeight;
	float dAspect;
	HBITMAP hbm, hbmOldAV;
	HRGN rgn = 0;
	int avatar_size = cfg::dat.avatarSize;
	DWORD av_saved_left;
	StatusItems_t *item = contact->wStatus == ID_STATUS_OFFLINE ? arStatusItems[ID_EXTBKAVATARFRAMEOFFLINE - ID_STATUS_OFFLINE] : arStatusItems[ID_EXTBKAVATARFRAME - ID_STATUS_OFFLINE];
	int  skinMarginX, skinMarginY;
	BOOL fOverlay = (cfg::dat.dwFlags & CLUI_FRAME_OVERLAYICONS);

	contact->avatarLeft = -1;
	if (!cfg::dat.bAvatarServiceAvail || dat->bisEmbedded)
		return 0;

	if (contact->ace != NULL && contact->ace->cbSize == sizeof(struct avatarCacheEntry)) {
		if (contact->ace->dwFlags & AVS_HIDEONCLIST)
			return (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? avatar_size + 2 : 0;

		bmHeight = contact->ace->bmHeight;
		bmWidth = contact->ace->bmWidth;
		if (bmWidth != 0)
			dAspect = (float)bmHeight / (float)bmWidth;
		else
			dAspect = 1.0;
		hbm = contact->ace->hbmPic;
		contact->ace->t_lastAccess = cfg::dat.t_now;
	}
	else return (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) ? avatar_size + 2 : 0;

	if (bmHeight == 0 || bmWidth == 0 || hbm == 0)
		return 0;

	g_maxAV_X = max(bmWidth, g_maxAV_X);
	g_maxAV_Y = max(bmHeight, g_maxAV_Y);

	if (dAspect >= 1.0) {
		skinMarginY = item->IGNORED ? 0 : (item->MARGIN_TOP + item->MARGIN_BOTTOM);
		skinMarginX = item->IGNORED ? 0 : (item->MARGIN_LEFT + item->MARGIN_RIGHT);

		dScale = (float)(avatar_size - 2) / (float)bmHeight;
		newHeight = (float)(avatar_size - skinMarginY - 2);
		newWidth = (float)(bmWidth * dScale) - skinMarginX;
	}
	else {
		skinMarginY = item->IGNORED ? 0 : (item->MARGIN_LEFT + item->MARGIN_RIGHT);
		skinMarginX = item->IGNORED ? 0 : (item->MARGIN_LEFT + item->MARGIN_RIGHT);

		newWidth = (float)(avatar_size - 2) - skinMarginX;
		dScale = (float)(avatar_size - 2) / (float)bmWidth;
		newHeight = (float)(bmHeight * dScale) - skinMarginY;
	}
	topoffset = rowHeight > (int)newHeight ? (rowHeight - (int)newHeight) / 2 : 0;
	if (!item->IGNORED) {
		//topoffset += item->MARGIN_TOP;
		leftoffset = item->MARGIN_LEFT;
	}
	// create the region for the avatar border - use the same region for clipping, if needed.

	av_saved_left = rc->left;
	if (cfg::dat.bCenterStatusIcons && newWidth < newHeight)
		rc->left += (((avatar_size - 2) - ((int)newWidth + skinMarginX)) / 2);

	if (cfg::dat.dwFlags & CLUI_FRAME_ROUNDAVATAR)
		rgn = CreateRoundRectRgn(leftoffset + rc->left, y + topoffset, leftoffset + rc->left + (int)newWidth + 1, y + topoffset + (int)newHeight + 1, 2 * cfg::dat.avatarRadius, 2 * cfg::dat.avatarRadius);
	else
		rgn = CreateRectRgn(leftoffset + rc->left, y + topoffset, leftoffset + rc->left + (int)newWidth, y + topoffset + (int)newHeight);

	hbmOldAV = reinterpret_cast<HBITMAP>(SelectObject(hdcAV, hbm));
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = contact->ace->dwFlags & AVS_PREMULTIPLIED ? AC_SRC_ALPHA : 0;

	SelectClipRgn(hdcMem, rgn);
	SetStretchBltMode(hdcMem, HALFTONE);
	if (bf.SourceConstantAlpha == 255 && bf.AlphaFormat == 0) {
		StretchBlt(hdcMem, leftoffset + rc->left - (g_RTL ? 1 : 0), y + topoffset, (int)newWidth, (int)newHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, SRCCOPY);
	}
	else {
		SetStretchBltMode(hdcTempAV, HALFTONE);
		StretchBlt(hdcTempAV, 0, 0, bmWidth, bmHeight, hdcMem, leftoffset + rc->left, y + topoffset, (int)newWidth, (int)newHeight, SRCCOPY);
		GdiAlphaBlend(hdcTempAV, 0, 0, bmWidth, bmHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, bf);
		StretchBlt(hdcMem, leftoffset + rc->left - (g_RTL ? 1 : 0), y + topoffset, (int)newWidth, (int)newHeight, hdcTempAV, 0, 0, bmWidth, bmHeight, SRCCOPY);
	}
	SelectObject(hdcAV, hbmOldAV);

	if (cfg::dat.dwFlags & CLUI_FRAME_AVATARBORDER) {
		if (g_RTL)
			OffsetRgn(rgn, -1, 0);
		FrameRgn(hdcMem, rgn, cfg::dat.hBrushAvatarBorder, 1, 1);
	}

	if (fOverlay)
		fOverlay = (dwFlags & ECF_HIDEOVERLAY) ? 0 : 1;
	else
		fOverlay = (dwFlags & ECF_FORCEOVERLAY) ? 1 : 0;

	if (fOverlay && cstatus && (int)newHeight >= g_cysmIcon)
		DrawIconEx(hdcMem, rc->left + (int)newWidth - 15, y + topoffset + (int)newHeight - 15, overlayicons[cstatus - ID_STATUS_OFFLINE], g_cxsmIcon, g_cysmIcon, 0, 0, DI_NORMAL | DI_COMPAT);

	SelectClipRgn(hdcMem, NULL);
	DeleteObject(rgn);

	if (!item->IGNORED) {
		RECT rcFrame;
		BOOL inClCPaint_save = g_inCLCpaint;
		HDC  hdcTemp = 0;
		HBITMAP hbmOld, hbmTemp;

		g_inCLCpaint = FALSE;
		rcFrame.left = rc->left;
		rcFrame.top = y + topoffset - item->MARGIN_TOP;
		rcFrame.right = rcFrame.left + (int)newWidth + item->MARGIN_RIGHT + item->MARGIN_LEFT;
		rcFrame.bottom = rcFrame.top + (int)newHeight + item->MARGIN_BOTTOM + item->MARGIN_TOP;
		if (g_RTL) {
			OffsetRect(&rcFrame, 1, 0);

			RECT rcTemp;
			rcTemp.left = rcTemp.top = 0;
			rcTemp.right = rcFrame.right - rcFrame.left;
			rcTemp.bottom = rcFrame.bottom - rcFrame.top;

			hdcTemp = CreateCompatibleDC(g_HDC);
			hbmTemp = CreateCompatibleBitmap(g_HDC, rcTemp.right, rcTemp.bottom);
			hbmOld = reinterpret_cast<HBITMAP>(SelectObject(hdcTemp, hbmTemp));
			SetLayout(hdcTemp, LAYOUT_RTL);
			BitBlt(hdcTemp, 0, 0, rcTemp.right, rcTemp.bottom,
				hdcMem, rcFrame.left, rcFrame.top, SRCCOPY);
			SetLayout(hdcTemp, 0);
			DrawAlpha(hdcTemp, &rcTemp, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT,
				item->CORNER, item->BORDERSTYLE, item->imageItem);
			BitBlt(hdcMem, rcFrame.left, rcFrame.top, rcFrame.right - rcFrame.left, rcFrame.bottom - rcFrame.top,
				hdcTemp, 0, 0, SRCCOPY);
			SelectObject(hdcTemp, hbmOld);
			DeleteObject(hbmTemp);
			DeleteDC(hdcTemp);
		}
		else DrawAlpha(hdcMem, &rcFrame, item->COLOR, item->ALPHA, item->COLOR2, item->COLOR2_TRANSPARENT, item->GRADIENT,
			item->CORNER, item->BORDERSTYLE, item->imageItem);
		g_inCLCpaint = inClCPaint_save;
	}
	contact->avatarLeft = rc->left;
	avatar_done = TRUE;
	rc->left = av_saved_left;
	return avatar_size + 2;

}

static BOOL pi_avatar = FALSE;
static RECT rcContent;
static BOOL pi_selectiveIcon;

static BOOL av_left, av_right, av_rightwithnick;
static BOOL mirror_rtl, mirror_always, mirror_rtltext;

BYTE savedCORNER = -1;
int  g_padding_y = 0;

void __inline PaintItem(HDC hdcMem, ClcGroup *group, ClcContact *contact, int indent, int y, struct ClcData *dat, int index, HWND hwnd, DWORD style, RECT *clRect, BOOL *bFirstNGdrawn, int groupCountsFontTopShift, int rowHeight)
{
	RECT rc;
	SIZE textSize = { 0 }, countsSize = { 0 }, spaceSize = { 0 };
	int fontHeight;
	BOOL twoRows = FALSE;
	DWORD leftOffset = 0, rightOffset = 0;
	int iconXSpace = dat->iconXSpace;
	//BOOL xStatusValid = 0;
	HFONT hPreviousFont = 0;
	COLORREF oldGroupColor = -1;
	DWORD qLeft = 0;
	int leftX = dat->leftMargin + indent * dat->groupIndent;
	int bg_indent_r = 0;
	int bg_indent_l = 0;
	int rightIcons = 0;
	DWORD dt_nickflags = 0, dt_2ndrowflags = 0;
	DWORD dwFlags = cfg::dat.dwFlags;
	BOOL check_selected, fLocalTime;

	rowHeight -= cfg::dat.bRowSpacing;
	savedCORNER = -1;

	if (group == NULL || contact == NULL)
		return;

	g_RTL = FALSE;
	int scanIndex = group->scanIndex;
	BOOL av_local_wanted = (g_list_avatars > 0);

	BYTE type = contact->type;
	BYTE flags = contact->flags;
	int  selected = index == dat->selection && (dat->showSelAlways || dat->exStyle &CLS_EX_SHOWSELALWAYS || g_focusWnd == hwnd) && type != CLCIT_DIVIDER;
	avatar_done = FALSE;

	TExtraCache *cEntry;
	if (contact->pExtra)
		cEntry = contact->pExtra;
	else
		cEntry = cfg::arCache[0];

	if (dat->bisEmbedded)
		goto set_bg_l;

	if (type == CLCIT_CONTACT && (cEntry->dwCFlags & ECF_RTLNICK || mirror_always)) {
		if (mirror_rtl || mirror_always) {
			g_RTL = TRUE;
			bg_indent_r = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
		}
		else if (mirror_rtltext) {
			bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
			dt_nickflags = DT_RTLREADING | DT_RIGHT;
		}
		else bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
	}
	else if (type == CLCIT_GROUP) {
		if ((contact->isRtl && cfg::dat.bGroupAlign == CLC_GROUPALIGN_AUTO) || cfg::dat.bGroupAlign == CLC_GROUPALIGN_RIGHT) {
			g_RTL = TRUE;
			bg_indent_r = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
		}
		else bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
	}
	else bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;

set_bg_l:
	g_hottrack = dat->exStyle & CLS_EX_TRACKSELECT && type == CLCIT_CONTACT && dat->iHotTrack == index;
	if (g_hottrack == selected)
		g_hottrack = 0;

	saved_alpha = 0;

	//setup
	if (type == CLCIT_GROUP)
		ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
	else if (type == CLCIT_INFO) {
		if (flags & CLCIIF_GROUPFONT)
			ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
		else
			ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);
	}
	else if (type == CLCIT_DIVIDER) {
		ChangeToFont(hdcMem, dat, FONTID_DIVIDERS, &fontHeight);
		GetTextExtentPoint32(hdcMem, contact->szText, (int)mir_tstrlen(contact->szText), &textSize);
	}
	else if (type == CLCIT_CONTACT && flags & CONTACTF_NOTONLIST)
		ChangeToFont(hdcMem, dat, FONTID_NOTONLIST, &fontHeight);
	else if (type == CLCIT_CONTACT && ((flags & CONTACTF_INVISTO && GetRealStatus(contact, my_status) != ID_STATUS_INVISIBLE) || (flags & CONTACTF_VISTO && GetRealStatus(contact, my_status) == ID_STATUS_INVISIBLE))) {
		// the contact is in the always visible list and the proto is invisible
		// the contact is in the always invisible and the proto is in any other mode
		ChangeToFont(hdcMem, dat, flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS, &fontHeight);
	}
	else if (type == CLCIT_CONTACT && !(flags & CONTACTF_ONLINE))
		ChangeToFont(hdcMem, dat, FONTID_OFFLINE, &fontHeight);
	else
		ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);

	char *szCounts = NULL;
	if (type == CLCIT_GROUP) {
		GetTextExtentPoint32(hdcMem, contact->szText, (int)mir_tstrlen(contact->szText), &textSize);
		int width = textSize.cx;
		szCounts = pcli->pfnGetGroupCountsText(dat, contact);
		if (szCounts[0]) {
			GetTextExtentPoint32(hdcMem, _T(" "), 1, &spaceSize);
			ChangeToFont(hdcMem, dat, FONTID_GROUPCOUNTS, &fontHeight);
			GetTextExtentPoint32A(hdcMem, szCounts, (int)mir_strlen(szCounts), &countsSize);
			width += spaceSize.cx + countsSize.cx;
		}
	}

	int checkboxWidth;
	if ((style & CLS_CHECKBOXES && type == CLCIT_CONTACT) || (style & CLS_GROUPCHECKBOXES && type == CLCIT_GROUP) || (type == CLCIT_INFO && flags & CLCIIF_CHECKBOX))
		checkboxWidth = dat->checkboxSize + 2;
	else
		checkboxWidth = 0;

	rc.left = 0;
	WORD cstatus = contact->wStatus;

	/***** BACKGROUND DRAWING *****/
	// contacts

	g_hottrack_done = 0;
	check_selected = (!selected || selBlend);

	if (dat->bisEmbedded) {
		rc.left = bg_indent_l;
		rc.top = y;
		rc.right = clRect->right - bg_indent_r;
		rc.bottom = y + rowHeight;
		if (selected) {
			FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			SetTextColor(hdcMem, dat->selTextColour);
		}
		else {
			FillRect(hdcMem, &rc, cfg::dat.hBrushCLCBk);
			if (g_hottrack)
				SetHotTrackColour(hdcMem, dat);
		}
		goto bgskipped;
	}

	if (type == CLCIT_CONTACT || type == CLCIT_DIVIDER) {
		StatusItems_t *sitem, *sfirstitem, *ssingleitem, *slastitem, *slastitem_NG,
			*sfirstitem_NG, *ssingleitem_NG, *sevencontact_pos, *soddcontact_pos, *pp_item;

		if (cstatus >= ID_STATUS_OFFLINE && cstatus <= ID_STATUS_OUTTOLUNCH) {
			BYTE perstatus_ignored;

			if ((flags & CONTACTF_IDLE) && !arStatusItems[ID_EXTBKIDLE - ID_STATUS_OFFLINE]->IGNORED)
				sitem = arStatusItems[ID_EXTBKIDLE - ID_STATUS_OFFLINE];
			else
				sitem = arStatusItems[cstatus - ID_STATUS_OFFLINE];

			if (!dat->bisEmbedded) {
				pp_item = cEntry->status_item ? cEntry->status_item : cEntry->proto_status_item;

				if (!(perstatus_ignored = sitem->IGNORED) && !(flags & CONTACTF_NOTONLIST))
					SetTextColor(hdcMem, sitem->TEXTCOLOR);

				if (cfg::dat.bUsePerProto && pp_item && !pp_item->IGNORED) {
					sitem = pp_item;
					if ((perstatus_ignored || cfg::dat.bOverridePerStatusColors) && sitem->TEXTCOLOR != -1)
						SetTextColor(hdcMem, sitem->TEXTCOLOR);
				}
			}
			else if (!sitem->IGNORED)
				SetTextColor(hdcMem, sitem->TEXTCOLOR);

			sevencontact_pos = arStatusItems[ID_EXTBKEVEN_CNTCTPOS - ID_STATUS_OFFLINE];
			soddcontact_pos = arStatusItems[ID_EXTBKODD_CNTCTPOS - ID_STATUS_OFFLINE];
			sfirstitem = arStatusItems[ID_EXTBKFIRSTITEM - ID_STATUS_OFFLINE];
			ssingleitem = arStatusItems[ID_EXTBKSINGLEITEM - ID_STATUS_OFFLINE];
			slastitem = arStatusItems[ID_EXTBKLASTITEM - ID_STATUS_OFFLINE];

			sfirstitem_NG = arStatusItems[ID_EXTBKFIRSTITEM_NG - ID_STATUS_OFFLINE];
			ssingleitem_NG = arStatusItems[ID_EXTBKSINGLEITEM_NG - ID_STATUS_OFFLINE];
			slastitem_NG = arStatusItems[ID_EXTBKLASTITEM_NG - ID_STATUS_OFFLINE];

			rc.left = sitem->MARGIN_LEFT + bg_indent_l;
			rc.top = y + sitem->MARGIN_TOP;
			rc.right = clRect->right - sitem->MARGIN_RIGHT - bg_indent_r;
			rc.bottom = y + rowHeight - sitem->MARGIN_BOTTOM;

			// check for special cases (first item, single item, last item)
			// this will only change the shape for this status. Color will be blended over with ALPHA value
			if (!ssingleitem->IGNORED && scanIndex == 0 && group->cl.count == 1 && group->parent != NULL) {
				rc.left = ssingleitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + ssingleitem->MARGIN_TOP;
				rc.right = clRect->right - ssingleitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - ssingleitem->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if (!sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, ssingleitem->CORNER, ssingleitem->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (!soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, ssingleitem->CORNER, ssingleitem->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, ssingleitem->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = ssingleitem->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, ssingleitem->COLOR, ssingleitem->ALPHA, ssingleitem->COLOR2, ssingleitem->COLOR2_TRANSPARENT, ssingleitem->GRADIENT, ssingleitem->CORNER, ssingleitem->BORDERSTYLE, ssingleitem->imageItem);
			}
			else if (scanIndex == 0 && group->cl.count > 1 && !sfirstitem->IGNORED && group->parent != NULL) {
				rc.left = sfirstitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sfirstitem->MARGIN_TOP;
				rc.right = clRect->right - sfirstitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sfirstitem->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if (!sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, sfirstitem->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (!soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, sfirstitem->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, sfirstitem->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = sfirstitem->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, sfirstitem->COLOR, sfirstitem->ALPHA, sfirstitem->COLOR2, sfirstitem->COLOR2_TRANSPARENT, sfirstitem->GRADIENT, sfirstitem->CORNER, sfirstitem->BORDERSTYLE, sfirstitem->imageItem);
			}
			else if (scanIndex == group->cl.count - 1 && !slastitem->IGNORED && group->parent != NULL) {
				// last item of group
				rc.left = slastitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + slastitem->MARGIN_TOP;
				rc.right = clRect->right - slastitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - slastitem->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, slastitem->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, slastitem->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, slastitem->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = slastitem->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, slastitem->COLOR, slastitem->ALPHA, slastitem->COLOR2, slastitem->COLOR2_TRANSPARENT, slastitem->GRADIENT, slastitem->CORNER, slastitem->BORDERSTYLE, slastitem->imageItem);
			}
			// --- Non-grouped items ---
			else if (type != CLCIT_GROUP && group->parent == NULL && !sfirstitem_NG->IGNORED && scanIndex != group->cl.count - 1 && !(*bFirstNGdrawn)) {
				// first NON-grouped
				*bFirstNGdrawn = TRUE;
				rc.left = sfirstitem_NG->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sfirstitem_NG->MARGIN_TOP;
				rc.right = clRect->right - sfirstitem_NG->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sfirstitem_NG->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, sfirstitem_NG->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, sfirstitem_NG->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, sfirstitem_NG->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = sfirstitem_NG->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, sfirstitem_NG->COLOR, sfirstitem_NG->ALPHA, sfirstitem_NG->COLOR2, sfirstitem_NG->COLOR2_TRANSPARENT, sfirstitem_NG->GRADIENT, sfirstitem_NG->CORNER, sfirstitem->BORDERSTYLE, sfirstitem->imageItem);
			}
			else if (type != CLCIT_GROUP && group->parent == NULL && !slastitem_NG->IGNORED && scanIndex == group->cl.count - 1 && (*bFirstNGdrawn)) {
				// last item of list (NON-group)
				// last NON-grouped
				rc.left = slastitem_NG->MARGIN_LEFT + bg_indent_l;
				rc.top = y + slastitem_NG->MARGIN_TOP;
				rc.right = clRect->right - slastitem_NG->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - slastitem_NG->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, slastitem_NG->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, slastitem_NG->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, slastitem_NG->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = slastitem_NG->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, slastitem_NG->COLOR, slastitem_NG->ALPHA, slastitem_NG->COLOR2, slastitem_NG->COLOR2_TRANSPARENT, slastitem_NG->GRADIENT, slastitem_NG->CORNER, slastitem->BORDERSTYLE, slastitem->imageItem);
			}
			else if (type != CLCIT_GROUP && group->parent == NULL && !slastitem_NG->IGNORED && !(*bFirstNGdrawn)) {
				// single item of NON-group
				// single NON-grouped
				rc.left = ssingleitem_NG->MARGIN_LEFT + bg_indent_l;
				rc.top = y + ssingleitem_NG->MARGIN_TOP;
				rc.right = clRect->right - ssingleitem_NG->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - ssingleitem_NG->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, ssingleitem_NG->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, ssingleitem_NG->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (!sitem->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, ssingleitem_NG->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
					savedCORNER = ssingleitem_NG->CORNER;
				}
				if (check_selected)
					DrawAlpha(hdcMem, &rc, ssingleitem_NG->COLOR, ssingleitem_NG->ALPHA, ssingleitem_NG->COLOR2, ssingleitem_NG->COLOR2_TRANSPARENT, ssingleitem_NG->GRADIENT, ssingleitem_NG->CORNER, ssingleitem->BORDERSTYLE, ssingleitem->imageItem);
			}
			else if (!sitem->IGNORED) {
				// draw default grouped
				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, sevencontact_pos->COLOR, sevencontact_pos->ALPHA, sevencontact_pos->COLOR2, sevencontact_pos->COLOR2_TRANSPARENT, sevencontact_pos->GRADIENT, sitem->CORNER, sevencontact_pos->BORDERSTYLE, sevencontact_pos->imageItem);
				}
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED) {
					if (check_selected)
						DrawAlpha(hdcMem, &rc, soddcontact_pos->COLOR, soddcontact_pos->ALPHA, soddcontact_pos->COLOR2, soddcontact_pos->COLOR2_TRANSPARENT, soddcontact_pos->GRADIENT, sitem->CORNER, soddcontact_pos->BORDERSTYLE, soddcontact_pos->imageItem);
				}

				if (check_selected)
					DrawAlpha(hdcMem, &rc, sitem->COLOR, sitem->ALPHA, sitem->COLOR2, sitem->COLOR2_TRANSPARENT, sitem->GRADIENT, sitem->CORNER, sitem->BORDERSTYLE, sitem->imageItem);
				savedCORNER = sitem->CORNER;
			}
		}
	}

	if (type == CLCIT_GROUP) {
		StatusItems_t *sempty = arStatusItems[ID_EXTBKEMPTYGROUPS - ID_STATUS_OFFLINE];
		StatusItems_t *sexpanded = arStatusItems[ID_EXTBKEXPANDEDGROUP - ID_STATUS_OFFLINE];
		StatusItems_t *scollapsed = arStatusItems[ID_EXTBKCOLLAPSEDDGROUP - ID_STATUS_OFFLINE];

		ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
		if (contact->group->cl.count == 0) {
			if (!sempty->IGNORED) {
				rc.left = sempty->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sempty->MARGIN_TOP;
				rc.right = clRect->right - sempty->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sempty->MARGIN_BOTTOM;
				DrawAlpha(hdcMem, &rc, sempty->COLOR, sempty->ALPHA, sempty->COLOR2, sempty->COLOR2_TRANSPARENT, sempty->GRADIENT, sempty->CORNER, sempty->BORDERSTYLE, sempty->imageItem);
				savedCORNER = sempty->CORNER;
				oldGroupColor = SetTextColor(hdcMem, sempty->TEXTCOLOR);
			}
		}
		else if (contact->group->expanded) {
			if (!sexpanded->IGNORED) {
				rc.left = sexpanded->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sexpanded->MARGIN_TOP;
				rc.right = clRect->right - sexpanded->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - (char)sexpanded->MARGIN_BOTTOM;
				DrawAlpha(hdcMem, &rc, sexpanded->COLOR, sexpanded->ALPHA, sexpanded->COLOR2, sexpanded->COLOR2_TRANSPARENT, sexpanded->GRADIENT, sexpanded->CORNER, sexpanded->BORDERSTYLE, sexpanded->imageItem);
				savedCORNER = sexpanded->CORNER;
				oldGroupColor = SetTextColor(hdcMem, sexpanded->TEXTCOLOR);
			}
		}
		else if (!scollapsed->IGNORED) {
			// collapsed but not empty
			rc.left = scollapsed->MARGIN_LEFT + bg_indent_l;
			rc.top = y + scollapsed->MARGIN_TOP;
			rc.right = clRect->right - scollapsed->MARGIN_RIGHT - bg_indent_r;
			rc.bottom = y + rowHeight - scollapsed->MARGIN_BOTTOM;
			DrawAlpha(hdcMem, &rc, scollapsed->COLOR, scollapsed->ALPHA, scollapsed->COLOR2, scollapsed->COLOR2_TRANSPARENT, scollapsed->GRADIENT, scollapsed->CORNER, scollapsed->BORDERSTYLE, scollapsed->imageItem);
			savedCORNER = scollapsed->CORNER;
			oldGroupColor = SetTextColor(hdcMem, scollapsed->TEXTCOLOR);
		}
	}
	if (selected) {
		StatusItems_t *sselected = arStatusItems[ID_EXTBKSELECTION - ID_STATUS_OFFLINE];

		if (!g_ignoreselforgroups || type != CLCIT_GROUP) {
			if (!sselected->IGNORED) {
				rc.left = sselected->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sselected->MARGIN_TOP;
				rc.right = clRect->right - sselected->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sselected->MARGIN_BOTTOM;
				if (cfg::getByte("CLCExt", "EXBK_EqualSelection", 0) == 1 && savedCORNER != -1)
					DrawAlpha(hdcMem, &rc, sselected->COLOR, sselected->ALPHA, sselected->COLOR2, sselected->COLOR2_TRANSPARENT, sselected->GRADIENT, savedCORNER, sselected->BORDERSTYLE, sselected->imageItem);
				else
					DrawAlpha(hdcMem, &rc, sselected->COLOR, sselected->ALPHA, sselected->COLOR2, sselected->COLOR2_TRANSPARENT, sselected->GRADIENT, sselected->CORNER, sselected->BORDERSTYLE, sselected->imageItem);

				SetTextColor(hdcMem, sselected->TEXTCOLOR);
			}
			else {
				rc.left = bg_indent_l;
				rc.top = y;
				rc.right = clRect->right - bg_indent_r;
				rc.bottom = y + rowHeight;
				FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
				SetTextColor(hdcMem, dat->selTextColour);
			}
		}
	}
	else if (g_hottrack) {
		StatusItems_t *ht = arStatusItems[ID_EXTBKHOTTRACK - ID_STATUS_OFFLINE];

		SetHotTrackColour(hdcMem, dat);
		if (ht->IGNORED == 0)
			SetTextColor(hdcMem, ht->TEXTCOLOR);

		if (!g_hottrack_done)
			if (ht->IGNORED == 0)
				DrawAlpha(hdcMem, &rc, ht->COLOR, ht->ALPHA, ht->COLOR2, ht->COLOR2_TRANSPARENT, ht->GRADIENT,
				ht->CORNER, ht->BORDERSTYLE, ht->imageItem);
	}

	if (g_RTL)
		SetLayout(hdcMem, LAYOUT_RTL | LAYOUT_BITMAPORIENTATIONPRESERVED);

bgskipped:
	rcContent.top = y + g_padding_y;
	rcContent.bottom = y + rowHeight - (2 * g_padding_y);
	rcContent.left = leftX;
	rcContent.right = clRect->right - dat->rightMargin;
	twoRows = ((dat->fontInfo[FONTID_STATUS].fontHeight + fontHeight <= rowHeight + 1) && (contact->bSecondLine != MULTIROW_NEVER)) && !dat->bisEmbedded;

	pi_avatar = !dat->bisEmbedded && type == CLCIT_CONTACT && (contact->cFlags & ECF_AVATAR) && contact->ace != 0 && !(contact->ace->dwFlags & AVS_HIDEONCLIST);

	//checkboxes
	if (checkboxWidth) {
		HANDLE hTheme = 0;
		if (IS_THEMED)
			hTheme = OpenThemeData(hwnd, L"BUTTON");

		RECT rc;
		rc.left = leftX;
		rc.right = rc.left + dat->checkboxSize;
		rc.top = y + ((rowHeight - dat->checkboxSize) >> 1);
		rc.bottom = rc.top + dat->checkboxSize;
		if (hTheme) {
			DrawThemeBackground(hTheme, hdcMem, BP_CHECKBOX, flags & CONTACTF_CHECKED ? (g_hottrack ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL) : (g_hottrack ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL), &rc, &rc);
			CloseThemeData(hTheme);
			hTheme = 0;
		}
		else DrawFrameControl(hdcMem, &rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | (flags & CONTACTF_CHECKED ? DFCS_CHECKED : 0) | (g_hottrack ? DFCS_HOT : 0));

		rcContent.left += checkboxWidth;
		leftX += checkboxWidth;
	}

	int iImage;
	if (type == CLCIT_GROUP)
		iImage = (contact->group->expanded) ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
	else if (type == CLCIT_CONTACT)
		iImage = contact->iImage;
	else
		iImage = -1;

	if (pi_avatar && (av_left || av_right)) {
		RECT rc;

		rc.left = rcContent.left;
		rc.right = clRect->right;
		rc.top = y;
		rc.bottom = rc.top + rowHeight;

		if (av_left) {
			leftOffset += DrawAvatar(hdcMem, &rc, contact, y, dat, (WORD)(iImage ? cstatus : 0), rowHeight, cEntry->dwDFlags);
			rcContent.left += leftOffset;
			leftX += leftOffset;
		}
		else {
			rc.left = (rcContent.right - cfg::dat.avatarSize) + 1;
			rightOffset += DrawAvatar(hdcMem, &rc, contact, y, dat, (WORD)(iImage ? cstatus : 0), rowHeight, cEntry->dwDFlags);
			rcContent.right -= (rightOffset);
		}
	}
	else if (type == CLCIT_CONTACT && !dat->bisEmbedded && !g_selectiveIcon && (dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) && av_local_wanted && (av_left || av_right)) {
		if (av_right)
			rcContent.right -= (cfg::dat.avatarSize + 2);
		if (av_left)
			rcContent.left += (cfg::dat.avatarSize + 2);
	}

	// icon
	// skip icon for groups if the option is enabled...

	if (type == CLCIT_GROUP && dwFlags & CLUI_FRAME_NOGROUPICON)
		iconXSpace = 0;
	else if (iImage != -1) {
		// this doesnt use CLS_CONTACTLIST since the colour prolly wont match anyway
		COLORREF colourFg = dat->selBkColour;
		//int clientId = contact->clientId;
		int mode = ILD_NORMAL;
		pi_selectiveIcon = g_selectiveIcon && (type == CLCIT_CONTACT);

		if ((dwFlags & CLUI_FRAME_STATUSICONS && !pi_selectiveIcon) || type != CLCIT_CONTACT || (pi_selectiveIcon && !avatar_done)) {
			HIMAGELIST hImgList = hCListImages;
			if (!dat->bisEmbedded && type == CLCIT_CONTACT && contact->xStatusIcon != -1 && (dwFlags & CLUI_FRAME_USEXSTATUSASSTATUS))
				iImage = contact->xStatusIcon;

			if (g_hottrack)
				colourFg = dat->hotTextColour;
			else if (type == CLCIT_CONTACT && flags & CONTACTF_NOTONLIST) {
				colourFg = dat->fontInfo[FONTID_NOTONLIST].colour;
				mode = ILD_BLEND50;
			}
			if (type == CLCIT_CONTACT && dat->showIdle && (flags & CONTACTF_IDLE) && contact->wStatus != ID_STATUS_OFFLINE)
				mode = ILD_SELECTED;

			if (pi_selectiveIcon && av_right) {
				ImageList_DrawEx(hImgList, iImage, hdcMem, rcContent.right - 18, (twoRows && type == CLCIT_CONTACT && !cfg::dat.bCenterStatusIcons) ? y + 2 : y + ((rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
				rcContent.right -= 18;
			}
			else {
				LONG offset = 0;
				BOOL centered = FALSE;
				offset += (type != CLCIT_CONTACT || avatar_done || !(av_local_wanted) ? 20 : dwFlags & CLUI_FRAME_ALWAYSALIGNNICK && av_left && g_selectiveIcon ? cfg::dat.avatarSize + 2 : 20);
				centered = (cfg::dat.bCenterStatusIcons && offset == cfg::dat.avatarSize + 2);
				ImageList_DrawEx(hImgList, iImage, hdcMem, centered ? rcContent.left + offset / 2 - 10 : rcContent.left, (twoRows && type == CLCIT_CONTACT && !cfg::dat.bCenterStatusIcons) ? y + 2 : y + ((rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
				rcContent.left += offset;
			}
		}
		else iconXSpace = 0;
	}

	contact->extraIconRightBegin = 0;
	for (int i = dat->extraColumnsCount - 1; i >= 0; i--) {
		if (contact->iExtraImage[i] == EMPTY_EXTRA_ICON)
			continue;

		if (contact->extraIconRightBegin == 0)
			contact->extraIconRightBegin = rcContent.right;

		rightIcons++;
		ImageList_DrawEx(dat->himlExtraColumns, contact->iExtraImage[i], hdcMem,
			rcContent.right - dat->extraColumnSpacing * rightIcons,
			y + (rowHeight - g_cysmIcon) / 2,
			0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
	}

	//text
	if (type == CLCIT_DIVIDER) {
		RECT rc;
		rc.top = y + ((rowHeight) >> 1); rc.bottom = rc.top + 2;
		rc.left = rcContent.left;
		rc.right = rc.left - dat->rightMargin + ((clRect->right - rc.left - textSize.cx) >> 1) - 3;
		DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
		TextOut(hdcMem, rc.right + 3, y + ((rowHeight - fontHeight) >> 1), contact->szText, (int)mir_tstrlen(contact->szText));
		rc.left = rc.right + 6 + textSize.cx;
		rc.right = clRect->right - dat->rightMargin;
		DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
	}
	else if (type == CLCIT_GROUP) {
		RECT rc;
		int leftLineEnd = 0, rightLineStart;
		fontHeight = dat->fontInfo[FONTID_GROUPS].fontHeight;
		rc.top = y + ((rowHeight - fontHeight) >> 1) + cfg::dat.group_padding;
		rc.bottom = rc.top + textSize.cy;
		if (szCounts[0]) {
			COLORREF clr = GetTextColor(hdcMem);

			int height = 0;
			ChangeToFont(hdcMem, dat, FONTID_GROUPCOUNTS, &height);
			if (oldGroupColor != -1)
				SetTextColor(hdcMem, clr);

			rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + iconXSpace;
			rc.right = clRect->right - dat->rightMargin;

			if (indent == 0 && iconXSpace == 0)
				rc.left += 2;

			int required = textSize.cx + countsSize.cx + spaceSize.cx;

			if (required > rc.right - rc.left)
				textSize.cx = (rc.right - rc.left) - countsSize.cx - spaceSize.cx;

			int labelWidth = textSize.cx + countsSize.cx + spaceSize.cx;
			int offset = (g_center) ? ((rc.right - rc.left) - labelWidth) / 2 : 0;

			TextOutA(hdcMem, rc.left + offset + textSize.cx + spaceSize.cx, rc.top + groupCountsFontTopShift, szCounts, (int)mir_strlen(szCounts));
			rightLineStart = rc.left + offset + textSize.cx + spaceSize.cx + countsSize.cx + 2;

			if (selected && !g_ignoreselforgroups)
				SetTextColor(hdcMem, dat->selTextColour);
			else
				SetTextColor(hdcMem, clr);
			ChangeToFont(hdcMem, dat, FONTID_GROUPS, &height);
			SetTextColor(hdcMem, clr);
			rc.left += offset;
			rc.right = rc.left + textSize.cx;
			leftLineEnd = rc.left - 2;
			qLeft = rc.left;
			DrawText(hdcMem, contact->szText, -1, &rc, DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS);
		}
		else if (g_center && !szCounts[0]) {
			rc.left = rcContent.left;
			rc.right = clRect->right - dat->rightMargin;
			if (textSize.cx >= rc.right - rc.left)
				textSize.cx = rc.right - rc.left;

			int offset = ((rc.right - rc.left) - textSize.cx) / 2;
			rc.left += offset;
			rc.right = rc.left + textSize.cx;
			leftLineEnd = rc.left - 2;
			rightLineStart = rc.right + 2;
			DrawText(hdcMem, contact->szText, -1, &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);
			qLeft = rc.left;
		}
		else {
			qLeft = rcContent.left + (indent == 0 && iconXSpace == 0 ? 2 : 0);
			rc.left = qLeft;
			rc.right = min(rc.left + textSize.cx, clRect->right - dat->rightMargin);
			DrawText(hdcMem, contact->szText, -1, &rc, DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS);
			rightLineStart = qLeft + textSize.cx + 2;
		}

		if (dat->exStyle & CLS_EX_LINEWITHGROUPS) {
			if (!g_center) {
				rc.top = y + ((rowHeight) >> 1); rc.bottom = rc.top + 2;
				rc.left = rightLineStart;
				rc.right = clRect->right - 1 - rightIcons*dat->extraColumnSpacing - dat->rightMargin;
				if (rc.right - rc.left > 1)
					DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
			}
			else {
				rc.top = y + ((rowHeight) >> 1); rc.bottom = rc.top + 2;
				rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + iconXSpace;
				rc.right = leftLineEnd;
				if (rc.right > rc.left)
					DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
				rc.right = clRect->right - dat->rightMargin;
				rc.left = rightLineStart;
				if (rc.right > rc.left)
					DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
			}
		}
	}
	else {
		TCHAR *szText = contact->szText;

		rcContent.top = y + ((rowHeight - fontHeight) >> 1);

		// avatar

		if (!dat->bisEmbedded) {
			if (av_local_wanted && !avatar_done && pi_avatar) {
				if (av_rightwithnick) {
					RECT rcAvatar = rcContent;

					rcAvatar.left = rcContent.right - (cfg::dat.avatarSize - 1);
					DrawAvatar(hdcMem, &rcAvatar, contact, y, dat, (WORD)(iImage ? cstatus : 0), rowHeight, cEntry->dwDFlags);
					rcContent.right -= (cfg::dat.avatarSize + 2);
				}
				else rcContent.left += DrawAvatar(hdcMem, &rcContent, contact, y, dat, (WORD)(iImage ? cstatus : 0), rowHeight, cEntry->dwDFlags);
			}
			else if (dwFlags & CLUI_FRAME_ALWAYSALIGNNICK && !avatar_done && av_local_wanted)
				rcContent.left += (dwFlags & (CLUI_FRAME_AVATARSLEFT | CLUI_FRAME_AVATARSRIGHT | CLUI_FRAME_AVATARSRIGHTWITHNICK) ? 0 : cfg::dat.avatarSize + 2);
		}

		// nickname
		if (!twoRows)
			DrawText(hdcMem, szText, -1, &rcContent, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE | dt_nickflags);
		else {
			int statusFontHeight;
			DWORD dtFlags = DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE;
			DWORD saved_right = rcContent.right;
			BOOL verticalfit = FALSE;

			rcContent.top = y + cfg::dat.avatarPadding / 2;

			if (cfg::dat.bShowLocalTime)
				fLocalTime = cEntry->dwDFlags & ECF_HIDELOCALTIME ? 0 : 1;
			else
				fLocalTime = cEntry->dwDFlags & ECF_FORCELOCALTIME ? 1 : 0;

			if (cEntry->hTimeZone && fLocalTime) {
				SIZE szTime;
				RECT rc = rcContent;
				int fHeight = 0;

				TCHAR szResult[80];
				if (tmi.printDateTime(cEntry->hTimeZone, _T("t"), szResult, SIZEOF(szResult), 0))
					goto nodisplay;

				COLORREF oldColor = GetTextColor(hdcMem);
				int idOldFont = dat->currentFontID;
				ChangeToFont(hdcMem, dat, FONTID_TIMESTAMP, &fHeight);
				GetTextExtentPoint32(hdcMem, szResult, (int)mir_tstrlen(szResult), &szTime);
				verticalfit = (rowHeight - fHeight >= g_cysmIcon + 1);

				if (av_right) {
					if (verticalfit)
						rc.left = rcContent.right + (rightIcons * dat->extraColumnSpacing) - szTime.cx - 2;
					else
						rc.left = rcContent.right - szTime.cx - 2;
				}
				else if (av_rightwithnick) {
					if (verticalfit && rightIcons * dat->extraColumnSpacing >= szTime.cx)
						rc.left = clRect->right - dat->rightMargin - szTime.cx;
					else if (verticalfit && !avatar_done)
						rc.left = clRect->right - dat->rightMargin - szTime.cx;
					else {
						rc.left = rcContent.right - szTime.cx - 2;
						rcContent.right = rc.left - 2;
					}
				}
				else {
					if (verticalfit)
						rc.left = clRect->right - dat->rightMargin - szTime.cx;
					else
						rc.left = rcContent.right - szTime.cx - 2;
				}
				DrawText(hdcMem, szResult, -1, &rc, DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE);
				ChangeToFont(hdcMem, dat, idOldFont, 0);
				SetTextColor(hdcMem, oldColor);

				verticalfit = (rowHeight - fontHeight >= g_cysmIcon + 1);
				if (verticalfit && av_right)
					rcContent.right = min(clRect->right - cfg::dat.avatarSize - 2, rc.left - 2);
				else if (verticalfit && !av_rightwithnick)
					rcContent.right = min(clRect->right - dat->rightMargin, rc.left - 3);
			}
			else {
			nodisplay:
				verticalfit = (rowHeight - fontHeight >= g_cysmIcon + 1);
				if (avatar_done) {
					if (verticalfit && av_right)
						rcContent.right = clRect->right - cfg::dat.avatarSize - 2;
					else if (verticalfit && !av_rightwithnick)
						rcContent.right = clRect->right - dat->rightMargin;
				}
			}
			DrawText(hdcMem, szText, -1, &rcContent, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE | dt_nickflags);

			rcContent.right = saved_right;
			rcContent.top += (fontHeight - 1);
			hPreviousFont = ChangeToFont(hdcMem, dat, FONTID_STATUS, &statusFontHeight);
			//if (selected)
			//	SetTextColor(hdcMem, dat->selTextColour);
			rcContent.bottom = y + rowHeight;

			if (cstatus >= ID_STATUS_OFFLINE && cstatus <= ID_STATUS_OUTTOLUNCH) {
				TCHAR *szText = NULL;
				BYTE smsgValid = cEntry->bStatusMsgValid;

				if ((dwFlags & CLUI_FRAME_SHOWSTATUSMSG && smsgValid > STATUSMSG_XSTATUSID) || smsgValid == STATUSMSG_XSTATUSNAME)
					szText = cEntry->statusMsg;
				else
					szText = &statusNames[cstatus - ID_STATUS_OFFLINE][0];

				if (cEntry->dwCFlags & ECF_RTLSTATUSMSG && cfg::dat.bUseDCMirroring == 3)
					dt_2ndrowflags |= (DT_RTLREADING | DT_RIGHT);

				if (rightIcons == 0) {
					if ((rcContent.bottom - rcContent.top) >= (2 * statusFontHeight)) {
						dtFlags &= ~(DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP);
						dtFlags |= DT_WORDBREAK;
						rcContent.bottom -= ((rcContent.bottom - rcContent.top) % statusFontHeight);
					}
					DrawText(hdcMem, szText, -1, &rcContent, dtFlags | dt_2ndrowflags);
				}
				else {
					if ((rcContent.bottom - rcContent.top) < (2 * statusFontHeight) - 2)
						DrawText(hdcMem, szText, -1, &rcContent, dtFlags | dt_2ndrowflags);
					else {
						DRAWTEXTPARAMS dtp = { 0 };
						LONG rightIconsTop = rcContent.bottom - g_cysmIcon;
						LONG old_right = rcContent.right;
						ULONG textCounter = 0;
						size_t ulLen = mir_tstrlen(szText);
						LONG old_bottom = rcContent.bottom;
						DWORD i_dtFlags = DT_WORDBREAK | DT_NOPREFIX | dt_2ndrowflags;
						dtp.cbSize = sizeof(dtp);
						rcContent.right = clRect->right - dat->rightMargin - rightOffset;
						do {
							if (rcContent.top + (statusFontHeight - 1) > rightIconsTop + 1)
								rcContent.right = old_right;
							dtp.uiLengthDrawn = 0;
							rcContent.bottom = rcContent.top + statusFontHeight - 1;
							if (rcContent.bottom + statusFontHeight >= old_bottom)
								i_dtFlags |= DT_END_ELLIPSIS;
							DrawTextEx(hdcMem, &szText[textCounter], -1, &rcContent, i_dtFlags, &dtp);
							rcContent.top += statusFontHeight;
							textCounter += dtp.uiLengthDrawn;
						}
						while (textCounter <= ulLen && dtp.uiLengthDrawn && rcContent.top + statusFontHeight <= old_bottom);
					}
				}
			}
		}
	}
	if (selected) {
		if (type != CLCIT_DIVIDER) {
			TCHAR *szText = contact->szText;
			RECT rc;
			int qlen = (int)mir_tstrlen(dat->szQuickSearch);
			if (hPreviousFont)
				SelectObject(hdcMem, hPreviousFont);
			SetTextColor(hdcMem, dat->quickSearchColour);
			if (type == CLCIT_CONTACT) {
				rc.left = rcContent.left;
				rc.top = y + ((rowHeight - fontHeight) >> 1);
				rc.right = rcContent.right;
				rc.bottom = rc.top;
				if (twoRows)
					rc.top = y;
			}
			else {
				rc.left = qLeft;
				rc.top = y + ((rowHeight - fontHeight) >> 1);
				rc.right = clRect->right - rightOffset;
				rc.bottom = rc.top;
			}
			if (qlen)
				DrawText(hdcMem, szText, qlen, &rc, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE);
		}
	}

	if (g_RTL)
		SetLayout(hdcMem, 0);
}

void SkinDrawBg(HWND hwnd, HDC hdc)
{
	RECT rcCl;
	POINT ptTest = { 0 };

	ClientToScreen(hwnd, &ptTest);
	GetClientRect(hwnd, &rcCl);

	BitBlt(hdc, 0, 0, rcCl.right - rcCl.left, rcCl.bottom - rcCl.top, cfg::dat.hdcBg, ptTest.x - cfg::dat.ptW.x, ptTest.y - cfg::dat.ptW.y, SRCCOPY);
}

void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT *rcPaint)
{
	RECT clRect;
	DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
	int grey = 0;
	BOOL bFirstNGdrawn = FALSE;
	int line_num = -1;
	COLORREF tmpbkcolour = style & CLS_CONTACTLIST ? (dat->useWindowsColours ? GetSysColor(COLOR_3DFACE) : dat->bkColour) : dat->bkColour;
	selBlend = cfg::getByte("CLCExt", "EXBK_SelBlend", 1);
	g_inCLCpaint = TRUE;
	g_focusWnd = GetFocus();
	my_status = GetGeneralisedStatus();
	g_HDC = hdc;

	/*
	* temporary DC for avatar drawing
	*/

	g_padding_y = 0;

	hdcTempAV = CreateCompatibleDC(g_HDC);
	hdcAV = CreateCompatibleDC(g_HDC);
	hbmTempAV = CreateCompatibleBitmap(g_HDC, g_maxAV_X, g_maxAV_Y);
	hbmTempOldAV = reinterpret_cast<HBITMAP>(SelectObject(hdcTempAV, hbmTempAV));

	cfg::dat.t_now = time(NULL);
	GetSystemTime(&cfg::dat.st);
	SystemTimeToFileTime(&cfg::dat.st, &cfg::dat.ft);

	cfg::dat.bUseFastGradients = cfg::dat.bWantFastGradients && (GdiGradientFill != 0);

	av_left = (cfg::dat.dwFlags & CLUI_FRAME_AVATARSLEFT);
	av_right = (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHT);
	av_rightwithnick = (cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHTWITHNICK);

	mirror_rtl = (cfg::dat.bUseDCMirroring == 2);
	mirror_always = (cfg::dat.bUseDCMirroring == 1);
	mirror_rtltext = (cfg::dat.bUseDCMirroring == 3);

	g_center = cfg::getByte("CLCExt", "EXBK_CenterGroupnames", 0) && !dat->bisEmbedded;
	g_ignoreselforgroups = cfg::getByte("CLC", "IgnoreSelforGroups", 0);

	if (dat->greyoutFlags & pcli->pfnClcStatusToPf2(my_status) || style & WS_DISABLED)
		grey = 1;
	else if (GetFocus() != hwnd && dat->greyoutFlags & GREYF_UNFOCUS)
		grey = 1;
	GetClientRect(hwnd, &clRect);
	if (rcPaint == NULL)
		rcPaint = &clRect;
	if (IsRectEmpty(rcPaint)) {
		SelectObject(hdcTempAV, hbmTempOldAV);
		DeleteObject(hbmTempAV);
		DeleteDC(hdcTempAV);

		DeleteDC(hdcAV);
		return;
	}

	int y = -dat->yScroll;
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), NULL);

	HBITMAP hOldBitmap = reinterpret_cast<HBITMAP>(SelectObject(hdcMem, hBmpOsb));

	TEXTMETRIC tm;
	HFONT hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPS].hFont));
	GetTextMetrics(hdcMem, &tm);
	int groupCountsFontTopShift = tm.tmAscent;
	SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
	GetTextMetrics(hdcMem, &tm);
	groupCountsFontTopShift -= tm.tmAscent;

	int fontHeight;
	ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);

	SetBkMode(hdcMem, TRANSPARENT);

	HBRUSH hBrush = CreateSolidBrush(tmpbkcolour);
	HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
	FillRect(hdcMem, rcPaint, hBrush);

	SelectObject(hdcMem, hoBrush);
	DeleteObject(hBrush);

	if (cfg::dat.bWallpaperMode && !dat->bisEmbedded) {
		SkinDrawBg(hwnd, hdcMem);
		goto bgdone;
	}

	if (dat->hBmpBackground) {
		int destw, desth;
		// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
		SetStretchBltMode(hdcMem, HALFTONE);

		BITMAP bmp;
		GetObject(dat->hBmpBackground, sizeof(bmp), &bmp);
		HDC hdcBmp = CreateCompatibleDC(hdcMem);
		SelectObject(hdcBmp, dat->hBmpBackground);
		int y = dat->backgroundBmpUse & CLBF_SCROLL ? -dat->yScroll : 0;
		int maxx = dat->backgroundBmpUse & CLBF_TILEH ? clRect.right : 1;
		int maxy = dat->backgroundBmpUse & CLBF_TILEV ? maxy = rcPaint->bottom : y + 1;
		switch (dat->backgroundBmpUse & CLBM_TYPE) {
		case CLB_STRETCH:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				if (clRect.right * bmp.bmHeight < clRect.bottom * bmp.bmWidth) {
					desth = clRect.bottom;
					destw = desth * bmp.bmWidth / bmp.bmHeight;
				}
				else {
					destw = clRect.right;
					desth = destw * bmp.bmHeight / bmp.bmWidth;
				}
			}
			else {
				destw = clRect.right;
				desth = clRect.bottom;
			}
			break;
		case CLB_STRETCHH:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				destw = clRect.right;
				desth = destw * bmp.bmHeight / bmp.bmWidth;
			}
			else {
				destw = clRect.right;
				desth = bmp.bmHeight;
			}
			break;
		case CLB_STRETCHV:
			if (dat->backgroundBmpUse & CLBF_PROPORTIONAL) {
				desth = clRect.bottom;
				destw = desth * bmp.bmWidth / bmp.bmHeight;
			}
			else {
				destw = bmp.bmWidth;
				desth = clRect.bottom;
			}
			break;
		default:
			//clb_topleft
			destw = bmp.bmWidth;
			desth = bmp.bmHeight;
			break;
		}

		int bitx = 0, bity = 0;

		for (; y < maxy; y += desth) {
			if (y < rcPaint->top - desth)
				continue;
			for (int x = 0; x < maxx; x += destw)
				StretchBlt(hdcMem, x, y, destw, desth, cfg::dat.hdcPic, bitx, bity, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		}
		DeleteDC(hdcBmp);
	}

bgdone:
	ClcGroup *group = &dat->list;
	group->scanIndex = 0;

	if (dat->row_heights == NULL)
		RowHeight::calcRowHeights(dat, hwnd);

	group = &dat->list;
	group->scanIndex = 0;

	g_list_avatars = 0;
	while (true) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			if (group == NULL) break;	// Finished list
			group->scanIndex++;
			continue;
		}

		if (group->cl.items[group->scanIndex]->cFlags & ECF_AVATAR)
			g_list_avatars++;

		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (group->cl.items[group->scanIndex]->group->expanded)) {
			group = group->cl.items[group->scanIndex]->group;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	g_selectiveIcon = (g_list_avatars > 0) && (cfg::dat.dwFlags & CLUI_FRAME_SELECTIVEICONS) && !dat->bisEmbedded;

	group = &dat->list;
	group->scanIndex = 0;
	int indent = 0;

	for (int index = 0; y< rcPaint->bottom;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			indent--;
			if (group == NULL)
				break;

			group->scanIndex++;
			continue;
		}

		line_num++;
		if (cfg::dat.bForceRefetchOnPaint)
			group->cl.items[group->scanIndex]->ace = (struct avatarCacheEntry*) - 1;

		if (y > rcPaint->top - dat->row_heights[line_num] && y <= rcPaint->bottom) {
			if (group->cl.items[group->scanIndex]->ace == (struct avatarCacheEntry*) - 1)
				group->cl.items[group->scanIndex]->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)group->cl.items[group->scanIndex]->hContact, 0);
			RowHeight::getRowHeight(dat, group->cl.items[group->scanIndex], line_num, style);
			PaintItem(hdcMem, group, group->cl.items[group->scanIndex], indent, y, dat, index, hwnd, style, &clRect, &bFirstNGdrawn, groupCountsFontTopShift, dat->row_heights[line_num]);
		}
		index++;
		y += dat->row_heights[line_num];
		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && (group->cl.items[group->scanIndex]->group->expanded)) {
			group = group->cl.items[group->scanIndex]->group;
			indent++;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}

	SelectObject(hdcTempAV, hbmTempOldAV);
	DeleteObject(hbmTempAV);
	DeleteDC(hdcTempAV);

	DeleteDC(hdcAV);

	if (dat->iInsertionMark != -1) {
		//insertion mark
		POINT pts[8];
		pts[0].x = dat->leftMargin;   pts[0].y = RowHeight::getItemTopY(dat, dat->iInsertionMark) - dat->yScroll - 4;
		pts[1].x = pts[0].x + 2;      pts[1].y = pts[0].y + 3;
		pts[2].x = clRect.right - 4;  pts[2].y = pts[1].y;
		pts[3].x = clRect.right - 1;  pts[3].y = pts[0].y - 1;
		pts[4].x = pts[3].x;          pts[4].y = pts[0].y + 7;
		pts[5].x = pts[2].x + 1;      pts[5].y = pts[1].y + 2;
		pts[6].x = pts[1].x;          pts[6].y = pts[5].y;
		pts[7].x = pts[0].x;          pts[7].y = pts[4].y;
		HRGN hRgn = CreatePolygonRgn(pts, SIZEOF(pts), ALTERNATE);
		HBRUSH hBrush = CreateSolidBrush(dat->fontInfo[FONTID_CONTACTS].colour);
		HBRUSH hoBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
		FillRgn(hdcMem, hRgn, hBrush);
		SelectObject(hdcMem, hoBrush);
		DeleteObject(hBrush);
	}
	if (!grey)
		BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);

	SelectObject(hdcMem, hOldBitmap);
	SelectObject(hdcMem, hOldFont);

	DeleteDC(hdcMem);
	if (grey) {
		PBYTE bits;
		BITMAPINFOHEADER bmih = { 0 };

		int i;
		int greyRed, greyGreen, greyBlue;
		COLORREF greyColour;
		bmih.biBitCount = 32;
		bmih.biSize = sizeof(bmih);
		bmih.biCompression = BI_RGB;
		bmih.biHeight = -clRect.bottom;
		bmih.biPlanes = 1;
		bmih.biWidth = clRect.right;
		bits = (PBYTE)mir_alloc(4 * bmih.biWidth * -bmih.biHeight);
		GetDIBits(hdc, hBmpOsb, 0, clRect.bottom, bits, (BITMAPINFO *)&bmih, DIB_RGB_COLORS);
		greyColour = GetSysColor(COLOR_3DFACE);
		greyRed = GetRValue(greyColour) * 2;
		greyGreen = GetGValue(greyColour) * 2;
		greyBlue = GetBValue(greyColour) * 2;
		if (divide3[0] == 255) {
			for (i = 0; i < sizeof(divide3) / sizeof(divide3[0]); i++)
				divide3[i] = (i + 1) / 3;
		}
		for (i = 4 * clRect.right *clRect.bottom - 4; i >= 0; i -= 4) {
			bits[i] = divide3[bits[i] + greyBlue];
			bits[i + 1] = divide3[bits[i + 1] + greyGreen];
			bits[i + 2] = divide3[bits[i + 2] + greyRed];
		}
		SetDIBitsToDevice(hdc, 0, 0, clRect.right, clRect.bottom, 0, 0, 0, clRect.bottom, bits, (BITMAPINFO *)&bmih, DIB_RGB_COLORS);
		mir_free(bits);
	}
	DeleteObject(hBmpOsb);
	g_inCLCpaint = FALSE;
}
