/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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

#include "stdafx.h"

struct CAPTCHA_FORM_PARAMS
{
	LPCTSTR from;
	LPCTSTR challenge;
	LPCTSTR fromjid;
	LPCTSTR sid;
	LPCTSTR to;
	LPCTSTR hint;
	HBITMAP bmp;
	int w,h;
	wchar_t Result[MAX_PATH];
};

INT_PTR CALLBACK JabberCaptchaFormDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAPTCHA_FORM_PARAMS *params = (CAPTCHA_FORM_PARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG: {
		TranslateDialogDefault(hwndDlg);
		Window_SetSkinIcon_IcoLib(hwndDlg, IDI_KEYS);
		params = (CAPTCHA_FORM_PARAMS*)lParam;

		LPCTSTR hint = params->hint;
		if (hint == nullptr)
			hint = TranslateT("Enter the text you see");
		SetDlgItemText(hwndDlg, IDC_INSTRUCTION, TranslateW(hint));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)params);

		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
		switch(GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT:
		case IDC_INSTRUCTION:
		case IDC_TITLE:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
		return 0;

	case WM_PAINT:
		if (params) {
			PAINTSTRUCT ps;
			HDC hdc, hdcMem;
			RECT rc;

			GetClientRect(hwndDlg, &rc);
			hdc = BeginPaint(hwndDlg, &ps);
			hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ hOld = SelectObject(hdcMem, params->bmp);

			int y = (rc.bottom + rc.top - params->h) / 2;
			int x = (rc.right + rc.left - params->w) / 2;
			BitBlt(hdc, x, y, params->w, params->h, hdcMem, 0,0, SRCCOPY);
			SelectObject(hdcMem, hOld);
			DeleteDC(hdcMem);

			EndPaint(hwndDlg, &ps);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return TRUE;

		case IDC_SUBMIT:
			GetDlgItemText(hwndDlg, IDC_VALUE, params->Result, _countof(params->Result));
			EndDialog(hwndDlg, 1);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		break;
	}
	return FALSE;
}

bool CJabberProto::ProcessCaptcha(HXML node, HXML parentNode, ThreadData *info)
{
	HXML x = XmlGetChildByTag(node, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
	if (x == nullptr)
		return false;

	HXML y = XmlGetChildByTag(x, L"field", L"var", L"from");
	if (y == nullptr)
		return false;
	if ((y = XmlGetChild(y, "value")) == nullptr)
		return false;

	CAPTCHA_FORM_PARAMS param;
	param.fromjid = XmlGetText(y);

	if ((y = XmlGetChildByTag(x, L"field", L"var", L"sid")) == nullptr)
		return false;
	if ((y = XmlGetChild(y, "value")) == nullptr)
		return false;
	param.sid = XmlGetText(y);

	if ((y = XmlGetChildByTag(x, L"field", L"var", L"ocr")) == nullptr)
		return false;
	param.hint = XmlGetAttrValue (y, L"label");

	param.from = XmlGetAttrValue(parentNode, L"from");
	param.to = XmlGetAttrValue(parentNode, L"to");
	param.challenge = XmlGetAttrValue(parentNode, L"id");
	HXML o = XmlGetChild(parentNode, "data");
	if (o == nullptr || XmlGetText(o) == nullptr)
		return false;

	unsigned bufferLen;
	ptrA buffer((char*)mir_base64_decode( _T2A(XmlGetText(o)), &bufferLen));
	if (buffer == nullptr)
		return false;
	
	IMGSRVC_MEMIO memio;
	memio.iLen = bufferLen;
	memio.pBuf = (void *)buffer;
	memio.fif = FIF_UNKNOWN; /* detect */
	memio.flags = 0;
	param.bmp = (HBITMAP)CallService(MS_IMG_LOADFROMMEM, (WPARAM)&memio, 0);

	BITMAP bmp = {0};
	GetObject(param.bmp, sizeof(bmp), &bmp);
	param.w = bmp.bmWidth;
	param.h = bmp.bmHeight;
	int res = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CAPTCHAFORM), nullptr, JabberCaptchaFormDlgProc, (LPARAM)&param);
	if (mir_wstrcmp(param.Result, L"") == 0 || !res)
		sendCaptchaError(info, param.from, param.to, param.challenge);
	else
		sendCaptchaResult(param.Result, info, param.from, param.challenge, param.fromjid, param.sid);
	return true;
}

void CJabberProto::sendCaptchaResult(wchar_t* buf, ThreadData *info, LPCTSTR from, LPCTSTR challenge, LPCTSTR fromjid,  LPCTSTR sid)
{
	XmlNodeIq iq(L"set", SerialNext());
	HXML query= iq <<XATTR(L"to", from) << XCHILDNS(L"captcha", L"urn:xmpp:captcha") << XCHILDNS(L"x", JABBER_FEAT_DATA_FORMS) << XATTR(L"type", L"submit");
		query << XCHILD(L"field") << XATTR (L"var", L"FORM_TYPE") << XCHILD(L"value", L"urn:xmpp:captcha");
		query << XCHILD(L"field") << XATTR (L"var", L"from") << XCHILD(L"value", fromjid);
		query << XCHILD(L"field") << XATTR (L"var", L"challenge") << XCHILD(L"value", challenge);
		query << XCHILD(L"field") << XATTR (L"var", L"sid") << XCHILD(L"value", sid);
		query << XCHILD(L"field") << XATTR (L"var", L"ocr") << XCHILD(L"value", buf);
	info -> send (iq);
}

void CJabberProto::sendCaptchaError(ThreadData *info, LPCTSTR from, LPCTSTR to, LPCTSTR challenge)
{
	XmlNode message(L"message");
	message << XATTR(L"type", L"error") << XATTR(L"to", from) << XATTR(L"id", challenge) << XATTR(L"from", to)
		<< XCHILD(L"error") << XATTR(L"type", L"modify")
			<< XCHILDNS(L"not-acceptable", L"urn:ietf:params:xml:ns:xmpp-stanzas");
	info->send(message);
}
