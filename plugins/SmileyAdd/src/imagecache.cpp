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

#include "stdafx.h"

static mir_cs csCache;
static OBJLIST<ImageBase> g_imagecache(25, ImageType::CompareImg);

static CMStringW lastdllname;
static HMODULE lastmodule;
static time_t laststamp;
static UINT_PTR timerId;

static void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lck(csCache);

	const time_t ts = time(0) - 10;
	if (lastmodule && ts > laststamp) {
		FreeLibrary(lastmodule);
		lastmodule = nullptr;
		lastdllname.Empty();
	}

	for (auto &it : g_imagecache.rev_iter())
		it->ProcessTimerTick(ts);

	if (g_imagecache.getCount() == 0) {
		g_imagecache.destroy();
		if (timerId && (timerId + 1) && lastmodule == nullptr) {
			KillTimer(nullptr, timerId);
			timerId = 0;
		}
	}
}

static HMODULE LoadDll(const CMStringW &file)
{
	mir_cslock lck(csCache);

	if (lastdllname != file) {
		FreeLibrary(lastmodule);
		lastdllname = file;

		lastmodule = LoadLibraryEx(file.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
	}

	laststamp = time(0);
	return lastmodule;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ImageBase

ImageBase::ImageBase(unsigned id)
{
	m_id = id;
	m_lRefCount = 1;
	m_timestamp = 0;
}

long ImageBase::AddRef(void)
{
	mir_cslock lck(csCache);
	return ++m_lRefCount;
}

long ImageBase::Release(void)
{
	mir_cslock lck(csCache);

	long cnt = m_lRefCount;
	if (cnt)
		m_lRefCount = --cnt;
	if (cnt == 0)
		m_timestamp = time(0);
	return cnt;
}

void ImageBase::ProcessTimerTick(time_t ts)
{
	mir_cslock lck(csCache);
	if (m_lRefCount == 0 && m_timestamp < ts)
		if (!g_imagecache.remove(this))
			delete this;
}

int ImageBase::CompareImg(const ImageBase *p1, const ImageBase *p2)
{
	unsigned id1 = p1->m_id;
	unsigned id2 = p2->m_id;

	if (id1 == id2) return 0;
	else return id1 < id2 ? -1 : 1;
}

void ImageBase::Draw(HDC hdc, RECT &rc, bool clip)
{
	HRGN hrgn = nullptr;
	if (clip) {
		hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		SelectClipRgn(hdc, hrgn);
	}

	SIZE iSize;
	GetSize(iSize);

	const int sizeX = rc.right - rc.left;
	const int sizeY = rc.bottom - rc.top;

	const int x = rc.left + (sizeX > iSize.cx || clip ? (sizeX - iSize.cx) / 2 : 0);
	const int y = rc.top + (sizeY > iSize.cy || clip ? (sizeY - iSize.cy) / 2 : 0);

	const int scaleX = sizeX > iSize.cx || clip ? iSize.cx : sizeX;
	const int scaleY = sizeY > iSize.cy || clip ? iSize.cy : sizeY;

	DrawInternal(hdc, x, y, scaleX, scaleY);

	if (clip) {
		SelectClipRgn(hdc, nullptr);
		DeleteObject(hrgn);
	}
}

HBITMAP ImageBase::GetBitmap(COLORREF bkgClr, int sizeX, int sizeY)
{
	RECT rc = { 0, 0, sizeX, sizeY };

	if (sizeX == 0 || sizeY == 0) {
		SIZE iSize;
		GetSize(iSize);

		if (sizeX == 0) rc.right = iSize.cx;
		if (sizeY == 0) rc.bottom = iSize.cy;
	}

	HBRUSH hBkgBrush = CreateSolidBrush(bkgClr);
	HDC hdc = GetDC(nullptr);
	HBITMAP hBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBmp);

	FillRect(hdcMem, &rc, hBkgBrush);

	Draw(hdcMem, rc, false);

	DeleteDC(hdcMem);
	ReleaseDC(nullptr, hdc);
	DeleteObject(hBkgBrush);

	return hBmp;
}

int ImageBase::SelectNextFrame(const int frame)
{
	int res = (frame + 1) % GetFrameCount();
	SelectFrame(res);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// IconType

IconType::IconType(const unsigned id, const CMStringW &file, const int index, const IcoTypeEnum type) :
	ImageBase(id)
{
	m_SmileyIcon = nullptr;

	switch (type) {
	case icoDll:
		{
			const HMODULE hModule = LoadDll(file);
			if (hModule != nullptr)
				m_SmileyIcon = (HICON)LoadImage(hModule, MAKEINTRESOURCE(-index), IMAGE_ICON, 0, 0, 0);
		}
		break;

	case icoFile:
		m_SmileyIcon = (HICON)LoadImage(nullptr, file.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
		break;

	default:
		ExtractIconEx(file.c_str(), index, nullptr, &m_SmileyIcon, 1);
		break;
	}
}

IconType::~IconType()
{
	DestroyIcon(m_SmileyIcon);
}

void IconType::DrawInternal(HDC hdc, int x, int y, int sizeX, int sizeY)
{
	if (m_SmileyIcon != nullptr)
		DrawIconEx(hdc, x, y, m_SmileyIcon, sizeX, sizeY, 0, nullptr, DI_NORMAL);
}

HICON IconType::GetIcon(void)
{
	return (HICON)CopyImage(m_SmileyIcon, IMAGE_ICON, 0, 0, 0);
}

void IconType::GetSize(SIZE &size)
{
	if (m_SmileyIcon == nullptr)
		return;

	ICONINFO ii;
	GetIconInfo(m_SmileyIcon, &ii);

	BITMAP bm;
	GetObject(ii.hbmColor, sizeof(bm), &bm);
	size.cx = bm.bmWidth;
	size.cy = bm.bmHeight;
	DeleteObject(ii.hbmMask);
	DeleteObject(ii.hbmColor);
}

/////////////////////////////////////////////////////////////////////////////////////////
// ImageListItemType

ImageListItemType::ImageListItemType(const unsigned id, HIMAGELIST hImList, int index) :
	ImageBase(id)
{
	m_index = index;
	m_hImList = hImList;
}

void ImageListItemType::DrawInternal(HDC hdc, int x, int y, int sizeX, int sizeY)
{
	SIZE iSize;
	GetSize(iSize);

	if (sizeX >= iSize.cx && sizeY >= iSize.cy)
		ImageList_Draw(m_hImList, m_index, hdc, x, y, ILD_TRANSPARENT);
	else {
		HICON hIcon = ImageList_GetIconFixed(m_hImList, m_index, ILD_TRANSPARENT);
		DrawIconEx(hdc, x, y, hIcon, sizeX, sizeY, 0, nullptr, DI_NORMAL);
		DestroyIcon(hIcon);
	}
}

HICON ImageListItemType::GetIcon(void)
{
	return ImageList_GetIconFixed(m_hImList, m_index, ILD_TRANSPARENT);
}

void ImageListItemType::GetSize(SIZE &size)
{
	ImageList_GetIconSize(m_hImList, (int*)&size.cx, (int*)&size.cy);
}

/////////////////////////////////////////////////////////////////////////////////////////
// ImageType

ImageType::ImageType(const unsigned id, const CMStringW &file, IStream *pStream) :
	ImageBase(id)
{
	m_bmp = nullptr;
	m_pPropertyItem = nullptr;
	m_nCurrentFrame = 0;
	m_nFrameCount = 0;

	if (!InitGdiPlus()) return;

	if (pStream)
		m_bmp = new Gdiplus::Bitmap(pStream);
	else
		m_bmp = new Gdiplus::Bitmap(file.c_str());

	if (m_bmp->GetLastStatus() != Gdiplus::Ok) {
		delete m_bmp;
		m_bmp = nullptr;
		return;
	}

	GUID pageGuid = Gdiplus::FrameDimensionTime;
	m_nFrameCount = m_bmp->GetFrameCount(&pageGuid);

	if (IsAnimated()) {
		int nSize = m_bmp->GetPropertyItemSize(PropertyTagFrameDelay);
		m_pPropertyItem = (Gdiplus::PropertyItem*) new char[nSize];
		m_bmp->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);
	}
}

ImageType::ImageType(const unsigned id, const CMStringW &file, const int index, const IcoTypeEnum type) :
	ImageBase(id)
{
	m_bmp = nullptr;
	m_pPropertyItem = nullptr;
	m_nCurrentFrame = 0;
	m_nFrameCount = 0;

	if (!InitGdiPlus()) return;

	switch (type) {
	case icoDll:
		{
			const HMODULE hModule = LoadDll(file);
			if (hModule != nullptr) {
				HICON hIcon = (HICON)LoadImage(hModule, MAKEINTRESOURCE(-index), IMAGE_ICON, 0, 0, 0);
				m_bmp = new Gdiplus::Bitmap(hIcon);
				DestroyIcon(hIcon);
			}
		}
		break;

	case icoFile:
		m_bmp = new Gdiplus::Bitmap(file.c_str());
		break;

	default:
		HICON hIcon = nullptr;
		ExtractIconEx(file.c_str(), index, nullptr, &hIcon, 1);
		m_bmp = new Gdiplus::Bitmap(hIcon);
		DestroyIcon(hIcon);
		break;
	}

	if (m_bmp->GetLastStatus() != Gdiplus::Ok) {
		delete m_bmp;
		m_bmp = nullptr;
	}
}

ImageType::~ImageType(void)
{
	delete[] m_pPropertyItem;
	delete m_bmp;
}

void ImageType::SelectFrame(int frame)
{
	if ((unsigned)frame >= (unsigned)m_nFrameCount) frame = 0;
	if (IsAnimated() && frame != m_nCurrentFrame) {
		m_nCurrentFrame = frame;
		GUID pageGuid = Gdiplus::FrameDimensionTime;
		m_bmp->SelectActiveFrame(&pageGuid, frame);
	}
}

void ImageType::DrawInternal(HDC hdc, int x, int y, int sizeX, int sizeY)
{
	if (m_bmp == nullptr)
		return;

	mir_cslock lck(csCache);

	Gdiplus::Graphics grp(hdc);
	grp.DrawImage(m_bmp, x, y, sizeX, sizeY);
}

int ImageType::GetFrameDelay(void) const
{
	int Del = ((long*)m_pPropertyItem->value)[m_nCurrentFrame];
	return (Del ? Del : 5);
}

HICON ImageType::GetIcon(void)
{
	if (m_bmp == nullptr)
		return nullptr;

	mir_cslock lck(csCache);

	HICON hIcon = nullptr;
	m_bmp->GetHICON(&hIcon);
	return hIcon;
}

void ImageType::GetSize(SIZE &size)
{
	if (m_bmp) {
		size.cx = m_bmp->GetWidth();
		size.cy = m_bmp->GetHeight();
	}
	else size.cx = size.cy = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ImageFType

ImageFType::ImageFType(const unsigned id) :
	ImageBase(id)
{
	m_bmp = nullptr;
}

ImageFType::ImageFType(const unsigned id, const CMStringW &file)
	: ImageBase(id)
{
	m_bmp = nullptr;

	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(file.c_str(), 0);
	if (fif == FIF_UNKNOWN)
		fif = FreeImage_GetFIFFromFilenameU(file.c_str());
	if (fif == FIF_UNKNOWN) return;

	FIBITMAP *dib = FreeImage_LoadU(fif, file.c_str(), 0);
	if (dib == nullptr) return;

	bool transp = FreeImage_IsTransparent(dib) != 0;
	FREE_IMAGE_TYPE imt = FreeImage_GetImageType(dib);
	unsigned bpp = FreeImage_GetBPP(dib);

	if (transp && bpp != 32 || imt == FIT_RGBA16) {
		FIBITMAP *tdib = FreeImage_ConvertTo32Bits(dib);
		FreeImage_Unload(dib);
		dib = tdib;
	}
	else if (!transp && bpp > 24) {
		FIBITMAP *tdib = FreeImage_ConvertTo24Bits(dib);
		FreeImage_Unload(dib);
		dib = tdib;
	}

	m_bmp = FreeImage_CreateHBITMAPFromDIB(dib);
	FreeImage_Unload(dib);

	if (transp)
		FreeImage_Premultiply(m_bmp);
}

ImageFType::~ImageFType()
{
	DeleteObject(m_bmp);
}

void ImageFType::DrawInternal(HDC hdc, int x, int y, int sizeX, int sizeY)
{
	if (m_bmp == nullptr) return;

	HDC hdcImg = CreateCompatibleDC(hdc);
	HBITMAP oldBmp = (HBITMAP)SelectObject(hdcImg, m_bmp);

	BITMAP bm;
	GetObject(m_bmp, sizeof(bm), &bm);

	if (bm.bmBitsPixel == 32) {
		BLENDFUNCTION bf = {};
		bf.SourceConstantAlpha = 255;
		bf.AlphaFormat = AC_SRC_ALPHA;
		GdiAlphaBlend(hdc, x, y, sizeX, sizeY, hdcImg, 0, 0, bm.bmWidth, bm.bmHeight, bf);
	}
	else BitBlt(hdc, x, y, sizeX, sizeY, hdcImg, 0, 0, SRCCOPY);

	SelectObject(hdcImg, oldBmp);
	DeleteDC(hdcImg);
}

HICON ImageFType::GetIcon(void)
{
	if (m_bmp == nullptr)
		return nullptr;

	BITMAP bm;
	GetObject(m_bmp, sizeof(bm), &bm);

	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, nullptr);
	ii.hbmColor = m_bmp;
	HICON hIcon = CreateIconIndirect(&ii);
	DeleteObject(ii.hbmMask);
	return hIcon;
}

void ImageFType::GetSize(SIZE &size)
{
	if (m_bmp) {
		BITMAP bm;
		GetObject(m_bmp, sizeof(bm), &bm);
		size.cx = bm.bmWidth;
		size.cy = bm.bmHeight;
	}
	else size.cx = size.cy = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

static void CALLBACK sttMainThreadCallback(PVOID)
{
	if (timerId == 0xffffffff)
		timerId = SetTimer(nullptr, 0, 10000, (TIMERPROC)timerProc);
}

void DestroyImageCache(void)
{
	if (timerId) KillTimer(nullptr, timerId);
	if (lastmodule) FreeLibrary(lastmodule);

	g_imagecache.destroy();
}

ImageBase* AddCacheImage(const CMStringW &file, int index)
{
	CMStringW tmpfile(file); tmpfile.AppendFormat(L"#%d", index);
	unsigned id = mir_hash(tmpfile.c_str(), tmpfile.GetLength() * sizeof(wchar_t));

	mir_cslock lck(csCache);

	ImageBase srch(id);
	ImageBase *img = g_imagecache.find(&srch);
	if (img == nullptr) {
		int ind = file.ReverseFind('.');
		if (ind == -1)
			return nullptr;

		CMStringW ext = file.Mid(ind + 1);
		ext.MakeLower();
		if (ext == L"dll" || ext == L"exe")
			img = opt.HQScaling ? (ImageBase*)new ImageType(id, file, index, icoDll) : (ImageBase*)new IconType(id, file, index, icoDll);
		else if (ext == L"ico")
			img = opt.HQScaling ? (ImageBase*)new ImageType(id, file, 0, icoFile) : (ImageBase*)new IconType(id, file, 0, icoFile);
		else if (ext == L"icl")
			img = opt.HQScaling ? (ImageBase*)new ImageType(id, file, index, icoIcl) : (ImageBase*)new IconType(id, file, index, icoIcl);
		else if (ext == L"gif" || ext == L"tif" || ext == L"tiff" || ext == L"webp")
			img = new ImageType(id, file, nullptr);
		else
			img = opt.HQScaling ? (ImageBase*)new ImageType(id, file, nullptr) : (ImageBase*)new ImageFType(id, file);

		g_imagecache.insert(img);

		if (timerId == 0) {
			timerId = 0xffffffff;
			CallFunctionAsync(sttMainThreadCallback, nullptr);
		}
	}
	else img->AddRef();

	return img;
}
