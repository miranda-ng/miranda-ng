/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

class ClistOptsDlg : public CDlgBase
{
	CCtrlSpin cycleTime;
	CCtrlCombo status;
	CCtrlCheck sortByName, sortByStatus, sortByProto, chkOfflineBottom;
	CCtrlCheck chkCycle, chkDontCycle, chkMulti;

public:
	ClistOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CLIST),
		chkCycle(this, IDC_CYCLE),
		chkMulti(this, IDC_MULTITRAY),
		chkDontCycle(this, IDC_DONTCYCLE),
		cycleTime(this, IDC_CYCLETIMESPIN, 120, 1),
		status(this, IDC_PRIMARYSTATUS),
		sortByName(this, IDC_SORTBYNAME),
		sortByProto(this, IDC_SORTBYPROTO),
		sortByStatus(this, IDC_SORTBYSTATUS),
		chkOfflineBottom(this, IDC_MOVE_OFFLINE_BOTTOM)
	{
		chkCycle.OnChange = chkDontCycle.OnChange = chkMulti.OnChange = Callback(this, &ClistOptsDlg::onChange_Tray);
	}

	bool OnInitDialog() override
	{
		sortByName.SetState(!g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT) && !g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT));
		sortByProto.SetState(g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT));
		sortByStatus.SetState(g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT));
		chkOfflineBottom.SetState(g_plugin.getByte("NoOfflineBottom"));

		CheckDlgButton(m_hwnd, IDC_ALWAYSMULTI, !g_plugin.getByte("AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT));

		int iTrayIcon = g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT);
		chkCycle.SetState(iTrayIcon == SETTING_TRAYICON_CYCLE);
		chkMulti.SetState(iTrayIcon == SETTING_TRAYICON_MULTI);
		chkDontCycle.SetState(iTrayIcon == SETTING_TRAYICON_SINGLE);
		
		cycleTime.SetPosition(g_plugin.getWord("CycleTime", SETTING_CYCLETIME_DEFAULT));

		ptrA szPrimaryStatus(g_plugin.getStringA("PrimaryStatus"));

		status.AddString(TranslateT("Global"));
		status.SetCurSel(0);

		for (auto &pa : Accounts()) {
			if (!pa->IsEnabled() || CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
				continue;

			int item = status.AddString(pa->tszAccountName, (LPARAM)pa);
			if (!mir_strcmp(szPrimaryStatus, pa->szModuleName))
				status.SetCurSel(item);
		}
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("SortByStatus", g_bSortByStatus = sortByStatus.IsChecked());
		g_plugin.setByte("SortByProto", g_bSortByProto = sortByProto.IsChecked());
		g_plugin.setByte("OfflineBottom", g_bOfflineToBottom = chkOfflineBottom.IsChecked());
		g_plugin.setByte("AlwaysMulti", !IsDlgButtonChecked(m_hwnd, IDC_ALWAYSMULTI));
		g_plugin.setByte("TrayIcon", (chkDontCycle.IsChecked() ? SETTING_TRAYICON_SINGLE : (chkCycle.IsChecked() ? SETTING_TRAYICON_CYCLE : SETTING_TRAYICON_MULTI)));
		g_plugin.setWord("CycleTime", cycleTime.GetPosition());

		PROTOACCOUNT *pa = (PROTOACCOUNT *)status.GetCurData();
		if (pa == nullptr)
			g_plugin.delSetting("PrimaryStatus");
		else
			g_plugin.setString("PrimaryStatus", pa->szModuleName);

		Clist_TrayIconIconsChanged();
		Clist_LoadContactTree();
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);
		return true;
	}

	void onChange_Tray(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_PRIMARYSTATUS), chkDontCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CYCLETIME), chkCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CYCLETIMESPIN), chkCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_ALWAYSMULTI), chkMulti.IsChecked());
	}
};

/****************************************************************************************/

int CListOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = LPGEN("Contact list");
	odp.szTab.a = LPGEN("General");
	odp.pDialog = new ClistOptsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
