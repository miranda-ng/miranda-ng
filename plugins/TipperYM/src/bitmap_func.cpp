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

#include "common.h"

TOOLTIPSKIN skin = {0};

HBITMAP CreateBitmapPart(FIBITMAP *fibSrc, int iSrcWidth, int iSrcHeight, int iDesWidth, int iDesHeight, TransformationMode transfMode) 
{
	FIBITMAP *fibMem = NULL;
	HBITMAP hbmpDes = NULL;

	if (!fibSrc) 
		return NULL;
		
	switch (transfMode) {
	case TM_NONE:
	case TM_CENTRE:
	case TM_TILE_ALL:
		hbmpDes = fii->FI_CreateHBITMAPFromDIB(fibSrc);
		break;

	case TM_STRECH_ALL:
		fibMem = fii->FI_Rescale(fibSrc, iDesWidth, iDesHeight, FILTER_BILINEAR);
		hbmpDes = fii->FI_CreateHBITMAPFromDIB(fibMem);
		break;

	case TM_STRECH_HORIZONTAL:
	case TM_TILE_VERTICAL:
		fibMem = fii->FI_Rescale(fibSrc, iDesWidth, iSrcHeight, FILTER_BILINEAR);
		hbmpDes = fii->FI_CreateHBITMAPFromDIB(fibMem);
		break;

	case TM_STRECH_VERTICAL:
	case TM_TILE_HORIZONTAL:
		fibMem = fii->FI_Rescale(fibSrc, iSrcWidth, iSrcHeight, FILTER_BILINEAR);
		hbmpDes = fii->FI_CreateHBITMAPFromDIB(fibMem);
		break;
	}

	if (fibMem) fii->FI_Unload(fibMem);
	return hbmpDes;
}

void DrawBitmapPart(HDC hdcMem, HBITMAP hbmpPart, RECT *rcDes, TransformationMode transfMode) 
{
	if (!hbmpPart) 
		return;
		
	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	BITMAP bitmap;
	GetObject(hbmpPart, sizeof(bitmap), &bitmap);
	int iBmpWidth = bitmap.bmWidth, iBmpHeight = bitmap.bmHeight;
	int iDesWidth = rcDes->right - rcDes->left, iDesHeight = rcDes->bottom - rcDes->top;
	int x, y, iRepX, iRepY, iEndX, iEndY;

	SelectObject(hdcMem, hbmpPart);

	switch (transfMode) {
	case TM_NONE:
	case TM_STRECH_ALL:
	case TM_STRECH_HORIZONTAL:
	case TM_STRECH_VERTICAL:
		AlphaBlend(skin.hdc, rcDes->left, rcDes->top, iBmpWidth, iBmpHeight, hdcMem, 0, 0, iBmpWidth, iBmpHeight, blend);
		break;

	case TM_CENTRE:
		iRepX = rcDes->left + ((iDesWidth - iBmpWidth) / 2);
		iRepY = rcDes->top + ((iDesHeight - iBmpHeight) / 2);
		AlphaBlend(skin.hdc, iRepX, iRepY, iBmpWidth, iBmpHeight, hdcMem, 0, 0, iBmpWidth, iBmpHeight, blend);
		break;

	case TM_TILE_ALL:
		iRepX = iDesWidth / iBmpWidth;
		iRepY = iDesHeight / iBmpHeight;
		iEndX = iDesWidth % iBmpWidth;
		iEndY = iDesHeight % iBmpHeight;

		for (y = 0; y < iRepY; y++) {
			for (x = 0; x < iRepX; x++)
				AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top + (y * iBmpHeight), iBmpWidth, iBmpHeight, hdcMem, 0, 0, iBmpWidth, iBmpHeight, blend);

			AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top + (y * iBmpHeight), iEndX, iBmpHeight, hdcMem, 0, 0, iEndX, iBmpHeight, blend);
		}

		for (x = 0; x < iRepX; x++) 
			AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top + (y * iBmpHeight), iBmpWidth, iEndY, hdcMem, 0, 0, iBmpWidth, iEndY, blend);

		AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top + (y * iBmpHeight), iEndX, iEndY, hdcMem, 0, 0, iEndX, iEndY, blend);
		break;

	case TM_TILE_HORIZONTAL:
		iRepX = iDesWidth / iBmpWidth;
		iEndX = iDesWidth % iBmpWidth;

		for (x = 0; x < iRepX; x++)
			AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top, iBmpWidth, iDesHeight, hdcMem, 0, 0, iBmpWidth, iDesHeight, blend);

		AlphaBlend(skin.hdc, rcDes->left + (x * iBmpWidth), rcDes->top, iEndX, iDesHeight, hdcMem, 0, 0, iEndX, iDesHeight, blend);
		break;

	case TM_TILE_VERTICAL:
		iRepY = iDesHeight / iBmpHeight;
		iEndY = iDesHeight % iBmpHeight;

		for(y = 0; y < iRepY; y++) 
			AlphaBlend(skin.hdc, rcDes->left, rcDes->top + (y * iBmpHeight), iDesWidth, iBmpHeight, hdcMem, 0, 0, iDesWidth, iBmpHeight, blend);

		AlphaBlend(skin.hdc, rcDes->left, rcDes->top + (y * iBmpHeight), iDesWidth, iEndY, hdcMem, 0, 0, iDesWidth, iEndY, blend);
		break;
	}
}

