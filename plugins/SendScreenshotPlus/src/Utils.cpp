/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

void ComboBox_SelectItem(HWND hCombo, LPARAM data)
{
	for (int i = 0;; i++) {
		LPARAM itemData = ComboBox_GetItemData(hCombo, i);
		if (itemData == data) {
			ComboBox_SetCurSel(hCombo, i);
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MonitorInfoEnum

static BOOL CALLBACK MonitorInfoEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
{
	MONITORS* monitors = (MONITORS*)dwData;
	++monitors->count;
	monitors->info = (MONITORINFOEX*)mir_realloc(monitors->info, sizeof(MONITORINFOEX)*monitors->count);
	monitors->info[monitors->count - 1].cbSize = sizeof(MONITORINFOEX);
	if (!GetMonitorInfo(hMonitor, (LPMONITORINFO)(monitors->info + monitors->count - 1)))
		return FALSE; // stop enumeration if error

	return TRUE;
}

size_t MonitorInfoEnum(MONITORINFOEX* &myMonitors, RECT &virtualScreen)
{
	MONITORS tmp = { 0, nullptr };
	if (EnumDisplayMonitors(nullptr, nullptr, MonitorInfoEnumProc, (LPARAM)&tmp)) {
		myMonitors = tmp.info;
		memset(&virtualScreen, 0, sizeof(virtualScreen));
		for (size_t i = 0; i < tmp.count; ++i) {
			UnionRect(&virtualScreen, &virtualScreen, &tmp.info[i].rcMonitor);
		}
		return tmp.count;
	}
	
	mir_free(tmp.info);
	return 0;
}

FIBITMAP* CreateDIBFromDC(HDC hDC, const RECT* rect, HWND hCapture = nullptr);

/////////////////////////////////////////////////////////////////////////////////////////
// capture window as FIBITMAP - caller must FreeImage_Unload(dib)

FIBITMAP* CaptureWindow(HWND hCapture, BOOL bClientArea, BOOL bIndirectCapture)
{
	FIBITMAP* dib;
	HWND hForegroundWin;
	RECT rect; // cropping rect

	if (!hCapture || !IsWindow(hCapture))
		return nullptr;
	
	hForegroundWin = GetForegroundWindow(); // old foreground window
	SetForegroundWindow(hCapture); // force target foreground
	BringWindowToTop(hCapture); // bring it to top as well
	
	// redraw window to prevent runtime artifacts in picture
	UpdateWindow(hCapture);

	HWND hParent = GetAncestor(hCapture, GA_PARENT);
	if (hParent && !IsChild(hParent, hCapture))
		hParent = nullptr;
	if (bIndirectCapture) {
		intptr_t wastopmost = GetWindowLongPtr(hCapture, GWL_EXSTYLE)&WS_EX_TOPMOST;
		if (!wastopmost)
			SetWindowPos(hCapture, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		if (bClientArea) {
			GetClientRect(hCapture, &rect);
			ClientToScreen(hCapture, (POINT*)&rect);
			rect.right += rect.left; rect.bottom += rect.top;
		}
		else
			GetWindowRect(hCapture, &rect);
		dib = CaptureMonitor(nullptr, &rect);
		if (!wastopmost)
			SetWindowPos(hCapture, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else {
		HDC hDCsrc;
		GetWindowRect(hCapture, &rect);
		if (hParent)
			hDCsrc = GetDC(hCapture); // hCapture is part of a window, capture that
		else
			hDCsrc = GetWindowDC(hCapture); // entire window w/ title bar
		rect.right = ABS(rect.right - rect.left);
		rect.bottom = ABS(rect.bottom - rect.top);
		rect.left = rect.top = 0;
		// capture window and get FIBITMAP
		dib = CreateDIBFromDC(hDCsrc, &rect, hCapture);
		ReleaseDC(hCapture, hDCsrc);

		// we could capture directly, but doing so breaks GetWindowRgn() and also includes artifacts...
		if (bClientArea) {
			GetWindowRect(hCapture, &rect);
			RECT rectCA; GetClientRect(hCapture, &rectCA);
			ClientToScreen(hCapture, (POINT*)&rectCA);
			rectCA.left = ABS(rectCA.left - rect.left);
			rectCA.top = ABS(rectCA.top - rect.top);
			rectCA.right += rectCA.left; rectCA.bottom += rectCA.top;
			
			// crop the window to ClientArea
			FIBITMAP* dibClient = FreeImage_Copy(dib, rectCA.left, rectCA.top, rectCA.right, rectCA.bottom);
			FreeImage_Unload(dib);
			dib = dibClient;
		}
	}
	
	// restore previous foreground window
	if (hForegroundWin) {
		SetForegroundWindow(hForegroundWin);
		BringWindowToTop(hForegroundWin);
	}
	return dib;
}

FIBITMAP* CaptureMonitor(const wchar_t* szDevice, const RECT* cropRect/*=NULL*/)
{
	HDC hScrDC;
	RECT rect;
	
	// get screen resolution
	if (!szDevice) {
		hScrDC = CreateDC(L"DISPLAY", nullptr, nullptr, nullptr);
		rect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
		rect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
		rect.right = GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN);
		rect.bottom = GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	else {
		hScrDC = CreateDC(szDevice, nullptr, nullptr, nullptr);
		rect.left = rect.top = 0;
		rect.right = GetDeviceCaps(hScrDC, HORZRES);
		rect.bottom = GetDeviceCaps(hScrDC, VERTRES);
	}
	if (cropRect) {
		if (cropRect->left > rect.left) rect.left = cropRect->left;
		if (cropRect->top > rect.top) rect.top = cropRect->top;
		if (cropRect->right < rect.right) rect.right = cropRect->right;
		if (cropRect->bottom < rect.bottom) rect.bottom = cropRect->bottom;
	}
	
	FIBITMAP *dib = CreateDIBFromDC(hScrDC, &rect);
	ReleaseDC(nullptr, hScrDC);
	return dib;
}

FIBITMAP* CreateDIBFromDC(HDC hDC, const RECT* rect, HWND hCapture/*=NULL*/)
{
	long width = rect->right - rect->left;
	long height = rect->bottom - rect->top;

	// create a DC for the screen and create
	// a memory DC compatible to screen DC
	HDC hScrDC = hDC;
	if (!hScrDC)
		hScrDC = GetDC(hCapture);
	HDC hMemDC = CreateCompatibleDC(hScrDC);
	// create a bitmap compatible with the screen DC
	HBITMAP hBitmap = CreateCompatibleBitmap(hScrDC, width, height);
	// select new bitmap into memory DC
	SelectObject(hMemDC, hBitmap);

	if (hCapture && hDC)
		PrintWindow(hCapture, hMemDC, 0);
	else // bitblt screen DC to memory DC
		BitBlt(hMemDC, 0, 0, width, height, hScrDC, rect->left, rect->top, CAPTUREBLT | SRCCOPY);

	FIBITMAP *dib = FreeImage_CreateDIBFromHBITMAP(hBitmap);

	// alpha channel from window is always wrong and sometimes even for desktop (Win7, no aero)
	// coz GDI do not draw all in alpha mode.
	// we have to create our own new alpha channel.
	bool bFixAlpha = true;
	bool bInvert = false;
	HBRUSH hBr = CreateSolidBrush(RGB(255, 255, 255)); // Create a SolidBrush object for non transparent area
	HBITMAP hMask = CreateBitmap(width, height, 1, 1, nullptr); // Create monochrome (1 bit) B+W mask bitmap.
	HDC hMaskDC = CreateCompatibleDC(nullptr);
	SelectBitmap(hMaskDC, hMask);
	HRGN hRgn = CreateRectRgn(0, 0, 0, 0);
	if (hCapture && GetWindowRgn(hCapture, hRgn) == ERROR) {
		if ((GetWindowLongPtr(hCapture, GWL_EXSTYLE)&WS_EX_LAYERED)) {
			uint8_t bAlpha = 0;
			COLORREF crKey = 0x00000000;
			DWORD dwFlags = 0;
			if (GetLayeredWindowAttributes(hCapture, &crKey, &bAlpha, &dwFlags)) {
				// per window transparency (like fading in a whole window)
				if ((dwFlags&LWA_COLORKEY)) {
					SetBkColor(hMemDC, crKey);
					BitBlt(hMaskDC, 0, 0, width, height, hMemDC, rect->left, rect->top, SRCCOPY);
					bInvert = true;
				}
				else if ((dwFlags&LWA_ALPHA)) {
					bFixAlpha = false;
				}
			}
			else { // per-pixel transparency (won't use the WM_PAINT)
				bFixAlpha = false;
			}
		}
		else { // not layered - fill the window region
			SetRectRgn(hRgn, 0, 0, width, height);
			FillRgn(hMaskDC, hRgn, hBr);
		}
	}
	else {
		if (!hCapture) SetRectRgn(hRgn, 0, 0, width, height); // client area only, no transparency
		FillRgn(hMaskDC, hRgn, hBr);
	}
	DeleteObject(hRgn);
	if (bFixAlpha) {
		FIBITMAP* dibMask = FreeImage_CreateDIBFromHBITMAP(hMask);
		if (bInvert) FreeImage_Invert(dibMask);
		FIBITMAP* dib8 = FreeImage_ConvertTo8Bits(dibMask);
		// copy the dib8 alpha mask to dib32 main bitmap
		FreeImage_SetChannel(dib, dib8, FICC_ALPHA);
		FreeImage_Unload(dibMask);
		FreeImage_Unload(dib8);
	}
	DeleteDC(hMaskDC);
	DeleteObject(hMask);
	DeleteObject(hBr);
	
	// clean up
	DeleteDC(hMemDC);
	DeleteObject(hBitmap);
	if (!hDC)
		ReleaseDC(nullptr, hScrDC);

#ifdef _DEBUG
	switch (FreeImage_GetImageType(dib)) {
	case FIT_UNKNOWN:
		OutputDebugStringA("FIBITMAP Type: FIT_UNKNOWN\r\n");
		break;
	case FIT_BITMAP:
		OutputDebugStringA("FIBITMAP Type: FIT_BITMAP\r\n");
		break;
	case FIT_UINT16:
		OutputDebugStringA("FIBITMAP Type: FIT_UINT16\r\n");
		break;
	case FIT_INT16:
		OutputDebugStringA("FIBITMAP Type: FIT_INT16\r\n");
		break;
	case FIT_UINT32:
		OutputDebugStringA("FIBITMAP Type: FIT_UINT32\r\n");
		break;
	case FIT_INT32:
		OutputDebugStringA("FIBITMAP Type: FIT_INT32\r\n");
		break;
	case FIT_FLOAT:
		OutputDebugStringA("FIBITMAP Type: FIT_FLOAT\r\n");
		break;
	case FIT_DOUBLE:
		OutputDebugStringA("FIBITMAP Type: FIT_DOUBLE\r\n");
		break;
	case FIT_COMPLEX:
		OutputDebugStringA("FIBITMAP Type: FIT_COMPLEX\r\n");
		break;
	case FIT_RGB16:
		OutputDebugStringA("FIBITMAP Type: FIT_RGB16\r\n");
		break;
	case FIT_RGBA16:
		OutputDebugStringA("FIBITMAP Type: FIT_RGBA16\r\n");
		break;
	case FIT_RGBF:
		OutputDebugStringA("FIBITMAP Type: FIT_RGBF\r\n");
		break;
	case FIT_RGBAF:
		OutputDebugStringA("FIBITMAP Type: FIT_RGBAF\r\n");
		break;
	default:
		OutputDebugStringA("FIBITMAP Type: non detectable image type (error)\r\n");
		break;
	}
	BOOL inf = FreeImage_IsTransparent(dib);
	OutputDebugStringA(inf ? "FIBITMAP Transparent: true\r\n" : "FIBITMAP Transparent: false\r\n");
#endif
	return dib;
}

/////////////////////////////////////////////////////////////////////////////////////////

char* GetFileNameA(const wchar_t* pszPath)
{
	const wchar_t* slash = wcsrchr(pszPath, '\\');
	if (!slash) slash = wcsrchr(pszPath, '/');
	if (slash)
		return mir_u2a(slash + 1);
	else
		return mir_u2a(pszPath);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL GetEncoderClsid(wchar_t *wchMimeType, CLSID &clsidEncoder)
{
	UINT uiNum = 0;
	UINT uiSize = 0;
	BOOL bOk = FALSE;
	Gdiplus::GetImageEncodersSize(&uiNum, &uiSize);
	if (uiSize > 0) {
		Gdiplus::ImageCodecInfo* pImageCodecInfo = (Gdiplus::ImageCodecInfo*)mir_alloc(uiSize);
		if (pImageCodecInfo) {
			Gdiplus::GetImageEncoders(uiNum, uiSize, pImageCodecInfo);
			for (UINT i = 0; i < uiNum; ++i) {
				if (!mir_wstrcmp(pImageCodecInfo[i].MimeType, wchMimeType)) {
					clsidEncoder = pImageCodecInfo[i].Clsid;
					bOk = TRUE;
				}
			}
			mir_free(pImageCodecInfo);
		}
	}
	return bOk;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SaveGIF(HBITMAP hBmp, const wchar_t *szFilename)
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR                    gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE));
	if (pBitmap) {
		// Get the CLSID of the GIF encoder.
		CLSID clsidEncoder;
		if (GetEncoderClsid(L"image/gif", clsidEncoder)) {
			LPWSTR pswFile = mir_wstrdup(szFilename);
			pBitmap->Save((const wchar_t*)pswFile, &clsidEncoder, nullptr);
			mir_free(pswFile);
		}
		delete pBitmap;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
}

/////////////////////////////////////////////////////////////////////////////////////////

void SaveTIF(HBITMAP hBmp, const wchar_t *szFilename)
{
	// http://www.codeproject.com/Messages/1406708/How-to-reduce-the-size-of-an-Image-using-GDIplus.aspx
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	Gdiplus::Status stat;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE));
	if (pBitmap) {
		// Get the CLSID of the GIF encoder.
		CLSID EncCLSID;
		if (GetEncoderClsid(L"image/tiff", EncCLSID)) {
			//--- Create a 2-parameter array, for Compression and for Color Bit depth
			Gdiplus::EncoderParameters* EncParams = (Gdiplus::EncoderParameters*) malloc(sizeof(Gdiplus::EncoderParameters) + 1 * sizeof(Gdiplus::EncoderParameter));
			//	Gdiplus::EncoderParameters pEncoderParameters;
			//--- Use LZW Compression instead of Group 4, since it works for color and G4 doesn't
			ULONG ulCompression = Gdiplus::EncoderValueCompressionLZW;
			ULONG ulColorDepth = 24L;

			EncParams->Count = 2;
			EncParams->Parameter[0].Guid = Gdiplus::EncoderCompression;
			EncParams->Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
			EncParams->Parameter[0].NumberOfValues = 1;
			EncParams->Parameter[0].Value = &ulCompression;
			EncParams->Parameter[1].Guid = Gdiplus::EncoderColorDepth;
			EncParams->Parameter[1].Type = Gdiplus::EncoderParameterValueTypeLong;
			EncParams->Parameter[1].NumberOfValues = 1;
			EncParams->Parameter[1].Value = &ulColorDepth;

			LPWSTR pswFile = mir_wstrdup(szFilename);
			stat = pBitmap->Save((const wchar_t*)pswFile, &EncCLSID, EncParams);
			mir_free(pswFile);
			free(EncParams);
		}
		delete pBitmap;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
}
