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

extern HINSTANCE hInst;

static INT_PTR CALLBACK DlgProcGpgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcGpgBinOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcGpgMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcGpgAdvOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL CheckStateLoadDB(HWND hwndDlg, int idCtrl, const char* szSetting, BYTE bDef)
{
	BOOL state = db_get_b(NULL, szGPGModuleName, szSetting, bDef);
	CheckDlgButton(hwndDlg, idCtrl, state);
	return state;
}

BOOL CheckStateStoreDB(HWND hwndDlg, int idCtrl, const char* szSetting)
{
	BOOL state = IsDlgButtonChecked(hwndDlg, idCtrl);
	db_set_b(NULL, szGPGModuleName, szSetting, (BYTE)state);
	return state;
}

int GpgOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GPG);
	odp.ptszTitle = _T(szGPGModuleName);
	odp.ptszGroup = LPGENT("Services");
	odp.ptszTab = LPGENT("Main");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.pfnDlgProc = DlgProcGpgOpts;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GPG_BIN);
	odp.ptszTab = LPGENT("GnuPG Variables");
	odp.pfnDlgProc = DlgProcGpgBinOpts;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GPG_MESSAGES);
	odp.ptszTab = LPGENT("Messages");
	odp.pfnDlgProc = DlgProcGpgMsgOpts;
	Options_AddPage(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GPG_ADVANCED);
	odp.ptszTab = LPGENT("Advanced");
	odp.pfnDlgProc = DlgProcGpgAdvOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

map<int, MCONTACT> user_data;

int item_num = 0;
HWND hwndList_p = NULL;
HWND hwndCurKey_p = NULL;

