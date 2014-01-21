/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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
		mir_sntprintf(szFilename, SIZEOF(szFilename), _T("%s"), ptszFileName);

	int filenameLen = lstrlen(szFilename);
	if (filenameLen > 4) {
		TCHAR* pszExt = szFilename + filenameLen - 4;

		if (ServiceExists(MS_IMG_LOAD))
			return CallService(MS_IMG_LOAD, (WPARAM)szFilename, IMGL_TCHAR);

		if (!lstrcmpi(pszExt, _T(".bmp")) || !lstrcmpi(pszExt, _T(".rle"))) {
			//LoadImage can do this much faster
			return (INT_PTR)LoadImage(hInst, szFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		}

		if (!lstrcmpi(pszExt, _T(".png"))) {
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

	if (S_OK != OleLoadPicturePath(LPOLESTR((const wchar_t*)StrConvU(szFilename)), NULL, 0, 0, IID_IPicture, (PVOID*)&pic))
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
	return sttBitmapLoader(StrConvT((const char*)lParam));
}

static INT_PTR BmpFilterLoadBitmapW(WPARAM, LPARAM lParam)
{
	return sttBitmapLoader((const wchar_t*)lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR BmpFilterGetStrings(WPARAM wParam, LPARAM lParam)
{
	int bytesLeft = wParam;
	char *filter = (char*)lParam, *pfilter;

	lstrcpynA(filter, Translate("All bitmaps"), bytesLeft); bytesLeft-=lstrlenA(filter);
	strncat(filter, " (*.bmp;*.jpg;*.gif;*.png)", bytesLeft);
	pfilter = filter+lstrlenA(filter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpynA(pfilter, Translate("Windows bitmaps"), bytesLeft); bytesLeft-=lstrlenA(pfilter);
	strncat(pfilter, " (*.bmp;*.rle)", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*.BMP;*.RLE", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpynA(pfilter, Translate("JPEG bitmaps"), bytesLeft); bytesLeft-=lstrlenA(pfilter);
	strncat(pfilter, " (*.jpg;*.jpeg)", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*.JPG;*.JPEG", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpynA(pfilter, Translate("GIF bitmaps"), bytesLeft); bytesLeft-=lstrlenA(pfilter);
	strncat(pfilter, " (*.gif)", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*.GIF", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpynA(pfilter, Translate("PNG bitmaps"), bytesLeft); bytesLeft-=lstrlenA(pfilter);
	strncat(pfilter, " (*.png)", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*.PNG", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpynA(pfilter, Translate("All files"), bytesLeft); bytesLeft-=lstrlenA(pfilter);
	strncat(pfilter, " (*)", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpynA(pfilter, "*", bytesLeft);
	pfilter+=lstrlenA(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	if (bytesLeft) *pfilter = '\0';
	return 0;
}

static INT_PTR BmpFilterGetStringsW(WPARAM wParam, LPARAM lParam)
{
	int bytesLeft = wParam;
	TCHAR *filter = (TCHAR*)lParam, *pfilter;

	lstrcpyn(filter, TranslateT("All bitmaps"), bytesLeft); bytesLeft-=lstrlen(filter);
	_tcsncat(filter, _T(" (*.bmp;*.jpg;*.gif;*.png)"), bytesLeft);
	pfilter = filter+lstrlen(filter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("Windows bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.bmp;*.rle)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.BMP;*.RLE"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("JPEG bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.jpg;*.jpeg)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.JPG;*.JPEG"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("GIF bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.gif)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.GIF"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("PNG bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.png)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.PNG"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("All files"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft = wParam-(pfilter-filter);

	if (bytesLeft) *pfilter = '\0';
	return 0;
}

int InitBitmapFilter(void)
{
	CreateServiceFunction(MS_UTILS_LOADBITMAP, BmpFilterLoadBitmap);
	CreateServiceFunction(MS_UTILS_GETBITMAPFILTERSTRINGS, BmpFilterGetStrings);
	CreateServiceFunction(MS_UTILS_GETBITMAPFILTERSTRINGSW, BmpFilterGetStringsW);
	CreateServiceFunction(MS_UTILS_LOADBITMAPW, BmpFilterLoadBitmapW);
	return 0;
}
