// Copyright © 2010-19 sss
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

#include "utf8.h"

void ShowExportKeysDlg();
void ShowLoadPublicKeyDialog(bool = false);

void GetFilePath(wchar_t *WindowTittle, char *szSetting, wchar_t *szExt, wchar_t *szExtDesc)
{
	wchar_t str[MAX_PATH + 2] = { 0 }, *tmp;
	OPENFILENAME ofn = { 0 };
	wchar_t filter[512], *pfilter;
	ofn.lStructSize = CDSIZEOF_STRUCT(OPENFILENAME, lpTemplateName);
	ofn.Flags = OFN_EXPLORER;
	ofn.lpstrTitle = TranslateW(WindowTittle);
	wcsncpy(filter, TranslateW(szExtDesc), _countof(filter) - 1);
	pfilter = filter + mir_wstrlen(filter) + 1;
	mir_wstrcpy(pfilter, szExt);
	pfilter[mir_wstrlen(pfilter) + 1] = '\0';
	pfilter[mir_wstrlen(pfilter) + 2] = '\0';
	ofn.lpstrFilter = filter;
	tmp = db_get_wsa(0, MODULENAME, szSetting, L"");
	wcsncpy(str, tmp, _countof(str) - 1);
	mir_free(tmp);
	if (mir_wstrlen(str) < 2)
		str[0] = '\0';
	ofn.lpstrFile = str;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	if (!GetOpenFileName(&ofn))
		return;
	g_plugin.setWString(szSetting, str);
}

wchar_t* GetFilePath(wchar_t *WindowTittle, wchar_t *szExt, wchar_t *szExtDesc, bool save_file)
{
	wchar_t *str = new wchar_t[MAX_PATH + 2];
	OPENFILENAME ofn = { 0 };
	wchar_t filter[512], *pfilter;
	ofn.lStructSize = CDSIZEOF_STRUCT(OPENFILENAME, lpTemplateName);
	ofn.Flags = OFN_EXPLORER;
	ofn.lpstrTitle = TranslateW(WindowTittle);
	mir_wstrcpy(filter, TranslateW(szExtDesc));
	pfilter = filter + mir_wstrlen(filter) + 1;
	mir_wstrcpy(pfilter, szExt);
	pfilter[mir_wstrlen(pfilter) + 1] = '\0';
	pfilter[mir_wstrlen(pfilter) + 2] = '\0';
	ofn.lpstrFilter = filter;
	mir_wstrcpy(str, L"");
	if (mir_wstrlen(str) < 2)
		str[0] = '\0';
	ofn.lpstrFile = str;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	if (!save_file) {
		if (!GetOpenFileName(&ofn)) {
			delete[] str;
			return nullptr;
		}
	}
	else {
		if (!GetSaveFileName(&ofn)) {
			delete[] str;
			return nullptr;
		}
	}
	return str;
}

void GetFolderPath(wchar_t *WindowTittle)
{
	BROWSEINFO pbi = {};
	pbi.lpszTitle = WindowTittle;
	pbi.ulFlags = BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_SHAREABLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&pbi);
	if (pidl != nullptr) {
		wchar_t path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path)) {
			g_plugin.setWString("szHomePath", path);
		}
		IMalloc * imalloc = nullptr;
		if (SUCCEEDED(SHGetMalloc(&imalloc))) {
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
}

INT_PTR LoadKey(WPARAM w, LPARAM)
{
	globals.user_data[(int)1] = (MCONTACT)w;
	ShowLoadPublicKeyDialog();
	return 0;
}

INT_PTR SendKey(WPARAM w, LPARAM)
{
	MCONTACT hContact = db_mc_tryMeta(w);
	char *szMessage;
	std::string key_id_str;
	{
		LPSTR proto = GetContactProto(hContact);
		PROTOACCOUNT *acc = Proto_GetAccount(proto);
		std::string acc_str;
		if (acc) {
			acc_str = toUTF8(acc->tszAccountName);
			acc_str += "(";
			acc_str += acc->szModuleName;
			acc_str += ")";
			key_id_str = acc_str;
			key_id_str += "_KeyID";
			acc_str += "_GPGPubKey";
		}
		szMessage = db_get_sa(0, MODULENAME, acc_str.empty() ? "GPGPubKey" : acc_str.c_str(), "");
		if (!szMessage[0]) {
			mir_free(szMessage);
			szMessage = db_get_sa(0, MODULENAME, "GPGPubKey", ""); //try to get default key as fallback in any way
		}
	}
	if (szMessage[0]) {
		BYTE enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
		g_plugin.setByte(hContact, "GPGEncryption", 0);
		ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)szMessage);
		std::string msg = "Public key ";
		char *keyid = db_get_sa(0, MODULENAME, key_id_str.c_str(), "");
		if (!keyid[0]) {
			mir_free(keyid);
			keyid = db_get_sa(0, MODULENAME, "KeyID", "");
		}
		msg += keyid;
		mir_free(keyid);
		msg += " sent";
		mir_free(szMessage);
		szMessage = mir_strdup(msg.c_str());
		HistoryLog(hContact, db_event(szMessage, 0, 0, DBEF_SENT));
		g_plugin.setByte(hContact, "GPGEncryption", enc);
	}
	else mir_free(szMessage);

	return 0;
}

