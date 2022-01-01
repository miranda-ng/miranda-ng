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

#ifndef UTILSH
#define UTILSH

#define SPP_USERPANE 1

#define ABS(x) ((x)<0?-(x):(x))

struct MONITORS
{
	size_t count;
	MONITORINFOEX*	info;
};

extern HWND g_hCapture;
extern HBITMAP g_hBitmap, g_hbmMask;

/////////////////////////////////////////////////////////////////////////////////////////

size_t MonitorInfoEnum(MONITORINFOEX* &myMonitors, RECT &virtualScreen);

FIBITMAP* CaptureWindow(HWND hCapture, BOOL bClientArea, BOOL bIndirectCapture);
FIBITMAP* CaptureMonitor(const wchar_t *pwszDevice, const RECT *cropRect = nullptr);

char* GetFileNameA(const wchar_t *pwszPath);

BOOL GetEncoderClsid(wchar_t *wchMimeType, CLSID &clsidEncoder);

void SaveGIF(HBITMAP hBmp, const wchar_t *pwszFilename);
void SaveTIF(HBITMAP hBmp, const wchar_t *pwszFilename);

/////////////////////////////////////////////////////////////////////////////////////////

class EventHandle
{
	HANDLE _hEvent;
public:
	inline EventHandle() { _hEvent = CreateEvent(nullptr, 0, 0, nullptr); }
	inline ~EventHandle() { CloseHandle(_hEvent); }
	inline void Set() { SetEvent(_hEvent); }
	inline void Wait() { WaitForSingleObject(_hEvent, INFINITE); }
	inline void Wait(uint32_t dwMilliseconds) { WaitForSingleObject(_hEvent, dwMilliseconds); }
	inline operator HANDLE() { return _hEvent; }
};

#endif
