/*
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

class CVoipOptionsDlg : public CDlgBase
{
	CCtrlEdit edtStun;
	CCtrlCheck chkEnable;

public:
	CVoipOptionsDlg(CJabberAccount *pAcc) :
		CDlgBase(g_plugin, IDD_OPTIONS),
		edtStun(this, IDC_STUN_ADDRESS),
		chkEnable(this, IDC_ENABLE_VOIP)
	{
		CreateLink(edtStun, pAcc->m_szStunServer);
		CreateLink(chkEnable, pAcc->m_bEnableVOIP);
	}

	void onChange_Enable(CCtrlCheck *)
	{
		bool bEnable = chkEnable.IsChecked();
		edtStun.Enable(bEnable);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module entry point

int OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Network");
	odp.szTab.w = LPGENW("VoIP");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;

	for (auto &it : g_arJabber) {
		if (auto *pa = Proto_GetAccount(it->m_szModuleName)) {
			odp.szTitle.w = pa->tszAccountName;
			odp.pDialog = new CVoipOptionsDlg(it);
			g_plugin.addOptions(wParam, &odp);
		}
	}

	return 0;
}
