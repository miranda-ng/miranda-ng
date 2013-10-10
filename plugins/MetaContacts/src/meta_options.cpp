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

/** @file meta_menu.c 
*
* Functions needed to handle MetaContacts.
* Centralizes functions called when the user chooses
* an option integrated in the context-menu of the \c CList.
*/

#include "metacontacts.h"

MetaOptions options;
MetaOptions options_changes;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	TCHAR buff[40];

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		options_changes = options;

		CheckDlgButton(hwndDlg, IDC_CHK_SETDEFAULTRECV, options_changes.set_default_on_recv ? TRUE : FALSE);
		hw = GetDlgItem(hwndDlg, IDC_CHK_TEMPDEFAULT);
		EnableWindow(hw, options_changes.set_default_on_recv);
		CheckDlgButton(hwndDlg, IDC_CHK_TEMPDEFAULT, options_changes.temp_default ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_ALWAYSUSEDEFAULT, options_changes.always_use_default ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SUPPRESSSTATUS, options_changes.suppress_status ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_RAD_UID, options_changes.menu_contact_label == DNT_UID);
		CheckDlgButton(hwndDlg, IDC_RAD_DID, options_changes.menu_contact_label == DNT_DID);

		CheckDlgButton(hwndDlg, IDC_RAD_MSG, options_changes.menu_function == FT_MSG);
		CheckDlgButton(hwndDlg, IDC_RAD_MENU, options_changes.menu_function == FT_MENU);
		CheckDlgButton(hwndDlg, IDC_RAD_INFO, options_changes.menu_function == FT_INFO);

		CheckDlgButton(hwndDlg, IDC_RAD_NICK, options_changes.clist_contact_name == CNNT_NICK);
		CheckDlgButton(hwndDlg, IDC_RAD_NAME, options_changes.clist_contact_name == CNNT_DISPLAYNAME);
		CheckDlgButton(hwndDlg, IDC_CHK_LOCKHANDLE, options_changes.lockHandle ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SUBWINDOW, options_changes.subcontact_windows ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_METAHISTORY, options_changes.metahistory ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SUBHISTORY, options_changes.subhistory ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_COPYDATA, options_changes.copydata ? TRUE : FALSE);

		if ( !options_changes.subcontact_windows) {
			hw = GetDlgItem(hwndDlg, IDC_CHK_METAHISTORY);
			EnableWindow(hw, FALSE);
		} else {
			hw = GetDlgItem(hwndDlg, IDC_CHK_SUBHISTORY);
			EnableWindow(hw, FALSE);
		}

		CheckDlgButton(hwndDlg, IDC_CHK_COPYHISTORY, options_changes.copy_subcontact_history ? TRUE : FALSE);
		hw = GetDlgItem(hwndDlg, IDC_ED_DAYS);
		_itot(options_changes.days_history, buff, 10);
		SetWindowText(hw, buff);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_SETDEFAULTRECV:
				options_changes.set_default_on_recv = IsDlgButtonChecked(hwndDlg, IDC_CHK_SETDEFAULTRECV);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_TEMPDEFAULT), options_changes.set_default_on_recv);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_TEMPDEFAULT:
				options_changes.temp_default = IsDlgButtonChecked(hwndDlg, IDC_CHK_TEMPDEFAULT);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_ALWAYSUSEDEFAULT:
				options_changes.always_use_default = IsDlgButtonChecked(hwndDlg, IDC_CHK_ALWAYSUSEDEFAULT);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_SUPPRESSSTATUS:
				options_changes.suppress_status = IsDlgButtonChecked(hwndDlg, IDC_CHK_SUPPRESSSTATUS);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_COPYHISTORY:
				options_changes.copy_subcontact_history = IsDlgButtonChecked(hwndDlg, IDC_CHK_COPYHISTORY);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_METAHISTORY:
				options_changes.metahistory = IsDlgButtonChecked(hwndDlg, IDC_CHK_METAHISTORY);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_SUBHISTORY:
				options_changes.subhistory = IsDlgButtonChecked(hwndDlg, IDC_CHK_SUBHISTORY);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_COPYDATA:
				options_changes.copydata = IsDlgButtonChecked(hwndDlg, IDC_CHK_COPYDATA);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_UID:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_UID)) {
					options_changes.menu_contact_label = DNT_UID;
					CheckDlgButton(hwndDlg, IDC_RAD_DID, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_DID:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_DID)) {
					options_changes.menu_contact_label = DNT_DID;
					CheckDlgButton(hwndDlg, IDC_RAD_UID, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_MSG:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MSG)) {
					options_changes.menu_function = FT_MSG;
					CheckDlgButton(hwndDlg, IDC_RAD_MENU, FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_INFO, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_MENU:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MENU)) {
					options_changes.menu_function = FT_MENU;
					CheckDlgButton(hwndDlg, IDC_RAD_MSG, FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_INFO, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_INFO:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_INFO)) {
					options_changes.menu_function = FT_INFO;
					CheckDlgButton(hwndDlg, IDC_RAD_MSG, FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_MENU, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_NICK:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_NICK)) {
					options_changes.clist_contact_name = CNNT_NICK;
					CheckDlgButton(hwndDlg, IDC_RAD_NAME, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_NAME:
				if (IsDlgButtonChecked(hwndDlg, IDC_RAD_NAME)) {
					options_changes.clist_contact_name = CNNT_DISPLAYNAME;
					CheckDlgButton(hwndDlg, IDC_RAD_NICK, FALSE);
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_CHK_SUBWINDOW:
				options_changes.subcontact_windows = IsDlgButtonChecked(hwndDlg, IDC_CHK_SUBWINDOW);

				if (options_changes.subcontact_windows) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_METAHISTORY), TRUE);
					CheckDlgButton(hwndDlg, IDC_CHK_SUBHISTORY, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SUBHISTORY), FALSE);
					options_changes.subhistory = TRUE;
				} else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_SUBHISTORY), TRUE);
					CheckDlgButton(hwndDlg, IDC_CHK_METAHISTORY, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_METAHISTORY), FALSE);
					options_changes.metahistory = TRUE;
				}
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_CHK_LOCKHANDLE:
				options_changes.lockHandle = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOCKHANDLE);
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
			hw = GetDlgItem(hwndDlg, IDC_ED_DAYS);
			GetWindowText(hw, buff, SIZEOF(buff));
			if (buff[0] != 0)
				options_changes.days_history = _ttoi(buff);

			options = options_changes;
			Meta_WriteOptions(&options);

			Meta_SuppressStatus(options.suppress_status);
			Meta_SetAllNicks();
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int Meta_WriteOptions(MetaOptions *opt)
{
	db_set_b(NULL, META_PROTO, "SetDefaultOnRecv", (BYTE)(opt->set_default_on_recv ? 1 : 0));
	db_set_b(NULL, META_PROTO, "TempDefault", (BYTE)(opt->temp_default ? 1 : 0));
	db_set_b(NULL, META_PROTO, "AlwaysUseDefault", (BYTE)(opt->always_use_default ? 1 : 0));
	db_set_b(NULL, META_PROTO, "SuppressStatus", (BYTE)(opt->suppress_status ? 1 : 0));
	db_set_w(NULL, META_PROTO, "MenuContactLabel", (WORD)opt->menu_contact_label);
	db_set_w(NULL, META_PROTO, "MenuContactFunction", (WORD)opt->menu_function);
	db_set_w(NULL, META_PROTO, "CListContactName", (WORD)opt->clist_contact_name);
	db_set_b(NULL, META_PROTO, "CopyHistory", (BYTE)(opt->copy_subcontact_history ? 1 : 0));
	db_set_dw(NULL, META_PROTO, "DaysHistory", (DWORD)(opt->days_history));
	db_set_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", (DWORD)(opt->set_status_from_offline_delay));
	db_set_b(NULL, META_PROTO, "SubcontactWindows", (BYTE)(opt->subcontact_windows ? 1 : 0));
	db_set_b(NULL, META_PROTO, "CopyData", (BYTE)(opt->copydata ? 1 : 0));
	db_set_b(NULL, META_PROTO, "LockHandle", (BYTE)(opt->lockHandle ? 1 : 0));
	db_set_b(NULL, META_PROTO, "MetaMessageIcon", (BYTE)(opt->flash_meta_message_icon ? 1 : 0));
	db_set_b(NULL, META_PROTO, "CopyUserInfo", (BYTE)(opt->copy_userinfo ? 1 : 0));

	if ( !opt->subcontact_windows)
		db_set_b(NULL, META_PROTO, "MetaHistory", 1);
	else
		db_set_b(NULL, META_PROTO, "MetaHistory", (BYTE)(opt->metahistory ? 1 : 0));

	if (opt->subcontact_windows)
		db_set_b(NULL, META_PROTO, "SubcontactHistory", 1);
	else
		db_set_b(NULL, META_PROTO, "SubcontactHistory", (BYTE)(opt->subhistory ? 1 : 0));
	return 0;

	db_set_b(NULL, META_PROTO, "UseProtoRecv", (BYTE)(opt->use_proto_recv ? 1 : 0));
}

