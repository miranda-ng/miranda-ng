/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "modern_clcpaint.h"

CLCPaint g_clcPainter;

uint32_t CLCPaint::HASH[hi_LastItem] = { 0 };
const char* CLCPaint::HASHTEXT[hi_LastItem] = {
	"Module",
	"ID",
	"Type",
	"Open",
	"IsEmpty",
	"SubPos",
	"Protocol",
	"RootGroup",
	"Status",
	"HasAvatar",
	"GroupPos",
	"Selected",
	"Hot",
	"Odd",
	"Indent",
	"Index",
	"Name",
	"Group",
	"True",
	"False",
	"ONLINE",
	"AWAY",
	"DND",
	"NA",
	"OCCUPIED",
	"FREECHAT",
	"INVISIBLE",
	"IDLE",
	"OFFLINE",
	"Row",
	"CL",
	"SubContact",
	"MetaContact",
	"Contact",
	"Divider",
	"Info",
	"First-Single",
	"First",
	"Middle",
	"Mid",
	"Single",
	"Last",
	"Rate",
	"None",
	"Low",
	"Medium",
	"High",
	"State",
	"Active",
	"Inactive"
	//ADD item here
};
const int CLCPaint::HORIZONTAL_SPACE = 2;
const int CLCPaint::EXTRA_CHECKBOX_SPACE = 2;
const int CLCPaint::EXTRA_SPACE = 2;
const int CLCPaint::SELECTION_BORDER = 6;
const int CLCPaint::MIN_TEXT_WIDTH = 20;
const int CLCPaint::BUF2SIZE = 7;

const uint8_t CLCPaint::GIM_SELECTED_AFFECT = 1;
const uint8_t CLCPaint::GIM_HOT_AFFECT = 2;
const uint8_t CLCPaint::GIM_TEMP_AFFECT = 4;
const uint8_t CLCPaint::GIM_IDLE_AFFECT = 8;
const uint8_t CLCPaint::GIM_EXTRAICON_AFFECT = CLCPaint::GIM_SELECTED_AFFECT | CLCPaint::GIM_HOT_AFFECT | CLCPaint::GIM_IDLE_AFFECT | CLCPaint::GIM_TEMP_AFFECT;
const uint8_t CLCPaint::GIM_STATUSICON_AFFECT = CLCPaint::GIM_IDLE_AFFECT | CLCPaint::GIM_TEMP_AFFECT;
const uint8_t CLCPaint::GIM_AVATAR_AFFECT = CLCPaint::GIM_IDLE_AFFECT | CLCPaint::GIM_TEMP_AFFECT;

CLCPaint::CLCPaint()
{
	_FillQuickHash();
};

void CLCPaint::cliPaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *rcPaint)
{
	if (MirandaExiting()) return;
	g_CluiData.mutexPaintLock++;
	g_clcPainter._PaintClc(hwnd, dat, hdc, rcPaint);
	g_CluiData.mutexPaintLock--;
}

BOOL  CLCPaint::IsForegroundWindow(HWND hWnd)
{
	HWND hWindow;
	hWindow = hWnd;
	while (hWindow) {
		if (GetForegroundWindow() == hWindow) return TRUE;
		hWindow = GetParent(hWindow);
	}
	return FALSE;
}

HFONT CLCPaint::ChangeToFont(HDC hdc, ClcData *dat, int id, int *fontHeight)
{
	if (!dat)
		dat = (ClcData*)GetWindowLongPtr(g_clistApi.hwndContactTree, 0);

	if (!dat)
		return nullptr;

	HFONT res = (HFONT)SelectObject(hdc, dat->fontModernInfo[id].hFont);
	SetTextColor(hdc, dat->fontModernInfo[id].colour);

	if (fontHeight)
		*fontHeight = dat->fontModernInfo[id].fontHeight;
	ske_ResetTextEffect(hdc);

	if (dat->hWnd == g_clistApi.hwndContactTree && dat->fontModernInfo[id].effect != 0)
		ske_SelectTextEffect(hdc, dat->fontModernInfo[id].effect - 1, dat->fontModernInfo[id].effectColour1, dat->fontModernInfo[id].effectColour2);
	else
		ske_ResetTextEffect(hdc);

	return res;
}

int CLCPaint::GetBasicFontID(ClcContact *contact)
{
	switch (contact->type) {
	case CLCIT_GROUP:
		return (contact->group->expanded) ? FONTID_OPENGROUPS : FONTID_CLOSEDGROUPS;

	case CLCIT_INFO:
		return (contact->flags & CLCIIF_GROUPFONT) ? FONTID_OPENGROUPS : FONTID_CONTACTS;

	case CLCIT_DIVIDER:
		return FONTID_DIVIDERS;

	case CLCIT_CONTACT:
		if (contact->flags & CONTACTF_NOTONLIST)
			return FONTID_NOTONLIST;
		if (((contact->flags & CONTACTF_INVISTO) && Clist_GetRealStatus(contact, ID_STATUS_OFFLINE) != ID_STATUS_INVISIBLE) ||
			((contact->flags & CONTACTF_VISTO && Clist_GetRealStatus(contact, ID_STATUS_OFFLINE) == ID_STATUS_INVISIBLE))) {
			// the contact is in the always visible list and the proto is invisible
			// the contact is in the always invisible and the proto is in any other mode
			return (contact->flags & CONTACTF_ONLINE) ? FONTID_INVIS : FONTID_OFFINVIS;
		}

		switch (contact->pce->getStatus()) {
		case ID_STATUS_OFFLINE: return FONTID_OFFLINE;
		case ID_STATUS_AWAY: return FONTID_AWAY;
		case ID_STATUS_DND: return FONTID_DND;
		case ID_STATUS_NA: return FONTID_NA;
		case ID_STATUS_OCCUPIED: return FONTID_OCCUPIED;
		case ID_STATUS_FREECHAT: return FONTID_CHAT;
		case ID_STATUS_INVISIBLE: return FONTID_INVISIBLE;
		}

	default:
		return FONTID_CONTACTS;
	}
}

void CLCPaint::GetTextSize(SIZE *text_size, HDC hdcMem, RECT free_row_rc, wchar_t *szText, SortedList *plText, UINT uTextFormat, int smiley_height)
{
	if (szText == nullptr || !szText[0]) {
		text_size->cy = 0;
		text_size->cx = 0;
	}
	else {
		RECT text_rc = free_row_rc;
		int free_width;
		int free_height;

		free_width = text_rc.right - text_rc.left;
		free_height = text_rc.bottom - text_rc.top;

		// Always need cy...
		text_size->cy = ske_DrawText(hdcMem, szText, (int)mir_wstrlen(szText), &text_rc, DT_CALCRECT | uTextFormat);
		text_size->cy = min(text_size->cy, free_height);
		if (plText == nullptr)
			text_size->cx = min(text_rc.right - text_rc.left + 2, free_width);
		else {
			// See each item of list
			int i;

			text_size->cy = min(max(text_size->cy, smiley_height), free_height);

			text_size->cx = 0;

			for (i = 0; i < plText->realCount && text_size->cx < free_width; i++) {
				ClcContactTextPiece *piece = (ClcContactTextPiece *)plText->items[i];

				if (piece->type == TEXT_PIECE_TYPE_TEXT) {
					text_rc = free_row_rc;

					ske_DrawText(hdcMem, &szText[piece->start_pos], piece->len, &text_rc, DT_CALCRECT | uTextFormat);
					text_size->cx = min(text_size->cx + text_rc.right - text_rc.left + 2, free_width);
				}
				else {
					double factor;

					if (piece->smiley_height > text_size->cy)
						factor = text_size->cy / (double)piece->smiley_height;
					else
						factor = 1;

					text_size->cx = min(text_size->cx + (long)(factor * piece->smiley_width), free_width);
				}
			}
		}
	}
}

void CLCPaint::AddParam(MODERNMASK *mpModernMask, uint32_t dwParamHash, const char *const szValue, uint32_t dwValueHash)
{
	static MASKPARAM param = { 0 }; //AddParameter will clear it so it can be static to avoid initializations
	_FillParam(&param, dwParamHash, szValue, dwValueHash);
	_AddParameter(mpModernMask, &param);
}

BOOL  CLCPaint::CheckMiniMode(ClcData *dat, BOOL selected)
{
	if (!dat->bCompactMode || ((dat->bCompactMode & 0x01) && selected))
		return FALSE;
	return TRUE;
}

tPaintCallbackProc CLCPaint::PaintCallbackProc(HWND hWnd, HDC hDC, RECT *rcPaint, HRGN, uint32_t, void *)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(hWnd, 0);
	if (dat)
		cliPaintClc(hWnd, dat, hDC, rcPaint);
	return nullptr;
}

void CLCPaint::_FillQuickHash()
{
	for (int i = 0; i < hi_LastItem; i++)
		HASH[i] = mod_CalcHash(HASHTEXT[i]);
}

void CLCPaint::_SetHotTrackColour(HDC hdc, ClcData *dat)
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
			newCol = RGB(GetRValue(dat->hotTextColour)*oldLum / newLum, GetGValue(dat->hotTextColour)*oldLum / newLum, GetBValue(dat->hotTextColour)*oldLum / newLum);
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
				if (r > 255) r = 255;
				b += (g - 255) * 59 / 41;
				g = 255;
			}
			if (b > 255) {
				r += (b - 255) * 11 / 89;
				if (r > 255) r = 255;
				g += (b - 255) * 11 / 89;
				if (g > 255) g = 255;
				b = 255;
			}
			newCol = RGB(r, g, b);
		}
		else newCol = dat->hotTextColour;
		SetTextColor(hdc, newCol);
	}
	else
		SetTextColor(hdc, dat->hotTextColour);
}

RECT  CLCPaint::_GetRectangle(ClcData *dat, RECT *row_rc, RECT *free_row_rc, int *left_pos, int *right_pos, BOOL left, int real_width, int width, int height, int horizontal_space)
{
	RECT rc = *free_row_rc;
	int width_tmp = width;
	if (left) {
		if (dat->row_align_left_items_to_left)
			width_tmp = real_width;

		rc.left += (width_tmp - real_width) >> 1;
		rc.right = rc.left + real_width;
		rc.top += (rc.bottom - rc.top - height) >> 1;
		rc.bottom = rc.top + height;
		*left_pos += width_tmp + horizontal_space;
		free_row_rc->left = row_rc->left + *left_pos;
	}
	else {
		if (dat->row_align_right_items_to_right)
			width_tmp = real_width;

		if (width_tmp > rc.right - rc.left) {
			rc.left = rc.right + 1;
		}
		else {
			rc.left = max(rc.left + horizontal_space, rc.right - width_tmp) + ((width_tmp - real_width) >> 1);
			rc.right = min(rc.left + real_width, rc.right);
			rc.top += max(0, (rc.bottom - rc.top - height) >> 1);
			rc.bottom = min(rc.top + height, rc.bottom);

			*right_pos += min(width_tmp + horizontal_space, free_row_rc->right - free_row_rc->left);
			free_row_rc->right = row_rc->right - *right_pos;
		}
	}

	return rc;
}



void CLCPaint::_DrawTextSmiley(HDC hdcMem, RECT *free_rc, SIZE *text_size, wchar_t *szText, int start, int len, SortedList *plText, UINT uTextFormat, BOOL ResizeSizeSmiley)
{
	if (szText == nullptr)
		return;

	uTextFormat &= ~DT_RIGHT;
	if (plText == nullptr) {
		if (start) {
			SIZE size;
			GetTextExtentPoint32(hdcMem, szText, start, &size);
			free_rc->left += size.cx;
		}

		ske_DrawText(hdcMem, szText + start, len, free_rc, uTextFormat);
	}
	else {
		// Draw list
		int i;
		int pos_x = 0;
		int row_height;
		RECT tmp_rc = *free_rc;
		if (len == -1) len = (int)mir_wstrlen(szText);
		if (uTextFormat & DT_RTLREADING)
			i = plText->realCount - 1;
		else
			i = 0;

		// Get real height of the line
		row_height = ske_DrawText(hdcMem, L"A", 1, &tmp_rc, DT_CALCRECT | uTextFormat);

		// Just draw ellipsis
		if (free_rc->right <= free_rc->left) {
			if (g_bTrimText)
				ske_DrawText(hdcMem, L"...", 3, free_rc, uTextFormat & ~DT_END_ELLIPSIS);
		}
		else {
			// Draw text and smileys
			for (; i < plText->realCount && i >= 0 && pos_x < text_size->cx && len > 0; i += (uTextFormat & DT_RTLREADING ? -1 : 1)) {
				ClcContactTextPiece *piece = (ClcContactTextPiece *)plText->items[i];
				RECT text_rc = *free_rc;

				if (uTextFormat & DT_RTLREADING)
					text_rc.right -= pos_x;
				else
					text_rc.left += pos_x;

				if (piece->type == TEXT_PIECE_TYPE_TEXT) {
					tmp_rc = text_rc;
					tmp_rc.right += 50;
					ske_DrawText(hdcMem, &szText[piece->start_pos + start], min(len, piece->len), &tmp_rc, DT_CALCRECT | (uTextFormat & ~DT_END_ELLIPSIS));
					pos_x += tmp_rc.right - tmp_rc.left + 2;

					if (uTextFormat & DT_RTLREADING)
						text_rc.left = max(text_rc.left, text_rc.right - (tmp_rc.right - tmp_rc.left));

					ske_DrawText(hdcMem, &szText[piece->start_pos + start], min(len, piece->len), &text_rc, uTextFormat);
					len -= piece->len;
				}
				else {
					float factor = 0;

					if (len < piece->len) {
						len = 0;
					}
					else {
						LONG fac_width, fac_height;
						len -= piece->len;

						if (piece->smiley_height > row_height && ResizeSizeSmiley) {
							factor = row_height / (float)piece->smiley_height;
						}
						else {
							factor = 1;
						}

						fac_width = (LONG)(piece->smiley_width * factor);
						fac_height = (LONG)(piece->smiley_height * factor);

						if (uTextFormat & DT_RTLREADING)
							text_rc.left = max(text_rc.right - fac_width, text_rc.left);

						if (fac_width <= text_rc.right - text_rc.left) {
							text_rc.top += (row_height - fac_height) >> 1;

							ske_DrawIconEx(hdcMem, text_rc.left, text_rc.top, piece->smiley,
								fac_width, fac_height, 0, nullptr, DI_NORMAL | ((factor < 1) ? 128 : 0)); //TO DO enchance drawing quality
						}
						else {
							ske_DrawText(hdcMem, L"...", 3, &text_rc, uTextFormat);
						}

						pos_x += fac_width;
					}
				}
			}
		}
	}
}


