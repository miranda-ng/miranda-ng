// Copyright © 2010-22 sss
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

HWND hwndCurKey_p = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// Load existing key dialog

class CDlgLoadExistingKey : public CDlgBase
{
	wchar_t id[16];
	CCtrlListView list_EXISTING_KEY_LIST;

public:
	CDlgLoadExistingKey() :
		CDlgBase(g_plugin, IDD_LOAD_EXISTING_KEY),
		list_EXISTING_KEY_LIST(this, IDC_EXISTING_KEY_LIST)
	{
		id[0] = 0;

		list_EXISTING_KEY_LIST.OnClick = Callback(this, &CDlgLoadExistingKey::onChange_EXISTING_KEY_LIST);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "LoadKeyWindow");

		list_EXISTING_KEY_LIST.AddColumn(0, TranslateT("Key ID"), 50);
		list_EXISTING_KEY_LIST.AddColumn(1, TranslateT("Email"), 30);
		list_EXISTING_KEY_LIST.AddColumn(2, TranslateT("Name"), 250);
		list_EXISTING_KEY_LIST.AddColumn(3, TranslateT("Creation date"), 30);
		list_EXISTING_KEY_LIST.AddColumn(4, TranslateT("Expiration date"), 30);
		list_EXISTING_KEY_LIST.AddColumn(5, TranslateT("Key length"), 30);
		list_EXISTING_KEY_LIST.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);

		// parse gpg output
		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"--list-keys");
		if (!gpg_launcher(params))
			return false;
		if (params.result == pxNotFound)
			return false;

		int i = 1;
		string out(params.out);
		string::size_type p = 0, p2 = 0, stop = 0;
		while (p != string::npos) {
			if ((p = out.find("pub  ", p)) == string::npos)
				break;
			p += 5;
			if (p < stop)
				break;
			stop = p;
			p2 = out.find("/", p) - 1;

			int row = list_EXISTING_KEY_LIST.AddItem(L"", 0);
			list_EXISTING_KEY_LIST.SetItemText(row, 5, toUTF16(out.substr(p, p2 - p)).c_str());

			p2 += 2;
			p = out.find(" ", p2);
			list_EXISTING_KEY_LIST.SetItemText(row, 0, toUTF16(out.substr(p2, p - p2)).c_str());

			p++;
			p2 = out.find("\n", p);
			string::size_type p3 = out.substr(p, p2 - p).find("[");
			if (p3 != string::npos) {
				p3 += p;
				p2 = p3;
				p2--;
				p3++;
				p3 += mir_strlen("expires: ");
				string::size_type p4 = out.find("]", p3);
				list_EXISTING_KEY_LIST.SetItemText(row, 4, toUTF16(out.substr(p3, p4 - p3)).c_str());
			}
			else p2--;

			list_EXISTING_KEY_LIST.SetItemText(row, 3, toUTF16(out.substr(p, p2 - p)).c_str());

			p = out.find("uid  ", p);
			p += mir_strlen("uid ");
			p2 = out.find("\n", p);
			p3 = out.substr(p, p2 - p).find("<");
			if (p3 != string::npos) {
				p3 += p;
				p2 = p3;
				p2--;
				p3++;
				string::size_type p4 = out.find(">", p3);
				list_EXISTING_KEY_LIST.SetItemText(row, 1, toUTF16(out.substr(p3, p4 - p3)).c_str());
			}
			else p2--;

			p = out.find_first_not_of(" ", p);
			list_EXISTING_KEY_LIST.SetItemText(row, 2, toUTF16(out.substr(p, p2 - p)).c_str());
			i++;
		}

		if (list_EXISTING_KEY_LIST.GetItemCount()) {
			list_EXISTING_KEY_LIST.SetColumnWidth(0, LVSCW_AUTOSIZE);
			list_EXISTING_KEY_LIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
			list_EXISTING_KEY_LIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
			list_EXISTING_KEY_LIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
			list_EXISTING_KEY_LIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
			list_EXISTING_KEY_LIST.SetColumnWidth(5, LVSCW_AUTOSIZE);
		}
		return true;
	}

	bool OnApply() override
	{
		int i = list_EXISTING_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return false; //TODO: error message

		list_EXISTING_KEY_LIST.GetItemText(i, 0, id, _countof(id));
		extern CCtrlEdit *edit_p_PubKeyEdit;

		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"-a");
		params.addParam(L"--export");
		params.addParam(id);
		if (!gpg_launcher(params))
			return false;
		if (params.result == pxNotFound)
			return false;

		string out(params.out);
		size_t p1 = out.find("-----BEGIN PGP PUBLIC KEY BLOCK-----");
		if (p1 != std::string::npos) {
			size_t p2 = out.find("-----END PGP PUBLIC KEY BLOCK-----", p1);
			if (p2 != std::string::npos) {
				p2 += mir_strlen("-----END PGP PUBLIC KEY BLOCK-----");
				out = out.substr(p1, p2 - p1);
				if (edit_p_PubKeyEdit)
					edit_p_PubKeyEdit->SetText(_A2T(out.c_str()));
			}
			else MessageBox(nullptr, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);
		}
		else MessageBox(nullptr, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);

		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "LoadKeyWindow");
	}

	void onChange_EXISTING_KEY_LIST(CCtrlListView::TEventInfo *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDOK), TRUE);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Import key dialog

