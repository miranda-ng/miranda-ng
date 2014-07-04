// Copyright � 2010-2012 sss
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


#include "commonheaders.h"

void ShowExportKeysDlg();
void ShowLoadPublicKeyDialog();

extern HINSTANCE hInst;
extern HANDLE hLoadPublicKey;
extern HGENMENU hToggleEncryption, hSendKey;

TCHAR* __stdcall UniGetContactSettingUtf(MCONTACT hContact, const char *szModule,const char* szSetting, TCHAR* szDef)
{
  DBVARIANT dbv = {DBVT_DELETED};
  TCHAR* szRes = NULL;
  if (db_get_ts(hContact, szModule, szSetting, &dbv))
	  return mir_tstrdup(szDef);
  else if(dbv.pszVal)
	  szRes = mir_tstrdup(dbv.ptszVal);
  else
	  szRes = mir_tstrdup(szDef);

  db_free(&dbv);
  return szRes;
}

char* __stdcall UniGetContactSettingUtf(MCONTACT hContact, const char *szModule,const char* szSetting, char* szDef)
{
  DBVARIANT dbv = {DBVT_DELETED};
  char* szRes = NULL;
  if (db_get_s(hContact, szModule, szSetting, &dbv))
	  return mir_strdup(szDef);
  else if(dbv.pszVal)
	  szRes = mir_strdup(dbv.pszVal);
  else
	  szRes = mir_strdup(szDef);
  db_free(&dbv);
  return szRes;
}

void GetFilePath(TCHAR *WindowTittle, char *szSetting, TCHAR *szExt, TCHAR *szExtDesc)
{
	TCHAR str[MAX_PATH+2] = {0}, *tmp;
	OPENFILENAME ofn={0};
	TCHAR filter[512], *pfilter;
	ofn.lStructSize=CDSIZEOF_STRUCT(OPENFILENAME,lpTemplateName);
	ofn.Flags=OFN_EXPLORER;
	ofn.lpstrTitle=TranslateW(WindowTittle);
	_tcscpy(filter,TranslateW(szExtDesc));
	pfilter=filter+_tcslen(filter)+1;
	_tcscpy(pfilter, szExt);
	pfilter[_tcslen(pfilter)+1] = '\0';
	pfilter[_tcslen(pfilter)+2] = '\0';
	ofn.lpstrFilter=filter;
	tmp = UniGetContactSettingUtf(0, szGPGModuleName, szSetting, _T(""));
	_tcscpy(str, tmp);
	mir_free(tmp);
	if(_tcslen(str)< 2)
		str[0] = '\0';
	ofn.lpstrFile=str;
	ofn.nMaxFile=_MAX_PATH;
	ofn.nMaxFileTitle=MAX_PATH;
	if(!GetOpenFileName(&ofn)) 
		return;
	db_set_ts(0, szGPGModuleName, szSetting, str);
}

TCHAR *GetFilePath(TCHAR *WindowTittle, TCHAR *szExt, TCHAR *szExtDesc, bool save_file)
{
	TCHAR *str = new TCHAR [MAX_PATH+2];
	OPENFILENAME ofn={0};
	TCHAR filter[512], *pfilter;
	ofn.lStructSize=CDSIZEOF_STRUCT(OPENFILENAME,lpTemplateName);
	ofn.Flags=OFN_EXPLORER;
	ofn.lpstrTitle=TranslateW(WindowTittle);
	_tcscpy(filter,TranslateW(szExtDesc));
	pfilter=filter+_tcslen(filter)+1;
	_tcscpy(pfilter, szExt);
	pfilter[_tcslen(pfilter)+1] = '\0';
	pfilter[_tcslen(pfilter)+2] = '\0';
	ofn.lpstrFilter=filter;
	_tcscpy(str, _T(""));
	if(_tcslen(str)< 2)
		str[0] = '\0';
	ofn.lpstrFile=str;
	ofn.nMaxFile=_MAX_PATH;
	ofn.nMaxFileTitle=MAX_PATH;
	if(!save_file)
	{
		if(!GetOpenFileName(&ofn)) 
		{
			delete [] str;
			return NULL;
		}
	}
	else
	{
		if(!GetSaveFileName(&ofn)) 
		{
			delete [] str;
			return NULL;
		}
	}
	return str;
}

void GetFolderPath(TCHAR *WindowTittle, char *szSetting)
{
	BROWSEINFO pbi = {0};
	pbi.lpszTitle = WindowTittle;
	pbi.ulFlags = BIF_EDITBOX|BIF_NEWDIALOGSTYLE|BIF_SHAREABLE;
	LPITEMIDLIST pidl = SHBrowseForFolder(&pbi);
	if (pidl != 0)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			db_set_ts(NULL, szGPGModuleName, "szHomePath", path);
		}
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
}

INT_PTR LoadKey(WPARAM w, LPARAM)
{
	user_data[1] = (MCONTACT)w;
	ShowLoadPublicKeyDialog();
	return 0;
}

INT_PTR SendKey(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	if(metaIsProtoMetaContacts(hContact))
		hContact = metaGetMostOnline(hContact);
	char *szMessage;
	std::string key_id_str;
	{
		LPSTR proto = GetContactProto(hContact);
		PROTOACCOUNT *acc = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)proto);
		std::string acc_str;
		if(acc)
		{
			acc_str = toUTF8(acc->tszAccountName);
			acc_str += "(";
			acc_str += acc->szModuleName;
			acc_str += ")" ;
			key_id_str = acc_str;
			key_id_str += "_KeyID";
			acc_str += "_GPGPubKey";
		}
		szMessage = UniGetContactSettingUtf(NULL, szGPGModuleName, acc_str.empty()?"GPGPubKey":acc_str.c_str(), "");
		if(!szMessage[0])
		{
			mir_free(szMessage);
			szMessage = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", ""); //try to get default key as fallback in any way
		}
	}
	if(szMessage[0])
	{
		BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
		db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
		CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)szMessage);
		std::string msg = "Public key ";
		char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, key_id_str.c_str(), "");
		if(!keyid[0])
		{
			mir_free(keyid);
			keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
		}
		msg += keyid;
		mir_free(keyid);
		msg += " sent";
		mir_free(szMessage);
		szMessage = mir_strdup(msg.c_str());
		HistoryLog(hContact, db_event(szMessage, 0, 0, DBEF_SENT));
		db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
	}
	else
		mir_free(szMessage);
	return 0;
}

INT_PTR ToggleEncryption(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	BYTE enc = 0;
	if(metaIsProtoMetaContacts(hContact))
		enc = db_get_b(metaGetMostOnline(hContact), szGPGModuleName, "GPGEncryption", 0);
	else
		enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
	if(metaIsProtoMetaContacts(hContact))
	{
		if(MessageBox(0, TranslateT("Do you want to toggle encryption for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES)
		{
			int count = db_mc_getSubCount(hContact);
			for(int i = 0; i < count; i++)
			{
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if(hcnt)
					db_set_b(hcnt, szGPGModuleName, "GPGEncryption", enc?0:1);
			}
			db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc?0:1);
		}
	}
	else
		db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc?0:1);
	void setSrmmIcon(MCONTACT hContact);
	void setClistIcon(MCONTACT hContact);
	setSrmmIcon(hContact);
	setClistIcon(hContact);
	enc = enc?0:1;
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME;
	enc?mi.pszName="Turn off GPG encryption":mi.pszName="Turn on GPG encryption";
	Menu_ModifyItem(hToggleEncryption, &mi);
	return 0;
}

int OnPreBuildContactMenu(WPARAM w, LPARAM l)
{
	MCONTACT hContact = (MCONTACT)w;
	if(metaIsProtoMetaContacts(hContact))
		hContact = metaGetMostOnline(hContact);
	
	{
		CLISTMENUITEM mi2 = { sizeof(mi2) };
		LPSTR proto = GetContactProto(hContact);
		PROTOACCOUNT *acc = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)proto);
		std::string setting;
		if(acc)
		{
			setting = toUTF8(acc->tszAccountName);
			setting += "(";
			setting += acc->szModuleName;
			setting += ")" ;
			setting += "_KeyID";
		}
		char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, setting.c_str(), "");
		if(!keyid[0])
		{
			mir_free(keyid);
			keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
		}
		TCHAR buf[128] = {0};
		mir_sntprintf(buf, SIZEOF(buf), _T("%s: %s"), TranslateT("Send public key"), toUTF16(keyid).c_str());
		mir_free(keyid);
		mi2.ptszName = buf;
		mi2.flags = CMIM_NAME | CMIF_TCHAR;
		Menu_ModifyItem(hSendKey, &mi2);
	}
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_NAME;
	TCHAR *tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", _T(""));
	if(!tmp[0])
	{
		db_unset(hContact, szGPGModuleName, "GPGEncryption");
		mi.flags += CMIM_FLAGS | CMIF_GRAYED;
	}
	else
		mi.flags = CMIM_NAME | CMIM_FLAGS;
	mi.pszName = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0)?"Turn off GPG encryption":"Turn on GPG encryption";
	Menu_ModifyItem(hToggleEncryption, &mi);
	mir_free(tmp);
	return 0;
}


