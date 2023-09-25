/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "mir_smileys.h"

#include <richedit.h>

#include <m_smileyadd.h>
#include <newpluginapi.h>
#include <m_langpack.h>
#include <m_database.h>
#include <commctrl.h>
#include <m_skin_eng.h>

#include <tchar.h>

// Prototypes

#define TEXT_PIECE_TYPE_TEXT   0
#define TEXT_PIECE_TYPE_SMILEY 1
typedef struct
{
	int type;
	int len;
	union
	{
		struct
		{
			int start_pos;
		};
		struct
		{
			HICON smiley;
			int smiley_width;
			int smiley_height;
		};
	};
}
TextPiece;

SortedList * ReplaceSmileys(const wchar_t *text, int text_size, const char *protocol, int *max_smiley_height);
void DrawTextSmiley(HDC hdcMem, RECT free_rc, const wchar_t *szText, int len, SortedList *plText, UINT uTextFormat, int max_smiley_height);
void DestroySmileyList(SortedList* p_list);
SIZE GetTextSize(HDC hdcMem, const wchar_t *szText, SortedList *plText, UINT uTextFormat, int max_smiley_height);

// Functions

int InitContactListSmileys()
{
	// Register smiley category
	SMADD_REGCAT rc;
	rc.name = "clist";
	rc.dispname = TranslateA_LP("Contact List smileys");
	CallService(MS_SMILEYADD_REGISTERCATEGORY, 0, (LPARAM)&rc);
	return 0;
}

SmileysParseInfo Smileys_PreParse(const wchar_t* lpString, int nCount, const char *protocol)
{
	SmileysParseInfo info = (SmileysParseInfo)mir_calloc(sizeof(_SmileysParseInfo));

	info->pieces = ReplaceSmileys(lpString, nCount, protocol, &info->max_height);

	return info;
}

void Smileys_FreeParse(SmileysParseInfo parseInfo)
{
	if (parseInfo != nullptr)
	{
		if (parseInfo->pieces != nullptr)
			DestroySmileyList(parseInfo->pieces);

		mir_free(parseInfo);
	}
}

int skin_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	if ((uFormat & DT_CALCRECT) == 0 && ServiceExists(MS_SKINENG_ALPHATEXTOUT))
	{
		COLORREF color = SetTextColor(hDC, 0);
		SetTextColor(hDC, color);

		return AlphaText(hDC, lpString, nCount, lpRect, uFormat, color);
	}

	return DrawText(hDC, lpString, nCount, lpRect, uFormat);
}

int skin_DrawIconEx(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth,
	UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags)
{
	if (ServiceExists(MS_SKINENG_DRAWICONEXFIX))
		return mod_DrawIconEx_helper(hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
	else
		return DrawIconEx(hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags);
}



// Similar to DrawText win32 api function
// Pass uFormat | DT_CALCRECT to calc rectangle to be returned by lpRect
// parseInfo is optional (pass NULL and it will be calculated and deleted inside function
int Smileys_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, SmileysParseInfo parseInfo)
{
	SmileysParseInfo info;
	int ret;

	if (nCount < 0)
		nCount = (int)mir_wstrlen(lpString);

	// Get parse info
	if (parseInfo == nullptr)
		info = Smileys_PreParse(lpString, nCount, protocol);
	else
		info = parseInfo;

	if (uFormat & DT_CALCRECT)
	{
		SIZE text_size = GetTextSize(hDC, lpString, info->pieces, uFormat, info->max_height);

		lpRect->bottom = min(lpRect->bottom, lpRect->top + text_size.cy);

		if (text_size.cx < lpRect->right - lpRect->left)
		{
			if (uFormat & DT_RIGHT)
				lpRect->left = lpRect->right - text_size.cx;
			else
				lpRect->right = lpRect->left + text_size.cx;
		}

		ret = text_size.cy;
	}
	else
	{
		// Clipping rgn
		HRGN oldRgn = CreateRectRgn(0, 0, 1, 1);
		if (GetClipRgn(hDC, oldRgn) != 1)
		{
			DeleteObject(oldRgn);
			oldRgn = nullptr;
		}

		HRGN rgn = CreateRectRgnIndirect(lpRect);
		ExtSelectClipRgn(hDC, rgn, RGN_AND);

		// Draw
		if (info->pieces == nullptr)
		{
			ret = skin_DrawText(hDC, lpString, nCount, lpRect, uFormat);
		}
		else
		{
			RECT rc = *lpRect;

			SIZE text_size = GetTextSize(hDC, lpString, info->pieces, uFormat, info->max_height);

			if (text_size.cx < rc.right - rc.left)
			{
				if (uFormat & DT_RIGHT)
					rc.left = rc.right - text_size.cx;
				else
					rc.right = rc.left + text_size.cx;
			}

			ret = text_size.cy;

			DrawTextSmiley(hDC, rc, lpString, nCount, info->pieces, uFormat, info->max_height);
		}

		// Clipping rgn
		SelectClipRgn(hDC, oldRgn);
		DeleteObject(rgn);
		if (oldRgn) DeleteObject(oldRgn);
	}


	// Free parse info
	if (parseInfo == nullptr)
		Smileys_FreeParse(info);

	return ret;
}



