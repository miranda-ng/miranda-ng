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

#include "headers.h"

LRESULT CALLBACK MTextControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT MTextControl_OnPaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
//LRESULT MTextControl_Measure(HWND hwnd, int maxw, SIZE *size);

struct TextControlData
{
	HANDLE htu;
	TCHAR *text;
	HANDLE mtext;
};

void MTextControl_RegisterClass()
{
	WNDCLASSEX wcl;
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = MTextControlWndProc;
	wcl.style = CS_GLOBALCLASS;
	wcl.cbClsExtra = 0;
	wcl.cbWndExtra = 0;
	wcl.hInstance = hInst;
	wcl.hIcon = NULL;
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszMenuName = NULL;
	wcl.lpszClassName = _T(MODULNAME);
	wcl.hIconSm = 0;
	RegisterClassEx(&wcl);
}

LRESULT CALLBACK MTextControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TextControlData *data = (TextControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
	case WM_CREATE:
		data = new TextControlData;
		data->text = 0;
		data->mtext = 0;
		data->htu = htuDefault;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
		PostMessage(hwnd, MTM_UPDATE, 0, 0);
		return 0;

	case MTM_SETUSER:
		data->htu = wParam ? (HANDLE)wParam : htuDefault;
		// falldown, DefWindowProc won't process WM_USER ;)

	case WM_SETTEXT:
		DefWindowProc(hwnd, msg, wParam, lParam);
		// falldown

	case MTM_UPDATE:
		if (data->text) delete[] data->text;
		if (data->mtext) MTI_MTextDestroy(data->mtext);
		{
			int textLength = GetWindowTextLength(hwnd);
			data->text = new TCHAR[textLength + 1];
			GetWindowText(hwnd, data->text, textLength + 1);
			data->mtext = MTI_MTextCreateW(data->htu, data->text);

			RECT rc; GetClientRect(hwnd, &rc);
			MTI_MTextSetParent(data->mtext, hwnd, rc);

			InvalidateRect(hwnd, 0, TRUE);
		}
		return TRUE;

	case WM_PAINT:
		return MTextControl_OnPaint(hwnd, wParam, lParam);

	case WM_ERASEBKGND:
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect((HDC)wParam, &rc, GetSysColorBrush(COLOR_BTNFACE));
		return TRUE;

	case WM_MOUSEMOVE:
		if (data && data->mtext)
			return MTI_MTextSendMessage(hwnd, data->mtext, msg, wParam, lParam);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/// Paint ////////////////////////////////////
LRESULT MTextControl_OnPaint(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);

	RECT rc;
	GetClientRect(hwnd, &rc);
	FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);

	// Find the text to draw
	TextControlData *data = (TextControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (data->mtext) {
		HFONT hfntSave = 0;
		HFONT hfnt = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		if (!hfnt)
			hfnt = (HFONT)SendMessage(GetParent(hwnd), WM_GETFONT, 0, 0);
		if (hfnt) {
			LOGFONT lf;
			GetObject(hfnt, sizeof(lf), &lf);
			hfntSave = (HFONT)SelectObject(hdc, hfnt);
		}

		// Draw the text
		RECT rc;
		GetClientRect(hwnd, &rc);
		POINT pos;
		pos.x = 0;
		pos.y = 2;
		SIZE sz;
		sz.cx = rc.right - rc.left;
		sz.cy = rc.bottom - rc.top - 4;
		MTI_MTextDisplay(hdc, pos, sz, data->mtext);

		if (hfntSave)
			SelectObject(hdc, hfntSave);
	}

	// Release the device context
	EndPaint(hwnd, &ps);

	return 0;
}
