#include "StdAfx.h"
#include "sametime.h"

INT_PTR CALLBACK SessionAnnounceDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SessionAnnounceDialogProc_arg* arg = (SessionAnnounceDialogProc_arg*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	CSametimeProto *proto;
	if (arg != NULL)
		proto = arg->proto;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			SessionAnnounceDialogProc_arg* arg = (SessionAnnounceDialogProc_arg*)lParam;
			proto = arg->proto;
			proto->debugLog(_T("SessionAnnounceDialogProc WM_INITDIALOG"));

			SendDlgItemMessage(hwndDlg, IDC_LST_ANTO, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
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
				// Initialize LVITEM members that are common to all items. 
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE; 
				lvI.iItem = 0;
				lvI.iSubItem = 0;

				for (MCONTACT hContact = db_find_first(proto->m_szModuleName); hContact; hContact = db_find_next(hContact, proto->m_szModuleName)) {
					if (db_get_b(hContact, proto->m_szModuleName, "ChatRoom", 0) == 0
						&& db_get_w(hContact, proto->m_szModuleName, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
						lvI.lParam = (LPARAM)hContact;
						lvI.pszText = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR);
						ListView_InsertItem(GetDlgItem(hwndDlg, IDC_LST_ANTO), &lvI);
						lvI.iItem++;
					}
				}
			}
		}
		return 0;

	case WM_CLOSE:
		proto->debugLog(_T("SessionAnnounceDialogProc WM_CLOSE"));
		mir_free(arg);
		DestroyWindow(hwndDlg);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			int size;
			switch (LOWORD(wParam)) {
			case IDC_BUT_SELALL:
				size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
				for (int i = 0; i < size; i++)
					ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i, true);
				return 0;

			case IDC_BUT_SELINV:
				size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
				for (int i = 0; i < size; i++)
					ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i,
					!ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i));
				return 0;

			case IDOK:
				proto->debugLog(_T("SessionAnnounceDialogProc IDOK BN_CLICKED"));
				{
					// build SendAnnouncementFunc_arg
					SendAnnouncementFunc_arg* safArg = (SendAnnouncementFunc_arg*)mir_calloc(sizeof(SendAnnouncementFunc_arg));
					DBVARIANT dbv;
					LVITEM lvI = { 0 };

					char id[1024];
					strcpy(id, "@U");		// documentation says prepend '@U' to usernames and '@G' to notes group names - but
					char *p = id + 2;		// it's wrong - it works for a list of user id's with no prefix - so we'll do both

					// build recipient list
					safArg->recipients = 0;

					int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_ANTO));
					int send_count = 0;
					for (int i = 0; i < size; i++) {
						if (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_ANTO), i)) {
							lvI.iItem = i;
							lvI.iSubItem = 0;
							lvI.mask = LVIF_PARAM;
							ListView_GetItem(GetDlgItem(hwndDlg, IDC_LST_ANTO), &lvI);

							if (!db_get_utf((MCONTACT)lvI.lParam, proto->m_szModuleName, "stid", &dbv)) {
								safArg->recipients = g_list_prepend(safArg->recipients, _strdup(dbv.pszVal));
								strcpy(p, dbv.pszVal);
								safArg->recipients = g_list_prepend(safArg->recipients, _strdup(id));
								send_count++;
								db_free(&dbv);
							}
						}
					}

					if (send_count > 0) {
						GetWindowText(GetDlgItem(hwndDlg, IDC_ED_ANMSG), safArg->msg, MAX_MESSAGE_SIZE);
						safArg->proto = proto;
						SendAnnouncementFunc sendAnnouncementFunc = arg->sendAnnouncementFunc;
						sendAnnouncementFunc(safArg);
					}

					// clean up recipient list
					if (safArg->recipients) {
						for (GList *rit = safArg->recipients; rit; rit = rit->next) {
							free(rit->data);
						}
						g_list_free(safArg->recipients);
					}

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
