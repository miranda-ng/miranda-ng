// Copyright © 2010-20 sss
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

globals_s globals;


int item_num = 0;
HWND hwndList_p = nullptr;
HWND hwndCurKey_p = nullptr;

void ShowLoadPublicKeyDialog(bool = false);

class COptGpgMainDlg : public CDlgBase
{
	CCtrlListView list_USERLIST;
	CCtrlData lbl_CURRENT_KEY;
	CCtrlEdit edit_LOG_FILE_EDIT;
	CCtrlCheck check_DEBUG_LOG, check_JABBER_API, check_AUTO_EXCHANGE, check_FILE_TRANSFERS;
	CCtrlButton btn_DELETE_KEY_BUTTON, btn_SELECT_KEY, btn_SAVE_KEY_BUTTON, btn_COPY_KEY, btn_LOG_FILE_SET;

public:
	COptGpgMainDlg() : CDlgBase(g_plugin, IDD_OPT_GPG),
		list_USERLIST(this, IDC_USERLIST), lbl_CURRENT_KEY(this, IDC_CURRENT_KEY), edit_LOG_FILE_EDIT(this, IDC_LOG_FILE_EDIT),
		check_DEBUG_LOG(this, IDC_DEBUG_LOG), check_JABBER_API(this, IDC_JABBER_API), check_AUTO_EXCHANGE(this, IDC_AUTO_EXCHANGE), check_FILE_TRANSFERS(this, IDC_FILE_TRANSFERS),
		btn_DELETE_KEY_BUTTON(this, IDC_DELETE_KEY_BUTTON), btn_SELECT_KEY(this, IDC_SELECT_KEY), btn_SAVE_KEY_BUTTON(this, IDC_SAVE_KEY_BUTTON), btn_COPY_KEY(this, IDC_COPY_KEY), btn_LOG_FILE_SET(this, IDC_LOG_FILE_SET)
	{
		btn_DELETE_KEY_BUTTON.OnClick = Callback(this, &COptGpgMainDlg::onClick_DELETE_KEY_BUTTON);
		btn_SELECT_KEY.OnClick = Callback(this, &COptGpgMainDlg::onClick_SELECT_KEY);
		btn_SAVE_KEY_BUTTON.OnClick = Callback(this, &COptGpgMainDlg::onClick_SAVE_KEY_BUTTON);
		btn_COPY_KEY.OnClick = Callback(this, &COptGpgMainDlg::onClick_COPY_KEY);
		btn_LOG_FILE_SET.OnClick = Callback(this, &COptGpgMainDlg::onClick_LOG_FILE_SET);

		check_JABBER_API.OnChange = Callback(this, &COptGpgMainDlg::onChange_JABBER_API);
	}

	bool OnInitDialog() override
	{
		list_USERLIST.AddColumn(0, TranslateT("Contact"), 60);
		list_USERLIST.AddColumn(1, TranslateT("Key ID"), 50);
		list_USERLIST.AddColumn(2, TranslateT("Name"), 50);
		list_USERLIST.AddColumn(3, TranslateT("Email"), 50);
		list_USERLIST.AddColumn(4, TranslateT("Protocol"), 60);
		list_USERLIST.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);
		int i = 1;
		for (auto &hContact : Contacts()) {
			if (isContactHaveKey(hContact)) {
				wchar_t *name = Clist_GetContactDisplayName(hContact);

				int row = list_USERLIST.AddItem(L"", 0);
				list_USERLIST.SetItemText(row, 0, name);

				list_USERLIST.SetItemText(row, 4, _A2T(Proto_GetBaseAccountName(hContact)));

				CMStringW tmp = g_plugin.getMStringW(hContact, "KeyID", L"not set");
				list_USERLIST.SetItemText(row, 1, tmp);

				tmp = g_plugin.getMStringW(hContact, "KeyMainName", L"not set");
				list_USERLIST.SetItemText(row, 2, tmp);

				tmp = g_plugin.getMStringW(hContact, "KeyMainEmail", L"not set");
				list_USERLIST.SetItemText(row, 3, tmp);
				
				if (g_plugin.getByte(hContact, "GPGEncryption", 0))
					list_USERLIST.SetCheckState(row, 1);
				globals.user_data[(int)i] = hContact;
				list_USERLIST.SetColumnWidth(0, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
				i++;
			}
		}
		edit_LOG_FILE_EDIT.SetText(ptrW(g_plugin.getWStringA("szLogFilePath", L"")));

		check_DEBUG_LOG.SetState(g_plugin.getByte("bDebugLog", 0));
		check_JABBER_API.Enable();
		check_AUTO_EXCHANGE.Enable(globals.bJabberAPI);

		CMStringW keyinfo = TranslateT("Default private key ID");
		keyinfo += L": ";
			
		ptrW keyid(g_plugin.getWStringA("KeyID"));
		keyinfo += (mir_wstrlen(keyid) > 0) ? keyid : TranslateT("not set");
		lbl_CURRENT_KEY.SetText(keyinfo);

		check_JABBER_API.SetState(g_plugin.getByte("bJabberAPI", 1));
		check_FILE_TRANSFERS.SetState(g_plugin.getByte("bFileTransfers", 0));
		check_AUTO_EXCHANGE.SetState(g_plugin.getByte("bAutoExchange", 0));

		//TODO: get rid of following s..t
		////////////////
		hwndList_p = list_USERLIST.GetHwnd();
		hwndCurKey_p = lbl_CURRENT_KEY.GetHwnd();
		////////////////

		list_USERLIST.OnItemChanged = Callback(this, &COptGpgMainDlg::onItemChanged_USERLIST);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("bDebugLog", globals.bDebugLog = check_DEBUG_LOG.GetState());

		if (globals.bDebugLog)
			globals.debuglog.init();
		g_plugin.setByte("bJabberAPI", globals.bJabberAPI = check_JABBER_API.GetState());
		bool old_bFileTransfers = g_plugin.getByte("bFileTransfers", 0) != 0;
		g_plugin.setByte("bFileTransfers", globals.bFileTransfers = check_FILE_TRANSFERS.GetState());
		if (globals.bFileTransfers != old_bFileTransfers) {
			g_plugin.setByte("bSameAction", 0);
			globals.bSameAction = false;
		}
		g_plugin.setByte("bAutoExchange", globals.bAutoExchange = check_AUTO_EXCHANGE.GetState());
		g_plugin.setWString("szLogFilePath", ptrW(edit_LOG_FILE_EDIT.GetText()));
		return true;
	}

