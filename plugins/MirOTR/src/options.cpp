#include "stdafx.h"
extern "C"{
	#include "otrlextensions.h"
}

char g_private_key_filename[MAX_PATH];
char g_fingerprint_store_filename[MAX_PATH];
char g_instag_filename[MAX_PATH];
HANDLE hPATH_MIROTR;
Options options;
#define DATA_DIRECTORY MIRANDA_USERDATA "\\" MODULENAME
HANDLE hHookOptions;
struct PROTOREGENKEYOPTIONS {
	HWND refresh;
	TCHAR proto[129];
};

void SetFilenames(const char *path)
{
	if (!path || !path[0]) 
		return;
	CreateDirectoryTree(path);
	
	strcpy(g_private_key_filename, path);
	strcat(g_private_key_filename, ("\\"));
	strcat(g_private_key_filename, PRIVATE_KEY_FILENAME);
	
	strcpy(g_fingerprint_store_filename, path);
	strcat(g_fingerprint_store_filename, ("\\"));
	strcat(g_fingerprint_store_filename, FINGERPRINT_STORE_FILENAME);
	
	strcpy(g_instag_filename, path);
	strcat(g_instag_filename, ("\\"));
	strcat(g_instag_filename, INSTAG_FILENAME);
}

int FoldersChanged(WPARAM wParam, LPARAM lParam)
{
	char path[MAX_PATH];
	if ( FoldersGetCustomPath(hPATH_MIROTR, path, SIZEOF(path), ""))
		SetFilenames( VARS(DATA_DIRECTORY));
	else
		SetFilenames(path);

	ReadPrivkeyFiles();
	return 0;
}

void LoadFilenames()
{
	if (hPATH_MIROTR = FoldersRegisterCustomPath("OTR", LPGEN("Private Data"), DATA_DIRECTORY)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersChanged);

		// get the path - above are only defaults - there may be a different value in the db
		FoldersChanged(0, 0);
	}
	else {
		SetFilenames( VARS(DATA_DIRECTORY));
		ReadPrivkeyFiles();
	}
}

void LoadOptions() {
	options.default_policy = db_get_w(0, MODULENAME, "DefaultPolicy", OTRL_POLICY_OPPORTUNISTIC);
	// deal with changed flags in proto.h and new interpretation of 'manual' mode (see common.h)
	switch(options.default_policy) {
	case OTRL_POLICY_MANUAL:
		options.default_policy = OTRL_POLICY_MANUAL_MOD;
		break;
	case OTRL_POLICY_OPPORTUNISTIC:
	case OTRL_POLICY_MANUAL_MOD:
	case OTRL_POLICY_ALWAYS:
	case OTRL_POLICY_NEVER:
		break;
	default:
		options.default_policy = OTRL_POLICY_OPPORTUNISTIC;
		break;
	}
	options.err_method = (ErrorDisplay)db_get_w(0, MODULENAME, "ErrorDisplay", ED_POP);
	options.prefix_messages = (db_get_b(0, MODULENAME, "PrefixMessages", 0) == 1);
	options.msg_inline = (db_get_b(0, MODULENAME, "MsgInline", 0) == 1);
	options.msg_popup = (db_get_b(0, MODULENAME, "MsgPopup", 1) == 1);
	options.delete_history = (db_get_b(0, MODULENAME, "NoHistory", 0) == 1);
	options.delete_systeminfo = (db_get_b(0, MODULENAME, "NoSystemHistory", 0) == 1);
	options.autoshow_verify = (db_get_b(0, MODULENAME, "AutoShowVerify", 1) == 1);

	DBVARIANT dbv;
	if (!db_get_utf(0, MODULENAME, "Prefix", &dbv)) {
		mir_strncpy(options.prefix, dbv.pszVal, OPTIONS_PREFIXLEN);
		db_free(&dbv);
	} else
		mir_strncpy(options.prefix, OPTIONS_DEFAULT_PREFIX, OPTIONS_PREFIXLEN);

	options.end_offline = (db_get_b(0, MODULENAME, "EndOffline", 1) == 1);
	options.end_window_close = (db_get_b(0, MODULENAME, "EndWindowClose", 0) == 1);

	options.bHavePopups = 0 != ServiceExists(MS_POPUP_ADDPOPUPT) && ServiceExists(MS_POPUP_SHOWMESSAGE);
	options.bHaveSecureIM = 0 != ServiceExists("SecureIM/IsContactSecured");
	options.bHaveButtonsBar = 0 != ServiceExists(MS_BB_ADDBUTTON);

	LoadFilenames();
}

