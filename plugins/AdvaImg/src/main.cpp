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

static const PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {7C070F7C-459E-46b7-8E6D-BC6EFAA22F78}
	{0x7c070f7c, 0x459e, 0x46b7, {0x8e, 0x6d, 0xbc, 0x6e, 0xfa, 0xa2, 0x2f, 0x78}}
};

// Resize /////////////////////////////////////////////////////////////////////////////////////////


// Returns a copy of the bitmap with the size especified
// !! the caller is responsible for destroying the original bitmap when it is no longer needed !!
// wParam = ResizeBitmap *
// lParam = NULL
// return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized

static INT_PTR serviceBmpFilterResizeBitmap(WPARAM wParam,LPARAM)
{
	BITMAP bminfo;
	int width, height;
	int xOrig, yOrig, widthOrig, heightOrig;
	ResizeBitmap *info = (ResizeBitmap *) wParam;

	if (info == nullptr || info->size != sizeof(ResizeBitmap)
		|| info->hBmp == nullptr
		|| info->max_width < 0 || info->max_height < 0
		|| (info->fit & ~RESIZEBITMAP_FLAG_DONT_GROW) < RESIZEBITMAP_STRETCH
		|| (info->fit & ~RESIZEBITMAP_FLAG_DONT_GROW) > RESIZEBITMAP_MAKE_SQUARE)
		return 0;

	// Well, lets do it

	// Calc final size
	GetObject(info->hBmp, sizeof(bminfo), &bminfo);

	width = info->max_width == 0 ? bminfo.bmWidth : info->max_width;
	height = info->max_height == 0 ? bminfo.bmHeight : info->max_height;

	xOrig = 0;
	yOrig = 0;
	widthOrig = bminfo.bmWidth;
	heightOrig = bminfo.bmHeight;

	if (widthOrig == 0 || heightOrig == 0)
		return 0;

	switch(info->fit & ~RESIZEBITMAP_FLAG_DONT_GROW)
	{
		case RESIZEBITMAP_STRETCH:
		{
			// Do nothing
			break;
		}
		case RESIZEBITMAP_KEEP_PROPORTIONS:
		{
			if (height * widthOrig / heightOrig <= width)
			{
				if (info->fit & RESIZEBITMAP_FLAG_DONT_GROW)
					height = min(height, bminfo.bmHeight);
				width = height * widthOrig / heightOrig;
			}
			else
			{
				if (info->fit & RESIZEBITMAP_FLAG_DONT_GROW)
					width = min(width, bminfo.bmWidth);
				height = width * heightOrig / widthOrig;
			}

			break;
		}
		case RESIZEBITMAP_MAKE_SQUARE:
		{
			if (info->fit & RESIZEBITMAP_FLAG_DONT_GROW)
			{
				width = min(width, bminfo.bmWidth);
				height = min(height, bminfo.bmHeight);
			}

			width = height = min(width, height);
			// Do not break. Use crop calcs to make size
		}
		case RESIZEBITMAP_CROP:
		{
			if (heightOrig * width / height >= widthOrig)
			{
				heightOrig = widthOrig * height / width;
				yOrig = (bminfo.bmHeight - heightOrig) / 2;
			}
			else
			{
				widthOrig = heightOrig * width / height;
				xOrig = (bminfo.bmWidth - widthOrig) / 2;
			}

			break;
		}
	}

	if ((width == bminfo.bmWidth && height == bminfo.bmHeight)
		|| ((info->fit & RESIZEBITMAP_FLAG_DONT_GROW)
			&& !(info->fit & RESIZEBITMAP_MAKE_SQUARE)
			&& width > bminfo.bmWidth && height > bminfo.bmHeight))
	{
		// Do nothing
		return (INT_PTR)info->hBmp;
	}
	else
	{
		FIBITMAP *dib = FreeImage_CreateDIBFromHBITMAP(info->hBmp);
		if (dib == nullptr)
			return NULL;

		FIBITMAP *dib_tmp;
		if (xOrig > 0 || yOrig > 0)
			dib_tmp = FreeImage_Copy(dib, xOrig, yOrig, xOrig + widthOrig, yOrig + heightOrig);
		else
			dib_tmp = dib;

		if (dib_tmp == nullptr)
		{
			FreeImage_Unload(dib);
			return NULL;
		}

		FIBITMAP *dib_new = FreeImage_Rescale(dib_tmp, width, height, FILTER_CATMULLROM);

		HBITMAP bitmap_new = FreeImage_CreateHBITMAPFromDIB(dib_new);

		if (dib_new != dib_tmp)
			FreeImage_Unload(dib_new);
		if (dib_tmp != dib)
			FreeImage_Unload(dib_tmp);
        FreeImage_Unload(dib);

		return (INT_PTR)bitmap_new;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Load - initializes the plugin instance

static INT_PTR serviceLoad(WPARAM wParam, LPARAM lParam)
{
	char *lpszFilename = (char *)wParam;
	if(lpszFilename==nullptr) return 0;
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	if(lParam & IMGL_WCHAR)
		fif = FreeImage_GetFileTypeU((wchar_t *)lpszFilename, 0);
	else
		fif = FreeImage_GetFileType(lpszFilename, 0);

	if(fif == FIF_UNKNOWN) {
		if(lParam & IMGL_WCHAR)
			fif = FreeImage_GetFIFFromFilenameU((wchar_t *)lpszFilename);
		else
			fif = FreeImage_GetFIFFromFilename(lpszFilename);
	}
	// check that the plugin has reading capabilities ...

	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		// ok, let's load the file
		FIBITMAP *dib;

		if (lParam & IMGL_WCHAR)
			dib = FreeImage_LoadU(fif, (wchar_t *)lpszFilename, 0);
		else
			dib = FreeImage_Load(fif, lpszFilename, 0);

		if(dib == nullptr || (lParam & IMGL_RETURNDIB))
			return (INT_PTR)dib;

		HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);
		FreeImage_Unload(dib);
		FreeImage_CorrectBitmap32Alpha(hbm, FALSE);
		return ((INT_PTR)hbm);
	}
	return NULL;
}