	void onClick_DELETE_KEY_BUTTON(CCtrlButton*)
	{
		void setClistIcon(MCONTACT hContact);
		void setSrmmIcon(MCONTACT hContact);
		{ //gpg execute block
			wchar_t *ptmp;
			bool keep = false;
			bool ismetacontact = false;
			MCONTACT meta = NULL;
			MCONTACT hContact = globals.user_data[item_num + 1];
			if (db_mc_isMeta(hContact)) {
				meta = hContact;
				hContact = metaGetMostOnline(hContact);
				ismetacontact = true;
			}
			else if ((meta = db_mc_getMeta(globals.user_data[item_num + 1])) != NULL) {
				hContact = metaGetMostOnline(meta);
				ismetacontact = true;
			}

			CMStringA tmp(g_plugin.getMStringA(hContact, "KeyID"));
			for (auto &hcnttmp : Contacts()) {
				if (hcnttmp != hContact) {
					ptrA tmp2(g_plugin.getStringA(hcnttmp, "KeyID"));
					if (!mir_strcmp(tmp, tmp2)) {
						keep = true;
						break;
					}
				}
			}

			if (!keep)
				if (MessageBox(nullptr, TranslateT("This key is not used by any contact. Do you want to remove it from public keyring?"), TranslateT("Key info"), MB_YESNO) == IDYES) {
					gpg_execution_params params;
					params.addParam(L"--batch");
					params.addParam(L"--yes");
					params.addParam(L"--delete-key");
					ptmp = mir_a2u(tmp);
					params.addParam(ptmp);
					mir_free(ptmp);
					if (!gpg_launcher(params))
						return;

					if (params.result == pxNotFound)
						return;

					if (params.out.Find("--delete-secret-keys") != -1)
						MessageBox(nullptr, TranslateT("we have secret key for this public key, do not removing from GPG keyring"), TranslateT("info"), MB_OK);
					else
						MessageBox(nullptr, TranslateT("Key removed from GPG keyring"), TranslateT("info"), MB_OK);
				}

			if (ismetacontact) {
				if (MessageBox(nullptr, TranslateT("Do you want to remove key from entire metacontact (all subcontacts)?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES) {
					MCONTACT hcnt = NULL;
					int count = db_mc_getSubCount(meta);
					for (int i = 0; i < count; i++) {
						hcnt = db_mc_getSub(meta, i);
						if (hcnt) {
							g_plugin.delSetting(hcnt, "KeyID");
							g_plugin.delSetting(hcnt, "GPGPubKey");
							g_plugin.delSetting(hcnt, "KeyMainName");
							g_plugin.delSetting(hcnt, "KeyType");
							g_plugin.delSetting(hcnt, "KeyMainEmail");
							g_plugin.delSetting(hcnt, "KeyComment");
							setClistIcon(hcnt);
							setSrmmIcon(hcnt);
						}
					}
				}
				else {
					g_plugin.delSetting(hContact, "KeyID");
					g_plugin.delSetting(hContact, "GPGPubKey");
					g_plugin.delSetting(hContact, "KeyMainName");
					g_plugin.delSetting(hContact, "KeyType");
					g_plugin.delSetting(hContact, "KeyMainEmail");
					g_plugin.delSetting(hContact, "KeyComment");
					setClistIcon(hContact);
					setSrmmIcon(hContact);
				}
			}
			else {
				g_plugin.delSetting(globals.user_data[item_num + 1], "KeyID");
				g_plugin.delSetting(globals.user_data[item_num + 1], "GPGPubKey");
				g_plugin.delSetting(globals.user_data[item_num + 1], "KeyMainName");
				g_plugin.delSetting(globals.user_data[item_num + 1], "KeyType");
				g_plugin.delSetting(globals.user_data[item_num + 1], "KeyMainEmail");
				g_plugin.delSetting(globals.user_data[item_num + 1], "KeyComment");
				setClistIcon(globals.user_data[item_num + 1]);
				setSrmmIcon(globals.user_data[item_num + 1]);
			}
		}
		list_USERLIST.SetItemText(item_num, 3, TranslateT("not set"));
		list_USERLIST.SetItemText(item_num, 2, TranslateT("not set"));
		list_USERLIST.SetItemText(item_num, 1, TranslateT("not set"));
	}

	void onClick_SELECT_KEY(CCtrlButton*)
	{
		CDlgFirstRun *d = new CDlgFirstRun;
		d->Show();
	}

	void onClick_SAVE_KEY_BUTTON(CCtrlButton*)
	{
		ptrW tmp(GetFilePath(TranslateT("Export public key"), L"*", TranslateT(".asc pubkey file"), true));
		if (tmp) {
			CMStringW str(g_plugin.getMStringW(globals.user_data[item_num + 1], "GPGPubKey"));
			str.Replace(L"\r", L"");

			wfstream f(tmp, std::ios::out);
			f << str.c_str();
			f.close();
		}
	}

	void onClick_COPY_KEY(CCtrlButton*)
	{
		if (OpenClipboard(m_hwnd)) {
			CMStringA str(g_plugin.getMStringA("GPGPubKey"));
			str.Replace("\n", "\r\n");
			
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.GetLength() + 1);
			if (!hMem) {
				MessageBox(nullptr, TranslateT("Failed to allocate memory"), TranslateT("Error"), MB_OK);
				return;
			}
			
			char *szKey = (char*)GlobalLock(hMem);
			if (!szKey) {
				wchar_t msg[64];
				mir_snwprintf(msg, TranslateT("Failed to lock memory with error %d"), GetLastError());
				MessageBox(nullptr, msg, TranslateT("Error"), MB_OK);
				GlobalFree(hMem);
			}
			else {
				memcpy(szKey, str.c_str(), str.GetLength());
				szKey[str.GetLength()] = '\0';

				EmptyClipboard();
				GlobalUnlock(hMem);
				if (!SetClipboardData(CF_OEMTEXT, hMem)) {
					GlobalFree(hMem);
					wchar_t msg[64];
					mir_snwprintf(msg, TranslateT("Failed write to clipboard with error %d"), GetLastError());
					MessageBox(nullptr, msg, TranslateT("Error"), MB_OK);
				}
				CloseClipboard();
			}
		}
		else {
			wchar_t msg[64];
			mir_snwprintf(msg, TranslateT("Failed to open clipboard with error %d"), GetLastError());
			MessageBox(nullptr, msg, TranslateT("Error"), MB_OK);
		}
	}

	void onClick_LOG_FILE_SET(CCtrlButton*)
	{
		edit_LOG_FILE_EDIT.SetText(ptrW(GetFilePath(TranslateT("Set log file"), L"*", TranslateT("LOG files"), 1)));
	}

	void onChange_JABBER_API(CCtrlCheck *chk)
	{
		check_AUTO_EXCHANGE.Enable(chk->GetState());
	}

	void onItemChanged_USERLIST(CCtrlListView::TEventInfo *ev)
	{
		//TODO: get rid of "item_num"
		if (ev->nmlv)
		{
			NMLISTVIEW *hdr = ev->nmlv;
			
			if (hdr->iItem == -1)
				return;
			void setClistIcon(MCONTACT hContact);
			void setSrmmIcon(MCONTACT hContact);
			item_num = hdr->iItem;
			if (list_USERLIST.GetCheckState(hdr->iItem))
				g_plugin.setByte(globals.user_data[item_num + 1], "GPGEncryption", 1);
			else
				g_plugin.setByte(globals.user_data[item_num + 1], "GPGEncryption", 0);
			setClistIcon(globals.user_data[item_num + 1]);
			setSrmmIcon(globals.user_data[item_num + 1]);
		}
	}

	void onClick_USERLIST(CCtrlListView::TEventInfo *ev)
	{
		//TODO: get rid of "item_num"
		if (ev->nmlv)
		{
			NMLISTVIEW *hdr = ev->nmlv;

			if (hdr->iItem == -1)
				return;

			item_num = hdr->iItem;
		}
	}
};


class COptGpgBinDlg : public CDlgBase
{
	CCtrlEdit edit_BIN_PATH, edit_HOME_DIR;
	CCtrlButton btn_SET_BIN_PATH, btn_SET_HOME_DIR;

public:
	COptGpgBinDlg() : CDlgBase(g_plugin, IDD_OPT_GPG_BIN),
		edit_BIN_PATH(this, IDC_BIN_PATH), edit_HOME_DIR(this, IDC_HOME_DIR),
		btn_SET_BIN_PATH(this, IDC_SET_BIN_PATH), btn_SET_HOME_DIR(this, IDC_SET_HOME_DIR)
	{
		btn_SET_BIN_PATH.OnClick = Callback(this, &COptGpgBinDlg::onClick_SET_BIN_PATH);
		btn_SET_HOME_DIR.OnClick = Callback(this, &COptGpgBinDlg::onClick_SET_HOME_DIR);

	}

