// Copyright © 2010-2012 sss
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

#pragma comment(lib, "shlwapi.lib")


void FirstRun()
{
	if (!db_get_b(NULL, szGPGModuleName, "FirstRun", 1))
		return;
	CDlgGpgBinOpts *d = new CDlgGpgBinOpts;
	d->Show();
}

void InitCheck()
{
	{
		// parse gpg output
		wchar_t *current_home = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
		db_set_ws(NULL, szGPGModuleName, "szHomePath", L""); //we do not need home for gpg binary validation
		globals.gpg_valid = isGPGValid();
		db_set_ws(NULL, szGPGModuleName, "szHomePath", current_home); //return current home dir back
		mir_free(current_home);
		bool home_dir_access = false, temp_access = false;
		wchar_t *home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
		std::wstring test_path = home_dir;
		mir_free(home_dir);
		test_path += L"/";
		test_path += toUTF16(get_random(13));
		wfstream test_file;
		test_file.open(test_path, std::ios::trunc | std::ios::out);
		if (test_file.is_open() && test_file.good()) {
			test_file << L"access_test\n";
			if (test_file.good())
				home_dir_access = true;
			test_file.close();
			boost::filesystem::remove(test_path);
		}
		home_dir = _tgetenv(L"TEMP");
		test_path = home_dir;
		test_path += L"/";
		test_path += toUTF16(get_random(13));
		test_file.open(test_path, std::ios::trunc | std::ios::out);
		if (test_file.is_open() && test_file.good()) {
			test_file << L"access_test\n";
			if (test_file.good())
				temp_access = true;
			test_file.close();
			boost::filesystem::remove(test_path);
		}
		if (!home_dir_access || !temp_access || !globals.gpg_valid) {
			wchar_t buf[4096];
			wcsncpy(buf, globals.gpg_valid ? TranslateT("GPG binary is set and valid (this is good).\n") : TranslateT("GPG binary unset or invalid (plugin will not work).\n"), _countof(buf));
			mir_wstrncat(buf, home_dir_access ? TranslateT("Home dir write access granted (this is good).\n") : TranslateT("Home dir has no write access (plugin most probably will not work).\n"), _countof(buf) - mir_wstrlen(buf));
			mir_wstrncat(buf, temp_access ? TranslateT("Temp dir write access granted (this is good).\n") : TranslateT("Temp dir has no write access (plugin should work, but may have some problems, file transfers will not work)."), _countof(buf) - mir_wstrlen(buf));
			if (!globals.gpg_valid)
				mir_wstrncat(buf, TranslateT("\nGPG will be disabled until you solve these problems"), _countof(buf) - mir_wstrlen(buf));
			MessageBox(nullptr, buf, TranslateT("GPG plugin problems"), MB_OK);
		}
		if (!globals.gpg_valid)
			return;
		globals.gpg_keyexist = isGPGKeyExist();
		string out;
		DWORD code;
		pxResult result;
		wstring::size_type p = 0, p2 = 0;
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
		home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
		wstring tmp_dir = home_dir;
		mir_free(home_dir);
		tmp_dir += L"\\tmp";
		_wmkdir(tmp_dir.c_str());
		int count = 0;
		PROTOACCOUNT **accounts;
		Proto_EnumAccounts(&count, &accounts);
		string question;
		char *keyid = nullptr;
		for (int i = 0; i < count; i++) {
			if (StriStr(accounts[i]->szModuleName, "metacontacts"))
				continue;
			if (StriStr(accounts[i]->szModuleName, "weather"))
				continue;
			std::string acc = toUTF8(accounts[i]->tszAccountName);
			acc += "(";
			acc += accounts[i]->szModuleName;
			acc += ")";
			acc += "_KeyID";
			keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, acc.c_str(), "");
			if (keyid[0]) {
				question = Translate("Your secret key with ID: ");
				mir_free(keyid);
				keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
				if ((p = out.find(keyid)) == string::npos) {
					question += keyid;
					question += Translate(" for account ");
					question += toUTF8(accounts[i]->tszAccountName);
					question += Translate(" deleted from GPG secret keyring.\nDo you want to set another key?");
					if (MessageBoxA(nullptr, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
					{
						CDlgFirstRun *d = new CDlgFirstRun;
						d->DoModal();
					}
				}
				p2 = p;
				p = out.find("[", p);
				p2 = out.find("\n", p2);
				if ((p != std::string::npos) && (p < p2)) {
					p = out.find("expires:", p);
					p += mir_strlen("expires:");
					p++;
					p2 = out.find("]", p);
					wchar_t *expire_date = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
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
						question += keyid;
						question += Translate(" for account ");
						question += toUTF8(accounts[i]->tszAccountName);
						question += Translate(" expired and will not work.\nDo you want to set another key?");
						if (MessageBoxA(nullptr, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
						{
							CDlgFirstRun *d = new CDlgFirstRun;
							d->DoModal();
						}
					}
					mir_free(expire_date);
				}
			}
			if (keyid) {
				mir_free(keyid);
				keyid = nullptr;
			}
		}
		question = Translate("Your secret key with ID: ");
		keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
		char *key = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
		if (!db_get_b(NULL, szGPGModuleName, "FirstRun", 1) && (!keyid[0] || !key[0])) {
			question = Translate("You didn't set a private key.\nWould you like to set it now?");
			if (MessageBoxA(nullptr, question.c_str(), Translate("Own private key warning"), MB_YESNO) == IDYES)
			{
				CDlgFirstRun *d = new CDlgFirstRun;
				d->DoModal();
			}
		}
		if ((p = out.find(keyid)) == string::npos) {
			question += keyid;
			question += Translate(" deleted from GPG secret keyring.\nDo you want to set another key?");
			if (MessageBoxA(nullptr, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
			{
				CDlgFirstRun *d = new CDlgFirstRun;
				d->DoModal();
			}
		}
		p2 = p;
		p = out.find("[", p);
		p2 = out.find("\n", p2);
		if ((p != std::string::npos) && (p < p2)) {
			p = out.find("expires:", p);
			p += mir_strlen("expires:");
			p++;
			p2 = out.find("]", p);
			wchar_t *expire_date = mir_wstrdup(toUTF16(out.substr(p, p2 - p)).c_str());
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
				question += keyid;
				question += Translate(" expired and will not work.\nDo you want to set another key?");
				if (MessageBoxA(nullptr, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
				{
					CDlgFirstRun *d = new CDlgFirstRun;
					d->DoModal();
				}
			}
			mir_free(expire_date);
		}
		// TODO: check for expired key
		mir_free(keyid);
		mir_free(key);
	}
	{
		wchar_t *path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"");
		DWORD dwFileAttr = GetFileAttributes(path);
		if (dwFileAttr != INVALID_FILE_ATTRIBUTES) {
			dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(path, dwFileAttr);
		}
		mir_free(path);
	}
	if (globals.bAutoExchange) {
		int count = 0;
		PROTOACCOUNT **accounts;
		Proto_EnumAccounts(&count, &accounts);
		ICQ_CUSTOMCAP cap;
		cap.cbSize = sizeof(ICQ_CUSTOMCAP);
		cap.hIcon = nullptr;
		strncpy(cap.name, "GPG Key AutoExchange", MAX_CAPNAME);
		strncpy(cap.caps, "GPGAutoExchange", sizeof(cap.caps));

		for (int i = 0; i < count; i++)
			if (ProtoServiceExists(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY))
				CallProtoService(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&cap);
	}
	if (globals.bFileTransfers) {
		int count = 0;
		PROTOACCOUNT **accounts;
		Proto_EnumAccounts(&count, &accounts);
		ICQ_CUSTOMCAP cap;
		cap.cbSize = sizeof(ICQ_CUSTOMCAP);
		cap.hIcon = nullptr;
		strncpy(cap.name, "GPG Encrypted FileTransfers", MAX_CAPNAME);
		strncpy(cap.caps, "GPGFileTransfer", sizeof(cap.caps));

		for (int i = 0; i < count; i++)
			if (ProtoServiceExists(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY))
				CallProtoService(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&cap);
	}
}

void ImportKey(MCONTACT hContact, std::wstring new_key)
{
	bool for_all_sub = false;
	if (db_mc_isMeta(hContact)) {
		if (MessageBox(nullptr, TranslateT("Do you want to load key for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES)
			for_all_sub = true;

		if (for_all_sub) {
			int count = db_mc_getSubCount(hContact);
			for (int i = 0; i < count; i++) {
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if (hcnt)
					db_set_ws(hcnt, szGPGModuleName, "GPGPubKey", new_key.c_str());
			}
		}
		else db_set_ws(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", new_key.c_str());
	}
	else db_set_ws(hContact, szGPGModuleName, "GPGPubKey", new_key.c_str());

	// gpg execute block
	std::vector<wstring> cmd;
	wchar_t tmp2[MAX_PATH] = { 0 };
	{
		wcsncpy(tmp2, ptrW(UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", L"")), MAX_PATH - 1);
		mir_wstrncat(tmp2, L"\\", _countof(tmp2) - mir_wstrlen(tmp2));
		mir_wstrncat(tmp2, L"temporary_exported.asc", _countof(tmp2) - mir_wstrlen(tmp2));
		boost::filesystem::remove(tmp2);

		ptrW ptmp;
		if (db_mc_isMeta(hContact))
			ptmp = UniGetContactSettingUtf(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", L"");
		else
			ptmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", L"");

		wfstream f(tmp2, std::ios::out);
		f << ptmp.get();
		f.close();
		cmd.push_back(L"--batch");
		cmd.push_back(L"--import");
		cmd.push_back(tmp2);
	}

	gpg_execution_params params(cmd);
	string output;
	DWORD exitcode;
	pxResult result;
	params.out = &output;
	params.code = &exitcode;
	params.result = &result;
	if (!gpg_launcher(params))
		return;
	if (result == pxNotFound)
		return;

	if (db_mc_isMeta(hContact)) {
		if (for_all_sub) {
			int count = db_mc_getSubCount(hContact);
			for (int i = 0; i < count; i++) {
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if (hcnt) {
					char *tmp = nullptr;
					string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
					string::size_type s2 = output.find(":", s);
					db_set_s(hcnt, szGPGModuleName, "KeyID", output.substr(s, s2 - s).c_str());
					s = output.find("вЂњ", s2);
					if (s == string::npos) {
						s = output.find("\"", s2);
						s += 1;
					}
					else s += 3;

					bool uncommon = false;
					if ((s2 = output.find("(", s)) == string::npos) {
						if ((s2 = output.find("<", s)) == string::npos) {
							s2 = output.find("вЂќ", s);
							uncommon = true;
						}
					}
					else if (s2 > output.find("<", s))
						s2 = output.find("<", s);
					if (s != string::npos && s2 != string::npos) {
						tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
						mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
						mir_utf8decode(tmp, nullptr);
						db_set_s(hcnt, szGPGModuleName, "KeyMainName", tmp);
						mir_free(tmp);
					}

					if ((s = output.find(")", s2)) == string::npos)
						s = output.find(">", s2);
					else if (s > output.find(">", s2))
						s = output.find(">", s2);
					s2++;
					if (s != string::npos && s2 != string::npos) {
						if (output[s] == ')') {
							tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
							mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
							mir_utf8decode(tmp, nullptr);
							db_set_s(hcnt, szGPGModuleName, "KeyComment", tmp);
							mir_free(tmp);
							s += 3;
							s2 = output.find(">", s);
							if (s != string::npos && s2 != string::npos) {
								tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
								mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
								mir_utf8decode(tmp, nullptr);
								db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", tmp);
								mir_free(tmp);
							}
						}
						else {
							tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
							mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
							mir_utf8decode(tmp, nullptr);
							db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
							mir_free(tmp);
						}
					}
					db_unset(hcnt, szGPGModuleName, "bAlwatsTrust");
				}
			}
		}
		else {
			char *tmp = nullptr;
			string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
			string::size_type s2 = output.find(":", s);
			db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyID", output.substr(s, s2 - s).c_str());
			s = output.find("вЂњ", s2);
			if (s == string::npos) {
				s = output.find("\"", s2);
				s += 1;
			}
			else
				s += 3;
			bool uncommon = false;
			if ((s2 = output.find("(", s)) == string::npos) {
				if ((s2 = output.find("<", s)) == string::npos) {
					s2 = output.find("вЂќ", s);
					uncommon = true;
				}
			}
			else if (s2 > output.find("<", s))
				s2 = output.find("<", s);
			if (s != string::npos && s2 != string::npos) {
				tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
				mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
				mir_utf8decode(tmp, nullptr);
				db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainName", tmp);
				mir_free(tmp);
			}
			if ((s = output.find(")", s2)) == string::npos)
				s = output.find(">", s2);
			else if (s > output.find(">", s2))
				s = output.find(">", s2);
			s2++;
			if (s != string::npos && s2 != string::npos) {
				if (output[s] == ')') {
					tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
					mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
					mir_utf8decode(tmp, nullptr);
					db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyComment", tmp);
					mir_free(tmp);
					s += 3;
					s2 = output.find(">", s);
					if (s != string::npos && s2 != string::npos) {
						tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
						mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
						mir_utf8decode(tmp, nullptr);
						db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", tmp);
						mir_free(tmp);
					}
				}
				else {
					tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
					mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
					mir_utf8decode(tmp, nullptr);
					db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
					mir_free(tmp);
				}
			}
			db_unset(metaGetMostOnline(hContact), szGPGModuleName, "bAlwatsTrust");
		}
	}
	else {
		char *tmp = nullptr;
		string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
		string::size_type s2 = output.find(":", s);
		db_set_s(hContact, szGPGModuleName, "KeyID", output.substr(s, s2 - s).c_str());
		s = output.find("вЂњ", s2);
		if (s == string::npos) {
			s = output.find("\"", s2);
			s += 1;
		}
		else
			s += 3;
		bool uncommon = false;
		if ((s2 = output.find("(", s)) == string::npos) {
			if ((s2 = output.find("<", s)) == string::npos) {
				s2 = output.find("вЂќ", s);
				uncommon = true;
			}
		}
		else if (s2 > output.find("<", s))
			s2 = output.find("<", s);
		if (s != string::npos && s2 != string::npos) {
			tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
			mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
			mir_utf8decode(tmp, nullptr);
			db_set_s(hContact, szGPGModuleName, "KeyMainName", tmp);
			mir_free(tmp);
		}
		if ((s = output.find(")", s2)) == string::npos)
			s = output.find(">", s2);
		else if (s > output.find(">", s2))
			s = output.find(">", s2);
		s2++;
		if (s != string::npos && s2 != string::npos) {
			if (output[s] == ')') {
				tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
				mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
				mir_utf8decode(tmp, nullptr);
				db_set_s(hContact, szGPGModuleName, "KeyComment", tmp);
				mir_free(tmp);
				s += 3;
				s2 = output.find(">", s);
				if (s != string::npos && s2 != string::npos) {
					tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
					mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
					mir_utf8decode(tmp, nullptr);
					db_set_s(hContact, szGPGModuleName, "KeyMainEmail", tmp);
					mir_free(tmp);
				}
			}
			else {
				tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
				mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
				mir_utf8decode(tmp, nullptr);
				db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2, s - s2).c_str());
				mir_free(tmp);
			}
		}
		db_unset(hContact, szGPGModuleName, "bAlwatsTrust");
	}

	MessageBox(nullptr, toUTF16(output).c_str(), L"", MB_OK);
	boost::filesystem::remove(tmp2);
}
