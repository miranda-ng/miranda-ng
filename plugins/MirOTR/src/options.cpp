#include "stdafx.h"
extern "C"{
	#include "otrlextensions.h"
}

wchar_t g_private_key_filename[MAX_PATH];
wchar_t g_fingerprint_store_filename[MAX_PATH];
wchar_t g_instag_filename[MAX_PATH];
HANDLE hPATH_MIROTR;
Options options;
#define DATA_DIRECTORY MIRANDA_USERDATAW L"\\" _A2W(MODULENAME)

struct PROTOREGENKEYOPTIONS
{
	HWND refresh;
	char *szProto;
	wchar_t wszTitle[129];
};

void SetFilenames(const wchar_t *path)
{
	if (!path || !path[0]) 
		return;
	CreateDirectoryTreeW(path);
	
	mir_snwprintf(g_private_key_filename,       L"%s\\" _A2W(PRIVATE_KEY_FILENAME),       path);
	mir_snwprintf(g_fingerprint_store_filename, L"%s\\" _A2W(FINGERPRINT_STORE_FILENAME), path);
	mir_snwprintf(g_instag_filename,            L"%s\\" _A2W(INSTAG_FILENAME),            path);
}

int FoldersChanged(WPARAM, LPARAM)
{
	wchar_t path[MAX_PATH];
	if ( FoldersGetCustomPathW(hPATH_MIROTR, path, _countof(path), L""))
		SetFilenames(VARSW(DATA_DIRECTORY));
	else
		SetFilenames(path);

	ReadPrivkeyFiles();
	return 0;
}

void LoadFilenames()
{
	if (hPATH_MIROTR = FoldersRegisterCustomPathW("OTR", LPGEN("Private Data"), DATA_DIRECTORY)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersChanged);

		// get the path - above are only defaults - there may be a different value in the db
		FoldersChanged(0, 0);
	}
	else {
		SetFilenames( VARSW(DATA_DIRECTORY));
		ReadPrivkeyFiles();
	}
}

void LoadOptions()
{
	options.default_policy = g_plugin.getWord("DefaultPolicy", OTRL_POLICY_OPPORTUNISTIC);
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
	options.err_method = (ErrorDisplay)g_plugin.getWord("ErrorDisplay", ED_POP);
	options.prefix_messages = (g_plugin.getByte("PrefixMessages", 0) == 1);
	options.msg_inline = (g_plugin.getByte("MsgInline", 0) == 1);
	options.msg_popup = (g_plugin.getByte("MsgPopup", 1) == 1);
	options.delete_history = (g_plugin.getByte("NoHistory", 0) == 1);
	options.delete_systeminfo = (g_plugin.getByte("NoSystemHistory", 0) == 1);
	options.autoshow_verify = (g_plugin.getByte("AutoShowVerify", 1) == 1);

	DBVARIANT dbv;
	if (!db_get_utf(0, MODULENAME, "Prefix", &dbv)) {
		mir_strncpy(options.prefix, dbv.pszVal, OPTIONS_PREFIXLEN);
		db_free(&dbv);
	} else
		mir_strncpy(options.prefix, OPTIONS_DEFAULT_PREFIX, OPTIONS_PREFIXLEN);

	options.end_offline = (g_plugin.getByte("EndOffline", 1) == 1);
	options.end_window_close = (g_plugin.getByte("EndWindowClose", 0) == 1);
	options.bHaveSecureIM = 0 != ServiceExists("SecureIM/IsContactSecured");

	LoadFilenames();
}

void SaveOptions()
{
	g_plugin.setWord("DefaultPolicy", options.default_policy);
	g_plugin.setWord("ErrorDisplay", (int)options.err_method);
	g_plugin.setByte("PrefixMessages", options.prefix_messages ? 1 : 0);
	g_plugin.setByte("MsgInline", options.msg_inline ? 1 : 0);
	g_plugin.setByte("MsgPopup", options.msg_popup ? 1 : 0);

	g_plugin.setByte("NoHistory", options.delete_history ? 1 : 0);
	g_plugin.setByte("NoSystemHistory", options.delete_systeminfo ? 1 : 0);
	g_plugin.setByte("AutoShowVerify", options.autoshow_verify ? 1 : 0);

	db_set_utf(0, MODULENAME, "Prefix", options.prefix);

	g_plugin.setByte("EndOffline", options.end_offline ? 1 : 0);
	g_plugin.setByte("EndWindowClose", options.end_window_close ? 1 : 0);
}

