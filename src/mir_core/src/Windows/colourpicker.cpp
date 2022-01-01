/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../stdafx.h"

static LRESULT CALLBACK ColourPickerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_CREATE:
		SetWindowLongPtr(hwnd, 0, 0);
		SetWindowLongPtr(hwnd, sizeof(COLORREF), 0);
		break;

	case CPM_SETDEFAULTCOLOUR:
		SetWindowLongPtr(hwnd, sizeof(COLORREF), lParam);
		break;

	case CPM_GETDEFAULTCOLOUR:
		return GetWindowLongPtr(hwnd, sizeof(COLORREF));

	case CPM_SETCOLOUR:
		SetWindowLongPtr(hwnd, 0, lParam);
		InvalidateRect(hwnd, nullptr, FALSE);
		break;

	case CPM_GETCOLOUR:
		return GetWindowLongPtr(hwnd, 0);

	case WM_LBUTTONUP:
		{
			COLORREF custColours[16] = { 0 };
			custColours[0] = GetWindowLongPtr(hwnd, sizeof(COLORREF));

			CHOOSECOLOR cc = { 0 };
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hwnd;
			cc.hInstance = (HWND)g_hInst;
			cc.rgbResult = GetWindowLongPtr(hwnd, 0);
			cc.lpCustColors = custColours;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
			if (ChooseColor(&cc)) {
				SetWindowLongPtr(hwnd, 0, cc.rgbResult);
				SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), CPN_COLOURCHANGED), (LPARAM)hwnd);
				InvalidateRect(hwnd, nullptr, FALSE);
			}
		}
		break;

	case WM_ENABLE:
		InvalidateRect(hwnd, nullptr, FALSE);
		break;

	case WM_NCPAINT:
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc1 = BeginPaint(hwnd, &ps);

		RECT rc;
		GetClientRect(hwnd, &rc);
		DrawEdge(hdc1, &rc, EDGE_ETCHED, BF_RECT);
		InflateRect(&rc, -2, -2);

		HBRUSH hBrush = (IsWindowEnabled(hwnd)) ? CreateSolidBrush(GetWindowLongPtr(hwnd, 0)) : CreateHatchBrush(HS_BDIAGONAL, GetSysColor(COLOR_GRAYTEXT));
		SetBkColor(hdc1, GetSysColor(COLOR_BTNFACE));
		FillRect(hdc1, &rc, hBrush);
		DeleteObject(hBrush);

		EndPaint(hwnd, &ps);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void InitColourPicker(void)
{
	WNDCLASS wcl = { 0 };
	wcl.lpfnWndProc = ColourPickerWndProc;
	wcl.cbWndExtra = sizeof(COLORREF) * 2;
	wcl.hInstance = g_hInst;
	wcl.lpszClassName = _T(WNDCLASS_COLOURPICKER);
	wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wcl.style = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	RegisterClass(&wcl);
}