INT_PTR ToggleEncryption(WPARAM w, LPARAM)
{
	MCONTACT hContact = (MCONTACT)w;
	BYTE enc;
	if (db_mc_isMeta(hContact)) {
		enc = db_get_b(metaGetMostOnline(hContact), MODULENAME, "GPGEncryption", 0);
		if (MessageBox(nullptr, TranslateT("Do you want to toggle encryption for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES) {
			int count = db_mc_getSubCount(hContact);
			for (int i = 0; i < count; i++) {
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if (hcnt)
					db_set_b(hcnt, MODULENAME, "GPGEncryption", enc ? 0 : 1);
			}
			g_plugin.setByte(hContact, "GPGEncryption", enc ? 0 : 1);
		}
	}
	else {
		enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
		g_plugin.setByte(hContact, "GPGEncryption", enc ? 0 : 1);
	}
	void setSrmmIcon(MCONTACT hContact);
	void setClistIcon(MCONTACT hContact);
	setSrmmIcon(hContact);
	setClistIcon(hContact);

	Menu_ModifyItem(globals.hToggleEncryption, enc ? LPGENW("Turn off GPG encryption") : LPGENW("Turn on GPG encryption"));
	return 0;
}

int OnPreBuildContactMenu(WPARAM w, LPARAM)
{
	MCONTACT hContact = db_mc_tryMeta(w);
	{
		CMenuItem mi2(&g_plugin);
		LPSTR proto = GetContactProto(hContact);
		PROTOACCOUNT *acc = Proto_GetAccount(proto);
		std::string setting;
		if (acc) {
			setting = toUTF8(acc->tszAccountName);
			setting += "(";
			setting += acc->szModuleName;
			setting += ")";
			setting += "_KeyID";
		}
		char *keyid = db_get_sa(0, MODULENAME, setting.c_str(), "");
		if (!keyid[0]) {
			mir_free(keyid);
			keyid = db_get_sa(0, MODULENAME, "KeyID", "");
		}
		wchar_t buf[128] = { 0 };
		mir_snwprintf(buf, L"%s: %s", TranslateT("Send public key"), toUTF16(keyid).c_str());
		mir_free(keyid);
		Menu_ModifyItem(globals.hSendKey, buf);
	}

	int flags;
	wchar_t *tmp = db_get_wsa(hContact, MODULENAME, "GPGPubKey", L"");
	if (!tmp[0]) {
		g_plugin.delSetting(hContact, "GPGEncryption");
		flags = CMIF_GRAYED;
	}
	else flags = 0;

	Menu_ModifyItem(globals.hToggleEncryption,
		g_plugin.getByte(hContact, "GPGEncryption", 0) ? L"Turn off GPG encryption" : L"Turn on GPG encryption",
		INVALID_HANDLE_VALUE, flags);
	mir_free(tmp);
	return 0;
}


list<wstring> transfers;

int onProtoAck(WPARAM, LPARAM l)
{
	ACKDATA *ack = (ACKDATA*)l;
	if (ack->type == ACKTYPE_FILE) {
		switch (ack->result) {
		case ACKRESULT_DENIED:	case ACKRESULT_FAILED:
			break;
		case ACKRESULT_SUCCESS:
			if (ack->hProcess) {
				PROTOFILETRANSFERSTATUS *f = (PROTOFILETRANSFERSTATUS*)ack->hProcess;
				if (f == nullptr)
					break;

				if ((f->flags & PFTS_SENDING) != PFTS_SENDING) {
					wchar_t *filename = nullptr;
					if (f->flags & PFTS_UNICODE) {
						if (f->szCurrentFile.w && f->szCurrentFile.w[0])
							filename = mir_wstrdup(f->szCurrentFile.w);
						if (!filename)
							return 0;
					}
					else {
						if (f->szCurrentFile.a && f->szCurrentFile.a[0])
							filename = mir_utf8decodeW(f->szCurrentFile.a);
						if (!filename)
							return 0;
					}
					if (wcsstr(filename, L".gpg")) { //decrypt it
						//process encrypted file
						if (!globals.bFileTransfers && !globals.bSameAction) {
							void ShowEncryptedFileMsgBox();
							ShowEncryptedFileMsgBox();
						}
						if (!globals.bFileTransfers && globals.bSameAction)
							return 0;
						if (!globals.bDecryptFiles)
							return 0;
						HistoryLog(ack->hContact, db_event("Received encrypted file, trying to decrypt", 0, 0, 0));
						if (!boost::filesystem::exists(f->szCurrentFile.w))
							return 0;
						string out;
						DWORD code;
						pxResult result;
						std::vector<wstring> cmd;
						cmd.push_back(L"-o");
						wstring file = filename;
						wstring::size_type p1 = file.rfind(L".gpg");
						file.erase(p1, mir_wstrlen(L".gpg"));
						if (boost::filesystem::exists(file)) {
							if (MessageBox(nullptr, TranslateT("Target file exists, do you want to replace it?"), TranslateT("Warning"), MB_YESNO) == IDNO)
								return 0;
						}
						cmd.push_back(file);
						boost::filesystem::remove(file);
						{ // password
							wchar_t *pass = nullptr;
							char *keyid = db_get_sa(ack->hContact, MODULENAME, "KeyID", "");
							if (mir_strlen(keyid) > 0) {
								string dbsetting = "szKey_";
								dbsetting += keyid;
								dbsetting += "_Password";
								pass = db_get_wsa(0, MODULENAME, dbsetting.c_str(), L"");
								if (mir_wstrlen(pass) > 0 && globals.bDebugLog)
									globals.debuglog << std::string(time_str() + ": info: found password in database for key ID: " + keyid + ", trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password");
							}
							else {
								pass = db_get_wsa(0, MODULENAME, "szKeyPassword", L"");
								if (mir_wstrlen(pass) > 0 && globals.bDebugLog)
									globals.debuglog << std::string(time_str() + ": info: found password for all keys in database, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password");
							}
							if (mir_wstrlen(pass) > 0) {
								cmd.push_back(L"--passphrase");
								cmd.push_back(pass);
							}
							else if (globals.password) {
								if (globals.bDebugLog)
									globals.debuglog << std::string(time_str() + ": info: found password in memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password");
								cmd.push_back(L"--passphrase");
								cmd.push_back(globals.password);
							}
							else if (globals.bDebugLog)
								globals.debuglog << std::string(time_str() + ": info: passwords not found in database or memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with out password");
							mir_free(pass);
							mir_free(keyid);
						}
						cmd.push_back(L"-d");
						cmd.push_back(filename);
						gpg_execution_params params(cmd);
						params.out = &out;
						params.code = &code;
						params.result = &result;
						if (!gpg_launcher(params, boost::posix_time::minutes(15)))
							return 0;
						while (out.find("public key decryption failed: bad passphrase") != string::npos) {
							if (globals.bDebugLog)
								globals.debuglog << std::string(time_str() + ": info: failed to decrypt messaage from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " password needed, trying to get one");
							if (globals._terminate)
								break;
							{ //save inkey id
								string::size_type s = out.find(" encrypted with ");
								s = out.find(" ID ", s);
								s += mir_strlen(" ID ");
								string::size_type s2 = out.find(",", s);
								if (db_mc_isMeta(ack->hContact))
									db_set_s(metaGetMostOnline(ack->hContact), MODULENAME, "InKeyID", out.substr(s, s2 - s).c_str());
								else
									db_set_s(ack->hContact, MODULENAME, "InKeyID", out.substr(s, s2 - s).c_str());
							}
							CDlgKeyPasswordMsgBox *d = new CDlgKeyPasswordMsgBox(ack->hContact);
							d->DoModal();
							std::vector<wstring> cmd2 = cmd;
							if (globals.password) {
								if (globals.bDebugLog)
									globals.debuglog << std::string(time_str() + ": info: found password in memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)));
								std::vector<wstring> tmp;
								tmp.push_back(L"--passphrase");
								tmp.push_back(globals.password);
								cmd2.insert(cmd2.begin(), tmp.begin(), tmp.end());
							}
							out.clear();
							gpg_execution_params params2(cmd2);
							params2.out = &out;
							params2.code = &code;
							params2.result = &result;
							if (!gpg_launcher(params2, boost::posix_time::seconds(15)))
								return 0;
							if (result == pxNotFound)
								return 0;
						}
						if (result == pxSuccess)
							boost::filesystem::remove(filename);
					}
					mir_free(filename);
				}
			}
			break;
		}
	}
	else if (ack->type == ACKTYPE_MESSAGE) {
		extern std::list<HANDLE> sent_msgs;
		if (!sent_msgs.empty()) {
			if (ack->result == ACKRESULT_FAILED) {
				std::list<HANDLE>::iterator it = std::find(sent_msgs.begin(), sent_msgs.end(), ack->hProcess);
				if (it != sent_msgs.end())
					HistoryLog(ack->hContact, db_event("Failed to send encrypted message", 0, 0, 0));
			}
			else if (ack->result == ACKRESULT_SUCCESS) {
				std::list<HANDLE>::iterator it = std::find(sent_msgs.begin(), sent_msgs.end(), ack->hProcess);
				if (it != sent_msgs.end())
					sent_msgs.erase(it);
			}
		}
	}
	return 0;
}

std::wstring encrypt_file(MCONTACT hContact, wchar_t *filename)
{
	string out;
	DWORD code;
	pxResult result;
	MCONTACT hcnt = db_mc_isMeta(hContact) ? metaGetMostOnline(hContact) : hContact;
	std::vector<wstring> cmd;
	cmd.push_back(L"--batch");
	cmd.push_back(L"--tes");
	cmd.push_back(L"-r");
	char *keyid = db_get_sa(hcnt, MODULENAME, "KeyID", "");
	wchar_t *szKeyid = mir_a2u(keyid);
	wchar_t *name = wcsrchr(filename, '\\');
	if (!name)
		name = filename;
	else
		name++;
	wchar_t *file_out = new wchar_t[mir_wstrlen(name) + mir_wstrlen(L".gpg") + 1];
	mir_snwprintf(file_out, mir_wstrlen(name) + mir_wstrlen(L".gpg") + 1, L"%s.gpg", name);
	cmd.push_back(szKeyid);
	if (db_get_b(hcnt, MODULENAME, "bAlwaysTrust", 0)) {
		cmd.push_back(L"--trust-model");
		cmd.push_back(L"always");
	}
	mir_free(szKeyid);
	mir_free(keyid);
	cmd.push_back(L"-o");
	wchar_t *temp = _tgetenv(L"TEMP");
	cmd.push_back(wstring(temp) + L"\\" + file_out);
	wstring path_out = temp;
	path_out += L"\\";
	path_out += file_out;
	boost::filesystem::remove(path_out);
	cmd.push_back(L"-e");
	cmd.push_back(filename);
	gpg_execution_params params(cmd);
	params.out = &out;
	params.code = &code;
	params.result = &result;
	delete[] file_out;
	if (!gpg_launcher(params, boost::posix_time::minutes(3)))
		return nullptr;
	if (out.find("There is no assurance this key belongs to the named user") != string::npos) {
		out.clear();
		if (MessageBox(nullptr, TranslateT("We're trying to encrypt with untrusted key. Do you want to trust this key permanently?"), TranslateT("Warning"), MB_YESNO) == IDYES) {
			db_set_b(hcnt, MODULENAME, "bAlwaysTrust", 1);
			std::vector<std::wstring> tmp;
			tmp.push_back(L"--trust-model");
			tmp.push_back(L"always");
			cmd.insert(cmd.begin(), tmp.begin(), tmp.end());
			if (!gpg_launcher(params, boost::posix_time::minutes(3)))
				return nullptr;
		}
		else
			return nullptr;
	}
	return path_out;
}

//from secureim partially
INT_PTR onSendFile(WPARAM w, LPARAM l)
{
	CCSDATA *ccs = (CCSDATA*)l;
	if (!globals.bFileTransfers)
		return Proto_ChainSend(w, ccs);

	if (isContactSecured(ccs->hContact)) {
		char *proto = GetContactProto(ccs->hContact);
		bool cap_found = false, supported_proto = false;
		ptrA jid(db_get_utfa(ccs->hContact, proto, "jid", ""));
		if (jid[0]) {
			for (auto p : globals.Accounts) {
				ptrA caps(p->getJabberInterface()->GetResourceFeatures(jid));
				if (caps) {
					supported_proto = true;
					string str;
					for (int i = 0;; i++) {
						str.push_back(caps[i]);
						if (caps[i] == '\0')
							if (caps[i + 1] == '\0')
								break;
					}

					if (str.find("GPG_Encrypted_FileTransfers:0") != string::npos)
						cap_found = true;
				}
			}
		}

		if (supported_proto && !cap_found) {
			if (MessageBox(nullptr, TranslateT("Capability to decrypt file not found on other side.\nRecipient may be unable to decrypt file(s).\nDo you want to encrypt file(s) anyway?"), TranslateT("File transfer warning"), MB_YESNO) == IDNO)
				return Proto_ChainSend(w, ccs);
		}
		if (!supported_proto) {
			if (MessageBox(nullptr, TranslateT("Unable to check encryption support on other side.\nRecipient may be unable to decrypt file(s).\nCurrently capability check supported only for ICQ and Jabber protocols.\nIt will work for any other proto if Miranda with New_GPG is used on other side.\nDo you want to encrypt file(s) anyway?"), TranslateT("File transfer warning"), MB_YESNO) == IDNO)
				return Proto_ChainSend(w, ccs);
		}
		HistoryLog(ccs->hContact, db_event(Translate("encrypting file for transfer"), 0, 0, DBEF_SENT));
		if (StriStr(ccs->szProtoService, "/sendfilew")) {
			wchar_t **file = (wchar_t **)ccs->lParam;
			for (int i = 0; file[i]; i++) {
				if (!boost::filesystem::exists(file[i]))
					return 0; //we do not want to send file unencrypted (sometimes ack have wrong info)
				if (wcsstr(file[i], L".gpg"))
					continue;
				std::wstring path_out = encrypt_file(ccs->hContact, file[i]);
				mir_free(file[i]);
				file[i] = mir_wstrdup(path_out.c_str());
				transfers.push_back(path_out);
			}
		}
		else {
			char **file = (char**)ccs->lParam;
			for (int i = 0; file[i]; i++) {
				if (!boost::filesystem::exists(file[i]))
					return 0; //we do not want to send file unencrypted (sometimes ack have wrong info)
				if (strstr(file[i], ".gpg"))
					continue;
				wchar_t *tmp = mir_utf8decodeW(file[i]);
				std::wstring path_out = encrypt_file(ccs->hContact, tmp);
				mir_free(tmp);
				char* tmp2 = mir_utf8encodeW(path_out.c_str());
				mir_free(file[i]);
				file[i] = tmp2;
				transfers.push_back(path_out);

			}
		}
	}
	return Proto_ChainSend(w, ccs);
}


void HistoryLog(MCONTACT hContact, db_event evt)
{
	DBEVENTINFO Event = {};
	Event.szModule = MODULENAME;
	Event.eventType = evt.eventType;
	Event.flags = evt.flags;
	if (!evt.timestamp)
		Event.timestamp = (DWORD)time(0);
	else
		Event.timestamp = evt.timestamp;
	Event.cbBlob = (DWORD)mir_strlen((char*)evt.pBlob) + 1;
	Event.pBlob = (PBYTE)_strdup((char*)evt.pBlob);
	db_event_add(hContact, &Event);
}

static int ControlAddStringUtf(HWND ctrl, DWORD msg, const wchar_t *szString)
{
	int item = -1;
	item = SendMessage(ctrl, msg, 0, (LPARAM)szString);
	return item;
}

int ComboBoxAddStringUtf(HWND hCombo, const wchar_t *szString, DWORD data)
{
	int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
}


int GetJabberInterface(WPARAM, LPARAM) //get interface for all jabber accounts, options later
{
	void AddHandlers();

	list <JabberAccount*>::iterator p;
	globals.Accounts.clear();
	globals.Accounts.push_back(new JabberAccount);
	p = globals.Accounts.begin();
	(*p)->setAccountNumber(0);
	for (auto &pa : Accounts()) {
		IJabberInterface *JIftmp = getJabberApi(pa->szModuleName);
		int a = 0;
		if (JIftmp) {
			(*p)->setJabberInterface(JIftmp);
			if (pa->tszAccountName) {
				wchar_t* tmp = mir_wstrdup(pa->tszAccountName);
				(*p)->setAccountName(tmp);
			}
			else {
				wchar_t *tmp = mir_a2u(pa->szModuleName);
				(*p)->setAccountName(tmp);
			}
			(*p)->setAccountNumber(a);
			a++;
			globals.Accounts.push_back(new JabberAccount);
			p++;
		}
	}
	globals.Accounts.pop_back();
	AddHandlers();
	return 0;
}

static JABBER_HANDLER_FUNC SendHandler(IJabberInterface *ji, TiXmlElement *node, void*)
{
	TiXmlDocument *pDoc = node->GetDocument();

	for (auto *local_node = node->FirstChildElement(); local_node; local_node = local_node->NextSiblingElement()) {
		LPCSTR str = local_node->GetText();
		LPCSTR nodename = local_node->Name();
		LPCSTR attr = local_node->Attribute("to");
		if (attr) {
			MCONTACT hContact = ji->ContactFromJID(attr);
			if (hContact)
				if (!isContactSecured(hContact))
					break;
		}

		if (str == nullptr)
			continue;

		// TODO: make following block more readable
		if (strstr(str, "-----BEGIN PGP MESSAGE-----") && strstr(str, "-----END PGP MESSAGE-----")) {
			string data = str;
			local_node->SetText("This message is encrypted.");
			string::size_type p1 = data.find("-----BEGIN PGP MESSAGE-----") + mir_strlen("-----BEGIN PGP MESSAGE-----");
			while (data.find("Version: ", p1) != wstring::npos) {
				p1 = data.find("Version: ", p1);
				p1 = data.find("\n", p1);
			}
			while (data.find("Comment: ", p1) != wstring::npos) {
				p1 = data.find("Comment: ", p1);
				p1 = data.find("\n", p1);
			}
			while (data.find("Encoding: ", p1) != wstring::npos) {
				p1 = data.find("Encoding: ", p1);
				p1 = data.find("\n", p1);
			}
			p1 += 3;
			string::size_type p2 = data.find("-----END PGP MESSAGE-----");
			string data2 = data.substr(p1, p2 - p1 - 2);
			strip_line_term(data2);
			if (globals.bDebugLog)
				globals.debuglog << std::string(time_str() + ": jabber_api: attaching:\r\n\r\n" + data2 + "\n\n\t to outgoing xml");
			
			TiXmlElement *encrypted_data = pDoc->NewElement("x"); node->InsertEndChild(encrypted_data);
			encrypted_data->SetText(data2.c_str());
			encrypted_data->SetAttribute("xmlns", "jabber:x:encrypted");
			break;
		}

		if (globals.bPresenceSigning && nodename && strstr(nodename, "status")) {
			char *path_c = db_get_utfa(0, MODULENAME, "szHomePath", "");
			string path_out = path_c;
			string file = get_random(10);
			mir_free(path_c);
			path_out += "\\tmp\\";
			path_out += file;
			boost::filesystem::remove(path_out);
			wfstream f(path_out.c_str(), std::ios::out);
			f << str;
			f.close();
			if (!boost::filesystem::exists(path_out)) {
				if (globals.bDebugLog)
					globals.debuglog << std::string(time_str() + ": info: Failed to write prescense in file");
				break;
			}

			string out;
			DWORD code;
			std::vector<wstring> cmd;
			{
				char* inkeyid = nullptr;

				char setting[64];
				mir_snprintf(setting, sizeof(setting) - 1, "%s_KeyID", ji->GetModuleName());
				inkeyid = db_get_sa(0, MODULENAME, setting, "");
				if (!inkeyid[0]) {
					mir_free(inkeyid);
					inkeyid = db_get_sa(0, MODULENAME, "KeyID", "");
				}

				ptrW pass;
				if (inkeyid[0]) {
					string dbsetting = "szKey_";
					dbsetting += inkeyid;
					dbsetting += "_Password";
					pass = db_get_wsa(0, MODULENAME, dbsetting.c_str(), L"");
					if (pass[0] && globals.bDebugLog)
						globals.debuglog << std::string(time_str() + ": info: found password in database for key ID: " + inkeyid + ", trying to encrypt message from self with password");
				}
				if (inkeyid && inkeyid[0])
					mir_free(inkeyid);
				else {
					pass = db_get_wsa(0, MODULENAME, "szKeyPassword", L"");
					if (pass[0] && globals.bDebugLog)
						globals.debuglog << std::string(time_str() + ": info: found password for all keys in database, trying to encrypt message from self with password");
				}
				if (pass[0]) {
					cmd.push_back(L"--passphrase");
					cmd.push_back(pass.get());
				}
				else if (globals.password && globals.password[0]) {
					if (globals.bDebugLog)
						globals.debuglog << std::string(time_str() + ": info: found password in memory, trying to encrypt message from self with password");
					cmd.push_back(L"--passphrase");
					cmd.push_back(globals.password);
				}
				else if (globals.bDebugLog)
					globals.debuglog << std::string(time_str() + ": info: passwords not found in database or memory, trying to encrypt message from self with out password");
			}

			cmd.push_back(L"--local-user");
			cmd.push_back(ptrW(db_get_wsa(0, MODULENAME, "KeyID", L"")).get());
			cmd.push_back(L"--default-key");
			cmd.push_back(ptrW(db_get_wsa(0, MODULENAME, "KeyID", L"")).get());
			cmd.push_back(L"--batch");
			cmd.push_back(L"--yes");
			cmd.push_back(L"-abs");
			cmd.push_back(Utf2T(path_out.c_str()).get());
			gpg_execution_params params(cmd);
			pxResult result;
			params.out = &out;
			params.code = &code;
			params.result = &result;
			gpg_launcher(params, boost::posix_time::seconds(15)); // TODO: handle errors
			boost::filesystem::remove(path_out);
			path_out += ".asc";
			f.open(path_out.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
			wstring data;
			if (f.is_open()) {
				std::wifstream::pos_type size = f.tellg();
				wchar_t *tmp = new wchar_t[(std::ifstream::pos_type)size + (std::ifstream::pos_type)1];
				f.seekg(0, std::ios::beg);
				f.read(tmp, size);
				tmp[size] = '\0';
				data.append(tmp);
				delete[] tmp;
				f.close();
				boost::filesystem::remove(path_out);
			}
			if (data.empty()) {
				if (globals.bDebugLog)
					globals.debuglog << std::string(time_str() + ": info: Failed to read prescense sign from file");
				break;
			}
			if (data.find(L"-----BEGIN PGP SIGNATURE-----") != wstring::npos && data.find(L"-----END PGP SIGNATURE-----") != wstring::npos) {
				wstring::size_type p1 = data.find(L"-----BEGIN PGP SIGNATURE-----") + mir_wstrlen(L"-----BEGIN PGP SIGNATURE-----");
				if (data.find(L"Version: ", p1) != wstring::npos) {
					p1 = data.find(L"Version: ", p1);
					p1 = data.find(L"\n", p1);
					if (data.find(L"Version: ", p1) != wstring::npos) {
						p1 = data.find(L"Version: ", p1);
						p1 = data.find(L"\n", p1) + 1;
					}
					else
						p1 += 1;
				}
				if (data.find(L"Comment: ", p1) != wstring::npos) {
					p1 = data.find(L"Comment: ", p1);
					p1 = data.find(L"\n", p1);
					if (data.find(L"Comment: ", p1) != wstring::npos) {
						p1 = data.find(L"Comment: ", p1);
						p1 = data.find(L"\n", p1) + 1;
					}
					else
						p1 += 1;
				}
				else
					p1 += 1;
				p1++;
				wstring::size_type p2 = data.find(L"-----END PGP SIGNATURE-----");

				std::wstring tmp = data.substr(p1, p2 - p1);
				strip_line_term(tmp);
				TiXmlElement* encrypted_data = pDoc->NewElement("x"); node->InsertEndChild(encrypted_data);
				encrypted_data->SetText(tmp.c_str());
				encrypted_data->SetAttribute("xmlns", "jabber:x:signed");
			}
			break;
		}
	}
	return FALSE;
}

static JABBER_HANDLER_FUNC PresenceHandler(IJabberInterface*, TiXmlElement* node, void*)
{
	for (auto *local_node = node->FirstChildElement(); local_node; local_node = local_node->NextSiblingElement()) {
		LPCSTR nodename = local_node->Name();
		if (nodename == nullptr)
			continue;

		if (!mir_strcmp(nodename, "x"))
			continue;

		for (auto *pAttr = local_node->FirstAttribute(); pAttr; pAttr = pAttr->Next()) {
			LPCSTR value = pAttr->Value();
			if (!mir_strcmp(value, "jabber:x:signed")) {
				std::string status_str;
				
				for (auto *local_node2 = node->FirstChildElement(); local_node2; local_node2 = local_node2->NextSiblingElement()) {
					LPCSTR nodename2 = local_node2->Name();
					if (!mir_strcmp(nodename2, "status")) {
						LPCSTR status = local_node2->GetText();
						if (status)
							status_str = status;
						break;
					}
				}

				LPCSTR data = local_node->GetText();
				string sign = "-----BEGIN PGP SIGNATURE-----\n\n";
				wstring file = toUTF16(get_random(10)), status_file = toUTF16(get_random(10));
				sign += data;
				sign += "\n-----END PGP SIGNATURE-----\n";
				wchar_t *path_c = db_get_wsa(0, MODULENAME, "szHomePath", L"");
				wstring path_out = path_c, status_file_out = path_c;
				mir_free(path_c);
				path_out += L"\\tmp\\";
				path_out += file;
				path_out += L".sig";
				status_file_out += L"\\tmp\\";
				status_file_out += status_file;
				status_file_out += L".status";
				
				boost::filesystem::remove(path_out);
				boost::filesystem::remove(status_file_out);
				wfstream f(path_out.c_str(), std::ios::out);
				while (!f.is_open())
					f.open(path_out.c_str(), std::ios::out);
				f << sign.c_str();
				f.close();
				f.open(status_file_out.c_str(), std::ios::out);
				while (!f.is_open())
					f.open(status_file_out.c_str(), std::ios::out);
				f << status_str.c_str();
				f.close();
				if (!boost::filesystem::exists(path_out)) {
					if (globals.bDebugLog)
						globals.debuglog << std::string(time_str() + ": info: Failed to write sign in file");
					return FALSE;
				}
				{ //gpg
					string out;
					DWORD code;
					std::vector<wstring> cmd;
					cmd.push_back(L"--verify");
					cmd.push_back(L"-a");
					cmd.push_back(path_out);
					cmd.push_back(status_file_out);
					gpg_execution_params params(cmd);
					pxResult result;
					params.out = &out;
					params.code = &code;
					params.result = &result;
					if (!gpg_launcher(params, boost::posix_time::seconds(15))) {
						return FALSE;
					}
					if (result == pxNotFound) {
						return FALSE;
					}
					boost::filesystem::remove(path_out);
					boost::filesystem::remove(status_file_out);
					if (out.find("key ID ") != string::npos) {
						//need to get hcontact here, i can get jid from hxml, and get handle from jid, maybe exists better way ?
						string::size_type p1 = out.find("key ID ") + mir_strlen("key ID ");
						string::size_type p2 = out.find("\n", p1);
						if (p1 != string::npos && p2 != string::npos) {
							MCONTACT hContact = NULL;

							for (auto p : globals.Accounts) {
								hContact = p->getJabberInterface()->ContactFromJID(node->Attribute("from"));
								if (hContact)
									globals.hcontact_data[hContact].key_in_prescense = out.substr(p1, p2 - p1 - 1).c_str();
							}
						}
					}
				}
				return FALSE;
			}
		}
	}
	return FALSE;
}

static JABBER_HANDLER_FUNC MessageHandler(IJabberInterface*, TiXmlElement*, void*)
{
	return FALSE;
}

void AddHandlers()
{
	for (auto p : globals.Accounts) {
		/*if (p)
			break;*/
		if (p->getSendHandler() == INVALID_HANDLE_VALUE)
			p->setSendHandler(p->getJabberInterface()->AddSendHandler((JABBER_HANDLER_FUNC)SendHandler));
		
		if (p->getPresenceHandler() == INVALID_HANDLE_VALUE)
			p->setPresenceHandler(p->getJabberInterface()->AddPresenceHandler((JABBER_HANDLER_FUNC)PresenceHandler));

		if (globals.bAutoExchange) {
			p->getJabberInterface()->RegisterFeature("GPG_Key_Auto_Exchange:0", "Indicates that gpg installed and configured to public key auto exchange (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)");
			p->getJabberInterface()->AddFeatures("GPG_Key_Auto_Exchange:0\0\0");
		}
		if (globals.bFileTransfers) {
			p->getJabberInterface()->RegisterFeature("GPG_Encrypted_FileTransfers:0", "Indicates that gpg installed and configured to encrypt files (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)");
			p->getJabberInterface()->AddFeatures("GPG_Encrypted_FileTransfers:0\0\0");
		}
	}
}

bool isContactSecured(MCONTACT hContact)
{
	BYTE gpg_enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
	if (!gpg_enc) {
		if (globals.bDebugLog)
			globals.debuglog << std::string(time_str() + ": encryption is turned off for " + toUTF8(Clist_GetContactDisplayName(hContact)));
		return false;
	}
	if (!db_mc_isMeta(hContact)) {
		wchar_t *key = db_get_wsa(hContact, MODULENAME, "GPGPubKey", L"");
		if (!key[0]) {
			mir_free(key);
			if (globals.bDebugLog)
				globals.debuglog << std::string(time_str() + ": encryption is turned off for " + toUTF8(Clist_GetContactDisplayName(hContact)));
			return false;
		}
		mir_free(key);
	}
	if (globals.bDebugLog)
		globals.debuglog << std::string(time_str() + ": encryption is turned on for " + toUTF8(Clist_GetContactDisplayName(hContact)));
	return true;
}

bool isContactHaveKey(MCONTACT hContact)
{
	wchar_t *key = db_get_wsa(hContact, MODULENAME, "GPGPubKey", L"");
	if (mir_wstrlen(key) > 0) {
		mir_free(key);
		return true;
	}
	mir_free(key);
	return false;
}

bool isGPGKeyExist()
{
	wchar_t *id = db_get_wsa(0, MODULENAME, "KeyID", L"");
	char *key = db_get_sa(0, MODULENAME, "GPGPubKey", "");
	if (id[0] && key[0]) {
		mir_free(id);
		mir_free(key);
		return true;
	}
	mir_free(id);
	mir_free(key);
	return false;
}
bool isGPGValid()
{
	wchar_t *tmp = nullptr;
	bool gpg_exists = false, is_valid = true;
	tmp = db_get_wsa(0, MODULENAME, "szGpgBinPath", L"");
	boost::filesystem::path p(tmp);

	if (boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p))
		gpg_exists = true;
	else {
		mir_free(tmp);
		tmp = nullptr;
		wchar_t *path = (wchar_t*)mir_alloc(sizeof(wchar_t)*MAX_PATH);
		wchar_t *mir_path = (wchar_t*)mir_alloc(MAX_PATH * sizeof(wchar_t));
		PathToAbsoluteW(L"\\", mir_path);
		SetCurrentDirectoryW(mir_path);
		tmp = mir_wstrdup(mir_path);
		mir_free(mir_path);
		//mir_realloc(path, (mir_wstrlen(path)+64)*sizeof(wchar_t));
		wchar_t *gpg_path = (wchar_t*)mir_alloc(sizeof(wchar_t)*MAX_PATH);
		mir_wstrcpy(gpg_path, tmp);
		mir_wstrcat(gpg_path, L"\\GnuPG\\gpg.exe");
		mir_free(tmp);
		tmp = nullptr;
		p = boost::filesystem::path(gpg_path);
		if (boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p)) {
			gpg_exists = true;
			mir_wstrcpy(path, L"GnuPG\\gpg.exe");
		}
		mir_free(gpg_path);
		tmp = mir_wstrdup(path);
		mir_free(path);
	}

	if (gpg_exists) {
		g_plugin.setWString("szGpgBinPath", tmp);
		mir_free(tmp);
		tmp = nullptr;
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
		string::size_type p1 = out.find("(GnuPG) ");
		if (p1 == string::npos)
			is_valid = false;
	}
	if (tmp) {
		mir_free(tmp);
		tmp = nullptr;
	}
	/*	if(!gpg_exists)
		{
			wstring path_ = _wgetenv(L"APPDATA");
			path_ += L"\\GnuPG";
			tmp = db_get_wsa(0, MODULENAME, "szHomePath", (wchar_t*)path_.c_str());
		}
		if(tmp)
			mir_free(tmp); */
	return is_valid && gpg_exists;
}

#define NEWTSTR_MALLOC(A) (A==NULL)?NULL:mir_strcpy((char*)mir_alloc(sizeof(char)*(mir_strlen(A)+1)),A)

const bool StriStr(const char *str, const char *substr)
{
	bool i = false;
	char *str_up = NEWTSTR_MALLOC(str);
	char *substr_up = NEWTSTR_MALLOC(substr);

	CharUpperBuffA(str_up, (DWORD)mir_strlen(str_up));
	CharUpperBuffA(substr_up, (DWORD)mir_strlen(substr_up));

	if (strstr(str_up, substr_up))
		i = true;

	mir_free(str_up);
	mir_free(substr_up);

	return i;
}

bool IsOnline(MCONTACT hContact)
{
	if (g_plugin.getByte(hContact, "Status", 0) == ID_STATUS_OFFLINE)
		return false;
	return true;
}

//from secureim
#include <process.h>

struct TFakeAckParams
{
	inline TFakeAckParams(HANDLE p1, MCONTACT p2, LONG p3, LPCSTR p4) :
		hEvent(p1),
		hContact(p2),
		id(p3),
		msg(p4)
	{
	}

	HANDLE hEvent;
	MCONTACT hContact;
	LONG id;
	LPCSTR msg;
};

__forceinline int SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return ProtoBroadcastAck(GetContactProto(hContact), hContact, type, result, hProcess, lParam);
}

unsigned __stdcall sttFakeAck(void *param)
{
	TFakeAckParams *tParam = (TFakeAckParams*)param;
	WaitForSingleObject(tParam->hEvent, INFINITE);

	Sleep(100);
	if (tParam->msg == nullptr)
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tParam->id, 0);
	else
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tParam->id, LPARAM(tParam->msg));

	CloseHandle(tParam->hEvent);
	delete tParam;

	return 0;
}


