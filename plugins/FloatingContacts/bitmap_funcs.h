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

#ifndef __bitmap_funcs_h__
#define __bitmap_funcs_h__

// This should make bitmap manipulations much easier...
class MyBitmap
{
public:
	typedef unsigned long COLOR32;
	static inline COLOR32 RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xff)
	{
		return (a << 24) | (r << 16) | (g << 8) | b;
	};

private:
	HBITMAP hBmpSave, hBmp;
	HDC dcBmp;
	COLOR32 *bits;
	COLOR32 *bitsSave;
	int width, height;

	void free();

	bool loadFromFile_pixel(const char *fn, const char *fnAlpha = 0);
	bool loadFromFile_gradient(const char *fn, const char *fnAlpha = 0);
	bool loadFromFile_png(const char *fn, const char *fnAlpha = 0);
	bool loadFromFile_default(const char *fn, const char *fnAlpha = 0);
	void premultipleChannels();

public:
	MyBitmap();
	MyBitmap(int w, int h);
	MyBitmap(const char *fn, const char *fnAlpha = 0);
	~MyBitmap();
	void allocate(int w, int h);

	bool loadFromFile(const char *fn, const char *fnAlpha = 0);

	int getWidth() { return width; }
	int getHeight() { return height; }

	HDC getDC() { return dcBmp; }
	HBITMAP getBitmap() { return hBmp; }

	void setAlpha(BYTE level);
	void setAlphaRect(int x1, int y1, int x2, int y2, BYTE level);
	void setAlphaRect(RECT rc, BYTE level) { setAlphaRect(rc.left, rc.top, rc.right, rc.bottom, level);  }

	void makeOpaque();
	void makeOpaqueRect(int x1, int y1, int x2, int y2);
	void makeOpaqueRect(RECT rc) { makeOpaqueRect(rc.left, rc.top, rc.right, rc.bottom);  }

	void saveAlpha(int x = 0, int y = 0, int w = 0, int h = 0);
	void restoreAlpha(int x = 0, int y = 0, int w = 0, int h = 0);

	void DrawBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h);
	void BlendBits(COLOR32 *inbits, int inw, int inh, int x, int y, int w, int h);

	void DrawNoAlpha(MyBitmap *bmp, int x, int y, int w, int h);

	void Blend(MyBitmap *bmp, int x, int y, int w, int h);
	void Draw(MyBitmap *bmp, int x, int y, int w, int h);

	void BlendColorized(MyBitmap *bmp, int x, int y, int w, int h, COLOR32 color);
	void DrawColorized(MyBitmap *bmp, int x, int y, int w, int h, COLOR32 color);

	void BlendPart(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h);
	void BlendPartColorized(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h, COLOR32 color);
	void DrawPart(MyBitmap *bmp, int xin, int yin, int win, int hin, int x, int y, int w, int h);
//	void DrawPartNoAlpha(MyBitmap *bmp, int x, int y, int w, int h);
//	void DrawPartColorized(MyBitmap *bmp, int x, int y, int w, int h, COLOR32 color);

	void Blur(int w, int h);
	void IncreaseAlpha(float q);

	void DrawIcon(HICON hic, int x, int y, int w = 0, int h = 0);
	void DrawText(TCHAR *str, int x, int y, int blur = 0, int strength = 0);

	__forceinline COLOR32 *getBits() { return bits; }
	__forceinline COLOR32 *getRow(int row) { return bits + row * width; }
	__forceinline COLOR32 *operator[] (int row) { return bits + row * width; }

	static __forceinline COLOR32 rgba(COLOR32 r, COLOR32 g, COLOR32 b, COLOR32 a)
	{
		return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
	}
	static __forceinline COLOR32 getr(COLOR32 c)
	{
		return (c >> 16) & 0xff;
	}
	static __forceinline COLOR32 getg(COLOR32 c)
	{
		return (c >> 8) & 0xff;
	}
	static __forceinline COLOR32 getb(COLOR32 c)
	{
		return c & 0xff;
	}
	static __forceinline COLOR32 geta(COLOR32 c)
	{
		return (c >> 24) & 0xff;
	}

	HRGN buildOpaqueRgn(int level = 64, bool opaque = true);
};

#endif // __bitmap_funcs_h__
