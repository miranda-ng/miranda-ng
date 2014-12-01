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

typedef DWORD ARGB;

void InitBitmapInfo(BITMAPINFO &bmi,  const SIZE &size)
{
	memset(&bmi, 0, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	bmi.bmiHeader.biWidth = size.cx;
	bmi.bmiHeader.biHeight = size.cy;
}

void ConvertToPARGB32(HDC hdc, ARGB *pargb, HBITMAP hbmp, SIZE& sizImage, int cxRow)
{
	BITMAPINFO bmi;
	InitBitmapInfo(bmi, sizImage);

	void *pvBits = malloc(sizImage.cx * 4 * sizImage.cy);
	if (GetDIBits(hdc, hbmp, 0, bmi.bmiHeader.biHeight, pvBits, &bmi, DIB_RGB_COLORS) == bmi.bmiHeader.biHeight) {
		ULONG cxDelta = cxRow - bmi.bmiHeader.biWidth;
		ARGB *pargbMask = (ARGB *)pvBits;

		for (ULONG y = bmi.bmiHeader.biHeight + 1; --y;) {
			for (ULONG x = bmi.bmiHeader.biWidth + 1; --x;) {
				if (*pargbMask++) {
					// transparent pixel
					*pargb++=0;
				}
				else {
					// opaque pixel
					*pargb++ |= 0xFF000000;
				}
			}

			pargb += cxDelta;
		}
	}
	free(pvBits);
}

bool HasAlpha(ARGB *pargb, SIZE& sizImage, int cxRow)
{
	ULONG cxDelta = cxRow - sizImage.cx;
	for (ULONG y = sizImage.cy; y--;) {
		for (ULONG x = sizImage.cx; x--;) {
			if (*pargb++ & 0xFF000000)
				return true;
		}
		pargb += cxDelta;
	}

	return false;
}

void ConvertBufferToPARGB32(HANDLE hPaintBuffer, HDC hdc, HICON hIcon, SIZE& sizIcon)
{
	RGBQUAD *prgbQuad;
	int cxRow;
	HRESULT hr = getBufferedPaintBits(hPaintBuffer, &prgbQuad, &cxRow);
	if (SUCCEEDED(hr)) {
		ARGB *pargb = (ARGB *)prgbQuad;
		if (!HasAlpha(pargb, sizIcon, cxRow)) {
			ICONINFO info;
			if (GetIconInfo(hIcon, &info)) {
				if (info.hbmMask)
					ConvertToPARGB32(hdc, pargb, info.hbmMask, sizIcon, cxRow);

				DeleteObject(info.hbmColor);
				DeleteObject(info.hbmMask);
			}
		}
	}
}

HBITMAP ConvertIconToBitmap(HICON hicon, HIMAGELIST hIml, int iconId)
{
	SIZE sizIcon;
	sizIcon.cx = GetSystemMetrics(SM_CXSMICON);
	sizIcon.cy = GetSystemMetrics(SM_CYSMICON);

	RECT rcIcon = { 0, 0, sizIcon.cx, sizIcon.cy };

	HDC hdc = CreateCompatibleDC(NULL);

	BITMAPINFO bmi;
	InitBitmapInfo(bmi, sizIcon);

	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);

	BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BP_PAINTPARAMS paintParams = {0};
	paintParams.cbSize = sizeof(paintParams);
	paintParams.dwFlags = BPPF_ERASE;
	paintParams.pBlendFunction = &bfAlpha;

	HDC hdcBuffer;
	HANDLE hPaintBuffer = beginBufferedPaint(hdc, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
	if (hPaintBuffer) {
		if (hIml)
			ImageList_Draw(hIml, iconId, hdc, 0, 0, ILD_TRANSPARENT);
		else
			DrawIconEx(hdcBuffer, 0, 0, hicon, sizIcon.cx, sizIcon.cy, 0, NULL, DI_NORMAL);

		// If icon did not have an alpha channel we need to convert buffer to PARGB
		ConvertBufferToPARGB32(hPaintBuffer, hdc, hicon, sizIcon);

		// This will write the buffer contents to the destination bitmap
		endBufferedPaint(hPaintBuffer, TRUE);
	}

	SelectObject(hdc, hbmpOld);
	DeleteDC(hdc);

	return hbmp;
}
