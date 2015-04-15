#include "common.h"

Options options;

HICON hPopupIcon = 0;

void LoadModuleDependentOptions() {
	if (ServiceExists(MS_AV_DRAWAVATAR))
		options.av_layout = (PopupAvLayout)db_get_b(0, MODULE, "AVLayout", PAV_RIGHT);
	else
		options.av_layout = PAV_NONE;

	options.time_layout = (PopupTimeLayout)db_get_b(0, MODULE, "TimeLayout", (ServiceExists(MS_AV_DRAWAVATAR) ? PT_WITHAV : PT_RIGHT));
	if (options.time_layout == PT_WITHAV && !ServiceExists(MS_AV_DRAWAVATAR))
		options.time_layout = PT_RIGHT;
}

void LoadOptions()
{
	options.default_timeout = db_get_dw(0, MODULE, "DefaultTimeout", 7);
	options.win_width       = db_get_dw(0, MODULE, "WinWidth", 220);
	options.win_max_height  = db_get_dw(0, MODULE, "WinMaxHeight", 400);
	options.location        = (PopupLocation)db_get_b(0, MODULE, "Location", (BYTE)PL_BOTTOMRIGHT);
	options.opacity         = db_get_b(0, MODULE, "Opacity", 75);
	options.border          = db_get_b(0, MODULE, "Border", 1) == 1;
	options.round           = db_get_b(0, MODULE, "RoundCorners", 1) == 1;
	options.av_round        = db_get_b(0, MODULE, "AvatarRoundCorners", 1) == 1;
	options.animate         = db_get_b(0, MODULE, "Animate", 0);
	options.trans_bg        = db_get_b(0, MODULE, "TransparentBg", 0) == 1;
	options.use_mim_monitor = db_get_b(0, MODULE, "UseMimMonitor", 1) == 1;
	options.right_icon      = db_get_b(0, MODULE, "RightIcon", 0) == 1;
	options.av_layout       = PAV_NONE; // corrected in LoadModuleDependentOptions function above
	options.av_size         = db_get_dw(0, MODULE, "AVSize", 40); //tweety
	options.text_indent     = db_get_dw(0, MODULE, "TextIndent", 22); 
	options.global_hover    = db_get_b(0, MODULE, "GlobalHover", 1) == 1;
	options.time_layout     = (PopupTimeLayout)db_get_b(0, MODULE, "TimeLayout", PT_RIGHT);

	char buff[128];
	for (int i = 0; i < 10; i++) {
		mir_snprintf(buff, SIZEOF(buff), "DisableStatus%d", i - 1); // -1 because i forgot offline status earlier!
		options.disable_status[i] = (db_get_b(0, MODULE, buff, 0) == 1);
	}

	options.disable_full_screen = db_get_b(0, MODULE, "DisableFullScreen", 1) == 1;
	options.drop_shadow = db_get_b(0, MODULE, "DropShadow", 0) == 1;
	options.sb_width = db_get_dw(0, MODULE, "SidebarWidth", 22);
	options.padding = db_get_dw(0, MODULE, "Padding", 4);
	options.av_padding = db_get_dw(0, MODULE, "AvatarPadding", 4);
}

