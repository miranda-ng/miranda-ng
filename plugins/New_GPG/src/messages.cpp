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
HANDLE new_key_hcnt = NULL;
boost::mutex new_key_hcnt_mutex;
bool _terminate = false;
int returnNoError(HANDLE hContact);

std::list<HANDLE> sent_msgs;

void RecvMsgSvc_func(HANDLE hContact, std::wstring str, char *msg, DWORD flags, DWORD timestamp)
{		
	DWORD dbflags = DBEF_UTF;
	{ //check for gpg related data
		wstring::size_type s1 = wstring::npos, s2 = wstring::npos;

		s1 = str.find(_T("-----BEGIN PGP MESSAGE-----"));
		s2 = str.find(_T("-----END PGP MESSAGE-----"));
		if((s2 != wstring::npos) && (s1 != wstring::npos))
		{ //this is generic encrypted data block
			void setSrmmIcon(HANDLE);
			void setClistIcon(HANDLE);
			bool isContactHaveKey(HANDLE hContact);
			if(!isContactSecured(hContact))
			{
				debuglog<<time_str()<<": info: "<<"received message from: "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" with turned off encryption\n";
				if(MessageBox(0, TranslateT("We received encrypted message from contact with encryption turned off.\nDo you want turn on encryption for this contact ?"), TranslateT("Warning"), MB_YESNO) == IDYES)
				{
					if(!isContactHaveKey(hContact))
					{
						void ShowLoadPublicKeyDialog();
						extern map<int, HANDLE> user_data;
						extern int item_num;
						item_num = 0;		 //black magic here
						user_data[1] = hContact;
						ShowLoadPublicKeyDialog();
					}
					else
					{
						DBWriteContactSettingByte(metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 1);
						setSrmmIcon(hContact);
						setClistIcon(hContact);
					}
					if(isContactHaveKey(hContact))
					{
						DBWriteContactSettingByte(metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 1);
						setSrmmIcon(hContact);
						setClistIcon(hContact);
					}
				}
				else if(MessageBox(0, TranslateT("Do you want try to decrypt encrypted message ?"), TranslateT("Warning"), MB_YESNO) == IDNO)
				{
					
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					return;
				}
			}
			{
				wstring::size_type p = 0;
				while((p = str.find(_T("\r"), p)) != wstring::npos)
					str.erase(p, 1);
			}
			s2 += _tcslen(_T("-----END PGP MESSAGE-----"));
			char *tmp = mir_t2a(str.substr(s1,s2-s1).c_str());
			TCHAR *tmp2 = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			wstring path = tmp2;
			wstring encfile = toUTF16(get_random(10));
			wstring decfile = toUTF16(get_random(10));
			path.append(_T("\\tmp\\"));
			path.append(encfile);
			DeleteFile(path.c_str());
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
				wstring cmd;
				cmd += _T("--batch ");
				{
					char *inkeyid = UniGetContactSettingUtf(metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "InKeyID", "");
					TCHAR *pass = NULL;
					if(inkeyid[0])
					{
						string dbsetting = "szKey_";
						dbsetting += inkeyid;
						dbsetting += "_Password";
						pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), _T(""));
						if(pass[0])
							debuglog<<time_str()<<": info: found password in database for key id: "<<inkeyid<<", trying to decrypt message from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" with password\n";
					}
					else
					{
						pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", _T(""));
						if(pass[0])
							debuglog<<time_str()<<": info: found password for all keys in database, trying to decrypt message from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" with password\n";
					}
					if(pass && pass[0])
					{
						cmd += _T("--passphrase \"");
						cmd += pass;
						cmd += _T("\" ");
					}
					else if(password && password[0])
					{
						debuglog<<time_str()<<": info: found password in memory, trying to decrypt message from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" with password\n";
						cmd += _T("--passphrase \"");
						cmd += password;
						cmd += _T("\" ");
					}
					else
						debuglog<<time_str()<<": info: passwords not found in database or memory, trying to decrypt message from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" with out password\n";
					mir_free(pass);
					mir_free(inkeyid);
				}
				{
					wstring path = tmp2;
					path += _T("\\tmp\\");
					path += decfile;
					DeleteFile(path.c_str());
				}
				cmd += _T("--output \"");
				cmd += tmp2;
				cmd += _T("\\tmp\\");
				cmd += decfile;
				cmd += _T("\"");
				cmd += _T(" -d -a \"");
				cmd += path;
				cmd += _T("\"");
				gpg_execution_params params;
				pxResult result;
				params.cmd = &cmd;
				params.useless = "";
				params.out = &out;
				params.code = &code;
				params.result = &result;
				boost::thread *gpg_thread = new boost::thread(boost::bind(&pxEexcute_thread, &params));
				if(!gpg_thread->timed_join(boost::posix_time::seconds(10)))
				{
					delete gpg_thread;
					TerminateProcess(params.hProcess, 1);
					params.hProcess = NULL;
					debuglog<<time_str()<<": GPG execution timed out, aborted\n";
					DeleteFile(path.c_str());
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
					DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
					CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
					HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
					DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
					return;
				}
				if(result == pxNotFound)
				{
					DeleteFile(path.c_str());
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					return;
				}
				//TODO: check gpg output for errors
				_terminate = false;
				while(out.find("public key decryption failed: bad passphrase") != string::npos)
				{
					debuglog<<time_str()<<": info: failed to decrypt messaage from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" password needed, trying to get one\n";
					if(_terminate)
					{
						BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
						break;
					}
					{ //save inkey id
						string::size_type s = out.find(" encrypted with ");
						s = out.find(" ID ", s);
						s += strlen(" ID ");
						string::size_type s2 = out.find(",",s);
						DBWriteContactSettingString(metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "InKeyID", out.substr(s, s2-s).c_str());
					}
					void ShowLoadKeyPasswordWindow();
					new_key_hcnt_mutex.lock();
					new_key_hcnt = hContact;
					ShowLoadKeyPasswordWindow();
					wstring cmd2 = cmd;
					if(password)
					{
						debuglog<<time_str()<<": info: found password in memory, trying to decrypt message from "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<"\n";
						wstring tmp = _T("--passphrase \"");
						tmp += password;
						tmp += _T("\" ");
						cmd2.insert(0, tmp);
					}
					out.clear();
					gpg_execution_params params;
					pxResult result;
					params.cmd = &cmd2;
					params.useless = "";
					params.out = &out;
					params.code = &code;
					params.result = &result;
					gpg_thread = gpg_thread = new boost::thread(boost::bind(&pxEexcute_thread, &params));
					if(!gpg_thread->timed_join(boost::posix_time::seconds(10)))
					{
						delete gpg_thread;
						TerminateProcess(params.hProcess, 1);
						params.hProcess = NULL;
						debuglog<<time_str()<<": GPG execution timed out, aborted\n";
						DeleteFile(path.c_str());
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
						return;
					}
					if(result == pxNotFound)
					{
						DeleteFile(path.c_str());
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						return;
					}
					//TODO: check gpg output for errors
				}
				out.clear();
				gpg_thread = new boost::thread(boost::bind(&pxEexcute_thread, &params));
				if(!gpg_thread->timed_join(boost::posix_time::seconds(10)))
				{
					delete gpg_thread;
					TerminateProcess(params.hProcess, 1);
					params.hProcess = NULL;
					debuglog<<time_str()<<": GPG execution timed out, aborted\n";
					DeleteFile(path.c_str());
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
					BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
					DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
					CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
					HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
					DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
					return;
				}
				if(result == pxNotFound)
				{
					DeleteFile(path.c_str());
					HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
				}
				//TODO: check gpg output for errors
				{
					wstring tmp = tmp2;
					tmp += _T("\\tmp\\");
					tmp += encfile;
					DeleteFile(tmp.c_str());
				}
				{
					wstring tmp = tmp2;
					tmp += _T("\\tmp\\");
					tmp += decfile;
					if(_waccess(tmp.c_str(), 0) == -1)
					{
						if(errno == ENOENT)
						{
							string str = msg;
							str.insert(0, "Received unencrypted message:\n");
							debuglog<<time_str()<<": info: Failed to decrypt GPG encrypted message.\n";
							char *tmp = new char [str.length()+1];
							strcpy(tmp, str.c_str());
							HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
							BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
							DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
							CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
							HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
							DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
							mir_free(tmp);
							return;
						}
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
						DeleteFile(path.c_str());
					}
					if(str.empty())
					{
						string str = msg;
						str.insert(0, "Failed to decrypt GPG encrypted message.\nMessage body for manual decryption:\n");
						debuglog<<time_str()<<": info: Failed to decrypt GPG encrypted message.\n";
						char *tmp = mir_strdup(str.c_str());
						HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
						BYTE enc = DBGetContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", 0);
						CallContactService(hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"Unable to decrypt PGP encrypted message");
						HistoryLog(hContact, db_event("Error message sent", 0, 0, DBEF_SENT));
						DBWriteContactSettingByte(hContact, szGPGModuleName, "GPGEncryption", enc);
						mir_free(tmp);
						return;
					}
					else
					{
						if(bAppendTags)
						{
							str.insert(0, inopentag);
							str.append(inclosetag);
						}
						if(metaIsSubcontact(hContact))
						{
							char *msg = mir_strdup(toUTF8(str).c_str());
							HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags|DBEF_READ));
							HistoryLog(metaGetContact(hContact), db_event(msg, timestamp, 0, dbflags));
							mir_free(msg);
							return;
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
	if(DBGetContactSettingByte(metaIsProtoMetaContacts(hContact)?metaGetMostOnline(hContact):hContact, szGPGModuleName, "GPGEncryption", 0))
	{
		if(metaIsSubcontact(hContact))
		{
			HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags|DBEF_READ));
			HistoryLog(metaGetContact(hContact), db_event(msg, timestamp, 0, dbflags));
			return;
		}
		HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags|DBEF_READ));
		return;
	}
	HistoryLog(hContact, db_event(msg, timestamp, 0, dbflags));
	return;
}