void ShowLoadPublicKeyDialog();
static INT_PTR CALLBACK DlgProcGpgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList=GetDlgItem(hwndDlg, IDC_USERLIST);
	hwndList_p = hwndList;
	hwndCurKey_p = GetDlgItem(hwndDlg, IDC_CURRENT_KEY);
	LVCOLUMN col = {0};
	LVITEM item = {0};
	TCHAR *tmp = NULL;
	char *tmp2 = NULL;
	extern bool bIsMiranda09, bJabberAPI;
	NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
		col.pszText = TranslateT("Contact");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 60;
		ListView_InsertColumn(hwndList, 0, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Key ID");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 1, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Name");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 2, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Email");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 3, &col);
		memset(&col, 0, sizeof(col));
		col.pszText = TranslateT("Protocol");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 60;
		ListView_InsertColumn(hwndList, 4, &col);
		ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);
		int i = 1, iRow = 0;
		for(MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact)) {
			if(isContactHaveKey(hContact)) {
				TCHAR *name = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
				item.mask = LVIF_TEXT;
				item.iItem = i;
				item.iSubItem = 0;
				item.pszText = name;
				iRow = ListView_InsertItem(hwndList, &item);
				ListView_SetItemText(hwndList, iRow, 0, name);
				TCHAR *tmp = mir_a2t(GetContactProto(hContact));
				ListView_SetItemText(hwndList, iRow, 4, tmp);
				mir_free(tmp);
				tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
				tmp = mir_a2t(tmp2);
				mir_free(tmp2);
				ListView_SetItemText(hwndList, iRow, 1, (_tcslen(tmp) > 1)?tmp:_T("not set"));
				mir_free(tmp);
				char *tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainName", "");
				if(!toUTF16(tmp2).empty())
					tmp = mir_wstrdup(toUTF16(tmp2).c_str());
				else
					tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainName", _T(""));
				mir_free(tmp2);
				ListView_SetItemText(hwndList, iRow, 2, (_tcslen(tmp) > 1)?tmp:_T("not set"));
				mir_free(tmp);
				tmp2 = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainEmail", "");
				if(!toUTF16(tmp2).empty())
					tmp = mir_wstrdup(toUTF16(tmp2).c_str());
				else
					tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyMainEmail", _T(""));
				mir_free(tmp2);
				ListView_SetItemText(hwndList, iRow, 3, (_tcslen(tmp) > 1)?tmp:_T("not set"));
				mir_free(tmp);
				if(db_get_b(hContact, szGPGModuleName, "GPGEncryption", 0))
					ListView_SetCheckState(hwndList, iRow, 1);
				user_data[i] = hContact;
				memset(&item, 0, sizeof(item));
				ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);// not sure about this
				ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
				ListView_SetColumnWidth(hwndList, 4, LVSCW_AUTOSIZE);
				i++;
			}			
		}
		tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szLogFilePath", _T(""));
		SetDlgItemText(hwndDlg, IDC_LOG_FILE_EDIT, (_tcslen(tmp) > 1)?tmp:_T("c:\\GPGdebug.log"));
		mir_free(tmp);
		CheckStateLoadDB(hwndDlg, IDC_DEBUG_LOG, "bDebugLog", 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_JABBER_API), bIsMiranda09);
		EnableWindow(GetDlgItem(hwndDlg, IDC_AUTO_EXCHANGE), (bIsMiranda09 && bJabberAPI));
		{
			string keyinfo = Translate("Default private key ID");
			keyinfo += ": ";
			char *keyid = UniGetContactSettingUtf(NULL, szGPGModuleName, "KeyID", "");
			keyinfo += (strlen(keyid) > 0)?keyid:Translate("not set");
			mir_free(keyid);
			SetDlgItemTextA(hwndDlg, IDC_CURRENT_KEY, keyinfo.c_str());
		}
		if(bIsMiranda09)
			CheckStateLoadDB(hwndDlg, IDC_JABBER_API, "bJabberAPI", 1);
		CheckStateLoadDB(hwndDlg, IDC_FILE_TRANSFERS, "bFileTransfers", 0);
		CheckStateLoadDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange", 0);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_DELETE_KEY_BUTTON:
		  void setClistIcon(MCONTACT hContact);
		  void setSrmmIcon(MCONTACT hContact);
		  { //gpg execute block
			  TCHAR tmp2[MAX_PATH] = {0};
			  TCHAR *ptmp;
			  char *tmp;
			  bool keep = false;
			  bool ismetacontact = false;
			  MCONTACT meta = NULL;
			  MCONTACT hContact = user_data[item_num+1];
			  if(db_mc_isMeta(hContact))
			  {
				  meta = hContact;
				  hContact = metaGetMostOnline(hContact);
				  ismetacontact = true;
			  }
			  else if((meta = db_mc_getMeta(user_data[item_num+1])) != NULL)
			  {
				  hContact = metaGetMostOnline(meta);
				  ismetacontact = true;
			  }
			  tmp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", "");
			  for(MCONTACT hcnttmp = db_find_first(); hcnttmp != NULL; hcnttmp = db_find_next(hcnttmp)) {
				  if(hcnttmp != hContact) {
					  char *tmp2 = UniGetContactSettingUtf(hcnttmp, szGPGModuleName, "KeyID", "");
					  if(!strcmp(tmp, tmp2)) {
						  mir_free(tmp2);
						  keep = true;
						  break;
					  }
					  mir_free(tmp2);
				  }
			  }
			  if(!keep)
				  if(MessageBox(0, TranslateT("This key is not used by any contact. Do you want to remove it from public keyring?"), TranslateT("Key info"), MB_YESNO) == IDYES)
				  {
					  std::vector<wstring> cmd;
					  string output;
					  DWORD exitcode;
					  cmd.push_back(L"--batch");
					  cmd.push_back(L"--yes");
					  cmd.push_back(L"--delete-key");
					  ptmp = mir_a2t(tmp);
					  cmd.push_back(ptmp);
					  mir_free(ptmp);
					  gpg_execution_params params(cmd);
					  pxResult result;
					  params.out = &output;
					  params.code = &exitcode;
					  params.result = &result;
					  if(!gpg_launcher(params))
					  {
						  mir_free(tmp);
						  break;
					  }
					  if(result == pxNotFound)
					  {
						  mir_free(tmp);
						  break;
					  }
					  if(output.find("--delete-secret-keys") != string::npos)
						  MessageBox(0, TranslateT("we have secret key for this public key, do not removing from GPG keyring"), TranslateT("info"), MB_OK);
					  else
						  MessageBox(0, TranslateT("Key removed from GPG keyring"), TranslateT("info"), MB_OK);
				  }
				  mir_free(tmp);
				  if(ismetacontact)
				  {
					  if(MessageBox(0, TranslateT("Do you want to remove key from entire metacontact (all subcontacts)?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES)
					  {
						  MCONTACT hcnt = NULL;
						  int count = db_mc_getSubCount(meta);
						  for(int i = 0; i < count; i++)
						  {
							  hcnt = db_mc_getSub(meta, i);
							  if(hcnt)
							  {
								  db_unset(hcnt, szGPGModuleName, "KeyID");
								  db_unset(hcnt, szGPGModuleName, "GPGPubKey");
								  db_unset(hcnt, szGPGModuleName, "KeyMainName");
								  db_unset(hcnt, szGPGModuleName, "KeyType");
								  db_unset(hcnt, szGPGModuleName, "KeyMainEmail");
								  db_unset(hcnt, szGPGModuleName, "KeyComment");
								  setClistIcon(hcnt);
								  setSrmmIcon(hcnt);
							  }
						  }
					  }
					  else
					  {
						  db_unset(hContact, szGPGModuleName, "KeyID");
						  db_unset(hContact, szGPGModuleName, "GPGPubKey");
						  db_unset(hContact, szGPGModuleName, "KeyMainName");
						  db_unset(hContact, szGPGModuleName, "KeyType");
						  db_unset(hContact, szGPGModuleName, "KeyMainEmail");
						  db_unset(hContact, szGPGModuleName, "KeyComment");
						  setClistIcon(hContact);
						  setSrmmIcon(hContact);
					  }
				  }
				  else
				  {
					  db_unset(user_data[item_num+1], szGPGModuleName, "KeyID");
					  db_unset(user_data[item_num+1], szGPGModuleName, "GPGPubKey");
					  db_unset(user_data[item_num+1], szGPGModuleName, "KeyMainName");
					  db_unset(user_data[item_num+1], szGPGModuleName, "KeyType");
					  db_unset(user_data[item_num+1], szGPGModuleName, "KeyMainEmail");
					  db_unset(user_data[item_num+1], szGPGModuleName, "KeyComment");
					  setClistIcon(user_data[item_num+1]);
					  setSrmmIcon(user_data[item_num+1]);
				  }
		  }
		  ListView_SetItemText(hwndList, item_num, 3, TranslateT("not set"));
		  ListView_SetItemText(hwndList, item_num, 2, TranslateT("not set"));
		  ListView_SetItemText(hwndList, item_num, 1, TranslateT("not set"));
		  break;
	  case IDC_SELECT_KEY:
		  {
			  void ShowFirstRunDialog();
			  ShowFirstRunDialog();
		  }
		  break;
	  case IDC_SAVE_KEY_BUTTON:
		  {
			  tmp = GetFilePath(TranslateT("Export public key"), _T("*"), TranslateT(".asc pubkey file"), true);
			  if(!tmp)
			  {
				  break;
			  }
			  wstring str;
			  {
				  TCHAR *tmp = UniGetContactSettingUtf(user_data[item_num+1], szGPGModuleName, "GPGPubKey", _T(""));
				  str.append(tmp);
				  mir_free(tmp);
			  }
			  wstring::size_type s = 0;
			  while((s = str.find(_T("\r"), s)) != wstring::npos)
			  {
				  str.erase(s, 1);
			  }
			  wfstream f(tmp, std::ios::out);
			  delete [] tmp;
			  f<<str.c_str();
			  f.close();
		  }
		  break;
	  case IDC_COPY_KEY:
		  {
			  if(OpenClipboard(hwndDlg))
			  {
				  char *szKey = UniGetContactSettingUtf(NULL, szGPGModuleName, "GPGPubKey", "");
				  std::string str = szKey;
				  mir_free(szKey);
				  boost::algorithm::replace_all(str, "\n", "\r\n");
				  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, str.size() +1);
				  if(!hMem)
				  {
					  MessageBox(0, TranslateT("Failed to allocate memory"), TranslateT("Error"), MB_OK);
					  break;
				  }
				  szKey = (char*)GlobalLock(hMem);
				  if(!szKey)
				  {
					  TCHAR msg[64];
					  mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to lock memory with error %d"), GetLastError());
					  MessageBox(0, msg, TranslateT("Error"), MB_OK);
					  GlobalFree(hMem);
				  }
				  memcpy(szKey, str.c_str(), str.size());
				  szKey[str.size()] = '\0';
				  str.clear();
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
			  else
			  {
				  TCHAR msg[64];
				  mir_sntprintf(msg, SIZEOF(msg), TranslateT("Failed to open clipboard with error %d"), GetLastError());
				  MessageBox(0, msg, TranslateT("Error"), MB_OK);
			  }
		  }
		  break;
	  case IDC_LOG_FILE_SET:
		  {
			  tmp = GetFilePath(TranslateT("Set log file"), _T("*"), TranslateT("LOG files"), 1);
			  SetDlgItemText(hwndDlg, IDC_LOG_FILE_EDIT, tmp);
			  mir_free(tmp);
		  }
		  break; 
	  default:
		  break;
      }      
      SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
      break;
    }
   
  case WM_NOTIFY:
    {
		EnableWindow(GetDlgItem(hwndDlg, IDC_AUTO_EXCHANGE), (bIsMiranda09 && IsDlgButtonChecked(hwndDlg, IDC_JABBER_API)));
		if(hdr && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != (-1))
		{
			if(hdr->hdr.code == NM_CLICK)
			{
				item_num = hdr->iItem;
			}
			else if(hdr->hdr.code == LVN_ITEMCHANGED)
			{
				void setClistIcon(MCONTACT hContact);
				void setSrmmIcon(MCONTACT hContact);
				if(ListView_GetCheckState(hwndList, item_num))
					db_set_b(user_data[item_num+1], szGPGModuleName, "GPGEncryption", 1);
				else
					db_set_b(user_data[item_num+1], szGPGModuleName, "GPGEncryption", 0);
				setClistIcon(user_data[item_num+1]);
				setSrmmIcon(user_data[item_num+1]);
			}
		}
		switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
			extern bool bJabberAPI, bFileTransfers;
			bDebugLog = CheckStateStoreDB(hwndDlg, IDC_DEBUG_LOG, "bDebugLog") != 0;
			if(bDebugLog)
				debuglog.init();
			bJabberAPI = CheckStateStoreDB(hwndDlg, IDC_JABBER_API, "bJabberAPI") != 0;
			bool old_bFileTransfers = db_get_b(NULL, szGPGModuleName, "bFileTransfers", 0) != 0;
			bFileTransfers = CheckStateStoreDB(hwndDlg, IDC_FILE_TRANSFERS, "bFileTransfers") != 0;
			if(bFileTransfers != old_bFileTransfers)
			{
				db_set_b(NULL, szGPGModuleName, "bSameAction", 0);
				bSameAction = false;
			}
			bAutoExchange = CheckStateStoreDB(hwndDlg, IDC_AUTO_EXCHANGE, "bAutoExchange") != 0;
			{
				TCHAR tmp[512];
				GetDlgItemText(hwndDlg, IDC_LOG_FILE_EDIT, tmp, SIZEOF(tmp));
				db_set_ts(NULL, szGPGModuleName, "szLogFilePath", tmp);
			}
          return TRUE;
        }
      }
    }
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
		tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T("gpg.exe"));
		SetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp);
		mir_free(tmp);
		tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T("gpg"));
		SetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp);
		mir_free(tmp);
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_SET_BIN_PATH:
		  {
			  GetFilePath(TranslateT("Choose gpg.exe"), "szGpgBinPath", _T("*.exe"), TranslateT("EXE Executables"));
			  tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T("gpg.exe"));
			  SetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp);
			  bool gpg_exists = false;
			  {
				  if(_waccess(tmp, 0) != -1)
					  gpg_exists = true;
				  if(gpg_exists)
				  {
					  bool bad_version = false;
					  TCHAR *tmp_path = UniGetContactSettingUtf(NULL, szGPGModuleName, "szGpgBinPath", _T(""));
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
					  auto old_gpg_state = gpg_valid;
					  gpg_valid = true;
					  gpg_launcher(params);
					  gpg_valid = old_gpg_state;
					  db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp_path);
					  mir_free(tmp_path);
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
						  MessageBox(0, TranslateT("This is not GnuPG binary!\nIt is recommended to use GnuPG v1.x.x with this plugin."), TranslateT("Warning"), MB_OK);
					  }
