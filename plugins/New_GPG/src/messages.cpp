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

#include "commonheaders.h"


wstring new_key;
MCONTACT new_key_hcnt = NULL;
boost::mutex new_key_hcnt_mutex;
bool _terminate = false;
int returnNoError(MCONTACT hContact);

std::list<HANDLE> sent_msgs;

void RecvMsgSvc_func(MCONTACT hContact, std::wstring str, char *msg, DWORD flags, DWORD timestamp)
{		
	DWORD dbflags = DBEF_UTF;
	{ //check for gpg related data
		wstring::size_type s1 = wstring::npos, s2 = wstring::npos;

		s1 = str.find(_T("-----BEGIN PGP MESSAGE-----"));
		s2 = str.find(_T("-----END PGP MESSAGE-----"));
		if((s2 != wstring::npos) && (s1 != wstring::npos))
		{ //this is generic encrypted data block
			if(!isContactSecured(hContact))
			{
				if(bDebugLog)
					debuglog<<std::string(time_str()+": info: received encrypted message from: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" with turned off encryption");
				if(MessageBox(0, TranslateT("We received encrypted message from contact with encryption turned off.\nDo you want to turn on encryption for this contact?"), TranslateT("Warning"), MB_YESNO) == IDYES)
				{
					if(!isContactHaveKey(hContact))
					{
						void ShowLoadPublicKeyDialog();
						item_num = 0;		 //black magic here
						user_data[1] = hContact;
						ShowLoadPublicKeyDialog();
					}
					else
					{
						db_set_b(db_mc_isMeta(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 1);
						setSrmmIcon(hContact);
						setClistIcon(hContact);
					}
					if(isContactHaveKey(hContact))
					{
						db_set_b(db_mc_isMeta(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 1);
						setSrmmIcon(hContact);
						setClistIcon(hContact);
					}
				}
				else if(MessageBox(0, TranslateT("Do you want to try to decrypt encrypted message?"), TranslateT("Warning"), MB_YESNO) == IDNO)
				{
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					return;
				}
			}
			else if(bDebugLog)
			{
				debuglog<<std::string(time_str()+": info: received encrypted message from: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			}
			boost::algorithm::erase_all(str, "\r");
			s2 += mir_tstrlen(_T("-----END PGP MESSAGE-----"));
			char *tmp = mir_t2a(str.substr(s1,s2-s1).c_str());
			TCHAR *tmp2 = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			wstring path = tmp2;
			wstring encfile = toUTF16(get_random(10));
			wstring decfile = toUTF16(get_random(10));
			path.append(_T("\\tmp\\"));
			path.append(encfile);
			boost::filesystem::remove(path);
			fstream f(path.c_str(), std::ios::out);
			while(!f.is_open())
				f.open(path.c_str(), std::ios::out);
			f<<tmp;
			mir_free(tmp);
			f.close();
			{
				extern TCHAR *password;
				string out;
				DWORD code;
				std::vector<wstring> cmd;
				cmd.push_back(L"--batch");
				{
					char *inkeyid = UniGetContactSettingUtf(db_mc_isMeta(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "InKeyID", "");
					TCHAR *pass = NULL;
					if(inkeyid[0])
					{
						string dbsetting = "szKey_";
						dbsetting += inkeyid;
						dbsetting += "_Password";
						pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), _T(""));
						if(pass[0] && bDebugLog)
							debuglog<<std::string(time_str()+": info: found password in database for key ID: "+inkeyid+", trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" with password");
					}
					else
					{
						pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
						if(pass[0] && bDebugLog)
							debuglog<<std::string(time_str()+": info: found password for all keys in database, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" with password");
					}
					if(pass && pass[0])
					{
						cmd.push_back(L"--passphrase");
						cmd.push_back(pass);
					}
					else if(password && password[0])
					{
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: found password in memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" with password");
						cmd.push_back(L"--passphrase");
						cmd.push_back(password);
					}
					else if (bDebugLog)
						debuglog<<std::string(time_str()+": info: passwords not found in database or memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" with out password");
					mir_free(pass);
					mir_free(inkeyid);
				}
				{
					wstring path = tmp2;
					path += _T("\\tmp\\");
					path += decfile;
					boost::filesystem::remove(path);
				}
				cmd.push_back(L"--output");
				cmd.push_back(std::wstring(tmp2) + L"\\tmp\\" + decfile);
				cmd.push_back(L"-d");
				cmd.push_back(L"-a");
				cmd.push_back(path);
				gpg_execution_params params(cmd);
				pxResult result;
				params.out = &out;
				params.code = &code;
				params.result = &result;
				if(!gpg_launcher(params))
				{
					boost::filesystem::remove(path);
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
					CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
					HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
					return;
				}
				if(result == pxNotFound)
				{
					boost::filesystem::remove(path);
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					return;
				}
				if(result == pxSuccessExitCodeInvalid)
				{
					boost::filesystem::remove(path);
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					HistoryLog(hContact, db_event(Translate("failed to decrypt message, GPG returned error, turn on debug log for more details"), timestamp, 0, 0));
					return;
				}

				//TODO: check gpg output for errors
				_terminate = false;
				while(out.find("public key decryption failed: bad passphrase") != string::npos)
				{
					if(bDebugLog)
						debuglog<<std::string(time_str()+": info: failed to decrypt messaage from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" password needed, trying to get one");
					if(_terminate)
					{
						BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
						break;
					}
					{ //save inkey id
						string::size_type s = out.find(" encrypted with ");
						s = out.find(" ID ", s);
						s += mir_strlen(" ID ");
						string::size_type s2 = out.find(",",s);
						db_set_s(db_mc_isMeta(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "InKeyID", out.substr(s, s2-s).c_str());
					}
					void ShowLoadKeyPasswordWindow();
					new_key_hcnt_mutex.lock();
					new_key_hcnt = hContact;
					ShowLoadKeyPasswordWindow();
					std::vector<wstring> cmd2 = cmd;
					if(password)
					{
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: found password in memory, trying to decrypt message from "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
						std::vector<wstring> tmp;
						tmp.push_back(L"--passphrase");
						tmp.push_back(password);
						cmd2.insert(cmd2.begin(), tmp.begin(), tmp.end());
					}
					out.clear();
					gpg_execution_params params(cmd2);
					pxResult result;
					params.out = &out;
					params.code = &code;
					params.result = &result;
					if(!gpg_launcher(params))
					{
						boost::filesystem::remove(path);
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
						return;
					}
					if(result == pxNotFound)
					{
						boost::filesystem::remove(path);
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						return;
					}
				}
				out.clear();
				if(!gpg_launcher(params))
				{
					boost::filesystem::remove(path);
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
					CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
					HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
					db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
					return;
				}
				if(result == pxNotFound)
				{
					boost::filesystem::remove(path);
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
				}
				{
					wstring tmp = tmp2;
					tmp += _T("\\tmp\\");
					tmp += encfile;
					boost::filesystem::remove(tmp);
				}
				{
					wstring tmp = tmp2;
					tmp += _T("\\tmp\\");
					tmp += decfile;
					if(!boost::filesystem::exists(tmp))
					{
						string str = msg;
						str.insert(0, "Received unencrypted message:\n");
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: Failed to decrypt GPG encrypted message.");
						char *tmp = (char*)mir_alloc(sizeof(char)*(str.length()+1));
						strcpy(tmp, str.c_str());
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
						mir_free(tmp);
						return;
					}
				}
				str.clear();
				{
					wstring path = tmp2;
					mir_free(tmp2);
					path += _T("\\tmp\\");
					path += decfile;
					fstream f(path.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
					if(f.is_open())
					{
						std::wifstream::pos_type size = f.tellg();
						char *tmp = new char [(std::ifstream::pos_type)size+(std::ifstream::pos_type)1];
						f.seekg(0, std::ios::beg);
						f.read(tmp, size);
						tmp[size] = '\0';
						toUTF16(tmp);
						str.append(toUTF16(tmp));
						delete [] tmp;
						f.close();
						boost::filesystem::remove(path);
					}
					if(str.empty())
					{
						string str = msg;
						str.insert(0, "Failed to decrypt GPG encrypted message.\nMessage body for manual decryption:\n");
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: Failed to decrypt GPG encrypted message.");
						char *tmp = mir_strdup(str.c_str());
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						BYTE enc = db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						db_set_b(hContact, szGPGModuleName, "GPGEncryption", enc);
						mir_free(tmp);
						return;
					}
					else
					{
						fix_line_term(str);
						if(bAppendTags)
						{
							str.insert(0, inopentag);
							str.append(inclosetag);
						}

						char *tmp = mir_strdup(toUTF8(str).c_str());
						HistoryLog(hContact, db_event(tmp, timestamp, 0, dbflags));
						mir_free(tmp);
						return;
					}
				}
			}
		}
	}
	if(db_get_b(db_mc_isMeta(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 0))
	{
		HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags|DBEF_READ));
		return;
	}
	HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
	return;
}

INT_PTR RecvMsgSvc(WPARAM w, LPARAM l)
{
	CCSDATA *ccs = (CCSDATA*)l;
	if (!ccs)
		return CallService(MS_PROTO_CHAINRECV, w, l);
	PROTORECVEVENT *pre = (PROTORECVEVENT*)(ccs->lParam);
	if (!pre)
		return CallService(MS_PROTO_CHAINRECV, w, l);
	char *msg = pre->szMessage;
	if (!msg)
		return CallService(MS_PROTO_CHAINRECV, w, l);
	DWORD dbflags = DBEF_UTF;
	if(db_mc_isMeta(ccs->hContact))
	{
		if(!strstr(msg, "-----BEGIN PGP MESSAGE-----"))
			return CallService(MS_PROTO_CHAINRECV, w, l);
		else
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info: blocked pgp message to metacontact:"+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
			return 0;
		}
	}
	wstring str = toUTF16(msg);
	wstring::size_type s1 = wstring::npos, s2 = wstring::npos;
	if(bAutoExchange && (str.find(_T("-----PGP KEY RESPONSE-----")) != wstring::npos))
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info(autoexchange): parsing key response:"+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
		s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"));
		s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
		if(s1 != wstring::npos && s2 != wstring::npos)
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info(autoexchange): found pubkey block:"+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
			s2 += mir_tstrlen(_T("-----END PGP PUBLIC KEY BLOCK-----"));
			db_set_ts(ccs->hContact, szGPGModuleName, "GPGPubKey", str.substr(s1,s2-s1).c_str());
			{ //gpg execute block
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
					TCHAR *tmp3 = mir_a2t(get_random(5).c_str());
					_tcscat(tmp2, tmp3);
					_tcscat(tmp2, _T(".asc"));
					mir_free(tmp3);
					//_tcscat(tmp2, _T("temporary_exported.asc"));
					boost::filesystem::remove(tmp2);
					wfstream f(tmp2, std::ios::out);
					while(!f.is_open())
						f.open(tmp2, std::ios::out);
					ptmp = UniGetContactSettingUtf(ccs->hContact, szGPGModuleName, "GPGPubKey", _T(""));
					wstring new_key = ptmp;
					mir_free(ptmp);
					f<<new_key.c_str();
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
					return 1;
				boost::filesystem::remove(tmp2);
				if(result == pxNotFound)
					return 1;
				if(result == pxSuccessExitCodeInvalid)
				{
					HistoryLog(ccs->hContact, db_event(Translate("failed to decrypt message, GPG returned error, turn on debug log for more details")));
					return 1;
				}
				{
					char *tmp = NULL;
					string::size_type s = output.find("gpg: key ") + mir_strlen("gpg: key ");
					string::size_type s2 = output.find(":", s);
					db_set_s(ccs->hContact, szGPGModuleName, "KeyID", output.substr(s,s2-s).c_str());
					s2+=2;
					s = output.find("â€œ", s2);
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
					tmp = (char*)mir_alloc(output.substr(s,s2-s-1).length()+1);
					strcpy(tmp, output.substr(s,s2-s-1).c_str());
					mir_utf8decode(tmp, 0);
					db_set_s(ccs->hContact, szGPGModuleName, "KeyMainName", tmp);
					mir_free(tmp);
					if((s = output.find(")", s2)) == string::npos)
						s = output.find(">", s2);
					else if(s > output.find(">", s2))
						s = output.find(">", s2);
					s2++;
					if(output[s] == ')')
					{
						tmp = (char*)mir_alloc(output.substr(s2,s-s2).length()+1);
						strcpy(tmp, output.substr(s2,s-s2).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(ccs->hContact, szGPGModuleName, "KeyComment", tmp);
						mir_free(tmp);
						s+=3;
						s2 = output.find(">", s);
						tmp = (char*)mir_alloc(output.substr(s,s2-s).length()+1);
						strcpy(tmp, output.substr(s,s2-s).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(ccs->hContact, szGPGModuleName, "KeyMainEmail", tmp);
						mir_free(tmp);
					}
					else
					{
						tmp = (char*)mir_alloc(output.substr(s2,s-s2).length()+1);
						strcpy(tmp, output.substr(s2,s-s2).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(ccs->hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
						mir_free(tmp);
					}
					db_set_b(ccs->hContact, szGPGModuleName, "GPGEncryption", 1);
					db_set_b(ccs->hContact, szGPGModuleName, "bAlwatsTrust", 1);
					setSrmmIcon(ccs->hContact);
					setClistIcon(ccs->hContact);
					if(db_mc_isSub(ccs->hContact))
					{
						setSrmmIcon(db_mc_getMeta(ccs->hContact));
						setClistIcon(db_mc_getMeta(ccs->hContact));
					}
					HistoryLog(ccs->hContact, "PGP Encryption turned on by key autoexchange feature");
				}
				}
				return 1;
			}
		}
		if(((s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"))) == wstring::npos) || ((s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"))) == wstring::npos))
		{
			s2 = str.find(_T("-----END PGP PRIVATE KEY BLOCK-----"));
			s1 = str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
		}
		if((s2 != wstring::npos) && (s1 != wstring::npos))
		{  //this is public key
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info: received key from: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
			s1 = 0;
			while((s1 = str.find(_T("\r"), s1)) != wstring::npos)
				str.erase(s1, 1);
			void ShowNewKeyDialog();
			if(((s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"))) != wstring::npos) && ((s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"))) != wstring::npos))
			{
				s2 += mir_tstrlen(_T("-----END PGP PUBLIC KEY BLOCK-----"));
			}
			else if(((s2 = str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"))) != wstring::npos) && ((s1 = str.find(_T("-----END PGP PRIVATE KEY BLOCK-----"))) != wstring::npos))
			{
				s2 += mir_tstrlen(_T("-----END PGP PRIVATE KEY BLOCK-----"));
			}
			new_key.append(str.substr(s1,s2-s1));
			//new_key_hcnt_mutex.lock();
			new_key_hcnt = ccs->hContact;
			ShowNewKeyDialog();
			HistoryLog(ccs->hContact, db_event(msg, 0, 0, dbflags));
			return 0;
		}
		if(bAutoExchange && strstr(msg, "-----PGP KEY REQUEST-----") && gpg_valid && gpg_keyexist)
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info(autoexchange): received key request from: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
			char *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
			if(tmp[0])
			{
				int enc_state = db_get_b(ccs->hContact, szGPGModuleName, "GPGEncryption", 0);
				if(enc_state)
					db_set_b(ccs->hContact, szGPGModuleName, "GPGEncryption", 0);
				string str = "-----PGP KEY RESPONSE-----";
				str.append(tmp);
				CallContactService(ccs->hContact, PSS_MESSAGE, 0, (LPARAM)str.c_str());
				if(enc_state)
					db_set_b(ccs->hContact, szGPGModuleName, "GPGEncryption", 1);
			}
			mir_free(tmp);
			return 0;
		}
		else if(!isContactHaveKey(ccs->hContact) && bAutoExchange && gpg_valid && gpg_keyexist)
		{
			char *proto = GetContactProto(ccs->hContact);
			DWORD uin = db_get_dw(ccs->hContact, proto, "UIN", 0);
			if(uin) {
				if( ProtoServiceExists(proto, PS_ICQ_CHECKCAPABILITY)) {
					ICQ_CUSTOMCAP cap = {0};
					strncpy(cap.caps, "GPGAutoExchange", sizeof(cap.caps));
					if(ProtoCallService(proto, PS_ICQ_CHECKCAPABILITY, (WPARAM)ccs->hContact, (LPARAM)&cap)) {
						CallContactService(ccs->hContact, PSS_MESSAGE, 0, (LPARAM)"-----PGP KEY REQUEST-----");
						return 0;
					}
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
							wstring str;
							for(int i =0;;i++)
							{
								str.push_back(caps[i]);
								if(caps[i] == '\0')
									if(caps[i+1] == '\0')
										break;
							}
							mir_free(caps);
							if(str.find(_T("GPG_Key_Auto_Exchange:0")) != string::npos)
							{
								CallContactService(ccs->hContact, PSS_MESSAGE, 0, (LPARAM)"-----PGP KEY REQUEST-----");
								return 0;
							}
						}
					}
				}
				mir_free(jid);
			}
		}
		if(!strstr(msg, "-----BEGIN PGP MESSAGE-----"))
			return CallService(MS_PROTO_CHAINRECV, w, l);
		boost::thread *thr = new boost::thread(boost::bind(RecvMsgSvc_func, ccs->hContact, str, msg, (DWORD)ccs->wParam, pre->timestamp));
		return 0;
}

void SendMsgSvc_func(MCONTACT hContact, char *msg, DWORD flags)
{
	bool isansi = false;
	DWORD dbflags = DBEF_UTF;
	wstring str = toUTF16(msg);
	if(bStripTags && bAppendTags)
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: stripping tags in outgoing message, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
		strip_tags(str);
	}
/*	for(std::wstring::size_type i = str.find(_T("\r\n")); i != std::wstring::npos; i = str.find(_T("\r\n"), i+1))
		str.replace(i, 2, _T("\n")); */
	string out;
	DWORD code;
	wstring file = toUTF16(get_random(10)), path;
	std::vector<std::wstring> cmd;
	extern bool bJabberAPI, bIsMiranda09;
	char *tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
	if(!tmp[0])
	{
		mir_free(tmp);
		HistoryLog(hContact, db_event("Failed to encrypt message with GPG (not found key for encryption in db)", 0,0, DBEF_SENT));
		CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
		return;
	}
	if(!bJabberAPI || !bIsMiranda09) //force jabber to handle encrypted message by itself
	{
		cmd.push_back(L"--comment");
		cmd.push_back(L"\"\"");
		cmd.push_back(L"--no-version");
	}
	if(db_get_b(hContact, szGPGModuleName, "bAlwaysTrust", 0))
	{
		cmd.push_back(L"--trust-model");
		cmd.push_back(L"always");
	}
	cmd.push_back(L"--batch");
	cmd.push_back(L"--yes");
	cmd.push_back(L"-eatr");
	TCHAR *tmp2 = mir_a2t(tmp);
	mir_free(tmp);
	cmd.push_back(tmp2);
	mir_free(tmp2);
	tmp2 = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
	path = tmp2;
	cmd.push_back(std::wstring(tmp2) + L"\\tmp\\" + file);
	mir_free(tmp2);
	path += L"\\tmp\\";
	path += file;
	{
		fstream f(path.c_str(), std::ios::out);
		while(!f.is_open())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(50));
			f.open(path.c_str(), std::ios::out);
		}
		std::string tmp = toUTF8(str);
		f.write(tmp.c_str(), tmp.size());
		f.close();
	}
	gpg_execution_params params(cmd);
	pxResult result;
	params.out = &out;
	params.code = &code;
	params.result = &result;
	if(!gpg_launcher(params))
	{
		//mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
		return;
	}
	if(result == pxNotFound)
	{
		//mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
		return;
	}
	if(out.find("There is no assurance this key belongs to the named user") != string::npos)
	{
		out.clear();
		if(MessageBox(0, TranslateT("We're trying to encrypt with untrusted key. Do you want to trust this key permanently?"), TranslateT("Warning"), MB_YESNO) == IDYES)
		{
			db_set_b(hContact, szGPGModuleName, "bAlwaysTrust", 1);
			std::vector<std::wstring> tmp;
			tmp.push_back(L"--trust-model");
			tmp.push_back(L"always");
			cmd.insert(cmd.begin(), tmp.begin(), tmp.end());
			gpg_execution_params params(cmd);
			pxResult result;
			params.out = &out;
			params.code = &code;
			params.result = &result;
			if(!gpg_launcher(params))
			{
				//mir_free(msg);
				CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
				return;
			}
			if(result == pxNotFound)
			{
				//mir_free(msg);
				CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
				return;
			}
			//TODO: check gpg output for errors
		}
		else
		{
			//mir_free(msg);
			return;
		}
	}
	if(result == pxSuccessExitCodeInvalid)
	{
		//mir_free(msg);
		HistoryLog(hContact, db_event(Translate("failed to encrypt message, GPG returned error, turn on debug log for more details"), 0,0, DBEF_SENT));
		boost::filesystem::remove(path);
		return;
	}
	if(out.find("usage: ") != string::npos)
	{
		MessageBox(0, TranslateT("Something is wrong, GPG does not understand us, aborting encryption."), TranslateT("Warning"), MB_OK);
		//mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
		boost::filesystem::remove(path);
		return;
	}
	boost::filesystem::remove(path);
	path.append(_T(".asc"));
	wfstream f(path.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	while(!f.is_open())
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
		f.open(path.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	}
	str.clear();
	if(f.is_open())
	{
		std::wifstream::pos_type size = f.tellg();
		TCHAR *tmp = new TCHAR [(std::ifstream::pos_type)size+(std::ifstream::pos_type)1];
		f.seekg(0, std::ios::beg);
		f.read(tmp, size);
		tmp[size]= '\0';
		str.append(tmp);
		delete [] tmp;
		f.close();
		boost::filesystem::remove(path);
	}
	if(str.empty())
	{
		HistoryLog(hContact, db_event("Failed to encrypt message with GPG", 0,0, DBEF_SENT));
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: Failed to encrypt message with GPG");
		CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)msg);
		return;
	}
	string str_event = msg;
	if(bAppendTags)
	{
		str_event.insert(0, toUTF8(outopentag));
		str_event.append(toUTF8(outclosetag));
	}

	if(bDebugLog)
		debuglog<<std::string(time_str()+": adding event to contact: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" on send message.");

	fix_line_term(str);
	sent_msgs.push_back((HANDLE)CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)toUTF8(str).c_str()));
}

