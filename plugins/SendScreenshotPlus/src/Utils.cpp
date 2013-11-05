/*

Miranda IM: the free IM client for Microsoft* Windows*
Copyright 2000-2009 Miranda ICQ/IM project,

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

#include "global.h"

//---------------------------------------------------------------------------
//Workaround for MS bug ComboBox_SelectItemData
int ComboBox_SelectItemData(HWND hwndCtl, int indexStart, LPARAM data) {
	int i = 0;
	for ( i ; i < ComboBox_GetCount(hwndCtl); i++) {
		if(data == ComboBox_GetItemData(hwndCtl, i)) {
			ComboBox_SetCurSel (hwndCtl,i);
			return i;
		}
	}
	return CB_ERR;
}

//---------------------------------------------------------------------------
// MonitorInfoEnum
size_t MonitorInfoEnum(MONITORINFOEX* & myMonitors, RECT & virtualScreen) {
	MONITORS tmp = {0,0};
	if (EnumDisplayMonitors(NULL, NULL, MonitorInfoEnumProc, (LPARAM)&tmp)){
		myMonitors = tmp.info;
		memset(&virtualScreen, 0, sizeof(virtualScreen));
		for (size_t i = 0; i < tmp.count; ++i) {
			UnionRect(&virtualScreen, &virtualScreen, &tmp.info[i].rcMonitor);
		}
		return tmp.count;
	}
	else {
		if (tmp.info) mir_free(tmp.info);
	}
	return 0;
}

// MonitorInfoEnumProc - CALLBACK for MonitorInfoEnum
BOOL CALLBACK MonitorInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	MONITORS* monitors = (MONITORS*)dwData;
	++monitors->count;
	monitors->info = (MONITORINFOEX*)mir_realloc(monitors->info, sizeof(MONITORINFOEX)*monitors->count);
	monitors->info[monitors->count-1].cbSize = sizeof(MONITORINFOEX);
	if(!GetMonitorInfo(hMonitor, (LPMONITORINFO)(monitors->info + monitors->count-1))) {
		return FALSE;	// stop enumeration if error
	}
	return TRUE;
}

//---------------------------------------------------------------------------
// capture window as FIBITMAP - caller must FIP->FI_Unload(dib)
FIBITMAP* CaptureWindow  (HWND hCapture, BOOL ClientArea) {
	FIBITMAP *dib;
	HWND	hForegroundWin;
	HDC		hScrDC;						// screen DC
	RECT	rect;					// screen RECT
	SIZE	size;						// DIB width and height = window resolution

	if (!hCapture || !IsWindow(hCapture)) return 0;
	hForegroundWin = GetForegroundWindow();	//Saving foreground window
	BringWindowToTop(hCapture);				// This window bring the target window to the top of all others
	SetForegroundWindow(hCapture);			// Make sure the target window is the foreground one
	// redraw window to prevent runtime artefact in picture
	UpdateWindow(hCapture);

	hScrDC = GetWindowDC(hCapture);

	// get window resolution
	GetWindowRect(hCapture, &rect);
	size.cx=ABS(rect.right-rect.left);
	size.cy=ABS(rect.bottom-rect.top);
	//capture window and get FIBITMAP
	dib = CaptureScreen(hScrDC,size,hCapture);
	ReleaseDC(hCapture,hScrDC);
	if(ClientArea) {
		RECT rectCA	= {0};
		POINT pt	= {0};
		GetClientRect (hCapture, &rectCA);
		ClientToScreen(hCapture, &pt);
		//crop the window to ClientArea
		FIBITMAP* dibClient	= FIP->FI_Copy(dib,
					pt.x - rect.left,
					pt.y - rect.top,
					pt.x - rect.left + rectCA.right,
					pt.y - rect.top + rectCA.bottom);
		FIP->FI_Unload(dib);
		dib = dibClient;
	}
	// Restoring foreground window
	if(hForegroundWin){
		SetForegroundWindow(hForegroundWin);
	}
	return dib;
}

FIBITMAP* CaptureMonitor (LPTSTR szDevice) {
	SIZE size;
	HDC hScrDC;
	FIBITMAP *dib = NULL;
	// get screen resolution
	if(!szDevice) {
		hScrDC	= GetDC(NULL);	/*Get full virtualscreen*/
		size.cx	= GetSystemMetrics(SM_CXVIRTUALSCREEN);
		size.cy	= GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}
	else {
		hScrDC = CreateDC(szDevice, NULL, NULL, NULL);
		size.cx	= GetDeviceCaps(hScrDC, HORZRES);
		size.cy	= GetDeviceCaps(hScrDC, VERTRES);
	}
	dib = CaptureScreen (hScrDC, size);
	ReleaseDC(NULL, hScrDC);
	return dib;
}