class CDlgImportKey : public CDlgBase
{
	MCONTACT hContact;
	CCtrlCombo combo_KEYSERVER;
	CCtrlButton btn_IMPORT;

public:
	CDlgImportKey(MCONTACT _hContact) :
		CDlgBase(g_plugin, IDD_IMPORT_KEY),
		combo_KEYSERVER(this, IDC_KEYSERVER),
		btn_IMPORT(this, IDC_IMPORT)
	{
		hContact = _hContact;
		btn_IMPORT.OnClick = Callback(this, &CDlgImportKey::onClick_IMPORT);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "ImportKeyWindow");

		combo_KEYSERVER.AddString(L"subkeys.pgp.net");
		combo_KEYSERVER.AddString(L"keys.gnupg.net");
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "ImportKeyWindow");
	}

	void onClick_IMPORT(CCtrlButton *)
	{
		gpg_execution_params params;
		params.addParam(L"--keyserver");
		params.addParam(combo_KEYSERVER.GetText());
		params.addParam(L"--recv-keys");
		params.addParam(toUTF16(globals.hcontact_data[hContact].key_in_prescense));
		gpg_launcher(params);

		MessageBoxA(nullptr, params.out.c_str(), "GPG output", MB_OK);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// COptGpgMainDlg class

static class COptGpgMainDlg *g_pMainDlg;

class COptGpgMainDlg : public CDlgBase
{
	bool old_bFileTransfers = g_plugin.bFileTransfers;

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

		list_USERLIST.OnItemChanged = Callback(this, &COptGpgMainDlg::onItemChanged_USERLIST);

		CreateLink(check_DEBUG_LOG, g_plugin.bDebugLog);
		CreateLink(check_JABBER_API, g_plugin.bJabberAPI);
		CreateLink(check_AUTO_EXCHANGE, g_plugin.bAutoExchange);
		CreateLink(check_FILE_TRANSFERS, g_plugin.bFileTransfers);
	}

	bool OnInitDialog() override
	{
		g_pMainDlg = this;

		list_USERLIST.AddColumn(0, TranslateT("Contact"), 60);
		list_USERLIST.AddColumn(1, TranslateT("Key ID"), 50);
		list_USERLIST.AddColumn(2, TranslateT("Name"), 50);
		list_USERLIST.AddColumn(3, TranslateT("Email"), 50);
		list_USERLIST.AddColumn(4, TranslateT("Account"), 60);
		list_USERLIST.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);

		for (auto &hContact : Contacts()) {
			if (!isContactHaveKey(hContact))
				continue;

			auto *pa = Proto_GetAccount(Proto_GetBaseAccountName(hContact));
			if (pa == nullptr)
				continue;

			wchar_t *name = Clist_GetContactDisplayName(hContact);

			int row = list_USERLIST.AddItem(L"", 0, hContact);
			list_USERLIST.SetItemText(row, 0, name);

			list_USERLIST.SetItemText(row, 4, pa->tszAccountName);

			CMStringW tmp = g_plugin.getMStringW(hContact, "KeyID", L"not set");
			list_USERLIST.SetItemText(row, 1, tmp);

			tmp = g_plugin.getMStringW(hContact, "KeyMainName", L"not set");
			list_USERLIST.SetItemText(row, 2, tmp);

			tmp = g_plugin.getMStringW(hContact, "KeyMainEmail", L"not set");
			list_USERLIST.SetItemText(row, 3, tmp);
				
			if (g_plugin.getByte(hContact, "GPGEncryption", 0))
				list_USERLIST.SetCheckState(row, 1);
		}

		SetListAutoSize();

		edit_LOG_FILE_EDIT.SetText(ptrW(g_plugin.getWStringA("szLogFilePath", L"")));

		check_JABBER_API.Enable();
		check_AUTO_EXCHANGE.Enable(g_plugin.bJabberAPI);

		lbl_CURRENT_KEY.SetText(CMStringW(FORMAT, L"%s: %s", TranslateT("Default private key ID"), ptrW(g_plugin.getWStringA("KeyID", TranslateT("not set"))).get()));

		check_JABBER_API.SetState(g_plugin.getByte("bJabberAPI", 1));
		check_FILE_TRANSFERS.SetState(g_plugin.getByte("bFileTransfers", 0));
		check_AUTO_EXCHANGE.SetState(g_plugin.getByte("bAutoExchange", 0));

		//TODO: get rid of following s..t
		////////////////
		hwndCurKey_p = lbl_CURRENT_KEY.GetHwnd();
		////////////////
		return true;
	}

	bool OnApply() override
	{
		globals.debuglog.init();

		if (g_plugin.bFileTransfers != old_bFileTransfers)
			g_plugin.bSameAction = false;

		g_plugin.setWString("szLogFilePath", ptrW(edit_LOG_FILE_EDIT.GetText()));
		return true;
	}

	void OnDestroy() override
	{
		hwndCurKey_p = nullptr;
		g_pMainDlg = nullptr;
	}

	void onClick_DELETE_KEY_BUTTON(CCtrlButton*)
	{
		int idx = list_USERLIST.GetSelectionMark();
		if (idx == -1)
			return;

		bool keep = false;
		bool ismetacontact = false;
		MCONTACT meta = NULL;
		MCONTACT hContact = list_USERLIST.GetItemData(idx);
		if (db_mc_isMeta(hContact)) {
			meta = hContact;
			hContact = metaGetMostOnline(hContact);
			ismetacontact = true;
		}
		else if ((meta = db_mc_getMeta(hContact)) != NULL) {
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
				params.addParam(_A2T(tmp).get());
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
				setSrmmIcon(hContact);
			}
		}
		else {
			g_plugin.delSetting(hContact, "KeyID");
			g_plugin.delSetting(hContact, "GPGPubKey");
			g_plugin.delSetting(hContact, "KeyMainName");
			g_plugin.delSetting(hContact, "KeyType");
			g_plugin.delSetting(hContact, "KeyMainEmail");
			g_plugin.delSetting(hContact, "KeyComment");
			setSrmmIcon(hContact);
		}

		list_USERLIST.SetItemText(idx, 3, TranslateT("not set"));
		list_USERLIST.SetItemText(idx, 2, TranslateT("not set"));
		list_USERLIST.SetItemText(idx, 1, TranslateT("not set"));
	}

	void onClick_SELECT_KEY(CCtrlButton*)
	{
		ShowFirstRunDialog();
	}

	void onClick_SAVE_KEY_BUTTON(CCtrlButton*)
	{
		int idx = list_USERLIST.GetSelectionMark();
		if (idx == -1)
			return;

		MCONTACT hContact = list_USERLIST.GetItemData(idx);
		ptrW tmp(GetFilePath(TranslateT("Export public key"), L"*", TranslateT(".asc pubkey file"), true));
		if (tmp) {
			CMStringW str(g_plugin.getMStringW(hContact, "GPGPubKey"));
			str.Replace(L"\r", L"");

			wfstream f(tmp, std::ios::out);
			f << str.c_str();
			f.close();
		}
	}

	void onClick_COPY_KEY(CCtrlButton *)
	{
		CMStringW str(g_plugin.getMStringW("GPGPubKey"));
		str.Replace(L"\n", L"\r\n");
		Utils_ClipboardCopy(str);
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
		NMLISTVIEW *hdr = ev->nmlv;
		if (hdr->iItem == -1)
			return;

		MCONTACT hContact = list_USERLIST.GetItemData(hdr->iItem);
		if (list_USERLIST.GetCheckState(hdr->iItem))
			g_plugin.setByte(hContact, "GPGEncryption", 1);
		else
			g_plugin.setByte(hContact, "GPGEncryption", 0);
		setSrmmIcon(hContact);
	}

	void SetLineText(int i, const wchar_t *pwszText)
	{
		int idx = list_USERLIST.GetSelectionMark();
		if (idx != -1)
			list_USERLIST.SetItemText(idx, i, pwszText);			
	}

	void SetListAutoSize()
	{
		if (list_USERLIST.GetItemCount() == 0)
			return;

		list_USERLIST.SetColumnWidth(0, LVSCW_AUTOSIZE);
		list_USERLIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
		list_USERLIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
		list_USERLIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
		list_USERLIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// COptGpgBinDlg class

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
		g_plugin.setWString("szGpgBinPath", ptrW(edit_BIN_PATH.GetText()));

		ptrW wszHomeDir(edit_HOME_DIR.GetText());
		while (wszHomeDir[mir_wstrlen(wszHomeDir) - 1] == '\\')
			wszHomeDir[mir_wstrlen(wszHomeDir) - 1] = '\0';
		g_plugin.setWString("szHomePath", wszHomeDir);
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
	{
		CreateLink(check_STRIP_TAGS, g_plugin.bStripTags);
		CreateLink(check_APPEND_TAGS, g_plugin.bAppendTags);
	}

	bool OnInitDialog() override
	{
		edit_IN_OPEN_TAG.SetText(g_plugin.getMStringW("szInOpenTag", L"<GPGdec>"));
		edit_IN_CLOSE_TAG.SetText(g_plugin.getMStringW("szInCloseTag", L"</GPGdec>"));
		edit_OUT_OPEN_TAG.SetText(g_plugin.getMStringW("szOutOpenTag", L"<GPGenc>"));
		edit_OUT_CLOSE_TAG.SetText(g_plugin.getMStringW("szOutCloseTag", L"</GPGenc>"));
		return true;
	}

	bool OnApply() override
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
		return true;
	}
};

