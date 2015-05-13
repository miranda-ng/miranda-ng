/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_defsettings.h"

typedef struct _StatusBarProtocolOptions
{
	char *szName;
	BOOL AccountIsCustomized;
	BOOL HideAccount;
	BYTE SBarShow;
	BYTE SBarRightClk;
	BYTE UseConnectingIcon;
	BYTE ShowUnreadEmails;
	BYTE ShowXStatus;
	int PaddingLeft;
	int PaddingRight;
} StatusBarProtocolOptions;

static StatusBarProtocolOptions _GlobalOptions = { 0 };

static void UpdateXStatusIconOptions(HWND hwndDlg, StatusBarProtocolOptions &dat)
{
	int en = IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR) && IsDlgButtonChecked(hwndDlg, IDC_SHOWICON);

	if (IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH)) CheckDlgButton(hwndDlg, IDC_SHOWNORMAL, BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWBOTH), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL));

	if (IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL))	CheckDlgButton(hwndDlg, IDC_SHOWBOTH, BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOWNORMAL), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));

	EnableWindow(GetDlgItem(hwndDlg, IDC_TRANSPARENTOVERLAY), en && IsDlgButtonChecked(hwndDlg, IDC_SHOWXSTATUS) && IsDlgButtonChecked(hwndDlg, IDC_SHOWNORMAL) && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_SHOWBOTH));

	BYTE val = 0;
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

	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0), NULL);
	if (dat == NULL)
		perProto = FALSE;

	StatusBarProtocolOptions &sbpo = (dat == NULL) ? _GlobalOptions : *dat;

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
	{
		BYTE showOpts = sbpo.SBarShow;
		CheckDlgButton(hwndDlg, IDC_SHOWICON, showOpts & 1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWPROTO, showOpts & 2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWSTATUS, showOpts & 4 ? BST_CHECKED : BST_UNCHECKED);
	}
	CheckDlgButton(hwndDlg, IDC_RIGHTSTATUS, sbpo.SBarRightClk ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(hwndDlg, IDC_RIGHTMIRANDA, IsDlgButtonChecked(hwndDlg, IDC_RIGHTSTATUS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);

	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_SETRANGE, 0, MAKELONG(50, 0));
	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_SETPOS, 0, MAKELONG(sbpo.PaddingLeft, 2));

	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_SETRANGE, 0, MAKELONG(50, 0));
	SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_SETPOS, 0, MAKELONG(sbpo.PaddingRight, 2));

	if (!sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, sbpo);

	{
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
	}

	if (!perProto || sbpo.AccountIsCustomized)
		UpdateXStatusIconOptions(hwndDlg, sbpo);
}

INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndComboBox = GetDlgItem(hwndDlg, IDC_STATUSBAR_PROTO_LIST);
	StatusBarProtocolOptions *dat = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0), 0);
	BOOL perProto = IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO);
	if (dat == NULL) {
		perProto = FALSE;
		dat = &_GlobalOptions;
	}

	switch (msg) {
	case WM_INITDIALOG:
		perProto = (BOOL)db_get_b(NULL, "CLUI", "SBarPerProto", SETTING_SBARPERPROTO_DEFAULT);

		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_SHOWSBAR, db_get_b(NULL, "CLUI", "ShowSBar", SETTING_SHOWSBAR_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_STATUSBAR_PER_PROTO, perProto ? BST_CHECKED : BST_UNCHECKED);
		{
			// populate per-proto list box.
			SendMessage(hwndComboBox, CB_RESETCONTENT, 0, 0);

			int count;
			PROTOACCOUNT **accs;
			ProtoEnumAccounts(&count, &accs);

			SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)TranslateT("<<Global>>"));
			SendMessage(hwndComboBox, CB_SETITEMDATA, 0, 0);

			for (int i = 0; i < count; i++) {
				if (accs[i]->bIsVirtual)
					continue;

				char *szName = accs[i]->szModuleName;
				dat = (StatusBarProtocolOptions *)mir_calloc(sizeof(StatusBarProtocolOptions));
				dat->szName = szName;

				DWORD dwNewId = SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)accs[i]->tszAccountName);
				SendMessage(hwndComboBox, CB_SETITEMDATA, dwNewId, (LPARAM)dat);

				char buf[256];
				mir_snprintf(buf, SIZEOF(buf), "SBarAccountIsCustom_%s", szName);
				dat->AccountIsCustomized = db_get_b(NULL, "CLUI", buf, SETTING_SBARACCOUNTISCUSTOM_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "HideAccount_%s", szName);
				dat->HideAccount = db_get_b(NULL, "CLUI", buf, SETTING_SBARHIDEACCOUNT_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "SBarShow_%s", szName);
				dat->SBarShow = db_get_b(NULL, "CLUI", buf, SETTING_SBARSHOW_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "SBarRightClk_%s", szName);
				dat->SBarRightClk = db_get_b(NULL, "CLUI", buf, SETTING_SBARRIGHTCLK_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "ShowUnreadEmails_%s", szName);
				dat->ShowUnreadEmails = db_get_b(NULL, "CLUI", buf, SETTING_SHOWUNREADEMAILS_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "ShowXStatus_%s", szName);
				dat->ShowXStatus = db_get_b(NULL, "CLUI", buf, SETTING_SHOWXSTATUS_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "UseConnectingIcon_%s", szName);
				dat->UseConnectingIcon = db_get_b(NULL, "CLUI", buf, SETTING_USECONNECTINGICON_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "PaddingLeft_%s", szName);
				dat->PaddingLeft = db_get_dw(NULL, "CLUI", buf, SETTING_PADDINGLEFT_DEFAULT);

				mir_snprintf(buf, SIZEOF(buf), "PaddingRight_%s", szName);
				dat->PaddingRight = db_get_dw(NULL, "CLUI", buf, SETTING_PADDINGRIGHT_DEFAULT);
			}

			if (count)
				SendMessage(hwndComboBox, CB_SETCURSEL, 0, 0);
		}

		_GlobalOptions.AccountIsCustomized = TRUE;
		_GlobalOptions.SBarRightClk = db_get_b(NULL, "CLUI", "SBarRightClk", SETTING_SBARRIGHTCLK_DEFAULT);
		_GlobalOptions.ShowUnreadEmails = db_get_b(NULL, "CLUI", "ShowUnreadEmails", SETTING_SHOWUNREADEMAILS_DEFAULT);
		_GlobalOptions.ShowXStatus = db_get_b(NULL, "CLUI", "ShowXStatus", SETTING_SHOWXSTATUS_DEFAULT);
		_GlobalOptions.UseConnectingIcon = db_get_b(NULL, "CLUI", "UseConnectingIcon", SETTING_USECONNECTINGICON_DEFAULT);
		_GlobalOptions.SBarShow = db_get_b(NULL, "CLUI", "SBarShow", SETTING_SBARSHOW_DEFAULT);

		CheckDlgButton(hwndDlg, IDC_EQUALSECTIONS, db_get_b(NULL, "CLUI", "EqualSections", SETTING_EQUALSECTIONS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, "CLUI", "StatusBarProtosPerLine", SETTING_PROTOSPERLINE_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_SETPOS, 0, MAKELONG(db_get_dw(NULL, "CLUI", "LeftOffset", SETTING_LEFTOFFSET_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_SETPOS, 0, MAKELONG(db_get_dw(NULL, "CLUI", "RightOffset", SETTING_RIGHTOFFSET_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_dw(NULL, "CLUI", "TopOffset", SETTING_TOPOFFSET_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_SETPOS, 0, MAKELONG(db_get_dw(NULL, "CLUI", "BottomOffset", SETTING_BOTTOMOFFSET_DEFAULT), 0));

		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_SETRANGE, 0, MAKELONG(50, 0));
		SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_SETPOS, 0, MAKELONG(db_get_dw(NULL, "CLUI", "SpaceBetween", SETTING_SPACEBETWEEN_DEFAULT), 2));

		{
			TCHAR *align[] = { LPGENT("Left"), LPGENT("Center"), LPGENT("Right") };
			for (int i = 0; i < SIZEOF(align); i++)
				SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateTS(align[i]));
			SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_SETCURSEL, db_get_b(NULL, "CLUI", "Align", SETTING_ALIGN_DEFAULT), 0);
		}

		{
			TCHAR *align[] = { LPGENT("Top"), LPGENT("Center"), LPGENT("Bottom") };
			for (int i = 0; i < SIZEOF(align); i++)
				SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_ADDSTRING, 0, (LPARAM)TranslateTS(align[i]));
			SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_SETCURSEL, db_get_b(NULL, "CLUI", "VAlign", SETTING_VALIGN_DEFAULT), 0);
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
		else if (LOWORD(wParam) == IDC_SHOWXSTATUS || LOWORD(wParam) == IDC_SHOWBOTH || LOWORD(wParam) == IDC_SHOWNORMAL || LOWORD(wParam) == IDC_TRANSPARENTOVERLAY || LOWORD(wParam) == IDC_SHOWXSTATUSNAME)
		{
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
			dat->PaddingLeft = (DWORD)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_LEFT, UDM_GETPOS, 0, 0);
		}
		else if (LOWORD(wParam) == IDC_OFFSETICON_RIGHT) {
			dat->PaddingRight = (DWORD)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN_RIGHT, UDM_GETPOS, 0, 0);
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
			db_set_b(NULL, "CLUI", "SBarPerProto", IsDlgButtonChecked(hwndDlg, IDC_STATUSBAR_PER_PROTO));

			int count = SendMessage(hwndComboBox, CB_GETCOUNT, 0, 0);
			for (int i = 1; i < count; i++) {
				StatusBarProtocolOptions *sbpo = (StatusBarProtocolOptions*)SendMessage(hwndComboBox, CB_GETITEMDATA, i, 0);

				char settingBuf[256];
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarAccountIsCustom_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->AccountIsCustomized);

				mir_snprintf(settingBuf, SIZEOF(settingBuf), "HideAccount_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->HideAccount);

				mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarShow_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->SBarShow);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "SBarRightClk_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->SBarRightClk);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "UseConnectingIcon_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->UseConnectingIcon);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "ShowUnreadEmails_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, (BYTE)sbpo->ShowUnreadEmails);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "ShowXStatus_%s", sbpo->szName);
				db_set_b(NULL, "CLUI", settingBuf, sbpo->ShowXStatus);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "PaddingLeft_%s", sbpo->szName);
				db_set_dw(NULL, "CLUI", settingBuf, sbpo->PaddingLeft);
				mir_snprintf(settingBuf, SIZEOF(settingBuf), "PaddingRight_%s", sbpo->szName);
				db_set_dw(NULL, "CLUI", settingBuf, sbpo->PaddingRight);
			}

			db_set_b(NULL, "CLUI", "SBarShow", (BYTE)_GlobalOptions.SBarShow);
			db_set_b(NULL, "CLUI", "SBarRightClk", (BYTE)_GlobalOptions.SBarRightClk);
			db_set_b(NULL, "CLUI", "UseConnectingIcon", (BYTE)_GlobalOptions.UseConnectingIcon);
			db_set_b(NULL, "CLUI", "ShowUnreadEmails", (BYTE)_GlobalOptions.ShowUnreadEmails);
			db_set_b(NULL, "CLUI", "ShowXStatus", _GlobalOptions.ShowXStatus);
			db_set_dw(NULL, "CLUI", "PaddingLeft", _GlobalOptions.PaddingLeft);
			db_set_dw(NULL, "CLUI", "PaddingRight", _GlobalOptions.PaddingRight);


			db_set_b(NULL, "CLUI", "StatusBarProtosPerLine", (BYTE)SendDlgItemMessage(hwndDlg, IDC_MULTI_SPIN, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CLUI", "EqualSections", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_EQUALSECTIONS));
			db_set_b(NULL, "CLUI", "Align", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SBAR_HORIZ_ALIGN, CB_GETCURSEL, 0, 0));
			db_set_b(NULL, "CLUI", "VAlign", (BYTE)SendDlgItemMessage(hwndDlg, IDC_SBAR_VERT_ALIGN, CB_GETCURSEL, 0, 0));
			db_set_dw(NULL, "CLUI", "LeftOffset", (DWORD)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN, UDM_GETPOS, 0, 0));
			db_set_dw(NULL, "CLUI", "RightOffset", (DWORD)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN2, UDM_GETPOS, 0, 0));
			db_set_dw(NULL, "CLUI", "TopOffset", (DWORD)SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_TOP_SPIN, UDM_GETPOS, 0, 0));
			db_set_dw(NULL, "CLUI", "BottomOffset", (DWORD)SendDlgItemMessage(hwndDlg, IDC_SBAR_BORDER_BOTTOM_SPIN, UDM_GETPOS, 0, 0));
			db_set_dw(NULL, "CLUI", "SpaceBetween", (DWORD)SendDlgItemMessage(hwndDlg, IDC_OFFSETSPIN3, UDM_GETPOS, 0, 0));
			db_set_b(NULL, "CLUI", "ShowSBar", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_SHOWSBAR));

			LoadStatusBarData();
			cliCluiProtocolStatusChanged(0, 0);
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