int RecvMsgSvc(WPARAM w, LPARAM l)
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
	if(metaIsProtoMetaContacts(ccs->hContact))
	{
		if(!strstr(msg, "-----BEGIN PGP MESSAGE-----"))
			return CallService(MS_PROTO_CHAINRECV, w, l);
		else
			return 0;
	}
	wstring str = toUTF16(msg);
	wstring::size_type s1 = wstring::npos, s2 = wstring::npos;
	if(bAutoExchange && (str.find(_T("-----PGP KEY RESPONSE-----")) != wstring::npos))
	{
		s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"));
		s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
		if(s1 != wstring::npos && s2 != wstring::npos)
		{
			s2 += _tcslen(_T("-----END PGP PUBLIC KEY BLOCK-----"));
			DBWriteContactSettingTString(ccs->hContact, szGPGModuleName, "GPGPubKey", str.substr(s1,s2-s1).c_str());
			{ //gpg execute block
				wstring cmd;
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
					DeleteFile(tmp2);
					wfstream f(tmp2, std::ios::out);
					while(!f.is_open())
						f.open(tmp2, std::ios::out);
					ptmp = UniGetContactSettingUtf(ccs->hContact, szGPGModuleName, "GPGPubKey", _T(""));
					wstring new_key = ptmp;
					mir_free(ptmp);
					f<<new_key.c_str();
					f.close();
					cmd += _T(" --batch ");
					cmd += _T(" --import \"");
					cmd += tmp2;
					cmd += _T("\"");
				}
				gpg_execution_params params;
				pxResult result;
				params.cmd = &cmd;
				params.useless = "";
				params.out = &output;
				params.code = &exitcode;
				params.result = &result;
				boost::thread gpg_thread(boost::bind(&pxEexcute_thread, &params));
				if(!gpg_thread.timed_join(boost::posix_time::seconds(10)))
				{
					gpg_thread.~thread();
					TerminateProcess(params.hProcess, 1);
					params.hProcess = NULL;
					debuglog<<time_str()<<": GPG execution timed out, aborted\n";
					return 1;
				}
				if(result == pxNotFound)
					return 1;
				DeleteFile(tmp2);
				//TODO: check gpg output for errors
				{
					char *tmp = NULL;
					string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
					string::size_type s2 = output.find(":", s);
					DBWriteContactSettingString(ccs->hContact, szGPGModuleName, "KeyID", output.substr(s,s2-s).c_str());
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
					DBWriteContactSettingString(ccs->hContact, szGPGModuleName, "KeyMainName", tmp);
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
						DBWriteContactSettingString(ccs->hContact, szGPGModuleName, "KeyComment", tmp);
						mir_free(tmp);
						s+=3;
						s2 = output.find(">", s);
						tmp = (char*)mir_alloc(output.substr(s,s2-s).length()+1);
						strcpy(tmp, output.substr(s,s2-s).c_str());
						mir_utf8decode(tmp, 0);
						DBWriteContactSettingString(ccs->hContact, szGPGModuleName, "KeyMainEmail", tmp);
						mir_free(tmp);
					}
					else
					{
						tmp = (char*)mir_alloc(output.substr(s2,s-s2).length()+1);
						strcpy(tmp, output.substr(s2,s-s2).c_str());
						mir_utf8decode(tmp, 0);
						DBWriteContactSettingString(ccs->hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
						mir_free(tmp);
					}
					DBWriteContactSettingByte(ccs->hContact, szGPGModuleName, "GPGEncryption", 1);
					DBWriteContactSettingByte(ccs->hContact, szGPGModuleName, "bAlwatsTrust", 1);
					void setSrmmIcon(HANDLE);
					void setClistIcon(HANDLE);
					setSrmmIcon(ccs->hContact);
					setClistIcon(ccs->hContact);
					if(metaIsSubcontact(ccs->hContact))
					{
						setSrmmIcon(metaGetContact(ccs->hContact));
						setClistIcon(metaGetContact(ccs->hContact));
						HistoryLog(metaGetContact(ccs->hContact), "PGP Encryption turned on by key autoexchange feature");
					}
					HistoryLog(ccs->hContact, "PGP Encryption turned on by key autoexchange feature");
				}
				}
				return 1;
			}
		}
		if((str.find(_T("-----END PGP PUBLIC KEY BLOCK-----")) != wstring::npos) && (str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----")) != wstring::npos))
		{
			s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"));
			s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
		}
		else if((str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----")) != wstring::npos) && (str.find(_T("-----END PGP PRIVATE KEY BLOCK-----")) != wstring::npos))
		{
			s2 = str.find(_T("-----END PGP PRIVATE KEY BLOCK-----"));
			s1 = str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
		}
		if((s2 != wstring::npos) && (s1 != wstring::npos))
		{  //this is public key
			debuglog<<time_str()<<": info: "<<"received key from: "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ccs->hContact, GCDNF_TCHAR)<<"\n";
			s1 = 0;
			while((s1 = str.find(_T("\r"), s1)) != wstring::npos)
				str.erase(s1, 1);
			void ShowNewKeyDialog();
			if((str.find(_T("-----END PGP PUBLIC KEY BLOCK-----")) != wstring::npos) && (str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----")) != wstring::npos))
			{
				s2 = str.find(_T("-----END PGP PUBLIC KEY BLOCK-----"));
				s1 = str.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
				s2 += _tcslen(_T("-----END PGP PUBLIC KEY BLOCK-----"));
			}
			else if((str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----")) != wstring::npos) && (str.find(_T("-----END PGP PRIVATE KEY BLOCK-----")) != wstring::npos))
			{
				s2 = str.find(_T("-----END PGP PRIVATE KEY BLOCK-----"));
				s1 = str.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
				s2 += _tcslen(_T("-----END PGP PRIVATE KEY BLOCK-----"));
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
			char *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
			if(tmp[0])
			{
				int enc_state = DBGetContactSettingByte(ccs->hContact, szGPGModuleName, "GPGEncryption", 0);
				if(enc_state)
					DBWriteContactSettingByte(ccs->hContact, szGPGModuleName, "GPGEncryption", 0);
				string str = "-----PGP KEY RESPONSE-----";
				str.append(tmp);
				CallContactService(ccs->hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)str.c_str());
				if(enc_state)
					DBWriteContactSettingByte(ccs->hContact, szGPGModuleName, "GPGEncryption", 1);
			}
			mir_free(tmp);
			return 0;
		}
		else if(!isContactHaveKey(ccs->hContact) && bAutoExchange && gpg_valid && gpg_keyexist)
		{
			char *proto = GetContactProto(ccs->hContact);
			DWORD uin = DBGetContactSettingDword(ccs->hContact, proto, "UIN", 0);
			if(uin) {
				char svc[64];
				strcpy(svc, proto);
				strcat(svc, PS_ICQ_CHECKCAPABILITY);
				if(ServiceExists(svc))
				{
					ICQ_CUSTOMCAP cap = {0};
					strcpy(cap.caps, "GPG AutoExchange");
					if(CallService(svc, (WPARAM)ccs->hContact, (LPARAM)&cap))
					{
						CallContactService(ccs->hContact, PSS_MESSAGE, (WPARAM)PREF_UTF, (LPARAM)"-----PGP KEY REQUEST-----");
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
						TCHAR *caps = (*p)->getJabberInterface()->Net()->GetResourceFeatures(jid);
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
			}
		}
		if(!strstr(msg, "-----BEGIN PGP MESSAGE-----"))
			return CallService(MS_PROTO_CHAINRECV, w, l);
		boost::thread *thr = new boost::thread(boost::bind(RecvMsgSvc_func, ccs->hContact, str, msg, ccs->wParam, pre->timestamp));
		return 0;
}

void SendMsgSvc_func(HANDLE hContact, char *msg, DWORD flags)
{
	wstring str;
	bool isansi = false;
	DWORD dbflags = 0;
	if(flags & PREF_UTF)
		dbflags |= DBEF_UTF;
	if(!metaIsSubcontact(hContact))
		str = toUTF16(msg);
	else
	{//workaround ...
		wchar_t *tmp = mir_utf8decodeW(msg);
		if(!tmp)
		{
			tmp = mir_a2t(msg);
			isansi = true;
		}
		str.append(tmp);
		mir_free(tmp);
	}
	if(bStripTags && bAppendTags)
	{
		std::wstring::size_type p;
		for(p = str.find(inopentag); p != std::wstring::npos; p = str.find(inopentag))
			str.erase(p, _tcslen(inopentag));
		for(p = str.find(inclosetag); p != std::wstring::npos; p = str.find(inclosetag))
			str.erase(p, _tcslen(inclosetag));
		for(p = str.find(outopentag); p != std::wstring::npos; p = str.find(outopentag))
			str.erase(p, _tcslen(outopentag));
		for(p = str.find(outclosetag); p != std::wstring::npos; p = str.find(outclosetag))
			str.erase(p, _tcslen(outclosetag));
	}
/*	for(std::wstring::size_type i = str.find(_T("\r\n")); i != std::wstring::npos; i = str.find(_T("\r\n"), i+1))
		str.replace(i, 2, _T("\n")); */
	string out;
	DWORD code;
	wstring cmd;
	wstring file = toUTF16(get_random(10));
	wstring path;
	extern bool bJabberAPI, bIsMiranda09;
	char *tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
	if(!tmp[0])
	{
		mir_free(tmp);
		HistoryLog(hContact, db_event("Failed to encrypt message with GPG (not found key for encryption in db)", 0,0, DBEF_SENT));
		hcontact_data[hContact].msgs_to_pass.push_back("Failed to encrypt message with GPG (not found key for encryption in db)");
		mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
		return;
	}
	if(!bJabberAPI || !bIsMiranda09) //force jabber to handle encrypted message by itself
		cmd += _T("--comment \"\" --no-version ");
	if(DBGetContactSettingByte(hContact, szGPGModuleName, "bAlwaysTrust", 0))
		cmd += _T("--trust-model always ");
	cmd += _T("--batch --yes -e -a -t -r ");
	TCHAR *tmp2 = mir_a2t(tmp);
	mir_free(tmp);
	cmd += tmp2;
	mir_free(tmp2);
	cmd += _T(" \"");
	tmp2 = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
	path.append(tmp2);
	cmd += tmp2;
	mir_free(tmp2);
	cmd += _T("\\tmp\\");
	cmd += file;
	path.append(_T("\\tmp\\"));
	path += file;
	cmd += _T("\"");
	{
		fstream f(path.c_str(), std::ios::out);
		while(!f.is_open())
			f.open(path.c_str(), std::ios::out);
		std::string tmp = toUTF8(str);
		f.write(tmp.c_str(), tmp.size());
		f.close();
	}
	gpg_execution_params params;
	pxResult result;
	params.cmd = &cmd;
	params.useless = "";
	params.out = &out;
	params.code = &code;
	params.result = &result;
	boost::thread gpg_thread(boost::bind(&pxEexcute_thread, &params));
	if(!gpg_thread.timed_join(boost::posix_time::seconds(10)))
	{
		gpg_thread.~thread();
		TerminateProcess(params.hProcess, 1);
		params.hProcess = NULL;
		debuglog<<time_str()<<": GPG execution timed out, aborted\n";
		mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
		return;
	}
	if(result == pxNotFound)
	{
		mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
		return;
	}
	//TODO: check gpg output for errors
	if(out.find("There is no assurance this key belongs to the named user") != string::npos)
	{
		out.clear();
		if(MessageBox(0, TranslateT("We trying to encrypt with untrusted key, do you want to trust this key permanently ?"), TranslateT("Warning"), MB_YESNO) == IDYES)
		{
			DBWriteContactSettingByte(hContact, szGPGModuleName, "bAlwaysTrust", 1);
			cmd.insert(0, _T("--trust-model always "));
			gpg_execution_params params;
			pxResult result;
			params.cmd = &cmd;
			params.useless = "";
			params.out = &out;
			params.code = &code;
			params.result = &result;
			boost::thread gpg_thread(boost::bind(&pxEexcute_thread, &params));
			if(!gpg_thread.timed_join(boost::posix_time::seconds(10)))
			{
				gpg_thread.~thread();
				TerminateProcess(params.hProcess, 1);
				params.hProcess = NULL;
				debuglog<<time_str()<<": GPG execution timed out, aborted\n";
				mir_free(msg);
				CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
				return;
			}
			if(result == pxNotFound)
			{
				mir_free(msg);
				CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
				return;
			}
			//TODO: check gpg output for errors
		}
		else
		{
			mir_free(msg);
			return;
		}
	}
	if(out.find("usage: ") != string::npos)
	{
		MessageBox(0, TranslateT("Something wrong, gpg does not understand us, aborting encryption."), TranslateT("Warning"), MB_OK);
		DeleteFile(path.c_str());
		mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
		return;
	}
	DeleteFile(path.c_str());
	path.append(_T(".asc"));
	wfstream f(path.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
	while(!f.is_open())
		f.open(path.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
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
		DeleteFile(path.c_str());
	}
	if(str.empty())
	{
		HistoryLog(hContact, db_event("Failed to encrypt message with GPG", 0,0, DBEF_SENT));
		hcontact_data[hContact].msgs_to_pass.push_back("Failed to encrypt message with GPG");
		debuglog<<time_str()<<": info: Failed to encrypt message with GPG\n";
		mir_free(msg);
		CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)msg);
		return;
	}
	string str_event = msg;
	if(bAppendTags)
	{
		str_event.insert(0, toUTF8(outopentag));
		str_event.append(toUTF8(outclosetag));
	}
/*	if(metaIsSubcontact(hContact))
	{
		hcontact_data[metaGetContact(hContact)].msgs_to_pass.push_back(str_event);
		debuglog<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<"is subcontact of"<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)metaGetContact(hContact), GCDNF_TCHAR)<<"\n";
		debuglog<<time_str()<<": adding event to metacontact: "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)metaGetContact(hContact), GCDNF_TCHAR)<<" on send message.\n";
		HistoryLog(metaGetContact(hContact), db_event((char*)str_event.c_str(), 0,0, DBEF_SENT|dbflags));
	} */ //unneeded ?
	hcontact_data[hContact].msgs_to_pass.push_back(str_event);
	debuglog<<time_str()<<": adding event to contact: "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" on send message.\n";
	HistoryLog(hContact, db_event((char*)str_event.c_str(), 0,0, dbflags|DBEF_SENT));
	if(!(flags & PREF_UTF))
		flags |= PREF_UTF; 
	sent_msgs.push_back((HANDLE)CallContactService(hContact, PSS_MESSAGE, (WPARAM)flags, (LPARAM)toUTF8(str).c_str()));
	mir_free(msg);
	return;
}

int SendMsgSvc(WPARAM w, LPARAM l)
{
	CCSDATA *ccs = (CCSDATA*)l;
	if (!ccs)
		return CallService(MS_PROTO_CHAINSEND, w, l);
	char *msg = mir_strdup((char*)(ccs->lParam));
	if (!msg)
	{
		mir_free(msg);
		return CallService(MS_PROTO_CHAINSEND, w, l);
	}
	if(strstr(msg,"-----BEGIN PGP MESSAGE-----"))
		return CallService(MS_PROTO_CHAINSEND, w, l);
	if(!isContactHaveKey(ccs->hContact))
	{
		if(bAutoExchange && !strstr(msg, "-----PGP KEY REQUEST-----") && !strstr(msg, "-----BEGIN PGP PUBLIC KEY BLOCK-----") && gpg_valid)
		{
			void send_encrypted_msgs_thread(HANDLE hContact);
			LPSTR proto = GetContactProto(ccs->hContact);
			DWORD uin = DBGetContactSettingDword(ccs->hContact, proto, "UIN", 0);
			if(uin)
			{
				char *proto = GetContactProto(ccs->hContact);
				char svc[64];
				strcpy(svc, proto);
				strcat(svc, PS_ICQ_CHECKCAPABILITY);

				if(ServiceExists(svc))
				{
					ICQ_CUSTOMCAP cap = {0};
					strcpy(cap.caps, "GPG AutoExchange");
					if(CallService(svc, (WPARAM)ccs->hContact, (LPARAM)&cap))
					{
						CallContactService(ccs->hContact, PSS_MESSAGE, (WPARAM)ccs->wParam, (LPARAM)"-----PGP KEY REQUEST-----");
						hcontact_data[ccs->hContact].msgs_to_send.push_back(msg);
						boost::thread *thr = new boost::thread(boost::bind(send_encrypted_msgs_thread, ccs->hContact));
						mir_free(msg);
						return returnNoError(ccs->hContact);
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
						TCHAR *caps = (*p)->getJabberInterface()->Net()->GetResourceFeatures(jid);
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
								CallContactService(ccs->hContact, PSS_MESSAGE, (WPARAM)ccs->wParam, (LPARAM)"-----PGP KEY REQUEST-----");
								hcontact_data[ccs->hContact].msgs_to_send.push_back(msg);
								boost::thread *thr = new boost::thread(boost::bind(send_encrypted_msgs_thread, ccs->hContact));
								mir_free(msg);
								return returnNoError(ccs->hContact);
							}
						}
					}
				}
			}
		}
		else
		{
			mir_free(msg);
			return CallService(MS_PROTO_CHAINSEND, w, l);
		}
	}
	if(!isContactSecured(ccs->hContact) || metaIsProtoMetaContacts(ccs->hContact))
	{
		mir_free(msg);
		return CallService(MS_PROTO_CHAINSEND, w, l);
	}
	boost::thread *thr = new boost::thread(boost::bind(SendMsgSvc_func, ccs->hContact, msg, (DWORD)ccs->wParam));
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
	HANDLE hContact = (HANDLE)w;
	if(dbei->flags & DBEF_SENT)
	{
		if(isContactSecured(hContact) && strstr((char*)dbei->pBlob, "-----BEGIN PGP MESSAGE-----")) //our service data, can be double added by metacontacts e.t.c.
			return 1;
		if(bAutoExchange && (strstr((char*)dbei->pBlob, "-----PGP KEY RESPONSE-----") || strstr((char*)dbei->pBlob, "-----PGP KEY REQUEST-----"))) ///do not show service data in history
			return 1;
	}
	if(isContactSecured(hContact) && (dbei->flags & DBEF_SENT)) //aggressive outgoing events filtering
	{
		if(!hcontact_data[hContact].msgs_to_pass.empty())
		{
			event_processing_mutex.lock();
			std::list<string>::iterator end = hcontact_data[hContact].msgs_to_pass.end();
			for(std::list<string>::iterator i = hcontact_data[hContact].msgs_to_pass.begin(); i != end; ++i)
			{
				if(!strcmp((*i).c_str(), (char*)dbei->pBlob))
				{
					hcontact_data[hContact].msgs_to_pass.erase(i);
					debuglog<<time_str()<<": event message: \""<<(char*)dbei->pBlob<<"\" passed event filter, contact "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<", message is in allowed list\n";
					event_processing_mutex.unlock();
					return 0;
				}
			}
			event_processing_mutex.unlock();
		}
		if(metaIsProtoMetaContacts(hContact) && !isContactSecured(metaGetMostOnline(hContact)))
			return 0;
		return 1;
	}
	if(!isContactSecured(hContact))
	{
		debuglog<<time_str()<<": event message: \""<<(char*)dbei->pBlob<<"\" passed event filter, contact "<<(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_TCHAR)<<" is unsecured\n";
		return 0;
	}
	if(!(dbei->flags & DBEF_SENT) && metaIsProtoMetaContacts((HANDLE)w))
	{
		char tmp[29];
		strncpy(tmp, (char*)dbei->pBlob, 27);
		tmp[28] = '\0';
		if(strstr(tmp, "-----BEGIN PGP MESSAGE-----"))
			return 1;
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
		SetDlgItemTextA(hwndDlg, IDC_KEYID, questionstr.c_str());
		EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULT_PASSWORD), 0);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
      case IDOK:
        {
			TCHAR tmp[64];
			GetDlgItemText(hwndDlg, IDC_KEY_PASSWORD, tmp, 64);
			if(tmp[0])
			{
				extern TCHAR *password;
				if(IsDlgButtonChecked(hwndDlg, IDC_SAVE_PASSWORD))
				{
					if(inkeyid && inkeyid[0] && !IsDlgButtonChecked(hwndDlg, IDC_DEFAULT_PASSWORD))
					{
						string dbsetting = "szKey_";
						dbsetting += inkeyid;
						dbsetting += "_Password";
						DBWriteContactSettingTString(NULL, szGPGModuleName, dbsetting.c_str(), tmp);
					}
					else
						DBWriteContactSettingTString(NULL, szGPGModuleName, "szKeyPassword", tmp);
				}
				if(password)
					delete [] password;
				password = new TCHAR [_tcslen(tmp)+1];
				_tcscpy(password, tmp);
			}
			mir_free(tmp);
			mir_free(inkeyid);
			DestroyWindow(hwndDlg);
          break;
        }
	  case IDCANCEL:
		  mir_free(inkeyid);
		  _terminate = true;
		  DestroyWindow(hwndDlg);
		  break;
	  default:
		break;
      }
      
      break;
    }
    
  case WM_NOTIFY:
    {
/*      switch (((LPNMHDR)lParam)->code)
      {
	  default:
		  EnableWindow(GetDlgItem(hwndDlg, IDC_DEFAULT_PASSWORD), IsDlgButtonChecked(hwndDlg, IDC_SAVE_PASSWORD)?1:0);
		  break;
      }*/
	}
    break;
  case WM_CLOSE:
	  mir_free(inkeyid);
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  {
		  GetWindowRect(hwndDlg, &key_password_rect);
		  DBWriteContactSettingDword(NULL, szGPGModuleName, "PasswordWindowX", key_password_rect.left);
		  DBWriteContactSettingDword(NULL, szGPGModuleName, "PasswordWindowY", key_password_rect.top);
	  }
	  break;

  }
  return FALSE;
}

void ShowLoadKeyPasswordWindow()
{
	extern HINSTANCE hInst;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_KEY_PASSWD), NULL, DlgProcKeyPassword);
}
