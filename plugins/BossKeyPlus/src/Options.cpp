/*
	BossKey - Hide Miranda from your boss :)
	Copyright (C) 2002-2003 Goblineye Entertainment, (C) 2007-2010 Billy_Bons

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/

#include "stdafx.h"

bool g_fOptionsOpen;
bool g_fReqRass;

class COptMainDlg : public CDlgBase
{
	CCtrlHyperlink m_hlnkHotKey;
	CCtrlCheck m_chkChangeStatus, m_chkOnlineOnly, m_chkUsrDefMsg, m_chkDisSounds, m_chkTrayIcon, m_chkReqPass, m_chkSetOnlineBack;
	CCtrlCombo m_cbStatusList;
	CCtrlEdit m_edtStatMsg, m_edtPass;
	CCtrlMButton m_mbtnVarHelp;

	void ComboboxSelState()
	{
		uint8_t bSelection = (uint8_t)m_cbStatusList.GetCurSel();
		uint16_t wMode = STATUS_ARR_TO_ID[bSelection];
		if (m_chkUsrDefMsg.GetState()) {
			wchar_t *ptszDefMsg = GetDefStatusMsg(wMode, nullptr);
			m_edtStatMsg.SetText(ptszDefMsg);
			if (ptszDefMsg)
				mir_free(ptszDefMsg);
		}
		else {
			DBVARIANT dbVar;
			m_edtStatMsg.SendMsg(EM_LIMITTEXT, 1024, 0);
			if (!g_plugin.getWString("statmsg", &dbVar)) {
				m_edtStatMsg.SetText(dbVar.pwszVal);
				db_free(&dbVar);
			}
		}
		m_edtStatMsg.Enable(m_chkChangeStatus.GetState() && !m_chkUsrDefMsg.GetState());
		m_mbtnVarHelp.Show(m_edtStatMsg.Enabled() && ServiceExists(MS_VARS_FORMATSTRING));
	}

	void EnableWindowChecks()
	{
		bool fEnable = m_chkChangeStatus.GetState();
		m_cbStatusList.Enable(fEnable);
		m_chkSetOnlineBack.Enable(fEnable);
		m_chkOnlineOnly.Enable(fEnable);
		m_chkUsrDefMsg.Enable(fEnable);
		m_edtPass.Enable(m_chkReqPass.GetState());
	}

public:
	COptMainDlg() :
		CDlgBase(g_plugin, IDD_OPTDIALOGMAIN),
		m_hlnkHotKey(this, IDC_MAINOPT_LNK_HOTKEY),
		m_chkChangeStatus(this, IDC_MAINOPT_CHANGESTATUSBOX),
		m_cbStatusList(this, IDC_MAINOPT_CHGSTS),
		m_chkOnlineOnly(this, IDC_MAINOPT_ONLINEONLY),
		m_chkUsrDefMsg(this, IDC_MAINOPT_USEDEFMSG),
		m_edtStatMsg(this, IDC_MAINOPT_STATMSG),
		m_mbtnVarHelp(this, IDC_MAINOPT_VARHELP, 0, ""),
		m_chkDisSounds(this, IDC_MAINOPT_DISABLESNDS),
		m_chkTrayIcon(this, IDC_MAINOPT_TRAYICON),
		m_chkReqPass(this, IDC_MAINOPT_REQPASS),
		m_edtPass(this, IDC_MAINOPT_PASS),
		m_chkSetOnlineBack(this, IDC_MAINOPT_SETONLINEBACK)
	{
		m_hlnkHotKey.OnClick = Callback(this, &COptMainDlg::OnHotKeyClick);
		m_cbStatusList.OnChange = Callback(this, &COptMainDlg::OnStatusListChange);
		m_chkUsrDefMsg.OnChange = Callback(this, &COptMainDlg::OnUsrDefMsgChange);
		m_chkChangeStatus.OnChange = Callback(this, &COptMainDlg::OnChangeStatusChange);
		m_chkReqPass.OnChange = Callback(this, &COptMainDlg::OnReqPassChange);
		m_mbtnVarHelp.OnClick = Callback(this, &COptMainDlg::OnVarHelpClick);
	}

	bool OnInitDialog() override
	{
		g_fOptionsOpen = true;

		// set icon and tooltip for variables help button
		variables_skin_helpbutton(m_hwnd, IDC_MAINOPT_VARHELP);

		m_edtPass.SendMsg(EM_LIMITTEXT, MAXPASSLEN, 0);

		DBVARIANT dbVar;

		if (!g_plugin.getString("password", &dbVar)) {
			m_edtPass.SetTextA(dbVar.pszVal);
			db_free(&dbVar);
		}

		m_chkSetOnlineBack.SetState(g_wMask & OPT_SETONLINEBACK);
		m_chkChangeStatus.SetState(g_wMask & OPT_CHANGESTATUS);
		m_chkDisSounds.SetState(g_wMask & OPT_DISABLESNDS);
		m_chkReqPass.SetState(g_wMask & OPT_REQPASS);
		m_chkOnlineOnly.SetState(g_wMask & OPT_ONLINEONLY);
		m_chkUsrDefMsg.SetState(g_wMask & OPT_USEDEFMSG);
		m_chkTrayIcon.SetState(g_wMask & OPT_TRAYICON);

		for (int i = ID_STATUS_OFFLINE; i <= ID_STATUS_MAX; i++)
			m_cbStatusList.InsertString(Clist_GetStatusModeDescription(i, 0), -1);

		m_cbStatusList.SetCurSel(g_plugin.getByte("stattype", 2));

		EnableWindowChecks();
		ComboboxSelState();

		return true;
	}

	void OnHotKeyClick(CCtrlHyperlink*)
	{
		g_plugin.openOptions(L"Customize", L"Hotkeys");
	}

	bool OnApply() override
	{
		uint16_t wMask = 0;
		// we apply changes here
		// this plugin ain't that big, no need for a seperate routine

		// write down status type
		if (m_chkChangeStatus.GetState()) {
			g_plugin.setByte("stattype", (uint8_t)m_cbStatusList.GetCurSel());

			// status msg, if needed
			if (m_edtStatMsg.Enabled()) { // meaning we should save it
				wchar_t tszMsg[1025];
				m_edtStatMsg.GetText(tszMsg, _countof(tszMsg));
				if (tszMsg[0] != 0)
					g_plugin.setWString("statmsg", tszMsg);
				else // delete current setting
					g_plugin.delSetting("statmsg");
			}
			wMask |= OPT_CHANGESTATUS;
		}

		// checkbox
		if (m_chkReqPass.GetState()) {
			char szPass[MAXPASSLEN + 1];
			m_edtPass.GetTextA(szPass, _countof(szPass));
			if (szPass[0] != 0) {
				g_plugin.setString("password", szPass);
				wMask |= OPT_REQPASS;
			}
		}
		if (m_chkSetOnlineBack.GetState())
			wMask |= OPT_SETONLINEBACK;
		if (m_chkDisSounds.GetState())
			wMask |= OPT_DISABLESNDS;
		if (m_chkOnlineOnly.GetState())
			wMask |= OPT_ONLINEONLY;
		if (m_chkUsrDefMsg.GetState())
			wMask |= OPT_USEDEFMSG;
		if (m_chkTrayIcon.GetState())
			wMask |= OPT_TRAYICON;

		g_plugin.setWord("optsmask", wMask);
		g_wMask = wMask;
		return true;
	}

	void OnStatusListChange(CCtrlBase*)
	{
		ComboboxSelState();
	}

	void OnUsrDefMsgChange(CCtrlBase*)
	{
		ComboboxSelState();
	}

	void OnChangeStatusChange(CCtrlBase*)
	{
		EnableWindowChecks();
		ComboboxSelState();
	}

	void OnReqPassChange(CCtrlBase*)
	{
		EnableWindowChecks();
	}

	void OnVarHelpClick(CCtrlBase*)
	{
		variables_showhelp(m_hwnd, IDC_MAINOPT_STATMSG, VHF_INPUT | VHF_HELP, nullptr, nullptr);
	}

	void OnDestroy() override
	{
		g_fOptionsOpen = false;
	}
};

class COptAdvDlg : public CDlgBase
{
	CCtrlCheck m_chkHideIfLock, m_chkHideIfScrSvr, m_chkHideIfWinIdle, m_chkHideIfMirIdle, m_chkHideOnStart, m_chkRestore;
	CCtrlEdit m_edtTime;
	CCtrlSpin m_spnTime;

public:
	COptAdvDlg() :
		CDlgBase(g_plugin, IDD_OPTDIALOGADV),
		m_chkHideIfLock(this, IDC_MAINOPT_HIDEIFLOCK),
		m_chkHideIfScrSvr(this, IDC_MAINOPT_HIDEIFSCRSVR),
		m_chkHideIfWinIdle(this, IDC_MAINOPT_HIDEIFWINIDLE),
		m_chkHideIfMirIdle(this, IDC_MAINOPT_HIDEIFMIRIDLE),
		m_edtTime(this, IDC_MAINOPT_TIME),
		m_spnTime(this, IDC_MAINOPT_SPIN_TIME, 99, 1),
		m_chkHideOnStart(this, IDC_MAINOPT_HIDEONSTART),
		m_chkRestore(this, IDC_MAINOPT_RESTORE)
	{
		m_chkHideIfWinIdle.OnChange = Callback(this, &COptAdvDlg::OnHideIfWinIdleChange);
		m_chkHideIfMirIdle.OnChange = Callback(this, &COptAdvDlg::OnHideIfMirIdleChange);
	}

	bool OnInitDialog() override
	{
		g_fOptionsOpen = true;

		m_edtTime.SendMsg(EM_LIMITTEXT, 2, 0);
		m_edtTime.SetInt(g_plugin.getByte("time", 10));
		m_chkHideIfLock.SetState(g_wMaskAdv & OPT_HIDEIFLOCK);
		m_chkHideIfWinIdle.SetState(g_wMaskAdv & OPT_HIDEIFWINIDLE);
		m_chkHideIfMirIdle.SetState(g_wMaskAdv & OPT_HIDEIFMIRIDLE);
		m_chkHideIfScrSvr.SetState(g_wMaskAdv & OPT_HIDEIFSCRSVR);
		m_chkHideOnStart.SetState(g_wMaskAdv & OPT_HIDEONSTART);
		m_chkRestore.SetState(g_wMaskAdv & OPT_RESTORE);
		return true;
	}

	bool OnApply() override
	{
		uint16_t wMaskAdv = 0;
		if (m_chkHideIfLock.GetState())
			wMaskAdv |= OPT_HIDEIFLOCK;
		if (m_chkHideIfWinIdle.GetState())
			wMaskAdv |= OPT_HIDEIFWINIDLE;
		if (m_chkHideIfMirIdle.GetState())
			wMaskAdv |= OPT_HIDEIFMIRIDLE;
		if (m_chkHideIfScrSvr.GetState())
			wMaskAdv |= OPT_HIDEIFSCRSVR;
		if (m_chkHideOnStart.GetState())
			wMaskAdv |= OPT_HIDEONSTART;
		if (m_chkRestore.GetState())
			wMaskAdv |= OPT_RESTORE;

		wchar_t szMinutes[4] = { 0 };
		m_edtTime.GetText(szMinutes, _countof(szMinutes));
		UINT minutes = _wtoi(szMinutes);
		if (minutes < 1)
			minutes = 1;
		g_plugin.setByte("time", minutes);
		g_plugin.setWord("optsmaskadv", wMaskAdv);
		g_wMaskAdv = wMaskAdv;
		return true;
	}

	void OnHideIfWinIdleChange(CCtrlBase*)
	{
		if (m_chkHideIfWinIdle.GetState())
			m_chkHideIfMirIdle.SetState(0);
	}

	void OnHideIfMirIdleChange(CCtrlBase*)
	{
		if (m_chkHideIfMirIdle.GetState())
			m_chkHideIfWinIdle.SetState(0);
	}
	
	void OnDestroy() override
	{
		g_fOptionsOpen = false;
	}
};

int OptsDlgInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE optDi = {};
	optDi.flags = ODPF_BOLDGROUPS;
	optDi.szTitle.a = LPGEN("BossKey");
	optDi.szGroup.a = LPGEN("Events");

	optDi.szTab.a = LPGEN("Main");
	optDi.pDialog = new COptMainDlg();
	g_plugin.addOptions(wParam, &optDi);

	optDi.szTab.a = LPGEN("Advanced");
	optDi.pDialog = new COptAdvDlg();
	g_plugin.addOptions(wParam, &optDi);
	return 0;
}
