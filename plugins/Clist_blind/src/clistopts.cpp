/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

class CGenOptsDlg : public CDlgBase
{
	CCtrlCheck chkBlink, chkCycle, chkDontCycle, chkMultiTray;

public:
	CGenOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_CLIST),
		chkBlink(this, IDC_DISABLEBLINK),
		chkCycle(this, IDC_CYCLE),
		chkDontCycle(this, IDC_DONTCYCLE),
		chkMultiTray(this, IDC_MULTITRAY)
	{
		chkBlink.OnChange = Callback(this, &CGenOptsDlg::onChange_Blink);
		chkCycle.OnChange = chkDontCycle.OnChange = chkMultiTray.OnChange = Callback(this, &CGenOptsDlg::onChange_Cycle);
	}

	bool OnInitDialog() override
	{
		CheckDlgButton(m_hwnd, IDC_SORTBYNAME, !g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT) && !g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SORTBYSTATUS, g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_SORTBYPROTO, g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		int iTrayIcon = g_plugin.getByte("TrayIcon", SETTING_TRAYICON_DEFAULT);
		chkCycle.SetState(iTrayIcon == SETTING_TRAYICON_CYCLE);
		chkDontCycle.SetState(iTrayIcon == SETTING_TRAYICON_SINGLE);

		CheckDlgButton(m_hwnd, IDC_ALWAYSMULTI, !g_plugin.getByte("AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_DISABLEBLINK, g_plugin.getByte("DisableTrayFlash", 0) == 1 ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_SETRANGE, 0, MAKELONG(120, 1));
		SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("CycleTime", SETTING_CYCLETIME_DEFAULT), 0));

		ptrA szPrimaryStatus(g_plugin.getStringA("PrimaryStatus"));

		int item = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)TranslateT("Global"));
		SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)0);

		for (auto &pa : Accounts()) {
			if (!pa->IsEnabled() || CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
				continue;

			item = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_ADDSTRING, 0, (LPARAM)pa->tszAccountName);
			SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETITEMDATA, item, (LPARAM)pa);
			if (!mir_strcmp(szPrimaryStatus, pa->szModuleName))
				SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETCURSEL, item, 0);
		}

		if (CB_ERR == (int)SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0))
			SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_SETCURSEL, 0, 0);
		SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(m_hwnd, IDC_BLINKTIME), 0);   // set buddy
		SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_SETRANGE, 0, MAKELONG(0x3FFF, 250));
		SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_SETPOS, 0, MAKELONG(g_plugin.getWord("IconFlashTime", 550), 0));
		return true;
	}

	bool OnApply() override
	{
		int iTrayIcon = chkDontCycle.IsChecked() ? SETTING_TRAYICON_SINGLE : (chkCycle.IsChecked() ? SETTING_TRAYICON_CYCLE : SETTING_TRAYICON_MULTI);

		g_plugin.setByte("SortByStatus", (uint8_t)(g_bSortByStatus = IsDlgButtonChecked(m_hwnd, IDC_SORTBYSTATUS)));
		g_plugin.setByte("SortByProto", (uint8_t)(g_bSortByProto = IsDlgButtonChecked(m_hwnd, IDC_SORTBYPROTO)));
		g_plugin.setByte("AlwaysMulti", (uint8_t)!IsDlgButtonChecked(m_hwnd, IDC_ALWAYSMULTI));
		g_plugin.setByte("TrayIcon", iTrayIcon);
		g_plugin.setWord("CycleTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_CYCLETIMESPIN, UDM_GETPOS, 0, 0));
		g_plugin.setWord("IconFlashTime", (uint16_t)SendDlgItemMessage(m_hwnd, IDC_BLINKSPIN, UDM_GETPOS, 0, 0));
		g_plugin.setByte("DisableTrayFlash", (uint8_t)IsDlgButtonChecked(m_hwnd, IDC_DISABLEBLINK));

		int cur = SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETCURSEL, 0, 0);
		PROTOACCOUNT *pa = (PROTOACCOUNT *)SendDlgItemMessage(m_hwnd, IDC_PRIMARYSTATUS, CB_GETITEMDATA, cur, 0);
		if (pa == nullptr)
			g_plugin.delSetting("PrimaryStatus");
		else
			g_plugin.setString("PrimaryStatus", pa->szModuleName);

		Clist_TrayIconIconsChanged();
		Clist_LoadContactTree();  /* this won't do job properly since it only really works when changes happen */
		g_clistApi.pfnInvalidateDisplayNameCacheEntry(INVALID_CONTACT_ID);        /* force reshuffle */
		return true;
	}

	void onChange_Cycle(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_PRIMARYSTATUS), chkDontCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CYCLETIME), chkCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_CYCLETIMESPIN), chkCycle.IsChecked());
		EnableWindow(GetDlgItem(m_hwnd, IDC_ALWAYSMULTI), chkMultiTray.IsChecked());
	}

	void onChange_Blink(CCtrlCheck *pCheck)
	{
		bool bEnabled = !pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_BLINKTIME), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_BLINKSPIN), bEnabled);
		EnableWindow(GetDlgItem(m_hwnd, IDC_STMSDELAY), bEnabled);
	}
};

/****************************************************************************************/

int CListOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = -1000000000;
	odp.szTitle.a = LPGEN("Contact list");
	odp.pDialog = new CGenOptsDlg();
	odp.flags = ODPF_BOLDGROUPS;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
