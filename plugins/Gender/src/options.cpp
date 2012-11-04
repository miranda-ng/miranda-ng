/*
   Show Contact Gender plugin for Miranda-IM (www.miranda-im.org)
   (c) 2006-2011 by Thief

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

   File name      : $URL: http://svn.miranda.im/mainrepo/gender/trunk/options.cpp $
   Revision       : $Rev: 1687 $
   Last change on : $Date: 2011-01-22 18:44:21 +0200 (Сб, 22 янв 2011) $
   Last change by : $Author: Thief $

*/

#include "commonheaders.h"

int applyExtraImage(HANDLE hContact);

extern HINSTANCE g_hInst;
extern int clistIcon;
extern HANDLE g_hExtraIcon;
extern byte bContactMenuItems, bEnableClistIcon, bDrawNoGenderIcon;

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		bInitializing = 1;

		CheckDlgButton(hwndDlg, IDC_MENUITEMS, DBGetContactSettingByte(NULL, MODULENAME, "MenuItems", 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAWUNDEFICON, bDrawNoGenderIcon ? BST_CHECKED : BST_UNCHECKED);

		bInitializing = 0;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MENUITEMS:
		case IDC_DRAWUNDEFICON:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0)
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{			   
					bDrawNoGenderIcon = IsDlgButtonChecked(hwndDlg, IDC_DRAWUNDEFICON);

					HANDLE hContact = db_find_first();
					while (hContact) {         
						applyExtraImage(hContact);
						hContact = db_find_next(hContact);
					}

					DBWriteContactSettingByte(NULL, MODULENAME, "ClistIcon", bEnableClistIcon);
					DBWriteContactSettingByte(NULL, MODULENAME, "NoGenderIcon", bDrawNoGenderIcon);
					DBWriteContactSettingByte(NULL, MODULENAME, "MenuItems", IsDlgButtonChecked(hwndDlg, IDC_MENUITEMS)); 
				}
				return TRUE;
		}
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Plugins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_GENDER_OPT);
	odp.pszTitle = LPGEN("Gender");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	
	return 0;
}