FIBITMAP* CaptureScreen  (HDC hDC,SIZE size,HWND hCapture){
//HDC GetDC			(NULL)		entire desktp
//HDC GetDC			(HWND hWnd)	client area of the specified window. (may include artifacts)
//HDC GetWindowDC	(HWND hWnd)	entire window.
	FIBITMAP *dib = NULL;
	HBITMAP hBitmap;					// handles to device-dependent bitmaps
	HDC hScrDC, hMemDC;					// screen DC and memory DC

	// create a DC for the screen and create
	// a memory DC compatible to screen DC
	if(!(hScrDC=hDC)) hScrDC=GetDC(hCapture);
	hMemDC = CreateCompatibleDC(hScrDC);
	// create a bitmap compatible with the screen DC
	hBitmap = CreateCompatibleBitmap(hScrDC,size.cx,size.cy);
	// select new bitmap into memory DC
	SelectObject(hMemDC, hBitmap);

	if(hCapture) {
		PrintWindow(hCapture,hMemDC,0);
	}else{// bitblt screen DC to memory DC
		BitBlt(hMemDC,0,0,size.cx,size.cy,hScrDC,0,0,CAPTUREBLT|SRCCOPY);
	}
	dib = FIP->FI_CreateDIBFromHBITMAP(hBitmap);

	//alpha channel from window is always wrong,
	//coz GDI do not draw all in alpha mode.
	//we have to create our own new alpha channel.
	bool bFixAlpha = true;
	bool bInvert = false;

	// Create monochrome (1 bit) B+W mask bitmap.
	HBITMAP hMask = CreateBitmap(size.cx,size.cy, 1, 1, NULL);
	HDC hMaskDC = CreateCompatibleDC(0);
	SelectBitmap(hMaskDC, hMask);

	//Create a SolidBrush object for non transparent area
	HBRUSH hBr = CreateSolidBrush(RGB(255,255,255));

	HRGN hrgn = NULL;
	int regionType;
	if(hCapture) {
		hrgn = CreateRectRgn(0,0,0,0);
		regionType = GetWindowRgn(hCapture, hrgn);
		if (regionType != ERROR) {
			// not layerd - fill the window region
			FillRgn(hMaskDC, hrgn, hBr);
		}
		else {	//layerd window (WS_EX_LAYERED)
			BYTE bAlpha= 0;
			COLORREF crKey=0;			//0x00bbggrr
			DWORD dwFlags=0;
			if(GetLayeredWindowAttributes(hCapture,&crKey,&bAlpha,&dwFlags)) {
				//per window transparency (like fading in a whole window).
				if((dwFlags & LWA_ALPHA) == LWA_ALPHA) {
					//Use bAlpha to determine the opacity of the layered window.
					bFixAlpha = false;
				}
				if((dwFlags & LWA_COLORKEY) == LWA_COLORKEY) {
					//Use crKey as the transparency color.
					SetBkColor(hMemDC, crKey);
					BitBlt(hMaskDC, 0, 0, size.cx, size.cy, hMemDC, 0, 0, SRCCOPY);
					bInvert = true;
					bFixAlpha = true;
				}
			}
			else {
				//per-pixel transparency (won't use the WM_PAINT )
				bFixAlpha = false;
			}
		}
	}
	else {	//fill the desktop region
		hrgn = CreateRectRgn(0,0,size.cx,size.cy);
		FillRgn(hMaskDC, hrgn, hBr);
	}

	if(bFixAlpha) {
		FIBITMAP* dibMask	= FIP->FI_CreateDIBFromHBITMAP(hMask);
		if(bInvert) FIP->FI_Invert(dibMask);
		FIBITMAP* dib8		= FIP->FI_ConvertTo8Bits(dibMask);

		//copy the dib8 alpha mask to dib32 main bitmap
		FIP->FI_SetChannel(dib,dib8,FICC_ALPHA);
		FIP->FI_Unload(dibMask);
		FIP->FI_Unload(dib8);
	}

	//clean up
	DeleteObject(hBr);
	if(hrgn) DeleteObject(hrgn);
	DeleteDC(hMaskDC);
	DeleteObject(hMask);
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	if(!hDC) ReleaseDC(NULL, hScrDC);
	DeleteObject(hBitmap);

	#ifdef _DEBUG
	switch (FIP->FI_GetImageType(dib)){
		case FIT_UNKNOWN:
			OutputDebugStringA("FIBITMAP Typ: FIT_UNKNOWN\r\n" );
			break;
		case FIT_BITMAP:
			OutputDebugStringA("FIBITMAP Typ: FIT_BITMAP\r\n" );
			break;
		case FIT_UINT16:
			OutputDebugStringA("FIBITMAP Typ: FIT_UINT16\r\n" );
			break;
		case FIT_INT16:
			OutputDebugStringA("FIBITMAP Typ: FIT_INT16\r\n" );
			break;
		case FIT_UINT32:
			OutputDebugStringA("FIBITMAP Typ: FIT_UINT32\r\n" );
			break;
		case FIT_INT32:
			OutputDebugStringA("FIBITMAP Typ: FIT_INT32\r\n" );
			break;
		case FIT_FLOAT:
			OutputDebugStringA("FIBITMAP Typ: FIT_FLOAT\r\n" );
			break;
		case FIT_DOUBLE:
			OutputDebugStringA("FIBITMAP Typ: FIT_DOUBLE\r\n" );
			break;
		case FIT_COMPLEX:
			OutputDebugStringA("FIBITMAP Typ: FIT_COMPLEX\r\n" );
			break;
		case FIT_RGB16:
			OutputDebugStringA("FIBITMAP Typ: FIT_RGB16\r\n" );
			break;
		case FIT_RGBA16:
			OutputDebugStringA("FIBITMAP Typ: FIT_RGBA16\r\n" );
			break;
		case FIT_RGBF:
			OutputDebugStringA("FIBITMAP Typ: FIT_RGBF\r\n" );
			break;
		case FIT_RGBAF:
			OutputDebugStringA("FIBITMAP Typ: FIT_RGBAF\r\n" );
			break;
		default:
			OutputDebugStringA("FIBITMAP Typ: non detectable image type (error)\r\n" );
			break;
	}
	BOOL inf = FIP->FI_IsTransparent(dib);
	OutputDebugStringA(inf ? "FIBITMAP Transparent: true\r\n" : "FIBITMAP Transparent: fase\r\n");
	#endif

	return dib;
}
/*
FIBITMAP* CaptureDesktop()  {//emulate print screen
	FIBITMAP *dib = NULL;
	HBITMAP hBitmap;				// handles to device-dependent bitmaps
	BOOL bBitmap = false;
	int i = 0;
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY, 0);
	keybd_event(VK_SNAPSHOT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	do {//Clipboard need time to get bitmap from keybd_event,
		i++;	//we use a counter to get this time.
		bBitmap = IsClipboardFormatAvailable(CF_BITMAP);
		if(i == 500) return (FIBITMAP*)0;		//emergency exit if something go wrong
	} while (!bBitmap);
	#ifdef _DEBUG
		char mess[120] = {0};
		LPSTR pszMess = mess;
		mir_snprintf(pszMess,120,"SS Bitmap counter: %i\r\n",i);
		OutputDebugStringA( pszMess );
	#endif
	//get clipboard data
	OpenClipboard(NULL);
	hBitmap = (HBITMAP)GetClipboardData(CF_BITMAP);

	//create FIBITMAP * from HBITMAP
	FIP->FI_CorrectBitmap32Alpha(hBitmap, FALSE);
	dib = FIP->FI_CreateDIBFromHBITMAP(hBitmap);
	CloseClipboard();

	return dib;
}*/