void CreateFromBitmaps(bool bServiceTip)
{
	int rcWidth, rcHeight;
	int iCentWidth, iCentHeight; 
	int iBmpWidth, iBmpHeight; 
	int top, right, bottom, left;

	BLENDFUNCTION blend;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	for (int i = 0; i < SKIN_ITEMS_COUNT; i++) { 
		if (i == SKIN_ITEM_SIDEBAR && (!opt.iSidebarWidth || bServiceTip))
			continue;

		TCHAR* tszFileName = opt.szImgFile[i];
		if (tszFileName && *tszFileName != 0) {
			FIBITMAP *fib = NULL;
			if (!skin.bCached) {
				FIBITMAP *fibLoad = (FIBITMAP *)CallService(MS_IMG_LOAD, (WPARAM)tszFileName, (LPARAM)(IMGL_TCHAR | IMGL_RETURNDIB));
				if (!fibLoad) continue;

				if (fii->FI_GetBPP(fibLoad) != 32) 
					fib = fii->FI_ConvertTo32Bits(fibLoad);
				else
					fib = fibLoad;

				if (fib != fibLoad) 
					fii->FI_Unload(fibLoad);

				skin.fib[i] = fib;		
			} 
			else fib = skin.fib[i];

			// destination rectangle size
			if (i == SKIN_ITEM_BG) {
				rcWidth = skin.iWidth;
				rcHeight = skin.iHeight;
			} 
			else if (i == SKIN_ITEM_SIDEBAR) {
				rcWidth = opt.iSidebarWidth;
				rcHeight = skin.iHeight;
			}

			// bitmap size
			iBmpWidth = fii->FI_GetWidth(fib);
			iBmpHeight = fii->FI_GetHeight(fib);	

			// margins
			top = opt.margins[i].top < iBmpHeight ? opt.margins[i].top : 0;
			right = opt.margins[i].right < iBmpWidth ? opt.margins[i].right : 0;
			bottom = opt.margins[i].bottom < iBmpHeight ? opt.margins[i].bottom : 0;
			left = opt.margins[i].left < iBmpWidth ? opt.margins[i].left : 0;

			// centre area size
			iCentWidth = max(rcWidth - left - right, 0);
			iCentHeight = max(rcHeight - top - bottom, 0);

			FIBITMAP *fibCentre = NULL, *fibMem = NULL;
			if (opt.margins[i].left || opt.margins[i].top || opt.margins[i].right || opt.margins[i].bottom) {
				// create corners bitmaps
				if (!skin.bCached) {
					if (top > 0 && left > 0) { // TL
						fibMem = fii->FI_Copy(fib, 0, 0, left, top);
						skin.hbmpSkinParts[i][SP_CORNER_TL] = fii->FI_CreateHBITMAPFromDIB(fibMem);
						if (fibMem) fii->FI_Unload(fibMem);
					}
					if (top > 0 && right > 0) { // TR
						fibMem = fii->FI_Copy(fib, iBmpWidth - right, 0, iBmpWidth, top);
						skin.hbmpSkinParts[i][SP_CORNER_TR] = fii->FI_CreateHBITMAPFromDIB(fibMem);
						if (fibMem) fii->FI_Unload(fibMem);
					}
					if (bottom > 0 && right > 0) { // BR
						fibMem = fii->FI_Copy(fib, iBmpWidth - right, iBmpHeight - bottom, iBmpWidth, iBmpHeight);
						skin.hbmpSkinParts[i][SP_CORNER_BR] = fii->FI_CreateHBITMAPFromDIB(fibMem);
						if (fibMem) fii->FI_Unload(fibMem);
					}
					if (bottom > 0 && left > 0) { // BL
						fibMem = fii->FI_Copy(fib, 0, iBmpHeight - bottom, left, iBmpHeight);
						skin.hbmpSkinParts[i][SP_CORNER_BL] = fii->FI_CreateHBITMAPFromDIB(fibMem);
						if (fibMem) fii->FI_Unload(fibMem);
					}
				}

				// create edge parts bitmaps
				if (top > 0 && iCentWidth > 0) { // top
					fibMem = fii->FI_Copy(fib, left, 0, iBmpWidth - right, top);
					skin.hbmpSkinParts[i][SP_EDGE_TOP] = CreateBitmapPart(fibMem, iBmpWidth - left - right, top, iCentWidth, top, opt.transfMode[i]);
					if (fibMem) fii->FI_Unload(fibMem);
				}
				if (right > 0 && iCentHeight > 0) { // right
					fibMem = fii->FI_Copy(fib, iBmpWidth - right, top, iBmpWidth, iBmpHeight - bottom);
					skin.hbmpSkinParts[i][SP_EDGE_RIGHT] = CreateBitmapPart(fibMem, right, iBmpHeight - top - bottom, right, iCentHeight, opt.transfMode[i]);
					if (fibMem) fii->FI_Unload(fibMem);
				}
				if (bottom > 0 && iCentWidth > 0) { // bottom
					fibMem = fii->FI_Copy(fib, left, iBmpHeight - bottom, iBmpWidth - right, iBmpHeight);
					skin.hbmpSkinParts[i][SP_EDGE_BOTTOM] = CreateBitmapPart(fibMem, iBmpWidth - left - right, bottom, iCentWidth, bottom, opt.transfMode[i]);
					if (fibMem) fii->FI_Unload(fibMem);
				}
				if (left > 0 && iCentHeight > 0) { // left
					fibMem = fii->FI_Copy(fib, 0, top, left, iBmpHeight - bottom);
					skin.hbmpSkinParts[i][SP_EDGE_LEFT] = CreateBitmapPart(fibMem, left, iBmpHeight - top - bottom, left, iCentHeight, opt.transfMode[i]);
					if (fibMem) fii->FI_Unload(fibMem);
				}	
		
				fibCentre = fii->FI_Copy(fib, left, top, iBmpWidth - right, iBmpHeight - bottom);
				if (fibCentre) {
					fib = fibCentre;
					iBmpWidth = fii->FI_GetWidth(fib);
					iBmpHeight = fii->FI_GetHeight(fib);
				}
			}

			// create centre area bitmap
			skin.hbmpSkinParts[i][SP_CENTRE_AREA] = CreateBitmapPart(fib, iBmpWidth, iBmpHeight, iCentWidth, iCentHeight, opt.transfMode[i]);			
			if (fibCentre) 
				fii->FI_Unload(fibCentre);

			if (i == SKIN_ITEM_SIDEBAR) {
				int limit = skin.bCached ? SP_CORNER_TL : SKIN_PARTS_COUNT; // don't premultiply corner bitmaps multiple times
				for (int j = 0; j < limit; j++) {
					if (skin.hbmpSkinParts[i][j]) 
						fii->FI_Premultiply(skin.hbmpSkinParts[i][j]);
				}
			}

			HDC hdcMem = CreateCompatibleDC(0);	
			RECT rc = {0};

			if (skin.hbmpSkinParts[i][SP_CENTRE_AREA]) {
				SetRect(&rc, left, top, rcWidth - right, rcHeight - bottom);
				DrawBitmapPart(hdcMem, skin.hbmpSkinParts[i][SP_CENTRE_AREA], &rc, opt.transfMode[i]);
			}

			if (opt.margins[i].left || opt.margins[i].top || opt.margins[i].right || opt.margins[i].bottom) {
				// draw edge parts
				if (skin.hbmpSkinParts[i][SP_EDGE_TOP]) { // top
					SetRect(&rc, left, 0, rcWidth - right, top);
					DrawBitmapPart(hdcMem, skin.hbmpSkinParts[i][SP_EDGE_TOP], &rc, opt.transfMode[i]);
				}
				if (skin.hbmpSkinParts[i][SP_EDGE_RIGHT]) { // right
					SetRect(&rc, rcWidth - right, top, rcWidth, rcHeight - bottom);
					DrawBitmapPart(hdcMem, skin.hbmpSkinParts[i][SP_EDGE_RIGHT], &rc, opt.transfMode[i]);
				}
				if (skin.hbmpSkinParts[i][SP_EDGE_BOTTOM]) { // bottom
					SetRect(&rc, left, rcHeight - bottom, rcWidth - right, rcHeight);
					DrawBitmapPart(hdcMem, skin.hbmpSkinParts[i][SP_EDGE_BOTTOM], &rc, opt.transfMode[i]);
				}
				if (skin.hbmpSkinParts[i][SP_EDGE_LEFT]) { // left
					SetRect(&rc, 0, top, left, rcHeight - bottom);
					DrawBitmapPart(hdcMem, skin.hbmpSkinParts[i][SP_EDGE_LEFT], &rc, opt.transfMode[i]);
				}

				// draw corners
				if (skin.hbmpSkinParts[i][SP_CORNER_TL]) { // TL
					SelectObject(hdcMem, skin.hbmpSkinParts[i][SP_CORNER_TL]);
					AlphaBlend(skin.hdc, 0, 0, left, top, hdcMem, 0, 0, left, top, blend);
				}
				if (skin.hbmpSkinParts[i][SP_CORNER_TR]) { // TR
					SelectObject(hdcMem, skin.hbmpSkinParts[i][SP_CORNER_TR]);
					AlphaBlend(skin.hdc, rcWidth - right, 0, right, top, hdcMem, 0, 0, right, top, blend);
				}
				if (skin.hbmpSkinParts[i][SP_CORNER_BR]) { // BR
					SelectObject(hdcMem, skin.hbmpSkinParts[i][SP_CORNER_BR]);
					AlphaBlend(skin.hdc, rcWidth - right, rcHeight - bottom, right, bottom, hdcMem, 0, 0, right, bottom, blend);
				}
				if (skin.hbmpSkinParts[i][SP_CORNER_BL]) { // BL
					SelectObject(hdcMem, skin.hbmpSkinParts[i][SP_CORNER_BL]);
					AlphaBlend(skin.hdc, 0, rcHeight - bottom, left, bottom, hdcMem, 0, 0, left, bottom, blend);
				}
			}

			for (int j = 0; j < SP_CORNER_TL; j++) 
				if (skin.hbmpSkinParts[i][j]) {
					DeleteObject(skin.hbmpSkinParts[i][j]);
					skin.hbmpSkinParts[i][j] = NULL;
				}

			skin.bNeedLayerUpdate = true;

			DeleteDC(hdcMem);
		}
	}

	if (!bServiceTip)
		skin.bCached = true;
}

