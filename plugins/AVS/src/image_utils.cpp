#include "commonheaders.h"
#include "image_utils.h"

#define GET_PIXEL(__P__, __X__, __Y__) ( __P__ + width * 4 * (__Y__) + 4 * (__X__))

// Make a bitmap all transparent, but only if it is a 32bpp
void MakeBmpTransparent(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);
	if (bmp.bmBitsPixel != 32)
		return;

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	BYTE *p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return;

	memset(p, 0, dwLen);
	SetBitmapBits(hBitmap, dwLen, p);

	free(p);
}

/////////////////////////////////////////////////////////////////////////////////////////

HBITMAP CopyBitmapTo32(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * 4;
	BYTE *p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return NULL;

	// Create bitmap
	BITMAPINFO RGB32BitsBITMAPINFO;
	memset(&RGB32BitsBITMAPINFO, 0, sizeof(BITMAPINFO));
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = bmp.bmWidth;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = bmp.bmHeight;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

	BYTE *ptPixels;
	HBITMAP hDirectBitmap = CreateDIBSection(NULL, (BITMAPINFO *)&RGB32BitsBITMAPINFO, DIB_RGB_COLORS, (void **)&ptPixels, NULL, 0);

	// Copy data
	if (bmp.bmBitsPixel != 32) {
		HDC hdcOrig = CreateCompatibleDC(NULL);
		HBITMAP oldOrig = (HBITMAP)SelectObject(hdcOrig, hBitmap);

		HDC hdcDest = CreateCompatibleDC(NULL);
		HBITMAP oldDest = (HBITMAP)SelectObject(hdcDest, hDirectBitmap);

		BitBlt(hdcDest, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcOrig, 0, 0, SRCCOPY);

		SelectObject(hdcDest, oldDest);
		DeleteObject(hdcDest);
		SelectObject(hdcOrig, oldOrig);
		DeleteObject(hdcOrig);

		// Set alpha
		fei->FI_CorrectBitmap32Alpha(hDirectBitmap, FALSE);
	}
	else {
		GetBitmapBits(hBitmap, dwLen, p);
		SetBitmapBits(hDirectBitmap, dwLen, p);
	}

	free(p);

	return hDirectBitmap;
}

HBITMAP CreateBitmap32(int cx, int cy)
{
	BITMAPINFO RGB32BitsBITMAPINFO;
	memset(&RGB32BitsBITMAPINFO, 0, sizeof(BITMAPINFO));
	RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	RGB32BitsBITMAPINFO.bmiHeader.biWidth = cx;//bm.bmWidth;
	RGB32BitsBITMAPINFO.bmiHeader.biHeight = cy;//bm.bmHeight;
	RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
	RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;

	UINT *ptPixels;
	return CreateDIBSection(NULL, (BITMAPINFO *)&RGB32BitsBITMAPINFO, DIB_RGB_COLORS, (void **)&ptPixels, NULL, 0);
}

// Set the color of points that are transparent
void SetTranspBkgColor(HBITMAP hBitmap, COLORREF color)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	if (bmp.bmBitsPixel != 32)
		return;

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	BYTE *p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	bool changed = false;
	for (int y = 0; y < bmp.bmHeight; ++y) {
		BYTE *px = p + bmp.bmWidth * 4 * y;

		for (int x = 0; x < bmp.bmWidth; ++x) {
			if (px[3] == 0) {
				px[0] = GetBValue(color);
				px[1] = GetGValue(color);
				px[2] = GetRValue(color);
				changed = true;
			}
			px += 4;
		}
	}

	if (changed)
		SetBitmapBits(hBitmap, dwLen, p);

	free(p);
}


#define HIMETRIC_INCH   2540    // HIMETRIC units per inch

void SetHIMETRICtoDP(HDC hdc, SIZE* sz)
{
	POINT pt;
	int nMapMode = GetMapMode(hdc);
	if (nMapMode < MM_ISOTROPIC && nMapMode != MM_TEXT) {
		// when using a constrained map mode, map against physical inch
		SetMapMode(hdc, MM_HIMETRIC);
		pt.x = sz->cx;
		pt.y = sz->cy;
		LPtoDP(hdc, &pt, 1);
		sz->cx = pt.x;
		sz->cy = pt.y;
		SetMapMode(hdc, nMapMode);
	}
	else {
		// map against logical inch for non-constrained mapping modes
		int cxPerInch, cyPerInch;
		cxPerInch = GetDeviceCaps(hdc, LOGPIXELSX);
		cyPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
		sz->cx = MulDiv(sz->cx, cxPerInch, HIMETRIC_INCH);
		sz->cy = MulDiv(sz->cy, cyPerInch, HIMETRIC_INCH);
	}

	pt.x = sz->cx;
	pt.y = sz->cy;
	DPtoLP(hdc, &pt, 1);
	sz->cx = pt.x;
	sz->cy = pt.y;
}

