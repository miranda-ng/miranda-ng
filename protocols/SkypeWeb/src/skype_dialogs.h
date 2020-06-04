/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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
	CCtrlCombo m_combo;

public:
	CSkypeInviteDlg(CSkypeProto *proto);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	MCONTACT m_hContact = 0;
};

class CSkypeGCCreateDlg : public CSkypeDlgBase
{
	CCtrlClc m_clc;

public:
	CSkypeGCCreateDlg(CSkypeProto *proto);
	~CSkypeGCCreateDlg();

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void FilterList(CCtrlClc*);
	void ResetListOptions(CCtrlClc*);

	LIST<char> m_ContactsList;
};


#endif //_SKYPE_DIALOGS_H_