int Meta_ReadOptions(MetaOptions *opt)
{
	opt->set_default_on_recv = (db_get_b(NULL, META_PROTO, "SetDefaultOnRecv", 1) == 1 ? TRUE : FALSE);
	opt->temp_default = (db_get_b(NULL, META_PROTO, "TempDefault", 1) == 1 ? TRUE : FALSE);
	opt->always_use_default = (db_get_b(NULL, META_PROTO, "AlwaysUseDefault", 1) == 1 ? TRUE : FALSE);
	opt->suppress_status = (db_get_b(NULL, META_PROTO, "SuppressStatus", 1) == 1 ? TRUE : FALSE);
	opt->menu_contact_label = (int)db_get_w(NULL, META_PROTO, "MenuContactLabel", DNT_UID);
	opt->menu_function = (int)db_get_w(NULL, META_PROTO, "MenuContactFunction", FT_MENU);
	opt->clist_contact_name = (int)db_get_w(NULL, META_PROTO, "CListContactName", CNNT_NICK);
	opt->copy_subcontact_history = (db_get_b(NULL, META_PROTO, "CopyHistory", 1) == 1 ? TRUE : FALSE);
	opt->days_history = (int)db_get_dw(NULL, META_PROTO, "DaysHistory", 0);
	opt->set_status_from_offline_delay = (int)db_get_dw(NULL, META_PROTO, "SetStatusFromOfflineDelay", DEFAULT_SET_STATUS_SLEEP_TIME);
	opt->subcontact_windows = (db_get_b(NULL, META_PROTO, "SubcontactWindows", 0) == 1 ? TRUE : FALSE);
	opt->copydata = (db_get_b(NULL, META_PROTO, "CopyData", 1) == 1 ? TRUE : FALSE);
	opt->lockHandle = (db_get_b(NULL, META_PROTO, "LockHandle", 0) == 1 ? TRUE : FALSE);
	opt->flash_meta_message_icon = (db_get_b(NULL, META_PROTO, "MetaMessageIcon", 1) == 1 ? TRUE : FALSE);
	opt->copy_userinfo = (db_get_b(NULL, META_PROTO, "CopyUserInfo", 1) == 1 ? TRUE : FALSE);

	if ( !opt->subcontact_windows)
		opt->metahistory = TRUE;
	else
		opt->metahistory = (db_get_b(NULL, META_PROTO, "MetaHistory", 1) == 1 ? TRUE : FALSE);

	if (opt->subcontact_windows)
		opt->subhistory = TRUE;
	else
		opt->subhistory = (db_get_b(NULL, META_PROTO, "SubcontactHistory", 1) == 1 ? TRUE : FALSE);

	opt->use_proto_recv = (db_get_b(NULL, META_PROTO, "UseProtoRecv", 1) == 1 ? TRUE : FALSE);
	return 0;
}