void SaveOptions()
{
	db_set_dw(0, MODULE, "DefaultTimeout", options.default_timeout);
	db_set_dw(0, MODULE, "WinWidth", options.win_width);
	db_set_dw(0, MODULE, "WinMaxHeight", options.win_max_height);
	db_set_b(0, MODULE, "Location", (BYTE)options.location);
	db_set_b(0, MODULE, "Opacity", (BYTE)options.opacity);
	db_set_b(0, MODULE, "Border", (options.border ? 1 : 0));
	db_set_b(0, MODULE, "RoundCorners", (options.round ? 1 : 0));
	db_set_b(0, MODULE, "AvatarRoundCorners", (options.av_round ? 1 : 0));
	db_set_b(0, MODULE, "Animate", options.animate);
	db_set_b(0, MODULE, "TransparentBg", (options.trans_bg ? 1 : 0));
	db_set_b(0, MODULE, "UseMimMonitor", (options.use_mim_monitor ? 1 : 0));
	db_set_b(0, MODULE, "RightIcon", (options.right_icon ? 1 : 0));
	if (ServiceExists(MS_AV_DRAWAVATAR))
		db_set_b(0, MODULE, "AVLayout", (BYTE)options.av_layout);
	db_set_dw(0, MODULE, "AVSize", options.av_size);
	db_set_dw(0, MODULE, "TextIndent", options.text_indent);
	db_set_b(0, MODULE, "GlobalHover", (options.global_hover ? 1 : 0));
	db_set_b(0, MODULE, "TimeLayout", (BYTE)options.time_layout);

	char buff[128];
	for (int i = 0; i < 9; i++) {
		mir_snprintf(buff, SIZEOF(buff), "DisableStatus%d", i - 1);
		db_set_b(0, MODULE, buff, options.disable_status[i] ? 1 : 0);
	}
	db_set_b(0, MODULE, "DisableFullScreen", (options.disable_full_screen ? 1 : 0));
	db_set_b(0, MODULE, "DropShadow", (options.drop_shadow ? 1 : 0));
	db_set_dw(0, MODULE, "SidebarWidth", options.sb_width);
	db_set_dw(0, MODULE, "Padding", options.padding);
	db_set_dw(0, MODULE, "AvatarPadding", options.av_padding);
}

void ShowExamplePopups()
{
	PopupData pd = {sizeof(PopupData)};
	pd.hIcon = hPopupIcon;
	pd.flags = PDF_TCHAR;

	pd.ptzTitle = TranslateT("Example");
	pd.ptzText = TranslateT("The quick brown fox jumped over the lazy dog.");
	ShowPopup(pd);

	pd.ptzTitle = TranslateT("Example With a Long Title");
	pd.ptzText = TranslateT("The quick brown fox jumped over the lazy dog.");
	ShowPopup(pd);

	pd.ptzTitle = TranslateT("Example");
	pd.ptzText = TranslateT("Thequickbrownfoxjumpedoverthelazydog.");
	ShowPopup(pd);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (options.av_layout != PAV_NONE && ServiceExists(MS_AV_DRAWAVATAR)) {
			AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
			if (ace && (ace->dwFlags & AVS_BITMAP_VALID)) {
				pd.hContact = hContact;
				pd.ptzText = TranslateT("An avatar.");
				ShowPopup(pd);
				break;
			}
		}
	}
}

