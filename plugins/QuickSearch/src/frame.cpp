/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#define colorName "Frame background"

static HWND hwndFrame = nullptr;
static int frameId = -1;
static WNDPROC OldEditProc;
static CMStringW wszPattern;
static HBRUSH hBrush;
static COLORREF frm_bkg;

/////////////////////////////////////////////////////////////////////////////////////////
// frame window procedure

static INT_PTR CALLBACK NewEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 27) { // Escape
			SendMessage(hwnd, WM_SETTEXT, 0, 0);
			return 0;
		}
		if (wParam == 13) { // Enter
			CallService(QS_SHOWSERVICE, WPARAM(wszPattern.c_str()), 0);
			return 0;
		}
	}
	return CallWindowProc(OldEditProc, hwnd, msg, wParam, lParam);
}

static int Resizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_FRAME_OPEN: return RD_ANCHORX_LEFT | RD_ANCHORY_CENTRE;
	case IDC_FRAME_EDIT: return RD_ANCHORX_WIDTH | RD_ANCHORY_CENTRE;
	}
	return 0;
}

static INT_PTR CALLBACK FrameWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND hwndTooltip;

	switch (msg) {
	case WM_INITDIALOG:
		OldEditProc = (WNDPROC)SetWindowLongPtrW(GetDlgItem(hwndDlg, IDC_FRAME_EDIT), GWLP_WNDPROC, LPARAM(&NewEditProc));

		hwndTooltip = CreateWindowW(TOOLTIPS_CLASS, nullptr, TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, 0, g_plugin.getInst(), 0);
		{
			TOOLINFOW ti = {};
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			ti.hwnd = hwndDlg;
			ti.hinst = g_plugin.getInst();
			ti.uId = (UINT_PTR)GetDlgItem(hwndDlg, IDC_FRAME_OPEN);
			ti.lpszText = TranslateT("Open QS window");
			SendMessageW(hwndTooltip, TTM_ADDTOOLW, 0, LPARAM(&ti));
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_FRAME_OPEN), BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_plugin.getIcon(IDI_QS));
		return TRUE;

	case WM_DESTROY:
		DeleteObject(hBrush);
		hBrush = nullptr;
		break;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_FRAME), Resizer);
		break;

	case WM_ERASEBKGND:
		GetClientRect(hwndDlg, &rc);
		FillRect(HDC(wParam), &rc, hBrush);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE) {
			wchar_t str[100];
			int length = GetDlgItemTextW(hwndDlg, IDC_FRAME_EDIT, str, _countof(str));
			if (length > 0) {
				CharLowerW(str);
				wszPattern = str;
			}
			else wszPattern.Empty();
		}
		else if (wParam == IDC_FRAME_OPEN) {
			CallService(QS_SHOWSERVICE, WPARAM(wszPattern.c_str()), 0);
		}
		break;
	}

	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnChangeColors(WPARAM, LPARAM)
{
	frm_bkg = Colour_Get("QuickSearch", colorName);

	DeleteObject(hBrush);
	hBrush = CreateSolidBrush(frm_bkg);
	return 0;
}

void CreateFrame(HWND hwndParent)
{
	if (!ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
		return;

	if (hwndParent)
		hwndParent = g_clistApi.hwndContactList;

	if (hwndFrame == nullptr)
		hwndFrame = CreateDialogW(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FRAME), hwndParent, FrameWndProc);

	if (hwndFrame == nullptr)
		return;

	RECT tr;
	GetWindowRect(hwndFrame, &tr);

	CLISTFrame Frame = { sizeof(Frame) };
	Frame.hWnd = hwndFrame;
	Frame.hIcon = g_plugin.getIcon(IDI_QS);
	Frame.align = alTop;
	Frame.height = tr.bottom - tr.top + 2;
	Frame.Flags = F_NOBORDER | F_UNICODE;
	Frame.szName.w = TranslateT("Quick search");

	frameId = CallService(MS_CLIST_FRAMES_ADDFRAME, WPARAM(&Frame), LPARAM(&g_plugin));
	if (frameId <= 0)
		return;

	CallService(MS_CLIST_FRAMES_UPDATEFRAME, frameId, FU_FMPOS);

	HWND wnd = g_clistApi.hwndContactList;
	DWORD tmp = SendMessage(wnd, CLM_GETEXSTYLE, 0, 0);
	SendMessage(wnd, CLM_SETEXSTYLE, tmp | CLS_EX_SHOWSELALWAYS, 0);

	ColourID cid;
	strncpy_s(cid.group, MODULENAME, _TRUNCATE);
	strncpy_s(cid.dbSettingsGroup, MODULENAME, _TRUNCATE);
	strncpy_s(cid.name, colorName, _TRUNCATE);
	strncpy_s(cid.setting, "frame_back", _TRUNCATE);
	cid.defcolour = GetSysColor(COLOR_3DFACE);
	cid.order = 0;
	g_plugin.addColor(&cid);

	HookEvent(ME_COLOUR_RELOAD, &OnChangeColors);
	OnChangeColors(0,0);
}

void DestroyFrame()
{
	if (frameId >= 0) {
		CallService(MS_CLIST_FRAMES_REMOVEFRAME, frameId, 0);
		frameId = -1;
	}

	DestroyWindow(hwndFrame);
	hwndFrame = nullptr;
}