void CLCPaint::_AddParameter(MODERNMASK *mpModernMask, MASKPARAM *lpParam)
{
	mpModernMask->pl_Params = (MASKPARAM *)mir_realloc(mpModernMask->pl_Params, (mpModernMask->dwParamCnt + 1)*sizeof(MASKPARAM));
	memmove(&(mpModernMask->pl_Params[mpModernMask->dwParamCnt]), lpParam, sizeof(MASKPARAM));
	mpModernMask->dwParamCnt++;
	memset(lpParam, 0, sizeof(MASKPARAM));
}

void CLCPaint::_FillParam(MASKPARAM *lpParam, uint32_t dwParamHash, const char *const szValue, uint32_t dwValueHash)
{
	lpParam->bMaskParamFlag = MPF_EQUAL | MPF_HASHED;
	lpParam->dwId = dwParamHash;

	if (!dwValueHash && szValue && szValue[0])
		lpParam->dwValueHash = mod_CalcHash(szValue);
	else
		lpParam->dwValueHash = dwValueHash;

	if (szValue)
		lpParam->szValue = mir_strndup(szValue, (int)mir_strlen(szValue));
	else
		lpParam->szValue = nullptr;
}

void CLCPaint::_AddParamShort(MODERNMASK *mpModernMask, uint32_t dwParamIndex, uint32_t dwValueIndex)
{
	AddParam(mpModernMask, HASH[dwParamIndex], HASHTEXT[dwValueIndex], HASH[dwValueIndex]);
}

MODERNMASK* CLCPaint::_GetCLCContactRowBackModernMask(ClcGroup *group, ClcContact *Drawing, int indent, int index, BOOL selected, BOOL hottrack, ClcData *dat)
{
	if (Drawing == nullptr)
		return nullptr;

	char buf[BUF2SIZE] = { 0 };
	MODERNMASK *mpModernMask = (MODERNMASK*)mir_calloc(sizeof(MODERNMASK));

	_AddParamShort(mpModernMask, hi_Module, hi_CL);
	_AddParamShort(mpModernMask, hi_ID, hi_Row);
	switch (Drawing->type) {
	case CLCIT_GROUP:
		_AddParamShort(mpModernMask, hi_Type, hi_Group);
		if (Drawing->group) {
			_AddParamShort(mpModernMask, hi_Open, Drawing->group->expanded ? hi_True : hi_False);
			_AddParamShort(mpModernMask, hi_IsEmpty, (Drawing->group->cl.getCount() == 0) ? hi_True : hi_False);
		}
		break;

	case CLCIT_CONTACT:
		{
			ClcContact *mCont = Drawing;
			if (Drawing->iSubNumber) {
				_AddParamShort(mpModernMask, hi_Type, hi_SubContact);
				if (Drawing->iSubNumber == 1 && Drawing->subcontacts->iSubAllocated == 1)
					_AddParamShort(mpModernMask, hi_SubPos, hi_First_Single);
				else if (Drawing->iSubNumber == 1)
					_AddParamShort(mpModernMask, hi_SubPos, hi_First);
				else if (Drawing->iSubNumber == Drawing->subcontacts->iSubAllocated)
					_AddParamShort(mpModernMask, hi_SubPos, hi_Last);
				else
					_AddParamShort(mpModernMask, hi_SubPos, hi_Middle);
				mCont = Drawing->subcontacts;
			}
			else if (Drawing->iSubAllocated) {
				_AddParamShort(mpModernMask, hi_Type, hi_MetaContact);
				_AddParamShort(mpModernMask, hi_Open, (Drawing->bSubExpanded) ? hi_True : hi_False);
			}
			else _AddParamShort(mpModernMask, hi_Type, hi_Contact);

			AddParam(mpModernMask, HASH[hi_Protocol], Drawing->pce->szProto, 0);
			_AddParamShort(mpModernMask, hi_RootGroup, (group && group->parent == nullptr) ? hi_True : hi_False);
			switch (GetContactCachedStatus(Drawing->hContact)) {
			case ID_STATUS_ONLINE:      _AddParamShort(mpModernMask, hi_Status, hi_ONLINE);    break;
			case ID_STATUS_AWAY:        _AddParamShort(mpModernMask, hi_Status, hi_AWAY);      break;
			case ID_STATUS_DND:         _AddParamShort(mpModernMask, hi_Status, hi_DND);       break;
			case ID_STATUS_NA:          _AddParamShort(mpModernMask, hi_Status, hi_NA);        break;
			case ID_STATUS_OCCUPIED:    _AddParamShort(mpModernMask, hi_Status, hi_OCCUPIED);  break;
			case ID_STATUS_FREECHAT:    _AddParamShort(mpModernMask, hi_Status, hi_FREECHAT);  break;
			case ID_STATUS_INVISIBLE:   _AddParamShort(mpModernMask, hi_Status, hi_INVISIBLE); break;
			case ID_STATUS_IDLE:        _AddParamShort(mpModernMask, hi_Status, hi_IDLE);      break;
			default:                    _AddParamShort(mpModernMask, hi_Status, hi_OFFLINE);
			}
			_AddParamShort(mpModernMask, hi_HasAvatar, (dat->avatars_show && Drawing->avatar_data != nullptr) ? hi_True : hi_False);
			_AddParamShort(mpModernMask, hi_Rate, hi_None + Drawing->bContactRate);
		}
		break;

	case CLCIT_DIVIDER:
		_AddParamShort(mpModernMask, hi_Type, hi_Divider);
		break;

	case CLCIT_INFO:
		_AddParamShort(mpModernMask, hi_Type, hi_Info);
		break;
	}

	if (group->scanIndex == 0 && group->cl.getCount() == 1)
		_AddParamShort(mpModernMask, hi_GroupPos, hi_First_Single);
	else if (group->scanIndex == 0)
		_AddParamShort(mpModernMask, hi_GroupPos, hi_First);
	else if (group->scanIndex + 1 == group->cl.getCount())
		_AddParamShort(mpModernMask, hi_GroupPos, hi_Last);
	else
		_AddParamShort(mpModernMask, hi_GroupPos, hi_Mid);

	_AddParamShort(mpModernMask, hi_Selected, (selected) ? hi_True : hi_False);
	_AddParamShort(mpModernMask, hi_Hot, (hottrack) ? hi_True : hi_False);
	_AddParamShort(mpModernMask, hi_Odd, (index & 1) ? hi_True : hi_False);

	_itoa(indent, buf, BUF2SIZE);
	AddParam(mpModernMask, HASH[hi_Indent], buf, 0);
	_itoa(index, buf, BUF2SIZE);
	AddParam(mpModernMask, HASH[hi_Index], buf, 0);
	{
		wchar_t *b2 = NEWWSTR_ALLOCA(Drawing->szText);
		for (int i = 0; b2[i] != 0; i++)
			if (b2[i] == ',')
				b2[i] = '.';

		AddParam(mpModernMask, HASH[hi_Name], T2Utf(b2), 0);
	}

	if (group->parent) {
		wchar_t *b2 = NEWWSTR_ALLOCA(group->parent->cl[0]->szText);
		for (int i = 0; b2[i] != 0; i++)
			if (b2[i] == ',')
				b2[i] = '.';

		AddParam(mpModernMask, HASH[hi_Group], T2Utf(b2), 0);
	}
	return mpModernMask;
}


void CLCPaint::_RTLRect(RECT *rect, int width)
{
	if (!rect) return;
	int left = (width)-rect->right;
	int right = (width)-rect->left;
	rect->left = left;//-offset;
	rect->right = right;//-offset;
}

