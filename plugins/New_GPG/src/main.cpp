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

extern HFONT bold_font;
extern bool bAutoExchange;

void ShowFirstRunDialog();

HWND hwndFirstRun = NULL, hwndSetDirs = NULL, hwndNewKey = NULL, hwndKeyGen = NULL, hwndSelectExistingKey = NULL;

int itemnum = 0;

HWND hwndList_g = NULL;
BOOL CheckStateStoreDB(HWND hwndDlg, int idCtrl, const char* szSetting);

TCHAR key_id_global[17] = {0};

static INT_PTR CALLBACK DlgProcFirstRun(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	HWND hwndList=GetDlgItem(hwndDlg, IDC_KEY_LIST);
	hwndList_g = hwndList;
	LVCOLUMN col = {0};
	LVITEM item = {0};
	NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
	TCHAR fp[16] = {0};
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		SetWindowPos(hwndDlg, 0, firstrun_rect.left, firstrun_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		TranslateDialogDefault(hwndDlg);
		SetWindowText(hwndDlg, TranslateT("Set own key"));
		EnableWindow(GetDlgItem(hwndDlg, IDC_COPY_PUBKEY), 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT_PRIVATE), 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PASSWD), 0);
		col.pszText = _T("Key ID");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 0, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Email");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 1, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Name");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 250;
		ListView_InsertColumn(hwndList, 2, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Creation date");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 3, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Expire date");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 4, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Key length");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 5, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Accounts");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 6, &col);
		ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_FULLROWSELECT);
		int i = 1, iRow = 0;
		{ 
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = 0;
			item.pszText = _T("");
			{//parse gpg output
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
					if(!gpg_launcher(params))
					{
						break;
					}
					if(result == pxNotFound)
						break;
				}
				while(p != string::npos)
				{
					if((p = out.find("sec  ", p)) == string::npos)
						break;
					p += 5;
					if(p < stop)
						break;
					stop = p;
					p2 = out.find("/", p) - 1;
					TCHAR *key_len = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str()), *creation_date = NULL, *expire_date = NULL;
					p2+=2;
					p = out.find(" ", p2);
					std::wstring key_id = toUTF16(out.substr(p2,p-p2));
					p += 1;
					p2 = out.find(" ", p);
					std::string::size_type p3 = out.find("\n", p);
					if((p2 != std::string::npos) && (p3 < p2))
					{
						p2 = p3;
						creation_date = mir_wstrdup(toUTF16(out.substr(p,p2-p-1)).c_str());
					}
					else
					{
						creation_date = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
						p2 = out.find("[", p2);
						p2 = out.find("expires:", p2);
						p2 += strlen("expires:");
						if(p2 != std::string::npos)
						{
							p2++;
							p = p2;
							p2 = out.find("]", p);
							expire_date = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
							//check expiration
							bool expired = false;
							{
								boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
								TCHAR buf[5];
								_tcsncpy_s(buf, expire_date, _TRUNCATE);
								int year = _ttoi(buf);
								if(year < now.date().year())
									expired = true;
								else if(year == now.date().year())
								{
									_tcsncpy_s(buf, (expire_date + 5), _TRUNCATE);
									int month = _ttoi(buf);
									if(month < now.date().month())
										expired = true;
									else if(month == now.date().month()) {
										_tcsncpy_s(buf, (expire_date + 8), _TRUNCATE);
										unsigned day = _ttoi(buf);
										if(day <= now.date().day_number())
											expired = true;
									}
								}
							}
							if(expired)
							{
								mir_free(key_len);
								mir_free(creation_date);
								mir_free(expire_date);
								//mimic normal behaviour
								p = out.find("uid  ", p);
								p2 = out.find_first_not_of(" ", p+5);
								p = out.find("<", p2);
								p++;
								//p2 = out.find(">", p);
								//
								continue; //does not add to key list
							}
						}
					}
					iRow = ListView_InsertItem(hwndList, &item);
					ListView_SetItemText(hwndList, iRow, 3, creation_date);
					mir_free(creation_date);
					if(expire_date)
					{
						ListView_SetItemText(hwndList, iRow, 4, expire_date);
						mir_free(expire_date);
					}
					ListView_SetItemText(hwndList, iRow, 5, key_len);
					mir_free(key_len);
					ListView_SetItemText(hwndList, iRow, 0, (TCHAR*)key_id.c_str());
					p = out.find("uid  ", p);
					p2 = out.find_first_not_of(" ", p+5);
					p = out.find("<", p2);
					TCHAR *tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
					ListView_SetItemText(hwndList, iRow, 2, tmp);
					mir_free(tmp);
					p++;
					p2 = out.find(">", p);
					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					ListView_SetItemText(hwndList, iRow, 1, tmp);
					mir_free(tmp);
					{ //get accounts
						int count = 0;
						PROTOACCOUNT **accounts;
						ProtoEnumAccounts(&count, &accounts);
						std::wstring accs;
						for(int i = 0; i < count; i++)
						{
							std::string setting = toUTF8(accounts[i]->tszAccountName);
							setting += "(";
							setting += accounts[i]->szModuleName;
							setting += ")" ;
							setting += "_KeyID";
							TCHAR *str = UniGetContactSettingUtf(NULL, szGPGModuleName, setting.c_str(), _T(""));
							if(key_id == str)
							{
								if(accs.empty())
									accs += accounts[i]->tszAccountName;
								else
								{
									accs += _T(",");
									accs += accounts[i]->tszAccountName;
								}
							}
							mir_free(str);
						}
						ListView_SetItemText(hwndList, iRow, 6, (TCHAR*)accs.c_str());
					}
					i++;
				}
				ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 4, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 5, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 6, LVSCW_AUTOSIZE);
			}
		}
		{
			SendDlgItemMessageA(hwndDlg, IDC_ACCOUNT, CB_ADDSTRING, 0, (LPARAM)Translate("Default"));
			int count = 0;
			PROTOACCOUNT **accounts;
			ProtoEnumAccounts(&count, &accounts);
			for(int i = 0; i < count; i++)
			{
				if(StriStr(accounts[i]->szModuleName, "metacontacts"))
					continue;
				if(StriStr(accounts[i]->szModuleName, "weather"))
					continue;
				std::string acc = toUTF8(accounts[i]->tszAccountName);
				acc += "(";
				acc += accounts[i]->szModuleName;
				acc += ")";
				//acc += "_KeyID";
				SendDlgItemMessageA(hwndDlg, IDC_ACCOUNT, CB_ADDSTRING, 0, (LPARAM)acc.c_str());
			}
			SendDlgItemMessageA(hwndDlg, IDC_ACCOUNT, CB_SELECTSTRING, 0, (LPARAM)Translate("Default"));
			string keyinfo = Translate("key ID");
			keyinfo += ": ";
			char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
			keyinfo += (strlen(keyid) > 0)?keyid:Translate("not set");
			mir_free(keyid);
			SetDlgItemTextA(hwndDlg, IDC_KEY_ID, keyinfo.c_str());
		}
		return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_GENERATE_KEY:
		  void ShowKeyGenDialog();
		  ShowKeyGenDialog();
		  break;
	  case ID_OK:
		  {
			  ListView_GetItemText(hwndList, itemnum, 0, fp, SIZEOF(fp));
			  TCHAR *name = new TCHAR [64];
			  ListView_GetItemText(hwndList, itemnum, 2, name, 64);
			  {
				  if(_tcschr(name, _T('(')))
				  {
					  wstring str = name;
					  wstring::size_type p = str.find(_T("("))-1;
					  _tcscpy(name, str.substr(0, p).c_str());
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
			  if(!gpg_launcher(params))
			  {
				  break;
			  }
			  if(result == pxNotFound)
				  break;
			  string::size_type s = 0;
			  boost::algorithm::erase_all(out, "\r");
			  {
				  char buf[64];
				  GetDlgItemTextA(hwndDlg, IDC_ACCOUNT, buf, SIZEOF(buf));
				  if(!strcmp(buf, Translate("Default")))
				  {
					  db_set_s(NULL, szGPGModuleName, "GPGPubKey", out.c_str());
					  db_set_ts(NULL, szGPGModuleName, "KeyMainName", name);
					  db_set_ts(NULL, szGPGModuleName, "KeyID", fp);
				  }
				  else
				  {
					  std::string acc_str = buf;
					  acc_str += "_GPGPubKey";
					  db_set_s(NULL, szGPGModuleName, acc_str.c_str(), out.c_str());
					  acc_str = buf;
					  acc_str += "_KeyMainName";
					  db_set_ts(NULL, szGPGModuleName, acc_str.c_str(), name);
					  acc_str = buf;
					  acc_str += "_KeyID";
					  db_set_ts(NULL, szGPGModuleName, acc_str.c_str(), fp);
				  }
				  if(!strcmp(buf, Translate("Default")))
				  {
					  wstring keyinfo = TranslateT("Default private key ID");
					  keyinfo += _T(": ");
					  keyinfo += (fp[0])?fp:_T("not set");
					  extern HWND hwndCurKey_p;
					  SetWindowText(hwndCurKey_p, keyinfo.c_str());
				  }
			  }
			  TCHAR passwd[64];
			  GetDlgItemText(hwndDlg, IDC_KEY_PASSWORD, passwd, SIZEOF(passwd));
			  if(passwd[0])
			  {
				  string dbsetting = "szKey_";
				  char *keyid = mir_t2a(fp);
				  dbsetting += keyid;
				  mir_free(keyid);
				  dbsetting += "_Password";
				  db_set_ts(NULL, szGPGModuleName, dbsetting.c_str(), passwd);
			  }
			  delete [] name;
		  }
		  bAutoExchange = CheckStateStoreDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange") != 0;
		  gpg_valid = isGPGValid();
		  gpg_keyexist = isGPGKeyExist();
		  DestroyWindow(hwndDlg);
		  break;
	  case IDC_OTHER:
		  {
			  void ShowLoadPublicKeyDialog();
			  item_num = 0;		 //black magic here
			  user_data[1] = 0;
			  ShowLoadPublicKeyDialog();
			  ListView_DeleteAllItems(hwndList);
			  { 
				  int i = 1, iRow = 0;
				  item.mask = LVIF_TEXT;
				  item.iItem = i;
				  item.iSubItem = 0;
				  item.pszText = _T("");
				  {//parse gpg output
					  string out;
					  DWORD code;
					  wstring::size_type p = 0, p2 = 0, stop = 0;
					  {
						  std::vector<wstring> cmd;
						  cmd.push_back(L"--batch");
						  cmd.push_back(L"--list-secret-keys");
						  gpg_execution_params params(cmd);
						  pxResult result;
						  params.out = &out;
						  params.code = &code;
						  params.result = &result;
						  if(!gpg_launcher(params))
						  {
							  break;
						  }
						  if(result == pxNotFound)
							  break;
					  }
					  while(p != string::npos)
					  {
						  if((p = out.find("sec  ", p)) == string::npos)
							  break;
						  p += 5;
						  if(p < stop)
							  break;
						  stop = p;
						  p2 = out.find("/", p) - 1;
						  TCHAR *tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
						  item.pszText = tmp;
						  iRow = ListView_InsertItem(hwndList, &item);
						  ListView_SetItemText(hwndList, iRow, 4, tmp);
						  mir_free(tmp);
						  p2+=2;
						  p = out.find(" ", p2);
						  tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
						  ListView_SetItemText(hwndList, iRow, 0, tmp);
						  mir_free(tmp);
						  p = out.find("uid  ", p);
						  p2 = out.find_first_not_of(" ", p+5);
						  p = out.find("<", p2);
						  tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
						  ListView_SetItemText(hwndList, iRow, 2, tmp);
						  mir_free(tmp);
						  p++;
						  p2 = out.find(">", p);
						  tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
						  ListView_SetItemText(hwndList, iRow, 1, tmp);
						  mir_free(tmp);
						  p = out.find("ssb  ", p2) + 6;
						  p = out.find(" ", p) + 1;
						  p2 = out.find("\n", p);
						  tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p-1)).c_str());
						  ListView_SetItemText(hwndList, iRow, 3, tmp);
						  mir_free(tmp);
						  ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);// not sure about this
						  ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
						  ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
						  ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
						  ListView_SetColumnWidth(hwndList, 4, LVSCW_AUTOSIZE);
						  i++;
					  }
				  }
			  }
		  }
		  break;
	  case IDC_DELETE_KEY:
		  ListView_GetItemText(hwndList, itemnum, 0, fp, SIZEOF(fp));
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
			  if(!gpg_launcher(params))
			  {
				  break;
			  }
			  if(result == pxNotFound)
				  break;
			  string::size_type s = out.find("Key fingerprint = ");
			  s += strlen("Key fingerprint = ");
			  string::size_type s2 = out.find("\n", s);
			  TCHAR *fp = NULL;
			  {
				  string tmp = out.substr(s, s2-s-1).c_str();
				  string::size_type p = 0;
				  while((p = tmp.find(" ", p)) != string::npos)
				  {
					  tmp.erase(p, 1);
				  }
				  fp = mir_a2t(tmp.c_str());
			  }
			  cmd.clear();
			  out.clear();
			  cmd.push_back(L"--batch");
			  cmd.push_back(L"--delete-secret-and-public-key");
			  cmd.push_back(L"--fingerprint");
			  cmd.push_back(fp);
			  mir_free(fp);
			  if(!gpg_launcher(params))
			  {
				  break;
			  }
			  if(result == pxNotFound)
				  break;
		  }
		  {
			  char buf[64];
			  GetDlgItemTextA(hwndDlg, IDC_ACCOUNT, buf, SIZEOF(buf));
			  if(!strcmp(buf, Translate("Default")))
			  {
				  db_unset(NULL, szGPGModuleName, "GPGPubKey");
				  db_unset(NULL, szGPGModuleName, "KeyID");
				  db_unset(NULL, szGPGModuleName, "KeyComment");
				  db_unset(NULL, szGPGModuleName, "KeyMainName");
				  db_unset(NULL, szGPGModuleName, "KeyMainEmail");
				  db_unset(NULL, szGPGModuleName, "KeyType");
			  }
			  else
			  {
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
		  }
		  ListView_DeleteItem(hwndList, itemnum);
		  break;
	  case IDC_GENERATE_RANDOM:
		  {
			  wstring path;
			  { //generating key file
				  TCHAR *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				  path = tmp;
				  mir_free(tmp);
				  path.append(_T("\\new_key"));
				  wfstream f(path.c_str(), std::ios::out);
				  if(!f.is_open())
				  {
					  MessageBox(0, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  f<<"Key-Type: RSA";
				  f<<"\n";
				  f<<"Key-Length: 4096";
				  f<<"\n";
				  f<<"Subkey-Type: RSA";
				  f<<"\n";
				  f<<"Name-Real: ";
				  f<<get_random(6).c_str();
				  f<<"\n";
				  f<<"Name-Email: ";
				  f<<get_random(5).c_str();
				  f<<"@";
				  f<<get_random(5).c_str();
				  f<<".";
				  f<<get_random(3).c_str();
				  f<<"\n";
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
				  extern HFONT bold_font;
				  SendDlgItemMessage(hwndDlg, IDC_GENERATING_KEY, WM_SETFONT, (WPARAM)bold_font, TRUE);
				  SetDlgItemText(hwndDlg, IDC_GENERATING_KEY, TranslateT("Generating new random key, please wait"));
				  EnableWindow(GetDlgItem(hwndDlg, IDC_GENERATE_KEY), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_OTHER), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_KEY), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_LIST), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_GENERATE_RANDOM), 0);
				  if(!gpg_launcher(params, boost::posix_time::minutes(10)))
				  {
					  break;
				  }
				  if(result == pxNotFound)
					  break;

				  boost::filesystem::remove(path);
				  string::size_type p1 = 0;
				  if((p1 = out.find("key ")) != string::npos)
					  path = toUTF16(out.substr(p1+4, 8));
				  else
					  path.clear();
			  }
			  if(!path.empty())
			  {
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
				  if(!gpg_launcher(params))
				  {
					  break;
				  }
				  if(result == pxNotFound)
					  break;
				  string::size_type s = 0;
				  while((s = out.find("\r", s)) != string::npos)
				  {
					  out.erase(s, 1);
				  }
				  {
					  char buf[64];
					  GetDlgItemTextA(hwndDlg, IDC_ACCOUNT, buf, SIZEOF(buf));
					  if(!strcmp(buf, Translate("Default")))
					  {
						  db_set_s(NULL, szGPGModuleName, "GPGPubKey", out.c_str());
						  db_set_ts(NULL, szGPGModuleName, "KeyID", fp);
					  }
					  else
					  {
						  std::string acc_str = buf;
						  acc_str += "_GPGPubKey";
						  db_set_s(NULL, szGPGModuleName, acc_str.c_str(), out.c_str());
						  acc_str = buf;
						  acc_str += "_KeyID";
						  db_set_ts(NULL, szGPGModuleName, acc_str.c_str(), fp);
					  }
				  }
				  extern HWND hwndCurKey_p;
				  SetWindowText(hwndCurKey_p, path.c_str());
			  }
		  }
		  DestroyWindow(hwndDlg);
		  break;
		  		case IDC_ACCOUNT:
			{
				char buf[64];
				GetDlgItemTextA(hwndDlg, IDC_ACCOUNT, buf, SIZEOF(buf));
				if(!strcmp(buf, Translate("Default")))
				{
					string keyinfo = Translate("key ID");
					keyinfo += ": ";
					char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
					keyinfo += (strlen(keyid) > 0)?keyid:Translate("not set");
					mir_free(keyid);
					SetDlgItemTextA(hwndDlg, IDC_KEY_ID, keyinfo.c_str());
				}
				else
				{
					string keyinfo = Translate("key ID");
					keyinfo += ": ";
					std::string acc_str= buf;
					acc_str += "_KeyID";
					char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, acc_str.c_str(), "");
					keyinfo += (strlen(keyid) > 0)?keyid:Translate("not set");
					mir_free(keyid);
					SetDlgItemTextA(hwndDlg, IDC_KEY_ID, keyinfo.c_str());
				}
			}
			break;
				case IDC_COPY_PUBKEY:
					{
						if(OpenClipboard(hwndDlg))
						{
							ListView_GetItemText(hwndList, itemnum, 0, fp, SIZEOF(fp));
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
							if(!gpg_launcher(params))
							{
								break;
							}
							if(result == pxNotFound)
								break;
							boost::algorithm::erase_all(out, "\r");
							HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, out.size() +1);
							if(!hMem)
							{
								MessageBox(0, TranslateT("Failed to allocate memory"), TranslateT("Error"), MB_OK);
								break;
							}
							char *szKey = (char*)GlobalLock(hMem);
							if(!szKey)
							{
								TCHAR msg[64];
								mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to lock memory with error %d"), GetLastError());
								MessageBox(0, msg, TranslateT("Error"), MB_OK);
								GlobalFree(hMem);
							}
							memcpy(szKey, out.c_str(), out.size());
							szKey[out.size()] = '\0';
							EmptyClipboard();
							GlobalUnlock(hMem);
							if(!SetClipboardData(CF_OEMTEXT, hMem))
							{
								GlobalFree(hMem);
								TCHAR msg[64];
								mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed write to clipboard with error %d"), GetLastError());
								MessageBox(0, msg, TranslateT("Error"), MB_OK);
							}
							CloseClipboard();
						}
					}
					break;
				case IDC_EXPORT_PRIVATE:
					{
						TCHAR *p = GetFilePath(_T("Choose file to export key"), _T("*"), _T("Any file"), true);
						if(!p || !p[0])
						{
							delete [] p;
							//TODO: handle error
							break;
						}
						char *path = mir_t2a(p);
						delete [] p;
						std::ofstream file;
						file.open(path, std::ios::trunc | std::ios::out);
						mir_free(path);
						if(!file.is_open())
							break; //TODO: handle error
						ListView_GetItemText(hwndList, itemnum, 0, fp, SIZEOF(fp));
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
						if(!gpg_launcher(params))
						{
							break;
						}
						if(result == pxNotFound)
							break;
						boost::algorithm::erase_all(out, "\r");
						file<<out;
						if(file.is_open())
							file.close();
					}
					break;
				case IDC_CHANGE_PASSWD:
					ListView_GetItemText(hwndList, itemnum, 0, key_id_global, SIZEOF(key_id_global));
