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

#include "stdafx.h"

LRESULT CALLBACK MTextControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT MTextControl_OnPaint(HWND hwnd, WPARAM wParam, LPARAM lParam);
//LRESULT MTextControl_Measure(HWND hwnd, int maxw, SIZE *size);

struct TextControlData
{
	HANDLE htu;
	wchar_t *text;
	struct TextObject *mtext;
};

void MTextControl_RegisterClass()
{
	WNDCLASSEX wcl = {};
	wcl.cbSize = sizeof(wcl);
	wcl.lpfnWndProc = MTextControlWndProc;
	wcl.style = CS_GLOBALCLASS;
	wcl.hInstance = g_hInst;
	wcl.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcl.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.lpszClassName = L"MTextControl";
	RegisterClassExW(&wcl);
}

void MTextControl_UnregisterClass()
{
	UnregisterClassW(L"MTextControl", g_hInst);
}

LRESULT CALLBACK MTextControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TextControlData *data = (TextControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
	case WM_CREATE:
		data = new TextControlData;
		data->text = nullptr;
		data->mtext = nullptr;
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
		if (data->mtext) MTextDestroy(data->mtext);
		{
			int textLength = GetWindowTextLength(hwnd);
			data->text = new wchar_t[textLength + 1];
			GetWindowText(hwnd, data->text, textLength + 1);
			data->mtext = MTextCreateW(data->htu, 0, data->text);

			MTextSetParent(data->mtext, hwnd);

			InvalidateRect(hwnd, nullptr, TRUE);
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
			return MTextSendMessage(hwnd, data->mtext, msg, wParam, lParam);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/// Paint ////////////////////////////////////
LRESULT MTextControl_OnPaint(HWND hwnd, WPARAM, LPARAM)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		FrameRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);

	// Find the text to draw
	TextControlData *data = (TextControlData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (data->mtext) {
		HFONT hfntSave = nullptr;
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
		MTextDisplay(hdc, pos, sz, data->mtext);

		if (hfntSave)
			SelectObject(hdc, hfntSave);
	}

	// Release the device context
	EndPaint(hwnd, &ps);
	return 0;
}
