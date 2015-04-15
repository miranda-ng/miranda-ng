/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

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


#include "commons.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

Options opts;

// Functions //////////////////////////////////////////////////////////////////////////////////////

static OptPageControl pageControls[] = {
	{ &opts.cycle_through_protocols, CONTROL_CHECKBOX, IDC_CYCLE_THROUGH_PROTOS, "CicleThroughtProtocols", (BYTE)1 },
	{ &opts.seconds_to_show_protocol, CONTROL_SPIN, IDC_CYCLE_TIME, "CicleTime", (WORD)5, IDC_CYCLE_TIME_SPIN, (WORD)1, (WORD)255 },
	{ &opts.draw_show_protocol_name, CONTROL_CHECKBOX, IDC_SHOW_PROTO_NAME, "ShowProtocolName", (BYTE)1 },
	{ &opts.show_protocol_cycle_button, CONTROL_CHECKBOX, IDC_SHOW_CYCLE_PROTO_BUTTON, "ShowProtocolCycleButton", (BYTE)0 },
	{ &opts.draw_text_rtl, CONTROL_CHECKBOX, IDC_TEXT_RTL, "TextRTL", (BYTE)0 },
	{ &opts.draw_text_align_right, CONTROL_CHECKBOX, IDC_TEXT_ALIGN_RIGHT, "TextAlignRight", (BYTE)0 },
	{ &opts.replace_smileys, CONTROL_CHECKBOX, IDC_REPLACE_SMILEYS, "ReplaceSmileys", (BYTE)1 },
	{ &opts.resize_smileys, CONTROL_CHECKBOX, IDC_RESIZE_SMILEYS, "ResizeSmileys", (BYTE)0 },
	{ &opts.use_contact_list_smileys, CONTROL_CHECKBOX, IDC_USE_CONTACT_LIST_SMILEYS, "UseContactListSmileys", (BYTE)0 },
	{ &opts.global_on_avatar, CONTROL_CHECKBOX, IDC_GLOBAL_ON_AVATAR, "GlobalOnAvatar", (BYTE)0 },
	{ &opts.global_on_nickname, CONTROL_CHECKBOX, IDC_GLOBAL_ON_NICKNAME, "GlobalOnNickname", (BYTE)0 },
	{ &opts.global_on_status, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS, "GlobalOnStatus", (BYTE)0 },
	{ &opts.global_on_status_message, CONTROL_CHECKBOX, IDC_GLOBAL_ON_STATUS_MESSAGE, "GlobalOnStatusMessage", (BYTE)0 },
	{ &opts.draw_avatar_allow_to_grow, CONTROL_CHECKBOX, IDC_AVATAR_ALLOW_TO_GROW, "AvatarAllowToGrow", (BYTE)0 },
	{ &opts.draw_avatar_custom_size, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_SIZE_CHK, "AvatarCustomSize", (BYTE)0 },
	{ &opts.draw_avatar_custom_size_pixels, CONTROL_SPIN, IDC_AVATAR_CUSTOM_SIZE, "AvatarCustomSizePixels", (WORD)30, IDC_AVATAR_CUSTOM_SIZE_SPIN, (WORD)1, (WORD)255 },
	{ &opts.draw_avatar_border, CONTROL_CHECKBOX, IDC_AVATAR_DRAW_BORDER, "AvatarDrawBorders", (BYTE)0 },
	{ &opts.draw_avatar_round_corner, CONTROL_CHECKBOX, IDC_AVATAR_ROUND_CORNERS, "AvatarRoundCorners", (BYTE)1 },
	{ &opts.draw_avatar_use_custom_corner_size, CONTROL_CHECKBOX, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK, "AvatarUseCustomCornerSize", (BYTE)0 },
	{ &opts.draw_avatar_custom_corner_size, CONTROL_SPIN, IDC_AVATAR_CUSTOM_CORNER_SIZE, "AvatarCustomCornerSize", (WORD)4, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN, (WORD)1, (WORD)255 },
	{ &opts.use_avatar_space_to_draw_text, CONTROL_CHECKBOX, IDC_AVATAR_USE_FREE_SPACE, "AvatarUseFreeSpaceToDrawText", (BYTE)1 },
	{ &opts.resize_frame, CONTROL_CHECKBOX, IDC_RESIZE_FRAME, "ResizeFrame", (BYTE)0 },
	{ &opts.use_skinning, CONTROL_CHECKBOX, IDC_USE_SKINNING, "UseSkinning", (BYTE)0 },
	{ &opts.borders[RIGHT], CONTROL_SPIN, IDC_BORDER_RIGHT, "BorderRight", (WORD)8, IDC_BORDER_RIGHT_SPIN, (WORD)0, (WORD)255 },
	{ &opts.borders[LEFT], CONTROL_SPIN, IDC_BORDER_LEFT, "BorderLeft", (WORD)8, IDC_BORDER_LEFT_SPIN, (WORD)0, (WORD)255 },
	{ &opts.borders[TOP], CONTROL_SPIN, IDC_BORDER_TOP, "BorderTop", (WORD)8, IDC_BORDER_TOP_SPIN, (WORD)0, (WORD)255 },
	{ &opts.borders[BOTTOM], CONTROL_SPIN, IDC_BORDER_BOTTOM, "BorderBottom", (WORD)8, IDC_BORDER_BOTTOM_SPIN, (WORD)0, (WORD)255 },
};


