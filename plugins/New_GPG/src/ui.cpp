// Copyright (c) 2017-22 sss
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

/////////////////////////////////////////////////////////////////////////////////////////

bool CDlgEncryptedFileMsgBox::OnInitDialog()
{
	globals.bDecryptFiles = false;
	return true;
}

CDlgEncryptedFileMsgBox::CDlgEncryptedFileMsgBox() :
	CDlgBase(g_plugin, IDD_ENCRYPTED_FILE_MSG_BOX),
	chk_REMEMBER(this, IDC_REMEMBER),
	btn_IGNORE(this, IDC_IGNORE),
	btn_DECRYPT(this, IDC_DECRYPT)
{
	btn_IGNORE.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_IGNORE);
	btn_DECRYPT.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_DECRYPT);
}

void CDlgEncryptedFileMsgBox::onClick_IGNORE(CCtrlButton*)
{
	if (chk_REMEMBER.GetState())
		g_plugin.bSameAction = true;

	this->Close();
}

void CDlgEncryptedFileMsgBox::onClick_DECRYPT(CCtrlButton*)
{
	globals.bDecryptFiles = true;
	if (chk_REMEMBER.GetState()) {
		g_plugin.bFileTransfers = true;
		g_plugin.bSameAction = false;
	}
	this->Close();
}

/////////////////////////////////////////////////////////////////////////////////////////

class CDlgExportKeysMsgBox : public CDlgBase
{
	CCtrlCheck chk_PUBLIC, chk_PRIVATE, chk_ALL;

public:
	CDlgExportKeysMsgBox() :
		CDlgBase(g_plugin, IDD_EXPORT_TYPE),
		chk_PUBLIC(this, IDC_PUBLIC),
		chk_PRIVATE(this, IDC_PRIVATE),
		chk_ALL(this, IDC_ALL)
	{
	}

	bool OnInitDialog() override
	{
		chk_PUBLIC.SetState(true);
		return true;
	}

	bool OnApply() override
	{
		if (chk_PUBLIC.GetState())
			ExportGpGKeysFunc(0);
		else if (chk_PRIVATE.GetState())
			ExportGpGKeysFunc(1);
		else if (chk_ALL.GetState())
			ExportGpGKeysFunc(2);
		return true;
	}
};

