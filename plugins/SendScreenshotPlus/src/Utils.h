/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#ifndef UTILSH
#define UTILSH

#define SPP_USERPANE 1

extern FI_INTERFACE *FIP;

#define ABS(x) ((x)<0?-(x):(x))

typedef struct TEnumDataTemp {
size_t			count;
MONITORINFOEX*	info;
}MONITORS;

extern HWND g_hCapture;
extern HBITMAP g_hBitmap, g_hbmMask;

//---------------------------------------------------------------------------
int				ComboBox_SelectItemData(HWND hwndCtl, int indexStart, LPARAM data);

size_t			MonitorInfoEnum(MONITORINFOEX* & myMonitors, RECT & virtualScreen);
BOOL CALLBACK	MonitorInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

FIBITMAP*		CaptureWindow(HWND hCapture, BOOL bClientArea, BOOL bIndirectCapture);
FIBITMAP*		CaptureMonitor(TCHAR* szDevice);
FIBITMAP*		CaptureScreen(HDC hDC, SIZE size, HWND hCapture=0);
//FIBITMAP*		CaptureDesktop();	/*emulate print screen (not used)*/
TCHAR*			SaveImage(FREE_IMAGE_FORMAT fif, FIBITMAP* dib, TCHAR* pszFilename, TCHAR* pszExt, int flag=0);

TCHAR*			GetFileNameW(const TCHAR* pszPath);
TCHAR*			GetFileExtW (const TCHAR* pszPath);
char*			GetFileNameA(const TCHAR* pszPath);
char*			GetFileExtA (const TCHAR* pszPath);
#ifdef _UNICODE
#	define GetFileName GetFileNameW
#	define GetFileExt GetFileExtW
#else
#	define GetFileName GetFileNameA
#	define GetFileExt GetFileExtA
#endif // _UNICODE

BOOL GetEncoderClsid(wchar_t *wchMimeType, CLSID& clsidEncoder);
//void SavePNG(HBITMAP hBmp, TCHAR* szFilename);
void SaveGIF(HBITMAP hBmp, TCHAR* szFilename);
void SaveTIF(HBITMAP hBmp, TCHAR* szFilename);

//---------------------------------------------------------------------------
/* Old stuff from Borland C++
//void			ShowPopup(char *title, char *text);

*/
#endif