/*					  if(bad_version) //looks like working fine with gpg2
						  MessageBox(0, TranslateT("Unsupported GnuPG version found, use at you own risk!\nIt is recommended to use GnuPG v1.x.x with this plugin."), _T("Warning"), MB_OK); */
				  }
			  }
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
			  GetFolderPath(TranslateT("Set home directory"), "szHomePath");
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
			  mir_free(atmp);
		  }
		  break;
	  default:
		break;
      }
      
      SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
      break;
    }
    
  case WM_NOTIFY:
    {
      switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
		  TCHAR tmp[512];
		  GetDlgItemText(hwndDlg, IDC_BIN_PATH, tmp, SIZEOF(tmp));
		  db_set_ts(NULL, szGPGModuleName, "szGpgBinPath", tmp);
		  GetDlgItemText(hwndDlg, IDC_HOME_DIR, tmp, SIZEOF(tmp));
		  while(tmp[_tcslen(tmp)-1] == '\\')
			  tmp[_tcslen(tmp)-1] = '\0';
		  db_set_ts(NULL, szGPGModuleName, "szHomePath", tmp);
          return TRUE;
        }
      }
	}
    break;
  }
  return FALSE;
}

static INT_PTR CALLBACK DlgProcGpgMsgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
    {
		TranslateDialogDefault(hwndDlg);
		CheckStateLoadDB(hwndDlg, IDC_APPEND_TAGS, "bAppendTags", 0);
		CheckStateLoadDB(hwndDlg, IDC_STRIP_TAGS, "bStripTags", 0);
		{
			TCHAR *tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInOpenTag", _T("<GPGdec>"));
			SetDlgItemText(hwndDlg, IDC_IN_OPEN_TAG, tmp);
			mir_free(tmp);
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szInCloseTag", _T("</GPGdec>"));
			SetDlgItemText(hwndDlg, IDC_IN_CLOSE_TAG, tmp);
			mir_free(tmp);
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutOpenTag", _T("<GPGenc>"));
			SetDlgItemText(hwndDlg, IDC_OUT_OPEN_TAG, tmp);
			mir_free(tmp);
			tmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szOutCloseTag", _T("</GPGenc>"));
			SetDlgItemText(hwndDlg, IDC_OUT_CLOSE_TAG, tmp);
			mir_free(tmp);
		}
      return TRUE;
    }
    
 
  case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
	  case IDC_APPEND_TAGS:
		  break;
	  default:
		break;
      }
      
      SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
      break;
    }
    
  case WM_NOTIFY:
    {
      switch (((LPNMHDR)lParam)->code)
      {
        
      case PSN_APPLY:
        {
			bAppendTags = CheckStateStoreDB(hwndDlg, IDC_APPEND_TAGS, "bAppendTags") != 0;
			bStripTags = CheckStateStoreDB(hwndDlg, IDC_STRIP_TAGS, "bStripTags") != 0;
			{
				TCHAR tmp[128];
				GetDlgItemText(hwndDlg, IDC_IN_OPEN_TAG, tmp, SIZEOF(tmp));
				db_set_ts(NULL, szGPGModuleName, "szInOpenTag", tmp);
				mir_free(inopentag);
				inopentag = (TCHAR*)mir_alloc(sizeof(TCHAR)* (_tcslen(tmp)+1));
				_tcscpy(inopentag, tmp);
				GetDlgItemText(hwndDlg, IDC_IN_CLOSE_TAG, tmp, SIZEOF(tmp));
				db_set_ts(NULL, szGPGModuleName, "szInCloseTag", tmp);
				mir_free(inclosetag);
				inclosetag = (TCHAR*)mir_alloc(sizeof(TCHAR)* (_tcslen(tmp)+1));
				_tcscpy(inclosetag, tmp);
				GetDlgItemText(hwndDlg, IDC_OUT_OPEN_TAG, tmp, SIZEOF(tmp));
				db_set_ts(NULL, szGPGModuleName, "szOutOpenTag", tmp);
				mir_free(outopentag);
				outopentag = (TCHAR*)mir_alloc(sizeof(TCHAR)* (_tcslen(tmp)+1));
				_tcscpy(outopentag, tmp);
				GetDlgItemText(hwndDlg, IDC_OUT_CLOSE_TAG, tmp, SIZEOF(tmp));
				db_set_ts(NULL, szGPGModuleName, "szOutCloseTag", tmp);
				mir_free(outclosetag);
				outclosetag = (TCHAR*)mir_alloc(sizeof(TCHAR)*(_tcslen(tmp)+1));
				_tcscpy(outclosetag, tmp);
			}
          return TRUE;
        }
      }
	}
    break;
  }
  return FALSE;
}