INT_PTR SendMsgSvc(WPARAM w, LPARAM l)
{
	CCSDATA *ccs = (CCSDATA*)l;
	if (!ccs)
		return CallService(MS_PROTO_CHAINSEND, w, l);
	if(!ccs->lParam)
		return CallService(MS_PROTO_CHAINSEND, w, l);
	char *msg = (char*)ccs->lParam;
	if (!msg)
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: failed to get message data, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
		return CallService(MS_PROTO_CHAINSEND, w, l);
	}
	if(strstr(msg,"-----BEGIN PGP MESSAGE-----"))
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: encrypted messge, let it go, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
		return CallService(MS_PROTO_CHAINSEND, w, l);
	}
	if(bDebugLog)
		debuglog<<std::string(time_str()+": info: contact have key, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
	if(bDebugLog && db_mc_isMeta(ccs->hContact))
		debuglog<<std::string(time_str()+": info: protocol is metacontacts, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
	if(!isContactSecured(ccs->hContact) || db_mc_isMeta(ccs->hContact))
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: contact not secured, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)));
		return CallService(MS_PROTO_CHAINSEND, w, l);
	}
	return returnNoError(ccs->hContact);
}

boost::mutex event_processing_mutex;

int HookSendMsg(WPARAM w, LPARAM l)
{
	if(!l)
		return 0;
	DBEVENTINFO * dbei = (DBEVENTINFO*)l;
	if(dbei->eventType != EVENTTYPE_MESSAGE)
		return 0;
	MCONTACT hContact = (MCONTACT)w;
	if(dbei->flags & DBEF_SENT)
	{
		if(isContactSecured(hContact) && strstr((char*)dbei->pBlob, "-----BEGIN PGP MESSAGE-----")) //our service data, can be double added by metacontacts e.t.c.
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info(send handler): block pgp message event, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			return 1;
		}
		if(bAutoExchange && (strstr((char*)dbei->pBlob, "-----PGP KEY RESPONSE-----") || strstr((char*)dbei->pBlob, "-----PGP KEY REQUEST-----"))) ///do not show service data in history
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info(send handler): block pgp key request/response event, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			return 1;
		}
	}
	if(db_mc_isMeta(hContact))
		return 0;

	if(!isContactHaveKey(hContact))
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": info: contact have not key, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
		if(bAutoExchange && !strstr((char*)dbei->pBlob, "-----PGP KEY REQUEST-----") && !strstr((char*)dbei->pBlob, "-----BEGIN PGP PUBLIC KEY BLOCK-----") && gpg_valid)
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info: checking for autoexchange possibility, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			LPSTR proto = GetContactProto(hContact);
			DWORD uin = db_get_dw(hContact, proto, "UIN", 0);
			if(uin)
			{
				if(bDebugLog)
					debuglog<<std::string(time_str()+": info(autoexchange): protocol looks like icq, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));

				char *proto = GetContactProto(hContact);
				if( ProtoServiceExists(proto, PS_ICQ_CHECKCAPABILITY)) {
					if(bDebugLog)
						debuglog<<std::string(time_str()+": info(autoexchange, icq): checking for autoexchange icq capability, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
					ICQ_CUSTOMCAP cap = {0};
					strncpy(cap.caps, "GPGAutoExchange", sizeof(cap.caps));
					if( ProtoCallService(proto, PS_ICQ_CHECKCAPABILITY, hContact, (LPARAM)&cap)) {
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info(autoexchange, icq): sending key requiest, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
						CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"-----PGP KEY REQUEST-----");
						hcontact_data[hContact].msgs_to_send.push_back((char*)dbei->pBlob);
						boost::thread *thr = new boost::thread(boost::bind(send_encrypted_msgs_thread, (void*)hContact));
						//TODO: wait for message
						return 0;
					}
				}
			}
			else
			{
				TCHAR *jid = UniGetContactSettingUtf(hContact, proto, "jid", _T(""));
				if(jid[0])
				{
					if(bDebugLog)
						debuglog<<std::string(time_str()+": info(autoexchange): protocol looks like jabber, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
					extern list <JabberAccount*> Accounts;
					list<JabberAccount*>::iterator end = Accounts.end();
					for(list<JabberAccount*>::iterator p = Accounts.begin(); p != end; p++)
					{
						TCHAR *caps = (*p)->getJabberInterface()->GetResourceFeatures(jid);
						if(caps)
						{
							wstring str;
							for(int i=0;;i++)
							{
								str.push_back(caps[i]);
								if(caps[i] == '\0')
									if(caps[i+1] == '\0')
										break;
							}
							mir_free(caps);
							if(str.find(_T("GPG_Key_Auto_Exchange:0")) != string::npos)
							{
								if(bDebugLog)
									debuglog<<std::string(time_str()+": info(autoexchange, jabber): autoexchange capability found, sending key request, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
								CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)"-----PGP KEY REQUEST-----");
								hcontact_data[hContact].msgs_to_send.push_back((char*)dbei->pBlob);
								boost::thread *thr = new boost::thread(boost::bind(send_encrypted_msgs_thread, (void*)hContact));
								//mir_free((char*)dbei->pBlob);
								//TODO: wait for message
								return 0;
							}
						}
					}
				}
				mir_free(jid);
			}
		}
		else
		{
			return 0;
		}
	}
	if(isContactSecured(hContact) && (dbei->flags & DBEF_SENT)) //aggressive outgoing events filtering
	{
		SendMsgSvc_func(hContact, (char*)dbei->pBlob, 0);
		//TODO: handle errors somehow ...
		if(bAppendTags)
		{
			string str_event = (char*)dbei->pBlob;
			//mir_free(dbei->pBlob);
			str_event.insert(0, toUTF8(outopentag));
			str_event.append(toUTF8(outclosetag));
			dbei->pBlob = (PBYTE)mir_strdup(str_event.c_str());
			dbei->cbBlob = (DWORD)str_event.length() + 1;
		}

		return 0;
	}
	if(!isContactSecured(hContact))
	{
		if(bDebugLog)
			debuglog<<std::string(time_str()+": event message: \""+(char*)dbei->pBlob+"\" passed event filter, contact "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR))+" is unsecured");
		return 0;
	}
	if(!(dbei->flags & DBEF_SENT) && db_mc_isMeta((MCONTACT)w))
	{
		char tmp[29];
		strncpy(tmp, (char*)dbei->pBlob, 27);
		tmp[28] = '\0';
		if(strstr(tmp, "-----BEGIN PGP MESSAGE-----"))
		{
			if(bDebugLog)
				debuglog<<std::string(time_str()+": info(send handler): block pgp message event, name: "+toUTF8((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR)));
			return 1;
		}
	}
	return 0;
}


