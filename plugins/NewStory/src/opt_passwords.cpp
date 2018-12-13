#include "stdafx.h"

int icoidNoPassword, icoidPassword;

void SetAllContactIcons(HWND hwndList)
{
	for (auto &hContact : Contacts()) {
		MCONTACT hItem = (MCONTACT)SendMessage(hwndList, CLM_FINDCONTACT, (WPARAM)hContact, 0);
		SendMessage(hwndList, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(0, CheckPassword(hContact, "") ? icoidNoPassword : icoidPassword));
	}
}

static void ResetListOptions(HWND hwndList)
{
	SendMessage(hwndList, CLM_SETBKBITMAP, 0, (LPARAM)(HBITMAP)NULL);
	SendMessage(hwndList, CLM_SETBKCOLOR, GetSysColor(COLOR_WINDOW), 0);
	SendMessage(hwndList, CLM_SETGREYOUTFLAGS, 0, 0);
	SendMessage(hwndList, CLM_SETLEFTMARGIN, 2, 0);
	SendMessage(hwndList, CLM_SETINDENT, 10, 0);
	for (int i = 0; i <= FONTID_MAX; i++)
		SendMessage(hwndList, CLM_SETTEXTCOLOR, i, GetSysColor(COLOR_WINDOWTEXT));
	SetWindowLongPtr(hwndList, GWL_STYLE, GetWindowLongPtr(hwndList, GWL_STYLE) | CLS_SHOWHIDDEN);
}

INT_PTR CALLBACK OptPasswordsDlgProc(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	HTREEITEM hitmGlobal, hitmMaster;
	switch (msg) {
	case WM_INITDIALOG:
		HIMAGELIST himg;
		himg = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 3, 3);
		icoidNoPassword = ImageList_AddIcon(himg, GetIcon(ICO_NOPASSWORD));
		icoidPassword = ImageList_AddIcon(himg, GetIcon(ICO_PASSWORD));
		SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)himg);

		SendDlgItemMessage(hwnd, IDC_LIST, CLM_SETEXTRACOLUMNS, 1, 0);
		{
			CLCINFOITEM cii = { 0 };
			cii.cbSize = sizeof(cii);
			cii.flags = CLCIIF_GROUPFONT;

			cii.pszText = TranslateT("** Global **");
			hitmGlobal = (HTREEITEM)SendDlgItemMessage(hwnd, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			SendMessage(GetDlgItem(hwnd, IDC_LIST), CLM_SETEXTRAIMAGE, (WPARAM)hitmGlobal, MAKELPARAM(0, icoidNoPassword));

			cii.pszText = TranslateT("** Master **");
			hitmMaster = (HTREEITEM)SendDlgItemMessage(hwnd, IDC_LIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
			SendMessage(GetDlgItem(hwnd, IDC_LIST), CLM_SETEXTRAIMAGE, (WPARAM)hitmMaster, MAKELPARAM(0, icoidNoPassword));
		}
		SetAllContactIcons(GetDlgItem(hwnd, IDC_LIST));
		ResetListOptions(GetDlgItem(hwnd, IDC_LIST));

		SendMessage(GetDlgItem(hwnd, IDC_ICO_NOPASSWORD), STM_SETICON, (WPARAM)GetIcon(ICO_NOPASSWORD), 0);
		SendMessage(GetDlgItem(hwnd, IDC_ICO_PASSWORD), STM_SETICON, (WPARAM)GetIcon(ICO_PASSWORD), 0);

		SendMessage(GetDlgItem(hwnd, IDC_SAVEPASSWORD), BUTTONSETASFLATBTN, 0, 0);
		SendMessage(GetDlgItem(hwnd, IDC_SAVEPASSWORD), BUTTONADDTOOLTIP, (WPARAM)Translate("Save Password"), 0);
		SendMessage(GetDlgItem(hwnd, IDC_SAVEPASSWORD), BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIcon(ICO_SAVEPASS));
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				return TRUE;

			case PSN_APPLY:
				return TRUE;
			}
			break;

		case IDC_LIST:
			switch (((LPNMHDR)lParam)->code) {
			case CLN_NEWCONTACT:
			case CLN_LISTREBUILT:
				SetAllContactIcons(GetDlgItem(hwnd, IDC_LIST));
				break;
				//fall through
//						case CLN_CONTACTMOVED:
//							SetListGroupIcons(GetDlgItem(hwndDlg,IDC_LIST),(HANDLE)SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETNEXTITEM,CLGN_ROOT,0),hItemAll,NULL);
//							break;

			case CLN_OPTIONSCHANGED:
				ResetListOptions(GetDlgItem(hwnd, IDC_LIST));
				break;
				/*
										case NM_CLICK:
										{	HANDLE hItem;
											NMCLISTCONTROL *nm=(NMCLISTCONTROL*)lParam;
											DWORD hitFlags;
											int iImage;
											int itemType;

											// Make sure we have an extra column
											if (nm->iColumn == -1)
												break;

											// Find clicked item
											hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_HITTEST, (WPARAM)&hitFlags, MAKELPARAM(nm->pt.x,nm->pt.y));
											// Nothing was clicked
											if (hItem == NULL) break;
											// It was not a visbility icon
											if (!(hitFlags & CLCHT_ONITEMEXTRA)) break;

											// Get image in clicked column (0=none, 1=visible, 2=invisible)
											iImage = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, 0));
											if (iImage == 0)
												iImage=nm->iColumn + 1;
											else
												if (iImage == 1 || iImage == 2)
													iImage = 0;

											// Get item type (contact, group, etc...)
											itemType = SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETITEMTYPE, (WPARAM)hItem, 0);

											// Update list, making sure that the options are mutually exclusive
											if (itemType == CLCIT_CONTACT) { // A contact
												SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn, iImage));
												if (iImage && SendDlgItemMessage(hwndDlg,IDC_LIST,CLM_GETEXTRAIMAGE,(WPARAM)hItem,MAKELPARAM(nm->iColumn?0:1,0))!=0xFF)
													SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(nm->iColumn?0:1, 0));
											}
											else if (itemType == CLCIT_INFO) {	 // All Contacts
												SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
												if (iImage)
													SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn?0:1, 0);
											}
											else if (itemType == CLCIT_GROUP) { // A group
												hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
												if (hItem) {
													SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn, iImage);
													if (iImage)
														SetAllChildIcons(GetDlgItem(hwndDlg, IDC_LIST), hItem, nm->iColumn?0:1, 0);
												}
											}
											// Update the all/none icons
											SetListGroupIcons(GetDlgItem(hwndDlg, IDC_LIST), (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, CLM_GETNEXTITEM, CLGN_ROOT, 0), hItemAll, NULL);

											// Activate Apply button
											SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
											break;
										}
				*/
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}
