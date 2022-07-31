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

#include "utf8.h"

void GetFilePath(wchar_t *WindowTittle, char *szSetting, wchar_t *szExt, wchar_t *szExtDesc)
{
	wchar_t str[MAX_PATH + 2] = {};
	OPENFILENAME ofn = {};
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
	wcsncpy(str, g_plugin.getMStringW(szSetting), _countof(str) - 1);
	if (mir_wstrlen(str) < 2)
		str[0] = '\0';
	ofn.lpstrFile = str;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	if (GetOpenFileName(&ofn))
		g_plugin.setWString(szSetting, str);
}

wchar_t* GetFilePath(wchar_t *WindowTittle, wchar_t *szExt, wchar_t *szExtDesc, bool save_file)
{
	wchar_t str[MAX_PATH + 1];
	OPENFILENAME ofn = {};
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
		if (!GetOpenFileName(&ofn))
			return nullptr;
	}
	else {
		if (!GetSaveFileName(&ofn))
			return nullptr;
	}
	return mir_wstrdup(str);
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
	ShowLoadPublicKeyDialog((MCONTACT)w, false);
	return 0;
}

INT_PTR SendKey(WPARAM w, LPARAM)
{
	MCONTACT hContact = db_mc_tryMeta(w);
	std::string key_id_str;

	LPSTR proto = Proto_GetBaseAccountName(hContact);
	PROTOACCOUNT *acc = Proto_GetAccount(proto);
	std::string acc_str;
	if (acc) {
		acc_str = acc->szModuleName;
		key_id_str = acc_str;
		key_id_str += "_KeyID";
		acc_str += "_GPGPubKey";
	}

	CMStringA szMessage = g_plugin.getMStringA(acc_str.empty() ? "GPGPubKey" : acc_str.c_str());
	if (szMessage.IsEmpty())
		szMessage = g_plugin.getMStringA("GPGPubKey"); //try to get default key as fallback in any way

	if (!szMessage.IsEmpty()) {
		uint8_t enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
		g_plugin.setByte(hContact, "GPGEncryption", 0);
		ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)szMessage.c_str());
		std::string msg = "Public key ";
		CMStringA keyid = g_plugin.getMStringA(key_id_str.c_str());
		if (keyid.IsEmpty())
			keyid = g_plugin.getMStringA("KeyID");
		msg += keyid;
		msg += " sent";

		HistoryLog(hContact, msg.c_str(), DBEF_SENT);
		g_plugin.setByte(hContact, "GPGEncryption", enc);
	}

	return 0;
}

