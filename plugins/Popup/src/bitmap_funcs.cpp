/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko
			© 2010 MPK
			© 2010 Merlin_de

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

#include "headers.h"
#include "bitmap_funcs.h"

#define PU_BMP_ACCURATE_ARITHMETICS

#ifdef PU_BMP_ACCURATE_ARITHMETICS
#define	PU_DIV255(x)	((x)/255)
#define	PU_DIV128(x)	((x)/128)
typedef float pu_koef;
#else
#define	PU_DIV255(x)	((x)>>8)
#define	PU_DIV128(x)	((x)>>7)
typedef long pu_koef;
#endif

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

MyBitmap::MyBitmap(const TCHAR *fn, const TCHAR *fnAlpha)
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
	freemem();
}

void MyBitmap::makeOpaque()
{
	if (!bits) return;

	GdiFlush();
	for (int i = 0; i < width*height; i++)
		bits[i] |= 0xff000000;
}

void MyBitmap::makeOpaqueRect(int x1, int y1, int x2, int y2)
{
	if (!bits) return;

	GdiFlush();
	for (int i = y1; i < y2; i++)
		for (int j = x1; j < x2; j++) {
		int idx = i * width + j;
		bits[idx] |= 0xff000000;
		}
}

void MyBitmap::saveAlpha(int x, int y, int w, int h)
{
	if (bitsSave)
		delete[] bitsSave;

	GdiFlush();

	if (!w) w = width;
	if (!h) h = height;
	if (!w || !h)
		return;
	if (x < 0 || y < 0)
		return;

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

	GdiFlush();

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

void MyBitmap::DrawBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h)
{
	if (!(bits && inbits)) return;

	GdiFlush();

	float kx = (float)inw / w;
	float ky = (float)inh / h;

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
			bits[(i + y)*width + (j + x)] = inbits[int(i*ky)*inw + int(j*kx)];
		}
	}
}

void MyBitmap::BlendBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h)
{
	if (!(bits && inbits)) return;

	GdiFlush();

	float kx = (float)inw / w;
	float ky = (float)inh / h;

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
			COLOR32 src = inbits[int(i*ky)*inw + int(j*kx)];
			COLOR32 dst = bits[(i + y)*width + (j + x)];
			long alpha = geta(src);
			bits[(i + y)*width + (j + x)] = rgba(
				getr(src) + PU_DIV255((255 - alpha)*getr(dst)),
				getg(src) + PU_DIV255((255 - alpha)*getg(dst)),
				getb(src) + PU_DIV255((255 - alpha)*getb(dst)),
				geta(src) + PU_DIV255((255 - alpha)*geta(dst))
				);
		}
	}
}

void MyBitmap::Blend(MyBitmap *bmp, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

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
				getr(src) + PU_DIV255((255 - alpha)*getr(dst)),
				getg(src) + PU_DIV255((255 - alpha)*getg(dst)),
				getb(src) + PU_DIV255((255 - alpha)*getb(dst)),
				geta(src) + PU_DIV255((255 - alpha)*geta(dst))
				);
		}
	}
}

void MyBitmap::Draw(MyBitmap *bmp, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

	if (!w) w = bmp->width;
	if (!h) h = bmp->height;

	if (!x && !y && (w == width) && (h == height) && (w == bmp->width) && (h == bmp->height)) {
		// fast bitmap copy is possible good for animated avatars
		CopyMemory(bits, bmp->bits, width*height*sizeof(COLOR32));
		return;
	}

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
			bits[(i + y)*width + (j + x)] = bmp->bits[int(i*ky)*bmp->width + int(j*kx)];
		}
	}
}

void MyBitmap::BlendColorized(MyBitmap *bmp, int x, int y, int w, int h, COLOR32 color)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

	if (!w) w = bmp->width;
	if (!h) h = bmp->height;
	float kx = (float)bmp->width / w;
	float ky = (float)bmp->height / h;

	// we should swap B and R channels when working with win32 COLORREF
	float koef1r = (255 - getb(color)) / 128.0;
	float koef1g = (255 - getg(color)) / 128.0;
	float koef1b = (255 - getr(color)) / 128.0;

	int br = -255 + 2 * getb(color);
	int bg = -255 + 2 * getg(color);
	int bb = -255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0;
	float koef2g = (getg(color)) / 128.0;
	float koef2b = (getr(color)) / 128.0;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;

			long alpha = geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]) * 255 / alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			COLOR32 src = (cl > 128) ?
				rgba(
				PU_DIV255((koef1r * cl + br)*alpha),
				PU_DIV255((koef1g * cl + bg)*alpha),
				PU_DIV255((koef1b * cl + bb)*alpha),
				alpha) :
				rgba(
				PU_DIV255(koef2r * cl * alpha),
				PU_DIV255(koef2g * cl * alpha),
				PU_DIV255(koef2b * cl * alpha),
				alpha);
