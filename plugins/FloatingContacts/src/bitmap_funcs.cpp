/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
			© 2004-2007 Victor Pavlychko

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

#include "stdhdr.h"

#define PU_FONT_THRESHOLD				96
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
	allocate(w,h);
}

MyBitmap::MyBitmap(const char *fn, const char *fnAlpha)
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
	delete [] bitsSave;
	free();
}

void MyBitmap::setAlpha(BYTE level)
{
	if (!bits) return;

	GdiFlush();
	for (int i = 0; i < width*height; i++)
	{
		if (bits[i] & 0xff000000)
		{
			bits[i] = rgba(getr(bits[i])*level/255, getg(bits[i])*level/255, getb(bits[i])*level/255, geta(bits[i])*level/255);
		} else
		{
			bits[i] = rgba(getr(bits[i])*level/255, getg(bits[i])*level/255, getb(bits[i])*level/255, level);
		}
	}
}

void MyBitmap::setAlphaRect(int x1, int y1, int x2, int y2, BYTE level)
{
	if (!bits) return;

	GdiFlush();
	for (int i = y1; i < y2; i++)
		for (int j = x1; j < x2; j++) {
			int idx = i * width + j;
			if (bits[idx] & 0xff000000)
				bits[idx] = rgba(getr(bits[idx])*level/255, getg(bits[idx])*level/255, getb(bits[idx])*level/255, geta(bits[idx])*level/255);
			else
				bits[idx] = rgba(getr(bits[idx])*level/255, getg(bits[idx])*level/255, getb(bits[idx])*level/255, level);
		}
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
	delete [] bitsSave;

	GdiFlush();

	if (!w) w = width;
	if (!h) h = height;

	bitsSave = new COLOR32[w*h];
	COLOR32 *p1 = bitsSave;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		COLOR32 *p2 = bits + (y+i)*width + x;
		p1 = bitsSave + i*w;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			*p1++=*p2++;
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

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		COLOR32 *p2 = bits + (y+i)*width + x;
		p1 = bitsSave + i*w;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			if ((*p1&0x00ffffff) != (*p2&0x00ffffff))
			{
				*p2 |= 0xff000000;
			} else
			{
				*p2 = (*p2&0x00ffffff) | (*p1&0xff000000);
			}
			++p1;
			++p2;
		}
	}

	delete [] bitsSave;
	bitsSave = 0;
}

void MyBitmap::DrawBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h)
{
	if (!(bits && inbits)) return;

	GdiFlush();

	float kx = (float)inw / w;
	float ky = (float)inh / h;

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			bits[(i+y)*width + (j+x)] = inbits[int(i*ky)*inw + int(j*kx)];
		}
	}
}

void MyBitmap::BlendBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h)
{
	if (!(bits && inbits)) return;

	GdiFlush();

	float kx = (float)inw / w;
	float ky = (float)inh / h;

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			COLOR32 src = inbits[int(i*ky)*inw + int(j*kx)];
			COLOR32 dst = bits[(i+y)*width + (j+x)];
			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+PU_DIV255((255-alpha)*getr(dst)),
					getg(src)+PU_DIV255((255-alpha)*getg(dst)),
					getb(src)+PU_DIV255((255-alpha)*getb(dst)),
					geta(src)+PU_DIV255((255-alpha)*geta(dst))
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

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			COLOR32 src = bmp->bits[int(i*ky)*bmp->width + int(j*kx)];
			COLOR32 dst = bits[(i+y)*width + (j+x)];
			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+PU_DIV255((255-alpha)*getr(dst)),
					getg(src)+PU_DIV255((255-alpha)*getg(dst)),
					getb(src)+PU_DIV255((255-alpha)*getb(dst)),
					geta(src)+PU_DIV255((255-alpha)*geta(dst))
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

	if (!x && !y && (w == width) && (h == height) && (w == bmp->width) && (h == bmp->height))
	{
		// fast bitmap copy is possible good for animated avatars
		CopyMemory(bits, bmp->bits, width*height*sizeof(COLOR32));
		return;
	}

	float kx = (float)bmp->width / w;
	float ky = (float)bmp->height / h;

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			bits[(i+y)*width + (j+x)] = bmp->bits[int(i*ky)*bmp->width + int(j*kx)];
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
	float koef1r = (255 - getb(color)) / 128.0f;
	float koef1g = (255 - getg(color)) / 128.0f;
	float koef1b = (255 - getr(color)) / 128.0f;

	int br = - 255 + 2 * getb(color);
	int bg = - 255 + 2 * getg(color);
	int bb = - 255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0f;
	float koef2g = (getg(color)) / 128.0f;
	float koef2b = (getr(color)) / 128.0f;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;

//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
//			bits[(i+y)*width + (j+x)] = (cl > 128) ?
//				rgba(koef1r * cl + br, koef1g * cl + bg, koef1b * cl + bb, geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)])):
//				rgba(koef2r * cl, koef2g * cl, koef2b * cl, geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]));

			long alpha = geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)])*255/alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			COLOR32 src = (cl > 128) ?
				rgba(
					PU_DIV255((koef1r * cl + br)*alpha),
					PU_DIV255((koef1g * cl + bg)*alpha),
					PU_DIV255((koef1b * cl + bb)*alpha),
					alpha):
				rgba(
					PU_DIV255(koef2r * cl * alpha),
					PU_DIV255(koef2g * cl * alpha),
					PU_DIV255(koef2b * cl * alpha),
					alpha);
