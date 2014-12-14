#include "common.h"

PingOptions options;

// main ping options 
static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		if(ServiceExists(MS_CLIST_FRAMES_ADDFRAME))
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_ATTACH), FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_ATTACH, options.attach_to_clist ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemInt(hwndDlg, IDC_PPM, options.ping_period, FALSE);
		SetDlgItemInt(hwndDlg, IDC_PT, options.ping_timeout, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP, options.show_popup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP2, options.show_popup2 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_BLOCK, options.block_reps ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LOG, options.logging ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_LOGCSV, options.log_csv ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_NOTESTICON, options.no_test_icon ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg, IDC_SP_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		SendDlgItemMessage(hwndDlg, IDC_SP_INDENT, UDM_SETPOS, 0, options.indent);
		SendDlgItemMessage(hwndDlg, IDC_SP_ROWHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 6));
		SendDlgItemMessage(hwndDlg, IDC_SP_ROWHEIGHT, UDM_SETPOS, 0, options.row_height);

		SetDlgItemInt(hwndDlg, IDC_RPT, options.retries, FALSE);

		SetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename);
		if(!options.logging) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_FILENAME), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGCSV), FALSE);
		}

		if( !ServiceExists( MS_POPUP_ADDPOPUPT )) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECKPOPUP), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHECKPOPUP2), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_BLOCK), FALSE);
		} 
		return TRUE;
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
				EnableWindow(GetDlgItem(hwndDlg, IDC_ED_FILENAME), IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE), IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_LOGCSV), IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
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
			{
				TCHAR filter[MAX_PATH];
				mir_sntprintf(filter, SIZEOF(filter), _T("%s%c*.txt%c%s%c*.*%c"), TranslateT("Text Files (*.txt)"), 0, 0, TranslateT("All Files"), 0, 0);
				OPENFILENAME ofn = {0};
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = options.log_filename;
				ofn.hwndOwner = hwndDlg;
				ofn.nMaxFile = SIZEOF(options.log_filename);
				ofn.lpstrTitle = TranslateT("Open log file");
				ofn.lpstrFilter = filter;
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST;

				if(GetOpenFileName(&ofn) == TRUE) {
					SetDlgItemText(hwndDlg, IDC_ED_FILENAME, ofn.lpstrFile);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
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
			GetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename, SIZEOF(options.log_filename));

			options.no_test_icon = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTESTICON) == BST_CHECKED;

			options.indent = SendDlgItemMessage(hwndDlg, IDC_SP_INDENT, UDM_GETPOS, 0, 0);
			options.row_height = SendDlgItemMessage(hwndDlg, IDC_SP_ROWHEIGHT, UDM_GETPOS, 0, 0);

			DWORD new_retries = GetDlgItemInt(hwndDlg, IDC_RPT, &trans_success, FALSE);
			if (trans_success) {
				options.retries = new_retries;
			}

			bool new_attach = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ATTACH) == BST_CHECKED);
			if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME) && options.attach_to_clist != new_attach)
				AttachToClist(new_attach);

			options.attach_to_clist = new_attach;

			SaveOptions();

			RefreshWindow(0, 0);

			if(options.logging) CallService(PLUG "/Log", (WPARAM)_T("options changed"), 0);
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
	switch ( msg ) {
	case WM_INITDIALOG: 
		TranslateDialogDefault(hwndDlg);
		for(int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
			INT_PTR ret = CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)i, GSMDF_TCHAR);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ret);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)ret);
		}

		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_SETCURSEL, 1, 0);
		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_SETCURSEL, 0, 0);

		SetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName);
		SetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel);
		SetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand);
		SetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams);

		CheckDlgButton(hwndDlg, IDC_CHK_DESTTCP, add_edit_addr.port != -1 ? BST_CHECKED : BST_UNCHECKED);
		if(add_edit_addr.port != -1) {
			EnableWindow(GetDlgItem(hwndDlg, IDC_ED_DESTPORT), TRUE);
			SetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, add_edit_addr.port, FALSE);
		}

		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<none>"));
		SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<all>"));

		{
			int num_protocols = 0;
			PROTOACCOUNT **pppDesc;

			ProtoEnumAccounts(&num_protocols,&pppDesc);
			for(int i = 0; i < num_protocols; i++)
				SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->tszAccountName);
				
		}

		if(add_edit_addr.pszProto[0] == '\0') {
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_SETCURSEL, 0, 0);
		} else {
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_SELECTSTRING, 0, (LPARAM)add_edit_addr.pszProto);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT), TRUE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_SETCURSEL, (WPARAM)(add_edit_addr.set_status - ID_STATUS_OFFLINE), 0);

			EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2), TRUE);
			SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_SETCURSEL, (WPARAM)(add_edit_addr.get_status - ID_STATUS_OFFLINE), 0);
		}

		// ? doesn't work? ?
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_DESTLAB));
		return FALSE;
	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO_DESTPROTO) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_GETCURSEL, 0, 0);
			if(sel != CB_ERR) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT), sel != 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2), sel != 0);
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			switch( LOWORD( wParam ))
			{
			case IDC_CHK_DESTTCP:
				EnableWindow(GetDlgItem(hwndDlg, IDC_ED_DESTPORT), IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP));
				break;
			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName, SIZEOF(add_edit_addr.pszName));
				GetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel, SIZEOF(add_edit_addr.pszLabel));
				GetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand, SIZEOF(add_edit_addr.pszCommand));
				GetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams, SIZEOF(add_edit_addr.pszParams));

				if(SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTPROTO, CB_GETCURSEL, 0, 0) != -1)
				{
					GetDlgItemTextA(hwndDlg, IDC_COMBO_DESTPROTO, add_edit_addr.pszProto, SIZEOF(add_edit_addr.pszProto));
					if(!strcmp(add_edit_addr.pszProto, Translate("<none>")))
						add_edit_addr.pszProto[0] = '\0';
					else {
						int sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT, CB_GETCURSEL, 0, 0);
						if(sel != CB_ERR)
							add_edit_addr.set_status = ID_STATUS_OFFLINE + sel;

						sel = SendDlgItemMessage(hwndDlg, IDC_COMBO_DESTSTAT2, CB_GETCURSEL, 0, 0);
						if(sel != CB_ERR)
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
				RefreshWindow(0, 0);
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
	switch ( msg ) {
	case WM_INITDIALOG: 
			TranslateDialogDefault( hwndDlg );

			Lock(&data_list_cs, "init options dialog");
			temp_list = data_list;
			Unlock(&data_list_cs);

			for (pinglist_it i = temp_list.begin(); i != temp_list.end(); ++i)
			{
				int index = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)i->pszLabel);
				SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, index, (LPARAM)&(*i));
			}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_DEST)
		{
			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTREM), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT), TRUE);

				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), (sel > 0));
				int count = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCOUNT, 0, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), (sel < count - 1));
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED )
		{
			switch( LOWORD( wParam ))
			{
			case IDC_BTN_DESTEDIT:
			{
				int sel = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR)
				{
					PINGADDRESS *item = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel, 0);
					PINGADDRESS temp = *item;
					if (Edit(hwndDlg, temp))
					{
						*item = temp;
						SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)sel, 0);
						SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)sel, (LPARAM)item->pszLabel);
						SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)sel, (LPARAM)item);
						SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETCURSEL, (WPARAM)sel, 0);

						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTREM), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), sel > 0);
						int count = SendDlgItemMessage(hwndDlg, IDC_BTN_DESTDOWN, LB_GETCOUNT, 0, 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), (sel < count - 1));

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}
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

					int index = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)add_edit_addr.pszLabel);
					SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETCURSEL, (WPARAM)index, 0);
					SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)index, (LPARAM)&(temp_list.back()));

					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTREM), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT), TRUE);

					int sel = (int)temp_list.size() - 1;
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), (sel > 0));
					int count = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCOUNT, 0, 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), (sel < count - 1));

					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}

				break;
			case IDC_BTN_DESTREM:
			{
				int sel = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR) {
					PINGADDRESS *item = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel, 0);
					SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)sel, 0);
					temp_list.remove(*item);
				}

				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTREM), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), FALSE);

				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				RefreshWindow(0, 0);
				break;
			}
			case IDC_BTN_DESTDOWN:
				{
					int sel2 = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						PINGADDRESS *item = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel2, 0),
							*item2 = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel2 + 1, 0);
						if(item && item2)
						{
							add_edit_addr = *item;
							*item = *item2;
							*item2 = add_edit_addr;

							// keep indexes the same, as they're used for sorting the binary tree
							int index = item->index, index2 = item2->index;						
							item->index = index2;
							item2->index = index;

							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)sel2, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)item->pszLabel);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)sel2, (LPARAM)item);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)(sel2 + 1), 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)(sel2 + 1), (LPARAM)item2->pszLabel);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)(sel2 + 1), (LPARAM)item2);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETCURSEL, (WPARAM)(sel2 + 1), 0);

							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), (sel2 + 1 > 0));
							int count = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCOUNT, 0, 0);
							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), (sel2 + 1 < count - 1));

							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
					}
				}
				break;
			case IDC_BTN_DESTUP:
				{
					int sel2 = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						PINGADDRESS *item = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel2, 0),
							*item2 = (PINGADDRESS *)SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETITEMDATA, sel2 - 1, 0);

						if (item && item2)
						{
							add_edit_addr = *item;
							*item = *item2;
							*item2 = add_edit_addr;

							// keep indexes the same, as they're used for sorting the binary tree
							int index = item->index, index2 = item2->index;						
							item->index = index2;
							item2->index = index;

							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)sel2, 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)item->pszLabel);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)sel2, (LPARAM)item);

							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_DELETESTRING, (WPARAM)(sel2 - 1), 0);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_INSERTSTRING, (WPARAM)(sel2 - 1), (LPARAM)item2->pszLabel);
							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETITEMDATA, (WPARAM)(sel2 - 1), (LPARAM)item2);

							SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_SETCURSEL, (WPARAM)(sel2 - 1), 0);

							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTUP), (sel2 - 1 > 0));
							int count = SendDlgItemMessage(hwndDlg, IDC_LST_DEST, LB_GETCOUNT, 0, 0);
							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN), (sel2 - 1 < count - 1));

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

int PingOptInit(WPARAM wParam,LPARAM)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
	odp.ptszGroup = LPGENT("Network");
	odp.ptszTitle = LPGENT("Ping");

	odp.ptszTab = LPGENT("Settings");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG1);
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam,&odp);

	odp.ptszTab = LPGENT("Hosts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG2);
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