void CLCPaint::_PaintRowItemsEx(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int selected, int hottrack)
{
	int dy = row_rc.top + dat->row_border;
	int dg = 0;

	// Let calc placeholder
	int minheight = dat->row_min_heigh;
	BOOL InClistWindow = (dat->hWnd == g_clistApi.hwndContactTree);
	int height = RowHeight_CalcRowHeight(dat, Drawing, -1);

	// TO DO DEPRECATE OLD ROW LAYOUT

	if (Drawing->type == CLCIT_GROUP &&
		Drawing->group->parent->groupId == 0 &&
		Drawing->group->parent->cl[0] != Drawing) {
		dg = dat->row_before_group_space;
		free_row_rc.top += dg;
		height -= dg;
	}
	if (!InClistWindow || !gl_RowRoot || Drawing->type == CLCIT_GROUP) {
		// to do paint simple
		RECT fr_rc = free_row_rc;

		// 1 draw icon
		if (!(Drawing->type == CLCIT_GROUP && InClistWindow && dat->row_hide_group_icon)) {
			int iImage = -1;
			// Get image
			if (Drawing->type == CLCIT_GROUP) {
				iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
			}
			else if (Drawing->type == CLCIT_CONTACT)
				iImage = Drawing->iImage;

			if (iImage != -1) {
				COLORREF colourFg;
				int mode;
				RECT p_rect = { 0 };
				p_rect.top = fr_rc.top + ((fr_rc.bottom - fr_rc.top - ICON_HEIGHT) >> 1);
				p_rect.left = fr_rc.left;
				p_rect.right = p_rect.left + ICON_HEIGHT;
				p_rect.bottom = p_rect.top + ICON_HEIGHT;
				// Store pos
				if (dat->text_rtl != 0) _RTLRect(&p_rect, free_row_rc.right);
				Drawing->pos_icon = p_rect;
				if (hottrack) {
					colourFg = dat->hotTextColour;
					mode = ILD_NORMAL;
				}
				else if (Drawing->type == CLCIT_CONTACT && Drawing->flags & CONTACTF_NOTONLIST) {
					colourFg = dat->fontModernInfo[FONTID_NOTONLIST].colour;
					mode = ILD_BLEND50;
				}
				else {
					colourFg = dat->selBkColour;
					mode = ILD_NORMAL;
				}

				if (Drawing->type == CLCIT_CONTACT && dat->bShowIdle && (Drawing->flags & CONTACTF_IDLE) &&
					Clist_GetRealStatus(Drawing, ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
					mode = ILD_SELECTED;
				}
				_DrawStatusIcon(Drawing, dat, iImage, hdcMem, p_rect.left, p_rect.top, 0, 0, CLR_NONE, colourFg, mode);
			}
			fr_rc.left += ICON_HEIGHT + 2;
		}
		// 2 draw extra
		{
			RECT p_rect = { 0 };
			p_rect.top = fr_rc.top + ((fr_rc.bottom - fr_rc.top - ICON_HEIGHT) >> 1);
			p_rect.bottom = p_rect.top + ICON_HEIGHT;

			if ((Drawing->type == CLCIT_GROUP || Drawing->type == CLCIT_CONTACT || Drawing->type == CLCIT_INFO)
				&& dat->extraColumnsCount > 0 && (!InClistWindow || Drawing->type == CLCIT_CONTACT)) {
				int BlendedInActiveState = dat->dbbBlendInActiveState;
				int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
				int iImage;
				int count = 0;
				RECT rc;
				int x = 0;
				for (iImage = dat->extraColumnsCount - 1; iImage >= 0; iImage--) {
					COLORREF colourFg = dat->selBkColour;
					int mode = BlendedInActiveState ? BlendValue : ILD_NORMAL;
					if (Drawing->iExtraImage[iImage] == EMPTY_EXTRA_ICON) {
						x += (x > 0) ? dat->extraColumnSpacing : ICON_HEIGHT;
						SetRect(&rc, fr_rc.right - x, p_rect.top, fr_rc.right - x + ICON_HEIGHT, p_rect.bottom);
						if (dat->text_rtl != 0) _RTLRect(&rc, free_row_rc.right);
						Drawing->pos_extra[iImage] = rc;
						count++;
						continue;
					}

					if (selected)
						mode = BlendedInActiveState ? ILD_NORMAL : ILD_SELECTED;
					else if (hottrack) {
						mode = BlendedInActiveState ? ILD_NORMAL : ILD_FOCUS;
						colourFg = dat->hotTextColour;
					}
					else if (Drawing->type == CLCIT_CONTACT && Drawing->flags & CONTACTF_NOTONLIST) {
						colourFg = dat->fontModernInfo[FONTID_NOTONLIST].colour;
						mode = BlendValue;
					}

					x += (x > 0) ? dat->extraColumnSpacing : ICON_HEIGHT;
					SetRect(&rc, fr_rc.right - x, p_rect.top, fr_rc.right - x + ICON_HEIGHT, p_rect.bottom);
					if (dat->text_rtl != 0) _RTLRect(&rc, free_row_rc.right);
					Drawing->pos_extra[iImage] = rc;
					if (Drawing->iExtraImage[iImage] != EMPTY_EXTRA_ICON)
						ske_ImageList_DrawEx(dat->himlExtraColumns, Drawing->iExtraImage[iImage], hdcMem, rc.left, rc.top, 0, 0, CLR_NONE, colourFg, mode);
				}
				fr_rc.right -= x;
			}
		}
		// 3 draw text
		{
			SIZE text_size = { 0 };
			wchar_t *szCounts = nullptr;
			RECT text_rect = fr_rc;
			RECT counts_rc = { 0 };
			UINT uTextFormat = DT_LEFT | DT_VCENTER | (g_bTrimText ? DT_END_ELLIPSIS : 0) | DT_SINGLELINE;
			uTextFormat |= dat->text_rtl ? DT_RTLREADING : 0;
			// Select font
			ChangeToFont(hdcMem, dat, GetBasicFontID(Drawing), nullptr);

			// Get text size
			GetTextSize(&text_size, hdcMem, fr_rc, Drawing->szText, Drawing->ssText.plText, uTextFormat,
				dat->text_resize_smileys ? 0 : Drawing->ssText.iMaxSmileyHeight);
			// counters
			if (Drawing->type == CLCIT_GROUP && InClistWindow) {
				RECT nameRect = fr_rc;
				RECT countRect = { 0 };
				RECT count_rc = { 0 };
				SIZE count_size = { 0 };
				int space_width = 0;
				szCounts = Clist_GetGroupCountsText(dat, Drawing);
				// Has to draw the count?
				if (szCounts && mir_wstrlen(szCounts) > 0) {
					// calc width and height
					ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
					ske_DrawText(hdcMem, L" ", 1, &count_rc, DT_CALCRECT | DT_NOPREFIX);
					count_size.cx = count_rc.right - count_rc.left;
					space_width = count_size.cx;
					count_rc.right = 0;
					count_rc.left = 0;
					ske_DrawText(hdcMem, szCounts, (int)mir_wstrlen(szCounts), &count_rc, DT_CALCRECT);
					count_size.cx += count_rc.right - count_rc.left;
					count_size.cy = count_rc.bottom - count_rc.top;
				}
				// modify text rect
				{
					SIZE grp_size = { 0 };
					int wid = fr_rc.right - fr_rc.left;
					ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPS : FONTID_CLOSEDGROUPS, nullptr);
					GetTextSize(&grp_size, hdcMem, fr_rc, Drawing->szText, Drawing->ssText.plText, 0, dat->text_resize_smileys ? 0 : Drawing->ssText.iMaxSmileyHeight);

					if (wid - count_size.cx > grp_size.cx) {
						if (dat->row_align_group_mode != 2) { //center or left
							int x = (dat->row_align_group_mode == 1) ? (wid - (grp_size.cx + count_size.cx)) >> 1 : 0;
							nameRect.left += x;
							nameRect.right = nameRect.left + grp_size.cx;
							countRect.left = nameRect.right + space_width;
							countRect.right = countRect.left + count_size.cx - space_width;
						}
						else {
							countRect.right = nameRect.right;
							countRect.left = countRect.right - ((count_size.cx > 0) ? (count_size.cx - space_width) : 0);
							nameRect.right = countRect.left - ((count_size.cx > 0) ? space_width : 0);
							nameRect.left = nameRect.right - grp_size.cx;
						}
					}
					else {
						countRect.right = nameRect.right;
						nameRect.right -= count_size.cx;
						countRect.left = nameRect.right + space_width;
					}
					countRect.bottom = nameRect.bottom;
					countRect.top = nameRect.top;
				}

				uTextFormat |= DT_VCENTER;
				ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPS : FONTID_CLOSEDGROUPS, nullptr);
				if (selected)
					SetTextColor(hdcMem, dat->selTextColour);
				else if (hottrack)
					_SetHotTrackColour(hdcMem, dat);
				if (dat->text_rtl != 0) _RTLRect(&nameRect, free_row_rc.right);
				_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, 0, (int)mir_wstrlen(Drawing->szText), Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);

				if (selected && !dat->bFilterSearch) {
					SetTextColor(hdcMem, dat->quickSearchColour);
					_DrawTextSmiley(hdcMem, &nameRect, &text_size, Drawing->szText, 0, (int)mir_wstrlen(Drawing->szText), Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);
				}

				if (szCounts && mir_wstrlen(szCounts) > 0) {
					ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
					if (selected)
						SetTextColor(hdcMem, dat->selTextColour);
					else if (hottrack)
						_SetHotTrackColour(hdcMem, dat);
					if (dat->text_rtl != 0) _RTLRect(&countRect, free_row_rc.right);
					ske_DrawText(hdcMem, szCounts, (int)mir_wstrlen(szCounts), &countRect, uTextFormat);
				}
				Drawing->pos_label = nameRect;
				return;
			}
			else if (Drawing->type == CLCIT_GROUP) {
				szCounts = Clist_GetGroupCountsText(dat, Drawing);
				// Has to draw the count?
				if (szCounts && szCounts[0]) {
					RECT space_rc = fr_rc;

					int text_width = 0;
					SIZE space_size = { 0 };
					SIZE counts_size = { 0 };
					// Get widths
					counts_rc = fr_rc;
					DrawText(hdcMem, L" ", 1, &space_rc, DT_CALCRECT | DT_NOPREFIX);

					space_size.cx = space_rc.right - space_rc.left;
					space_size.cy = min(space_rc.bottom - space_rc.top, fr_rc.bottom - fr_rc.top);

					ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
					DrawText(hdcMem, szCounts, (int)mir_wstrlen(szCounts), &counts_rc, DT_CALCRECT);

					counts_size.cx = counts_rc.right - counts_rc.left;
					counts_size.cy = min(counts_rc.bottom - counts_rc.top, fr_rc.bottom - fr_rc.top);

					text_width = fr_rc.right - fr_rc.left - space_size.cx - counts_size.cx;

					if (text_width > 4) {
						text_size.cx = min(text_width, text_size.cx);
						text_width = text_size.cx + space_size.cx + counts_size.cx;
					}
					else {
						text_width = text_size.cx;
						space_size.cx = 0;
						counts_size.cx = 0;
					}
					text_rect.right = text_rect.left + text_size.cx;
					counts_rc = text_rect;
					counts_rc.left = text_rect.right + space_size.cx;
					counts_rc.right = counts_rc.left + counts_size.cx;
				}
			}
			ChangeToFont(hdcMem, dat, GetBasicFontID(Drawing), nullptr);

			// Set color
			if (selected)
				SetTextColor(hdcMem, dat->selTextColour);
			else if (hottrack)
				_SetHotTrackColour(hdcMem, dat);
			if (dat->text_rtl != 0) _RTLRect(&text_rect, free_row_rc.right);
			_DrawTextSmiley(hdcMem, &text_rect, &text_size, Drawing->szText, 0, (int)mir_wstrlen(Drawing->szText), Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);
			if (((dat->bFilterSearch && Drawing->type != CLCIT_GROUP) || selected) && dat->szQuickSearch[0] != '\0') {
				int idx = 0;
				if (dat->bFilterSearch) {
					wchar_t *lowered = CharLowerW(NEWWSTR_ALLOCA(Drawing->szText));
					wchar_t *lowered_search = CharLowerW(NEWWSTR_ALLOCA(dat->szQuickSearch));
					wchar_t *p1 = wcsstr(lowered, lowered_search);
					if (p1)
						idx = int(p1 - lowered);
				}
				SetTextColor(hdcMem, dat->quickSearchColour);
				_DrawTextSmiley(hdcMem, &text_rect, &text_size, Drawing->szText, idx, (int)mir_wstrlen(dat->szQuickSearch), Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);
			}
			if (Drawing->type == CLCIT_GROUP && szCounts && szCounts[0] && counts_rc.right - counts_rc.left > 0) {
				ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
				if (dat->text_rtl != 0) _RTLRect(&counts_rc, free_row_rc.right);
				ske_DrawText(hdcMem, szCounts, (int)mir_wstrlen(szCounts), &counts_rc, uTextFormat);
				if (dat->text_rtl == 0)
					text_rect.right = counts_rc.right;
				else
					text_rect.left = counts_rc.left;
			}
			Drawing->pos_label = text_rect;

			RECT rcPos = fr_rc;
			if (dat->text_rtl != 0)
				_RTLRect(&rcPos, free_row_rc.right);

			if ((!InClistWindow || !g_CluiData.fLayered) && ((Drawing->type == CLCIT_DIVIDER) || (Drawing->type == CLCIT_GROUP && dat->exStyle & CLS_EX_LINEWITHGROUPS))) {
				RECT rc = fr_rc;
				if (dat->text_rtl != 0) {
					rc.left = rcPos.left;
					rc.right = text_rect.left - 3;
				}
				else rc.left = text_rect.right + 3;

				if (rc.right - rc.left > 4) {
					rc.top += ((rc.bottom - rc.top) >> 1) - 1;
					rc.bottom = rc.top + 2;
					DrawEdge(hdcMem, &rc, BDR_SUNKENOUTER, BF_RECT);
					ske_SetRectOpaque(hdcMem, &rc);
				}
			}
		}
		return;
	}

	minheight = max(minheight, height);
	dy += (minheight > height) ? ((minheight - height) >> 1) : 0;
}

void CLCPaint::_DrawStatusIcon(ClcContact *Drawing, ClcData *dat, int iImage, HDC hdcMem, int x, int y, int cx, int cy, uint32_t colorbg, uint32_t colorfg, int mode)
{
	if (Drawing->type != CLCIT_CONTACT)
		ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem, x, y, cx, cy, colorbg, colorfg, mode);

	else if (Drawing->bImageIsSpecial)
		ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem, x, y, cx, cy, colorbg, colorfg, mode);

	else if (iImage != -1 && HIWORD(iImage) && dat->drawOverlayedStatus) {
		int status = GetContactCachedStatus(Drawing->hContact);
		if (status < ID_STATUS_OFFLINE)
			status = ID_STATUS_OFFLINE;
		else if (status > ID_STATUS_MAX)
			status = ID_STATUS_ONLINE;
		ske_ImageList_DrawEx(g_himlCListClc, HIWORD(iImage), hdcMem, x, y, cx, cy, colorbg, colorfg, mode);
		if (dat->drawOverlayedStatus & 2) //draw overlay
			ske_ImageList_DrawEx(hAvatarOverlays, g_pStatusOverlayIcons[status - ID_STATUS_OFFLINE].listID, hdcMem, x, y, cx, cy, colorbg, colorfg, mode);
	}
	else ske_ImageList_DrawEx(g_himlCListClc, LOWORD(iImage), hdcMem, x, y, cx, cy, colorbg, colorfg, mode);
}

BOOL CLCPaint::_DrawNonEnginedBackground(HDC hdcMem, RECT *rcPaint, RECT clRect, ClcData *dat)
{
	if (!dat->hBmpBackground)
		return FALSE;

	// XXX: Halftone isnt supported on 9x, however the scretch problems dont happen on 98.
	SetStretchBltMode(hdcMem, HALFTONE);

	BITMAP bmp;
	GetObject(dat->hBmpBackground, sizeof(bmp), &bmp);
	HDC hdcBmp = CreateCompatibleDC(hdcMem);
	HBITMAP oldbm = (HBITMAP)SelectObject(hdcBmp, dat->hBmpBackground);
	int x, y = dat->backgroundBmpUse & CLBF_SCROLL ? -dat->yScroll : 0;
	int maxx = dat->backgroundBmpUse & CLBF_TILEH ? clRect.right : 1;
	int maxy = dat->backgroundBmpUse & CLBF_TILEV ? rcPaint->bottom : y + 1;
	int destw, desth;

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
			if (dat->backgroundBmpUse & CLBF_TILEVTOROWHEIGHT)
				desth = dat->row_min_heigh;
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

	default:    //clb_topleft
		destw = bmp.bmWidth;
		desth = bmp.bmHeight;
		if (dat->backgroundBmpUse & CLBF_TILEVTOROWHEIGHT)
			desth = dat->row_min_heigh;
		break;
	}

	for (; y < maxy; y += desth) {
		if (y < rcPaint->top - desth) continue;
		for (x = 0; x < maxx; x += destw)
			StretchBlt(hdcMem, x, y, destw, desth, hdcBmp, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
	}
	SelectObject(hdcBmp, oldbm);
	DeleteDC(hdcBmp);
	return TRUE;
}

