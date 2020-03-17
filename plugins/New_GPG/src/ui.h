// Copyright © 2017-20 sss
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

#ifndef UI_H
#define UI_H

void ShowLoadPublicKeyDialog(MCONTACT hContact, bool bModal);

class CDlgEncryptedFileMsgBox : public CDlgBase
{
	CCtrlCheck chk_REMEMBER;
	CCtrlButton btn_IGNORE, btn_DECRYPT;

public:
	CDlgEncryptedFileMsgBox();
	bool OnInitDialog() override;

	void onClick_IGNORE(CCtrlButton*);
	void onClick_DECRYPT(CCtrlButton*);
};

class CDlgExportKeysMsgBox : public CDlgBase
{
	CCtrlCheck chk_PUBLIC, chk_PRIVATE, chk_ALL;

public:
	CDlgExportKeysMsgBox();

	bool OnInitDialog() override;
	bool OnApply() override;
};

class CDlgChangePasswdMsgBox : public CDlgBase //always modal
{
	CCtrlEdit edit_NEW_PASSWD1, edit_NEW_PASSWD2, edit_OLD_PASSWD;

public:
	CDlgChangePasswdMsgBox();

	bool OnApply() override;
};

class CDlgFirstRun : public CDlgBase
{
	void refresh_key_list();
	CCtrlListView list_KEY_LIST;
	CCtrlButton btn_COPY_PUBKEY, btn_EXPORT_PRIVATE, btn_CHANGE_PASSWD, btn_GENERATE_RANDOM, btn_GENERATE_KEY, btn_OTHER, btn_DELETE_KEY, btn_OK;
	CCtrlEdit edit_KEY_PASSWORD;
	CCtrlCombo combo_ACCOUNT;
	CCtrlData lbl_KEY_ID, lbl_GENERATING_KEY;
	wchar_t fp[16];
	const char *m_szCurrAcc = nullptr;

public:
	CDlgFirstRun();
	bool OnInitDialog() override;
	void OnDestroy() override;

	void onClick_COPY_PUBKEY(CCtrlButton*);
	void onClick_EXPORT_PRIVATE(CCtrlButton*);
	void onClick_CHANGE_PASSWD(CCtrlButton*);
	void onClick_GENERATE_RANDOM(CCtrlButton*);
	void onClick_GENERATE_KEY(CCtrlButton*);
	void onClick_OTHER(CCtrlButton*);
	void onClick_DELETE_KEY(CCtrlButton*);
	void onClick_OK(CCtrlButton*);
	void onChange_ACCOUNT(CCtrlCombo*);
	void onChange_KEY_LIST(CCtrlListView::TEventInfo *ev);
};

class CDlgGpgBinOpts : public CDlgBase
{
	CCtrlButton btn_SET_BIN_PATH, btn_SET_HOME_DIR, btn_OK, btn_GENERATE_RANDOM;
	CCtrlEdit edit_BIN_PATH, edit_HOME_DIR;
	CCtrlCheck chk_AUTO_EXCHANGE;

public:
	CDlgGpgBinOpts();
	bool OnInitDialog() override;
	void OnDestroy() override;
	
	void onClick_SET_BIN_PATH(CCtrlButton*);
	void onClick_SET_HOME_DIR(CCtrlButton*);
	void onClick_OK(CCtrlButton*);
	void onClick_GENERATE_RANDOM(CCtrlButton*);
};

class CDlgNewKey : public CDlgBase
{
	wstring new_key;
	MCONTACT hContact;
	CCtrlData lbl_KEY_FROM, lbl_MESSAGE;
	CCtrlButton btn_IMPORT, btn_IMPORT_AND_USE, btn_IGNORE_KEY;

public:
	CDlgNewKey(MCONTACT hContact, wstring new_key);
	bool OnInitDialog() override;
	void OnDestroy() override;
	
	void onClick_IMPORT(CCtrlButton*);
	void onClick_IMPORT_AND_USE(CCtrlButton*);
	void onClick_IGNORE_KEY(CCtrlButton*);
};

class CDlgKeyGen : public CDlgBase //TODO: in modal mode window destroying on any button press even without direct "Close" call
{
	CCtrlCombo combo_KEY_TYPE;
	CCtrlEdit edit_KEY_LENGTH, edit_KEY_PASSWD, edit_KEY_REAL_NAME, edit_KEY_EMAIL, edit_KEY_COMMENT, edit_KEY_EXPIRE_DATE;
	CCtrlData lbl_GENERATING_TEXT;

public:
	CDlgKeyGen();

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
};

class CDlgLoadExistingKey : public CDlgBase
{
	wchar_t id[16];
	CCtrlListView list_EXISTING_KEY_LIST;

public:
	CDlgLoadExistingKey();
	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void onChange_EXISTING_KEY_LIST(CCtrlListView::TEventInfo * /*ev*/);
};

class CDlgImportKey : public CDlgBase
{
	MCONTACT hContact;
	CCtrlCombo combo_KEYSERVER;
	CCtrlButton btn_IMPORT;

public:
	CDlgImportKey(MCONTACT hContact);
	bool OnInitDialog() override;
	void OnDestroy() override;
	
	void onClick_IMPORT(CCtrlButton*);
};

class CDlgKeyPasswordMsgBox : public CDlgBase //always modal
{
	char *inkeyid = nullptr;
	MCONTACT hContact;
	CCtrlData lbl_KEYID;
	CCtrlEdit edit_KEY_PASSWORD;
	CCtrlCheck chk_DEFAULT_PASSWORD, chk_SAVE_PASSWORD;
	CCtrlButton btn_OK, btn_CANCEL;

public:
	CDlgKeyPasswordMsgBox(MCONTACT _hContact);
	bool OnInitDialog() override;
	void OnDestroy() override;
	
	void onClick_OK(CCtrlButton*);
	void onClick_CANCEL(CCtrlButton*);
};

#endif // UI_H