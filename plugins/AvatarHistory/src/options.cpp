/*
Avatar History Plugin
 Copyright (C) 2006  Matthew Wild - Email: mwild1@gmail.com

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "AvatarHistory.h"

Options opts;

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

static OptPageControl optionsControls[] = { 
	{ NULL, CONTROL_PROTOCOL_LIST, IDC_PROTOCOLS, "%sEnabled", TRUE }
};

static OptPageControl popupsControls[] = { 
	{ NULL,                           CONTROL_CHECKBOX, IDC_POPUPS,         "AvatarPopups",      AVH_DEF_AVPOPUPS },
   { &opts.popup_bkg_color,          CONTROL_COLOR,    IDC_BGCOLOR,        "PopupsBgColor",     AVH_DEF_POPUPBG },
   { &opts.popup_text_color,         CONTROL_COLOR,    IDC_TEXTCOLOR,      "PopupsTextColor",   AVH_DEF_POPUPFG },
   { &opts.popup_use_win_colors,     CONTROL_CHECKBOX, IDC_WINCOLORS,      "PopupsWinColors",   FALSE },
   { &opts.popup_use_default_colors, CONTROL_CHECKBOX, IDC_DEFAULTCOLORS,  "PopupsDefaultColors", AVH_DEF_DEFPOPUPS },
   { &opts.popup_delay_type,         CONTROL_RADIO,    IDC_DELAYFROMPU,    "PopupsDelayType",   POPUP_DELAY_DEFAULT, POPUP_DELAY_DEFAULT },
   { NULL,                           CONTROL_RADIO,    IDC_DELAYCUSTOM,    "PopupsDelayType",   POPUP_DELAY_DEFAULT, POPUP_DELAY_CUSTOM },
   { NULL,                           CONTROL_RADIO,    IDC_DELAYPERMANENT, "PopupsDelayType",   POPUP_DELAY_DEFAULT, POPUP_DELAY_PERMANENT },
   { &opts.popup_timeout,            CONTROL_SPIN,     IDC_DELAY,          "PopupsTimeout", 10, IDC_DELAY_SPIN, (WORD) 1, (WORD) 255 },
   { &opts.popup_right_click_action, CONTROL_COMBO,    IDC_RIGHT_ACTION,   "PopupsRightClick",  POPUP_ACTION_CLOSEPOPUP },
   { &opts.popup_left_click_action,  CONTROL_COMBO,    IDC_LEFT_ACTION,    "PopupsLeftClick",   POPUP_ACTION_OPENAVATARHISTORY },
   { &opts.popup_show_changed,       CONTROL_CHECKBOX, IDC_CHANGED_L,      "PopupsShowChanged", TRUE },
   { &opts.popup_changed,            CONTROL_TEXT,     IDC_CHANGED,        "PopupsTextChanged", (ULONG_PTR) DEFAULT_TEMPLATE_CHANGED },
   { &opts.popup_show_removed,       CONTROL_CHECKBOX, IDC_REMOVED_L,      "PopupsShowRemoved", TRUE },
   { &opts.popup_removed,            CONTROL_TEXT,     IDC_REMOVED,        "PopupsTextRemoved", (ULONG_PTR) DEFAULT_TEMPLATE_REMOVED }
};

// Options dialog procedure ///////////////////////////////////////////////////////////////////////

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return SaveOptsDlgProc(optionsControls, SIZEOF(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

// Popup options dialog procedure /////////////////////////////////////////////////////////////////

static void PopupsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_POPUPS);
	EnableWindow(GetDlgItem(hwndDlg, IDC_COLOURS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYFROMPU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYCUSTOM), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAYPERMANENT), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ACTIONS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_LEFT_ACTION), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_PREV), enabled);
	
	EnableWindow(GetDlgItem(hwndDlg, IDC_BGCOLOR), enabled && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_TEXTCOLOR), enabled && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULTCOLORS), enabled && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS));
	EnableWindow(GetDlgItem(hwndDlg, IDC_WINCOLORS), enabled && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS));

	EnableWindow(GetDlgItem(hwndDlg, IDC_DELAY), enabled && IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM));

	EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGED_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGED), enabled && IsDlgButtonChecked(hwndDlg, IDC_CHANGED_L));
	EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVED), enabled && IsDlgButtonChecked(hwndDlg, IDC_REMOVED_L));
}

static INT_PTR CALLBACK PopupsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Do nothing"));
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Close popup"));
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show avatar history"));
		SendDlgItemMessage(hwndDlg, IDC_RIGHT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show contact history"));

		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Do nothing"));
		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Close popup"));
		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show avatar history"));
		SendDlgItemMessage(hwndDlg, IDC_LEFT_ACTION, CB_ADDSTRING, 0, (LPARAM)TranslateT("Show contact history"));

		// Needs to be called here in this case
		SaveOptsDlgProc(popupsControls, SIZEOF(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
		PopupsEnableDisableCtrls(hwndDlg);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_POPUPS:
		case IDC_WINCOLORS:
		case IDC_DEFAULTCOLORS:
		case IDC_DELAYFROMPU:
		case IDC_DELAYPERMANENT:
		case IDC_DELAYCUSTOM:
		case IDC_CHANGED_L:
		case IDC_REMOVED_L:
			if (HIWORD(wParam) == BN_CLICKED)
				PopupsEnableDisableCtrls(hwndDlg);
			break;

		case IDC_PREV: 
			Options op = opts;
			if (IsDlgButtonChecked(hwndDlg, IDC_DELAYFROMPU))
				op.popup_delay_type = POPUP_DELAY_DEFAULT;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYCUSTOM))
				op.popup_delay_type = POPUP_DELAY_CUSTOM;
			else if (IsDlgButtonChecked(hwndDlg, IDC_DELAYPERMANENT))
				op.popup_delay_type = POPUP_DELAY_PERMANENT;

			op.popup_timeout = GetDlgItemInt(hwndDlg,IDC_DELAY, NULL, FALSE);
			op.popup_bkg_color = SendDlgItemMessage(hwndDlg,IDC_BGCOLOR,CPM_GETCOLOUR,0,0);
			op.popup_text_color = SendDlgItemMessage(hwndDlg,IDC_TEXTCOLOR,CPM_GETCOLOUR,0,0);
			op.popup_use_win_colors = IsDlgButtonChecked(hwndDlg, IDC_WINCOLORS) != 0;
			op.popup_use_default_colors = IsDlgButtonChecked(hwndDlg, IDC_DEFAULTCOLORS) != 0;

			MCONTACT hContact = db_find_first();
			ShowTestPopup(hContact,TranslateT("Test Contact"), TranslateT("Test description"), &op);
			break;
		}
	}

	return SaveOptsDlgProc(popupsControls, SIZEOF(popupsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}

// Functions //////////////////////////////////////////////////////////////////////////////////////

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.ptszGroup = LPGENT("History"); // group to put your item under
	odp.ptszTitle = LPGENT("Avatar"); // name of the item
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	Options_AddPage(wParam, &odp);

	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		odp.ptszGroup = LPGENT("Popups");
		odp.ptszTitle = LPGENT("Avatar Change");
		odp.pfnDlgProc = PopupsDlgProc;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_POPUPS);
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

void LoadOptions()
{
	LoadOpts(optionsControls, SIZEOF(optionsControls), MODULE_NAME);
	LoadOpts(popupsControls, SIZEOF(popupsControls), MODULE_NAME);

	opts.log_per_contact_folders = db_get_b(NULL, MODULE_NAME, "LogPerContactFolders", 0);
	opts.log_keep_same_folder = db_get_b(NULL, MODULE_NAME, "LogKeepSameFolder", 0);
	opts.log_store_as_hash = db_get_b(NULL, MODULE_NAME, "StoreAsHash", 1);
}
