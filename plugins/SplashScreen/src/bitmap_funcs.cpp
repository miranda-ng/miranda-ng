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

#include "headers.h"

BOOL (WINAPI *_mempng2dib) (BYTE*, DWORD, BITMAPINFOHEADER**);

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

MyBitmap::MyBitmap(TCHAR *fn, TCHAR *fnAlpha)
{
	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
	bitsSave = 0;
	loadFromFile(fn, fnAlpha);
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

void MyBitmap::Blend(MyBitmap *bmp, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;

	if (!w) w = bmp->width;
	if (!h) h = bmp->height;
	float kx = (float)bmp->width / w;
	float ky = (float)bmp->height / h;

	if (x + w >= this->getWidth())
		w = this->getWidth() - x;
	if (y + h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;
			COLOR32 src = bmp->bits[int(i*ky)*bmp->width + int(j*kx)];
			COLOR32 dst = bits[(i + y)*width + (j + x)];
			long alpha = geta(src);
			bits[(i + y)*width + (j + x)] = rgba(
				getr(src) + (255 - alpha)*getr(dst) / 255,
				getg(src) + (255 - alpha)*getg(dst) / 255,
				getb(src) + (255 - alpha)*getb(dst) / 255,
				geta(src) + (255 - alpha)*geta(dst) / 255
				);
		}
	}
}

void MyBitmap::DrawText(TCHAR *str, int x, int y)
{
	SIZE sz; GetTextExtentPoint32(this->getDC(), str, (int)mir_tstrlen(str), &sz);
	RECT rc; SetRect(&rc, x, y, x + 10000, y + 10000);
	this->saveAlpha(x - 2, y - 2, sz.cx + 2, sz.cy + 2);
	::DrawText(this->getDC(), str, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
	this->restoreAlpha(x - 2, y - 2, sz.cx + 2, sz.cy + 2);
	//(x,y,sz.cx,sz.cy);
}

HRGN MyBitmap::buildOpaqueRgn()
{
	return CreateRectRgn(0, 0, width, height);

	//
	int i, rectCount = 0;
	for (i = 0; i < width*height; i++)
		if (((bits[i] >> 24) & 0xff) >= 128)
			rectCount++;

	RGNDATA *rgnData = (RGNDATA *)malloc(sizeof(RGNDATAHEADER) + rectCount * sizeof(RECT));
	rgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
	rgnData->rdh.iType = RDH_RECTANGLES;
	rgnData->rdh.nCount = rectCount;
	rgnData->rdh.nRgnSize = rectCount * sizeof(RECT);
	SetRect(&(rgnData->rdh.rcBound), 0, 0, width, height);

	char *p = (char *)&(rgnData->Buffer);
	for (i = 0; i < width*height; i++)
		if (((bits[i] >> 24) & 0xff) >= 128) {
			SetRect((LPRECT)p, i%width, i / width, i%width + 1, i / width + 1);
			p += sizeof(RECT);
		}

	HRGN rgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + rectCount * sizeof(RECT), rgnData);
	::free(rgnData);

	return rgn;
}