extern "C" void set_context_contact(void *, ConnContext *context)
{
	MCONTACT hContact = find_contact(context->username, context->protocol);
	context->app_data = (void*)hContact;
}

void ReadPrivkeyFiles()
{
	DEBUGOUTA("READ privkey");

	mir_cslock lck(lib_cs);
	otrl_privkey_read(otr_user_state, g_private_key_filename);
	otrl_privkey_read_fingerprints(otr_user_state, g_fingerprint_store_filename, set_context_contact, nullptr);
	otrl_instag_read(otr_user_state, g_instag_filename);
}

static INT_PTR CALLBACK DlgProcMirOTROpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInit = true;
	wchar_t *prefix;
	switch ( msg ) {
	case WM_INITDIALOG:
		bInit = false;
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

		prefix = mir_utf8decodeW(options.prefix);
		SetDlgItemText(hwndDlg, IDC_ED_PREFIX, prefix);
		mir_free(prefix);
		bInit = true;
		return FALSE;

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
						if (bInit)
							SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case EN_CHANGE:
				if ((LOWORD( wParam ) == IDC_ED_PREFIX) && bInit)
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

			prefix = (wchar_t*)_alloca(sizeof(wchar_t)*OPTIONS_PREFIXLEN);
			GetDlgItemText(hwndDlg, IDC_ED_PREFIX, prefix, OPTIONS_PREFIXLEN);

			T2Utf prefix_utf(prefix);
			if (!mir_strlen(prefix_utf)) {
				SetDlgItemTextA(hwndDlg, IDC_ED_PREFIX, OPTIONS_DEFAULT_PREFIX);
				mir_strncpy(options.prefix, OPTIONS_DEFAULT_PREFIX, OPTIONS_PREFIXLEN);
			}
			else mir_strncpy(options.prefix, prefix_utf, OPTIONS_PREFIXLEN);

			SaveOptions();
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static unsigned int CALLBACK regen_key_thread(void* param)
{
	Thread_Push(nullptr);
	PROTOREGENKEYOPTIONS *opts = (PROTOREGENKEYOPTIONS *)param;
	wchar_t buff[512];

	mir_snwprintf(buff, TranslateW(LANG_OTR_ASK_NEWKEY), opts->wszTitle);
	EnableWindow(opts->refresh, FALSE);
	if (IDYES == MessageBox(opts->refresh, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION|MB_YESNO)) {
		otr_gui_create_privkey(nullptr, opts->szProto, opts->szProto);
		SendMessage(opts->refresh, WMU_REFRESHPROTOLIST, 0, 0);
	}
	EnableWindow(opts->refresh, TRUE);
	delete opts;
	Thread_Pop();
	return 0;
}

static char* GetProtoName(HWND hwndList, int iItem)
{
	LV_ITEM item;
	item.iItem = iItem;
	item.mask = LVIF_PARAM;
	return (ListView_GetItem(hwndList, &item) == -1) ? nullptr : (char*)item.lParam;
}

static void ChangeContactSetting(HWND hwndDlg, int iItem, bool changeHtml)
{
	if (iItem < 0)
		return;

	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	
	ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), &lvi);

	ContactPolicyMap* cpm = (ContactPolicyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	MCONTACT hContact = (MCONTACT)lvi.lParam;

	// Handle HtmlConv
	{
		wchar_t buff[50];
		ListView_GetItemText(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), lvi.iItem, 3, buff, _countof(buff));

		bool htmlEnabled = !wcsncmp(buff, TranslateW(LANG_YES), 50);
		if (changeHtml) {
			htmlEnabled = !htmlEnabled;
		}

		// Update wanted state
		(*cpm)[hContact].htmlconv = htmlEnabled ? HTMLCONV_ENABLE : HTMLCONV_DISABLE;
		ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), lvi.iItem, 3, TranslateW(htmlEnabled ? LANG_YES : LANG_NO));
	}
	
	// Handle Policy
	{
		OtrlPolicy policy = CONTACT_DEFAULT_POLICY;

		int sel = SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETCURSEL, 0, 0);
		if (sel != CB_ERR) {
			int len = SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETLBTEXTLEN, sel, 0);
			if (len >= 0) {
				wchar_t *text = new wchar_t[len + 1];
				SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_GETLBTEXT, sel, (LPARAM)text);
				ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS), iItem, 2, text);
				policy = policy_from_string(text);
				delete[] text;
			}
		}

		(*cpm)[hContact].policy = policy;
	}

	SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
}

