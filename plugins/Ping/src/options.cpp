#include "common.h"

PingOptions options;

// main ping options 
static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	OPENFILENAME ofn = {0};

	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );

		if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			hw = GetDlgItem(hwndDlg, IDC_CHK_ATTACH);
			EnableWindow(hw, FALSE);
		}
		CheckDlgButton(hwndDlg, IDC_CHK_ATTACH, options.attach_to_clist);

		SetDlgItemInt(hwndDlg, IDC_PPM, options.ping_period, FALSE);
		SetDlgItemInt(hwndDlg, IDC_PT, options.ping_timeout, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP, options.show_popup);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP2, options.show_popup2);
		CheckDlgButton(hwndDlg, IDC_CHK_BLOCK, options.block_reps);
		CheckDlgButton(hwndDlg, IDC_CHK_LOG, options.logging);
		CheckDlgButton(hwndDlg, IDC_CHK_LOGCSV, options.log_csv);
		CheckDlgButton(hwndDlg, IDC_CHK_NOTESTICON, options.no_test_icon);

		SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_SETPOS, 0, options.indent);
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 6));
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_SETPOS, 0, options.row_height);

		SetDlgItemInt(hwndDlg, IDC_RPT, options.retries, FALSE);

		SetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename);
		if(!options.logging) {
			hw = GetDlgItem(hwndDlg, IDC_ED_FILENAME);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_LOGCSV);
			EnableWindow(hw, FALSE);
		}

		if( !ServiceExists( MS_POPUP_ADDPOPUP )) {
			hw = GetDlgItem(hwndDlg, IDC_CHECKPOPUP);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHECKPOPUP2);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_BLOCK);
			EnableWindow(hw, FALSE);
		} 
		return TRUE;
	}
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_PPM:
			case IDC_PT:
			case IDC_ED_FILENAME:
			case IDC_RPT:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
			break;
		}

		if (HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_LOG:
				hw = GetDlgItem(hwndDlg, IDC_ED_FILENAME);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				hw = GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				hw = GetDlgItem(hwndDlg, IDC_CHK_LOGCSV);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				// drop through
			case IDC_CHK_LOGCSV:
			case IDC_CHECKPOPUP:
			case IDC_CHECKPOPUP2:
			case IDC_CHK_BLOCK:
			case IDC_CHK_MINMAX:
			case IDC_CHK_NOTESTICON:
			case IDC_CHK_ATTACH:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_BTN_VIEWLOG:
				CallService(PLUG "/ViewLogData", 0, 0);
				break;
			case IDC_BTN_LOGBROWSE:
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = options.log_filename;
				ofn.hwndOwner = hwndDlg;
				ofn.Flags = CC_FULLOPEN;
				//ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(options.log_filename);
				ofn.lpstrFilter = LPGENT("All\0*.*\0Text\0*.TXT\0");
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST;

				if(GetOpenFileName(&ofn) == TRUE) {
					SetDlgItemText(hwndDlg, IDC_ED_FILENAME, ofn.lpstrFile);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == UDN_DELTAPOS ) {
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		} 
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {

			BOOL trans_success;

			DWORD new_ping_period = GetDlgItemInt( hwndDlg, IDC_PPM, &trans_success, FALSE);
			if(trans_success) {
				options.ping_period = new_ping_period;
			}
			DWORD new_ping_timeout = GetDlgItemInt( hwndDlg, IDC_PT, &trans_success, FALSE);
			if(trans_success) {
				options.ping_timeout = new_ping_timeout;
			}
			options.show_popup = IsDlgButtonChecked(hwndDlg, IDC_CHECKPOPUP) == BST_CHECKED;
			options.show_popup2 = IsDlgButtonChecked(hwndDlg, IDC_CHECKPOPUP2) == BST_CHECKED;
			options.block_reps = IsDlgButtonChecked(hwndDlg, IDC_CHK_BLOCK) == BST_CHECKED;
			options.logging = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG) == BST_CHECKED;
			options.log_csv = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOGCSV) == BST_CHECKED;
			GetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename, MAX_PATH);

			options.no_test_icon = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTESTICON) == BST_CHECKED;

			options.indent = SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_GETPOS, 0, 0);
			options.row_height = SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_GETPOS, 0, 0);

			DWORD new_retries = GetDlgItemInt( hwndDlg, IDC_RPT, &trans_success, FALSE);
			if(trans_success) {
				options.retries = new_retries;
			}

			bool new_attach = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ATTACH) == BST_CHECKED);
			if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && options.attach_to_clist != new_attach)
				AttachToClist(new_attach);

			options.attach_to_clist = new_attach;

			SaveOptions();

			RefreshWindow(0, 0);

			if(options.logging) CallService(PLUG "/Log", (WPARAM)"options changed", 0);
			if(hWakeEvent) SetEvent(hWakeEvent);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

