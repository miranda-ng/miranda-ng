/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (C) 2012-22 Miranda NG team

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
	const char *from;
	const char *challenge;
	const char *fromjid;
	const char *sid;
	const char *to;
	const char *hint;
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
		Window_SetIcon_IcoLib(hwndDlg, Skin_GetIconHandle(SKINICON_OTHER_KEYS));
		params = (CAPTCHA_FORM_PARAMS*)lParam;

		const char *hint = params->hint;
		if (hint == nullptr)
			SetDlgItemTextW(hwndDlg, IDC_INSTRUCTION, TranslateT("Enter the text you see"));
		else
			SetDlgItemTextW(hwndDlg, IDC_INSTRUCTION, TranslateW(Utf2T(hint)));
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

		case IDOK:
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

bool CJabberProto::ProcessCaptcha(const TiXmlElement *node, const TiXmlElement *parentNode, ThreadData *info)
{
	auto *x = XmlGetChildByTag(node, "x", "xmlns", JABBER_FEAT_DATA_FORMS);
	if (x == nullptr)
		return false;

	auto *y = XmlGetChildByTag(x, "field", "var", "from");
	if (y == nullptr)
		return false;
	if ((y = XmlFirstChild(y, "value")) == nullptr)
		return false;

	CAPTCHA_FORM_PARAMS param;
	param.fromjid = y->GetText();

	if ((y = XmlGetChildByTag(x, "field", "var", "sid")) == nullptr)
		return false;
	if ((y = XmlFirstChild(y, "value")) == nullptr)
		return false;
	param.sid = y->GetText();

	if ((y = XmlGetChildByTag(x, "field", "var", "ocr")) == nullptr)
		return false;
	param.hint = XmlGetAttr(y, "label");

	param.from = XmlGetAttr(parentNode, "from");
	param.to = XmlGetAttr(parentNode, "to");
	param.challenge = XmlGetAttr(parentNode, "id");
	auto *o = XmlFirstChild(parentNode, "data");
	if (o == nullptr || o->GetText() == nullptr)
		return false;

	size_t bufferLen;
	ptrA buffer((char*)mir_base64_decode(o->GetText(), &bufferLen));
	if (buffer == nullptr)
		return false;
	
	param.bmp = Image_LoadFromMem(buffer, bufferLen, FIF_UNKNOWN);

	BITMAP bmp = {0};
	GetObject(param.bmp, sizeof(bmp), &bmp);
	param.w = bmp.bmWidth;
	param.h = bmp.bmHeight;
	int res = DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CAPTCHAFORM), nullptr, JabberCaptchaFormDlgProc, (LPARAM)&param);
	if (mir_wstrcmp(param.Result, L"") == 0 || !res)
		sendCaptchaError(info, param.from, param.to, param.challenge);
	else
		sendCaptchaResult(T2Utf(param.Result), info, param.from, param.challenge, param.fromjid, param.sid);
	return true;
}

void CJabberProto::sendCaptchaResult(char* buf, ThreadData *info, const char *from, const char *challenge, const char *fromjid,  const char *sid)
{
	XmlNodeIq iq("set", SerialNext());
	TiXmlElement *query= iq << XATTR("to", from) << XCHILDNS("captcha", "urn:xmpp:captcha") << XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit");
		query << XCHILD("field") << XATTR ("var", "FORM_TYPE") << XCHILD("value", "urn:xmpp:captcha");
		query << XCHILD("field") << XATTR ("var", "from") << XCHILD("value", fromjid);
		query << XCHILD("field") << XATTR ("var", "challenge") << XCHILD("value", challenge);
		query << XCHILD("field") << XATTR ("var", "sid") << XCHILD("value", sid);
		query << XCHILD("field") << XATTR ("var", "ocr") << XCHILD("value", buf);
	info -> send (iq);
}

void CJabberProto::sendCaptchaError(ThreadData *info, const char *from, const char *to, const char *challenge)
{
	XmlNode message("message");
	message << XATTR("type", "error") << XATTR("to", from) << XATTR("id", challenge) << XATTR("from", to)
		<< XCHILD("error") << XATTR("type", "modify")
			<< XCHILDNS("not-acceptable", "urn:ietf:params:xml:ns:xmpp-stanzas");
	info->send(message);
}