int returnNoError(MCONTACT hContact)
{
	HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	unsigned int tID;
	CloseHandle((HANDLE)_beginthreadex(nullptr, 0, sttFakeAck, new TFakeAckParams(hEvent, hContact, 777, nullptr), 0, &tID));
	SetEvent(hEvent);
	return 777;
}
// end from secureim



string toUTF8(wstring str)
{
	string ustr;
	try {
		utf8::utf16to8(str.begin(), str.end(), back_inserter(ustr));
	}
	catch (const utf8::exception& e) {
		if (globals.bDebugLog)
			globals.debuglog << std::string("utf8cpp encoding exception: ") + (char*)e.what();
		//TODO
	}
	return ustr;
}



wstring toUTF16(string str) //convert as much as possible
{
	wstring ustr;
	string tmpstr;
	try {
		utf8::replace_invalid(str.begin(), str.end(), back_inserter(tmpstr));
		utf8::utf8to16(tmpstr.begin(), tmpstr.end(), back_inserter(ustr));
	}
	catch (const utf8::exception& e) {
		if (globals.bDebugLog)
			globals.debuglog << std::string("utf8cpp decoding exception: ") + (char*)e.what();
		//TODO
	}
	return ustr;
}

string get_random(int length)
{
	string chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
	string data;
	boost::random_device rng;
	boost::variate_generator<boost::random_device&, boost::uniform_int<>> gen(rng, boost::uniform_int<>(0, (int)chars.length() - 1));
	for (int i = 0; i < length; ++i)
		data += chars[gen()];
	return data;
}