INT_PTR ToggleEncryption(WPARAM w, LPARAM)
{
	MCONTACT hContact = (MCONTACT)w;
	uint8_t enc;
	if (db_mc_isMeta(hContact)) {
		enc = g_plugin.getByte(metaGetMostOnline(hContact), "GPGEncryption");
		if (MessageBox(nullptr, TranslateT("Do you want to toggle encryption for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES) {
			int count = db_mc_getSubCount(hContact);
			for (int i = 0; i < count; i++) {
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if (hcnt)
					g_plugin.getByte(hcnt, "GPGEncryption", enc ? 0 : 1);
			}
			g_plugin.setByte(hContact, "GPGEncryption", enc ? 0 : 1);
		}
	}
	else {
		enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
		g_plugin.setByte(hContact, "GPGEncryption", enc ? 0 : 1);
	}
	setSrmmIcon(hContact);
	return 0;
}

int OnPreBuildContactMenu(WPARAM w, LPARAM)
{
	MCONTACT hContact = db_mc_tryMeta(w);
	{
		CMenuItem mi2(&g_plugin);
		LPSTR proto = Proto_GetBaseAccountName(hContact);
		PROTOACCOUNT *acc = Proto_GetAccount(proto);
		std::string setting;
		if (acc) {
			setting = acc->szModuleName;
			setting += "_KeyID";
		}
		
		CMStringA keyid = g_plugin.getMStringA(setting.c_str());
		if (keyid.IsEmpty())
			keyid = g_plugin.getMStringA("KeyID");

		wchar_t buf[128] = { 0 };
		mir_snwprintf(buf, L"%s: %S", TranslateT("Send public key"), keyid.c_str());
		Menu_ModifyItem(g_plugin.hSendKey, buf);
	}

	int flags;
	CMStringA tmp = g_plugin.getMStringW(hContact, "GPGPubKey");
	if (tmp.IsEmpty()) {
		g_plugin.delSetting(hContact, "GPGEncryption");
		flags = CMIF_GRAYED;
	}
	else flags = 0;

	if (g_plugin.getByte(hContact, "GPGEncryption"))
		Menu_ModifyItem(g_plugin.hToggleEncryption, LPGENW("Turn off GPG encryption"), g_plugin.getIconHandle(IDI_SECURED), flags);
	else
		Menu_ModifyItem(g_plugin.hToggleEncryption, LPGENW("Turn on GPG encryption"), g_plugin.getIconHandle(IDI_UNSECURED), flags);
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
						if (!g_plugin.bFileTransfers && !g_plugin.bSameAction) {
							void ShowEncryptedFileMsgBox();
							ShowEncryptedFileMsgBox();
						}
						if (!g_plugin.bFileTransfers && g_plugin.bSameAction)
							return 0;
						if (!globals.bDecryptFiles)
							return 0;
						HistoryLog(ack->hContact, "Received encrypted file, trying to decrypt");
						if (!boost::filesystem::exists(f->szCurrentFile.w))
							return 0;

						gpg_execution_params params;
						params.addParam(L"-o");
						wstring file = filename;
						wstring::size_type p1 = file.rfind(L".gpg");
						file.erase(p1, mir_wstrlen(L".gpg"));
						if (boost::filesystem::exists(file)) {
							if (MessageBox(nullptr, TranslateT("Target file exists, do you want to replace it?"), TranslateT("Warning"), MB_YESNO) == IDNO)
								return 0;
						}
						params.addParam(file);
						boost::filesystem::remove(file);
						{
							// password
							CMStringW pass;
							CMStringA keyid = g_plugin.getMStringA(ack->hContact, "KeyID");
							if (!keyid.IsEmpty()) {
								string dbsetting = "szKey_";
								dbsetting += keyid;
								dbsetting += "_Password";
								pass = g_plugin.getMStringW(dbsetting.c_str());
								if (!pass.IsEmpty() && globals.debuglog)
									globals.debuglog << "info: found password in database for key ID: " + string(keyid.c_str()) + ", trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password";
							}
							else {
								pass = g_plugin.getMStringW("szKeyPassword");
								if (!pass.IsEmpty() && globals.debuglog)
									globals.debuglog << "info: found password for all keys in database, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password";
							}
							if (!pass.IsEmpty()) {
								params.addParam(L"--passphrase");
								params.addParam(pass.c_str());
							}
							else if (!globals.wszPassword.IsEmpty()) {
								if (globals.debuglog)
									globals.debuglog << "info: found password in memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " with password";
								params.addParam(L"--passphrase");
								params.addParam(globals.wszPassword.c_str());
							}
							else if (globals.debuglog)
								globals.debuglog << "info: passwords not found in database or memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " without password";
						}
						params.addParam(L"-d");
						params.addParam(filename);
						if (!gpg_launcher(params, boost::posix_time::minutes(15)))
							return 0;

						string out(params.out);
						while (out.find("public key decryption failed: bad passphrase") != string::npos) {
							if (globals.debuglog)
								globals.debuglog << "info: failed to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact)) + " password needed, trying to get one";
							if (globals._terminate)
								break;
							{ //save inkey id
								string::size_type s = out.find(" encrypted with ");
								s = out.find(" ID ", s);
								s += mir_strlen(" ID ");
								string::size_type s2 = out.find(",", s);
								if (db_mc_isMeta(ack->hContact))
									g_plugin.setString(metaGetMostOnline(ack->hContact), "InKeyID", out.substr(s, s2 - s).c_str());
								else
									g_plugin.setString(ack->hContact, "InKeyID", out.substr(s, s2 - s).c_str());
							}
							
							CDlgKeyPasswordMsgBox(ack->hContact).DoModal();

							if (!globals.wszPassword.IsEmpty()) {
								if (globals.debuglog)
									globals.debuglog << "info: found password in memory, trying to decrypt message from " + toUTF8(Clist_GetContactDisplayName(ack->hContact));

								params.addParam(L"--passphrase");
								params.addParam(globals.wszPassword.c_str());
							}

							if (!gpg_launcher(params, boost::posix_time::seconds(15)))
								return 0;
							if (params.result == pxNotFound)
								return 0;
						}
						if (params.result == pxSuccess)
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
					HistoryLog(ack->hContact, "Failed to send encrypted message");
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
	MCONTACT hcnt = db_mc_isMeta(hContact) ? metaGetMostOnline(hContact) : hContact;
	
	gpg_execution_params params;
	params.addParam(L"--batch");
	params.addParam(L"--tes");
	params.addParam(L"-r");
	
	CMStringW keyid = g_plugin.getMStringW(hcnt, "KeyID");
	wchar_t *name = wcsrchr(filename, '\\');
	if (!name)
		name = filename;
	else
		name++;
	wchar_t *file_out = new wchar_t[mir_wstrlen(name) + mir_wstrlen(L".gpg") + 1];
	mir_snwprintf(file_out, mir_wstrlen(name) + mir_wstrlen(L".gpg") + 1, L"%s.gpg", name);
	params.addParam(keyid.c_str());

	if (g_plugin.getByte(hcnt, "bAlwaysTrust")) {
		params.addParam(L"--trust-model");
		params.addParam(L"always");
	}

	params.addParam(L"-o");
	wchar_t *temp = _tgetenv(L"TEMP");
	params.addParam(wstring(temp) + L"\\" + file_out);
	wstring path_out = temp;
	path_out += L"\\";
	path_out += file_out;
	boost::filesystem::remove(path_out);
	params.addParam(L"-e");
	params.addParam(filename);
	delete[] file_out;

	if (!gpg_launcher(params, boost::posix_time::minutes(3)))
		return nullptr;

	if (params.out.Find("There is no assurance this key belongs to the named user") != -1) {
		if (IDYES != MessageBox(nullptr, TranslateT("We're trying to encrypt with untrusted key. Do you want to trust this key permanently?"), TranslateT("Warning"), MB_YESNO))
			return nullptr;
			
		g_plugin.setByte(hcnt, "bAlwaysTrust", 1);

		params.addParam(L"--trust-model");
		params.addParam(L"always");
		if (!gpg_launcher(params, boost::posix_time::minutes(3)))
			return nullptr;
	}
	return path_out;
}

//from secureim partially
INT_PTR onSendFile(WPARAM w, LPARAM l)
{
	CCSDATA *ccs = (CCSDATA*)l;
	if (!g_plugin.bFileTransfers)
		return Proto_ChainSend(w, ccs);

	if (isContactSecured(ccs->hContact)) {
		char *proto = Proto_GetBaseAccountName(ccs->hContact);
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
		HistoryLog(ccs->hContact, TranslateU("encrypting file for transfer"), DBEF_SENT);
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

void HistoryLog(MCONTACT hContact, const char *msg, uint32_t _time, uint32_t flags)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.flags = DBEF_UTF | flags;
	dbei.timestamp = (_time) ? _time : (uint32_t)time(0);
	dbei.cbBlob = (uint32_t)mir_strlen(msg) + 1;
	dbei.pBlob = (uint8_t*)msg;
	db_event_add(hContact, &dbei);
}

static int ControlAddStringUtf(HWND ctrl, uint32_t msg, const wchar_t *szString)
{
	int item = -1;
	item = SendMessage(ctrl, msg, 0, (LPARAM)szString);
	return item;
}

int ComboBoxAddStringUtf(HWND hCombo, const wchar_t *szString, uint32_t data)
{
	int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
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
		if (g_plugin.bPresenceSigning && nodename && strstr(nodename, "status")) {
			string path_out = ptrA(g_plugin.getUStringA("szHomePath", ""));
			string file = get_random(10);
			path_out += "\\tmp\\";
			path_out += file;
			boost::filesystem::remove(path_out);
			wfstream f(path_out.c_str(), std::ios::out);
			f << str;
			f.close();
			if (!boost::filesystem::exists(path_out)) {
				if (globals.debuglog)
					globals.debuglog << "info: Failed to write prescense in file";
				break;
			}

			gpg_execution_params params;
			{
				char setting[64];
				mir_snprintf(setting, sizeof(setting) - 1, "%s_KeyID", ji->GetModuleName());
				CMStringA inkeyid = g_plugin.getMStringA(setting);
				if (inkeyid.IsEmpty())
					inkeyid = g_plugin.getMStringA("KeyID");

				CMStringW pass;
				if (!inkeyid.IsEmpty()) {
					string dbsetting = "szKey_";
					dbsetting += inkeyid;
					dbsetting += "_Password";
					pass = g_plugin.getMStringW(dbsetting.c_str());
					if (!pass.IsEmpty() && globals.debuglog)
						globals.debuglog << "info: found password in database for key ID: " + string(inkeyid.c_str()) + ", trying to encrypt message from self with password";
				}
				else {
					pass = g_plugin.getMStringW("szKeyPassword");
					if (!pass.IsEmpty() && globals.debuglog)
						globals.debuglog << "info: found password for all keys in database, trying to encrypt message from self with password";
				}
				if (pass[0]) {
					params.addParam(L"--passphrase");
					params.addParam(pass.c_str());
				}
				else if (!globals.wszPassword.IsEmpty()) {
					if (globals.debuglog)
						globals.debuglog << "info: found password in memory, trying to encrypt message from self with password";
					params.addParam(L"--passphrase");
					params.addParam(globals.wszPassword.c_str());
				}
				else if (globals.debuglog)
					globals.debuglog << "info: passwords not found in database or memory, trying to encrypt message from self without password";
			}

			params.addParam(L"--local-user");
			params.addParam(g_plugin.getMStringW("KeyID").c_str());
			params.addParam(L"--default-key");
			params.addParam(g_plugin.getMStringW("KeyID").c_str());
			params.addParam(L"--batch");
			params.addParam(L"--yes");
			params.addParam(L"-abs");
			params.addParam(Utf2T(path_out.c_str()).get());
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
				if (globals.debuglog)
					globals.debuglog << "info: Failed to read prescense sign from file";
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

static JABBER_HANDLER_FUNC PresenceHandler(IJabberInterface *ji, TiXmlElement* node, void*)
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
				
				CMStringW path_out = g_plugin.getMStringW("szHomePath"), status_file_out = path_out;
				path_out += L"\\tmp\\";
				path_out += file.c_str();
				path_out += L".sig";
				status_file_out += L"\\tmp\\";
				status_file_out += status_file.c_str();
				status_file_out += L".status";
				
				boost::filesystem::remove(path_out.c_str());
				boost::filesystem::remove(status_file_out.c_str());
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
				if (!boost::filesystem::exists(path_out.c_str())) {
					if (globals.debuglog)
						globals.debuglog << "info: Failed to write sign in file";
					return FALSE;
				}
				{
					// gpg
					gpg_execution_params params;
					params.addParam(L"--verify");
					params.addParam(L"-a");
					params.addParam(path_out.c_str());
					params.addParam(status_file_out.c_str());
					if (!gpg_launcher(params, boost::posix_time::seconds(15)))
						return FALSE;
					if (params.result == pxNotFound)
						return FALSE;

					boost::filesystem::remove(path_out.c_str());
					boost::filesystem::remove(status_file_out.c_str());

					string out(params.out);
					if (out.find("key ID ") != string::npos) {
						//need to get hcontact here, i can get jid from hxml, and get handle from jid, maybe exists better way ?
						string::size_type p1 = out.find("key ID ") + mir_strlen("key ID ");
						string::size_type p2 = out.find("\n", p1);
						if (p1 != string::npos && p2 != string::npos) {
							MCONTACT hContact = ji->ContactFromJID(node->Attribute("from"));
							if (hContact)
								globals.hcontact_data[hContact].key_in_prescense = out.substr(p1, p2 - p1 - 1).c_str();
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

int GetJabberInterface(WPARAM, LPARAM) //get interface for all jabber accounts, options later
{
	list <JabberAccount *>::iterator p;
	globals.Accounts.clear();

	int accNum = 0;
	for (auto &pa : Accounts()) {
		IJabberInterface *pApi = getJabberApi(pa->szModuleName);
		if (pApi == nullptr)
			continue;

		auto *pAcc = new JabberAccount();
		pAcc->setJabberInterface(pApi);
		if (pa->tszAccountName)
			pAcc->setAccountName(mir_wstrdup(pa->tszAccountName));
		else
			pAcc->setAccountName(mir_a2u(pa->szModuleName));

		pAcc->setAccountNumber(accNum++);
		pAcc->setSendHandler(pApi->AddSendHandler((JABBER_HANDLER_FUNC)SendHandler));
		pAcc->setPresenceHandler(pApi->AddPresenceHandler((JABBER_HANDLER_FUNC)PresenceHandler));

		if (g_plugin.bAutoExchange) {
			pApi->RegisterFeature("GPG_Key_Auto_Exchange:0", "Indicates that gpg installed and configured to public key auto exchange (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)");
			pApi->AddFeatures("GPG_Key_Auto_Exchange:0\0\0");
		}
		if (g_plugin.bFileTransfers) {
			pApi->RegisterFeature("GPG_Encrypted_FileTransfers:0", "Indicates that gpg installed and configured to encrypt files (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)");
			pApi->AddFeatures("GPG_Encrypted_FileTransfers:0\0\0");
		}

		globals.Accounts.push_back(pAcc);
	}

	return 0;
}

void RemoveHandlers()
{
	for (auto &it : globals.Accounts) {
		auto *pApi = it->getJabberInterface();
		if (pApi == nullptr)
			continue;

		if (it->getMessageHandler() != INVALID_HANDLE_VALUE)
			pApi->RemoveHandler(it->getMessageHandler());
		if (it->getPresenceHandler() != INVALID_HANDLE_VALUE)
			pApi->RemoveHandler(it->getPresenceHandler());
		pApi->RemoveFeatures("GPG_Encrypted_FileTransfers:0\0\0");
		pApi->RemoveFeatures("GPG_Key_Auto_Exchange:0\0\0");
	}
}

bool isContactSecured(MCONTACT hContact)
{
	uint8_t gpg_enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
	if (!gpg_enc) {
		if (globals.debuglog)
			globals.debuglog << "encryption is turned off for " + toUTF8(Clist_GetContactDisplayName(hContact));
		return false;
	}
	if (!db_mc_isMeta(hContact)) {
		CMStringW key = g_plugin.getMStringW(hContact, "GPGPubKey");
		if (key.IsEmpty()) {
			if (globals.debuglog)
				globals.debuglog << "encryption is turned off for " + toUTF8(Clist_GetContactDisplayName(hContact));
			return false;
		}
	}
	if (globals.debuglog)
		globals.debuglog << "encryption is turned on for " + toUTF8(Clist_GetContactDisplayName(hContact));
	return true;
}

bool isContactHaveKey(MCONTACT hContact)
{
	ptrW key(g_plugin.getWStringA(hContact, "GPGPubKey"));
	return (mir_wstrlen(key) > 0);
}

bool isGPGKeyExist()
{
	CMStringW id(g_plugin.getMStringW("KeyID"));
	CMStringA key(g_plugin.getMStringA("GPGPubKey"));
	return (!id.IsEmpty() && !key.IsEmpty());
}

bool isGPGValid()
{
	ptrW tmp(g_plugin.getWStringA("szGpgBinPath", L""));
	bool gpg_exists = false, is_valid = true;
	boost::filesystem::path p(tmp);

	if (boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p))
		gpg_exists = true;
	else {
		wchar_t path[MAX_PATH], mir_path[MAX_PATH];
		PathToAbsoluteW(L"\\", mir_path);
		SetCurrentDirectoryW(mir_path);

		//mir_realloc(path, (mir_wstrlen(path)+64)*sizeof(wchar_t));
		wchar_t gpg_path[MAX_PATH];
		mir_wstrcpy(gpg_path, mir_path);
		mir_wstrcat(gpg_path, L"\\GnuPG\\gpg.exe");

		p = boost::filesystem::path(gpg_path);
		if (boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p)) {
			gpg_exists = true;
			mir_wstrcpy(path, L"GnuPG\\gpg.exe");
		}
		tmp = mir_wstrdup(path);
	}

	if (gpg_exists) {
		g_plugin.setWString("szGpgBinPath", tmp);

		gpg_execution_params params;
		params.addParam(L"--version");
		bool _gpg_valid = globals.gpg_valid;
		globals.gpg_valid = true;
		gpg_launcher(params);
		globals.gpg_valid = _gpg_valid; //TODO: check this
		int p1 = params.out.Find("(GnuPG) ");
		if (p1 == -1)
			is_valid = false;
	}

	return is_valid && gpg_exists;
}

#define NEWTSTR_MALLOC(A) (A==NULL)?NULL:mir_strcpy((char*)mir_alloc(sizeof(char)*(mir_strlen(A)+1)),A)

const bool StriStr(const char *str, const char *substr)
{
	bool i = false;
	char *str_up = NEWTSTR_MALLOC(str);
	char *substr_up = NEWTSTR_MALLOC(substr);

	CharUpperBuffA(str_up, (uint32_t)mir_strlen(str_up));
	CharUpperBuffA(substr_up, (uint32_t)mir_strlen(substr_up));

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

string toUTF8(wstring str)
{
	string ustr;
	try {
		utf8::utf16to8(str.begin(), str.end(), back_inserter(ustr));
	}
	catch (const utf8::exception& e) {
		if (globals.debuglog)
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
		if (globals.debuglog)
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
		while (!isContactSecured(hContact))
			Sleep(1000);
		
		if (!globals.hcontact_data[hContact].msgs_to_send.empty()) {
			Sleep(1000);
			list<string>::iterator end = globals.hcontact_data[hContact].msgs_to_send.end();
			extern std::list<HANDLE> sent_msgs;
			for (list<string>::iterator p = globals.hcontact_data[hContact].msgs_to_send.begin(); p != end; ++p) {
				sent_msgs.push_back((HANDLE)ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)p->c_str()));
				HistoryLog(hContact, p->c_str(), DBEF_SENT);
				Sleep(1000);
			}
			globals.hcontact_data[hContact].msgs_to_send.clear();
			return;
		}
		else
			return;
	}
}

void ExportGpGKeysFunc(int type)
{
	ptrW p(GetFilePath(L"Choose file to export keys", L"*", L"Any file", true));
	if (!p || !p[0])
		return;

	std::ofstream file;
	file.open(p, std::ios::trunc | std::ios::out);
	if (!file.is_open())
		return; //TODO: handle error

	int exported_keys = 0;
	if (!type || type == 2) {
		for (auto &hContact : Contacts()) {
			CMStringA key = g_plugin.getMStringA(hContact, "GPGPubKey");
			if (key.IsEmpty())
				continue;

			ptrW wszLogin(Contact::GetInfo(CNF_UNIQUEID, 0, Proto_GetBaseAccountName(hContact))), wszContact(Contact::GetInfo(CNF_UNIQUEID, hContact));
			if (wszLogin == nullptr || wszContact == nullptr)
				continue;

			std::string id = "Comment: login ";
			id += T2Utf(wszLogin).get();
			id += " contact_id ";
			id += T2Utf(wszContact).get();
			id += '\n';

			int p1 = key.Find("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			if (p1 == -1)
				continue;
			p1 += mir_strlen("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			p1++;
			key.Insert(p1, id.c_str());
			file << key;
			file << std::endl;
			exported_keys++;
		}
	}
	
	if (type == 1 || type == 2) {
		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"--export-secret-keys");
		params.addParam(L"-a");
		gpg_launcher(params); //TODO: handle errors

		file << params.out.c_str();
		file << std::endl;
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

INT_PTR ImportGpGKeys(WPARAM, LPARAM)
{
	ptrW p(GetFilePath(L"Choose file to import keys from", L"*", L"Any file"));
	if (!p || !p[0])
		return 1;

	std::ifstream file;
	file.open(p, std::ios::in);
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
			
			PROTOACCOUNT *pFoundAcc = nullptr;
			for (auto &pa : Accounts()) {
				ptrW wszUniqueId(Contact::GetInfo(CNF_UNIQUEID, 0, pa->szModuleName));
				if (wszUniqueId == nullptr)
					continue;

				if (login == T2Utf(wszUniqueId).get()) {
					pFoundAcc = pa;
					break;
				}
			}

			if (pFoundAcc == nullptr)
				continue;

			for (auto &hContact : Contacts(pFoundAcc->szModuleName)) {
				ptrW wszUniqueId(Contact::GetInfo(CNF_UNIQUEID, hContact, pFoundAcc->szModuleName));
				if (wszUniqueId == nullptr)
					continue;

				if (contact_id != T2Utf(wszUniqueId).get())
					continue;

				CMStringW path = g_plugin.getMStringW("szHomePath");

				gpg_execution_params params;
				{
					wstring rand = toUTF16(get_random(10));
					path += L"\\";
					path += rand.c_str();
					boost::filesystem::remove(path.c_str());
					wfstream f(path, std::ios::out);
					f << toUTF16(key).c_str();
					f.close();
					params.addParam(L"--batch");
					params.addParam(L"--import");
					params.addParam(path.c_str());
				}
				if (!gpg_launcher(params))
					break;
				if (params.result == pxNotFound)
					break;
				if (params.result == pxSuccess)
					processed_keys++;

				string output(params.out);
				if (output.find("already in secret keyring") != string::npos) {
					MessageBox(nullptr, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
					boost::filesystem::remove(path.c_str());
					break;
				}
				char *tmp2;
				string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
				string::size_type s2 = output.find(":", s);
				tmp2 = (char *)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
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
					tmp2 = (char *)mir_alloc((output.substr(s, s2 - s - 1).length() + 1) * sizeof(char));
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
						tmp2 = (char *)mir_alloc((output.substr(s2, s - s2).length() + 1) * sizeof(char));
						mir_strcpy(tmp2, output.substr(s2, s - s2).c_str());
						mir_utf8decode(tmp2, nullptr);
						if (hContact)
							g_plugin.setString(hContact, "KeyComment", output.substr(s2, s - s2).c_str());
						mir_free(tmp2);
						s += 3;
						s2 = output.find(">", s);
						tmp2 = (char *)mir_alloc((output.substr(s, s2 - s).length() + 1) * sizeof(char));
						mir_strcpy(tmp2, output.substr(s, s2 - s).c_str());
						mir_utf8decode(tmp2, nullptr);
						if (hContact)
							g_plugin.setString(hContact, "KeyMainEmail", output.substr(s, s2 - s).c_str());
						mir_free(tmp2);
					}
					else {
						tmp2 = (char *)mir_alloc((output.substr(s2, s - s2).length() + 1) * sizeof(char));
						mir_strcpy(tmp2, output.substr(s2, s - s2).c_str());
						mir_utf8decode(tmp2, nullptr);
						if (hContact)
							g_plugin.setString(hContact, "KeyMainEmail", output.substr(s2, s - s2).c_str());
						mir_free(tmp2);
					}
				}
				g_plugin.setByte(hContact, "GPGEncryption", 1);
				g_plugin.setWString(hContact, "GPGPubKey", toUTF16(key).c_str());

				boost::filesystem::remove(path.c_str());
				break;
			}
			key.clear();
		}
		else if (strstr(line, "-----END PGP PRIVATE KEY BLOCK-----")) {
			CMStringW tmp2 = g_plugin.getMStringW("szHomePath");
			tmp2 += L"\\temporary_exported.asc";
			boost::filesystem::remove(tmp2.c_str());

			wfstream f(tmp2, std::ios::out);
			f << toUTF16(key).c_str();
			f.close();

			gpg_execution_params params;
			params.addParam(L"--batch");
			params.addParam(L"--import");
			params.addParam(tmp2.c_str());
			if (!gpg_launcher(params))
				break;
			if (params.result == pxNotFound)
				break;
			if (params.result == pxSuccess)
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

void SendErrorMessage(MCONTACT hContact)
{
	if (!g_plugin.bSendErrorMessages)
		return;

	uint8_t enc = g_plugin.getByte(hContact, "GPGEncryption", 0);
	g_plugin.setByte(hContact, "GPGEncryption", 0);
	ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
	HistoryLog(hContact, "Error message sent", DBEF_SENT);
	g_plugin.setByte(hContact, "GPGEncryption", enc);
}

void fix_line_term(std::string &s)
{
	if (s.empty())
		return;

	boost::algorithm::erase_all(s, "\r\r");

	// unified line endings for unix & windows port
	boost::algorithm::replace_all(s, "\r\n", "\n");
	boost::algorithm::replace_all(s, "\n", "\r\n");
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

void strip_tags(std::string &str)
{
	if (str.empty())
		return;
	boost::algorithm::erase_all(str, globals.wszInopentag.c_str());
	boost::algorithm::erase_all(str, globals.wszInclosetag.c_str());
	boost::algorithm::erase_all(str, globals.wszOutopentag.c_str());
	boost::algorithm::erase_all(str, globals.wszOutclosetag.c_str());
}


void ShowEncryptedFileMsgBox()
{
	CDlgEncryptedFileMsgBox *d = new CDlgEncryptedFileMsgBox;
	d->DoModal();
}

void clean_temp_dir()
{
	using namespace boost::filesystem;
	wchar_t mir_path[MAX_PATH];
	PathToAbsoluteW(L"\\", mir_path);
	SetCurrentDirectoryW(mir_path);
	
	CMStringW tmp = mir_path;
	tmp += g_plugin.getMStringW("szHomePath");
	tmp += L"\\tmp";
	if (exists(tmp.c_str()) && is_directory(tmp.c_str())) {
		boost::filesystem::path p(tmp);
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
		CMStringW path = g_plugin.getMStringW("szHomePath");
		uint32_t dwFileAttr = GetFileAttributes(path);
		if (dwFileAttr != INVALID_FILE_ATTRIBUTES) {
			dwFileAttr &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(path, dwFileAttr);
		}
	}
	return true;
}

void gpg_save_paths(wchar_t *gpg_bin_path, wchar_t *gpg_home_path)
{
	g_plugin.setWString("szGpgBinPath", gpg_bin_path);
	g_plugin.setWString("szHomePath", gpg_home_path);
}

bool gpg_use_new_random_key(const char *account_name)
{
	CMStringW path = g_plugin.getMStringW("szHomePath");
	path += L"\\new_key";

	// generating key file
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

	{
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
		string out(params.out);
		int p1 = params.out.Find("key ");
		if (p1 != -1)
			path = ptrW(mir_utf8decodeW(params.out.Mid(p1 + 4, 8).c_str()));
		else
			path.Empty();
	}

	if (!path.IsEmpty()) {
		gpg_execution_params params;
		params.addParam(L"--batch");
		params.addParam(L"-a");
		params.addParam(L"--export");
		params.addParam(path.c_str());
		if (!gpg_launcher(params))
			return false;

		if (params.result == pxNotFound)
			return false;

		params.out.Remove('\r');

		if (account_name == nullptr) {
			g_plugin.setString("GPGPubKey", params.out.c_str());
			g_plugin.setWString("KeyID", path.c_str());
		}
		else {
			CMStringA acc_str = account_name;
			g_plugin.setString(acc_str + "_GPGPubKey", params.out.c_str());
			g_plugin.setWString(acc_str + "_KeyID", path.c_str());
		}
	}
	return true;
}