bool MyBitmap::loadFromFile(TCHAR *fn, TCHAR *fnAlpha)
{
	if (bits) free();

	SIZE sz;

	TCHAR *ext;
	ext = &fn[mir_tstrlen(fn) - 4];

	if (!mir_tstrcmpi(ext, _T(".png"))) {
		HANDLE hFile, hMap = NULL;
		BYTE* ppMap = NULL;
		long  cbFileSize = 0;
		BITMAPINFOHEADER* pDib;
		BYTE* pDibBits = 0;

		if (!png2dibConvertor) {
			return false;
		}

		if ((hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
			if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
				if ((ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
					cbFileSize = GetFileSize(hFile, NULL);
		if (cbFileSize != 0) {
			PNG2DIB param;
			param.pSource = ppMap;
			param.cbSourceSize = cbFileSize;
			param.pResult = &pDib;

			if (png2dibConvertor((char*)param.pSource, param.cbSourceSize, param.pResult))
				pDibBits = (BYTE*)(pDib + 1);
			else
				cbFileSize = 0;
#ifdef _DEBUG
			logMessage(_T("Loading splash file"), _T("done"));
#endif
		}

		if (ppMap) UnmapViewOfFile(ppMap);
		if (hMap) CloseHandle(hMap);
		if (hFile) CloseHandle(hFile);

		if (!cbFileSize) return false;

		BITMAPINFO *bi = (BITMAPINFO*)pDib;
		BYTE *pt = (BYTE*)bi;
		pt += bi->bmiHeader.biSize;
		HBITMAP hBitmap = NULL;

		if (bi->bmiHeader.biBitCount != 32) {
			allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
			HDC hdcTmp = CreateCompatibleDC(getDC());
			HBITMAP hBitmap = CreateDIBitmap(getDC(), pDib, CBM_INIT, pDibBits, bi, DIB_PAL_COLORS);
			SelectObject(hdcTmp, hBitmap);
			BitBlt(this->getDC(), 0, 0, abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight), hdcTmp, 0, 0, SRCCOPY);
			this->makeOpaque();
			DeleteDC(hdcTmp);
			DeleteObject(hBitmap);

		}
		else {
			BYTE *ptPixels = pt;
			hBitmap = CreateDIBSection(NULL, bi, DIB_RGB_COLORS, (void **)&ptPixels, NULL, 0);
			memcpy(ptPixels, pt, bi->bmiHeader.biSizeImage);

			allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
			//memcpy(bits, pt, bi->bmiHeader.biSizeImage);

			BYTE *p2 = (BYTE *)pt;
			for (int y = 0; y < bi->bmiHeader.biHeight; ++y) {
				BYTE *p1 = (BYTE *)bits + (bi->bmiHeader.biHeight - y - 1)*bi->bmiHeader.biWidth * 4;
				for (int x = 0; x < bi->bmiHeader.biWidth; ++x) {
					p1[0] = p2[0];
					p1[1] = p2[1];
					p1[2] = p2[2];
					p1[3] = p2[3];
					p1 += 4;
					p2 += 4;
				}
			}

			premultipleChannels();
		}

		GlobalFree(pDib);
		DeleteObject(hBitmap);
		return true;
	}
	else {
		HBITMAP hBmpLoaded = (HBITMAP)LoadImage(NULL, fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (!hBmpLoaded) {
#ifdef _DEBUG
			logMessage(_T("MyBitmap::loadFromFile"), _T("Bitmap load failed"));
#endif
			return false;
		}

		BITMAP bm; GetObject(hBmpLoaded, sizeof(bm), &bm);
		SetBitmapDimensionEx(hBmpLoaded, bm.bmWidth, bm.bmHeight, NULL);

		HDC dcTmp = CreateCompatibleDC(0);
		GetBitmapDimensionEx(hBmpLoaded, &sz);
		HBITMAP hBmpDcSave = (HBITMAP)SelectObject(dcTmp, hBmpLoaded);

		allocate(sz.cx, sz.cy);
		BitBlt(dcBmp, 0, 0, width, height, dcTmp, 0, 0, SRCCOPY);

		DeleteObject(SelectObject(dcTmp, hBmpDcSave));
		DeleteDC(dcTmp);

		MyBitmap alpha;
		if (fnAlpha && alpha.loadFromFile(fnAlpha) &&
			(alpha.getWidth() == width) &&
			(alpha.getHeight() == height)) {
			for (int i = 0; i < width*height; i++)
				bits[i] = (bits[i] & 0x00ffffff) | ((alpha.bits[i] & 0x000000ff) << 24);
			premultipleChannels();
		}
		else {
			makeOpaque();
		}
		return true;
	}
	// unreachable place
	return false;
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

	hBmp = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, (void **)&bits, 0, 0);
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