	bool OnInitDialog() override
	{
		edit_BIN_PATH.SetText(g_plugin.getMStringW("szGpgBinPath", L"gpg.exe"));
		edit_HOME_DIR.SetText(g_plugin.getMStringW("szHomePath", L"gpg"));
		return true;
	}

	bool OnApply() override
	{
		wchar_t tmp[8192];
		g_plugin.setWString("szGpgBinPath", edit_BIN_PATH.GetText());
		mir_wstrncpy(tmp, edit_HOME_DIR.GetText(), 8191);
		while (tmp[mir_wstrlen(tmp) - 1] == '\\')
			tmp[mir_wstrlen(tmp) - 1] = '\0';
		g_plugin.setWString("szHomePath", tmp);
		return true;
	}

	void onClick_SET_BIN_PATH(CCtrlButton*)
	{
		GetFilePath(TranslateT("Choose gpg.exe"), "szGpgBinPath", L"*.exe", TranslateT("EXE Executables"));
		CMStringW tmp(g_plugin.getMStringW("szGpgBinPath", L"gpg.exe"));
		edit_BIN_PATH.SetText(tmp);
		bool gpg_exists = false;
		{
			if (_waccess(tmp, 0) != -1)
				gpg_exists = true;
			if (gpg_exists) {
				bool bad_version = false;
				CMStringW tmp_path = g_plugin.getMStringW("szGpgBinPath", L"");
				g_plugin.setWString("szGpgBinPath", tmp);

				gpg_execution_params params;
				params.addParam(L"--version");

				bool old_gpg_state = globals.gpg_valid;
				globals.gpg_valid = true;
				gpg_launcher(params);
				globals.gpg_valid = old_gpg_state;
				g_plugin.setWString("szGpgBinPath", tmp_path);

				int p1 = params.out.Find("(GnuPG) ");
				if (p1 != string::npos) {
					p1 += mir_strlen("(GnuPG) ");
					if (params.out[p1] != '1')
						bad_version = true;
				}
				else {
					bad_version = false;
					MessageBox(nullptr, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
				}
			}
		}
		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		if (tmp.Find(mir_path, 0) == 0) {
			CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
			edit_BIN_PATH.SetText(path);
		}
	}

	void onClick_SET_HOME_DIR(CCtrlButton*)
	{
		GetFolderPath(TranslateT("Set home directory"));
		CMStringW tmp(g_plugin.getMStringW("szHomePath", L""));
		edit_HOME_DIR.SetText(tmp);
		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		if (tmp.Find(mir_path, 0) == 0) {
			CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
			edit_HOME_DIR.SetText(tmp);
		}
	}
};

class COptGpgMsgDlg : public CDlgBase
{
	CCtrlCheck check_APPEND_TAGS, check_STRIP_TAGS;
	CCtrlEdit edit_IN_OPEN_TAG, edit_IN_CLOSE_TAG, edit_OUT_OPEN_TAG, edit_OUT_CLOSE_TAG;

public:
	COptGpgMsgDlg() : CDlgBase(g_plugin, IDD_OPT_GPG_MESSAGES),
		check_APPEND_TAGS(this, IDC_APPEND_TAGS), check_STRIP_TAGS(this, IDC_STRIP_TAGS),
		edit_IN_OPEN_TAG(this, IDC_IN_OPEN_TAG), edit_IN_CLOSE_TAG(this, IDC_IN_CLOSE_TAG), edit_OUT_OPEN_TAG(this, IDC_OUT_OPEN_TAG), edit_OUT_CLOSE_TAG(this, IDC_OUT_CLOSE_TAG)
	{}