extern "C" int OpenOptions(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position    = 100;
	odp.hInstance   = hInst;
	odp.ptszGroup   = LPGENT("Services");
	odp.ptszTitle   = _T("OTR");
	odp.flags       = ODPF_BOLDGROUPS|ODPF_TCHAR;

	odp.ptszTab     = LANG_OPT_GENERAL;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENERAL);
	odp.pfnDlgProc  = DlgProcMirOTROpts;
	Options_AddPage(wParam, &odp);

	odp.ptszTab     = LANG_OPT_PROTO;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PROTO);
	odp.pfnDlgProc  = DlgProcMirOTROptsProto;
	Options_AddPage(wParam, &odp);

	odp.ptszTab     = LANG_OPT_CONTACTS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	odp.pfnDlgProc  = DlgProcMirOTROptsContacts;
	Options_AddPage(wParam, &odp);

	odp.ptszTab     = LANG_OPT_FINGER;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FINGER);
	odp.pfnDlgProc  = DlgProcMirOTROptsFinger;
	Options_AddPage(wParam, &odp);
	return 0;
}

void InitOptions() {
	LoadOptions();
	hHookOptions = HookEvent(ME_OPT_INITIALISE, OpenOptions);
}

void DeinitOptions() {
	UnhookEvent(hHookOptions);
}

void SaveOptions() {
	db_set_w(0, MODULENAME, "DefaultPolicy", options.default_policy);
	db_set_w(0, MODULENAME, "ErrorDisplay", (int)options.err_method);
	db_set_b(0, MODULENAME, "PrefixMessages", options.prefix_messages ? 1 : 0);
	db_set_b(0, MODULENAME, "MsgInline", options.msg_inline ? 1 : 0);
	db_set_b(0, MODULENAME, "MsgPopup", options.msg_popup ? 1 : 0);

	db_set_b(0, MODULENAME, "NoHistory", options.delete_history ? 1 : 0);
	db_set_b(0, MODULENAME, "NoSystemHistory", options.delete_systeminfo ? 1 : 0);
	db_set_b(0, MODULENAME, "AutoShowVerify", options.autoshow_verify ? 1 : 0);

	db_set_utf(0, MODULENAME, "Prefix", options.prefix);

	db_set_b(0, MODULENAME, "EndOffline", options.end_offline ? 1 : 0);
	db_set_b(0, MODULENAME, "EndWindowClose", options.end_window_close ? 1 : 0);
}

extern "C" void set_context_contact(void *data, ConnContext *context)
{
	MCONTACT hContact = find_contact(context->username, context->protocol);
	context->app_data = (void*)hContact;
}

void ReadPrivkeyFiles() {
	DEBUGOUT_T("READ privkey");
	lib_cs_lock();
	otrl_privkey_read(otr_user_state, g_private_key_filename);
	otrl_privkey_read_fingerprints(otr_user_state, g_fingerprint_store_filename, set_context_contact, 0);
	otrl_instag_read(otr_user_state, g_instag_filename);
	lib_cs_unlock();
}