#pragma warning(pop)
//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
//			COLOR32 src = (cl > 128) ?
//				rgba(koef1r * cl + br, koef1g * cl + bg, koef1b * cl + bb, alpha):
//				rgba(koef2r * cl, koef2g * cl, koef2b * cl, alpha);
			COLOR32 dst = bits[(i+y)*width + (j+x)];
//			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+PU_DIV255((255-alpha)*getr(dst)),
					getg(src)+PU_DIV255((255-alpha)*getg(dst)),
					getb(src)+PU_DIV255((255-alpha)*getb(dst)),
					geta(src)+PU_DIV255((255-alpha)*geta(dst))
				);

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
	float koef1r = (255 - getb(color)) / 128.0f;
	float koef1g = (255 - getg(color)) / 128.0f;
	float koef1b = (255 - getr(color)) / 128.0f;

	int br = - 255 + 2 * getb(color);
	int bg = - 255 + 2 * getg(color);
	int bb = - 255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0f;
	float koef2g = (getg(color)) / 128.0f;
	float koef2b = (getr(color)) / 128.0f;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;

			long alpha = geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)])*255/alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			bits[(i+y)*width + (j+x)] = (cl > 128) ?
				rgba(
					PU_DIV255((koef1r * cl + br)*alpha),
					PU_DIV255((koef1g * cl + bg)*alpha),
					PU_DIV255((koef1b * cl + bb)*alpha),
					alpha):
				rgba(
					PU_DIV255(koef2r * cl * alpha),
					PU_DIV255(koef2g * cl * alpha),
					PU_DIV255(koef2b * cl * alpha),
					alpha);
#pragma warning(pop)
//			bits[(i+y)*width + (j+x)] = (cl > 128) ?
//				rgba(koef1r * cl + br, koef1g * cl + bg, koef1b * cl + bb, geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)])):
//				rgba(koef2r * cl, koef2g * cl, koef2b * cl, geta(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]));
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

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			COLOR32 src = bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)];
			COLOR32 dst = bits[(i+y)*width + (j+x)];
			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+PU_DIV255((255-alpha)*getr(dst)),
					getg(src)+PU_DIV255((255-alpha)*getg(dst)),
					getb(src)+PU_DIV255((255-alpha)*getb(dst)),
					geta(src)+PU_DIV255((255-alpha)*geta(dst))
				);
//			bits[(i+y)*width + (j+x)] = bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)];
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

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	// we should swap B and R channels when working with win32 COLORREF
	float koef1r = (255 - getb(color)) / 128.0f;
	float koef1g = (255 - getg(color)) / 128.0f;
	float koef1b = (255 - getr(color)) / 128.0f;

	int br = - 255 + 2 * getb(color);
	int bg = - 255 + 2 * getg(color);
	int bb = - 255 + 2 * getr(color);

	float koef2r = (getb(color)) / 128.0f;
	float koef2g = (getg(color)) / 128.0f;
	float koef2b = (getr(color)) / 128.0f;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;

			long alpha = geta(bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)]);