HBITMAP BmpFilterLoadBitmap(BOOL *bIsTransparent, const TCHAR *ptszFilename)
{
	if (fei == NULL)
		return 0;

	FIBITMAP *dib = (FIBITMAP*)CallService(MS_IMG_LOAD, (WPARAM)ptszFilename, IMGL_RETURNDIB | IMGL_TCHAR);
	if (dib == NULL)
		return 0;

	FIBITMAP *dib32 = NULL;
	if (fei->FI_GetBPP(dib) != 32) {
		dib32 = fei->FI_ConvertTo32Bits(dib);
		fei->FI_Unload(dib);
	}
	else dib32 = dib;

	if (dib32 == NULL)
		return NULL;

	if (fei->FI_IsTransparent(dib32))
		if (bIsTransparent)
			*bIsTransparent = TRUE;

	if (fei->FI_GetWidth(dib32) > 128 || fei->FI_GetHeight(dib32) > 128) {
		FIBITMAP *dib_new = fei->FI_MakeThumbnail(dib32, 128, FALSE);
		fei->FI_Unload(dib32);
		if (dib_new == NULL)
			return 0;
		dib32 = dib_new;
	}

	HBITMAP bitmap = fei->FI_CreateHBITMAPFromDIB(dib32);
	fei->FI_Unload(dib32);
	fei->FI_CorrectBitmap32Alpha(bitmap, FALSE);
	return bitmap;
}

static HWND hwndClui = 0;

//
// Save ///////////////////////////////////////////////////////////////////////////////////////////
// PNG and BMP will be saved as 32bit images, jpg as 24bit with default quality (75)
// returns 1 on success, 0 on failure

int BmpFilterSaveBitmap(HBITMAP hBmp, const TCHAR *ptszFile, int flags)
{
	if (fei == NULL)
		return -1;

	TCHAR tszFilename[MAX_PATH];
	if (!PathToAbsoluteT(ptszFile, tszFilename))
		_tcsncpy_s(tszFilename, ptszFile, _TRUNCATE);

	if (_tcslen(tszFilename) <= 4)
		return -1;
	
	IMGSRVC_INFO i = { 0 };
	i.cbSize = sizeof(IMGSRVC_INFO);
	i.wszName = tszFilename;
	i.hbm = hBmp;
	i.dwMask = IMGI_HBITMAP;
	i.fif = FIF_UNKNOWN;

	return !CallService(MS_IMG_SAVE, (WPARAM)&i, MAKELONG(IMGL_TCHAR, flags));
}

// Other utilities ////////////////////////////////////////////////////////////////////////////////

static BOOL ColorsAreTheSame(int colorDiff, BYTE *px1, BYTE *px2)
{
	return abs(px1[0] - px2[0]) <= colorDiff
		&& abs(px1[1] - px2[1]) <= colorDiff
		&& abs(px1[2] - px2[2]) <= colorDiff;
}

void AddToStack(int *stack, int *topPos, int x, int y)
{
	// Already is in stack?
	for (int i = 0; i < *topPos; i += 2)
		if (stack[i] == x && stack[i + 1] == y)
			return;

	stack[*topPos] = x;
	(*topPos)++;

	stack[*topPos] = y;
	(*topPos)++;
}

BOOL GetColorForPoint(int colorDiff, BYTE *p, int width, int height,
							 int x0, int y0, int x1, int y1, int x2, int y2, BOOL *foundBkg, BYTE colors[][3])
{
	BYTE *px1 = GET_PIXEL(p, x0, y0);
	BYTE *px2 = GET_PIXEL(p, x1, y1);
	BYTE *px3 = GET_PIXEL(p, x2, y2);

	// If any of the corners have transparency, forget about it
	// Not using != 255 because some MSN bmps have 254 in some positions
	if (px1[3] < 253 || px2[3] < 253 || px3[3] < 253)
		return FALSE;

	// See if is the same color
	if (ColorsAreTheSame(colorDiff, px1, px2) && ColorsAreTheSame(colorDiff, px3, px2)) {
		*foundBkg = TRUE;
		memmove(colors, px1, 3);
	}
	else *foundBkg = FALSE;

	return TRUE;
}