// Initializations needed by options
void LoadOptions()
{
	LoadOpts(pageControls, SIZEOF(pageControls), MODULE_NAME);

	// This is created here to assert that this key always exists
	opts.refresh_status_message_timer = db_get_w(NULL, "MyDetails", "RefreshStatusMessageTimer", 12);
	db_set_w(NULL, "MyDetails", "RefreshStatusMessageTimer", opts.refresh_status_message_timer);

	SetCycleTime();
	RefreshFrameAndCalcRects();
}

static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL ret = SaveOptsDlgProc(pageControls, SIZEOF(pageControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) {
	case WM_INITDIALOG:
		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), FALSE);
		}

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS) || BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), FALSE);
		}

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOW_PROTO_NAME))
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_CYCLE_PROTO_BUTTON), FALSE);

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_SIZE_CHK)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_SIZE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_SIZE_SPIN), FALSE);
		}

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_CYCLE_THROUGH_PROTOS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLE_TIME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLE_TIME_SPIN), FALSE);
		}

		if (BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYS)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_CONTACT_LIST_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESIZE_SMILEYS), FALSE);
		}

		if (!ServiceExists(MS_SMILEYADD_BATCHPARSE)) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_REPLACE_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_CONTACT_LIST_SMILEYS), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESIZE_SMILEYS), FALSE);
		}

		if (!ServiceExists(MS_CLIST_FRAMES_SETFRAMEOPTIONS))
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESIZE_FRAME), FALSE);

		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_AVATAR_ROUND_CORNERS || LOWORD(wParam) == IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_AVATAR_ROUND_CORNERS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK), enabled);
			enabled = enabled && IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_CHECK);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_CORNER_SIZE_SPIN), enabled);
		}
		else if (LOWORD(wParam) == IDC_SHOW_PROTO_NAME) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_SHOW_PROTO_NAME);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_CYCLE_PROTO_BUTTON), enabled);
		}
		else if (LOWORD(wParam) == IDC_AVATAR_CUSTOM_SIZE_CHK) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_AVATAR_CUSTOM_SIZE_CHK);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_SIZE), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR_CUSTOM_SIZE_SPIN), enabled);
		}
		else if (LOWORD(wParam) == IDC_CYCLE_THROUGH_PROTOS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_CYCLE_THROUGH_PROTOS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLE_TIME), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CYCLE_TIME_SPIN), enabled);
		}
		else if (LOWORD(wParam) == IDC_REPLACE_SMILEYS) {
			BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_REPLACE_SMILEYS);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USE_CONTACT_LIST_SMILEYS), enabled);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RESIZE_SMILEYS), enabled);
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
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = -200000000;
	odp.hInstance = hInst;
	odp.pfnDlgProc = DlgProcOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTS);
	odp.ptszGroup = LPGENT("Contact list");
	odp.ptszTitle = LPGENT("My details");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	Options_AddPage(wParam, &odp);
	return 0;
}

void InitOptions()
{
	LoadOptions();

	HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}
