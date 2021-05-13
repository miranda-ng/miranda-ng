/*
Copyright © 2016-21 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

class CDiscardAccountOptions : public CProtoDlgBase<CDiscordProto>
{
	CCtrlCheck chkUseChats, chkHideChats, chkUseGroups, chkDeleteMsgs;
	CCtrlEdit m_edGroup, m_edUserName, m_edPassword;
	ptrW m_wszOldGroup;

public:
	CDiscardAccountOptions(CDiscordProto *ppro, int iDlgID, bool bFullDlg) :
		CProtoDlgBase<CDiscordProto>(ppro, iDlgID),
		m_edGroup(this, IDC_GROUP),
		m_edUserName(this, IDC_USERNAME),
		m_edPassword(this, IDC_PASSWORD),
		chkUseChats(this, IDC_USEGUILDS),
		chkHideChats(this, IDC_HIDECHATS),
		chkUseGroups(this, IDC_USEGROUPS),
		chkDeleteMsgs(this, IDC_DELETE_MSGS),
		m_wszOldGroup(mir_wstrdup(ppro->m_wszDefaultGroup))
	{
		CreateLink(m_edGroup, ppro->m_wszDefaultGroup);
		CreateLink(m_edUserName, ppro->m_wszEmail);
		if (bFullDlg) {
			CreateLink(chkUseChats, ppro->m_bUseGroupchats);
			CreateLink(chkHideChats, ppro->m_bHideGroupchats);
			CreateLink(chkUseGroups, ppro->m_bUseGuildGroups);
			CreateLink(chkDeleteMsgs, ppro->m_bSyncDeleteMsgs);

			chkUseChats.OnChange = Callback(this, &CDiscardAccountOptions::onChange_GroupChats);
		}
	}

	bool OnInitDialog() override
	{
		ptrW buf(m_proto->getWStringA(DB_KEY_PASSWORD));
		if (buf)
			m_edPassword.SetText(buf);
		return true;
	}

	bool OnApply() override
	{
		if (mir_wstrcmp(m_proto->m_wszDefaultGroup, m_wszOldGroup))
			Clist_GroupCreate(0, m_proto->m_wszDefaultGroup);

		ptrW buf(m_edPassword.GetText());
		m_proto->setWString(DB_KEY_PASSWORD, buf);
		return true;
	}

	void onChange_GroupChats(CCtrlCheck*)
	{
		bool bEnabled = chkUseChats.GetState();
		chkHideChats.Enable(bEnabled);
		chkUseGroups.Enable(bEnabled);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::SvcCreateAccMgrUI(WPARAM, LPARAM hwndParent)
{
	CDiscardAccountOptions *pDlg = new CDiscardAccountOptions(this, IDD_OPTIONS_ACCMGR, false);
	pDlg->SetParent((HWND)hwndParent);
	pDlg->Create();
	return (INT_PTR)pDlg->GetHwnd();
}

int CDiscordProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CDiscardAccountOptions(this, IDD_OPTIONS_ACCOUNT, true);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
