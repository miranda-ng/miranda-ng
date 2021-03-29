/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

HWND g_hSDlg;

bool bSC = false;

/////////////////////////////////////////////////////////////////////////////////////////

int SaveSessionDate()
{
	if (session_list[0] != 0) {
		int TimeSize = GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, nullptr, nullptr, nullptr, 0);
		wchar_t *szTimeBuf = (wchar_t*)mir_alloc((TimeSize + 1)*sizeof(wchar_t));

		GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, nullptr, nullptr, szTimeBuf, TimeSize);

		int DateSize = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, nullptr, 0);
		wchar_t *szDateBuf = (wchar_t*)mir_alloc((DateSize + 1)*sizeof(wchar_t));

		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, nullptr, nullptr, szDateBuf, DateSize);
		int lenn = (DateSize + TimeSize + 5);
		wchar_t *szSessionTime = (wchar_t*)mir_alloc(lenn*sizeof(wchar_t));
		mir_snwprintf(szSessionTime, lenn, L"%s - %s", szTimeBuf, szDateBuf);

		char szSetting[256];
		mir_snprintf(szSetting, "%s_%d", "SessionDate", 0);
		wchar_t *ptszSaveSessionDate = g_plugin.getWStringA(szSetting);

		g_plugin.setWString(szSetting, szSessionTime);
		mir_free(szSessionTime);

		if (ptszSaveSessionDate)
			ResaveSettings("SessionDate", 1, g_ses_limit, ptszSaveSessionDate);

		if (szTimeBuf)
			mir_free(szTimeBuf);
		if (szDateBuf)
			mir_free(szDateBuf);
	}
	
	if (g_bCrashRecovery)
		g_plugin.setByte("lastSaveCompleted", 1);
	return 0;
}

static int SaveUserSessionName(MCONTACT *pSession, wchar_t *szUSessionName)
{
	if (pSession[0] == 0)
		return 1;

	char szSetting[256];
	mir_snprintf(szSetting, "%s_%u", "UserSessionDsc", 0);
	wchar_t *ptszUserSessionName = g_plugin.getWStringA(szSetting);
	if (ptszUserSessionName)
		ResaveSettings("UserSessionDsc", 1, 255, ptszUserSessionName);

	g_plugin.setWString(szSetting, szUSessionName);
	return 0;
}

