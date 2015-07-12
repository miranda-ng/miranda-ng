/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda IM: the free IM client for Microsoft* Windows*
 *
 * Copyright 2000-2010 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of clist_ng plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * $Id: clcpaint.cpp 134 2010-10-01 10:23:10Z silvercircle $
 *
 */

#include <commonheaders.h>

HANDLE						CLC::hTheme = 0;

extern 	ORDERTREEDATA 		OrderTreeData[];
extern 	HICON 				overlayicons[];
extern  FRAMEWND*			wndFrameCLC;
extern wchar_t*				statusNames[];

int 						g_center, g_ignoreselforgroups, g_selectiveIcon, g_exIconSpacing;
HWND 						g_focusWnd;
BYTE 						selBlend;
int 						my_status;

//static Gdiplus::Graphics*	_g;

static wchar_t wszTimeFormat[] = L"%H:%M";
static LONG		dxBufferDiff, dBufferDiff;

HFONT CLCPaintHelper::changeToFont(const unsigned int id)
{
	HFONT 	hOldFont = 0;

	hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, dat->fontInfo[id].hFont));
	Gfx::setTextColor(dat->fontInfo[id].colour);
	m_fontHeight = dat->fontInfo[id].fontHeight;

	dat->currentFontID = id;
	return hOldFont;
}

void CLCPaintHelper::setHotTrackColour()
{
	if (dat->gammaCorrection) {
		COLORREF oldCol, newCol;
		int oldLum, newLum;

		oldCol = GetTextColor(hdcMem);
		oldLum = (GetRValue(oldCol) * 30 + GetGValue(oldCol) * 59 + GetBValue(oldCol) * 11) / 100;
		newLum = (GetRValue(dat->hotTextColour) * 30 + GetGValue(dat->hotTextColour) * 59 + GetBValue(dat->hotTextColour) * 11) / 100;
		if (newLum == 0) {
			Gfx::setTextColor(dat->hotTextColour);
			return;
		}
		if (newLum >= oldLum + 20) {
			oldLum += 20;
			newCol = RGB(GetRValue(dat->hotTextColour) * oldLum / newLum, GetGValue(dat->hotTextColour) * oldLum / newLum, GetBValue(dat->hotTextColour) * oldLum / newLum);
		} else if (newLum <= oldLum) {
			int r, g, b;
			r = GetRValue(dat->hotTextColour) * oldLum / newLum;
			g = GetGValue(dat->hotTextColour) * oldLum / newLum;
			b = GetBValue(dat->hotTextColour) * oldLum / newLum;
			if (r > 255) {
				g +=(r-255)*3 / 7;
				b +=(r-255)*3 / 7;
				r = 255;
			}
			if (g > 255) {
				r +=(g-255)*59 / 41;
				if (r > 255)
					r = 255;
				b +=(g-255)*59 / 41;
				g = 255;
			}
			if (b > 255) {
				r +=(b-255)*11 / 89;
				if (r > 255)
					r = 255;
				g +=(b-255)*11 / 89;
				if (g > 255)
					g = 255;
				b = 255;
			}
			newCol = RGB(r, g, b);
		} else
			newCol = dat->hotTextColour;
		Gfx::setTextColor(newCol);
	} else
		Gfx::setTextColor(dat->hotTextColour);
}

/**
 * return "relative" onlineness value based on the status mode
 *
 * @param status 				 status mode
 * @param ignoreConnectingState  ignore the special "in
 *  							 connection progress" status
 *
 * @return int  				 "onlineness" value for the
 *  	   given status mode
 */
static int __fastcall GetRealStatus(struct ClcContact *contact, int status)
{
	int i;
	char *szProto = contact->proto;
	if (!szProto)
		return status;
	for (i = 0; i < pcli->hClcProtoCount; i++) {
		if (!lstrcmpA(pcli->clcProto[i].szProto, szProto)) {
			return pcli->clcProto[i].dwStatus;
		}
	}
	return status;
}

int GetBasicFontID(struct ClcContact * contact)
{
	switch (contact->type)	{
		case CLCIT_CONTACT:
			if (contact->flags & CONTACTF_NOTONLIST)
				return FONTID_NOTONLIST;
			else if ((contact->flags&CONTACTF_INVISTO && GetRealStatus(contact, ID_STATUS_OFFLINE) != ID_STATUS_INVISIBLE)
				|| (contact->flags&CONTACTF_VISTO && GetRealStatus(contact, ID_STATUS_OFFLINE) == ID_STATUS_INVISIBLE))
				return contact->flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS;
			else
				return contact->flags & CONTACTF_ONLINE ? FONTID_CONTACTS : FONTID_OFFLINE;
			break;
		case CLCIT_GROUP:
			return FONTID_GROUPS;
		case CLCIT_INFO:
			if(contact->flags & CLCIIF_GROUPFONT)
				return FONTID_GROUPS;
			else
				return FONTID_CONTACTS;
		case CLCIT_DIVIDER:
			return FONTID_DIVIDERS;
		default:
			return FONTID_CONTACTS;
	}
}

static BLENDFUNCTION bf = {0, 0, AC_SRC_OVER, 0};
static BOOL avatar_done = FALSE;
HDC g_HDC;
static BOOL g_RTL;
HDC hdcTempAV;
HBITMAP hbmTempAV, hbmTempOldAV;

LONG g_maxAV_X = 200, g_maxAV_Y = 200;

