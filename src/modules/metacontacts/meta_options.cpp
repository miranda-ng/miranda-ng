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

MetaOptions options;
MetaOptions options_changes;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		options_changes = options;

		CheckDlgButton(hwndDlg, IDC_CHK_SUPPRESSSTATUS, options_changes.bSuppressStatus ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_RAD_UID, options_changes.menu_contact_label == DNT_UID ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_DID, options_changes.menu_contact_label == DNT_DID ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_RAD_MSG, options_changes.menu_function == FT_MSG ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_MENU, options_changes.menu_function == FT_MENU ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_INFO, options_changes.menu_function == FT_INFO ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_RAD_NICK, options_changes.clist_contact_name == CNNT_NICK ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_NAME, options_changes.clist_contact_name == CNNT_DISPLAYNAME ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LOCKHANDLE, options_changes.bLockHandle ? BST_CHECKED : BST_UNCHECKED);
		{
			TCHAR buff[40];
			_itot(options_changes.days_history, buff, SIZEOF(buff));
			SetDlgItemText(hwndDlg, IDC_ED_DAYS, buff);
		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_CHK_SUPPRESSSTATUS:
				options_changes.bSuppressStatus = IsDlgButtonChecked(hwndDlg, IDC_CHK_SUPPRESSSTATUS) != 0;
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_UID:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_UID)) {
					options_changes.menu_contact_label = DNT_UID;
					CheckDlgButton(hwndDlg, IDC_RAD_DID, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_DID:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_DID)) {
					options_changes.menu_contact_label = DNT_DID;
					CheckDlgButton(hwndDlg, IDC_RAD_UID, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_MSG:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MSG)) {
					options_changes.menu_function = FT_MSG;
					CheckDlgButton(hwndDlg, IDC_RAD_MENU, BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_RAD_INFO, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_MENU:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MENU)) {
					options_changes.menu_function = FT_MENU;
					CheckDlgButton(hwndDlg, IDC_RAD_MSG, BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_RAD_INFO, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_INFO:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_INFO)) {
					options_changes.menu_function = FT_INFO;
					CheckDlgButton(hwndDlg, IDC_RAD_MSG, BST_UNCHECKED);
					CheckDlgButton(hwndDlg, IDC_RAD_MENU, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_NICK:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_NICK)) {
					options_changes.clist_contact_name = CNNT_NICK;
					CheckDlgButton(hwndDlg, IDC_RAD_NAME, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_NAME:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_NAME)) {
					options_changes.clist_contact_name = CNNT_DISPLAYNAME;
					CheckDlgButton(hwndDlg, IDC_RAD_NICK, BST_UNCHECKED);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_CHK_LOCKHANDLE:
				options_changes.bLockHandle = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOCKHANDLE) != 0;
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}
		else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			TCHAR buff[40];
			GetDlgItemText(hwndDlg, IDC_ED_DAYS, buff, SIZEOF(buff));
			if (buff[0] != 0)
				options_changes.days_history = _ttoi(buff);

			options = options_changes;
			Meta_WriteOptions(&options);

			Meta_SuppressStatus(options.bSuppressStatus);
			Meta_SetAllNicks();
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int Meta_WriteOptions(MetaOptions *opt)
{
	db_set_b(NULL, META_PROTO, "SuppressStatus", opt->bSuppressStatus);
	db_set_w(NULL, META_PROTO, "MenuContactLabel", (WORD)opt->menu_contact_label);
	db_set_w(NULL, META_PROTO, "MenuContactFunction", (WORD)opt->menu_function);
	db_set_w(NULL, META_PROTO, "CListContactName", (WORD)opt->clist_contact_name);
	db_set_dw(NULL, META_PROTO, "DaysHistory", (DWORD)(opt->days_history));
	db_set_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", (DWORD)(opt->set_status_from_offline_delay));
	db_set_b(NULL, META_PROTO, "LockHandle", opt->bLockHandle);
	return 0;
}

int Meta_ReadOptions(MetaOptions *opt)
{
	db_mc_enable(db_get_b(NULL, META_PROTO, "Enabled", true) != 0);
	opt->bSuppressStatus = db_get_b(NULL, META_PROTO, "SuppressStatus", true) != 0;
	opt->menu_contact_label = (int)db_get_w(NULL, META_PROTO, "MenuContactLabel", DNT_UID);
	opt->menu_function = (int)db_get_w(NULL, META_PROTO, "MenuContactFunction", FT_MENU);
	opt->clist_contact_name = (int)db_get_w(NULL, META_PROTO, "CListContactName", CNNT_DISPLAYNAME);
	opt->days_history = (int)db_get_dw(NULL, META_PROTO, "DaysHistory", 0);
	opt->set_status_from_offline_delay = (int)db_get_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", DEFAULT_SET_STATUS_SLEEP_TIME);
	opt->bLockHandle = db_get_b(NULL, META_PROTO, "LockHandle", false) != 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int Meta_OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_METAOPTIONS);
	odp.pszTitle = LPGEN("Metacontacts");
	odp.pszGroup = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}
