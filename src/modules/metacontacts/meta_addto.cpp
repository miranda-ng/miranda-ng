/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "..\..\core\commonheaders.h"

#include "metacontacts.h"

/** Adds all the metacontacts desired in the listview.
*
* Adds all the metacontacts present in the database in the list,
*
* @param list : HANDLE to the list which will contain the columns.
* @param nb_contacts : Number of loaded contacts.
* @param contacts : A list of the contacts' identifiers
*
* @param id : Reference to a list of the MetaContacts IDs loaded in the listview.
*             Since this list is resized, its address must be passed.
*
* @return An integer specifying the number of rows added in the list.
*/

static int FillList(HWND list, BOOL sort)
{
	int i = 0;

	// The DB is searched through, to get all the metacontacts
	for (MCONTACT hMetaUser = db_find_first(); hMetaUser; hMetaUser = db_find_next(hMetaUser)) {
		// if it's not a MetaContact, go to the next
		DBCachedContact *cc = CheckMeta(hMetaUser);
		if (cc == NULL)
			continue;

		// get contact display name from clist
		TCHAR *swzContactDisplayName = cli.pfnGetContactDisplayName(hMetaUser, 0);
		// don't insert huge strings that we have to compare with later
		if (mir_tstrlen(swzContactDisplayName) > 1023)
			swzContactDisplayName[1024] = 0;

		int pos = -1;
		if (sort) {
			for (pos = 0; pos < i; pos++) {
				TCHAR buff[1024];
				SendMessage(list, LB_GETTEXT, pos, (LPARAM)buff);
				if (mir_tstrcmp(buff, swzContactDisplayName) > 0)
					break;
			}
		}

		int index = SendMessage(list, LB_INSERTSTRING, pos, (LPARAM)swzContactDisplayName);
		SendMessage(list, LB_SETITEMDATA, index, hMetaUser);
		i++;
	}
	return i;
}

/** Build or update the list.
*
* @param list : HANDLE to the list which will contain the columns
* @param id : Reference to a list that will contain all the MetaContacts IDs loaded in the listview
*             otherwise the list is only refilled \n (Warning : this value must be
*             set to TRUE only once per Dialog display, otherwise all columns will be doubled)
*
* @returns An integer specifying the number of rows inserted or -1 if there was a problem
*/

static int BuildList(HWND list, BOOL sort)
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
	return FillList(list, sort);
}

/** Callback function for the <b>'Add To'</b> Dialog.
*
* All the UI is controlled here, from display to functionnalities.
*
* @param hwndDlg : HANDLE to the <b>'Add To'</b> Dialog.
* @param uMsg : Specifies the message received by this dialog.
* @param wParam : Specifies additional message-specific information. 
* @param lParam : Specifies additional message-specific information. 
*
* @return TRUE if the dialog processed the message, FALSE if it did not.
*/

#define szConvMsg LPGEN("Either there is no metacontact in the database (in this case you should first convert a contact into one)\n\
or there is none that can host this contact.\n\
Another solution could be to convert this contact into a new metacontact.\n\nConvert this contact into a new metacontact?")

static INT_PTR CALLBACK Meta_SelectDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			DBCachedContact *cc = currDb->m_cache->GetCachedContact(lParam);
			if (cc == NULL) {
				DestroyWindow(hwndDlg);
				return TRUE;
			}

			if (cc->IsMeta()) {
				MessageBox(hwndDlg,
							  TranslateT("This contact is a metacontact.\nYou can't add a metacontact to another metacontact.\n\nPlease choose another."),
							  TranslateT("Metacontact conflict"), MB_ICONERROR);
				DestroyWindow(hwndDlg);
				return TRUE;
			}

			if (cc->IsSub()) {
				MessageBox(hwndDlg,
							  TranslateT("This contact is already associated to a metacontact.\nYou cannot add a contact to multiple metacontacts."),
							  TranslateT("Multiple metacontacts"), MB_ICONERROR);
				DestroyWindow(hwndDlg);
				return TRUE;
			}
		}

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam); // user data is contact handle

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_ADD));

		// Initialize the graphical part
		CheckDlgButton(hwndDlg, IDC_ONLYAVAIL, BST_CHECKED); // Initially checked; display all metacontacts is only an option
		// Besides, we can check if there is at least one metacontact to add the contact to.
		if (BuildList(GetDlgItem(hwndDlg, IDC_METALIST), FALSE) <= 0) {
			if (MessageBox(hwndDlg, TranslateT(szConvMsg), TranslateT("No suitable metacontact found"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDYES)
				Meta_Convert(lParam, 0);
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		else {
			// get contact display name from clist
			TCHAR *ptszCDN = cli.pfnGetContactDisplayName(lParam, 0);
			if (!ptszCDN)
				ptszCDN = TranslateT("a contact");

			// ... and set it to the Window title.
			TCHAR buf[256];
			mir_sntprintf(buf, SIZEOF(buf), TranslateT("Adding %s..."), ptszCDN);
			SetWindowText(hwndDlg, buf);
		}
		ShowWindow(hwndDlg, SW_SHOWNORMAL);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_DBLCLK) // emulate click ok Ok
			wParam = MAKEWPARAM(IDOK, BN_CLICKED);

		if (HIWORD(wParam) != BN_CLICKED)
			break;	// Only clicks of buttons are relevant, let other COMMANDs through

		switch (LOWORD(wParam)) {
			case IDOK:
			{
				int item = SendDlgItemMessage(hwndDlg, IDC_METALIST, LB_GETCURSEL, 0, 0);	// Get the index of the selected metacontact
				if (item == -1)
					return IDOK == MessageBox(hwndDlg, TranslateT("Please select a metacontact"), TranslateT("No metacontact selected"), MB_ICONHAND);

				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				MCONTACT hMeta = (MCONTACT)SendDlgItemMessage(hwndDlg, IDC_METALIST, LB_GETITEMDATA, item, 0);
				if (!Meta_Assign(hContact, hMeta, FALSE))
					MessageBox(hwndDlg, TranslateT("Assignment to the metacontact failed."), TranslateT("Assignment failure"), MB_ICONERROR);
			}
			// fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			break;

		case IDC_CHK_SRT:
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_METALIST), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_METALIST), GWL_STYLE) ^ LBS_SORT);
			if (BuildList(GetDlgItem(hwndDlg, IDC_METALIST), IsDlgButtonChecked(hwndDlg, IDC_CHK_SRT) ? TRUE : FALSE) <= 0) {
				if (MessageBox(hwndDlg, TranslateT(szConvMsg), TranslateT("No suitable metacontact found"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDYES)
					Meta_Convert(lParam, 0);
				DestroyWindow(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_DESTROY:
		// Free all allocated memory and return the focus to the CList
		HWND clist = GetParent(hwndDlg);
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		EndDialog(hwndDlg, TRUE);
		SetFocus(clist);
		return TRUE;
	}
	return FALSE;	// All other Message are not handled
}

/** Display the <b>'Add to'</b> Dialog
*
* Present a dialog in which the user can choose to which MetaContact this
* contact will be added
*
* @param wParam : HANDLE to the contact that has been chosen.
* @param lParam :	Allways set to 0.
*/

INT_PTR Meta_AddTo(WPARAM hContact, LPARAM)
{
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_METASELECT), clui, &Meta_SelectDialogProc, hContact);
	return 0;
}