void SolidColorFill(bool bServiceTip)
{
	RECT rc = {0}; 
	rc.right = skin.iWidth;
	rc.bottom = skin.iHeight;
	HBRUSH hBrush = CreateSolidBrush(opt.colBg);
	FillRect(skin.hdc, &rc, hBrush);
	DeleteObject(hBrush);

	if (opt.iSidebarWidth > 0 && !bServiceTip) {
		rc.right = opt.iSidebarWidth;
		hBrush = CreateSolidBrush(opt.colSidebar);
		FillRect(skin.hdc, &rc, hBrush);
		DeleteObject(hBrush);	
	}
}

void CreateSkinBitmap(int iWidth, int iHeight, bool bServiceTip)
{
	if (skin.hdc) {
		if (skin.hBitmap) {
			SelectObject(skin.hdc, skin.hOldBitmap);
			DeleteObject(skin.hBitmap);
			skin.hBitmap = NULL;
		}
		DeleteDC(skin.hdc);
		skin.hdc = NULL;
	}

	skin.iWidth = iWidth;
	skin.iHeight = iHeight;
	skin.bNeedLayerUpdate = false;

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = skin.iWidth;
	bi.bmiHeader.biHeight = -skin.iHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	skin.hBitmap = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, (void **)&skin.colBits, 0, 0);
	if (!skin.hBitmap) 
		return;

	skin.hdc = CreateCompatibleDC(0);
	skin.hOldBitmap = (HBITMAP)SelectObject(skin.hdc, skin.hBitmap);

	if (opt.skinMode == SM_COLORFILL)
		SolidColorFill(bServiceTip);
	else  {
		CreateFromBitmaps(bServiceTip);
		if (opt.iEnableColoring == 1)
			ColorizeBitmap();
	}
}