static INT_PTR CALLBACK DlgProcGpgAdvOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		extern bool bJabberAPI;
		TranslateDialogDefault(hwndDlg);
		CheckStateLoadDB(hwndDlg, IDC_PRESCENSE_SUBSCRIPTION, "bPresenceSigning", 0);
		EnableWindow(GetDlgItem(hwndDlg, IDC_PRESCENSE_SUBSCRIPTION), bJabberAPI);

		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EXPORT:
			{
				INT_PTR ExportGpGKeys(WPARAM w, LPARAM l);
				ExportGpGKeys(NULL, NULL);
			}
			break;
		case IDC_IMPORT:
			{
				INT_PTR ImportGpGKeys(WPARAM w, LPARAM l);
				ImportGpGKeys(NULL, NULL);
			}
			break;
		}

		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			bPresenceSigning = CheckStateStoreDB(hwndDlg, IDC_PRESCENSE_SUBSCRIPTION, "bPresenceSigning") != 0;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

HWND hPubKeyEdit = NULL;

static LRESULT CALLBACK editctrl_ctrl_a(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_KEYDOWN:
		if(wParam == 0x41 && GetKeyState(VK_CONTROL)< 0 )
			SendMessage(hwndDlg, EM_SETSEL, 0, -1);
		return 0;
	}
	return mir_callNextSubclass(hwndDlg, editctrl_ctrl_a, msg, wParam, lParam);
}

