/*
 * test for gdi+
 */


#include "hdr/modern_commonheaders.h"

#undef Translate	//otherwise will conflict with gdi plus Translate
#include <gdiplus.h>

#include "hdr/modern_global_structure.h"

using namespace Gdiplus;

HBITMAP ske_CreateDIB32(int cx, int cy);

BYTE      saved_alpha;
ULONG_PTR g_gdiplusToken;
int       g_hottrack;

void InitGdiPlus(void)
{
	GdiplusStartupInput gdiplusStartupInput;
	if (g_gdiplusToken == 0)
		GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
}

void ShutdownGdiPlus(void)
{
	GdiplusStartupInput gdiplusStartupInput;
	if (g_gdiplusToken)
		GdiplusShutdown(g_gdiplusToken);
	g_gdiplusToken = 0;
}

static ColorMatrix ClrMatrix =
{
	1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

DWORD argb_from_cola(COLORREF col, BYTE alpha)
{
	return((BYTE)(alpha) << 24 | col);
}

HBITMAP GDIPlus_LoadGlyphImage(const TCHAR *tszFileName)
{
	// Create a Bitmap object from a JPEG file.
	Bitmap bitmap(tszFileName, 0);

	// Clone a portion of the bitmap.
	Bitmap* clone = bitmap.Clone(0, 0, bitmap.GetWidth(), bitmap.GetHeight(), PixelFormat32bppPARGB);
	HBITMAP hbmp = NULL;
	if (clone) {
		clone->GetHBITMAP(Color(0, 0, 0), &hbmp);
		delete clone;
	}
	return hbmp;
}

void DrawAvatarImageWithGDIp(HDC hDestDC, int x, int y, DWORD width, DWORD height, HBITMAP hbmp, int x1, int y1, DWORD width1, DWORD height1, DWORD flag, BYTE alpha)
{
	BITMAP bmp;
	Bitmap *bm;
	BYTE * bmbits = NULL;
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	Graphics g(hDestDC);
	if (bmp.bmBitsPixel == 32 && (flag&AVS_PREMULTIPLIED)) {
		bmbits = (BYTE*)bmp.bmBits;
		if (!bmbits) {
			bmbits = (BYTE*)malloc(bmp.bmHeight*bmp.bmWidthBytes);
			GetBitmapBits(hbmp, bmp.bmHeight*bmp.bmWidthBytes, bmbits);
		}
		bm = new Bitmap(bmp.bmWidth, bmp.bmHeight, bmp.bmWidthBytes, PixelFormat32bppPARGB, bmbits);
		bm->RotateFlip(RotateNoneFlipY);
		if (!bmp.bmBits) {
			bm->RotateFlip(RotateNoneFlipY);
			free(bmbits);
		}
	}
	else bm = new Bitmap(hbmp, NULL);

	ImageAttributes attr;
	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, ((float)alpha) / 255, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	attr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	RectF rect((float)x, (float)y, (float)width, (float)height);
	g.DrawImage(bm, rect, (float)x1, (float)y1, (float)width1, (float)height1, UnitPixel, &attr, NULL, NULL);
	delete bm;
}

BOOL GDIPlus_AlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest, HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION * bf)
{
	Graphics g(hdcDest);
	BITMAP bmp;
	HBITMAP hbmp = (HBITMAP)GetCurrentObject(hdcSrc, OBJ_BITMAP);
	GetObject(hbmp, sizeof(BITMAP), &bmp);

	Bitmap *bm;
	if (bmp.bmBitsPixel == 32 && bf->AlphaFormat) {
		bm = new Bitmap(bmp.bmWidth, bmp.bmHeight, bmp.bmWidthBytes, PixelFormat32bppPARGB, (BYTE*)bmp.bmBits);
		bm->RotateFlip(RotateNoneFlipY);
	}
	else bm = new Bitmap(hbmp, NULL);

	ImageAttributes attr;
	ColorMatrix ClrMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, ((float)bf->SourceConstantAlpha) / 255, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	attr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, ColorAdjustTypeBitmap);

	if (bf->BlendFlags & 128 && nWidthDest < nWidthSrc && nHeightDest < nHeightSrc) {
		g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		attr.SetGamma((REAL)0.8, ColorAdjustTypeBitmap);
	}
	else g.SetInterpolationMode(InterpolationModeLowQuality);

	RectF rect((float)nXOriginDest, (float)nYOriginDest, (float)nWidthDest, (float)nHeightDest);
	g.DrawImage(bm, rect, (float)nXOriginSrc, (float)nYOriginSrc, (float)nWidthSrc, (float)nHeightSrc, UnitPixel, &attr, NULL, NULL);
	delete bm;
	return TRUE;
}

