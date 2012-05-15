/*
   IgnoreState plugin for Miranda-IM (www.miranda-im.org)
   (c) 2010 by Kildor

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

TCHAR* szAdvancedIconsModern[] = {
	_T("E-mail"),
	_T("Web page"),
	_T("Phone/SMS"),
	_T("Advanced #1"),
	_T("Advanced #2"),
	_T("Advanced #3"),
	_T("Client"),
	_T("Advanced #4"),
	_T("Protocol"),
	_T("Visibility/Chat activity")
};
//const int cAdvancedIconsModern = sizeof(szAdvancedIconsModern) / sizeof(szAdvancedIconsModern[0]);

TCHAR* szAdvancedIconsNicer[] = {
	_T("Reserved"),
	_T("E-mail"),
	_T("Homepage"),
	_T("Telephone"),
	_T("Advanced #1"),
	_T("Advanced #2"),
	_T("Client"),
	_T("Advanced #3"),
	_T("Advanced #4"),
	_T("Reserved #1"),
	_T("Reserved #2")
	};
//const int cAdvancedIconsNicer = sizeof(szAdvancedIconsNicer) / sizeof(szAdvancedIconsNicer[0]);

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HIMAGELIST himlButtonIcons=NULL;
	INT bInitializing = 0;
	INT usedClist = clist_none;
	INT cAdvancedIcons;
	isExtraiconsUsed = ServiceExists(MS_EXTRAICON_REGISTER);
//	const char **szAdvancedIcons;
	HWND hTree=GetDlgItem(hwndDlg,IDC_FILTER);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);
			bInitializing = 1;
			fill_filter();
			if (ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
			{
				if (hExtraIcon == NULL)
				{
					if (ServiceExists(MS_SKINENG_DRAWICONEXFIX))
					{ // modern?
						cAdvancedIcons  = SIZEOF(szAdvancedIconsModern);
						usedClist = clist_modern;
					}
					else
					{
						if (ServiceExists("CLN/About"))
						{
							usedClist = clist_nicer;
							cAdvancedIcons = SIZEOF(szAdvancedIconsNicer);
						}
						else
						{
							usedClist = clist_modern;
							cAdvancedIcons  = SIZEOF(szAdvancedIconsModern) - 1; // mw doesn`t have last modern extraicon
						}
					}
				}
				else
				{
				SendDlgItemMessage(hwndDlg,IDC_NOTICE, WM_SETTEXT,0,(LPARAM)TranslateT("Extraicons plugin is used, use Contact list > Extraicons for customize."));
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTICE), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADVICON), 0);
				}
			}
			else
			{
				cAdvancedIcons = 0;
//				SendDlgItemMessage(hwndDlg,IDC_USED_CLIST, WM_SETTEXT,0,(LPARAM)_T("Your clist: classic"));
				SendDlgItemMessage(hwndDlg,IDC_NOTICE, WM_SETTEXT,0,(LPARAM)TranslateT("Warning: classic contact list plugin doesn`t have ability to use extraicons"));
				ShowWindow(GetDlgItem(hwndDlg, IDC_NOTICE), SW_SHOW);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ADVICON), 0);
			}
			if (usedClist != clist_none) {
				for (int i = 0; i < cAdvancedIcons; i++)
				{
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_ADDSTRING, 0, (LPARAM) TranslateTS( usedClist==clist_modern ? szAdvancedIconsModern[i] : szAdvancedIconsNicer[i]));
				}

				if (usedClist == clist_modern )
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_SETCURSEL, clistIcon-1, 0);
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_SETCURSEL, clistIcon, 0);
			}

			SetWindowLong(hTree,GWL_STYLE,GetWindowLong(hTree,GWL_STYLE)|TVS_NOHSCROLL);
			{
				himlButtonIcons=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
				TreeView_SetImageList(hTree,himlButtonIcons,TVSIL_NORMAL);
			}
			TreeView_DeleteAllItems(hTree);

			int i;
			int cii = SIZEOF(ii);
			for (i=2; i<cii; i++) // we don`t need it IGNORE_ALL and IGNORE_MESSAGE
			{
				TVINSERTSTRUCT tvis={0};
				HTREEITEM hti;
                int index=ImageList_AddIcon(himlButtonIcons, LoadSkinnedIcon(ii[i].icon));
				tvis.hParent=NULL;
				tvis.hInsertAfter=TVI_LAST;
				tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
				tvis.item.lParam=(LPARAM)(ii[i].type);
				tvis.item.pszText=TranslateTS(ii[i].name);
				tvis.item.iImage=tvis.item.iSelectedImage=index;
				hti=TreeView_InsertItem(hTree,&tvis);
				TreeView_SetCheckState(hTree, hti, checkState(ii[i].type));
//				TreeView_SetCheckState(hTree, hti, (bool)(ii[i].filtered || ii[0].filtered));
			}

			CheckDlgButton(hwndDlg, IDC_IGNORE_IGNOREALL, bUseMirandaSettings );
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER), !bUseMirandaSettings);
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
				case IDC_IGNORE_IGNOREALL:
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_FILTER),  !IsDlgButtonChecked(hwndDlg,IDC_IGNORE_IGNOREALL));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->idFrom)
			{
			case 0:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							if (usedClist==clist_none && hExtraIcon == NULL )
								return TRUE;
///////////
							DWORD flags=0;
							TVITEM tvi;
							tvi.mask=TVIF_HANDLE|TBIF_LPARAM;
							HWND hTree=GetDlgItem(hwndDlg,IDC_FILTER);
							tvi.hItem=TreeView_GetRoot(hTree); //check ignore all
							while(tvi.hItem)
							{
								TreeView_GetItem(hTree,&tvi);
								if(TreeView_GetCheckState(hTree,tvi.hItem)) flags|=1<<(tvi.lParam-1);
								tvi.hItem=TreeView_GetNextSibling(hTree,tvi.hItem);
							}
							DBWriteContactSettingDword(NULL, MODULENAME, "Filter", flags);

							bUseMirandaSettings = IsDlgButtonChecked(hwndDlg,IDC_IGNORE_IGNOREALL) ? 1 : 0;
							DBWriteContactSettingByte(NULL, MODULENAME, "UseMirandaSettings", bUseMirandaSettings);

							fill_filter();
//////////
							if (hExtraIcon == NULL)
							{
								clistIcon = SendMessage(GetDlgItem(hwndDlg, IDC_ADVICON), CB_GETCURSEL, 0, 0);
								if (usedClist == clist_modern )
									clistIcon++;

								//Store options values to DB
								DBWriteContactSettingByte(NULL, MODULENAME, "AdvancedIcon", clistIcon);

								HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
									while (hContact)
								{
									CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM)hContact, (LPARAM)&g_IECClear);
									hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
								}
								g_IECClear.ColumnType = clistIcon;
								if (hExtraIcon == NULL)
									onExtraImageListRebuild(0,0);
								hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
								while (hContact)
								{
									onExtraImageApplying((WPARAM)hContact,0);
									hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
								}
							}
						}
					}
				}
			case IDC_FILTER:
				if (((LPNMHDR)lParam)->code == NM_CLICK)
				{
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
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
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = 0;
	odp.hInstance = g_hInst;
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszGroup = _T("Contact List");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_RATE_OPT);
	odp.ptszTitle = _T(MODULENAME);
	odp.pfnDlgProc = DlgProcOptions;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) &odp);

	return 0;
}
