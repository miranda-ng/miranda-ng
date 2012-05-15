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

#ifndef __MIR_SMILEYS_H__
#define __MIR_SMILEYS_H__

#define DT_RESIZE_SMILEYS		0x10000000

// smileyadd options
#define SMILEYADD_ENABLE		1
#define SMILEYADD_USEPROTO		2
#define SMILEYADD_ONLYISOLATED	4
#define SMILEYADD_RESIZE		8
 
#define TEXT_PIECE_TYPE_TEXT    0
#define TEXT_PIECE_TYPE_SMILEY  1

typedef struct {
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
} TEXTPIECE;

typedef struct tagSMILEYPARSEINFO {
	SortedList *pieces;
	int row_height[512]; // max 512 rows
	int max_height;
} *SMILEYPARSEINFO;

int InitTipperSmileys();
SMILEYPARSEINFO Smileys_PreParse(LPCTSTR lpString, int nCount, const char *protocol);
void Smileys_FreeParse(SMILEYPARSEINFO parseInfo);

SortedList *ReplaceSmileys(const TCHAR *text, int text_size, const char *protocol, int *max_smiley_height);
SIZE GetTextSize(HDC hdcMem, const TCHAR *szText, SMILEYPARSEINFO info, UINT uTextFormat, int max_smiley_height, int max_width);
void DrawTextSmiley(HDC hdcMem, RECT free_rc, const TCHAR *szText, int len, SMILEYPARSEINFO info, UINT uTextFormat, int max_smiley_height);
void DestroySmileyList(SortedList* p_list);

int Smileys_DrawText(HDC hDC, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, const char *protocol, SMILEYPARSEINFO parseInfo);
int DrawTextExt(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat, LPCSTR lpProto, SMILEYPARSEINFO spi);

#endif // __MIR_SMILEYS_H__