COLORREF __inline _revcolref(COLORREF colref)
{
	return RGB(GetBValue(colref), GetGValue(colref), GetRValue(colref));
}

/////////////////////////////////////////////////////////////////////////////////
// GDIPlus_IsAnimatedGIF and GDIPlus_ExtractAnimatedGIF
// based on routine from http://www.codeproject.com/vcpp/gdiplus/imageexgdi.asp
//

BOOL GDIPlus_IsAnimatedGif(TCHAR * szName)
{
	int nFrameCount = 0;
	Image image(szName);
	UINT count = 0;

	count = image.GetFrameDimensionsCount();
	GUID* pDimensionIDs = new GUID[count];

	// Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	// Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	delete[]  pDimensionIDs;

	return (BOOL)(nFrameCount > 1);
}

void GDIPlus_ExtractAnimatedGIF(TCHAR *szName, int width, int height, HBITMAP &pBitmap, int* &pframesDelay, int &pframesCount, SIZE &pSizeAvatar)
{
	int nFrameCount = 0;
	Bitmap image(szName);
	PropertyItem * pPropertyItem;

	UINT count = image.GetFrameDimensionsCount();
	GUID *pDimensionIDs = new GUID[count];

	// Get the list of frame dimensions from the Image object.
	image.GetFrameDimensionsList(pDimensionIDs, count);

	// Get the number of frames in the first dimension.
	nFrameCount = image.GetFrameCount(&pDimensionIDs[0]);

	// Assume that the image has a property item of type PropertyItemEquipMake.
	// Get the size of that property item.
	int nSize = image.GetPropertyItemSize(PropertyTagFrameDelay);

	// Allocate a buffer to receive the property item.
	pPropertyItem = (PropertyItem*)malloc(nSize);

	image.GetPropertyItem(PropertyTagFrameDelay, nSize, pPropertyItem);

	int clipWidth;
	int clipHeight;
	int imWidth = image.GetWidth();
	int imHeight = image.GetHeight();
	float xscale = (float)width / imWidth;
	float yscale = (float)height / imHeight;
	xscale = min(xscale, yscale);
	clipWidth = (int)(xscale*imWidth + .5);
	clipHeight = (int)(xscale*imHeight + .5);

	HBITMAP hBitmap = ske_CreateDIB32(clipWidth*nFrameCount, height);
	HDC hdc = CreateCompatibleDC(NULL);
	HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hBitmap);
	Graphics graphics(hdc);
	ImageAttributes attr;

	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
	graphics.SetPixelOffsetMode(PixelOffsetModeHalf);
	int *delays = (int*)malloc(nFrameCount*sizeof(int));
	memset(delays, 0, nFrameCount*sizeof(int));

	// The GDIPlus strange behavior: without next 2 lines it will draw first frame anti aliased, but next - not
	GUID pageGuid = FrameDimensionTime;
	if (nFrameCount > 1)
		image.SelectActiveFrame(&pageGuid, 1);

	for (int i = 0; i < nFrameCount; i++) {
		image.SelectActiveFrame(&pageGuid, i);
		graphics.DrawImage(&image, Rect(i*clipWidth, 0, clipWidth, clipHeight), 0, 0, imWidth, imHeight, UnitPixel, &attr);
		long lPause = ((long*)pPropertyItem->value)[i] * 10;
		delays[i] = (int)lPause;
	}
	SelectObject(hdc, oldBmp);
	DeleteDC(hdc);
	free(pPropertyItem);
	delete[] pDimensionIDs;

	pBitmap = hBitmap;
	pframesDelay = delays;
	pframesCount = nFrameCount;
	pSizeAvatar.cx = clipWidth;
	pSizeAvatar.cy = clipHeight;

	GdiFlush();
}
