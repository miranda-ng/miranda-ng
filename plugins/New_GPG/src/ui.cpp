// Copyright (c) 2017-18 sss
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

//HWND hwndFirstRun = nullptr, hwndSetDirs = nullptr, hwndNewKey = nullptr, hwndKeyGen = nullptr, hwndSelectExistingKey = nullptr;

CCtrlListView *list_hwndList_g = nullptr;


void CDlgEncryptedFileMsgBox::OnInitDialog()
{
	globals.bDecryptFiles = false;
}

CDlgEncryptedFileMsgBox::CDlgEncryptedFileMsgBox() : CDlgBase(g_plugin.getInst(), IDD_ENCRYPTED_FILE_MSG_BOX),
chk_REMEMBER(this, IDC_REMEMBER),
btn_IGNORE(this, IDC_IGNORE), btn_DECRYPT(this, IDC_DECRYPT)
{
	btn_IGNORE.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_IGNORE);
	btn_DECRYPT.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_DECRYPT);
}


void CDlgEncryptedFileMsgBox::onClick_IGNORE(CCtrlButton*)
{
	if (chk_REMEMBER.GetState())
	{
		db_set_b(NULL, szGPGModuleName, "bSameAction", 1);
		globals.bSameAction = true;
	}
	this->Close();
}

void CDlgEncryptedFileMsgBox::onClick_DECRYPT(CCtrlButton*)
{
	globals.bDecryptFiles = true;
	if (chk_REMEMBER.GetState())
	{
		db_set_b(NULL, szGPGModuleName, "bFileTransfers", 1);
		globals.bFileTransfers = true;
		db_set_b(NULL, szGPGModuleName, "bSameAction", 0);
		globals.bSameAction = false;
	}
	this->Close();
}



CDlgExportKeysMsgBox::CDlgExportKeysMsgBox() : CDlgBase(g_plugin.getInst(), IDD_EXPORT_TYPE),
btn_OK(this, IDC_OK), btn_CANCEL(this, IDC_CANCEL),
chk_PUBLIC(this, IDC_PUBLIC), chk_PRIVATE(this, IDC_PRIVATE), chk_ALL(this, IDC_ALL)
{
	btn_OK.OnClick = Callback(this, &CDlgExportKeysMsgBox::onClick_OK);
	btn_CANCEL.OnClick = Callback(this, &CDlgExportKeysMsgBox::onClick_CANCEL);
}
void CDlgExportKeysMsgBox::OnInitDialog()
{
	chk_PUBLIC.SetState(1);
}
void CDlgExportKeysMsgBox::onClick_OK(CCtrlButton*)
{
	if (chk_PUBLIC.GetState())
		ExportGpGKeysFunc(0);
	else if (chk_PRIVATE.GetState())
		ExportGpGKeysFunc(1);
	else if (chk_ALL.GetState())
		ExportGpGKeysFunc(2);
	this->Close();
}
void CDlgExportKeysMsgBox::onClick_CANCEL(CCtrlButton*)
{
	this->Close();
}



CDlgChangePasswdMsgBox::CDlgChangePasswdMsgBox() : CDlgBase(g_plugin.getInst(), IDD_CHANGE_PASSWD),
btn_OK(this, ID_OK),
edit_NEW_PASSWD1(this, IDC_NEW_PASSWD1), edit_NEW_PASSWD2(this, IDC_NEW_PASSWD2), edit_OLD_PASSWD(this, IDC_OLD_PASSWD)
{
	btn_OK.OnClick = Callback(this, &CDlgChangePasswdMsgBox::onClick_OK);
}

void CDlgChangePasswdMsgBox::onClick_OK(CCtrlButton*)
{
	//TODO: show some prgress
	{
		if (mir_wstrcmp(edit_NEW_PASSWD1.GetText(), edit_NEW_PASSWD2.GetText()))
		{
			MessageBox(m_hwnd, TranslateT("New passwords do not match"), TranslateT("Error"), MB_OK);
			return;
		}
		std::string old_pass, new_pass;
		//			wchar_t buf[256] = { 0 };
		new_pass = toUTF8(edit_NEW_PASSWD1.GetText());
		old_pass = toUTF8(edit_OLD_PASSWD.GetText());
		bool old_pass_match = false;
		wchar_t *pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", L"");
		if (!mir_wstrcmp(pass, edit_OLD_PASSWD.GetText()))
			old_pass_match = true;
		mir_free(pass);

		if (!old_pass_match) {
			if (globals.key_id_global[0]) {
				string dbsetting = "szKey_";
				dbsetting += toUTF8(globals.key_id_global);
				dbsetting += "_Password";
				pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), L"");
				if (!mir_wstrcmp(pass, edit_OLD_PASSWD.GetText()))
					old_pass_match = true;
				mir_free(pass);
			}
		}

		if (!old_pass_match)
			if (MessageBox(m_hwnd, TranslateT("Old password does not match, you can continue, but GPG will reject wrong password.\nDo you want to continue?"), TranslateT("Error"), MB_YESNO) == IDNO)
				return;

		std::vector<std::wstring> cmd;
		string output;
		DWORD exitcode;
		cmd.push_back(L"--edit-key");
		cmd.push_back(globals.key_id_global);
		cmd.push_back(L"passwd");
		gpg_execution_params_pass params(cmd, old_pass, new_pass);
		pxResult result;
		params.out = &output;
		params.code = &exitcode;
		params.result = &result;
		boost::thread gpg_thread(boost::bind(&pxEexcute_passwd_change_thread, &params));
		if (!gpg_thread.timed_join(boost::posix_time::minutes(10))) {
			gpg_thread.~thread();
			if (params.child)
				boost::process::terminate(*(params.child));
			if (globals.bDebugLog)
				globals.debuglog << std::string(time_str() + ": GPG execution timed out, aborted");
			this->Close();
			return;
		}
		if (result == pxNotFound)
			return;
	}
	this->Close();
}



