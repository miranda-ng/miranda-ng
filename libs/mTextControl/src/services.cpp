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
#include "FormattedTextDraw.h"

struct TextObject
{
	uint32_t options;
	const char *szProto;
	IFormattedTextDraw *ftd;
	TextObject() : options(0), ftd(nullptr) {}
	~TextObject() { if (ftd) delete ftd; }
};

/////////////////////////////////////////////////////////////////////////////////////////
// elper functions

void MText_InitFormatting0(IFormattedTextDraw *ftd, uint32_t)
{
	LRESULT lResult;

	// urls
	ftd->getTextService()->TxSendMessage(EM_AUTOURLDETECT, TRUE, 0, &lResult);
}

void MText_InitFormatting1(TextObject *text)
{
	// bbcodes
	bbCodeParse(text->ftd);

	// smilies
	HWND hwnd = CreateProxyWindow(text->ftd->getTextService());

	SMADD_RICHEDIT3 sm = {};
	sm.cbSize = sizeof(sm);
	sm.hwndRichEditControl = hwnd;
	sm.rangeToReplace = nullptr;
	sm.Protocolname = text->szProto;
	sm.flags = SAFLRE_INSERTEMF;
	CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);

	DestroyWindow(hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// allocate text object (unicode)

MTEXTCONTROL_DLL(HANDLE) MTextCreateW(HANDLE userHandle, const char *szProto, const wchar_t *text)
{
	TextObject *result = new TextObject;
	result->szProto = szProto;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw;
	result->ftd->Create();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	result->ftd->putTextW((wchar_t*)text);
	MText_InitFormatting1(result);

	return (HANDLE)result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// allocate text object (advanced)

MTEXTCONTROL_DLL(HANDLE) MTextCreateEx(HANDLE userHandle, void *text, uint32_t flags)
{
	TextObject *result = new TextObject;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw;
	result->ftd->Create();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	if (flags & MTEXT_FLG_WCHAR)
		result->ftd->putTextW((wchar_t *)text);
	else
		result->ftd->putTextA((char *)text);
	MText_InitFormatting1(result);
	delete result;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// measure text object

MTEXTCONTROL_DLL(int) MTextMeasure(HDC dc, SIZE *sz, HANDLE text)
{
	if (!text) return 0;

	long lWidth = sz->cx, lHeight = sz->cy;
	((TextObject *)text)->ftd->get_NaturalSize(dc, &lWidth, &lHeight);
	sz->cx = lWidth;
	sz->cy = lHeight;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// display text object

MTEXTCONTROL_DLL(int) MTextDisplay(HDC dc, POINT pos, SIZE sz, HANDLE text)
{
	if (!text) return 0;

	COLORREF cl = GetTextColor(dc);

	LRESULT lResult;
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = cl;
	((TextObject *)text)->ftd->getTextService()->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);

	SetBkMode(dc, TRANSPARENT);

	long lWidth = sz.cx, lHeight;
	((TextObject *)text)->ftd->get_NaturalSize(dc, &lWidth, &lHeight);

	RECT rt;
	rt.left = pos.x;
	rt.top = pos.y;
	rt.right = pos.x + lWidth;
	rt.bottom = pos.y + lHeight;
	((TextObject *)text)->ftd->Draw(dc, &rt);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// set parent window for text object (this is required for mouse handling, etc)

MTEXTCONTROL_DLL(int) MTextSetParent(HANDLE text, HWND hwnd)
{
	if (text) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		((TextObject *)text)->ftd->setParentWnd(hwnd, rc);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// send message to an object

MTEXTCONTROL_DLL(int) MTextSendMessage(HWND hwnd, HANDLE text, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!text)
		return 0;

	LRESULT lResult;
	((TextObject *)text)->ftd->getTextService()->TxSendMessage(msg, wParam, lParam, &lResult);

	if (hwnd && (msg == WM_MOUSEMOVE)) {
		HDC hdc = GetDC(hwnd);
		((TextObject *)text)->ftd->getTextService()->OnTxSetCursor(DVASPECT_CONTENT, 0, nullptr, nullptr, hdc, nullptr, nullptr, LOWORD(lParam), HIWORD(lParam));
		ReleaseDC(hwnd, hdc);
	}

	return lResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// destroy text object

MTEXTCONTROL_DLL(int) MTextDestroy(HANDLE text)
{
	delete (TextObject *)text;
	return 0;
}