SIZE GetTextSize(HDC hdcMem, const wchar_t *szText, SortedList *plText, UINT uTextFormat, int max_smiley_height)
{
	SIZE text_size;

	if (szText == nullptr) {
		text_size.cy = 0;
		text_size.cx = 0;
	}
	else {
		RECT text_rc = { 0, 0, 0x7FFFFFFF, 0x7FFFFFFF };

		// Always need cy...
		DrawText(hdcMem, szText, -1, &text_rc, DT_CALCRECT | uTextFormat);
		text_size.cy = text_rc.bottom - text_rc.top;

		if (plText == nullptr)
			text_size.cx = text_rc.right - text_rc.left;
		else {
			if (!(uTextFormat & DT_RESIZE_SMILEYS))
				text_size.cy = max(text_size.cy, max_smiley_height);

			text_size.cx = 0;

			// See each item of list
			for (int i = 0; i < plText->realCount; i++) {
				TextPiece *piece = (TextPiece *)plText->items[i];

				if (piece->type == TEXT_PIECE_TYPE_TEXT) {
					RECT rc = { 0, 0, 0x7FFFFFFF, 0x7FFFFFFF };

					DrawText(hdcMem, &szText[piece->start_pos], piece->len, &rc, DT_CALCRECT | uTextFormat);
					text_size.cx = text_size.cx + rc.right - rc.left;
				}
				else {
					double factor;

					if ((uTextFormat & DT_RESIZE_SMILEYS) && piece->smiley_height > text_size.cy)
						factor = text_size.cy / (double)piece->smiley_height;
					else
						factor = 1;

					text_size.cx = text_size.cx + (LONG)(factor * piece->smiley_width);
				}
			}
		}
	}

	return text_size;
}

void DrawTextSmiley(HDC hdcMem, RECT free_rc, const wchar_t *szText, int len, SortedList *plText, UINT uTextFormat, int max_smiley_height)
{
	if (szText == nullptr)
		return;

	uTextFormat &= ~DT_RIGHT;

	// Draw list
	int i;
	int pos_x = 0;
	int row_height, text_height;
	RECT tmp_rc = free_rc;

	if (uTextFormat & DT_RTLREADING)
		i = plText->realCount - 1;
	else
		i = 0;

	// Get real height of the line
	text_height = skin_DrawText(hdcMem, L"A", 1, &tmp_rc, DT_CALCRECT | uTextFormat);
	if (uTextFormat & DT_RESIZE_SMILEYS)
		row_height = text_height;
	else
		row_height = max(text_height, max_smiley_height);

	// Just draw ellipsis
	if (free_rc.right <= free_rc.left)
	{
		skin_DrawText(hdcMem, L"...", 3, &free_rc, uTextFormat & ~DT_END_ELLIPSIS);
	}
	else
	{
		// Draw text and smileys
		for (; i < plText->realCount && i >= 0 && pos_x < free_rc.right - free_rc.left && len > 0; i += (uTextFormat & DT_RTLREADING ? -1 : 1))
		{
			TextPiece *piece = (TextPiece *)plText->items[i];
			RECT text_rc = free_rc;

			if (uTextFormat & DT_RTLREADING)
				text_rc.right -= pos_x;
			else
				text_rc.left += pos_x;

			if (piece->type == TEXT_PIECE_TYPE_TEXT)
			{
				text_rc.top += (row_height - text_height) >> 1;

				tmp_rc = text_rc;
				tmp_rc.right += 50;
				skin_DrawText(hdcMem, &szText[piece->start_pos], min(len, piece->len), &tmp_rc, DT_CALCRECT | (uTextFormat & ~DT_END_ELLIPSIS));
				pos_x += tmp_rc.right - tmp_rc.left;

				if (uTextFormat & DT_RTLREADING)
					text_rc.left = max(text_rc.left, text_rc.right - (tmp_rc.right - tmp_rc.left));

				skin_DrawText(hdcMem, &szText[piece->start_pos], min(len, piece->len), &text_rc, uTextFormat);
				len -= piece->len;
			}
			else
			{
				double factor;

				if (len < piece->len)
				{
					len = 0;
				}
				else
				{
					len -= piece->len;

					if ((uTextFormat & DT_RESIZE_SMILEYS) && piece->smiley_height > row_height)
					{
						factor = row_height / (double)piece->smiley_height;
					}
					else
					{
						factor = 1;
					}

					if (uTextFormat & DT_RTLREADING)
						text_rc.left = max(text_rc.right - (LONG)(piece->smiley_width * factor), text_rc.left);

					if ((LONG)(piece->smiley_width * factor) <= text_rc.right - text_rc.left)
					{
						text_rc.top += (row_height - (LONG)(piece->smiley_height * factor)) >> 1;

						skin_DrawIconEx(hdcMem, text_rc.left, text_rc.top, piece->smiley,
							(LONG)(piece->smiley_width * factor), (LONG)(piece->smiley_height * factor), 0, nullptr, DI_NORMAL);
					}
					else
					{
						text_rc.top += (row_height - text_height) >> 1;
						skin_DrawText(hdcMem, L"...", 3, &text_rc, uTextFormat);
					}

					pos_x += (LONG)(piece->smiley_width * factor);
				}
			}
		}
	}
}


