/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

struct EnterStringFormParam : public ENTER_STRING
{
	int idcControl;
	int height;
};

static int UIEmulateBtnClick(HWND hwndDlg, UINT idcButton)
{
	if (IsWindowEnabled(GetDlgItem(hwndDlg, idcButton)))
		PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(idcButton, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, idcButton));
	return 0;
}

static int sttEnterStringResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_TXT_MULTILINE:
	case IDC_TXT_COMBO:
	case IDC_TXT_RICHEDIT:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP | RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDOK:
	case IDCANCEL:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static void ComboLoadRecentStrings(HWND hwndDlg, EnterStringFormParam *pForm)
{
	for (int i = 0; i < pForm->recentCount; i++) {
		char setting[MAXMODULELABELLENGTH];
		mir_snprintf(setting, "%s%d", pForm->szDataPrefix, i);
		ptrW tszRecent(db_get_wsa(0, pForm->szModuleName, setting));
		if (tszRecent != nullptr)
			SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_ADDSTRING, 0, tszRecent);
	}

	if (!SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_GETCOUNT, 0, 0))
		SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_ADDSTRING, 0, (LPARAM)L"");
}

static void ComboAddRecentString(HWND hwndDlg, EnterStringFormParam *pForm)
{
	wchar_t *string = pForm->ptszResult;
	if (!string || !*string)
		return;
	
	if (SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_FINDSTRING, (WPARAM)-1, (LPARAM)string) != CB_ERR)
		return;

	int id;
	SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_ADDSTRING, 0, (LPARAM)string);
	if ((id = SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_FINDSTRING, (WPARAM)-1, (LPARAM)L"")) != CB_ERR)
		SendDlgItemMessage(hwndDlg, pForm->idcControl, CB_DELETESTRING, id, 0);

	id = db_get_b(0, pForm->szModuleName, pForm->szDataPrefix, 0);
	char setting[MAXMODULELABELLENGTH];
	mir_snprintf(setting, "%s%d", pForm->szDataPrefix, id);
	db_set_ws(0, pForm->szModuleName, setting, string);
	db_set_b(0, pForm->szModuleName, pForm->szDataPrefix, (id + 1) % pForm->idcControl);
}

