#include "session_announce_win.h"


INT_PTR CALLBACK SessionAnnounceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam); // save lParam
			SendMessage(GetDlgItem(hwndDlg, IDC_LST_ANTO),LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		
			{
				LVCOLUMN lvc; 
				// Initialize the LVCOLUMN structure.
				// The mask specifies that the format, width, text, and
				// subitem members of the structure are valid. 
				lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
				lvc.fmt = LVCFMT_LEFT;
		  
				lvc.iSubItem = 0;
				lvc.pszText = TranslateT("Recipients");	
				lvc.cx = 300;     // width of column in pixels
				ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LST_ANTO), 0, &lvc);
			}

			//enumerate plugins, fill in list
			{
				ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_LST_ANTO));

				LVITEM lvI;

				// Some code to create the list-view control.
				// Initialize LVITEM members that are common to all
				// items. 
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE; 

				char *proto;
				HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
				while ( hContact != NULL )
				{
					proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
					if ( proto && !strcmp( PROTO, proto)) {
						if(DBGetContactSettingByte(hContact, PROTO, "ChatRoom", 0) == 0
							&& DBGetContactSettingWord(hContact, PROTO, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) 
						{
							lvI.iSubItem = 0;
							lvI.lParam = (LPARAM)hContact;
							lvI.pszText = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
							lvI.iItem = ListView_InsertItem(GetDlgItem(hwndDlg, IDC_LST_ANTO), &lvI);

						}
					}

					hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				}
			}

			return 0;

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;

		case WM_COMMAND:
			if ( HIWORD( wParam ) == BN_CLICKED ) {
				switch( LOWORD( wParam )) {
					case IDC_BUT_SELALL: {
							int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
							for(int i = 0; i < size; i++) ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i, true);
						}
						return 0;
					case IDC_BUT_SELINV: {
							int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
							for(int i = 0; i < size; i++) 
								ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i,
									!ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i)
								);
						}
						return 0;
					case IDOK: {
							// build AnnouncementData
							AnnouncementData ad;
							DBVARIANT dbv;
							LVITEM lvI = {0};	
							
							char id[1024];
							strcpy(id, "@U");		// documentation says prepend '@U' to usernames and '@G' to notes group names - but
							char *p = id + 2;		// it's wrong - it works for a list of user id's with no prefix - so we'll do both

							// build recipient list
							ad.recipients = 0;

							int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
							int send_count = 0;
							for(int i = 0; i < size; i++) {
								if(ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i)) {
									lvI.iItem = i;
									lvI.iSubItem = 0;
									lvI.mask = LVIF_PARAM;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_LST_ANTO), &lvI);

									if(!DBGetContactSettingUtf((HANDLE)lvI.lParam, PROTO, "stid", &dbv)) {
										ad.recipients = g_list_prepend(ad.recipients, _strdup(dbv.pszVal));
										strcpy(p, dbv.pszVal);
										ad.recipients = g_list_prepend(ad.recipients, _strdup(id));
										send_count++;
										DBFreeVariant(&dbv);
									}
								}
							}

							// call function pointed to by lParam
							if(send_count > 0) {
								GetWindowText(GetDlgItem(hwndDlg, IDC_ED_ANMSG), ad.msg, MAX_MESSAGE_SIZE);
								SendAnnounceFunc f = (SendAnnounceFunc)GetWindowLong(hwndDlg, GWL_USERDATA);
								f(&ad);
							}
				
							// clean up recipient list
							for(GList *rit = ad.recipients; rit; rit = rit->next) {
								free(rit->data);
							}
							if(ad.recipients) g_list_free(ad.recipients);

							DestroyWindow(hwndDlg);
						}
						return 0;
					case IDCANCEL:
						DestroyWindow(hwndDlg);
						return 0;
				}
			}
			break;
	}

	return 0;
}