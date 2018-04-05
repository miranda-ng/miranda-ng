// Copyright (c) 2010-18 sss
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


map<int, MCONTACT> user_data;

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
	COptGpgMainDlg() : CDlgBase(globals.hInst, IDD_OPT_GPG),
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

	virtual void OnInitDialog() override
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

				wchar_t *tmp = mir_a2u(GetContactProto(hContact));
				list_USERLIST.SetItemText(row, 4, tmp);
				mir_free(tmp);

				char *tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
				tmp = mir_a2u(tmp2);
				mir_free(tmp2);
				list_USERLIST.SetItemText(row, 1, (mir_wstrlen(tmp) > 1) ? tmp : L"not set");
				mir_free(tmp);

				tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainName", "");
				if (!toUTF16(tmp2).empty())
					tmp = mir_wstrdup(toUTF16(tmp2).c_str());
				else
					tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainName", L"");
				mir_free(tmp2);
				list_USERLIST.SetItemText(row, 2, (mir_wstrlen(tmp) > 1) ? tmp : L"not set");
				mir_free(tmp);

				tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainEmail", "");
				if (!toUTF16(tmp2).empty())
					tmp = mir_wstrdup(toUTF16(tmp2).c_str());
				else
					tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainEmail", L"");
				mir_free(tmp2);
				list_USERLIST.SetItemText(row, 3, (mir_wstrlen(tmp) > 1) ? tmp : L"not set");
				mir_free(tmp);

				
				if (db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0))
					list_USERLIST.SetCheckState(row, 1);
				user_data[i] = hContact;
				list_USERLIST.SetColumnWidth(0, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
				list_USERLIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
				i++;
			}
		}
		edit_LOG_FILE_EDIT.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szLogFilePath", L"")));

		check_DEBUG_LOG.SetState(db_get_b(NULL, szGPGModuleName, "bDebugLog", 0));
		check_JABBER_API.Enable();
		check_AUTO_EXCHANGE.Enable(globals.bJabberAPI);

		{
			string keyinfo = Translate("Default private key ID");
			keyinfo += ": ";
			char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
			keyinfo += (mir_strlen(keyid) > 0) ? keyid : Translate("not set");
			mir_free(keyid);
			lbl_CURRENT_KEY.SetTextA(keyinfo.c_str());
		}
		check_JABBER_API.SetState(db_get_b(NULL, szGPGModuleName, "bJabberAPI", 1));
		check_FILE_TRANSFERS.SetState(db_get_b(NULL, szGPGModuleName, "bFileTransfers", 0));
		check_AUTO_EXCHANGE.SetState(db_get_b(NULL, szGPGModuleName, "bAutoExchange", 0));

		//TODO: get rid of following s..t
		////////////////
		hwndList_p = list_USERLIST.GetHwnd();
		hwndCurKey_p = lbl_CURRENT_KEY.GetHwnd();
		////////////////

		list_USERLIST.OnItemChanged = Callback(this, &COptGpgMainDlg::onItemChanged_USERLIST);
	}

	virtual void OnApply() override
	{
		db_set_b(NULL, szGPGModuleName, "bDebugLog", globals.bDebugLog = check_DEBUG_LOG.GetState());

		if (globals.bDebugLog)
			globals.debuglog.init();
		db_set_b(NULL, szGPGModuleName, "bJabberAPI", globals.bJabberAPI = check_JABBER_API.GetState());
		bool old_bFileTransfers = db_get_b(NULL, szGPGModuleName, "bFileTransfers", 0) != 0;
		db_set_b(NULL, szGPGModuleName, "bFileTransfers", globals.bFileTransfers = check_FILE_TRANSFERS.GetState());
		if (globals.bFileTransfers != old_bFileTransfers) {
			db_set_b(NULL, szGPGModuleName, "bSameAction", 0);
			globals.bSameAction = false;
		}
		db_set_b(NULL, szGPGModuleName, "bAutoExchange", globals.bAutoExchange = check_AUTO_EXCHANGE.GetState());
		db_set_ws(NULL, szGPGModuleName, "szLogFilePath", ptrW(edit_LOG_FILE_EDIT.GetText()));
	}

	void onClick_DELETE_KEY_BUTTON(CCtrlButton*)
	{
		void setClistIcon(MCONTACT hContact);
		void setSrmmIcon(MCONTACT hContact);
		{ //gpg execute block
			wchar_t *ptmp;
			char *tmp;
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
			tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
			for (auto &hcnttmp : Contacts()) {
				if (hcnttmp != hContact) {
					char *tmp2 = UniGetContactSettingUtf(hcnttmp, szGPGModuleName, "KeyID", "");
					if (!mir_strcmp(tmp, tmp2)) {
						mir_free(tmp2);
						keep = true;
						break;
					}
					mir_free(tmp2);
				}
			}
			if (!keep)
				if (MessageBox(nullptr, TranslateT("This key is not used by any contact. Do you want to remove it from public keyring?"), TranslateT("Key info"), MB_YESNO) == IDYES) {
					std::vector<wstring> cmd;
					string output;
					DWORD exitcode;
					cmd.push_back(L"--batch");
					cmd.push_back(L"--yes");
					cmd.push_back(L"--delete-key");
					ptmp = mir_a2u(tmp);
					cmd.push_back(ptmp);
					mir_free(ptmp);
					gpg_execution_params params(cmd);
					pxResult result;
					params.out = &output;
					params.code = &exitcode;
					params.result = &result;
					if (!gpg_launcher(params)) {
						mir_free(tmp);
						return;
					}
					if (result == pxNotFound) {
						mir_free(tmp);
						return;
					}
					if (output.find("--delete-secret-keys") != string::npos)
						MessageBox(nullptr, TranslateT("we have secret key for this public key, do not removing from GPG keyring"), TranslateT("info"), MB_OK);
					else
						MessageBox(nullptr, TranslateT("Key removed from GPG keyring"), TranslateT("info"), MB_OK);
				}
			mir_free(tmp);
			if (ismetacontact) {
				if (MessageBox(nullptr, TranslateT("Do you want to remove key from entire metacontact (all subcontacts)?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES) {
					MCONTACT hcnt = NULL;
					int count = db_mc_getSubCount(meta);
					for (int i = 0; i < count; i++) {
						hcnt = db_mc_getSub(meta, i);
						if (hcnt) {
							db_unset(hcnt, szGPGModuleName, "KeyID");
							db_unset(hcnt, szGPGModuleName, "GPGPubKey");
							db_unset(hcnt, szGPGModuleName, "KeyMainName");
							db_unset(hcnt, szGPGModuleName, "KeyType");
							db_unset(hcnt, szGPGModuleName, "KeyMainEmail");
							db_unset(hcnt, szGPGModuleName, "KeyComment");
							setClistIcon(hcnt);
							setSrmmIcon(hcnt);
						}
					}
				}
				else {
					db_unset(hContact, szGPGModuleName, "KeyID");
					db_unset(hContact, szGPGModuleName, "GPGPubKey");
					db_unset(hContact, szGPGModuleName, "KeyMainName");
					db_unset(hContact, szGPGModuleName, "KeyType");
					db_unset(hContact, szGPGModuleName, "KeyMainEmail");
					db_unset(hContact, szGPGModuleName, "KeyComment");
					setClistIcon(hContact);
					setSrmmIcon(hContact);
				}
			}
			else {
				db_unset(user_data[item_num + 1], szGPGModuleName, "KeyID");
				db_unset(user_data[item_num + 1], szGPGModuleName, "GPGPubKey");
				db_unset(user_data[item_num + 1], szGPGModuleName, "KeyMainName");
				db_unset(user_data[item_num + 1], szGPGModuleName, "KeyType");
				db_unset(user_data[item_num + 1], szGPGModuleName, "KeyMainEmail");
				db_unset(user_data[item_num + 1], szGPGModuleName, "KeyComment");
				setClistIcon(user_data[item_num + 1]);
				setSrmmIcon(user_data[item_num + 1]);
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
		wchar_t *tmp = GetFilePath(TranslateT("Export public key"), L"*", TranslateT(".asc pubkey file"), true);
		if (tmp) {
			wstring str(ptrW(UniGetContactSettingUtf(user_data[item_num + 1], szGPGModuleName, "GPGPubKey", L"")));
			wstring::size_type s = 0;
			while ((s = str.find(L"\r", s)) != wstring::npos)
				str.erase(s, 1);

			wfstream f(tmp, std::ios::out);
			delete[] tmp;
			f << str.c_str();
			f.close();
		}
	}

	void onClick_COPY_KEY(CCtrlButton*)
	{
		if (OpenClipboard(m_hwnd)) {
			char *szKey = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
			std::string str = szKey;
			mir_free(szKey);
			boost::algorithm::replace_all(str, "\n", "\r\n");
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.size() + 1);
			if (!hMem) {
				MessageBox(nullptr, TranslateT("Failed to allocate memory"), TranslateT("Error"), MB_OK);
				return;
			}
			szKey = (char*)GlobalLock(hMem);
			if (!szKey) {
				wchar_t msg[64];
				mir_snwprintf(msg, TranslateT("Failed to lock memory with error %d"), GetLastError());
				MessageBox(nullptr, msg, TranslateT("Error"), MB_OK);
				GlobalFree(hMem);
			}
			else {
				memcpy(szKey, str.c_str(), str.size());
				szKey[str.size()] = '\0';
				str.clear();
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
				db_set_b(user_data[item_num + 1], szGPGModuleName, "GPGEncryption", 1);
			else
				db_set_b(user_data[item_num + 1], szGPGModuleName, "GPGEncryption", 0);
			setClistIcon(user_data[item_num + 1]);
			setSrmmIcon(user_data[item_num + 1]);
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
	COptGpgBinDlg() : CDlgBase(globals.hInst, IDD_OPT_GPG_BIN),
		edit_BIN_PATH(this, IDC_BIN_PATH), edit_HOME_DIR(this, IDC_HOME_DIR),
		btn_SET_BIN_PATH(this, IDC_SET_BIN_PATH), btn_SET_HOME_DIR(this, IDC_SET_HOME_DIR)
	{
		btn_SET_BIN_PATH.OnClick = Callback(this, &COptGpgBinDlg::onClick_SET_BIN_PATH);
		btn_SET_HOME_DIR.OnClick = Callback(this, &COptGpgBinDlg::onClick_SET_HOME_DIR);

	}

	virtual void OnInitDialog() override
	{
		edit_BIN_PATH.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", L"gpg.exe")));
		edit_HOME_DIR.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"gpg")));
	}

	virtual void OnApply() override
	{
		wchar_t tmp[8192];
		db_set_ws(NULL, szGPGModuleName, "szGpgBinPath", edit_BIN_PATH.GetText());
		mir_wstrncpy(tmp, edit_HOME_DIR.GetText(), 8191);
		while (tmp[mir_wstrlen(tmp) - 1] == '\\')
			tmp[mir_wstrlen(tmp) - 1] = '\0';
		db_set_ws(NULL, szGPGModuleName, "szHomePath", tmp);
	}

	void onClick_SET_BIN_PATH(CCtrlButton*)
	{
		GetFilePath(TranslateT("Choose gpg.exe"), "szGpgBinPath", L"*.exe", TranslateT("EXE Executables"));
		CMStringW tmp(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", L"gpg.exe")));
		edit_BIN_PATH.SetText(tmp);
		bool gpg_exists = false;
		{
			if (_waccess(tmp, 0) != -1)
				gpg_exists = true;
			if (gpg_exists) {
				bool bad_version = false;
				wchar_t *tmp_path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", L"");
				db_set_ws(NULL, szGPGModuleName, "szGpgBinPath", tmp);
				string out;
				DWORD code;
				std::vector<wstring> cmd;
				cmd.push_back(L"--version");
				gpg_execution_params params(cmd);
				pxResult result;
				params.out = &out;
				params.code = &code;
				params.result = &result;
				bool old_gpg_state = globals.gpg_valid;
				globals.gpg_valid = true;
				gpg_launcher(params);
				globals.gpg_valid = old_gpg_state;
				db_set_ws(NULL, szGPGModuleName, "szGpgBinPath", tmp_path);
				mir_free(tmp_path);
				string::size_type p1 = out.find("(GnuPG) ");
				if (p1 != string::npos) {
					p1 += mir_strlen("(GnuPG) ");
					if (out[p1] != '1')
						bad_version = true;
				}
				else {
					bad_version = false;
					MessageBox(nullptr, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
				}
				/*					  if(bad_version) //looks like working fine with gpg2
				MessageBox(0, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), L"Warning", MB_OK); */
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
		GetFolderPath(TranslateT("Set home directory"), "szHomePath");
		CMStringW tmp(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"")));
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
	COptGpgMsgDlg() : CDlgBase(globals.hInst, IDD_OPT_GPG_MESSAGES),
		check_APPEND_TAGS(this, IDC_APPEND_TAGS), check_STRIP_TAGS(this, IDC_STRIP_TAGS),
		edit_IN_OPEN_TAG(this, IDC_IN_OPEN_TAG), edit_IN_CLOSE_TAG(this, IDC_IN_CLOSE_TAG), edit_OUT_OPEN_TAG(this, IDC_OUT_OPEN_TAG), edit_OUT_CLOSE_TAG(this, IDC_OUT_CLOSE_TAG)
	{}

	virtual void OnInitDialog() override
	{
		check_APPEND_TAGS.SetState(db_get_b(NULL, szGPGModuleName, "bAppendTags", 0));
		check_STRIP_TAGS.SetState(db_get_b(NULL, szGPGModuleName, "bStripTags", 0));
		edit_IN_OPEN_TAG.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szInOpenTag", L"<GPGdec>")));
		edit_IN_CLOSE_TAG.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szInCloseTag", L"</GPGdec>")));
		edit_OUT_OPEN_TAG.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutOpenTag", L"<GPGenc>")));
		edit_OUT_CLOSE_TAG.SetText(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutCloseTag", L"</GPGenc>")));
	}

	virtual void OnApply() override
	{
		db_set_b(NULL, szGPGModuleName, "bAppendTags", globals.bAppendTags = check_APPEND_TAGS.GetState());
		db_set_b(NULL, szGPGModuleName, "bStripTags", globals.bStripTags = check_STRIP_TAGS.GetState());
		{
			wchar_t *tmp = mir_wstrdup(edit_IN_OPEN_TAG.GetText());
			db_set_ws(NULL, szGPGModuleName, "szInOpenTag", tmp);
			mir_free(globals.inopentag);
			globals.inopentag = tmp;
			tmp = mir_wstrdup(edit_IN_CLOSE_TAG.GetText());
			db_set_ws(NULL, szGPGModuleName, "szInCloseTag", tmp);
			mir_free(globals.inclosetag);
			globals.inclosetag = tmp;
			tmp = mir_wstrdup(edit_OUT_OPEN_TAG.GetText());
			db_set_ws(NULL, szGPGModuleName, "szOutOpenTag", tmp);
			mir_free(globals.outopentag);
			globals.outopentag = tmp;
			tmp = mir_wstrdup(edit_OUT_CLOSE_TAG.GetText());
			db_set_ws(NULL, szGPGModuleName, "szOutCloseTag", tmp);
			mir_free(globals.outclosetag);
			globals.outclosetag = tmp;
		}
	}
};

class COptGpgAdvDlg : public CDlgBase
{
	CCtrlButton btn_EXPORT, btn_IMPORT;
	CCtrlCheck check_PRESCENSE_SUBSCRIPTION;

public:
	COptGpgAdvDlg() : CDlgBase(globals.hInst, IDD_OPT_GPG_ADVANCED),
		btn_EXPORT(this, IDC_EXPORT), btn_IMPORT(this, IDC_IMPORT),
		check_PRESCENSE_SUBSCRIPTION(this, IDC_PRESCENSE_SUBSCRIPTION)
	{
		btn_EXPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_EXPORT);
		btn_IMPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_IMPORT);
	}

	virtual void OnInitDialog() override
	{
		check_PRESCENSE_SUBSCRIPTION.SetState(db_get_b(NULL, szGPGModuleName, "bPresenceSigning", 0));
		check_PRESCENSE_SUBSCRIPTION.Enable(globals.bJabberAPI);
	}

	virtual void OnApply() override
	{
		db_set_b(NULL, szGPGModuleName, "bPresenceSigning", globals.bPresenceSigning = check_PRESCENSE_SUBSCRIPTION.GetState());
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
	CDlgLoadPubKeyDlg() : CDlgBase(globals.hInst, IDD_LOAD_PUBLIC_KEY),
		chk_ENABLE_ENCRYPTION(this, IDC_ENABLE_ENCRYPTION),
		btn_SELECT_EXISTING(this, IDC_SELECT_EXISTING), btn_OK(this, ID_OK), btn_LOAD_FROM_FILE(this, ID_LOAD_FROM_FILE), btn_IMPORT(this, IDC_IMPORT),
		edit_PUBLIC_KEY_EDIT(this, IDC_PUBLIC_KEY_EDIT)
	{
		btn_SELECT_EXISTING.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_SELECT_EXISTING);
		btn_OK.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_OK);
		btn_LOAD_FROM_FILE.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_LOAD_FROM_FILE);
		btn_IMPORT.OnClick = Callback(this, &CDlgLoadPubKeyDlg::onClick_IMPORT);
	}

	virtual void OnInitDialog() override
	{
		hContact = user_data[1];
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
			wchar_t *tmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "GPGPubKey", L"");
			wstring str = tmp;
			mir_free(tmp); tmp = nullptr;
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
			//			char *tmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "KeyID_Prescense", "");
			if (!globals.hcontact_data[hcnt].key_in_prescense.empty()) {
				char *tmp2 = UniGetContactSettingUtf(hcnt, szGPGModuleName, "KeyID", "");
				if (!tmp2[0]) {
					string out;
					DWORD code;
					std::vector<wstring> cmd;
					cmd.push_back(L"--export");
					cmd.push_back(L"-a");
					cmd.push_back(toUTF16(globals.hcontact_data[hcnt].key_in_prescense));
					gpg_execution_params params(cmd);
					pxResult result;
					params.out = &out;
					params.code = &code;
					params.result = &result;
					gpg_launcher(params); //TODO: handle errors
					if ((out.find("-----BEGIN PGP PUBLIC KEY BLOCK-----") != string::npos) && (out.find("-----END PGP PUBLIC KEY BLOCK-----") != string::npos)) {
						boost::algorithm::replace_all(out, "\n", "\r\n");

						wchar_t *tmp3 = mir_a2u(out.c_str());
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
				mir_free(tmp2);
			}
			if (tmp)
				mir_free(tmp);
			edit_PUBLIC_KEY_EDIT.SetText(!str.empty() ? str.c_str() : L"");
		}
		edit_p_PubKeyEdit = &edit_PUBLIC_KEY_EDIT;
	}

	virtual void OnDestroy() override
	{
		GetWindowRect(m_hwnd, &globals.load_key_rect);
		db_set_dw(NULL, szGPGModuleName, "LoadKeyWindowX", globals.load_key_rect.left);
		db_set_dw(NULL, szGPGModuleName, "LoadKeyWindowY", globals.load_key_rect.top);
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
							db_set_ws(hcnt, szGPGModuleName, "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
					}
				}
				else db_set_ws(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
			}
			else db_set_ws(hContact, szGPGModuleName, "GPGPubKey", key_buf.substr(ws1, ws2 - ws1).c_str());
		}
		tmp = (wchar_t*)mir_alloc(sizeof(wchar_t) * (key_buf.length() + 1));
		mir_wstrcpy(tmp, key_buf.substr(ws1, ws2 - ws1).c_str());
		{ //gpg execute block
			std::vector<wstring> cmd;
			wchar_t tmp2[MAX_PATH] = { 0 };
			wchar_t *ptmp;
			string output;
			DWORD exitcode;
			{
				MCONTACT hcnt = db_mc_tryMeta(hContact);
				ptmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
				wcsncpy(tmp2, ptmp, MAX_PATH - 1);
				mir_free(ptmp);
				mir_wstrncat(tmp2, L"\\", _countof(tmp2) - mir_wstrlen(tmp2));
				mir_wstrncat(tmp2, L"temporary_exported.asc", _countof(tmp2) - mir_wstrlen(tmp2));
				boost::filesystem::remove(tmp2);
				wfstream f(tmp2, std::ios::out);
				ptmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "GPGPubKey", L"");
				wstring str = ptmp;
				mir_free(ptmp);
				wstring::size_type s = 0;
				while ((s = str.find(L"\r", s)) != wstring::npos) {
					str.erase(s, 1);
				}
				f << str.c_str();
				f.close();
				cmd.push_back(L"--batch");
				cmd.push_back(L"--import");
				cmd.push_back(tmp2);
			}
			gpg_execution_params params(cmd);
			pxResult result;
			params.out = &output;
			params.code = &exitcode;
			params.result = &result;
			if (!gpg_launcher(params))
				return;
			if (result == pxNotFound)
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
								db_unset(hcnt, szGPGModuleName, "bAlwatsTrust");
						}
					}
					else db_unset(metaGetMostOnline(hContact), szGPGModuleName, "bAlwatsTrust");
				}
				else db_unset(hContact, szGPGModuleName, "bAlwatsTrust");
			}
			{
				if (output.find("already in secret keyring") != string::npos) {
					MessageBox(nullptr, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
					boost::filesystem::remove(tmp2);
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
										db_set_s(hcnt, szGPGModuleName, "KeyID", tmp3);
								}
							}
							else
								db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyID", tmp3);
						}
						else
							db_set_s(hContact, szGPGModuleName, "KeyID", tmp3);
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
											db_set_s(hcnt, szGPGModuleName, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
									}
								}
								else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
							}
							else db_set_s(hContact, szGPGModuleName, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
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
												db_set_s(hcnt, szGPGModuleName, "KeyComment", output.substr(s2, s - s2).c_str());
										}
									}
									else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyComment", output.substr(s2, s - s2).c_str());
								}
								else db_set_s(hContact, szGPGModuleName, "KeyComment", output.substr(s2, s - s2).c_str());
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
												db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s, s2 - s).c_str());
										}
									}
									else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s, s2 - s).c_str());
								}
								else db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s, s2 - s).c_str());
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
												db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
										}
									}
									else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
								}
								else db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
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
				wchar_t *fp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", L"");
				{
					string out;
					DWORD code;
					std::vector<wstring> cmds;
					cmds.push_back(L"--batch");
					cmds.push_back(L"-a");
					cmds.push_back(L"--export");
					cmds.push_back(fp);
					mir_free(fp);
					gpg_execution_params params2(cmds);
					pxResult result2;
					params2.out = &out;
					params2.code = &code;
					params2.result = &result2;
					if (!gpg_launcher(params2))
						return;
					if (result2 == pxNotFound)
						return;
					string::size_type s = 0;
					while ((s = out.find("\r", s)) != string::npos) {
						out.erase(s, 1);
					}
					db_set_s(hContact, szGPGModuleName, "GPGPubKey", out.c_str());
				}
			}
			tmp = mir_wstrdup(toUTF16(output).c_str());
			MessageBox(nullptr, tmp, L"", MB_OK);
			mir_free(tmp);
			boost::filesystem::remove(tmp2);
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
								if (!isContactSecured(hcnt))
									db_set_b(hcnt, szGPGModuleName, "GPGEncryption", 1);
								else
									db_set_b(hcnt, szGPGModuleName, "GPGEncryption", 0);
								setSrmmIcon(hContact);
								setClistIcon(hContact);
							}
						}
					}
					else if (!isContactSecured(hContact))
						db_set_b(metaGetMostOnline(hContact), szGPGModuleName, "GPGEncryption", 1);
					else
						db_set_b(metaGetMostOnline(hContact), szGPGModuleName, "GPGEncryption", 0);
				}
				else if (!isContactSecured(hContact))
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", 1);
				else
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
			}
		}
		this->Close();
	}

	void onClick_LOAD_FROM_FILE(CCtrlButton*)
	{
		wchar_t *tmp = GetFilePath(TranslateT("Set file containing GPG public key"), L"*", TranslateT("GPG public key file"));
		if (!tmp)
			return;

		wfstream f(tmp, std::ios::in | std::ios::ate | std::ios::binary);
		delete[] tmp;
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
	void onClick_IMPORT(CCtrlButton*)
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
	odp.szTitle.w = _T(szGPGModuleName);

	odp.szTab.w = LPGENW("Main");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pDialog = new COptGpgMainDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("GnuPG Variables");
	odp.pDialog = new COptGpgBinDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("Messages");
	odp.pDialog = new COptGpgMsgDlg();
	Options_AddPage(wParam, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new COptGpgAdvDlg();
	Options_AddPage(wParam, &odp);
	return 0;
}