list<wstring> transfers;

DWORD file_msg_state = -1;

int onProtoAck(WPARAM w, LPARAM l)
{
	ACKDATA *ack=(ACKDATA*)l;
	CCSDATA *ccs=(CCSDATA*)ack->lParam;

	if(ack->type == ACKTYPE_FILE)
	{
		switch(ack->result)
		{
		case ACKRESULT_DENIED:	case ACKRESULT_FAILED: 
			break;
		case ACKRESULT_SUCCESS:
			{
				PROTOFILETRANSFERSTATUS *f = (PROTOFILETRANSFERSTATUS*) ack->hProcess;
				if((f->flags & PFTS_SENDING) != PFTS_SENDING)
				{
					TCHAR *filename = NULL;
					if(f->flags & PFTS_UNICODE)
					{
						if(f->tszCurrentFile && f->tszCurrentFile[0])
							filename = mir_wstrdup(f->tszCurrentFile);
						if(!filename)
							return 0;
					}
					else
					{
						if(f->szCurrentFile && f->szCurrentFile[0])
							filename = mir_utf8decodeT(f->szCurrentFile);
						if(!filename)
							return 0;
					}
					if(_tcsstr(filename, _T(".gpg"))) //decrypt it
					{ //process encrypted file
						if(!bFileTransfers && !bSameAction)
						{
							void ShowEncryptedFileMsgBox();
							ShowEncryptedFileMsgBox();
						}
						if(!bFileTransfers && bSameAction)
							return 0;
						if(file_msg_state < 1)
							return 0;
						HistoryLog(ack->hContact, db_event("Received encrypted file, trying to decrypt", 0,0, 0));
						if(!boost::filesystem::exists(f->tszCurrentFile))
							return 0;
						string out;
						DWORD code;
						pxResult result;
						std::vector<wstring> cmd;
						cmd.push_back(L"-o");
						wstring file = filename;
						wstring::size_type p1 = file.rfind(_T(".gpg"));
						file.erase(p1, _tcslen(_T(".gpg")));
						if(boost::filesystem::exists(file))
						{
							if(MessageBox(0, TranslateT("Target file exists, do you want to replace it?"), TranslateT("Warning"), MB_YESNO) == IDNO)
								return 0;
						}
						cmd.push_back(file);
						boost::filesystem::remove(file);
						extern TCHAR *password;
						{ // password
							TCHAR *pass = NULL;
							char *keyid = UniGetContactSettingUtf(ack->hContact, szGPGModuleName, "KeyID", "");
							if(strlen(keyid) > 0)
							{
								string dbsetting = "szKey_";
								dbsetting += keyid;
								dbsetting += "_Password";
								pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), _T(""));
								if(_tcslen(pass) > 0 && bDebugLog)
									debuglog<<std::string(time_str()+": info: found password in database for key ID: "+keyid+", trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR))+" with password");
							}
							else
							{
								pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
								if(_tcslen(pass) > 0 && bDebugLog)
									debuglog<<std::string(time_str()+": info: found password for all keys in database, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR))+" with password");
							}
							if(_tcslen(pass) > 0)
							{
								cmd.push_back(L"--passphrase");
								cmd.push_back(pass);
							}
							else if(password)
							{
								if(bDebugLog)
									debuglog<<std::string(time_str()+": info: found password in memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR))+" with password");
								cmd.push_back(L"--passphrase");
								cmd.push_back(password);
							}
							else if (bDebugLog)
								debuglog<<std::string(time_str()+": info: passwords not found in database or memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR))+" with out password");
							mir_free(pass);
							mir_free(keyid);
						}
						cmd.push_back(L"-d");
						cmd.push_back(filename);
						gpg_execution_params params(cmd);
						params.out = &out;
						params.code = &code;
						params.result = &result;
						if(!gpg_launcher(params, boost::posix_time::minutes(15)))
							return 0;
						while(out.find("public key decryption failed: bad passphrase") != string::npos)
						{
							if(bDebugLog)
								debuglog<<std::string(time_str()+": info: failed to decrypt messaage from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR))+" password needed, trying to get one");
							if(_terminate)
								break;
							{ //save inkey id
								string::size_type s = out.find(" encrypted with ");
								s = out.find(" ID ", s);
								s += strlen(" ID ");
								string::size_type s2 = out.find(",",s);
								if(metaIsProtoMetaContacts(ack->hContact))
									db_set_s(metaGetMostOnline(ack->hContact), szGPGModuleName, "InKeyID", out.substr(s, s2-s).c_str());
								else
									db_set_s(ack->hContact, szGPGModuleName, "InKeyID", out.substr(s, s2-s).c_str());
							}
							void ShowLoadKeyPasswordWindow();
							new_key_hcnt_mutex.lock();
							new_key_hcnt = ack->hContact;
							ShowLoadKeyPasswordWindow();
							std::vector<wstring> cmd2 = cmd;
							if(password)
							{
								if(bDebugLog)
									debuglog<<std::string(time_str()+": info: found password in memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ack->hContact, GCDNF_TCHAR)));
								std::vector<wstring> tmp;
								tmp.push_back(L"--passphrase");
								tmp.push_back(password);
								cmd2.insert(cmd2.begin(), tmp.begin(), tmp.end());
							}
							out.clear();
							gpg_execution_params params(cmd2);
							//pxResult result;
							params.out = &out;
							params.code = &code;
							params.result = &result;
							if(!gpg_launcher(params, boost::posix_time::seconds(15)))
							{
								//boost::filesystem::remove(filename);
								return 0;
							}
							if(result == pxNotFound)
							{
								//boost::filesystem::remove(filename);
								return 0;
							}
						}
						if(result == pxSuccess)
							boost::filesystem::remove(filename);
						mir_free(filename);
				}
			}
		}
		break;
	}
	}
	else if(ack->type == ACKTYPE_MESSAGE)
	{
		extern std::list<HANDLE> sent_msgs;
		if(!sent_msgs.empty())
		{
			if(ack->result == ACKRESULT_FAILED)
			{
				std::list<HANDLE>::iterator it = std::find(sent_msgs.begin(), sent_msgs.end(), ack->hProcess);
				if(it != sent_msgs.end())
				{
					HistoryLog(ack->hContact, db_event("Failed to send encrypted message", 0,0, 0));
					
				}
			}
			else if(ack->result == ACKRESULT_SUCCESS)
			{
				std::list<HANDLE>::iterator it = std::find(sent_msgs.begin(), sent_msgs.end(), ack->hProcess);
				if(it != sent_msgs.end())
					sent_msgs.erase(it);
			}
		}
	}
	return 0;
}

std::wstring encrypt_file(MCONTACT hContact, TCHAR *filename)
{
	string out;
	DWORD code;
	pxResult result;
	MCONTACT hcnt = metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact;
	std::vector<wstring> cmd;
	cmd.push_back(L"--batch");
	cmd.push_back(L"--tes");
	cmd.push_back(L"-r");
	char *keyid = UniGetContactSettingUtf(hcnt, szGPGModuleName, "KeyID", "");
	TCHAR *szKeyid = mir_a2t(keyid);
	TCHAR *name = _tcsrchr(filename,_T('\\'));
	if( !name )
		name = filename;
	else
		name++;
	TCHAR *file_out =  new TCHAR [_tcslen(name) + _tcslen(_T(".gpg")) + 1];
	mir_sntprintf(file_out, _tcslen(name) + _tcslen(_T(".gpg")) + 1, _T("%s.gpg"), name);
	cmd.push_back(szKeyid);
	if(db_get_b(hcnt, szGPGModuleName, "bAlwaysTrust", 0))
	{
		cmd.push_back(L"--trust-model");
		cmd.push_back(L"always");
	}
	mir_free(szKeyid);
	mir_free(keyid);
	cmd.push_back(L"-o");
	TCHAR *temp = _tgetenv(_T("TEMP"));
	cmd.push_back(wstring(temp) + L"\\" + file_out);
	wstring path_out = temp;
	path_out += _T("\\");
	path_out += file_out;
	boost::filesystem::remove(path_out);
	cmd.push_back(L"-e");
	cmd.push_back(filename);
	gpg_execution_params params(cmd);
	params.out = &out;
	params.code = &code;
	params.result = &result;
	delete [] file_out;
	if(!gpg_launcher(params, boost::posix_time::minutes(3)))
		return 0;
	if(out.find("There is no assurance this key belongs to the named user") != string::npos)
	{
		out.clear();
		if(MessageBox(0, TranslateT("We're trying to encrypt with untrusted key. Do you want to trust this key permanently?"), TranslateT("Warning"), MB_YESNO) == IDYES)
		{
			db_set_b(hcnt, szGPGModuleName, "bAlwaysTrust", 1);
			std::vector<std::wstring> tmp;
			tmp.push_back(L"--trust-model");
			tmp.push_back(L"always");
			cmd.insert(cmd.begin(), tmp.begin(), tmp.end());
			if(!gpg_launcher(params, boost::posix_time::minutes(3)))
				return 0;
		}
		else
			return 0;
	}
	return path_out;
}

//from secureim partially
INT_PTR onSendFile(WPARAM w, LPARAM l)
{
	if(!bFileTransfers)
		return CallService(MS_PROTO_CHAINSEND, w, l);
	CCSDATA *ccs=(CCSDATA*)l;
	if(isContactSecured(ccs->hContact))
	{
		char *proto = GetContactProto(ccs->hContact);
		DWORD uin = db_get_dw(ccs->hContact, proto, "UIN", 0);
		bool cap_found = false, supported_proto = false;
		if(uin)
		{
			if( ProtoServiceExists(proto, PS_ICQ_CHECKCAPABILITY)) {
				supported_proto = true;
				ICQ_CUSTOMCAP cap = {0};
				strcpy(cap.caps, "GPG FileTransfer");
				if( ProtoCallService(proto, PS_ICQ_CHECKCAPABILITY, (WPARAM)ccs->hContact, (LPARAM)&cap))
					cap_found = true;
			}
		}
		else
		{
			TCHAR *jid = UniGetContactSettingUtf(ccs->hContact, proto, "jid", _T(""));
			if(jid[0])
			{
				extern list <JabberAccount*> Accounts;
				list<JabberAccount*>::iterator end = Accounts.end();
				for(list<JabberAccount*>::iterator p = Accounts.begin(); p != end; p++)
				{
					TCHAR *caps = (*p)->getJabberInterface()->GetResourceFeatures(jid);
					if(caps)
					{
						supported_proto = true;
						wstring str;
						for(int i=0;;i++)
						{
							str.push_back(caps[i]);
							if(caps[i] == '\0')
								if(caps[i+1] == '\0')
									break;
						}
						mir_free(caps);
						if(str.find(_T("GPG_Encrypted_FileTransfers:0")) != string::npos)
							cap_found = true;
					}
				}
			}
			mir_free(jid);
		}
		if(supported_proto && !cap_found)
		{
			if(MessageBox(0, TranslateT("Capability to decrypt file not found on other side.\nRecipient may be unable to decrypt file(s).\nDo you want to encrypt file(s) anyway?"), TranslateT("File transfer warning"), MB_YESNO) == IDNO)
				return CallService(MS_PROTO_CHAINSEND, w, l);
		}
		if(!supported_proto)
		{
			if(MessageBox(0, TranslateT("Unable to check encryption support on other side.\nRecipient may be unable to decrypt file(s).\nCurrently capability check supported only for ICQ and Jabber protocols.\nIt will work for any other proto if Miranda with New_GPG is used on other side.\nDo you want to encrypt file(s) anyway?"), TranslateT("File transfer warning"), MB_YESNO) == IDNO)
				return CallService(MS_PROTO_CHAINSEND, w, l);
		}
		HistoryLog(ccs->hContact, db_event(Translate("encrypting file for transfer"), 0, 0, DBEF_SENT));
		DWORD flags = (DWORD)ccs->wParam; //check for PFTS_UNICODE here
		int i;
//		if(flags & PFTS_UNICODE) //this does not work ....
		if(StriStr(ccs->szProtoService, "/sendfilew"))
		{
			TCHAR **file=(TCHAR **)ccs->lParam;
			for(i = 0; file[i]; i++)
			{
				if(!boost::filesystem::exists(file[i]))
					return 0; //we do not want to send file unencrypted (sometimes ack have wrong info)
				if (_tcsstr(file[i],_T(".gpg")))
					continue;
				std::wstring path_out = encrypt_file(ccs->hContact, file[i]);
				mir_free(file[i]);
				file[i] = mir_tstrdup(path_out.c_str());
				transfers.push_back(path_out);
			}
		}
		else
		{
			char **file = (char**) ccs->lParam;
			for(i = 0; file[i]; i++)
			{
				if(!boost::filesystem::exists(file[i]))
					return 0; //we do not want to send file unencrypted (sometimes ack have wrong info)
				if (strstr(file[i],".gpg"))
					continue;
				TCHAR *tmp = mir_utf8decodeT(file[i]);
				std::wstring path_out = encrypt_file(ccs->hContact, tmp);
				mir_free(tmp);
				char* tmp2 = mir_utf8encodeW(path_out.c_str());
				mir_free(file[i]);
				file[i] = tmp2;
				transfers.push_back(path_out);

			}
		}
	}
	return CallService(MS_PROTO_CHAINSEND, w, l);
}


void HistoryLog(MCONTACT hContact, db_event evt)
{
	DBEVENTINFO Event = { sizeof(Event) };
	Event.szModule = szGPGModuleName;
	Event.eventType = evt.eventType;
	Event.flags = evt.flags;
	if(!evt.timestamp)
		Event.timestamp = (DWORD)time(NULL);
	else
		Event.timestamp = evt.timestamp;
	Event.cbBlob = (DWORD)strlen((char*)evt.pBlob)+1;
	Event.pBlob = (PBYTE)_strdup((char*)evt.pBlob);
	db_event_add(hContact, &Event);
}

static int ControlAddStringUtf(HWND ctrl, DWORD msg, const TCHAR *szString)
{
	int item = -1;
	item = SendMessage(ctrl, msg, 0, (LPARAM)szString);
	return item;
}

int ComboBoxAddStringUtf(HWND hCombo, const TCHAR *szString, DWORD data)
{
	int item = ControlAddStringUtf(hCombo, CB_ADDSTRING, szString);
	SendMessage(hCombo, CB_SETITEMDATA, item, data);

	return item;
}


int GetJabberInterface(WPARAM w, LPARAM l) //get interface for all jabber accounts, options later
{
	extern list <JabberAccount*> Accounts;
	void AddHandlers();
	int count = 0;
	PROTOACCOUNT **accounts;
	ProtoEnumAccounts(&count, &accounts);
	list <JabberAccount*>::iterator p;
	Accounts.clear();
	Accounts.push_back(new JabberAccount);
	p = Accounts.begin();
	(*p)->setAccountNumber(0);
	for(int i = 0; i < count; i++) //get only jabber accounts from all accounts
	{
		IJabberInterface *JIftmp = getJabberApi(accounts[i]->szModuleName);
		int a = 0;
		if(JIftmp)
		{
			(*p)->setJabberInterface(JIftmp);
			if(accounts[i]->tszAccountName)
			{
				TCHAR* tmp = mir_tstrdup(accounts[i]->tszAccountName);
				(*p)->setAccountName(tmp);
			}
			else
			{
				TCHAR *tmp = mir_a2t(accounts[i]->szModuleName);
				(*p)->setAccountName(tmp);
			}
			(*p)->setAccountNumber(a);
			a++;
			Accounts.push_back(new JabberAccount);
			p++;
		}
	}
	Accounts.pop_back();
	AddHandlers();
	return 0;
}

static JABBER_HANDLER_FUNC SendHandler(IJabberInterface *ji, HXML node, void *pUserData)
{
	HXML local_node = node;
	for(int n = 0; n <= xi.getChildCount(node); n++)
	{
		LPCTSTR str = xi.getText(local_node); 
		LPCTSTR nodename = xi.getName(local_node);
		LPCTSTR attr = xi.getAttrValue(local_node, _T("to"));
		if(attr)
		{
			MCONTACT hContact = ji->ContactFromJID(attr);
			if(hContact)
				if(!isContactSecured(hContact))
					return FALSE;
		}
		if(str)
		{
			if(_tcsstr(str, _T("-----BEGIN PGP MESSAGE-----")) && _tcsstr(str, _T("-----END PGP MESSAGE-----")))
			{
				wstring data = str;
				xi.setText(local_node, _T("This message is encrypted."));
				wstring::size_type p1 = data.find(_T("-----BEGIN PGP MESSAGE-----")) + _tcslen(_T("-----BEGIN PGP MESSAGE-----"));
				while(data.find(_T("Version: "), p1) != wstring::npos)
				{
					p1 = data.find(_T("Version: "), p1);
					p1 = data.find(_T("\n"), p1);
				}
				while(data.find(_T("Comment: "), p1) != wstring::npos)
				{
					p1 = data.find(_T("Comment: "), p1);
					p1 = data.find(_T("\n"), p1);
				}
				while(data.find(_T("Encoding: "), p1) != wstring::npos)
				{
					p1 = data.find(_T("Encoding: "), p1);
					p1 = data.find(_T("\n"), p1);
				}
				p1+=3;
				wstring::size_type p2 = data.find(_T("-----END PGP MESSAGE-----"));
				wstring data2 = data.substr(p1, p2-p1-2);
				strip_line_term(data2);
				HXML encrypted_data = xi.addChild(node, _T("x"), data2.c_str());
				xi.addAttr(encrypted_data, _T("xmlns"), _T("jabber:x:encrypted"));
				return FALSE;
			}
		}
		if(bPresenceSigning && nodename)
		{
			if(_tcsstr(nodename, _T("status")))
			{
				TCHAR *path_c = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				wstring path_out = path_c;
				wstring file = toUTF16(get_random(10));
				mir_free(path_c);
				path_out += _T("\\tmp\\");
				path_out += file;
				boost::filesystem::remove(path_out);
				wfstream f(path_out.c_str(), std::ios::out);
				f<<toUTF8(str).c_str();
				f.close();
				if(!boost::filesystem::exists(path_out))
				{
					if(bDebugLog)
						debuglog<<std::string(time_str()+": info: Failed to write prescense in file");
					return FALSE;
				}
				{
					extern TCHAR *password;
					string out;
					DWORD code;
					std::vector<wstring> cmd;
					{
						char *inkeyid;
						{
							char *proto = ji->GetModuleName();
							char setting[64];
							strcpy(setting, proto);
							strcat(setting, "_KeyID");
							inkeyid = UniGetContactSettingUtf(NULL, szGPGModuleName, setting, "");
							if(!inkeyid[0])
							{
								mir_free(inkeyid);
								inkeyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
							}
						}
						TCHAR *pass = NULL;
						if(inkeyid[0])
						{
							string dbsetting = "szKey_";
							dbsetting += inkeyid;
							dbsetting += "_Password";
							pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), _T(""));
							if(pass[0] && bDebugLog)
								debuglog<<std::string(time_str()+": info: found password in database for key ID: "+inkeyid+", trying to encrypt message from self with password");
						}
						else
						{
							pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
							if(pass[0] && bDebugLog)
								debuglog<<std::string(time_str()+": info: found password for all keys in database, trying to encrypt message from self with password");
						}
						if(pass[0])
						{
							cmd.push_back(L"--passphrase");
							cmd.push_back(pass);
						}
						else if(password)
						{
							if(bDebugLog)
								debuglog<<std::string(time_str()+": info: found password in memory, trying to encrypt message from self with password");
							cmd.push_back(L"--passphrase");
							cmd.push_back(password);
						}
						else if (bDebugLog)
							debuglog<<std::string(time_str()+": info: passwords not found in database or memory, trying to encrypt message from self with out password");
						mir_free(pass);
						mir_free(inkeyid);
					}
					cmd.push_back(L"--local-user");
					path_c = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", _T(""));
					cmd.push_back(path_c);
					cmd.push_back(L"--default-key");
					cmd.push_back(path_c);
					mir_free(path_c);
					cmd.push_back(L"--batch");
					cmd.push_back(L"--yes");
					cmd.push_back(L"-abs");
					cmd.push_back(path_out);
					gpg_execution_params params(cmd);
					pxResult result;
					params.out = &out;
					params.code = &code;
					params.result = &result;
					gpg_launcher(params, boost::posix_time::seconds(15)); // TODO: handle errors
					boost::filesystem::remove(path_out);
					path_out += _T(".asc");
					f.open(path_out.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
					wstring data;
					if(f.is_open())
					{
						std::wifstream::pos_type size = f.tellg();
						TCHAR *tmp = new TCHAR [(std::ifstream::pos_type)size+(std::ifstream::pos_type)1];
						f.seekg(0, std::ios::beg);
						f.read(tmp, size);
						tmp[size]= '\0';
						data.append(tmp);
						delete [] tmp;
						f.close();
						boost::filesystem::remove(path_out);
					}
					if(data.empty())
					{
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: Failed to read prescense sign from file");
						return FALSE;
					}
					if(data.find(_T("-----BEGIN PGP SIGNATURE-----")) != wstring::npos && data.find(_T("-----END PGP SIGNATURE-----")) != wstring::npos)
					{
						wstring::size_type p1 = data.find(_T("-----BEGIN PGP SIGNATURE-----")) + _tcslen(_T("-----BEGIN PGP SIGNATURE-----"));
						if(data.find(_T("Version: "), p1) != wstring::npos)
						{
							p1 = data.find(_T("Version: "), p1);
							p1 = data.find(_T("\n"), p1);
							if(data.find(_T("Version: "), p1) != wstring::npos)
							{
								p1 = data.find(_T("Version: "), p1);
								p1 = data.find(_T("\n"), p1)+1;
							}
							else
								p1 += 1;
						}
						if(data.find(_T("Comment: "), p1) != wstring::npos)
						{
							p1 = data.find(_T("Comment: "), p1);
							p1 = data.find(_T("\n"), p1);
							if(data.find(_T("Comment: "), p1) != wstring::npos)
							{
								p1 = data.find(_T("Comment: "), p1);
								p1 = data.find(_T("\n"), p1)+1;
							}
							else
								p1 += 1;
						}
						else
							p1+=1;
						p1++;
						wstring::size_type p2 = data.find(_T("-----END PGP SIGNATURE-----"));
						{
							std::wstring tmp = data.substr(p1, p2-p1);
							strip_line_term(tmp);
							HXML encrypted_data = xi.addChild(node, _T("x"), tmp.c_str());
							xi.addAttr(encrypted_data, _T("xmlns"), _T("jabber:x:signed"));
						}
					}
					return FALSE;
				}
			}
		}
		local_node = xi.getChild(node, n);
	}
	return FALSE;
}

//boost::mutex sign_file_mutex;

static JABBER_HANDLER_FUNC PrescenseHandler(IJabberInterface *ji, HXML node, void *pUserData)
{
	HXML local_node = node;
	for(int n = 0; n <= xi.getChildCount(node); n++)
	{
		LPCTSTR str = xi.getText(local_node); 
		LPCTSTR nodename = xi.getName(local_node);
		if(nodename)
		{
			if(_tcsstr(nodename, _T("x")))
			{
				for(int n = 0; n < xi.getAttrCount(local_node); n++)
				{
					LPCTSTR name = xi.getAttrName(local_node, n);
					LPCTSTR value = xi.getAttrValue(local_node, name);
					if(_tcsstr(value, _T("jabber:x:signed")))
					{
						std::wstring status_str;
						HXML local_node2 = node;
						for(int n = 0; n <= xi.getChildCount(node); n++)
						{
							LPCTSTR nodename2 = xi.getName(local_node2);
							if(_tcsstr(nodename2, _T("status")))
							{
								LPCTSTR status = xi.getText(local_node2);
								if(status)
									status_str = status;
								break;
							}
							local_node2 = xi.getChild(node, n);
						}
						LPCTSTR data = xi.getText(local_node);
						wstring sign = _T("-----BEGIN PGP SIGNATURE-----\n\n");
						wstring file = toUTF16(get_random(10)), status_file = toUTF16(get_random(10));
						sign += data;
						sign += _T("\n-----END PGP SIGNATURE-----\n");
						TCHAR *path_c = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
						wstring path_out = path_c, status_file_out = path_c;
						mir_free(path_c);
						path_out += L"\\tmp\\";
						path_out += file;
						path_out += L".sig";
						status_file_out += L"\\tmp\\";
						status_file_out += status_file;
						status_file_out += L".status";
//						sign_file_mutex.lock();
						boost::filesystem::remove(path_out);
						boost::filesystem::remove(status_file_out);
						wfstream f(path_out.c_str(), std::ios::out);
						while(!f.is_open())
							f.open(path_out.c_str(), std::ios::out);
						f<<toUTF8(sign).c_str();
						f.close();
						f.open(status_file_out.c_str(), std::ios::out);
						while(!f.is_open())
							f.open(status_file_out.c_str(), std::ios::out);
						f<<toUTF8(status_str).c_str();
						f.close();
						if(!boost::filesystem::exists(path_out))
						{
//								sign_file_mutex.unlock();
							if(bDebugLog)
								debuglog<<std::string(time_str()+": info: Failed to write sign in file");
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
							if(!gpg_launcher(params, boost::posix_time::seconds(15)))
							{
								return FALSE;
							}
							if(result == pxNotFound)
							{
								return FALSE;
							}
							boost::filesystem::remove(path_out);
							boost::filesystem::remove(status_file_out);
							if(out.find("key ID ") != string::npos)
							{
								//need to get hcontact here, i can get jid from hxml, and get handle from jid, maybe exists better way ?
								string::size_type p1 = out.find("key ID ") + strlen("key ID ");
								string::size_type p2 = out.find("\n", p1);
								if(p1 != string::npos && p2 != string::npos)
								{
									MCONTACT hContact = NULL;
									{
										extern list <JabberAccount*> Accounts;
										list <JabberAccount*>::iterator p = Accounts.begin();	
										for(unsigned int i = 0; i < Accounts.size(); i++, p++)
										{
											if(!(*p))
												break;
											hContact = (*p)->getJabberInterface()->ContactFromJID(xi.getAttrValue(node, _T("from")));
											if(hContact)
												hcontact_data[hContact].key_in_prescense = out.substr(p1, p2-p1-1).c_str();
										}
									}
								}
							}
						}
						return FALSE;
					}
				}
			}
		}
		local_node = xi.getChild(node, n);
	}
	return FALSE;
}

static JABBER_HANDLER_FUNC MessageHandler(IJabberInterface *ji, HXML node, void *pUserData)
{
	return FALSE;
}




void AddHandlers()
{
	extern list<JabberAccount*> Accounts;
	list<JabberAccount*>::iterator end = Accounts.end();
	for(list<JabberAccount*>::iterator p = Accounts.begin(); p != end; p++)
	{
		if(!(*p))
			break;
		if((*p)->getSendHandler() == INVALID_HANDLE_VALUE)
			(*p)->setSendHandler((*p)->getJabberInterface()->AddSendHandler((JABBER_HANDLER_FUNC)SendHandler));
		if((*p)->getPrescenseHandler() == INVALID_HANDLE_VALUE)
			(*p)->setPrescenseHandler((*p)->getJabberInterface()->AddPresenceHandler((JABBER_HANDLER_FUNC)PrescenseHandler));
//		if((*p)->getMessageHandler() == INVALID_HANDLE_VALUE)
//			(*p)->setMessageHandler((*p)->getJabberInterface()->AddMessageHandler((JABBER_HANDLER_FUNC)MessageHandler, JABBER_MESSAGE_TYPE_ANY ,NULL,NULL));
		if(bAutoExchange)
		{
			(*p)->getJabberInterface()->RegisterFeature(_T("GPG_Key_Auto_Exchange:0"), _T("Indicates that gpg installed and configured to public key auto exchange (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)"));
			(*p)->getJabberInterface()->AddFeatures(_T("GPG_Key_Auto_Exchange:0\0\0"));
		}
		if(bFileTransfers)
		{
			(*p)->getJabberInterface()->RegisterFeature(_T("GPG_Encrypted_FileTransfers:0"), _T("Indicates that gpg installed and configured to encrypt files (currently implemented in new_gpg plugin for Miranda IM and Miranda NG)"));
			(*p)->getJabberInterface()->AddFeatures(_T("GPG_Encrypted_FileTransfers:0\0\0"));
		}
	}
}

bool isContactSecured(MCONTACT hContact)
{
	BYTE gpg_enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
	if(!gpg_enc)
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": encryption is turned off for "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
		return false;
	}
	if(!metaIsProtoMetaContacts(hContact))
	{
		TCHAR *key = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", _T(""));
		if(!key[0])
		{
			mir_free(key);
			if(bDebugLog)
				debuglog<<std::string(time_str()+": encryption is turned off for "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			return false;
		}
		mir_free(key);
	}
	if(bDebugLog)
		debuglog<<std::string(time_str()+": encryption is turned on for "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
	return true;
}

bool isContactHaveKey(MCONTACT hContact)
{
	TCHAR *key = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", _T(""));
	if(_tcslen(key) > 0)
	{
		mir_free(key);
		return true;
	}
	mir_free(key);
	return false;
}

bool isGPGKeyExist()
{
	TCHAR *id = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", _T(""));
	char *key = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
	if(id[0] && key[0])
	{
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
	TCHAR *tmp = NULL;
	bool gpg_exists = false, is_valid = true;
	tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T(""));
	boost::filesystem::path p(tmp);

	if(boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p))
		gpg_exists = true;
	else
	{
		mir_free(tmp);
		tmp = NULL;
		TCHAR *path = (TCHAR*)mir_alloc(sizeof(TCHAR)*MAX_PATH);
		char *mir_path = (char*)mir_alloc(MAX_PATH);
		PathToAbsolute("\\", mir_path);
		SetCurrentDirectoryA(mir_path);
		tmp = mir_a2t(mir_path);
		mir_free(mir_path);
		//mir_realloc(path, (_tcslen(path)+64)*sizeof(TCHAR));
		TCHAR *gpg_path = (TCHAR*)mir_alloc(sizeof(TCHAR)*MAX_PATH);
		_tcscpy(gpg_path, tmp);
		_tcscat(gpg_path, _T("\\GnuPG\\gpg.exe"));
		mir_free(tmp);
		tmp = NULL;
		p = boost::filesystem::path(gpg_path);
		if(boost::filesystem::exists(p) && boost::filesystem::is_regular_file(p))
		{
			gpg_exists = true;
			_tcscpy(path, _T("GnuPG\\gpg.exe"));
		}
		mir_free(gpg_path);
		tmp = mir_tstrdup(path);
		mir_free(path);
	}
	DWORD len = MAX_PATH;
	if(gpg_exists)
	{
		db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
		mir_free(tmp);
		tmp = NULL;
		string out;
		DWORD code;
		std::vector<wstring> cmd;
		cmd.push_back(L"--version");
		gpg_execution_params params(cmd);
		pxResult result;
		params.out = &out;
		params.code = &code;
		params.result = &result;
		gpg_valid = true;
		gpg_launcher(params);
		gpg_valid = false;
		string::size_type p1 = out.find("(GnuPG) ");
		if(p1 == string::npos)
			is_valid = false;
	}
	if(tmp)
	{
		mir_free(tmp);
		tmp = NULL;
	}
/*	if(!gpg_exists)
	{
		wstring path_ = _wgetenv(_T("APPDATA"));
		path_ += _T("\\GnuPG");
		tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", (TCHAR*)path_.c_str());
	}
	if(tmp)
		mir_free(tmp); */
	return is_valid && gpg_exists;
}

#define NEWTSTR_MALLOC(A) (A==NULL)?NULL:strcpy((char*)mir_alloc(sizeof(char)*(strlen(A)+1)),A)

const bool StriStr(const char *str, const char *substr)
{
	bool i = false;
	char *str_up = NEWTSTR_MALLOC(str);
	char *substr_up = NEWTSTR_MALLOC(substr);

	CharUpperBuffA(str_up, (DWORD)strlen(str_up));
	CharUpperBuffA(substr_up, (DWORD)strlen(substr_up));

	if(strstr (str_up, substr_up))
		i = true;

	mir_free(str_up);
	mir_free(substr_up);
	
	return i;
}

bool IsOnline(MCONTACT hContact)
{
	if(db_get_b(hContact, szGPGModuleName, "Status", 0) == ID_STATUS_OFFLINE)
		return false;
	return true;
}

//from secureim
#include <process.h>

struct TFakeAckParams {
	inline TFakeAckParams( HANDLE p1, MCONTACT p2, LONG p3, LPCSTR p4 ) :
		hEvent( p1 ),
		hContact( p2 ),
		id( p3 ),
		msg( p4 )
		{}

	HANDLE hEvent;
	MCONTACT hContact;
	LONG id;
	LPCSTR msg;
};

__forceinline int SendBroadcast(MCONTACT hContact, int type, int result, HANDLE hProcess, LPARAM lParam)
{
	return ProtoBroadcastAck( GetContactProto(hContact), hContact, type, result, hProcess, lParam);
}

unsigned __stdcall sttFakeAck(void *param)
{
	TFakeAckParams *tParam = ( TFakeAckParams* )param;
	WaitForSingleObject( tParam->hEvent, INFINITE );

	Sleep( 100 );
	if ( tParam->msg == NULL )
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tParam->id, 0 );
	else
		SendBroadcast(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)tParam->id, LPARAM(tParam->msg));

	CloseHandle( tParam->hEvent );
	delete tParam;

	return 0;
}


int returnNoError(MCONTACT hContact) {
	HANDLE hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	unsigned int tID;
	CloseHandle( (HANDLE) _beginthreadex(NULL, 0, sttFakeAck, new TFakeAckParams(hEvent,hContact,777,0), 0, &tID) );
	SetEvent( hEvent );
	return 777;
}
// end from secureim



string toUTF8(wstring str)
{
	string ustr;
	try{
	utf8::utf16to8(str.begin(), str.end(), back_inserter(ustr));
	}
	catch(const utf8::exception& e)
	{
		if(bDebugLog)
			debuglog<<std::string("utf8cpp encoding exception: ")+(char*)e.what();
	    //TODO
	}
	return ustr;
}



wstring toUTF16(string str) //convert as much as possible
{
	wstring ustr;
	string tmpstr;
	try{
	utf8::replace_invalid(str.begin(), str.end(), back_inserter(tmpstr));
	utf8::utf8to16(tmpstr.begin(), tmpstr.end(), back_inserter(ustr));
	}
	catch(const utf8::exception& e)
	{
		if(bDebugLog)
			debuglog<<std::string("utf8cpp decoding exception: ")+(char*)e.what();
	    //TODO
	}
	return ustr;
}

string get_random(int length)
{
	string chars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
	string data;
	boost::random_device rng;
	boost::variate_generator<boost::random_device&, boost::uniform_int<>> gen(rng, boost::uniform_int<>(0, (int)chars.length()-1));
	for(int i = 0; i < length; ++i) 
        data += chars[gen()];
	return data;
}

void send_encrypted_msgs_thread(void *param)
{
	MCONTACT hContact = (MCONTACT)param;
	while(true)
	{
		//char *key = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", "");
		while(!isContactSecured(hContact))
			boost::this_thread::sleep(boost::posix_time::seconds(1));
		if(!hcontact_data[hContact].msgs_to_send.empty())
		{
			boost::this_thread::sleep(boost::posix_time::seconds(1));
			list<string>::iterator end = hcontact_data[hContact].msgs_to_send.end();
			extern std::list<HANDLE> sent_msgs;
			for(list<string>::iterator p = hcontact_data[hContact].msgs_to_send.begin(); p != end; ++p)
			{
				sent_msgs.push_back((HANDLE)CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)p->c_str()));
				HistoryLog(hContact, db_event((char*)p->c_str(),0,0, DBEF_SENT));
				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
			hcontact_data[hContact].msgs_to_send.clear();
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

int handleEnum(const char *szSetting, LPARAM lParam)
{
	if(!*(bool*)lParam && szSetting[0] && StriStr(szSetting, "tabsrmm"))
	{
		bool f = false, *found = (bool*)lParam;
		f = !db_get_b(NULL, "PluginDisable", szSetting, 0); 
		if(f)
			*found = f;
	}
	return 0;
}

bool isTabsrmmUsed()
{
	DBCONTACTENUMSETTINGS enm = {0};
	bool found = false;
	enm.lParam = (LPARAM)&found;
	enm.pfnEnumProc = handleEnum;
	enm.szModule = "PluginDisable";
	if(CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&enm) == -1)
		return false;

	return found;
}

void ExportGpGKeysFunc(int type)
{
		TCHAR *p = GetFilePath(_T("Choose file to export keys"), _T("*"), _T("Any file"), true);
	if(!p || !p[0])
	{
		delete [] p;
		//TODO: handle error
		return;
	}
	char *path = mir_t2a(p);
	delete [] p;
	std::ofstream file;
	file.open(path, std::ios::trunc | std::ios::out);
	mir_free(path);
	int exported_keys = 0;
	if(!file.is_open())
		return; //TODO: handle error
	if(!type || type == 2) {
		for(MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
			char *k = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", "");
			std::string key;
			if(!k[0])
			{
				mir_free(k);
				continue;
			}
			else
			{
				key = k;
				mir_free(k);
			}
			
			const char* proto = (const char*)GetContactProto(hContact);
			std::string id = "Comment: login ";
			const char * uid = (const char*)CallProtoService(proto, PS_GETCAPS,  (WPARAM)PFLAG_UNIQUEIDSETTING, 0);
			DBVARIANT dbv = {0};
			db_get(0, proto, uid, &dbv);
			switch(dbv.type) {
			case DBVT_DELETED:
				continue;

			case DBVT_BYTE:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.bVal);
					id += _id;
				}
				break;
			case DBVT_WORD:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.wVal);
					id += _id;
				}
				break;
			case DBVT_DWORD:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.dVal);
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
					if(tmp[0])
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
			ZeroMemory(&dbv, sizeof(dbv));
			db_get(hContact, proto, uid, &dbv);
			switch(dbv.type) {
			case DBVT_DELETED:
				continue;
			case DBVT_BYTE:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.bVal);
					id += _id;
				}
				break;
			case DBVT_WORD:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.wVal);
					id += _id;
				}
				break;
			case DBVT_DWORD:
				{
					char _id[64];
					mir_snprintf(_id, 64, "%d", dbv.dVal);
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
					if(tmp[0])
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
			if(p1 == std::string::npos)
				continue;
			p1 += strlen("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			p1 ++;
			id += '\n';
			key.insert(p1, id);
			file<<key;
			file<<std::endl;
			exported_keys++;
		}
	}
	if(type == 1 || type == 2)
	{
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
			file<<out;
			file<<std::endl;
		}
	}
	if(file.is_open())
		file.close();
	TCHAR msg[512];
	if(type == 2)
		mir_sntprintf(msg, 512, TranslateT("We have successfully exported %d public keys and all private keys."), exported_keys);
	else if(type == 1)
		mir_sntprintf(msg, 512, TranslateT("We have successfully exported all private keys."));
	else if(!type)
		mir_sntprintf(msg, 512, TranslateT("We have successfully exported %d public keys."), exported_keys);
	MessageBox(NULL, msg, TranslateT("Keys export result"), MB_OK);
}