class COptGpgAdvDlg : public CDlgBase
{
	CCtrlButton btn_EXPORT, btn_IMPORT;
	CCtrlCheck chkPresenceSub, chkSendErrors;

public:
	COptGpgAdvDlg() : CDlgBase(g_plugin, IDD_OPT_GPG_ADVANCED),
		btn_EXPORT(this, IDC_EXPORT),
		btn_IMPORT(this, IDC_IMPORT),
		chkSendErrors(this, IDC_SEND_ERRORS),
		chkPresenceSub(this, IDC_PRESCENSE_SUBSCRIPTION)
	{
		btn_EXPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_EXPORT);
		btn_IMPORT.OnClick = Callback(this, &COptGpgAdvDlg::onClick_IMPORT);

		CreateLink(chkSendErrors, g_plugin.bSendErrorMessages);
		CreateLink(chkPresenceSub, g_plugin.bPresenceSigning);
	}

	bool OnInitDialog() override
	{
		chkPresenceSub.Enable(g_plugin.bJabberAPI);
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
	CDlgLoadPubKeyDlg(MCONTACT _p1) :
		CDlgBase(g_plugin, IDD_LOAD_PUBLIC_KEY),
		hContact(_p1),
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
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "LoadKeyWindow");

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
		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "LoadKeyWindow");
		edit_p_PubKeyEdit = nullptr;
	}

	void onClick_SELECT_EXISTING(CCtrlButton*)
	{
		(new CDlgLoadExistingKey())->Show();
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

				if (hContact && g_pMainDlg)
					g_pMainDlg->SetLineText(1, toUTF16(output.substr(s, s2 - s)).c_str());

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

					if (hContact && g_pMainDlg)
						g_pMainDlg->SetLineText(2, toUTF16(output.substr(s, s2 - s - 1)).c_str());

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

							if (hContact && g_pMainDlg)
								g_pMainDlg->SetLineText(3, toUTF16(output.substr(s, s2 - s)).c_str());
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

							if (hContact && g_pMainDlg)
								g_pMainDlg->SetLineText(3, toUTF16(output.substr(s2, s - s2)).c_str());
						}
					}
				}
				if (hContact && g_pMainDlg)
					g_pMainDlg->SetListAutoSize();
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
			if (globals.debuglog)
				globals.debuglog << "info: Failed to read key file";
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


void ShowLoadPublicKeyDialog(MCONTACT hContact, bool bModal)
{
	CDlgLoadPubKeyDlg *d = new CDlgLoadPubKeyDlg(hContact);
	if (bModal)
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
