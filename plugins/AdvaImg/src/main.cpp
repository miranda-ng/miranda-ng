/*
Plugin of Miranda IM for reading/writing PNG images.
Copyright (c) 2004-6 George Hazan (ghazan@postman.ru)

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

#include <windows.h>
#include <commdlg.h>
#include <malloc.h>

#include <m_clui.h>

#include <m_imgsrvc.h>

#include "newpluginapi.h"
#include "version.h"

#include "libpng/png.h"

static const PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {7C070F7C-459E-46b7-8E6D-BC6EFAA22F78}
	{0x7c070f7c, 0x459e, 0x46b7, {0x8e, 0x6d, 0xbc, 0x6e, 0xfa, 0xa2, 0x2f, 0x78}}
};

/*
* freeimage helper functions
*/

// Correct alpha from bitmaps loaded without it (it cames with 0 and should be 255)
// originally in loadavatars...

static void FI_CorrectBitmap32Alpha(HBITMAP hBitmap, BOOL force)
{
	BITMAP bmp;
	DWORD dwLen;
	BYTE *p;
	int x, y;
	BOOL fixIt;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	fixIt = TRUE;
	for (y = 0; fixIt && y < bmp.bmHeight; ++y) {
		BYTE *px = p + bmp.bmWidth * 4 * y;

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
static BOOL FreeImage_PreMultiply(HBITMAP hBitmap)
{
	BOOL transp = FALSE;

	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel == 32) {
		int width = bmp.bmWidth;
		int height = bmp.bmHeight;
		int dwLen = width * height * 4;
		BYTE *p = (BYTE *)malloc(dwLen);
		if (p != NULL) {
			GetBitmapBits(hBitmap, dwLen, p);

			for (int y = 0; y < height; ++y) {
				BYTE *px = p + width * 4 * y;
				for (int x = 0; x < width; ++x) {
					BYTE alpha = px[3];
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

static HBITMAP FreeImage_CreateHBITMAPFromDIB(FIBITMAP *in)
{
    FIBITMAP *dib = NULL;
	int bpp = FreeImage_GetBPP(in);

	if (bpp == 48)
		dib = FreeImage_ConvertTo24Bits(in);
	else if (FreeImage_GetBPP(in) > 32)
		dib = FreeImage_ConvertTo32Bits(in);
    else
        dib = in;

	BYTE *ptPixels;
	BITMAPINFO *info = FreeImage_GetInfo(dib);
	HBITMAP hBmp = CreateDIBSection(NULL, info, DIB_RGB_COLORS, (void **)&ptPixels, NULL, 0);
	if (ptPixels != NULL)
		memmove(ptPixels, FreeImage_GetBits(dib), FreeImage_GetPitch(dib) * FreeImage_GetHeight(dib));

	if (dib != in)
		FreeImage_Unload(dib);

	return hBmp;
}

static FIBITMAP *FreeImage_CreateDIBFromHBITMAP(HBITMAP hBmp)
{
	BITMAP bm;

	if(hBmp) {
		GetObject(hBmp, sizeof(BITMAP), (LPSTR) &bm);
		FIBITMAP *dib = FreeImage_Allocate(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel,0,0,0);
		// The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (dont't know why)
		// So we save these infos below. This is needed for palettized images only.
		int nColors = FreeImage_GetColorsUsed(dib);
		HDC dc = GetDC(NULL);
		int Success = GetDIBits(dc, hBmp, 0, FreeImage_GetHeight(dib),
			FreeImage_GetBits(dib), FreeImage_GetInfo(dib), DIB_RGB_COLORS);
		ReleaseDC(NULL, dc);
		// restore BITMAPINFO members
		FreeImage_GetInfoHeader(dib)->biClrUsed = nColors;
		FreeImage_GetInfoHeader(dib)->biClrImportant = nColors;
		return dib;
	}
	return NULL;
}


// Resize /////////////////////////////////////////////////////////////////////////////////////////


// Returns a copy of the bitmap with the size especified
// !! the caller is responsible for destroying the original bitmap when it is no longer needed !!
// wParam = ResizeBitmap *
// lParam = NULL
// return NULL on error, ResizeBitmap->hBmp if don't need to resize or a new HBITMAP if resized

static INT_PTR serviceBmpFilterResizeBitmap(WPARAM wParam,LPARAM lParam)
{
	BITMAP bminfo;
	int width, height;
	int xOrig, yOrig, widthOrig, heightOrig;
	ResizeBitmap *info = (ResizeBitmap *) wParam;

	if (info == NULL || info->size != sizeof(ResizeBitmap)
		|| info->hBmp == NULL
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
		if (dib == NULL)
			return NULL;

		FIBITMAP *dib_tmp;
		if (xOrig > 0 || yOrig > 0)
			dib_tmp = FreeImage_Copy(dib, xOrig, yOrig, xOrig + widthOrig, yOrig + heightOrig);
		else
			dib_tmp = dib;

		if (dib_tmp == NULL)
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

FIBITMAP* FreeImage_LoadFromMem(FREE_IMAGE_FORMAT fif, fiio_mem_handle *handle, int flags)
{
	if (handle && handle->data) {
		FIMEMORY *hmem = FreeImage_OpenMemory((BYTE *)handle->data, handle->datalen);
		FREE_IMAGE_FORMAT _fif = (fif != FIF_UNKNOWN) ? fif : FreeImage_GetFileTypeFromMemory(hmem, 0);
		FIBITMAP *dib = FreeImage_LoadFromMemory(_fif, hmem, flags);
		FreeImage_CloseMemory(hmem);
	}

	return NULL;
}

FIMEMORY* FreeImage_SaveToMem(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, fiio_mem_handle *handle, int flags)
{
	if (handle) {
		FIMEMORY *hmem = FreeImage_OpenMemory(NULL, 0);
		if(fif == FIF_UNKNOWN)
			fif = FIF_BMP;
		handle->curpos = 0;
		FreeImage_SaveToMemory(fif, dib, hmem, flags);
		FreeImage_AcquireMemory(hmem, (BYTE **)&handle->data, (DWORD *)&handle->datalen);
		return hmem;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Load - initializes the plugin instance

FI_INTERFACE feif = {0};

/*
* caller MUST supply the desired version in wParam
* if it doesn't match, error will be returned
*/

static INT_PTR serviceGetInterface(WPARAM wParam, LPARAM lParam)
{
	FI_INTERFACE **ppfe = (FI_INTERFACE **)lParam;

	if ((DWORD)wParam != FI_IF_VERSION)
		return CALLSERVICE_NOTFOUND;

	if(ppfe) {
		*ppfe = &feif;
		return S_OK;
	}
	else
		return CALLSERVICE_NOTFOUND;
}

static INT_PTR serviceLoad(WPARAM wParam, LPARAM lParam)
{
	char *lpszFilename = (char *)wParam;
	if(lpszFilename==NULL) return 0;
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

		if(dib == NULL || (lParam & IMGL_RETURNDIB))
			return (INT_PTR)dib;

		HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);
		FreeImage_Unload(dib);
		FI_CorrectBitmap32Alpha(hbm, FALSE);
		return ((INT_PTR)hbm);
	}
	return NULL;
}

static INT_PTR serviceLoadFromMem(WPARAM wParam, LPARAM lParam)
{
	IMGSRVC_MEMIO *mio = (IMGSRVC_MEMIO *)wParam;
	if(mio->iLen == 0 || mio->pBuf == NULL)
		return 0;

	FIMEMORY *hmem = FreeImage_OpenMemory((BYTE *)mio->pBuf, mio->iLen);
	FREE_IMAGE_FORMAT fif = (mio->fif != FIF_UNKNOWN) ? mio->fif : mio->fif = FreeImage_GetFileTypeFromMemory(hmem, 0);
	FIBITMAP *dib = FreeImage_LoadFromMemory(fif, hmem, mio->flags);
	FreeImage_CloseMemory(hmem);

	if(dib == NULL || (lParam & IMGL_RETURNDIB))
		return (INT_PTR)dib;

	HBITMAP hbm = FreeImage_CreateHBITMAPFromDIB(dib);

	FreeImage_Unload(dib);
	return (INT_PTR)hbm;
}

static INT_PTR serviceUnload(WPARAM wParam, LPARAM lParam)
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
	FIBITMAP *dib = NULL;

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

			if(isi->hbm != 0 && (isi->dwMask & IMGI_HBITMAP) && !(isi->dwMask & IMGI_FBITMAP)) {
				// create temporary dib, because we got a HBTIMAP passed
				fUnload = TRUE;
				FI_CorrectBitmap32Alpha(isi->hbm, FALSE);
				dib = FreeImage_CreateDIBFromHBITMAP(isi->hbm);
			}
			else if(isi->dib != NULL && (isi->dwMask & IMGI_FBITMAP) && !(isi->dwMask & IMGI_HBITMAP))
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

static INT_PTR serviceGetVersion(WPARAM wParam, LPARAM lParam)
{
	return FI_IF_VERSION;
}

DLL_API FIBITMAP *DLL_CALLCONV FreeImage_RotateClassic(FIBITMAP *dib, double angle)
{
	return FreeImage_Rotate(dib, angle, NULL);
}

void FI_Populate(void)
{
	/*
	* populate the interface
	*/

	feif.version = FI_IF_VERSION;

	feif.FI__AllocateT = FreeImage_AllocateT;
	feif.FI_Allocate = FreeImage_Allocate;
	feif.FI_Clone = FreeImage_Clone;
	feif.FI_Unload = FreeImage_Unload;

	feif.FI_Load = FreeImage_Load;
	feif.FI_LoadFromHandle = FreeImage_LoadFromHandle;
	feif.FI_LoadU = FreeImage_LoadU;
	feif.FI_Save = FreeImage_Save;
	feif.FI_SaveToHandle = FreeImage_SaveToHandle;
	feif.FI_SaveU = FreeImage_SaveU;

	feif.FI_OpenMemory = FreeImage_OpenMemory;
	feif.FI_CloseMemory = FreeImage_CloseMemory;
	feif.FI_LoadFromMemory = FreeImage_LoadFromMemory;
	feif.FI_SaveToMemory = FreeImage_SaveToMemory;
	feif.FI_TellMemory = FreeImage_TellMemory;
	feif.FI_SeekMemory = FreeImage_SeekMemory;
	feif.FI_AcquireMemory = FreeImage_AcquireMemory;
	feif.FI_ReadMemory = FreeImage_ReadMemory;
	feif.FI_WriteMemory = FreeImage_WriteMemory;
	feif.FI_LoadMultiBitmapFromMemory = FreeImage_LoadMultiBitmapFromMemory;

	feif.FI_OpenMultiBitmap = FreeImage_OpenMultiBitmap;
	feif.FI_OpenMultiBitmapU = FreeImage_OpenMultiBitmapU;
	feif.FI_CloseMultiBitmap = FreeImage_CloseMultiBitmap;
	feif.FI_GetPageCount = FreeImage_GetPageCount;
	feif.FI_AppendPage = FreeImage_AppendPage;
	feif.FI_InsertPage = FreeImage_InsertPage;
	feif.FI_DeletePage = FreeImage_DeletePage;
	feif.FI_LockPage = FreeImage_LockPage;
	feif.FI_UnlockPage = FreeImage_UnlockPage;
	feif.FI_MovePage = FreeImage_MovePage;
	feif.FI_GetLockedPageNumbers = FreeImage_GetLockedPageNumbers;

	feif.FI_GetFileType = FreeImage_GetFileType;
	feif.FI_GetFileTypeU = FreeImage_GetFileTypeU;
	feif.FI_GetFileTypeFromHandle = FreeImage_GetFileTypeFromHandle;
	feif.FI_GetFileTypeFromMemory = FreeImage_GetFileTypeFromMemory;
	feif.FI_GetImageType = FreeImage_GetImageType;

	feif.FI_GetBits = FreeImage_GetBits;
	feif.FI_GetScanLine = FreeImage_GetScanLine;
	feif.FI_GetPixelIndex = FreeImage_GetPixelIndex;
	feif.FI_GetPixelColor = FreeImage_GetPixelColor;
	feif.FI_SetPixelColor = FreeImage_SetPixelColor;
	feif.FI_SetPixelIndex = FreeImage_SetPixelIndex;

	feif.FI_GetColorsUsed = FreeImage_GetColorsUsed;
	feif.FI_GetBPP = FreeImage_GetBPP;
	feif.FI_GetWidth = FreeImage_GetWidth;
	feif.FI_GetHeight = FreeImage_GetHeight;
	feif.FI_GetLine = FreeImage_GetLine;
	feif.FI_GetPitch = FreeImage_GetPitch;
	feif.FI_GetDIBSize = FreeImage_GetDIBSize;
	feif.FI_GetPalette = FreeImage_GetPalette;
	feif.FI_GetDotsPerMeterX = FreeImage_GetDotsPerMeterX;
	feif.FI_GetDotsPerMeterY = FreeImage_GetDotsPerMeterY;
	feif.FI_SetDotsPerMeterX = FreeImage_SetDotsPerMeterX;
	feif.FI_SetDotsPerMeterY = FreeImage_SetDotsPerMeterY;
	feif.FI_GetInfoHeader = FreeImage_GetInfoHeader;
	feif.FI_GetInfo = FreeImage_GetInfo;
	feif.FI_GetColorType = FreeImage_GetColorType;
	feif.FI_GetRedMask = FreeImage_GetRedMask;
	feif.FI_GetGreenMask = FreeImage_GetGreenMask;
	feif.FI_GetBlueMask = FreeImage_GetBlueMask;
	feif.FI_GetTransparencyCount = FreeImage_GetTransparencyCount;
	feif.FI_GetTransparencyTable = FreeImage_GetTransparencyTable;
	feif.FI_SetTransparent = FreeImage_SetTransparent;
	feif.FI_SetTransparencyTable = FreeImage_SetTransparencyTable;
	feif.FI_IsTransparent = FreeImage_IsTransparent;
	feif.FI_HasBackgroundColor = FreeImage_HasBackgroundColor;
	feif.FI_GetBackgroundColor = FreeImage_GetBackgroundColor;
	feif.FI_SetBackgroundColor = FreeImage_SetBackgroundColor;

	feif.FI_ConvertTo4Bits = FreeImage_ConvertTo4Bits;
	feif.FI_ConvertTo8Bits = FreeImage_ConvertTo8Bits;
	feif.FI_ConvertToGreyscale = FreeImage_ConvertToGreyscale;
	feif.FI_ConvertTo16Bits555 = FreeImage_ConvertTo16Bits555;
	feif.FI_ConvertTo16Bits565 = FreeImage_ConvertTo16Bits565;
	feif.FI_ConvertTo24Bits = FreeImage_ConvertTo24Bits;
	feif.FI_ConvertTo32Bits = FreeImage_ConvertTo32Bits;
	feif.FI_ColorQuantize = FreeImage_ColorQuantize;
	feif.FI_ColorQuantizeEx = FreeImage_ColorQuantizeEx;
	feif.FI_Threshold = FreeImage_Threshold;
	feif.FI_Dither = FreeImage_Dither;
	feif.FI_ConvertFromRawBits = FreeImage_ConvertFromRawBits;
	feif.FI_ConvertToRawBits = FreeImage_ConvertToRawBits;
	feif.FI_ConvertToRGBF = FreeImage_ConvertToRGBF;
	feif.FI_ConvertToStandardType = FreeImage_ConvertToStandardType;
	feif.FI_ConvertToType = FreeImage_ConvertToType;

	feif.FI_RegisterLocalPlugin = FreeImage_RegisterLocalPlugin;
	feif.FI_RegisterExternalPlugin = FreeImage_RegisterExternalPlugin;
	feif.FI_GetFIFCount = FreeImage_GetFIFCount;
	feif.FI_SetPluginEnabled = FreeImage_SetPluginEnabled;
	feif.FI_IsPluginEnabled = FreeImage_IsPluginEnabled;
	feif.FI_GetFIFFromFormat = FreeImage_GetFIFFromFormat;
	feif.FI_GetFIFFromMime = FreeImage_GetFIFFromMime;
	feif.FI_GetFormatFromFIF = FreeImage_GetFormatFromFIF;
	feif.FI_GetFIFExtensionList = FreeImage_GetFIFExtensionList;
	feif.FI_GetFIFDescription = FreeImage_GetFIFDescription;
	feif.FI_GetFIFRegExpr = FreeImage_GetFIFRegExpr;
	feif.FI_GetFIFMimeType = FreeImage_GetFIFMimeType;
	feif.FI_GetFIFFromFilename = FreeImage_GetFIFFromFilename;
	feif.FI_GetFIFFromFilenameU = FreeImage_GetFIFFromFilenameU;
	feif.FI_FIFSupportsReading = FreeImage_FIFSupportsReading;
	feif.FI_FIFSupportsWriting = FreeImage_FIFSupportsWriting;
	feif.FI_FIFSupportsExportBPP = FreeImage_FIFSupportsExportBPP;
	feif.FI_FIFSupportsExportType = FreeImage_FIFSupportsExportType;
	feif.FI_FIFSupportsICCProfiles = FreeImage_FIFSupportsICCProfiles;

	feif.FI_RotateClassic = FreeImage_RotateClassic;
	feif.FI_RotateEx = FreeImage_RotateEx;
	feif.FI_FlipHorizontal = FreeImage_FlipHorizontal;
	feif.FI_FlipVertical = FreeImage_FlipVertical;
	feif.FI_JPEGTransform = FreeImage_JPEGTransform;

	feif.FI_Rescale = FreeImage_Rescale;
	feif.FI_MakeThumbnail = FreeImage_MakeThumbnail;

	feif.FI_AdjustCurve = FreeImage_AdjustCurve;
	feif.FI_AdjustGamma = FreeImage_AdjustGamma;
	feif.FI_AdjustBrightness = FreeImage_AdjustBrightness;
	feif.FI_AdjustContrast = FreeImage_AdjustContrast;
	feif.FI_Invert = FreeImage_Invert;
	feif.FI_GetHistogram = FreeImage_GetHistogram;

	feif.FI_GetChannel = FreeImage_GetChannel;
	feif.FI_SetChannel = FreeImage_SetChannel;
	feif.FI_GetComplexChannel = FreeImage_GetComplexChannel;
	feif.FI_SetComplexChannel = FreeImage_SetComplexChannel;

	feif.FI_Copy = FreeImage_Copy;
	feif.FI_Paste = FreeImage_Paste;
	feif.FI_Composite = FreeImage_Composite;
	feif.FI_JPEGCrop = FreeImage_JPEGCrop;

	feif.FI_LoadFromMem = FreeImage_LoadFromMem;
	feif.FI_SaveToMem = FreeImage_SaveToMem;

	feif.FI_CreateDIBFromHBITMAP = FreeImage_CreateDIBFromHBITMAP;
	feif.FI_CreateHBITMAPFromDIB = FreeImage_CreateHBITMAPFromDIB;

	feif.FI_Premultiply = FreeImage_PreMultiply;
	feif.FI_BmpFilterResizeBitmap = serviceBmpFilterResizeBitmap;

	feif.FI_IsLittleEndian = FreeImage_IsLittleEndian;
	feif.FI_LookupX11Color = FreeImage_LookupX11Color;
	feif.FI_LookupSVGColor = FreeImage_LookupSVGColor;

	feif.FI_GetICCProfile = FreeImage_GetICCProfile;
	feif.FI_CreateICCProfile = FreeImage_CreateICCProfile;
	feif.FI_DestroyICCProfile = FreeImage_DestroyICCProfile;

	feif.FI_ToneMapping = FreeImage_ToneMapping;
	feif.FI_TmoDrago03 = FreeImage_TmoDrago03;
	feif.FI_TmoReinhard05 = FreeImage_TmoReinhard05;

	feif.FI_ZLibCompress = FreeImage_ZLibCompress;
	feif.FI_ZLibUncompress = FreeImage_ZLibUncompress;
	feif.FI_ZLibGZip = FreeImage_ZLibGZip;
	feif.FI_ZLibGUnzip = FreeImage_ZLibGUnzip;
	feif.FI_ZLibCRC32 = FreeImage_ZLibCRC32;

	feif.FI_CreateTag = FreeImage_CreateTag;
	feif.FI_DeleteTag = FreeImage_DeleteTag;
	feif.FI_CloneTag = FreeImage_CloneTag;

	feif.FI_GetTagKey = FreeImage_GetTagKey;
	feif.FI_GetTagDescription = FreeImage_GetTagDescription;
	feif.FI_GetTagID = FreeImage_GetTagID;
	feif.FI_GetTagType = FreeImage_GetTagType;
	feif.FI_GetTagCount = FreeImage_GetTagCount;
	feif.FI_GetTagLength = FreeImage_GetTagLength;
	feif.FI_GetTagValue = FreeImage_GetTagValue;

	feif.FI_SetTagKey = FreeImage_SetTagKey;
	feif.FI_SetTagDescription = FreeImage_SetTagDescription;
	feif.FI_SetTagID = FreeImage_SetTagID;
	feif.FI_SetTagType = FreeImage_SetTagType;
	feif.FI_SetTagCount = FreeImage_SetTagCount;
	feif.FI_SetTagLength = FreeImage_SetTagLength;
	feif.FI_SetTagValue = FreeImage_SetTagValue;

	feif.FI_FindFirstMetadata = FreeImage_FindFirstMetadata;
	feif.FI_FindNextMetadata = FreeImage_FindNextMetadata;
	feif.FI_FindCloseMetadata = FreeImage_FindCloseMetadata;
	feif.FI_SetMetadata = FreeImage_SetMetadata;
	feif.FI_GetMetadata = FreeImage_GetMetadata;

	feif.FI_GetMetadataCount = FreeImage_GetMetadataCount;
	feif.FI_TagToString = FreeImage_TagToString;

	feif.FI_CorrectBitmap32Alpha = FI_CorrectBitmap32Alpha;
}

static int IMGSERVICE_Load()
{
	FI_Populate();

	CreateServiceFunction(MS_IMG_GETINTERFACE, serviceGetInterface);
	CreateServiceFunction(MS_IMG_LOAD, serviceLoad);
	CreateServiceFunction(MS_IMG_LOADFROMMEM, serviceLoadFromMem);
	CreateServiceFunction(MS_IMG_SAVE, serviceSave);
	CreateServiceFunction(MS_IMG_UNLOAD, serviceUnload);
	CreateServiceFunction(MS_IMG_RESIZE, serviceBmpFilterResizeBitmap);
	CreateServiceFunction(MS_IMG_GETIFVERSION, serviceGetVersion);
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

extern "C" __declspec(dllexport) const PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}