int CLCPaint::_DetermineDrawMode(HWND hWnd, ClcData *dat)
{
	int paintMode = DM_LAYERED; // by default

	if (dat->bForceInDialog)
		paintMode = DM_CONTROL;
	else if (g_CluiData.fDisableSkinEngine)
		paintMode = DM_CLASSIC;
	else if (!g_CluiData.fLayered)
		paintMode = DM_NON_LAYERED;

	if (!(paintMode & DM_CONTROL) && !CLUI_IsInMainWindow(hWnd))
		paintMode |= DM_FLOAT;

	int nStatus = Clist_GetGeneralizedStatus();
	LONG lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	if ((lStyle & WS_DISABLED) || (dat->greyoutFlags & Clist_ClcStatusToPf2(nStatus)) || ((dat->greyoutFlags & GREYF_UNFOCUS) && (GetFocus() != hWnd)))
		paintMode |= DM_GRAY;

	if (lStyle & CLS_GREYALTERNATE)
		paintMode |= DM_GREYALTERNATE;

	return paintMode;
}

void CLCPaint::_PreparePaintContext(ClcData *dat, HDC hdc, int paintMode, RECT &clRect, _PaintContext &pc)
{
	if ((paintMode & DM_GRAY) && !(paintMode & DM_LAYERED)) {
		pc.hdcMem2 = CreateCompatibleDC(hdc);
		if (paintMode & DM_CLASSIC)
			pc.hBmpOsb2 = CreateBitmap(clRect.right, clRect.bottom, 1, GetDeviceCaps(hdc, BITSPIXEL), nullptr);
		else
			pc.hBmpOsb2 = ske_CreateDIB32(clRect.right, clRect.bottom);
		pc.oldbmp2 = (HBITMAP)SelectObject(pc.hdcMem2, pc.hBmpOsb2);
		pc.fRelease |= _PaintContext::release_hdcmem2;
	}

	if (paintMode & (DM_DRAW_OFFSCREEN | DM_GRAY)) {
		pc.hdcMem = CreateCompatibleDC(hdc);
		pc.fRelease |= _PaintContext::release_hdcmem;
		pc.hBmpOsb = ske_CreateDIB32(clRect.right, clRect.bottom);
		pc.oldbmp = (HBITMAP)SelectObject(pc.hdcMem, pc.hBmpOsb);
	}

	if ((paintMode & DM_CONTROL) && !dat->bkChanged) {
		pc.tmpbkcolour = GetSysColor(COLOR_3DFACE);
		pc.tmpforecolour = GetSysColor(COLOR_BTNTEXT);
	}
	else {
		pc.tmpbkcolour = (!(paintMode & DM_CONTROL) && dat->bkChanged) ? dat->bkColour : (!dat->bUseWindowsColours ? dat->bkColour : GetSysColor(COLOR_3DFACE));
		pc.tmpforecolour = dat->fontModernInfo[FONTID_CONTACTS].colour;
	}

	if (paintMode & DM_GREYALTERNATE) {
		int rDelta = (GetRValue(pc.tmpbkcolour) > GetRValue(pc.tmpforecolour)) ? -10 : 10;
		int gDelta = (GetGValue(pc.tmpbkcolour) > GetGValue(pc.tmpforecolour)) ? -10 : 10;
		int bDelta = (GetBValue(pc.tmpbkcolour) > GetBValue(pc.tmpforecolour)) ? -10 : 10;
		int rValue = GetRValue(pc.tmpbkcolour) + rDelta;
		int gValue = GetRValue(pc.tmpbkcolour) + gDelta;
		int bValue = GetRValue(pc.tmpbkcolour) + bDelta;

		uint8_t brValue = (rValue > 255) ? 255 : rValue < 0 ? 0 : (uint8_t)rValue;
		uint8_t bgValue = (gValue > 255) ? 255 : gValue < 0 ? 0 : (uint8_t)gValue;
		uint8_t bbValue = (bValue > 255) ? 255 : bValue < 0 ? 0 : (uint8_t)bValue;

		pc.hBrushAlternateGrey = CreateSolidBrush(GetNearestColor(pc.hdcMem, RGB(brValue, bgValue, bbValue)));
	}

	// Set some draw states
	SetBkMode(pc.hdcMem, TRANSPARENT);
	SetStretchBltMode(pc.hdcMem, HALFTONE);

	POINT org;
	GetBrushOrgEx(pc.hdcMem, &org);
	SetBrushOrgEx(pc.hdcMem, org.x, org.y, nullptr);
}

void CLCPaint::_DrawBackground(HWND hWnd, ClcData *dat, int paintMode, RECT *rcPaint, RECT &clRect, _PaintContext &pc)
{
	if (paintMode & (DM_FLOAT | DM_CONTROL)) {
		HBRUSH hBrush = CreateSolidBrush(pc.tmpbkcolour);
		FillRect(pc.hdcMem, rcPaint, hBrush);
		DeleteObject(hBrush);
		ske_SetRectOpaque(pc.hdcMem, rcPaint);
		if (!(paintMode & DM_GREYALTERNATE))
			SkinDrawGlyph(pc.hdcMem, &clRect, rcPaint, "CL,ID=Background,Type=Control");
	}
	else if (paintMode & DM_CLASSIC) {
		if (!_DrawNonEnginedBackground(pc.hdcMem, rcPaint, clRect, dat)) {
			HBRUSH hBrush = CreateSolidBrush(pc.tmpbkcolour);
			FillRect(pc.hdcMem, rcPaint, hBrush);
			DeleteObject(hBrush);
		}
	}
	else {
		if (paintMode & DM_NON_LAYERED)
			ske_BltBackImage(hWnd, (paintMode & DM_GRAY) ? pc.hdcMem2 : pc.hdcMem, rcPaint);

		SkinDrawGlyph(pc.hdcMem, &clRect, rcPaint, "CL,ID=Background");
	}
}

void CLCPaint::_DrawLines(HWND hWnd, ClcData *dat, int paintMode, RECT *rcPaint, RECT &clRect, _PaintContext &pc)
{
	ClcGroup *group = &dat->list;
	group->scanIndex = 0;
	int indent = 0;
	int subindex = -1;
	int line_num = -1;
	int y = -dat->yScroll;
	BOOL is_foreground = IsForegroundWindow(hWnd);
	LONG lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);

	while (y < rcPaint->bottom) {
		if (subindex == -1) {
			if (group->scanIndex >= group->cl.getCount()) {
				group = group->parent;
				indent--;
				if (group == nullptr) break;  // Finished list
				group->scanIndex++;
				continue;
			}
		}

		line_num++;

		// Draw line, if needed
		if (y > rcPaint->top - dat->getRowHeight(line_num)) {
			RECT rc;

			// Get item to draw
			int subident = 0;
			ClcContact *Drawing;
			if (group->scanIndex < group->cl.getCount()) {
				if (subindex == -1)
					Drawing = group->cl[group->scanIndex];
				else {
					Drawing = &group->cl[group->scanIndex]->subcontacts[subindex];
					subident = dat->subIndent;
				}
			}
			else Drawing = nullptr;

			// Something to draw?
			if (Drawing) {
				// Calc row height
				if (!gl_RowRoot)
					RowHeights_GetRowHeight(dat, hWnd, Drawing, line_num);
				else
					RowHeight_CalcRowHeight(dat, Drawing, line_num);

				// Init settings
				int selected = ((line_num == dat->selection) && (dat->hwndRenameEdit != nullptr || dat->bShowSelAlways || dat->exStyle & CLS_EX_SHOWSELALWAYS || is_foreground) && Drawing->type != CLCIT_DIVIDER);
				int hottrack = dat->exStyle & CLS_EX_TRACKSELECT && Drawing->type != CLCIT_DIVIDER && dat->iHotTrack == line_num;
				int left_pos = clRect.left + dat->leftMargin + indent * dat->groupIndent + subident;
				int right_pos = dat->rightMargin;   // Border

				RECT row_rc;
				SetRect(&row_rc, clRect.left, y, clRect.right, y + dat->getRowHeight(line_num));

				RECT free_row_rc = row_rc;
				free_row_rc.left += left_pos;
				free_row_rc.right -= right_pos;
				free_row_rc.top += dat->row_border;
				free_row_rc.bottom -= dat->row_border;
				{
					HRGN rgn = CreateRectRgn(row_rc.left, row_rc.top, row_rc.right, row_rc.bottom);
					SelectClipRgn(pc.hdcMem, rgn);
					DeleteObject(rgn);
				}

				// Store pos
				Drawing->pos_indent = free_row_rc.left;
				memset(&Drawing->pos_check, 0, sizeof(Drawing->pos_check));
				memset(&Drawing->pos_avatar, 0, sizeof(Drawing->pos_avatar));
				memset(&Drawing->pos_icon, 0, sizeof(Drawing->pos_icon));
				memset(&Drawing->pos_label, 0, sizeof(Drawing->pos_label));
				memset(&Drawing->pos_extra, 0, sizeof(Drawing->pos_extra));

				//**** Draw Background

				// Alternating grey
				if (paintMode & DM_GREYALTERNATE && (line_num & 1)) {
					if (paintMode & DM_CONTROL || dat->bkChanged)
						FillRect(pc.hdcMem, &row_rc, pc.hBrushAlternateGrey);
					else
						SkinDrawGlyph(pc.hdcMem, &row_rc, rcPaint, "CL,ID=GreyAlternate");
				}
				MODERNMASK *mpRequest = nullptr;
				if (!(paintMode & (DM_CLASSIC | DM_CONTROL))) {
					// Row background
					if (!(paintMode & DM_CONTROL)) {   //Build mpRequest string
						mpRequest = _GetCLCContactRowBackModernMask(group, Drawing, indent, line_num, selected, hottrack, dat);

						RECT mrc = row_rc;
						if (group->parent == nullptr
							&& group->scanIndex != 0
							&& group->scanIndex < group->cl.getCount()
							&& group->cl[group->scanIndex]->type == CLCIT_GROUP) {
							mrc.top += dat->row_before_group_space;
						}
						SkinDrawGlyphMask(pc.hdcMem, &mrc, rcPaint, mpRequest);
					}
					if (selected || hottrack) {
						RECT mrc = row_rc;
						if (Drawing->type == CLCIT_GROUP &&
							Drawing->group->parent->groupId == 0 &&
							Drawing->group->parent->cl[0] != Drawing) {
							mrc.top += dat->row_before_group_space;
						}
						// Selection background ( only if hole line - full/less )
						if (dat->HiLightMode == 1) { // Full  or default
							if (selected)
								SkinDrawGlyph(pc.hdcMem, &mrc, rcPaint, "CL,ID=Selection");
							if (hottrack)
								SkinDrawGlyph(pc.hdcMem, &mrc, rcPaint, "CL,ID=HotTracking");
						}
						else if (dat->HiLightMode == 2) { // Less
							if (selected)
								SkinDrawGlyph(pc.hdcMem, &mrc, rcPaint, "CL,ID=Selection");      //instead of free_row_rc
							if (hottrack)
								SkinDrawGlyph(pc.hdcMem, &mrc, rcPaint, "CL,ID=HotTracking");
						}
					}
				}
				else {
					// background
					if (selected) {
						int row_height;
						switch (dat->HiLightMode) {
						case 0:
						case 1:
							row_height = row_rc.bottom - row_rc.top;
							for (int i = y; i < y + row_height; i += max(dat->row_min_heigh, 1)) {
								ImageList_DrawEx(dat->himlHighlight, 0, pc.hdcMem, 0, i, clRect.right,
									min(y + row_height - i, max(dat->row_min_heigh, 1)), CLR_NONE, CLR_NONE,
									dat->exStyle & CLS_EX_NOTRANSLUCENTSEL ? ILD_NORMAL : ILD_BLEND25);
							}
							SetTextColor(pc.hdcMem, paintMode & DM_CONTROL ? GetSysColor(COLOR_HIGHLIGHTTEXT) : dat->selTextColour);
							break;

						case 2:
							row_height = row_rc.bottom - row_rc.top - 1;
							for (int i = y + 1; i < y + row_height; i += max(dat->row_min_heigh, 1)) {
								ImageList_DrawEx(dat->himlHighlight, 0, pc.hdcMem, 1, i, clRect.right - 2,
									min(y + row_height - i, max(dat->row_min_heigh, 1)), CLR_NONE, CLR_NONE,
									dat->exStyle & CLS_EX_NOTRANSLUCENTSEL ? ILD_NORMAL : ILD_BLEND25);
							}
							SetTextColor(pc.hdcMem, paintMode & DM_CONTROL ? GetSysColor(COLOR_HIGHLIGHTTEXT) : dat->selTextColour);
							break;
						}
					}
				}
				//**** Checkboxes
				if (Drawing->isCheckBox(lStyle)) {
					rc = free_row_rc;
					rc.right = rc.left + dat->checkboxSize;
					rc.top += (rc.bottom - rc.top - dat->checkboxSize) >> 1;
					rc.bottom = rc.top + dat->checkboxSize;

					if (dat->text_rtl != 0)
						_RTLRect(&rc, free_row_rc.right);

					if (xpt_IsThemed(dat->hCheckBoxTheme))
						xpt_DrawThemeBackground(dat->hCheckBoxTheme, pc.hdcMem, BP_CHECKBOX, Drawing->flags & CONTACTF_CHECKED ? (hottrack ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL) : (hottrack ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL), &rc, &rc);
					else
						DrawFrameControl(pc.hdcMem, &rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT | (Drawing->flags & CONTACTF_CHECKED ? DFCS_CHECKED : 0) | (hottrack ? DFCS_HOT : 0));

					left_pos += dat->checkboxSize + EXTRA_CHECKBOX_SPACE + HORIZONTAL_SPACE;
					free_row_rc.left = row_rc.left + left_pos;

					// Store pos
					Drawing->pos_check = rc;
				}
				_PaintRowItems(pc.hdcMem, dat, Drawing, row_rc, free_row_rc, left_pos, right_pos, selected, hottrack, rcPaint);
				if (mpRequest) {
					if (!dat->bForceInDialog) {
						mir_free(mpRequest->pl_Params[1].szValue);
						mpRequest->pl_Params[1].szValue = mir_strndup("Ovl", 3);
						mpRequest->pl_Params[1].dwValueHash = mod_CalcHash("Ovl");

						RECT mrc = row_rc;
						if (Drawing->type == CLCIT_GROUP &&
							Drawing->group->parent->groupId == 0 &&
							Drawing->group->parent->cl[0] != Drawing) {
							mrc.top += dat->row_before_group_space;
						}
						SkinDrawGlyphMask(pc.hdcMem, &mrc, rcPaint, mpRequest);
						SkinSelector_DeleteMask(mpRequest);
					}
					mir_free(mpRequest);
				}
			}
		}
		y += dat->getRowHeight(line_num);

		// increment by subcontacts
		ClcContact *cc = group->cl[group->scanIndex];
		if (cc != nullptr && cc->subcontacts != nullptr && cc->type != CLCIT_GROUP && cc->bSubExpanded && dat->bMetaExpanding) {
			if (subindex < cc->iSubAllocated - 1)
				subindex++;
			else
				subindex = -1;
		}

		if (cc && subindex == -1) {
			if (cc->type == CLCIT_GROUP && cc->group->expanded) {
				group = cc->group;
				indent++;
				group->scanIndex = 0;
				subindex = -1;
				continue;
			}
			group->scanIndex++;
		}
		else if (group->scanIndex >= group->cl.getCount()) {
			subindex = -1;
		}
	}
	SelectClipRgn(pc.hdcMem, nullptr);
}

