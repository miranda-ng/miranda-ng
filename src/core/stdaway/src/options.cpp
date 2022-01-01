/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

static const int statusModes[] =
{
	ID_STATUS_OFFLINE, ID_STATUS_ONLINE, ID_STATUS_AWAY, ID_STATUS_NA, ID_STATUS_OCCUPIED, ID_STATUS_DND,
	ID_STATUS_FREECHAT, ID_STATUS_INVISIBLE, ID_STATUS_IDLE
};

class CAwayMsgOptsDlg : public CDlgBase
{
	struct
	{
		int ignore;
		int noDialog;
		int usePrevious;
		wchar_t msg[1024];
	}
	m_info[_countof(statusModes)];

	int oldPage = -1;

	CCtrlEdit etdMsg;
	CCtrlCombo cmbStatus;
	CCtrlCheck chkDontReply, chkUsePrev, chkUseSpecific, chkNoDialog;

public:
	CAwayMsgOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_AWAYMSG),
		etdMsg(this, IDC_MSG),
		cmbStatus(this, IDC_STATUS),
		chkUsePrev(this, IDC_USEPREVIOUS),
		chkNoDialog(this, IDC_NODIALOG),
		chkDontReply(this, IDC_DONTREPLY),
		chkUseSpecific(this, IDC_USESPECIFIC)
	{
		chkUsePrev.OnChange = chkDontReply.OnChange = chkUseSpecific.OnChange = Callback(this, &CAwayMsgOptsDlg::onSelChange_Status);
		cmbStatus.OnSelChanged = Callback(this, &CAwayMsgOptsDlg::onSelChange_Status);
	}

	bool OnInitDialog() override
	{
		for (auto &it : statusModes) {
			if (!(protoModeMsgFlags & Proto_Status2Flag(it)))
				continue;

			int j = cmbStatus.AddString(Clist_GetStatusModeDescription(it, 0), it);
			m_info[j].ignore = g_plugin.GetStatusModeByte(it, "Ignore");
			m_info[j].noDialog = g_plugin.GetStatusModeByte(it, "NoDlg", true);
			m_info[j].usePrevious = g_plugin.GetStatusModeByte(it, "UsePrev");

			DBVARIANT dbv;
			if (g_plugin.getWString(StatusModeToDbSetting(it, "Default"), &dbv))
				if (g_plugin.getWString(StatusModeToDbSetting(it, "Msg"), &dbv))
					dbv.pwszVal = mir_wstrdup(GetDefaultMessage(it));
			mir_wstrcpy(m_info[j].msg, dbv.pwszVal);
			mir_free(dbv.pwszVal);
		}
		
		cmbStatus.SetCurSel(0);
		onSelChange_Status(0);
		return true;
	}

	bool OnApply() override
	{
		onSelChange_Status(0);
		
		for (int i = cmbStatus.GetCount() - 1; i >= 0; i--) {
			int status = cmbStatus.GetItemData(i);
			g_plugin.SetStatusModeByte(status, "Ignore", (uint8_t)m_info[i].ignore);
			g_plugin.SetStatusModeByte(status, "NoDlg", (uint8_t)m_info[i].noDialog);
			g_plugin.SetStatusModeByte(status, "UsePrev", (uint8_t)m_info[i].usePrevious);
			g_plugin.setWString(StatusModeToDbSetting(status, "Default"), m_info[i].msg);
		}
		return true;
	}

	void onSelChange_Status(CCtrlCombo*)
	{
		if (oldPage != -1) {
			m_info[oldPage].ignore = chkDontReply.GetState();
			m_info[oldPage].noDialog = chkNoDialog.GetState();
			m_info[oldPage].usePrevious = chkUsePrev.GetState();
			etdMsg.GetText(m_info[oldPage].msg, _countof(m_info[oldPage].msg));
		}

		int i = cmbStatus.GetCurSel();
		chkDontReply.SetState(i < 0 ? 0 : m_info[i].ignore);
		chkNoDialog.SetState(i < 0 ? 0 : m_info[i].noDialog);
		chkUsePrev.SetState(i < 0 ? 0 : m_info[i].usePrevious);
		chkUseSpecific.SetState(i < 0 ? 0 : !m_info[i].usePrevious);

		etdMsg.SetText(i < 0 ? L"" : m_info[i].msg);

		chkNoDialog.Enable(i < 0 ? 0 : !m_info[i].ignore);
		chkUsePrev.Enable(i < 0 ? 0 : !m_info[i].ignore);
		chkUseSpecific.Enable(i < 0 ? 0 : !m_info[i].ignore);
		etdMsg.Enable(i < 0 ? 0 : !(m_info[i].ignore || m_info[i].usePrevious));
		oldPage = i;
	}
};

int AwayMsgOptInitialise(WPARAM wParam, LPARAM)
{
	if (protoModeMsgFlags == 0)
		return 0;

	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 870000000;
	odp.szTitle.a = LPGEN("Status messages");
	odp.szGroup.a = LPGEN("Status");
	odp.pDialog = new CAwayMsgOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
