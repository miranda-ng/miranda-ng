/*
   Authorization State plugin for Miranda-IM (www.miranda-im.org)
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

   File name      : $URL: http://svn.miranda.im/mainrepo/authstate/trunk/options.cpp $
   Revision       : $Rev: 1686 $
   Last change on : $Date: 2010-10-06 08:26:58 +0200 (Ср, 06 окт 2010) $
   Last change by : $Author: ghazan $

*/

#include "commonheaders.h"

const TCHAR* szAdvancedIcons[] = {_T("Email"), _T("Protocol"), _T("SMS"), _T("Advanced 1"), _T("Advanced 2"), _T("Web"), _T("Client"), _T("Advanced 3"), _T("Advanced 4"), _T("Advanced 5")};
const INT cAdvancedIcons = sizeof(szAdvancedIcons) / sizeof(szAdvancedIcons[0]);

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			bInitializing = 1;
			if (hExtraIcon==NULL)
			{
				for (int i = 0; i < cAdvancedIcons; i++)
				{
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_ADDSTRING, 0, (LPARAM) TranslateTS(szAdvancedIcons[i]));
				}
				SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_SETCURSEL, clistIcon-1, 0);
			}
			else
			{
				SendDlgItemMessage(hwndDlg,IDC_NOTICE, WM_SETTEXT,0,(LPARAM)TranslateT("Extraicons plugin is used, use Contact list > Extraicons for customize."));
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTICE), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADVICON), 0);
			}

			CheckDlgButton(hwndDlg, IDC_AUTHICON, bUseAuthIcon ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_GRANTICON, bUseGrantIcon ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ENABLEMENUITEM, bContactMenuItem ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ICONSFORRECENT, bIconsForRecentContacts ? BST_CHECKED : BST_UNCHECKED);

			bInitializing = 0;
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_AUTHICON:
				case IDC_GRANTICON:
				case IDC_ENABLEMENUITEM:
				case IDC_ICONSFORRECENT:
				{
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}
				case IDC_ADVICON:
				{
					if ((HIWORD(wParam) == EN_CHANGE) && (!bInitializing) || (HIWORD(wParam) == CBN_SELENDOK))
					{
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
					break;
				}
			}
			break;
		}

		case WM_NOTIFY:
		{
			if (((LPNMHDR)lParam)->idFrom == 0)
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
					{
						bUseAuthIcon = IsDlgButtonChecked(hwndDlg, IDC_AUTHICON);
						bUseGrantIcon = IsDlgButtonChecked(hwndDlg, IDC_GRANTICON);
						bContactMenuItem = IsDlgButtonChecked(hwndDlg, IDC_ENABLEMENUITEM);
						bIconsForRecentContacts = IsDlgButtonChecked(hwndDlg, IDC_ICONSFORRECENT);

						clistIcon = SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_GETCURSEL, 0, 0);
						clistIcon++;

						HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
						while (hContact)
						{
							CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&g_IECClear);
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
						}

						g_IECClear.ColumnType = clistIcon;
						onExtraImageListRebuild(0,0);

						hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
						while (hContact)
						{
							onExtraImageApplying((WPARAM)hContact,0);
							hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
						}

						//Store options values to DB
						DBWriteContactSettingByte(NULL, MODULENAME, "AdvancedIcon", clistIcon);
						DBWriteContactSettingByte(NULL, MODULENAME, "EnableAuthIcon", bUseAuthIcon);
						DBWriteContactSettingByte(NULL, MODULENAME, "EnableGrantIcon", bUseGrantIcon);
						DBWriteContactSettingByte(NULL, MODULENAME, "MenuItem", bContactMenuItem);
						DBWriteContactSettingByte(NULL, MODULENAME, "EnableOnlyForRecent", bIconsForRecentContacts);
					}
					return TRUE;
				}
		}

		 case WM_DESTROY:
		 break;

	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = g_hInst;
	odp.ptszGroup = _T("Contact List");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_AUTHSTATE_OPT);
	odp.ptszTitle = _T("Auth State");
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);

	return 0;
}