void CLCPaint::_DrawInsertionMark(ClcData *dat, RECT &clRect, _PaintContext &pc)
{
	int identation = dat->nInsertionLevel * dat->groupIndent;
	int yt = cliGetRowTopY(dat, dat->iInsertionMark);

	POINT pts[8];
	pts[0].y = yt - dat->yScroll - 4;
	if (pts[0].y < -3) pts[0].y = -3;
	pts[0].x = 1 + identation * (dat->text_rtl ? 0 : 1);

	pts[1].x = pts[0].x + 2;
	pts[1].y = pts[0].y + 3;

	pts[2].x = clRect.right - identation * (dat->text_rtl ? 1 : 0) - 4;
	pts[2].y = pts[1].y;

	pts[3].x = clRect.right - 1 - identation * (dat->text_rtl ? 1 : 0);
	pts[3].y = pts[0].y - 1;

	pts[4].x = pts[3].x;        pts[4].y = pts[0].y + 7;
	pts[5].x = pts[2].x + 1;    pts[5].y = pts[1].y + 2;
	pts[6].x = pts[1].x;        pts[6].y = pts[5].y;
	pts[7].x = pts[0].x;        pts[7].y = pts[4].y;

	HRGN hRgn = CreatePolygonRgn(pts, sizeof(pts) / sizeof(pts[0]), ALTERNATE);
	HBRUSH hBrush = CreateSolidBrush(dat->fontModernInfo[FONTID_CONTACTS].colour);
	HBRUSH hoBrush = (HBRUSH)SelectObject(pc.hdcMem, hBrush);
	FillRgn(pc.hdcMem, hRgn, hBrush);
	ske_SetRgnOpaque(pc.hdcMem, hRgn);
	SelectObject(pc.hdcMem, hoBrush);
	DeleteObject(hBrush);
}

void CLCPaint::_CopyPaintToDest(HDC hdc, int paintMode, RECT *rcPaint, _PaintContext &pc)
{
	if (!(paintMode & DM_GRAY) && (paintMode & DM_DRAW_OFFSCREEN))
		BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, pc.hdcMem, rcPaint->left, rcPaint->top, SRCCOPY);

	if ((paintMode & DM_GRAY) && hdc && hdc != pc.hdcMem) {
		BLENDFUNCTION bf = { AC_SRC_OVER, 0, 80, AC_SRC_ALPHA };
		BOOL a = ((paintMode & DM_GRAY) && ((paintMode & DM_NON_LAYERED)));
		ske_AlphaBlend(a ? pc.hdcMem2 : hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, pc.hdcMem, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, bf);
		if (a)
			BitBlt(hdc, rcPaint->left, rcPaint->top, rcPaint->right - rcPaint->left, rcPaint->bottom - rcPaint->top, pc.hdcMem2, rcPaint->left, rcPaint->top, SRCCOPY);
	}
}

void CLCPaint::_FreePaintContext(_PaintContext &pc)
{
	if (pc.hBrushAlternateGrey) DeleteObject(pc.hBrushAlternateGrey);
	SelectObject(pc.hdcMem, (HFONT)GetStockObject(DEFAULT_GUI_FONT));
	if (pc.fRelease & _PaintContext::release_hdcmem) {
		SelectObject(pc.hdcMem, pc.oldbmp);
		DeleteObject(pc.hBmpOsb);
		DeleteDC(pc.hdcMem);
	}
	if (pc.fRelease & _PaintContext::release_hdcmem2) {
		SelectObject(pc.hdcMem2, pc.oldbmp2);
		DeleteObject(pc.hBmpOsb2);
		DeleteDC(pc.hdcMem2);
	}
}

void CLCPaint::_PaintClc(HWND hwnd, ClcData *dat, HDC hdc, RECT *_rcPaint)
{
	g_CluiData.t_now = time(0);

	if (_rcPaint && IsRectEmpty(_rcPaint)) return;       // check if draw area is not empty
	if (!IsWindowVisible(hwnd)) return;                  // and window is visible

	RECT clRect;
	GetClientRect(hwnd, &clRect);

	clRect.top += dat->topMargin;
	clRect.bottom -= dat->bottomMargin;

	RECT *rcPaint = _rcPaint ? _rcPaint : &clRect;       // if null is transmitted - use whole client area

	// Invalidate ani avatars, avatars have to be validated in row paint routine below
	if (rcPaint->top == 0 && rcPaint->bottom == clRect.bottom && dat->avatars_show)
		AniAva_InvalidateAvatarPositions(0);

	// Now determine paint mode
	int paintMode = _DetermineDrawMode(hwnd, dat);

	// Preparing drawing contexts
	_PaintContext pc(hdc);
	_PreparePaintContext(dat, hdc, paintMode, clRect, pc);

	// Draw background
	_DrawBackground(hwnd, dat, paintMode, rcPaint, clRect, pc);

	// Draw lines
	if (!dat->row_variable_height || dat->row_heights) {
		_DrawLines(hwnd, dat, paintMode, rcPaint, clRect, pc);

		// insertion mark
		if (dat->iInsertionMark != -1)
			_DrawInsertionMark(dat, clRect, pc);
	}

	// BitBlt from memory to destination
	_CopyPaintToDest(hdc, paintMode, rcPaint, pc);

	// Free
	_FreePaintContext(pc);

	// all still non-validated animated avatars have to be removed
	AniAva_RemoveInvalidatedAvatars();
}

void CLCPaint::_StoreItemPos(ClcContact *contact, int ItemType, RECT *rc)
{
	contact->ext_mpItemsDesc[contact->ext_nItemsNum].itemType = ItemType;
	contact->ext_mpItemsDesc[contact->ext_nItemsNum].itemRect = *rc;
	contact->ext_nItemsNum++;
	switch (ItemType) {
	case CIT_AVATAR:
		contact->pos_avatar = *rc;
		break;
	case CIT_ICON:
		contact->pos_icon = *rc;
		break;
	case CIT_TIME:
		contact->pos_contact_time = *rc;
		break;
	case CIT_TEXT:
		//	RECT pos_label;//TODO label ( CIT_TEXT??? )
		//	RECT pos_rename; //TODO		( CIT_TEXT??? )
		break;
	case CIT_CHECKBOX:
		break;
	default:
		if ((ItemType & CIT_EXTRA) == CIT_EXTRA) {
			int iImage = ItemType & 0x3F;
			contact->pos_extra[iImage] = *rc;
		}
		break;
	}
}

