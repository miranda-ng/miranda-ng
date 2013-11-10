/*
Miranda SmileyAdd Plugin
Copyright (C) 2008 - 2011 Boris Krasnovskiy

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SMILEYADD_IMAGECACHE_H_
#define SMILEYADD_IMAGECACHE_H_

class ImageBase
{
protected:
	unsigned m_id;
	long m_lRefCount;
	time_t m_timestamp;

public:

	ImageBase(unsigned id);
	virtual ~ImageBase() {}

	long AddRef(void);
	long Release(void);

	void ProcessTimerTick(time_t ts);

	virtual void GetSize(SIZE& /* size */) {};
	virtual int  GetFrameCount(void) const { return 0; }
	virtual int  GetFrameDelay(void) const { return 0; }
	virtual HICON GetIcon(void) { return NULL; };
	virtual void DrawInternal(HDC /* dc */, int /* x */, int /* y */, int /* sizeX */, int /* sizeY */) {};
	virtual void SelectFrame(int /* frame */) {}

	bool IsAnimated(void) const { return GetFrameCount() > 1; }
	HBITMAP GetBitmap(COLORREF bkgClr, int sizeX, int sizeY);
	void Draw(HDC dc, RECT &rc, bool clip);
	int SelectNextFrame(const int frame);

	static int CompareImg(const ImageBase* p1, const ImageBase* p2);
};


typedef enum
{
	icoDll,
	icoFile,
	icoIcl
} 
IcoTypeEnum;


class IconType : public ImageBase
{
private:
	HICON m_SmileyIcon;

public:
	IconType(const unsigned id, const CMString& file, const int index, const IcoTypeEnum type);
	~IconType();

	void DrawInternal(HDC dc, int x, int y, int sizeX, int sizeY);
	HICON GetIcon(void);
	void GetSize(SIZE& size);
};


class ImageListItemType : public ImageBase
{
private:
	int m_index;
	HIMAGELIST m_hImList;

public:
	ImageListItemType(const unsigned id, HIMAGELIST hImList, int index);

	void DrawInternal(HDC dc, int x, int y, int sizeX, int sizeY);
	HICON GetIcon(void);
	void GetSize(SIZE& size);
};


class ImageType : public ImageBase
{
private:
	int m_nCurrentFrame;
	int m_nFrameCount;

	Gdiplus::Bitmap*        m_bmp;
	Gdiplus::PropertyItem*	m_pPropertyItem;

public:

	ImageType(const unsigned id, const CMString& file, IStream* pStream);
	ImageType(const unsigned id, const CMString& file, const int index, const IcoTypeEnum type);
	~ImageType();

	void SelectFrame(int frame);

	void DrawInternal(HDC dc, int x, int y, int sizeX, int sizeY);
	HICON GetIcon(void);
	void GetSize(SIZE& size);

	int  GetFrameDelay(void) const;
	int  GetFrameCount(void) const { return m_nFrameCount; }
};

class ImageFType : public ImageBase
{
protected:
	HBITMAP m_bmp;

public:

	ImageFType(const unsigned id);
	ImageFType(const unsigned id, const CMString& file);
	~ImageFType();

	void DrawInternal(HDC dc, int x, int y, int sizeX, int sizeY);
	HICON GetIcon(void);
	void GetSize(SIZE& size);
};

ImageBase* AddCacheImage(const CMString& file, int index);

void InitImageCache(void);
void DestroyImageCache(void);

#endif

