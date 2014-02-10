/*
MetaContacts Plugin for Miranda IM.

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

/** @file edit.c 
*
* Functions for the <b>'Edit'</b> Dialog.
* Contains all the functions and all the structures
* needed to display and control the <b>'Edit'</b> Dialog.
*/

#include "metacontacts.h"

//! Holds the differents changes that have to made
typedef struct tag_CHANGES {
	MCONTACT hMeta;							//!< \c HANDLE of the MetaContact that is edited.
	MCONTACT hDefaultContact;					//!< \c HANDLE of the new default contact
	MCONTACT hOfflineContact;
	int num_deleted,						//!< \c DWORD number of deleted contacts
		num_contacts;						//!< \c DWORD number of contacts
	MCONTACT hDeletedContacts[MAX_CONTACTS];	//!< \c HANDLEs of the subcontacts to be removed from this metacontact
	MCONTACT hContact[MAX_CONTACTS];			//!< \c HANDLEs of the subcontacts, in the order they should be in
	int force_default;
} CHANGES;

CHANGES changes;							//!< \c global CHANGES structure

/** Fills the list of contacts
*
* @param chg : Structure holding all the change info (See CHANGES).
*/

void FillContactList(HWND hWndDlg, CHANGES *chg)
{
	HWND hList = GetDlgItem(hWndDlg, IDC_LST_CONTACTS);
	TCHAR buff[256];

	SendMessage(hList, LVM_DELETEALLITEMS, 0, 0);

	LVITEM LvItem = { 0 };
	LvItem.mask = LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of test

	for (int i = 0; i < chg->num_contacts; i++)  {
		LvItem.iItem = i;
	
		TCHAR *ptszCDN = pcli->pfnGetContactDisplayName(chg->hContact[i], GCDNF_TCHAR);
		if (ptszCDN == NULL)
			ptszCDN = TranslateT("(Unknown Contact)");

		LvItem.iSubItem = 0; // clist display name
		LvItem.pszText = ptszCDN;
		ListView_InsertItem(hList, &LvItem);

		LvItem.iSubItem = 1; // id
		char *szProto = GetContactProto(chg->hContact[i]);
		if (szProto) {
			char *szField = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

			DBVARIANT dbv;
			if ( !db_get(chg->hContact[i], szProto, szField, &dbv)) {
				switch(dbv.type) {
				case DBVT_ASCIIZ:
					_tcsncpy(buff, _A2T(dbv.pszVal), SIZEOF(buff));
					break;
				case DBVT_WCHAR:
					_tcsncpy(buff, dbv.ptszVal, SIZEOF(buff));
					break;
				case DBVT_BYTE:
					_itot(dbv.bVal, buff, 10);
					break;
				case DBVT_WORD:
					_itot(dbv.wVal, buff, 10);
					break;
				case DBVT_DWORD:
					_itot(dbv.dVal, buff, 10);
					break;
				default:
					buff[0] = 0;
				}
				db_free(&dbv);
			}
			else buff[0] = 0;

			LvItem.pszText = buff;
			SendMessage(hList,LVM_SETITEM,0,(LPARAM)&LvItem); // Enter text to SubItems

			LvItem.iSubItem = 2; // protocol
			_tcsncpy(buff, _A2T(szProto), SIZEOF(buff));
			ListView_SetItem(hList, &LvItem);
		}
		else {
			LvItem.pszText = _T("Unknown");
			ListView_SetItem(hList, &LvItem);

			LvItem.iSubItem = 2; // protocol
			ListView_SetItem(hList, &LvItem);
		}
		LvItem.iSubItem = 3; // Default (Yes/No)
		LvItem.pszText = (chg->hContact[i] == chg->hDefaultContact ? TranslateT("Yes") : TranslateT("No"));
		ListView_SetItem(hList, &LvItem);

		LvItem.iSubItem = 4; // Offline (Yes/No)
		LvItem.pszText = (chg->hContact[i] == chg->hOfflineContact ? TranslateT("Yes") : TranslateT("No"));
		ListView_SetItem(hList, &LvItem);
	}
}

