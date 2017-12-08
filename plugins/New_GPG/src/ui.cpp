// Copyright � 2017 sss
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



void CDlgEncryptedFileMsgBox::OnInitDialog()
{
	globals.bDecryptFiles = false;
}

CDlgEncryptedFileMsgBox::CDlgEncryptedFileMsgBox() : CDlgBase(globals.hInst, IDD_ENCRYPTED_FILE_MSG_BOX),
chk_REMEMBER(this, IDC_REMEMBER),
btn_IGNORE(this, IDC_IGNORE), btn_DECRYPT(this, IDC_DECRYPT)
{
	btn_IGNORE.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_IGNORE);
	btn_DECRYPT.OnClick = Callback(this, &CDlgEncryptedFileMsgBox::onClick_DECRYPT);
}

void CDlgEncryptedFileMsgBox::OnDestroy()
{
	delete this;
}

void CDlgEncryptedFileMsgBox::onClick_IGNORE(CCtrlButton*)
{
	if (chk_REMEMBER.GetState())
	{
		db_set_b(NULL, szGPGModuleName, "bSameAction", 1);
		globals.bSameAction = true;
	}
	this->Close();
}

void CDlgEncryptedFileMsgBox::onClick_DECRYPT(CCtrlButton*)
{
	globals.bDecryptFiles = true;
	if (chk_REMEMBER.GetState())
	{
		db_set_b(NULL, szGPGModuleName, "bFileTransfers", 1);
		globals.bFileTransfers = true;
		db_set_b(NULL, szGPGModuleName, "bSameAction", 0);
		globals.bSameAction = false;
	}
	this->Close();
}



CDlgExportKeysMsgBox::CDlgExportKeysMsgBox() : CDlgBase(globals.hInst, IDD_EXPORT_TYPE),
btn_OK(this, IDC_OK), btn_CANCEL(this, IDC_CANCEL),
chk_PUBLIC(this, IDC_PUBLIC), chk_PRIVATE(this, IDC_PRIVATE), chk_ALL(this, IDC_ALL)
{
	btn_OK.OnClick = Callback(this, &CDlgExportKeysMsgBox::onClick_OK);
	btn_CANCEL.OnClick = Callback(this, &CDlgExportKeysMsgBox::onClick_CANCEL);
}
void CDlgExportKeysMsgBox::OnInitDialog()
{
	chk_PUBLIC.SetState(1);
}
void CDlgExportKeysMsgBox::OnDestroy()
{
	delete this;
}
void CDlgExportKeysMsgBox::onClick_OK(CCtrlButton*)
{
	if (chk_PUBLIC.GetState())
		ExportGpGKeysFunc(0);
	else if (chk_PRIVATE.GetState())
		ExportGpGKeysFunc(1);
	else if (chk_ALL.GetState())
		ExportGpGKeysFunc(2);
	this->Close();
}
void CDlgExportKeysMsgBox::onClick_CANCEL(CCtrlButton*)
{
	this->Close();
}



CDlgChangePasswdMsgBox::CDlgChangePasswdMsgBox() : CDlgBase(globals.hInst, IDD_CHANGE_PASSWD),
btn_OK(this, ID_OK),
edit_NEW_PASSWD1(this, IDC_NEW_PASSWD1), edit_NEW_PASSWD2(this, IDC_NEW_PASSWD2), edit_OLD_PASSWD(this, IDC_OLD_PASSWD)
{
	btn_OK.OnClick = Callback(this, &CDlgChangePasswdMsgBox::onClick_OK);
}
void CDlgChangePasswdMsgBox::OnDestroy()
{
	delete this;
}
void CDlgChangePasswdMsgBox::onClick_OK(CCtrlButton*)
{
	//TODO: show some prgress
	{
		if (mir_wstrcmp(edit_NEW_PASSWD1.GetText(), edit_NEW_PASSWD2.GetText()))
		{
			MessageBox(m_hwnd, TranslateT("New passwords do not match"), TranslateT("Error"), MB_OK);
			return;
		}
		std::string old_pass, new_pass;
		//			wchar_t buf[256] = { 0 };
		new_pass = toUTF8(edit_NEW_PASSWD1.GetText());
		old_pass = toUTF8(edit_OLD_PASSWD.GetText());
		bool old_pass_match = false;
		wchar_t *pass = UniGetContactSettingUtf(NULL, szGPGModuleName, "szKeyPassword", L"");
		if (!mir_wstrcmp(pass, edit_OLD_PASSWD.GetText()))
			old_pass_match = true;
		mir_free(pass);

		if (!old_pass_match) {
			if (globals.key_id_global[0]) {
				string dbsetting = "szKey_";
				dbsetting += toUTF8(globals.key_id_global);
				dbsetting += "_Password";
				pass = UniGetContactSettingUtf(NULL, szGPGModuleName, dbsetting.c_str(), L"");
				if (!mir_wstrcmp(pass, edit_OLD_PASSWD.GetText()))
					old_pass_match = true;
				mir_free(pass);
			}
		}

		if (!old_pass_match)
			if (MessageBox(m_hwnd, TranslateT("Old password does not match, you can continue, but GPG will reject wrong password.\nDo you want to continue?"), TranslateT("Error"), MB_YESNO) == IDNO)
				return;

		std::vector<std::wstring> cmd;
		string output;
		DWORD exitcode;
		cmd.push_back(L"--edit-key");
		cmd.push_back(globals.key_id_global);
		cmd.push_back(L"passwd");
		gpg_execution_params_pass params(cmd, old_pass, new_pass);
		pxResult result;
		params.out = &output;
		params.code = &exitcode;
		params.result = &result;
		boost::thread gpg_thread(boost::bind(&pxEexcute_passwd_change_thread, &params));
		if (!gpg_thread.timed_join(boost::posix_time::minutes(10))) {
			gpg_thread.~thread();
			if (params.child)
				boost::process::terminate(*(params.child));
			if (globals.bDebugLog)
				globals.debuglog << std::string(time_str() + ": GPG execution timed out, aborted");
			this->Close();
			return;
		}
		if (result == pxNotFound)
			return;
	}
	this->Close();
}
