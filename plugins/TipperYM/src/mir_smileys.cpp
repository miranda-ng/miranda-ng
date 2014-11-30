/*
Copyright (C) 2005		Ricardo Pescuma Domenecci
Copyright (C) 2007-2010	Jan Holub

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

#include "common.h"

int InitTipperSmileys()
{
	// Register smiley category
	if (ServiceExists(MS_SMILEYADD_REGISTERCATEGORY))
	{
		SMADD_REGCAT rc;
		rc.cbSize = sizeof(rc);
		rc.name = "tipper";
		rc.dispname = Translate("Tipper smileys");
		CallService(MS_SMILEYADD_REGISTERCATEGORY, 0, (LPARAM)&rc);
	}

	return 0;
}

SMILEYPARSEINFO Smileys_PreParse(LPCTSTR lpString, int nCount, const char *protocol)
{
	if (!(opt.iSmileyAddFlags & SMILEYADD_ENABLE))
		return NULL;

	if (nCount == -1)
		nCount = mir_tstrlen(lpString);

	SMILEYPARSEINFO info = (SMILEYPARSEINFO) mir_calloc(sizeof(tagSMILEYPARSEINFO));
	info->pieces = ReplaceSmileys(lpString, nCount, protocol, &info->max_height);

	if (!info->pieces)
	{
		mir_free(info);
		return NULL;
	}

	return info;
}

void Smileys_FreeParse(SMILEYPARSEINFO parseInfo)
{
	if (parseInfo != NULL)
	{
		if (parseInfo->pieces != NULL)
			DestroySmileyList(parseInfo->pieces);

		mir_free(parseInfo);
	}
}

// Similar to DrawText win32 api function
// Pass uFormat | DT_CALCRECT to calc rectangle to be returned by lpRect
// parseInfo is optional (pass NULL and it will be calculated and deleted inside function
int Smileys_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, SMILEYPARSEINFO parseInfo)
{
	if (nCount == -1)
		nCount = mir_tstrlen(lpString);

	if (uFormat & DT_CALCRECT) {
		SIZE text_size = GetTextSize(hDC, lpString, parseInfo, uFormat, parseInfo->max_height, (lpRect->right - lpRect->left));
		lpRect->bottom = text_size.cy;

		if (text_size.cx < lpRect->right - lpRect->left) {
			if (uFormat & DT_RIGHT)
				lpRect->left = lpRect->right - text_size.cx;
			else
				lpRect->right = lpRect->left + text_size.cx;
		}

		return text_size.cy;
	}


	// Draw
	if (parseInfo->pieces == NULL)
		return DrawText(hDC, lpString, nCount, lpRect, uFormat);

	RECT rc = *lpRect;
	SIZE text_size = GetTextSize(hDC, lpString, parseInfo, uFormat, parseInfo->max_height, (lpRect->right - lpRect->left));

	if (text_size.cx < rc.right - rc.left) {
		if (uFormat & DT_RIGHT)
			rc.left = rc.right - text_size.cx;
		else
			rc.right = rc.left + text_size.cx;
	}

	DrawTextSmiley(hDC, rc, lpString, nCount, parseInfo, uFormat, parseInfo->max_height);
	return text_size.cy;
}

SIZE GetTextSize(HDC hdcMem, const TCHAR *szText, SMILEYPARSEINFO info, UINT uTextFormat, int max_smiley_height, int max_width)
{
	SIZE text_size = {0};
	int text_height;
	int row_count = 0, pos_x = 0;

	if (szText == NULL || _tcsclen(szText) == 0)
	{
		text_size.cy = 0;
		text_size.cx = 0;
	}
	else
	{
		RECT text_rc = {0, 0, 2048, 2048};

		if (info->pieces == NULL)
		{
			DrawText(hdcMem, szText, -1, &text_rc, DT_CALCRECT | uTextFormat);
			text_size.cx = text_rc.right - text_rc.left;
			text_size.cy = text_rc.bottom - text_rc.top;
		}
		else
		{
			// Get real height of the line
			text_height = DrawText(hdcMem, _T("A"), 1, &text_rc, DT_CALCRECT | uTextFormat);

			// See each item of list
			int i;
			for (i = 0; i < info->pieces->realCount; i++)
			{
				TEXTPIECE *piece = (TEXTPIECE *) info->pieces->items[i];
				info->row_height[row_count] = max(info->row_height[row_count], text_height);

				if (piece->type == TEXT_PIECE_TYPE_TEXT)
				{
					RECT text_rc = {0, 0, 2048, 2048};

					DrawText(hdcMem, szText + piece->start_pos, piece->len, &text_rc, DT_CALCRECT | uTextFormat);
					pos_x += (text_rc.right - text_rc.left);
					if (pos_x > max_width)
					{
						text_size.cx = max(text_size.cx, pos_x - (text_rc.right - text_rc.left));
						pos_x = text_rc.right - text_rc.left;
						info->row_height[++row_count] = text_height;
					}

					if (szText[piece->start_pos + piece->len - 1] == '\n')
					{
						text_size.cx = max(text_size.cx, pos_x);
						pos_x = 0;
						info->row_height[++row_count] = 0;
					}

				}
				else
				{
					double factor;

					if (uTextFormat & DT_RESIZE_SMILEYS && piece->smiley_height > text_height)
						factor = text_height / (double) piece->smiley_height;
					else
						factor = 1;

					info->row_height[row_count] = max(info->row_height[row_count], piece->smiley_height * factor);

					pos_x += piece->smiley_width * factor;
					if (pos_x > max_width)
					{
						text_size.cx = max(text_size.cx, pos_x - (piece->smiley_width * factor));
						pos_x = piece->smiley_width * factor;
						info->row_height[++row_count] = piece->smiley_height * factor;
					}
				}
			}

			text_size.cx = max(text_size.cx, pos_x);
			for (i = 0; i < row_count + 1; i++)
				text_size.cy += info->row_height[i];
		}
	}

	return text_size;
}

void DrawTextSmiley(HDC hdcMem, RECT free_rc, const TCHAR *szText, int len, SMILEYPARSEINFO info, UINT uTextFormat, int max_smiley_height)
{
	if (szText == NULL)
		return;

	uTextFormat &= ~DT_RIGHT;

	// Draw list
	int text_height, i, shift;
	int row_count = 0, pos_x = 0;

	RECT tmp_rc = free_rc;

	if (uTextFormat & DT_RTLREADING)
		i = info->pieces->realCount - 1;
	else
		i = 0;

	// Get real height of the line
	text_height = DrawText(hdcMem, _T("A"), 1, &tmp_rc, DT_CALCRECT | uTextFormat);

	SaveAlpha(&free_rc);

	// Just draw ellipsis
	if (free_rc.right <= free_rc.left)
	{
		DrawText(hdcMem, _T("..."), 3, &free_rc, uTextFormat & ~DT_END_ELLIPSIS);
	}
	else
	{
		// Draw text and smileys
		RECT text_rc = free_rc;
		for (; i < info->pieces->realCount && i >= 0 && len > 0; i += (uTextFormat & DT_RTLREADING ? -1 : 1))
		{
			TEXTPIECE *piece = (TEXTPIECE *) info->pieces->items[i];

			if (uTextFormat & DT_RTLREADING)
				text_rc.right = free_rc.right - pos_x;
			else
				text_rc.left = free_rc.left + pos_x;

			if (piece->type == TEXT_PIECE_TYPE_TEXT)
			{
				tmp_rc = text_rc;
				tmp_rc.right = 2048;

				DrawText(hdcMem, szText + piece->start_pos, min(len, piece->len), &tmp_rc, DT_CALCRECT | (uTextFormat & ~DT_END_ELLIPSIS));

				pos_x += (tmp_rc.right - tmp_rc.left);
				if (pos_x > (free_rc.right - free_rc.left))
				{
					pos_x = tmp_rc.right - tmp_rc.left;
					text_rc.left = free_rc.left;
					text_rc.right = free_rc.right;
					text_rc.top += info->row_height[row_count];
					row_count++;
				}

				shift = (info->row_height[row_count] - text_height) >> 1;
				text_rc.top += shift;

				if (uTextFormat & DT_RTLREADING)
					text_rc.left = max(text_rc.left, text_rc.right - (tmp_rc.right - tmp_rc.left));

				DrawText(hdcMem, szText + piece->start_pos, min(len, piece->len), &text_rc, uTextFormat);
				len -= piece->len;
				text_rc.top -= shift;

				if (szText[piece->start_pos + piece->len - 1] == '\n')
				{
					text_rc.left = free_rc.left;
					text_rc.right = free_rc.right;
					text_rc.top += info->row_height[row_count];
					pos_x = 0;
					row_count++;
				}
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

					if (uTextFormat & DT_RESIZE_SMILEYS && piece->smiley_height > text_height)
						factor = text_height / (double) piece->smiley_height;
					else
						factor = 1;

					if (uTextFormat & DT_RTLREADING)
						text_rc.left = max(text_rc.right - (int)(piece->smiley_width * factor), text_rc.left);

					pos_x += piece->smiley_width * factor;
					if (pos_x > (free_rc.right - free_rc.left))
					{
						pos_x = piece->smiley_width * factor;
						text_rc.left = free_rc.left;
						text_rc.right = free_rc.right;
						text_rc.top += info->row_height[row_count];
						row_count++;
					}

					shift = (info->row_height[row_count] - (LONG)(piece->smiley_height * factor)) >> 1;
					DrawIconExAlpha(hdcMem, text_rc.left, text_rc.top + shift, piece->smiley, piece->smiley_width * factor, piece->smiley_height * factor, 0, NULL, DI_NORMAL, true);
				}
			}
		}
	}

	RestoreAlpha(&free_rc);
}

void DestroySmileyList(SortedList* p_list)
{
	if (p_list == NULL)
		return;

	if (p_list->items != NULL)
	{
		int i;
		for (i = 0 ; i < p_list->realCount ; i++)
		{
			TEXTPIECE *piece = (TEXTPIECE *)p_list->items[i];
			if (piece != NULL)
			{
				if (piece->type == TEXT_PIECE_TYPE_SMILEY)
					DestroyIcon(piece->smiley);

				mir_free(piece);	//this free the p_list->items[i]
			}
		}
	}
	List_Destroy(p_list);		//this free the p_list->items member
	mir_free(p_list);				//this free the p_list itself (alloc by List_Create)
}

// Generete the list of smileys / text to be drawn
SortedList *ReplaceSmileys(const TCHAR *text, int text_size, const char *protocol, int *max_smiley_height)
{
	SMADD_BATCHPARSE2 sp = {0};
	SMADD_BATCHPARSERES *spres;
	char smileyProto[64];

	*max_smiley_height = 0;

	if (!text || !text[0] || !ServiceExists(MS_SMILEYADD_BATCHPARSE))
		return NULL;

	if (protocol == NULL)
		strcpy(smileyProto, "tipper");
	else if (strcmp(protocol, META_PROTO) == 0)
		strcpy(smileyProto, "tipper");
	else
		strcpy(smileyProto, protocol);

	// Parse it!
	sp.cbSize = sizeof(sp);
	sp.str = (TCHAR *)text;
	sp.flag = SAFL_TCHAR;
	sp.Protocolname = (opt.iSmileyAddFlags & SMILEYADD_USEPROTO) ? smileyProto : "tipper";
	spres = (SMADD_BATCHPARSERES *)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);

	if (!spres) // Did not find a smiley
		return NULL;

	// Lets add smileys
	SortedList *plText = List_Create(0, 10);

	TCHAR *word_start, *word_end;
	TCHAR *smiley_start, *smiley_end;
	TCHAR *last_text_pos = _tcsninc(text, text_size);

	word_start = word_end = (TCHAR *)text;

	for (unsigned i = 0; i < sp.numSmileys; i++)
	{
		// Get smile position
		smiley_start = _tcsninc(text, spres[i].startChar);
		smiley_end = _tcsninc(smiley_start, spres[i].size);

		if (spres[i].hIcon) // For deffective smileypacks
		{
			if (opt.iSmileyAddFlags & SMILEYADD_ONLYISOLATED)
			{
				if ((smiley_start > text && *(smiley_start - 1) != ' ' && *(smiley_start - 1) != '\n' && *smiley_end != '\r') ||
					(*smiley_end != '\0' && *smiley_end != ' ' && *smiley_end != '\n' && *smiley_end != '\r'))
					continue;
			}

			// Add text
			while (word_end != smiley_start)
			{
				while (word_end[0] != ' ' && word_end[0] != '\n')
					word_end++;

				if (word_end > smiley_start)
					word_end = smiley_start;
				else
					word_end++;

				if (word_end > word_start)
				{
					TEXTPIECE *piece = (TEXTPIECE *)mir_calloc(sizeof(TEXTPIECE));
					piece->type = TEXT_PIECE_TYPE_TEXT;
					piece->start_pos = word_start - text;
					piece->len = word_end - word_start;
					List_Insert(plText, piece, plText->realCount);
					word_start = word_end;
				}
			}

			// Add smiley
			{
				BITMAP bm;
				ICONINFO icon;
				TEXTPIECE *piece = (TEXTPIECE *) mir_calloc(sizeof(TEXTPIECE));

				piece->type = TEXT_PIECE_TYPE_SMILEY;
				piece->len = spres[i].size;
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

			word_start = word_end = smiley_end;
		}
	}

	// Add rest of the text
	while (word_end != last_text_pos)
	{
		while (word_end[0] && word_end[0] != ' ' && word_end[0] != '\n')
			word_end++;

		if (word_end[0])
			word_end++;

		if (word_end > word_start)
		{
			TEXTPIECE *piece = (TEXTPIECE *)mir_calloc(sizeof(TEXTPIECE));
			piece->type = TEXT_PIECE_TYPE_TEXT;
			piece->start_pos = word_start - text;
			piece->len = word_end - word_start;
			List_Insert(plText, piece, plText->realCount);
			word_start = word_end;
		}

	}

	CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)spres);

	return plText;
}

int DrawTextExt(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, LPCSTR lpProto, SMILEYPARSEINFO spi)
{
	if ((opt.iSmileyAddFlags & SMILEYADD_ENABLE) && spi != NULL)
	{
		if (opt.iSmileyAddFlags & SMILEYADD_RESIZE)
			uFormat |= DT_RESIZE_SMILEYS;

		return Smileys_DrawText(hdc, lpString, nCount, lpRect, uFormat, lpProto, spi);
	}
	else
	{
		if (uFormat & DT_CALCRECT)
		{
			return DrawText(hdc, lpString, nCount, lpRect, uFormat);
		}
		else
		{
			return DrawTextAlpha(hdc, lpString, nCount, lpRect, uFormat);
		}
	}
}
