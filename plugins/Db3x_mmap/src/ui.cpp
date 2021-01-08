/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDb3Mmap *db = (CDb3Mmap *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		db = (CDb3Mmap*)lParam;
		CheckRadioButton(hwndDlg, IDC_STANDARD, IDC_TOTAL, IDC_STANDARD + db->isEncrypted());
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			if (IsDlgButtonChecked(hwndDlg, IDC_TOTAL) != (UINT)db->isEncrypted()) {
				db->ToggleEncryption();
				CheckRadioButton(hwndDlg, IDC_STANDARD, IDC_TOTAL, IDC_STANDARD + db->isEncrypted());
			}
			break;
		}
		break;
	}

	return FALSE;
}

static int OnOptionsInit(PVOID obj, WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -790000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = LPGEN("Database");
	odp.pfnDlgProc = DlgProcOptions;
	odp.dwInitParam = (LPARAM)obj;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

static int OnModulesLoaded(PVOID obj, WPARAM, LPARAM)
{
	HookEventObj(ME_OPT_INITIALISE, OnOptionsInit, obj);
	return 0;
}

void CDb3Mmap::InitDialogs()
{
	hHook = HookEventObj(ME_SYSTEM_MODULESLOADED, OnModulesLoaded, this);
}