void send_encrypted_msgs_thread(void *param)
{
	MCONTACT hContact = (MCONTACT)(DWORD_PTR)param;
	while (true) {
		//char *key = db_get_wsa(hContact, MODULENAME, "GPGPubKey", "");
		while (!isContactSecured(hContact))
			Sleep(1000);
		
		if (!globals.hcontact_data[hContact].msgs_to_send.empty()) {
			Sleep(1000);
			list<string>::iterator end = globals.hcontact_data[hContact].msgs_to_send.end();
			extern std::list<HANDLE> sent_msgs;
			for (list<string>::iterator p = globals.hcontact_data[hContact].msgs_to_send.begin(); p != end; ++p) {
				sent_msgs.push_back((HANDLE)ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)p->c_str()));
				HistoryLog(hContact, db_event((char*)p->c_str(), 0, 0, DBEF_SENT));
				Sleep(1000);
			}
			globals.hcontact_data[hContact].msgs_to_send.clear();
			return;
		}
		else
			return;
	}
}

string time_str()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	return (string)boost::posix_time::to_simple_string(now);
}

int handleEnum(const char *szSetting, void *lParam)
{
	if (!*(bool*)lParam && szSetting[0] && StriStr(szSetting, "tabsrmm")) {
		bool f = false, *found = (bool*)lParam;
		f = !db_get_b(0, "PluginDisable", szSetting, 0);
		if (f)
			*found = f;
	}
	return 0;
}