void DestroySkinBitmap()
{
	for (int i = 0; i < SKIN_ITEMS_COUNT; i++) { 
		if (skin.fib[i]) {
			fii->FI_Unload(skin.fib[i]);
			skin.fib[i] = NULL;
		}

		for (int j = SP_CORNER_TL; j < SKIN_PARTS_COUNT; j++) {
			if (skin.hbmpSkinParts[i][j]) {
				DeleteObject(skin.hbmpSkinParts[i][j]);
				skin.hbmpSkinParts[i][j] = NULL;
			}
		}
	}

	skin.bCached = false;
}

COLOR32* SaveAlpha(LPRECT lpRect)
{
	GdiFlush();

	if (lpRect->left < 0) lpRect->left = 0; 
	if (lpRect->top < 0) lpRect->top = 0;
	if (lpRect->right - lpRect->left > skin.iWidth) lpRect->right = lpRect->left + skin.iWidth;
	if (lpRect->bottom - lpRect->top > skin.iHeight) lpRect->bottom = lpRect->top + skin.iHeight;

	int x = lpRect->left;
	int y = lpRect->top;
	int w = lpRect->right - lpRect->left;
	int h = lpRect->bottom - lpRect->top;

	COLOR32 *res = (COLOR32 *)mir_alloc(sizeof(COLOR32) * w * h);
	COLOR32 *p1 = res;

	for (int i = 0; i < h; i++) {
		if (i+y < 0) continue;
		if (i+y >= skin.iHeight) break;
		COLOR32 *p2 = skin.colBits + (y+i)*skin.iWidth + x;
		for (int j = 0; j < w; j++) {
			if (j+x < 0) continue;
			if (j+x >= skin.iWidth) break;
			*p1++ = *p2++;
		}
	}
	return res;
}