PINGLIST temp_list;
PINGADDRESS add_edit_addr;

// host edit
INT_PTR CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hw;
	int sel;
	char *strptr;

	switch ( msg ) {
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		{
			for(int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
				strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)i, (LPARAM)0);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
			}

			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
			SendMessage(hw, CB_SETCURSEL, 1, 0);
			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
			SendMessage(hw, CB_SETCURSEL, 0, 0);

			SetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName);
			SetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel);
			SetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand);
			SetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams);

			CheckDlgButton(hwndDlg, IDC_CHK_DESTTCP, add_edit_addr.port != -1);
			if(add_edit_addr.port != -1) {
				hw = GetDlgItem(hwndDlg, IDC_ED_DESTPORT);
				EnableWindow(hw, TRUE);
				SetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, add_edit_addr.port, FALSE);
			}
			{
				int num_protocols;
				PROTOACCOUNT **pppDesc;

				ProtoEnumAccounts(&num_protocols,&pppDesc);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<none>"));
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<all>"));
				for(int i = 0; i < num_protocols; i++) {
						SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->tszAccountName);
				}

				if(add_edit_addr.pszProto[0] == '\0') {
					SendMessage(hw, CB_SETCURSEL, 0, 0);
				} else {
					SendMessage(hw, CB_SELECTSTRING, 0, (LPARAM)add_edit_addr.pszProto);
					hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
					EnableWindow(hw, TRUE);
					SendMessage(hw, CB_SETCURSEL, (WPARAM)(add_edit_addr.set_status - ID_STATUS_OFFLINE), 0);
					hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
					EnableWindow(hw, TRUE);
					SendMessage(hw, CB_SETCURSEL, (WPARAM)(add_edit_addr.get_status - ID_STATUS_OFFLINE), 0);
				}
			}
			// ? doesn't work? ?
			hw = GetDlgItem(hwndDlg, IDC_ED_DESTLAB);
			SetFocus(hw);
		}
		return FALSE;
	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO_DESTPROTO) {
			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
			sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
			if(sel != CB_ERR) {
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
				EnableWindow(hw, sel != 0);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
				EnableWindow(hw, sel != 0);
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			switch( LOWORD( wParam ))
			{
			case IDC_CHK_DESTTCP:
				hw = GetDlgItem(hwndDlg, IDC_ED_DESTPORT);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP));
				break;
			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName, MAX_PINGADDRESS_STRING_LENGTH);
				GetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel, MAX_PINGADDRESS_STRING_LENGTH);
				GetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand, MAX_PATH);
				GetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams, MAX_PATH);

				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
				if(SendMessage(hw, CB_GETCURSEL, 0, 0) != -1)
				{
					GetDlgItemText(hwndDlg, IDC_COMBO_DESTPROTO, add_edit_addr.pszProto, MAX_PINGADDRESS_STRING_LENGTH);
					if(!strcmp(add_edit_addr.pszProto, Translate("<none>"))) add_edit_addr.pszProto[0] = '\0';
					else {
						hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
						sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
						if(sel != -1)
							add_edit_addr.set_status = ID_STATUS_OFFLINE + sel;
						hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
						sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
						if(sel != -1)
							add_edit_addr.get_status = ID_STATUS_OFFLINE + sel;
					}
				} else
					add_edit_addr.pszProto[0] = '\0';

				if (IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP))
				{
					BOOL tr;
					int port = GetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, &tr, FALSE);
					if(tr) add_edit_addr.port = port;
					else add_edit_addr.port = -1;
				} else
					add_edit_addr.port = -1;

				EndDialog(hwndDlg, IDOK);
				break;
			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}

		}

		return TRUE;
	}
	return FALSE;
}

bool Edit(HWND hwnd, PINGADDRESS &addr)
{
	add_edit_addr = addr;
	if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwnd, DlgProcDestEdit) == IDOK)
	{
		addr = add_edit_addr;
		return true;
	}

	return false;
}