int CLCPaintHelper::drawAvatar(RECT *rc, ClcContact *contact, int y, WORD cstatus, int rowHeight)
{
	float 		dScale = 0.;
	float 		newHeight, newWidth, dAspect;
	DWORD 		topoffset = 0, leftoffset = 0, dwFlags = contact->dwDFlags;
	LONG 		bmWidth, bmHeight;
	HBITMAP 	hbm;
	HRGN 		rgn = 0;
	int 		avatar_size = cfg::dat.avatarSize;
	DWORD 		av_saved_left;
	TStatusItem *item = contact->wStatus == ID_STATUS_OFFLINE ? &Skin::statusItems[ID_EXTBKAVATARFRAMEOFFLINE] : &Skin::statusItems[ID_EXTBKAVATARFRAME];
	int  		skinMarginX, skinMarginY;
	BOOL 		fOverlay = (cfg::dat.dwFlags & CLUI_FRAME_OVERLAYICONS);

	contact->avatarLeft = -1;
	if(dat->bisEmbedded)
		return 0;

	if(contact->ace) {
		if(contact->ace->dwFlags & AVS_HIDEONCLIST) {
			if (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK)
				return avatar_size + 2;
			else
				return 0;
		}
		bmHeight = contact->ace->bmHeight;
		bmWidth = contact->ace->bmWidth;
		if(bmWidth != 0)
			dAspect = (float)bmHeight / (float)bmWidth;
		else
			dAspect = 1.0;
		hbm = contact->ace->hbmPic;
		contact->ace->t_lastAccess = cfg::dat.t_now;
	}
	else if (cfg::dat.dwFlags & CLUI_FRAME_ALWAYSALIGNNICK)
		return avatar_size + 2;
	else
		return 0;

	if(bmHeight == 0 || bmWidth == 0 || hbm == 0)
		return 0;

	g_maxAV_X = max(bmWidth, g_maxAV_X);
	g_maxAV_Y = max(bmHeight, g_maxAV_Y);

	if(dAspect >= 1.0) {            // height > width
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
	if(!item->IGNORED) {
		//topoffset += item->MARGIN_TOP;
		leftoffset = item->MARGIN_LEFT;
	}
	// create the region for the avatar border - use the same region for clipping, if needed.

	av_saved_left = rc->left;
	if(cfg::dat.bCenterStatusIcons && newWidth < newHeight)
		rc->left += (((avatar_size - 2) - ((int)newWidth + skinMarginX)) / 2);

	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = contact->ace->dwFlags & AVS_PREMULTIPLIED ? AC_SRC_ALPHA : 0;


	FIBITMAP *fib = cfg::fif->FI_CreateDIBFromHBITMAP(contact->ace->hbmPic);
	FIBITMAP *fibnew = cfg::fif->FI_Rescale(fib, newWidth, newHeight, FILTER_LANCZOS3);
	HBITMAP hbmNew = cfg::fif->FI_CreateHBITMAPFromDIB(fibnew);
	HBITMAP hbmTempOld = (HBITMAP)SelectObject(hdcTempAV, hbmNew);

	Api::pfnAlphaBlend(hdcMem, leftoffset + rc->left - (g_RTL ? 1 : 0), y + topoffset, (int)newWidth, (int)newHeight, hdcTempAV, 0, 0, newWidth, newHeight, bf);

	SelectObject(hdcTempAV, hbmTempOld);
	if(hbmNew != contact->ace->hbmPic)
		DeleteObject(hbmNew);
	cfg::fif->FI_Unload(fib);
	cfg::fif->FI_Unload(fibnew);

	if(cfg::dat.dwFlags & CLUI_FRAME_AVATARBORDER) {
		rgn = CreateRectRgn(leftoffset + rc->left, y + topoffset, leftoffset + rc->left + (int)newWidth, y + topoffset + (int)newHeight);
		if(g_RTL)
			OffsetRgn(rgn, -1 , 0);
		FrameRgn(hdcMem, rgn, cfg::dat.hBrushAvatarBorder, 1, 1);
		DeleteObject(rgn);
	}

	if(fOverlay)
		fOverlay = (dwFlags & ECF_HIDEOVERLAY) ? 0 : 1;
	else
		fOverlay = (dwFlags & ECF_FORCEOVERLAY) ? 1 : 0;

	if(fOverlay && cstatus && (int)newHeight >= CYSMICON)
		DrawIconEx(hdcMem, rc->left + (int)newWidth - 15, y + topoffset + (int)newHeight - 15, overlayicons[cstatus - ID_STATUS_OFFLINE], CYSMICON, CXSMICON, 0, 0, DI_NORMAL | DI_COMPAT);

	if(!item->IGNORED) {
		RECT rcFrame;
		bool inClCPaint_save = CLC::fInPaint;
		HDC  hdcTemp = 0;
		HBITMAP hbmOld, hbmTemp;

		CLC::fInPaint = false;
		rcFrame.left = rc->left;
		rcFrame.top = y + topoffset - item->MARGIN_TOP;
		rcFrame.right = rcFrame.left + (int)newWidth + item->MARGIN_RIGHT + item->MARGIN_LEFT;
		rcFrame.bottom = rcFrame.top + (int)newHeight + item->MARGIN_BOTTOM + item->MARGIN_TOP;
		if(g_RTL) {
			RECT rcTemp;

			OffsetRect(&rcFrame, 1, 0);
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
			Gfx::renderSkinItem(hdcTemp, &rcTemp, item->imageItem);
			BitBlt(hdcMem, rcFrame.left, rcFrame.top, rcFrame.right - rcFrame.left, rcFrame.bottom - rcFrame.top,
				   hdcTemp, 0, 0, SRCCOPY);
			SelectObject(hdcTemp, hbmOld);
			DeleteObject(hbmTemp);
			DeleteDC(hdcTemp);
		} else
			Gfx::renderSkinItem(hdcMem, &rcFrame, item->imageItem);
		CLC::fInPaint = inClCPaint_save;
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

void CLCPaintHelper::Paint(ClcGroup *group, ClcContact *contact, int rowHeight)
{
	RECT 				rc;
	int 				iImage = -1;
	SIZE 				textSize = { 0 }, countsSize = { 0 }, spaceSize = { 0 };
	int 				width, checkboxWidth;
	TCHAR*				szCounts = 0;
	wchar_t*			wszCounts = 0;
	BOOL 				twoRows = FALSE;
	WORD 				cstatus;
	DWORD 				leftOffset = 0, rightOffset = 0;
	int 				iconXSpace = dat->iconXSpace;
	HFONT 				hPreviousFont = 0;
	BYTE 				type;
	BYTE 				flags;
	COLORREF 			oldGroupColor = -1;
	DWORD 				qLeft = 0;
	int 				leftX = dat->leftMargin + indent * dat->groupIndent;
	int 				bg_indent_r = 0;
	int 				bg_indent_l = 0;
	int 				rightIcons = 0;
	DWORD 				dt_nickflags = 0, dt_2ndrowflags = 0;
	TExtraCache*		cEntry = NULL;
	DWORD 				dwFlags = cfg::dat.dwFlags;
	int 				scanIndex;
	BOOL 				av_local_wanted, fLocalTime;

	if(!isContactFloater)
		current_shape = 0;

	rowHeight -= cfg::dat.bRowSpacing;

	if(group == NULL || contact == NULL)
		return;

	g_RTL = FALSE;
	scanIndex = group->scanIndex;
	av_local_wanted = (CLC::uNrAvatars > 0);

	type = contact->type;
	flags = contact->flags;
	avatar_done = FALSE;
	if(contact->extraCacheEntry >= 0 && contact->extraCacheEntry < cfg::nextCacheEntry)
		cEntry = &cfg::eCache[contact->extraCacheEntry];
	else
		cEntry = cfg::eCache;

	if(dat->bisEmbedded)
		goto set_bg_l;

	if(type == CLCIT_CONTACT && (cEntry->dwCFlags & ECF_RTLNICK || mirror_always)) {
		if(mirror_rtl || mirror_always) {
			g_RTL = TRUE;
			bg_indent_r = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
		}
		else if(mirror_rtltext) {
			bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
			dt_nickflags = DT_RTLREADING | DT_RIGHT;
		}
		else
			bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
	}
	else if(type == CLCIT_GROUP) {
		if((contact->isRtl && cfg::dat.bGroupAlign == CLC_GROUPALIGN_AUTO) || cfg::dat.bGroupAlign == CLC_GROUPALIGN_RIGHT) {
			g_RTL = TRUE;
			bg_indent_r = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
		}
		else
			bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;
	}
	else
		bg_indent_l = cfg::dat.bApplyIndentToBg ? indent * dat->groupIndent : 0;

set_bg_l:
	//setup
	if (type == CLCIT_GROUP) {
		changeToFont(FONTID_GROUPS);
		GetTextExtentPoint32(hdcMem, contact->szText, lstrlen(contact->szText), &textSize);
		width = textSize.cx;
		szCounts = pcli->pfnGetGroupCountsText(dat, contact);
		if (szCounts[0]) {
			GetTextExtentPoint32W(hdcMem, L" ", 1, &spaceSize);
			changeToFont(FONTID_GROUPCOUNTS);
			GetTextExtentPoint32W(hdcMem, szCounts, lstrlenW(szCounts), &countsSize);
			width += spaceSize.cx + countsSize.cx;
			wszCounts = szCounts;
		}
	} else if (type == CLCIT_INFO) {
		if (flags & CLCIIF_GROUPFONT)
			changeToFont(FONTID_GROUPS);
		else
			changeToFont(FONTID_CONTACTS);
	} else if (type == CLCIT_DIVIDER) {
		changeToFont(FONTID_DIVIDERS);
		GetTextExtentPoint32(hdcMem, contact->szText, lstrlen(contact->szText), &textSize);
	}
	else if (type == CLCIT_CONTACT && flags & CONTACTF_NOTONLIST)
		changeToFont(FONTID_NOTONLIST);
	else if (type == CLCIT_CONTACT && ((flags & CONTACTF_INVISTO && GetRealStatus(contact, my_status) != ID_STATUS_INVISIBLE) || (flags & CONTACTF_VISTO && GetRealStatus(contact, my_status) == ID_STATUS_INVISIBLE))) {
		changeToFont(flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS);
	} else if (type == CLCIT_CONTACT && !(flags & CONTACTF_ONLINE))
		changeToFont(FONTID_OFFLINE);
	else
		changeToFont(FONTID_CONTACTS);

	if ((style & CLS_CHECKBOXES && type == CLCIT_CONTACT) || (style & CLS_GROUPCHECKBOXES && type == CLCIT_GROUP) || (type == CLCIT_INFO && flags & CLCIIF_CHECKBOX))
		checkboxWidth = dat->checkboxSize + 2;
	else
		checkboxWidth = 0;

	rc.left = 0;
	cstatus = contact->wStatus;

	/***** BACKGROUND DRAWING *****/
	// contacts

	CLC::fHottrackDone = false;

	if(dat->bisEmbedded) {
		rc.left = bg_indent_l;
		rc.top = y;
		rc.right = clRect->right - bg_indent_r;
		rc.bottom = y + rowHeight;
		if (fSelected) {
			FillRect(hdcMem, &rc, GetSysColorBrush(COLOR_HIGHLIGHT));
			Gfx::setTextColor(dat->selTextColour);
		}
		else {
			FillRect(hdcMem, &rc, type == CLCIT_GROUP ? cfg::dat.hBrushCLCGroupsBk : cfg::dat.hBrushCLCBk);
			if(CLC::iHottrackItem)
				setHotTrackColour();
		}
		goto bgskipped;
	}
	if (type == CLCIT_CONTACT || type == CLCIT_DIVIDER) {
		TStatusItem *sitem, *pp_item;

		if (cstatus >= ID_STATUS_OFFLINE && cstatus <= ID_STATUS_OUTTOLUNCH) {
			BYTE perstatus_ignored;

			if((flags & CONTACTF_IDLE) && !Skin::statusItems[ID_EXTBKIDLE].IGNORED)
				sitem = &Skin::statusItems[ID_EXTBKIDLE];
			else
				sitem = &Skin::statusItems[cstatus - ID_STATUS_OFFLINE];

			if(!dat->bisEmbedded) {
				pp_item = cEntry->status_item ? cEntry->status_item : cEntry->proto_status_item;

				if (!(perstatus_ignored = sitem->IGNORED) && !(flags & CONTACTF_NOTONLIST))
					Gfx::setTextColor(sitem->TEXTCOLOR);

				if(cfg::dat.bUsePerProto && pp_item && !pp_item->IGNORED) {
					sitem = pp_item;
					if((perstatus_ignored || cfg::dat.bOverridePerStatusColors) && sitem->TEXTCOLOR != -1)
						Gfx::setTextColor(sitem->TEXTCOLOR);
				}
			}
			else if(!sitem->IGNORED)
				Gfx::setTextColor(sitem->TEXTCOLOR);

			rc.left = sitem->MARGIN_LEFT + bg_indent_l;
			rc.top = y + sitem->MARGIN_TOP;
			rc.right = clRect->right - sitem->MARGIN_RIGHT - bg_indent_r;
			rc.bottom = y + rowHeight - sitem->MARGIN_BOTTOM;

			// single item in a group
			if (!ssingleitem->IGNORED && scanIndex == 0 && group->cl.count == 1 && group->parent != NULL) {
				rc.left = ssingleitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + ssingleitem->MARGIN_TOP;
				rc.right = clRect->right - ssingleitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - ssingleitem->MARGIN_BOTTOM;

				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
					Gfx::renderSkinItem(this, sevencontact_pos, &rc);
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
					Gfx::renderSkinItem(this, soddcontact_pos, &rc);

				if (!sitem->IGNORED)
					Gfx::renderSkinItem(this, sitem, &rc);
				Gfx::renderSkinItem(this, ssingleitem, &rc);

			// first item in a group
			} else if (scanIndex == 0 && group->cl.count > 1 && !sfirstitem->IGNORED && group->parent != NULL) {
				rc.left = sfirstitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sfirstitem->MARGIN_TOP;
				rc.right = clRect->right - sfirstitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sfirstitem->MARGIN_BOTTOM;

				current_shape = sfirstitem->rect;
				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
					Gfx::renderSkinItem(this, sevencontact_pos, &rc);
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
					Gfx::renderSkinItem(this, soddcontact_pos, &rc);

				if (!sitem->IGNORED)
					Gfx::renderSkinItem(this, sitem, &rc);
				Gfx::renderSkinItem(this, sfirstitem, &rc);

			// last item in a group
			} else if (scanIndex == group->cl.count - 1 && !slastitem->IGNORED && group->parent != NULL) {
				rc.left = slastitem->MARGIN_LEFT + bg_indent_l;
				rc.top = y + slastitem->MARGIN_TOP;
				rc.right = clRect->right - slastitem->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - slastitem->MARGIN_BOTTOM;
				rc.bottom = y + rowHeight - slastitem->MARGIN_BOTTOM;

				current_shape = slastitem->rect;
				// draw odd/even contact underlay
				if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
					Gfx::renderSkinItem(this, sevencontact_pos, &rc);
				else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
					Gfx::renderSkinItem(this, soddcontact_pos, &rc);

				if (!sitem->IGNORED)
					Gfx::renderSkinItem(this, sitem, &rc);
				Gfx::renderSkinItem(this, slastitem, &rc);
			} else {
				// - - - Non-grouped items - - -
				if (type != CLCIT_GROUP && group->parent == NULL && !sfirstitem_NG->IGNORED && scanIndex != group->cl.count - 1 && !(bFirstNGdrawn)) {
					// first NON-grouped
					bFirstNGdrawn = TRUE;
					rc.left = sfirstitem_NG->MARGIN_LEFT + bg_indent_l;
					rc.top = y + sfirstitem_NG->MARGIN_TOP;
					rc.right = clRect->right - sfirstitem_NG->MARGIN_RIGHT - bg_indent_r;
					rc.bottom = y + rowHeight - sfirstitem_NG->MARGIN_BOTTOM;

					// draw odd/even contact underlay
					if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
						Gfx::renderSkinItem(this, sevencontact_pos, &rc);
					else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
						Gfx::renderSkinItem(this, soddcontact_pos, &rc);

					if (!sitem->IGNORED)
						Gfx::renderSkinItem(this, sitem, &rc);
					Gfx::renderSkinItem(this, sfirstitem, &rc);

				} else if (type != CLCIT_GROUP && group->parent == NULL && !slastitem_NG->IGNORED && scanIndex == group->cl.count - 1 && (bFirstNGdrawn)) {
					// last item of list (NON-group)
					// last NON-grouped
					rc.left = slastitem_NG->MARGIN_LEFT + bg_indent_l;
					rc.top = y + slastitem_NG->MARGIN_TOP;
					rc.right = clRect->right - slastitem_NG->MARGIN_RIGHT - bg_indent_r;
					rc.bottom = y + rowHeight - slastitem_NG->MARGIN_BOTTOM;

					// draw odd/even contact underlay
					if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
						Gfx::renderSkinItem(this, sevencontact_pos, &rc);
					else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
						Gfx::renderSkinItem(this, soddcontact_pos, &rc);

					if (!sitem->IGNORED)
						Gfx::renderSkinItem(this, sitem, &rc);
					Gfx::renderSkinItem(this, slastitem, &rc);
				} else if (type != CLCIT_GROUP	&& group->parent == NULL && !slastitem_NG->IGNORED && !(bFirstNGdrawn)) {
					// single item of NON-group
					// single NON-grouped
					rc.left = ssingleitem_NG->MARGIN_LEFT + bg_indent_l;
					rc.top = y + ssingleitem_NG->MARGIN_TOP;
					rc.right = clRect->right - ssingleitem_NG->MARGIN_RIGHT - bg_indent_r;
					rc.bottom = y + rowHeight - ssingleitem_NG->MARGIN_BOTTOM;

					// draw odd/even contact underlay
					if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
						Gfx::renderSkinItem(this, sevencontact_pos, &rc);
					else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
						Gfx::renderSkinItem(this, soddcontact_pos, &rc);

					if (!sitem->IGNORED)
						Gfx::renderSkinItem(this, sitem, &rc);
					Gfx::renderSkinItem(this, ssingleitem, &rc);
				} else {
					// draw default grouped
					// draw odd/even contact underlay
					if ((scanIndex == 0 || scanIndex % 2 == 0) && !sevencontact_pos->IGNORED)
						Gfx::renderSkinItem(this, sevencontact_pos, &rc);
					else if (scanIndex % 2 != 0 && !soddcontact_pos->IGNORED)
						Gfx::renderSkinItem(this, soddcontact_pos, &rc);

					if(!sitem->IGNORED)
						Gfx::renderSkinItem(this, sitem, &rc);
				}
			}
		}
	}
	if (type == CLCIT_GROUP) {
		TStatusItem *sempty = &Skin::statusItems[ID_EXTBKEMPTYGROUPS];
		TStatusItem *sexpanded = &Skin::statusItems[ID_EXTBKEXPANDEDGROUP];
		TStatusItem *scollapsed = &Skin::statusItems[ID_EXTBKCOLLAPSEDDGROUP];

		changeToFont(FONTID_GROUPS);
		if (contact->group->cl.count == 0) {
			if (!sempty->IGNORED) {
				rc.left = sempty->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sempty->MARGIN_TOP;
				rc.right = clRect->right - sempty->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - sempty->MARGIN_BOTTOM;
				Gfx::renderSkinItem(this, sempty, &rc);
				oldGroupColor = GetTextColor(hdcMem);
				Gfx::setTextColor(sempty->TEXTCOLOR);
			}
		} else if (contact->group->expanded) {
			if (!sexpanded->IGNORED) {
				rc.left = sexpanded->MARGIN_LEFT + bg_indent_l;
				rc.top = y + sexpanded->MARGIN_TOP;
				rc.right = clRect->right - sexpanded->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - (char) sexpanded->MARGIN_BOTTOM;
				Gfx::renderSkinItem(this, sexpanded, &rc);
				oldGroupColor =GetTextColor(hdcMem);
				Gfx::setTextColor(sexpanded->TEXTCOLOR);
			}
		} else {
			if (!scollapsed->IGNORED) {
				// collapsed but not empty
				rc.left = scollapsed->MARGIN_LEFT + bg_indent_l;
				rc.top = y + scollapsed->MARGIN_TOP;
				rc.right = clRect->right - scollapsed->MARGIN_RIGHT - bg_indent_r;
				rc.bottom = y + rowHeight - scollapsed->MARGIN_BOTTOM;
				Gfx::renderSkinItem(this, scollapsed, &rc);
				oldGroupColor = GetTextColor(hdcMem);
				Gfx::setTextColor(scollapsed->TEXTCOLOR);
			}
		}
	}
	if (fSelected) {
		TStatusItem *sselected = &Skin::statusItems[ID_EXTBKSELECTION];

		if (!g_ignoreselforgroups || type != CLCIT_GROUP) {
			rc.left = sselected->MARGIN_LEFT + bg_indent_l;
			rc.top = y + sselected->MARGIN_TOP;
			rc.right = clRect->right - sselected->MARGIN_RIGHT - bg_indent_r;
			rc.bottom = y + rowHeight - sselected->MARGIN_BOTTOM;
			Gfx::renderSkinItem(this, sselected, &rc);
			Gfx::setTextColor(sselected->TEXTCOLOR);
		}
	}
	else if (CLC::iHottrackItem) {
		TStatusItem *ht = &Skin::statusItems[ID_EXTBKHOTTRACK];

		if(ht->IGNORED == 0)
			Gfx::setTextColor(ht->TEXTCOLOR);
		if(!CLC::fHottrackDone) {
			if (ht->IGNORED == 0) {
				Gfx::renderSkinItem(this, ht, &rc);
			}
		}
	}

	if(g_RTL) {
		SetLayout(hdcMem, LAYOUT_RTL | LAYOUT_BITMAPORIENTATIONPRESERVED);
		dxBufferDiff = dBufferDiff;
	}
	else
		dxBufferDiff = 0;					// adjust discrepancy between the width of the buffered paint dc and the actual window width (bug in buffered paint?)
bgskipped:

	rcContent.top = y + g_padding_y;
	rcContent.bottom = y + rowHeight - (2 * g_padding_y);
	rcContent.left = leftX + dxBufferDiff;
	rcContent.right = clRect->right - dat->rightMargin + dxBufferDiff;
	twoRows = ((dat->fontInfo[FONTID_STATUS].fontHeight + m_fontHeight <= rowHeight + 1) && (fSecondLine)) && !dat->bisEmbedded;

	pi_avatar = (!dat->bisEmbedded && fAvatar);

	//checkboxes
	if (checkboxWidth) {
		RECT 	rc;

		rc.left = leftX;
		rc.right = rc.left + dat->checkboxSize;
		rc.top = y + ((rowHeight - dat->checkboxSize) >> 1);
		rc.bottom = rc.top + dat->checkboxSize;
		if (hTheme) {
			Api::pfnDrawThemeBackground(hTheme, hdcMem, BP_CHECKBOX, flags & CONTACTF_CHECKED ? (CLC::iHottrackItem ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL) :
					(CLC::iHottrackItem ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL), &rc, &rc);
		}
		else
			DrawFrameControl(hdcMem, &rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | (flags & CONTACTF_CHECKED ? DFCS_CHECKED : 0) | (CLC::iHottrackItem ? DFCS_HOT : 0));
		rcContent.left += checkboxWidth;
		leftX += checkboxWidth;
	}

	if (type == CLCIT_GROUP)
		iImage = (contact->group->expanded) ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
	else if (type == CLCIT_CONTACT)
		iImage = contact->iImage;


	if(pi_avatar && (av_left || av_right)) {
		RECT rc;

		rc.left = rcContent.left;
		rc.right = clRect->right;
		rc.top = y;
		rc.bottom = rc.top + rowHeight;

		if(av_left) {
			leftOffset += drawAvatar(&rc, contact, y, (WORD)(iImage ? cstatus : 0), rowHeight);
			rcContent.left += leftOffset;
			leftX += leftOffset;
		}
		else {
			rc.left = (rcContent.right - cfg::dat.avatarSize) + 1;
			rightOffset += drawAvatar(&rc, contact, y, (WORD)(iImage ? cstatus : 0), rowHeight);
			rcContent.right -= (rightOffset);
		}
	}
	else if(type == CLCIT_CONTACT && !dat->bisEmbedded && !g_selectiveIcon && (dwFlags & CLUI_FRAME_ALWAYSALIGNNICK) && av_local_wanted && (av_left || av_right)) {
		if(av_right)
			rcContent.right -= (cfg::dat.avatarSize + 2);
		if(av_left)
			rcContent.left += (cfg::dat.avatarSize + 2);
	}
	//icon

	// skip icon for groups if the option is enabled...

	if(type == CLCIT_GROUP && dwFlags & CLUI_FRAME_NOGROUPICON) {
		iconXSpace = 0;
		goto text;
	}
	if (iImage != -1) {
		// this doesnt use CLS_CONTACTLIST since the colour prolly wont match anyway
		COLORREF colourFg = dat->selBkColour;
		//int clientId = contact->clientId;
		int mode = ILD_NORMAL;
		pi_selectiveIcon = g_selectiveIcon && (type == CLCIT_CONTACT);

		if((dwFlags & CLUI_FRAME_STATUSICONS && !pi_selectiveIcon) || type != CLCIT_CONTACT || (pi_selectiveIcon && !avatar_done)) {
			HIMAGELIST hImgList = CLC::hClistImages;

			if (CLC::iHottrackItem) {
				colourFg = dat->hotTextColour;
			} else if (type == CLCIT_CONTACT && flags & CONTACTF_NOTONLIST) {
				colourFg = dat->fontInfo[FONTID_NOTONLIST].colour;
				mode = ILD_BLEND50;
			}
			if (type == CLCIT_CONTACT && dat->showIdle && (flags & CONTACTF_IDLE) && contact->wStatus != ID_STATUS_OFFLINE)
				mode = ILD_SELECTED;

			if(pi_selectiveIcon && av_right) {
				ImageList_DrawEx(hImgList, iImage, hdcMem, rcContent.right - 18, (twoRows && type == CLCIT_CONTACT && !cfg::dat.bCenterStatusIcons) ? y + 2 : y + ((rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
				rcContent.right -= 18;
			}
			else {
				LONG offset = 0;
				BOOL centered = FALSE;
				offset +=  (type != CLCIT_CONTACT || avatar_done || !(av_local_wanted) ? 20 : dwFlags & CLUI_FRAME_ALWAYSALIGNNICK && av_left && g_selectiveIcon ? cfg::dat.avatarSize + 2 : 20);
				centered = (cfg::dat.bCenterStatusIcons && offset == cfg::dat.avatarSize + 2);
				ImageList_DrawEx(hImgList, iImage, hdcMem,  centered ? rcContent.left + offset / 2 - 10 : rcContent.left, (twoRows && type == CLCIT_CONTACT && !cfg::dat.bCenterStatusIcons) ? y + 2 : y + ((rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
				rcContent.left += offset;
			}
		}
		else
			iconXSpace = 0;
		if (type == CLCIT_CONTACT && !dat->bisEmbedded) {
			BYTE bApparentModeDontCare = !((flags & CONTACTF_VISTO) ^ (flags & CONTACTF_INVISTO));
			contact->extraIconRightBegin = 0;
			if(cEntry && (contact->extraCacheEntry >= 0 && contact->extraCacheEntry < cfg::nextCacheEntry && cEntry->iExtraValid)) {
				int i, iIndex, id;
				DWORD dwOldMask = cEntry->dwXMask;

				for(i = EXICON_COUNT - 1; i >= 0; i--) {
					iIndex = cfg::dat.exIconOrder[i] - 1;
					if(iIndex >= 0 && iIndex < EXICON_COUNT) {
						id = OrderTreeData[iIndex].ID;
						if(cEntry->iExtraImage[id] != 0xff && ((1 << id) & cEntry->dwXMask)) {
							if(contact->extraIconRightBegin == 0 && i != (EXICON_COUNT - 1))
								contact->extraIconRightBegin = rcContent.right;
							ImageList_DrawEx(dat->himlExtraColumns, cEntry->iExtraImage[id], hdcMem, rcContent.right - cfg::dat.exIconScale, twoRows ? rcContent.bottom - g_exIconSpacing : y + ((rowHeight - cfg::dat.exIconScale) >> 1),
								0, 0, CLR_NONE, CLR_NONE, ILD_NORMAL);
							rcContent.right -= g_exIconSpacing;
							rightIcons++;
						}
					}
				}
				cEntry->dwXMask = dwOldMask;
			}
			if (!bApparentModeDontCare && (dwFlags & CLUI_SHOWVISI) && contact->proto) {
				BOOL fVisi;

				if(dwFlags & CLUI_SHOWVISI)
					fVisi = contact->dwDFlags & ECF_HIDEVISIBILITY ? 0 : 1;
				else
					fVisi = contact->dwDFlags & ECF_FORCEVISIBILITY ? 1 : 0;

				if(fVisi) {
					if(cEntry->isChatRoom)
						DrawIconEx(hdcMem, rcContent.right - cfg::dat.exIconScale, twoRows ? rcContent.bottom - g_exIconSpacing : y + ((rowHeight - cfg::dat.exIconScale) >> 1),
						cfg::dat.hIconChatactive, cfg::dat.exIconScale, cfg::dat.exIconScale, 0, 0, DI_NORMAL | DI_COMPAT);
					else
						DrawIconEx(hdcMem, rcContent.right - cfg::dat.exIconScale, twoRows ? rcContent.bottom - g_exIconSpacing : y + ((rowHeight - cfg::dat.exIconScale) >> 1),
						flags & CONTACTF_VISTO ? cfg::dat.hIconVisible : cfg::dat.hIconInvisible, cfg::dat.exIconScale, cfg::dat.exIconScale, 0, 0, DI_NORMAL | DI_COMPAT);
					rcContent.right -= g_exIconSpacing;
					rightIcons++;
				}
			}
		}
	}
	//text
text:
	if (type == CLCIT_DIVIDER) {
		RECT rc, rcText;
		rc.top = y + ((rowHeight) >> 1) - 1; rc.bottom = rc.top + 2;
		rc.left = clRect->left + dat->leftMargin;
		rc.right = rc.left - dat->rightMargin + ((clRect->right - rc.left - textSize.cx) >> 1) - 3;
		DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
		Api::pfnBufferedPaintSetAlpha(hbp, &rc, 255);
		rcText.left = rc.right + 3;
		rcText.top = y;
		rcText.bottom = y + rowHeight;
		rcText.right = rcText.left + textSize.cx;
		Gfx::renderText(hdcMem, hTheme, contact->szText, &rcText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		rc.left = rc.right + 6 + textSize.cx;
		rc.right = clRect->right - dat->rightMargin;
		DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
		Api::pfnBufferedPaintSetAlpha(hbp, &rc, 255);
	} else if (type == CLCIT_GROUP) {
		RECT rc;
		m_fontHeight = dat->fontInfo[FONTID_GROUPS].fontHeight;
		rc.top = y + ((rowHeight - m_fontHeight) >> 1) + cfg::dat.group_padding;
		rc.bottom = rc.top + textSize.cy;
		if (szCounts[0]) {
			int required, labelWidth, offset = 0;
			COLORREF clr = GetTextColor(hdcMem);

			changeToFont(FONTID_GROUPCOUNTS);
			if(oldGroupColor != -1)
				Gfx::setTextColor(clr);

			rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + iconXSpace;
			rc.right = clRect->right - dat->rightMargin;

			if(indent == 0 && iconXSpace == 0)
				rc.left += 2;

			required = textSize.cx + countsSize.cx + spaceSize.cx;

			if(required > rc.right - rc.left)
				textSize.cx = (rc.right - rc.left) - countsSize.cx - spaceSize.cx;

			labelWidth = textSize.cx + countsSize.cx + spaceSize.cx;
			if(g_center)
				offset = ((rc.right - rc.left) - labelWidth) / 2;

			RECT rcCounts = rc;
			rcCounts.left += (offset + textSize.cx + spaceSize.cx + dxBufferDiff);
			rcCounts.right += dxBufferDiff;
			rcCounts.top += groupCountsFontTopShift;
			Gfx::renderText(hdcMem, hTheme, wszCounts, &rcCounts, DT_SINGLELINE);
			mir_free(wszCounts);

			if (fSelected && !g_ignoreselforgroups)
				clr = dat->selTextColour;
			else
				Gfx::setTextColor(clr);
			changeToFont(FONTID_GROUPS);
			rc.left += (offset + dxBufferDiff);
			rc.right = rc.left + textSize.cx;
			qLeft = rc.left;

			Gfx::renderText(hdcMem, hTheme, contact->szText, &rc, DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS);
		} else if (g_center && !szCounts[0]) {
			int offset;

			rc.left = rcContent.left;
			rc.right = clRect->right - dat->rightMargin;
			if(textSize.cx >= rc.right - rc.left)
				textSize.cx = rc.right - rc.left;

			offset = ((rc.right - rc.left) - textSize.cx) / 2;
			rc.left += (offset + dxBufferDiff);
			rc.right = rc.left + textSize.cx + dxBufferDiff;
			Gfx::renderText(hdcMem, hTheme, contact->szText, &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);
			qLeft = rc.left;
		} else {
			qLeft = rcContent.left + (indent == 0 && iconXSpace == 0 ? 2 : 0);
			rc.left = qLeft;
			rc.right = min(rc.left + textSize.cx, clRect->right - dat->rightMargin) + dxBufferDiff;
			Gfx::renderText(hdcMem, hTheme, contact->szText, &rc, DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_WORD_ELLIPSIS);
		}
	} else {
		wchar_t *szText = contact->szText;

		rcContent.top = y + ((rowHeight - m_fontHeight) >> 1);

		// avatar

		if(!dat->bisEmbedded) {
			if(av_local_wanted && !avatar_done && pi_avatar) {
				if(av_rightwithnick) {
					RECT rcAvatar = rcContent;

					rcAvatar.left = rcContent.right - (cfg::dat.avatarSize - 1);
					drawAvatar(&rcAvatar, contact, y, (WORD)(iImage ? cstatus : 0), rowHeight);
					rcContent.right -= (cfg::dat.avatarSize + 2);
				}
				else
					rcContent.left += drawAvatar(&rcContent, contact, y, (WORD)(iImage ? cstatus : 0), rowHeight);
			}
			else if(dwFlags & CLUI_FRAME_ALWAYSALIGNNICK && !avatar_done && av_local_wanted)
				rcContent.left += (dwFlags & (CLUI_FRAME_AVATARSLEFT | CLUI_FRAME_AVATARSRIGHT | CLUI_FRAME_AVATARSRIGHTWITHNICK) ? 0 : cfg::dat.avatarSize + 2);
		}

		// nickname
		if(!twoRows)
			Gfx::renderText(hdcMem, hTheme, szText, &rcContent, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE | dt_nickflags);
		else {
			DWORD dtFlags = DT_WORD_ELLIPSIS | DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE;
			DWORD saved_right = rcContent.right;
			BOOL verticalfit = FALSE;

			rcContent.top = y + cfg::dat.avatarPadding / 2;

			if(cfg::dat.bShowLocalTime)
				fLocalTime = contact->dwDFlags & ECF_HIDELOCALTIME ? 0 : 1;
			else
				fLocalTime = contact->dwDFlags & ECF_FORCELOCALTIME ? 1 : 0;

			if(cEntry->hTimeZone != 0 && fLocalTime) {
				wchar_t 	szResult[80];
				int  		idOldFont;
				SIZE 		szTime;
				RECT 		rc = rcContent;
				COLORREF 	oldColor = 0;

				if (TimeZone_PrintDateTime(cEntry->hTimeZone, _T("t"), szResult, _countof(szResult), 0))
					goto nodisplay;

				oldColor = Gfx::getTextColor();
				idOldFont = dat->currentFontID;
				changeToFont(FONTID_TIMESTAMP);
				GetTextExtentPoint32(hdcMem, szResult, lstrlenW(szResult), &szTime);
				verticalfit = ((rowHeight - szTime.cy) >= (cfg::dat.exIconScale + 1));

				if(av_right) {
					if(verticalfit)
						rc.left = rcContent.right + (rightIcons * g_exIconSpacing) - szTime.cx - 2;
					else
						rc.left = rcContent.right - szTime.cx - 2;
				}
				else if(av_rightwithnick) {
					if(verticalfit && rightIcons * g_exIconSpacing >= szTime.cx)
						rc.left = (clRect->right - dat->rightMargin - szTime.cx) + dxBufferDiff;
					else if(verticalfit && !avatar_done)
						rc.left = (clRect->right - dat->rightMargin - szTime.cx) + dxBufferDiff;
					else {
						rc.left = rcContent.right - szTime.cx - 2;
						rcContent.right = rc.left - 2;
					}
				}
				else {
					if(verticalfit)
						rc.left = (clRect->right - dat->rightMargin - szTime.cx) + dxBufferDiff;
					else
						rc.left = rcContent.right - szTime.cx - 2;
				}
				rc.right = clRect->right + dxBufferDiff;
				Gfx::renderText(hdcMem, hTheme, szResult, &rc, DT_NOPREFIX | DT_NOCLIP | DT_SINGLELINE);
				changeToFont(idOldFont);
				Gfx::setTextColor(oldColor);

				verticalfit = (rowHeight - m_fontHeight >= cfg::dat.exIconScale + 1);
				if(verticalfit && av_right)
					rcContent.right = min((clRect->right - cfg::dat.avatarSize - 2) + dxBufferDiff, rc.left - 2);
				else if(verticalfit && !av_rightwithnick)
					rcContent.right = min((clRect->right - dat->rightMargin) + dxBufferDiff, rc.left - 3);

			}
			else {
nodisplay:
				verticalfit = (rowHeight - m_fontHeight >= cfg::dat.exIconScale + 1);
				if(avatar_done) {
					if(verticalfit && av_right)
						rcContent.right = (clRect->right - cfg::dat.avatarSize - 2) + dxBufferDiff;
					else if(verticalfit && !av_rightwithnick)
						rcContent.right = (clRect->right - dat->rightMargin) + dxBufferDiff;
				}
			}
			Gfx::renderText(hdcMem, hTheme, szText, &rcContent, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE | dt_nickflags);

			rcContent.right = saved_right;
			rcContent.top += (m_fontHeight - 1);
			hPreviousFont = changeToFont(FONTID_STATUS);
			rcContent.bottom = y + rowHeight;

			if(cstatus >= ID_STATUS_OFFLINE && cstatus <= ID_STATUS_OUTTOLUNCH) {
				wchar_t *szText = NULL;
				BYTE smsgValid = cEntry->bStatusMsgValid;

				if((dwFlags & CLUI_FRAME_SHOWSTATUSMSG && smsgValid > STATUSMSG_XSTATUSID) || smsgValid == STATUSMSG_XSTATUSNAME)
					szText = cEntry->statusMsg;
				else
					szText = &statusNames[cstatus - ID_STATUS_OFFLINE][0];
				if(cEntry->dwCFlags & ECF_RTLSTATUSMSG && cfg::dat.bUseDCMirroring == 3)
					dt_2ndrowflags |= (DT_RTLREADING | DT_RIGHT);

				if(rightIcons == 0) {
					if((rcContent.bottom - rcContent.top) >= (2 * m_fontHeight)) {
						dtFlags &= ~(DT_SINGLELINE | DT_BOTTOM | DT_NOCLIP);
						dtFlags |= DT_WORDBREAK;
						rcContent.bottom -= ((rcContent.bottom - rcContent.top) % m_fontHeight);
					}
					Gfx::renderText(hdcMem, hTheme, szText, &rcContent, dtFlags | dt_2ndrowflags);
				}
				else {
					if((rcContent.bottom - rcContent.top) < (2 * m_fontHeight) - 2)
						Gfx::renderText(hdcMem, hTheme, szText, &rcContent, dtFlags | dt_2ndrowflags);
					else {
						DRAWTEXTPARAMS dtp = {0};
						LONG 	rightIconsTop = rcContent.bottom - g_exIconSpacing;
						LONG 	old_right = rcContent.right;
						ULONG 	textCounter = 0;
						ULONG 	ulLen = lstrlen(szText);
						LONG 	old_bottom = rcContent.bottom;
						DWORD 	i_dtFlags = DT_WORDBREAK | DT_NOPREFIX | dt_2ndrowflags;
						dtp.cbSize = sizeof(dtp);
						rcContent.right = clRect->right - dat->rightMargin - rightOffset;
						do {
							if(rcContent.top + (m_fontHeight - 1) > rightIconsTop + 1)
								rcContent.right = old_right;
							dtp.uiLengthDrawn = 0;
							rcContent.bottom = rcContent.top + m_fontHeight - 1;
							if(rcContent.bottom + m_fontHeight >= old_bottom)
								i_dtFlags |= DT_END_ELLIPSIS;
							DrawTextEx(hdcMem, &szText[textCounter], -1, &rcContent, i_dtFlags | DT_CALCRECT, &dtp);
							if(dtp.uiLengthDrawn)
								Gfx::renderText(hdcMem, hTheme, &szText[textCounter], &rcContent, i_dtFlags, 0, dtp.uiLengthDrawn);
							rcContent.top += m_fontHeight;
							textCounter += dtp.uiLengthDrawn;
						} while (textCounter <= ulLen && dtp.uiLengthDrawn && rcContent.top + m_fontHeight <= old_bottom);
					}
				}
			}
		}
	}
	if (fSelected) {
		if (type != CLCIT_DIVIDER) {
			wchar_t *szText = contact->szText;
			RECT rc;
			int qlen = lstrlen(dat->szQuickSearch);
			if(hPreviousFont)
				SelectObject(hdcMem, hPreviousFont);
			Gfx::setTextColor(dat->quickSearchColour);
			if(type == CLCIT_CONTACT) {
				rc.left = rcContent.left;
				rc.top = y + ((rowHeight - m_fontHeight) >> 1);
				rc.right = clRect->right - rightOffset;
				rc.right = rcContent.right;
				rc.bottom = rc.top;
				if(twoRows)
					rc.top = y;
			}
			else {
				rc.left = qLeft;
				rc.top = y + ((rowHeight - m_fontHeight) >> 1);
				rc.right = clRect->right - rightOffset;
				rc.bottom = rc.top;
			}
			if (qlen)
				Gfx::renderText(hdcMem, hTheme, szText, &rc, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE);
		}
	}
	//extra icons
	DWORD spacing = 1;
	for (int i = dat->extraColumnsCount - 1; i >= 0; i--) {
		COLORREF colourFg = dat->selBkColour;
		int mode = ILD_NORMAL;
		if (contact->iExtraImage[i] == 0xffff)
			continue;
		if (fSelected)
			mode = ILD_SELECTED;
		else if (CLC::iHottrackItem) {
			mode = ILD_FOCUS; colourFg = dat->hotTextColour;
		} else if (type == CLCIT_CONTACT && flags & CONTACTF_NOTONLIST) {
			colourFg = dat->fontInfo[FONTID_NOTONLIST].colour; mode = ILD_BLEND50;
		}
		ImageList_DrawEx(dat->himlExtraColumns, contact->iExtraImage[i], hdcMem, clRect->right - rightOffset - dat->extraColumnSpacing * (spacing++), y + ((rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
	}
	if(g_RTL)
		SetLayout(hdcMem, 0);
}

void CLC::Paint(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint)
{
	RGBQUAD*	rgq = 0;
	ClcContact* contact;
	int			ipxWidth = 0;
	HDC 		hdcMem;
	RECT 		clRect;
	ClcGroup*	group;
	HANDLE 		hbp = 0;
	HFONT 		hOldFont = 0;
	DWORD 		style = GetWindowLong(hwnd, GWL_STYLE);
	int 		grey = 0, groupCountsFontTopShift;
	int 		line_num = -1;
	COLORREF 	tmpbkcolour = style & CLS_CONTACTLIST ? (dat->useWindowsColours ? GetSysColor(COLOR_3DFACE) : dat->bkColour) : dat->bkColour;
	BOOL		isFloating = (wndFrameCLC && wndFrameCLC->floating) ? TRUE : FALSE;
	bool		fFocusCheck;

	fInPaint = true;
	g_focusWnd = GetFocus();
	my_status = getGeneralisedStatus();
	g_HDC = hdc;

	/*
	 * temporary DC for avatar drawing
	*/

	g_padding_y = 0;

	hdcTempAV = CreateCompatibleDC(g_HDC);
	hbmTempAV = CreateCompatibleBitmap(g_HDC, g_maxAV_X, g_maxAV_Y);
	hbmTempOldAV = reinterpret_cast<HBITMAP>(SelectObject(hdcTempAV, hbmTempAV));

	cfg::dat.t_now = time(NULL);

	/*
	 * setup static globals used by PaintItem();
	 */
	av_left = 			(cfg::dat.dwFlags & CLUI_FRAME_AVATARSLEFT);
	av_right = 			(cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHT);
	av_rightwithnick = 	(cfg::dat.dwFlags & CLUI_FRAME_AVATARSRIGHTWITHNICK);

	mirror_rtl = 		(cfg::dat.bUseDCMirroring == 2);
	mirror_always = 	(cfg::dat.bUseDCMirroring == 1);
	mirror_rtltext = 	(cfg::dat.bUseDCMirroring == 3);

	g_center = 			cfg::getByte("CLCExt", "EXBK_CenterGroupnames", 0) && !dat->bisEmbedded;
	g_ignoreselforgroups = cfg::getByte("CLC", "IgnoreSelforGroups", 0);
	g_exIconSpacing = 	cfg::dat.exIconScale + 2;

	/*
	 * check if list should be greyed out based on focus or current status
	 */
	if (GetForegroundWindow()!= pcli->hwndContactList && dat->greyoutFlags & GREYF_UNFOCUS)
		grey = 1;
	else if((dat->greyoutFlags & pcli->pfnClcStatusToPf2(my_status)) || style & WS_DISABLED)
		grey = 1;

	GetClientRect(hwnd, &clRect);
	if (rcPaint == NULL)
		rcPaint = &clRect;

#ifndef _USE_D2D
	if (IsRectEmpty(rcPaint)) {
		SelectObject(hdcTempAV, hbmTempOldAV);
		DeleteObject(hbmTempAV);
		DeleteDC(hdcTempAV);
		return;
	}
#endif

#ifdef _USE_D2D
	HBITMAP hbmMem, hbmMemOld;
	if(hwnd == pcli->hwndContactTree) {
		rcPaint = &clRect;
		hdcMem = CreateCompatibleDC(cfg::dat.hdcBg);
		hbmMem = Gfx::createRGBABitmap(clRect.right - clRect.left, clRect.bottom - clRect.top, hdcMem);
		hbmMemOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
	}
	else
		hbp = Gfx::initiateBufferedPaint(hdc, clRect, hdcMem);
#else
	/* explanation for this weird code.. (dBufferDiff etc..)
	 * a bufferend paint dc is normally not the same size as the client rectangle of a
	 * child window (CLC in this case). It's usually much wider. This isn't a problem
	 * for LTR rendering where everything starts at (0,0) but when using SetLayout()
	 * to mirror the DC (for RTL support) it becomes a HUGE problem, because right
	 * becomes left and content will vanish outside the client area.
	 *
	 * dBufferDiff is the difference between the width of the buffered DC and the width
	 * of the actual client rectangle. PaintItem() will add that offset to rcContent.left
	 * when rendering RTL rows to compensate for the difference in width.
	 *
	 */
	dBufferDiff = 0;
	INIT_PAINT(hdc, clRect, hdcMem);
	Api::pfnGetBufferedPaintBits(hbp, &rgq, &ipxWidth);
	dBufferDiff = ipxWidth - clRect.right;
#endif

	{
		TEXTMETRIC tm;
		hOldFont = reinterpret_cast<HFONT>(SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPS].hFont));
		GetTextMetrics(hdcMem, &tm);
		groupCountsFontTopShift = tm.tmAscent;
		SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
		GetTextMetrics(hdcMem, &tm);
		groupCountsFontTopShift -= tm.tmAscent;
	}

	SetBkMode(hdcMem, TRANSPARENT);
	if(!cfg::isAero || dat->bisEmbedded || isFloating) {
		HBRUSH hBrush, hoBrush;
		hBrush = CreateSolidBrush(tmpbkcolour);
		hoBrush = (HBRUSH) SelectObject(hdcMem, hBrush);
		FillRect(hdcMem, rcPaint, hBrush);
		SelectObject(hdcMem, hoBrush);
		DeleteObject(hBrush);
	}

	if(!dat->bisEmbedded && rcPaint && !isFloating)
		Gfx::drawBGFromSurface(hwnd, *rcPaint, hdcMem);

	g_selectiveIcon = (CLC::uNrAvatars > 0) && (cfg::dat.dwFlags & CLUI_FRAME_SELECTIVEICONS) && !dat->bisEmbedded;

	group = &dat->list;
	group->scanIndex = 0;

	CLCPaintHelper* ph = dat->ph;

	fFocusCheck = dat->showSelAlways || dat->exStyle &CLS_EX_SHOWSELALWAYS || g_focusWnd == hwnd;

	// set up the paint helper 
	ph->setHDC(hdcMem);
	ph->aggctx->attach(rgq, ipxWidth, clRect.bottom);
	ph->hTheme = Api::pfnOpenThemeData(hwnd, L"BUTTON");
	ph->style = style;
	ph->clRect = &clRect;
	ph->y = -dat->yScroll;
	ph->indent = 0;
	ph->bFirstNGdrawn = false;
	ph->groupCountsFontTopShift = groupCountsFontTopShift;
	ph->hbp = hbp;

	ph->sevencontact_pos = &Skin::statusItems[ID_EXTBKEVEN_CNTCTPOS];
	ph->soddcontact_pos = &Skin::statusItems[ID_EXTBKODD_CNTCTPOS];
	ph->sfirstitem = &Skin::statusItems[ID_EXTBKFIRSTITEM];
	ph->ssingleitem = &Skin::statusItems[ID_EXTBKSINGLEITEM];
	ph->slastitem = &Skin::statusItems[ID_EXTBKLASTITEM];

	ph->sfirstitem_NG = &Skin::statusItems[ID_EXTBKFIRSTITEM_NG];
	ph->ssingleitem_NG = &Skin::statusItems[ID_EXTBKSINGLEITEM_NG];
	ph->slastitem_NG = &Skin::statusItems[ID_EXTBKLASTITEM_NG];


	hOldFont = ph->changeToFont(FONTID_CONTACTS);

	if ( dat->row_heights == NULL )
		RowHeight::calcRowHeights(dat, hwnd, ph);

	for (ph->index = 0; ph->y < rcPaint->bottom;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			ph->indent--;
			if (group == NULL) {
				break;
			}
			group->scanIndex++;
			continue;
		}

		line_num++;
		contact = group->cl.items[group->scanIndex];
		if(cfg::dat.bForceRefetchOnPaint)
			contact->ace = (struct avatarCacheEntry*)-1;

		if (ph->y > rcPaint->top - dat->row_heights[line_num] && ph->y <= rcPaint->bottom) {
			if (contact->ace == (struct avatarCacheEntry*)-1 )
				contact->ace = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)group->cl.items[group->scanIndex]->hContact, 0);

			ph->fSelected = (ph->index == dat->selection && contact->type != CLCIT_DIVIDER && fFocusCheck);
			iHottrackItem = dat->exStyle & CLS_EX_TRACKSELECT && contact->type == CLCIT_CONTACT && dat->iHotTrack == ph->index;
			if (iHottrackItem == ph->fSelected)
				iHottrackItem = 0;
			if(ph->fSelected && dsp_default.dspOverride[DSP_OVR_SELECTED].fActive)
				ph->dsp = &dsp_default.dspOverride[DSP_OVR_SELECTED];
			else if(iHottrackItem && dsp_default.dspOverride[DSP_OVR_HOVERED].fActive)
				ph->dsp = &dsp_default.dspOverride[DSP_OVR_HOVERED];
			else if(ID_STATUS_OFFLINE == contact->wStatus)
				ph->dsp = &dsp_default.dspOverride[DSP_OVR_OFFLINE];
			else if(ID_STATUS_OFFLINE != contact->wStatus)
				ph->dsp = &dsp_default.dspOverride[DSP_OVR_ONLINE];
			else
				ph->dsp = 0;						// calc dsp profile
			RowHeight::getRowHeight(dat, contact, line_num, style, ph);
			ph->Paint(group, contact, dat->row_heights[line_num]);
		}
		ph->index++;
		ph->y += dat->row_heights[line_num];
		if (contact->type == CLCIT_GROUP && (contact->group->expanded)) {
			group = contact->group;
			ph->indent++;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}

	SelectObject(hdcTempAV, hbmTempOldAV);
	DeleteObject(hbmTempAV);
	DeleteDC(hdcTempAV);
	Api::pfnCloseThemeData(ph->hTheme);

	if (dat->iInsertionMark != -1) {
		//insertion mark
		HBRUSH hBrush, hoBrush;
		POINT pts[8];
		HRGN hRgn;

		pts[0].x=dat->leftMargin; pts[0].y = RowHeight::getItemTopY(dat, dat->iInsertionMark) - dat->yScroll - 4;
		//pts[0]. x = dat->leftMargin; pts[0]. y = dat->iInsertionMark * rowHeight - dat->yScroll - 4;
		pts[1]. x = pts[0].x + 2;      pts[1]. y = pts[0].y + 3;
		pts[2]. x = clRect.right - 4;  pts[2]. y = pts[1].y;
		pts[3]. x = clRect.right - 1;  pts[3]. y = pts[0].y - 1;
		pts[4]. x = pts[3].x;        pts[4]. y = pts[0].y + 7;
		pts[5]. x = pts[2].x + 1;      pts[5]. y = pts[1].y + 2;
		pts[6]. x = pts[1].x;        pts[6]. y = pts[5].y;
		pts[7]. x = pts[0].x;        pts[7]. y = pts[4].y;
		hRgn = CreatePolygonRgn(pts, sizeof(pts) / sizeof(pts[0]), ALTERNATE);
		hBrush = CreateSolidBrush(dat->fontInfo[FONTID_CONTACTS].colour);
		hoBrush = (HBRUSH) SelectObject(hdcMem, hBrush);
		FillRgn(hdcMem, hRgn, hBrush);
		SelectObject(hdcMem, hoBrush);
		DeleteObject(hBrush);
	}
	if(hOldFont)
		SelectObject(hdcMem, hOldFont);

	fInPaint = false;
	if (grey)
		Skin::renderNamedImageItem("@GreyOut", &clRect, hdcMem);

#if defined _USE_D2D
	if(hwnd == pcli->hwndContactTree) {
		MapWindowPoints(pcli->hwndContactTree, pcli->hwndContactList, (POINT*)&clRect, 2);
		Gfx::setBitmapAlpha(hbmMem, 0);
		Api::pfnAlphaBlend(cfg::dat.hdcBg, clRect.left, clRect.top, clRect.right - clRect.left, clRect.bottom - clRect.top,
				hdcMem, 0, 0, clRect.right - clRect.left, clRect.bottom - clRect.top, CLUI::bf);
		SelectObject(hdcMem, hbmMemOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);
		CLUI::updateLayers();
	}
	else
		FINALIZE_PAINT(hbp, &clRect);
#else
	//delete _g;
	FINALIZE_PAINT(hbp, &clRect, isFloating ? 255 : 0);
#endif
}
