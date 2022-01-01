/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

static int SaveUserSessionName(MCONTACT *contacts, wchar_t *szUSessionName)
{
	if (contacts[0] == 0)
		return 1;

	CSession *pSession = nullptr;
	for (auto &it : g_arUserSessions)
		if (!it->wszName.CompareNoCase(szUSessionName))
			pSession = it;

	if (pSession)
		pSession->contacts.clear();
	else {
		g_plugin.g_lastUserId = g_plugin.g_lastUserId + 1;

		pSession = new CSession();
		pSession->id = g_plugin.g_lastUserId;
		pSession->bIsUser = true;
		pSession->wszName = szUSessionName;
		g_arUserSessions.insert(pSession);
	}

	for (int i = 0; contacts[i]; i++)
		pSession->contacts.push_back(contacts[i]);

	pSession->save();

	while (g_arUserSessions.getCount() > g_ses_limit) {
		g_arUserSessions[0].remove();
		g_arUserSessions.remove(int(0));
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
		LoadSessionToCombobox(m_sessions, true);

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
		rtrimw(szUserSessionName);
		if (szUserSessionName[0] == 0) {
			MessageBox(nullptr, TranslateT("Session name is empty, enter the name and try again"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
			return false;
		}

		if (chkSelContacts.IsChecked() && bSC) {
			int i = 0;
			for (auto &hContact : Contacts()) {
				uint8_t res = m_clist.GetCheck(m_clist.FindContact(hContact));
				if (res) {
					user_session_list[i] = hContact;
					i++;
				}
			}

			SaveUserSessionName(user_session_list, szUserSessionName);
			return true;
		}

		if (!SaveUserSessionName(session_list, szUserSessionName)) {
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