DWORD GetImgHash(HBITMAP hBitmap)
{
	BITMAP bmp;
	GetObject(hBitmap, sizeof(bmp), &bmp);

	DWORD dwLen = bmp.bmWidth * bmp.bmHeight * (bmp.bmBitsPixel / 8);
	WORD *p = (WORD *)malloc(dwLen);
	if (p == NULL)
		return 0;
	memset(p, 0, dwLen);

	GetBitmapBits(hBitmap, dwLen, p);

	DWORD ret = 0;
	for (DWORD i = 0; i < dwLen / 2; i++)
		ret += p[i];

	free(p);

	return ret;
}

/*
 * Changes the handle to a grayscale image
 */
HBITMAP MakeGrayscale(MCONTACT hContact, HBITMAP hBitmap)
{
	if (hBitmap) {
		FIBITMAP *dib = fei->FI_CreateDIBFromHBITMAP(hBitmap);
		if (dib) {
			FIBITMAP *dib_new = fei->FI_ConvertToGreyscale(dib);
			fei->FI_Unload(dib);
			if (dib_new) {
				DeleteObject(hBitmap);
				HBITMAP hbm_new = fei->FI_CreateHBITMAPFromDIB(dib_new);
				fei->FI_Unload(dib_new);
				return hbm_new;
			}
		}
	}
	return hBitmap;
}

/*
 * See if finds a transparent background in image, and set its transparency
 * Return TRUE if found a transparent background
 */
