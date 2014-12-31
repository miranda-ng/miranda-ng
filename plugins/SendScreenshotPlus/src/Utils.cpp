/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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

FIBITMAP* CreateDIBFromDC(HDC hDC,const RECT* rect,HWND hCapture=0);
//---------------------------------------------------------------------------
// capture window as FIBITMAP - caller must FIP->FI_Unload(dib)
FIBITMAP* CaptureWindow(HWND hCapture, BOOL bClientArea, BOOL bIndirectCapture){
	FIBITMAP* dib;
	HWND hForegroundWin;
	RECT rect;//cropping rect

	if(!hCapture || !IsWindow(hCapture))
		return NULL;
	hForegroundWin=GetForegroundWindow();	// old foreground window
	SetForegroundWindow(hCapture);			// force target foreground
	BringWindowToTop(hCapture);				// bring it to top as well
	/// redraw window to prevent runtime artifacts in picture
	UpdateWindow(hCapture);

	HWND hParent=GetAncestor(hCapture,GA_PARENT);
	if(hParent && !IsChild(hParent,hCapture))
		hParent=NULL;
	if(bIndirectCapture){
		intptr_t wastopmost=GetWindowLongPtr(hCapture,GWL_EXSTYLE)&WS_EX_TOPMOST;
		if(!wastopmost)
			SetWindowPos(hCapture,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		if(bClientArea){
			GetClientRect(hCapture,&rect);
			ClientToScreen(hCapture,(POINT*)&rect);
			rect.right+=rect.left; rect.bottom+=rect.top;
		}else
			GetWindowRect(hCapture,&rect);
		dib=CaptureMonitor(NULL,&rect);
		if(!wastopmost)
			SetWindowPos(hCapture,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	}else{
		HDC hDCsrc;
		GetWindowRect(hCapture,&rect);
		if(hParent)
			hDCsrc=GetDC(hCapture);//hCapture is part of a window, capture that
		else
			hDCsrc=GetWindowDC(hCapture);//entire window w/ title bar
		rect.right=ABS(rect.right-rect.left);
		rect.bottom=ABS(rect.bottom-rect.top);
		rect.left=rect.top=0;
		/// capture window and get FIBITMAP
		dib = CreateDIBFromDC(hDCsrc,&rect,hCapture);
		ReleaseDC(hCapture,hDCsrc);
		if(bClientArea){//we could capture directly, but doing so breaks GetWindowRgn() and also includes artifacts...
			GetWindowRect(hCapture,&rect);
			RECT rectCA; GetClientRect(hCapture,&rectCA);
			ClientToScreen(hCapture,(POINT*)&rectCA);
			rectCA.left=ABS(rectCA.left-rect.left);
			rectCA.top=ABS(rectCA.top-rect.top);
			rectCA.right+=rectCA.left; rectCA.bottom+=rectCA.top;
			/// crop the window to ClientArea
			FIBITMAP* dibClient = FIP->FI_Copy(dib,rectCA.left,rectCA.top,rectCA.right,rectCA.bottom);
			FIP->FI_Unload(dib);
			dib = dibClient;
		}
	}
	if(hForegroundWin){//restore previous foreground window
		SetForegroundWindow(hForegroundWin);
		BringWindowToTop(hForegroundWin);
	}
	return dib;
}

FIBITMAP* CaptureMonitor(const TCHAR* szDevice,const RECT* cropRect/*=NULL*/){
	HDC hScrDC;
	RECT rect;
	FIBITMAP* dib;
	/// get screen resolution
	if(!szDevice){
		hScrDC = CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
		rect.left=GetSystemMetrics(SM_XVIRTUALSCREEN);
		rect.top=GetSystemMetrics(SM_YVIRTUALSCREEN);
		rect.right=GetSystemMetrics(SM_CXVIRTUALSCREEN);
		rect.bottom=GetSystemMetrics(SM_CYVIRTUALSCREEN);
	}else{
		hScrDC = CreateDC(szDevice,NULL,NULL,NULL);
		rect.left=rect.top=0;
		rect.right=GetDeviceCaps(hScrDC,HORZRES);
		rect.bottom=GetDeviceCaps(hScrDC,VERTRES);
	}
	if(cropRect){
		if(cropRect->left > rect.left) rect.left=cropRect->left;
		if(cropRect->top > rect.top) rect.top=cropRect->top;
		if(cropRect->right < rect.right) rect.right=cropRect->right;
		if(cropRect->bottom < rect.bottom) rect.bottom=cropRect->bottom;
	}
	dib=CreateDIBFromDC(hScrDC,&rect);
	ReleaseDC(NULL,hScrDC);
	return dib;
}

FIBITMAP* CreateDIBFromDC(HDC hDC,const RECT* rect,HWND hCapture/*=NULL*/){
///HDC GetDC			(NULL)		entire desktp
///HDC GetDC			(HWND hWnd)	client area of the specified window. (may include artifacts)
///HDC GetWindowDC		(HWND hWnd)	entire window.
	FIBITMAP* dib;// return value
	HBITMAP hBitmap;					// handles to device-dependent bitmaps
	HDC hScrDC, hMemDC;					// screen DC and memory DC
	long width=rect->right-rect->left;
	long height=rect->bottom-rect->top;

	// create a DC for the screen and create
	// a memory DC compatible to screen DC
	if(!(hScrDC=hDC)) hScrDC=GetDC(hCapture);
	hMemDC = CreateCompatibleDC(hScrDC);
	// create a bitmap compatible with the screen DC
	hBitmap = CreateCompatibleBitmap(hScrDC,width,height);//width,height
	// select new bitmap into memory DC
	SelectObject(hMemDC, hBitmap);

	if(hCapture && hDC){
		PrintWindow(hCapture,hMemDC,0);
	}else{// bitblt screen DC to memory DC
		BitBlt(hMemDC,0,0,width,height,hScrDC,rect->left,rect->top,CAPTUREBLT|SRCCOPY);
	}
	dib = FIP->FI_CreateDIBFromHBITMAP(hBitmap);

	//alpha channel from window is always wrong and sometimes even for desktop (Win7, no aero)
	//coz GDI do not draw all in alpha mode.
	//we have to create our own new alpha channel.
	bool bFixAlpha=true;
	bool bInvert=false;
	HBRUSH hBr=CreateSolidBrush(RGB(255,255,255));//Create a SolidBrush object for non transparent area
	HBITMAP hMask=CreateBitmap(width,height,1,1,NULL);// Create monochrome (1 bit) B+W mask bitmap.
	HDC hMaskDC=CreateCompatibleDC(0);
	SelectBitmap(hMaskDC,hMask);
	HRGN hRgn=CreateRectRgn(0,0,0,0);
	if(hCapture && GetWindowRgn(hCapture,hRgn)==ERROR){
		if((GetWindowLongPtr(hCapture,GWL_EXSTYLE)&WS_EX_LAYERED)){
			BYTE bAlpha=0;
			COLORREF crKey=0x00000000;
			DWORD dwFlags=0;
			if(GetLayeredWindowAttributes(hCapture,&crKey,&bAlpha,&dwFlags)) {
				/// per window transparency (like fading in a whole window)
				if((dwFlags&LWA_COLORKEY)){
					SetBkColor(hMemDC,crKey);
					BitBlt(hMaskDC,0,0,width,height,hMemDC,rect->left,rect->top,SRCCOPY);
					bInvert=true;
				}else if((dwFlags&LWA_ALPHA)){
					bFixAlpha=false;
				}
			}else{//per-pixel transparency (won't use the WM_PAINT)
				bFixAlpha=false;
			}
		}else{//not layered - fill the window region
			SetRectRgn(hRgn,0,0,width,height);
			FillRgn(hMaskDC,hRgn,hBr);
		}
	}else{
		if(!hCapture) SetRectRgn(hRgn,0,0,width,height);//client area only, no transparency
		FillRgn(hMaskDC,hRgn,hBr);
	}
	DeleteObject(hRgn);
	if(bFixAlpha){
		FIBITMAP* dibMask = FIP->FI_CreateDIBFromHBITMAP(hMask);
		if(bInvert) FIP->FI_Invert(dibMask);
		FIBITMAP* dib8 = FIP->FI_ConvertTo8Bits(dibMask);
		//copy the dib8 alpha mask to dib32 main bitmap
		FIP->FI_SetChannel(dib,dib8,FICC_ALPHA);
		FIP->FI_Unload(dibMask);
		FIP->FI_Unload(dib8);
	}
	DeleteDC(hMaskDC);
	DeleteObject(hMask);
	DeleteObject(hBr);
	//clean up
	DeleteDC(hMemDC);
	DeleteObject(hBitmap);
	if(!hDC) ReleaseDC(NULL, hScrDC);

	#ifdef _DEBUG
	switch (FIP->FI_GetImageType(dib)){
		case FIT_UNKNOWN:
			OutputDebugStringA("FIBITMAP Type: FIT_UNKNOWN\r\n" );
			break;
		case FIT_BITMAP:
			OutputDebugStringA("FIBITMAP Type: FIT_BITMAP\r\n" );
			break;
		case FIT_UINT16:
			OutputDebugStringA("FIBITMAP Type: FIT_UINT16\r\n" );
			break;
		case FIT_INT16:
			OutputDebugStringA("FIBITMAP Type: FIT_INT16\r\n" );
			break;
		case FIT_UINT32:
			OutputDebugStringA("FIBITMAP Type: FIT_UINT32\r\n" );
			break;
		case FIT_INT32:
			OutputDebugStringA("FIBITMAP Type: FIT_INT32\r\n" );
			break;
		case FIT_FLOAT:
			OutputDebugStringA("FIBITMAP Type: FIT_FLOAT\r\n" );
			break;
		case FIT_DOUBLE:
			OutputDebugStringA("FIBITMAP Type: FIT_DOUBLE\r\n" );
			break;
		case FIT_COMPLEX:
			OutputDebugStringA("FIBITMAP Type: FIT_COMPLEX\r\n" );
			break;
		case FIT_RGB16:
			OutputDebugStringA("FIBITMAP Type: FIT_RGB16\r\n" );
			break;
		case FIT_RGBA16:
			OutputDebugStringA("FIBITMAP Type: FIT_RGBA16\r\n" );
			break;
		case FIT_RGBF:
			OutputDebugStringA("FIBITMAP Type: FIT_RGBF\r\n" );
			break;
		case FIT_RGBAF:
			OutputDebugStringA("FIBITMAP Type: FIT_RGBAF\r\n" );
			break;
		default:
			OutputDebugStringA("FIBITMAP Type: non detectable image type (error)\r\n" );
			break;
	}
	BOOL inf = FIP->FI_IsTransparent(dib);
	OutputDebugStringA(inf ? "FIBITMAP Transparent: true\r\n" : "FIBITMAP Transparent: false\r\n");
	#endif
	return dib;
}

TCHAR* SaveImage(FREE_IMAGE_FORMAT fif, FIBITMAP* dib, const TCHAR* pszFilename, const TCHAR* pszExt, int flag) {
	int ret=0;
	TCHAR* pszFile = NULL;
	TCHAR* FileExt = GetFileExt(pszFilename);
	if(!FileExt) {
		if(!pszExt) return NULL;
		mir_tstradd(pszFile, pszFilename);
		mir_tstradd(pszFile, _T("."));
		mir_tstradd(pszFile, pszExt);
	}
	else {
		mir_tstradd(pszFile, pszFilename);
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
TCHAR* GetFileNameW(const TCHAR* pszPath) {
	const TCHAR* slash=_tcsrchr(pszPath,_T('\\'));
	if(!slash) slash=_tcsrchr(pszPath,_T('/'));
	if(slash)
		return mir_t2u(slash+1);
	else
		return mir_t2u(pszPath);
}
TCHAR* GetFileExtW(const TCHAR* pszPath) {
	const TCHAR* slash=_tcsrchr(pszPath,_T('.'));
	if(slash)
		return mir_t2u(slash);
	return NULL;
}

char* GetFileNameA(const TCHAR* pszPath) {
	const TCHAR* slash=_tcsrchr(pszPath,_T('\\'));
	if(!slash) slash=_tcsrchr(pszPath,_T('/'));
	if(slash)
		return mir_t2a(slash+1);
	else
		return mir_t2a(pszPath);
}
char* GetFileExtA(const TCHAR* pszPath) {
	const TCHAR* slash=_tcsrchr(pszPath,_T('.'));
	if(slash)
		return mir_t2a(slash);
	return NULL;
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
void SavePNG(HBITMAP hBmp, TCHAR* szFilename) {
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
}*/

void SaveGIF(HBITMAP hBmp, TCHAR* szFilename) {
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
}

void SaveTIF(HBITMAP hBmp, TCHAR* szFilename) {
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
}
