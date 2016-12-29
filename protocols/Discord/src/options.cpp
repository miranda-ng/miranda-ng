/*
Copyright © 2016 Miranda NG team

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
	CCtrlEdit m_edGroup, m_edUserName, m_edPassword;
	ptrW m_wszOldGroup;

public:
	CDiscardAccountOptions(CDiscordProto *ppro) :
		CProtoDlgBase<CDiscordProto>(ppro, IDD_OPTIONS_ACCOUNT),
		m_edGroup(this, IDC_GROUP),
		m_edUserName(this, IDC_USERNAME),
		m_edPassword(this, IDC_PASSWORD),
		m_wszOldGroup(mir_wstrdup(ppro->m_wszDefaultGroup))
	{
		CreateLink(m_edGroup, ppro->m_wszDefaultGroup);
		CreateLink(m_edUserName, ppro->m_wszEmail);
	}

	virtual void OnInitDialog() override
	{
		ptrW buf(m_proto->getWStringA(DB_KEY_PASSWORD));
		if (buf)
			m_edPassword.SetText(buf);
	}

	virtual void OnApply() override
	{
		if (mir_wstrcmp(m_proto->m_wszDefaultGroup, m_wszOldGroup))
			Clist_GroupCreate(NULL, m_proto->m_wszDefaultGroup);

		ptrW buf(m_edPassword.GetText());
		m_proto->setWString(DB_KEY_PASSWORD, buf);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

int CDiscordProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInstance;
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");

	odp.position = 1;
	odp.szTab.w = LPGENW("Account");
	odp.pDialog = new CDiscardAccountOptions(this);
	Options_AddPage(wParam, &odp);
	return 0;
}