static INT_PTR CALLBACK DlgProcOpts1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_SETCURSEL, (int)options.location, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on right"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_SETCURSEL, (options.right_icon ? 1 : 0), 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("No time"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time on left"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time on right"));
		if (ServiceExists(MS_AV_DRAWAVATAR))
			SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time above avatar"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_SETCURSEL, (int)options.time_layout, 0);

		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("No avatar"));
		if (ServiceExists(MS_AV_DRAWAVATAR)) {
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left avatar"));
			SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right avatar"));
		}
		else {
			HWND hw = GetDlgItem(hwndDlg, IDC_CMB_AV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_SPIN_AVSIZE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_ED_AVSIZE);
			EnableWindow(hw, FALSE);
		}
		SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_SETCURSEL, (int)options.av_layout, 0);

		CheckDlgButton(hwndDlg, IDC_CHK_GLOBALHOVER, options.global_hover ? BST_CHECKED : BST_UNCHECKED);

		{
			// initialise and fill listbox
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LST_STATUS);
			ListView_DeleteAllItems(hwndList);

			SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			LVCOLUMN lvc = {0};
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Status");	
			lvc.cx = 200;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT;

			int i = 0;
			for (; i < 10; i++) {
				lvI.pszText = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, ID_STATUS_OFFLINE + i, GSMDF_TCHAR);
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, options.disable_status[i]);
			}
			lvI.pszText = TranslateT("Full-screen app running");
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);
			ListView_SetCheckState(hwndList, i, options.disable_full_screen);
		}

		SendDlgItemMessage(hwndDlg, IDC_SPIN_TIMEOUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(360, 1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_MAXHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_AVSIZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 16));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_SBWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN_PADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));

		if (options.default_timeout == -1) {
			CheckDlgButton(hwndDlg, IDC_RAD_NOTIMEOUT, BST_CHECKED);
			HWND hw = GetDlgItem(hwndDlg, IDC_ED_TIMEOUT);
			EnableWindow(hw, FALSE);
			SetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, 7, FALSE);
		}
		else {
			CheckDlgButton(hwndDlg, IDC_RAD_TIMEOUT, BST_CHECKED);
			SetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, options.default_timeout, FALSE);
		}
		
		if (options.right_icon)
			CheckDlgButton(hwndDlg, IDC_RAD_RIGHTICON, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_RAD_LEFTICON, BST_CHECKED);

		if (ServiceExists(MS_AV_DRAWAVATAR)) {
			switch(options.av_layout) {
				case PAV_NONE: CheckDlgButton(hwndDlg, IDC_RAD_NOAV, BST_CHECKED); break;
				case PAV_RIGHT: CheckDlgButton(hwndDlg, IDC_RAD_RIGHTAV, BST_CHECKED); break;
				case PAV_LEFT: CheckDlgButton(hwndDlg, IDC_RAD_LEFTAV, BST_CHECKED); break;
			}
		}
		else {
			CheckDlgButton(hwndDlg, IDC_RAD_NOAV, BST_CHECKED);
			HWND hw = GetDlgItem(hwndDlg, IDC_RAD_RIGHTAV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_RAD_LEFTAV);
			EnableWindow(hw, FALSE);
		}

		SetDlgItemInt(hwndDlg, IDC_ED_WIDTH, options.win_width, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, options.win_max_height, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, options.av_size, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_INDENT, options.text_indent, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, options.sb_width, FALSE);
		SetDlgItemInt(hwndDlg, IDC_ED_PADDING, options.padding, FALSE);

		switch(options.location) {
			case PL_BOTTOMRIGHT: CheckDlgButton(hwndDlg, IDC_RAD_BOTTOMRIGHT, BST_CHECKED); break;
			case PL_BOTTOMLEFT: CheckDlgButton(hwndDlg, IDC_RAD_BOTTOMLEFT, BST_CHECKED); break;
			case PL_TOPRIGHT: CheckDlgButton(hwndDlg, IDC_RAD_TOPRIGHT, BST_CHECKED); break;
			case PL_TOPLEFT: CheckDlgButton(hwndDlg, IDC_RAD_TOPLEFT, BST_CHECKED); break;
		}

		SetDlgItemInt(hwndDlg, IDC_ED_TRANS, options.opacity, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_BORDER, options.border ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERS, options.round ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ROUNDCORNERSAV, options.av_round ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("No animate"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Horizontal animate"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Vertical animate"));
		SendDlgItemMessage(hwndDlg, IDC_CMB_ANIMATE, CB_SETCURSEL, options.animate, 0);

		CheckDlgButton(hwndDlg, IDC_CHK_TRANSBG, options.trans_bg ? BST_CHECKED : BST_UNCHECKED);
		return FALSE;

	case WM_COMMAND:
		if ( HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if ( HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus())
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if ( HIWORD(wParam) == BN_CLICKED ) {
			if (LOWORD(wParam) == IDC_BTN_PREVIEW)
				ShowExamplePopups();
			else {
				HWND hw = GetDlgItem(hwndDlg, IDC_ED_TIMEOUT);
				switch( LOWORD(wParam)) {
				case IDC_RAD_NOTIMEOUT:
					EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_RAD_TIMEOUT));
					break;
				case IDC_RAD_TIMEOUT:
					EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_RAD_TIMEOUT));
					break;
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		break;

	case WM_NOTIFY:
		if (IsWindowVisible(hwndDlg) && ((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LST_STATUS)) {
			switch (((LPNMHDR) lParam)->code) {
			case LVN_ITEMCHANGED:
				NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
				if ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0);
				break;
			}
		}
		else if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			BOOL trans;
			int new_val;
			if (IsDlgButtonChecked(hwndDlg, IDC_RAD_NOTIMEOUT))
				options.default_timeout = -1;
			else {
				new_val = GetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, &trans, FALSE);
				if (trans) options.default_timeout = new_val;
			}
			if (options.default_timeout == 0) {
				SetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, options.default_timeout, FALSE);
				MessageBox(hwndDlg, TranslateT("You cannot set a default timeout of 0.\nValue has been reset."), TranslateT("Error"), MB_OK | MB_ICONWARNING);
				options.default_timeout = 7; // prevent instant timeout
			}

			new_val = GetDlgItemInt(hwndDlg, IDC_ED_WIDTH, &trans, FALSE);
			if (trans) options.win_width = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_MAXHEIGHT, &trans, FALSE);
			if (trans) options.win_max_height = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_AVSIZE, &trans, FALSE);
			if (trans) options.av_size = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_INDENT, &trans, FALSE);
			if (trans) options.text_indent = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_SBWIDTH, &trans, FALSE);
			if (trans) options.sb_width = new_val;
			new_val = GetDlgItemInt(hwndDlg, IDC_ED_PADDING, &trans, FALSE);
			if (trans) options.padding = new_val;

			options.location = (PopupLocation)SendDlgItemMessage(hwndDlg, IDC_CMB_PLACEMENT, CB_GETCURSEL, 0, 0);
			options.right_icon = (SendDlgItemMessage(hwndDlg, IDC_CMB_ICON, CB_GETCURSEL, 0, 0) == 1);
			options.av_layout = (PopupAvLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_AV, CB_GETCURSEL, 0, 0);
			options.time_layout = (PopupTimeLayout)SendDlgItemMessage(hwndDlg, IDC_CMB_TIME, CB_GETCURSEL, 0, 0);

			new_val = GetDlgItemInt(hwndDlg, IDC_ED_TRANS, &trans, FALSE);
			if (trans) options.opacity = new_val;
			options.border = IsDlgButtonChecked(hwndDlg, IDC_CHK_BORDER) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_BORDER)) ? true : false;
			options.round = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERS) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERS))  ? true : false;
			options.av_round = IsDlgButtonChecked(hwndDlg, IDC_CHK_ROUNDCORNERSAV) && IsWindowEnabled(GetDlgItem(hwndDlg, IDC_CHK_ROUNDCORNERSAV))  ? true : false;
			options.animate = SendDlgItemMessage(hwndDlg, IDC_CMB_ANIMATE, CB_GETCURSEL, 0, 0);
			options.trans_bg = IsDlgButtonChecked(hwndDlg, IDC_CHK_TRANSBG) ? true : false;
			options.global_hover = IsDlgButtonChecked(hwndDlg, IDC_CHK_GLOBALHOVER) ? true : false;

			int i = 0;
			for (; i < 10; i++)
				options.disable_status[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_STATUS), i) == 1);
			options.disable_full_screen = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_STATUS), i) == 1);

			SaveOptions();
			return TRUE;
		}
		break;
	}

	return 0;
}