//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
			COLOR32 cl = alpha ? getr(bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)])*255/alpha : 0;
#pragma warning(push)
#pragma warning(disable: 4244)
			COLOR32 src = (cl > 128) ?
				rgba(
					PU_DIV255((koef1r * cl + br)*alpha),
					PU_DIV255((koef1g * cl + bg)*alpha),
					PU_DIV255((koef1b * cl + bb)*alpha),
					alpha):
				rgba(
					PU_DIV255(koef2r * cl * alpha),
					PU_DIV255(koef2g * cl * alpha),
					PU_DIV255(koef2b * cl * alpha),
					alpha);
#pragma warning(pop)
//			COLOR32 cl = getr(bmp->bits[int(i*ky)*bmp->width + int(j*kx)]);
//			COLOR32 src = (cl > 128) ?
//				rgba(koef1r * cl + br, koef1g * cl + bg, koef1b * cl + bb, alpha):
//				rgba(koef2r * cl, koef2g * cl, koef2b * cl, alpha);
			COLOR32 dst = bits[(i+y)*width + (j+x)];
//			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+PU_DIV255((255-alpha)*getr(dst)),
					getg(src)+PU_DIV255((255-alpha)*getg(dst)),
					getb(src)+PU_DIV255((255-alpha)*getb(dst)),
					geta(src)+PU_DIV255((255-alpha)*geta(dst))
				);

/*			COLOR32 src = bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)];
			COLOR32 dst = bits[(i+y)*width + (j+x)];
			long alpha = geta(src);
			bits[(i+y)*width + (j+x)] = rgba(
					getr(src)+(255-alpha)*getr(dst)/255,
					getg(src)+(255-alpha)*getg(dst)/255,
					getb(src)+(255-alpha)*getb(dst)/255,
					geta(src)+(255-alpha)*geta(dst)/255
				);*/
//			bits[(i+y)*width + (j+x)] = bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)];
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

	if (x+w >= this->getWidth())
		w = this->getWidth() - x;
	if (y+h >= this->getHeight())
		h = this->getHeight() - y;

	for (int i = 0; i < h; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < w; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			bits[(i+y)*width + (j+x)] = bmp->bits[int(yin+i*ky)*bmp->width + int(xin+j*kx)];
		}
	}
}

void MyBitmap::DrawNoAlpha(MyBitmap *bmp, int x, int y, int w, int h)
{
	if (!(bits && bmp && bmp->bits)) return;

	GdiFlush();

	for (int i = 0; i < bmp->height; i++)
	{
		if (i+y < 0) continue;
		if (i+y >= height) break;
		for (int j = 0; j < bmp->width; j++)
		{
			if (j+x < 0) continue;
			if (j+x >= width) break;
			bits[(i+y)*width + (j+x)] = bmp->bits[i*bmp->width + j];
		}
	}
}

static __forceinline int ReadP(long *p, int w, int h, int x, int y, int k)
{
	if (x<0) x = 0; else if (x>=w) x = w-1;
	if (y<0) y = 0; else if (y>=h) y = h-1;
	return p[(x+y*w)*4+k];
}

