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

// Resize /////////////////////////////////////////////////////////////////////////////////////////

// Returns a copy of the bitmap with the size especified
// !! the caller is responsible for destroying the original bitmap when it is no longer needed !!
// wParam = ResizeBitmap *
// lParam = NULL
// return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized

MIR_APP_DLL(HBITMAP) Image_Resize(HBITMAP hBmp, int fit /* RESIZEBITMAP_*/, int max_width, int max_height)
{
	if (hBmp == nullptr
		|| max_width < 0 || max_height < 0
		|| (fit & ~RESIZEBITMAP_FLAG_DONT_GROW) < RESIZEBITMAP_STRETCH
		|| (fit & ~RESIZEBITMAP_FLAG_DONT_GROW) > RESIZEBITMAP_MAKE_SQUARE)
		return 0;

	// Well, lets do it

	// Calc final size
	BITMAP bminfo;
	GetObject(hBmp, sizeof(bminfo), &bminfo);

	int width = max_width == 0 ? bminfo.bmWidth : max_width;
	int height = max_height == 0 ? bminfo.bmHeight : max_height;

	int xOrig = 0;
	int yOrig = 0;
	int widthOrig = bminfo.bmWidth;
	int heightOrig = bminfo.bmHeight;

	if (widthOrig == 0 || heightOrig == 0)
		return 0;

	switch (fit & ~RESIZEBITMAP_FLAG_DONT_GROW) {
	case RESIZEBITMAP_STRETCH:
		// Do nothing
		break;

	case RESIZEBITMAP_KEEP_PROPORTIONS:
		if (height * widthOrig / heightOrig <= width) {
			if (fit & RESIZEBITMAP_FLAG_DONT_GROW)
				height = min(height, (int)bminfo.bmHeight);
			width = height * widthOrig / heightOrig;
		}
		else {
			if (fit & RESIZEBITMAP_FLAG_DONT_GROW)
				width = min(width, (int)bminfo.bmWidth);
			height = width * heightOrig / widthOrig;
		}
		break;

	case RESIZEBITMAP_MAKE_SQUARE:
		if (fit & RESIZEBITMAP_FLAG_DONT_GROW) {
			width = min(width, (int)bminfo.bmWidth);
			height = min(height, (int)bminfo.bmHeight);
		}

		width = height = min(width, height);
		// Do not break. Use crop calcs to make size

	case RESIZEBITMAP_CROP:
		if (heightOrig * width / height >= widthOrig) {
			heightOrig = widthOrig * height / width;
			yOrig = (bminfo.bmHeight - heightOrig) / 2;
		}
		else {
			widthOrig = heightOrig * width / height;
			xOrig = (bminfo.bmWidth - widthOrig) / 2;
		}
		break;
	}

	if ((width == bminfo.bmWidth && height == bminfo.bmHeight) ||
		 ((fit & RESIZEBITMAP_FLAG_DONT_GROW) && !(fit & RESIZEBITMAP_MAKE_SQUARE) && width > bminfo.bmWidth && height > bminfo.bmHeight)) {
		// Do nothing
		return hBmp;
	}

	FIBITMAP *dib = FreeImage_CreateDIBFromHBITMAP(hBmp);
	if (dib == nullptr)
		return 0;

	FIBITMAP *dib_tmp;
	if (xOrig > 0 || yOrig > 0)
		dib_tmp = FreeImage_Copy(dib, xOrig, yOrig, xOrig + widthOrig, yOrig + heightOrig);
	else
		dib_tmp = dib;

	if (dib_tmp == nullptr) {
		FreeImage_Unload(dib);
		return 0;
	}

	FIBITMAP *dib_new = FreeImage_Rescale(dib_tmp, width, height, FILTER_CATMULLROM);
	HBITMAP bitmap_new = FreeImage_CreateHBITMAPFromDIB(dib_new);

	if (dib_new != dib_tmp)
		FreeImage_Unload(dib_new);
	if (dib_tmp != dib)
		FreeImage_Unload(dib_tmp);
	FreeImage_Unload(dib);

	return bitmap_new;
}

///////////////////////////////////////////////////////////////////////////////
// Image_Load - initializes the plugin instance

MIR_APP_DLL(HBITMAP) Image_Load(const wchar_t *pwszPath, int iFlags)
{
	if (pwszPath == nullptr)
		return 0;
	
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(pwszPath, 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilenameU(pwszPath);
	
	// check that the plugin has reading capabilities ...
	if (fif != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fif)) {
		// ok, let's load the file
		FIBITMAP *dib = FreeImage_LoadU(fif, pwszPath, 0);
		if (dib == nullptr || (iFlags & IMGL_RETURNDIB))
			return HBITMAP(dib);

		HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);
		FreeImage_Unload(dib);
		FreeImage_CorrectBitmap32Alpha(hbm, FALSE);
		return hbm;
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Image_LoadFromMem - loads an image from memory

MIR_APP_DLL(HBITMAP) Image_LoadFromMem(const void *pBuf, size_t cbLen, FREE_IMAGE_FORMAT fif)
{
	if (cbLen == 0 || pBuf == nullptr)
		return 0;

	FIMEMORY *hmem = FreeImage_OpenMemory((uint8_t *)pBuf, (uint32_t)cbLen);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFileTypeFromMemory(hmem, 0);

	FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, 0);
	FreeImage_CloseMemory(hmem);
	if (dib == nullptr)
		return nullptr;

	HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);
	FreeImage_Unload(dib);
	return hbm;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Image_Save(const IMGSRVC_INFO *isi, int iFlags)
{
	if (isi == nullptr)
		return 0;
	if (isi->cbSize != sizeof(IMGSRVC_INFO))
		return 0;
	if (isi->pwszName == nullptr)
		return 0;

	FREE_IMAGE_FORMAT fif;
	BOOL fUnload = FALSE;
	FIBITMAP *dib = nullptr;

	if (isi->fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilenameU(isi->pwszName);
	else
		fif = isi->fif;

	if (fif == FIF_UNKNOWN) // default, save as bmp
		fif = FIF_BMP;

	if (isi->hbm != nullptr && (isi->dwMask & IMGI_HBITMAP) && !(isi->dwMask & IMGI_FBITMAP)) {
		// create temporary dib, because we got a HBTIMAP passed
		fUnload = TRUE;
		FreeImage_CorrectBitmap32Alpha(isi->hbm, FALSE);
		dib = FreeImage_CreateDIBFromHBITMAP(isi->hbm);
	}
	else if (isi->dib != nullptr && (isi->dwMask & IMGI_FBITMAP) && !(isi->dwMask & IMGI_HBITMAP))
		dib = isi->dib;

	if (dib == nullptr)
		return 0;

	int ret = 0;
	if (fif == FIF_PNG || fif == FIF_BMP/* || fif == FIF_JNG*/)
		ret = FreeImage_SaveU(fif, dib, isi->pwszName, iFlags);
	else {
		FIBITMAP *dib_new = FreeImage_ConvertTo24Bits(dib);
		ret = FreeImage_SaveU(fif, dib_new, isi->pwszName, iFlags);
		FreeImage_Unload(dib_new);
	}

	if (fUnload)
		FreeImage_Unload(dib);
	return ret;
}