void DestroySmileyList(SortedList* p_list)
{
	if (p_list == nullptr)
		return;

	if (p_list->items != nullptr)
	{
		int i;
		for (i = 0; i < p_list->realCount; i++)
		{
			TextPiece *piece = (TextPiece *)p_list->items[i];

			if (piece != nullptr)
			{
				if (piece->type == TEXT_PIECE_TYPE_SMILEY)
					DestroyIcon(piece->smiley);

				mir_free(piece);
			}
		}
	}

	List_Destroy(p_list);
}



// Generete the list of smileys / text to be drawn
SortedList * ReplaceSmileys(const wchar_t *text, int text_size, const char *protocol, int *max_smiley_height)
{
	*max_smiley_height = 0;

	if (text[0] == '\0' || !ServiceExists(MS_SMILEYADD_BATCHPARSE))
		return nullptr;

	// Parse it!
	SMADD_BATCHPARSE sp = {};
	sp.Protocolname = protocol;
	sp.str.w = text;
	sp.flag = SAFL_UNICODE;
	SMADD_BATCHPARSERES *spres = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);
	if (spres == nullptr)
		// Did not find a simley
		return nullptr;

	// Lets add smileys
	SortedList *plText = List_Create(0, 10);

	const wchar_t *next_text_pos = text;
	const wchar_t *last_text_pos = _wcsninc(text, text_size);

	for (unsigned int i = 0; i < sp.numSmileys; i++) {
		wchar_t *start = _wcsninc(text, spres[i].startChar);
		wchar_t *end = _wcsninc(start, spres[i].size);

		if (spres[i].hIcon != nullptr) { // For defective smileypacks
			// Add text
			if (start > next_text_pos) {
				TextPiece *piece = (TextPiece *)mir_calloc(sizeof(TextPiece));

				piece->type = TEXT_PIECE_TYPE_TEXT;
				piece->start_pos = next_text_pos - text;
				piece->len = start - next_text_pos;

				List_Insert(plText, piece, plText->realCount);
			}

			// Add smiley
			{
				BITMAP bm;
				ICONINFO icon;
				TextPiece *piece = (TextPiece *)mir_calloc(sizeof(TextPiece));

				piece->type = TEXT_PIECE_TYPE_SMILEY;
				piece->len = end - start;
				piece->smiley = spres[i].hIcon;

				piece->smiley_width = 16;
				piece->smiley_height = 16;
				if (GetIconInfo(piece->smiley, &icon))
				{
					if (GetObject(icon.hbmColor, sizeof(BITMAP), &bm))
					{
						piece->smiley_width = bm.bmWidth;
						piece->smiley_height = bm.bmHeight;
					}

					DeleteObject(icon.hbmMask);
					DeleteObject(icon.hbmColor);
				}

				*max_smiley_height = max(piece->smiley_height, *max_smiley_height);

				List_Insert(plText, piece, plText->realCount);
			}

			next_text_pos = end;
		}
	}

	// Add rest of text
	if (last_text_pos > next_text_pos)
	{
		TextPiece *piece = (TextPiece *)mir_calloc(sizeof(TextPiece));

		piece->type = TEXT_PIECE_TYPE_TEXT;
		piece->start_pos = next_text_pos - text;
		piece->len = last_text_pos - next_text_pos;

		List_Insert(plText, piece, plText->realCount);
	}

	CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)spres);

	return plText;
}
