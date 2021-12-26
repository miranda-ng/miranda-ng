#include "stdafx.h"

extern HINSTANCE hinstance;


void HalfBitmap32Alpha(HBITMAP hBitmap)
{
	BITMAP bmp;
	uint32_t dwLen;
	uint8_t *p;
	int x, y;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	p = (uint8_t *)malloc(dwLen);
	if (p == nullptr)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	for (y = 0; y < bmp.bmHeight; ++y) {
		uint8_t *px = p + bmp.bmWidth * 4 * y;

		for (x = 0; x < bmp.bmWidth; ++x)
		{
			px[3] >>= 1;
			px += 4;
		}
	}

	SetBitmapBits(hBitmap, dwLen, p);

	free(p);
}


// Make a bitmap all transparent, but only if it is a 32bpp
//void MakeBmpTransparent(HBITMAP hBitmap)
//{
//	BITMAP bmp;
//	uint32_t dwLen;
//	uint8_t *p;
//
//	GetObject(hBitmap, sizeof(bmp), &bmp);
//
//	if (bmp.bmBitsPixel != 32)
//		return;
//
//	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
//	p = (uint8_t *)malloc(dwLen);
//	if (p == NULL)
//		return;
//
//	memset(p, 0, dwLen);
//	SetBitmapBits(hBitmap, dwLen, p);
//
//	free(p);
//}


// Correct alpha from bitmaps loaded without it (it cames with 0 and should be 255)
void CorrectBitmap32Alpha(HBITMAP hBitmap, BOOL force)
{
	BITMAP bmp;
	uint32_t dwLen;
	uint8_t *p;
	int x, y;
	BOOL fixIt;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	p = (uint8_t *)malloc(dwLen);
	if (p == nullptr)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	fixIt = TRUE;
	for (y = 0; fixIt && y < bmp.bmHeight; ++y) {
		uint8_t *px = p + bmp.bmWidth * 4 * y;

		for (x = 0; fixIt && x < bmp.bmWidth; ++x)
		{
			if (px[3] != 0 && !force)
			{
				fixIt = FALSE;
			}
			else
			{
				if (px[0] != 0 || px[1] != 0 || px[2] != 0)
					px[3] = 255;
			}

			px += 4;
		}
	}

	if (fixIt)
		SetBitmapBits(hBitmap, dwLen, p);

	free(p);
}


HBITMAP CopyBitmapTo32(HBITMAP hBitmap)
{
	uint8_t * ptPixels;

	BITMAP bmp;
	uint32_t dwLen;
	uint8_t *p;

	GetObject(hBitmap, sizeof(bmp), &bmp);

	dwLen = bmp.bmWidth * bmp.bmHeight * 4;
	p = (uint8_t *)malloc(dwLen);
	if (p == nullptr)
		return nullptr;

	// Create bitmap
	BITMAPINFO RGB32BitsBITMAPINFO;
	memset(&RGB32BitsBITMAPINFO, 0, sizeof(BITMAPINFO));
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = bmp.bmWidth;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = bmp.bmHeight;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

	HBITMAP hDirectBitmap = CreateDIBSection(nullptr,
		(BITMAPINFO *)&RGB32BitsBITMAPINFO,
		DIB_RGB_COLORS,
		(void **)&ptPixels,
		nullptr, 0);

	// Copy data
	if (bmp.bmBitsPixel != 32)
	{
		HDC hdcOrig, hdcDest;
		HBITMAP oldOrig, oldDest;

		hdcOrig = CreateCompatibleDC(nullptr);
		oldOrig = (HBITMAP)SelectObject(hdcOrig, hBitmap);

		hdcDest = CreateCompatibleDC(nullptr);
		oldDest = (HBITMAP)SelectObject(hdcDest, hDirectBitmap);

		BitBlt(hdcDest, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcOrig, 0, 0, SRCCOPY);

		SelectObject(hdcDest, oldDest);
		DeleteObject(hdcDest);
		SelectObject(hdcOrig, oldOrig);
		DeleteObject(hdcOrig);

		// Set alpha
		CorrectBitmap32Alpha(hDirectBitmap, FALSE);
	}
	else
	{
		GetBitmapBits(hBitmap, dwLen, p);
		SetBitmapBits(hDirectBitmap, dwLen, p);
	}

	free(p);

	return hDirectBitmap;
}


//HBITMAP CreateBitmap32(int cx, int cy)
//{
//	BITMAPINFO RGB32BitsBITMAPINFO;
//	UINT * ptPixels;
//	HBITMAP DirectBitmap;
//
//	memset(&RGB32BitsBITMAPINFO, 0, sizeof(BITMAPINFO));
//	RGB32BitsBITMAPINFO.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
//	RGB32BitsBITMAPINFO.bmiHeader.biWidth=cx;//bm.bmWidth;
//	RGB32BitsBITMAPINFO.bmiHeader.biHeight=cy;//bm.bmHeight;
//	RGB32BitsBITMAPINFO.bmiHeader.biPlanes=1;
//	RGB32BitsBITMAPINFO.bmiHeader.biBitCount=32;
//
//	DirectBitmap = CreateDIBSection(NULL,
//									(BITMAPINFO *)&RGB32BitsBITMAPINFO,
//									DIB_RGB_COLORS,
//									(void **)&ptPixels,
//									NULL, 0);
//	return DirectBitmap;
//}


BOOL MakeBitmap32(HBITMAP *hBitmap)
{
	BITMAP bmp;

	GetObject(*hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32) {
		// Convert to 32 bpp
		HBITMAP hBmpTmp = CopyBitmapTo32(*hBitmap);
		DeleteObject(*hBitmap);
		*hBitmap = hBmpTmp;
	}

	return TRUE;
}


