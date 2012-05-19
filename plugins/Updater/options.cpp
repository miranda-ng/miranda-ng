#include "common.h"
#include "options.h"
#include "icons.h"

Options options;

HWND hwndOptions;

HANDLE hMainMenuItemRestart, hMainMenuItemUpdateAndExit;
HANDLE hBackupPath, hDataPath, hArchivePath;

void add_restart_menu_item() {
	CLISTMENUITEM menu = {0};

	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL | CMIF_ICONFROMICOLIB;
	menu.icolibItem = GetIconHandle(I_RSTRT);

	menu.pszName = "Restart";
	menu.pszService= MS_UPDATE_MENURESTART;
	menu.position = 2000099900;
	hMainMenuItemRestart = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);
}

void add_update_and_exit_menu_item() {
	CLISTMENUITEM menu = {0};

	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL | CMIF_ICONFROMICOLIB;
	menu.icolibItem = GetIconHandle(I_CHKUPDEXT);

	menu.pszName = "Update and Exit";
	menu.pszService= MS_UPDATE_MENUUPDATEANDEXIT;
	menu.position = 2000099901;
	hMainMenuItemUpdateAndExit = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);
}

void remove_restart_menu_item() {
#ifdef MS_CLIST_REMOVEMAINMENUITEM
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)hMainMenuItemRestart, 0);
#endif
}

void remove_update_and_exit_menu_item() {
#ifdef MS_CLIST_REMOVEMAINMENUITEM
	CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)hMainMenuItemUpdateAndExit, 0);
#endif
}

static int EnumerateFuncFillList(char *szComponentName, UpdateOptions *update_options, LPARAM lParam) {
	LVITEM lvI = {0};
	HWND hwndDlg = (HWND)lParam;
	HWND hwndLV = GetDlgItem(hwndDlg, IDC_LST_REGISTERED);
	TCHAR *temp_str;

	// Some code to create the list-view control.
	// Initialize LVITEM members that are common to all
	// items. 
	lvI.mask = LVIF_TEXT | LVIF_PARAM;
	lvI.lParam = (update_options->enabled ? 1 : 0) | (update_options->use_beta ? 2 : 0) | (update_options->fixed ? 4 : 0);

	lvI.iSubItem = 0;
	lvI.pszText = (temp_str = GetTString(szComponentName));
	lvI.iItem = ListView_InsertItem(hwndLV, &lvI);
	mir_free(temp_str);

	lvI.mask = LVIF_TEXT;
	lvI.iSubItem = 1;
	//lvI.pszText = LPSTR_TEXTCALLBACK;
	if(((int)lvI.lParam & 4) == 4) {
		lvI.pszText = (((int)lvI.lParam & 2) == 2 ? TranslateT("FORCED") : TranslateT("N/A"));
	} else {
		lvI.pszText = (((int)lvI.lParam & 2) == 2 ? TranslateT("TRUE") : TranslateT("FALSE"));
	}
	
	ListView_SetItem(hwndLV, &lvI);

	ListView_SetCheckState(hwndLV, lvI.iItem, update_options->enabled);

	return 0;
}