INT_PTR ExportGpGKeys(WPARAM, LPARAM)
{
	(new CDlgExportKeysMsgBox())->Show();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class CDlgChangePasswdMsgBox : public CDlgBase //always modal
{
	CCtrlEdit edit_NEW_PASSWD1, edit_NEW_PASSWD2, edit_OLD_PASSWD;

public:
	CDlgChangePasswdMsgBox() :
		CDlgBase(g_plugin, IDD_CHANGE_PASSWD),
		edit_NEW_PASSWD1(this, IDC_NEW_PASSWD1),
		edit_NEW_PASSWD2(this, IDC_NEW_PASSWD2),
		edit_OLD_PASSWD(this, IDC_OLD_PASSWD)
	{
	}

	bool OnApply() override
	{
		//TODO: show some prgress
		if (mir_wstrcmp(edit_NEW_PASSWD1.GetText(), edit_NEW_PASSWD2.GetText())) {
			MessageBox(m_hwnd, TranslateT("New passwords do not match"), TranslateT("Error"), MB_OK);
			return false;
		}

		std::string old_pass, new_pass;
		new_pass = toUTF8(ptrW(edit_NEW_PASSWD1.GetText()).get());
		old_pass = toUTF8(ptrW(edit_OLD_PASSWD.GetText()).get());

		bool old_pass_match = false;
		if (!mir_strcmp(ptrA(g_plugin.getUStringA("szKeyPassword")), old_pass.c_str()))
			old_pass_match = true;

		if (!old_pass_match) {
			if (globals.key_id_global[0]) {
				string dbsetting = "szKey_";
				dbsetting += toUTF8(globals.key_id_global);
				dbsetting += "_Password";
				ptrA pass(g_plugin.getUStringA(dbsetting.c_str()));
				if (!mir_strcmp(pass, old_pass.c_str()))
					old_pass_match = true;
			}
		}

		if (!old_pass_match)
			if (MessageBox(m_hwnd, TranslateT("Old password does not match, you can continue, but GPG will reject wrong password.\nDo you want to continue?"), TranslateT("Error"), MB_YESNO) == IDNO)
				return false;

		gpg_execution_params_pass params(old_pass, new_pass);
		params.addParam(L"--edit-key");
		params.addParam(globals.key_id_global);
		params.addParam(L"passwd");

		HANDLE hThread = mir_forkThread<gpg_execution_params_pass>(&pxEexcute_passwd_change_thread, &params);
		if (WaitForSingleObject(hThread, 600000) != WAIT_OBJECT_0) {
			if (params.child)
				params.child->terminate();
			if (globals.debuglog)
				globals.debuglog << "GPG execution timed out, aborted";
			return true;
		}

		return params.result != pxNotFound;
	}
};

void ShowChangePasswdDlg()
{
	CDlgChangePasswdMsgBox *d = new CDlgChangePasswdMsgBox;
	d->DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////
// New key generation dialog

class CDlgKeyGen : public CDlgBase
{
	CCtrlCombo combo_KEY_TYPE;
	CCtrlEdit edit_KEY_LENGTH, edit_KEY_PASSWD, edit_KEY_REAL_NAME, edit_KEY_EMAIL, edit_KEY_COMMENT, edit_KEY_EXPIRE_DATE;
	CCtrlData lbl_GENERATING_TEXT;

public:
	CDlgKeyGen() :
		CDlgBase(g_plugin, IDD_KEY_GEN),
		combo_KEY_TYPE(this, IDC_KEY_TYPE),
		edit_KEY_LENGTH(this, IDC_KEY_LENGTH),
		edit_KEY_PASSWD(this, IDC_KEY_PASSWD),
		edit_KEY_REAL_NAME(this, IDC_KEY_REAL_NAME),
		edit_KEY_EMAIL(this, IDC_KEY_EMAIL),
		edit_KEY_COMMENT(this, IDC_KEY_COMMENT),
		edit_KEY_EXPIRE_DATE(this, IDC_KEY_EXPIRE_DATE),
		lbl_GENERATING_TEXT(this, IDC_GENERATING_TEXT)
	{
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "KeygenWindow");
		SetCaption(TranslateT("Key Generation dialog"));

		combo_KEY_TYPE.AddString(L"RSA");
		combo_KEY_TYPE.AddString(L"DSA");
		combo_KEY_TYPE.SelectString(L"RSA");
		edit_KEY_EXPIRE_DATE.SetText(L"0");
		edit_KEY_LENGTH.SetText(L"4096");
		return true;
	}

	bool OnApply() override
	{
		// data sanity checks
		ptrW tmp(combo_KEY_TYPE.GetText());
		if (mir_wstrlen(tmp) < 3) {
			MessageBox(nullptr, TranslateT("You must set encryption algorithm first"), TranslateT("Error"), MB_OK);
			return false;
		}

		tmp = edit_KEY_LENGTH.GetText();
		int length = _wtoi(tmp);
		if (length < 1024 || length > 4096) {
			MessageBox(nullptr, TranslateT("Key length must be of length from 1024 to 4096 bits"), TranslateT("Error"), MB_OK);
			return false;
		}

		tmp = edit_KEY_EXPIRE_DATE.GetText();
		if (mir_wstrlen(tmp) != 10 && tmp[0] != '0') {
			MessageBox(nullptr, TranslateT("Invalid date"), TranslateT("Error"), MB_OK);
			return false;
		}

		tmp = edit_KEY_REAL_NAME.GetText();
		if (mir_wstrlen(tmp) < 4) {
			MessageBox(nullptr, TranslateT("Name must contain at least 4 characters"), TranslateT("Error"), MB_OK);
			return false;
		}
		if (wcschr(tmp, '(') || wcschr(tmp, ')')) {
			MessageBox(nullptr, TranslateT("Name cannot contain '(' or ')'"), TranslateT("Error"), MB_OK);
			return false;
		}

		tmp = edit_KEY_EMAIL.GetText();
		if (mir_wstrlen(tmp) < 5 || !wcschr(tmp, '@')) {
			MessageBox(nullptr, TranslateT("Invalid Email"), TranslateT("Error"), MB_OK);
			return false;
		}

		// generating key file
		CMStringW path = g_plugin.getMStringW("szHomePath");
		path += L"\\new_key";
		wfstream f(path.c_str(), std::ios::out);
		if (!f.is_open()) {
			MessageBox(nullptr, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
			return false;
		}

		f << "Key-Type: ";
		char *tmp2 = mir_u2a(combo_KEY_TYPE.GetText());
		char *subkeytype = (char *)mir_alloc(6);
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
		if (edit_KEY_PASSWD.GetText()[0]) {
			f << "Passphrase: ";
			f << toUTF8(edit_KEY_PASSWD.GetText()).c_str();
			f << "\n";
		}
		f << "Name-Real: ";
		f << toUTF8(edit_KEY_REAL_NAME.GetText()).c_str();
		f << "\n";
		if (edit_KEY_COMMENT.GetText()[0]) {
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

		lbl_GENERATING_TEXT.SendMsg(WM_SETFONT, (WPARAM)globals.bold_font, TRUE);
		lbl_GENERATING_TEXT.SetText(TranslateT("Generating new key, please wait..."));
		combo_KEY_TYPE.Disable();
		edit_KEY_LENGTH.Disable();
		edit_KEY_PASSWD.Disable();
		edit_KEY_REAL_NAME.Disable();
		edit_KEY_EMAIL.Disable();
		edit_KEY_COMMENT.Disable();
		edit_KEY_EXPIRE_DATE.Disable();

		// gpg execution
		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"--yes");
		params.addParam(L"--gen-key");
		params.addParam(path.c_str());
		params.bNoOutput = true;
		if (!gpg_launcher(params, boost::posix_time::minutes(10)))
			return false;
		if (params.result == pxNotFound)
			return false;

		boost::filesystem::remove(path.c_str());
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "KeygenWindow");
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// First run dialog

class CDlgFirstRun : public CDlgBase
{
	void refresh_key_list()
	{
		list_KEY_LIST.DeleteAllItems();
		int i = 1;

		// parse gpg output
		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"--list-secret-keys");
		if (!gpg_launcher(params))
			return;
		if (params.result == pxNotFound)
			return;

		wstring::size_type p = 0, p2 = 0, stop = 0;
		string out(params.out);
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
			list_KEY_LIST.SetItemText(row, 0, (wchar_t *)key_id.c_str());
			p = out.find("uid  ", p);
			p2 = out.find_first_not_of(" ", p + 5);
			p = out.find("<", p2);

			wstring tmp = toUTF16(out.substr(p2, p - p2));
			list_KEY_LIST.SetItemText(row, 2, (wchar_t *)tmp.c_str());

			p++;
			p2 = out.find(">", p);

			tmp = toUTF16(out.substr(p, p2 - p));
			list_KEY_LIST.SetItemText(row, 1, (wchar_t *)tmp.c_str());

			// get accounts
			std::wstring accs;
			for (auto &pa : Accounts()) {
				std::string setting = pa->szModuleName;
				setting += "_KeyID";
				ptrW str(g_plugin.getWStringA(setting.c_str(), L""));
				if (key_id == str.get()) {
					if (!accs.empty())
						accs += L",";
					accs += pa->tszAccountName;
				}
			}
			list_KEY_LIST.SetItemText(row, 6, accs.c_str());
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

	CCtrlListView list_KEY_LIST;
	CCtrlButton btn_COPY_PUBKEY, btn_EXPORT_PRIVATE, btn_CHANGE_PASSWD, btn_GENERATE_RANDOM, btn_GENERATE_KEY, btn_OTHER, btn_DELETE_KEY, btn_OK;
	CCtrlEdit edit_KEY_PASSWORD;
	CCtrlCombo combo_ACCOUNT;
	CCtrlData lbl_KEY_ID, lbl_GENERATING_KEY;
	wchar_t fp[16];
	const char *m_szCurrAcc = nullptr;

public:
	CDlgFirstRun() :
		CDlgBase(g_plugin, IDD_FIRST_RUN),
		list_KEY_LIST(this, IDC_KEY_LIST),
		btn_COPY_PUBKEY(this, IDC_COPY_PUBKEY),
		btn_EXPORT_PRIVATE(this, IDC_EXPORT_PRIVATE),
		btn_CHANGE_PASSWD(this, IDC_CHANGE_PASSWD),
		btn_GENERATE_RANDOM(this, IDC_GENERATE_RANDOM),
		btn_GENERATE_KEY(this, IDC_GENERATE_KEY),
		btn_OTHER(this, IDC_OTHER),
		btn_DELETE_KEY(this, IDC_DELETE_KEY),
		btn_OK(this, ID_OK),
		edit_KEY_PASSWORD(this, IDC_KEY_PASSWORD),
		combo_ACCOUNT(this, IDC_ACCOUNT),
		lbl_KEY_ID(this, IDC_KEY_ID),
		lbl_GENERATING_KEY(this, IDC_GENERATING_KEY)
	{
		fp[0] = 0;

		btn_COPY_PUBKEY.OnClick = Callback(this, &CDlgFirstRun::onClick_COPY_PUBKEY);
		btn_EXPORT_PRIVATE.OnClick = Callback(this, &CDlgFirstRun::onClick_EXPORT_PRIVATE);
		btn_CHANGE_PASSWD.OnClick = Callback(this, &CDlgFirstRun::onClick_CHANGE_PASSWD);
		btn_GENERATE_RANDOM.OnClick = Callback(this, &CDlgFirstRun::onClick_GENERATE_RANDOM);
		btn_GENERATE_KEY.OnClick = Callback(this, &CDlgFirstRun::onClick_GENERATE_KEY);
		btn_OTHER.OnClick = Callback(this, &CDlgFirstRun::onClick_OTHER);
		btn_DELETE_KEY.OnClick = Callback(this, &CDlgFirstRun::onClick_DELETE_KEY);
		btn_OK.OnClick = Callback(this, &CDlgFirstRun::onClick_OK);

		combo_ACCOUNT.OnChange = Callback(this, &CDlgFirstRun::onChange_ACCOUNT);

		list_KEY_LIST.OnClick = Callback(this, &CDlgFirstRun::onChange_KEY_LIST);
	}

	bool OnInitDialog() override
	{
		Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "FirstrunWindow");
		SetCaption(TranslateT("Bind own keys to accounts"));
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

			combo_ACCOUNT.AddString(pa->tszAccountName, (LPARAM)pa->szModuleName);
		}
		combo_ACCOUNT.SetCurSel(0);

		lbl_KEY_ID.SetText(CMStringW(FORMAT, L"%s: %s", TranslateT("key ID"), ptrW(g_plugin.getWStringA("KeyID", TranslateT("not set"))).get()));
		return true;
	}

	void OnDestroy() override
	{
		Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "FirstrunWindow");
	}

	void onClick_COPY_PUBKEY(CCtrlButton *)
	{
		int i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));

		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"-a");
		params.addParam(L"--export");
		params.addParam(fp);
		if (!gpg_launcher(params))
			return;
		if (params.result == pxNotFound)
			return;

		params.out.Remove('\r');
		Utils_ClipboardCopy(params.out);
	}

	void onClick_EXPORT_PRIVATE(CCtrlButton *)
	{
		int i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		ptrW p(GetFilePath(L"Choose file to export key", L"*", L"Any file", true));
		if (!p || !p[0])
			return;

		std::ofstream file;
		file.open(p, std::ios::trunc | std::ios::out);
		if (!file.is_open())
			return; //TODO: handle error

		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));

		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"-a");
		params.addParam(L"--export-secret-keys");
		params.addParam(fp);
		if (!gpg_launcher(params))
			return;
		if (params.result == pxNotFound)
			return;

		params.out.Remove('\r');
		file << params.out.c_str();
		if (file.is_open())
			file.close();
	}

	void onClick_CHANGE_PASSWD(CCtrlButton *)
	{
		int i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		list_KEY_LIST.GetItemText(i, 0, globals.key_id_global, _countof(globals.key_id_global));

		// temporary code follows
		std::string old_pass, new_pass;

		gpg_execution_params_pass params(old_pass, new_pass);
		params.addParam(L"--edit-key");
		params.addParam(globals.key_id_global);
		params.addParam(L"passwd");

		HANDLE hThread = mir_forkThread<gpg_execution_params_pass>(pxEexcute_passwd_change_thread, &params);
		if (WaitForSingleObject(hThread, 600000) != WAIT_OBJECT_0) {
			if (params.child)
				params.child->terminate();
			if (globals.debuglog)
				globals.debuglog << "GPG execution timed out, aborted";
			this->Close();
		}
	}

	void onClick_GENERATE_RANDOM(CCtrlButton *)
	{
		lbl_GENERATING_KEY.SendMsg(WM_SETFONT, (WPARAM)globals.bold_font, TRUE);
		lbl_GENERATING_KEY.SetText(TranslateT("Generating new random key, please wait"));
		btn_GENERATE_KEY.Disable();
		btn_OTHER.Disable();
		btn_DELETE_KEY.Disable();
		list_KEY_LIST.Disable();
		btn_GENERATE_RANDOM.Disable();
		gpg_use_new_random_key(m_szCurrAcc);
		this->Close();
	}

	void onClick_GENERATE_KEY(CCtrlButton *)
	{
		CDlgKeyGen().DoModal();
		refresh_key_list();
	}

	void onClick_OTHER(CCtrlButton *)
	{
		ShowLoadPublicKeyDialog(0, true);
		refresh_key_list();
	}

	void onClick_DELETE_KEY(CCtrlButton *)
	{
		int  i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
		{
			gpg_execution_params params;
			params.addParam(L"--batch");
			params.addParam(L"--fingerprint");
			params.addParam(fp);
			if (!gpg_launcher(params))
				return;
			if (params.result == pxNotFound)
				return;

			int s = params.out.Find("Key fingerprint = ");
			s += mir_strlen("Key fingerprint = ");
			int s2 = params.out.Find("\n", s);

			CMStringW tmp = params.out.Mid(s, s2 - s);
			tmp.Remove(' ');

			gpg_execution_params params2;
			params2.addParam(L"--batch");
			params2.addParam(L"--delete-secret-and-public-key");
			params2.addParam(L"--fingerprint");
			params2.addParam(tmp.c_str());

			if (!gpg_launcher(params2))
				return;
			if (params2.result == pxNotFound)
				return;
		}

		if (m_szCurrAcc == nullptr) {
			g_plugin.delSetting("GPGPubKey");
			g_plugin.delSetting("KeyID");
			g_plugin.delSetting("KeyComment");
			g_plugin.delSetting("KeyMainName");
			g_plugin.delSetting("KeyMainEmail");
			g_plugin.delSetting("KeyType");
		}
		else {
			CMStringA acc_str = m_szCurrAcc;
			g_plugin.delSetting(acc_str + "_GPGPubKey");
			g_plugin.delSetting(acc_str + "_KeyMainName");
			g_plugin.delSetting(acc_str + "_KeyID");
			g_plugin.delSetting(acc_str + "_KeyComment");
			g_plugin.delSetting(acc_str + "_KeyMainEmail");
			g_plugin.delSetting(acc_str + "_KeyType");
		}

		list_KEY_LIST.DeleteItem(i);
	}

	void onClick_OK(CCtrlButton *)
	{
		int  i = list_KEY_LIST.GetSelectionMark();
		if (i == -1)
			return;

		list_KEY_LIST.GetItemText(i, 0, fp, _countof(fp));
		wchar_t name[65];
		list_KEY_LIST.GetItemText(i, 2, name, 64);
		{
			if (wcschr(name, '(')) {
				wstring str = name;
				wstring::size_type p = str.find(L"(") - 1;
				mir_wstrcpy(name, str.substr(0, p).c_str());
			}
		}

		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"-a");
		params.addParam(L"--export");
		params.addParam(fp);
		if (!gpg_launcher(params))
			return;
		if (params.result == pxNotFound)
			return;

		params.out.Remove('\r');

		if (m_szCurrAcc == nullptr) {
			g_plugin.setString("GPGPubKey", params.out.c_str());
			g_plugin.setWString("KeyMainName", name);
			g_plugin.setWString("KeyID", fp);

			wstring keyinfo = TranslateT("Default private key ID");
			keyinfo += L": ";
			keyinfo += (fp[0]) ? fp : L"not set";
			extern HWND hwndCurKey_p;
			SetWindowText(hwndCurKey_p, keyinfo.c_str());
		}
		else {
			CMStringA acc_str = m_szCurrAcc;
			g_plugin.setString(acc_str + "_GPGPubKey", params.out.c_str());
			g_plugin.setWString(acc_str + "_KeyMainName", name);
			g_plugin.setWString(acc_str + "_KeyID", fp);
		}

		ptrW passwd(edit_KEY_PASSWORD.GetText());
		if (mir_wstrlen(passwd)) {
			string dbsetting = "szKey_";
			dbsetting += _T2A(fp);
			dbsetting += "_Password";
			g_plugin.setWString(dbsetting.c_str(), passwd);
		}

		//bAutoExchange = CheckStateStoreDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange") != 0; //TODO: check is it just typo, or doing something
		globals.gpg_valid = isGPGValid();
		globals.gpg_keyexist = isGPGKeyExist();
		DestroyWindow(m_hwnd);
	}

	void onChange_ACCOUNT(CCtrlCombo *pCombo)
	{
		CMStringW keyinfo = TranslateT("key ID");
		keyinfo += ": ";

		m_szCurrAcc = (const char *)pCombo->GetCurData();
		if (m_szCurrAcc == nullptr) {
			keyinfo += g_plugin.getMStringW("KeyID", TranslateT("not set"));
		}
		else {
			std::string acc_str = m_szCurrAcc;
			acc_str += "_KeyID";
			keyinfo += g_plugin.getMStringW(acc_str.c_str(), TranslateT("not set"));
		}
		lbl_KEY_ID.SetText(keyinfo);
	}

	void onChange_KEY_LIST(CCtrlListView::TEventInfo *ev)
	{
		if (ev->nmlv) {
			NMLISTVIEW *hdr = ev->nmlv;

			if (hdr->hdr.code == NM_CLICK) {
				btn_OK.Enable();
				btn_COPY_PUBKEY.Enable();
				btn_EXPORT_PRIVATE.Enable();
				btn_CHANGE_PASSWD.Enable();
			}
		}
	}
};

