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

bool g_hghostw, g_bStartup;

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK LoadSessionDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM)
{
	static int ses_count;

	g_hDlg = hdlg;
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			int iDelay = g_plugin.getWord("StartupModeDelay", 1500);
			if (g_hghostw == TRUE)
				SetTimer(hdlg, TIMERID_LOAD, iDelay, nullptr);
			else {
				if ((ses_count = LoadSessionToCombobox(hdlg, 0, g_ses_limit, "SessionDate", 0)) == g_ses_limit)
					EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), TRUE);

				if (LoadSessionToCombobox(hdlg, 0, 255, "UserSessionDsc", g_ses_limit) == 0 && ses_count != 0)
					ses_count = 0;

				if (session_list_recovered[0])
					ses_count = 256;

				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);
				LoadPosition(hdlg, "LoadDlg");
				if (g_bStartup)
					SetTimer(hdlg, TIMERID_SHOW, iDelay, nullptr);
				else
					ShowWindow(g_hDlg, SW_SHOW);
			}
		}
		break;

	case WM_TIMER:
		if (wparam == TIMERID_SHOW) {
			KillTimer(hdlg, TIMERID_SHOW);
			ShowWindow(hdlg, SW_SHOW);
			g_bStartup = FALSE;
		}
		else {
			KillTimer(hdlg, TIMERID_LOAD);
			LoadSession(0, g_bIncompletedSave ? 256 : 0);
			g_hghostw = g_bStartup = FALSE;
			DestroyWindow(hdlg);
			g_hDlg = nullptr;
		}
		break;

	case WM_CLOSE:
		SavePosition(hdlg, "LoadDlg");
		DestroyWindow(hdlg);
		g_hDlg = nullptr;
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam)) {
		case IDC_LIST:
			switch (HIWORD(wparam)) {
			case CBN_SELCHANGE:
				HWND hCombo = GetDlgItem(hdlg, IDC_LIST);
				int index = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
				if (index != CB_ERR)
					ses_count = SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)index, 0);
			}
			break;

		case IDC_SESSDEL:
			if (session_list_recovered[0] && ses_count == 256) {
				for (int i = 0; session_list_recovered[i]; i++)
					g_plugin.setByte(session_list_recovered[i], "wasInLastSession", 0);

				memset(session_list_recovered, 0, sizeof(session_list_recovered));
				g_bIncompletedSave = 0;

				EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), FALSE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

				if ((ses_count = LoadSessionToCombobox(hdlg, 1, g_ses_limit, "SessionDate", 0)) == g_ses_limit)
					EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), TRUE);

				if (LoadSessionToCombobox(hdlg, 1, 255, "UserSessionDsc", g_ses_limit) == 0 && ses_count != 0)
					ses_count = 0;

				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);

			}
			else if (ses_count >= g_ses_limit) {
				ses_count -= g_ses_limit;
				DelUserDefSession(ses_count);
				EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), FALSE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);

				if ((ses_count = LoadSessionToCombobox(hdlg, 0, g_ses_limit, "SessionDate", 0)) == g_ses_limit)
					EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), TRUE);

				if (LoadSessionToCombobox(hdlg, 0, 255, "UserSessionDsc", g_ses_limit) == 0 && ses_count != 0)
					ses_count = 0;

				if (session_list_recovered[0])
					ses_count = 256;

				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);
			}
			else {
				DeleteAutoSession(ses_count);
				EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), FALSE);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_RESETCONTENT, 0, 0);
				if ((ses_count = LoadSessionToCombobox(hdlg, 0, g_ses_limit, "SessionDate", 0)) == g_ses_limit)
					EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), TRUE);

				if (LoadSessionToCombobox(hdlg, 0, 255, "UserSessionDsc", g_ses_limit) == 0 && ses_count != 0)
					ses_count = 0;

				if (session_list_recovered[0])
					ses_count = 256;

				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETCURSEL, 0, 0);
			}
			if (SendDlgItemMessage(hdlg, IDC_LIST, CB_GETCOUNT, 0, 0))
				EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), TRUE);
			else
				EnableWindow(GetDlgItem(hdlg, IDC_SESSDEL), FALSE);
			break;

		case IDOK:
			if (!LoadSession(0, ses_count)) {
				SavePosition(hdlg, "LoadDlg");
				DestroyWindow(hdlg);
				g_hDlg = nullptr;
			}
			break;

		case IDCANCEL:
			SavePosition(hdlg, "LoadDlg");
			DestroyWindow(hdlg);
			g_hDlg = nullptr;
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR OpenSessionsManagerWindow(WPARAM, LPARAM)
{
	if (g_hDlg) {
		ShowWindow(g_hDlg, SW_SHOW);
		return 0;
	}

	ptrW tszSession(g_plugin.getWStringA("SessionDate_0")), tszUserSession(g_plugin.getWStringA("UserSessionDsc_0"));
	if (g_bIncompletedSave || tszSession || tszUserSession) {
		g_hDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WLCMDIALOG), nullptr, LoadSessionDlgProc);
		return 0;
	}
	
	if (g_bOtherWarnings)
		MessageBox(nullptr, TranslateT("No sessions to open"), TranslateT("Sessions Manager"), MB_OK | MB_ICONWARNING);
	return 1;
}

void CALLBACK LaunchSessions()
{
	int startup = g_plugin.getByte("StartupMode", 3);
	if (startup == 1 || (startup == 3 && isLastTRUE == TRUE)) {
		g_bStartup = TRUE;
		g_hDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WLCMDIALOG), nullptr, LoadSessionDlgProc);
	}
	else if (startup == 2 && isLastTRUE == TRUE) {
		g_hghostw = TRUE;
		g_hDlg = CreateDialog(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WLCMDIALOG), nullptr, LoadSessionDlgProc);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR LoadLastSession(WPARAM wparam, LPARAM lparam)
{
	if (isLastTRUE)
		return LoadSession(wparam, lparam);
	if (g_bOtherWarnings)
		MessageBox(nullptr, TranslateT("Last Sessions is empty"), TranslateT("Sessions Manager"), MB_OK);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load session dialog

int LoadSession(WPARAM, LPARAM lparam)
{
	int dup = 0;
	int hidden[255] = { '0' };
	MCONTACT session_list_t[255] = { 0 };
	int mode = 0;
	if ((int)lparam >= g_ses_limit && lparam != 256) {
		mode = 1;
		lparam -= g_ses_limit;
	}
	if (session_list_recovered[0] && lparam == 256 && mode == 0)
		memcpy(session_list_t, session_list_recovered, sizeof(session_list_t));
	else
		for (auto &hContact : Contacts())
			if (LoadContactsFromMask(hContact, mode, lparam)) {
				int i = GetInSessionOrder(hContact, mode, lparam);
				session_list_t[i] = hContact;
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