BOOL MakeTransparentBkg(MCONTACT hContact, HBITMAP *hBitmap)
{
	int i, j;

	BITMAP bmp;
	GetObject(*hBitmap, sizeof(bmp), &bmp);
	int width = bmp.bmWidth;
	int height = bmp.bmHeight;
	int colorDiff = db_get_w(hContact, "ContactPhoto", "TranspBkgColorDiff", db_get_w(0, AVS_MODULE, "TranspBkgColorDiff", 10));

	// Min 5x5 to easy things in loop
	if (width <= 4 || height <= 4)
		return FALSE;

	DWORD dwLen = width * height * 4;
	BYTE *p = (BYTE *)malloc(dwLen);
	if (p == NULL)
		return FALSE;

	HBITMAP hBmpTmp;
	if (bmp.bmBitsPixel == 32)
		hBmpTmp = *hBitmap;
	else // Convert to 32 bpp
		hBmpTmp = CopyBitmapTo32(*hBitmap);

	GetBitmapBits(hBmpTmp, dwLen, p);

	// **** Get corner colors

	// Top left
	BYTE colors[8][3];
	BOOL foundBkg[8];
	if (!GetColorForPoint(colorDiff, p, width, height, 0, 0, 0, 1, 1, 0, &foundBkg[0], &colors[0])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Top center
	if (!GetColorForPoint(colorDiff, p, width, height, width / 2, 0, width / 2 - 1, 0, width / 2 + 1, 0, &foundBkg[1], &colors[1])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Top Right
	if (!GetColorForPoint(colorDiff, p, width, height,
		width - 1, 0, width - 1, 1, width - 2, 0, &foundBkg[2], &colors[2])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Center left
	if (!GetColorForPoint(colorDiff, p, width, height, 0, height / 2, 0, height / 2 - 1, 0, height / 2 + 1, &foundBkg[3], &colors[3])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Center left
	if (!GetColorForPoint(colorDiff, p, width, height, width - 1, height / 2, width - 1, height / 2 - 1, width - 1, height / 2 + 1, &foundBkg[4], &colors[4])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Bottom left
	if (!GetColorForPoint(colorDiff, p, width, height, 0, height - 1, 0, height - 2, 1, height - 1, &foundBkg[5], &colors[5])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Bottom center
	if (!GetColorForPoint(colorDiff, p, width, height, width / 2, height - 1, width / 2 - 1, height - 1, width / 2 + 1, height - 1, &foundBkg[6], &colors[6])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Bottom Right
	if (!GetColorForPoint(colorDiff, p, width, height, width - 1, height - 1, width - 1, height - 2, width - 2, height - 1, &foundBkg[7], &colors[7])) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// **** X corners have to have the same color

	int count = 0;
	for (i = 0; i < 8; i++)
		if (foundBkg[i])
			count++;

	if (count < db_get_w(hContact, "ContactPhoto", "TranspBkgNumPoints", db_get_w(0, AVS_MODULE, "TranspBkgNumPoints", 5))) {
		if (hBmpTmp != *hBitmap)
			DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Ok, X corners at least have a color, lets compare then
	int maxCount = 0, selectedColor;
	for (i = 0; i < 8; i++) {
		if (foundBkg[i]) {
			count = 0;

			for (j = 0; j < 8; j++) {
				if (foundBkg[j] && ColorsAreTheSame(colorDiff, (BYTE *)&colors[i], (BYTE *)&colors[j]))
					count++;
			}

			if (count > maxCount) {
				maxCount = count;
				selectedColor = i;
			}
		}
	}

	if (maxCount < db_get_w(hContact, "ContactPhoto", "TranspBkgNumPoints",
		db_get_w(0, AVS_MODULE, "TranspBkgNumPoints", 5))) {
		// Not enought corners with the same color
		if (hBmpTmp != *hBitmap) DeleteObject(hBmpTmp);
		free(p);
		return FALSE;
	}

	// Get bkg color as mean of colors
	{
		int bkgColor[3];
		bkgColor[0] = 0;
		bkgColor[1] = 0;
		bkgColor[2] = 0;
		for (i = 0; i < 8; i++) {
			if (foundBkg[i] && ColorsAreTheSame(colorDiff, (BYTE *)&colors[i], (BYTE *)&colors[selectedColor])) {
				bkgColor[0] += colors[i][0];
				bkgColor[1] += colors[i][1];
				bkgColor[2] += colors[i][2];
			}
		}
		bkgColor[0] /= maxCount;
		bkgColor[1] /= maxCount;
		bkgColor[2] /= maxCount;

		colors[selectedColor][0] = bkgColor[0];
		colors[selectedColor][1] = bkgColor[1];
		colors[selectedColor][2] = bkgColor[2];
	}

	// **** Set alpha for the background color, from the borders
	if (hBmpTmp != *hBitmap) {
		DeleteObject(*hBitmap);

		*hBitmap = hBmpTmp;

		GetObject(*hBitmap, sizeof(bmp), &bmp);
		GetBitmapBits(*hBitmap, dwLen, p);
	}

	// Set alpha from borders
	bool transpProportional = (db_get_b(NULL, AVS_MODULE, "MakeTransparencyProportionalToColorDiff", 0) != 0);

	int *stack = (int *)malloc(width * height * 2 * sizeof(int));
	if (stack == NULL) {
		free(p);
		return FALSE;
	}

	// Put four corners
	int topPos = 0;
	AddToStack(stack, &topPos, 0, 0);
	AddToStack(stack, &topPos, width / 2, 0);
	AddToStack(stack, &topPos, width - 1, 0);
	AddToStack(stack, &topPos, 0, height / 2);
	AddToStack(stack, &topPos, width - 1, height / 2);
	AddToStack(stack, &topPos, 0, height - 1);
	AddToStack(stack, &topPos, width / 2, height - 1);
	AddToStack(stack, &topPos, width - 1, height - 1);

	int curPos = 0;
	while (curPos < topPos) {
		// Get pos
		int x = stack[curPos]; curPos++;
		int y = stack[curPos]; curPos++;

		// Get pixel
		BYTE *px1 = GET_PIXEL(p, x, y);

		// It won't change the transparency if one exists
		// (This avoid an endless loop too)
		// Not using == 255 because some MSN bmps have 254 in some positions
		if (px1[3] >= 253) {
			if (ColorsAreTheSame(colorDiff, px1, (BYTE *)&colors[selectedColor])) {
				px1[3] = (transpProportional) ? min(252,
									 (abs(px1[0] - colors[selectedColor][0])
									 + abs(px1[1] - colors[selectedColor][1])
									 + abs(px1[2] - colors[selectedColor][2])) / 3) : 0;
	
				// Add 4 neighbours
				if (x + 1 < width)
					AddToStack(stack, &topPos, x + 1, y);

				if (x - 1 >= 0)
					AddToStack(stack, &topPos, x - 1, y);

				if (y + 1 < height)
					AddToStack(stack, &topPos, x, y + 1);

				if (y - 1 >= 0)
					AddToStack(stack, &topPos, x, y - 1);
			}
		}
	}

	free(stack);

	SetBitmapBits(*hBitmap, dwLen, p);
	free(p);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Other utils

int SaveAvatar(const char *protocol, const TCHAR *tszFileName)
{
	INT_PTR result = CallProtoService(protocol, PS_SETMYAVATART, 0, (LPARAM)tszFileName);
	if (result == CALLSERVICE_NOTFOUND)
		result = CallProtoService(protocol, PS_SETMYAVATAR, 0, (LPARAM)(char*)_T2A(tszFileName));

	return result;
}
