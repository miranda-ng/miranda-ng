/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci
Based on work (C) Heiko Schillinger

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;

// Functions //////////////////////////////////////////////////////////////////////////////////////

// Initializations needed by options
void LoadOptions()
{
	opts.last_sent_enable = g_plugin.getByte("EnableLastSentTo", TRUE);
	opts.last_sent_msg_type = g_plugin.getWord("MsgTypeRec", TYPE_GLOBAL);
	opts.hide_from_offline_proto = g_plugin.getByte("HideOfflineFromOfflineProto", TRUE);
	opts.group_append = g_plugin.getByte("AppendGroupName", FALSE);
	opts.group_column = g_plugin.getByte("GroupColumn", FALSE);
	opts.group_column_left = g_plugin.getByte("GroupColumnLeft", FALSE);
	opts.hide_subcontacts = g_plugin.getByte("HideSubcontacts", TRUE);
	opts.keep_subcontacts_from_offline = g_plugin.getByte("KeepSubcontactsFromOffline", TRUE);
}

// Options page

static OptPageControl controls[] = {
	{ nullptr, CONTROL_CHECKBOX, IDC_LASTSENTTO, "EnableLastSentTo", (uint8_t)TRUE },
	{ nullptr, CONTROL_RADIO, IDC_GLOBAL, "MsgTypeRec", (uint16_t)TYPE_GLOBAL, TYPE_GLOBAL },
	{ nullptr, CONTROL_RADIO, IDC_LOCAL, "MsgTypeRec", (uint16_t)TYPE_GLOBAL, TYPE_LOCAL },
	{ nullptr, CONTROL_PROTOCOL_LIST, IDC_PROTOCOLS, "ShowOffline%s", (uint8_t)FALSE },
	{ nullptr, CONTROL_CHECKBOX, IDC_HIDE_OFFLINE, "HideOfflineFromOfflineProto", (uint8_t)TRUE },
	{ nullptr, CONTROL_CHECKBOX, IDC_APPEND_GROUP, "AppendGroupName", (uint8_t)FALSE },
	{ nullptr, CONTROL_CHECKBOX, IDC_GROUP_COLUMN, "GroupColumn", (uint8_t)FALSE },
	{ nullptr, CONTROL_CHECKBOX, IDC_GROUP_LEFT, "GroupColumnLeft", (uint8_t)FALSE },
	{ nullptr, CONTROL_CHECKBOX, IDC_SUBCONTACTS, "HideSubcontacts", (uint8_t)TRUE },
	{ nullptr, CONTROL_CHECKBOX, IDC_KEEP_OFFLINE, "KeepSubcontactsFromOffline", (uint8_t)TRUE }
};

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int ret = SaveOptsDlgProc(controls, _countof(controls), MODULENAME, hwndDlg, msg, wParam, lParam);

	switch (msg) {
	case WM_INITDIALOG:
		{
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_LASTSENTTO);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GLOBAL), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOCAL), enabled);

			enabled = IsDlgButtonChecked(hwndDlg, IDC_SUBCONTACTS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_KEEP_OFFLINE), enabled);
			return TRUE;
		}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LASTSENTTO) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_LASTSENTTO);
			EnableWindow(GetDlgItem(hwndDlg, IDC_GLOBAL), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOCAL), enabled);
		}

		if (LOWORD(wParam) == IDC_SUBCONTACTS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SUBCONTACTS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_KEEP_OFFLINE), enabled);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				LoadOptions();
				return TRUE;
			}
			break;
		}
		break;
	}

	return ret;
}

int InitOptionsCallback(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Contacts");
	odp.szTitle.w = LPGENW("Quick Contacts");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT);
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