/*
#define ID_STATUS_OFFLINE               40071		->8
#define ID_STATUS_ONLINE                40072		->0
#define ID_STATUS_AWAY                  40073		->4
#define ID_STATUS_DND                   40074		->7
#define ID_STATUS_NA                    40075		->6
#define ID_STATUS_OCCUPIED              40076		->5
#define ID_STATUS_FREECHAT              40077		->1
#define ID_STATUS_INVISIBLE             40078		->0
#define ID_STATUS_ONTHEPHONE            40079		->2
#define ID_STATUS_OUTTOLUNCH            40080		->3
*/

int GetDefaultPrio(int status)
{
	switch( status ) {
		case ID_STATUS_OFFLINE:    return 8;
		case ID_STATUS_AWAY:       return 4;
		case ID_STATUS_DND:        return 7;
		case ID_STATUS_NA:         return 6;
		case ID_STATUS_OCCUPIED:   return 5;
		case ID_STATUS_FREECHAT:   return 1;
		case ID_STATUS_ONTHEPHONE: return 2;
		case ID_STATUS_OUTTOLUNCH: return 3;
	}

	return 0;
}

typedef struct {
	int prio[10]; // priority for each status
	BOOL def[10]; // use default for this one?
} ProtoStatusPrio;

ProtoStatusPrio *priorities = 0;

