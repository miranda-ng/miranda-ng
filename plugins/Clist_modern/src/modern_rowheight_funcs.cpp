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

Created by Pescuma, modified by Artem Shpynov
*/

#include "stdafx.h"
#include "modern_clcpaint.h"

ROWCELL *gl_RowTabAccess[TC_ELEMENTSCOUNT + 1] = {};	// Массив, через который осуществляется доступ к элементам контакта.
ROWCELL *gl_RowRoot;

SIZE GetAvatarSize(int imageWidth, int imageHeight, int maxWidth, int maxHeight)
{
	float scalefactor = 0;
	SIZE sz = { 0 };
	if (imageWidth == 0 || imageHeight == 0)
		return sz;

	if (maxWidth == 0)
		maxWidth = maxHeight;
	scalefactor = min((float)maxWidth / imageWidth, (float)maxHeight / imageHeight);
	sz.cx = (LONG)(imageWidth*scalefactor);
	sz.cy = (LONG)(imageHeight*scalefactor);
	return sz;
}

int RowHeight_CalcRowHeight(ClcData *dat, ClcContact *contact, int item)
{
	if (!RowHeights_Alloc(dat, item + 1))
		return -1;

	if (!g_clistApi.hwndContactTree)
		return 0;

	ClcCacheEntry *pdnce = contact->pce;
	if (dat->hWnd != g_clistApi.hwndContactTree || !gl_RowRoot || contact->type == CLCIT_GROUP) {
		int tmp = dat->fontModernInfo[g_clcPainter.GetBasicFontID(contact)].fontHeight;
		if (dat->text_replace_smileys && dat->first_line_draw_smileys && !dat->text_resize_smileys)
			tmp = max(tmp, contact->ssText.iMaxSmileyHeight);
		if (contact->type == CLCIT_GROUP) {
			wchar_t *szCounts = Clist_GetGroupCountsText(dat, contact);
			// Has the count?
			if (szCounts && szCounts[0])
				tmp = max(tmp, dat->fontModernInfo[contact->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS].fontHeight);
		}
		tmp = max(tmp, ICON_HEIGHT);
		tmp = max(tmp, dat->row_min_heigh);
		tmp += dat->row_border * 2;
		if (contact->type == CLCIT_GROUP && contact->group->parent->groupId == 0 && contact->group->parent->cl[0] != contact)
			tmp += dat->row_before_group_space;
		if (item != -1)
			dat->row_heights[item] = tmp;
		return tmp;
	}

	bool hasAvatar = contact->avatar_data != nullptr;
	for (int i = 0;; i++) {
		ROWCELL *pCell = gl_RowTabAccess[i];
		if (pCell == nullptr)
			break;

		if (pCell->type != TC_SPACE) {
			pCell->h = 0;
			pCell->w = 0;
			SetRect(&pCell->r, 0, 0, 0, 0);
			switch (pCell->type) {
			case TC_TEXT1:
				pCell->h = dat->fontModernInfo[g_clcPainter.GetBasicFontID(contact)].fontHeight;
				if (dat->text_replace_smileys && dat->first_line_draw_smileys && !dat->text_resize_smileys)
					pCell->h = max(pCell->h, contact->ssText.iMaxSmileyHeight);
				if (item == -1) {
					// calculate text width here
					SIZE size = { 0 };
					RECT dummyRect = { 0, 0, 1024, pCell->h };
					HDC hdc = CreateCompatibleDC(nullptr);
					g_clcPainter.ChangeToFont(hdc, dat, g_clcPainter.GetBasicFontID(contact), nullptr);
					g_clcPainter.GetTextSize(&size, hdc, dummyRect, contact->szText, contact->ssText.plText, 0, dat->text_resize_smileys ? 0 : contact->ssText.iMaxSmileyHeight);
					if (contact->type == CLCIT_GROUP) {
						wchar_t *szCounts = Clist_GetGroupCountsText(dat, contact);
						if (szCounts && mir_wstrlen(szCounts) > 0) {
							RECT count_rc = { 0 };
							// calc width and height
							g_clcPainter.ChangeToFont(hdc, dat, contact->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
							ske_DrawText(hdc, L" ", 1, &count_rc, DT_CALCRECT | DT_NOPREFIX);
							size.cx += count_rc.right - count_rc.left;
							count_rc.right = 0;
							count_rc.left = 0;
							ske_DrawText(hdc, szCounts, (int)mir_wstrlen(szCounts), &count_rc, DT_CALCRECT);
							size.cx += count_rc.right - count_rc.left;
							pCell->h = max(pCell->h, count_rc.bottom - count_rc.top);
						}
					}
					pCell->w = size.cx;
					SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
					ske_ResetTextEffect(hdc);
					DeleteDC(hdc);
				}
				break;

			case TC_TEXT2:
				if (g_plugin.secondLine.bActive && pdnce->szSecondLineText && pdnce->szSecondLineText[0]) {
					pCell->h = dat->fontModernInfo[FONTID_SECONDLINE].fontHeight;
					if (dat->text_replace_smileys && g_plugin.secondLine.bDrawSmilies && !dat->text_resize_smileys)
						pCell->h = max(pCell->h, pdnce->ssSecondLine.iMaxSmileyHeight);
					if (item == -1) {
						// calculate text width here
						SIZE size = { 0 };
						RECT dummyRect = { 0, 0, 1024, pCell->h };
						HDC hdc = CreateCompatibleDC(nullptr);
						g_clcPainter.ChangeToFont(hdc, dat, FONTID_SECONDLINE, nullptr);
						g_clcPainter.GetTextSize(&size, hdc, dummyRect, pdnce->szSecondLineText, pdnce->ssSecondLine.plText, 0, dat->text_resize_smileys ? 0 : pdnce->ssSecondLine.iMaxSmileyHeight);
						pCell->w = size.cx;
						SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
						ske_ResetTextEffect(hdc);
						DeleteDC(hdc);
					}
				}
				break;

			case TC_TEXT3:
				if (g_plugin.thirdLine.bActive && pdnce->szThirdLineText && pdnce->szThirdLineText[0]) {
					pCell->h = dat->fontModernInfo[FONTID_THIRDLINE].fontHeight;
					if (dat->text_replace_smileys && g_plugin.thirdLine.bDrawSmilies && !dat->text_resize_smileys)
						pCell->h = max(pCell->h, pdnce->ssThirdLine.iMaxSmileyHeight);
					if (item == -1) {
						//calculate text width here
						SIZE size = { 0 };
						RECT dummyRect = { 0, 0, 1024, pCell->h };
						HDC hdc = CreateCompatibleDC(nullptr);
						g_clcPainter.ChangeToFont(hdc, dat, FONTID_THIRDLINE, nullptr);
						g_clcPainter.GetTextSize(&size, hdc, dummyRect, pdnce->szThirdLineText, pdnce->ssThirdLine.plText, 0, dat->text_resize_smileys ? 0 : pdnce->ssThirdLine.iMaxSmileyHeight);
						pCell->w = size.cx;
						SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
						ske_ResetTextEffect(hdc);
						DeleteDC(hdc);
					}
				}
				break;

			case TC_STATUS:
				if ((contact->type == CLCIT_GROUP && !dat->row_hide_group_icon) ||
					(contact->type == CLCIT_CONTACT && contact->iImage != -1 &&
					!(dat->icon_hide_on_avatar && dat->avatars_show && (hasAvatar || (!hasAvatar && dat->icon_draw_on_avatar_space && contact->iImage != -1)) && !contact->bImageIsSpecial))) {
					pCell->h = ICON_HEIGHT;
					pCell->w = ICON_HEIGHT;
				}
				break;

			case TC_AVATAR:
				if (dat->avatars_show &&
					contact->type == CLCIT_CONTACT &&
					(hasAvatar || (dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space && contact->iImage != -1))) {
					int iW = 0, iH = 0;
					if (contact->avatar_data) {
						iH = contact->avatar_data->bmHeight;
						iW = contact->avatar_data->bmWidth;
					}

					SIZE sz = GetAvatarSize(iW, iH, dat->avatars_maxwidth_size, dat->avatars_maxheight_size);
					if ((sz.cx == 0 || sz.cy == 0) && dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space && contact->iImage != -1)
						sz.cx = ICON_HEIGHT, sz.cy = ICON_HEIGHT;

					pCell->h = sz.cy;
					pCell->w = sz.cx;
				}
				break;

			case TC_EXTRA: // Draw extra icons
				if (contact->type == CLCIT_CONTACT &&
					(!contact->iSubNumber || db_get_b(0, "CLC", "MetaHideExtra", SETTING_METAHIDEEXTRA_DEFAULT) == 0 && dat->extraColumnsCount > 0)) {
					bool hasExtra = false;
					int width = 0;
					for (int k = 0; k < dat->extraColumnsCount; k++)
						if (contact->iExtraImage[k] != EMPTY_EXTRA_ICON || !dat->bMetaIgnoreEmptyExtra) {
							hasExtra = true;
							if (item != -1) break;
							width += (width > 0) ? dat->extraColumnSpacing : (dat->extraColumnSpacing - 2);
						}
					if (hasExtra) {
						pCell->h = ICON_HEIGHT;
						pCell->w = width;
					}
				}
				break;

			case TC_EXTRA1:
			case TC_EXTRA2:
			case TC_EXTRA3:
			case TC_EXTRA4:
			case TC_EXTRA5:
			case TC_EXTRA6:
			case TC_EXTRA7:
			case TC_EXTRA8:
			case TC_EXTRA9:
				if (contact->type == CLCIT_CONTACT &&
					(!contact->iSubNumber || db_get_b(0, "CLC", "MetaHideExtra", SETTING_METAHIDEEXTRA_DEFAULT) == 0 && dat->extraColumnsCount > 0)) {
					int eNum = pCell->type - TC_EXTRA1;
					if (eNum < dat->extraColumnsCount)
						if (contact->iExtraImage[eNum] != EMPTY_EXTRA_ICON || !dat->bMetaIgnoreEmptyExtra) {
							pCell->h = ICON_HEIGHT;
							pCell->w = ICON_HEIGHT;
						}
				}
				break;

			case TC_TIME:
				if (contact->type == CLCIT_CONTACT && dat->contact_time_show && pdnce->hTimeZone) {
					pCell->h = dat->fontModernInfo[FONTID_CONTACT_TIME].fontHeight;
					if (item == -1) {
						wchar_t szResult[80];

						if (!TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", szResult, _countof(szResult), 0)) {
							SIZE text_size = { 0 };
							RECT rc = { 0 };
							// Select font
							HDC hdc = CreateCompatibleDC(nullptr);
							g_clcPainter.ChangeToFont(hdc, dat, FONTID_CONTACT_TIME, nullptr);

							// Get text size
							text_size.cy = ske_DrawText(hdc, szResult, (int)mir_wstrlen(szResult), &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
							SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
							ske_ResetTextEffect(hdc);
							DeleteDC(hdc);
							text_size.cx = rc.right - rc.left;
							pCell->w = text_size.cx;
						}
					}
				}
				break;
			}
		}
	}

	int height = dat->row_border * 2;
	height = max(height, dat->row_min_heigh);
	if (item != -1)
		dat->row_heights[item] = height;
	return height;
}

BOOL RowHeights_Initialize(struct ClcData *dat)
{
	dat->rowHeight = 0;
	dat->row_heights_size = 0;
	dat->row_heights_allocated = 0;
	dat->row_heights = nullptr;
	return TRUE;
}

void RowHeights_Free(ClcData *dat)
{
	if (dat->row_heights != nullptr) {
		free(dat->row_heights);
		dat->row_heights = nullptr;
	}

	dat->row_heights_allocated = 0;
	dat->row_heights_size = 0;
}

void RowHeights_Clear(ClcData *dat)
{
	dat->row_heights_size = 0;
}

BOOL RowHeights_Alloc(ClcData *dat, int size)
{
	if (size > dat->row_heights_size) {
		if (size > dat->row_heights_allocated) {
			int size_grow = size;

			size_grow += 100 - (size_grow % 100);

			if (dat->row_heights != nullptr) {
				int *tmp = (int *)realloc((void *)dat->row_heights, sizeof(int)* size_grow);
				if (tmp == nullptr) {
					TRACE("Out of memory: realloc returned nullptr (RowHeights_Alloc)");
					RowHeights_Free(dat);
					return FALSE;
				}

				dat->row_heights = tmp;
				memset(dat->row_heights + (dat->row_heights_allocated), 0, sizeof(int)* (size_grow - dat->row_heights_allocated));
			}
			else {
				dat->row_heights = (int *)malloc(sizeof(int)* size_grow);
				if (dat->row_heights == nullptr) {
					TRACE("Out of memory: alloc returned nullptr (RowHeights_Alloc)");
					RowHeights_Free(dat);
					return FALSE;
				}
				memset(dat->row_heights, 0, sizeof(int)* size_grow);
			}
			dat->row_heights_allocated = size_grow;
		}
		dat->row_heights_size = size;
	}

	return TRUE;
}

// Calc and store max row height

static int contact_fonts[] = {
	FONTID_CONTACTS, FONTID_INVIS, FONTID_OFFLINE, FONTID_NOTONLIST, FONTID_OFFINVIS,
	FONTID_AWAY, FONTID_DND, FONTID_NA, FONTID_OCCUPIED, FONTID_CHAT, FONTID_INVISIBLE };

static int other_fonts[] = { FONTID_OPENGROUPS, FONTID_OPENGROUPCOUNTS, FONTID_CLOSEDGROUPS, FONTID_CLOSEDGROUPCOUNTS, FONTID_DIVIDERS, FONTID_CONTACT_TIME };

int RowHeights_GetMaxRowHeight(ClcData *dat, HWND hwnd)
{
	int max_height = 0;
	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);

	if (!dat->text_ignore_size_for_row_height) {
		// Get contact font size
		int tmp = 0;
		for (auto &it : contact_fonts)
			if (tmp < dat->fontModernInfo[it].fontHeight)
				tmp = dat->fontModernInfo[it].fontHeight;

		if (dat->text_replace_smileys && dat->first_line_draw_smileys && !dat->text_resize_smileys)
			tmp = max(tmp, dat->text_smiley_height);

		max_height += tmp;

		if (g_plugin.secondLine.bActive) {
			tmp = dat->fontModernInfo[FONTID_SECONDLINE].fontHeight;
			if (dat->text_replace_smileys && g_plugin.secondLine.bDrawSmilies && !dat->text_resize_smileys)
				tmp = max(tmp, dat->text_smiley_height);
			max_height += g_plugin.secondLine.iTopSpace + tmp;
		}

		if (g_plugin.thirdLine.bActive) {
			tmp = dat->fontModernInfo[FONTID_THIRDLINE].fontHeight;
			if (dat->text_replace_smileys && g_plugin.thirdLine.bDrawSmilies && !dat->text_resize_smileys)
				tmp = max(tmp, dat->text_smiley_height);
			max_height += g_plugin.thirdLine.iTopSpace + tmp;
		}

		// Get other font sizes
		for (auto &it : other_fonts)
			if (max_height < dat->fontModernInfo[it].fontHeight)
				max_height = dat->fontModernInfo[it].fontHeight;
	}

	// Avatar size
	if (dat->avatars_show && !dat->avatars_ignore_size_for_row_height)
		max_height = max(max_height, dat->avatars_maxheight_size);

	// Checkbox size
	if (style & CLS_CHECKBOXES || style & CLS_GROUPCHECKBOXES)
		max_height = max(max_height, dat->checkboxSize);

	// Icon size
	if (!dat->icon_ignore_size_for_row_height)
		max_height = max(max_height, ICON_HEIGHT);

	max_height += 2 * dat->row_border;

	// Min size
	max_height = max(max_height, dat->row_min_heigh);

	dat->rowHeight = max_height;
	return max_height;
}

// Calc and store row height for all items in the list
void RowHeights_CalcRowHeights(ClcData *dat, HWND hwnd)
{
	if (MirandaExiting()) return;

	// Draw lines
	ClcGroup *group = &dat->list;
	group->scanIndex = 0;
	int indent = 0;
	int subindex = -1;
	int line_num = -1;

	RowHeights_Clear(dat);

	while (true) {
		int subident;
		ClcContact *Drawing;
		if (subindex == -1) {
			if (group->scanIndex == group->cl.getCount()) {
				if ((group = group->parent) == nullptr)
					break;
				group->scanIndex++;
				indent--;
				continue;
			}

			// Get item to draw
			Drawing = group->cl[group->scanIndex];
			subident = 0;
		}
		else {
			// Get item to draw
			Drawing = &group->cl[group->scanIndex]->subcontacts[subindex];
			subident = dat->subIndent;
		}

		line_num++;

		// Calc row height
		if (!gl_RowRoot)
			RowHeights_GetRowHeight(dat, hwnd, Drawing, line_num);
		else
			RowHeight_CalcRowHeight(dat, Drawing, line_num);

		// increment by subcontacts
		if (group->cl[group->scanIndex]->subcontacts != nullptr && group->cl[group->scanIndex]->type != CLCIT_GROUP) {
			if (group->cl[group->scanIndex]->bSubExpanded && dat->bMetaExpanding) {
				if (subindex < group->cl[group->scanIndex]->iSubAllocated - 1)
					subindex++;
				else
					subindex = -1;
			}
		}

		if (subindex == -1) {
			if (group->cl[group->scanIndex]->type == CLCIT_GROUP && group->cl[group->scanIndex]->group->expanded) {
				group = group->cl[group->scanIndex]->group;
				indent++;
				group->scanIndex = 0;
				subindex = -1;
				continue;
			}
			group->scanIndex++;
		}
	}
}

// Calc and store row height
int RowHeights_GetRowHeight(ClcData *dat, HWND hwnd, ClcContact *contact, int item)
{
	if (!dat->row_variable_height)
		return dat->rowHeight;

	uint32_t style = GetWindowLongPtr(hwnd, GWL_STYLE);
	//TODO replace futher code with new rowheight definition
	BOOL selected = ((item == dat->selection) && (dat->hwndRenameEdit != nullptr || dat->bShowSelAlways || (dat->exStyle & CLS_EX_SHOWSELALWAYS) || g_clcPainter.IsForegroundWindow(hwnd)) && contact->type != CLCIT_DIVIDER);
	BOOL minimalistic = (g_clcPainter.CheckMiniMode(dat, selected));

	if (!RowHeights_Alloc(dat, item + 1))
		return -1;

	int height = 0;
	ClcCacheEntry *pdnce = contact->pce;

	if (!dat->text_ignore_size_for_row_height) {
		int tmp = dat->fontModernInfo[g_clcPainter.GetBasicFontID(contact)].fontHeight;
		if (dat->text_replace_smileys && dat->first_line_draw_smileys && !dat->text_resize_smileys)
			tmp = max(tmp, contact->ssText.iMaxSmileyHeight);
		height += tmp;

		if (pdnce && !minimalistic) {
			if (g_plugin.secondLine.bActive && pdnce->szSecondLineText && pdnce->szSecondLineText[0]) {
				tmp = dat->fontModernInfo[FONTID_SECONDLINE].fontHeight;
				if (dat->text_replace_smileys && g_plugin.secondLine.bDrawSmilies && !dat->text_resize_smileys)
					tmp = max(tmp, pdnce->ssSecondLine.iMaxSmileyHeight);
				height += g_plugin.secondLine.iTopSpace + tmp;
			}

			if (g_plugin.thirdLine.bActive && pdnce->szThirdLineText && pdnce->szThirdLineText[0]) {
				tmp = dat->fontModernInfo[FONTID_THIRDLINE].fontHeight;
				if (dat->text_replace_smileys && g_plugin.thirdLine.bDrawSmilies && !dat->text_resize_smileys)
					tmp = max(tmp, pdnce->ssThirdLine.iMaxSmileyHeight);
				height += g_plugin.thirdLine.iTopSpace + tmp;
			}
		}
	}

	// Avatar size
	if (dat->avatars_show && !dat->avatars_ignore_size_for_row_height && contact->type == CLCIT_CONTACT && contact->avatar_data != nullptr && !minimalistic)
		height = max(height, dat->avatars_maxheight_size);

	// Checkbox size
	if (contact->isCheckBox(style))
		height = max(height, dat->checkboxSize);

	// Icon size
	if (!dat->icon_ignore_size_for_row_height) {
		if (contact->type == CLCIT_GROUP ||
			(contact->type == CLCIT_CONTACT && contact->iImage != -1 && !(dat->icon_hide_on_avatar && dat->avatars_show && contact->avatar_data != nullptr && !contact->bImageIsSpecial))) {
			height = max(height, ICON_HEIGHT);
		}
	}

	height += 2 * dat->row_border;

	// Min size
	return dat->row_heights[item] = max(height, dat->row_min_heigh);
}

// Calc item top Y (using stored data)
int cliGetRowTopY(ClcData *dat, int item)
{
	if (!dat->row_variable_height)
		return item * dat->rowHeight;

	if (item >= dat->row_heights_size)
		return cliGetRowBottomY(dat, item - 1);

	int y = 0;
	for (int i = 0; i < item; i++)
		y += dat->row_heights[i];
	return y;
}

// Calc item bottom Y (using stored data)
int cliGetRowBottomY(ClcData *dat, int item)
{
	if (!dat->row_variable_height)
		return (item + 1) * dat->rowHeight;

	if (item >= dat->row_heights_size)
		return -1;

	int y = 0;
	for (int i = 0; i <= item; i++)
		y += dat->row_heights[i];
	return y;
}


// Calc total height of rows (using stored data)
int cliGetRowTotalHeight(ClcData *dat)
{
	if (!dat->row_variable_height)
		return corecli.pfnGetRowTotalHeight(dat);

	int y = 0;
	for (int i = 0; i < dat->row_heights_size; i++)
		y += dat->row_heights[i];

	return y;
}

// Return the line that pos_y is at or -1 (using stored data)
int cliRowHitTest(ClcData *dat, int pos_y)
{
	if (pos_y < 0)
		return -1;

	if (!dat->row_variable_height && dat->rowHeight)
		return pos_y / dat->rowHeight;

	int y = 0;
	for (int i = 0; i < dat->row_heights_size; i++) {
		y += dat->row_heights[i];
		if (pos_y < y)
			return i;
	}

	return -1;
}

int cliGetRowHeight(ClcData *dat, int item)
{
	if (!dat->row_variable_height || item >= dat->row_heights_size || item < 0)
		return dat->rowHeight;

	return dat->row_heights[item];
}
