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

#include "stdafx.h"

typedef DWORD ARGB;

HBITMAP ConvertIconToBitmap(HIMAGELIST hIml, int iconId)
{
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biWidth = g_iIconSX;
	bmi.bmiHeader.biHeight = g_iIconSY;

	HDC hdc = CreateCompatibleDC(NULL);
	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);

	BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BP_PAINTPARAMS paintParams = {0};
	paintParams.cbSize = sizeof(paintParams);
	paintParams.dwFlags = BPPF_ERASE;
	paintParams.pBlendFunction = &bfAlpha;

	HDC hdcBuffer;
	RECT rcIcon = { 0, 0, g_iIconSX, g_iIconSY };
	HANDLE hPaintBuffer = beginBufferedPaint(hdc, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
	if (hPaintBuffer) {
		ImageList_Draw(hIml, iconId, hdc, 0, 0, ILD_TRANSPARENT);
		endBufferedPaint(hPaintBuffer, TRUE);
	}

	SelectObject(hdc, hbmpOld);
	DeleteDC(hdc);

	return hbmp;
}