static INT_PTR CALLBACK DlgProcOpts1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		hwndOptions = hwndDlg;
		TranslateDialogDefault( hwndDlg );

		CheckDlgButton(hwndDlg, IDC_CHK_ONSTART, options.check_on_startup);
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_ONCE);
			EnableWindow(hw, (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONSTART) ? TRUE : FALSE));
		}
		CheckDlgButton(hwndDlg, IDC_CHK_ONCE, options.check_once);
		CheckDlgButton(hwndDlg, IDC_CHK_DAILY, options.check_daily);
		if(!ServiceExists(MS_TRIGGER_REGISTERACTION)) {
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_NOCONFIDLE);
			EnableWindow(hw, (IsDlgButtonChecked(hwndDlg, IDC_CHK_DAILY) ? TRUE : FALSE));
		}
		CheckDlgButton(hwndDlg, IDC_CHK_NOCONFIDLE, options.no_conf_idle);
		CheckDlgButton(hwndDlg, IDC_CHK_BACKUP, options.backup);
		CheckDlgButton(hwndDlg, IDC_CHK_STARTOFFLINE, options.start_offline);
		CheckDlgButton(hwndDlg, IDC_CHK_POPUP, options.popup_notify);
		CheckDlgButton(hwndDlg, IDC_CHK_RESTART, options.restart_menu_item);
		CheckDlgButton(hwndDlg, IDC_CHK_UPDATEANDEXIT, options.update_and_exit_menu_item);
		CheckDlgButton(hwndDlg, IDC_CHK_SAVEZIPS, options.save_zips);
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_POPUP);
			EnableWindow(hw, ArePopupsEnabled() ? TRUE : FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_NOUNZIP);
			EnableWindow(hw, options.save_zips ? TRUE : FALSE);
		}
		CheckDlgButton(hwndDlg, IDC_CHK_NOUNZIP, options.no_unzip);
		CheckDlgButton(hwndDlg, IDC_CHK_DLLSONLY, options.auto_dll_only);

		// set up Version Requirement slider
		SendDlgItemMessage(hwndDlg, IDC_SLID_VERREQ, TBM_SETRANGE, 0, MAKELONG(0, 3));

		SendDlgItemMessage(hwndDlg, IDC_SLID_VERREQ, TBM_SETPOS, TRUE, (int)options.ver_req);
		switch(options.ver_req) {
		case VR_MAJOR:
			SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on major version change."));
			break;
		case VR_MINOR:
			SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on minor version change."));
			break;
		case VR_RELEASE:
			SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on release number change."));
			break;
		case VR_BUILD:
			SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on build number change."));
			break;
		}


		SendMessage(GetDlgItem(hwndDlg, IDC_LST_REGISTERED),LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		{
			LVCOLUMN lvc; 
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
	  
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Component Name");	
			lvc.cx = 260;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT("Use Beta?");	
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), 1, &lvc);
		}

		ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_LST_REGISTERED));
		CallService(MS_UPDATE_ENUMERATE, (WPARAM)EnumerateFuncFillList, (LPARAM)hwndDlg);
		
		//SendDlgItemMessage(hwndDlg, IDC_BTN_CHECK, WM_SETICON, ICON_SMALL, (LPARAM)hIconCheck);
		//SendDlgItemMessage(hwndDlg, IDC_BTN_CHECKNORESTART, WM_SETICON, ICON_SMALL, (LPARAM)hIconCheckExit);
		SendDlgItemMessage(hwndDlg, IDC_BTN_CHECK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(I_CHKUPD));
		SendDlgItemMessage(hwndDlg, IDC_BTN_CHECKNORESTART, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIconEx(I_CHKUPDEXT));

		return FALSE;

	case WMU_CHECKING:
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_BTN_CHECK);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_BTN_CHECKNORESTART);
			EnableWindow(hw, FALSE);
		}
		return TRUE;

	case WMU_DONECHECKING:
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_BTN_CHECK);
			EnableWindow(hw, TRUE);
			hw = GetDlgItem(hwndDlg, IDC_BTN_CHECKNORESTART);
			EnableWindow(hw, TRUE);
		}
		return TRUE;

	case WM_HSCROLL:
		//if (LOWORD(wParam) == TB_PAGEUP || LOWORD(wParam) == TB_PAGEDOWN) 
		{
			VersionRequirement vr = (VersionRequirement)SendDlgItemMessage(hwndDlg, IDC_SLID_VERREQ, TBM_GETPOS, 0, 0);
			switch(vr) {
			case VR_MAJOR:
				SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on major version change."));
				break;
			case VR_MINOR:
				SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on minor version change."));
				break;
			case VR_RELEASE:
				SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on release number change."));
				break;
			case VR_BUILD:
				SetDlgItemText(hwndDlg, IDC_ST_VERREQ, TranslateT("Update on build number change."));
				break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
				case IDC_CHK_ONSTART:
					{
						HWND hw = GetDlgItem(hwndDlg, IDC_CHK_ONCE);
						EnableWindow(hw, (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONSTART) ? TRUE : FALSE));
					}
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_CHK_ONCE:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_CHK_DAILY:
					if(!ServiceExists(MS_TRIGGER_REGISTERACTION)) {
						HWND hw = GetDlgItem(hwndDlg, IDC_CHK_NOCONFIDLE);
						EnableWindow(hw, (IsDlgButtonChecked(hwndDlg, IDC_CHK_DAILY) ? TRUE : FALSE));
					}
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_CHK_NOCONFIDLE:
				case IDC_CHK_BACKUP:
				case IDC_CHK_STARTOFFLINE:
				case IDC_CHK_POPUP:
				case IDC_CHK_RESTART:
				case IDC_CHK_UPDATEANDEXIT:
				case IDC_CHK_NOUNZIP:
				case IDC_CHK_DLLSONLY:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_CHK_SAVEZIPS:
					{
						HWND hw = GetDlgItem(hwndDlg, IDC_CHK_NOUNZIP);
						EnableWindow(hw, (IsDlgButtonChecked(hwndDlg, IDC_CHK_SAVEZIPS) ? TRUE : FALSE));
					}
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					return TRUE;
				case IDC_BTN_CHECK:
					CallService(MS_UPDATE_CHECKFORUPDATES, 0, 0);
					return TRUE;
				case IDC_BTN_CHECKNORESTART:
					CallService(MS_UPDATE_CHECKFORUPDATES, 0, 1);
					return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		if(((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LST_REGISTERED)) {
			switch (((LPNMHDR) lParam)->code) {
			case NM_CLICK:
				if (((LPNMLISTVIEW)lParam)->iSubItem == 1) {
					LVITEM lvi = {0};
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((LPNMLISTVIEW)lParam)->iItem;
					lvi.iSubItem = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), LVM_GETITEM, 0, (LPARAM)&lvi);
					if(((int)lvi.lParam & 4) == 0) {
						lvi.lParam ^= 2;
						ListView_SetItem(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), &lvi);

						lvi.iSubItem = 1;
						lvi.mask = LVIF_TEXT;
						if(((int)lvi.lParam & 4) == 4) {
							lvi.pszText = (((int)lvi.lParam & 2) == 2 ? TranslateT("FORCED") : TranslateT("N/A"));
						} else {
							lvi.pszText = (((int)lvi.lParam & 2) == 2 ? TranslateT("TRUE") : TranslateT("FALSE"));
						}
						ListView_SetItem(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), &lvi);

						ListView_RedrawItems(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), lvi.iItem, lvi.iItem);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				return 0;
			case LVN_ITEMCHANGED:
				{
					NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;

					if(IsWindowVisible(GetDlgItem(hwndDlg, IDC_LST_REGISTERED)) && ((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK))
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case LVN_GETDISPINFO:
				{
					LVITEM lvi = {0};
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((NMLVDISPINFO *)lParam)->item.iItem;
					lvi.iSubItem = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), LVM_GETITEM, 0, (LPARAM)&lvi);
					switch (((NMLVDISPINFO *)lParam)->item.iSubItem) {
					case 0:
						MessageBox(0, _T("LVN_GETDISPINFO (0)"), _T("msg"), MB_OK);
						break;
					case 1:
						if(((int)lvi.lParam & 4) == 4) {
							((NMLVDISPINFO *)lParam)->item.pszText = ((lvi.lParam & 2) == 2 ? TranslateT("FORCED") : TranslateT("N/A"));
							((NMLVDISPINFO *)lParam)->item.cchTextMax = (int)_tcslen(((NMLVDISPINFO *)lParam)->item.pszText);
						} else {
							((NMLVDISPINFO *)lParam)->item.pszText = ((lvi.lParam & 2) == 2 ? TranslateT("TRUE") : TranslateT("FALSE"));
							((NMLVDISPINFO *)lParam)->item.cchTextMax = (int)_tcslen(((NMLVDISPINFO *)lParam)->item.pszText);
						}
						break;
					}
				}
				return 0;
			}
		} else 
			if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
				options.check_on_startup = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONSTART) ? true : false);
				options.check_once = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ONCE) ? true : false);
				options.check_daily = (IsDlgButtonChecked(hwndDlg, IDC_CHK_DAILY) ? true : false);
				options.no_conf_idle = (IsDlgButtonChecked(hwndDlg, IDC_CHK_NOCONFIDLE) ? true : false);
				options.backup = (IsDlgButtonChecked(hwndDlg, IDC_CHK_BACKUP) ? true : false);
				options.start_offline = (IsDlgButtonChecked(hwndDlg, IDC_CHK_STARTOFFLINE) ? true : false);
				options.popup_notify = (IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUP) ? true : false);
				options.ver_req = (VersionRequirement)SendDlgItemMessage(hwndDlg, IDC_SLID_VERREQ, TBM_GETPOS, 0, 0);
				options.save_zips = (IsDlgButtonChecked(hwndDlg, IDC_CHK_SAVEZIPS) ? true : false);
				options.no_unzip = (IsDlgButtonChecked(hwndDlg, IDC_CHK_NOUNZIP) ? true : false);
				options.auto_dll_only = (IsDlgButtonChecked(hwndDlg, IDC_CHK_DLLSONLY) ? true : false);

				bool restart_menu_item = (IsDlgButtonChecked(hwndDlg, IDC_CHK_RESTART) ? true : false);
				if(options.restart_menu_item != restart_menu_item) {
					options.restart_menu_item = restart_menu_item;
					if(options.restart_menu_item) add_restart_menu_item();
					else remove_restart_menu_item();
				}

				bool update_and_exit_menu_item = (IsDlgButtonChecked(hwndDlg, IDC_CHK_UPDATEANDEXIT) ? true : false);
				if(options.update_and_exit_menu_item != update_and_exit_menu_item) {
					options.update_and_exit_menu_item = update_and_exit_menu_item;
					if(options.update_and_exit_menu_item) add_update_and_exit_menu_item();
					else remove_update_and_exit_menu_item();
				}

				int count = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_LST_REGISTERED));
				UpdateOptions uo;
				LVITEM lvi;
				lvi.mask = LVIF_TEXT | LVIF_PARAM;
				TCHAR buff[256];
				lvi.pszText = buff;
				lvi.cchTextMax = 256;
				lvi.iSubItem = 0;
				for(int i = 0; i < count; i++) {
					lvi.iItem = i;
					SendMessage(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), LVM_GETITEM, 0, (LPARAM)&lvi);

					uo.enabled = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_REGISTERED), lvi.iItem) == TRUE);
					uo.use_beta = (((int)lvi.lParam & 2) == 2);
					uo.fixed = (((int)lvi.lParam & 4) == 4);
					
					CallService(MS_UPDATE_SETUPDATEOPTIONS, (WPARAM)lvi.pszText, (LPARAM)&uo);
				}

				SaveOptions();
				return TRUE;
			}
		break;
	case WM_DESTROY:
		ReleaseIconEx((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_CHECK, BM_SETIMAGE, IMAGE_ICON, 0));
		ReleaseIconEx((HICON)SendDlgItemMessage(hwndDlg, IDC_BTN_CHECKNORESTART, BM_SETIMAGE, IMAGE_ICON, 0));
		hwndOptions = 0;
		break;
	}

	return FALSE;
}