static INT_PTR CALLBACK DlgProcLoadPublicKey(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static MCONTACT hContact;
	TCHAR *tmp = NULL;
	wstring key_buf;
	wstring::size_type ws1 = 0, ws2 = 0;
	switch (msg) {
	case WM_INITDIALOG:
		{
			hContact = user_data[1];
			SetWindowPos(hwndDlg, 0, load_key_rect.left, load_key_rect.top, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_PUBLIC_KEY_EDIT), editctrl_ctrl_a);
			MCONTACT hcnt = db_mc_tryMeta(hContact);
			TranslateDialogDefault(hwndDlg);
			{
				wstring msg = TranslateT("Load Public GPG Key for ");
				msg += (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hcnt, GCDNF_TCHAR);
				SetWindowText(hwndDlg, msg.c_str());
			}
			if(!hcnt)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_SELECT_EXISTING), 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_ENCRYPTION), 0);
			}
			if(isContactSecured(hcnt))
				SetDlgItemText(hwndDlg, IDC_ENABLE_ENCRYPTION, TranslateT("Turn off encryption"));
			else
			{
				SetDlgItemText(hwndDlg, IDC_ENABLE_ENCRYPTION, TranslateT("Turn on encryption"));
				CheckDlgButton(hwndDlg, IDC_ENABLE_ENCRYPTION, 1);
			}
			if(hcnt)
			{
				tmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "GPGPubKey", _T(""));
				wstring str = tmp;
				mir_free(tmp); tmp = NULL;
				wstring::size_type p = 0, stop = 0;
				if(!str.empty())
				{
					for(;;)
					{
						if((p = str.find(_T("\n"), p+2)) != wstring::npos)
						{
							if(p > stop)
							{
								stop = p;
								str.insert(p, _T("\r"));
							}			
							else 
								break;
						}
					}
				}
				//			char *tmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "KeyID_Prescense", "");
				if(!hcontact_data[hcnt].key_in_prescense.empty())
				{
					char *tmp2 = UniGetContactSettingUtf(hcnt, szGPGModuleName, "KeyID", "");
					if(!tmp2[0])
					{
						string out;
						DWORD code;
						std::vector<wstring> cmd;
						cmd.push_back(L"--export");
						cmd.push_back(L"-a");
						cmd.push_back(toUTF16(hcontact_data[hcnt].key_in_prescense));
						gpg_execution_params params(cmd);
						pxResult result;
						params.out = &out;
						params.code = &code;
						params.result = &result;
						gpg_launcher(params); //TODO: handle errors
						if((out.find("-----BEGIN PGP PUBLIC KEY BLOCK-----") != string::npos) && (out.find("-----END PGP PUBLIC KEY BLOCK-----") != string::npos))
						{
							string::size_type p = 0, stop = 0;
							for(;;)
							{
								if((p = out.find("\n", p+2)) != string::npos)
								{
									if(p > stop)
									{
										stop = p;
										out.insert(p, "\r");
									}			
									else 
										break;
								}
							}
							TCHAR *tmp3 = mir_a2t(out.c_str());
							str.clear();
							str.append(tmp3);
							mir_free(tmp3);
							string msg = Translate("Load Public GPG Key for ");
							msg += (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hcnt, 0);
							msg += " (Key ID: ";
							msg += hcontact_data[hcnt].key_in_prescense;
							msg += Translate(" found in presence, and exists in keyring.)");
							SetWindowTextA(hwndDlg, msg.c_str());
						}
						else
						{
							string msg = Translate("Load Public GPG Key (Key ID: ");
							msg += hcontact_data[hcnt].key_in_prescense;
							msg += Translate(" found in presence.)");
							SetWindowTextA(hwndDlg, msg.c_str());
							EnableWindow(GetDlgItem(hwndDlg, IDC_IMPORT), 1);
						}
					}
					mir_free(tmp2);
				}
				if(tmp)
					mir_free(tmp);
				SetDlgItemText(hwndDlg, IDC_PUBLIC_KEY_EDIT, !str.empty()?str.c_str():_T(""));
			}
			hPubKeyEdit = GetDlgItem(hwndDlg, IDC_PUBLIC_KEY_EDIT);
			return TRUE;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case ID_OK:
				{
					tmp = new TCHAR [40960];
					TCHAR *begin, *end;
					GetDlgItemText(hwndDlg, IDC_PUBLIC_KEY_EDIT, tmp, 40960);
					key_buf.append(tmp);
					key_buf.append(_T("\n")); //no new line at end of file )
					delete [] tmp;
					while((ws1 = key_buf.find(_T("\r"), ws1)) != wstring::npos)
					{
						key_buf.erase(ws1, 1); //remove windows specific trash
					}
					ws1 = 0;
					if(((ws2 = key_buf.find(_T("-----END PGP PUBLIC KEY BLOCK-----"))) != wstring::npos) && ((ws1 = key_buf.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"))) != wstring::npos))
					{
						begin = (TCHAR*)mir_alloc(sizeof(TCHAR) * (_tcslen(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----")) + 1));
						_tcscpy(begin, _T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
						end = (TCHAR*)mir_alloc(sizeof( TCHAR) * (_tcslen(_T("-----END PGP PUBLIC KEY BLOCK-----")) + 1));
						_tcscpy(end, _T("-----END PGP PUBLIC KEY BLOCK-----"));
					}
					else if(((ws2 = key_buf.find(_T("-----END PGP PRIVATE KEY BLOCK-----"))) != wstring::npos) && ((ws1 = key_buf.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"))) != wstring::npos))
					{
						begin = (TCHAR*)mir_alloc(sizeof(TCHAR) * (_tcslen(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----")) + 1));
						_tcscpy(begin, _T("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
						end = (TCHAR*)mir_alloc(sizeof(TCHAR) * (_tcslen(_T("-----END PGP PRIVATE KEY BLOCK-----")) + 1));
						_tcscpy(end, _T("-----END PGP PRIVATE KEY BLOCK-----"));
					}
					else
					{
						MessageBox(0, TranslateT("This is not public or private key"), _T("INFO"), MB_OK);
						break;
					}
					ws2 += _tcslen(end);
					bool allsubcontacts = false;
					{
						if(db_mc_isMeta(hContact))
						{
							if(MessageBox(0, TranslateT("Do you want to load key for all subcontacts?"), TranslateT("Metacontact detected"), MB_YESNO) == IDYES)
							{
								allsubcontacts = true;
								int count = db_mc_getSubCount(hContact);
								for(int i = 0; i < count; i++)
								{
									MCONTACT hcnt = db_mc_getSub(hContact, i);
									if(hcnt)
										db_set_ts(hcnt, szGPGModuleName, "GPGPubKey", key_buf.substr(ws1,ws2-ws1).c_str());
								}
							}
							else db_set_ts(metaGetMostOnline(hContact), szGPGModuleName, "GPGPubKey", key_buf.substr(ws1,ws2-ws1).c_str());
						}
						else db_set_ts(hContact, szGPGModuleName, "GPGPubKey", key_buf.substr(ws1,ws2-ws1).c_str());
					}
					tmp = (TCHAR*)mir_alloc(sizeof( TCHAR) * (key_buf.length()+1));
					_tcscpy(tmp, key_buf.substr(ws1,ws2-ws1).c_str());
					{ //gpg execute block
						std::vector<wstring> cmd;
						TCHAR tmp2[MAX_PATH] = {0};
						TCHAR *ptmp;
						string output;
						DWORD exitcode;
						{
							MCONTACT hcnt = db_mc_tryMeta(hContact);
							ptmp = UniGetContactSettingUtf(NULL, szGPGModuleName, "szHomePath", _T(""));
							_tcscpy(tmp2, ptmp);
							mir_free(ptmp);
							_tcscat(tmp2, _T("\\"));
							_tcscat(tmp2, _T("temporary_exported.asc"));
							boost::filesystem::remove(tmp2);
							wfstream f(tmp2, std::ios::out);
							ptmp = UniGetContactSettingUtf(hcnt, szGPGModuleName, "GPGPubKey", _T(""));
							wstring str = ptmp;
							mir_free(ptmp);
							wstring::size_type s = 0;
							while((s = str.find(_T("\r"), s)) != wstring::npos)
							{
								str.erase(s, 1);
							}
							f<<str.c_str();
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
						mir_free(begin);
						mir_free(end);
						if(hContact)
						{
							if(db_mc_isMeta(hContact))
							{
								if(allsubcontacts)
								{
									int count = db_mc_getSubCount(hContact);
									for(int i = 0; i < count; i++)
									{
										MCONTACT hcnt = db_mc_getSub(hContact, i);
										if(hcnt)
											db_unset(hcnt, szGPGModuleName, "bAlwatsTrust");
									}
								}
								else db_unset(metaGetMostOnline(hContact), szGPGModuleName, "bAlwatsTrust");
							}
							else db_unset(hContact, szGPGModuleName, "bAlwatsTrust");
						}
						{
							TCHAR *tmp;
							if(output.find("already in secret keyring") != string::npos)
							{
								MessageBox(0, TranslateT("Key already in secret keyring."), TranslateT("Info"), MB_OK);
								boost::filesystem::remove(tmp2);
								break;
							}
							char *tmp2;
							string::size_type s = output.find("gpg: key ") + strlen("gpg: key ");
							string::size_type s2 = output.find(":", s);
							tmp2 = (char*)mir_alloc((output.substr(s,s2-s).length()+1)*sizeof(char));
							strcpy(tmp2, output.substr(s,s2-s).c_str());
							mir_utf8decode(tmp2, 0);
							{
								if(db_mc_isMeta(hContact))
								{
									if(allsubcontacts)
									{
										int count = db_mc_getSubCount(hContact);
										for(int i = 0; i < count; i++)
										{
											MCONTACT hcnt = db_mc_getSub(hContact, i);
											if(hcnt)
												db_set_s(hcnt, szGPGModuleName, "KeyID", tmp2);
										}
									}
									else
										db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyID", tmp2);
								}
								else
									db_set_s(hContact, szGPGModuleName, "KeyID", tmp2);
							}
							mir_free(tmp2);
							tmp = mir_wstrdup(toUTF16(output.substr(s,s2-s)).c_str());
							if(hContact && hwndList_p)
								ListView_SetItemText(hwndList_p, item_num, 1, tmp);
							mir_free(tmp);
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
							if(s2 != string::npos && s != string::npos)
							{
								tmp2 = (char*)mir_alloc(sizeof(char)*(output.substr(s,s2-s-(uncommon?1:0)).length()+1));
								strcpy(tmp2, output.substr(s,s2-s-(uncommon?1:0)).c_str());
								mir_utf8decode(tmp2, 0);
								if(hContact)
								{
									if(db_mc_isMeta(hContact))
									{
										if(allsubcontacts)
										{
											int count = db_mc_getSubCount(hContact);
											for(int i = 0; i < count; i++)
											{
												MCONTACT hcnt = db_mc_getSub(hContact, i);
												if(hcnt)
													db_set_s(hcnt, szGPGModuleName, "KeyMainName", output.substr(s,s2-s-1).c_str());
											}
										}
										else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainName", output.substr(s,s2-s-1).c_str());
									}
									else db_set_s(hContact, szGPGModuleName, "KeyMainName", output.substr(s,s2-s-1).c_str());
								}
								mir_free(tmp2);
								tmp = mir_wstrdup(toUTF16(output.substr(s,s2-s-1)).c_str());
								if(hContact && hwndList_p)
									ListView_SetItemText(hwndList_p, item_num, 2, tmp);
								mir_free(tmp);
								if((s = output.find(")", s2)) == string::npos)
									s = output.find(">", s2);
								else if(s > output.find(">", s2))
									s = output.find(">", s2);
								s2++;
								if(s != string::npos && s2 != string::npos)
								{
									if(output[s] == ')')
									{
										tmp2 = (char*)mir_alloc((output.substr(s2,s-s2).length()+1)*sizeof(char));
										strcpy(tmp2, output.substr(s2,s-s2).c_str());
										mir_utf8decode(tmp2, 0);
										if(hContact)
										{
											if(db_mc_isMeta(hContact))
											{
												if(allsubcontacts)
												{
													int count = db_mc_getSubCount(hContact);
													for(int i = 0; i < count; i++)
													{
														MCONTACT hcnt = db_mc_getSub(hContact, i);
														if(hcnt)
															db_set_s(hcnt, szGPGModuleName, "KeyComment", output.substr(s2,s-s2).c_str());
													}
												}
												else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyComment", output.substr(s2,s-s2).c_str());
											}
											else db_set_s(hContact, szGPGModuleName, "KeyComment", output.substr(s2,s-s2).c_str());
										}
										mir_free(tmp2);
										s+=3;
										s2 = output.find(">", s);
										tmp2 = (char*)mir_alloc((output.substr(s,s2-s).length()+1) * sizeof(char));
										strcpy(tmp2, output.substr(s,s2-s).c_str());
										mir_utf8decode(tmp2, 0);
										if(hContact)
										{
											if(db_mc_isMeta(hContact))
											{
												if(allsubcontacts)
												{
													int count = db_mc_getSubCount(hContact);
													for(int i = 0; i < count; i++)
													{
														MCONTACT hcnt = db_mc_getSub(hContact, i);
														if(hcnt)
															db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s,s2-s).c_str());
													}
												}
												else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s,s2-s).c_str());
											}
											else db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s,s2-s).c_str());
										}
										mir_free(tmp2);
										tmp = mir_wstrdup(toUTF16(output.substr(s,s2-s)).c_str());
										if(hContact && hwndList_p)
											ListView_SetItemText(hwndList_p, item_num, 3, tmp);
										mir_free(tmp);
									}
									else
									{
										tmp2 = (char*)mir_alloc(output.substr(s2,s-s2).length()+1);
										strcpy(tmp2, output.substr(s2,s-s2).c_str());
										mir_utf8decode(tmp2, 0);
										if(hContact)
										{
											if(db_mc_isMeta(hContact))
											{
												if(allsubcontacts)
												{
													int count = db_mc_getSubCount(hContact);
													for(int i = 0; i < count; i++)
													{
														MCONTACT hcnt = db_mc_getSub(hContact, i);
														if(hcnt)
															db_set_s(hcnt, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
													}
												}
												else db_set_s(metaGetMostOnline(hContact), szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
											}
											else db_set_s(hContact, szGPGModuleName, "KeyMainEmail", output.substr(s2,s-s2).c_str());
										}
										mir_free(tmp2);
										tmp = mir_wstrdup(toUTF16(output.substr(s2,s-s2)).c_str());
										if(hContact && hwndList_p)
											ListView_SetItemText(hwndList_p, item_num, 3, tmp);
										mir_free(tmp);
									}
								}
							}
							if(hContact && hwndList_p)
							{					  
								ListView_SetColumnWidth(hwndList_p, 0, LVSCW_AUTOSIZE);
								ListView_SetColumnWidth(hwndList_p, 1, LVSCW_AUTOSIZE);
								ListView_SetColumnWidth(hwndList_p, 2, LVSCW_AUTOSIZE);
								ListView_SetColumnWidth(hwndList_p, 3, LVSCW_AUTOSIZE);
							}
						}
						if(!hContact)
						{				  
							TCHAR *fp = UniGetContactSettingUtf(hContact, szGPGModuleName, "KeyID", _T(""));
							{
								string out;
								DWORD code;
								std::vector<wstring> cmd;
								cmd.push_back(L"--batch");
								cmd.push_back(L"-a");
								cmd.push_back(L"--export");
								cmd.push_back(fp);
								mir_free(fp);
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
								db_set_s(hContact, szGPGModuleName, "GPGPubKey", out.c_str());
							}
						}
						tmp = mir_wstrdup(toUTF16(output).c_str());
						MessageBox(0, tmp, _T(""), MB_OK);
						mir_free(tmp);
						boost::filesystem::remove(tmp2);
					}
					key_buf.clear();
					if(IsDlgButtonChecked(hwndDlg, IDC_ENABLE_ENCRYPTION))
					{
						if(hContact)
						{
							if(db_mc_isMeta(hContact))
							{
								if(allsubcontacts)
								{
									int count = db_mc_getSubCount(hContact);
									for(int i = 0; i < count; i++)
									{
										MCONTACT hcnt = db_mc_getSub(hContact, i);
										if(hcnt)
										{
											if(!isContactSecured(hcnt))
												db_set_b(hcnt, szGPGModuleName, "GPGEncryption", 1);
											else
												db_set_b(hcnt, szGPGModuleName, "GPGEncryption", 0);
											setSrmmIcon(hContact);
											setClistIcon(hContact);
										}
									}
								}
								else if(!isContactSecured(hContact))
									db_set_b(metaGetMostOnline(hContact), szGPGModuleName, "GPGEncryption", 1);
								else
									db_set_b(metaGetMostOnline(hContact), szGPGModuleName, "GPGEncryption", 0);
							}
							else if(!isContactSecured(hContact))
								db_set_b(hContact, szGPGModuleName, "GPGEncryption", 1);
							else
								db_set_b(hContact, szGPGModuleName, "GPGEncryption", 0);
						}
					}
					DestroyWindow(hwndDlg);
				}
				break;
			case ID_LOAD_FROM_FILE:
				{
					tmp = GetFilePath(TranslateT("Set file containing GPG public key"), _T("*"), TranslateT("GPG public key file"));
					if(!tmp)
					{
						break;
					}
					wfstream f(tmp, std::ios::in | std::ios::ate | std::ios::binary);
					delete [] tmp;
					if(!f.is_open())
					{
						MessageBox(0, TranslateT("Failed to open file"), TranslateT("Error"), MB_OK);
						break;
					}
					if(f.is_open())
					{
						std::wifstream::pos_type size = f.tellg();
						TCHAR *tmp = new TCHAR [(std::ifstream::pos_type)size+(std::ifstream::pos_type)1];
						f.seekg(0, std::ios::beg);
						f.read(tmp, size);
						tmp[size]= '\0';
						key_buf.append(tmp);
						delete [] tmp;
						f.close();
					}
					if(key_buf.empty())
					{
						key_buf.clear();
						if(bDebugLog)
							debuglog<<std::string(time_str()+": info: Failed to read key file");
						break;
					}
					ws2 = key_buf.find(_T("-----END PGP PUBLIC KEY BLOCK-----"));
					ws1 = key_buf.find(_T("-----BEGIN PGP PUBLIC KEY BLOCK-----"));
					if(ws2 == wstring::npos || ws1 == wstring::npos)
					{
						ws2 = key_buf.find(_T("-----END PGP PRIVATE KEY BLOCK-----"));
						ws1 = key_buf.find(_T("-----BEGIN PGP PRIVATE KEY BLOCK-----"));
					}
					if(ws2 == wstring::npos || ws1 == wstring::npos)
					{
						MessageBox(0, TranslateT("There is no public or private key."), TranslateT("Info"), MB_OK);
						break;
					}
					ws2 += _tcslen(_T("-----END PGP PUBLIC KEY BLOCK-----"));
					SetDlgItemText(hwndDlg, IDC_PUBLIC_KEY_EDIT, key_buf.substr(ws1,ws2-ws1).c_str());
					key_buf.clear();
				}
				break;
			case IDC_IMPORT:
				new_key_hcnt_mutex.lock();
				new_key_hcnt = hContact;
				void ShowImportKeyDialog();
				ShowImportKeyDialog();
				break;
			case IDC_SELECT_EXISTING:
				void ShowSelectExistingKeyDialog();
				ShowSelectExistingKeyDialog();
				break;
			}
			break;
		}

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			return TRUE;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		GetWindowRect(hwndDlg, &load_key_rect);
		db_set_dw(NULL, szGPGModuleName, "LoadKeyWindowX", load_key_rect.left);
		db_set_dw(NULL, szGPGModuleName, "LoadKeyWindowY", load_key_rect.top);
		break;
	}

	return FALSE;
}

void ShowLoadPublicKeyDialog()
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_LOAD_PUBLIC_KEY), NULL, DlgProcLoadPublicKey);
}
