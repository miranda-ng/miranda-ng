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
	uint32_t options = 0;
	const char *szProto = nullptr;
	CFormattedTextDraw *ftd = nullptr;

	TextObject() {}

	~TextObject()
	{
		delete ftd;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

void MText_InitFormatting0(CFormattedTextDraw *ftd, uint32_t)
{
	// urls
	LRESULT lResult;
	ftd->getTextService()->TxSendMessage(EM_AUTOURLDETECT, TRUE, 0, &lResult);
}

void MText_InitFormatting1(TextObject *text)
{
	// bbcodes
	bbCodeParse(text->ftd);

	// smilies
	HWND hwnd = CreateProxyWindow();
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)text->ftd->getTextService());

	SMADD_RICHEDIT sm = {};
	sm.hwndRichEditControl = hwnd;
	sm.rangeToReplace = nullptr;
	sm.Protocolname = text->szProto;
	sm.flags = SAFLRE_INSERTEMF;
	CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);

	SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// allocate text object (unicode)

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	CMStringA *rtf = (CMStringA *)dwCookie;
	rtf->Append((char *)pbBuff, cb);
	*pcb = cb;
	return 0;
}

MTEXTCONTROL_DLL(TextObject *) MTextCreateW(HANDLE userHandle, const char *szProto, const wchar_t *text)
{
	TextObject *result = new TextObject;
	result->szProto = szProto;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	result->ftd->putTextW((wchar_t *)text);
	MText_InitFormatting1(result);

	/*
	LRESULT res;
	CMStringA buf;
	EDITSTREAM es = { 0 };
	es.dwCookie = (DWORD_PTR)&buf;
	es.pfnCallback = &EditStreamOutCallback;
	result->ftd->getTextService()->TxSendMessage(EM_STREAMOUT, SF_RTF, (LPARAM)&es, &res);

	Netlib_Logf(0, "Rtf created: %s", buf.c_str());*/
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
// allocate text object (advanced)

MTEXTCONTROL_DLL(TextObject *) MTextCreateEx(HANDLE userHandle, void *text, uint32_t flags)
{
	TextObject *result = new TextObject;
	result->options = TextUserGetOptions(userHandle);
	result->ftd = new CFormattedTextDraw();
	InitRichEdit(result->ftd->getTextService());

	MText_InitFormatting0(result->ftd, result->options);
	if (flags & MTEXT_FLG_RTF) {
		result->ftd->putRTFText((MRtfProvider *)text);
	}
	else {
		if (flags & MTEXT_FLG_WCHAR)
			result->ftd->putTextW((wchar_t *)text);
		else
			result->ftd->putTextA((char *)text);
	}
	MText_InitFormatting1(result);
	delete result;

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// measure text object

MTEXTCONTROL_DLL(int) MTextMeasure(HDC dc, SIZE *sz, TextObject *text)
{
	if (!text || !dc)
		return 0;

	long lWidth = sz->cx, lHeight = sz->cy;
	text->ftd->get_NaturalSize(dc, &lWidth, &lHeight);
	sz->cx = lWidth;
	sz->cy = lHeight;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// display text object

MTEXTCONTROL_DLL(int) MTextDisplay(HDC dc, POINT pos, SIZE sz, TextObject *text)
{
	if (!text || !dc)
		return 0;

	COLORREF cl = GetTextColor(dc);

	LRESULT lResult;
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = cl;
	text->ftd->getTextService()->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &lResult);

	SetBkMode(dc, TRANSPARENT);

	long lWidth = sz.cx, lHeight;
	text->ftd->get_NaturalSize(dc, &lWidth, &lHeight);

	RECT rt;
	rt.left = pos.x;
	rt.top = pos.y;
	rt.right = pos.x + lWidth;
	rt.bottom = pos.y + lHeight;
	text->ftd->Draw(dc, &rt);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// set parent window for text object (this is required for mouse handling, etc)

MTEXTCONTROL_DLL(int) MTextSetParent(TextObject *text, HWND hwnd)
{
	if (text) {
		RECT rc;
		GetClientRect(hwnd, &rc);

		text->ftd->setParentWnd(hwnd, rc);

		DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
		SetWindowLong(hwnd, GWL_STYLE, dwStyle | 8);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// send message to an object

MTEXTCONTROL_DLL(int) MTextSendMessage(HWND hwnd, TextObject *text, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!text) return 0;

	LRESULT lResult;
	text->ftd->getTextService()->TxSendMessage(msg, wParam, lParam, &lResult);

	if (hwnd && (msg == WM_MOUSEMOVE)) {
		HDC hdc = GetDC(hwnd);
		text->ftd->getTextService()->OnTxSetCursor(DVASPECT_CONTENT, 0, nullptr, nullptr, hdc, nullptr, nullptr, LOWORD(lParam), HIWORD(lParam));
		ReleaseDC(hwnd, hdc);
	}

	return lResult;
}

/////////////////////////////////////////////////////////////////////////////////////////
// activates text object

MTEXTCONTROL_DLL(int) MTextActivate(TextObject *text, bool bActivate)
{
	if (!text) return 0;

	if (bActivate)
		text->ftd->getTextService()->OnTxInPlaceActivate(0);
	else
		text->ftd->getTextService()->OnTxInPlaceDeactivate();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// destroy text object

MTEXTCONTROL_DLL(int) MTextDestroy(TextObject *text)
{
	delete text;
	return 0;
}
