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

/////////////////////////////////////////////////////////////////////////////////////////
// Account tab

#define WMU_REFRESHPROTOLIST (WM_USER + 0x100)

struct PROTOREGENKEYOPTIONS
{
	HWND refresh;
	char *szProto;
	wchar_t wszTitle[129];
};

static unsigned int CALLBACK regen_key_thread(void* param)
{
	Thread_Push(nullptr);
	PROTOREGENKEYOPTIONS *opts = (PROTOREGENKEYOPTIONS *)param;
	wchar_t buff[512];

	mir_snwprintf(buff, TranslateW(LANG_OTR_ASK_NEWKEY), opts->wszTitle);
	EnableWindow(opts->refresh, FALSE);
	if (IDYES == MessageBox(opts->refresh, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION|MB_YESNO)) {
		otr_gui_create_privkey(nullptr, opts->szProto, opts->szProto);
		PostMessage(opts->refresh, WMU_REFRESHPROTOLIST, 0, 0);
	}
	EnableWindow(opts->refresh, TRUE);
	delete opts;
	Thread_Pop();
	return 0;
}

class CAccountOptionsDlg : public CDlgBase
{
	bool bMenuEnable = false;
	CCtrlListView m_list;
	CCtrlCombo cmbPolicy;

	char* GetProtoName(int iItem)
	{
		LV_ITEM item;
		item.iItem = iItem;
		item.mask = LVIF_PARAM;
		return (!m_list.GetItem(&item)) ? nullptr : (char *)item.lParam;
	}

