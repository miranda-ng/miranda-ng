/*
Miranda Text Control - Plugin for Miranda IM
Copyright (C) 2005 Victor Pavlychko (nullbie@gmail.com)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __services_h__
#define __services_h__

void LoadServices();

__declspec(dllexport) HANDLE	DLL_CALLCONV	MTI_MTextCreateW		(HANDLE userHandle, WCHAR *text);
__declspec(dllexport) HANDLE	DLL_CALLCONV	MTI_MTextCreateEx		(HANDLE userHandle, MCONTACT hContact, void *text, DWORD flags);
__declspec(dllexport) int		DLL_CALLCONV	MTI_MTextMeasure		(HDC dc, SIZE *sz, HANDLE text);
__declspec(dllexport) int		DLL_CALLCONV	MTI_MTextDisplay		(HDC dc, POINT pos, SIZE sz, HANDLE text);
__declspec(dllexport) int		DLL_CALLCONV	MTI_MTextSetParent		(HANDLE text, HWND hwnd, RECT rect);
__declspec(dllexport) int		DLL_CALLCONV	MTI_MTextSendMessage	(HWND hwnd, HANDLE text, UINT msg, WPARAM wParam, LPARAM lParam);
__declspec(dllexport) HWND		DLL_CALLCONV	MTI_MTextCreateProxy	(HANDLE text);
__declspec(dllexport) int		DLL_CALLCONV	MTI_MTextDestroy		(HANDLE text);

#endif // __services_h__