static INT_PTR CALLBACK DlgProcMirOTROptsProto(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LV_PROTO_PROTOS);
	int sel;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			HWND cmb = GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_DEFAULT));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_ALWAYS));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_OPP));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_MANUAL));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_NEVER));
			SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), FALSE);
		}

		SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);
		{
			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateW(LANG_PROTO);
			lvc.cx = 85;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateW(LANG_POLICY);
			lvc.cx = 80;     // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateW(LANG_FINGERPRINT);
			lvc.cx = 275;     // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);
		}
		PostMessage(hwndDlg, WMU_REFRESHPROTOLIST, 0, 0);
		return TRUE;

	case WMU_REFRESHPROTOLIST:
		ListView_DeleteAllItems(hwndList);
		{
			LV_ITEM item = { 0 };

			for (auto &pa : Accounts()) {
				if (!mir_strcmp(pa->szModuleName, META_PROTO))
					continue;
				if ((CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) == 0)
					continue;

				item.mask = LVIF_TEXT | LVIF_PARAM;
				item.pszText = pa->tszAccountName;
				item.lParam = (LPARAM)pa->szModuleName;
				int ilvItem = ListView_InsertItem(hwndList, &item);

				ListView_SetItemText(hwndList, ilvItem, 1, (wchar_t*)policy_to_string(db_get_dw(0, MODULENAME"_ProtoPol", pa->szModuleName, CONTACT_DEFAULT_POLICY)));

				char fprint[45];
				if (otrl_privkey_fingerprint(otr_user_state, fprint, pa->szModuleName, pa->szModuleName)) {
					wchar_t *temp = mir_a2u(fprint);
					ListView_SetItemText(hwndList, ilvItem, 2, temp);
					mir_free(temp);
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_BTN_PROTO_NEWKEY:
				sel = ListView_GetSelectionMark(hwndList);
				if (sel != -1) {
					PROTOREGENKEYOPTIONS *opts = new PROTOREGENKEYOPTIONS();
					opts->refresh = hwndDlg;
					opts->szProto = GetProtoName(hwndList, sel);
					ListView_GetItemText(hwndList, sel, 0, opts->wszTitle, _countof(opts->wszTitle));
					CloseHandle((HANDLE)_beginthreadex(nullptr, 0, regen_key_thread, opts, 0, nullptr));
				}
				break;
			
			case IDC_BTN_PROTO_FORGET:
				sel = ListView_GetSelectionMark(hwndList);
				if (sel != -1) {
					wchar_t buff_proto[128];
					ListView_GetItemText(hwndList, sel, 0, buff_proto, _countof(buff_proto));
					wchar_t buff[512];
					mir_snwprintf(buff, TranslateW(LANG_OTR_ASK_REMOVEKEY), buff_proto);
					if (IDYES == MessageBox(hwndDlg, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION | MB_YESNO)) {
						char *proto = GetProtoName(hwndList, sel);
						if (proto == nullptr)
							break;

						OtrlPrivKey *key = otrl_privkey_find(otr_user_state, proto, proto);
						if (key) {
							otrl_privkey_forget(key);
							otrl_privkey_write(otr_user_state, g_private_key_filename);
							ListView_SetItemText(hwndList, sel, 2, L"");
						}
					}
				}
			}
			break;

		case CBN_SELCHANGE:
			switch (LOWORD(wParam)) {
			case IDC_CMB_PROTO_POLICY:
				int proto = ListView_GetSelectionMark(hwndList);
				if (proto == -1) break;
				sel = SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETCURSEL, 0, 0);
				if (sel == CB_ERR) break;
				int len = SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETLBTEXTLEN, sel, 0);
				if (len < 0) break;
				wchar_t *text = new wchar_t[len + 1];
				SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_GETLBTEXT, sel, (LPARAM)text);
				ListView_SetItemText(hwndList, proto, 1, text);
				delete[] text;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED && ((LPNMHDR)lParam)->hwndFrom == hwndList) {
			LPNMLISTVIEW notif = (LPNMLISTVIEW)lParam;
			if ((notif->uNewState & LVIS_SELECTED) == 0)
				break;

			if (notif->iItem == -1) {
				SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), FALSE);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_PROTO_POLICY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_NEWKEY), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_PROTO_FORGET), TRUE);
				wchar_t buff[50];
				ListView_GetItemText(((LPNMHDR)lParam)->hwndFrom, notif->iItem, 1, buff, _countof(buff));
				SendDlgItemMessage(hwndDlg, IDC_CMB_PROTO_POLICY, CB_SELECTSTRING, (LPARAM)-1, (WPARAM)buff);
			}
		}
		else if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int cnt = ListView_GetItemCount(hwndList);
			wchar_t policy[64];
			for (int i = 0; i < cnt; ++i) {
				char *proto = GetProtoName(hwndList, i);
				if (proto == nullptr)
					continue;

				ListView_GetItemText(hwndList, i, 1, policy, _countof(policy));
				db_set_dw(0, MODULENAME"_ProtoPol", proto, policy_from_string(policy));
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
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) new ContactPolicyMap());

			HWND cmb = GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY);
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_DEFAULT));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_ALWAYS));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_OPP));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_MANUAL));
			SendMessage(cmb, CB_ADDSTRING, 0, (WPARAM)TranslateW(LANG_POLICY_NEVER));
			EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), FALSE);

			SendDlgItemMessage(hwndDlg, IDC_LV_CONT_CONTACTS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);

			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateW(LANG_CONTACT);
			lvc.cx = 150;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateW(LANG_PROTO);
			lvc.cx = 100;     // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateW(LANG_POLICY);
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = TranslateW(LANG_HTMLCONV);
			lvc.cx = 80;     // width of column in pixels
			ListView_InsertColumn(hwndList, 3, &lvc);
		}
		SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);
		return TRUE;

	case WMU_REFRESHLIST:
		ListView_DeleteAllItems(hwndList);
		{
			LVITEM lvI = { 0 };

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			for (auto &hContact : Contacts()) {
				const char *proto = Proto_GetBaseAccountName(hContact);
				if (proto && db_get_b(hContact, proto, "ChatRoom", 0) == 0 && Proto_IsProtoOnContact(hContact, MODULENAME) // ignore chatrooms
					&& mir_strcmp(proto, META_PROTO) != 0) // and MetaContacts
				{
					lvI.iItem = 0;
					lvI.iSubItem = 0;
					lvI.lParam = hContact;
					lvI.pszText = (wchar_t*)contact_get_nameT(hContact);
					lvI.iItem = ListView_InsertItem(hwndList, &lvI);

					PROTOACCOUNT *pa = Proto_GetAccount(proto);
					ListView_SetItemText(hwndList, lvI.iItem, 1, pa->tszAccountName);

					ListView_SetItemText(hwndList, lvI.iItem, 2, (wchar_t*)policy_to_string((OtrlPolicy)g_plugin.getDword(hContact, "Policy", CONTACT_DEFAULT_POLICY)));
					ListView_SetItemText(hwndList, lvI.iItem, 3, (g_plugin.getByte(hContact, "HTMLConv", 0)) ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case CBN_SELCHANGE:
			switch (LOWORD(wParam)) {
			case IDC_CMB_CONT_POLICY:
				int iUser = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_CONT_CONTACTS));
				ChangeContactSetting(hwndDlg, iUser, false);
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			// handle apply
			ContactPolicyMap *cpm = (ContactPolicyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

			// Iterate over the map and print out all key/value pairs.
			// Using a const_iterator since we are not going to change the values.
			for (ContactPolicyMap::const_iterator it = cpm->begin(); it != cpm->end(); ++it) {
				if (!it->first) continue;
				db_set_dw(it->first, MODULENAME, "Policy", (DWORD)it->second.policy);
				db_set_b(it->first, MODULENAME, "HTMLConv", it->second.htmlconv);
			}
			return TRUE;
		}
		if (((LPNMHDR)lParam)->hwndFrom == hwndList) {
			LPNMLISTVIEW notif = (LPNMLISTVIEW)lParam;
			if (((LPNMHDR)lParam)->code == LVN_ITEMCHANGED && (notif->uNewState & LVIS_SELECTED)) {
				if (notif->iItem == -1) {
					SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_SETCURSEL, (LPARAM)-1, 0);
					EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), FALSE);
				}
				else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_CONT_POLICY), TRUE);
					wchar_t buff[50];
					ListView_GetItemText(((LPNMHDR)lParam)->hwndFrom, notif->iItem, 2, buff, _countof(buff));
					SendDlgItemMessage(hwndDlg, IDC_CMB_CONT_POLICY, CB_SELECTSTRING, (LPARAM)-1, (WPARAM)buff);
				}
			}
			else if (((LPNMHDR)lParam)->code == NM_CLICK) {
				if (notif->iSubItem == 3) {
					ChangeContactSetting(hwndDlg, notif->iItem, true);
				}
			}
		}
		break;

	case WM_DESTROY:
		ContactPolicyMap *cpm = (ContactPolicyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		cpm->clear();
		delete cpm;
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcMirOTROptsFinger(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) new FPModifyMap());

		SendDlgItemMessage(hwndDlg, IDC_LV_FINGER_LIST, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);
		{
			// add list columns
			LVCOLUMN lvc;
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid.
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateW(LANG_CONTACT);
			lvc.cx = 100;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = TranslateW(LANG_PROTO);
			lvc.cx = 90;     // width of column in pixels
			ListView_InsertColumn(hwndList, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = TranslateW(LANG_ACTIVE);
			lvc.cx = 50;     // width of column in pixels
			ListView_InsertColumn(hwndList, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = TranslateW(LANG_VERIFIED);
			lvc.cx = 50;     // width of column in pixels
			ListView_InsertColumn(hwndList, 3, &lvc);

			lvc.iSubItem = 4;
			lvc.pszText = TranslateW(LANG_FINGERPRINT);
			lvc.cx = 300;     // width of column in pixels
			ListView_InsertColumn(hwndList, 4, &lvc);
		}
		SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);
		return TRUE;

	case WMU_REFRESHLIST:
		// enumerate contacts, fill in list
		ListView_DeleteAllItems(hwndList);
		{
			LVITEM lvI = { 0 };

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items.
			lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE;

			for (ConnContext *context = otr_user_state->context_root; context; context = context->next) {
				if (context->app_data) {
					wchar_t *user = (wchar_t*)contact_get_nameT((UINT_PTR)context->app_data);
					if (user) {
						PROTOACCOUNT *pa = Proto_GetAccount(context->protocol);

						wchar_t hash[45];
						for (Fingerprint *fp = context->fingerprint_root.next; fp; fp = fp->next) {
							otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
							lvI.iSubItem = 0;
							lvI.lParam = (LPARAM)fp;
							lvI.pszText = user;
							int d = ListView_InsertItem(hwndList, &lvI);

							ListView_SetItemText(hwndList, d, 1, pa->tszAccountName);
							ListView_SetItemText(hwndList, d, 2, (context->active_fingerprint == fp) ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
							ListView_SetItemText(hwndList, d, 3, (fp->trust && fp->trust[0] != '\0') ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
							ListView_SetItemText(hwndList, d, 4, hash);
						}
					}
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		int sel;
		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_BTN_FINGER_DONTTRUST:
				sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
				if (sel != -1) {
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = sel;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
					Fingerprint *fp = (Fingerprint*)lvi.lParam;
					FPModifyMap* fpm = (FPModifyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					(*fpm)[fp] = FPM_NOTRUST;
					ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel, 3, TranslateW(LANG_NO));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			case IDC_BTN_FINGER_TRUST:
				sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
				if (sel != -1) {
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = sel;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
					Fingerprint *fp = (Fingerprint*)lvi.lParam;
					FPModifyMap* fpm = (FPModifyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					(*fpm)[fp] = FPM_VERIFY;
					ListView_SetItemText(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel, 3, TranslateW(LANG_YES));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
				break;
			
			case IDC_BTN_FINGER_FORGET:
				sel = ListView_GetSelectionMark(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST));
				if (sel != -1) {
					LVITEM lvi = { 0 };
					lvi.mask = LVIF_PARAM;
					lvi.iItem = sel;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), &lvi);
					Fingerprint *fp = (Fingerprint*)lvi.lParam;
					if (fp->context->active_fingerprint == fp) {
						MCONTACT hContact = (UINT_PTR)fp->context->app_data;
						wchar_t buff[1024], hash[45];
						otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
						PROTOACCOUNT *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
						mir_snwprintf(buff, TranslateW(LANG_FINGERPRINT_STILL_IN_USE), hash, contact_get_nameT(hContact), pa->tszAccountName);
						ShowError(buff);
					}
					else {
						FPModifyMap* fpm = (FPModifyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						(*fpm)[fp] = FPM_DELETE;
						ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_LV_FINGER_LIST), sel);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
			// handle apply
			FPModifyMap *fpm = (FPModifyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			// Iterate over the map and print out all key/value pairs.
			// Using a const_iterator since we are not going to change the values.
			for (FPModifyMap::const_iterator it = fpm->begin(); it != fpm->end(); ++it) {
				if (!it->first) continue;
				switch (it->second) {
				case FPM_DELETE:
					if (it->first->context->active_fingerprint == it->first) {
						MCONTACT hContact = (UINT_PTR)it->first->context->app_data;
						wchar_t buff[1024], hash[45];
						otrl_privkey_hash_to_humanT(hash, it->first->fingerprint);
						PROTOACCOUNT *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
						mir_snwprintf(buff, TranslateW(LANG_FINGERPRINT_NOT_DELETED), hash, contact_get_nameT(hContact), pa->tszAccountName);
						ShowError(buff);
					}
					else otrl_context_forget_fingerprint(it->first, 1);
					break;

				case FPM_VERIFY:
					otrl_context_set_trust(it->first, "verified");
					if (it->first == it->first->context->active_fingerprint)
						VerifyFingerprint(it->first->context, true);
					break;

				case FPM_NOTRUST:
					otrl_context_set_trust(it->first, nullptr);
					if (it->first == it->first->context->active_fingerprint)
						VerifyFingerprint(it->first->context, false);
					break;
				}
			}

			if (!fpm->empty())
				otr_gui_write_fingerprints(nullptr);
			fpm->clear();
			SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		FPModifyMap *fpm = (FPModifyMap*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		fpm->clear();
		delete fpm;
		break;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OpenOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = L"OTR";
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	odp.szTab.w = LANG_OPT_GENERAL;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENERAL);
	odp.pfnDlgProc = DlgProcMirOTROpts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LANG_OPT_PROTO;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PROTO);
	odp.pfnDlgProc = DlgProcMirOTROptsProto;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LANG_OPT_CONTACTS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	odp.pfnDlgProc = DlgProcMirOTROptsContacts;
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LANG_OPT_FINGER;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FINGER);
	odp.pfnDlgProc = DlgProcMirOTROptsFinger;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void InitOptions()
{
	LoadOptions();
	HookEvent(ME_OPT_INITIALISE, OpenOptions);
}