int GetRealPriority(char *proto, int status)
{
	char szSetting[256];
	if ( !proto) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", status);
		return db_get_w(0, META_PROTO, szSetting, GetDefaultPrio(status));
	}

	mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", proto, status);
	int prio = db_get_w(0, META_PROTO, szSetting, 0xFFFF);
	if (prio == 0xFFFF) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", status);
		return db_get_w(0, META_PROTO, szSetting, GetDefaultPrio(status));
	}
	return prio;
}

void ReadPriorities()
{
	char szSetting[256];
	int num_protocols, i, j;
	PROTOACCOUNT **pppDesc;
	ProtoEnumAccounts(&num_protocols, &pppDesc);

	ProtoStatusPrio *current = priorities = (ProtoStatusPrio *)malloc((num_protocols + 1) * sizeof(ProtoStatusPrio));
	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", i);
		current->def[i - ID_STATUS_OFFLINE] = TRUE;
		current->prio[i - ID_STATUS_OFFLINE] = db_get_w(0, META_PROTO, szSetting, GetDefaultPrio(i));
	}

	for (i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for (j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", pppDesc[i]->szModuleName, j);
			current->prio[j - ID_STATUS_OFFLINE] = db_get_w(0, META_PROTO, szSetting, 0xFFFF);
			current->def[j - ID_STATUS_OFFLINE] = (current->prio[j - ID_STATUS_OFFLINE] == 0xFFFF);
		}
	}
}

void WritePriorities()
{
	char szSetting[256];
	ProtoStatusPrio *current = priorities;
	int i, j;

	int num_protocols;
	PROTOACCOUNT **pppDesc;
	ProtoEnumAccounts(&num_protocols, &pppDesc);

	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", i);
		if (current->prio[i - ID_STATUS_OFFLINE] != GetDefaultPrio(i))
			db_set_w(0, META_PROTO, szSetting, (WORD)current->prio[i - ID_STATUS_OFFLINE]);
		else
			db_unset(0, META_PROTO, szSetting);
	}
	for (i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for (j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", pppDesc[i]->szModuleName, j);
			if ( !current->def[j - ID_STATUS_OFFLINE])
				db_set_w(0, META_PROTO, szSetting, (WORD)current->prio[j - ID_STATUS_OFFLINE]);
			else
				db_unset(0, META_PROTO, szSetting);
		}
	}
}

int GetIsDefault(int proto_index, int status)
{
	return (priorities + (proto_index + 1))->def[status - ID_STATUS_OFFLINE];
}

BOOL GetPriority(int proto_index, int status)
{
	if (proto_index == -1)
		return priorities->prio[status - ID_STATUS_OFFLINE];

	ProtoStatusPrio *current = priorities + (proto_index + 1);
	if (current->def[status - ID_STATUS_OFFLINE])
		current = priorities;

	return current->prio[status - ID_STATUS_OFFLINE];
}

