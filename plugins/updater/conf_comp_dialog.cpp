#include "common.h"
#include "conf_comp_dialog.h"

void FillFolder(TCHAR *folder_name, StrList *filenames) {
	TCHAR filename[MAX_PATH], *p;

	mir_sntprintf(filename, SIZEOF(filename), _T("%s\\*.*"), folder_name);
	p = _tcsrchr(filename, '\\' ) + 1;

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(filename, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do {
			_tcscpy(p, findData.cFileName);

			if (findData.cFileName[0] != '.') 
			{
				if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					FillFolder(filename, filenames);
				} else {
					filenames->insert(mir_tstrdup(filename));
				}
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}
}

INT_PTR CALLBACK DlgProcConfirmComponents(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SendMessage(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS),LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		{
			LVCOLUMN lvc; 
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
	  
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");	
			lvc.cx = 180;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), 0, &lvc);

		}

		//enumerate plugins, fill in list
		{
			StrList filenames;
	
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));

			LVITEM lvI;

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT | LVIF_NORECOMPUTE;// | LVIF_IMAGE; 


			size_t temp_folder_name_len = _tcslen(options.temp_folder);
			TCHAR *p;

			FillFolder(options.temp_folder, &filenames);
			for (int i = 0; i < filenames.getCount(); ++i) 
            {
				if(_tcslen(filenames[i]) > temp_folder_name_len) 
                {
					p = filenames[i] + temp_folder_name_len + 1;

					lvI.iSubItem = 0;
					lvI.pszText = p;
					lvI.iItem = ListView_InsertItem(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), &lvI);

					ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), lvI.iItem, TRUE);
				}
			}

			// do this after filling list - enables 'ITEMCHANGED' below
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 1);
		}

		Utils_RestoreWindowPositionNoSize(hwndDlg,0,MODULE,"ConfirmCompWindow");

		return TRUE;

	case WM_NOTIFY:
		if(((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS)) {
			switch (((LPNMHDR) lParam)->code) {
				
			case LVN_ITEMCHANGED:
				if(GetWindowLongPtr(hwndDlg, GWLP_USERDATA)) {
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					if((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {

						int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));
						bool enableOk = false;
						for(int i = 0; i < size; i++) {
							if(ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i)) {
								enableOk = true;
								break;
							}
						}
						HWND hwOk = GetDlgItem(hwndDlg, IDOK);
						EnableWindow(hwOk, enableOk ? TRUE : FALSE);
					}
				}
				break;
			}				
		}
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_SELECTALL:
				{
					int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));
					bool enableOk = false;
					for(int i = 0; i < size; i++) {
						ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i, TRUE);
						enableOk = true;
					}
					HWND hwOk = GetDlgItem(hwndDlg, IDOK);
					EnableWindow(hwOk, enableOk ? TRUE : FALSE);
				}
				return TRUE;
			case IDC_BTN_SELECTINV:
				{
					int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));
					bool enableOk = false, flag;
					for(int i = 0; i < size; i++) {
						flag = !ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i);
						ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i, flag);
						enableOk |= flag;
					}
					HWND hwOk = GetDlgItem(hwndDlg, IDOK);
					EnableWindow(hwOk, enableOk ? TRUE : FALSE);
				}
				return TRUE;
			case IDC_BTN_SELECTDLLS:
				{
					int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));
					TCHAR fname_buff[MAX_PATH];
					bool enableOk = false, flag;
					for(int i = 0; i < size; i++) {
						ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i, 0, fname_buff, MAX_PATH);
						flag = _tcsstr(fname_buff, _T(".dll")) != 0;
						ListView_SetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i, flag);
						enableOk |= flag;
					}
					HWND hwOk = GetDlgItem(hwndDlg, IDOK);
					EnableWindow(hwOk, enableOk ? TRUE : FALSE);
				}
				return TRUE;
			case IDOK:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hwndDlg,0,MODULE,"ConfirmCompWindow");

				{
					TCHAR fname_buff[MAX_PATH];
					_tcscpy(fname_buff, options.temp_folder);
					_tcscat(fname_buff, _T("\\"));
					int path_len = (int)_tcslen(fname_buff);
					TCHAR *p = fname_buff + path_len;

					int size = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS));
					//bool enableOk = false;
					for(int i = 0; i < size; i++) {
						if(!ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i)) {
							// delete file
							ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LIST_COMPONENTS), i, 0, p, MAX_PATH - path_len);
							DeleteFile(fname_buff);

							// attempt to remove parent folder, if any, in case it's now empty
							TCHAR *last_slash = _tcsrchr(fname_buff, _T('\\'));
							if(last_slash != p - 1) {
								*last_slash = 0;
								RemoveDirectory(fname_buff);
							}
						}
					}
				}

				EndDialog(hwndDlg, IDOK);
				return TRUE;
			case IDCANCEL:
				SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
				Utils_SaveWindowPosition(hwndDlg,0,MODULE,"ConfirmCompWindow");
				EndDialog(hwndDlg, IDCANCEL);
				return TRUE;
			}
		}
		break;
	}

	return FALSE;

}