void CLCPaint::_CalcItemsPos(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *in_row_rc, RECT *in_free_row_rc, int left_pos, int right_pos, int selected)
{
	ClcCacheEntry *pdnce = Drawing->pce;
	int item_iterator, item, item_text = 0, text_left_pos;
	BOOL left = TRUE;               //TODO remove
	RECT free_row_rc = *in_free_row_rc;
	RECT row_rc = *in_row_rc;
	Drawing->ext_nItemsNum = 0;
	text_left_pos = row_rc.right;
	// Now let's check what we have to draw
	for (item_iterator = 0; item_iterator < NUM_ITEM_TYPE && free_row_rc.left < free_row_rc.right; item_iterator++) {
		if (Drawing->ext_nItemsNum >= _countof(Drawing->ext_mpItemsDesc)) break;
		if (left)
			item = item_iterator;
		else
			item = NUM_ITEM_TYPE - (item_iterator - item_text);

		switch (dat->row_items[item]) {
		case ITEM_AVATAR: ///////////////////////////////////////////////////////////////////////////////////////////////////
			{
				int max_width;
				int width;
				int height;
				BOOL miniMode;
				if (!dat->avatars_show || Drawing->type != CLCIT_CONTACT)
					break;
				miniMode = CheckMiniMode(dat, selected);
				AniAva_InvalidateAvatarPositions(Drawing->hContact);
				if (dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space)
					max_width = max(dat->iconXSpace, dat->avatars_maxheight_size);
				else
					max_width = dat->avatars_maxheight_size;

				// Has to draw?
				if (Drawing->avatar_data == nullptr || miniMode) {
					// Don't have to draw avatar

					// Has to draw icon instead?
					if (dat->icon_hide_on_avatar && dat->icon_draw_on_avatar_space && Drawing->iImage != -1) {
						// Make rectangle
						RECT rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, dat->iconXSpace, max_width, ICON_HEIGHT, HORIZONTAL_SPACE);
						if (rc.left < rc.right) {
							// center icon in avatar place
							if (rc.right - rc.left > 16) rc.left += (((rc.right - rc.left) - 16) >> 1);
							if (rc.bottom - rc.top > 16) rc.top += (((rc.bottom - rc.top) - 16) >> 1);

							// Store position
							_StoreItemPos(Drawing, CIT_ICON, &rc);
						}
					}
					else {
						// Has to keep the empty space??
						if ((left && !dat->row_align_left_items_to_left) || (!left && !dat->row_align_right_items_to_right)) {
							// Make rectangle
							// NOTE: Calling _GetRectangle on next line has to be here, because it provides side-effect for positioning contact name correctly if contact has no avatar.
							// It makes it aligned same way as if avatar was here. Just defining rc variable is useless, as it is never used (and just shows warning).
							/*RECT rc =*/ _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, max_width, max_width, dat->avatars_maxheight_size, HORIZONTAL_SPACE);

							// Store position
							//StoreItemPos( Drawing, CIT_AVATAR, &rc );
							//Drawing->pos_avatar = rc;
						}
					}
					break;
				}

				// Has to draw avatar
				if (dat->avatar_cache.nodes && Drawing->avatar_pos > AVATAR_POS_DONT_HAVE) {
					width = dat->avatar_cache.nodes[Drawing->avatar_pos].width;
					height = dat->avatar_cache.nodes[Drawing->avatar_pos].height;
				}
				else if (Drawing->avatar_pos == AVATAR_POS_ANIMATED) {
					width = Drawing->avatar_size.cx;
					height = Drawing->avatar_size.cy;
				}
				else {
					width = 0;
					height = 0;
				}

				// Make rectangle
				RECT rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, width, max_width, height, HORIZONTAL_SPACE);

				rc.top = max(free_row_rc.top, rc.top);
				rc.bottom = min(free_row_rc.bottom, rc.bottom);

				if (rc.left < rc.right) // Store position
					_StoreItemPos(Drawing, CIT_AVATAR, &rc);
				//TO DO: CALC avatar overlays
			}
			break;

		case ITEM_ICON: /////////////////////////////////////////////////////////////////////////////////////////////////////
			{
				int iImage = -1;
				BOOL has_avatar = Drawing->avatar_data != nullptr && !CheckMiniMode(dat, selected);

				if (Drawing->type == CLCIT_CONTACT
					&& dat->icon_hide_on_avatar
					&& !dat->icon_draw_on_avatar_space
					&& has_avatar
					&& !Drawing->bImageIsSpecial) {
					// Don't have to draw, but has to keep the empty space?
					if ((left && !dat->row_align_left_items_to_left) || (!left && !dat->row_align_right_items_to_right)) {
						RECT rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, dat->iconXSpace, dat->iconXSpace, ICON_HEIGHT, HORIZONTAL_SPACE);
						if (rc.left < rc.right) // Store position
							_StoreItemPos(Drawing, CIT_ICON, &rc);
					}
					break;
				}
				if (Drawing->type == CLCIT_CONTACT
					&& dat->icon_hide_on_avatar
					&& dat->icon_draw_on_avatar_space
					&& (!Drawing->bImageIsSpecial || !has_avatar ||
					(dat->avatars_draw_overlay
					&& dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
					&& GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < _countof(g_pAvatarOverlayIcons)
					&& dat->avatars_overlay_type == SETTING_AVATAR_OVERLAY_TYPE_CONTACT))) {
					// Don't have to draw and don't have to keep the empty space
					break;
				}

				// Get image
				iImage = -1;
				if (Drawing->type == CLCIT_GROUP && !dat->row_hide_group_icon)
					iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
				else if (Drawing->type == CLCIT_CONTACT)
					iImage = Drawing->iImage;

				// Has image to draw?
				if (iImage != -1) {
					// Make rectangle
					RECT rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, dat->iconXSpace, dat->iconXSpace, ICON_HEIGHT, HORIZONTAL_SPACE);
					if (rc.left < rc.right) // Store position
						_StoreItemPos(Drawing, CIT_ICON, &rc);
				}
			}
			break;

		case ITEM_CONTACT_TIME: /////////////////////////////////////////////////////////////////////////////////////////////////////
			if (Drawing->type == CLCIT_CONTACT && dat->contact_time_show && pdnce->hTimeZone) {
				wchar_t szResult[80];

				if (!TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", szResult, _countof(szResult), 0)) {
					// Select font
					ChangeToFont(hdcMem, dat, FONTID_CONTACT_TIME, nullptr);

					// Get text size
					RECT rc;
					SIZE text_size;
					text_size.cy = ske_DrawText(hdcMem, szResult, (int)mir_wstrlen(szResult), &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE);
					text_size.cy = min(text_size.cy, free_row_rc.bottom - free_row_rc.top);
					text_size.cx = rc.right - rc.left;

					// Get rc
					rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, text_size.cx, text_size.cx, text_size.cy, HORIZONTAL_SPACE);
					if (rc.left < rc.right) { // Store pos
						Drawing->pos_contact_time = rc;
						_StoreItemPos(Drawing, CIT_TIME, &rc);
					}
				}
			}
			break;

		case ITEM_TEXT: /////////////////////////////////////////////////////////////////////////////////////////////////////
			// Store init text position:
			text_left_pos = left_pos;

			left_pos += MIN_TEXT_WIDTH;
			free_row_rc.left = row_rc.left + left_pos;

			item_text = item;
			left = FALSE;
			break;

		case ITEM_EXTRA_ICONS: //////////////////////////////////////////////////////////////////////////////////////////////
			// Draw extra icons
			if (!Drawing->iSubNumber || dat->dbbMetaHideExtra == 0 && dat->extraColumnsCount > 0) {
				int iImage;
				int count = 0;

				for (iImage = dat->extraColumnsCount - 1; iImage >= 0; iImage--) {
					if (Drawing->iExtraImage[iImage] != EMPTY_EXTRA_ICON || !dat->bMetaIgnoreEmptyExtra) {
						RECT rc = _GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos, left, dat->extraColumnSpacing, dat->extraColumnSpacing, ICON_HEIGHT, 0);
						if (rc.left < rc.right) {
							// Store position
							_StoreItemPos(Drawing, CIT_EXTRA | (iImage & 0x3F), &rc);
							// Drawing->pos_extra[iImage] = rc;
							count++;
						}
					}
				}
				// Add extra space
				if (count) {
					_GetRectangle(dat, &row_rc, &free_row_rc, &left_pos, &right_pos,
						left, HORIZONTAL_SPACE, HORIZONTAL_SPACE, ICON_HEIGHT, 0);
				}
			}
			break;
		}
	}
	if (text_left_pos < free_row_rc.right) {
		// Draw text
		int max_bottom_selection_border = SELECTION_BORDER;
		UINT uTextFormat = DT_NOPREFIX | /*DT_VCENTER |*/ DT_SINGLELINE | (dat->text_rtl ? DT_RTLREADING : 0) | (dat->text_align_right ? DT_RIGHT : 0);

		free_row_rc.left = text_left_pos;
		int free_height = free_row_rc.bottom - free_row_rc.top;

		// Select font
		ChangeToFont(hdcMem, dat, GetBasicFontID(Drawing), nullptr);

		// Get text size
		SIZE text_size = { 0 };
		GetTextSize(&text_size, hdcMem, free_row_rc, Drawing->szText, Drawing->ssText.plText, uTextFormat,
			dat->text_resize_smileys ? 0 : Drawing->ssText.iMaxSmileyHeight);

		// Get rect
		RECT text_rc = free_row_rc;

		free_height -= text_size.cy;
		text_rc.top += free_height >> 1;
		text_rc.bottom = text_rc.top + text_size.cy;

		if (dat->text_align_right)
			text_rc.left = max(free_row_rc.left, free_row_rc.right - text_size.cx);
		else
			text_rc.right = min(free_row_rc.right, free_row_rc.left + text_size.cx);

		RECT selection_text_rc = text_rc;
		SIZE counts_size = { 0 }, second_line_text_size = { 0 }, third_line_text_size = { 0 };

		// If group, can have the size of count
		if (Drawing->type == CLCIT_GROUP) {
			int full_text_width = text_size.cx;
			// Has to draw the count?
			wchar_t *szCounts = Clist_GetGroupCountsText(dat, Drawing);
			if (szCounts && szCounts[0]) {
				RECT space_rc = free_row_rc;
				RECT counts_rc = free_row_rc;

				free_height = free_row_rc.bottom - free_row_rc.top;

				// Get widths
				ske_DrawText(hdcMem, L" ", 1, &space_rc, DT_CALCRECT | DT_NOPREFIX);

				SIZE space_size;
				space_size.cx = space_rc.right - space_rc.left;
				space_size.cy = min(space_rc.bottom - space_rc.top, free_height);

				ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
				DrawText(hdcMem, szCounts, (int)mir_wstrlen(szCounts), &counts_rc, DT_CALCRECT);

				counts_size.cx = counts_rc.right - counts_rc.left;
				counts_size.cy = min(counts_rc.bottom - counts_rc.top, free_height);
				counts_size.cx++; // FIXME: This is workaround for sometimes stripped user counts by one pixel, it somehow wrongly counts width of space or counts text (used wrong font?)

				int text_width = free_row_rc.right - free_row_rc.left - space_size.cx - counts_size.cx;
				if (text_width > 4) {
					text_size.cx = min(text_width, text_size.cx);
					text_width = text_size.cx + space_size.cx + counts_size.cx;
				}
				else {
					text_width = text_size.cx;
					space_size.cx = 0;
					space_size.cy = 0;
					counts_size.cx = 0;
					counts_size.cy = 0;
				}

				// Get rect
				free_height -= max(text_size.cy, counts_size.cy);
				text_rc.top = free_row_rc.top + (free_height >> 1);
				text_rc.bottom = text_rc.top + max(text_size.cy, counts_size.cy);

				if (dat->text_align_right)
					text_rc.left = free_row_rc.right - text_width;
				else
					text_rc.right = free_row_rc.left + text_width;

				selection_text_rc = text_rc;
				full_text_width = text_width;
				ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPS : FONTID_CLOSEDGROUPS, nullptr);
			}

			if (dat->row_align_group_mode == 1) { // center
				int x = free_row_rc.left + ((free_row_rc.right - free_row_rc.left - full_text_width) >> 1);
				text_rc.left = x;
				text_rc.right = x + full_text_width;
			}
			else if (dat->row_align_group_mode == 2) { // right
				text_rc.left = free_row_rc.right - full_text_width;
				text_rc.right = free_row_rc.right;
			}
			else { // left
				text_rc.left = free_row_rc.left;
				text_rc.right = free_row_rc.left + full_text_width;
			}
		}
		else if (Drawing->type == CLCIT_CONTACT && !CheckMiniMode(dat, selected)) {
			if (g_plugin.secondLine.bActive) {
				if (g_plugin.secondLine.iType == TEXT_CONTACT_TIME && pdnce->hTimeZone) {
					// Get contact time
					wchar_t buf[70] = L"";
					TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", buf, _countof(buf), 0);
					replaceStrW(pdnce->szSecondLineText, buf);
				}

				if (pdnce->szSecondLineText && pdnce->szSecondLineText[0] && free_height > g_plugin.secondLine.iTopSpace) {
					ChangeToFont(hdcMem, dat, FONTID_SECONDLINE, nullptr);

					// Get sizes
					GetTextSize(&second_line_text_size, hdcMem, free_row_rc, pdnce->szSecondLineText, pdnce->ssSecondLine.plText,
						uTextFormat, dat->text_resize_smileys ? 0 : pdnce->ssSecondLine.iMaxSmileyHeight);

					// Get rect
					int tmp = min(free_height, g_plugin.secondLine.iTopSpace + second_line_text_size.cy);

					free_height -= tmp;
					text_rc.top = free_row_rc.top + (free_height >> 1);
					text_rc.bottom = text_rc.top + free_row_rc.bottom - free_row_rc.top - free_height;

					if (dat->text_align_right)
						text_rc.left = max(free_row_rc.left, min(text_rc.left, free_row_rc.right - second_line_text_size.cx));
					else
						text_rc.right = min(free_row_rc.right, max(text_rc.right, free_row_rc.left + second_line_text_size.cx));

					selection_text_rc.top = text_rc.top;
					selection_text_rc.bottom = min(selection_text_rc.bottom, selection_text_rc.top + text_size.cy);

					max_bottom_selection_border = min(max_bottom_selection_border, g_plugin.secondLine.iTopSpace / 2);
				}
			}
			if (g_plugin.thirdLine.bActive) {
				if (g_plugin.thirdLine.iType == TEXT_CONTACT_TIME && pdnce->hTimeZone) {
					// Get contact time
					wchar_t buf[70] = L"";
					TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", buf, _countof(buf), 0);
					replaceStrW(pdnce->szThirdLineText, buf);
				}
				if (pdnce->szThirdLineText != nullptr && pdnce->szThirdLineText[0] && free_height > g_plugin.thirdLine.iTopSpace) {
					ChangeToFont(hdcMem, dat, FONTID_THIRDLINE, nullptr);

					// Get sizes
					GetTextSize(&third_line_text_size, hdcMem, free_row_rc, pdnce->szThirdLineText, pdnce->ssThirdLine.plText,
						uTextFormat, dat->text_resize_smileys ? 0 : pdnce->ssThirdLine.iMaxSmileyHeight);

					// Get rect
					int tmp = min(free_height, g_plugin.thirdLine.iTopSpace + third_line_text_size.cy);

					free_height -= tmp;
					text_rc.top = free_row_rc.top + (free_height >> 1);
					text_rc.bottom = text_rc.top + free_row_rc.bottom - free_row_rc.top - free_height;

					if (dat->text_align_right)
						text_rc.left = max(free_row_rc.left, min(text_rc.left, free_row_rc.right - third_line_text_size.cx));
					else
						text_rc.right = min(free_row_rc.right, max(text_rc.right, free_row_rc.left + third_line_text_size.cx));

					selection_text_rc.top = text_rc.top;
					selection_text_rc.bottom = min(selection_text_rc.bottom, selection_text_rc.top + text_size.cy);

					max_bottom_selection_border = min(max_bottom_selection_border, g_plugin.thirdLine.iTopSpace / 2);
				}
			}

			ChangeToFont(hdcMem, dat, GetBasicFontID(Drawing), nullptr);
		}

		selection_text_rc.left = text_rc.left;
		selection_text_rc.right = text_rc.right;

		Drawing->pos_label = text_rc;

		selection_text_rc.top = max(selection_text_rc.top - SELECTION_BORDER, row_rc.top);
		selection_text_rc.bottom = min(selection_text_rc.bottom + max_bottom_selection_border, row_rc.bottom);

		if (dat->text_align_right)
			selection_text_rc.left = max(selection_text_rc.left - SELECTION_BORDER, free_row_rc.left);
		else
			selection_text_rc.right = min(selection_text_rc.right + SELECTION_BORDER, free_row_rc.right);
		_StoreItemPos(Drawing, CIT_SELECTION, &selection_text_rc);

		// Draw text
		uTextFormat = uTextFormat | (g_bTrimText ? DT_END_ELLIPSIS : 0);

		switch (Drawing->type) {
		case CLCIT_DIVIDER:
			{
				//devider
				RECT trc = free_row_rc;
				RECT rc = free_row_rc;
				rc.top += (rc.bottom - rc.top) >> 1;
				rc.bottom = rc.top + 2;
				rc.right = rc.left + ((rc.right - rc.left - text_size.cx) >> 1) - 3;
				trc.left = rc.right + 3;
				trc.right = trc.left + text_size.cx + 6;
				if (text_size.cy < trc.bottom - trc.top) {
					trc.top += (trc.bottom - trc.top - text_size.cy) >> 1;
					trc.bottom = trc.top + text_size.cy;
				}
				_StoreItemPos(Drawing, CIT_TEXT, &trc);
				rc.left = rc.right + 6 + text_size.cx;
				rc.right = free_row_rc.right;
			}
			break;
		case CLCIT_GROUP:
			{
				RECT rc = text_rc;

				// Get text rectangle
				if (dat->text_align_right)
					rc.left = rc.right - text_size.cx;
				else
					rc.right = rc.left + text_size.cx;

				if (text_size.cy < rc.bottom - rc.top) {
					rc.top += (rc.bottom - rc.top - text_size.cy) >> 1;
					rc.bottom = rc.top + text_size.cy;
				}

				// Draw text
				_StoreItemPos(Drawing, CIT_TEXT, &rc);

				// Has to draw the count?
				if (counts_size.cx > 0) {
					RECT counts_rc = text_rc;
					//counts_size.cx;
					if (dat->text_align_right)
						counts_rc.right = text_rc.left + counts_size.cx;
					else
						counts_rc.left = text_rc.right - counts_size.cx;

					if (counts_size.cy < counts_rc.bottom - counts_rc.top) {
						counts_rc.top += (counts_rc.bottom - counts_rc.top - counts_size.cy + 1) >> 1;
						counts_rc.bottom = counts_rc.top + counts_size.cy;
					}
					// Draw counts
					_StoreItemPos(Drawing, CIT_SUBTEXT1, &counts_rc);
				}
			}
			break;
		case CLCIT_CONTACT:
			{
				RECT free_rc = text_rc;
				if (text_size.cx > 0 && free_rc.bottom > free_rc.top) {
					RECT rc = free_rc;
					rc.bottom = min(rc.bottom, rc.top + text_size.cy);

					if (text_size.cx < rc.right - rc.left) {
						if (dat->text_align_right)
							rc.left = rc.right - text_size.cx;
						else
							rc.right = rc.left + text_size.cx;
					}
					uTextFormat |= DT_VCENTER;
					_StoreItemPos(Drawing, CIT_TEXT, &rc);
					free_rc.top = rc.bottom;
				}
				uTextFormat &= ~DT_VCENTER;
				if (second_line_text_size.cx > 0 && free_rc.bottom > free_rc.top) {
					free_rc.top += g_plugin.secondLine.iTopSpace;

					if (free_rc.bottom > free_rc.top) {
						RECT rc = free_rc;
						rc.bottom = min(rc.bottom, rc.top + second_line_text_size.cy);

						if (second_line_text_size.cx < rc.right - rc.left) {
							if (dat->text_align_right)
								rc.left = rc.right - second_line_text_size.cx;
							else
								rc.right = rc.left + second_line_text_size.cx;
						}
						_StoreItemPos(Drawing, CIT_SUBTEXT1, &rc);
						free_rc.top = rc.bottom;
					}
				}

				if (third_line_text_size.cx > 0 && free_rc.bottom > free_rc.top) {
					free_rc.top += g_plugin.thirdLine.iTopSpace;

					if (free_rc.bottom > free_rc.top) {
						RECT rc = free_rc;
						rc.bottom = min(rc.bottom, rc.top + third_line_text_size.cy);

						if (third_line_text_size.cx < rc.right - rc.left) {
							if (dat->text_align_right)
								rc.left = rc.right - third_line_text_size.cx;
							else
								rc.right = rc.left + third_line_text_size.cx;
						}
						_StoreItemPos(Drawing, CIT_SUBTEXT2, &rc);
						free_rc.top = rc.bottom;
					}
				}
			}
			break;

		default: // CLCIT_INFO
			_StoreItemPos(Drawing, CIT_TEXT, &text_rc);
		}
	}

	*in_free_row_rc = free_row_rc;
	*in_row_rc = row_rc;
	Drawing->ext_fItemsValid = FALSE; /*TO DO: correctly implement placement recalculation*/
}