int SaveSessionHandles(MCONTACT *pSession, bool bNewSession)
{
	if (pSession[0] == 0)
		return 1;

	int k = 0;
	for (auto &hContact : Contacts()) {
		if ((k = CheckForDuplicate(pSession, hContact)) != -1 && !(g_bExclHidden && !CheckContactVisibility(hContact))) {
			AddSessionMark(hContact, bNewSession, '1');
			AddInSessionOrder(hContact, bNewSession, k, 1);
		}
		else {
			AddSessionMark(hContact, bNewSession, '0');
			AddInSessionOrder(hContact, bNewSession, 0, 0);
		}
	}
	if (bNewSession) {
		g_ses_count++;
		g_plugin.setByte("UserSessionsCount", (BYTE)g_ses_count);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Save session dialog

class CSessionDlg : public CDlgBase
{
	CCtrlClc m_clist;
	CCtrlCombo m_sessions;
	CCtrlCheck chkSelContacts, chkSaveAndClose;

	MCONTACT user_session_list[255];

public:
	CSessionDlg() : 
		CDlgBase(g_plugin, IDD_SAVEDIALOG),
		m_clist(this, IDC_CLIST),
		m_sessions(this, IDC_LIST),
		chkSelContacts(this, IDC_SELCONTACTS),
		chkSaveAndClose(this, IDC_SANDCCHECK)
	{
		memset(user_session_list, 0, sizeof(user_session_list));

		chkSelContacts.OnChange = Callback(this, &CSessionDlg::onChange_SelContacts);

		m_clist.OnCheckChanged = Callback(this, &CSessionDlg::onCheckChanged_Clist);
	}

	bool OnInitDialog() override
	{
		g_hSDlg = m_hwnd;
		LoadSessionToCombobox(m_hwnd, 1, 5, "UserSessionDsc", 0);

		SetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE,
			GetWindowLongPtr(m_clist.GetHwnd(), GWL_STYLE) | CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE | CLS_GROUPCHECKBOXES);
		m_clist.SetExStyle(CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT);
		m_clist.AutoRebuild();

		LoadPosition(m_hwnd, "SaveDlg");
		return true;
	}

	bool OnApply() override
	{
		wchar_t szUserSessionName[MAX_PATH];
		m_sessions.GetText(szUserSessionName, _countof(szUserSessionName));
		if (szUserSessionName[0] == 0) {
			MessageBox(nullptr, TranslateT("Session name is empty, enter the name and try again"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
			return false;
		}

		if (chkSelContacts.IsChecked() && bSC) {
			int i = 0;
			for (auto &hContact : Contacts()) {
				BYTE res = m_clist.GetCheck(m_clist.FindContact(hContact));
				if (res) {
					user_session_list[i] = hContact;
					i++;
				}
			}

			SaveSessionHandles(user_session_list, true);
			SaveUserSessionName(user_session_list, szUserSessionName);
			return true;
		}

		if (!SaveUserSessionName(session_list, szUserSessionName)) {
			SaveSessionHandles(session_list, true);

			if (chkSaveAndClose.IsChecked())
				CloseCurrentSession(0, 0);
			return true;
		}

		MessageBox(nullptr, TranslateT("Current session is empty!"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
		return false;
	}

	void OnDestroy() override
	{
		SavePosition(m_hwnd, "SaveDlg");
		g_hSDlg = nullptr;
	}

	void onCheckChanged_Clist(CCtrlClc*)
	{
		bSC = TRUE;
		memcpy(user_session_list, session_list, sizeof(user_session_list));
	}

	void onChange_SelContacts(CCtrlCheck *)
	{
		if (!m_bInitialized)
			return;

		RECT rWnd;
		GetWindowRect(m_hwnd, &rWnd);

		int x = rWnd.right - rWnd.left, y = rWnd.bottom - rWnd.top, dy, dx, dd;

		if (chkSelContacts.IsChecked()) {
			chkSaveAndClose.Disable();
			dy = 20;
			dx = 150;
			dd = 5;
			m_clist.Show();
		}
		else {
			chkSaveAndClose.Enable();
			dy = -20;
			dx = -150;
			dd = 5;
			m_clist.Hide();
		}

		SetWindowPos(m_hwnd, nullptr, rWnd.left, rWnd.top, x + dx, y + (dx / 3), SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE);
		SetWindowPos(m_clist.GetHwnd(), nullptr, x - dd, dd, dx - dd, y + (dx / 12), SWP_NOZORDER);

		for (int i = 0; session_list[i] > 0; i++)
			m_clist.SetCheck(m_clist.FindContact(session_list[i]), 1);

		OffsetWindow(m_hwnd, GetDlgItem(m_hwnd, IDC_LIST), 0, dy);
		OffsetWindow(m_hwnd, GetDlgItem(m_hwnd, IDC_STATIC), 0, dy);
		OffsetWindow(m_hwnd, GetDlgItem(m_hwnd, IDC_SANDCCHECK), 0, dy);
		OffsetWindow(m_hwnd, GetDlgItem(m_hwnd, IDOK), 0, dy);
		OffsetWindow(m_hwnd, GetDlgItem(m_hwnd, IDCANCEL), 0, dy);
	}
};

INT_PTR SaveUserSessionHandles(WPARAM, LPARAM)
{
	if (g_hSDlg) {
		ShowWindow(g_hSDlg, SW_SHOW);
		return 1;
	}

	(new CSessionDlg())->Show();
	return 0;
}