void SetListSelection(HWND hList, int sel)
{
	ListView_SetItemState(hList, sel, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
}

/** Scans the \c CHANGES and call the appropriate function for each change.
*
* @param chg : Structure holding all the change info (See CHANGES).
*/

void ApplyChanges(CHANGES *chg)
{
	int i;

	// remove removed contacts
	for (i = 0; i < chg->num_deleted; i++) {
		Meta_Delete((WPARAM)chg->hDeletedContacts[i], 0);
		if (chg->hDeletedContacts[i] == chg->hDefaultContact)
			chg->hDefaultContact = 0;
		if (chg->hDeletedContacts[i] == chg->hOfflineContact)
			chg->hOfflineContact = 0;
	}

	// set contact positions
	for (i = 0; i < chg->num_contacts; i++) {
		if (Meta_GetContactNumber(chg->hContact[i]) != i)
			Meta_SwapContacts(chg->hMeta, Meta_GetContactNumber(chg->hContact[i]), i);
	}

	NotifyEventHooks(hSubcontactsChanged, (WPARAM)chg->hMeta, (LPARAM)chg->hDefaultContact);

	// set default
	if (chg->hDefaultContact)
		db_set_dw(chg->hMeta, META_PROTO, "Default", Meta_GetContactNumber(chg->hDefaultContact));
	else
		db_set_dw(chg->hMeta, META_PROTO, "Default", 0);
	NotifyEventHooks(hEventDefaultChanged, (WPARAM)chg->hMeta, (LPARAM)chg->hDefaultContact);

	// set offline
	if (chg->hOfflineContact)
		db_set_dw(chg->hMeta, META_PROTO, "OfflineSend", Meta_GetContactNumber(chg->hOfflineContact));
	else
		db_set_dw(chg->hMeta, META_PROTO, "OfflineSend", (DWORD)-1);

	// fix nick
	MCONTACT most_online = Meta_GetMostOnline(chg->hMeta);
	Meta_CopyContactNick(chg->hMeta, most_online);

	// fix status
	Meta_FixStatus(chg->hMeta);

	// fix avatar
	most_online = Meta_GetMostOnlineSupporting(chg->hMeta, PFLAGNUM_4, PF4_AVATARS);
	if (most_online) {
		PROTO_AVATAR_INFORMATIONT AI;

		AI.cbSize = sizeof(AI);
		AI.hContact = chg->hMeta;
		AI.format = PA_FORMAT_UNKNOWN;
		_tcscpy(AI.filename, _T("X"));

		if ((int)CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
	        db_set_ts(chg->hMeta, "ContactPhoto", "File",AI.filename);
	}

	if (MetaAPI_GetForceState((WPARAM)chg->hMeta, 0) != chg->force_default)
		MetaAPI_ForceDefault((WPARAM)chg->hMeta, 0);
}

LRESULT ProcessCustomDraw (LPARAM lParam)
{
    LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
	 switch(lplvcd->nmcd.dwDrawStage) {
	 case CDDS_PREPAINT : //Before the paint cycle begins
		 //request notifications for individual listview items
		 return CDRF_NOTIFYITEMDRAW;

	 case CDDS_ITEMPREPAINT: //Before an item is drawn
		 if (changes.hContact[(int)lplvcd->nmcd.dwItemSpec] == changes.hDefaultContact) {
			 lplvcd->clrText = RGB(255, 0, 0);
		 }
		 return CDRF_NEWFONT;
	 }

	 return 0;
}

/** Callback function for the <b>'Edit'</b> Dialog.
*
* All the UI is controlled here, from display to functionnalities.
*
* @param hwndDlg :	\c HANDLE to the <b>'Edit'</b> \c Dialog.
* @param uMsg :		Specifies the message received by this dialog.
* @param wParam :	Specifies additional message-specific information. 
* @param lParam :	Specifies additional message-specific information (handle of MetaContact to edit)
*
* @return			\c TRUE if the dialog processed the message, \c FALSE if it did not.
*/

#define WMU_SETTITLE		(WM_USER + 1)

INT_PTR CALLBACK Meta_EditDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	int sel, i;

	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_EDIT));
		{	
			// Disable the 'Apply' button.
			EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),FALSE);

			hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
			ListView_SetExtendedListViewStyle(hwnd, LVS_EX_FULLROWSELECT);

			// Create list columns
			LVCOLUMN LvCol = { 0 };
			LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

			LvCol.pszText = TranslateT("Contact");
			LvCol.cx = 100;
			ListView_InsertColumn(hwnd, 0, &LvCol);

			LvCol.pszText = TranslateT("ID");
			LvCol.cx = 130;
			ListView_InsertColumn(hwnd, 1, &LvCol);

			LvCol.pszText = TranslateT("Protocol");
			LvCol.cx = 100;
			ListView_InsertColumn(hwnd, 2, &LvCol);

			LvCol.pszText = TranslateT("Default");
			LvCol.cx = 60;
			ListView_InsertColumn(hwnd, 3, &LvCol);

			LvCol.pszText = TranslateT("Send Offline");
			LvCol.cx = 85;
			ListView_InsertColumn(hwnd, 4, &LvCol);

			// disable buttons until a selection is made in the list
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REM), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), FALSE);

			int nb_contacts = db_get_dw(lParam, META_PROTO, "NumContacts", 0);
			int default_contact_number = db_get_dw(lParam, META_PROTO, "Default", (DWORD)-1);
			int offline_contact_number = db_get_dw(lParam, META_PROTO, "OfflineSend", (DWORD)-1);

			changes.hMeta = lParam;
			changes.num_contacts = nb_contacts;
			changes.num_deleted = 0;
			changes.hDefaultContact = Meta_GetContactHandle(lParam, default_contact_number);
			changes.hOfflineContact = Meta_GetContactHandle(lParam, offline_contact_number);
			for (i = 0; i < nb_contacts; i++)
				changes.hContact[i] = Meta_GetContactHandle(lParam, i);
			changes.force_default = MetaAPI_GetForceState((WPARAM)lParam, 0);

			SendMessage(hwndDlg, WMU_SETTITLE, 0, lParam);

			CheckDlgButton(hwndDlg, IDC_CHK_FORCEDEFAULT, changes.force_default);

			FillContactList(hwndDlg, &changes);
		}
		return TRUE;

	case WMU_SETTITLE:
		{
			TCHAR *ptszCDN = pcli->pfnGetContactDisplayName(lParam, GCDNF_TCHAR);
			if (ptszCDN == NULL)
				ptszCDN = TranslateT("(Unknown Contact)");

			SetWindowText(GetDlgItem(hwndDlg, IDC_ED_NAME), ptszCDN);
		}
		return TRUE;

	case WM_NOTIFY: // the message that is being sent always
		switch(LOWORD(wParam)) { // hit control
		case IDC_LST_CONTACTS:      // did we hit our ListView contorl?
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LST_CONTACTS), -1, LVNI_FOCUSED|LVNI_SELECTED); // return item selected

				// enable buttons
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REM), sel != -1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT), sel != -1 && changes.hContact[sel] != changes.hDefaultContact);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE), sel != -1 && changes.hContact[sel] != changes.hOfflineContact);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), sel > 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), (sel != -1 && sel < changes.num_contacts - 1));
			}					
		}
		break;

	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case BN_CLICKED:	// A button ('Remove', 'OK', 'Cancel' or 'Apply', normally) has been clicked
			switch(LOWORD(wParam)) {
			case IDC_VALIDATE: // Apply changes, if there is still one contact attached to the metacontact.
				if (changes.num_contacts == 0) { // Otherwise, delete the metacontact.
					if (IDYES == MessageBox(hwndDlg, TranslateT(szDelMsg), TranslateT("Delete MetaContact?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1)) {
						Meta_Delete((WPARAM)changes.hMeta, 0);
						DestroyWindow(hwndDlg);
					}
					return TRUE;
				}
				ApplyChanges(&changes);

				// Disable the 'Apply' button.
				EnableWindow(GetDlgItem(hwndDlg, IDC_VALIDATE), FALSE);
				break;

			case IDOK:
				if ( IsWindowEnabled(GetDlgItem(hwndDlg, IDC_VALIDATE))) { // If there are changes that could be made,
					if (changes.num_contacts == 0) { // do the work that would have be done if the 'Apply' button was clicked.
						if (IDYES == MessageBox(hwndDlg, TranslateT(szDelMsg), TranslateT("Delete MetaContact?"), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1)) {
							Meta_Delete((WPARAM)changes.hMeta,0);
							DestroyWindow(hwndDlg);
						}
						return TRUE;
					}
					ApplyChanges(&changes);
				}
				EndDialog(hwndDlg, IDOK);
				return TRUE;

			case IDCANCEL:	// Simply close the dialog
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;

			case IDC_BTN_SETDEFAULT:
				hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				sel = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED);
				InvalidateRect(hwnd, 0, TRUE);
				changes.hDefaultContact = changes.hContact[sel];
				SendMessage(hwndDlg, WMU_SETTITLE, 0, (LPARAM)changes.hContact[sel]);

				FillContactList(hwndDlg, &changes);
				SetListSelection(hwnd, sel);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BTN_SETDEFAULT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;

			case IDC_BTN_SETOFFLINE:
				hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				sel = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED);
				InvalidateRect(hwnd, 0, TRUE);
				changes.hOfflineContact = changes.hContact[sel];

				FillContactList(hwndDlg, &changes);
				SetListSelection(hwnd, sel);
				EnableWindow(GetDlgItem(hwndDlg,IDC_BTN_SETOFFLINE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;

			case IDC_BTN_REM:
				hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				sel = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED);
				changes.num_contacts--;
				changes.hDeletedContacts[changes.num_deleted++] = changes.hContact[sel];
				if (changes.hDefaultContact == changes.hContact[sel]) {
					if (changes.num_contacts > 0) {
						changes.hDefaultContact = changes.hContact[0];
						SetWindowText(GetDlgItem(hwndDlg,IDC_ED_DEFAULT), pcli->pfnGetContactDisplayName(changes.hDefaultContact, GCDNF_TCHAR));
					}
					else {
						changes.hDefaultContact = 0;
						SetWindowText(GetDlgItem(hwndDlg,IDC_ED_DEFAULT), _T("None"));
					}
				}

				for (i = sel; i < changes.num_contacts; i++)
					changes.hContact[i] = changes.hContact[i + 1];
				FillContactList(hwndDlg, &changes);

				// disable buttons
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_REM), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SETOFFLINE), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), FALSE);

				// Enable the 'Apply' button.
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;

			case IDC_BTN_UP:
				hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				sel = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED);
				{
					MCONTACT temp = changes.hContact[sel];
					changes.hContact[sel] = changes.hContact[sel - 1];
					changes.hContact[sel-1] = temp;
				}
				FillContactList(hwndDlg, &changes);
				sel--;
				SetListSelection(hwnd, sel);

				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), (sel > 0));
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), (sel < changes.num_contacts - 1));
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;

			case IDC_BTN_DOWN:
				hwnd = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				sel = ListView_GetNextItem(hwnd, -1, LVNI_FOCUSED | LVNI_SELECTED);
				{
					MCONTACT temp = changes.hContact[sel];
					changes.hContact[sel] = changes.hContact[sel + 1];
					changes.hContact[sel + 1] = temp;
				}
				FillContactList(hwndDlg, &changes);
				sel++;
				SetListSelection(hwnd, sel);

				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UP), (sel > 0));
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DOWN), (sel < changes.num_contacts - 1));
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;

			case IDC_CHK_FORCEDEFAULT:
				changes.force_default = IsDlgButtonChecked(hwndDlg, IDC_CHK_FORCEDEFAULT);
				EnableWindow(GetDlgItem(hwndDlg,IDC_VALIDATE),TRUE);
				return TRUE;
			}
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
		EndDialog(hwndDlg, IDCANCEL);
		break;
	}

	return FALSE;
}
