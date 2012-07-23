/*
   Authorization State plugin for Miranda-IM (www.miranda-im.org)
   (c) 2006 by Thief
   Icons by Faith Healer

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

   File name      : $URL: svn://svnsrv.mirandaim.ru/mainrepo/authstate/trunk/options.cpp $
   Revision       : $Rev: 233 $
   Last change on : $Date: 2006-10-05 15:48:05 +0300 (Thu, 05 Oct 2006) $
   Last change by : $Author: Thief $

*/

#include "commonheaders.h"

extern HINSTANCE g_hInst;
extern int onExtraImageApplying(WPARAM wParam, LPARAM lParam);
extern int onExtraImageListRebuild(WPARAM wParam, LPARAM lParam);
extern IconExtraColumn g_IECAuth, g_IECGrant, g_IECAuthGrant, g_IECClear;
extern int clistIcon;
//extern byte bUseAuthIcon, bUseGrantIcon, bContactMenuItem, bIconsForRecentContacts;

//#define  EXTRA_ICON_RES0	0	// only used by nicer
//#define  EXTRA_ICON_EMAIL	1
//#define  EXTRA_ICON_WEB		2
//#define  EXTRA_ICON_SMS		3
//#define  EXTRA_ICON_ADV1	4
//#define  EXTRA_ICON_ADV2	5
//#define  EXTRA_ICON_ADV3	6
//#define  EXTRA_ICON_CLIENT	7
//#define  EXTRA_ICON_ADV4	8
//#define  EXTRA_ICON_RES1	9	// only used by nicer
//#define  EXTRA_ICON_PROTO	9	// used by mwclist and modern
//#define  EXTRA_ICON_RES2	10	// only used by nicer
//#define  EXTRA_ICON_VISMODE	10	// only used by modern

#define MS_SKINENG_DRAWICONEXFIX "SkinEngine/DrawIconEx_Fix"

enum {
	clist_none,
	clist_modern,
	clist_nicer
};

const char *szAdvancedIconsModern[] = {
	"E-mail",
	"Web page",
	"Phone/SMS",
	"Advanced #1",
	"Advanced #2",
	"Advanced #3",
	"Client",
	"Advanced #4",
	"Protocol",
	"Visibility/Chat activity"
};
//const int cAdvancedIconsModern = sizeof(szAdvancedIconsModern) / sizeof(szAdvancedIconsModern[0]);

const char *szAdvancedIconsNicer[] = {
	"Reserved",
	"E-mail",
	"Homepage",
	"Telephone",
	"Advanced #1",
	"Advanced #2",
	"Client",
	"Advanced #3",
	"Advanced #4",
	"Reserved #1",
	"Reserved #2"
	};
//const int cAdvancedIconsNicer = sizeof(szAdvancedIconsNicer) / sizeof(szAdvancedIconsNicer[0]);

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int bInitializing = 0;
	static int usedClist = clist_none;
	int cAdvancedIcons;
//	const char **szAdvancedIcons;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			bInitializing = 1;
			if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
			{
				if (ServiceExists(MS_SKINENG_DRAWICONEXFIX))
				{ // modern?
//					SendDlgItemMessage(hwndDlg,IDC_USED_CLIST, WM_SETTEXT,0,(LPARAM)_T("Your clist: modern"));
					cAdvancedIcons  = SIZEOF(szAdvancedIconsModern);
					usedClist = clist_modern;
				}
				else
				{
					if (ServiceExists("CLN/About"))
					{
						usedClist = clist_nicer;
//					szAdvancedIcons = szAdvancedIconsNicer;
						cAdvancedIcons = SIZEOF(szAdvancedIconsNicer);
						SendDlgItemMessage(hwndDlg,IDC_USED_CLIST, WM_SETTEXT,0,(LPARAM)TranslateT("Your clist: nicer"));
					}
					else
	        {
						usedClist = clist_modern;
						cAdvancedIcons  = SIZEOF(szAdvancedIconsModern) - 1; // mw doesn`t have last modern extraicon
//						SendDlgItemMessage(hwndDlg,IDC_USED_CLIST, WM_SETTEXT,0,(LPARAM)_T("Your clist: mw"));
	        }
					SendDlgItemMessage(hwndDlg,IDC_NOTMODERN_WARNING, WM_SETTEXT,0,(LPARAM)TranslateT("Warning: your contact list plugin doesn`t have ability to sort contacts by rate."));
					ShowWindow(GetDlgItem(hwndDlg, IDC_NOTMODERN_WARNING), SW_SHOW);
				}
			}
			else
			{
				cAdvancedIcons = 0;
//				SendDlgItemMessage(hwndDlg,IDC_USED_CLIST, WM_SETTEXT,0,(LPARAM)_T("Your clist: classic"));
				SendDlgItemMessage(hwndDlg,IDC_NOTMODERN_WARNING, WM_SETTEXT,0,(LPARAM)TranslateT("Warning: classic contact list plugin doesn`t have ability to use extraicons"));
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTMODERN_WARNING), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADVICON), 0);
			}
//			cAdvancedIcons = sizeof(szAdvancedIcons) / sizeof(szAdvancedIcons[0]);
//			cAdvancedIcons = SIZEOF(szAdvancedIcons);

			if (usedClist != clist_none) {
				for (int i = 0; i < cAdvancedIcons; i++)
				{
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_ADDSTRING, 0, (LPARAM) Translate( usedClist==clist_modern ? szAdvancedIconsModern[i] : szAdvancedIconsNicer[i]));
				}

				if (usedClist == clist_modern )
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_SETCURSEL, clistIcon-1, 0);
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_SETCURSEL, clistIcon, 0);
			}
			bInitializing = 0;
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) {
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
			{
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
					{
						if (usedClist==clist_none)
							return TRUE;

						clistIcon = SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_GETCURSEL, 0, 0);
						if (usedClist == clist_modern )
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
					}
					return TRUE;
				}
			}
		}
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

int onOptInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInst;
	odp.pszGroup = LPGEN("Contact List");
	odp.pszTitle = LPGEN("Rate");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_RATE_OPT);
	odp.pfnDlgProc = DlgProcOptions;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	return 0;
}