//					extern void ShowChangePasswdDlg();
//					ShowChangePasswdDlg();
					//temporary code follows
					std::vector<std::wstring> cmd;
					std::string old_pass, new_pass;
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
					//
					break;
	  }
	  break;
	}
    
  case WM_NOTIFY:
    {
		if(hdr && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != (-1))
		{
			if(hdr->hdr.code == NM_CLICK)
			{				
				EnableWindow(GetDlgItem(hwndDlg, ID_OK), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPY_PUBKEY), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT_PRIVATE), 1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHANGE_PASSWD), 1);
				itemnum = hdr->iItem;
			}
		}
/*		switch(LOWORD(wParam))
		{
		default:
			break;
		}; */
/*      switch (((LPNMHDR)lParam)->code)
      {
	  default:
		  break;
      } */
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		{
			GetWindowRect(hwndDlg, &firstrun_rect);
			db_set_dw(NULL, szGPGModuleName, "FirstrunWindowX", firstrun_rect.left);
			db_set_dw(NULL, szGPGModuleName, "FirstrunWindowY", firstrun_rect.top);
		}
		hwndFirstRun = NULL;
		break;

  }

  return FALSE;
}

static INT_PTR CALLBACK DlgProcGpgBinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *tmp = NULL;
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
		TCHAR *path = (TCHAR*)mir_alloc(sizeof(TCHAR) * MAX_PATH);
		bool gpg_exists = false, lang_exists = false;
		{
			char *mir_path = (char*)mir_alloc(sizeof(char) * MAX_PATH);
			PathToAbsolute("\\", mir_path);
			SetCurrentDirectoryA(mir_path);
			tmp = mir_a2t(mir_path);
			mir_free(mir_path);
			mir_realloc(path, (_tcslen(path)+128)*sizeof(TCHAR));
			TCHAR *gpg_path = (TCHAR*)mir_alloc(sizeof(TCHAR) * MAX_PATH), *gpg_lang_path = (TCHAR*)mir_alloc(sizeof(TCHAR) * MAX_PATH);
			_tcscpy(gpg_path, tmp);
			_tcscat(gpg_path, _T("\\GnuPG\\gpg.exe"));
			_tcscpy(gpg_lang_path, tmp);
			_tcscat(gpg_lang_path, _T("\\GnuPG\\gnupg.nls\\en@quot.mo"));
			mir_free(tmp);
			if(boost::filesystem::exists(gpg_path))
			{
				gpg_exists = true;
				_tcscpy(path, _T("GnuPG\\gpg.exe"));
			}
			if(boost::filesystem::exists(gpg_lang_path))
				lang_exists = true;
			if(gpg_exists && !lang_exists)
				MessageBox(0, TranslateT("GPG binary found in Miranda folder, but English locale does not exist.\nIt's highly recommended that you place \\gnupg.nls\\en@quot.mo in GnuPG folder under Miranda root.\nWithout this file you may experience many problems with GPG output on non-English systems\nand plugin may completely not work.\nYou have been warned."), TranslateT("Warning"), MB_OK);
			mir_free(gpg_path);
			mir_free(gpg_lang_path);
		}
		DWORD len = MAX_PATH;
		if(!gpg_exists)
		{
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", (SHGetValue(HKEY_CURRENT_USER, _T("Software\\GNU\\GnuPG"), _T("gpgProgram"), 0, path, &len) == ERROR_SUCCESS)?path:_T(""));
			if(tmp[0])
				if(!boost::filesystem::exists(tmp))
					MessageBox(0, TranslateT("Wrong GPG binary location found in system.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
		}
		else
			tmp = mir_wstrdup(path);
		mir_free(path);

		SetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp);
		bool bad_version = false;
		if(gpg_exists/* && lang_exists*/)
		{
			db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
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
			db_unset(NULL, szGPGModuleName, "szGpgBinPath");
			string::size_type p1 = out.find("(GnuPG) ");
			if(p1 != string::npos)
			{
				p1 += strlen("(GnuPG) ");
				if(out[p1] != '1')
					bad_version = true;
			}
			else
			{
				bad_version = false;
				MessageBox(0, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Error"), MB_OK);
			}
			if(bad_version)
				MessageBox(0, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
		}
		mir_free(tmp);
		{
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			if(!tmp[0])
			{
				mir_free(tmp);
				char *mir_path = (char*)mir_alloc(sizeof(char) * MAX_PATH);
				PathToAbsolute("\\", mir_path);
				strcat(mir_path, "\\gpg");
				if(_access(mir_path, 0) != -1)
				{
					tmp = mir_wstrdup(toUTF16(mir_path).c_str());
					MessageBox(0, TranslateT("\"GPG\" directory found in Miranda root.\nAssuming it's GPG home directory.\nGPG home directory set."), TranslateT("Info"), MB_OK);
				}
				else
				{
					wstring path_ = _wgetenv(_T("APPDATA"));
					path_ += _T("\\GnuPG");
					tmp = mir_wstrdup(path_.c_str());
				}
			}
			SetDlgItemText(hwndDlg, IDC_HOME_DIR, !gpg_exists?tmp:_T("gpg"));
			mir_free(tmp);
		}
		//TODO: additional check for write access
		if(gpg_exists && lang_exists && !bad_version)
			MessageBox(0, TranslateT("Your GPG version is supported. The language file was found.\nGPG plugin should work fine.\nPress OK to continue."), TranslateT("Info"), MB_OK);
		extern bool bIsMiranda09;
		EnableWindow(GetDlgItem(hwndDlg, IDC_AUTO_EXCHANGE), bIsMiranda09);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_SET_BIN_PATH:
		  {
			  GetFilePath(_T("Choose gpg.exe"), "szGpgBinPath", _T("*.exe"), _T("EXE Executables"));
			  tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T("gpg.exe"));
			  SetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp);
			  char mir_path[MAX_PATH];
			  char *atmp = mir_t2a(tmp);
			  mir_free(tmp);
			  PathToAbsolute("\\", mir_path);
			  char* p_path = NULL;
			  if(StriStr(atmp, mir_path))
			  {
				  p_path = atmp + strlen(mir_path);
				  tmp = mir_a2t(p_path);
				  SetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp);
			  }
		  }
		  break;
	  case IDC_SET_HOME_DIR:
		  {
			  GetFolderPath(_T("Set home directory"), "szHomePath");
			  tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			  SetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp);
			  char mir_path[MAX_PATH];
			  char *atmp = mir_t2a(tmp);
			  mir_free(tmp);
			  PathToAbsolute("\\", mir_path);
			  char* p_path = NULL;
			  if(StriStr(atmp, mir_path))
			  {
				  p_path = atmp + strlen(mir_path);
				  tmp = mir_a2t(p_path);
				  SetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp);
			  }
		  }
		  break;
      case ID_OK:
        {
		  TCHAR tmp[512];
		  GetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp, SIZEOF(tmp));
		  if(tmp[0])
		  {
			  char *mir_path = new char [MAX_PATH];
			  PathToAbsolute("\\", mir_path);
			  SetCurrentDirectoryA(mir_path);
			  delete [] mir_path;
			  if(!boost::filesystem::exists(tmp))
			  {
				  MessageBox(0, TranslateT("GPG binary does not exist.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
				  break;
			  }
		  }
		  else
		  {
			  MessageBox(0, TranslateT("Please choose GPG binary location"), TranslateT("Warning"), MB_OK);
			  break;
		  }
		  {
			  bool bad_version = false;
			  db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
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
			  db_unset(NULL, szGPGModuleName, "szGpgBinPath");
			  string::size_type p1 = out.find("(GnuPG) ");
			  if(p1 != string::npos)
			  {
				  p1 += strlen("(GnuPG) ");
				  if(out[p1] != '1')
					  bad_version = true;
			  }
			  else
			  {
				  bad_version = false;
				  MessageBox(0, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
			  }
			  if(bad_version)
				  MessageBox(0, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
		  }
		  db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
		  GetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp, SIZEOF(tmp));
		  while(tmp[_tcslen(tmp)-1] == '\\')
			  tmp[_tcslen(tmp)-1] = '\0';
		  if(!tmp[0])
		  {
			  MessageBox(0, TranslateT("Please set keyring's home directory"), TranslateT("Warning"), MB_OK);
			  break;
		  }
		  db_set_ts(NULL, szGPGModuleName, "szHomePath", tmp);
		  {
			  TCHAR *path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			  DWORD dwFileAttr = GetFileAttributes(path);
			  if (dwFileAttr != INVALID_FILE_ATTRIBUTES)
			  {
				  dwFileAttr &=~ FILE_ATTRIBUTE_READONLY;
				  SetFileAttributes(path, dwFileAttr);
			  }
			  mir_free(path);
		  }
		  gpg_valid = true;
		  db_set_b(NULL, szGPGModuleName, "FirstRun", 0);
		  DestroyWindow(hwndDlg);
		  ShowFirstRunDialog();
        }
		break;
	  case IDC_GENERATE_RANDOM:
        {
			TCHAR tmp[512];
			GetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp, SIZEOF(tmp));
			if(tmp[0])
			{
				char *mir_path = new char [MAX_PATH];
				PathToAbsolute("\\", mir_path);
				SetCurrentDirectoryA(mir_path);
				delete [] mir_path;
				if(!boost::filesystem::exists(tmp))
				{
					MessageBox(0, TranslateT("GPG binary does not exist.\nPlease choose another location"), TranslateT("Warning"), MB_OK);
					break;
				}
			}
			else
			{
				MessageBox(0, TranslateT("Please choose GPG binary location"), TranslateT("Warning"), MB_OK);
				break;
			}
			{
				bool bad_version = false;
				db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
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
				db_unset(NULL, szGPGModuleName, "szGpgBinPath");
				string::size_type p1 = out.find("(GnuPG) ");
				if(p1 != string::npos)
				{
					p1 += strlen("(GnuPG) ");
					if(out[p1] != '1')
						bad_version = true;
				}
				else
				{
					bad_version = false;
					MessageBox(0, TranslateT("This is not GnuPG binary!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
				}
				if(bad_version)
					MessageBox(0, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended that you use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
			}
			db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
			GetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp, SIZEOF(tmp));
			while(tmp[_tcslen(tmp)-1] == '\\')
				tmp[_tcslen(tmp)-1] = '\0';
			if(!tmp[0])
			{
				MessageBox(0, TranslateT("Please set keyring's home directory"), TranslateT("Warning"), MB_OK);
				break;
			}
			db_set_ts(NULL, szGPGModuleName, "szHomePath", tmp);
			{
				TCHAR *path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				DWORD dwFileAttr = GetFileAttributes(path);
				if (dwFileAttr != INVALID_FILE_ATTRIBUTES)
				{
					dwFileAttr &=~ FILE_ATTRIBUTE_READONLY;
					SetFileAttributes(path, dwFileAttr);
				}
				mir_free(path);
			}
		}
		  {
			  wstring path;
			  { //generating key file
				  TCHAR *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				  path = tmp;
				  mir_free(tmp);
				  path.append(_T("\\new_key"));
				  wfstream f(path.c_str(), std::ios::out);
				  if(!f.is_open())
				  {
					  MessageBox(0, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  f<<"Key-Type: RSA";
				  f<<"\n";
				  f<<"Key-Length: 2048";
				  f<<"\n";
				  f<<"Subkey-Type: RSA";
				  f<<"\n";
				  f<<"Name-Real: ";
				  f<<get_random(6).c_str();
				  f<<"\n";
				  f<<"Name-Email: ";
				  f<<get_random(5).c_str();
				  f<<"@";
				  f<<get_random(5).c_str();
				  f<<".";
				  f<<get_random(3).c_str();
				  f<<"\n";
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
				  gpg_valid = true;
				  if(!gpg_launcher(params, boost::posix_time::minutes(10)))
				  {
					  gpg_valid = false;
					  break;
				  }
				  gpg_valid = false;
				  if(result == pxNotFound)
					  break;
				  boost::filesystem::remove(path);
				  string::size_type p1 = 0;
				  if((p1 = out.find("key ")) != string::npos)
					  path = toUTF16(out.substr(p1+4, 8));
				  else
					  path.clear();
			  }
			  if(!path.empty())
			  {
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
				  gpg_valid = true;
				  if(!gpg_launcher(params))
				  {
					  gpg_valid = false;
					  break;
				  }
				  gpg_valid = false;
				  if(result == pxNotFound)
					  break;
				  string::size_type s = 0;
				  while((s = out.find("\r", s)) != string::npos)
				  {
					  out.erase(s, 1);
				  }
				  db_set_s(NULL, szGPGModuleName, "GPGPubKey", out.c_str());
				  db_set_ts(NULL, szGPGModuleName, "KeyID", path.c_str());
				  extern HWND hwndCurKey_p;
				  SetWindowText(hwndCurKey_p, path.c_str());
			  }
		  }
		  bAutoExchange = CheckStateStoreDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange") != 0;
		  gpg_valid = true;
		  db_set_b(NULL, szGPGModuleName, "FirstRun", 0);
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
		  break;
      }*/
	}
    break;
  case WM_CLOSE:
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  hwndSetDirs = NULL;
	  void InitCheck();
	  InitCheck();
	  break;

  }
  return FALSE;
}

static INT_PTR CALLBACK DlgProcNewKeyDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static MCONTACT hContact = INVALID_CONTACT_ID;
	void ImportKey();
	TCHAR *tmp = NULL;
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			hContact = new_key_hcnt;
			//new_key_hcnt_mutex.unlock();
			SetWindowPos(hwndDlg, 0, new_key_rect.left, new_key_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
			TranslateDialogDefault(hwndDlg);
			TCHAR *tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", _T(""));
			SetDlgItemText(hwndDlg, IDC_MESSAGE, tmp[0]?TranslateT("There is existing key for contact, would you like to replace it with new key?"):TranslateT("New public key was received, do you want to import it?"));
			EnableWindow(GetDlgItem(hwndDlg, IDC_IMPORT_AND_USE), db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0)?0:1);
			SetDlgItemText(hwndDlg, ID_IMPORT, tmp[0]?TranslateT("Replace"):TranslateT("Accept"));
			mir_free(tmp);
			tmp = new TCHAR [256];
			mir_sntprintf(tmp, 255*sizeof(TCHAR),TranslateT("Received key from %s"), CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR));
			SetDlgItemText(hwndDlg, IDC_KEY_FROM, tmp);
			delete [] tmp;
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_IMPORT:
			ImportKey();
			DestroyWindow(hwndDlg);
			break;
		case IDC_IMPORT_AND_USE:
			ImportKey();
			db_set_b(hContact, szGPGModuleName, "GPGEncryption", 1);
			void setSrmmIcon(MCONTACT hContact);
			void setClistIcon(MCONTACT hContact);
			setSrmmIcon(hContact);
			setClistIcon(hContact);
			DestroyWindow(hwndDlg);
			break;
		case IDC_IGNORE_KEY:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		{
			GetWindowRect(hwndDlg, &new_key_rect);
			db_set_dw(NULL, szGPGModuleName, "NewKeyWindowX", new_key_rect.left);
			db_set_dw(NULL, szGPGModuleName, "NewKeyWindowY", new_key_rect.top);
		}
		hwndNewKey = NULL;
		break;

	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcKeyGenDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		SetWindowPos(hwndDlg, 0, key_gen_rect.left, key_gen_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		TranslateDialogDefault(hwndDlg);
		SetWindowText(hwndDlg, TranslateT("Key Generation dialog"));
		ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_KEY_TYPE), _T("RSA"), 0);
		ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_KEY_TYPE), _T("DSA"), 1);
		SendDlgItemMessage(hwndDlg, IDC_KEY_TYPE, CB_SETCURSEL, 1, 0);
		SetDlgItemInt(hwndDlg, IDC_KEY_EXPIRE_DATE, 0, 0);
		SetDlgItemInt(hwndDlg, IDC_KEY_LENGTH, 4096, 0);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDCANCEL:
		  DestroyWindow(hwndDlg);
		  break;
	  case IDOK:
		  {
			  wstring path;
			  { //data sanity checks
				  TCHAR *tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*5);
				  GetDlgItemText(hwndDlg, IDC_KEY_TYPE, tmp, 5);
				  if(_tcslen(tmp) < 3)
				  {
					  mir_free(tmp); tmp = NULL;
					  MessageBox(0, TranslateT("You must set encryption algorithm first"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  if(tmp)
					  mir_free(tmp);
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*6);
				  GetDlgItemText(hwndDlg, IDC_KEY_LENGTH, tmp, 5);
				  int length = _ttoi(tmp);
				  mir_free(tmp);
				  if(length < 1024 || length > 4096)
				  {
					  MessageBox(0, TranslateT("Key length must be of length from 1024 to 4096 bits"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*12);
				  GetDlgItemText(hwndDlg, IDC_KEY_EXPIRE_DATE, tmp, 11);
				  if(_tcslen(tmp) != 10 && tmp[0] != '0')
				  {
					  MessageBox(0, TranslateT("Invalid date"), TranslateT("Error"), MB_OK);
					  mir_free(tmp);
					  break;
				  }
				  mir_free(tmp);
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*128);
				  GetDlgItemText(hwndDlg, IDC_KEY_REAL_NAME, tmp, 127);
				  if(_tcslen(tmp) < 5)
				  {
					  MessageBox(0, TranslateT("Name must contain at least 5 characters"), TranslateT("Error"), MB_OK);
					  mir_free(tmp);
					  break;
				  }
				  else if (_tcschr(tmp, _T('(')) || _tcschr(tmp, _T(')')))
				  {
					  MessageBox(0, TranslateT("Name cannot contain '(' or ')'"), TranslateT("Error"), MB_OK);
					  mir_free(tmp);
					  break;
				  }
				  mir_free(tmp);
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*128);
				  GetDlgItemText(hwndDlg, IDC_KEY_EMAIL, tmp, 128);
				  if((_tcslen(tmp)) < 5 || (!_tcschr(tmp, _T('@'))) || (!_tcschr(tmp, _T('.'))))
				  {
					  MessageBox(0, TranslateT("Invalid Email"), TranslateT("Error"), MB_OK);
					  mir_free(tmp);
					  break;
				  }
				  mir_free(tmp);
			  }		  
			  { //generating key file
				  TCHAR *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
				  char  *tmp2;// = mir_t2a(tmp);
				  path = tmp;
				  mir_free(tmp);
				  //			  mir_free(tmp2);
				  path.append(_T("\\new_key"));
				  wfstream f(path.c_str(), std::ios::out);
				  if(!f.is_open())
				  {
					  MessageBox(0, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  f<<"Key-Type: ";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*5);
				  GetDlgItemText(hwndDlg, IDC_KEY_TYPE, tmp, 5);
				  tmp2 = mir_t2a(tmp);
				  mir_free(tmp);
				  char *subkeytype = (char*)mir_alloc(6);
				  if(strstr(tmp2, "RSA"))
					  strcpy(subkeytype, "RSA");
				  else if(strstr(tmp2, "DSA")) //this is useless check for now, but it will be required if someone add another key types support
					  strcpy(subkeytype, "ELG-E");
				  f<<tmp2;
				  mir_free(tmp2);
				  f<<"\n";
				  f<<"Key-Length: ";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*5); 
				  GetDlgItemText(hwndDlg, IDC_KEY_LENGTH, tmp, 5);
				  int length = _ttoi(tmp);
				  mir_free(tmp);
				  f<<length;
				  f<<"\n";
				  f<<"Subkey-Length: ";
				  f<<length;
				  f<<"\n";
				  f<<"Subkey-Type: ";
				  f<<subkeytype;
				  mir_free(subkeytype);
				  f<<"\n";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*64); //i hope this is enough for password
				  GetDlgItemText(hwndDlg, IDC_KEY_PASSWD, tmp, 64);
				  if(tmp[0])
				  {
					  f<<"Passphrase: ";
					  tmp2 = mir_strdup(toUTF8(tmp).c_str());
					  f<<tmp2;
					  f<<"\n";
					  mir_free(tmp2);
				  }
				  mir_free(tmp);
				  f<<"Name-Real: ";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*128);
				  GetDlgItemText(hwndDlg, IDC_KEY_REAL_NAME, tmp, 128);
				  tmp2 = mir_strdup(toUTF8(tmp).c_str());
				  f<<tmp2;
				  mir_free(tmp2);
				  mir_free(tmp);
				  f<<"\n";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*512);
				  GetDlgItemText(hwndDlg, IDC_KEY_COMMENT, tmp, 512);
				  if(tmp[0])
				  {
					  tmp2 = mir_strdup(toUTF8(tmp).c_str());
					  f<<"Name-Comment: ";
					  f<<tmp2;
					  f<<"\n";
				  }
				  mir_free(tmp2);
				  mir_free(tmp);
				  f<<"Name-Email: ";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*128);
				  GetDlgItemText(hwndDlg, IDC_KEY_EMAIL, tmp, 128);
				  tmp2 = mir_strdup(toUTF8(tmp).c_str());
				  f<<tmp2;
				  mir_free(tmp2);
				  mir_free(tmp);
				  f<<"\n";
				  f<<"Expire-Date: ";
				  tmp = (TCHAR*)mir_alloc(sizeof(TCHAR)*12);
				  GetDlgItemText(hwndDlg, IDC_KEY_EXPIRE_DATE, tmp, 12);
				  tmp2 = mir_strdup(toUTF8(tmp).c_str());
				  f<<tmp2;
				  mir_free(tmp2);
				  mir_free(tmp);
				  f<<"\n";
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
				  SendDlgItemMessage(hwndDlg, IDC_GENERATING_TEXT, WM_SETFONT, (WPARAM)bold_font, TRUE);
				  SetDlgItemText(hwndDlg, IDC_GENERATING_TEXT, TranslateT("Generating new key, please wait..."));
				  EnableWindow(GetDlgItem(hwndDlg, IDCANCEL), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDOK), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_TYPE), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_LENGTH), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_PASSWD), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_REAL_NAME), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_EMAIL), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_COMMENT), 0);
				  EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_EXPIRE_DATE), 0);
				  if(!gpg_launcher(params, boost::posix_time::minutes(10)))
					  break;
				  if(result == pxNotFound)
					  break;
			  }
			  boost::filesystem::remove(path);
			  DestroyWindow(hwndDlg);
			{//parse gpg output
				LVITEM item = {0};
				int i = 1, iRow = 0;
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = 0;
				item.pszText = _T("");				
				string out;
				DWORD code;
				string::size_type p = 0, p2 = 0, stop = 0;
				{
					std::vector<wstring> cmd;
					cmd.push_back(L"--list-secret-keys");
					gpg_execution_params params(cmd);
					pxResult result;
					params.out = &out;
					params.code = &code;
					params.result = &result;
					if(!gpg_launcher(params))
						break;
					if(result == pxNotFound)
						break;
				}
				ListView_DeleteAllItems(hwndList_g);
				while(p != string::npos)
				{
					if((p = out.find("sec  ", p)) == string::npos)
						break;
					p += 5;
					if(p < stop)
						break;
					stop = p;
					p2 = out.find("/", p) - 1;
					TCHAR *tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					item.pszText = tmp;
					iRow = ListView_InsertItem(hwndList_g, &item);
					ListView_SetItemText(hwndList_g, iRow, 4, tmp);
					mir_free(tmp);
					p2+=2;
					p = out.find(" ", p2);
					tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
					ListView_SetItemText(hwndList_g, iRow, 0, tmp);
					mir_free(tmp);
					p = out.find("uid  ", p);
					p2 = out.find_first_not_of(" ", p+5);
					p = out.find("<", p2);
					tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
					ListView_SetItemText(hwndList_g, iRow, 2, tmp);
					mir_free(tmp);
					p++;
					p2 = out.find(">", p);
					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					ListView_SetItemText(hwndList_g, iRow, 1, tmp);
					mir_free(tmp);
					p = out.find("ssb  ", p2) + 6;
					p = out.find(" ", p) + 1;
					p2 = out.find("\n", p);
					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p-1)).c_str());
					ListView_SetItemText(hwndList_g, iRow, 3, tmp);
					mir_free(tmp);
					ListView_SetColumnWidth(hwndList_g, 0, LVSCW_AUTOSIZE);// not sure about this
					ListView_SetColumnWidth(hwndList_g, 1, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_g, 2, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_g, 3, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList_g, 4, LVSCW_AUTOSIZE);
					i++;
				}
			}
		  }
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
		  break;
      } */
	}
    break;
  case WM_CLOSE:
	  DestroyWindow(hwndDlg);
	  break;
  case WM_DESTROY:
	  {
		  GetWindowRect(hwndDlg, &key_gen_rect);
		  db_set_dw(NULL, szGPGModuleName, "KeyGenWindowX", key_gen_rect.left);
		  db_set_dw(NULL, szGPGModuleName, "KeyGenWindowY", key_gen_rect.top);
	  }
	  hwndKeyGen = NULL;
	  break;

  }
  return FALSE;
}

