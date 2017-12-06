/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

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

#ifndef _BITMAP_FUNC_INC
#define _BITMAP_FUNC_INC

// tooltip skin
#define SKIN_ITEMS_COUNT	2
#define SKIN_PARTS_COUNT	9
#define SKIN_ITEM_BG		0
#define SKIN_ITEM_SIDEBAR	1

// skin parts
#define SP_CENTRE_AREA		0
#define SP_EDGE_TOP			1
#define SP_EDGE_RIGHT		2
#define SP_EDGE_BOTTOM		3
#define SP_EDGE_LEFT		4
#define SP_CORNER_TL		5
#define SP_CORNER_TR		6
#define SP_CORNER_BR		7
#define SP_CORNER_BL		8

// image paint options
typedef enum {	
	TM_NONE = 0, TM_CENTRE = 1, TM_STRECH_ALL = 2, TM_STRECH_HORIZONTAL = 3, TM_STRECH_VERTICAL = 4,
	TM_TILE_ALL = 5, TM_TILE_HORIZONTAL = 6, TM_TILE_VERTICAL = 7
} TransformationMode;

typedef unsigned long COLOR32;
typedef struct {
	HDC hdc;
	HBITMAP hBitmap, hOldBitmap;
	HBITMAP hbmpSkinParts[SKIN_ITEMS_COUNT][SKIN_PARTS_COUNT];
	FIBITMAP *fib[SKIN_ITEMS_COUNT];
	COLOR32 *colBits;
	int iWidth;
	int iHeight;
	bool bNeedLayerUpdate;
	bool bCached;
} TOOLTIPSKIN;

void CreateSkinBitmap(int iWidth, int iHeight, bool bServiceTip);
void DestroySkinBitmap();
int DrawTextAlpha(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);
void DrawIconExAlpha(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags, bool bIsSmiley);
COLOR32* SaveAlpha(LPRECT lpRect);
void RestoreAlpha(LPRECT lpRect, COLOR32 *pBits, BYTE alpha = 0xff);
void PremultipleChannels();
void ColorizeBitmap();
HRGN CreateOpaqueRgn(BYTE level, bool bOpaque);

#endif