LIST<POPUPCLASS> arNewClasses(3);

static INT_PTR CALLBACK DlgProcOptsClasses(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		arNewClasses = arClasses;
		{
			int index;
			for (int i = 0; i < arNewClasses.getCount(); i++) {
				POPUPCLASS *pc = arNewClasses[i];
				if (pc->flags & PCF_UNICODE)
					index = SendDlgItemMessageW(hwndDlg, IDC_LST_CLASSES, LB_ADDSTRING, 0, (LPARAM)pc->pwszDescription);
				else
					index = SendDlgItemMessageA(hwndDlg, IDC_LST_CLASSES, LB_ADDSTRING, 0, (LPARAM)pc->pszDescription);

				SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_SETITEMDATA, index, i);
			}
		}
		return FALSE;

	case WM_COMMAND:
		if ( LOWORD(wParam) == IDC_LST_CLASSES && HIWORD(wParam) == LBN_SELCHANGE) {
			int index = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETCURSEL, 0, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PREVIEW), index != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COL_TEXT), index != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COL_BG), index != -1);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_TIMEOUT), index != -1);
			if (index != -1) {
				int i = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETITEMDATA, index, 0);
				SendDlgItemMessage(hwndDlg, IDC_COL_TEXT, CPM_SETCOLOUR, 0, (LPARAM)arNewClasses[i]->colorText);
				SendDlgItemMessage(hwndDlg, IDC_COL_BG, CPM_SETCOLOUR, 0, (LPARAM)arNewClasses[i]->colorBack);
				CheckDlgButton(hwndDlg, IDC_CHK_TIMEOUT, arNewClasses[i]->iSeconds != -1 ? BST_CHECKED : BST_UNCHECKED);
				SetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, arNewClasses[i]->iSeconds, TRUE);
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_TIMEOUT), index != -1 && IsDlgButtonChecked(hwndDlg, IDC_CHK_TIMEOUT));
			return TRUE;
		}
		if ( HIWORD(wParam) == EN_CHANGE && (HWND)lParam == GetFocus()) {
			int index = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETCURSEL, 0, 0);
			if (index != -1) {
				int i = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETITEMDATA, index, 0);
				BOOL tr;
				int t = GetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, &tr, FALSE);
				arNewClasses[i]->iSeconds = t;

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
		}
		else {
			int index = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETCURSEL, 0, 0);
			if (index != -1) {
				int i = SendDlgItemMessage(hwndDlg, IDC_LST_CLASSES, LB_GETITEMDATA, index, 0);
				switch(LOWORD(wParam)) {
				case IDC_CHK_TIMEOUT: 
					{
						BOOL isChecked = IsDlgButtonChecked(hwndDlg, IDC_CHK_TIMEOUT);
						EnableWindow(GetDlgItem(hwndDlg, IDC_ED_TIMEOUT), isChecked);
						if (isChecked) arNewClasses[i]->iSeconds = 0;
						else arNewClasses[i]->iSeconds = -1;
						SetDlgItemInt(hwndDlg, IDC_ED_TIMEOUT, arNewClasses[i]->iSeconds, TRUE);
					}
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0);
					break;

				case IDC_COL_TEXT:
					arNewClasses[i]->colorText = SendDlgItemMessage(hwndDlg, IDC_COL_TEXT, CPM_GETCOLOUR, 0, 0);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0);
					break;

				case IDC_COL_BG:
					arNewClasses[i]->colorBack = SendDlgItemMessage(hwndDlg, IDC_COL_BG, CPM_GETCOLOUR, 0, 0);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0);
					break;

				case IDC_BTN_PREVIEW:
					if (arNewClasses[i]->flags & PCF_UNICODE) {
						POPUPCLASS pc = *arNewClasses[i];
						pc.PluginWindowProc = 0;
						POPUPDATACLASS d = {sizeof(d), pc.pszName};
						d.pwszTitle = L"Preview";
						d.pwszText = L"The quick brown fox jumps over the lazy dog.";
						CallService(MS_POPUP_ADDPOPUPCLASS, (WPARAM)&pc, (LPARAM)&d);
					}
					else {
						POPUPCLASS pc = *arNewClasses[i];
						pc.PluginWindowProc = 0;
						POPUPDATACLASS d = {sizeof(d), pc.pszName};
						d.pszTitle = "Preview";
						d.pszText = "The quick brown fox jumps over the lazy dog.";
						CallService(MS_POPUP_ADDPOPUPCLASS, (WPARAM)&pc, (LPARAM)&d);
					}
					break;
				}
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			arClasses = arNewClasses;
			char setting[256];
			for (int i = 0; i < arClasses.getCount(); i++) {
				POPUPCLASS *pc = arClasses[i];
				mir_snprintf(setting, SIZEOF(setting), "%s/Timeout", pc->pszName);
				db_set_w(0, MODULE, setting, pc->iSeconds);
				mir_snprintf(setting, SIZEOF(setting), "%s/TextCol", pc->pszName);
				db_set_dw(0, MODULE, setting, (DWORD)pc->colorText);
				mir_snprintf(setting, SIZEOF(setting), "%s/BgCol", pc->pszName);
				db_set_dw(0, MODULE, setting, (DWORD)pc->colorBack);
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		arNewClasses.destroy();
		break;
	}
	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;
	odp.hInstance = hInst;
	odp.pszTitle = LPGEN("Popups");

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT1);
	odp.pszTab = LPGEN("Settings");
	odp.pfnDlgProc = DlgProcOpts1;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_CLASSES);
	odp.pszTab = LPGEN("Classes");
	odp.pfnDlgProc = DlgProcOptsClasses;
	Options_AddPage(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptInit);

	// an icon for preview popups
	hPopupIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	LoadOptions();
}