int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.flags						= ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.position					= -790000000;
	odp.hInstance					= hInst;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT1);
	odp.ptszTitle					= LPGENT("Updater");
	odp.ptszGroup					= LPGENT("Services");
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts1;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

#ifdef _UNICODE
#define DBGetString			DBGetContactSettingTString
#define DBWriteString		DBWriteContactSettingTString
#else
#define DBGetString			DBGetContactSetting
#define DBWriteString		DBWriteContactSettingString
#endif

void InitOptionsMenuItems() {
	if(options.restart_menu_item)
		add_restart_menu_item();
	if(options.update_and_exit_menu_item)
		add_update_and_exit_menu_item();
}

int FoldersPathChanged(WPARAM wParam, LPARAM lParam) 
{
	FOLDERSGETDATA fgd = {0};
	fgd.cbSize = sizeof(fgd);
	fgd.nMaxPathSize = MAX_PATH;

	fgd.szPathT = options.backup_folder;
	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hBackupPath, (WPARAM)&fgd);

	fgd.szPathT = options.data_folder;
	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hDataPath, (WPARAM)&fgd);

	fgd.szPathT = options.zip_folder;
	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hArchivePath, (WPARAM)&fgd);

	mir_sntprintf(options.temp_folder, SIZEOF(options.temp_folder), _T("%s\\updates"), options.data_folder);

	return 0;
}

