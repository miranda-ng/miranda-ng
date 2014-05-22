/* 
Copyright (C) 2006-2010 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "commons.h"

static LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK AddReplacementDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

struct Data
{
	Dictionary *dict;
	tstring find;
	tstring replace;
	BOOL useVariables;

	BOOL modal;
	BOOL findReadOnly;

	AutoReplaceDialogCallback callback;
	void *param;
};

BOOL ShowAutoReplaceDialog(HWND parent, BOOL modal, 
	Dictionary *dict, const TCHAR *find, const TCHAR *replace, BOOL useVariables,
	BOOL findReadOnly, AutoReplaceDialogCallback callback, void *param)
{
	Data *data = new Data();
	data->dict = dict;
	data->useVariables = useVariables;
	data->modal = modal;
	data->findReadOnly = findReadOnly;
	data->callback = callback;
	data->param = param;

	if (find != NULL)
		data->find = find;

	if (replace != NULL)
		data->replace = replace;

	if (modal)
		return DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ADD_REPLACEMENT), parent, AddReplacementDlgProc, (LPARAM)data);

	HWND hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADD_REPLACEMENT), parent, AddReplacementDlgProc, (LPARAM)data);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	SetFocus(GetDlgItem(hwnd, IDC_NEW));
	ShowWindow(hwnd, SW_SHOW);
	return TRUE;
}


static LRESULT CALLBACK OnlyCharsEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Data *data = (Data *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
	case WM_CHAR:
		if (wParam == VK_BACK)
			break;
		if (lParam & (1 << 28))	// ALT key
			break;
		if (GetKeyState(VK_CONTROL) & 0x8000)	// CTRL key
			break;
		{
			TCHAR c = (TCHAR) wParam;
			if (!data->dict->autoReplace->isWordChar(c))
				return 1;

			TCHAR tmp[2] = { c, 0 };
			CharLower(tmp);
			wParam = tmp[0];

			break;
		}
	}

	LRESULT ret = mir_callNextSubclass(hwnd, OnlyCharsEditProc, msg, wParam, lParam);

	switch(msg) {
	case EM_PASTESPECIAL:
	case WM_PASTE:
		TCHAR text[256];
		GetWindowText(hwnd, text, SIZEOF(text));

		scoped_free<TCHAR> dest = data->dict->autoReplace->filterText(text);
		SetWindowText(hwnd, dest);
		break;
	}

	return ret;
}

static BOOL CenterParent(HWND hwnd)
{
	HWND hwndParent = GetParent(hwnd);
	if (hwndParent == NULL)
		return FALSE;

	RECT rect, rectP;
	GetWindowRect(hwnd, &rect);
	GetWindowRect(hwndParent, &rectP);

	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	int x = ((rectP.right - rectP.left) - width) / 2 + rectP.left;
	int y = ((rectP.bottom - rectP.top) - height) / 2 + rectP.top;

	int screenwidth = GetSystemMetrics(SM_CXSCREEN);
	int screenheight = GetSystemMetrics(SM_CYSCREEN);

	if (x + width > screenwidth)  x = screenwidth - width;
	if (y + height > screenheight) y = screenheight - height;
	if (x < 0) x = 0;
	if (y < 0) y = 0;

	MoveWindow(hwnd, x, y, width, height, FALSE);

	return TRUE;
}

static void Close(HWND hwndDlg, int ret)
{
	Data *data = (Data *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	if (!ret)
		data->callback(TRUE, data->dict,
		data->find.c_str(), data->replace.c_str(), data->useVariables,
		data->find.c_str(), data->param);

	if (data->modal)
		EndDialog(hwndDlg, ret);
	else
		DestroyWindow(hwndDlg);

	delete data;
}

static INT_PTR CALLBACK AddReplacementDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			Data *data = (Data *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)data);

			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_OLD), GWLP_USERDATA, (LONG_PTR)data);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_OLD), OnlyCharsEditProc);

			HICON hIcon = Skin_GetIcon("spellchecker_enabled");
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			Skin_ReleaseIcon(hIcon);

			SendDlgItemMessage(hwndDlg, IDC_OLD, EM_LIMITTEXT, 256, 0);
			SendDlgItemMessage(hwndDlg, IDC_NEW, EM_LIMITTEXT, 256, 0);

			if (!data->find.empty()) {
				scoped_free<TCHAR> tmp = data->dict->autoReplace->filterText(data->find.c_str());
				SetDlgItemText(hwndDlg, IDC_OLD, tmp);
			}
			if (!data->replace.empty())
				SetDlgItemText(hwndDlg, IDC_NEW, data->replace.c_str());

			CheckDlgButton(hwndDlg, IDC_VARIABLES, data->useVariables ? BST_CHECKED : BST_UNCHECKED);

			if (data->findReadOnly) {
				SendDlgItemMessage(hwndDlg, IDC_OLD, EM_SETREADONLY, TRUE, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_OLD_PS), FALSE);
			}

			if (!variables_enabled) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_VARIABLES), FALSE);
				ShowWindow(GetDlgItem(hwndDlg, IDC_VAR_HELP), FALSE);

				RECT rc_old;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_OLD), &rc_old);
				RECT rc_new;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_NEW), &rc_new);
				rc_new.right = rc_old.right;

				SetWindowPos(GetDlgItem(hwndDlg, IDC_NEW), NULL, 0, 0,
								 rc_new.right - rc_new.left, rc_new.bottom - rc_new.top,
								 SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOZORDER);
			}
			else {
				variables_skin_helpbutton(hwndDlg, IDC_VAR_HELP);
				EnableWindow(GetDlgItem(hwndDlg, IDC_VAR_HELP), IsDlgButtonChecked(hwndDlg, IDC_VARIABLES));
			}

			CenterParent(hwndDlg);
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
			{
				Data *data = (Data *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

				TCHAR find[256];
				if (data->findReadOnly)
					lstrcpyn(find, data->find.c_str(), SIZEOF(find));
				else {
					GetDlgItemText(hwndDlg, IDC_OLD, find, SIZEOF(find));
					lstrtrim(find);
				}

				TCHAR replace[256];
				GetDlgItemText(hwndDlg, IDC_NEW, replace, SIZEOF(replace));
				lstrtrim(replace);

				if (!data->findReadOnly && find[0] == 0)
					MessageBox(hwndDlg, TranslateT("The wrong word can't be empty!"), TranslateT("Wrong Correction"), MB_OK | MB_ICONERROR);

				else if (replace[0] == 0)
					MessageBox(hwndDlg, TranslateT("The correction can't be empty!"), TranslateT("Wrong Correction"), MB_OK | MB_ICONERROR);

				else if (_tcscmp(find, replace) == 0)
					MessageBox(hwndDlg, TranslateT("The correction can't be equal to the wrong word!"), TranslateT("Wrong Correction"), MB_OK | MB_ICONERROR);

				else {
					data->callback(FALSE, data->dict,
										find, replace, IsDlgButtonChecked(hwndDlg, IDC_VARIABLES),
										data->find.c_str(), data->param);
					Close(hwndDlg, 1);
				}
			}
			break;

		case IDCANCEL:
			Close(hwndDlg, 0);
			break;

		case IDC_VARIABLES:
			EnableWindow(GetDlgItem(hwndDlg, IDC_VAR_HELP), IsDlgButtonChecked(hwndDlg, IDC_VARIABLES));
			break;

		case IDC_VAR_HELP:
			variables_showhelp(hwndDlg, IDC_NEW, VHF_FULLDLG, NULL, "The wrong word typed by the user");
			break;
		}
		break;

	case WM_CLOSE:
		Close(hwndDlg, 0);
		break;
	}

	return FALSE;
}