void SetPriority(int proto_index, int status, BOOL def, int prio)
{
	if (prio < 0) prio = 0;
	if (prio > 500) prio = 500;
	if (proto_index == -1)
		priorities->prio[status - ID_STATUS_OFFLINE] = prio;
	else {
		ProtoStatusPrio *current = priorities + (proto_index + 1);
		current->def[status - ID_STATUS_OFFLINE] = def;
		if ( !def)
			current->prio[status - ID_STATUS_OFFLINE] = prio;
	}
}

void ResetPriorities()
{
	int i, j, num_protocols;
	PROTOACCOUNT **pppDesc;
	ProtoEnumAccounts(&num_protocols, &pppDesc);

	ProtoStatusPrio *current = priorities;
	for (i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		current->def[i - ID_STATUS_OFFLINE] = TRUE;
		current->prio[i - ID_STATUS_OFFLINE] = GetDefaultPrio(i);
	}

	for (i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for (j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			current->def[j - ID_STATUS_OFFLINE] = TRUE;
		}
	}
}

#define WMU_FILLSTATUSCMB		(WM_USER + 0x100)
#define WMU_FILLPRIODATA		(WM_USER + 0x101)

INT_PTR CALLBACK DlgProcOptsPriorities(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	int sel;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		ReadPriorities();
		{
			int num_protocols;
			PROTOACCOUNT **pppDesc;
			ProtoEnumAccounts(&num_protocols, &pppDesc);

			hw = GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL);
			int index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<default>"));
			SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, -1);
			for (int i = 0; i < num_protocols; i++) {
				if (strcmp(pppDesc[i]->szModuleName, META_PROTO) != 0) {
					index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->tszAccountName);
					SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
				}
			}

			SendMessage(hw, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
			SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
		}
		return FALSE;

	case WMU_FILLPRIODATA:
		sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
		if (sel != -1) {
			int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
			sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
			if (sel != -1) {
				int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
				SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
				if (index == -1) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), TRUE);
					CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DEFAULT), FALSE);
				}
				else {
					if (GetIsDefault(index, status)) {
						CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), FALSE);
					}
					else {
						CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, FALSE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), TRUE);
					}

					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DEFAULT), TRUE);
				}
			}
		}
		return TRUE;

	case WMU_FILLSTATUSCMB:
		sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
		if (sel != -1) {
			int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
			HWND hw = GetDlgItem(hwndDlg, IDC_CMB_STATUS);
			SendMessage(hw, CB_RESETCONTENT, 0, 0);
			if (index == -1) {
				for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
					index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pcli->pfnGetStatusModeDescription(i, 0));
					SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
				}
			}
			else {
				int num_protocols;
				PROTOACCOUNT **pppDesc;
				ProtoEnumAccounts(&num_protocols, &pppDesc);

				int caps = CallProtoService(pppDesc[index]->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);

				for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
					if (caps & Proto_Status2Flag(i)) {
						index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pcli->pfnGetStatusModeDescription(i, 0));
						SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
					}
				}
			}
			SendMessage(hw, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
		}
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_DEFAULT:
				sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
				if (sel != -1) {
					int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
					sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
					if (sel != -1) {
						BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_CHK_DEFAULT);
						int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
						if (checked) {
							SetPriority(index, status, TRUE, 0);
							SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
						}
						else SetPriority(index, status, FALSE, GetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, 0, FALSE));

						EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), !checked);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), !checked);
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;

			case IDC_BTN_RESET:
				ResetPriorities();
				SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_SETCURSEL, 0, 0);
				SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
				SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}

		if ( HIWORD( wParam ) == EN_CHANGE && LOWORD(wParam) == IDC_ED_PRIORITY && ( HWND )lParam == GetFocus()) {
			sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
			if (sel != -1) {
				int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
				sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
				if (sel != -1) {
					int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
					int prio = GetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, 0, FALSE);
					SetPriority(index, status, FALSE, prio);
					if (prio != GetPriority(index, status))
						SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
					SendMessage( GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
		}
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			switch( LOWORD( wParam )) {
			case IDC_CMB_STATUS:
				SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
				break;
			case IDC_CMB_PROTOCOL:
				SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			WritePriorities();
			return TRUE;
		}
		break;
	case WM_DESTROY:
		free(priorities);
		priorities = 0;
		break;
	}

	return FALSE;
}
