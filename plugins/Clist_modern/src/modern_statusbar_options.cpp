/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "stdafx.h"

CMOption<bool> Statusbar::bShow("CLUI", "ShowSBar", true);
CMOption<bool> Statusbar::bPerProto("CLUI", "SBarPerProto", false);
CMOption<bool> Statusbar::bEqualSections("CLUI", "EqualSections", false);
CMOption<bool> Statusbar::bShowUnreadEmails("CLUI", "ShowUnreadEmails", true);
CMOption<bool> Statusbar::bUseConnectingIcon("CLUI", "UseConnectingIcon", true);

CMOption<uint8_t> Statusbar::iAlgn("CLUI", "Align", 0);
CMOption<uint8_t> Statusbar::iVAlign("CLUI", "VAlign", 1);
CMOption<uint8_t> Statusbar::iShowMode("CLUI", "SBarShow", 3);
CMOption<uint8_t> Statusbar::iRClickMode("CLUI", "SBarRightClk", 0);
CMOption<uint8_t> Statusbar::iXStatusMode("CLUI", "ShowXStatus", 6);
CMOption<uint8_t> Statusbar::iProtosPerLine("CLUI", "StatusBarProtosPerLine", 0);

CMOption<uint32_t> Statusbar::iLeftOffset("CLUI", "LeftOffset", 0);
CMOption<uint32_t> Statusbar::iRightOffset("CLUI", "RightOffset", 0);
CMOption<uint32_t> Statusbar::iPaddingLeft("CLUI", "PaddingLeft", 0);
CMOption<uint32_t> Statusbar::iPaddingRight("CLUI", "PaddingRight", 0);
CMOption<uint32_t> Statusbar::iTopOffset("CLUI", "TopOffset", 0);
CMOption<uint32_t> Statusbar::iBottomOffset("CLUI", "BottomOffset", 0);
CMOption<uint32_t> Statusbar::iSpaceBetween("CLUI", "SpaceBetween", 0);

static StatusBarProtocolOptions _GlobalOptions = {};

static void UpdateXStatusIconOptions(HWND hwndDlg, StatusBarProtocolOptions &dat)
{
	int en = IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR) && IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);

	if (IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH)) CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWBOTH), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));

	if (IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL))	CheckDlgButton(hwndDlg, IDC_SHOWBOTH, BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));

	EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));

	uint8_t val = 0;
	if (IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS)) {
		if (IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH)) val = 3;
		else if (IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL)) val = 2;
		else val = 1;
		val += IsDlgButtonChecked(hwndDlg, IDC_TRANSPARENTOVERLAY) ? 4 : 0;
	}
	val += IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUSNAME) ? 8 : 0;
	dat.ShowXStatus = val;
}

static void UpdateStatusBarOptionsDisplay(HWND hwndDlg)
{
	HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST);
	BOOL perProto = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);

	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0), 0);
	if (dat == nullptr)
		perProto = FALSE;

	StatusBarProtocolOptions &sbpo = (dat == nullptr) ? _GlobalOptions : *dat;

	if (perProto) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), TRUE);
		CheckDlgButton(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS, sbpo.AccountIsCustomized ? BST_CHECKED : BST_UNCHECKED);
	}
	else EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), FALSE);

	CheckDlgButton(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY, sbpo.HideAccount ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_USECONNECTINGICON, sbpo.UseConnectingIcon ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWXSTATUSNAME, ((sbpo.ShowXStatus & 8) > 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, ((sbpo.ShowXStatus & 3) > 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, ((sbpo.ShowXStatus & 3) == 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWBOTH, ((sbpo.ShowXStatus & 3) == 3) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWUNREADEMAIL, (sbpo.ShowUnreadEmails == 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_TRANSPARENTOVERLAY, (sbpo.ShowXStatus & 4) ? BST_CHECKED : BST_UNCHECKED);

	uint8_t showOpts = sbpo.SBarShow;
	CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, sbpo.SBarRightClk ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_SETRANGE, 0, MAKELONG(50, 0));
	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_SETPOS, 0, MAKELONG(sbpo.PaddingLeft, 2));

	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_SETRANGE, 0, MAKELONG(50, 0));
	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_SETPOS, 0, MAKELONG(sbpo.PaddingRight, 2));

	if (!sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, sbpo);

	BOOL enableIcons = IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);
	BOOL enableOptions = !perProto || sbpo.AccountIsCustomized;
	EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY), enableOptions && perProto);
	EnableWindow(GetDlgItem(hwndDlg, IDC_USECONNECTINGICON), enableOptions && enableIcons);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUSNAME), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUS), enableOptions && enableIcons);

	if (!enableOptions) {
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), enableOptions && enableIcons);
		EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWBOTH), enableOptions && enableIcons);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), enableOptions && enableIcons);
	}

	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWUNREADEMAIL), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_LEFT), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_LEFT), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_RIGHT), enableOptions);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_RIGHT), enableOptions);

	if (!perProto || sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, sbpo);
}

INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST);
	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0), 0);
	BOOL perProto = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);
	if (dat == nullptr) {
		perProto = FALSE;
		dat = &_GlobalOptions;
	}

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_SHOWSBAR, Statusbar::bShow);
		CheckDlgButton(hwndDlg, IDC_STATUSBAR_PER_PROTO, Statusbar::bPerProto);

		// populate per-proto list box.
		SendMessage(hwndComboBox, CB_RESETCONTENT, 0, 0);

		SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TranslateT("<<Global>>"));
		SendMessage(hwndComboBox, CB_SETITEMDATA, 0, 0);

		for (auto &pa : Accounts()) {
			if (pa->bIsVirtual)
				continue;

			char *szName = pa->szModuleName;
			dat = (StatusBarProtocolOptions *)mir_calloc(sizeof(StatusBarProtocolOptions));
			dat->szName = szName;

			uint32_t dwNewId = SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
			SendMessage(hwndComboBox, CB_SETITEMDATA, dwNewId, (LPARAM)dat);

			ptrA szSettings(db_get_sa(0, szName, "ModernSbar"));
			if (szSettings)
				dat->fromString(szSettings);
		}

		SendMessage(hwndComboBox, CB_SETCURSEL, 0, 0);

		_GlobalOptions.AccountIsCustomized = TRUE;
		_GlobalOptions.SBarRightClk = Statusbar::iRClickMode;
		_GlobalOptions.ShowXStatus = Statusbar::iXStatusMode;
		_GlobalOptions.ShowUnreadEmails = Statusbar::bShowUnreadEmails;
		_GlobalOptions.UseConnectingIcon = Statusbar::bUseConnectingIcon;
		_GlobalOptions.SBarShow = Statusbar::iShowMode;
		_GlobalOptions.PaddingLeft = Statusbar::iPaddingLeft;
		_GlobalOptions.PaddingRight = Statusbar::iPaddingRight;

		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, Statusbar::bEqualSections);

		SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_SETPOS, 0, MAKELONG(Statusbar::iProtosPerLine, 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_SETPOS, 0, MAKELONG(Statusbar::iLeftOffset, 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_SETPOS, 0, MAKELONG(Statusbar::iRightOffset, 0));

		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_SETPOS, 0, MAKELONG(Statusbar::iTopOffset, 0));

		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_SETPOS, 0, MAKELONG(Statusbar::iBottomOffset, 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_SETPOS, 0, MAKELONG(Statusbar::iSpaceBetween, 2));

		{
			wchar_t *align[] = { LPGENW("Left"), LPGENW("Center"), LPGENW("Right") };
			for (auto &it : align)
				SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
			SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_SETCURSEL, Statusbar::iAlgn, 0);
		}

		{
			wchar_t *align[] = { LPGENW("Top"), LPGENW("Center"), LPGENW("Bottom") };
			for (auto &it : align)
				SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateW(it));
			SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_SETCURSEL, Statusbar::iVAlign, 0);
		}

		{
			int en = IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR);
			int en_icons = IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);

			EnableWindow(GetDlgItem(hwndDlg, IDC_EQUALSECTIONS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN2), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON2), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN3), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON3), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MULTI_COUNT), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MULTI_SPIN), en);

			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USECONNECTINGICON), en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUSNAME), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUS), en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWBOTH), en && en_icons && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), en && en_icons && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));
			EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), en && en_icons && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWUNREADEMAIL), en);

			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_LEFT), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_LEFT), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON_RIGHT), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN_RIGHT), en);


			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PER_PROTO), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST), en && IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), FALSE);
		}

		UpdateStatusBarOptionsDisplay(hwndDlg);
		return TRUE;

	case WM_USER + 1:


	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_SBAR_VERT_ALIGN || (LOWORD(wParam) == IDC_SBAR_HORIZ_ALIGN) && HIWORD(wParam) == CBN_SELCHANGE))
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		else if (LOWORD(wParam) == IDC_SHOWSBAR) {
			int en = IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR);
			int en_icons = IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWICON), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWPROTO), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWSTATUS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTSTATUS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_RIGHTMIRANDA), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_EQUALSECTIONS), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_USECONNECTINGICON), en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN2), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON2), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETSPIN3), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFSETICON3), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_BORDER_TOP), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_BORDER_BOTTOM), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_HORIZ_ALIGN), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUSNAME), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWXSTATUS), en && en_icons);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWUNREADEMAIL), en);

			EnableWindow(GetDlgItem(hwndDlg, IDC_MULTI_COUNT), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MULTI_SPIN), en);

			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PER_PROTO), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST), en && IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), en && IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));

			UpdateStatusBarOptionsDisplay(hwndDlg);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		}
		else if (LOWORD(wParam) == IDC_STATUSBAR_PER_PROTO) {
			int en = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST), en);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY), en && perProto);
			EnableWindow(GetDlgItem(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS), en);

			UpdateStatusBarOptionsDisplay(hwndDlg);

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		}
		else if (LOWORD(wParam) == IDC_SHOWXSTATUS || LOWORD(wParam) == IDC_SHOWBOTH || LOWORD(wParam) == IDC_SHOWNORMAL || LOWORD(wParam) == IDC_TRANSPARENTOVERLAY || LOWORD(wParam) == IDC_SHOWXSTATUSNAME) {
			UpdateXStatusIconOptions(hwndDlg, *dat);
		}
		else if (LOWORD(wParam) == IDC_SBAR_USE_ACCOUNT_SETTINGS) {
			if (perProto) {
				dat->AccountIsCustomized = IsDlgButtonChecked(hwndDlg, IDC_SBAR_USE_ACCOUNT_SETTINGS);
				UpdateStatusBarOptionsDisplay(hwndDlg);
			}
		}
		else if (LOWORD(wParam) == IDC_SBAR_HIDE_ACCOUNT_COMPLETELY) {
			if (perProto)
				dat->HideAccount = IsDlgButtonChecked(hwndDlg, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY);
		}
		else if (LOWORD(wParam) == IDC_USECONNECTINGICON) {
			dat->UseConnectingIcon = IsDlgButtonChecked(hwndDlg, IDC_USECONNECTINGICON);
		}
		else if (LOWORD(wParam) == IDC_SHOWUNREADEMAIL) {
			dat->ShowUnreadEmails = IsDlgButtonChecked(hwndDlg, IDC_SHOWUNREADEMAIL);
		}
		else if (LOWORD(wParam) == IDC_SHOWICON || LOWORD(wParam) == IDC_SHOWPROTO || LOWORD(wParam) == IDC_SHOWSTATUS) {
			dat->SBarShow = (IsDlgButtonChecked(hwndDlg, IDC_SHOWICON) ? 1 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWPROTO) ? 2 : 0) |
				(IsDlgButtonChecked(hwndDlg, IDC_SHOWSTATUS) ? 4 : 0);

			UpdateStatusBarOptionsDisplay(hwndDlg);
		}
		else if (LOWORD(wParam) == IDC_RIGHTSTATUS || LOWORD(wParam) == IDC_RIGHTMIRANDA) {
			dat->SBarRightClk = IsDlgButtonChecked(hwndDlg, IDC_RIGHTMIRANDA);
		}
		else if (LOWORD(wParam) == IDC_OFFSETICON_LEFT) {
			dat->PaddingLeft = (uint32_t)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_GETPOS, 0, 0);
		}
		else if (LOWORD(wParam) == IDC_OFFSETICON_RIGHT) {
			dat->PaddingRight = (uint32_t)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_GETPOS, 0, 0);
		}
		else if ((LOWORD(wParam) == IDC_MULTI_COUNT || LOWORD(wParam) == IDC_OFFSETICON || LOWORD(wParam) == IDC_OFFSETICON2
			|| LOWORD(wParam) == IDC_OFFSETICON3 || LOWORD(wParam) == IDC_SBAR_BORDER_BOTTOM || LOWORD(wParam) == IDC_SBAR_BORDER_TOP)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0; // dont make apply enabled during buddy set crap
		else if (LOWORD(wParam) == IDC_STATUSBAR_PROTO_LIST) {
			UpdateStatusBarOptionsDisplay(hwndDlg);
			return 0;
		}
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			Statusbar::bPerProto = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);

			int count = SendMessage(hwndComboBox, CB_GETCOUNT, 0, 0);
			for (int i = 1; i < count; i++) {
				StatusBarProtocolOptions *sbpo = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, i, 0);
				db_set_s(0, sbpo->szName, "ModernSbar", sbpo->toString());
			}

			Statusbar::bEqualSections = IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS);
			Statusbar::bShowUnreadEmails = _GlobalOptions.ShowUnreadEmails;
			Statusbar::bUseConnectingIcon = _GlobalOptions.UseConnectingIcon;

			Statusbar::iShowMode = _GlobalOptions.SBarShow;
			Statusbar::iRClickMode = _GlobalOptions.SBarRightClk;
			Statusbar::iXStatusMode = _GlobalOptions.ShowXStatus;
			Statusbar::iPaddingLeft = _GlobalOptions.PaddingLeft;
			Statusbar::iPaddingRight = _GlobalOptions.PaddingRight;

			Statusbar::iProtosPerLine = SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_GETPOS, 0, 0);
			Statusbar::iAlgn = SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_GETCURSEL, 0, 0);
			Statusbar::iVAlign = SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_GETCURSEL, 0, 0);
			Statusbar::iLeftOffset = SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_GETPOS, 0, 0);
			Statusbar::iRightOffset = SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_GETPOS, 0, 0);
			Statusbar::iTopOffset = SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_GETPOS, 0, 0);
			Statusbar::iBottomOffset = SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_GETPOS, 0, 0);
			Statusbar::iSpaceBetween = SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_GETPOS, 0, 0);
			Statusbar::bShow = IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR);

			LoadStatusBarData();
			cliCluiProtocolStatusChanged(0, nullptr);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		int count = SendMessage(hwndComboBox, CB_GETCOUNT, 0, 0);
		for (int i = 0; i < count; i++)
			mir_free((void*)SendMessage(hwndComboBox, CB_GETITEMDATA, i, 0));
	}
	return FALSE;
}