CDlgFirstRun::CDlgFirstRun() : CDlgBase(g_plugin.getInst(), IDD_FIRST_RUN),
list_KEY_LIST(this, IDC_KEY_LIST),
btn_COPY_PUBKEY(this, IDC_COPY_PUBKEY), btn_EXPORT_PRIVATE(this, IDC_EXPORT_PRIVATE), btn_CHANGE_PASSWD(this, IDC_CHANGE_PASSWD), btn_GENERATE_RANDOM(this, IDC_GENERATE_RANDOM),
btn_GENERATE_KEY(this, IDC_GENERATE_KEY), btn_OTHER(this, IDC_OTHER), btn_DELETE_KEY(this, IDC_DELETE_KEY), btn_OK(this, ID_OK),
edit_KEY_PASSWORD(this, IDC_KEY_PASSWORD),
combo_ACCOUNT(this, IDC_ACCOUNT),
lbl_KEY_ID(this, IDC_KEY_ID), lbl_GENERATING_KEY(this, IDC_GENERATING_KEY)
{
	fp[0] = 0;
	list_hwndList_g = &list_KEY_LIST;

	btn_COPY_PUBKEY.OnClick = Callback(this, &CDlgFirstRun::onClick_COPY_PUBKEY);
	btn_EXPORT_PRIVATE.OnClick = Callback(this, &CDlgFirstRun::onClick_EXPORT_PRIVATE);
	btn_CHANGE_PASSWD.OnClick = Callback(this, &CDlgFirstRun::onClick_CHANGE_PASSWD);
	btn_GENERATE_RANDOM.OnClick = Callback(this, &CDlgFirstRun::onClick_GENERATE_RANDOM);
	btn_GENERATE_KEY.OnClick = Callback(this, &CDlgFirstRun::onClick_GENERATE_KEY);
	btn_OTHER.OnClick = Callback(this, &CDlgFirstRun::onClick_OTHER);
	btn_DELETE_KEY.OnClick = Callback(this, &CDlgFirstRun::onClick_DELETE_KEY);
	btn_OK.OnClick = Callback(this, &CDlgFirstRun::onClick_OK);

}
void CDlgFirstRun::OnInitDialog()
{
	SetWindowPos(m_hwnd, nullptr, globals.firstrun_rect.left, globals.firstrun_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetCaption(TranslateT("Set own key"));
	btn_COPY_PUBKEY.Disable();
	btn_EXPORT_PRIVATE.Disable();
	btn_CHANGE_PASSWD.Disable();

	list_KEY_LIST.AddColumn(0, TranslateT("Key ID"), 50);
	list_KEY_LIST.AddColumn(1, TranslateT("Email"), 30);
	list_KEY_LIST.AddColumn(2, TranslateT("Name"), 250);
	list_KEY_LIST.AddColumn(3, TranslateT("Creation date"), 30);
	list_KEY_LIST.AddColumn(4, TranslateT("Expire date"), 30);
	list_KEY_LIST.AddColumn(5, TranslateT("Key length"), 30);
	list_KEY_LIST.AddColumn(6, TranslateT("Accounts"), 30);
	list_KEY_LIST.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);

	refresh_key_list();

	combo_ACCOUNT.AddString(TranslateT("Default"));

	for (auto &pa : Accounts()) {
		if (StriStr(pa->szModuleName, "metacontacts"))
			continue;
		if (StriStr(pa->szModuleName, "weather"))
			continue;
		std::string acc = toUTF8(pa->tszAccountName);
		acc += "(";
		acc += pa->szModuleName;
		acc += ")";
		//acc += "_KeyID";
		combo_ACCOUNT.AddStringA(acc.c_str());
	}
	combo_ACCOUNT.SelectString(TranslateT("Default"));
	string keyinfo = Translate("key ID");
	keyinfo += ": ";
	char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
	keyinfo += (mir_strlen(keyid) > 0) ? keyid : Translate("not set");
	mir_free(keyid);
	lbl_KEY_ID.SetTextA(keyinfo.c_str());

	combo_ACCOUNT.OnChange = Callback(this, &CDlgFirstRun::onChange_ACCOUNT);
	list_KEY_LIST.OnClick = Callback(this, &CDlgFirstRun::onChange_KEY_LIST);
}
void CDlgFirstRun::onClick_COPY_PUBKEY(CCtrlButton*)
{
	int  i = list_KEY_LIST.GetSelectionMark();
	if (i == -1)
		return;
	if (OpenClipboard(m_hwnd))
	{
		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"-a");
		cmd.push_back(L"--export");
		cmd.push_back(fp);
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		if (!gpg_launcher(params))
			return;
		if (result == pxNotFound)
			return;
		boost::algorithm::erase_all(out, "\r");
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, out.size() + 1);
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
		memcpy(szKey, out.c_str(), out.size());
		szKey[out.size()] = '\0';
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
void CDlgFirstRun::onClick_EXPORT_PRIVATE(CCtrlButton*)
{
	{
		int  i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;
		wchar_t *p = GetFilePath(L"Choose file to export key", L"*", L"Any file", true);
		if (!p || !p[0]) {
			delete[] p;
			//TODO: handle error
			return;
		}
		char *path = mir_u2a(p);
		delete[] p;
		std::ofstream file;
		file.open(path, std::ios::trunc | std::ios::out);
		mir_free(path);
		if (!file.is_open())
			return; //TODO: handle error
		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"-a");
		cmd.push_back(L"--export-secret-keys");
		cmd.push_back(fp);
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		if (!gpg_launcher(params)) {
			return;
		}
		if (result == pxNotFound)
			return;
		boost::algorithm::erase_all(out, "\r");
		file << out;
		if (file.is_open())
			file.close();
	}
}
void CDlgFirstRun::onClick_CHANGE_PASSWD(CCtrlButton*)
{
	int  i = list_KEY_LIST.GetSelectionMark();
	if (i == -1)
		return;
	list_KEY_LIST.GetItemText(i, 0, globals.key_id_global, _countof(globals.key_id_global));

	//temporary code follows
	std::vector<std::wstring> cmd;
	std::string old_pass, new_pass;
	string output;
	DWORD exitcode;
	cmd.push_back(L"--edit-key");
	cmd.push_back(globals.key_id_global);
	cmd.push_back(L"passwd");
	gpg_execution_params_pass params(cmd, old_pass, new_pass);
	pxResult result;
	params.out = &output;
	params.code = &exitcode;
	params.result = &result;
	boost::thread gpg_thread(boost::bind(&pxEexcute_passwd_change_thread, &params));
	if (!gpg_thread.timed_join(boost::posix_time::minutes(10))) {
		gpg_thread.~thread();
		if (params.child)
			boost::process::terminate(*(params.child));
		if (globals.bDebugLog)
			globals.debuglog << std::string(time_str() + ": GPG execution timed out, aborted");
		this->Close();
	}

}
void CDlgFirstRun::onClick_GENERATE_RANDOM(CCtrlButton*)
{
	lbl_GENERATING_KEY.SendMsg(WM_SETFONT, (WPARAM)globals.bold_font, TRUE);
	lbl_GENERATING_KEY.SetText(TranslateT("Generating new random key, please wait"));
	btn_GENERATE_KEY.Disable();
	btn_OTHER.Disable();
	btn_DELETE_KEY.Disable();
	list_KEY_LIST.Disable();
	btn_GENERATE_RANDOM.Disable();
	gpg_use_new_random_key(combo_ACCOUNT.GetTextA());
	this->Close();
}
void CDlgFirstRun::onClick_GENERATE_KEY(CCtrlButton*)
{
	CDlgKeyGen *d = new CDlgKeyGen;
	d->DoModal();
	refresh_key_list();
}
void CDlgFirstRun::onClick_OTHER(CCtrlButton*)
{
	void ShowLoadPublicKeyDialog(bool = false);
	globals.item_num = 0;		 //black magic here
	globals.user_data[1] = 0;
	ShowLoadPublicKeyDialog(true);
	refresh_key_list();
}
void CDlgFirstRun::onClick_DELETE_KEY(CCtrlButton*)
{
	int  i = list_KEY_LIST.GetSelectionMark();
	if (i == -1)
		return;
	list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
	{
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"--fingerprint");
		cmd.push_back(fp);
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		if (!gpg_launcher(params))
			return;
		if (result == pxNotFound)
			return;
		string::size_type s = out.find("Key fingerprint = ");
		s += mir_strlen("Key fingerprint = ");
		string::size_type s2 = out.find("\n", s);
		wchar_t *str = nullptr;
		{
			string tmp = out.substr(s, s2 - s - 1).c_str();
			string::size_type p = 0;
			while ((p = tmp.find(" ", p)) != string::npos)
				tmp.erase(p, 1);

			str = mir_a2u(tmp.c_str());
		}
		cmd.clear();
		out.clear();
		cmd.push_back(L"--batch");
		cmd.push_back(L"--delete-secret-and-public-key");
		cmd.push_back(L"--fingerprint");
		cmd.push_back(str);
		mir_free(str);
		if (!gpg_launcher(params))
			return;
		if (result == pxNotFound)
			return;
	}
	{
		char *buf = mir_strdup(combo_ACCOUNT.GetTextA());
		if (!mir_strcmp(buf, Translate("Default"))) {
			db_unset(NULL, szGPGModuleName, "GPGPubKey");
			db_unset(NULL, szGPGModuleName, "KeyID");
			db_unset(NULL, szGPGModuleName, "KeyComment");
			db_unset(NULL, szGPGModuleName, "KeyMainName");
			db_unset(NULL, szGPGModuleName, "KeyMainEmail");
			db_unset(NULL, szGPGModuleName, "KeyType");
		}
		else {
			std::string acc_str = buf;
			acc_str += "_GPGPubKey";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
			acc_str = buf;
			acc_str += "_KeyMainName";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
			acc_str = buf;
			acc_str += "_KeyID";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
			acc_str = buf;
			acc_str += "_KeyComment";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
			acc_str = buf;
			acc_str += "_KeyMainEmail";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
			acc_str = buf;
			acc_str += "_KeyType";
			db_unset(NULL, szGPGModuleName, acc_str.c_str());
		}
		if (buf)
			mir_free(buf);
	}
	list_KEY_LIST.DeleteItem(i);
}
void CDlgFirstRun::onClick_OK(CCtrlButton*)
{
	{
		int  i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
		wchar_t *name = new wchar_t[65];
		list_KEY_LIST.GetItemText(i, 2, name, 64);
		{
			if (wcschr(name, '(')) {
				wstring str = name;
				wstring::size_type p = str.find(L"(") - 1;
				mir_wstrcpy(name, str.substr(0, p).c_str());
			}
		}
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"-a");
		cmd.push_back(L"--export");
		cmd.push_back(fp);
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		if (!gpg_launcher(params))
		{
			delete[] name;
			return;
		}
		if (result == pxNotFound)
		{
			delete[] name;
			return;
		}

		boost::algorithm::erase_all(out, "\r");
		{
			char *buf = mir_strdup(combo_ACCOUNT.GetTextA());
			if (!mir_strcmp(buf, Translate("Default"))) {
				db_set_s(NULL, szGPGModuleName, "GPGPubKey", out.c_str());
				db_set_ws(NULL, szGPGModuleName, "KeyMainName", name);
				db_set_ws(NULL, szGPGModuleName, "KeyID", fp);
			}
			else {
				std::string acc_str = buf;
				acc_str += "_GPGPubKey";
				db_set_s(NULL, szGPGModuleName, acc_str.c_str(), out.c_str());
				acc_str = buf;
				acc_str += "_KeyMainName";
				db_set_ws(NULL, szGPGModuleName, acc_str.c_str(), name);
				acc_str = buf;
				acc_str += "_KeyID";
				db_set_ws(NULL, szGPGModuleName, acc_str.c_str(), fp);
			}
			if (!mir_strcmp(buf, Translate("Default"))) {
				wstring keyinfo = TranslateT("Default private key ID");
				keyinfo += L": ";
				keyinfo += (fp[0]) ? fp : L"not set";
				extern HWND hwndCurKey_p;
				SetWindowText(hwndCurKey_p, keyinfo.c_str());
			}
			if (buf)
				mir_free(buf);
		}
		wchar_t *passwd = mir_wstrdup(edit_KEY_PASSWORD.GetText());
		if (passwd && passwd[0])
		{
			string dbsetting = "szKey_";
			char *keyid = mir_u2a(fp);
			dbsetting += keyid;
			mir_free(keyid);
			dbsetting += "_Password";
			db_set_ws(NULL, szGPGModuleName, dbsetting.c_str(), passwd);
		}
		mir_free(passwd);
		delete[] name;
	}
	//bAutoExchange = CheckStateStoreDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange") != 0; //TODO: check is it just typo, or doing something
	globals.gpg_valid = isGPGValid();
	globals.gpg_keyexist = isGPGKeyExist();
	DestroyWindow(m_hwnd);
}
void CDlgFirstRun::onChange_ACCOUNT(CCtrlCombo*)
{
	char *buf = mir_strdup(combo_ACCOUNT.GetTextA());
	if (!mir_strcmp(buf, Translate("Default"))) {
		string keyinfo = Translate("key ID");
		keyinfo += ": ";
		char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
		keyinfo += (mir_strlen(keyid) > 0) ? keyid : Translate("not set");
		mir_free(keyid);
		lbl_KEY_ID.SetTextA(keyinfo.c_str());
	}
	else {
		string keyinfo = Translate("key ID");
		keyinfo += ": ";
		std::string acc_str = buf;
		acc_str += "_KeyID";
		char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, acc_str.c_str(), "");
		keyinfo += (mir_strlen(keyid) > 0) ? keyid : Translate("not set");
		mir_free(keyid);
		lbl_KEY_ID.SetTextA(keyinfo.c_str());
	}
	if (buf)
		mir_free(buf);
}
void CDlgFirstRun::onChange_KEY_LIST(CCtrlListView::TEventInfo *ev) //TODO: check if this work
{
	if (ev->nmlv)
	{
		NMLISTVIEW *hdr = ev->nmlv;

		if (hdr->hdr.code == NM_CLICK) {
			btn_OK.Enable();
			btn_COPY_PUBKEY.Enable();
			btn_EXPORT_PRIVATE.Enable();
			btn_CHANGE_PASSWD.Enable();
		}
	}
}
void CDlgFirstRun::OnDestroy()
{
	GetWindowRect(m_hwnd, &globals.firstrun_rect);
	db_set_dw(NULL, szGPGModuleName, "FirstrunWindowX", globals.firstrun_rect.left);
	db_set_dw(NULL, szGPGModuleName, "FirstrunWindowY", globals.firstrun_rect.top);
}


