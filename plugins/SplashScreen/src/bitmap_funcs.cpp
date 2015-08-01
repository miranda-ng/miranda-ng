/*
Splash Screen Plugin for Miranda NG (www.miranda-ng.org)
(c) 2004-2007 nullbie, (c) 2005-2007 Thief

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

MyBitmap::MyBitmap()
{
	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
	bitsSave = 0;
}

MyBitmap::MyBitmap(int w, int h)
{
	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
	bitsSave = 0;
	allocate(w, h);
}

MyBitmap::MyBitmap(TCHAR *fn)
{
	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
	bitsSave = 0;
	loadFromFile(fn);
}

MyBitmap::~MyBitmap()
{
	if (bitsSave)
		delete[] bitsSave;
	free();
}

void MyBitmap::makeOpaque()
{
	if (!bits) return;

	for (int i = 0; i < width*height; i++)
		bits[i] |= 0xff000000;
}

void MyBitmap::saveAlpha(int x, int y, int w, int h)
{
	if (bitsSave)
		delete[] bitsSave;

	if (!w) w = width;
	if (!h) h = height;

	bitsSave = new COLOR32[w*h];
	COLOR32 *p1 = bitsSave;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		COLOR32 *p2 = bits + (y + i)*width + x;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;
			*p1++ = *p2++;
		}
	}
}

void MyBitmap::restoreAlpha(int x, int y, int w, int h)
{
	if (!bitsSave)
		return;

	if (!w) w = width;
	if (!h) h = height;

	COLOR32 *p1 = bitsSave;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		COLOR32 *p2 = bits + (y + i)*width + x;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;
			if ((*p1 & 0x00ffffff) != (*p2 & 0x00ffffff)) {
				*p2 |= 0xff000000;
			}
			else {
				*p2 = (*p2 & 0x00ffffff) | (*p1 & 0xff000000);
			}
			++p1;
			++p2;
		}
	}

	delete[] bitsSave;
	bitsSave = 0;
}

void MyBitmap::DrawText(TCHAR *str, int x, int y)
{
	SIZE sz; GetTextExtentPoint32(this->getDC(), str, (int)mir_tstrlen(str), &sz);
	RECT rc; SetRect(&rc, x, y, x + 10000, y + 10000);
	this->saveAlpha(x - 2, y - 2, sz.cx + 2, sz.cy + 2);
	::DrawText(this->getDC(), str, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
	this->restoreAlpha(x - 2, y - 2, sz.cx + 2, sz.cy + 2);
}

bool MyBitmap::loadFromFile(TCHAR *fn)
{
	if (bits) free();

	HBITMAP hBmpLoaded = (HBITMAP)CallService(MS_IMG_LOAD, (WPARAM)fn, IMGL_TCHAR);
	if (!hBmpLoaded) {
#ifdef _DEBUG
		logMessage(_T("MyBitmap::loadFromFile"), _T("Bitmap load failed"));
#endif
		return false;
	}

	BITMAP bm; GetObject(hBmpLoaded, sizeof(bm), &bm);
	SetBitmapDimensionEx(hBmpLoaded, bm.bmWidth, bm.bmHeight, NULL);

	SIZE sz;
	HDC dcTmp = CreateCompatibleDC(0);
	GetBitmapDimensionEx(hBmpLoaded, &sz);
	HBITMAP hBmpDcSave = (HBITMAP)SelectObject(dcTmp, hBmpLoaded);

	allocate(sz.cx, sz.cy);
	BitBlt(dcBmp, 0, 0, width, height, dcTmp, 0, 0, SRCCOPY);

	DeleteObject(SelectObject(dcTmp, hBmpDcSave));
	DeleteDC(dcTmp);

	if (bm.bmBitsPixel == 32)
		premultipleChannels();
	else
		makeOpaque();

	return true;
}

void MyBitmap::allocate(int w, int h)
{
	width = w;
	height = h;

	BITMAPINFO bi;

	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = -h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	if (dcBmp) {
		DeleteObject(SelectObject(dcBmp, hBmpSave));
		DeleteDC(dcBmp);
	}

	hBmp = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, (void**)&bits, 0, 0);
	dcBmp = CreateCompatibleDC(0);
	hBmpSave = (HBITMAP)SelectObject(dcBmp, hBmp);
}

void MyBitmap::free()
{
	DeleteObject(SelectObject(dcBmp, hBmpSave));
	DeleteDC(dcBmp);

	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
}

void MyBitmap::premultipleChannels()
{
	for (int i = 0; i < width*height; i++)
		bits[i] = rgba(getr(bits[i])*geta(bits[i]) / 255, getg(bits[i])*geta(bits[i]) / 255, getb(bits[i])*geta(bits[i]) / 255, geta(bits[i]));
}