	bool OnInitDialog() override
	{
		check_APPEND_TAGS.SetState(g_plugin.getByte("bAppendTags", 0));
		check_STRIP_TAGS.SetState(g_plugin.getByte("bStripTags", 0));
		edit_IN_OPEN_TAG.SetText(g_plugin.getMStringW("szInOpenTag", L"<GPGdec>"));
		edit_IN_CLOSE_TAG.SetText(g_plugin.getMStringW("szInCloseTag", L"</GPGdec>"));
		edit_OUT_OPEN_TAG.SetText(g_plugin.getMStringW("szOutOpenTag", L"<GPGenc>"));
		edit_OUT_CLOSE_TAG.SetText(g_plugin.getMStringW("szOutCloseTag", L"</GPGenc>"));
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("bAppendTags", globals.bAppendTags = check_APPEND_TAGS.GetState());
		g_plugin.setByte("bStripTags", globals.bStripTags = check_STRIP_TAGS.GetState());
		{
			ptrW tmp(edit_IN_OPEN_TAG.GetText());
			g_plugin.setWString("szInOpenTag", tmp);
			globals.wszInopentag = tmp;

			tmp = edit_IN_CLOSE_TAG.GetText();
			g_plugin.setWString("szInCloseTag", tmp);
			globals.wszInclosetag = tmp;

			tmp = mir_wstrdup(edit_OUT_OPEN_TAG.GetText());
			g_plugin.setWString("szOutOpenTag", tmp);
			globals.wszOutopentag = tmp;

			tmp = mir_wstrdup(edit_OUT_CLOSE_TAG.GetText());
			g_plugin.setWString("szOutCloseTag", tmp);
			globals.wszOutclosetag = tmp;
		}
		return true;
	}
};

class COptGpgAdvDlg : public CDlgBase
{
	CCtrlButton btn_EXPORT, btn_IMPORT;
	CCtrlCheck check_PRESCENSE_SUBSCRIPTION;

public:
	COptGpgAdvDlg() : CDlgBase(g_plugin, IDD_OPT_GPG_ADVANCED),
		btn_EXPORT(this, IDC_EXPORT), btn_IMPORT(this, IDC_IMPORT),
		check_PRESCENSE_SUBSCRIPTION(this, IDC_PRESCENSE_SUBSCRIPTION)
	{
		btn_EXPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_EXPORT);
		btn_IMPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_IMPORT);
	}

