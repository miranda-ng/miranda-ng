/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

extern HIMAGELIST himlCListClc;
static BYTE divide3[765] = { 255 };

static void ChangeToFont(HDC hdc, struct ClcData *dat, int id, int *fontHeight)
{
	SelectObject(hdc, dat->fontInfo[id].hFont);
	SetTextColor(hdc, dat->fontInfo[id].colour);
	if (fontHeight)
		*fontHeight = dat->fontInfo[id].fontHeight;
}

static void __inline SetHotTrackColour(HDC hdc, struct ClcData *dat)
{
	if (dat->gammaCorrection) {
		COLORREF oldCol, newCol;
		int oldLum, newLum;

		oldCol = GetTextColor(hdc);
		oldLum = (GetRValue(oldCol) * 30 + GetGValue(oldCol) * 59 + GetBValue(oldCol) * 11) / 100;
		newLum = (GetRValue(dat->hotTextColour) * 30 + GetGValue(dat->hotTextColour) * 59 + GetBValue(dat->hotTextColour) * 11) / 100;
		if (newLum == 0) {
			SetTextColor(hdc, dat->hotTextColour);
			return;
		}
		if (newLum >= oldLum + 20) {
			oldLum += 20;
			newCol =
				RGB(GetRValue(dat->hotTextColour) * oldLum / newLum, GetGValue(dat->hotTextColour) * oldLum / newLum,
				GetBValue(dat->hotTextColour) * oldLum / newLum);
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
		else
			newCol = dat->hotTextColour;
		SetTextColor(hdc, newCol);
	}
	else
		SetTextColor(hdc, dat->hotTextColour);
}

static int GetStatusOnlineness(int status)
{
	switch (status) {
		case ID_STATUS_FREECHAT:    return 110;
		case ID_STATUS_ONLINE:      return 100;
		case ID_STATUS_OCCUPIED:    return 60;
		case ID_STATUS_ONTHEPHONE:  return 50;
		case ID_STATUS_DND:         return 40;
		case ID_STATUS_AWAY:        return 30;
		case ID_STATUS_OUTTOLUNCH:  return 20;
		case ID_STATUS_NA:          return 10;
		case ID_STATUS_INVISIBLE:   return 5;
	}
	return 0;
}

static int GetGeneralisedStatus(void)
{
	int i, status, thisStatus, statusOnlineness, thisOnlineness;

	status = ID_STATUS_OFFLINE;
	statusOnlineness = 0;

	for (i = 0; i < pcli->hClcProtoCount; i++) {
		thisStatus = pcli->clcProto[i].dwStatus;
		if (thisStatus == ID_STATUS_INVISIBLE)
			return ID_STATUS_INVISIBLE;
		thisOnlineness = GetStatusOnlineness(thisStatus);
		if (thisOnlineness > statusOnlineness) {
			status = thisStatus;
			statusOnlineness = thisOnlineness;
		}
	}
	return status;
}

static int GetRealStatus(struct ClcContact *contact, int status)
{
	char *szProto = contact->proto;
	if (!szProto)
		return status;
	for (int i = 0; i < pcli->hClcProtoCount; i++) {
		if (!mir_strcmp(pcli->clcProto[i].szProto, szProto)) {
			return pcli->clcProto[i].dwStatus;
		}
	}
	return status;
}

void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint)
{
	RECT clRect;
	int y, indent, index, fontHeight;
	struct ClcGroup *group;
	HFONT hOldFont;
	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	int status = GetGeneralisedStatus();
	int grey = 0, groupCountsFontTopShift;
	HBRUSH hBrushAlternateGrey = NULL;
	// yes I know about GetSysColorBrush()
	COLORREF tmpbkcolour = style & CLS_CONTACTLIST ? (dat->useWindowsColours ? GetSysColor(COLOR_3DFACE) : dat->bkColour) : dat->bkColour;

	if (dat->greyoutFlags & pcli->pfnClcStatusToPf2(status) || style & WS_DISABLED)
		grey = 1;
	else if (GetFocus() != hwnd && dat->greyoutFlags & GREYF_UNFOCUS)
		grey = 1;
	GetClientRect(hwnd, &clRect);
	if (rcPaint == NULL)
		rcPaint = &clRect;
	if (IsRectEmpty(rcPaint))
		return;
	y = -dat->yScroll;
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hBmpOsb = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), NULL);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBmpOsb);
	{
		TEXTMETRIC tm;
		hOldFont = (HFONT)SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPS].hFont);
		GetTextMetrics(hdcMem, &tm);
		groupCountsFontTopShift = tm.tmAscent;
		SelectObject(hdcMem, dat->fontInfo[FONTID_GROUPCOUNTS].hFont);
		GetTextMetrics(hdcMem, &tm);
		groupCountsFontTopShift -= tm.tmAscent;
	}
	if (style & CLS_GREYALTERNATE)
		hBrushAlternateGrey =
		CreateSolidBrush(GetNearestColor(hdcMem, RGB(GetRValue(tmpbkcolour) - 10, GetGValue(tmpbkcolour) - 10, GetBValue(tmpbkcolour) - 10)));

	ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);
	SetBkMode(hdcMem, TRANSPARENT);
	{
		HBRUSH hBrush = CreateSolidBrush(tmpbkcolour);
		FillRect(hdcMem, rcPaint, hBrush);
		DeleteObject(hBrush);
		if (dat->hBmpBackground) {
			int x, y;
			int destw, desth;

			// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
			SetStretchBltMode(hdcMem, HALFTONE);

			BITMAP bmp;
			GetObject(dat->hBmpBackground, sizeof(bmp), &bmp);
			HDC hdcBmp = CreateCompatibleDC(hdcMem);
			SelectObject(hdcBmp, dat->hBmpBackground);
			y = dat->backgroundBmpUse & CLBF_SCROLL ? -dat->yScroll : 0;
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
			default:       //clb_topleft
				destw = bmp.bmWidth;
				desth = bmp.bmHeight;
				break;
			}
			for (; y < maxy; y += desth) {
				if (y < rcPaint->top - desth)
					continue;
				for (x = 0; x < maxx; x += destw)
					StretchBlt(hdcMem, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
			}
			DeleteDC(hdcBmp);
		}
	}
	group = &dat->list;
	group->scanIndex = 0;
	indent = 0;
	for (index = 0; y < rcPaint->bottom;) {
		if (group->scanIndex == group->cl.count) {
			group = group->parent;
			indent--;
			if (group == NULL)
				break;
			group->scanIndex++;
			continue;
		}
		if (y > rcPaint->top - dat->rowHeight) {
			int iImage = -1;
			int selected = index == dat->selection && (dat->showSelAlways || dat->exStyle & CLS_EX_SHOWSELALWAYS || GetFocus() == hwnd)
				&& group->cl.items[group->scanIndex]->type != CLCIT_DIVIDER;
			int hottrack = dat->exStyle & CLS_EX_TRACKSELECT && group->cl.items[group->scanIndex]->type != CLCIT_DIVIDER && dat->iHotTrack == index;
			SIZE textSize, countsSize, spaceSize;
			int width, checkboxWidth;
			char *szCounts;

			//alternating grey
			if (style & CLS_GREYALTERNATE && index & 1) {
				RECT rc;
				rc.top = y;
				rc.bottom = rc.top + dat->rowHeight;
				rc.left = 0;
				rc.right = clRect.right;
				FillRect(hdcMem, &rc, hBrushAlternateGrey);
			}

			//setup
			if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP)
				ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
			else if (group->cl.items[group->scanIndex]->type == CLCIT_INFO) {
				if (group->cl.items[group->scanIndex]->flags & CLCIIF_GROUPFONT)
					ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
				else
					ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);
			}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_DIVIDER)
				ChangeToFont(hdcMem, dat, FONTID_DIVIDERS, &fontHeight);
			else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags & CONTACTF_NOTONLIST)
				ChangeToFont(hdcMem, dat, FONTID_NOTONLIST, &fontHeight);
			else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT &&
				((group->cl.items[group->scanIndex]->flags & CONTACTF_INVISTO
				&& GetRealStatus(group->cl.items[group->scanIndex], status) != ID_STATUS_INVISIBLE)
				|| (group->cl.items[group->scanIndex]->flags & CONTACTF_VISTO
				&& GetRealStatus(group->cl.items[group->scanIndex], status) == ID_STATUS_INVISIBLE)
				)
				) {
					// the contact is in the always visible list and the proto is invisible
					// the contact is in the always invisible and the proto is in any other mode
					ChangeToFont(hdcMem, dat, group->cl.items[group->scanIndex]->flags & CONTACTF_ONLINE ? FONTID_INVIS : FONTID_OFFINVIS, &fontHeight);
				}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && !(group->cl.items[group->scanIndex]->flags & CONTACTF_ONLINE))
				ChangeToFont(hdcMem, dat, FONTID_OFFLINE, &fontHeight);
			else
				ChangeToFont(hdcMem, dat, FONTID_CONTACTS, &fontHeight);
			GetTextExtentPoint32(hdcMem, group->cl.items[group->scanIndex]->szText, (int)mir_tstrlen(group->cl.items[group->scanIndex]->szText), &textSize);
			width = textSize.cx;
			if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				szCounts = pcli->pfnGetGroupCountsText(dat, group->cl.items[group->scanIndex]);
				if (szCounts[0]) {
					GetTextExtentPoint32A(hdcMem, " ", 1, &spaceSize);
					ChangeToFont(hdcMem, dat, FONTID_GROUPCOUNTS, &fontHeight);
					GetTextExtentPoint32A(hdcMem, szCounts, (int)mir_strlen(szCounts), &countsSize);
					width += spaceSize.cx + countsSize.cx;
				}
			}

			if ((style & CLS_CHECKBOXES && group->cl.items[group->scanIndex]->type == CLCIT_CONTACT) ||
				(style & CLS_GROUPCHECKBOXES && group->cl.items[group->scanIndex]->type == CLCIT_GROUP) ||
				(group->cl.items[group->scanIndex]->type == CLCIT_INFO && group->cl.items[group->scanIndex]->flags & CLCIIF_CHECKBOX))
				checkboxWidth = dat->checkboxSize + 2;
			else
				checkboxWidth = 0;

			//background
			if (selected) {
				int x = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace - 2;
				ImageList_DrawEx(dat->himlHighlight, 0, hdcMem, x, y, min(width + 5, clRect.right - x), dat->rowHeight, CLR_NONE, CLR_NONE,
					dat->exStyle & CLS_EX_NOTRANSLUCENTSEL ? ILD_NORMAL : ILD_BLEND25);
				SetTextColor(hdcMem, dat->selTextColour);
			}
			else if (hottrack)
				SetHotTrackColour(hdcMem, dat);

			//checkboxes
			if (checkboxWidth) {
				RECT rc;
				HANDLE hTheme = OpenThemeData(hwnd, L"BUTTON");
				rc.left = dat->leftMargin + indent * dat->groupIndent;
				rc.right = rc.left + dat->checkboxSize;
				rc.top = y + ((dat->rowHeight - dat->checkboxSize) >> 1);
				rc.bottom = rc.top + dat->checkboxSize;
				DrawThemeBackground(hTheme, hdcMem, BP_CHECKBOX, group->cl.items[group->scanIndex]->flags & CONTACTF_CHECKED ? (hottrack ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL) : (hottrack ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL), &rc, &rc);
				CloseThemeData(hTheme);
			}

			//icon
			if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP)
				iImage = group->cl.items[group->scanIndex]->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
			else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT)
				iImage = group->cl.items[group->scanIndex]->iImage;
			if (iImage != -1) {
				/*COLORREF colourFg=dat->selBkColour;
				int mode=ILD_NORMAL;
				if(selected) mode=ILD_SELECTED;
				else if(hottrack) {mode=ILD_FOCUS; colourFg=dat->hotTextColour;}
				else if(group->cl.items[group->scanIndex]->type==CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags&CONTACTF_NOTONLIST) {colourFg=dat->fontInfo[FONTID_NOTONLIST].colour; mode=ILD_BLEND50;}
				ImageList_DrawEx(himlCListClc,iImage,hdcMem,dat->leftMargin+indent*dat->groupIndent+checkboxWidth,y+((dat->rowHeight-16)>>1),0,0,CLR_NONE,colourFg,mode);
				*/
				// this doesnt use CLS_CONTACTLIST since the colour prolly wont match anyway
				COLORREF colourFg = dat->selBkColour;
				int mode = ILD_NORMAL;
				if (hottrack) {
					colourFg = dat->hotTextColour;
				}
				else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags & CONTACTF_NOTONLIST) {
					colourFg = dat->fontInfo[FONTID_NOTONLIST].colour;
					mode = ILD_BLEND50;
				}
				if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && dat->showIdle
					&& (group->cl.items[group->scanIndex]->flags & CONTACTF_IDLE)
					&& GetRealStatus(group->cl.items[group->scanIndex], ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
					mode = ILD_SELECTED;
				ImageList_DrawEx(himlCListClc, iImage, hdcMem, dat->leftMargin + indent * dat->groupIndent + checkboxWidth,
					y + ((dat->rowHeight - 16) >> 1), 0, 0, CLR_NONE, colourFg, mode);
			}

			//text
			if (group->cl.items[group->scanIndex]->type == CLCIT_DIVIDER) {
				RECT rc;
				rc.top = y + (dat->rowHeight >> 1);
				rc.bottom = rc.top + 2;
				rc.left = dat->leftMargin + indent * dat->groupIndent;
				rc.right = rc.left + ((clRect.right - rc.left - textSize.cx) >> 1) - 3;
				DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
				TextOut(hdcMem, rc.right + 3, y + ((dat->rowHeight - fontHeight) >> 1), group->cl.items[group->scanIndex]->szText,
					(int)mir_tstrlen(group->cl.items[group->scanIndex]->szText));
				rc.left = rc.right + 6 + textSize.cx;
				rc.right = clRect.right;
				DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
			}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				RECT rc;
				if (szCounts[0]) {
					fontHeight = dat->fontInfo[FONTID_GROUPS].fontHeight;
					rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace;
					rc.right = min(clRect.right - countsSize.cx, rc.left + textSize.cx + spaceSize.cx);
					rc.top = y + ((dat->rowHeight - fontHeight) >> 1);
					rc.bottom = rc.top + textSize.cy;
					if (rc.right < rc.left + 4)
						rc.right = clRect.right + 1;
					else
						TextOutA(hdcMem, rc.right, rc.top + groupCountsFontTopShift, szCounts, (int)mir_strlen(szCounts));
					ChangeToFont(hdcMem, dat, FONTID_GROUPS, &fontHeight);
					if (selected)
						SetTextColor(hdcMem, dat->selTextColour);
					else if (hottrack)
						SetHotTrackColour(hdcMem, dat);
					rc.right--;
					ExtTextOut(hdcMem, rc.left, rc.top, ETO_CLIPPED, &rc, group->cl.items[group->scanIndex]->szText,
						(int)mir_tstrlen(group->cl.items[group->scanIndex]->szText), NULL);
				}
				else
					TextOut(hdcMem, dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace,
						y + ((dat->rowHeight - fontHeight) >> 1), group->cl.items[group->scanIndex]->szText,
						(int)mir_tstrlen(group->cl.items[group->scanIndex]->szText));
				if (dat->exStyle & CLS_EX_LINEWITHGROUPS) {
					rc.top = y + (dat->rowHeight >> 1);
					rc.bottom = rc.top + 2;
					rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace + width + 3;
					rc.right = clRect.right - 1 - dat->extraColumnSpacing * dat->extraColumnsCount;
					if (rc.right - rc.left > 1)
						DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
				}
			}
			else {
				TCHAR *szText = group->cl.items[group->scanIndex]->szText;
				RECT rc;
				rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace;
				rc.top = y + ((dat->rowHeight - fontHeight) >> 1);
				rc.right = (clRect.right - clRect.left);
				rc.bottom = rc.top;
				DrawText(hdcMem, szText, -1, &rc, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE);
			}
			if (selected) {
				if (group->cl.items[group->scanIndex]->type != CLCIT_DIVIDER) {
					TCHAR *szText = group->cl.items[group->scanIndex]->szText;
					RECT rc;
					int qlen = (int)mir_tstrlen(dat->szQuickSearch);
					SetTextColor(hdcMem, dat->quickSearchColour);
					rc.left = dat->leftMargin + indent * dat->groupIndent + checkboxWidth + dat->iconXSpace;
					rc.top = y + ((dat->rowHeight - fontHeight) >> 1);
					rc.right = (clRect.right - clRect.left);
					rc.bottom = rc.top;
					if (qlen)
						DrawText(hdcMem, szText, qlen, &rc, DT_EDITCONTROL | DT_NOPREFIX | DT_NOCLIP | DT_WORD_ELLIPSIS | DT_SINGLELINE);
				}
			}

			//extra icons
			for (iImage = 0; iImage < dat->extraColumnsCount; iImage++) {
				COLORREF colourFg = dat->selBkColour;
				int mode = ILD_NORMAL;
				if (group->cl.items[group->scanIndex]->iExtraImage[iImage] == 0xFF)
					continue;
				if (selected)
					mode = ILD_SELECTED;
				else if (hottrack) {
					mode = ILD_FOCUS;
					colourFg = dat->hotTextColour;
				}
				else if (group->cl.items[group->scanIndex]->type == CLCIT_CONTACT && group->cl.items[group->scanIndex]->flags & CONTACTF_NOTONLIST) {
					colourFg = dat->fontInfo[FONTID_NOTONLIST].colour;
					mode = ILD_BLEND50;
				}
				ImageList_DrawEx(dat->himlExtraColumns, group->cl.items[group->scanIndex]->iExtraImage[iImage], hdcMem,
					clRect.right - dat->extraColumnSpacing * (dat->extraColumnsCount - iImage), y + ((dat->rowHeight - 16) >> 1), 0, 0,
					CLR_NONE, colourFg, mode);
			}
		}
		index++;
		y += dat->rowHeight;
		if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP && group->cl.items[group->scanIndex]->group->expanded) {
			group = group->cl.items[group->scanIndex]->group;
			indent++;
			group->scanIndex = 0;
			continue;
		}
		group->scanIndex++;
	}
	if (dat->iInsertionMark != -1) {    //insertion mark
		HBRUSH hBrush;
		POINT pts[8];
		HRGN hRgn;

		pts[0].x = dat->leftMargin;
		pts[0].y = dat->iInsertionMark * dat->rowHeight - dat->yScroll - 4;
		pts[1].x = pts[0].x + 2;
		pts[1].y = pts[0].y + 3;
		pts[2].x = clRect.right - 4;
		pts[2].y = pts[1].y;
		pts[3].x = clRect.right - 1;
		pts[3].y = pts[0].y - 1;
		pts[4].x = pts[3].x;
		pts[4].y = pts[0].y + 7;
		pts[5].x = pts[2].x + 1;
		pts[5].y = pts[1].y + 2;
		pts[6].x = pts[1].x;
		pts[6].y = pts[5].y;
		pts[7].x = pts[0].x;
		pts[7].y = pts[4].y;
		hRgn = CreatePolygonRgn(pts, SIZEOF(pts), ALTERNATE);
		hBrush = CreateSolidBrush(dat->fontInfo[FONTID_CONTACTS].colour);
		FillRgn(hdcMem, hRgn, hBrush);
		DeleteObject(hBrush);
		DeleteObject(hRgn);
	}
	if (!grey)
		BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, hdcMem, rcPaint->left, rcPaint->top,
		SRCCOPY);
	SelectObject(hdcMem,hOldBitmap);
	SelectObject(hdcMem,hOldFont);
	DeleteDC(hdcMem);
	if (hBrushAlternateGrey)
		DeleteObject(hBrushAlternateGrey);
	if (grey) {
		BITMAPINFOHEADER bmih = { 0 };
		bmih.biBitCount = 32;
		bmih.biSize = sizeof(bmih);
		bmih.biCompression = BI_RGB;
		bmih.biHeight = -clRect.bottom;
		bmih.biPlanes = 1;
		bmih.biWidth = clRect.right;
		PBYTE bits = (PBYTE) malloc(4 * bmih.biWidth * -bmih.biHeight);
		GetDIBits(hdc, hBmpOsb, 0, clRect.bottom, bits, (BITMAPINFO *) & bmih, DIB_RGB_COLORS);
		COLORREF greyColour = GetSysColor(COLOR_3DFACE);
		int greyRed = GetRValue(greyColour) * 2;
		int greyGreen = GetGValue(greyColour) * 2;
		int greyBlue = GetBValue(greyColour) * 2;
		if (divide3[0] == 255) {
			for (int i = 0; i < SIZEOF(divide3); i++)
				divide3[i] = (i + 1) / 3;
		}
		for (int i = 4 * clRect.right * clRect.bottom - 4; i >= 0; i -= 4) {
			bits[i] = divide3[bits[i] + greyBlue];
			bits[i + 1] = divide3[bits[i + 1] + greyGreen];
			bits[i + 2] = divide3[bits[i + 2] + greyRed];
		}
		SetDIBitsToDevice(hdc, 0, 0, clRect.right, clRect.bottom, 0, 0, 0, clRect.bottom, bits, (BITMAPINFO *) & bmih, DIB_RGB_COLORS);
		free(bits);
	}
	DeleteObject(hBmpOsb);
}