// ping hosts list window
static INT_PTR CALLBACK DlgProcOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//OPENFILENAME ofn = {0};
	HWND hw;
	int sel;

	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault( hwndDlg );

			Lock(&data_list_cs, "init options dialog");
			temp_list = data_list;
			Unlock(&data_list_cs);

			hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
			for (pinglist_it i = temp_list.begin(); i != temp_list.end(); ++i)
			{
				int index = SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)i->pszLabel);
				SendMessage(hw, LB_SETITEMDATA, index, (LPARAM)&(*i));
			}

		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_DEST)
		{
			hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
			sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR)
			{
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
				EnableWindow(hw, TRUE);

				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
				EnableWindow(hw, (sel > 0));
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
				EnableWindow(hw, (sel < count - 1));
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			switch( LOWORD( wParam ))
			{
			case IDC_BTN_DESTEDIT:
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR)
				{
					PINGADDRESS *item = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel, 0);
					PINGADDRESS temp = *item;
					if (Edit(hwndDlg, temp))
					{
						*item = temp;
						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, 0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel, (LPARAM)item->pszLabel);
						SendMessage(hw, LB_SETITEMDATA, (WPARAM)sel, (LPARAM)item);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)sel, 0);

						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
						EnableWindow(hw, sel > 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
						int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
						EnableWindow(hw, (sel < count - 1));

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}
				}
				break;
			case IDC_BTN_DESTADD:

				memset(&add_edit_addr,0,sizeof(add_edit_addr));
				add_edit_addr.cbSize = sizeof(add_edit_addr);
				add_edit_addr.port = -1;
				add_edit_addr.set_status = ID_STATUS_ONLINE;
				add_edit_addr.get_status = ID_STATUS_OFFLINE;
				add_edit_addr.status = PS_NOTRESPONDING;
				add_edit_addr.item_id = 0;
				add_edit_addr.index = (int)temp_list.size();

				if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwndDlg, DlgProcDestEdit) == IDOK)
				{
					temp_list.push_back(add_edit_addr);

					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int index = SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)add_edit_addr.pszLabel);
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					SendMessage(hw, LB_SETCURSEL, (WPARAM)index, 0);
					SendMessage(hw, LB_SETITEMDATA, (WPARAM)index, (LPARAM)&(temp_list.back()));

					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
					EnableWindow(hw, TRUE);

					sel = (int)temp_list.size() - 1;
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
					EnableWindow(hw, (sel > 0));
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
					EnableWindow(hw, (sel < count - 1));

					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}

				break;
			case IDC_BTN_DESTREM:
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR) {
					PINGADDRESS *item = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel, 0);
					SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, 0);
					temp_list.remove(*item);
				}

				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
				EnableWindow(hw, FALSE);

				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_BTN_DESTDOWN:
				{
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						PINGADDRESS *item = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel2, 0),
							*item2 = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel2 + 1, 0);
						if(item && item2)
						{
							add_edit_addr = *item;
							*item = *item2;
							*item2 = add_edit_addr;

							// keep indexes the same, as they're used for sorting the binary tree
							int index = item->index, index2 = item2->index;						
							item->index = index2;
							item2->index = index;

							SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
							SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)item->pszLabel);
							SendMessage(hw, LB_SETITEMDATA, (WPARAM)sel2, (LPARAM)item);
							SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 + 1), (LPARAM)0);
							SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 + 1), (LPARAM)item2->pszLabel);
							SendMessage(hw, LB_SETITEMDATA, (WPARAM)(sel2 + 1), (LPARAM)item2);
							SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 + 1), 0);

							hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
							EnableWindow(hw, (sel2 + 1 > 0));
							hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
							int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
							hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
							EnableWindow(hw, (sel2 + 1 < count - 1));

							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
					}
				}
				break;
			case IDC_BTN_DESTUP:
				{
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						PINGADDRESS *item = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel2, 0),
							*item2 = (PINGADDRESS *)SendMessage(hw, LB_GETITEMDATA, sel2 - 1, 0);

						if (item && item2)
						{
							add_edit_addr = *item;
							*item = *item2;
							*item2 = add_edit_addr;

							// keep indexes the same, as they're used for sorting the binary tree
							int index = item->index, index2 = item2->index;						
							item->index = index2;
							item2->index = index;

							SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
							SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)item->pszLabel);
							SendMessage(hw, LB_SETITEMDATA, (WPARAM)sel2, (LPARAM)item);

							SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 - 1), (LPARAM)0);
							SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 - 1), (LPARAM)item2->pszLabel);
							SendMessage(hw, LB_SETITEMDATA, (WPARAM)(sel2 - 1), (LPARAM)item2);

							SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 - 1), 0);

							hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
							EnableWindow(hw, (sel2 - 1 > 0));
							hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
							int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
							hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
							EnableWindow(hw, (sel2 - 1 < count - 1));

							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
					}
				}

				break;
			}
		}
		if(LOWORD(wParam) == IDC_BGCOL
			|| LOWORD(wParam) == IDC_SP_INDENT || LOWORD(wParam) == IDC_SP_ROWHEIGHT)
		{
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY )
		{
			CallService(PLUG "/SetAndSavePingList", (WPARAM)&temp_list, 0);
			CallService(PLUG "/GetPingList", 0, (LPARAM)&temp_list);
			// the following will be affected due to list rebuild event
			//if(hWakeEvent) SetEvent(hWakeEvent);
			return TRUE;
		}
		break;

	}
	return FALSE;
}

int PingOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = LPGENT("PING");

	odp.ptszTab = LPGENT("Settings");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG1);
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam,&odp);

	odp.ptszTab = LPGENT("Hosts");
	odp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG2);
	odp.pfnDlgProc = DlgProcOpts2;
	Options_AddPage(wParam,&odp);
	return 0;
}

void LoadOptions() {
	options.ping_period = db_get_dw(NULL, PLUG, "PingPeriod", DEFAULT_PING_PERIOD);

	options.ping_timeout = db_get_dw(NULL, PLUG, "PingTimeout", DEFAULT_PING_TIMEOUT);
	CallService(PLUG "/SetPingTimeout", (WPARAM)options.ping_timeout, 0);
	options.show_popup = (db_get_b(NULL, PLUG, "ShowPopup", DEFAULT_SHOW_POPUP ? 1 : 0) == 1);
	options.show_popup2 = (db_get_b(NULL, PLUG, "ShowPopup2", DEFAULT_SHOW_POPUP2 ? 1 : 0) == 1);
	options.block_reps = (db_get_b(NULL, PLUG, "BlockReps", DEFAULT_BLOCK_REPS ? 1 : 0) == 1);
	options.logging = (db_get_b(NULL, PLUG, "LoggingEnabled", DEFAULT_LOGGING_ENABLED ? 1 : 0) == 1);

	options.no_test_icon = (db_get_b(NULL, PLUG, "NoTestStatus", DEFAULT_NO_TEST_ICON ? 1 : 0) == 1);

	options.indent = db_get_w(NULL, PLUG, "Indent", 0);
	options.row_height = db_get_w(NULL, PLUG, "RowHeight", GetSystemMetrics(SM_CYSMICON));

	options.retries = db_get_dw(NULL, PLUG, "Retries", 0);

	CallService(PLUG "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)options.log_filename);

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);

	options.attach_to_clist = (db_get_b(NULL, PLUG, "AttachToClist", DEFAULT_ATTACH_TO_CLIST ? 1 : 0) == 1);
	options.log_csv = (db_get_b(NULL, PLUG, "LogCSV", 0) == 1);
}

void SaveOptions() {
	db_set_dw(NULL, PLUG, "PingPeriod", options.ping_period);
	db_set_dw(NULL, PLUG, "PingTimeout", options.ping_timeout);
	CallService(PLUG "/SetPingTimeout", (WPARAM)options.ping_timeout, 0);
	db_set_b(NULL, PLUG, "ShowPopup", options.show_popup ? 1 : 0);
	db_set_b(NULL, PLUG, "ShowPopup2", options.show_popup2 ? 1 : 0);
	db_set_b(NULL, PLUG, "BlockReps", options.block_reps ? 1 : 0);
	db_set_b(NULL, PLUG, "LoggingEnabled", options.logging ? 1 : 0);

	db_set_b(NULL, PLUG, "NoTestStatus", options.no_test_icon ? 1 : 0);

	db_set_w(NULL, PLUG, "Indent", options.indent);
	db_set_w(NULL, PLUG, "RowHeight", options.row_height);

	db_set_dw(NULL, PLUG, "Retries", (DWORD)options.retries);

	CallService(PLUG "/SetLogFilename", (WPARAM)MAX_PATH, (LPARAM)options.log_filename);

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);

	db_set_b(NULL, PLUG, "AttachToClist", options.attach_to_clist ? 1 : 0);
	db_set_b(NULL, PLUG, "LogCSV", options.log_csv ? 1 : 0);
}
