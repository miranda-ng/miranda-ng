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

static bool g_bDontShow, g_bStartup;

/////////////////////////////////////////////////////////////////////////////////////////

class CLoadSessionDlg : public CDlgBase
{
	CSession *pSession = nullptr;

	CTimer timerShow, timerLoad;
	CCtrlCombo m_list;
	CCtrlButton btnDelete;

public:
	CLoadSessionDlg() :
		CDlgBase(g_plugin, IDD_WLCMDIALOG),
		m_list(this, IDC_LIST),
		btnDelete(this, IDC_SESSDEL),
		timerLoad(this, TIMERID_LOAD),
		timerShow(this, TIMERID_SHOW)
	{
		btnDelete.OnClick = Callback(this, &CLoadSessionDlg::onClick_Delete);

		m_list.OnSelChanged = Callback(this, &CLoadSessionDlg::onSelChange_List);

		timerLoad.OnEvent = Callback(this, &CLoadSessionDlg::onTimer_Load);
		timerShow.OnEvent = Callback(this, &CLoadSessionDlg::onTimer_Show);
	}

	bool OnInitDialog() override
	{
		g_hDlg = m_hwnd;

		LoadSessionToCombobox(m_list, false);
		LoadSessionToCombobox(m_list, true);
		m_list.SetCurSel(0);
		pSession = (CSession *)m_list.GetItemData(0);

		int iDelay = g_plugin.getWord("StartupModeDelay", 1500);
		if (g_bDontShow == TRUE)
			timerLoad.Start(iDelay);
		else {
			LoadPosition(m_hwnd, "LoadDlg");
			if (g_bStartup)
				timerShow.Start(iDelay);
			else
				Show();
		}

		return true;
	}

	bool OnApply() override
	{
		if (!LoadSession(pSession))
			return true;

		return false;
	}

	void OnDestroy() override
	{
		SavePosition(m_hwnd, "LoadDlg");
		g_hDlg = nullptr;
	}

	void onTimer_Load(CTimer *pTimer)
	{
		pTimer->Stop();
		LoadSession(pSession);
		g_bDontShow = g_bStartup = false;
		Close();
	}

	void onTimer_Show(CTimer *pTimer)
	{
		pTimer->Stop();
		Show();
		g_bStartup = FALSE;
	}

	void onSelChange_List(CCtrlCombo *)
	{
		int index = m_list.GetCurSel();
		if (index != CB_ERR)
			pSession = (CSession*)m_list.GetItemData(index);
	}

	void onClick_Delete(CCtrlButton *)
	{
		if (pSession == nullptr) {
			if (session_list_recovered[0]) {
				for (int i = 0; session_list_recovered[i]; i++)
					g_plugin.setByte(session_list_recovered[i], "wasInLastSession", 0);
				memset(session_list_recovered, 0, sizeof(session_list_recovered));
			}
			g_bIncompletedSave = 0;
		}
		else pSession->remove();

		m_list.ResetContent();
		LoadSessionToCombobox(m_list, false);
		LoadSessionToCombobox(m_list, true);

		m_list.SetCurSel(0);
		btnDelete.Enable(m_list.GetCount() != 0);
	}
};

INT_PTR OpenSessionsManagerWindow(WPARAM, LPARAM)
{
	if (g_hDlg) {
		ShowWindow(g_hDlg, SW_SHOW);
		return 0;
	}

	if (g_bIncompletedSave || g_arDateSessions.getCount() || g_arUserSessions.getCount()) {
		(new CLoadSessionDlg())->Create();
		return 0;
	}
	
	if (g_bOtherWarnings)
		MessageBox(nullptr, TranslateT("No sessions to open"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
	return 1;
}

void CALLBACK LaunchSessions()
{
	int startup = g_plugin.getByte("StartupMode", 3);
	if (startup == 1 || (startup == 3 && g_bLastSessionPresent)) {
		g_bStartup = true;
		(new CLoadSessionDlg())->Create();
	}
	else if (startup == 2 && g_bLastSessionPresent) {
		g_bDontShow = true;
		(new CLoadSessionDlg())->Create();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LoadLastSession(WPARAM, LPARAM)
{
	int cnt = g_arDateSessions.getCount();
	if (g_bLastSessionPresent && cnt)
		return LoadSession(&g_arDateSessions[cnt-1]);
	
	if (g_bOtherWarnings)
		MessageBox(nullptr, TranslateT("Last Sessions is empty"), TranslateT("Sessions Manager"), MB_OK);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load session dialog

int LoadSession(CSession *pSession)
{
	int dup = 0;
	int hidden[255] = { '0' };

	MCONTACT session_list_t[255] = {};
	if (session_list_recovered[0] && pSession == nullptr)
		memcpy(session_list_t, session_list_recovered, sizeof(session_list_t));
	else {
		int i = 0;
		for (auto &cc : pSession->contacts)
			session_list_t[i++] = cc;
	}

	int i = 0, j = 0;
	// TODO: change to "switch"
	while (session_list_t[i] != 0) {
		if (CheckForDuplicate(session_list, session_list_t[i]) == -1)
			Clist_ContactDoubleClicked(session_list_t[i]);
		else if (g_bWarnOnHidden) {
			if (!CheckContactVisibility(session_list_t[i])) {
				hidden[j] = i + 1;
				j++;
			}
			dup++;
		}
		i++;
	}

	if (i == 0) {
		if (g_bOtherWarnings)
			MessageBox(nullptr, TranslateT("No contacts to open"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
		return 1;
	}

	if (dup == i) {
		if (!hidden[i]) {
			if (g_bOtherWarnings)
				MessageBox(nullptr, TranslateT("This Session already opened"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
			return 1;
		}
		if (!g_bWarnOnHidden && g_bOtherWarnings) {
			MessageBox(nullptr, TranslateT("This Session already opened"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
			return 1;
		}
		if (g_bWarnOnHidden) {
			if (IDYES == MessageBox(nullptr, TranslateT("This Session already opened (but probably hidden).\nDo you want to show hidden contacts?"), TranslateT("Sessions Manager"), MB_YESNO | MB_ICONQUESTION))
				for (j = 0; hidden[j] != 0; j++)
					Clist_ContactDoubleClicked(session_list_t[hidden[j] - 1]);
		}
	}

	return 0;
}
