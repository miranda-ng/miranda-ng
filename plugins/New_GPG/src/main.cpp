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

#pragma comment(lib, "shlwapi.lib")

/////////////////////////////////////////////////////////////////////////////////////////
// GPG binaries options

class CDlgGpgBinOpts : public CDlgBase
{
	CCtrlButton btn_SET_BIN_PATH, btn_SET_HOME_DIR, btn_OK, btn_GENERATE_RANDOM;
	CCtrlEdit edit_BIN_PATH, edit_HOME_DIR;
	CCtrlCheck chk_AUTO_EXCHANGE;

public:
	CDlgGpgBinOpts() :
		CDlgBase(g_plugin, IDD_BIN_PATH),
		btn_SET_BIN_PATH(this, IDC_SET_BIN_PATH),
		btn_SET_HOME_DIR(this, IDC_SET_HOME_DIR),
		btn_OK(this, ID_OK),
		btn_GENERATE_RANDOM(this, IDC_GENERATE_RANDOM),
		edit_BIN_PATH(this, IDC_BIN_PATH),
		edit_HOME_DIR(this, IDC_HOME_DIR),
		chk_AUTO_EXCHANGE(this, IDC_AUTO_EXCHANGE)
	{
		btn_SET_BIN_PATH.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_SET_BIN_PATH);
		btn_SET_HOME_DIR.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_SET_HOME_DIR);
		btn_OK.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_OK);
		btn_GENERATE_RANDOM.OnClick = Callback(this, &CDlgGpgBinOpts::onClick_GENERATE_RANDOM);
	}

	bool OnInitDialog() override
	{
		CMStringW path;
		bool gpg_exists = false, lang_exists = false;

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

		DWORD len = MAX_PATH;
		bool bad_version = false;
		{
			ptrW tmp;
			if (!gpg_exists) {
				tmp = g_plugin.getWStringA("szGpgBinPath", (SHGetValueW(HKEY_CURRENT_USER, L"Software\\GNU\\GnuPG", L"gpgProgram", 0, (void *)path.c_str(), &len) == ERROR_SUCCESS) ? path.c_str() : L"");
				if (tmp[0])
					if (!boost::filesystem::exists((wchar_t *)tmp))
						MessageBoxW(nullptr, TranslateT("Wrong GPG binary location found in system.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
			}
			else tmp = mir_wstrdup(path.c_str());

			edit_BIN_PATH.SetText(tmp);
			if (gpg_exists/* && lang_exists*/) {
				g_plugin.setWString("szGpgBinPath", tmp);

				gpg_execution_params params;
				params.addParam(L"--version");
				bool _gpg_valid = globals.gpg_valid;
				globals.gpg_valid = true;
				gpg_launcher(params);
				globals.gpg_valid = _gpg_valid; //TODO: check this
				g_plugin.delSetting("szGpgBinPath");
				int p1 = params.out.Find("(GnuPG) ");
				if (p1 != -1) {
					p1 += mir_strlen("(GnuPG) ");
					if (params.out[p1] != '1')
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

		CMStringW tmp(g_plugin.getMStringW("szHomePath"));
		if (tmp.IsEmpty()) {
			mir_wstrcat(mir_path, L"\\gpg");
			if (_waccess(mir_path, 0) != -1) {
				tmp = mir_path;
				MessageBoxW(nullptr, TranslateT("\"GPG\" directory found in Miranda root.\nAssuming it's GPG home directory.\nGPG home directory set."), TranslateT("Info"), MB_OK);
			}
			else {
				wstring path_ = _wgetenv(L"APPDATA");
				path_ += L"\\GnuPG";
				tmp = path_.c_str();
			}
		}
		edit_HOME_DIR.SetText(!gpg_exists ? tmp : L"gpg");

		// TODO: additional check for write access
		if (gpg_exists && lang_exists && !bad_version)
			MessageBox(nullptr, TranslateT("Your GPG version is supported. The language file was found.\nGPG plugin should work fine.\nPress OK to continue."), TranslateT("Info"), MB_OK);
		chk_AUTO_EXCHANGE.Enable();
		return true;
	}

	void OnDestroy() override
	{
		void InitCheck();
		InitCheck();
	}

	void onClick_SET_BIN_PATH(CCtrlButton *)
	{
		GetFilePath(L"Choose gpg.exe", "szGpgBinPath", L"*.exe", L"EXE Executables");
		CMStringW tmp(g_plugin.getMStringW("szGpgBinPath", L"gpg.exe"));
		edit_BIN_PATH.SetText(tmp);

		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		if (tmp.Find(mir_path, 0) == 0) {
			CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
			edit_BIN_PATH.SetText(path);
		}
	}

	void onClick_SET_HOME_DIR(CCtrlButton *)
	{
		GetFolderPath(L"Set home directory");
		CMStringW tmp(g_plugin.getMStringW("szHomePath"));
		edit_HOME_DIR.SetText(tmp);

		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		PathToAbsoluteW(L"\\", mir_path);
		if (tmp.Find(mir_path, 0) == 0) {
			CMStringW path = tmp.Mid(mir_wstrlen(mir_path));
			edit_HOME_DIR.SetText(path);
		}
	}

	void onClick_OK(CCtrlButton *)
	{
		if (gpg_validate_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText())) {
			gpg_save_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText());
			globals.gpg_valid = true;
			g_plugin.setByte("FirstRun", 0);
			this->Hide();
			this->Close();
			ShowFirstRunDialog();
		}
	}

	void onClick_GENERATE_RANDOM(CCtrlButton *)
	{
		if (gpg_validate_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText())) {
			gpg_save_paths(edit_BIN_PATH.GetText(), edit_HOME_DIR.GetText());
			globals.gpg_valid = true;
			if (gpg_use_new_random_key(nullptr)) {
				g_plugin.bAutoExchange = chk_AUTO_EXCHANGE.GetState();
				globals.gpg_valid = true;
				g_plugin.setByte("FirstRun", 0);
				this->Close();
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

static int EnumProc(const char *szSetting, void *param)
{
	auto *list = (OBJLIST<CMStringA> *)param;
	if (strchr(szSetting, '(') && strchr(szSetting, ')'))
		list->insert(new CMStringA(szSetting));
	return 0;
}

void FirstRun()
{
	if (g_plugin.getByte("CompatLevel") != 1) {
		OBJLIST<CMStringA> settings(1);
		db_enum_settings(0, EnumProc, MODULENAME, &settings);

		for (auto &it : settings) {
			CMStringA newName(*it);
			int p1 = newName.Find('(');
			newName.Delete(0, p1+1);
			int p2 = newName.Find(')');
			if (p2 == -1)
				continue;
			newName.Delete(p2, 1);

			CMStringW val = g_plugin.getMStringW(it->c_str());
			g_plugin.delSetting(it->c_str());
			g_plugin.setWString(newName, val);		
		}

		g_plugin.setByte("CompatLevel", 1);
	}

	if (g_plugin.getByte("FirstRun", 1))
		(new CDlgGpgBinOpts())->Show();
}

void InitCheck()
{
	// parse gpg output
	{
		ptrW current_home(g_plugin.getWStringA("szHomePath", L""));
		g_plugin.setWString("szHomePath", L""); //we do not need home for gpg binary validation
		globals.gpg_valid = isGPGValid();
		g_plugin.setWString("szHomePath", current_home); //return current home dir back
	}
	{
		bool home_dir_access = false, temp_access = false;
		std::wstring test_path(ptrW(g_plugin.getWStringA("szHomePath", L"")));
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

		test_path = _wgetenv(L"TEMP");
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
			CMStringW buf;
			buf.Append(globals.gpg_valid ? TranslateT("GPG binary is set and valid (this is good).\n") : TranslateT("GPG binary unset or invalid (plugin will not work).\n"));
			buf.Append(home_dir_access ? TranslateT("Home dir write access granted (this is good).\n") : TranslateT("Home dir has no write access (plugin most probably will not work).\n"));
			buf.Append(temp_access ? TranslateT("Temp dir write access granted (this is good).\n") : TranslateT("Temp dir has no write access (plugin should work, but may have some problems, file transfers will not work)."));
			if (!globals.gpg_valid)
				buf.Append(TranslateT("\nGPG will be disabled until you solve these problems"));
			MessageBox(nullptr, buf, TranslateT("GPG plugin problems"), MB_OK);
		}
		if (!globals.gpg_valid)
			return;
		globals.gpg_keyexist = isGPGKeyExist();

		wstring::size_type p = 0, p2 = 0;

		gpg_execution_params params;
		params.addParam(L"--list-secret-keys");
		params.addParam(L"--batch");
		if (!gpg_launcher(params))
			return;
		if (params.result == pxNotFound)
			return;

		_wmkdir(g_plugin.getMStringW("szHomePath") + L"\\tmp");
		string out(params.out);

		CMStringW wszQuestion;
		for (auto &pa : Accounts()) {
			if (StriStr(pa->szModuleName, "metacontacts"))
				continue;
			if (StriStr(pa->szModuleName, "weather"))
				continue;
			
			std::string acc = pa->szModuleName;
			acc += "_KeyID";
			CMStringA keyid = g_plugin.getMStringA(acc.c_str());
			if (!keyid.IsEmpty()) {
				wszQuestion = TranslateT("Your secret key with ID: ");
				keyid = g_plugin.getMStringA("KeyID");
				if ((p = out.find(keyid)) == string::npos) {
					wszQuestion += keyid;
					wszQuestion += TranslateT(" for account ");
					wszQuestion += pa->tszAccountName;
					wszQuestion += TranslateT(" deleted from GPG secret keyring.\nDo you want to set another key?");
					if (MessageBoxW(nullptr, wszQuestion, TranslateT("Own secret key warning"), MB_YESNO) == IDYES)
						ShowFirstRunDialog();
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
						wszQuestion += keyid;
						wszQuestion += TranslateT(" for account ");
						wszQuestion += pa->tszAccountName;
						wszQuestion += TranslateT(" expired and will not work.\nDo you want to set another key?");
						if (MessageBoxW(nullptr, wszQuestion.c_str(), TranslateT("Own secret key warning"), MB_YESNO) == IDYES)
							ShowFirstRunDialog();
					}
					mir_free(expire_date);
				}
			}
		}

		wszQuestion = TranslateT("Your secret key with ID: ");
		CMStringA keyid(g_plugin.getMStringA("KeyID"));
		CMStringA key(g_plugin.getMStringA("GPGPubKey"));
		if (!g_plugin.getByte("FirstRun", 1) && (keyid.IsEmpty() || key.IsEmpty())) {
			wszQuestion = TranslateT("You didn't set a private key.\nWould you like to set it now?");
			if (MessageBoxW(nullptr, wszQuestion, TranslateT("Own private key warning"), MB_YESNO) == IDYES)
				ShowFirstRunDialog();
		}
		if ((p = out.find(keyid)) == string::npos) {
			wszQuestion += keyid;
			wszQuestion += TranslateT(" deleted from GPG secret keyring.\nDo you want to set another key?");
			if (MessageBoxW(nullptr, wszQuestion, TranslateT("Own secret key warning"), MB_YESNO) == IDYES)
				ShowFirstRunDialog();
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
				wszQuestion += keyid;
				wszQuestion += TranslateT(" expired and will not work.\nDo you want to set another key?");
				if (MessageBoxW(nullptr, wszQuestion, TranslateT("Own secret key warning"), MB_YESNO) == IDYES)
					ShowFirstRunDialog();
			}
			mir_free(expire_date);
		}
		// TODO: check for expired key
	}
	{
		CMStringW path(g_plugin.getMStringW("szHomePath"));
		uint32_t dwFileAttr = GetFileAttributes(path);
		if (dwFileAttr != INVALID_FILE_ATTRIBUTES) {
			dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(path, dwFileAttr);
		}
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
					g_plugin.setWString(hcnt, "GPGPubKey", new_key.c_str());
			}
		}
		else g_plugin.setWString(metaGetMostOnline(hContact), "GPGPubKey", new_key.c_str());
	}
	else g_plugin.setWString(hContact, "GPGPubKey", new_key.c_str());

	// gpg execute block
	CMStringW tmp2 = g_plugin.getMStringW("szHomePath");
	tmp2 += L"\\temporary_exported.asc";
	boost::filesystem::remove(tmp2.c_str());

	CMStringW ptmp;
	if (db_mc_isMeta(hContact))
		ptmp = g_plugin.getMStringW(metaGetMostOnline(hContact), "GPGPubKey");
	else
		ptmp = g_plugin.getMStringW(hContact, "GPGPubKey");

	wfstream f(tmp2, std::ios::out);
	f << ptmp.c_str();
	f.close();

	gpg_execution_params params;
	params.addParam(L"--batch");
	params.addParam(L"--import");
	params.addParam(tmp2.c_str());
	if (!gpg_launcher(params))
		return;
	if (params.result == pxNotFound)
		return;

	string output(params.out);
	if (db_mc_isMeta(hContact)) {
		if (for_all_sub) {
			int count = db_mc_getSubCount(hContact);
			for (int i = 0; i < count; i++) {
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if (hcnt) {
					char *tmp = nullptr;
					string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
					string::size_type s2 = output.find(":", s);
					g_plugin.setString(hcnt, "KeyID", output.substr(s, s2 - s).c_str());
					s = output.find(RUS_QUOTE, s2);
					if (s == string::npos) {
						s = output.find("\"", s2);
						s += 1;
					}
					else s += sizeof(RUS_QUOTE) - 1;

					bool uncommon = false;
					if ((s2 = output.find("(", s)) == string::npos) {
						if ((s2 = output.find("<", s)) == string::npos) {
							s2 = output.find(RUS_ANGLE, s);
							uncommon = true;
						}
					}
					else if (s2 > output.find("<", s))
						s2 = output.find("<", s);
					if (s != string::npos && s2 != string::npos) {
						tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
						mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
						mir_utf8decode(tmp, nullptr);
						g_plugin.setString(hcnt, "KeyMainName", tmp);
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
							g_plugin.setString(hcnt, "KeyComment", tmp);
							mir_free(tmp);
							s += 3;
							s2 = output.find(">", s);
							if (s != string::npos && s2 != string::npos) {
								tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
								mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
								mir_utf8decode(tmp, nullptr);
								g_plugin.setString(hcnt, "KeyMainEmail", tmp);
								mir_free(tmp);
							}
						}
						else {
							tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
							mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
							mir_utf8decode(tmp, nullptr);
							g_plugin.setString(hcnt, "KeyMainEmail", output.substr(s2, s - s2).c_str());
							mir_free(tmp);
						}
					}
					g_plugin.delSetting(hcnt, "bAlwatsTrust");
				}
			}
		}
		else {
			char *tmp = nullptr;
			string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
			string::size_type s2 = output.find(":", s);
			g_plugin.setString(metaGetMostOnline(hContact), "KeyID", output.substr(s, s2 - s).c_str());
			s = output.find(RUS_QUOTE, s2);
			if (s == string::npos) {
				s = output.find("\"", s2);
				s += 1;
			}
			else s += sizeof(RUS_QUOTE) - 1;

			bool uncommon = false;
			if ((s2 = output.find("(", s)) == string::npos) {
				if ((s2 = output.find("<", s)) == string::npos) {
					s2 = output.find(RUS_ANGLE, s);
					uncommon = true;
				}
			}
			else if (s2 > output.find("<", s))
				s2 = output.find("<", s);
			if (s != string::npos && s2 != string::npos) {
				tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
				mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
				mir_utf8decode(tmp, nullptr);
				g_plugin.setString(metaGetMostOnline(hContact), "KeyMainName", tmp);
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
					g_plugin.setString(metaGetMostOnline(hContact), "KeyComment", tmp);
					mir_free(tmp);
					s += 3;
					s2 = output.find(">", s);
					if (s != string::npos && s2 != string::npos) {
						tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
						mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
						mir_utf8decode(tmp, nullptr);
						g_plugin.setString(metaGetMostOnline(hContact), "KeyMainEmail", tmp);
						mir_free(tmp);
					}
				}
				else {
					tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
					mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
					mir_utf8decode(tmp, nullptr);
					g_plugin.setString(metaGetMostOnline(hContact), "KeyMainEmail", output.substr(s2, s - s2).c_str());
					mir_free(tmp);
				}
			}
			g_plugin.delSetting(metaGetMostOnline(hContact), "bAlwatsTrust");
		}
	}
	else {
		char *tmp = nullptr;
		string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
		string::size_type s2 = output.find(":", s);
		g_plugin.setString(hContact, "KeyID", output.substr(s, s2 - s).c_str());
		s = output.find(RUS_QUOTE, s2);
		if (s == string::npos) {
			s = output.find("\"", s2);
			s += 1;
		}
		else s += sizeof(RUS_QUOTE) - 1;
		
		bool uncommon = false;
		if ((s2 = output.find("(", s)) == string::npos) {
			if ((s2 = output.find("<", s)) == string::npos) {
				s2 = output.find(RUS_ANGLE, s);
				uncommon = true;
			}
		}
		else if (s2 > output.find("<", s))
			s2 = output.find("<", s);
		if (s != string::npos && s2 != string::npos) {
			tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s - (uncommon ? 1 : 0)).length() + 1));
			mir_strcpy(tmp, output.substr(s, s2 - s - (uncommon ? 1 : 0)).c_str());
			mir_utf8decode(tmp, nullptr);
			g_plugin.setString(hContact, "KeyMainName", tmp);
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
				g_plugin.setString(hContact, "KeyComment", tmp);
				mir_free(tmp);
				s += 3;
				s2 = output.find(">", s);
				if (s != string::npos && s2 != string::npos) {
					tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s, s2 - s).length() + 1));
					mir_strcpy(tmp, output.substr(s, s2 - s).c_str());
					mir_utf8decode(tmp, nullptr);
					g_plugin.setString(hContact, "KeyMainEmail", tmp);
					mir_free(tmp);
				}
			}
			else {
				tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2, s - s2).length() + 1));
				mir_strcpy(tmp, output.substr(s2, s - s2).c_str());
				mir_utf8decode(tmp, nullptr);
				g_plugin.setString(hContact, "KeyMainEmail", output.substr(s2, s - s2).c_str());
				mir_free(tmp);
			}
		}
		g_plugin.delSetting(hContact, "bAlwatsTrust");
	}

	MessageBox(nullptr, toUTF16(output).c_str(), L"", MB_OK);
	boost::filesystem::remove(tmp2.c_str());
}