static INT_PTR CALLBACK DlgProcMirOTROpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *prefix; char* prefix_utf;
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		// set default policy radio
		switch(options.default_policy) {
			case OTRL_POLICY_OPPORTUNISTIC:
				CheckDlgButton(hwndDlg, IDC_RAD_OPP, BST_CHECKED);
				break;
			case OTRL_POLICY_MANUAL_MOD:
				CheckDlgButton(hwndDlg, IDC_RAD_MANUAL, BST_CHECKED);
				break;
			case OTRL_POLICY_ALWAYS:
				CheckDlgButton(hwndDlg, IDC_RAD_ALWAYS, BST_CHECKED);
				break;
			case OTRL_POLICY_NEVER:
				CheckDlgButton(hwndDlg, IDC_RAD_NEVER, BST_CHECKED);
				break;
		}

		CheckDlgButton(hwndDlg, IDC_CHK_PREFIX, options.prefix_messages ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_MSGINLINE, options.msg_inline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_MSGPOPUP, options.msg_popup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_PREVENTSAVE, options.delete_history ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_PREVENTSYSSAVE, options.delete_systeminfo ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_AUTOSHOW_VERIFY, options.autoshow_verify ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ENDOFFLINE, options.end_offline ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHK_ENDCLOSE, options.end_window_close ? BST_CHECKED : BST_UNCHECKED);

		prefix = mir_utf8decodeT(options.prefix);
		SetDlgItemText(hwndDlg, IDC_ED_PREFIX, prefix);
		mir_free(prefix);

		return TRUE;
		break;

	case WM_COMMAND:
		switch ( HIWORD( wParam )) {
			case BN_CLICKED:
				switch ( LOWORD( wParam )) {
					case IDC_RAD_OPP:
					case IDC_RAD_MANUAL:
					case IDC_RAD_ALWAYS:
					case IDC_RAD_NEVER:
					case IDC_CHK_PREFIX:
					case IDC_CHK_MSGINLINE:
					case IDC_CHK_MSGPOPUP:
					case IDC_CHK_PREVENTSAVE:
					case IDC_CHK_AUTOSHOW_VERIFY:
					case IDC_CHK_ENDOFFLINE:
					case IDC_CHK_ENDCLOSE:
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case EN_CHANGE:
				if (LOWORD( wParam ) == IDC_ED_PREFIX && ( HWND )lParam == GetFocus())
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
		}
		break;

	case WM_NOTIFY:

		if (((LPNMHDR)lParam)->code == (UINT) PSN_APPLY ) {
			// handle apply

			if (IsDlgButtonChecked(hwndDlg, IDC_RAD_OPP))
				options.default_policy = OTRL_POLICY_OPPORTUNISTIC;
			else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_MANUAL))
				options.default_policy = OTRL_POLICY_MANUAL_MOD;
			else if (IsDlgButtonChecked(hwndDlg, IDC_RAD_ALWAYS))
				options.default_policy = OTRL_POLICY_ALWAYS;
			else
				options.default_policy = OTRL_POLICY_NEVER;

			options.prefix_messages = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_PREFIX));
			options.msg_inline = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_MSGINLINE));
			options.msg_popup = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_MSGPOPUP));
			options.delete_history = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_PREVENTSAVE));
			options.delete_systeminfo = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_PREVENTSYSSAVE));
			options.autoshow_verify = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_AUTOSHOW_VERIFY));
			options.end_offline = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_ENDOFFLINE));
			options.end_window_close = (TRUE==IsDlgButtonChecked(hwndDlg, IDC_CHK_ENDCLOSE));

			prefix = (TCHAR*)mir_alloc(sizeof(TCHAR)*OPTIONS_PREFIXLEN);
			GetDlgItemText(hwndDlg, IDC_ED_PREFIX, prefix, OPTIONS_PREFIXLEN);
			prefix_utf = mir_utf8encodeT(prefix);
			mir_free(prefix);
			if(!prefix_utf[0]){
				SetDlgItemTextA(hwndDlg, IDC_ED_PREFIX, OPTIONS_DEFAULT_PREFIX);
				mir_strncpy(options.prefix, OPTIONS_DEFAULT_PREFIX, OPTIONS_PREFIXLEN);
			} else
				mir_strncpy(options.prefix, prefix_utf, OPTIONS_PREFIXLEN);
			mir_free(prefix_utf);

			SaveOptions();

			return TRUE;
		}

		break;
	}
	return FALSE;
}

