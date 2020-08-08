/*
Miranda Database Tool
Copyright 2000-2011 Miranda ICQ/IM project,
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

#include "stdafx.h"

static HFONT hBoldFont = nullptr;
static HENHMETAFILE hEmfHeaderLogo = nullptr;

static BOOL CALLBACK MyControlsEnumChildren(HWND hwnd, LPARAM)
{
	DWORD style = GetWindowLongPtr(hwnd, GWL_STYLE);
	DWORD exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	char szClass[64];
	int makeBold = 0;

	GetClassNameA(hwnd, szClass, sizeof(szClass));
	if (!mir_strcmp(szClass, "Static")) {
		if (((style & SS_TYPEMASK) == SS_LEFT || (style & SS_TYPEMASK) == SS_CENTER || (style & SS_TYPEMASK) == SS_RIGHT) && exstyle & WS_EX_CLIENTEDGE)
			makeBold = 1;
	}
	else if (!mir_strcmp(szClass, "Button")) {
		if (exstyle & WS_EX_CLIENTEDGE)
			makeBold = 1;
	}
	if (makeBold) {
		if (hBoldFont == nullptr) {
			LOGFONT lf;
			hBoldFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
			GetObject(hBoldFont, sizeof(lf), &lf);
			lf.lfWeight = FW_BOLD;
			hBoldFont = CreateFontIndirect(&lf);
		}
		SendMessage(hwnd, WM_SETFONT, (WPARAM)hBoldFont, 0);
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle&~WS_EX_CLIENTEDGE);
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	return TRUE;
}

int DoMyControlProcessing(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam, INT_PTR *bReturn)
{
	switch (message) {
	case WM_INITDIALOG:
		EnumChildWindows(hdlg, MyControlsEnumChildren, 0);
		if (hEmfHeaderLogo == nullptr) {
			HRSRC hRsrc = FindResourceA(g_plugin.getInst(), MAKEINTRESOURCEA(IDE_HDRLOGO), "EMF");
			HGLOBAL hGlob = LoadResource(g_plugin.getInst(), hRsrc);
			hEmfHeaderLogo = SetEnhMetaFileBits(SizeofResource(g_plugin.getInst(), hRsrc), (PBYTE)LockResource(hGlob));
		}
		SendDlgItemMessage(hdlg, IDC_HDRLOGO, STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)hEmfHeaderLogo);
		break;

	case WM_CTLCOLORSTATIC:
		if ((GetWindowLongPtr((HWND)lParam, GWL_STYLE) & 0xFFFF) == 0) {
			char szText[256];
			GetWindowTextA((HWND)lParam, szText, _countof(szText));
			if (!mir_strcmp(szText, "whiterect")) {
				SetTextColor((HDC)wParam, RGB(255, 255, 255));
				SetBkColor((HDC)wParam, RGB(255, 255, 255));
				SetBkMode((HDC)wParam, OPAQUE);
				*bReturn = (INT_PTR)GetStockObject(WHITE_BRUSH);
				return TRUE;
			}

			SetBkMode((HDC)wParam, TRANSPARENT);
			*bReturn = (INT_PTR)GetStockObject(NULL_BRUSH);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK WizardDlgProc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hdlgPage;
	auto *opts = (DbToolOptions *)GetWindowLongPtr(hdlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		SetWindowLongPtr(hdlg, GWLP_USERDATA, lParam);
		SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_DBTOOL)));
		hdlgPage = nullptr;

		OpenDatabase(hdlg);
		return TRUE;

	case WZM_GOTOPAGE:
		if (hdlgPage != nullptr) DestroyWindow(hdlgPage);
		EnableWindow(GetDlgItem(hdlg, IDOK), TRUE);
		EnableWindow(GetDlgItem(hdlg, IDCANCEL), TRUE);
		SetDlgItemText(hdlg, IDCANCEL, TranslateT("Cancel"));
		hdlgPage = CreateDialogParamW(g_plugin.getInst(), MAKEINTRESOURCE(wParam), hdlg, (DLGPROC)lParam, LPARAM(opts));
		TranslateDialogDefault(hdlgPage);
		SetWindowPos(hdlgPage, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		ShowWindow(hdlgPage, SW_SHOW);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendMessage(hdlgPage, WZN_PAGECHANGING, wParam, 0);
			SendMessage(hdlgPage, message, wParam, lParam);
			break;

		case IDCANCEL:
			wizardResult = 0;
			SendMessage(hdlgPage, WZN_CANCELCLICKED, 0, 0);
			EndDialog(hdlg, wizardResult);
			break;
		}
		break;

	case WM_DESTROY:
		if (opts->dbChecker) {
			opts->dbChecker->Destroy();
			opts->dbChecker = nullptr;
		}
		delete opts;

		DestroyWindow(hdlgPage);
		if (hBoldFont != nullptr) {
			DeleteObject(hBoldFont);
			hBoldFont = nullptr;
		}
		if (hEmfHeaderLogo != nullptr) {
			DeleteEnhMetaFile(hEmfHeaderLogo);
			hEmfHeaderLogo = nullptr;
		}
		break;
	}
	return FALSE;
}