	bool OnInitDialog() override
	{
		check_PRESCENSE_SUBSCRIPTION.SetState(g_plugin.getByte("bPresenceSigning", 0));
		check_PRESCENSE_SUBSCRIPTION.Enable(globals.bJabberAPI);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("bPresenceSigning", globals.bPresenceSigning = check_PRESCENSE_SUBSCRIPTION.GetState());
		return true;
	}

	void onClick_EXPORT(CCtrlButton*)
	{
		INT_PTR ExportGpGKeys(WPARAM w, LPARAM l);
		ExportGpGKeys(NULL, NULL);
	}

	void onClick_IMPORT(CCtrlButton*)
	{
		INT_PTR ImportGpGKeys(WPARAM w, LPARAM l);
		ImportGpGKeys(NULL, NULL);
	}
};


CCtrlEdit *edit_p_PubKeyEdit = nullptr;

static LRESULT CALLBACK editctrl_ctrl_a(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_KEYDOWN:
		if (wParam == 0x41 && GetKeyState(VK_CONTROL) < 0)
			SendMessage(hwndDlg, EM_SETSEL, 0, -1);
		return 0;
	}
	return mir_callNextSubclass(hwndDlg, editctrl_ctrl_a, msg, wParam, lParam);
}

class CDlgLoadPubKeyDlg : public CDlgBase
{
	MCONTACT hContact;
	wstring key_buf;
	wstring::size_type ws1 = 0, ws2 = 0;
	CCtrlCheck chk_ENABLE_ENCRYPTION;
	CCtrlButton btn_SELECT_EXISTING, btn_OK, btn_LOAD_FROM_FILE, btn_IMPORT;
	CCtrlEdit edit_PUBLIC_KEY_EDIT;

public:
	CDlgLoadPubKeyDlg() : CDlgBase(g_plugin, IDD_LOAD_PUBLIC_KEY),
		chk_ENABLE_ENCRYPTION(this, IDC_ENABLE_ENCRYPTION),
		btn_SELECT_EXISTING(this, IDC_SELECT_EXISTING), btn_OK(this, ID_OK), btn_LOAD_FROM_FILE(this, ID_LOAD_FROM_FILE), btn_IMPORT(this, IDC_IMPORT),
		edit_PUBLIC_KEY_EDIT(this, IDC_PUBLIC_KEY_EDIT)
	{
		btn_SELECT_EXISTING.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_SELECT_EXISTING);
		btn_OK.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_OK);
		btn_LOAD_FROM_FILE.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_LOAD_FROM_FILE);
		btn_IMPORT.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_IMPORT);
	}

	bool OnInitDialog() override
	{
		hContact = globals.user_data[1];
		SetWindowPos(m_hwnd, nullptr, globals.load_key_rect.left, globals.load_key_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		mir_subclassWindow(GetDlgItem(m_hwnd, IDC_PUBLIC_KEY_EDIT), editctrl_ctrl_a);
		MCONTACT hcnt = db_mc_tryMeta(hContact);
		{
			wstring msg = TranslateT("Load Public GPG Key for ");
			msg += Clist_GetContactDisplayName(hcnt, 0);
			this->SetCaption(msg.c_str());
		}
		if (!hcnt) {
			btn_SELECT_EXISTING.Disable();
			chk_ENABLE_ENCRYPTION.Disable();
		}
		if (isContactSecured(hcnt))
			chk_ENABLE_ENCRYPTION.SetText(TranslateT("Turn off encryption"));
		else {
			chk_ENABLE_ENCRYPTION.SetText(TranslateT("Turn on encryption"));
			chk_ENABLE_ENCRYPTION.SetState(1);
		}
		if (hcnt) {
			wstring str = ptrW(g_plugin.getWStringA(hcnt, "GPGPubKey", L""));
			if (!str.empty()) {
				wstring::size_type p = 0, stop = 0;
				for (;;) {
					if ((p = str.find(L"\n", p + 2)) != wstring::npos) {
						if (p > stop) {
							stop = p;
							str.insert(p, L"\r");
						}
						else break;
					}
				}
			}

			if (!globals.hcontact_data[hcnt].key_in_prescense.empty()) {
				if (g_plugin.getMStringA(hcnt, "KeyID").IsEmpty()) {
					gpg_execution_params params;
					params.addParam(L"--export");
					params.addParam(L"-a");
					params.addParam(toUTF16(globals.hcontact_data[hcnt].key_in_prescense));
					gpg_launcher(params); //TODO: handle errors

					if ((params.out.Find("-----BEGIN PGP PUBLIC KEY BLOCK-----") != -1) && (params.out.Find("-----END PGP PUBLIC KEY BLOCK-----") != -1)) {
						params.out.Replace("\n", "\r\n");

						wchar_t *tmp3 = mir_a2u(params.out.c_str());
						str.clear();
						str.append(tmp3);
						mir_free(tmp3);
						string msg = Translate("Load Public GPG Key for ");
						msg += _T2A(Clist_GetContactDisplayName(hcnt));
						msg += " (Key ID: ";
						msg += globals.hcontact_data[hcnt].key_in_prescense;
						msg += Translate(" found in presence, and exists in keyring.)");
						SetCaption(toUTF16(msg).c_str());
					}
					else {
						string msg = Translate("Load Public GPG Key (Key ID: ");
						msg += globals.hcontact_data[hcnt].key_in_prescense;
						msg += Translate(" found in presence.)");
						SetCaption(toUTF16(msg).c_str());
						btn_IMPORT.Enable();
					}
				}
			}

			edit_PUBLIC_KEY_EDIT.SetText(!str.empty() ? str.c_str() : L"");
		}
		edit_p_PubKeyEdit = &edit_PUBLIC_KEY_EDIT;
		return true;
	}

	virtual void OnDestroy() override
	{
		GetWindowRect(m_hwnd, &globals.load_key_rect);
		g_plugin.setDword("LoadKeyWindowX", globals.load_key_rect.left);
		g_plugin.setDword("LoadKeyWindowY", globals.load_key_rect.top);
		edit_p_PubKeyEdit = nullptr;
	}

	void onClick_SELECT_EXISTING(CCtrlButton*)
	{
		CDlgLoadExistingKey *d = new CDlgLoadExistingKey;
		d->Show();
	}

	void onClick_OK(CCtrlButton*)
	{
		wchar_t *tmp = mir_wstrdup(edit_PUBLIC_KEY_EDIT.GetText());
		wchar_t *begin, *end;
		key_buf.append(tmp);
		key_buf.append(L"\n"); //no new line at end of file )
		mir_free(tmp);
		while ((ws1 = key_buf.find(L"\r", ws1)) != wstring::npos) {
			key_buf.erase(ws1, 1); //remove windows specific trash
		}
		ws1 = 0;
		if (((ws2 = key_buf.find(L"-----END PGP PUBLIC KEY BLOCK-----")) != wstring::npos) && ((ws1 = key_buf.find(L"-----BEGIN PGP PUBLIC KEY BLOCK-----")) != wstring::npos)) {
			begin = (wchar_t*)mir_alloc(sizeof(wchar_t) * (mir_wstrlen(L"-----BEGIN PGP PUBLIC KEY BLOCK-----") + 1));
			mir_wstrcpy(begin, L"-----BEGIN PGP PUBLIC KEY BLOCK-----");
			end = (wchar_t*)mir_alloc(sizeof(wchar_t) * (mir_wstrlen(L"-----END PGP PUBLIC KEY BLOCK-----") + 1));
			mir_wstrcpy(end, L"-----END PGP PUBLIC KEY BLOCK-----");
		}
		else if (((ws2 = key_buf.find(L"-----END PGP PRIVATE KEY BLOCK-----")) != wstring::npos) && ((ws1 = key_buf.find(L"-----BEGIN PGP PRIVATE KEY BLOCK-----")) != wstring::npos)) {
			begin = (wchar_t*)mir_alloc(sizeof(wchar_t) * (mir_wstrlen(L"-----BEGIN PGP PRIVATE KEY BLOCK-----") + 1));
			mir_wstrcpy(begin, L"-----BEGIN PGP PRIVATE KEY BLOCK-----");
			end = (wchar_t*)mir_alloc(sizeof(wchar_t) * (mir_wstrlen(L"-----END PGP PRIVATE KEY BLOCK-----") + 1));
			mir_wstrcpy(end, L"-----END PGP PRIVATE KEY BLOCK-----");
		}
		else {
			MessageBox(nullptr, TranslateT("This is not public or private key"), L"INFO", MB_OK);
			return;
		}
		ws2 += mir_wstrlen(end);
		bool allsubcontacts = false;
		{
			if (db_mc_isMeta(hContact)) {
				if (MessageBox(nullptr, TranslateT("Do you want to load key for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES) {
					allsubcontacts = true;
					int count = db_mc_getSubCount(hContact);
					for (int i = 0; i < count; i++) {
						MCONTACT hcnt = db_mc_getSub(hContact, i);
						if (hcnt)
							g_plugin.setWString(hcnt, "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
					}
				}
				else g_plugin.setWString(metaGetMostOnline(hContact), "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
			}
			else g_plugin.setWString(hContact, "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
		}
		tmp = (wchar_t*)mir_alloc(sizeof(wchar_t) * (key_buf.length() + 1));
		mir_wstrcpy(tmp, key_buf.substr(ws1, ws2 - ws1).c_str());
		{ //gpg execute block
			std::vector<wstring> cmd;
			CMStringW tmp2;
			{
				MCONTACT hcnt = db_mc_tryMeta(hContact);
				tmp2 = g_plugin.getMStringW("szHomePath");
				tmp2 += L"\\temporary_exported.asc";
				boost::filesystem::remove(tmp2.c_str());

				wfstream f(tmp2, std::ios::out);
				CMStringW str = g_plugin.getMStringW(hcnt, "GPGPubKey");
				str.Replace(L"\r", L"");
				f << str.c_str();
				f.close();
			}

			gpg_execution_params params;
			params.addParam(L"--batch");
			params.addParam(L"--import");
			params.addParam(tmp2.c_str());
			if (!gpg_launcher(params))
				return;
			if (params.result == pxNotFound)
				return;

			mir_free(begin);
			mir_free(end);
			if (hContact) {
				if (db_mc_isMeta(hContact)) {
					if (allsubcontacts) {
						int count = db_mc_getSubCount(hContact);
						for (int i = 0; i < count; i++) {
							MCONTACT hcnt = db_mc_getSub(hContact, i);
							if (hcnt)
								g_plugin.delSetting(hcnt, "bAlwatsTrust");
						}
					}
					else g_plugin.delSetting(metaGetMostOnline(hContact), "bAlwatsTrust");
				}
				else g_plugin.delSetting(hContact, "bAlwatsTrust");
			}

			string output(params.out);
			{
				if (output.find("already in secret keyring") != string::npos) {
					MessageBox(nullptr, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
					boost::filesystem::remove(tmp2.c_str());
					return;
				}
				string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
				string::size_type s2 = output.find(":", s);
				{
					char *tmp3 = (char*)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
					mir_strcpy(tmp3, output.substr(s, s2 - s).c_str());
					mir_utf8decode(tmp3, nullptr);
					{
						if (db_mc_isMeta(hContact)) {
							if (allsubcontacts) {
								int count = db_mc_getSubCount(hContact);
								for (int i = 0; i < count; i++) {
									MCONTACT hcnt = db_mc_getSub(hContact, i);
									if (hcnt)
										g_plugin.setString(hcnt, "KeyID", tmp3);
								}
							}
							else g_plugin.setString(metaGetMostOnline(hContact), "KeyID", tmp3);
						}
						else g_plugin.setString(hContact, "KeyID", tmp3);
					}
					mir_free(tmp3);
				}
				tmp = mir_wstrdup(toUTF16(output.substr(s, s2 - s)).c_str());
				if (hContact && hwndList_p)
					ListView_SetItemText(hwndList_p, item_num, 1, tmp);
				mir_free(tmp);
				s = output.find("“", s2);
				if (s == string::npos) {
					s = output.find("\"", s2);
					s += 1;
				}
				else
					s += 3;
				bool uncommon = false;
				if ((s2 = output.find("(", s)) == string::npos) {
					if ((s2 = output.find("<", s)) == string::npos) {
						s2 = output.find("”", s);
						uncommon = true;
					}
				}
				else if (s2 > output.find("<", s))
					s2 = output.find("<", s);
				if (s2 != string::npos && s != string::npos) {
					{
						char *tmp3 = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
						mir_strcpy(tmp3, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
						mir_utf8decode(tmp3, nullptr);
						if (hContact) {
							if (db_mc_isMeta(hContact)) {
								if (allsubcontacts) {
									int count = db_mc_getSubCount(hContact);
									for (int i = 0; i < count; i++) {
										MCONTACT hcnt = db_mc_getSub(hContact, i);
										if (hcnt)
											g_plugin.setString(hcnt, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
									}
								}
								else g_plugin.setString(metaGetMostOnline(hContact), "KeyMainName", output.substr(s, s2 - s - 1).c_str());
							}
							else g_plugin.setString(hContact, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
						}
						mir_free(tmp3);
					}
					tmp = mir_wstrdup(toUTF16(output.substr(s, s2 - s - 1)).c_str());
					if (hContact && hwndList_p)
						ListView_SetItemText(hwndList_p, item_num, 2, tmp); //TODO: do something with this
					mir_free(tmp);
					if ((s = output.find(")", s2)) == string::npos)
						s = output.find(">", s2);
					else if (s > output.find(">", s2))
						s = output.find(">", s2);
					s2++;
					if (s != string::npos && s2 != string::npos) {
						if (output[s] == ')') {
							char *tmp3 = (char*)mir_alloc((output.substr(s2, s - s2).length() + 1) * sizeof(char));
							mir_strcpy(tmp3, output.substr(s2, s - s2).c_str());
							mir_utf8decode(tmp3, nullptr);
							if (hContact) {
								if (db_mc_isMeta(hContact)) {
									if (allsubcontacts) {
										int count = db_mc_getSubCount(hContact);
										for (int i = 0; i < count; i++) {
											MCONTACT hcnt = db_mc_getSub(hContact, i);
											if (hcnt)
												g_plugin.setString(hcnt, "KeyComment", output.substr(s2, s - s2).c_str());
										}
									}
									else g_plugin.setString(metaGetMostOnline(hContact), "KeyComment", output.substr(s2, s - s2).c_str());
								}
								else g_plugin.setString(hContact, "KeyComment", output.substr(s2, s - s2).c_str());
							}
							mir_free(tmp3);
							s += 3;
							s2 = output.find(">", s);
							tmp3 = (char*)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
							mir_strcpy(tmp3, output.substr(s, s2 - s).c_str());
							mir_utf8decode(tmp3, nullptr);
							if (hContact) {
								if (db_mc_isMeta(hContact)) {
									if (allsubcontacts) {
										int count = db_mc_getSubCount(hContact);
										for (int i = 0; i < count; i++) {
											MCONTACT hcnt = db_mc_getSub(hContact, i);
											if (hcnt)
												g_plugin.setString(hcnt, "KeyMainEmail", output.substr(s, s2 - s).c_str());
										}
									}
									else g_plugin.setString(metaGetMostOnline(hContact), "KeyMainEmail", output.substr(s, s2 - s).c_str());
								}
								else g_plugin.setString(hContact, "KeyMainEmail", output.substr(s, s2 - s).c_str());
							}
							mir_free(tmp3);
							tmp = mir_wstrdup(toUTF16(output.substr(s, s2 - s)).c_str());
							if (hContact && hwndList_p)
								ListView_SetItemText(hwndList_p, item_num, 3, tmp);
							mir_free(tmp);
						}
						else {
							char *tmp3 = (char*)mir_alloc(output.substr(s2, s - s2).length() + 1);
							mir_strcpy(tmp3, output.substr(s2, s - s2).c_str());
							mir_utf8decode(tmp3, nullptr);
							if (hContact) {
								if (db_mc_isMeta(hContact)) {
									if (allsubcontacts) {
										int count = db_mc_getSubCount(hContact);
										for (int i = 0; i < count; i++) {
											MCONTACT hcnt = db_mc_getSub(hContact, i);
											if (hcnt)
												g_plugin.setString(hcnt, "KeyMainEmail", output.substr(s2, s - s2).c_str());
										}
									}
									else g_plugin.setString(metaGetMostOnline(hContact), "KeyMainEmail", output.substr(s2, s - s2).c_str());
								}
								else g_plugin.setString(hContact, "KeyMainEmail", output.substr(s2, s - s2).c_str());
							}
							mir_free(tmp3);
							tmp = mir_wstrdup(toUTF16(output.substr(s2, s - s2)).c_str());
							if (hContact && hwndList_p)
								ListView_SetItemText(hwndList_p, item_num, 3, tmp); //TODO: do something with this
							mir_free(tmp);
						}
					}
				}
				if (hContact && hwndList_p)  //TODO: do something with this
				{
					ListView_SetColumnWidth(hwndList_p, 0, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_p, 1, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_p, 2, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_p, 3, LVSCW_AUTOSIZE);
				}
			}
			if (!hContact) {
				gpg_execution_params params2;
				params.addParam(L"--batch");
				params.addParam(L"-a");
				params.addParam(L"--export");
				params.addParam(g_plugin.getMStringW(hContact, "KeyID").c_str());
				if (!gpg_launcher(params2))
					return;
				if (params2.result == pxNotFound)
					return;

				params2.out.Remove('\r');
				g_plugin.setString(hContact, "GPGPubKey", params2.out.c_str());
			}
			MessageBoxA(nullptr, output.c_str(), "", MB_OK);
			boost::filesystem::remove(tmp2.c_str());
		}
		key_buf.clear();
		if (chk_ENABLE_ENCRYPTION.GetState()) {
			if (hContact) {
				if (db_mc_isMeta(hContact)) {
					if (allsubcontacts) {
						int count = db_mc_getSubCount(hContact);
						for (int i = 0; i < count; i++) {
							MCONTACT hcnt = db_mc_getSub(hContact, i);
							if (hcnt) {
								g_plugin.setByte(hcnt, "GPGEncryption", !isContactSecured(hcnt));
								setSrmmIcon(hContact);
								setClistIcon(hContact);
							}
						}
					}
					else g_plugin.setByte(metaGetMostOnline(hContact), "GPGEncryption", !isContactSecured(hContact));
				}
				else g_plugin.setByte(hContact, "GPGEncryption", !isContactSecured(hContact));
			}
		}
		this->Close();
	}

	void onClick_LOAD_FROM_FILE(CCtrlButton *)
	{
		ptrW tmp(GetFilePath(TranslateT("Set file containing GPG public key"), L"*", TranslateT("GPG public key file")));
		if (!tmp)
			return;

		wfstream f(tmp, std::ios::in | std::ios::ate | std::ios::binary);
		if (!f.is_open()) {
			MessageBox(nullptr, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
			return;
		}
		if (f.is_open()) {
			std::wifstream::pos_type size = f.tellg();
			wchar_t *temp = new wchar_t[(std::ifstream::pos_type)size + (std::ifstream::pos_type)1];
			f.seekg(0, std::ios::beg);
			f.read(temp, size);
			temp[size] = '\0';
			key_buf.append(temp);
			delete[] temp;
			f.close();
		}
		if (key_buf.empty()) {
			key_buf.clear();
			if (globals.bDebugLog)
				globals.debuglog << std::string(time_str() + ": info: Failed to read key file");
			return;
		}
		ws2 = key_buf.find(L"-----END PGP PUBLIC KEY BLOCK-----");
		ws1 = key_buf.find(L"-----BEGIN PGP PUBLIC KEY BLOCK-----");
		if (ws2 == wstring::npos || ws1 == wstring::npos) {
			ws2 = key_buf.find(L"-----END PGP PRIVATE KEY BLOCK-----");
			ws1 = key_buf.find(L"-----BEGIN PGP PRIVATE KEY BLOCK-----");
		}
		if (ws2 == wstring::npos || ws1 == wstring::npos) {
			MessageBox(nullptr, TranslateT("There is no public or private key."), TranslateT("Info"), MB_OK);
			return;
		}
		ws2 += mir_wstrlen(L"-----END PGP PUBLIC KEY BLOCK-----");
		edit_PUBLIC_KEY_EDIT.SetText(key_buf.substr(ws1, ws2 - ws1).c_str());
		key_buf.clear();
	}

	void onClick_IMPORT(CCtrlButton *)
	{
		CDlgImportKey *d = new CDlgImportKey(hContact);
		d->Show();
	}
};


void ShowLoadPublicKeyDialog(bool modal)
{
	CDlgLoadPubKeyDlg *d = new CDlgLoadPubKeyDlg();
	if (modal)
		d->DoModal();
	else
		d->Show();
}

int GpgOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = _T(MODULENAME);

	odp.szTab.w = LPGENW("Main");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pDialog = new COptGpgMainDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("GnuPG Variables");
	odp.pDialog = new COptGpgBinDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Messages");
	odp.pDialog = new COptGpgMsgDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new COptGpgAdvDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