void MyBitmap::Blur(int w, int h)
{
	if ((w <= 0) || (h <= 0)) return;

	BYTE *buf_src = new BYTE[width*height*4];
	long *buf_tmp = new long[width*height*4];
	BYTE *buf_dst = (BYTE *)bits;
	memcpy(buf_src, buf_dst, width*height*4);

	BYTE *src, *dst;
	long *tmp;

	src = buf_src;
	tmp = buf_tmp;
	dst = buf_dst;

	int y;

	for (y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			for (int k = 0; k < 4; ++k)
			{
				int tot = src[0];
				if (x > 0) tot += tmp[-4];
				if (y > 0) tot += tmp[-width*4];
				if (x > 0 && y > 0) tot -= tmp[-(width+1)*4];
				*tmp = tot;

				++src;
				++tmp;
			}
		}
	}

	src = buf_src;
	tmp = buf_tmp;

	float mul = 1.f/((w*2+1)*(h*2+1));
	for (y = 0;y<height;y++)
	{
		for (int x = 0;x<width;x++)
		{
			for (int k = 0; k < 4; ++k)
			{
				int tot = ReadP(tmp,width,height,x+w,y+h,k) +
				          ReadP(tmp,width,height,x-w,y-h,k) -
				          ReadP(tmp,width,height,x-w,y+h,k) -
				          ReadP(tmp,width,height,x+w,y-h,k);

				*dst = BYTE(tot*mul);

				++dst;
				++src;
			}
		}
	}

	delete [] buf_src;
	delete [] buf_tmp;
}

void MyBitmap::IncreaseAlpha(float q)
{
	BYTE *p = (BYTE *)bits;

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			if (!p[3])
			{
				p += 4;
				continue;
			}

			float q1 = min(q, 255.f/p[3]);

			for (int k = 0; k < 4; ++k)
			{
				*p = (BYTE)min(255, *p * q1);
				++p;
			}
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

	if (bmpColor.bmBitsPixel == 32)
	{
		if ((w != abs(bmpMask.bmWidth)) || (h != abs(bmpMask.bmHeight)))
		{
			DeleteObject(info.hbmColor);
			DeleteObject(info.hbmMask);
			HICON hicTmp = (HICON)CopyImage(hic,IMAGE_ICON,w,h,LR_COPYFROMRESOURCE);
			GetIconInfo(hicTmp, &info);
			GetObject(info.hbmMask, sizeof(bmpMask), &bmpMask);
			GetObject(info.hbmColor, sizeof(bmpColor), &bmpColor);
			DestroyIcon(hicTmp);
		}

		BYTE *cbit = new BYTE[bmpColor.bmWidthBytes*bmpColor.bmHeight];
		BYTE *mbit = new BYTE[bmpMask.bmWidthBytes*bmpMask.bmHeight];
		GetBitmapBits(info.hbmColor, bmpColor.bmWidthBytes*bmpColor.bmHeight, cbit);
		GetBitmapBits(info.hbmMask, bmpMask.bmWidthBytes*bmpMask.bmHeight, mbit);

		for (int i = 0; i < bmpColor.bmHeight; i++)
		{
			for (int j = 0; j < bmpColor.bmWidth; j++)
			{
				BYTE *pixel = cbit + i*bmpColor.bmWidthBytes + j*4;
				if (!pixel[3])
				{
					pixel[3] = (*(mbit + i*bmpMask.bmWidthBytes + j*bmpMask.bmBitsPixel/8) & (1<<(7-j%8))) ? 0 : 255;
				}

				if (pixel[3] != 255)
				{
					pixel[0] = PU_DIV255(pixel[0] * pixel[3]);
					pixel[1] = PU_DIV255(pixel[1] * pixel[3]);
					pixel[2] = PU_DIV255(pixel[2] * pixel[3]);
				}
			}
		}

		this->BlendBits((COLOR32 *)cbit, bmpColor.bmWidth, bmpColor.bmHeight, x, y, w, h);

		delete [] mbit;
		delete [] cbit;
	} else
	{
		this->saveAlpha(x,y,w,h);
		DrawIconEx(this->getDC(), x, y, hic, w, h, 0, NULL, DI_NORMAL);
		this->restoreAlpha(x,y,w,h);
	}

	DeleteObject(info.hbmColor);
	DeleteObject(info.hbmMask);
}