INT_PTR ExportGpGKeys(WPARAM, LPARAM)
{
	ShowExportKeysDlg();
	return 0;
}

INT_PTR ImportGpGKeys(WPARAM w, LPARAM l)
{
	TCHAR *p = GetFilePath(_T("Choose file to import keys from"), _T("*"), _T("Any file"));
	if(!p || !p[0])
	{
		delete [] p;
		//TODO: handle error
		return 1;
	}
	char *path = mir_t2a(p);
	delete [] p;
	std::ifstream file;
	file.open(path, std::ios::in);
	mir_free(path);
	if(!file.is_open())
		return 1; //TODO: handle error
	PROTOACCOUNT **accs;
	int acc_count = 0, processed_keys = 0, processed_private_keys = 0;
	ProtoEnumAccounts(&acc_count, &accs);
	char line[256];
	file.getline(line, 255);
	if(!strstr(line, "-----BEGIN PGP PUBLIC KEY BLOCK-----") && !strstr(line, "-----BEGIN PGP PRIVATE KEY BLOCK-----"))
		return 1; //TODO: handle error
	std::string key, login, contact_id;
	key += line;
	key += '\n';
	while(file.is_open() && !file.eof())
	{
		file.getline(line, 255);
		key += line;
		key += '\n';
		if(strstr(line, "-----END PGP PUBLIC KEY BLOCK-----"))
		{
			std::string::size_type p1 = 0, p2 = 0;
			p1 = key.find("Comment: login ");
			p1 += strlen("Comment: login ");
			p2 = key.find(" contact_id ");
			login = key.substr(p1, p2-p1);
			p2 += strlen(" contact_id ");
			p1 = key.find("\n", p2);
			contact_id = key.substr(p2, p1-p2);
			p1 = key.find("Comment: login ");
			p2 = key.find("\n", p1);
			p2++;
			key.erase(p1, p2-p1);
			std::string acc;
			for(int i = 0; i < acc_count; i++)
			{
				if(acc.length())
					break;
				const char * uid = (const char*)CallProtoService(accs[i]->szModuleName, PS_GETCAPS,  (WPARAM)PFLAG_UNIQUEIDSETTING, 0);
				DBVARIANT dbv = {0};
				db_get(0, accs[i]->szModuleName, uid, &dbv);
				std::string id;
				switch(dbv.type)
				{
				case DBVT_DELETED:
					continue;
					break;
				case DBVT_BYTE:
					{
						char _id[64];
						mir_snprintf(_id, 64, "%d", dbv.bVal);
						id += _id;
						if(id == login)
							acc = accs[i]->szModuleName;
					}
					break;
				case DBVT_WORD:
					{
						char _id[64];
						mir_snprintf(_id, 64, "%d", dbv.wVal);
						id += _id;
						if(id == login)
							acc = accs[i]->szModuleName;
					}
					break;
				case DBVT_DWORD:
					{
						char _id[64];
						mir_snprintf(_id, 64, "%d", dbv.dVal);
						id += _id;
						if(id == login)
							acc = accs[i]->szModuleName;
					}
					break;
				case DBVT_ASCIIZ:
					{
						id += dbv.pszVal;
						db_free(&dbv);
						if(id == login)
							acc = accs[i]->szModuleName;
					}
					break;
				case DBVT_UTF8:
					{
						char *tmp = mir_utf8decodeA(dbv.pszVal);
						if(tmp[0])
							id += tmp;
						mir_free(tmp);
						db_free(&dbv);
						if(id == login)
							acc = accs[i]->szModuleName;
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
			if(acc.length())
			{
				const char * uid = (const char*)CallProtoService(acc.c_str(), PS_GETCAPS,  (WPARAM)PFLAG_UNIQUEIDSETTING, 0);
				for(MCONTACT hContact = db_find_first(acc.c_str()); hContact; hContact = db_find_next(hContact, acc.c_str())) {
					DBVARIANT dbv = {0};
					db_get(hContact, acc.c_str(), uid, &dbv);
					std::string id;
					bool found = false;
					switch(dbv.type)
					{
					case DBVT_DELETED:
						continue;
						break;
					case DBVT_BYTE:
						{
							char _id[64];
							mir_snprintf(_id, 64, "%d", dbv.bVal);
							id += _id;
							if(id == contact_id)
								found = true;
						}
						break;
					case DBVT_WORD:
						{
							char _id[64];
							mir_snprintf(_id, 64, "%d", dbv.wVal);
							id += _id;
							if(id == contact_id)
								found = true;
						}
						break;
					case DBVT_DWORD:
						{
							char _id[64];
							mir_snprintf(_id, 64, "%d", dbv.dVal);
							id += _id;
							if(id == contact_id)
								found = true;
						}
						break;
					case DBVT_ASCIIZ:
						{
							id += dbv.pszVal;
							db_free(&dbv);
							if(id == contact_id)
								found = true;
						}
						break;
					case DBVT_UTF8:
						{
							char *tmp = mir_utf8decodeA(dbv.pszVal);
							if(tmp[0])
								id += tmp;
							mir_free(tmp);
							db_free(&dbv);
							if(id == contact_id)
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
					if(found)
					{
						wstring path;
						std::vector<std::wstring> cmd;
						TCHAR *ptmp;
						string output;
						DWORD exitcode;
						{
							MCONTACT hcnt = hContact;
							ptmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
							path = ptmp;
							mir_free(ptmp);
							wstring rand = toUTF16(get_random(10));
							path += L"\\";
							path += rand;
							boost::filesystem::remove(path);
							wfstream f(path, std::ios::out);
							f<<toUTF16(key).c_str();
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
						if(!gpg_launcher(params))
							break;
						if(result == pxNotFound)
							break;
						if(result == pxSuccess)
							processed_keys++;
						{
							if(output.find("already in secret keyring") != string::npos)
							{
								MessageBox(0, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
								boost::filesystem::remove(path);
								break;
							}
							char *tmp2;
							string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
							string::size_type s2 = output.find(":", s);
							tmp2 = (char*)mir_alloc((output.substr(s,s2-s).length()+1) * sizeof(char));
							strcpy(tmp2, output.substr(s,s2-s).c_str());
							mir_utf8decode(tmp2, 0);
							db_set_s(hContact, szGPGModuleName, "KeyID", tmp2);
							mir_free(tmp2);
							s = output.find("“", s2);
							if(s == string::npos)
							{
								s = output.find("\"", s2);
								s += 1;
							}
							else
								s += 3;
							if((s2 = output.find("(", s)) == string::npos)
								s2 = output.find("<", s);
							else if(s2 > output.find("<", s))
								s2 = output.find("<", s);
							if(s2 != string::npos)
							{
								tmp2 = (char*)mir_alloc((output.substr(s,s2-s-1).length()+1) * sizeof(char));
								strcpy(tmp2, output.substr(s,s2-s-1).c_str());
								mir_utf8decode(tmp2, 0);
								if(hContact)
								{
									db_set_s(hContact, szGPGModuleName, "KeyMainName", output.substr(s,s2-s-1).c_str());
								}
								mir_free(tmp2);
								if((s = output.find(")", s2)) == string::npos)
									s = output.find(">", s2);
								else if(s > output.find(">", s2))
									s = output.find(">", s2);
								s2++;
								if(output[s] == ')')
								{
									tmp2 = (char*)mir_alloc((output.substr(s2,s-s2).length()+1) * sizeof(char));
									strcpy(tmp2, output.substr(s2,s-s2).c_str());
									mir_utf8decode(tmp2, 0);
									if(hContact)
										db_set_s(hContact, szGPGModuleName, "KeyComment", output.substr(s2,s-s2).c_str());
									mir_free(tmp2);
									s+=3;
									s2 = output.find(">", s);
									tmp2 = (char*)mir_alloc((output.substr(s,s2-s).length()+1) * sizeof(char));
									strcpy(tmp2, output.substr(s,s2-s).c_str());
									mir_utf8decode(tmp2, 0);
									if(hContact)
										db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s,s2-s).c_str());
									mir_free(tmp2);
								}
								else
								{
									tmp2 = (char*)mir_alloc((output.substr(s2,s-s2).length()+1) * sizeof(char));
									strcpy(tmp2, output.substr(s2,s-s2).c_str());
									mir_utf8decode(tmp2, 0);
									if(hContact)
										db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
									mir_free(tmp2);
								}
							}
							db_set_b(hContact, szGPGModuleName, "GPGEncryption", 1);
							db_set_ts(hContact, szGPGModuleName, "GPGPubKey", toUTF16(key).c_str());
						}
						boost::filesystem::remove(path);
						break;
					}
				}
			}
			key.clear();
		}
		if(strstr(line, "-----END PGP PRIVATE KEY BLOCK-----"))
		{
			std::vector<wstring> cmd;
			TCHAR tmp2[MAX_PATH] = {0};
			TCHAR *ptmp;
			string output;
			DWORD exitcode;
			{
				ptmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				_tcscpy(tmp2, ptmp);
				mir_free(ptmp);
				_tcscat(tmp2, _T("\\"));
				_tcscat(tmp2, _T("temporary_exported.asc"));
				boost::filesystem::remove(tmp2);
				wfstream f(tmp2, std::ios::out);
				f<<toUTF16(key).c_str();
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
			if(!gpg_launcher(params))
				break;
			if(result == pxNotFound)
				break;
			if(result == pxSuccess)
				processed_private_keys++;
			key.clear();
		}
	}
	if(file.is_open())
		file.close();
	TCHAR msg[512];
	if(processed_private_keys)
		mir_sntprintf(msg, 512, TranslateT("We have successfully processed %d public keys and some private keys."), processed_keys);
	else
		mir_sntprintf(msg, 512, TranslateT("We have successfully processed %d public keys."), processed_keys);
	MessageBox(NULL, msg, TranslateT("Keys import result"), MB_OK);
	return 0;
}

void fix_line_term(std::string &s)
{
	if(s.empty())
		return;
	boost::algorithm::erase_all(s, "\r\r");
}

void fix_line_term(std::wstring &s)
{
	if(s.empty())
		return;
	boost::algorithm::erase_all(s, _T("\r\r"));
}

void strip_line_term(std::wstring &s)
{
	if(s.empty())
		return;
	boost::algorithm::erase_all(s, _T("\r"));
	boost::algorithm::erase_all(s, _T("\n"));
}

void strip_line_term(std::string &s)
{
	if(s.empty())
		return;
	boost::algorithm::erase_all(s, "\r");
	boost::algorithm::erase_all(s, "\n");
}

void strip_tags(std::wstring &str)
{
	if(str.empty())
		return;
	boost::algorithm::erase_all(str, inopentag);
	boost::algorithm::erase_all(str, inclosetag);
	boost::algorithm::erase_all(str, outopentag);
	boost::algorithm::erase_all(str, outclosetag);
}


static INT_PTR CALLBACK DlgProcEncryptedFileMsgBox(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *inkeyid = NULL;
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
		file_msg_state = -1;
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case IDC_IGNORE:
		  if(IsDlgButtonChecked(hwndDlg, IDC_REMEMBER))
		  {
			  db_set_b(NULL, szGPGModuleName, "bSameAction", 1);
			  bSameAction = true;
		  }
		  DestroyWindow(hwndDlg);
		  break;

	  case IDC_DECRYPT:
		  file_msg_state = 1;
		  if(IsDlgButtonChecked(hwndDlg, IDC_REMEMBER))
		  {
			  db_set_b(NULL, szGPGModuleName, "bFileTransfers", 1);
			  bFileTransfers = true;
			  db_set_b(NULL, szGPGModuleName, "bSameAction", 0);
			  bSameAction = false;
		  }
			  
		  DestroyWindow(hwndDlg);
		  break;

	  default:
		break;
      }
      
      break;
    }
    
  case WM_NOTIFY:
    {
	}
    break;
  case WM_CLOSE:
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  {
	  }
	  break;
  }
  return FALSE;
}


void ShowEncryptedFileMsgBox()
{
	extern HINSTANCE hInst;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ENCRYPTED_FILE_MSG_BOX), NULL, DlgProcEncryptedFileMsgBox);
}


static INT_PTR CALLBACK DlgProcExportKeys(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case IDC_OK:
		  if(IsDlgButtonChecked(hwndDlg, IDC_PUBLIC))
			  ExportGpGKeysFunc(0);
		  else if(IsDlgButtonChecked(hwndDlg, IDC_PRIVATE))
			  ExportGpGKeysFunc(1);
		  else if(IsDlgButtonChecked(hwndDlg, IDC_ALL))
			  ExportGpGKeysFunc(2);
		  DestroyWindow(hwndDlg);
		  break;

	  case IDC_CANCEL:
		  DestroyWindow(hwndDlg);
		  break;

	  default:
		break;
      }
      
      break;
    }
    
  case WM_NOTIFY:
    {
	}
    break;
  case WM_CLOSE:
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  {
	  }
	  break;
  }
  return FALSE;
}