static INT_PTR CALLBACK DlgProcKeyPassword(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char *inkeyid = NULL;
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			inkeyid = UniGetContactSettingUtf(new_key_hcnt, szGPGModuleName, "InKeyID", "");
			new_key_hcnt_mutex.unlock();
			TCHAR *tmp = NULL;

			SetWindowPos(hwndDlg, 0, key_password_rect.left, key_password_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
			TranslateDialogDefault(hwndDlg);
			string questionstr = "Please enter password for key with ID: ";
			questionstr += inkeyid;
			mir_free(inkeyid);
			SetDlgItemTextA(hwndDlg, IDC_KEYID, questionstr.c_str());
			EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULT_PASSWORD), 0);
			return TRUE;
		}


	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			{
				TCHAR tmp[64];
				GetDlgItemText(hwndDlg, IDC_KEY_PASSWORD, tmp, SIZEOF(tmp));
				if(tmp[0])
				{
					extern TCHAR *password;
					if(IsDlgButtonChecked(hwndDlg, IDC_SAVE_PASSWORD))
					{
						inkeyid = UniGetContactSettingUtf(new_key_hcnt, szGPGModuleName, "InKeyID", "");
						if(inkeyid && inkeyid[0] && BST_UNCHECKED == IsDlgButtonChecked(hwndDlg, IDC_DEFAULT_PASSWORD))
						{
							string dbsetting = "szKey_";
							dbsetting += inkeyid;
							dbsetting += "_Password";
							db_set_ts(NULL, szGPGModuleName, dbsetting.c_str(), tmp);
						}
						else
							db_set_ts(NULL, szGPGModuleName, "szKeyPassword", tmp);
					}
					if(password)
						mir_free(password);
					password = (TCHAR*)mir_alloc(sizeof(TCHAR)*(mir_tstrlen(tmp)+1));
					_tcscpy(password, tmp);
				}
				mir_free(tmp);
				mir_free(inkeyid);
				DestroyWindow(hwndDlg);
			}
			break;

		case IDCANCEL:
			mir_free(inkeyid);
			_terminate = true;
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		mir_free(inkeyid);
		DestroyWindow(hwndDlg);
		break;
	
	case WM_DESTROY:
		GetWindowRect(hwndDlg, &key_password_rect);
		db_set_dw(NULL, szGPGModuleName, "PasswordWindowX", key_password_rect.left);
		db_set_dw(NULL, szGPGModuleName, "PasswordWindowY", key_password_rect.top);
		break;
	}
	return FALSE;
}

void ShowLoadKeyPasswordWindow()
{
	extern HINSTANCE hInst;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_KEY_PASSWD), NULL, DlgProcKeyPassword);
}