//#define GET_PIXEL(__P__, __X__, __Y__) ( __P__ + width * 4 * (__Y__) + 4 * (__X__))

//BOOL MakeGrayscale(HBITMAP *hBitmap)
//{
//	uint8_t *p = NULL;
//	uint8_t *p1;
//	uint32_t dwLen;
//    int width, height, x, y;
//    BITMAP bmp;
//
//	GetObject(*hBitmap, sizeof(bmp), &bmp);
//    width = bmp.bmWidth;
//	height = bmp.bmHeight;
//
//	dwLen = width * height * 4;
//	p = (uint8_t *)malloc(dwLen);
//    if (p == NULL)
//	{
//		return FALSE;
//	}
//
//	if (bmp.bmBitsPixel != 32)
//	{
//		// Convert to 32 bpp
//		HBITMAP hBmpTmp = CopyBitmapTo32(*hBitmap);
//		DeleteObject(*hBitmap);
//		*hBitmap = hBmpTmp;
//	}
//	GetBitmapBits(*hBitmap, dwLen, p);
//
//	// Make grayscale
//	for (y = 0 ; y < height ; y++)
//	{
//		for (x = 0 ; x < width ; x++)
//		{
//			p1 = GET_PIXEL(p, x, y);
//			p1[0] = p1[1] = p1[2] = ( p1[0] + p1[1] + p1[2] ) / 3;
//		}
//	}
//
//    dwLen = SetBitmapBits(*hBitmap, dwLen, p);
//    free(p);
//
//	return TRUE;
//}


HICON MakeHalfAlphaIcon(HICON SourceIcon)
{
	ICONINFO TargetIconInfo;
	BITMAP TargetBitmapInfo;
	HICON TargetIcon, TempIcon;

	TempIcon = CopyIcon(SourceIcon);
	if (!GetIconInfo(TempIcon, &TargetIconInfo))
		return nullptr;

	if (!GetObject(TargetIconInfo.hbmColor, sizeof(BITMAP), &TargetBitmapInfo))
		return nullptr;

	MakeBitmap32(&TargetIconInfo.hbmColor);
	HalfBitmap32Alpha(TargetIconInfo.hbmColor);
	TargetIcon = CreateIconIndirect(&TargetIconInfo);

	DestroyIcon(TempIcon);
	DeleteObject(TargetIconInfo.hbmColor);
	DeleteObject(TargetIconInfo.hbmMask);
	return TargetIcon;
}


//HICON MakeGrayscaleIcon(HICON SourceIcon)
//{
//	ICONINFO TargetIconInfo;
//	BITMAP TargetBitmapInfo;
//	HICON TargetIcon, TempIcon;
//
//	TempIcon = CopyIcon(SourceIcon);
//	if (! GetIconInfo(TempIcon, &TargetIconInfo) || GetObject(TargetIconInfo.hbmColor, sizeof(BITMAP), &TargetBitmapInfo)==0) return NULL;
//
//	MakeGrayscale(&TargetIconInfo.hbmColor);
//
//	TargetIcon = CreateIconIndirect(&TargetIconInfo);
//	DestroyIcon(TempIcon);
//
//	return TargetIcon;
//}


HICON BindOverlayIcon(HICON SourceIcon, int iIcon)
{
	HICON TempIcon = CopyIcon(SourceIcon);
	ICONINFO OverlayIconInfo, TargetIconInfo;
	if (!GetIconInfo(TempIcon, &TargetIconInfo))
		return nullptr;

	MakeBitmap32(&TargetIconInfo.hbmColor);
	CorrectBitmap32Alpha(TargetIconInfo.hbmColor, FALSE);

	BITMAP OverlayBitmapInfo, TargetBitmapInfo;
	GetObject(TargetIconInfo.hbmColor, sizeof(BITMAP), &TargetBitmapInfo);

	HICON OverlayIcon = g_plugin.getIcon(iIcon);
	if (!GetIconInfo(OverlayIcon, &OverlayIconInfo) || !GetObject(OverlayIconInfo.hbmColor, sizeof(BITMAP), &OverlayBitmapInfo))
		return nullptr;

	HDC TargetDC = CreateCompatibleDC(nullptr);
	HBITMAP OldTargetBitmap = (HBITMAP)SelectObject(TargetDC, TargetIconInfo.hbmColor);

	HDC OverlayDC = CreateCompatibleDC(nullptr);
	HBITMAP OldOverlayBitmap = (HBITMAP)SelectObject(OverlayDC, OverlayIconInfo.hbmColor);

	BLENDFUNCTION bf = { 0, 0, 255, 1 };
	AlphaBlend(TargetDC, 0, 0, TargetBitmapInfo.bmWidth, TargetBitmapInfo.bmHeight,
		OverlayDC, 0, 0, OverlayBitmapInfo.bmWidth, OverlayBitmapInfo.bmHeight, bf);

	SelectObject(TargetDC, TargetIconInfo.hbmMask);
	SelectObject(OverlayDC, OverlayIconInfo.hbmMask);

	BitBlt(TargetDC, 0, 0, TargetBitmapInfo.bmWidth, TargetBitmapInfo.bmHeight,
		OverlayDC, 0, 0, SRCCOPY);

	HICON TargetIcon = CreateIconIndirect(&TargetIconInfo);
	DestroyIcon(TempIcon);

	SelectObject(TargetDC, OldTargetBitmap);
	DeleteObject(TargetIconInfo.hbmColor);
	DeleteObject(TargetIconInfo.hbmMask);
	DeleteDC(TargetDC);

	SelectObject(OverlayDC, OldOverlayBitmap);
	DeleteObject(OverlayIconInfo.hbmColor);
	DeleteObject(OverlayIconInfo.hbmMask);
	DeleteDC(OverlayDC);

	return TargetIcon;
}


// EOF