void CDlgFirstRun::refresh_key_list()
{
	list_KEY_LIST.DeleteAllItems();
	int i = 1;
	{
		{
			//parse gpg output
			string out;
			DWORD code;
			pxResult result;
			wstring::size_type p = 0, p2 = 0, stop = 0;
			{
				std::vector<wstring> cmd;
				cmd.push_back(L"--batch");
				cmd.push_back(L"--list-secret-keys");
				gpg_execution_params params(cmd);
				params.out = &out;
				params.code = &code;
				params.result = &result;
				if (!gpg_launcher(params))
					return;
				if (result == pxNotFound)
					return;
			}
			while (p != string::npos) {
				if ((p = out.find("sec  ", p)) == string::npos)
					break;
				p += 5;
				if (p < stop)
					break;
				stop = p;
				p2 = out.find("/", p) - 1;
				wchar_t *key_len = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str()), *creation_date = nullptr, *expire_date = nullptr;
				p2 += 2;
				p = out.find(" ", p2);
				std::wstring key_id = toUTF16(out.substr(p2, p - p2));
				p += 1;
				p2 = out.find(" ", p);
				std::string::size_type p3 = out.find("\n", p);
				if ((p2 != std::string::npos) && (p3 < p2)) {
					p2 = p3;
					creation_date = mir_wstrdup(toUTF16(out.substr(p, p2 - p - 1)).c_str());
				}
				else {
					creation_date = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
					p2 = out.find("[", p2);
					p2 = out.find("expires:", p2);
					p2 += mir_strlen("expires:");
					if (p2 != std::string::npos) {
						p2++;
						p = p2;
						p2 = out.find("]", p);
						expire_date = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
						//check expiration
						bool expired = false;
						{
							boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
							wchar_t buf[5];
							wcsncpy_s(buf, expire_date, _TRUNCATE);
							int year = _wtoi(buf);
							if (year < now.date().year())
								expired = true;
							else if (year == now.date().year()) {
								wcsncpy_s(buf, (expire_date + 5), _TRUNCATE);
								int month = _wtoi(buf);
								if (month < now.date().month())
									expired = true;
								else if (month == now.date().month()) {
									wcsncpy_s(buf, (expire_date + 8), _TRUNCATE);
									unsigned day = _wtoi(buf);
									if (day <= now.date().day_number())
										expired = true;
								}
							}
						}
						if (expired) {
							mir_free(key_len);
							mir_free(creation_date);
							mir_free(expire_date);
							//mimic normal behaviour
							p = out.find("uid  ", p);
							p2 = out.find_first_not_of(" ", p + 5);
							p = out.find("<", p2);
							p++;
							//p2 = out.find(">", p);
							//
							continue; //does not add to key list
						}
					}
				}
				int row = list_KEY_LIST.AddItem(L"", 0);
				list_KEY_LIST.SetItemText(row, 3, creation_date);
				mir_free(creation_date);
				if (expire_date) {
					list_KEY_LIST.SetItemText(row, 4, expire_date);
					mir_free(expire_date);
				}
				list_KEY_LIST.SetItemText(row, 5, key_len);
				mir_free(key_len);
				list_KEY_LIST.SetItemText(row, 0, (wchar_t*)key_id.c_str());
				p = out.find("uid  ", p);
				p2 = out.find_first_not_of(" ", p + 5);
				p = out.find("<", p2);

				wstring tmp = toUTF16(out.substr(p2, p - p2));
				list_KEY_LIST.SetItemText(row, 2, (wchar_t*)tmp.c_str());

				p++;
				p2 = out.find(">", p);

				tmp = toUTF16(out.substr(p, p2 - p));
				list_KEY_LIST.SetItemText(row, 1, (wchar_t*)tmp.c_str());

				// get accounts
				std::wstring accs;
				for (auto &pa : Accounts()) {
					std::string setting = toUTF8(pa->tszAccountName);
					setting += "(";
					setting += pa->szModuleName;
					setting += ")";
					setting += "_KeyID";
					wchar_t *str = UniGetContactSettingUtf(NULL, szGPGModuleName, setting.c_str(), L"");
					if (key_id == str) {
						if (!accs.empty())
							accs += L",";
						accs += pa->tszAccountName;
					}
					mir_free(str);
				}
				list_KEY_LIST.SetItemText(row, 6, (wchar_t*)accs.c_str());
			}
			i++;
			list_KEY_LIST.SetColumnWidth(0, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(5, LVSCW_AUTOSIZE);
			list_KEY_LIST.SetColumnWidth(6, LVSCW_AUTOSIZE);
		}
	}
}