BOOL CLCPaint::__IsVisible(RECT *firtRect, RECT *secondRect)
{
	RECT res;
	IntersectRect(&res, firtRect, secondRect);
	return !IsRectEmpty(&res);
}

void CLCPaint::_GetBlendMode(IN ClcData *dat, IN ClcContact *Drawing, IN BOOL selected, IN BOOL hottrack, IN BOOL bFlag, OUT COLORREF *OutColourFg, OUT int *OutMode)
{
	COLORREF colourFg;
	int mode;
	int BlendedInActiveState = (dat->dbbBlendInActiveState);
	int BlendValue = dat->dbbBlend25 ? ILD_BLEND25 : ILD_BLEND50;
	if (selected && (bFlag & GIM_SELECTED_AFFECT)) {
		colourFg = dat->selBkColour;
		mode = BlendedInActiveState ? ILD_NORMAL : ILD_SELECTED;
	}
	else if (hottrack && (bFlag & GIM_HOT_AFFECT)) {
		mode = BlendedInActiveState ? ILD_NORMAL : ILD_FOCUS;
		colourFg = dat->hotTextColour;
	}
	else if (Drawing->type == CLCIT_CONTACT && Drawing->flags & CONTACTF_NOTONLIST && (bFlag & GIM_TEMP_AFFECT)) {
		colourFg = dat->fontModernInfo[FONTID_NOTONLIST].colour;
		mode = BlendValue;
	}
	else {
		colourFg = dat->selBkColour;
		mode = ILD_NORMAL;
	}
	if (Drawing->type == CLCIT_CONTACT && dat->bShowIdle && (Drawing->flags & CONTACTF_IDLE) &&
		Clist_GetRealStatus(Drawing, ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE &&
		(bFlag & GIM_IDLE_AFFECT))
		mode = ILD_SELECTED;
	if (OutColourFg) *OutColourFg = colourFg;
	if (OutMode) {
		if (OutColourFg)    *OutMode = mode;  //return ILD_MODE if color requested
		else *OutMode =
			(mode == ILD_BLEND50) ? 128 :
			(mode == ILD_BLEND25) ? 64 :
			255;    //return alpha otherwise
	}
}

void CLCPaint::_DrawContactAvatar(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *row_rc, int &selected, int &hottrack, RECT *prcItem)
{
	if (Drawing->avatar_pos == AVATAR_POS_ANIMATED) {
		int overlayIdx = -1;
		int blendmode = 255;
		if (dat->avatars_draw_overlay && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
			&& GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < _countof(g_pAvatarOverlayIcons)) {
			switch (dat->avatars_overlay_type) {
			case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
				overlayIdx = g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].listID;
				break;
			case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
				overlayIdx = g_clistApi.pfnIconFromStatusMode(Drawing->pce->szProto, Drawing->pce->szProto == nullptr ? ID_STATUS_OFFLINE : GetContactCachedStatus(Drawing->hContact), Drawing->hContact);
				break;
			case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
				overlayIdx = Drawing->iImage;
				break;
			}
		}
		_GetBlendMode(dat, Drawing, selected, hottrack, GIM_AVATAR_AFFECT, nullptr, &blendmode);
		AniAva_SetAvatarPos(Drawing->hContact, prcItem, overlayIdx, blendmode);
		AniAva_RenderAvatar(Drawing->hContact, hdcMem, prcItem);
	}
	else if (Drawing->avatar_pos > AVATAR_POS_DONT_HAVE) {
		int round_radius = 0;
		HRGN rgn;
		int blendmode = 255;

		_GetBlendMode(dat, Drawing, selected, hottrack, GIM_AVATAR_AFFECT, nullptr, &blendmode);

		//get round corner radius
		if (dat->avatars_round_corners) {
			if (dat->avatars_use_custom_corner_size)
				round_radius = dat->avatars_custom_corner_size;
			else
				round_radius = min(_rcWidth(prcItem), _rcHeight(prcItem)) / 5;
		}
		// draw borders
		if (dat->avatars_draw_border) {
			HBRUSH hBrush = CreateSolidBrush(dat->avatars_border_color);
			HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
			HRGN rgnOutside = CreateRoundRectRgn(prcItem->left, prcItem->top, prcItem->right + 1, prcItem->bottom + 1, round_radius << 1, round_radius << 1);
			HRGN rgnInside = CreateRoundRectRgn(prcItem->left + 1, prcItem->top + 1, prcItem->right, prcItem->bottom, round_radius << 1, round_radius << 1);
			CombineRgn(rgnOutside, rgnOutside, rgnInside, RGN_DIFF);
			FillRgn(hdcMem, rgnOutside, hBrush);
			ske_SetRgnOpaque(hdcMem, rgnOutside);
			SelectObject(hdcMem, hOldBrush);
			DeleteObject(hBrush);
			DeleteObject(rgnInside);
			DeleteObject(rgnOutside);
		}

		//set clip area to clip avatars within borders
		if (dat->avatars_round_corners || dat->avatars_draw_border) {
			int k = dat->avatars_draw_border ? 1 : 0;
			rgn = CreateRoundRectRgn(prcItem->left + k, prcItem->top + k, prcItem->right + 1 - k, prcItem->bottom + 1 - k, round_radius * 2, round_radius * 2);
		}
		else rgn = CreateRectRgn(prcItem->left, prcItem->top, prcItem->right, prcItem->bottom);
		ExtSelectClipRgn(hdcMem, rgn, RGN_AND);

		// Draw avatar
		ImageArray_DrawImage(&dat->avatar_cache, Drawing->avatar_pos, hdcMem, prcItem->left, prcItem->top, blendmode);

		// Restore region
		DeleteObject(rgn);
		rgn = CreateRectRgn(row_rc->left, row_rc->top, row_rc->right, row_rc->bottom);
		SelectClipRgn(hdcMem, rgn);
		DeleteObject(rgn);
		// Draw overlays
		if (dat->avatars_draw_overlay && dat->avatars_maxheight_size >= ICON_HEIGHT + (dat->avatars_draw_border ? 2 : 0)
			&& GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE < _countof(g_pAvatarOverlayIcons)) {
			POINT ptOverlay = { prcItem->right - ICON_HEIGHT, prcItem->bottom - ICON_HEIGHT };
			if (dat->avatars_draw_border) {
				ptOverlay.x--;
				ptOverlay.y--;
			}
			switch (dat->avatars_overlay_type) {
			case SETTING_AVATAR_OVERLAY_TYPE_NORMAL:
				ske_ImageList_DrawEx(hAvatarOverlays, g_pAvatarOverlayIcons[GetContactCachedStatus(Drawing->hContact) - ID_STATUS_OFFLINE].listID,
					hdcMem,
					ptOverlay.x, ptOverlay.y, ICON_HEIGHT, ICON_HEIGHT,
					CLR_NONE, CLR_NONE,
					(blendmode == 255) ? ILD_NORMAL : (blendmode == 128) ? ILD_BLEND50 : ILD_BLEND25);
				break;
			case SETTING_AVATAR_OVERLAY_TYPE_PROTOCOL:
				{
					int item = g_clistApi.pfnIconFromStatusMode(Drawing->pce->szProto, Drawing->pce->szProto == nullptr ? ID_STATUS_OFFLINE : GetContactCachedStatus(Drawing->hContact), Drawing->hContact);
					if (item != -1)
						_DrawStatusIcon(Drawing, dat, item, hdcMem,
							ptOverlay.x, ptOverlay.y, ICON_HEIGHT, ICON_HEIGHT,
							CLR_NONE, CLR_NONE, (blendmode == 255) ? ILD_NORMAL : (blendmode == 128) ? ILD_BLEND50 : ILD_BLEND25);
				}
				break;
			case SETTING_AVATAR_OVERLAY_TYPE_CONTACT:
				if (Drawing->iImage != -1)
					_DrawStatusIcon(Drawing, dat, Drawing->iImage, hdcMem,
						ptOverlay.x, ptOverlay.y, ICON_HEIGHT, ICON_HEIGHT,
						CLR_NONE, CLR_NONE, (blendmode == 255) ? ILD_NORMAL : (blendmode == 128) ? ILD_BLEND50 : ILD_BLEND25);
				break;
			}
		}
	}
}