void ShowExportKeysDlg()
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_EXPORT_TYPE), NULL, DlgProcExportKeys);
}

static INT_PTR CALLBACK DlgProcChangePasswd(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case IDC_OK:
		  //TODO: show some prgress
		  {
			  std::string old_pass, new_pass;
			  extern TCHAR key_id_global[17];
			  TCHAR buf[256] = {0};
			  GetDlgItemText(hwndDlg, IDC_NEW_PASSWD1, buf, 255);
			  new_pass = toUTF8(buf);
			  GetDlgItemText(hwndDlg, IDC_NEW_PASSWD2, buf, 255);
			  if(new_pass != toUTF8(buf))
			  {
				  MessageBox(hwndDlg, TranslateT("New passwords do not match"), TranslateT("Error"), MB_OK);
				  //key_id_global[0] = 0;
				  break;
			  }
			  GetDlgItemText(hwndDlg, IDC_OLD_PASSWD, buf, 255);
			  old_pass = toUTF8(buf);
			  bool old_pass_match = false;
			  TCHAR *pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
			  if(!_tcscmp(pass,buf))
				  old_pass_match = true;
			  mir_free(pass);
			  if(!old_pass_match)
			  {
				  if(key_id_global[0])
				  {
					  string dbsetting = "szKey_";
					  dbsetting += toUTF8(key_id_global);
					  dbsetting += "_Password";
					  pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), _T(""));
					  if(!_tcscmp(pass,buf))
						  old_pass_match = true;
					  mir_free(pass);
				  }
			  }
			  if(!old_pass_match)
			  {
				  if(MessageBox(hwndDlg, TranslateT("Old password does not match, you can continue, but GPG will reject wrong password.\nDo you want to continue?"), TranslateT("Error"), MB_YESNO) == IDNO)
				  {
					  //key_id_global[0] = 0;
					  break;
				  }
			  }
			  std::vector<std::wstring> cmd;
			  TCHAR tmp2[MAX_PATH] = {0};
			  string output;
			  DWORD exitcode;
			  cmd.push_back(L"--edit-key");
			  cmd.push_back(key_id_global);
			  cmd.push_back(L"passwd");
			  gpg_execution_params_pass params(cmd, old_pass, new_pass);
			  pxResult result;
			  params.out = &output;
			  params.code = &exitcode;
			  params.result = &result;
			  boost::thread gpg_thread(boost::bind(&pxEexcute_passwd_change_thread, &params));
			  if(!gpg_thread.timed_join(boost::posix_time::minutes(10)))
			  {
				  gpg_thread.~thread();
				  if(params.child)
					  boost::process::terminate(*(params.child));
				  if(bDebugLog)
					  debuglog<<std::string(time_str()+": GPG execution timed out, aborted");
				  DestroyWindow(hwndDlg);
				  break;
			  }
			  if(result == pxNotFound)
				  break;
			  //if(result == pxSuccess)
			  //TODO: save to db


		  }
		  DestroyWindow(hwndDlg);
		  break;
	  default:
		break;
      }
      
      break;
    }
    
  case WM_NOTIFY:
    {
	}
    break;
  case WM_CLOSE:
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  {
		  extern TCHAR key_id_global[17];
		  key_id_global[0] = 0;
	  }
	  break;
  }
  return FALSE;
}

void ShowChangePasswdDlg()
{
	extern HINSTANCE hInst;
	HWND hwndPaaswdDlg = NULL;
	hwndPaaswdDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHANGE_PASSWD), NULL, DlgProcChangePasswd);
	SetForegroundWindow(hwndPaaswdDlg);
}


void clean_temp_dir()
{
	using namespace boost::filesystem;
	char *mir_path = new char [MAX_PATH];
	PathToAbsolute("\\", mir_path);
	wstring path  = toUTF16(mir_path);
	SetCurrentDirectoryA(mir_path);
	delete [] mir_path;
	TCHAR *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
	path += tmp;
	mir_free(tmp);
	path += L"\\tmp";
	if(exists(path) && is_directory(path))
	{
		boost::filesystem::path p(path);
		for(directory_iterator i = directory_iterator(p), end = directory_iterator(); i != end; ++i)
		{
			if(boost::filesystem::is_regular_file(i->path()))
			{
				if((i->path().filename().generic_string().length() == 10 && (i->path().filename().generic_string().find(".") == std::string::npos)) ||
					i->path().extension() == ".sig" || i->path().extension() == ".asc" || i->path().extension() == ".status")
					boost::filesystem::remove(i->path());
			}
		}
	}
}