CDlgGpgBinOpts::CDlgGpgBinOpts() : CDlgBase(g_plugin.getInst(), IDD_BIN_PATH),
btn_SET_BIN_PATH(this, IDC_SET_BIN_PATH), btn_SET_HOME_DIR(this, IDC_SET_HOME_DIR), btn_OK(this, ID_OK), btn_GENERATE_RANDOM(this, IDC_GENERATE_RANDOM),
edit_BIN_PATH(this, IDC_BIN_PATH), edit_HOME_DIR(this, IDC_HOME_DIR),
chk_AUTO_EXCHANGE(this, IDC_AUTO_EXCHANGE)
{
	btn_SET_BIN_PATH.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_SET_BIN_PATH);
	btn_SET_HOME_DIR.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_SET_HOME_DIR);
	btn_OK.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_OK);
	btn_GENERATE_RANDOM.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_GENERATE_RANDOM);
}
void CDlgGpgBinOpts::OnInitDialog()
{
	CMStringW path;
	bool gpg_exists = false, lang_exists = false;
	{
		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		SetCurrentDirectoryW(mir_path);

		CMStringW gpg_path(mir_path); gpg_path.Append(L"\\GnuPG\\gpg.exe");
		CMStringW gpg_lang_path(mir_path); gpg_lang_path.Append(L"\\GnuPG\\gnupg.nls\\en@quot.mo");

		if (boost::filesystem::exists(gpg_path.c_str())) {
			gpg_exists = true;
			path = L"GnuPG\\gpg.exe";
		}
		else path = gpg_path;

		if (boost::filesystem::exists(gpg_lang_path.c_str()))
			lang_exists = true;
		if (gpg_exists && !lang_exists)
			MessageBox(nullptr, TranslateT("GPG binary found in Miranda folder, but English locale does not exist.\nIt's highly recommended that you place \\gnupg.nls\\en@quot.mo in GnuPG folder under Miranda root.\nWithout this file you may experience many problems with GPG output on non-English systems\nand plugin may completely not work.\nYou have been warned."), TranslateT("Warning"), MB_OK);
	}
	DWORD len = MAX_PATH;
	bool bad_version = false;
	{
		ptrW tmp;
		if (!gpg_exists) {
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", (SHGetValueW(HKEY_CURRENT_USER, L"Software\\GNU\\GnuPG", L"gpgProgram", 0, (void*)path.c_str(), &len) == ERROR_SUCCESS) ? path.c_str() : L"");
			if (tmp[0])
				if (!boost::filesystem::exists((wchar_t*)tmp))
					MessageBox(nullptr, TranslateT("Wrong GPG binary location found in system.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
		}
		else tmp = mir_wstrdup(path.c_str());

		edit_BIN_PATH.SetText(tmp);
		if (gpg_exists/* && lang_exists*/) {
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
			bool _gpg_valid = globals.gpg_valid;
			globals.gpg_valid = true;
			gpg_launcher(params);
			globals.gpg_valid = _gpg_valid; //TODO: check this
			db_unset(NULL, szGPGModuleName, "szGpgBinPath");
			string::size_type p1 = out.find("(GnuPG) ");
			if (p1 != string::npos) {
				p1 += mir_strlen("(GnuPG) ");
				if (out[p1] != '1')
					bad_version = true;
			}
			else {
				bad_version = false;
				MessageBox(nullptr, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Error"), MB_OK);
			}
			if (bad_version)
				MessageBox(nullptr, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
		}
	}
	{
		ptrW tmp(UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L""));
		if (!tmp[0]) {
			wchar_t mir_path[MAX_PATH];
			PathToAbsoluteW(L"\\", mir_path);
			mir_wstrcat(mir_path, L"\\gpg");
			if (_waccess(mir_path, 0) != -1) {
				tmp = mir_wstrdup(mir_path);
				MessageBox(nullptr, TranslateT("\"GPG\" directory found in Miranda root.\nAssuming it's GPG home directory.\nGPG home directory set."), TranslateT("Info"), MB_OK);
			}
			else {
				wstring path_ = _wgetenv(L"APPDATA");
				path_ += L"\\GnuPG";
				tmp = mir_wstrdup(path_.c_str());
			}
		}
		edit_HOME_DIR.SetText(!gpg_exists ? tmp : L"gpg");
	}
	//TODO: additional check for write access
	if (gpg_exists && lang_exists && !bad_version)
		MessageBox(nullptr, TranslateT("Your GPG version is supported. The language file was found.\nGPG plugin should work fine.\nPress OK to continue."), TranslateT("Info"), MB_OK);
	chk_AUTO_EXCHANGE.Enable();
}
void CDlgGpgBinOpts::onClick_SET_BIN_PATH(CCtrlButton*)
{
	GetFilePath(L"Choose gpg.exe", "szGpgBinPath", L"*.exe", L"EXE Executables");
	CMStringW tmp(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", L"gpg.exe")));
	edit_BIN_PATH.SetText(tmp);
	wchar_t mir_path[MAX_PATH];
	PathToAbsoluteW(L"\\", mir_path);
	if (tmp.Find(mir_path, 0) == 0)
	{
		CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
		edit_BIN_PATH.SetText(path);
	}

}
void CDlgGpgBinOpts::onClick_SET_HOME_DIR(CCtrlButton*)
{
	GetFolderPath(L"Set home directory", "szHomePath");
	CMStringW tmp(ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"")));
	edit_HOME_DIR.SetText(tmp);
	wchar_t mir_path[MAX_PATH];
	PathToAbsoluteW(L"\\", mir_path);
	PathToAbsoluteW(L"\\", mir_path);
	if (tmp.Find(mir_path, 0) == 0) {
		CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
		edit_HOME_DIR.SetText(path);
	}

}
void CDlgGpgBinOpts::onClick_OK(CCtrlButton*)
{
	if (gpg_validate_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText()))
	{
		gpg_save_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText());
		globals.gpg_valid = true;
		db_set_b(NULL, szGPGModuleName, "FirstRun", 0);
		this->Hide();
		CDlgFirstRun *d = new CDlgFirstRun;
		d->Show();
		this->Close();
	}
}
void CDlgGpgBinOpts::onClick_GENERATE_RANDOM(CCtrlButton*)
{
	if (gpg_validate_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText()))
	{
		gpg_save_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText());
		globals.gpg_valid = true;
		if (gpg_use_new_random_key())
		{
			db_set_b(NULL, szGPGModuleName, "bAutoExchange", globals.bAutoExchange = chk_AUTO_EXCHANGE.GetState());
			globals.gpg_valid = true;
			db_set_b(NULL, szGPGModuleName, "FirstRun", 0);
			this->Close();
		}
	}
}
void CDlgGpgBinOpts::OnDestroy()
{
	void InitCheck();
	InitCheck();
}