int itemnum2 = 0;

static INT_PTR CALLBACK DlgProcLoadExistingKey(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	HWND hwndList=GetDlgItem(hwndDlg, IDC_EXISTING_KEY_LIST);
	hwndList_g = hwndList;
	LVCOLUMN col = {0};
	LVITEM item = {0};
	NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
	TCHAR id[16] = {0};
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		SetWindowPos(hwndDlg, 0, load_existing_key_rect.left, load_existing_key_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
		TranslateDialogDefault(hwndDlg);
		col.pszText = _T("Key ID");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 0, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = _T("Email");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 1, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = _T("Name");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 250;
		ListView_InsertColumn(hwndList, 2, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = _T("Creation date");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 3, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = _T("Expiration date");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 4, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = _T("Key length");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 30;
		ListView_InsertColumn(hwndList, 5, &col);
		ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_FULLROWSELECT);
		int i = 1, iRow = 0;
		{ 
			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = 0;
			item.pszText = _T("");
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
					if(!gpg_launcher(params))
						break;
					if(result == pxNotFound)
						break;
				}
				while(p != string::npos)
				{
					if((p = out.find("pub  ", p)) == string::npos)
						break;
					p += 5;
					if(p < stop)
						break;
					stop = p;
					p2 = out.find("/", p) - 1;
					TCHAR *tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					item.pszText = tmp;
					iRow = ListView_InsertItem(hwndList, &item);
					ListView_SetItemText(hwndList, iRow, 5, tmp);
					mir_free(tmp);
					p2+=2;
					p = out.find(" ", p2);
					tmp = mir_wstrdup(toUTF16(out.substr(p2,p-p2)).c_str());
					ListView_SetItemText(hwndList, iRow, 0, tmp);
					mir_free(tmp);
					p++;
					p2 = out.find("\n", p);
					string::size_type p3 = out.substr(p, p2-p).find("[");
					if(p3 != string::npos)
					{
						p3+=p;
						p2 = p3;
						p2--;
						p3++;
						p3+=strlen("expires: ");
						string::size_type p4 = out.find("]", p3);
						tmp = mir_wstrdup(toUTF16(out.substr(p3,p4-p3)).c_str());
						ListView_SetItemText(hwndList, iRow, 4, tmp);
						mir_free(tmp);
					}
					else
						p2--;
					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					ListView_SetItemText(hwndList, iRow, 3, tmp);
					mir_free(tmp);
					p = out.find("uid  ", p);
					p+= strlen("uid ");
					p2 = out.find("\n", p);
					p3 = out.substr(p, p2-p).find("<");
					if(p3 != string::npos)
					{
						p3+=p;
						p2=p3;
						p2--;
						p3++;
						string::size_type p4 = out.find(">", p3);
						tmp = mir_wstrdup(toUTF16(out.substr(p3,p4-p3)).c_str());
						ListView_SetItemText(hwndList, iRow, 1, tmp);
						mir_free(tmp);
					}
					else
						p2--;
					p = out.find_first_not_of(" ", p);
					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					ListView_SetItemText(hwndList, iRow, 2, tmp);
					mir_free(tmp);
//					p = out.find("sub  ", p2) + 6;
//					p = out.find(" ", p) + 1;
//					p2 = out.find("\n", p);
//					tmp = mir_wstrdup(toUTF16(out.substr(p,p2-p-1)).c_str());
//					ListView_SetItemText(hwndList, iRow, 3, tmp);
//					mir_free(tmp);
					ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);// not sure about this
					ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList, 4, LVSCW_AUTOSIZE);
					ListView_SetColumnWidth(hwndList, 5, LVSCW_AUTOSIZE);
					i++;
				}
			}
		}
		return TRUE;
    } 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDOK:
		  {
			  ListView_GetItemText(hwndList, itemnum2, 0, id, SIZEOF(id));
			  extern HWND hPubKeyEdit;
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
			  if(!gpg_launcher(params))
				  break;
			  if(result == pxNotFound)
				  break;
			  string::size_type s = 0;
			  while((s = out.find("\r", s)) != string::npos)
			  {
				  out.erase(s, 1);
			  }
			  std::string::size_type p1 = 0, p2 = 0;
			  p1 = out.find("-----BEGIN PGP PUBLIC KEY BLOCK-----");
			  if(p1 != std::string::npos)
			  {
				  p2 = out.find("-----END PGP PUBLIC KEY BLOCK-----", p1);
				  if(p2 != std::string::npos)
				  {
					  p2 += strlen("-----END PGP PUBLIC KEY BLOCK-----");
					  out = out.substr(p1, p2-p1);
					  TCHAR *tmp = mir_a2t(out.c_str());
					  SetWindowText(hPubKeyEdit, tmp);
					  mir_free(tmp);
				  }
				  else
					  MessageBox(NULL, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);
			  }
			  else
				  MessageBox(NULL, TranslateT("Failed to export public key."), TranslateT("Error"), MB_OK);
//			  SetDlgItemText(hPubKeyEdit, IDC_PUBLIC_KEY_EDIT, tmp);
		  }
		  DestroyWindow(hwndDlg);
		  break;
	  case IDCANCEL:
		  DestroyWindow(hwndDlg);
		  break;
      }
      break;
    }
    
  case WM_NOTIFY:
    {
		if(hdr && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != (-1))
		{
			if(hdr->hdr.code == NM_CLICK)
			{				
				EnableWindow(GetDlgItem(hwndDlg, IDOK), 1);
				itemnum2 = hdr->iItem;
			}
		}

      switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
          return TRUE;
        }
      }
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	case WM_DESTROY:
		{
			GetWindowRect(hwndDlg, &load_existing_key_rect);
			db_set_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowX", load_existing_key_rect.left);
			db_set_dw(NULL, szGPGModuleName, "LoadExistingKeyWindowY", load_existing_key_rect.top);
		}
		hwndSelectExistingKey = NULL;
		break;

  }

  return FALSE;
}