void ShowFirstRunDialog()
{
	CDlgFirstRun().DoModal();
}

/////////////////////////////////////////////////////////////////////////////////////////

CDlgNewKey::CDlgNewKey(MCONTACT _hContact, wstring _new_key) :
	CDlgBase(g_plugin, IDD_NEW_KEY),
	lbl_KEY_FROM(this, IDC_KEY_FROM),
	lbl_MESSAGE(this, IDC_MESSAGE),
	btn_IMPORT(this, ID_IMPORT),
	btn_IMPORT_AND_USE(this, IDC_IMPORT_AND_USE),
	btn_IGNORE_KEY(this, IDC_IGNORE_KEY)
{
	hContact = _hContact;
	new_key = _new_key;
	btn_IMPORT.OnClick = Callback(this, &CDlgNewKey::onClick_IMPORT);
	btn_IMPORT_AND_USE.OnClick = Callback(this, &CDlgNewKey::onClick_IMPORT_AND_USE);
	btn_IGNORE_KEY.OnClick = Callback(this, &CDlgNewKey::onClick_IGNORE_KEY);
}

bool CDlgNewKey::OnInitDialog()
{
	Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "NewKeyWindow");

	CMStringW tmp = g_plugin.getMStringW(hContact, "GPGPubKey");
	lbl_MESSAGE.SetText(!tmp.IsEmpty() ? TranslateT("There is existing key for contact, would you like to replace it with new key?") : TranslateT("New public key was received, do you want to import it?"));
	btn_IMPORT_AND_USE.Enable(g_plugin.getByte(hContact, "GPGEncryption", 0));
	btn_IMPORT.SetText(!tmp.IsEmpty() ? TranslateT("Replace") : TranslateT("Accept"));

	tmp.Format(TranslateT("Received key from %s"), Clist_GetContactDisplayName(hContact));
	lbl_KEY_FROM.SetText(tmp);
	return true;
}