LPTSTR SaveImage(FREE_IMAGE_FORMAT fif, FIBITMAP* dib, LPTSTR pszFilename, LPTSTR pszExt, int flag) {
	int ret=0;
	LPTSTR pszFile = NULL;
	LPTSTR FileExt = (LPTSTR)GetFileExt (pszFilename, DBVT_TCHAR);
	if(!FileExt) {
		if(!pszExt) return NULL;
		mir_tcsadd(pszFile, pszFilename);
		mir_tcsadd(pszFile, _T("."));
		mir_tcsadd(pszFile, pszExt);
	}
	else {
		mir_tcsadd(pszFile, pszFilename);
	}

	if(fif==FIF_UNKNOWN) {
		fif = FIP->FI_GetFIFFromFilenameU(pszFile);
	}

	ret = FIP->FI_SaveU(fif, dib, pszFile, flag);
	

	mir_free(FileExt);

	if(ret) return pszFile;
	mir_free(pszFile);
	return NULL;
}

//---------------------------------------------------------------------------
//Draws a selection border on the window under cursor
void DrawBorderInverted(HWND hWindow) {
	if (!hWindow){
		return;
	}
	HDC hDC=GetWindowDC(hWindow);
	RECT rect={0};
	GetWindowRect(hWindow, &rect);

	int dcSave = SaveDC(hDC);

	SetROP2(hDC, R2_NOT);

	HPEN hPen=0;
	hPen = CreatePen(PS_SOLID, 10, RGB(0, 0, 0));

	SelectObject(hDC, &hPen);
	SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(hDC, 0, 0, rect.right-rect.left, rect.bottom-rect.top);
	Rectangle(hDC, 1, 1, rect.right-rect.left-1, rect.bottom-rect.top-1);
	Rectangle(hDC, 2, 2, rect.right-rect.left-2, rect.bottom-rect.top-2);

	RestoreDC(hDC, dcSave);
}