void RestoreAlpha(LPRECT lpRect, COLOR32 *pBits, BYTE alpha)
{
	GdiFlush();

	if (lpRect->left < 0) lpRect->left = 0; 
	if (lpRect->top < 0) lpRect->top = 0;
	if (lpRect->right > skin.iWidth) lpRect->right = skin.iWidth;
	if (lpRect->bottom > skin.iHeight) lpRect->bottom = skin.iHeight;

	int x = lpRect->left;
	int y = lpRect->top;
	int w = lpRect->right - lpRect->left;
	int h = lpRect->bottom - lpRect->top;

	COLOR32 *p1 = pBits;

	for (int i = 0; i < h; i++) {
		if (i+y < 0) continue;
		if (i+y >= skin.iHeight) break;
		COLOR32 *p2 = skin.colBits + (y+i)*skin.iWidth + x;
		for (int j = 0; j < w; j++) {
			if (j+x < 0) continue;
			if (j+x >= skin.iWidth) break;

			if ((*p1&0x00ffffff) != (*p2&0x00ffffff))
				*p2 |= (alpha << 24);
			else
				*p2 = (*p2&0x00ffffff) | (*p1&0xff000000);

			++p1; ++p2;
		}
	}
}

BOOL IsAlphaTransparent(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel != 32)
		return FALSE;

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	BYTE *p = (BYTE *)mir_calloc(dwLen);
	if (p == NULL)
		return FALSE;

	GetBitmapBits(hBitmap, dwLen, p);

	for (int y = 0; y < bmp.bmHeight; y++) {
		BYTE *px = p + bmp.bmWidth * 4 * y;
		for (int x = 0; x < bmp.bmWidth; x++) {
			if (px[3] != 0) {
				mir_free(p);
				return TRUE;
			}

			px += 4;
		}
	}

	mir_free(p);
	return FALSE;
}

