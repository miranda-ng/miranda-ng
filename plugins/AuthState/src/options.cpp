/*
   Authorization State plugin for Miranda NG (www.miranda-ng.org)
   (c) 2006-2010 by Thief

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

#include "commonheaders.h"

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		bInitializing = 1;

		CheckDlgButton(hwndDlg, IDC_AUTHICON, bUseAuthIcon ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_GRANTICON, bUseGrantIcon ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ENABLEMENUITEM, bContactMenuItem ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ICONSFORRECENT, bIconsForRecentContacts ? BST_CHECKED : BST_UNCHECKED);

		bInitializing = 0;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_AUTHICON:
		case IDC_GRANTICON:
		case IDC_ENABLEMENUITEM:
		case IDC_ICONSFORRECENT:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0)
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				bUseAuthIcon = IsDlgButtonChecked(hwndDlg, IDC_AUTHICON);
				bUseGrantIcon = IsDlgButtonChecked(hwndDlg, IDC_GRANTICON);
				bContactMenuItem = IsDlgButtonChecked(hwndDlg, IDC_ENABLEMENUITEM);
				bIconsForRecentContacts = IsDlgButtonChecked(hwndDlg, IDC_ICONSFORRECENT);

				for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
					onExtraImageApplying((WPARAM)hContact, 0);

				//Store options values to DB
				db_set_b(NULL, MODULENAME, "EnableAuthIcon", bUseAuthIcon);
				db_set_b(NULL, MODULENAME, "EnableGrantIcon", bUseGrantIcon);
				db_set_b(NULL, MODULENAME, "MenuItem", bContactMenuItem);
				db_set_b(NULL, MODULENAME, "EnableOnlyForRecent", bIconsForRecentContacts);
				return TRUE;
		}
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Icons");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_AUTHSTATE_OPT);
	odp.pszTitle = LPGEN("Auth State");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