static INT_PTR serviceLoadFromMem(WPARAM wParam, LPARAM lParam)
{
	IMGSRVC_MEMIO *mio = (IMGSRVC_MEMIO *)wParam;
	if(mio->iLen == 0 || mio->pBuf == nullptr)
		return 0;

	FIMEMORY *hmem = FreeImage_OpenMemory((BYTE *)mio->pBuf, mio->iLen);
	FREE_IMAGE_FORMAT fif = (mio->fif != FIF_UNKNOWN) ? mio->fif : mio->fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
	FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, mio->flags);
	FreeImage_CloseMemory(hmem);

	if(dib == nullptr || (lParam & IMGL_RETURNDIB))
		return (INT_PTR)dib;

	HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);

	FreeImage_Unload(dib);
	return (INT_PTR)hbm;
}

static INT_PTR serviceUnload(WPARAM wParam, LPARAM)
{
	FIBITMAP *dib = (FIBITMAP *)wParam;

	if(dib)
		FreeImage_Unload(dib);

	return 0;
}

static INT_PTR serviceSave(WPARAM wParam, LPARAM lParam)
{
	IMGSRVC_INFO *isi = (IMGSRVC_INFO *)wParam;
	FREE_IMAGE_FORMAT fif;
	BOOL fUnload = FALSE;
	FIBITMAP *dib = nullptr;

	if(isi) {
		if(isi->cbSize != sizeof(IMGSRVC_INFO))
			return 0;

		if(isi->szName || isi->wszName) {
			if(isi->fif == FIF_UNKNOWN) {
				if(lParam & IMGL_WCHAR)
					fif = FreeImage_GetFIFFromFilenameU(isi->wszName);
				else
					fif = FreeImage_GetFIFFromFilename(isi->szName);
			}
			else
				fif = isi->fif;

			if(fif == FIF_UNKNOWN)
				fif = FIF_BMP;                  // default, save as bmp

			if(isi->hbm != nullptr && (isi->dwMask & IMGI_HBITMAP) && !(isi->dwMask & IMGI_FBITMAP)) {
				// create temporary dib, because we got a HBTIMAP passed
				fUnload = TRUE;
				FreeImage_CorrectBitmap32Alpha(isi->hbm, FALSE);
				dib = FreeImage_CreateDIBFromHBITMAP(isi->hbm);
			}
			else if(isi->dib != nullptr && (isi->dwMask & IMGI_FBITMAP) && !(isi->dwMask & IMGI_HBITMAP))
				dib = isi->dib;

			int ret = 0;
			if(dib) {
				int flags = HIWORD(lParam);

				if(fif == FIF_PNG || fif == FIF_BMP || fif == FIF_JNG)
				{
					if(lParam & IMGL_WCHAR)
						ret = FreeImage_SaveU(fif, dib, isi->wszName, flags);
					else
						ret = FreeImage_Save(fif, dib, isi->szName, flags);
				}
				else
				{
					FIBITMAP *dib_new = FreeImage_ConvertTo24Bits(dib);
					if(lParam & IMGL_WCHAR)
						ret = FreeImage_SaveU(fif, dib_new, isi->wszName, flags);
					else
						ret = FreeImage_Save(fif, dib_new, isi->szName, flags);
					FreeImage_Unload(dib_new);
				}

				if(fUnload)
					FreeImage_Unload(dib);
				return ret;
			}
			return 0;
		}

	}
	return 0;
}

static int IMGSERVICE_Load()
{
	CreateServiceFunction(MS_IMG_LOAD, serviceLoad);
	CreateServiceFunction(MS_IMG_LOADFROMMEM, serviceLoadFromMem);
	CreateServiceFunction(MS_IMG_SAVE, serviceSave);
	CreateServiceFunction(MS_IMG_UNLOAD, serviceUnload);
	CreateServiceFunction(MS_IMG_RESIZE, serviceBmpFilterResizeBitmap);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	return IMGSERVICE_Load();
}

///////////////////////////////////////////////////////////////////////////////
// Unload - destroys the plugin instance

static int IMGSERVICE_Unload( void )
{
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return IMGSERVICE_Unload();
}

extern "C" __declspec(dllexport) const PLUGININFOEX * MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}
