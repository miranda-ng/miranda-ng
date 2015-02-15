/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-15 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Captcha form

struct CAPTCHA_FORM_PARAMS
{
	HBITMAP bmp;
	int w, h;
	char Result[100];
};

static INT_PTR CALLBACK CaptchaFormDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CAPTCHA_FORM_PARAMS *params = (CAPTCHA_FORM_PARAMS*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG: {
		TranslateDialogDefault(hwndDlg);
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIconByHandle(GetIconHandle("key"), TRUE));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)Skin_GetIconByHandle(GetIconHandle("key")));
		params = (CAPTCHA_FORM_PARAMS*)lParam;

		SetDlgItemText(hwndDlg, IDC_INSTRUCTION, TranslateT("Enter the text you see"));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)params);

		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
		switch (GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT:
		case IDC_INSTRUCTION:
		case IDC_TITLE:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
		return NULL;

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
			BitBlt(hdc, x, y, params->w, params->h, hdcMem, 0, 0, SRCCOPY);
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
			GetDlgItemTextA(hwndDlg, IDC_VALUE, params->Result, SIZEOF(params->Result));
			EndDialog(hwndDlg, 1);
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
		break;
	}
	return FALSE;
}

bool FacebookProto::RunCaptchaForm(std::string captchaUrl, std::string &result)
{
	debugLogA("    RunCaptchaForm: reading picture from %s", captchaUrl.c_str());
	result.clear();

	NETLIBHTTPREQUEST req = { sizeof(req) };
	req.requestType = REQUEST_GET;
	req.szUrl = (char*)captchaUrl.c_str();
	req.flags = NLHRF_NODUMPHEADERS;

	NETLIBHTTPREQUEST *reply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&req);
	if (reply == NULL)
		return false;

	if (reply->resultCode != HTTP_CODE_OK) {
		debugLogA("    RunCaptchaForm: failed with code %d", reply->resultCode);
		return false;
	}

	CAPTCHA_FORM_PARAMS param = { 0 };

	IMGSRVC_MEMIO memio = { 0 };
	memio.iLen = reply->dataLength;
	memio.pBuf = reply->pData;
	memio.fif = FIF_UNKNOWN; /* detect */
	param.bmp = (HBITMAP)CallService(MS_IMG_LOADFROMMEM, (WPARAM)&memio, 0);

	BITMAP bmp = { 0 };
	GetObject(param.bmp, sizeof(bmp), &bmp);
	param.w = bmp.bmWidth;
	param.h = bmp.bmHeight;
	int res = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_CAPTCHAFORM), NULL, CaptchaFormDlgProc, (LPARAM)&param);
	if (res == 0)
		return false;

	debugLogA("    RunCaptchaForm: user entered text %s", param.Result);
	result = param.Result;
	return true;
}
