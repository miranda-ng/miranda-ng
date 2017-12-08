// Copyright © 2017 sss
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


class CDlgEncryptedFileMsgBox : public CDlgBase
{
public:
	CDlgEncryptedFileMsgBox();
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	void onClick_IGNORE(CCtrlButton*);
	void onClick_DECRYPT(CCtrlButton*);

private:
	CCtrlCheck chk_REMEMBER;
	CCtrlButton btn_IGNORE, btn_DECRYPT;
};

class CDlgExportKeysMsgBox : public CDlgBase
{
public:
	CDlgExportKeysMsgBox();
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	void onClick_OK(CCtrlButton*);
	void onClick_CANCEL(CCtrlButton*);

private:
	CCtrlButton btn_OK, btn_CANCEL;
	CCtrlCheck chk_PUBLIC, chk_PRIVATE, chk_ALL;
};

class CDlgChangePasswdMsgBox : public CDlgBase //always modal
{
public:
	CDlgChangePasswdMsgBox();
	virtual void OnDestroy() override;
	void onClick_OK(CCtrlButton*);
private:
	CCtrlButton btn_OK;
	CCtrlEdit edit_NEW_PASSWD1, edit_NEW_PASSWD2, edit_OLD_PASSWD;
};




#endif // UI_H