#pragma warning(pop)
			COLOR32 dst = bits[(i + y)*width + (j + x)];
			bits[(i + y)*width + (j + x)] = rgba(
				getr(src) + PU_DIV255((255 - alpha)*getr(dst)),
				getg(src) + PU_DIV255((255 - alpha)*getg(dst)),
				getb(src) + PU_DIV255((255 - alpha)*getb(dst)),
				geta(src) + PU_DIV255((255 - alpha)*geta(dst)));
		}
	}
}

void MyBitmap::DrawColorized(MyBitmap *bmp, int x, int y, int w, int h, COLOR32 color)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

	if (!w) w = bmp->width;
	if (!h) h = bmp->height;
	float kx = (float)bmp->width / w;
	float ky = (float)bmp->height / h;

	// we should swap B and R channels when working with win32 COLORREF
	float koef1r = (255 - getb(color)) / 128.0;
	float koef1g = (255 - getg(color)) / 128.0;
	float koef1b = (255 - getr(color)) / 128.0;

	int br = -255 + 2 * getb(color);
	int bg = -255 + 2 * getg(color);
	int bb = -255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0;
	float koef2g = (getg(color)) / 128.0;
	float koef2b = (getr(color)) / 128.0;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;

			long alpha = geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]) * 255 / alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			bits[(i + y)*width + (j + x)] = (cl > 128) ?
				rgba(
				PU_DIV255((koef1r * cl + br)*alpha),
				PU_DIV255((koef1g * cl + bg)*alpha),
				PU_DIV255((koef1b * cl + bb)*alpha),
				alpha) :
				rgba(
				PU_DIV255(koef2r * cl * alpha),
				PU_DIV255(koef2g * cl * alpha),
				PU_DIV255(koef2b * cl * alpha),
				alpha);
#pragma warning(pop)
		}
	}
}

void MyBitmap::BlendPart(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;
	if (!win || !hin) return;

	GdiFlush();

	if (!w) w = win;
	if (!h) h = hin;
	float kx = (float)win / w;
	float ky = (float)hin / h;

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
			COLOR32 src = bmp->bits[int(yin + i*ky)*bmp->width + int(xin + j*kx)];
			COLOR32 dst = bits[(i + y)*width + (j + x)];
			long alpha = geta(src);
			bits[(i + y)*width + (j + x)] = rgba(
				getr(src) + PU_DIV255((255 - alpha)*getr(dst)),
				getg(src) + PU_DIV255((255 - alpha)*getg(dst)),
				getb(src) + PU_DIV255((255 - alpha)*getb(dst)),
				geta(src) + PU_DIV255((255 - alpha)*geta(dst))
				);
		}
	}
}