void CLCPaint::_DrawContactIcon(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int &selected, int &hottrack, RECT *prcItem)
{
	// Draw Icon
	int iImage = -1;
	// Get image
	if (Drawing->type == CLCIT_GROUP) {
		if (!dat->row_hide_group_icon) iImage = Drawing->group->expanded ? IMAGE_GROUPOPEN : IMAGE_GROUPSHUT;
		else iImage = -1;
	}
	else if (Drawing->type == CLCIT_CONTACT)
		iImage = Drawing->iImage;

	// Has image to draw?
	if (iImage != -1) {
		COLORREF colourFg;
		int mode;
		_GetBlendMode(dat, Drawing, selected, hottrack, GIM_STATUSICON_AFFECT, &colourFg, &mode);
		_DrawStatusIcon(Drawing, dat, iImage, hdcMem, prcItem->left, prcItem->top, 0, 0, CLR_NONE, colourFg, mode);
	}
}

void CLCPaint::_DrawContactText(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int &selected, int &hottrack, RECT &text_rc, RECT *prcItem, UINT uTextFormat)
{
	ChangeToFont(hdcMem, dat, GetBasicFontID(Drawing), nullptr);
	if (selected)
		SetTextColor(hdcMem, dat->bForceInDialog ? GetSysColor(COLOR_HIGHLIGHTTEXT) : dat->selTextColour);
	else if (hottrack || (dat->bFilterSearch && dat->szQuickSearch[0] != '\0' && Drawing->type != CLCIT_GROUP))
		_SetHotTrackColour(hdcMem, dat);

	if (Drawing->type == CLCIT_GROUP) {
		ske_DrawText(hdcMem, Drawing->szText, -1, prcItem, uTextFormat);
		if (selected && !dat->bFilterSearch) {
			SetTextColor(hdcMem, dat->quickSearchColour);
			ske_DrawText(hdcMem, Drawing->szText, (int)mir_wstrlen(dat->szQuickSearch), prcItem, uTextFormat);
		}
	}
	else if (Drawing->type == CLCIT_CONTACT) {
		SIZE text_size;
		text_size.cx = _rcWidth(prcItem);
		text_size.cy = _rcHeight(prcItem);
		uTextFormat |= DT_VCENTER;
		//get font
		_DrawTextSmiley(hdcMem, prcItem, &text_size, Drawing->szText, 0, -1, Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);
		if ((dat->bFilterSearch || selected) && dat->szQuickSearch[0] != '\0') {
			int idx = 0;
			if (dat->bFilterSearch) {
				wchar_t *lowered = CharLowerW(NEWWSTR_ALLOCA(Drawing->szText));
				wchar_t *lowered_search = CharLowerW(NEWWSTR_ALLOCA(dat->szQuickSearch));
				wchar_t *p1 = wcsstr(lowered, lowered_search);
				if (p1)
					idx = int(p1 - lowered);
			}
			SetTextColor(hdcMem, dat->quickSearchColour);
			_DrawTextSmiley(hdcMem, prcItem, &text_size, Drawing->szText, idx, (int)mir_wstrlen(dat->szQuickSearch), Drawing->ssText.plText, uTextFormat, dat->text_resize_smileys);
		}
	}
	else ske_DrawText(hdcMem, Drawing->szText, -1, prcItem, uTextFormat);

	text_rc.right = max(text_rc.right, prcItem->right);
	text_rc.left = min(text_rc.left, prcItem->left);
}

void CLCPaint::_DrawContactSubText(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int &selected, int &hottrack, RECT &text_rc, RECT *prcItem, UINT uTextFormat, uint8_t itemType)
{
	if (Drawing->type == CLCIT_GROUP) {
		wchar_t *szCounts = Clist_GetGroupCountsText(dat, Drawing);

		// Has to draw the count?
		if (szCounts && szCounts[0]) {
			ChangeToFont(hdcMem, dat, Drawing->group->expanded ? FONTID_OPENGROUPCOUNTS : FONTID_CLOSEDGROUPCOUNTS, nullptr);
			if (selected)
				SetTextColor(hdcMem, dat->selTextColour);
			else if (hottrack)
				_SetHotTrackColour(hdcMem, dat);
			ske_DrawText(hdcMem, szCounts, -1, prcItem, uTextFormat);
			ske_ResetTextEffect(hdcMem);
		}
	}
	else if (Drawing->type == CLCIT_CONTACT) {
		ClcCacheEntry *pdnce = Drawing->pce;
		SIZE text_size = { _rcWidth(prcItem), _rcHeight(prcItem) };
		ChangeToFont(hdcMem, dat, itemType == CIT_SUBTEXT1 ? FONTID_SECONDLINE : FONTID_THIRDLINE, nullptr);
		//draw second and third line
		if (selected)
			SetTextColor(hdcMem, dat->selTextColour);
		else if (hottrack)
			_SetHotTrackColour(hdcMem, dat);
		uTextFormat |= DT_VCENTER;
		if (itemType == CIT_SUBTEXT1)
			_DrawTextSmiley(hdcMem, prcItem, &text_size, pdnce->szSecondLineText, 0, -1, pdnce->ssSecondLine.plText, uTextFormat, dat->text_resize_smileys);
		else
			_DrawTextSmiley(hdcMem, prcItem, &text_size, pdnce->szThirdLineText, 0, -1, pdnce->ssThirdLine.plText, uTextFormat, dat->text_resize_smileys);
	}
	text_rc.right = max(text_rc.right, prcItem->right);
	text_rc.left = min(text_rc.left, prcItem->left);
}

void CLCPaint::_DrawContactTime(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *prcItem)
{
	wchar_t szResult[80];
	ClcCacheEntry *pdnce = Drawing->pce;
	if (!pdnce)
		return;

	if (!TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", szResult, _countof(szResult), 0)) {
		// Select font
		ChangeToFont(hdcMem, dat, FONTID_CONTACT_TIME, nullptr);
		ske_DrawText(hdcMem, szResult, (int)mir_wstrlen(szResult), prcItem, DT_NOPREFIX | DT_SINGLELINE);
	}
}

void CLCPaint::_DrawContactSelection(HDC hdcMem, ClcData *dat, int &selected, int &hottrack, RECT *rcPaint, RECT *prcItem)
{
	// Selection background
	if ((selected || hottrack) && dat->HiLightMode == 0) {
		if (selected)
			SkinDrawGlyph(hdcMem, prcItem, rcPaint, "Contact List/Selection");
		else if (hottrack)
			SkinDrawGlyph(hdcMem, prcItem, rcPaint, "Contact List/HotTracking");
	}
}

void CLCPaint::_DrawContactExtraIcon(HDC hdcMem, ClcData *dat, ClcContact *Drawing, int &selected, int &hottrack, RECT *rc, int iImage)
{
	//Draw extra icon
	if (iImage != -1) {
		int mode;
		COLORREF colourFg;
		_GetBlendMode(dat, Drawing, selected, hottrack, GIM_EXTRAICON_AFFECT, &colourFg, &mode);
		if (Drawing->iExtraImage[iImage] != EMPTY_EXTRA_ICON)
			ske_ImageList_DrawEx(dat->himlExtraColumns, Drawing->iExtraImage[iImage], hdcMem, rc->left, rc->top, 0, 0, CLR_NONE, colourFg, mode);
	}
}

void CLCPaint::_DrawContactLine(HDC hdcMem, ClcContact *Drawing, RECT *free_row_rc, RECT *rcPaint, RECT &text_rc)
{   //draw line
	RECT rc1 = *free_row_rc;
	RECT rc2 = *free_row_rc;
	rc1.right = text_rc.left - 3;
	rc2.left = text_rc.right + 3;
	rc1.top += (rc1.bottom - rc1.top) >> 1;
	rc1.bottom = rc1.top + 2;
	rc2.top += (rc2.bottom - rc2.top) >> 1;
	rc2.bottom = rc2.top + 2;
	{
		RECT rcTemp = rc1;
		IntersectRect(&rc1, &rcTemp, rcPaint);
	}
	{	//Subtract icon rect from left and right.
		RECT rcTemp;
		IntersectRect(&rcTemp, &Drawing->pos_icon, &rc1);
		if (!IsRectEmpty(&rcTemp)) rc1.right = rcTemp.left;
		IntersectRect(&rcTemp, &Drawing->pos_icon, &rc2);
		if (!IsRectEmpty(&rcTemp)) rc2.left = rcTemp.right;
	}

	if (rc1.right - rc1.left >= 6 && !IsRectEmpty(&rc1)) {
		DrawEdge(hdcMem, &rc1, BDR_SUNKENOUTER, BF_RECT);
		ske_SetRectOpaque(hdcMem, &rc1);
	}
	{
		RECT rcTemp = rc2;
		IntersectRect(&rc2, &rcTemp, rcPaint);
	}
	if (rc2.right - rc2.left >= 6 && !IsRectEmpty(&rc2)) {
		DrawEdge(hdcMem, &rc2, BDR_SUNKENOUTER, BF_RECT);
		ske_SetRectOpaque(hdcMem, &rc2);
	}
}

void CLCPaint::_DrawContactItems(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT *row_rc, RECT *free_row_rc, int selected, int hottrack, RECT *rcPaint)
{
	UINT uTextFormat = DT_NOPREFIX |
		/*DT_VCENTER |*/
		DT_SINGLELINE |
		(dat->text_rtl ? DT_RTLREADING : 0) |
		(dat->text_align_right ? DT_RIGHT : 0) |
		(g_bTrimText ? DT_END_ELLIPSIS : 0) |
		((dat->bForceInDialog || dat->bkChanged) ? DT_FORCENATIVERENDER : 0);

	RECT text_rc = *row_rc;

	text_rc.right = row_rc->left;
	text_rc.left = row_rc->right;

	for (int i = 0; i < Drawing->ext_nItemsNum; i++) {
		RECT *prcItem = &(Drawing->ext_mpItemsDesc[i].itemRect);
		if (__IsVisible(rcPaint, prcItem) || (Drawing->ext_mpItemsDesc[i].itemType == CIT_AVATAR && Drawing->avatar_pos == AVATAR_POS_ANIMATED)) {
			switch (Drawing->ext_mpItemsDesc[i].itemType) {
			case CIT_AVATAR:
				_DrawContactAvatar(hdcMem, dat, Drawing, row_rc, selected, hottrack, prcItem);
				break;
			case CIT_ICON:
				_DrawContactIcon(hdcMem, dat, Drawing, selected, hottrack, prcItem);
				break;
			case CIT_TEXT:
				_DrawContactText(hdcMem, dat, Drawing, selected, hottrack, text_rc, prcItem, uTextFormat);
				break;
			case CIT_SUBTEXT1:
			case CIT_SUBTEXT2:
				_DrawContactSubText(hdcMem, dat, Drawing, selected, hottrack, text_rc, prcItem, uTextFormat, Drawing->ext_mpItemsDesc[i].itemType);
				break;
			case CIT_TIME:
				_DrawContactTime(hdcMem, dat, Drawing, prcItem);
				break;
			case CIT_CHECKBOX:
				//_DrawNothing no check boxes in skinned mode
				break;
				//other here
			case CIT_SELECTION:
				_DrawContactSelection(hdcMem, dat, selected, hottrack, rcPaint, prcItem);
				break;
			default:
				if (Drawing->ext_mpItemsDesc[i].itemType & CIT_EXTRA)
					_DrawContactExtraIcon(hdcMem, dat, Drawing, selected, hottrack, prcItem, Drawing->ext_mpItemsDesc[i].itemType & 0x3F);
				break;
			}
		}
	}
	if ((Drawing->type == CLCIT_GROUP && dat->exStyle & CLS_EX_LINEWITHGROUPS)
		|| (Drawing->type == CLCIT_DIVIDER))
		_DrawContactLine(hdcMem, Drawing, free_row_rc, rcPaint, text_rc);
}

void CLCPaint::_PaintRowItems(HDC hdcMem, ClcData *dat, ClcContact *Drawing, RECT row_rc, RECT free_row_rc, int left_pos, int right_pos, int selected, int hottrack, RECT *rcPaint)
{
	// Extended LAYOUT
	if (gl_RowRoot && (dat->hWnd == g_clistApi.hwndContactTree)) {
		_PaintRowItemsEx(hdcMem, dat, Drawing, row_rc, free_row_rc, selected, hottrack);
		ske_ResetTextEffect(hdcMem);
		return;
	}
	// END OFF Extended LAYOUT
	if (!Drawing->ext_fItemsValid) _CalcItemsPos(hdcMem, dat, Drawing, &row_rc, &free_row_rc, left_pos, right_pos, selected);
	_DrawContactItems(hdcMem, dat, Drawing, &row_rc, &free_row_rc, selected, hottrack, rcPaint);
	ske_ResetTextEffect(hdcMem);
}

/* TODO Render items

V avatar
V avatars overlays
V avatar clipping if ignore avatar height for row height calculation is set
V icon
V text
V time
V extra icons
V selection/hot
V groups and divider lines

Milestones to implement Extended row layout

-   1. Implement separate Row item position calculation and drawing
V       a. Separation of painting and positions calculation
V       b. Use Items rect array for hit test
.       c. Calculate row height via appropriate function
.       d. Invalidate row items only when needed

.   2. Implement extended row item layout
.       a. layout template parsing
.       b. calculate positions according to template
.       c. GUI to modify template
.       d. skin defined template
*/