void DrawIconExAlpha(HDC hdc, int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hbrFlickerFreeDraw, UINT diFlags, bool bIsSmiley)
{
	mir_ptr<COLOR32> pBits;

	if (skin.bNeedLayerUpdate && !bIsSmiley) {
		ICONINFO icon;
		if ( GetIconInfo(hIcon, &icon)) {
			if ( !IsAlphaTransparent(icon.hbmColor)) {
				RECT rc;
				SetRect(&rc, xLeft, yTop, xLeft + cxWidth, yTop + cyWidth);
				pBits = SaveAlpha(&rc);
			}

			DeleteObject(icon.hbmColor);
			DeleteObject(icon.hbmMask);
		}
	}

	DrawIconEx(hdc, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hbrFlickerFreeDraw, diFlags); 

	if (skin.bNeedLayerUpdate && pBits != NULL) {
		RECT rc;
		SetRect(&rc, xLeft, yTop, xLeft + cxWidth, yTop + cyWidth);
		RestoreAlpha(&rc, pBits);
	}
}

int DrawTextAlpha(HDC hdc, LPCTSTR lpString, int nCount, LPRECT lpRect, UINT uFormat)
{
	RECT rc;
	SetRect(&rc, lpRect->left - 1, lpRect->top - 1, lpRect->right + 1, lpRect->bottom + 1);

	mir_ptr<COLOR32> pBits;
	if (skin.bNeedLayerUpdate) pBits = SaveAlpha(&rc);
	int result = DrawText(hdc, lpString, nCount, lpRect, uFormat);
	if (skin.bNeedLayerUpdate) RestoreAlpha(&rc, pBits);

	return result;
}

static __forceinline COLOR32 rgba(COLOR32 r, COLOR32 g, COLOR32 b, COLOR32 a)
{
	return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
}

static __forceinline COLOR32 getr(COLOR32 c) 
{
	return (c >> 16) & 0xff;
}

static __forceinline COLOR32 getg(COLOR32 c) 
{
	return (c >> 8) & 0xff;
}

static __forceinline COLOR32 getb(COLOR32 c) 
{
	return c & 0xff;
}

static __forceinline COLOR32 geta(COLOR32 c)
{
	return (c >> 24) & 0xff;
}