static INT_PTR CALLBACK DlgProcImportKeyDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = INVALID_CONTACT_ID;

	switch (msg) {
	case WM_INITDIALOG:
		{
			hContact = new_key_hcnt;
			new_key_hcnt_mutex.unlock();
			SetWindowPos(hwndDlg, 0 , import_key_rect.left, import_key_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
			TranslateDialogDefault(hwndDlg);
			ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_KEYSERVER), _T("subkeys.pgp.net"), 0);
			ComboBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_KEYSERVER), _T("keys.gnupg.net"), 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_IMPORT:
			{
				string out;
				DWORD code;
				std::vector<wstring> cmd;
				cmd.push_back(L"--keyserver");
				TCHAR *server= new TCHAR [128];
				GetDlgItemText(hwndDlg, IDC_KEYSERVER, server, 128);
				cmd.push_back(server);
				delete [] server;
				cmd.push_back(L"--recv-keys");
				cmd.push_back(toUTF16(hcontact_data[hContact].key_in_prescense));
				gpg_execution_params params(cmd);
				pxResult result;
				params.out = &out;
				params.code = &code;
				params.result = &result;
				gpg_launcher(params);
				MessageBoxA(0, out.c_str(), "GPG output", MB_OK);
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		GetWindowRect(hwndDlg, &import_key_rect);
		db_set_dw(NULL, szGPGModuleName, "ImportKeyWindowX", import_key_rect.left);
		db_set_dw(NULL, szGPGModuleName, "ImportKeyWindowY", import_key_rect.top);
		break;
	}
	return FALSE;
}

