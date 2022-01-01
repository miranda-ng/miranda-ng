/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
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
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// global/local message log options
// local (per user) template overrides
// view mode (ieview/default)
// text formatting

#include "stdafx.h"

#define UPREF_ACTION_APPLYOPTIONS 1
#define UPREF_ACTION_REMAKELOG 2
#define UPREF_ACTION_SWITCHLOGVIEWER 4

class CUserPrefsOptDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

	MCONTACT m_hContact;

	CCtrlCombo cmbPanel, cmbAvatar, cmbTextFormat;
	CCtrlCheck chkAlwaysTrim;

public:
	CUserPrefsOptDlg(MCONTACT hContact) :
		CSuper(g_plugin, IDD_USERPREFS),
		m_hContact(hContact),
		cmbPanel(this, IDC_INFOPANEL),
		cmbAvatar(this, IDC_SHOWAVATAR),
		cmbTextFormat(this, IDC_TEXTFORMATTING),
		chkAlwaysTrim(this, IDC_ALWAYSTRIM2)
	{
		chkAlwaysTrim.OnChange = Callback(this, &CUserPrefsOptDlg::onChange_Trim);
	}

	bool OnInitDialog() override
	{
		uint32_t maxhist = M.GetDword(m_hContact, "maxhist", 0);
		int iLocalFormat = M.GetDword(m_hContact, "sendformat", 0);
		uint8_t bSplit = M.GetByte(m_hContact, "splitoverride", 0);
		uint8_t bInfoPanel = M.GetByte(m_hContact, "infopanel", 0);
		uint8_t bAvatarVisible = M.GetByte(m_hContact, "hideavatar", -1);

		cmbPanel.AddString(TranslateT("Use global setting"));
		cmbPanel.AddString(TranslateT("Always on"));
		cmbPanel.AddString(TranslateT("Always off"));
		cmbPanel.SetCurSel(bInfoPanel);

		cmbAvatar.AddString(TranslateT("Use global setting"));
		cmbAvatar.AddString(TranslateT("Show always (if present)"));
		cmbAvatar.AddString(TranslateT("Never show it at all"));
		cmbAvatar.SetCurSel(bAvatarVisible == 0xff ? 0 : (bAvatarVisible == 1 ? 1 : 2));

		cmbTextFormat.AddString(TranslateT("Use global setting"));
		cmbTextFormat.AddString(TranslateT("BBCode"));
		cmbTextFormat.AddString(TranslateT("Force off"));
		cmbTextFormat.SetCurSel(iLocalFormat == 0 ? 0 : (iLocalFormat == -1 ? 2 : 1));

		CheckDlgButton(m_hwnd, IDC_PRIVATESPLITTER, bSplit);
		CheckDlgButton(m_hwnd, IDC_TEMPLOVERRIDE, db_get_b(m_hContact, TEMPLATES_MODULE, "enabled", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_RTLTEMPLOVERRIDE, db_get_b(m_hContact, RTLTEMPLATES_MODULE, "enabled", 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_LOADONLYACTUAL, M.GetByte(m_hContact, "ActualHistory", 0) ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_TRIMSPIN, UDM_SETRANGE, 0, MAKELONG(1000, 5));
		SendDlgItemMessage(m_hwnd, IDC_TRIMSPIN, UDM_SETPOS, 0, maxhist);
		Utils::enableDlgControl(m_hwnd, IDC_TRIMSPIN, maxhist != 0);
		Utils::enableDlgControl(m_hwnd, IDC_TRIM, maxhist != 0);
		chkAlwaysTrim.SetState(maxhist != 0);

		CheckDlgButton(m_hwnd, IDC_IGNORETIMEOUTS, M.GetByte(m_hContact, "no_ack", 0) ? BST_CHECKED : BST_UNCHECKED);

		ShowWindow(m_hwnd, SW_SHOW);
		return true;
	}

	bool OnApply() override
	{
		CMsgDialog *dat = Srmm_FindDialog(m_hContact);
		uint8_t bOldInfoPanel = M.GetByte(m_hContact, "infopanel", 0);

		int iIndex = SendDlgItemMessage(m_hwnd, IDC_TEXTFORMATTING, CB_GETCURSEL, 0, 0);
		if (iIndex != CB_ERR) {
			if (iIndex == 0)
				db_unset(m_hContact, SRMSGMOD_T, "sendformat");
			else
				db_set_dw(m_hContact, SRMSGMOD_T, "sendformat", iIndex == 2 ? -1 : 1);
		}

		db_set_b(m_hContact, SRMSGMOD_T, "splitoverride", (uint8_t)(IsDlgButtonChecked(m_hwnd, IDC_PRIVATESPLITTER) ? 1 : 0));

		db_set_b(m_hContact, TEMPLATES_MODULE, "enabled", (uint8_t)(IsDlgButtonChecked(m_hwnd, IDC_TEMPLOVERRIDE)));
		db_set_b(m_hContact, RTLTEMPLATES_MODULE, "enabled", (uint8_t)(IsDlgButtonChecked(m_hwnd, IDC_RTLTEMPLOVERRIDE)));

		uint8_t bAvatarVisible = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_SHOWAVATAR, CB_GETCURSEL, 0, 0);
		if (bAvatarVisible == 0)
			db_unset(m_hContact, SRMSGMOD_T, "hideavatar");
		else
			db_set_b(m_hContact, SRMSGMOD_T, "hideavatar", (uint8_t)(bAvatarVisible == 1 ? 1 : 0));

		uint8_t bInfoPanel = (uint8_t)SendDlgItemMessage(m_hwnd, IDC_INFOPANEL, CB_GETCURSEL, 0, 0);
		if (bInfoPanel != bOldInfoPanel) {
			db_set_b(m_hContact, SRMSGMOD_T, "infopanel", (uint8_t)(bInfoPanel == 0 ? 0 : (bInfoPanel == 1 ? 1 : -1)));
			if (dat)
				SendMessage(dat->GetHwnd(), DM_SETINFOPANEL, 0, 0);
		}
		if (chkAlwaysTrim.GetState())
			db_set_dw(m_hContact, SRMSGMOD_T, "maxhist", (uint32_t)SendDlgItemMessage(m_hwnd, IDC_TRIMSPIN, UDM_GETPOS, 0, 0));
		else
			db_set_dw(m_hContact, SRMSGMOD_T, "maxhist", 0);

		if (IsDlgButtonChecked(m_hwnd, IDC_LOADONLYACTUAL)) {
			db_set_b(m_hContact, SRMSGMOD_T, "ActualHistory", 1);
			if (dat)
				dat->m_bActualHistory = true;
		}
		else {
			db_set_b(m_hContact, SRMSGMOD_T, "ActualHistory", 0);
			if (dat)
				dat->m_bActualHistory = false;
		}

		if (IsDlgButtonChecked(m_hwnd, IDC_IGNORETIMEOUTS)) {
			db_set_b(m_hContact, SRMSGMOD_T, "no_ack", 1);
			if (dat)
				dat->m_sendMode |= SMODE_NOACK;
		}
		else {
			db_unset(m_hContact, SRMSGMOD_T, "no_ack");
			if (dat)
				dat->m_sendMode &= ~SMODE_NOACK;
		}
		if (dat) {
			SendMessage(dat->GetHwnd(), DM_CONFIGURETOOLBAR, 0, 1);
			dat->ShowPicture(false);
			dat->Resize();
			dat->DM_ScrollToBottom(0, 1);
		}
		return true;
	}

	void onChange_Trim(CCtrlCheck *)
	{
		bool bChecked = chkAlwaysTrim.GetState();
		Utils::enableDlgControl(m_hwnd, IDC_TRIMSPIN, bChecked);
		Utils::enableDlgControl(m_hwnd, IDC_TRIM, bChecked);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// loads message log and other "per contact" flags
// it uses the global flag value (0, mwflags) and then merges per contact settings
// based on the mask value.
//
// ALWAYS mask dat->dwFlags with MWF_LOG_ALL to only affect real flag bits and
// ignore temporary bits.

static struct _checkboxes
{
	UINT	uId;
	UINT	uFlag;
}
checkboxes[] = {
	IDC_UPREFS_GRID, MWF_LOG_GRID,
	IDC_UPREFS_SHOWICONS, MWF_LOG_SHOWICONS,
	IDC_UPREFS_SHOWSYMBOLS, MWF_LOG_SYMBOLS,
	IDC_UPREFS_INOUTICONS, MWF_LOG_INOUTICONS,
	IDC_UPREFS_SHOWTIMESTAMP, MWF_LOG_SHOWTIME,
	IDC_UPREFS_SHOWDATES, MWF_LOG_SHOWDATES,
	IDC_UPREFS_SHOWSECONDS, MWF_LOG_SHOWSECONDS,
	IDC_UPREFS_LOCALTIME, MWF_LOG_LOCALTIME,
	IDC_UPREFS_INDENT, MWF_LOG_INDENT,
	IDC_UPREFS_GROUPING, MWF_LOG_GROUPMODE,
	IDC_UPREFS_BBCODE, MWF_LOG_BBCODE,
	IDC_UPREFS_RTL, MWF_LOG_RTL,
	IDC_UPREFS_NORMALTEMPLATES, MWF_LOG_NORMALTEMPLATES,
	0, 0
};

int CMsgDialog::LoadLocalFlags()
{
	uint32_t	dwMask = M.GetDword(m_hContact, "mwmask", 0);
	uint32_t	dwLocal = M.GetDword(m_hContact, "mwflags", 0);
	uint32_t	dwGlobal = M.GetDword("mwflags", MWF_LOG_DEFAULT);

	m_dwFlags &= ~MWF_LOG_ALL;
	if (m_pContainer->m_theme.isPrivate)
		m_dwFlags |= (m_pContainer->m_theme.dwFlags & MWF_LOG_ALL);
	else
		m_dwFlags |= (dwGlobal & MWF_LOG_ALL);

	for (int i = 0; checkboxes[i].uId; i++) {
		uint32_t	maskval = checkboxes[i].uFlag;
		if (dwMask & maskval)
			m_dwFlags = (dwLocal & maskval) ? m_dwFlags | maskval : m_dwFlags & ~maskval;
	}

	return m_dwFlags & MWF_LOG_ALL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the user preferences dialog 
// (Second page, "per contact" message log options)

class CUserPrefsLogDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

	MCONTACT m_hContact;

	CCtrlButton btnReset;

public:
	CUserPrefsLogDlg(MCONTACT hContact) :
		CSuper(g_plugin, IDD_USERPREFS1),
		m_hContact(hContact),
		btnReset(this, IDC_REVERTGLOBAL)
	{
		btnReset.OnClick = Callback(this, &CUserPrefsLogDlg::onClick_Reset);
	}

	bool OnInitDialog() override
	{
		uint32_t	dwLocalFlags, dwLocalMask, maskval;

		dwLocalFlags = M.GetDword(m_hContact, "mwflags", 0);
		dwLocalMask = M.GetDword(m_hContact, "mwmask", 0);

		int i = 0;
		while (checkboxes[i].uId) {
			maskval = checkboxes[i].uFlag;

			if (dwLocalMask & maskval)
				CheckDlgButton(m_hwnd, checkboxes[i].uId, (dwLocalFlags & maskval) ? BST_CHECKED : BST_UNCHECKED);
			else
				CheckDlgButton(m_hwnd, checkboxes[i].uId, BST_INDETERMINATE);
			i++;
		}

		if (M.GetByte("logstatuschanges", 0) == M.GetByte(m_hContact, "logstatuschanges", 0))
			CheckDlgButton(m_hwnd, IDC_UPREFS_LOGSTATUS, BST_INDETERMINATE);
		else
			CheckDlgButton(m_hwnd, IDC_UPREFS_LOGSTATUS, M.GetByte(m_hContact, "logstatuschanges", 0) ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		uint32_t dwMask = 0, dwFlags = 0, maskval;

		int i = 0;
		while (checkboxes[i].uId) {
			maskval = checkboxes[i].uFlag;

			int state = IsDlgButtonChecked(m_hwnd, checkboxes[i].uId);
			if (state != BST_INDETERMINATE) {
				dwMask |= maskval;
				dwFlags = (state == BST_CHECKED) ? (dwFlags | maskval) : (dwFlags & ~maskval);
			}
			i++;
		}
		
		int state = IsDlgButtonChecked(m_hwnd, IDC_UPREFS_LOGSTATUS);
		if (state != BST_INDETERMINATE)
			db_set_b(m_hContact, SRMSGMOD_T, "logstatuschanges", (uint8_t)state);

		if (dwMask) {
			db_set_dw(m_hContact, SRMSGMOD_T, "mwmask", dwMask);
			db_set_dw(m_hContact, SRMSGMOD_T, "mwflags", dwFlags);
		}
		else {
			db_unset(m_hContact, SRMSGMOD_T, "mwmask");
			db_unset(m_hContact, SRMSGMOD_T, "mwflags");
		}
		return true;
	}

	void onClick_Reset(CCtrlButton *)
	{
		db_unset(m_hContact, SRMSGMOD_T, "mwmask");
		db_unset(m_hContact, SRMSGMOD_T, "mwflags");
		OnInitDialog();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the user preferences dialog

class CUserPrefsDlg : public CDlgBase
{
	typedef CDlgBase CSuper;

	MCONTACT m_hContact;

	CCtrlPages m_tab;

public:
	CUserPrefsDlg(MCONTACT hContact) :
		CSuper(g_plugin, IDD_USERPREFS_FRAME),
		m_hContact(hContact),
		m_tab(this, IDC_OPTIONSTAB)
	{
	}

	bool OnInitDialog() override
	{
		WindowList_Add(PluginConfig.hUserPrefsWindowList, m_hwnd, m_hContact);

		wchar_t szBuffer[180];
		mir_snwprintf(szBuffer, TranslateT("Set messaging options for %s"), Clist_GetContactDisplayName(m_hContact));
		SetCaption(szBuffer);

		m_tab.AddPage(TranslateT("General"), nullptr, new CUserPrefsOptDlg(m_hContact));
		m_tab.AddPage(TranslateT("Message Log"), nullptr, new CUserPrefsLogDlg(m_hContact));
		return true;
	}

	bool OnApply() override
	{
		CMsgDialog *dat = Srmm_FindDialog(m_hContact);
		if (dat) {
			uint32_t dwOldFlags = (dat->m_dwFlags & MWF_LOG_ALL);
			dat->SetDialogToType();
			dat->LoadLocalFlags();
			if ((dat->m_dwFlags & MWF_LOG_ALL) != dwOldFlags) {
				bool fShouldHide = true;
				if (IsIconic(dat->m_pContainer->m_hwnd))
					fShouldHide = false;
				else
					ShowWindow(dat->m_pContainer->m_hwnd, SW_HIDE);
				dat->DM_OptionsApplied();
				dat->RemakeLog();
				if (fShouldHide)
					ShowWindow(dat->m_pContainer->m_hwnd, SW_SHOWNORMAL);
			}
		}
		return true;
	}

	void OnDestroy() override
	{
		WindowList_Remove(PluginConfig.hUserPrefsWindowList, m_hwnd);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// service function. Invokes the user preferences dialog for the contact given in wParam

INT_PTR SetUserPrefs(WPARAM hContact, LPARAM)
{
	HWND hWnd = WindowList_Find(PluginConfig.hUserPrefsWindowList, hContact);
	if (hWnd)
		SetForegroundWindow(hWnd);			// already open, bring it to front
	else
		(new CUserPrefsDlg(hContact))->Show();
	return 0;
}