static INT_PTR CALLBACK sttEnterStringDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	EnterStringFormParam *params = (EnterStringFormParam *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_RENAME);
		params = (EnterStringFormParam *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)params);
		SetWindowText(hwndDlg, params->caption);
		{
			RECT rc; GetWindowRect(hwndDlg, &rc);
			switch (params->type) {
			case ESF_PASSWORD:
				params->idcControl = IDC_TXT_PASSWORD;
				params->height = rc.bottom - rc.top;
				break;

			case ESF_MULTILINE:
				params->idcControl = IDC_TXT_MULTILINE;
				params->height = 0;
				rc.bottom += (rc.bottom - rc.top) * 2;
				SetWindowPos(hwndDlg, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION);
				break;

			case ESF_COMBO:
				params->idcControl = IDC_TXT_COMBO;
				params->height = rc.bottom - rc.top;
				if (params->szDataPrefix && params->recentCount)
					ComboLoadRecentStrings(hwndDlg, params);
				break;

			case ESF_RICHEDIT:
				params->idcControl = IDC_TXT_RICHEDIT;
				SendDlgItemMessage(hwndDlg, IDC_TXT_RICHEDIT, EM_AUTOURLDETECT, TRUE, 0);
				SendDlgItemMessage(hwndDlg, IDC_TXT_RICHEDIT, EM_SETEVENTMASK, 0, ENM_LINK);
				params->height = 0;
				rc.bottom += (rc.bottom - rc.top) * 2;
				SetWindowPos(hwndDlg, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION);
				break;

			default: // single string edit field
				params->idcControl = IDC_TXT_SIMPLE;
				params->height = rc.bottom - rc.top;
			}
		}
		ShowWindow(GetDlgItem(hwndDlg, params->idcControl), SW_SHOW);
		if (params->ptszInitVal)
			SetDlgItemText(hwndDlg, params->idcControl, params->ptszInitVal);

		if (params->szDataPrefix)
			Utils_RestoreWindowPosition(hwndDlg, 0, params->szModuleName, params->szDataPrefix);

		SetTimer(hwndDlg, 1000, 50, nullptr);

		if (params->timeout > 0) {
			SetTimer(hwndDlg, 1001, 1000, nullptr);
			wchar_t buf[128];
			mir_snwprintf(buf, TranslateT("OK (%d)"), params->timeout);
			SetDlgItemText(hwndDlg, IDOK, buf);
		}

		return TRUE;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		break;

	case WM_TIMER:
		switch (wParam) {
		case 1000:
			KillTimer(hwndDlg, 1000);
			EnableWindow(GetParent(hwndDlg), TRUE);
			break;

		case 1001:
			wchar_t buf[128];
			mir_snwprintf(buf, TranslateT("OK (%d)"), --params->timeout);
			SetDlgItemText(hwndDlg, IDOK, buf);

			if (params->timeout < 0) {
				KillTimer(hwndDlg, 1001);
				UIEmulateBtnClick(hwndDlg, IDOK);
			}
		}
		return TRUE;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_ENTER_STRING), sttEnterStringResizer);
		break;

	case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
			if (params && params->height)
				lpmmi->ptMaxSize.y = lpmmi->ptMaxTrackSize.y = params->height;
		}
		break;

	case WM_NOTIFY:
		{
			ENLINK *param = (ENLINK *)lParam;
			if (param->nmhdr.idFrom != IDC_TXT_RICHEDIT) break;
			if (param->nmhdr.code != EN_LINK) break;
			if (param->msg != WM_LBUTTONUP) break;

			CHARRANGE sel;
			SendMessage(param->nmhdr.hwndFrom, EM_EXGETSEL, 0, (LPARAM)& sel);
			if (sel.cpMin != sel.cpMax) break; // allow link selection

			TEXTRANGE tr;
			tr.chrg = param->chrg;
			tr.lpstrText = (wchar_t *)mir_alloc(sizeof(wchar_t)*(tr.chrg.cpMax - tr.chrg.cpMin + 2));
			SendMessage(param->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

			Utils_OpenUrlW(tr.lpstrText);
			mir_free(tr.lpstrText);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TXT_MULTILINE:
		case IDC_TXT_RICHEDIT:
			if ((HIWORD(wParam) != EN_SETFOCUS) && (HIWORD(wParam) != EN_KILLFOCUS)) {
				SetDlgItemText(hwndDlg, IDOK, TranslateT("OK"));
				KillTimer(hwndDlg, 1001);
			}
			break;

		case IDC_TXT_COMBO:
			if ((HIWORD(wParam) != CBN_SETFOCUS) && (HIWORD(wParam) != CBN_KILLFOCUS)) {
				SetDlgItemText(hwndDlg, IDOK, TranslateT("OK"));
				KillTimer(hwndDlg, 1001);
			}
			break;

		case IDCANCEL:
			if (params->szDataPrefix)
				Utils_SaveWindowPosition(hwndDlg, 0, params->szModuleName, params->szDataPrefix);

			EndDialog(hwndDlg, 0);
			break;

		case IDOK:
			HWND hWnd = GetDlgItem(hwndDlg, params->idcControl);
			int len = GetWindowTextLength(hWnd)+1;
			params->ptszResult = (LPTSTR)mir_alloc(sizeof(wchar_t)*len);
			GetWindowText(hWnd, params->ptszResult, len);

			if ((params->type == ESF_COMBO) && params->szDataPrefix && params->recentCount)
				ComboAddRecentString(hwndDlg, params);
			if (params->szDataPrefix)
				Utils_SaveWindowPosition(hwndDlg, 0, params->szModuleName, params->szDataPrefix);

			EndDialog(hwndDlg, 1);
			break;
		}
	}

	return FALSE;
}

MIR_APP_DLL(bool) EnterString(ENTER_STRING *pForm)
{
	if (pForm == nullptr)
		return false;

	EnterStringFormParam param = {};
	memcpy(&param, pForm, sizeof(ENTER_STRING));
	if (!DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_ENTER_STRING), GetForegroundWindow(), sttEnterStringDlgProc, LPARAM(&param)))
		return false;

	pForm->ptszResult = param.ptszResult;
	return true;
}