void CDlgNewKey::OnDestroy()
{
	Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "NewKeyWindow");
}

void CDlgNewKey::onClick_IMPORT(CCtrlButton*)
{
	ImportKey(hContact, new_key);
	this->Close();
}

void CDlgNewKey::onClick_IMPORT_AND_USE(CCtrlButton*)
{
	ImportKey(hContact, new_key);
	g_plugin.setByte(hContact, "GPGEncryption", 1);
	setSrmmIcon(hContact);
	this->Close();
}

void CDlgNewKey::onClick_IGNORE_KEY(CCtrlButton*)
{
	this->Close();
}

/////////////////////////////////////////////////////////////////////////////////////////

CDlgKeyPasswordMsgBox::CDlgKeyPasswordMsgBox(MCONTACT _hContact) :
	CDlgBase(g_plugin, IDD_KEY_PASSWD),
	lbl_KEYID(this, IDC_KEYID),
	edit_KEY_PASSWORD(this, IDC_KEY_PASSWORD),
	chk_DEFAULT_PASSWORD(this, IDC_DEFAULT_PASSWORD),
	chk_SAVE_PASSWORD(this, IDC_SAVE_PASSWORD)
{
	hContact = _hContact;
}

bool CDlgKeyPasswordMsgBox::OnInitDialog()
{
	Utils_RestoreWindowPosition(m_hwnd, 0, MODULENAME, "PasswordWindow");

	CMStringW questionstr = TranslateT("Please enter password for key with ID: ");
	questionstr += g_plugin.getMStringW(hContact, "InKeyID");
	lbl_KEYID.SetText(questionstr.c_str());

	chk_DEFAULT_PASSWORD.Disable();
	return true;
}

bool CDlgKeyPasswordMsgBox::OnApply()
{
	ptrW tmp(edit_KEY_PASSWORD.GetText());
	if (tmp && tmp[0]) {
		if (chk_SAVE_PASSWORD.GetState()) {
			inkeyid = g_plugin.getStringA(hContact, "InKeyID", "");
			if (inkeyid && inkeyid[0] && !chk_DEFAULT_PASSWORD.GetState()) {
				string dbsetting = "szKey_";
				dbsetting += inkeyid;
				dbsetting += "_Password";
				g_plugin.setWString(dbsetting.c_str(), tmp);
			}
			else g_plugin.setWString("szKeyPassword", tmp);
		}
		globals.wszPassword = tmp;
	}
	mir_free(inkeyid);
	return true;
}

void CDlgKeyPasswordMsgBox::OnDestroy()
{
	if (!m_bSucceeded)
		globals._terminate = true;

	mir_free(inkeyid);
	Utils_SaveWindowPosition(m_hwnd, 0, MODULENAME, "PasswordWindow");
}