void LoadOptions(void) 
{
	options.check_on_startup = (DBGetContactSettingByte(0, MODULE, "CheckOnStartup", 1) == 1);
	options.check_once = (DBGetContactSettingByte(0, MODULE, "CheckOnce", 1) == 1);
	options.check_daily = (DBGetContactSettingByte(0, MODULE, "CheckDaily", 1) == 1);
	options.no_conf_idle = (DBGetContactSettingByte(0, MODULE, "NoConfIdle", 0) == 1);
	options.backup = (DBGetContactSettingByte(0, MODULE, "Backup", 1) == 1);
	//options.use_xml_backend = (DBGetContactSettingByte(0, MODULE, "UseXML", 1) == 1);
	options.use_xml_backend = true;
	options.start_offline = (DBGetContactSettingByte(0, MODULE, "StartOffline", 0) == 1);
	options.popup_notify = (DBGetContactSettingByte(0, MODULE, "PopupNotify", 0) == 1);
	options.restart_menu_item = (DBGetContactSettingByte(0, MODULE, "RestartMenuItem", 0) == 1);
	options.update_and_exit_menu_item = (DBGetContactSettingByte(0, MODULE, "UpdateAndExitMenuItem", 0) == 1);
	options.save_zips = (DBGetContactSettingByte(0, MODULE, "SaveZips", 0) == 1);
	options.no_unzip = (DBGetContactSettingByte(0, MODULE, "NoUnzip", 0) == 1);

	options.set_colours = false; // = (DBGetContactSettingByte(0, MODULE, "PopupSetColours", 0) == 1); // popup colours - not implemented
	options.bkCol = DBGetContactSettingDword(0, MODULE, "PopupBkCol", 0);
	options.textCol = DBGetContactSettingDword(0, MODULE, "PopupTextCol", 0x00FFFFFF);

	options.ver_req = (VersionRequirement)DBGetContactSettingByte(0, MODULE, "VersionRequirement", VR_BUILD);
	options.auto_dll_only = (DBGetContactSettingByte(0, MODULE, "AutoDLLOnly", 0) == 1);

	if(ServiceExists(MS_FOLDERS_GET_PATH)) 
	{
		FOLDERSDATA fd = {0};
		fd.cbSize = sizeof(fd);
#ifdef _UNICODE
		fd.flags = FF_UNICODE;
#endif
		strcpy(fd.szSection, "Updates");

		strcpy(fd.szName, "Backups");
		fd.szFormatT = _T("%miranda_userdata%\\updater\\backups");
		hBackupPath = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);

		strcpy(fd.szName, "Data");
		fd.szFormatT = _T("%miranda_userdata%\\updater\\data");
		hDataPath = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);

		strcpy(fd.szName, "Saved Archives");
		fd.szFormatT = _T("%miranda_userdata%\\updater\\archives");
		hArchivePath = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);

		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersPathChanged);
		FoldersPathChanged(0, 0);
	}
	else
	{
		TCHAR* mir_path = Utils_ReplaceVarsT(_T("%miranda_userdata%"));

		mir_sntprintf(options.backup_folder, SIZEOF(options.backup_folder), _T("%s\\Updater\\Backups"), mir_path);
		mir_sntprintf(options.data_folder, SIZEOF(options.data_folder), _T("%s\\Updater\\Data"), mir_path);
		mir_sntprintf(options.zip_folder, SIZEOF(options.zip_folder), _T("%s\\Updater\\Archives"), mir_path);
		mir_sntprintf(options.temp_folder, SIZEOF(options.temp_folder), _T("%s\\updates"), options.data_folder);

		mir_free(mir_path);
	}
}

void SaveOptions(void) 
{
	DBWriteContactSettingByte(0, MODULE, "CheckOnStartup", options.check_on_startup ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "CheckOnce", options.check_once ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "CheckDaily", options.check_daily ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "NoConfIdle", options.no_conf_idle ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "Backup", options.backup ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "UseXML", options.use_xml_backend ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "StartOffline", options.start_offline ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "PopupNotify", options.popup_notify ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "RestartMenuItem", options.restart_menu_item ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "UpdateAndExitMenuItem", options.update_and_exit_menu_item ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "VersionRequirement", (int)options.ver_req);
	DBWriteContactSettingByte(0, MODULE, "SaveZips", (int)options.save_zips);
	DBWriteContactSettingByte(0, MODULE, "NoUnzip", (int)options.no_unzip);
	DBWriteContactSettingByte(0, MODULE, "AutoDLLOnly", options.auto_dll_only ? 1 : 0);
}