void MyBitmap::BlendPartColorized(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h, COLOR32 color)
{
	if (!(bits && bmp && bmp->bits)) return;
	if (!win || !hin) return;

	GdiFlush();

	if (!w) w = win;
	if (!h) h = hin;
	float kx = (float)win / w;
	float ky = (float)hin / h;

	if (x + w >= this->getWidth())
		w = this->getWidth() - x;
	if (y + h >= this->getHeight())
		h = this->getHeight() - y;

	// we should swap B and R channels when working with win32 COLORREF
	float koef1r = (255 - getb(color)) / 128.0;
	float koef1g = (255 - getg(color)) / 128.0;
	float koef1b = (255 - getr(color)) / 128.0;

	int br = -255 + 2 * getb(color);
	int bg = -255 + 2 * getg(color);
	int bb = -255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0;
	float koef2g = (getg(color)) / 128.0;
	float koef2b = (getr(color)) / 128.0;

	for (int i = 0; i < h; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		for (int j = 0; j < w; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;

			long alpha = geta(bmp->bits[int(yin + i*ky)*bmp->width + int(xin + j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(yin + i*ky)*bmp->width + int(xin + j*kx)]) * 255 / alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			COLOR32 src = (cl > 128) ?
				rgba(
				PU_DIV255((koef1r * cl + br)*alpha),
				PU_DIV255((koef1g * cl + bg)*alpha),
				PU_DIV255((koef1b * cl + bb)*alpha),
				alpha) :
				rgba(
				PU_DIV255(koef2r * cl * alpha),
				PU_DIV255(koef2g * cl * alpha),
				PU_DIV255(koef2b * cl * alpha),
				alpha);
#pragma warning(pop)
			COLOR32 dst = bits[(i + y)*width + (j + x)];
			bits[(i + y)*width + (j + x)] = rgba(
				getr(src) + PU_DIV255((255 - alpha)*getr(dst)),
				getg(src) + PU_DIV255((255 - alpha)*getg(dst)),
				getb(src) + PU_DIV255((255 - alpha)*getb(dst)),
				geta(src) + PU_DIV255((255 - alpha)*geta(dst)));
		}
	}
}

void MyBitmap::DrawPart(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;
	if (!win || !hin) return;

	GdiFlush();

	if (!w) w = win;
	if (!h) h = hin;
	float kx = (float)win / w;
	float ky = (float)hin / h;

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
			bits[(i + y)*width + (j + x)] = bmp->bits[int(yin + i*ky)*bmp->width + int(xin + j*kx)];
		}
	}
}

void MyBitmap::DrawNoAlpha(MyBitmap *bmp, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

	for (int i = 0; i < bmp->height; i++) {
		if (i + y < 0) continue;
		if (i + y >= height) break;
		for (int j = 0; j < bmp->width; j++) {
			if (j + x < 0) continue;
			if (j + x >= width) break;
			bits[(i + y)*width + (j + x)] = bmp->bits[i*bmp->width + j];
		}
	}
}

void MyBitmap::DrawIcon(HICON hic, int x, int y, int w, int h)
{
	GdiFlush();

	ICONINFO info;
	GetIconInfo(hic, &info);

	BITMAP bmpColor, bmpMask;
	GetObject(info.hbmMask, sizeof(bmpMask), &bmpMask);
	GetObject(info.hbmColor, sizeof(bmpColor), &bmpColor);

	if (!w) w = abs(bmpMask.bmWidth);
	if (!h) h = abs(bmpMask.bmHeight);

	if (bmpColor.bmBitsPixel == 32) {
		if ((w != abs(bmpMask.bmWidth)) || (h != abs(bmpMask.bmHeight))) {
			DeleteObject(info.hbmColor);
			DeleteObject(info.hbmMask);
			HICON hicTmp = (HICON)CopyImage(hic, IMAGE_ICON, w, h, LR_COPYFROMRESOURCE);
			GetIconInfo(hicTmp, &info);
			GetObject(info.hbmMask, sizeof(bmpMask), &bmpMask);
			GetObject(info.hbmColor, sizeof(bmpColor), &bmpColor);
			DestroyIcon(hicTmp);
		}

		BYTE *cbit = new BYTE[bmpColor.bmWidthBytes*bmpColor.bmHeight];
		BYTE *mbit = new BYTE[bmpMask.bmWidthBytes*bmpMask.bmHeight];
		GetBitmapBits(info.hbmColor, bmpColor.bmWidthBytes*bmpColor.bmHeight, cbit);
		GetBitmapBits(info.hbmMask, bmpMask.bmWidthBytes*bmpMask.bmHeight, mbit);

		for (int i = 0; i < bmpColor.bmHeight; i++) {
			for (int j = 0; j < bmpColor.bmWidth; j++) {
				BYTE *pixel = cbit + i*bmpColor.bmWidthBytes + j * 4;
				if (!pixel[3])
					pixel[3] = (*(mbit + i*bmpMask.bmWidthBytes + j*bmpMask.bmBitsPixel / 8) & (1 << (7 - j % 8))) ? 0 : 255;

				if (pixel[3] != 255) {
					pixel[0] = PU_DIV255(pixel[0] * pixel[3]);
					pixel[1] = PU_DIV255(pixel[1] * pixel[3]);
					pixel[2] = PU_DIV255(pixel[2] * pixel[3]);
				}
			}
		}

		this->BlendBits((COLOR32 *)cbit, bmpColor.bmWidth, bmpColor.bmHeight, x, y, w, h);

		delete[] mbit;
		delete[] cbit;
	}
	else {
		this->saveAlpha(x, y, w, h);
		DrawIconEx(this->getDC(), x, y, hic, w, h, 0, NULL, DI_NORMAL);
		this->restoreAlpha(x, y, w, h);
	}

	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);
}