//---------------------------------------------------------------------------
//is left mouse button down
BOOL GetLmouse() {
	SHORT temp = GetAsyncKeyState((GetSystemMetrics(SM_SWAPBUTTON)) ? VK_RBUTTON : VK_LBUTTON);
	if ((temp & 0x8000) == 0x8000) {	// LBUTTON down
		return TRUE;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
INT_PTR GetFileName(LPTSTR pszPath, UINT typ) {
	/*DBVT_ASCIIZ, DBVT_WCHAR, DBVT_TCHAR*/
	LPTSTR slash = _tcsrchr(pszPath,_T('\\'));
	if (slash) {
		switch (typ) {
			case DBVT_ASCIIZ:
				return (INT_PTR)mir_t2a(slash+1);
			case DBVT_WCHAR:
				return (INT_PTR)mir_t2u(slash+1);
			default:
				return 0;
		}
	}
	else {
		switch (typ) {
			case DBVT_ASCIIZ:
				return (INT_PTR)mir_t2a(pszPath);
			case DBVT_WCHAR:
				return (INT_PTR)mir_t2u(pszPath);
			default:
				return 0;
		}
	}
}

INT_PTR GetFileExt (LPTSTR pszPath, UINT typ) {
	/*DBVT_ASCIIZ, DBVT_WCHAR, DBVT_TCHAR*/
	LPTSTR slash = _tcsrchr(pszPath,_T('.'));
	if (slash) {
		switch (typ) {
			case DBVT_ASCIIZ:
				return (INT_PTR)mir_t2a(slash);
			case DBVT_WCHAR:
				return (INT_PTR)mir_t2u(slash);
			default:
				return 0;
		}
	}
	else {
		return NULL;
	}
}

//---------------------------------------------------------------------------
BOOL GetEncoderClsid(wchar_t *wchMimeType, CLSID& clsidEncoder) {
	UINT uiNum=0;
	UINT uiSize=0;
	BOOL bOk=FALSE;
	Gdiplus::GetImageEncodersSize(&uiNum,&uiSize);
	if(uiSize>0){
		Gdiplus::ImageCodecInfo* pImageCodecInfo=(Gdiplus::ImageCodecInfo*)mir_alloc(uiSize);
		if(pImageCodecInfo){
			Gdiplus::GetImageEncoders(uiNum,uiSize,pImageCodecInfo);
			for( UINT i=0; i<uiNum; ++i){
				if(!wcscmp(pImageCodecInfo[i].MimeType,wchMimeType)){
					clsidEncoder=pImageCodecInfo[i].Clsid;
					bOk=TRUE;
				}
			}
			mir_free(pImageCodecInfo);
		}
	}
	return bOk;
}
/*
INT_PTR SavePNG(HBITMAP hBmp, LPTSTR szFilename) {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR                    gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE) );
	if( pBitmap ) {
		// Get the CLSID of the PNG encoder.
		CLSID clsidEncoder;
		if( GetEncoderClsid(L"image/png", clsidEncoder)) {
			LPWSTR pswFile = mir_t2u(szFilename);
			pBitmap->Save((const WCHAR*)pswFile, &clsidEncoder, NULL);
			mir_free(pswFile);
		}
		delete pBitmap;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}*/

INT_PTR SaveGIF(HBITMAP hBmp, LPTSTR szFilename) {
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR                    gdiplusToken;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE) );
	if( pBitmap ) {
		// Get the CLSID of the GIF encoder.
		CLSID clsidEncoder;
		if( GetEncoderClsid(L"image/gif", clsidEncoder)) {
			LPWSTR pswFile = mir_t2u(szFilename);
			pBitmap->Save((const WCHAR*)pswFile, &clsidEncoder, NULL);
			mir_free(pswFile);
		}
		delete pBitmap;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}

INT_PTR SaveTIF(HBITMAP hBmp, LPTSTR szFilename) {
//http://www.codeproject.com/Messages/1406708/How-to-reduce-the-size-of-an-Image-using-GDIplus.aspx
	ULONG_PTR						gdiplusToken;
	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	Gdiplus::Status stat;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Gdiplus::Bitmap *pBitmap = Gdiplus::Bitmap::FromHBITMAP(hBmp, (HPALETTE)GetStockObject(DEFAULT_PALETTE) );
	if( pBitmap ) {
		// Get the CLSID of the GIF encoder.
		CLSID EncCLSID;
		if( GetEncoderClsid(L"image/tiff", EncCLSID)) {
			//--- Create a 2-parameter array, for Compression and for Color Bit depth
			Gdiplus::EncoderParameters* EncParams = (Gdiplus::EncoderParameters*) malloc(sizeof(Gdiplus::EncoderParameters) + 1 * sizeof(Gdiplus::EncoderParameter));
		//	Gdiplus::EncoderParameters pEncoderParameters;
			//--- Use LZW Compression instead of Group 4, since it works for color and G4 doesn't
			ULONG ulCompression = Gdiplus::EncoderValueCompressionLZW ;
			ULONG ulColorDepth = 24L ;

			EncParams->Count = 2 ;
			EncParams->Parameter[0].Guid = Gdiplus::EncoderCompression ;
			EncParams->Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong ;
			EncParams->Parameter[0].NumberOfValues = 1 ;
			EncParams->Parameter[0].Value = &ulCompression ;
			EncParams->Parameter[1].Guid = Gdiplus::EncoderColorDepth ;
			EncParams->Parameter[1].Type = Gdiplus::EncoderParameterValueTypeLong ;
			EncParams->Parameter[1].NumberOfValues = 1 ;
			EncParams->Parameter[1].Value = &ulColorDepth ;

			LPWSTR pswFile = mir_t2u(szFilename);
			stat = pBitmap->Save((const WCHAR*)pswFile, &EncCLSID, EncParams);
			mir_free(pswFile);
			free(EncParams);
		}
		delete pBitmap;
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return 0;
}
