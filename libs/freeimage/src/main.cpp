/*
Plugin of Miranda IM for reading/writing PNG images.
Copyright (c) 2004-06 George Hazan (ghazan@postman.ru)

Portions of this code are gotten from the libpng codebase.
Copyright 2000, Willem van Schaik.  For conditions of distribution and
use, see the copyright/license/disclaimer notice in png.h

Miranda IM: the free icq client for MS Windows
Copyright (C) 2000-2002 Richard Hughes, Roland Rabien & Tristan Van de Vreede

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

#include "stdafx.h"

/*
* freeimage helper functions
*/

// Correct alpha from bitmaps loaded without it (it cames with 0 and should be 255)
// originally in loadavatars...

EXTERN_C DLL_API void DLL_CALLCONV FreeImage_CorrectBitmap32Alpha(HBITMAP hBitmap, BOOL force)
{
	BITMAP bmp;
	uint32_t dwLen;
	uint8_t *p;
	int x, y;
	BOOL fixIt;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	p = (uint8_t *)malloc(dwLen);
	if (p == nullptr)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	fixIt = TRUE;
	for (y = 0; fixIt && y < bmp.bmHeight; ++y) {
		uint8_t *px = p + bmp.bmWidth * 4 * y;

		for (x = 0; fixIt && x < bmp.bmWidth; ++x)
		{
			if (px[3] != 0 && !force)
			{
				fixIt = FALSE;
			}
			else
			{
				px[3] = 255;
			}

			px += 4;
		}
	}

	if (fixIt)
		SetBitmapBits(hBitmap, dwLen, p);

	free(p);
}

/*
* needed for per pixel transparent images. Such images should then be rendered by
* using AlphaBlend() with AC_SRC_ALPHA
* dwFlags will be set to AVS_PREMULTIPLIED
* return TRUE if the image has at least one pixel with transparency
*/

EXTERN_C DLL_API BOOL DLL_CALLCONV FreeImage_Premultiply(HBITMAP hBitmap)
{
	BOOL transp = FALSE;

	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel == 32) {
		int width = bmp.bmWidth;
		int height = bmp.bmHeight;
		int dwLen = width * height * 4;
		uint8_t *p = (uint8_t *)malloc(dwLen);
		if (p != nullptr) {
			GetBitmapBits(hBitmap, dwLen, p);

			for (int y = 0; y < height; ++y) {
				uint8_t *px = p + width * 4 * y;
				for (int x = 0; x < width; ++x) {
					uint8_t alpha = px[3];
					if (alpha < 255) {
						transp = TRUE;

						px[0] = px[0] * alpha/255;
						px[1] = px[1] * alpha/255;
						px[2] = px[2] * alpha/255;
					}

					px += 4;
				}
			}

			if (transp)
				dwLen = SetBitmapBits(hBitmap, dwLen, p);
			free(p);
		}
	}

	return transp;
}

EXTERN_C DLL_API HBITMAP DLL_CALLCONV FreeImage_CreateHBITMAPFromDIB(FIBITMAP *in)
{
    FIBITMAP *dib = nullptr;
	int bpp = FreeImage_GetBPP(in);

	if (bpp == 48)
		dib = FreeImage_ConvertTo24Bits(in);
	else if (FreeImage_GetBPP(in) > 32)
		dib = FreeImage_ConvertTo32Bits(in);
    else
        dib = in;

	uint8_t *ptPixels;
	BITMAPINFO *info = FreeImage_GetInfo(dib);
	HBITMAP hBmp = CreateDIBSection(nullptr, info, DIB_RGB_COLORS, (void **)&ptPixels, nullptr, 0);
	if (ptPixels != nullptr)
		memmove(ptPixels, FreeImage_GetBits(dib), FreeImage_GetPitch(dib) * FreeImage_GetHeight(dib));

	if (dib != in)
		FreeImage_Unload(dib);

	return hBmp;
}

EXTERN_C DLL_API FIBITMAP* DLL_CALLCONV FreeImage_CreateDIBFromHBITMAP(HBITMAP hBmp)
{
	if (!hBmp)
		return nullptr;
		
	BITMAP bm;
	GetObject(hBmp, sizeof(BITMAP), (LPSTR) &bm);
	FIBITMAP *dib = FreeImage_Allocate(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel,0,0,0);
	// The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (dont't know why)
	// So we save these infos below. This is needed for palettized images only.
	int nColors = FreeImage_GetColorsUsed(dib);
	HDC dc = GetDC(nullptr);
	GetDIBits(dc, hBmp, 0, FreeImage_GetHeight(dib), FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
	ReleaseDC(nullptr, dc);
	// restore BITMAPINFO members
	FreeImage_GetInfoHeader(dib)->biClrUsed = nColors;
	FreeImage_GetInfoHeader(dib)->biClrImportant = nColors;
	return dib;
}
