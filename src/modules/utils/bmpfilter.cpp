/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "..\..\core\commonheaders.h"
#include <olectl.h>

#include "m_png.h"
#include "m_imgsrvc.h"

static INT_PTR sttBitmapLoader(const TCHAR* ptszFileName)
{
	IPicture *pic;
	HBITMAP hBmp, hBmpCopy;
	HBITMAP hOldBitmap, hOldBitmap2;
	BITMAP bmpInfo;
	HDC hdc, hdcMem1, hdcMem2;
	short picType;

	TCHAR szFilename[MAX_PATH];
	if (!PathToAbsoluteT(ptszFileName, szFilename))
		_tcsncpy_s(szFilename, ptszFileName, _TRUNCATE);

	size_t filenameLen = mir_tstrlen(szFilename);
	if (filenameLen > 4) {
		TCHAR* pszExt = szFilename + filenameLen - 4;

		if (ServiceExists(MS_IMG_LOAD))
			return CallService(MS_IMG_LOAD, (WPARAM)szFilename, IMGL_TCHAR);

		if (!mir_tstrcmpi(pszExt, _T(".bmp")) || !mir_tstrcmpi(pszExt, _T(".rle"))) {
			//LoadImage can do this much faster
			return (INT_PTR)LoadImage(hInst, szFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		}

		if (!mir_tstrcmpi(pszExt, _T(".png"))) {
			HANDLE hFile, hMap = NULL;
			BYTE* ppMap = NULL;
			INT_PTR  cbFileSize = 0;
			BITMAPINFOHEADER* pDib;
			BYTE* pDibBits;

			if (!ServiceExists(MS_PNG2DIB)) {
				MessageBox(NULL, TranslateT("You need an image services plugin to process PNG images."), TranslateT("Error"), MB_OK);
				return 0;
			}

			if ((hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
				if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
					if ((ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
						cbFileSize = GetFileSize(hFile, NULL);

			if (cbFileSize != 0) {
				PNG2DIB param;
				param.pSource = ppMap;
				param.cbSourceSize = cbFileSize;
				param.pResult = &pDib;
				if (CallService(MS_PNG2DIB, 0, (LPARAM)&param)) {
					pDibBits = (BYTE*)(pDib+1);
					HDC sDC = GetDC(NULL);
					HBITMAP hBitmap = CreateDIBitmap(sDC, pDib, CBM_INIT, pDibBits, (BITMAPINFO*)pDib, DIB_PAL_COLORS);
					SelectObject(sDC, hBitmap);
					ReleaseDC(NULL, sDC);
					GlobalFree(pDib);
					cbFileSize = (INT_PTR)hBitmap;
				}
				else cbFileSize = 0;
			}

			if (ppMap != NULL)	UnmapViewOfFile(ppMap);
			if (hMap  != NULL)	CloseHandle(hMap);
			if (hFile != NULL) CloseHandle(hFile);

			return (INT_PTR)cbFileSize;
	}	}

	if (S_OK != OleLoadPicturePath(LPOLESTR(szFilename), NULL, 0, 0, IID_IPicture, (PVOID*)&pic))
		return 0;

	pic->get_Type(&picType);
	if (picType != PICTYPE_BITMAP) {
		pic->Release();
		return 0;
	}
	OLE_HANDLE hOleBmp;
	pic->get_Handle(&hOleBmp);
	hBmp = (HBITMAP)hOleBmp;
	GetObject(hBmp, sizeof(bmpInfo), &bmpInfo);

	//need to copy bitmap so we can free the IPicture
	hdc = GetDC(NULL);
	hdcMem1 = CreateCompatibleDC(hdc);
	hdcMem2 = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hdcMem1, hBmp);
	hBmpCopy = CreateCompatibleBitmap(hdcMem1, bmpInfo.bmWidth, bmpInfo.bmHeight);
	hOldBitmap2 = (HBITMAP)SelectObject(hdcMem2, hBmpCopy);
	BitBlt(hdcMem2, 0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, hdcMem1, 0, 0, SRCCOPY);
	SelectObject(hdcMem1, hOldBitmap);
	SelectObject(hdcMem2, hOldBitmap2);
	DeleteDC(hdcMem2);
	DeleteDC(hdcMem1);
	ReleaseDC(NULL, hdc);

	DeleteObject(hBmp);
	pic->Release();
	return (INT_PTR)hBmpCopy;
}

static INT_PTR BmpFilterLoadBitmap(WPARAM, LPARAM lParam)
{
	return sttBitmapLoader(_A2T((const char*)lParam));
}

static INT_PTR BmpFilterLoadBitmapW(WPARAM, LPARAM lParam)
{
	return sttBitmapLoader((const wchar_t*)lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

int InitBitmapFilter(void)
{
	CreateServiceFunction(MS_UTILS_LOADBITMAP, BmpFilterLoadBitmap);
	CreateServiceFunction(MS_UTILS_LOADBITMAPW, BmpFilterLoadBitmapW);
	return 0;
}