//Base on code by Artem Shpynov
//from clist_modern plugin
//slightly modified and integrated to MyBitmap class
void MyBitmap::DrawText(TCHAR *str, int x, int y, int blur, int strength)
{
	SIZE sz; GetTextExtentPoint32(this->getDC(), str, (int)mir_tstrlen(str), &sz);
	sz.cx += (blur+2)*2; sz.cy += (blur+2)*2;
	x -= blur+2; y -= blur+2;

	static BYTE             pbGammaWeight[256] = {0};
	static BOOL             bGammaWeightFilled = FALSE;

	if (!bGammaWeightFilled)
	{
		int i;
		for(i = 0;i<256;i++)
		{
			double f;
			double gamma = (double)700/1000;

			f = (double)i/255;
			f = pow(f,(1/gamma));

			pbGammaWeight[i] = (BYTE)(255*f);
		}
		bGammaWeightFilled = 1;
	}

	MyBitmap tmp(sz.cx, sz.cy);
	HFONT hfnTmp = (HFONT)SelectObject(tmp.getDC(), GetCurrentObject(this->getDC(), OBJ_FONT));

	RECT rc; SetRect(&rc, 0, 0, sz.cx, sz.cy);
	SetTextColor(tmp.getDC(), RGB(255,255,255));
	SetBkColor(tmp.getDC(), RGB(0,0,0));
	ExtTextOutA(tmp.getDC(), 0, 0, ETO_OPAQUE, &rc, "", 0, NULL);
	::DrawText(tmp.getDC(), str, -1, &rc, DT_CENTER|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
	SelectObject(tmp.getDC(), hfnTmp);

	GdiFlush();

	if (blur)
	{
		for (int i = 0; i < sz.cy; i++)
		{
			COLOR32 *row_src = tmp.bits + i * tmp.width;

			for (int j = 0; j < sz.cx; j++)
			{
				COLOR32 cl = row_src[j];
				if (!cl) continue;

				int a1 = (getr(cl) + getg(cl) + getb(cl)) / 3;
				row_src[j] = rgba(a1, a1, a1, a1);
			}
		}
		tmp.Blur(blur, blur);
		tmp.IncreaseAlpha((float)(strength ? strength : blur));
	}

	// use Get*Value for COLORREF and get* for COLOR32
	COLOR32 textColor = GetTextColor(this->getDC());
	COLOR32 r = GetRValue(textColor);
	COLOR32 g = GetGValue(textColor);
	COLOR32 b = GetBValue(textColor);

	int minx = max(0,-x);
	int miny = max(0,-y);
	int maxx = min(sz.cx, width-x);
	int maxy = min(sz.cy, height-y);

	for (int i = miny; i < maxy; i++)
	{
		COLOR32 *row_dst = bits + (i+y) * width + x;
		COLOR32 *row_src = tmp.bits + i * tmp.width;

		for (int j = minx; j < maxx; j++)
		{
			COLOR32 bx,rx,gx,mx;
			{
				bx = pbGammaWeight[getb(row_src[j])];
				gx = pbGammaWeight[getg(row_src[j])];
				rx = pbGammaWeight[getr(row_src[j])];
			}

			bx = (pbGammaWeight[bx]*(255-b)+bx*(b))/255;
			gx = (pbGammaWeight[gx]*(255-g)+gx*(g))/255;
			rx = (pbGammaWeight[rx]*(255-r)+rx*(r))/255;

			mx = (BYTE)(max(max(bx,rx),gx));

			if (1)
			{
				bx = (bx<mx)?(BYTE)(((WORD)bx*7+(WORD)mx)>>3):bx;
				rx = (rx<mx)?(BYTE)(((WORD)rx*7+(WORD)mx)>>3):rx;
				gx = (gx<mx)?(BYTE)(((WORD)gx*7+(WORD)mx)>>3):gx;
				// reduce boldeness at white fonts
			}
			COLOR32 cl = row_dst[j];
			if (mx)
			{
				COLOR32 rrx,grx,brx;
				COLOR32 rlx,glx,blx;
				COLOR32 axx = geta(cl);
				COLOR32 mmx = (bx+gx+rx)/3;
				COLOR32 nx = mmx; //pbGammaWeight[mx];//
				{
					//Normalize components	to alpha level
					bx = (nx*(255-axx)+bx*axx)/255;
					gx = (nx*(255-axx)+gx*axx)/255;
					rx = (nx*(255-axx)+rx*axx)/255;
					mx = (nx*(255-axx)+mmx*axx)/255;
				}
				{
					blx = getb(cl);
					glx = getg(cl);
					rlx = getr(cl);

					brx = (b-blx)*bx/255;
					grx = (g-glx)*gx/255;
					rrx = (r-rlx)*rx/255;
					row_dst[j] = rgba(rlx+rrx, glx+grx, blx+brx, mx+(255-mx)*axx/255);
				}
			}
		}
	}
}

// based on code by Yuriy Zaporozhets from:
// http://www.codeproject.com/gdi/coolrgn.asp?df = 100&forumid = 739&exp = 0&select = 6341
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
	for (int i = 0; i < height; i++)
	{
		int j; // we will need j after the loop!
		for (j = 0; j < width; j++)
		{
			ismask = opaque ? (int)geta(this->getRow(i)[j]) > level : (int)geta(this->getRow(i)[j]) < level;
			if (wasfirst)
			{
				if (!ismask)
				{
					SetRect(&pRects[pRgnData->rdh.nCount++], first, i, j, i+1);
					if ((int)(pRgnData->rdh.nCount) >= rectsCount)
					{
						rectsCount += addRectsCount;
						LPRGNDATA pRgnDataNew = (LPRGNDATA)(new BYTE[sizeof(RGNDATAHEADER) + (rectsCount)*sizeof(RECT)]);
						memcpy(pRgnDataNew, pRgnData, sizeof(RGNDATAHEADER) + pRgnData->rdh.nCount * sizeof(RECT));
						delete pRgnData;
						pRgnData = pRgnDataNew;
						pRects = (LPRECT)(&pRgnData->Buffer);
					}
					wasfirst = false;
				}
			} else
			if (ismask) // set wasfirst when mask is found
			{
				first = j;
				wasfirst = true;
			}
		}

		if (wasfirst && ismask)
		{
			SetRect(&pRects[pRgnData->rdh.nCount++], first, i, j, i+1);
			if ((int)(pRgnData->rdh.nCount) >= rectsCount)
			{
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

bool MyBitmap::loadFromFile_pixel(const char *fn, const char *fnAlpha)
{
	allocate(1,1);
	int r, g, b, a = 255;
	const char *p = fn + mir_strlen("pixel:");
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	*bits = rgba(r,g,b,a);
	return true;
}

bool MyBitmap::loadFromFile_gradient(const char *fn, const char *fnAlpha)
{
	const char *p = fn + mir_strlen("gradient:");

	if (*p == 'h') allocate(256,1);
	else allocate(1,256);

	int r, g, b, a = 255;

	p += 2;
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	COLOR32 from = rgba(r,g,b,a);

	p += 7;
	r = (hex2dec(p[0]) << 4) + hex2dec(p[1]);
	g = (hex2dec(p[2]) << 4) + hex2dec(p[3]);
	b = (hex2dec(p[4]) << 4) + hex2dec(p[5]);
	COLOR32 to = rgba(r,g,b,a);

	for (int i = 0; i < 256; ++i)
	{
		bits[i] = rgba(
				((255-i) * getr(from) + i * getr(to)) / 255,
				((255-i) * getg(from) + i * getg(to)) / 255,
				((255-i) * getb(from) + i * getb(to)) / 255,
				255
			);
	}

	return true;
}

bool MyBitmap::loadFromFile_png(const char *fn, const char *fnAlpha)
{
	if (ServiceExists(MS_PNG2DIB))
	{
		HANDLE hFile, hMap = 0;
		BYTE *ppMap = 0;
		long cbFileSize = 0;
		BITMAPINFOHEADER *pDib;
		BYTE *pDibBits;
		if ((hFile = CreateFileA(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
			if ((hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) != NULL)
					if ((ppMap = (BYTE*)MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0)) != NULL)
						cbFileSize = GetFileSize(hFile, NULL);
		if (cbFileSize)
		{
			PNG2DIB param;
			param.pSource = ppMap;
			param.cbSourceSize = cbFileSize;
			param.pResult = &pDib;
			if (CallService(MS_PNG2DIB, 0, (LPARAM)&param))
				pDibBits = (BYTE*)(pDib+1);
			else
				cbFileSize = 0;
		}

		if (ppMap) UnmapViewOfFile(ppMap);
		if (hMap) CloseHandle(hMap);
		if (hFile) CloseHandle(hFile);

		if (!cbFileSize) return false;

		BITMAPINFO *bi = (BITMAPINFO*)pDib;
		BYTE *pt = (BYTE*)bi;
		pt+=bi->bmiHeader.biSize;

		if (bi->bmiHeader.biBitCount != 32)
		{
			allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
			HDC hdcTmp = CreateCompatibleDC(getDC());
			HBITMAP hBitmap = CreateDIBitmap(getDC(), pDib, CBM_INIT, pDibBits, bi, DIB_PAL_COLORS);
			SelectObject(hdcTmp, hBitmap);
			BitBlt(this->getDC(), 0, 0, abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight), hdcTmp, 0, 0, SRCCOPY);
			this->makeOpaque();
			DeleteDC(hdcTmp);
			DeleteObject(hBitmap);
		} else
		{
			allocate(abs(bi->bmiHeader.biWidth), abs(bi->bmiHeader.biHeight));
			BYTE *p2 = (BYTE *)pt;
			for (int y = 0; y<bi->bmiHeader.biHeight; ++y)
			{
				BYTE *p1 = (BYTE *)bits + (bi->bmiHeader.biHeight-y-1)*bi->bmiHeader.biWidth*4;
				for (int x = 0; x<bi->bmiHeader.biWidth; ++x)
				{
					p1[0] = p2[0];
					p1[1] = p2[1];
					p1[2] = p2[2];
					p1[3] = p2[3];
					p1 += 4;
					p2 += 4;
				}
			}
//				memcpy(bits, pt, bi->bmiHeader.biSizeImage);
			premultipleChannels();
		}

		GlobalFree(pDib);
		return true;
	} else
	{
//			MessageBox(NULL, Translate("You need the png2dib plugin v. 0.1.3.x or later to process PNG images"), Translate("Error"), MB_OK);
		return false;
	}
}

bool MyBitmap::loadFromFile_default(const char *fn, const char *fnAlpha)
{
	SIZE sz;
	HBITMAP hBmpLoaded = (HBITMAP)LoadImageA(NULL, fn, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
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
		(alpha.getHeight() == height))
	{
		for (int i = 0; i < width*height; i++)
			bits[i] = (bits[i] & 0x00ffffff) | ((alpha.bits[i] & 0x000000ff) << 24 );
		premultipleChannels();
	} else
	{
		makeOpaque();
	}
	return true;
}

bool MyBitmap::loadFromFile(const char *fn, const char *fnAlpha)
{
	if (bits) free();

	if (!strncmp(fn, "pixel:", mir_strlen("pixel:")))
	{
		return loadFromFile_pixel(fn, fnAlpha);
	} else
	if (!strncmp(fn, "gradient:", mir_strlen("gradient:")))
	{
		return loadFromFile_gradient(fn, fnAlpha);
	} else
	{
		char ext[5];
		memcpy(ext,fn+(strlen(fn)-4),5);
		if (!mir_strcmpi(ext,".png"))
		{
			return loadFromFile_png(fn, fnAlpha);
		} else
		{
			return loadFromFile_default(fn, fnAlpha);
		}
	}
	// unreachable place
	return false;
}

void MyBitmap::allocate(int w, int h)
{
	if (dcBmp && (width == w) && (height == h)) return;

	width = w;
	height = h;

	BITMAPINFO bi;

    bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

	if (dcBmp)
	{
		DeleteObject(SelectObject(dcBmp, hBmpSave));
		DeleteDC(dcBmp);
	}

    hBmp = (HBITMAP)CreateDIBSection(0, &bi, DIB_RGB_COLORS, (void **)&bits, 0, 0);
    dcBmp = CreateCompatibleDC(0);
    hBmpSave = (HBITMAP)SelectObject(dcBmp, hBmp);

	GdiFlush();
}

void MyBitmap::free()
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
		bits[i] = rgba(getr(bits[i])*geta(bits[i])/255, getg(bits[i])*geta(bits[i])/255, getb(bits[i])*geta(bits[i])/255, geta(bits[i]));
}