CDlgNewKey::CDlgNewKey(MCONTACT _hContact, wstring _new_key) : CDlgBase(g_plugin.getInst(), IDD_NEW_KEY),
lbl_KEY_FROM(this, IDC_KEY_FROM), lbl_MESSAGE(this, IDC_MESSAGE),
btn_IMPORT(this, ID_IMPORT), btn_IMPORT_AND_USE(this, IDC_IMPORT_AND_USE), btn_IGNORE_KEY(this, IDC_IGNORE_KEY)
{
	hContact = _hContact;
	new_key = _new_key;
	btn_IMPORT.OnClick = Callback(this, &CDlgNewKey::onClick_IMPORT);
	btn_IMPORT_AND_USE.OnClick = Callback(this, &CDlgNewKey::onClick_IMPORT_AND_USE);
	btn_IGNORE_KEY.OnClick = Callback(this, &CDlgNewKey::onClick_IGNORE_KEY);

}
void CDlgNewKey::OnInitDialog()
{
	//new_key_hcnt_mutex.unlock();
	SetWindowPos(m_hwnd, nullptr, globals.new_key_rect.left, globals.new_key_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	wchar_t *tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", L"");
	lbl_MESSAGE.SetText(tmp[0] ? TranslateT("There is existing key for contact, would you like to replace it with new key?") : TranslateT("New public key was received, do you want to import it?"));
	btn_IMPORT_AND_USE.Enable(db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0));
	btn_IMPORT.SetText(tmp[0] ? TranslateT("Replace") : TranslateT("Accept"));
	mir_free(tmp);
	tmp = new wchar_t[256];
	mir_snwprintf(tmp, 255 * sizeof(wchar_t), TranslateT("Received key from %s"), Clist_GetContactDisplayName(hContact));
	lbl_KEY_FROM.SetText(tmp);
	mir_free(tmp);
}
void CDlgNewKey::OnDestroy()
{
	GetWindowRect(m_hwnd, &globals.new_key_rect);
	db_set_dw(NULL, szGPGModuleName, "NewKeyWindowX", globals.new_key_rect.left);
	db_set_dw(NULL, szGPGModuleName, "NewKeyWindowY", globals.new_key_rect.top);
}
void CDlgNewKey::onClick_IMPORT(CCtrlButton*)
{
	ImportKey(hContact, new_key);
	this->Close();
}
void CDlgNewKey::onClick_IMPORT_AND_USE(CCtrlButton*)
{
	ImportKey(hContact, new_key);
	db_set_b(hContact, szGPGModuleName, "GPGEncryption", 1);
	void setSrmmIcon(MCONTACT hContact);
	void setClistIcon(MCONTACT hContact);
	setSrmmIcon(hContact);
	setClistIcon(hContact);
	this->Close();
}
void CDlgNewKey::onClick_IGNORE_KEY(CCtrlButton*)
{
	this->Close();
}