void MyBitmap::Draw_Text(TCHAR *str, int x, int y)
{
	SIZE sz; GetTextExtentPoint32(this->getDC(), str, (int)mir_tstrlen(str), &sz);
	RECT rc; SetRect(&rc, x, y, x + 10000, y + 10000);
	this->saveAlpha(x, y, sz.cx, sz.cy);
	DrawText(this->getDC(), str, -1, &rc, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
	this->restoreAlpha(x, y, sz.cx, sz.cy);
}

// based on code by Yuriy Zaporozhets from:
// http://www.codeproject.com/gdi/coolrgn.asp?df=100&forumid=739&exp=0&select=6341
// slightly modified to integrate with MyBitmap class.
HRGN MyBitmap::buildOpaqueRgn(int level, bool opaque)
{
	GdiFlush();

	const int addRectsCount = 64;
	int rectsCount = addRectsCount;
	PRGNDATA pRgnData = (PRGNDATA)(new BYTE[sizeof(RGNDATAHEADER) + (rectsCount)*sizeof(RECT)]);
	LPRECT pRects = (LPRECT)(&pRgnData->Buffer);

	memset(pRgnData, 0, sizeof(RGNDATAHEADER) + (rectsCount)*sizeof(RECT));
	pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pRgnData->rdh.iType = RDH_RECTANGLES;

	int first = 0;
	bool wasfirst = false;
	bool ismask = false;
	for (int i = 0; i < height; i++) {
		int j; // we will need j after the loop!
		for (j = 0; j < width; j++) {
			ismask = opaque ? geta(this->getRow(i)[j]) > (DWORD)level : geta(this->getRow(i)[j]) < (DWORD)level;
			if (wasfirst) {
				if (!ismask) {
					SetRect(&pRects[pRgnData->rdh.nCount++], first, i, j, i + 1);
					if (pRgnData->rdh.nCount >= (DWORD)rectsCount) {
						rectsCount += addRectsCount;
						LPRGNDATA pRgnDataNew = (LPRGNDATA)(new BYTE[sizeof(RGNDATAHEADER) + (rectsCount)*sizeof(RECT)]);
						memcpy(pRgnDataNew, pRgnData, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount * sizeof(RECT));
						delete pRgnData;
						pRgnData = pRgnDataNew;
						pRects = (LPRECT)(&pRgnData->Buffer);
					}
					wasfirst = false;
				}
			}
			else if (ismask) { // set wasfirst when mask is found
				first = j;
				wasfirst = true;
			}
		}

		if (wasfirst && ismask) {
			SetRect(&pRects[pRgnData->rdh.nCount++], first, i, j, i + 1);
			if (pRgnData->rdh.nCount >= (DWORD)rectsCount) {
				rectsCount += addRectsCount;
				LPRGNDATA pRgnDataNew = (LPRGNDATA)(new BYTE[sizeof(RGNDATAHEADER) + (rectsCount)*sizeof(RECT)]);
				memcpy(pRgnDataNew, pRgnData, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount * sizeof(RECT));
				delete[] pRgnData;
				pRgnData = pRgnDataNew;
				pRects = (LPRECT)(&pRgnData->Buffer);
			}
			wasfirst = false;
		}
	}

	HRGN hRgn = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount*sizeof(RECT), (LPRGNDATA)pRgnData);
	delete[] pRgnData;
	return hRgn;
}

static int hex2dec(char hex)
{
	if ((hex >= '0') && (hex <= '9'))
		return hex - '0';
	if ((hex >= 'a') && (hex <= 'f'))
		return hex - 'a' + 0xa;
	if ((hex >= 'A') && (hex <= 'F'))
		return hex - 'A' + 0xa;
	return 0;
}

bool MyBitmap::loadFromFile_pixel(const TCHAR *fn, const TCHAR *fnAlpha)
{
	allocate(1, 1);
	int r, g, b, a = 255;
	const TCHAR *p = fn + mir_tstrlen(_T("pixel:"));
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	*bits = rgba(r, g, b, a);
	return true;
}