static unsigned int CALLBACK regen_key_thread(void* param)
{
	Thread_Push(0);
	PROTOREGENKEYOPTIONS *opts = (PROTOREGENKEYOPTIONS *)param;
	TCHAR buff[512];

	mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_OTR_ASK_NEWKEY), opts->proto);
	EnableWindow(opts->refresh, FALSE);
	if (IDYES == MessageBox(opts->refresh, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION|MB_YESNO))
	{
		char* proto = mir_t2a(opts->proto);
		otr_gui_create_privkey(0, proto, proto);
		SendMessage(opts->refresh, WMU_REFRESHPROTOLIST, 0, 0);
		/*
		char *fp = (char*)mir_alloc(20);
		otrl_privkey_fingerprint(otr_user_state, fp, proto, proto);
		TCHAR *fpt = mir_a2t(fp);
		mir_free(fp);
		ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS), sel, 2, fpt);
		mir_free(fpt);
		mir_free(proto);
		*/
	}
	EnableWindow(opts->refresh, TRUE);
	delete opts;
	Thread_Pop();
	return 0;
}

static INT_PTR CALLBACK DlgProcMirOTROptsProto(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND lv;
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		lv = GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS);

		{
			HWND cmb = GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_DEFAULT));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_ALWAYS));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_OPP));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_MANUAL));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_NEVER));
			SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), FALSE);
		}

		SendMessage(lv,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);
		{
			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT(LANG_PROTO);
			lvc.cx = 85;     // width of column in pixels
			ListView_InsertColumn(lv, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT(LANG_POLICY);
			lvc.cx = 80;     // width of column in pixels
			ListView_InsertColumn(lv, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateT(LANG_FINGERPRINT);
			lvc.cx = 275;     // width of column in pixels
			ListView_InsertColumn(lv, 2, &lvc);
		}
		PostMessage(hwndDlg, WMU_REFRESHPROTOLIST, 0, 0);

		return TRUE;
	case WMU_REFRESHPROTOLIST:
		{
			lv = GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS);
			ListView_DeleteAllItems(lv);
			int num_protocols;
			PROTOACCOUNT **pppDesc;
			int ilvItem;
			LV_ITEM item = {0};
			char fprint[45];
			TCHAR* temp;
			//BOOL unicode = ListView_GetUnicodeFormat(lv);

			ProtoEnumAccounts(&num_protocols, &pppDesc);
			for(int i = 0; i < num_protocols; i++) {
				if((strcmp(pppDesc[i]->szModuleName, META_PROTO) != 0)
					&& (CallProtoService(pppDesc[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) == PF1_IM)
				{
						item.mask = LVIF_TEXT;
						temp = pppDesc[i]->tszAccountName;
						item.pszText = temp;
						ilvItem = ListView_InsertItem(lv, &item);


						ListView_SetItemText(lv, ilvItem, 1, (TCHAR*)policy_to_string(db_get_dw(0,MODULENAME"_ProtoPol", pppDesc[i]->szModuleName, CONTACT_DEFAULT_POLICY)) );
						if(otrl_privkey_fingerprint(otr_user_state, fprint, pppDesc[i]->szModuleName, pppDesc[i]->szModuleName)) {
							temp = mir_a2t(fprint);
							ListView_SetItemText(lv, ilvItem, 2, temp);
							mir_free(temp);
						}
				}
			}
		}
		return TRUE;
	case WM_COMMAND:
		switch ( HIWORD( wParam )) {
			case BN_CLICKED:
				switch ( LOWORD( wParam )) {
					case IDC_BTN_PROTO_NEWKEY:
						{
							int sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS));
							if (sel != -1) {
								PROTOREGENKEYOPTIONS *opts = new PROTOREGENKEYOPTIONS();
								opts->refresh = hwndDlg;
								ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS), sel, 0, opts->proto, SIZEOF(opts->proto));
								CloseHandle((HANDLE)_beginthreadex(0, 0, regen_key_thread, opts, 0, 0));
							}
						}break;
					case IDC_BTN_PROTO_FORGET:
						{
							int sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS));
							if (sel != -1) {
								TCHAR buff_proto[128];
								ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS), sel, 0, buff_proto, SIZEOF(buff_proto));
								TCHAR buff[512];
								mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_OTR_ASK_REMOVEKEY), buff_proto);
								if (IDYES == MessageBox(hwndDlg, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION|MB_YESNO))
								{
									char* proto = mir_t2a(buff_proto);
									OtrlPrivKey *key = otrl_privkey_find(otr_user_state, proto, proto);
									mir_free(proto);
									if (key) {
										otrl_privkey_forget(key);
										otrl_privkey_write(otr_user_state, g_private_key_filename);
										ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS), sel, 2, _T(""));
									}
								}
							}
						}break;
				}
				break;
			case CBN_SELCHANGE:
				switch ( LOWORD( wParam )) {
					case IDC_CMB_PROTO_POLICY:
						{
							int proto = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS));
							if (proto == -1) break;
							int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETCURSEL, 0, 0);
							if (sel==CB_ERR) break;
							int len = SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETLBTEXTLEN, sel, 0);
							if (len < 0) break;
							TCHAR *text = new TCHAR[len+1];
							SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETLBTEXT, sel, (LPARAM)text);
							ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS), proto, 1, text);
							delete[] text;
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}break;
				}
				break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR) lParam)->code == (UINT) LVN_ITEMCHANGED && ((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS)
			&& (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED )) {
			int sel = ListView_GetSelectionMark(((LPNMHDR) lParam)->hwndFrom);
			if (sel == -1) {
				SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), FALSE);
			} else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), TRUE);
				TCHAR buff[50];
				ListView_GetItemText(((LPNMHDR)lParam)->hwndFrom, sel, 1, buff, SIZEOF(buff));
				SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SELECTSTRING, (LPARAM)-1, (WPARAM)buff);
			}

		} else if (((LPNMHDR)lParam)->code == (UINT) PSN_APPLY ) {
			lv = GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS);
			int cnt = ListView_GetItemCount(lv);
			TCHAR proto_t[128], policy[64];
			char* proto;
			for (int i = 0; i < cnt; ++i) {
				ListView_GetItemText(lv, i, 0, proto_t, SIZEOF(proto_t));
				ListView_GetItemText(lv, i, 1, policy, SIZEOF(policy));
				proto = mir_t2a(proto_t);
				db_set_dw(0, MODULENAME"_ProtoPol", proto, policy_from_string(policy));
				mir_free(proto);
			}
			// handle apply
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcMirOTROptsContacts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		{
			TranslateDialogDefault( hwndDlg );

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) new ContactPolicyMap());

			HWND cmb = GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_DEFAULT));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_ALWAYS));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_OPP));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_MANUAL));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateT(LANG_POLICY_NEVER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), FALSE);

			SendDlgItemMessage(hwndDlg, IDC_LV_CONT_CONTACTS ,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);


			HWND lv = GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS);


			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT(LANG_CONTACT);
			lvc.cx = 150;     // width of column in pixels
			ListView_InsertColumn(lv, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT(LANG_PROTO);
			lvc.cx = 100;     // width of column in pixels
			ListView_InsertColumn(lv, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateT(LANG_POLICY);
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(lv, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = TranslateT(LANG_HTMLCONV);
			lvc.cx = 80;     // width of column in pixels
			ListView_InsertColumn(lv, 3, &lvc);
		}
		SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);

		return TRUE;
	case WMU_REFRESHLIST:
		{
			HWND lv = GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS);

			ListView_DeleteAllItems(lv);

			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
				const char *proto = GetContactProto(hContact);
				if(proto && db_get_b(hContact, proto, "ChatRoom", 0) == 0 && CallService(MS_PROTO_ISPROTOONCONTACT, hContact, (LPARAM)MODULENAME) // ignore chatrooms
					&& strcmp(proto, META_PROTO) != 0) // and MetaContacts
				{
					lvI.iItem = 0;
					lvI.iSubItem = 0;
					lvI.lParam = hContact;
					lvI.pszText = (TCHAR*)contact_get_nameT(hContact);
					lvI.iItem = ListView_InsertItem(lv , &lvI);

					PROTOACCOUNT *pa = ProtoGetAccount(proto);
					ListView_SetItemText(lv, lvI.iItem, 1, pa->tszAccountName);

					ListView_SetItemText(lv, lvI.iItem, 2, (TCHAR*)policy_to_string((OtrlPolicy)db_get_dw(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY)) );
					ListView_SetItemText(lv, lvI.iItem, 3, (db_get_b(hContact, MODULENAME, "HTMLConv", 0))?TranslateT(LANG_YES):TranslateT(LANG_NO) );
				}
			}
		}
		return TRUE;
		break;

	case WM_COMMAND:
		switch ( HIWORD( wParam )) {
			case CBN_SELCHANGE:
				switch ( LOWORD( wParam )) {
					case IDC_CMB_CONT_POLICY:
						{
							MCONTACT hContact = 0;
							int iUser = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS));
							if (iUser == -1) break;
							int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETCURSEL, 0, 0);
							if (sel==CB_ERR) break;
							int len = SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETLBTEXTLEN, sel, 0);
							if (len < 0) break;
							TCHAR *text = new TCHAR[len+1];
							SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETLBTEXT, sel, (LPARAM)text);
							ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), iUser, 2, text);
							OtrlPolicy policy = policy_from_string(text);
							delete[] text;
							LVITEM lvi = {0};
							lvi.mask = LVIF_PARAM;
							lvi.iItem = iUser;
							lvi.iSubItem = 0;
							ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), &lvi);
							ContactPolicyMap* cpm = (ContactPolicyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
							hContact = (MCONTACT)lvi.lParam;
							(*cpm)[hContact].policy = policy;
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
						}break;
				}
				break;
		}
		break;

	case WM_NOTIFY:
		{
		UINT code = ((LPNMHDR) lParam)->code;
		if (code == (UINT) PSN_APPLY ) {
			// handle apply

			ContactPolicyMap *cpm = (ContactPolicyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			// Iterate over the map and print out all key/value pairs.
			// Using a const_iterator since we are not going to change the values.
			for(ContactPolicyMap::const_iterator it = cpm->begin(); it != cpm->end(); ++it)
			{
				if (!it->first) continue;
				if (it->second.policy) db_set_dw(it->first, MODULENAME, "Policy", (DWORD)it->second.policy);
				if (it->second.htmlconv) db_set_b(it->first, MODULENAME, "HTMLConv", it->second.htmlconv-1);
			}
			return TRUE;
		} else if (((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS)) {
			if (code == (UINT) LVN_ITEMCHANGED && (((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED )) {
				int sel = ListView_GetSelectionMark(((LPNMHDR) lParam)->hwndFrom);
				if (sel == -1) {
					SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), FALSE);
				} else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), TRUE);
					TCHAR buff[50];
					ListView_GetItemText(((LPNMHDR)lParam)->hwndFrom, sel, 2, buff, SIZEOF(buff));
					SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_SELECTSTRING, (LPARAM)-1, (WPARAM)buff);
				}
			} else if (code == (UINT) NM_CLICK) {
				if (((LPNMLISTVIEW)lParam)->iSubItem == 3) {
					LVITEM lvi;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((LPNMLISTVIEW)lParam)->iItem;
					if (lvi.iItem < 0) return FALSE;
					lvi.iSubItem = 0;
					SendDlgItemMessage(hwndDlg, IDC_LV_CONT_CONTACTS, LVM_GETITEM, 0, (LPARAM)&lvi);

					MCONTACT hContact = (MCONTACT)lvi.lParam;
					ContactPolicyMap *cp = (ContactPolicyMap *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					TCHAR buff[50];
					ListView_GetItemText(((LPNMHDR)lParam)->hwndFrom, lvi.iItem, 3, buff, SIZEOF(buff));
					if (_tcsncmp(buff, TranslateT(LANG_YES), 50)==0){
						(*cp)[hContact].htmlconv = HTMLCONV_DISABLE;
						ListView_SetItemText(((LPNMHDR)lParam)->hwndFrom, lvi.iItem, 3, TranslateT(LANG_NO));
					}else {
						(*cp)[hContact].htmlconv = HTMLCONV_ENABLE;
						ListView_SetItemText(((LPNMHDR)lParam)->hwndFrom, lvi.iItem, 3, TranslateT(LANG_YES));
					}
					(*cp)[hContact].htmlconv += 1;
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}


			}
		}
		}break;
	case WM_DESTROY:
		ContactPolicyMap *cpm = (ContactPolicyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		cpm->clear();
		delete cpm;
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcMirOTROptsFinger(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) new FPModifyMap());

		SendDlgItemMessage(hwndDlg, IDC_LV_FINGER_LIST ,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);

		{
			HWND lv = GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST);
			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT(LANG_CONTACT);
			lvc.cx = 100;     // width of column in pixels
			ListView_InsertColumn(lv, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateT(LANG_PROTO);
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(lv, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateT(LANG_ACTIVE);
			lvc.cx = 50;     // width of column in pixels
			ListView_InsertColumn(lv, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = TranslateT(LANG_VERIFIED);
			lvc.cx = 50;     // width of column in pixels
			ListView_InsertColumn(lv, 3, &lvc);

			lvc.iSubItem = 4;
			lvc.pszText = TranslateT(LANG_FINGERPRINT);
			lvc.cx = 300;     // width of column in pixels
			ListView_InsertColumn(lv, 4, &lvc);
		}
		SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);

		return TRUE;

	case WMU_REFRESHLIST:
		//enumerate contacts, fill in list
		{
			HWND lv = GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST);
			ListView_DeleteAllItems(lv);

			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE;
			
			TCHAR *user, hash[45] = {0};
			for (ConnContext *context = otr_user_state->context_root;context;context = context->next) {
				if (context->app_data) {
					user = (TCHAR*)contact_get_nameT((MCONTACT)context->app_data);
					if (user) {
						PROTOACCOUNT *pa = ProtoGetAccount(context->protocol);
						
						for(Fingerprint *fp = context->fingerprint_root.next;fp;fp = fp->next) {
							otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
							lvI.iSubItem = 0;
							lvI.lParam = (LPARAM)fp;
							lvI.pszText = user;
							int d = ListView_InsertItem(lv, &lvI);

							ListView_SetItemText(lv,d, 1, pa->tszAccountName);
							ListView_SetItemText(lv,d, 2, (context->active_fingerprint == fp)? TranslateT(LANG_YES) : TranslateT(LANG_NO));
							ListView_SetItemText(lv,d, 3, (fp->trust && fp->trust[0] != '\0')? TranslateT(LANG_YES) : TranslateT(LANG_NO));
							ListView_SetItemText(lv,d, 4, hash );
						}
					}
				}
			}
		}

			return TRUE;
	case WM_COMMAND:
		switch ( HIWORD( wParam )) {
			case BN_CLICKED:
				switch ( LOWORD( wParam )) {
					case IDC_BTN_FINGER_DONTTRUST:
						{
							int sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
							if (sel != -1) {
								LVITEM lvi = {0};
								lvi.mask = LVIF_PARAM;
								lvi.iItem = sel;
								ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
								Fingerprint *fp = (Fingerprint*) lvi.lParam;
								FPModifyMap* fpm = (FPModifyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
								(*fpm)[fp] = FPM_NOTRUST;
								ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel, 3, TranslateT(LANG_NO));
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}break;
					case IDC_BTN_FINGER_TRUST:
						{
							int sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
							if (sel != -1) {
								LVITEM lvi = {0};
								lvi.mask = LVIF_PARAM;
								lvi.iItem = sel;
								ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
								Fingerprint *fp = (Fingerprint*) lvi.lParam;
								FPModifyMap* fpm = (FPModifyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
								(*fpm)[fp] = FPM_VERIFY;
								ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel, 3, TranslateT(LANG_YES));
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
							}
						}break;
					case IDC_BTN_FINGER_FORGET:
						{
							int sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
							if (sel != -1) {
								LVITEM lvi = {0};
								lvi.mask = LVIF_PARAM;
								lvi.iItem = sel;
								ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
								Fingerprint *fp = (Fingerprint*) lvi.lParam;
								if (fp->context->active_fingerprint == fp) {
									MCONTACT hContact = (MCONTACT)fp->context->app_data;
									TCHAR buff[1024], hash[45];
									otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
									PROTOACCOUNT *pa = ProtoGetAccount(GetContactProto(hContact));
									mir_sntprintf(buff, SIZEOF(buff)-1, TranslateT(LANG_FINGERPRINT_STILL_IN_USE), hash, contact_get_nameT(hContact), pa->tszAccountName);
									buff[SIZEOF(buff)-1] = '\0';
									ShowError(buff);
								} else {
									FPModifyMap* fpm = (FPModifyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
									(*fpm)[fp] = FPM_DELETE;
									ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel);
									SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
								}
							}
						}break;
				}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT) PSN_APPLY ) {
			// handle apply

			FPModifyMap *fpm = (FPModifyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			// Iterate over the map and print out all key/value pairs.
			// Using a const_iterator since we are not going to change the values.
			for(FPModifyMap::const_iterator it = fpm->begin(); it != fpm->end(); ++it)
			{
				if (!it->first) continue;
				switch (it->second) {
					case FPM_DELETE:
						if (it->first->context->active_fingerprint == it->first) {
							MCONTACT hContact = (MCONTACT)it->first->context->app_data;
							TCHAR buff[1024], hash[45];
							otrl_privkey_hash_to_humanT(hash, it->first->fingerprint);
							PROTOACCOUNT *pa = ProtoGetAccount(GetContactProto(hContact));
							mir_sntprintf(buff, SIZEOF(buff)-1, TranslateT(LANG_FINGERPRINT_NOT_DELETED), hash, contact_get_nameT(hContact), pa->tszAccountName);
							buff[SIZEOF(buff)-1] = '\0';
							ShowError(buff);
						} else {
							otrl_context_forget_fingerprint(it->first, 1);
						}
						break;
					case FPM_VERIFY:
						otrl_context_set_trust(it->first, "verified");
						if (it->first == it->first->context->active_fingerprint)
							VerifyFingerprint(it->first->context, true);
							//SetEncryptionStatus((HANDLE)it->first->context->app_data, otr_context_get_trust(it->first->context));
						break;
					case FPM_NOTRUST:
						otrl_context_set_trust(it->first, NULL);
						if (it->first == it->first->context->active_fingerprint)
							VerifyFingerprint(it->first->context, false);
							//SetEncryptionStatus((HANDLE)it->first->context->app_data, otr_context_get_trust(it->first->context));
						break;
				}
			}
			if (!fpm->empty())	otr_gui_write_fingerprints(0);
			fpm->clear();
			SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);

			return TRUE;
		}
		break;
	case WM_DESTROY:
		FPModifyMap *fpm = (FPModifyMap*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		fpm->clear();
		delete fpm;
		break;
	}

	return FALSE;
}