bool isTabsrmmUsed()
{
	bool found = false;
	if (db_enum_settings(NULL, handleEnum, "PluginDisable", &found) == -1)
		return false;

	return found;
}

void ExportGpGKeysFunc(int type)
{
	wchar_t *p = GetFilePath(L"Choose file to export keys", L"*", L"Any file", true);
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
	int exported_keys = 0;
	if (!file.is_open())
		return; //TODO: handle error
	if (!type || type == 2) {
		for (auto &hContact : Contacts()) {
			char *k = db_get_sa(hContact, MODULENAME, "GPGPubKey", "");
			std::string key;
			if (!k[0]) {
				mir_free(k);
				continue;
			}
			else {
				key = k;
				mir_free(k);
			}

			const char *proto = GetContactProto(hContact);
			std::string id = "Comment: login ";
			const char *uid = Proto_GetUniqueId(proto);
			DBVARIANT dbv = { 0 };
			db_get(0, proto, uid, &dbv);
			switch (dbv.type) {
			case DBVT_DELETED:
				continue;

			case DBVT_BYTE:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.bVal);
					id += _id;
				}
				break;
			case DBVT_WORD:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.wVal);
					id += _id;
				}
				break;
			case DBVT_DWORD:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.dVal);
					id += _id;
				}
				break;
			case DBVT_ASCIIZ:
				{
					id += dbv.pszVal;
					db_free(&dbv);
				}
				break;
			case DBVT_UTF8:
				{
					char *tmp = mir_utf8decodeA(dbv.pszVal);
					if (tmp[0])
						id += tmp;
					mir_free(tmp);
					db_free(&dbv);
				}
				break;
			case DBVT_BLOB:
				//TODO
				db_free(&dbv);
				break;
			case DBVT_WCHAR:
				//TODO
				db_free(&dbv);
				break;
			}
			id += " contact_id ";
			memset(&dbv, 0, sizeof(dbv));
			db_get(hContact, proto, uid, &dbv);
			switch (dbv.type) {
			case DBVT_DELETED:
				continue;
			case DBVT_BYTE:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.bVal);
					id += _id;
				}
				break;
			case DBVT_WORD:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.wVal);
					id += _id;
				}
				break;
			case DBVT_DWORD:
				{
					char _id[64];
					mir_snprintf(_id, "%d", dbv.dVal);
					id += _id;
				}
				break;
			case DBVT_ASCIIZ:
				{
					id += dbv.pszVal;
					db_free(&dbv);
				}
				break;
			case DBVT_UTF8:
				{
					char *tmp = mir_utf8decodeA(dbv.pszVal);
					if (tmp[0])
						id += tmp;
					mir_free(tmp);
					db_free(&dbv);
				}
				break;
			case DBVT_BLOB:
				//TODO
				db_free(&dbv);
				break;
			case DBVT_WCHAR:
				//TODO
				db_free(&dbv);
				break;
			}
			std::string::size_type p1 = key.find("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			if (p1 == std::string::npos)
				continue;
			p1 += mir_strlen("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			p1++;
			id += '\n';
			key.insert(p1, id);
			file << key;
			file << std::endl;
			exported_keys++;
		}
	}
	if (type == 1 || type == 2) {
		string out;
		DWORD code;
		pxResult result;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"-export-secret-keys");
		cmd.push_back(L"-a");
		gpg_execution_params params(cmd);
		params.out = &out;
		params.code = &code;
		params.result = &result;
		gpg_launcher(params); //TODO: handle errors
		{
			file << out;
			file << std::endl;
		}
	}
	if (file.is_open())
		file.close();
	wchar_t msg[512];
	if (type == 2)
		mir_snwprintf(msg, TranslateT("We have successfully exported %d public keys and all private keys."), exported_keys);
	else if (type == 1)
		mir_snwprintf(msg, TranslateT("We have successfully exported all private keys."));
	else if (!type)
		mir_snwprintf(msg, TranslateT("We have successfully exported %d public keys."), exported_keys);
	MessageBox(nullptr, msg, TranslateT("Keys export result"), MB_OK);
}

