#include "common.h"
#include "conf_dialog.h"

#ifndef BCM_SETSHIELD
#define BCM_SETSHIELD  (0x1600 + 0x000C)
#endif

INT_PTR CALLBACK DlgProcConfirm(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_UPDATES);

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		if(options.save_zips && options.no_unzip) {
				CheckDlgButton(hwndDlg, IDC_CHK_CONFALL, FALSE);
				HWND hw = GetDlgItem(hwndDlg, IDC_CHK_CONFALL);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_CHK_NOINSTALL);
				EnableWindow(hw, FALSE);
				CheckDlgButton(hwndDlg, IDC_CHK_NOINSTALL, TRUE);
				SetWindowText(GetDlgItem(hwndDlg, IDOK), TranslateT("Download"));
		} else {
			if(DBGetContactSettingByte(0, "Updater", "DefaultConfAll", 0)) {
				CheckDlgButton(hwndDlg, IDC_CHK_CONFALL, TRUE);
				HWND hw = GetDlgItem(hwndDlg, IDC_CHK_NOINSTALL);
				EnableWindow(hw, FALSE);
			} else if(DBGetContactSettingByte(0, "Updater", "NoInstall", 0)) {
				CheckDlgButton(hwndDlg, IDC_CHK_NOINSTALL, TRUE);
				SetWindowText(GetDlgItem(hwndDlg, IDOK), TranslateT("Download"));
			}
		}

		{
			LVCOLUMN lvc = {0}; 
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
	  
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");	
			lvc.cx = 145;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT("New Version");	
			lvc.cx = 80;     // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateT("Current Version");	
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = TranslateT("Beta");	
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(hwndList, 3, &lvc);
		}

		//enumerate plugins, fill in list
		{
			bool one_enabled = false;
			ListView_DeleteAllItems(hwndList);

			LVITEM lvI = {0};
			TCHAR *temp_str;

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_NORECOMPUTE;// | LVIF_IMAGE; 

			UpdateList &todo = *(UpdateList *)lParam;
			for (int i = 0; i < todo.getCount(); ++i) 
			{
				lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_IMAGE; 
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)&todo[i];
				lvI.pszText = (temp_str = mir_a2t(todo[i].update.szComponentName));
				lvI.iItem = ListView_InsertItem(hwndList, &lvI);
				mir_free(temp_str);

				lvI.mask = LVIF_TEXT;// | LVIF_IMAGE; 

				lvI.iSubItem = 1;
				//lvI.pszText = LPSTR_TEXTCALLBACK; //i->newVersion;
				lvI.pszText = (temp_str = mir_a2t(((UpdateInternal *)lvI.lParam)->newVersion));
				ListView_SetItem(hwndList, &lvI);
				mir_free(temp_str);

				lvI.iSubItem = 2;
				//lvI.pszText = LPSTR_TEXTCALLBACK; //i->newVersion;
				lvI.pszText = (temp_str = mir_a2t((char *)((UpdateInternal *)lvI.lParam)->update.pbVersion));
				ListView_SetItem(hwndList, &lvI);
				mir_free(temp_str);

				lvI.iSubItem = 3;
				lvI.pszText = ((UpdateInternal *)lvI.lParam)->update_options.use_beta ? 
					TranslateT("Yes") : TranslateT("No");
				ListView_SetItem(hwndList, &lvI);

				//MessageBox(0, todo[i].newVersion, 
					//"Version", MB_OK);
					//todo[i].update.szComponentName, MB_OK);

				// remember whether the user has decided not to update this component with this particular new version
				char stored_setting[256];
				mir_snprintf(stored_setting, 256, "DisabledVer%s", todo[i].update.szComponentName);
				DBVARIANT dbv;
				bool check = todo[i].update_options.enabled;
				if (!DBGetContactSettingString(0, "Updater", stored_setting, &dbv)) {
					if(dbv.pszVal && strcmp(dbv.pszVal, ((UpdateInternal *)lvI.lParam)->newVersion) == 0)
						check = false;
					else
						DBDeleteContactSetting(0, "Updater", stored_setting);
					DBFreeVariant(&dbv);
				}
				one_enabled |= check;
				ListView_SetCheckState(hwndList, lvI.iItem, check);
			}

			HWND hwOk = GetDlgItem(hwndDlg, IDOK);
			EnableWindow(hwOk, one_enabled);
		}
		// do this after filling list - enables 'ITEMCHANGED' below
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, MODULE, "ConfirmWindow");
		if (IsAdminRequired())
			SendDlgItemMessage(hwndDlg, IDOK, BCM_SETSHIELD, 0, TRUE);

		return TRUE;

	case WM_NOTIFY:
		if(((LPNMHDR) lParam)->hwndFrom == hwndList) {
			switch (((LPNMHDR) lParam)->code) {
				
			case LVN_ITEMCHANGED:
				if(GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) {
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					LVITEM lvI = {0};
					
					lvI.iItem = nmlv->iItem;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_PARAM;
					ListView_GetItem(hwndList, &lvI);

					//if(IsWindowVisible(hwndList) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK)) {
					if((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
						((UpdateInternal *)lvI.lParam)->update_options.enabled = ListView_GetCheckState(hwndList, nmlv->iItem) != 0;

						char stored_setting[256];
						mir_snprintf(stored_setting, 256, "DisabledVer%s", ((UpdateInternal *)lvI.lParam)->update.szComponentName);

						if(((UpdateInternal *)lvI.lParam)->update_options.enabled)
							DBDeleteContactSetting(0, "Updater", stored_setting); // user has re-enabled update to this version - remove setting from db
						else
							DBWriteContactSettingString(0, "Updater", stored_setting, ((UpdateInternal *)lvI.lParam)->newVersion);
						//ListView_SetItem(hwndList, &lvI);
						

						UpdateList &todo = *(UpdateList *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						bool enableOk = false;
						for(int i=0; i<todo.getCount(); ++i) {
							if(todo[i].update_options.enabled) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hwndDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
					if(nmlv->uNewState & LVIS_SELECTED) {
						if((!((UpdateInternal *)lvI.lParam)->update_options.use_beta && ((UpdateInternal *)lvI.lParam)->file_id != -1) 
							|| (((UpdateInternal *)lvI.lParam)->update_options.use_beta && ((UpdateInternal *)lvI.lParam)->update.szBetaChangelogURL)) 
						{
							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CHANGELOG), TRUE);
						} else {
							EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CHANGELOG), FALSE);
						}
					}
				}
				break;
				
			case LVN_GETDISPINFO:
				{
					LVITEM lvi;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((NMLVDISPINFO *)lParam)->item.iItem;
					lvi.iSubItem = 0;
					SendMessage(hwndList, LVM_GETITEM, 0, (LPARAM)&lvi);
					switch (((NMLVDISPINFO *)lParam)->item.iSubItem) {
					case 0:
						MessageBox(0, _T("LVN_GETDISPINFO (0)"), _T("msg"), MB_OK);
						break;
					case 1:
						((NMLVDISPINFO *)lParam)->item.pszText = TranslateTS(((UpdateInternal *)lvi.lParam)->newVersion);
						break;
					case 2:
						((NMLVDISPINFO *)lParam)->item.pszText = TranslateTS((char *)((UpdateInternal *)lvi.lParam)->update.pbVersion);
						break;
					}
				}
				return 0;
			}

		}
		break;

	case WM_COMMAND:
		if (HIWORD( wParam ) == BN_CLICKED) 
		{
			switch(LOWORD(wParam)) 
			{
			case IDC_BTN_CHANGELOG:
				{
					char url[256];
					int sel = ListView_GetSelectionMark(hwndList);
					LVITEM lvI = {0};
					
					lvI.iItem = sel;
					lvI.iSubItem = 0;
					lvI.mask = LVIF_PARAM;
					ListView_GetItem(hwndList, &lvI);

					if(!((UpdateInternal *)lvI.lParam)->update_options.use_beta && ((UpdateInternal *)lvI.lParam)->file_id != -1) {
						sprintf(url, MIM_CHANGELOG_URL_PREFIX "%d", ((UpdateInternal *)lvI.lParam)->file_id);
						CallService(MS_UTILS_OPENURL, (WPARAM)TRUE, (LPARAM)url);
					} else if(((UpdateInternal *)lvI.lParam)->update_options.use_beta && ((UpdateInternal *)lvI.lParam)->update.szBetaChangelogURL) {
						CallService(MS_UTILS_OPENURL, TRUE, (LPARAM)((UpdateInternal *)lvI.lParam)->update.szBetaChangelogURL);
					}

				}
				return TRUE;

			case IDC_CHK_CONFALL:
				{
					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_NOINSTALL);
					EnableWindow(hw, !IsDlgButtonChecked(hwndDlg, IDC_CHK_CONFALL));
				}
				return TRUE;

			case IDC_CHK_NOINSTALL:
				{
					HWND hw = GetDlgItem(hwndDlg, IDC_CHK_CONFALL);
					EnableWindow(hw, !IsDlgButtonChecked(hwndDlg, IDC_CHK_NOINSTALL));

					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_NOINSTALL))
						SetWindowText(GetDlgItem(hwndDlg, IDOK), TranslateT("Download"));
					else
						SetWindowText(GetDlgItem(hwndDlg, IDOK), TranslateT("Update"));
				}
				return TRUE;

			case IDOK:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ConfirmWindow");
				DBWriteContactSettingByte(NULL, "Updater", "DefaultConfAll", IsDlgButtonChecked(hwndDlg, IDC_CHK_CONFALL) ? 1 : 0);
				DBWriteContactSettingByte(NULL, "Updater", "NoInstall", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOINSTALL) ? 1 : 0);

				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_CONFALL))
					EndDialog(hwndDlg, CD_CONFALL);
				else if(IsDlgButtonChecked(hwndDlg, IDC_CHK_NOINSTALL))
					EndDialog(hwndDlg, CD_NOINSTALL);
				else
					EndDialog(hwndDlg, CD_OK);
				return TRUE;

			case IDCANCEL:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hwndDlg, NULL, MODULE, "ConfirmWindow");
				EndDialog(hwndDlg, CD_CANCEL);
				return TRUE;
			}
		}
		break;
	}

	return FALSE;
}

