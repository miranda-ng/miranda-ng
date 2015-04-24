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

	void allocate(int w, int h);
	void free();

	void premultipleChannels();

public:
	MyBitmap();
	MyBitmap(int w, int h);
	MyBitmap(TCHAR *fn, TCHAR *fnAlpha = 0);
	~MyBitmap();

	bool loadFromFile(TCHAR *fn, TCHAR *fnAlpha = 0);

	int getWidth() { return width; }
	int getHeight() { return height; }

	HDC getDC() { return dcBmp; }
	HBITMAP getBitmap() { return hBmp; }

	
	void makeOpaque();

	void saveAlpha(int x = 0, int y = 0, int w = 0, int h = 0);
	void restoreAlpha(int x = 0, int y = 0, int w = 0, int h = 0);
	
	void DrawText(TCHAR *str, int x, int y);

	inline COLOR32 *getBits() { return bits; }
	inline COLOR32 *getRow(int row) { return bits + row * width; }
	inline COLOR32 *operator[] (int row) { return bits + row * width; }

	COLOR32 rgba(COLOR32 r, COLOR32 g, COLOR32 b, COLOR32 a)
	{
		return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
	}
	COLOR32 getr(COLOR32 c)
	{
		return (c >> 16) & 0xff;
	}
	COLOR32 getg(COLOR32 c)
	{
		return (c >> 8) & 0xff;
	}
	COLOR32 getb(COLOR32 c)
	{
		return c & 0xff;
	}
	COLOR32 geta(COLOR32 c)
	{
		return (c >> 24) & 0xff;
	}
};

#endif // __bitmap_funcs_h__
