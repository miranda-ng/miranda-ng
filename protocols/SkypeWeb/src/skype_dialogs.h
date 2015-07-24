/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SKYPE_DIALOGS_H_
#define _SKYPE_DIALOGS_H_

typedef CProtoDlgBase<CSkypeProto> CSkypeDlgBase;

class CSkypeInviteDlg : public CSkypeDlgBase
{
private:
	typedef CSkypeDlgBase CSuper;
	CCtrlButton m_ok;
	CCtrlButton m_cancel;
	CCtrlCombo  m_combo;

protected:

	void OnInitDialog();
	void btnOk_OnOk(CCtrlButton*);

public:
	MCONTACT m_hContact;

	CSkypeInviteDlg(CSkypeProto *proto);
};

class CSkypeGCCreateDlg : public CSkypeDlgBase
{
private:
	typedef CSkypeDlgBase CSuper;
	CCtrlButton m_ok;
	CCtrlButton m_cancel;
	CCtrlClc    m_clc;

protected:

	void OnInitDialog();
	void btnOk_OnOk(CCtrlButton*);
	void FilterList(CCtrlClc*);
	void ResetListOptions(CCtrlClc*);


public:

	std::vector<MCONTACT> m_hContacts;
	CSkypeGCCreateDlg(CSkypeProto *proto);
};


#endif //_SKYPE_DIALOGS_H_