bool MyBitmap::loadFromFile_gradient(const TCHAR *fn, const TCHAR *fnAlpha)
{
	const TCHAR *p = fn + mir_tstrlen(_T("gradient:"));

	if (*p == 'h') allocate(256, 1);
	else allocate(1, 256);

	int r, g, b, a = 255;

	p += 2;
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	COLOR32 from = rgba(r, g, b, a);

	p += 7;
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	COLOR32 to = rgba(r, g, b, a);

	for (int i = 0; i < 256; ++i) {
		bits[i] = rgba(
			((255 - i) * getr(from) + i * getr(to)) / 255,
			((255 - i) * getg(from) + i * getg(to)) / 255,
			((255 - i) * getb(from) + i * getb(to)) / 255,
			255);
	}

	return true;
}

bool MyBitmap::loadFromFile_png(const TCHAR *fn, const TCHAR *fnAlpha)
{
	if (!ServiceExists(MS_PNG2DIB))
		return false;

	HANDLE hFile, hMap = 0;
	BYTE *ppMap = 0;
	long cbFileSize = 0;
	BITMAPINFOHEADER *pDib;
	BYTE *pDibBits;
	if ((hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
		if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
			if ((ppMap = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
				cbFileSize = GetFileSize(hFile, NULL);
	if (cbFileSize) {
		PNG2DIB param;
		param.pSource = ppMap;
		param.cbSourceSize = cbFileSize;
		param.pResult = &pDib;
		if (CallService(MS_PNG2DIB, 0, (LPARAM)&param))
			pDibBits = (BYTE*)(pDib + 1);
		else
			cbFileSize = 0;
	}

	if (ppMap) UnmapViewOfFile(ppMap);
	if (hMap) CloseHandle(hMap);
	if (hFile) CloseHandle(hFile);

	if (!cbFileSize) return false;

	BITMAPINFO *bi = (BITMAPINFO*)pDib;
	BYTE *pt = (BYTE*)bi;
	pt += bi->bmiHeader.biSize;

	if (bi->bmiHeader.biBitCount != 32) {
		allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
		HDC hdcTmp = CreateCompatibleDC(getDC());
		HBITMAP hBitmap = CreateDIBitmap(getDC(), pDib, CBM_INIT, pDibBits, bi, DIB_PAL_COLORS);
		HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcTmp, hBitmap);
		BitBlt(this->getDC(), 0, 0, abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight), hdcTmp, 0, 0, SRCCOPY);
		this->makeOpaque();
		SelectObject(hdcTmp, hOldBmp);
		DeleteDC(hdcTmp);
		DeleteObject(hBitmap);
	}
	else {
		allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
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
	return true;
}

bool MyBitmap::loadFromFile_default(const TCHAR *fn, const TCHAR *fnAlpha)
{
	SIZE sz;
	HBITMAP hBmpLoaded = (HBITMAP)LoadImage(NULL, fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	if (!hBmpLoaded)
		return false;

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
	else makeOpaque();

	return true;
}

bool MyBitmap::loadFromFile(const TCHAR *fn, const TCHAR *fnAlpha)
{
	if (bits) freemem();

	if (!_tcsncmp(fn, _T("pixel:"), mir_tstrlen(_T("pixel:"))))
		return loadFromFile_pixel(fn, fnAlpha);

	if (!_tcsncmp(fn, _T("gradient:"), mir_tstrlen(_T("gradient:"))))
		return loadFromFile_gradient(fn, fnAlpha);

	TCHAR ext[5];
	_tcsncpy_s(ext, fn + (_tcslen(fn) - 4), _TRUNCATE);
	if (!mir_tstrcmpi(ext, _T(".png")))
		return loadFromFile_png(fn, fnAlpha);

	return loadFromFile_default(fn, fnAlpha);
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

	GdiFlush();
}

void MyBitmap::freemem()
{
	GdiFlush();

	DeleteObject(SelectObject(dcBmp, hBmpSave));
	DeleteDC(dcBmp);

	dcBmp = 0;
	hBmp = 0;
	bits = 0;
	width = height = 0;
}

void MyBitmap::premultipleChannels()
{
	GdiFlush();

	for (int i = 0; i < width*height; i++)
		bits[i] = rgba(getr(bits[i])*geta(bits[i]) / 255, getg(bits[i])*geta(bits[i]) / 255, getb(bits[i])*geta(bits[i]) / 255, geta(bits[i]));
}