	void RefreshList()
	{
		m_list.DeleteAllItems();

		LV_ITEM item = { 0 };

		for (auto &pa : Accounts()) {
			if (!mir_strcmp(pa->szModuleName, META_PROTO))
				continue;
			if ((CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM) == 0)
				continue;

			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.pszText = pa->tszAccountName;
			item.lParam = (LPARAM)pa->szModuleName;
			int ilvItem = m_list.InsertItem(&item);

			m_list.SetItemText(ilvItem, 1, (wchar_t *)policy_to_string(db_get_dw(0, MODULENAME"_ProtoPol", pa->szModuleName, CONTACT_DEFAULT_POLICY)));

			char fprint[45];
			if (otrl_privkey_fingerprint(otr_user_state, fprint, pa->szModuleName, pa->szModuleName))
				m_list.SetItemText(ilvItem, 2, _A2T(fprint));
		}
	}

public:
	CAccountOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_PROTO),
		m_list(this, IDC_LV_PROTO_PROTOS),
		cmbPolicy(this, IDC_CMB_PROTO_POLICY)
	{
		cmbPolicy.OnSelChanged = Callback(this, &CAccountOptionsDlg::onSelChanged_Combo);

		m_list.OnBuildMenu = Callback(this, &CAccountOptionsDlg::onContextMenu_List);
		m_list.OnItemChanged = Callback(this, &CAccountOptionsDlg::onItemChanged_List);
	}

	bool OnInitDialog() override
	{
		cmbPolicy.AddString(TranslateW(LANG_POLICY_DEFAULT));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_ALWAYS));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_OPP));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_MANUAL));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_NEVER));
		cmbPolicy.SetCurSel(-1);
		cmbPolicy.Disable();

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		// add list columns
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.iSubItem = 0;
		lvc.pszText = TranslateW(LANG_PROTO);
		lvc.cx = 85;     // width of column in pixels
		m_list.InsertColumn(0, &lvc);

		lvc.iSubItem = 1;
		lvc.pszText = TranslateW(LANG_POLICY);
		lvc.cx = 80;     // width of column in pixels
		m_list.InsertColumn(1, &lvc);

		lvc.iSubItem = 2;
		lvc.pszText = TranslateW(LANG_FINGERPRINT);
		lvc.cx = 275;     // width of column in pixels
		m_list.InsertColumn(2, &lvc);

		RefreshList();
		return true;
	}

	bool OnApply() override
	{
		int cnt = m_list.GetItemCount();
		wchar_t policy[64];
		for (int i = 0; i < cnt; ++i) {
			char *proto = GetProtoName(i);
			if (proto == nullptr)
				continue;

			m_list.GetItemText(i, 1, policy, _countof(policy));
			db_set_dw(0, MODULENAME"_ProtoPol", proto, policy_from_string(policy));
		}

		return true;
	}

	UI_MESSAGE_MAP(CAccountOptionsDlg, CDlgBase);
	case WMU_REFRESHPROTOLIST:
		RefreshList();
		break;
	UI_MESSAGE_MAP_END();

	void onSelChanged_Combo(CCtrlCombo *)
	{
		int proto = m_list.GetSelectionMark();
		if (proto == -1) return;
		
		int sel = cmbPolicy.GetCurSel();
		if (sel == CB_ERR) return;

		size_t len = cmbPolicy.SendMsg(CB_GETLBTEXTLEN, sel, 0);
		if (len < 0) return;

		wchar_t *text = new wchar_t[len + 1];
		cmbPolicy.SendMsg(CB_GETLBTEXT, sel, (LPARAM)text);
		m_list.SetItemText(proto, 1, text);
		delete[] text;
		NotifyChange();
	}

	void onContextMenu_List(CCtrlListView *)
	{
		int sel = m_list.GetSelectionMark();
		if (sel == -1)
			return;

		POINT pt;
		GetCursorPos(&pt);
		char *proto = GetProtoName(sel);

		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_OPT_ACCOUNT));
		TranslateMenu(hMenu);

		switch (TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, 0)) {
		case IDM_OPT_PROTO_NEWKEY:
			{
				PROTOREGENKEYOPTIONS *opts = new PROTOREGENKEYOPTIONS();
				opts->refresh = m_hwnd;
				opts->szProto = GetProtoName(sel);
				m_list.GetItemText(sel, 0, opts->wszTitle, _countof(opts->wszTitle));
				CloseHandle((HANDLE)_beginthreadex(nullptr, 0, regen_key_thread, opts, 0, nullptr));
			}
			break;

		case IDM_OPT_COPY:
			if (proto) {
				char fprint[45];
				if (otrl_privkey_fingerprint(otr_user_state, fprint, proto, proto))
					Utils_ClipboardCopy(fprint);
			}
			break;

		case IDM_OPT_PROTO_FORGET:
			wchar_t buff_proto[128];
			m_list.GetItemText(sel, 0, buff_proto, _countof(buff_proto));

			wchar_t buff[512];
			mir_snwprintf(buff, TranslateW(LANG_OTR_ASK_REMOVEKEY), buff_proto);
			if (IDYES == MessageBox(m_hwnd, buff, TranslateT(LANG_OTR_INFO), MB_ICONQUESTION | MB_YESNO)) {
				if (proto == nullptr)
					break;

				OtrlPrivKey *key = otrl_privkey_find(otr_user_state, proto, proto);
				if (key) {
					otrl_privkey_forget(key);
					otrl_privkey_write(otr_user_state, g_private_key_filename);
					m_list.SetItemText(sel, 2, L"");
				}
			}
			break;
		}

		DestroyMenu(hMenu);
	}

	void onItemChanged_List(CCtrlListView::TEventInfo *ev)
	{
		LPNMLISTVIEW notif = ev->nmlv;
		if ((notif->uNewState & LVIS_SELECTED) == 0)
			return;

		if (notif->iItem == -1) {
			cmbPolicy.SetCurSel(-1);
			cmbPolicy.Disable();
			bMenuEnable = false;
		}
		else {
			cmbPolicy.Enable();
			bMenuEnable = true;

			wchar_t buff[50];
			m_list.GetItemText(notif->iItem, 1, buff, _countof(buff));
			cmbPolicy.SelectString(buff);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Contacts tab

class CContactOptionsDlg : public CDlgBase
{
	CCtrlCombo cmbPolicy;
	CCtrlListView m_list;
	std::map<MCONTACT, CONTACT_DATA> m_map;

	void ChangeContactSetting(int iItem, bool changeHtml)
	{
		if (iItem < 0)
			return;

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		lvi.iItem = iItem;
		lvi.iSubItem = 0;
		m_list.GetItem(&lvi);

		MCONTACT hContact = (MCONTACT)lvi.lParam;

		// Handle HtmlConv
		wchar_t buff[50];
		m_list.GetItemText(lvi.iItem, 3, buff, _countof(buff));

		bool htmlEnabled = !wcsncmp(buff, TranslateW(LANG_YES), 50);
		if (changeHtml) {
			htmlEnabled = !htmlEnabled;
		}

		// Update wanted state
		m_map[hContact].htmlconv = htmlEnabled ? HTMLCONV_ENABLE : HTMLCONV_DISABLE;
		m_list.SetItemText(lvi.iItem, 3, TranslateW(htmlEnabled ? LANG_YES : LANG_NO));

		// Handle Policy
		OtrlPolicy policy = CONTACT_DEFAULT_POLICY;

		int sel = cmbPolicy.GetCurSel();
		if (sel != CB_ERR) {
			int len = cmbPolicy.SendMsg(CB_GETLBTEXTLEN, sel, 0);
			if (len >= 0) {
				wchar_t *text = new wchar_t[len + 1];
				cmbPolicy.SendMsg(CB_GETLBTEXT, sel, (LPARAM)text);
				m_list.SetItemText(iItem, 2, text);
				policy = policy_from_string(text);
				delete[] text;
			}
		}

		m_map[hContact].policy = policy;

		NotifyChange();
	}

	void RefreshList()
	{
		m_list.DeleteAllItems();

		LVITEM lvI = { 0 };
		lvI.mask = LVIF_TEXT | LVIF_PARAM;

		for (auto &hContact : Contacts()) {
			const char *proto = Proto_GetBaseAccountName(hContact);
			if (proto && !Contact::IsGroupChat(hContact, proto) // ignore chatrooms
				&& Proto_IsProtoOnContact(hContact, MODULENAME)
				&& mir_strcmp(proto, META_PROTO) != 0) // and MetaContacts
			{
				lvI.iItem = 0;
				lvI.iSubItem = 0;
				lvI.lParam = hContact;
				lvI.pszText = (wchar_t *)contact_get_nameT(hContact);
				lvI.iItem = m_list.InsertItem(&lvI);

				PROTOACCOUNT *pa = Proto_GetAccount(proto);
				m_list.SetItemText(lvI.iItem, 1, pa->tszAccountName);

				m_list.SetItemText(lvI.iItem, 2, (wchar_t *)policy_to_string((OtrlPolicy)g_plugin.getDword(hContact, "Policy", CONTACT_DEFAULT_POLICY)));
				m_list.SetItemText(lvI.iItem, 3, (g_plugin.getByte(hContact, "HTMLConv", 0)) ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
			}
		}
	}

public:
	CContactOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CONTACTS),
		m_list(this, IDC_LV_CONT_CONTACTS),
		cmbPolicy(this, IDC_CMB_CONT_POLICY)
	{
		cmbPolicy.OnSelChanged = Callback(this, &CContactOptionsDlg::onSelChanged_Combo);

		m_list.OnClick = Callback(this, &CContactOptionsDlg::onClick_List);
		m_list.OnItemChanged = Callback(this, &CContactOptionsDlg::onItemChanged_List);
	}

	bool OnInitDialog() override
	{
		cmbPolicy.AddString(TranslateW(LANG_POLICY_DEFAULT));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_ALWAYS));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_OPP));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_MANUAL));
		cmbPolicy.AddString(TranslateW(LANG_POLICY_NEVER));
		cmbPolicy.Disable();

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		// add list columns
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.iSubItem = 0;
		lvc.pszText = TranslateW(LANG_CONTACT);
		lvc.cx = 150;
		m_list.InsertColumn(0, &lvc);

		lvc.iSubItem = 1;
		lvc.pszText = TranslateW(LANG_PROTO);
		lvc.cx = 100;
		m_list.InsertColumn(1, &lvc);

		lvc.iSubItem = 2;
		lvc.pszText = TranslateW(LANG_POLICY);
		lvc.cx = 90;
		m_list.InsertColumn(2, &lvc);

		lvc.iSubItem = 3;
		lvc.pszText = TranslateW(LANG_HTMLCONV);
		lvc.cx = 80;
		m_list.InsertColumn(3, &lvc);

		RefreshList();
		return true;
	}

	bool OnApply() override
	{
		for (auto &it : m_map) {
			if (!it.first) continue;
			db_set_dw(it.first, MODULENAME, "Policy", (uint32_t)it.second.policy);
			db_set_b(it.first, MODULENAME, "HTMLConv", it.second.htmlconv);
		}
		return true;
	}

	void onClick_List(CCtrlListView::TEventInfo *ev)
	{
		if (ev->nmlv->iSubItem == 3)
			ChangeContactSetting(ev->nmlv->iItem, true);
	}

	void onItemChanged_List(CCtrlListView::TEventInfo *ev) {
		LPNMLISTVIEW notif = ev->nmlv;
		if (notif->uNewState & LVIS_SELECTED) {
			if (notif->iItem == -1) {
				cmbPolicy.SetCurSel(-1);
				cmbPolicy.Disable();
			}
			else {
				cmbPolicy.Enable();

				wchar_t buff[50];
				m_list.GetItemText(notif->iItem, 2, buff, _countof(buff));
				cmbPolicy.SelectString(buff);
			}
		}
	}

	void onSelChanged_Combo(CCtrlListView *)
	{
		ChangeContactSetting(m_list.GetSelectionMark(), false);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Fingerprints tab

class CFingerOptionsDlg : public CDlgBase
{
	CCtrlListView m_list;
	std::map<Fingerprint *, FPModify> m_map;

	void RefreshList()
	{
		m_list.DeleteAllItems();

		// enumerate contacts, fill in list
		LVITEM lvI = { };
		lvI.mask = LVIF_TEXT | LVIF_PARAM;

		for (ConnContext *context = otr_user_state->context_root; context; context = context->next) {
			if (!context->app_data)
				continue;

			wchar_t *user = (wchar_t *)contact_get_nameT((UINT_PTR)context->app_data);
			if (!user)
				continue;

			PROTOACCOUNT *pa = Proto_GetAccount(context->protocol);

			wchar_t hash[45];
			for (auto *fp = context->fingerprint_root.next; fp; fp = fp->next) {
				otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
				lvI.iSubItem = 0;
				lvI.lParam = (LPARAM)fp;
				lvI.pszText = user;
				int d = m_list.InsertItem(&lvI);

				m_list.SetItemText(d, 1, pa->tszAccountName);
				m_list.SetItemText(d, 2, (context->active_fingerprint == fp) ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
				m_list.SetItemText(d, 3, (fp->trust && fp->trust[0] != '\0') ? TranslateW(LANG_YES) : TranslateW(LANG_NO));
				m_list.SetItemText(d, 4, hash);
			}
		}
	}

public:
	CFingerOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_FINGER),
		m_list(this, IDC_LV_FINGER_LIST)
	{
		m_list.OnBuildMenu = Callback(this, &CFingerOptionsDlg::OnContextMenu);
	}

	bool OnInitDialog() override
	{
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		// add list columns
		LVCOLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;

		lvc.iSubItem = 0;
		lvc.pszText = TranslateW(LANG_CONTACT);
		lvc.cx = 100;
		m_list.InsertColumn(0, &lvc);

		lvc.iSubItem = 1;
		lvc.pszText = TranslateW(LANG_PROTO);
		lvc.cx = 90;
		m_list.InsertColumn(1, &lvc);

		lvc.iSubItem = 2;
		lvc.pszText = TranslateW(LANG_ACTIVE);
		lvc.cx = 50;
		m_list.InsertColumn(2, &lvc);

		lvc.iSubItem = 3;
		lvc.pszText = TranslateW(LANG_VERIFIED);
		lvc.cx = 50;
		m_list.InsertColumn(3, &lvc);

		lvc.iSubItem = 4;
		lvc.pszText = TranslateW(LANG_FINGERPRINT);
		lvc.cx = 300;
		m_list.InsertColumn(4, &lvc);

		RefreshList();
		return true;
	}

	bool OnApply() override
	{
		for (auto &it : m_map) {
			if (!it.first) continue;
			switch (it.second) {
			case FPM_DELETE:
				if (it.first->context->active_fingerprint == it.first) {
					MCONTACT hContact = (UINT_PTR)it.first->context->app_data;
					wchar_t buff[1024], hash[45];
					otrl_privkey_hash_to_humanT(hash, it.first->fingerprint);
					PROTOACCOUNT *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
					mir_snwprintf(buff, TranslateW(LANG_FINGERPRINT_NOT_DELETED), hash, contact_get_nameT(hContact), pa->tszAccountName);
					ShowError(buff);
				}
				else otrl_context_forget_fingerprint(it.first, 1);
				break;

			case FPM_VERIFY:
				otrl_context_set_trust(it.first, "verified");
				if (it.first == it.first->context->active_fingerprint)
					VerifyFingerprint(it.first->context, true);
				break;

			case FPM_NOTRUST:
				otrl_context_set_trust(it.first, nullptr);
				if (it.first == it.first->context->active_fingerprint)
					VerifyFingerprint(it.first->context, false);
				break;
			}
		}

		if (!m_map.empty())
			otr_gui_write_fingerprints(nullptr);

		m_map.clear();
		RefreshList();
		return true;
	}

	void OnContextMenu(CCtrlListView *)
	{
		int sel = m_list.GetSelectionMark();
		if (sel == -1)
			return;

		POINT pt;
		GetCursorPos(&pt);

		HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_OPT_FINGER));
		TranslateMenu(hMenu);

		LVITEM lvi = {};
		lvi.mask = LVIF_PARAM;
		lvi.iItem = sel;
		m_list.GetItem(&lvi);

		auto *fp = (Fingerprint *)lvi.lParam;

		switch (TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, 0)) {
		case IDM_OPT_FINGER_DONTTRUST:
			m_map[fp] = FPM_NOTRUST;
			m_list.SetItemText(sel, 3, TranslateW(LANG_NO));
			NotifyChange();
			break;

		case IDM_OPT_FINGER_TRUST:
			m_map[fp] = FPM_VERIFY;
			m_list.SetItemText(sel, 3, TranslateW(LANG_YES));
			NotifyChange();
			break;

		case IDM_OPT_COPY:
			wchar_t hash[45];
			otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
			Utils_ClipboardCopy(hash);
			break;

		case IDM_OPT_FINGER_FORGET:
			if (fp->context->active_fingerprint == fp) {
				MCONTACT hContact = (UINT_PTR)fp->context->app_data;
				wchar_t buff[1024];
				otrl_privkey_hash_to_humanT(hash, fp->fingerprint);
				PROTOACCOUNT *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
				mir_snwprintf(buff, TranslateW(LANG_FINGERPRINT_STILL_IN_USE), hash, contact_get_nameT(hContact), pa->tszAccountName);
				ShowError(buff);
			}
			else {
				m_map[fp] = FPM_DELETE;
				m_list.DeleteItem(sel);
				NotifyChange();
			}
			break;
		}

		DestroyMenu(hMenu);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int OpenOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100;
	odp.szGroup.a = LPGEN("Services");
	odp.szTitle.a = "OTR";
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENERAL);
	odp.pfnDlgProc = DlgProcMirOTROpts;
	g_plugin.addOptions(wParam, &odp);
	odp.pszTemplate = 0;
	odp.pfnDlgProc = 0;

	odp.szTab.a = LPGEN("Accounts");
	odp.pDialog = new CAccountOptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Contacts");
	odp.pDialog = new CContactOptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Fingerprints");
	odp.pDialog = new CFingerOptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void InitOptions()
{
	LoadOptions();
	HookEvent(ME_OPT_INITIALISE, OpenOptions);
}
