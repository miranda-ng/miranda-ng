#include "stdafx.h"
#include "shlicons.h"

#pragma comment(lib, "windowscodecs.lib")
/*
The following implementation has been ported from:

http://web.archive.org/web/20080121112802/http://shellrevealed.com/blogs/shellblog/archive/2007/02/06/Vista-Style-Menus_2C00_-Part-1-_2D00_-Adding-icons-to-standard-menus.aspx

It uses WIC (Windows Imaging Codec) to convert the given Icon into a bitmap in ARGB format, this is required
by Windows for use as an icon (but in bitmap format), so that Windows draws everything (including theme)
so we don't have to.

Why didn't they just do this themselves? ...
*/

/*
  The object returned from this function has to be released using the QI COM interface, don't forget.
  Note this function won't work on anything where WIC isn't installed (XP can have it installed, but not by default)
  anything less won't work.
*/

IWICImagingFactory* ARGB_GetWorker()
{
	IWICImagingFactory *res = NULL;
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (void**)&res);
	return res;
}

HBITMAP ARGB_BitmapFromIcon(IWICImagingFactory *Factory, HDC hDC, HICON hIcon)
{
	HBITMAP hBmp = NULL;

	// This code gives an icon to WIC and gets a bitmap object in return, it then creates a DIB section
	// which is 32bits and the same H*W as the icon. It then asks the bitmap object to copy itself into the DIB }

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;

	IWICBitmap *bitmap = NULL;
	HRESULT hr = Factory->CreateBitmapFromHICON(hIcon, &bitmap);
	if (hr == S_OK) {
		int cx, cy;
		hr = bitmap->GetSize((PUINT)&cx, (PUINT)&cy);
		if (hr == S_OK) {
			bmi.bmiHeader.biWidth = cx;
			bmi.bmiHeader.biHeight = -cy;

			void *pbBuffer;
			hBmp = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &pbBuffer, 0, 0);
			if (hBmp != 0) {
				UINT cbStride = cx * sizeof(DWORD); // ARGB = DWORD
				UINT cbBuffer = cy * cbStride;
				// note: the pbBuffer memory is owned by the DIB and will be freed when the bitmap is released
				hr = bitmap->CopyPixels(NULL, cbStride, cbBuffer, (PBYTE)pbBuffer);
				if (hr != S_OK) {
					// the copy failed, delete the DIB
					DeleteObject(hBmp);
					hBmp = NULL;
				}
			}
		}
		// release the bitmap object now
		bitmap->Release();
	}

	return hBmp;
}