extern HINSTANCE hInst;


void ShowFirstRunDialog()
{
	if (hwndFirstRun == NULL)
	{
		hwndFirstRun = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FIRST_RUN), NULL, DlgProcFirstRun);
	}
	SetForegroundWindow(hwndFirstRun);
}


void ShowSetDirsDialog()
{
	if (hwndSetDirs == NULL)
	{
		hwndSetDirs = CreateDialog(hInst, MAKEINTRESOURCE(IDD_BIN_PATH), NULL, DlgProcGpgBinOpts);
	}
	SetForegroundWindow(hwndSetDirs);
}

void ShowNewKeyDialog()
{
	hwndNewKey = CreateDialog(hInst, MAKEINTRESOURCE(IDD_NEW_KEY), NULL, DlgProcNewKeyDialog);
	SetForegroundWindow(hwndNewKey);
}

void ShowKeyGenDialog()
{
	if (hwndKeyGen == NULL)
	{
		hwndKeyGen = CreateDialog(hInst, MAKEINTRESOURCE(IDD_KEY_GEN), NULL, DlgProcKeyGenDialog);
	}
	SetForegroundWindow(hwndKeyGen);
}

void ShowSelectExistingKeyDialog()
{
	if (hwndSelectExistingKey == NULL)
	{
		hwndSelectExistingKey = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOAD_EXISTING_KEY), NULL, DlgProcLoadExistingKey);
	}
	SetForegroundWindow(hwndSelectExistingKey);
}