INT_PTR ExportGpGKeys(WPARAM, LPARAM)
{
	ShowExportKeysDlg();
	return 0;
}

INT_PTR ImportGpGKeys(WPARAM, LPARAM)
{
	wchar_t *p = GetFilePath(L"Choose file to import keys from", L"*", L"Any file");
	if (!p || !p[0]) {
		delete[] p;
		//TODO: handle error
		return 1;
	}
	std::ifstream file;
	{
		ptrA szPath(mir_u2a(p));
		delete[] p;

		file.open(szPath, std::ios::in);
	}
	if (!file.is_open())
		return 1; //TODO: handle error

	int processed_keys = 0, processed_private_keys = 0;

	char line[256];
	file.getline(line, 255);
	if (!strstr(line, "-----BEGIN PGP PUBLIC KEY BLOCK-----") && !strstr(line, "-----BEGIN PGP PRIVATE KEY BLOCK-----"))
		return 1; //TODO: handle error

	std::string key, login, contact_id;
	key += line;
	key += '\n';
	while (file.is_open() && !file.eof()) {
		file.getline(line, 255);
		key += line;
		key += '\n';
		if (strstr(line, "-----END PGP PUBLIC KEY BLOCK-----")) {
			std::string::size_type p1 = key.rfind("Comment: login "), p2 = 0;
			if (p1 == std::string::npos)
			{
				key.clear();
				continue; //TODO: warning about malformed file
			}
			p1 += mir_strlen("Comment: login ");
			p2 = key.rfind(" contact_id ");
			login = key.substr(p1, p2 - p1);
			p2 += mir_strlen(" contact_id ");
			p1 = key.find("\n", p2);
			contact_id = key.substr(p2, p1 - p2);
			p1 = key.rfind("Comment: login ");
			p2 = key.find("\n", p1);
			p2++;
			key.erase(p1, p2 - p1);
			std::string acc;

			for (auto &pa : Accounts()) {
				if (acc.length())
					break;
				const char *uid = Proto_GetUniqueId(pa->szModuleName);
				DBVARIANT dbv = { 0 };
				db_get(0, pa->szModuleName, uid, &dbv);
				std::string id;
				switch (dbv.type) {
				case DBVT_DELETED:
					continue;
					break;

				case DBVT_BYTE:
					char _id[64];
					mir_snprintf(_id, "%d", dbv.bVal);
					id += _id;
					if (id == login)
						acc = pa->szModuleName;
					break;

				case DBVT_WORD:
					mir_snprintf(_id, "%d", dbv.wVal);
					id += _id;
					if (id == login)
						acc = pa->szModuleName;
					break;

				case DBVT_DWORD:
					mir_snprintf(_id, "%d", dbv.dVal);
					id += _id;
					if (id == login)
						acc = pa->szModuleName;
					break;

				case DBVT_ASCIIZ:
					id += dbv.pszVal;
					db_free(&dbv);
					if (id == login)
						acc = pa->szModuleName;
					break;

				case DBVT_UTF8:
					{
						char *tmp = mir_utf8decodeA(dbv.pszVal);
						if (tmp[0])
							id += tmp;
						mir_free(tmp);
						db_free(&dbv);
						if (id == login)
							acc = pa->szModuleName;
					}
					break;

				case DBVT_BLOB:
					//TODO
					db_free(&dbv);
					break;

				case DBVT_WCHAR:
					//TODO
					db_free(&dbv);
					break;
				}
			}

			if (acc.length()) {
				const char *uid = Proto_GetUniqueId(acc.c_str());
				for (auto &hContact : Contacts(acc.c_str())) {
					DBVARIANT dbv = { 0 };
					db_get(hContact, acc.c_str(), uid, &dbv);
					std::string id;
					bool found = false;
					switch (dbv.type) {
					case DBVT_DELETED:
						continue;
						break;
					case DBVT_BYTE:
						{
							char _id[64];
							mir_snprintf(_id, "%d", dbv.bVal);
							id += _id;
							if (id == contact_id)
								found = true;
						}
						break;
					case DBVT_WORD:
						{
							char _id[64];
							mir_snprintf(_id, "%d", dbv.wVal);
							id += _id;
							if (id == contact_id)
								found = true;
						}
						break;
					case DBVT_DWORD:
						{
							char _id[64];
							mir_snprintf(_id, "%d", dbv.dVal);
							id += _id;
							if (id == contact_id)
								found = true;
						}
						break;
					case DBVT_ASCIIZ:
						{
							id += dbv.pszVal;
							db_free(&dbv);
							if (id == contact_id)
								found = true;
						}
						break;
					case DBVT_UTF8:
						{
							char *tmp = mir_utf8decodeA(dbv.pszVal);
							if (tmp[0])
								id += tmp;
							mir_free(tmp);
							db_free(&dbv);
							if (id == contact_id)
								found = true;
						}
						break;
					case DBVT_BLOB:
						//TODO
						db_free(&dbv);
						break;
					case DBVT_WCHAR:
						//TODO
						db_free(&dbv);
						break;
					}
					if (found) {
						wstring path;
						std::vector<std::wstring> cmd;
						wchar_t *ptmp;
						string output;
						DWORD exitcode;
						{
							ptmp = db_get_wsa(0, MODULENAME, "szHomePath", L"");
							path = ptmp;
							mir_free(ptmp);
							wstring rand = toUTF16(get_random(10));
							path += L"\\";
							path += rand;
							boost::filesystem::remove(path);
							wfstream f(path, std::ios::out);
							f << toUTF16(key).c_str();
							f.close();
							cmd.push_back(L"--batch");
							cmd.push_back(L"--import");
							cmd.push_back(path);
						}
						gpg_execution_params params(cmd);
						pxResult result;
						params.out = &output;
						params.code = &exitcode;
						params.result = &result;
						if (!gpg_launcher(params))
							break;
						if (result == pxNotFound)
							break;
						if (result == pxSuccess)
							processed_keys++;
						{
							if (output.find("already in secret keyring") != string::npos) {
								MessageBox(nullptr, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
								boost::filesystem::remove(path);
								break;
							}
							char *tmp2;
							string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
							string::size_type s2 = output.find(":", s);
							tmp2 = (char*)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
							mir_strcpy(tmp2, output.substr(s, s2 - s).c_str());
							mir_utf8decode(tmp2, nullptr);
							g_plugin.setString(hContact, "KeyID", tmp2);
							mir_free(tmp2);
							s = output.find("“", s2);
							if (s == string::npos) {
								s = output.find("\"", s2);
								s += 1;
							}
							else
								s += 3;
							if ((s2 = output.find("(", s)) == string::npos)
								s2 = output.find("<", s);
							else if (s2 > output.find("<", s))
								s2 = output.find("<", s);
							if (s2 != string::npos) {
								tmp2 = (char*)mir_alloc((output.substr(s, s2 - s - 1).length() + 1) * sizeof(char));
								mir_strcpy(tmp2, output.substr(s, s2 - s - 1).c_str());
								mir_utf8decode(tmp2, nullptr);
								if (hContact) {
									g_plugin.setString(hContact, "KeyMainName", output.substr(s, s2 - s - 1).c_str());
								}
								mir_free(tmp2);
								if ((s = output.find(")", s2)) == string::npos)
									s = output.find(">", s2);
								else if (s > output.find(">", s2))
									s = output.find(">", s2);
								s2++;
								if (output[s] == ')') {
									tmp2 = (char*)mir_alloc((output.substr(s2, s - s2).length() + 1) * sizeof(char));
									mir_strcpy(tmp2, output.substr(s2, s - s2).c_str());
									mir_utf8decode(tmp2, nullptr);
									if (hContact)
										g_plugin.setString(hContact, "KeyComment", output.substr(s2, s - s2).c_str());
									mir_free(tmp2);
									s += 3;
									s2 = output.find(">", s);
									tmp2 = (char*)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
									mir_strcpy(tmp2, output.substr(s, s2 - s).c_str());
									mir_utf8decode(tmp2, nullptr);
									if (hContact)
										g_plugin.setString(hContact, "KeyMainEmail", output.substr(s, s2 - s).c_str());
									mir_free(tmp2);
								}
								else {
									tmp2 = (char*)mir_alloc((output.substr(s2, s - s2).length() + 1) * sizeof(char));
									mir_strcpy(tmp2, output.substr(s2, s - s2).c_str());
									mir_utf8decode(tmp2, nullptr);
									if (hContact)
										g_plugin.setString(hContact, "KeyMainEmail", output.substr(s2, s - s2).c_str());
									mir_free(tmp2);
								}
							}
							g_plugin.setByte(hContact, "GPGEncryption", 1);
							g_plugin.setWString(hContact, "GPGPubKey", toUTF16(key).c_str());
						}
						boost::filesystem::remove(path);
						break;
					}
				}
			}
			key.clear();
		}
		else if (strstr(line, "-----END PGP PRIVATE KEY BLOCK-----")) {
			std::vector<wstring> cmd;
			wchar_t tmp2[MAX_PATH] = { 0 };
			wchar_t *ptmp;
			string output;
			DWORD exitcode;
			{
				ptmp = db_get_wsa(0, MODULENAME, "szHomePath", L"");
				wcsncpy(tmp2, ptmp, MAX_PATH - 1);
				mir_free(ptmp);
				mir_wstrncat(tmp2, L"\\", _countof(tmp2) - mir_wstrlen(tmp2));
				mir_wstrncat(tmp2, L"temporary_exported.asc", _countof(tmp2) - mir_wstrlen(tmp2));
				boost::filesystem::remove(tmp2);
				wfstream f(tmp2, std::ios::out);
				f << toUTF16(key).c_str();
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
				break;
			if (result == pxNotFound)
				break;
			if (result == pxSuccess)
				processed_private_keys++;
			key.clear();
		}
	}
	if (file.is_open())
		file.close();
	wchar_t msg[512];
	if (processed_private_keys)
		mir_snwprintf(msg, TranslateT("We have successfully processed %d public keys and some private keys."), processed_keys);
	else
		mir_snwprintf(msg, TranslateT("We have successfully processed %d public keys."), processed_keys);
	MessageBox(nullptr, msg, TranslateT("Keys import result"), MB_OK);
	return 0;
}

void fix_line_term(std::string &s)
{
	if (s.empty())
		return;
	boost::algorithm::erase_all(s, "\r\r");
}

void fix_line_term(std::wstring &s)
{
	if (s.empty())
		return;
	boost::algorithm::erase_all(s, L"\r\r");
}

void strip_line_term(std::wstring &s)
{
	if (s.empty())
		return;
	boost::algorithm::erase_all(s, L"\r");
	boost::algorithm::erase_all(s, L"\n");
}

void strip_line_term(std::string &s)
{
	if (s.empty())
		return;
	boost::algorithm::erase_all(s, "\r");
	boost::algorithm::erase_all(s, "\n");
}

void strip_tags(std::wstring &str)
{
	if (str.empty())
		return;
	boost::algorithm::erase_all(str, globals.inopentag);
	boost::algorithm::erase_all(str, globals.inclosetag);
	boost::algorithm::erase_all(str, globals.outopentag);
	boost::algorithm::erase_all(str, globals.outclosetag);
}




void ShowEncryptedFileMsgBox()
{
	CDlgEncryptedFileMsgBox *d = new CDlgEncryptedFileMsgBox;
	d->DoModal();
}




void ShowExportKeysDlg()
{
	CDlgExportKeysMsgBox *d = new CDlgExportKeysMsgBox;
	d->Show();
}




void ShowChangePasswdDlg()
{
	CDlgChangePasswdMsgBox *d = new CDlgChangePasswdMsgBox;
	d->DoModal();
}

void clean_temp_dir()
{
	using namespace boost::filesystem;
	char *mir_path = new char[MAX_PATH];
	PathToAbsolute("\\", mir_path);
	wstring path = toUTF16(mir_path);
	SetCurrentDirectoryA(mir_path);
	delete[] mir_path;
	wchar_t *tmp = db_get_wsa(0, MODULENAME, "szHomePath", L"");
	path += tmp;
	mir_free(tmp);
	path += L"\\tmp";
	if (exists(path) && is_directory(path)) {
		boost::filesystem::path p(path);
		for (directory_iterator i = directory_iterator(p), end = directory_iterator(); i != end; ++i) {
			if (boost::filesystem::is_regular_file(i->path())) {
				if ((i->path().filename().generic_string().length() == 10 && (i->path().filename().generic_string().find(".") == std::string::npos)) ||
					i->path().extension() == ".sig" || i->path().extension() == ".asc" || i->path().extension() == ".status")
					boost::filesystem::remove(i->path());
			}
		}
	}
}

bool gpg_validate_paths(wchar_t *gpg_bin_path, wchar_t *gpg_home_path)
{
	wstring tmp = gpg_bin_path;
	if (!tmp.empty()) {
		wchar_t mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		SetCurrentDirectoryW(mir_path);
		if (!boost::filesystem::exists(tmp)) {
			MessageBox(nullptr, TranslateT("GPG binary does not exist.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
			return false;
		}
	}
	else {
		MessageBox(nullptr, TranslateT("Please choose GPG binary location"), TranslateT("Warning"), MB_OK);
		return false;
	}
	{
		bool bad_version = false;
		g_plugin.setWString("szGpgBinPath", tmp.c_str());
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
		g_plugin.delSetting("szGpgBinPath");
		string::size_type p1 = out.find("(GnuPG) ");
		if (p1 != string::npos) {
			p1 += mir_strlen("(GnuPG) ");
			if (out[p1] != '1')
				bad_version = true;
		}
		else {
			bad_version = false;
			MessageBox(nullptr, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
			return false;
		}
		if (bad_version)
			MessageBox(nullptr, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
	}
	tmp = gpg_home_path;
	if (tmp[tmp.length()] == '\\')
		tmp.erase(tmp.length());
	if (tmp.empty()) {
		MessageBox(nullptr, TranslateT("Please set keyring's home directory"), TranslateT("Warning"), MB_OK);
		return false;
	}
	{
		wchar_t *path = db_get_wsa(0, MODULENAME, "szHomePath", L"");
		DWORD dwFileAttr = GetFileAttributes(path);
		if (dwFileAttr != INVALID_FILE_ATTRIBUTES) {
			dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(path, dwFileAttr);
		}
		mir_free(path);
	}
	return true;
}

void gpg_save_paths(wchar_t *gpg_bin_path, wchar_t *gpg_home_path)
{
	g_plugin.setWString("szGpgBinPath", gpg_bin_path);
	g_plugin.setWString("szHomePath", gpg_home_path);
}

bool gpg_use_new_random_key(char *account_name, wchar_t *gpg_bin_path, wchar_t *gpg_home_dir)
{
	if (gpg_bin_path && gpg_home_dir)
		gpg_save_paths(gpg_bin_path, gpg_home_dir);
	
	wstring path;
	{
		// generating key file
		wchar_t *tmp = nullptr;
		if (gpg_home_dir)
			tmp = gpg_home_dir;
		else
			tmp = db_get_wsa(0, MODULENAME, "szHomePath", L"");
		path = tmp;
		if (!gpg_home_dir)
			mir_free(tmp);
		path.append(L"\\new_key");
		wfstream f(path.c_str(), std::ios::out);
		if (!f.is_open()) {
			MessageBox(nullptr, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
			return false;
		}
		f << "Key-Type: RSA";
		f << "\n";
		f << "Key-Length: 4096";
		f << "\n";
		f << "Subkey-Type: RSA";
		f << "\n";
		f << "Name-Real: ";
		f << get_random(6).c_str();
		f << "\n";
		f << "Name-Email: ";
		f << get_random(5).c_str();
		f << "@";
		f << get_random(5).c_str();
		f << ".";
		f << get_random(3).c_str();
		f << "\n";
		f.close();
	}
	{	// gpg execution
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
		if (!gpg_launcher(params, boost::posix_time::minutes(10)))
			return false;
		if (result == pxNotFound)
			return false;

		boost::filesystem::remove(path);
		string::size_type p1 = 0;
		if ((p1 = out.find("key ")) != string::npos)
			path = toUTF16(out.substr(p1 + 4, 8));
		else
			path.clear();
	}

	if (!path.empty()) {
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--batch");
		cmd.push_back(L"-a");
		cmd.push_back(L"--export");
		cmd.push_back(path);
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		if (!gpg_launcher(params))
			return false;

		if (result == pxNotFound)
			return false;

		string::size_type s = 0;
		while ((s = out.find("\r", s)) != string::npos)
			out.erase(s, 1);

		if (!mir_strcmp(account_name, Translate("Default"))) {
			g_plugin.setString("GPGPubKey", out.c_str());
			g_plugin.setWString("KeyID", path.c_str());
		}
		else {
			std::string acc_str = account_name;
			acc_str += "_GPGPubKey";
			g_plugin.setString(acc_str.c_str(), out.c_str());
			acc_str = account_name;
			acc_str += "_KeyID";
			g_plugin.setWString(acc_str.c_str(), path.c_str());
		}
	}
	return true;
}