void PremultipleChannels()
{
	for (int i = 0; i < skin.iWidth * skin.iHeight; i++)
		skin.colBits[i] = rgba(	getr(skin.colBits[i])*geta(skin.colBits[i])/255, 
								getg(skin.colBits[i])*geta(skin.colBits[i])/255, 
								getb(skin.colBits[i])*geta(skin.colBits[i])/255, 
								geta(skin.colBits[i]));
}

#define	PU_DIV255(x)	((x)/255)

void ColorizeBitmap()
{
	if (!skin.colBits) 
		return;

	GdiFlush();

	int w = skin.iWidth;
	int h = skin.iHeight;

	// we should swap B and R channels when working with win32 COLORREF
	float koef1r = (255 - getb(opt.colBg)) / 128.0;
	float koef1g = (255 - getg(opt.colBg)) / 128.0;
	float koef1b = (255 - getr(opt.colBg)) / 128.0;

	int br = - 255 + 2 * getb(opt.colBg);
	int bg = - 255 + 2 * getg(opt.colBg);
	int bb = - 255 + 2 * getr(opt.colBg);

	float koef2r = (getb(opt.colBg)) / 128.0;
	float koef2g = (getg(opt.colBg)) / 128.0;
	float koef2b = (getr(opt.colBg)) / 128.0;

	for (int i = 0; i < w * h; i++) {
		long alpha = geta(skin.colBits[i]);
		COLOR32 cl = alpha ? getr(skin.colBits[i])*255/alpha : 0;

		skin.colBits[i] = (cl > 128) ?
			rgba(
				PU_DIV255((koef1r * cl + br)*alpha),
				PU_DIV255((koef1g * cl + bg)*alpha),
				PU_DIV255((koef1b * cl + bb)*alpha),
				alpha):
			rgba(
				PU_DIV255(koef2r * cl * alpha),
				PU_DIV255(koef2g * cl * alpha),
				PU_DIV255(koef2b * cl * alpha),
				alpha);
	}
}

// code from Clist Modern by FYR
HRGN CreateOpaqueRgn(BYTE level, bool bOpaque)
{
	if (!skin.colBits)
		return NULL;

	GdiFlush();

	RGBQUAD *buff = (RGBQUAD *)skin.colBits;

	int x,y;
	unsigned int cRect = 64;
	PRGNDATA pRgnData = (PRGNDATA)malloc(sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
	memset(pRgnData, 0, sizeof(RGNDATAHEADER));
	pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pRgnData->rdh.iType = RDH_RECTANGLES;

	for (y = 0; y < skin.iHeight; ++y) {
		bool inside = false;
		bool lastin = false;
		unsigned int entry = 0;

		for (x = 0; x < skin.iWidth; ++x) {
			inside = bOpaque ? (buff->rgbReserved > level) : (buff->rgbReserved < level);
			++buff;

			if (inside != lastin) {
				if (inside) {
					lastin = true;
					entry = x;
				} 
				else {
					if (pRgnData->rdh.nCount == cRect) {
						cRect = cRect + 64;
						pRgnData = (PRGNDATA)realloc(pRgnData, sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
					}

					SetRect(((LPRECT)pRgnData->Buffer) + pRgnData->rdh.nCount, entry, skin.iHeight - y, x, skin.iHeight - y + 1);
					pRgnData->rdh.nCount++;
					lastin = false;
				}
			}
		}

		if (lastin) {
			if (pRgnData->rdh.nCount == cRect) {
				cRect = cRect + 64;
				pRgnData = (PRGNDATA)realloc(pRgnData, sizeof(RGNDATAHEADER) + (cRect)*sizeof(RECT));
			}

			SetRect(((LPRECT)pRgnData->Buffer) + pRgnData->rdh.nCount, entry, skin.iHeight - y, x, skin.iHeight - y + 1);
			pRgnData->rdh.nCount++;
		}
	}

	HRGN hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount*sizeof(RECT), (LPRGNDATA)pRgnData);
	free(pRgnData);
	return hRgn;
}