void ShowImportKeyDialog()
{
	CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMPORT_KEY), NULL, DlgProcImportKeyDialog);
}




void FirstRun()
{
	DWORD pid = 0;
	if(!db_get_b(NULL, szGPGModuleName, "FirstRun", 1))
		return;
	ShowSetDirsDialog();
}

void InitCheck()
{
	{//parse gpg output
		TCHAR *current_home = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
		db_set_ts(NULL, szGPGModuleName, "szHomePath", _T("")); //we do not need home for gpg binary validation
		gpg_valid = isGPGValid();
		db_set_ts(NULL, szGPGModuleName, "szHomePath", current_home); //return current home dir back
		mir_free(current_home);
		bool home_dir_access = false, temp_access = false;
		TCHAR *home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
		std::wstring test_path = home_dir;
		mir_free(home_dir);
		test_path += _T("/");
		test_path += toUTF16(get_random(13));
		wfstream test_file;
		test_file.open(test_path, std::ios::trunc | std::ios::out);
		if(test_file.is_open() && test_file.good())
		{
			test_file<<_T("access_test\n");
			if(test_file.good())
				home_dir_access = true;
			test_file.close();
			boost::filesystem::remove(test_path);
		}
		home_dir = _tgetenv(_T("TEMP"));
		test_path = home_dir;
		test_path += _T("/");
		test_path += toUTF16(get_random(13));
		test_file.open(test_path, std::ios::trunc | std::ios::out);
		if(test_file.is_open() && test_file.good())
		{
			test_file<<_T("access_test\n");
			if(test_file.good())
				temp_access = true;
			test_file.close();
			boost::filesystem::remove(test_path);
		}
		if(!home_dir_access || !temp_access || !gpg_valid)
		{
			TCHAR buf[4096];
			_tcsncpy(buf, gpg_valid?TranslateT("GPG binary is set and valid (this is good).\n"):TranslateT("GPG binary unset or invalid (plugin will not work).\n"), SIZEOF(buf));
			_tcsncat(buf, home_dir_access?TranslateT("Home dir write access granted (this is good).\n"):TranslateT("Home dir has no write access (plugin most probably will not work).\n"), SIZEOF(buf));
			_tcsncat(buf, temp_access?TranslateT("Temp dir write access granted (this is good).\n"):TranslateT("Temp dir has no write access (plugin should work, but may have some problems, file transfers will not work)."), SIZEOF(buf));
			if(!gpg_valid)
				_tcsncat(buf, TranslateT("\nGPG will be disabled until you solve these problems"), SIZEOF(buf));
			MessageBox(0, buf, TranslateT("GPG plugin problems"), MB_OK);
		}
		if(!gpg_valid)
			return;
		gpg_keyexist = isGPGKeyExist();
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
			if(!gpg_launcher(params))
				return;
			if(result == pxNotFound)
				return;
		}
		home_dir = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
		wstring tmp_dir = home_dir;
		mir_free(home_dir);
		tmp_dir += _T("\\tmp");
		_wmkdir(tmp_dir.c_str());
		int count = 0;
		PROTOACCOUNT **accounts;
		ProtoEnumAccounts(&count, &accounts);
		string question;
		//char *keyid = nullptr, *key = nullptr;
		char *keyid = nullptr;
		for(int i = 0; i < count; i++)
		{
			if(StriStr(accounts[i]->szModuleName, "metacontacts"))
				continue;
			if(StriStr(accounts[i]->szModuleName, "weather"))
				continue;
			std::string acc = toUTF8(accounts[i]->tszAccountName);
			acc += "(";
			acc += accounts[i]->szModuleName;
			acc += ")";
			acc += "_KeyID";
			keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, acc.c_str(), "");
			if(keyid[0])
			{
				question = Translate("Your secret key with ID: ");
				mir_free(keyid);
				keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
				//key = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
				if((p = out.find(keyid)) == string::npos)
				{
					question += keyid;
					question += Translate(" for account ");
					question += toUTF8(accounts[i]->tszAccountName);
					question += Translate(" deleted from GPG secret keyring.\nDo you want to set another key?");
					if(MessageBoxA(0, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
						ShowFirstRunDialog();
				}
				p2 = p;
				p = out.find("[", p);
				p2 = out.find("\n", p2);
				if((p != std::string::npos) && (p < p2))
				{
					p = out.find("expires:", p);
					p += strlen("expires:");
					p++;
					p2 = out.find("]", p);
					TCHAR *expire_date = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
					bool expired = false;
					{
						boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
						TCHAR buf[5];
						_tcsncpy_s(buf, expire_date, _TRUNCATE);
						int year = _ttoi(buf);
						if(year < now.date().year())
							expired = true;
						else if(year == now.date().year())
						{
							_tcsncpy_s(buf, (expire_date + 5), _TRUNCATE);
							int month = _ttoi(buf);
							if(month < now.date().month())
								expired = true;
							else if(month == now.date().month())
							{
								_tcsncpy_s(buf, (expire_date + 8), _TRUNCATE);
								unsigned day = _ttoi(buf);
								if(day <= now.date().day_number())
									expired = true;
							}
						}
					}
					if(expired)
					{
						question += keyid;
						question += Translate(" for account ");
						question += toUTF8(accounts[i]->tszAccountName);
						question += Translate(" expired and will not work.\nDo you want to set another key?");
						if(MessageBoxA(0, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
							ShowFirstRunDialog();
					}
					mir_free(expire_date);
				}
			}
			if(keyid)
			{
				mir_free(keyid);
				keyid = nullptr;
			}
		}
		question = Translate("Your secret key with ID: ");
		keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
		char *key = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
		if(!db_get_b(NULL, szGPGModuleName, "FirstRun", 1) && (!keyid[0] || !key[0]))
		{
			question = Translate("You didn't set a private key.\nWould you like to set it now?");
			if(MessageBoxA(0, question.c_str(), Translate("Own private key warning"), MB_YESNO) == IDYES)
				ShowFirstRunDialog();
		}
		if((p = out.find(keyid)) == string::npos)
		{
			question += keyid;
			question += Translate(" deleted from GPG secret keyring.\nDo you want to set another key?");
			if(MessageBoxA(0, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
				ShowFirstRunDialog();
		}
		p2 = p;
		p = out.find("[", p);
		p2 = out.find("\n", p2);
		if((p != std::string::npos) && (p < p2))
		{
			p = out.find("expires:", p);
			p += strlen("expires:");
			p++;
			p2 = out.find("]", p);
			TCHAR *expire_date = mir_wstrdup(toUTF16(out.substr(p,p2-p)).c_str());
			bool expired = false;
			{
				boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
				TCHAR buf[5];
				_tcsncpy_s(buf, expire_date, _TRUNCATE);
				int year = _ttoi(buf);
				if(year < now.date().year())
					expired = true;
				else if(year == now.date().year())
				{
					_tcsncpy_s(buf, (expire_date + 5), _TRUNCATE);
					int month = _ttoi(buf);
					if(month < now.date().month())
						expired = true;
					else if(month == now.date().month())
					{
						_tcsncpy_s(buf, (expire_date + 8), _TRUNCATE);
						unsigned day = _ttoi(buf);
						if(day <= now.date().day_number())
							expired = true;
					}
				}
			}
			if(expired)
			{
				question += keyid;
				question += Translate(" expired and will not work.\nDo you want to set another key?");
				if(MessageBoxA(0, question.c_str(), Translate("Own secret key warning"), MB_YESNO) == IDYES)
					ShowFirstRunDialog();
			}
			mir_free(expire_date);
		}
		//TODO: check for expired key
		mir_free(keyid);
		//mir_free(key);
	}
	{
		TCHAR *path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
		DWORD dwFileAttr = GetFileAttributes(path);
		if (dwFileAttr != INVALID_FILE_ATTRIBUTES)
		{
			dwFileAttr &=~ FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(path, dwFileAttr);
		}
		mir_free(path);
	}
	if(bAutoExchange)
	{
		int count = 0;
		PROTOACCOUNT **accounts;
		ProtoEnumAccounts(&count, &accounts);
		ICQ_CUSTOMCAP cap;
		cap.cbSize = sizeof(ICQ_CUSTOMCAP);
		cap.hIcon = 0;
		strncpy(cap.name, "GPG Key AutoExchange", MAX_CAPNAME);
		strncpy(cap.caps, "GPGAutoExchange", sizeof(cap.caps));

		for(int i = 0; i < count; i++)
			if( ProtoServiceExists(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY))
				ProtoCallService(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&cap);
	}
	if(bFileTransfers)
	{
		int count = 0;
		PROTOACCOUNT **accounts;
		ProtoEnumAccounts(&count, &accounts);
		ICQ_CUSTOMCAP cap;
		cap.cbSize = sizeof(ICQ_CUSTOMCAP);
		cap.hIcon = 0;
		strncpy(cap.name, "GPG Encrypted FileTransfers", MAX_CAPNAME);
		strncpy(cap.caps, "GPGFileTransfer", sizeof(cap.caps));

		for(int i = 0; i < count; i++)
			if( ProtoServiceExists(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY))
				ProtoCallService(accounts[i]->szProtoName, PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&cap);
	}
}

void ImportKey()
{
	MCONTACT hContact = new_key_hcnt;
	new_key_hcnt_mutex.unlock();
	bool for_all_sub = false;
	if(db_mc_isMeta(hContact)) {
		if(MessageBox(0, TranslateT("Do you want to load key for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES)
			for_all_sub = true;

		if(for_all_sub)
		{
			int count = db_mc_getSubCount(hContact);
			for(int i = 0; i < count; i++)
			{
				MCONTACT hcnt = db_mc_getSub(hContact, i);
				if(hcnt)
					db_set_ts(hcnt, szGPGModuleName, "GPGPubKey", new_key.c_str());
			}
		}
		else db_set_ts(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", new_key.c_str());
	}
	else db_set_ts(hContact, szGPGModuleName, "GPGPubKey", new_key.c_str());

	new_key.clear();
	{ //gpg execute block
		std::vector<wstring> cmd;
		TCHAR tmp2[MAX_PATH] = {0};
		TCHAR *ptmp;
		string output;
		DWORD exitcode;
		{
			ptmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
			_tcsncpy(tmp2, ptmp, MAX_PATH-1);
			mir_free(ptmp);
			_tcsncat(tmp2, _T("\\"), MAX_PATH-1);
			_tcsncat(tmp2, _T("temporary_exported.asc"), MAX_PATH-1);
			boost::filesystem::remove(tmp2);
			wfstream f(tmp2, std::ios::out);
			if(db_mc_isMeta(hContact))
				ptmp = UniGetContactSettingUtf(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", _T(""));
			else
				ptmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "GPGPubKey", _T(""));
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
			return;
		if(result == pxNotFound)
			return;
		{
			if(db_mc_isMeta(hContact))
			{
				if(for_all_sub)
				{
					int count = db_mc_getSubCount(hContact);
					for(int i = 0; i < count; i++)
					{
						MCONTACT hcnt = db_mc_getSub(hContact, i);
						if(hcnt)
						{
							char *tmp = NULL;
							string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
							string::size_type s2 = output.find(":", s);
							db_set_s(hcnt, szGPGModuleName, "KeyID", output.substr(s,s2-s).c_str());
							s = output.find("“", s2);
							if(s == string::npos)
							{
								s = output.find("\"", s2);
								s += 1;
							}
							else
								s += 3;
							bool uncommon = false;
							if((s2 = output.find("(", s)) == string::npos)
							{
								if((s2 = output.find("<", s)) == string::npos)
								{
									s2 = output.find("”", s);
									uncommon = true;
								}
							}
							else if(s2 > output.find("<", s))
								s2 = output.find("<", s);
							if(s != string::npos && s2 != string::npos)
							{
								tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s,s2-s-(uncommon?1:0)).length()+1));
								strcpy(tmp, output.substr(s,s2-s-(uncommon?1:0)).c_str());
								mir_utf8decode(tmp, 0);
								db_set_s(hcnt, szGPGModuleName, "KeyMainName", tmp);
								mir_free(tmp);
							}

							if((s = output.find(")", s2)) == string::npos)
								s = output.find(">", s2);
							else if(s > output.find(">", s2))
								s = output.find(">", s2);
							s2++;
							if(s != string::npos && s2 != string::npos)
							{
								if(output[s] == ')')
								{
									tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
									strcpy(tmp, output.substr(s2,s-s2).c_str());
									mir_utf8decode(tmp, 0);
									db_set_s(hcnt, szGPGModuleName, "KeyComment", tmp);
									mir_free(tmp);
									s+=3;
									s2 = output.find(">", s);
									if(s != string::npos && s2 != string::npos)
									{
										tmp = (char*) mir_alloc(sizeof(char)*(output.substr(s,s2-s).length()+1));
										strcpy(tmp, output.substr(s,s2-s).c_str());
										mir_utf8decode(tmp, 0);
										db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", tmp);
										mir_free(tmp);
									}
								}
								else
								{
									tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
									strcpy(tmp, output.substr(s2,s-s2).c_str());
									mir_utf8decode(tmp, 0);
									db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
									mir_free(tmp);
								}
							}
							db_unset(hcnt, szGPGModuleName, "bAlwatsTrust");
						}
					}
				}
				else
				{
					char *tmp = NULL;
					string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
					string::size_type s2 = output.find(":", s);
					db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyID", output.substr(s,s2-s).c_str());
					s = output.find("“", s2);
					if(s == string::npos)
					{
						s = output.find("\"", s2);
						s += 1;
					}
					else
						s += 3;
					bool uncommon = false;
					if((s2 = output.find("(", s)) == string::npos)
					{
						if((s2 = output.find("<", s)) == string::npos)
						{
							s2 = output.find("”", s);
							uncommon = true;
						}
					}
					else if(s2 > output.find("<", s))
						s2 = output.find("<", s);
					if(s != string::npos && s2 != string::npos)
					{
						tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s,s2-s-(uncommon?1:0)).length()+1));
						strcpy(tmp, output.substr(s,s2-s-(uncommon?1:0)).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainName", tmp);
						mir_free(tmp);
					}
					if((s = output.find(")", s2)) == string::npos)
						s = output.find(">", s2);
					else if(s > output.find(">", s2))
						s = output.find(">", s2);
					s2++;
					if(s != string::npos && s2 != string::npos)
					{
						if(output[s] == ')')
						{
							tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
							strcpy(tmp, output.substr(s2,s-s2).c_str());
							mir_utf8decode(tmp, 0);
							db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyComment", tmp);
							mir_free(tmp);
							s+=3;
							s2 = output.find(">", s);
							if(s != string::npos && s2 != string::npos)
							{
								tmp = (char*) mir_alloc(sizeof(char)*(output.substr(s,s2-s).length()+1));
								strcpy(tmp, output.substr(s,s2-s).c_str());
								mir_utf8decode(tmp, 0);
								db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", tmp);
								mir_free(tmp);
							}
						}
						else
						{
							tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
							strcpy(tmp, output.substr(s2,s-s2).c_str());
							mir_utf8decode(tmp, 0);
							db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
							mir_free(tmp);
						}
					}
					db_unset(metaGetMostOnline(hContact), szGPGModuleName, "bAlwatsTrust");
				}
			}
			else
			{
				char *tmp = NULL;
				string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
				string::size_type s2 = output.find(":", s);
				db_set_s(hContact, szGPGModuleName, "KeyID", output.substr(s,s2-s).c_str());
				s = output.find("“", s2);
				if(s == string::npos)
				{
					s = output.find("\"", s2);
					s += 1;
				}
				else
					s += 3;
				bool uncommon = false;
				if((s2 = output.find("(", s)) == string::npos)
				{
					if((s2 = output.find("<", s)) == string::npos)
					{
						s2 = output.find("”", s);
						uncommon = true;
					}
				}
				else if(s2 > output.find("<", s))
					s2 = output.find("<", s);
				if(s != string::npos && s2 != string::npos)
				{
					tmp = (char*)mir_alloc(sizeof(char)*(output.substr(s,s2-s-(uncommon?1:0)).length()+1));
					strcpy(tmp, output.substr(s,s2-s-(uncommon?1:0)).c_str());
					mir_utf8decode(tmp, 0);
					db_set_s(hContact, szGPGModuleName, "KeyMainName", tmp);
					mir_free(tmp);
				}
				if((s = output.find(")", s2)) == string::npos)
					s = output.find(">", s2);
				else if(s > output.find(">", s2))
					s = output.find(">", s2);
				s2++;
				if(s != string::npos && s2 != string::npos)
				{
					if(output[s] == ')')
					{
						tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
						strcpy(tmp, output.substr(s2,s-s2).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(hContact, szGPGModuleName, "KeyComment", tmp);
						mir_free(tmp);
						s+=3;
						s2 = output.find(">", s);
						if(s != string::npos && s2 != string::npos)
						{
							tmp = (char*) mir_alloc(sizeof(char)*(output.substr(s,s2-s).length()+1));
							strcpy(tmp, output.substr(s,s2-s).c_str());
							mir_utf8decode(tmp, 0);
							db_set_s(hContact, szGPGModuleName, "KeyMainEmail", tmp);
							mir_free(tmp);
						}
					}
					else
					{
						tmp = (char*)mir_alloc(sizeof(char)* (output.substr(s2,s-s2).length()+1));
						strcpy(tmp, output.substr(s2,s-s2).c_str());
						mir_utf8decode(tmp, 0);
						db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
						mir_free(tmp);
					}
				}
				db_unset(hContact, szGPGModuleName, "bAlwatsTrust");
			}
		}
		ptmp = mir_wstrdup(toUTF16(output).c_str());
		MessageBox(0, ptmp, _T(""), MB_OK);
		mir_free(ptmp);
		boost::filesystem::remove(tmp2);
	}
}