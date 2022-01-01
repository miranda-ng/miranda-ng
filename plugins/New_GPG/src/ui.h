// Copyright © 2017-22 sss
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
void ShowFirstRunDialog();

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

class CDlgKeyPasswordMsgBox : public CDlgBase //always modal
{
	char *inkeyid = nullptr;
	MCONTACT hContact;
	CCtrlData lbl_KEYID;
	CCtrlEdit edit_KEY_PASSWORD;
	CCtrlCheck chk_DEFAULT_PASSWORD, chk_SAVE_PASSWORD;

public:
	CDlgKeyPasswordMsgBox(MCONTACT _hContact);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
};

#endif // UI_H