CDlgKeyGen::CDlgKeyGen() : CDlgBase(g_plugin.getInst(), IDD_KEY_GEN),
combo_KEY_TYPE(this, IDC_KEY_TYPE),
edit_KEY_LENGTH(this, IDC_KEY_LENGTH), edit_KEY_PASSWD(this, IDC_KEY_PASSWD), edit_KEY_REAL_NAME(this, IDC_KEY_REAL_NAME), edit_KEY_EMAIL(this, IDC_KEY_EMAIL), edit_KEY_COMMENT(this, IDC_KEY_COMMENT),
edit_KEY_EXPIRE_DATE(this, IDC_KEY_EXPIRE_DATE),
lbl_GENERATING_TEXT(this, IDC_GENERATING_TEXT),
btn_OK(this, IDOK), btn_CANCEL(this, IDCANCEL)
{
	btn_OK.OnClick = Callback(this, &CDlgKeyGen::onClick_OK);
	btn_CANCEL.OnClick = Callback(this, &CDlgKeyGen::onClick_CANCEL);
}
void CDlgKeyGen::OnInitDialog()
{
	SetWindowPos(m_hwnd, nullptr, globals.key_gen_rect.left, globals.key_gen_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	SetCaption(TranslateT("Key Generation dialog"));
	combo_KEY_TYPE.AddString(L"RSA");
	combo_KEY_TYPE.AddString(L"DSA");
	combo_KEY_TYPE.SelectString(L"RSA");
	edit_KEY_EXPIRE_DATE.SetText(L"0");
	edit_KEY_LENGTH.SetText(L"4096");
}

void CDlgKeyGen::onClick_OK(CCtrlButton*)
{
	{
		wstring path;
		{ //data sanity checks
			wchar_t *tmp = mir_wstrdup(combo_KEY_TYPE.GetText());
			if (!tmp)
			{
				MessageBox(nullptr, TranslateT("You must set encryption algorithm first"), TranslateT("Error"), MB_OK);
				return;
			}
			if (mir_wstrlen(tmp) < 3)
			{
				mir_free(tmp);
				tmp = nullptr;
				MessageBox(nullptr, TranslateT("You must set encryption algorithm first"), TranslateT("Error"), MB_OK);
				return;
			}
			mir_free(tmp);
			tmp = mir_wstrdup(edit_KEY_LENGTH.GetText());
			if (!tmp)
			{
				MessageBox(nullptr, TranslateT("Key length must be of length from 1024 to 4096 bits"), TranslateT("Error"), MB_OK);
				return;
			}
			int length = _wtoi(tmp);
			mir_free(tmp);
			if (length < 1024 || length > 4096) {
				MessageBox(nullptr, TranslateT("Key length must be of length from 1024 to 4096 bits"), TranslateT("Error"), MB_OK);
				return;
			}
			tmp = mir_wstrdup(edit_KEY_EXPIRE_DATE.GetText());
			if (!tmp)
			{
				MessageBox(nullptr, TranslateT("Invalid date"), TranslateT("Error"), MB_OK);
				return;
			}
			if (mir_wstrlen(tmp) != 10 && tmp[0] != '0') {
				MessageBox(nullptr, TranslateT("Invalid date"), TranslateT("Error"), MB_OK);
				mir_free(tmp);
				return;
			}
			mir_free(tmp);
			tmp = mir_wstrdup(edit_KEY_REAL_NAME.GetText());
			if (!tmp)
			{
				MessageBox(nullptr, TranslateT("Name must contain at least 5 characters"), TranslateT("Error"), MB_OK);
				return;
			}
			if (mir_wstrlen(tmp) < 5) {
				MessageBox(nullptr, TranslateT("Name must contain at least 5 characters"), TranslateT("Error"), MB_OK);
				mir_free(tmp);
				return;
			}
			else if (wcschr(tmp, '(') || wcschr(tmp, ')'))
			{
				MessageBox(nullptr, TranslateT("Name cannot contain '(' or ')'"), TranslateT("Error"), MB_OK);
				mir_free(tmp);
				return;
			}
			mir_free(tmp);
			tmp = mir_wstrdup(edit_KEY_EMAIL.GetText());
			if (!tmp)
			{
				MessageBox(nullptr, TranslateT("Invalid Email"), TranslateT("Error"), MB_OK);
				return;
			}
			if ((mir_wstrlen(tmp)) < 5 || (!wcschr(tmp, '@')) || (!wcschr(tmp, '.'))) {
				MessageBox(nullptr, TranslateT("Invalid Email"), TranslateT("Error"), MB_OK);
				mir_free(tmp);
				return;
			}
			mir_free(tmp);
		}
		{ //generating key file
			wchar_t *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
			char  *tmp2;// = mir_u2a(tmp);
			path = tmp;
			mir_free(tmp);
			//			  mir_free(tmp2);
			path.append(L"\\new_key");
			wfstream f(path.c_str(), std::ios::out);
			if (!f.is_open()) {
				MessageBox(nullptr, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
				return;
			}
			f << "Key-Type: ";
			tmp2 = mir_u2a(combo_KEY_TYPE.GetText());
			char *subkeytype = (char*)mir_alloc(6);
			if (strstr(tmp2, "RSA"))
				mir_strcpy(subkeytype, "RSA");
			else if (strstr(tmp2, "DSA")) //this is useless check for now, but it will be required if someone add another key types support
				mir_strcpy(subkeytype, "ELG-E");
			f << tmp2;
			mir_free(tmp2);
			f << "\n";
			f << "Key-Length: ";
			f << _wtoi(edit_KEY_LENGTH.GetText());
			f << "\n";
			f << "Subkey-Length: ";
			f << _wtoi(edit_KEY_LENGTH.GetText());
			f << "\n";
			f << "Subkey-Type: ";
			f << subkeytype;
			mir_free(subkeytype);
			f << "\n";
			if (edit_KEY_PASSWD.GetText()[0])
			{
				f << "Passphrase: ";
				f << toUTF8(edit_KEY_PASSWD.GetText()).c_str();
				f << "\n";
			}
			f << "Name-Real: ";
			f << toUTF8(edit_KEY_REAL_NAME.GetText()).c_str();
			f << "\n";
			if (edit_KEY_COMMENT.GetText()[0])
			{
				f << "Name-Comment: ";
				f << toUTF8(edit_KEY_COMMENT.GetText()).c_str();
				f << "\n";
			}
			f << "Name-Email: ";
			f << toUTF8(edit_KEY_EMAIL.GetText()).c_str();
			f << "\n";
			f << "Expire-Date: ";
			f << toUTF8(edit_KEY_EXPIRE_DATE.GetText()).c_str();
			f << "\n";
			f.close();
		}
		{ //gpg execution
			DWORD code;
			string out;
			std::vector<wstring> cmd;
			cmd.push_back(L"--batch");
			cmd.push_back(L"--yes");
			cmd.push_back(L"--gen-key");
			cmd.push_back(path);
			gpg_execution_params params(cmd);
			pxResult result;
			params.out = &out;
			params.code = &code;
			params.result = &result;
			lbl_GENERATING_TEXT.SendMsg(WM_SETFONT, (WPARAM)globals.bold_font, TRUE);
			lbl_GENERATING_TEXT.SetText(TranslateT("Generating new key, please wait..."));
			btn_CANCEL.Disable();
			btn_OK.Disable();
			combo_KEY_TYPE.Disable();
			edit_KEY_LENGTH.Disable();
			edit_KEY_PASSWD.Disable();
			edit_KEY_REAL_NAME.Disable();
			edit_KEY_EMAIL.Disable();
			edit_KEY_COMMENT.Disable();
			edit_KEY_EXPIRE_DATE.Disable();
			if (!gpg_launcher(params, boost::posix_time::minutes(10)))
				return;
			if (result == pxNotFound)
				return;
		}
		boost::filesystem::remove(path);
	}
	this->Close();
}
void CDlgKeyGen::onClick_CANCEL(CCtrlButton*)
{
	this->Close();
}
void CDlgKeyGen::OnDestroy()
{
	GetWindowRect(m_hwnd, &globals.key_gen_rect);
	db_set_dw(NULL, szGPGModuleName, "KeyGenWindowX", globals.key_gen_rect.left);
	db_set_dw(NULL, szGPGModuleName, "KeyGenWindowY", globals.key_gen_rect.top);
}

CDlgLoadExistingKey::CDlgLoadExistingKey() : CDlgBase(g_plugin.getInst(), IDD_LOAD_EXISTING_KEY),
btn_OK(this, IDOK), btn_CANCEL(this, IDCANCEL),
list_EXISTING_KEY_LIST(this, IDC_EXISTING_KEY_LIST)
{
	id[0] = 0;
	btn_OK.OnClick = Callback(this, &CDlgLoadExistingKey::onClick_OK);
	btn_CANCEL.OnClick = Callback(this, &CDlgLoadExistingKey::onClick_CANCEL);

}
void CDlgLoadExistingKey::OnInitDialog()
{
	SetWindowPos(m_hwnd, nullptr, globals.load_existing_key_rect.left, globals.load_existing_key_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);


	list_EXISTING_KEY_LIST.AddColumn(0, TranslateT("Key ID"), 50);
	list_EXISTING_KEY_LIST.AddColumn(1, TranslateT("Email"), 30);
	list_EXISTING_KEY_LIST.AddColumn(2, TranslateT("Name"), 250);
	list_EXISTING_KEY_LIST.AddColumn(3, TranslateT("Creation date"), 30);
	list_EXISTING_KEY_LIST.AddColumn(4, TranslateT("Expiration date"), 30);
	list_EXISTING_KEY_LIST.AddColumn(5, TranslateT("Key length"), 30);
	list_EXISTING_KEY_LIST.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_SINGLEROW);
	int i = 1;
	{
		{//parse gpg output
			string out;
			DWORD code;
			string::size_type p = 0, p2 = 0, stop = 0;
			{
				std::vector<wstring> cmd;
				cmd.push_back(L"--batch");
				cmd.push_back(L"--list-keys");
				gpg_execution_params params(cmd);
				pxResult result;
				params.out = &out;
				params.code = &code;
				params.result = &result;
				if (!gpg_launcher(params))
					return;
				if (result == pxNotFound)
					return;
			}
			while (p != string::npos)
			{
				if ((p = out.find("pub  ", p)) == string::npos)
					break;
				p += 5;
				if (p < stop)
					break;
				stop = p;
				p2 = out.find("/", p) - 1;
				wchar_t *tmp = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
				int row = list_EXISTING_KEY_LIST.AddItem(L"", 0);
				list_EXISTING_KEY_LIST.SetItemText(row, 5, tmp);
				mir_free(tmp);
				p2 += 2;
				p = out.find(" ", p2);
				tmp = mir_wstrdup(toUTF16(out.substr(p2, p - p2)).c_str());
				list_EXISTING_KEY_LIST.SetItemText(row, 0, tmp);
				mir_free(tmp);
				p++;
				p2 = out.find("\n", p);
				string::size_type p3 = out.substr(p, p2 - p).find("[");
				if (p3 != string::npos)
				{
					p3 += p;
					p2 = p3;
					p2--;
					p3++;
					p3 += mir_strlen("expires: ");
					string::size_type p4 = out.find("]", p3);
					tmp = mir_wstrdup(toUTF16(out.substr(p3, p4 - p3)).c_str());
					list_EXISTING_KEY_LIST.SetItemText(row, 4, tmp);
					mir_free(tmp);
				}
				else
					p2--;
				tmp = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
				list_EXISTING_KEY_LIST.SetItemText(row, 3, tmp);
				mir_free(tmp);
				p = out.find("uid  ", p);
				p += mir_strlen("uid ");
				p2 = out.find("\n", p);
				p3 = out.substr(p, p2 - p).find("<");
				if (p3 != string::npos)
				{
					p3 += p;
					p2 = p3;
					p2--;
					p3++;
					string::size_type p4 = out.find(">", p3);
					tmp = mir_wstrdup(toUTF16(out.substr(p3, p4 - p3)).c_str());
					list_EXISTING_KEY_LIST.SetItemText(row, 1, tmp);
					mir_free(tmp);
				}
				else
					p2--;
				p = out.find_first_not_of(" ", p);
				tmp = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
				list_EXISTING_KEY_LIST.SetItemText(row, 2, tmp);
				mir_free(tmp);
				//					p = out.find("sub  ", p2) + 6;
				//					p = out.find(" ", p) + 1;
				//					p2 = out.find("\n", p);
				//					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p-1)).c_str());
				//					ListView_SetItemText(hwndList, iRow, 3, tmp);
				//					mir_free(tmp);
				list_EXISTING_KEY_LIST.SetColumnWidth(0, LVSCW_AUTOSIZE);// not sure about this
				list_EXISTING_KEY_LIST.SetColumnWidth(1, LVSCW_AUTOSIZE);
				list_EXISTING_KEY_LIST.SetColumnWidth(2, LVSCW_AUTOSIZE);
				list_EXISTING_KEY_LIST.SetColumnWidth(3, LVSCW_AUTOSIZE);
				list_EXISTING_KEY_LIST.SetColumnWidth(4, LVSCW_AUTOSIZE);
				list_EXISTING_KEY_LIST.SetColumnWidth(5, LVSCW_AUTOSIZE);
				i++;
			}
		}
	}
	list_EXISTING_KEY_LIST.OnClick = Callback(this, &CDlgLoadExistingKey::onChange_EXISTING_KEY_LIST);
}
void CDlgLoadExistingKey::OnDestroy()
{
	GetWindowRect(m_hwnd, &globals.load_existing_key_rect);
	db_set_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowX", globals.load_existing_key_rect.left);
	db_set_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowY", globals.load_existing_key_rect.top);
}
void CDlgLoadExistingKey::onClick_OK(CCtrlButton*)
{
	int i = list_EXISTING_KEY_LIST.GetSelectionMark();
	if (i == -1)
		return; //TODO: error message

	list_EXISTING_KEY_LIST.GetItemText(i, 0, id, _countof(id));
	extern CCtrlEdit *edit_p_PubKeyEdit;
	string out;
	DWORD code;
	std::vector<wstring> cmd;
	cmd.push_back(L"--batch");
	cmd.push_back(L"-a");
	cmd.push_back(L"--export");
	cmd.push_back(id);
	gpg_execution_params params(cmd);
	pxResult result;
	params.out = &out;
	params.code = &code;
	params.result = &result;
	if (!gpg_launcher(params))
		return;
	if (result == pxNotFound)
		return;
	string::size_type s = 0;
	while ((s = out.find("\r", s)) != string::npos) {
		out.erase(s, 1);
	}
	std::string::size_type p1 = 0, p2 = 0;
	p1 = out.find("-----BEGIN PGP PUBLIC KEY BLOCK-----");
	if (p1 != std::string::npos) {
		p2 = out.find("-----END PGP PUBLIC KEY BLOCK-----", p1);
		if (p2 != std::string::npos) {
			p2 += mir_strlen("-----END PGP PUBLIC KEY BLOCK-----");
			out = out.substr(p1, p2 - p1);
			wchar_t *tmp = mir_a2u(out.c_str());
			if (edit_p_PubKeyEdit)
				edit_p_PubKeyEdit->SetText(tmp);
			mir_free(tmp);
		}
		else
			MessageBox(nullptr, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);
	}
	else
		MessageBox(nullptr, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);
	//			  SetDlgItemText(hPubKeyEdit, IDC_PUBLIC_KEY_EDIT, tmp);
	this->Close();
}
void CDlgLoadExistingKey::onClick_CANCEL(CCtrlButton*)
{
	this->Close();
}

void CDlgLoadExistingKey::onChange_EXISTING_KEY_LIST(CCtrlListView::TEventInfo * /*ev*/) //TODO: check if this work
{
	if (list_EXISTING_KEY_LIST.GetSelectionMark() != -1)
		btn_OK.Enable();
}

CDlgImportKey::CDlgImportKey(MCONTACT _hContact) : CDlgBase(g_plugin.getInst(), IDD_IMPORT_KEY),
combo_KEYSERVER(this, IDC_KEYSERVER),
btn_IMPORT(this, IDC_IMPORT)
{
	hContact = _hContact;
	btn_IMPORT.OnClick = Callback(this, &CDlgImportKey::onClick_IMPORT);
}
void CDlgImportKey::OnInitDialog()
{
	SetWindowPos(m_hwnd, nullptr, globals.import_key_rect.left, globals.import_key_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	combo_KEYSERVER.AddString(L"subkeys.pgp.net");
	combo_KEYSERVER.AddString(L"keys.gnupg.net");
}
void CDlgImportKey::OnDestroy()
{
	GetWindowRect(m_hwnd, &globals.import_key_rect);
	db_set_dw(NULL, szGPGModuleName, "ImportKeyWindowX", globals.import_key_rect.left);
	db_set_dw(NULL, szGPGModuleName, "ImportKeyWindowY", globals.import_key_rect.top);
}
void CDlgImportKey::onClick_IMPORT(CCtrlButton*)
{
	string out;
	DWORD code;
	std::vector<wstring> cmd;
	cmd.push_back(L"--keyserver");
	cmd.push_back(combo_KEYSERVER.GetText());
	cmd.push_back(L"--recv-keys");
	cmd.push_back(toUTF16(globals.hcontact_data[hContact].key_in_prescense));
	gpg_execution_params params(cmd);
	pxResult result;
	params.out = &out;
	params.code = &code;
	params.result = &result;
	gpg_launcher(params);
	MessageBoxA(nullptr, out.c_str(), "GPG output", MB_OK);
}



CDlgKeyPasswordMsgBox::CDlgKeyPasswordMsgBox(MCONTACT _hContact) : CDlgBase(g_plugin.getInst(), IDD_KEY_PASSWD),
lbl_KEYID(this, IDC_KEYID),
edit_KEY_PASSWORD(this, IDC_KEY_PASSWORD),
chk_DEFAULT_PASSWORD(this, IDC_DEFAULT_PASSWORD), chk_SAVE_PASSWORD(this, IDC_SAVE_PASSWORD),
btn_OK(this, IDOK), btn_CANCEL(this, IDCANCEL)
{
	hContact = _hContact;
	btn_OK.OnClick = Callback(this, &CDlgKeyPasswordMsgBox::onClick_OK);
	btn_CANCEL.OnClick = Callback(this, &CDlgKeyPasswordMsgBox::onClick_CANCEL);
}
void CDlgKeyPasswordMsgBox::OnInitDialog()
{
	inkeyid = UniGetContactSettingUtf(hContact, szGPGModuleName, "InKeyID", "");

	SetWindowPos(m_hwnd, nullptr, globals.key_password_rect.left, globals.key_password_rect.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
	{
		string questionstr = "Please enter password for key with ID: ";
		questionstr += inkeyid;
		mir_free(inkeyid);
		lbl_KEYID.SetTextA(questionstr.c_str());
		chk_DEFAULT_PASSWORD.Disable();
	}
}
void CDlgKeyPasswordMsgBox::OnDestroy()
{
	mir_free(inkeyid);
	GetWindowRect(m_hwnd, &globals.key_password_rect);
	db_set_dw(NULL, szGPGModuleName, "PasswordWindowX", globals.key_password_rect.left);
	db_set_dw(NULL, szGPGModuleName, "PasswordWindowY", globals.key_password_rect.top);
}
void CDlgKeyPasswordMsgBox::onClick_OK(CCtrlButton*)
{
	wchar_t *tmp = mir_wstrdup(edit_KEY_PASSWORD.GetText());
	if (tmp && tmp[0]) {
		if (chk_SAVE_PASSWORD.GetState()) {
			inkeyid = UniGetContactSettingUtf(hContact, szGPGModuleName, "InKeyID", "");
			if (inkeyid && inkeyid[0] && !chk_DEFAULT_PASSWORD.GetState()) {
				string dbsetting = "szKey_";
				dbsetting += inkeyid;
				dbsetting += "_Password";
				db_set_ws(NULL, szGPGModuleName, dbsetting.c_str(), tmp);
			}
			else
				db_set_ws(NULL, szGPGModuleName, "szKeyPassword", tmp);
		}
		if (globals.password)
			mir_free(globals.password);
		globals.password = (wchar_t*)mir_alloc(sizeof(wchar_t)*(mir_wstrlen(tmp) + 1));
		mir_wstrcpy(globals.password, tmp);
	}
	mir_free(inkeyid);
	DestroyWindow(m_hwnd);
}
void CDlgKeyPasswordMsgBox::onClick_CANCEL(CCtrlButton*)
{
	globals._terminate = true;
	DestroyWindow(m_hwnd);
}

