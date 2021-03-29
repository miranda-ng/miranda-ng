/*
Sessions Management plugin for Miranda IM

Copyright (C) 2007-2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

void AddSessionMark(MCONTACT hContact, int mode, char bit)
{
	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsMarks"));
		if (bit == '0' && szValue.IsEmpty())
			return;

		szValue.Insert(0, bit);
		szValue.Truncate(g_ses_limit);
		g_plugin.setString(hContact, "LastSessionsMarks", szValue);
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsMarks"));
		szValue.Insert(0, bit);
		szValue.Truncate((int)g_ses_count);
		g_plugin.setString(hContact, "UserSessionsMarks", szValue);
	}
}

void RemoveSessionMark(MCONTACT hContact, int mode, int marknum)
{
	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsMarks"));
		if (!szValue.IsEmpty() && marknum < szValue.GetLength()) {
			szValue.Delete(marknum, 1);
			g_plugin.setString(hContact, "LastSessionsMarks", szValue);
		}
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsMarks"));
		if (!szValue.IsEmpty() && marknum < szValue.GetLength()) {
			szValue.Delete(marknum, 1);
			g_plugin.setString(hContact, "UserSessionsMarks", szValue);
		}
	}
}

void SetSessionMark(MCONTACT hContact, int mode, char bit, int marknum)
{
	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsMarks"));
		if (!szValue.IsEmpty() && marknum < szValue.GetLength()) {
			szValue.SetAt(marknum, bit);
			g_plugin.setString(hContact, "LastSessionsMarks", szValue);
		}
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsMarks"));
		if (!szValue.IsEmpty() && marknum < szValue.GetLength()) {
			szValue.SetAt(marknum, bit);
			g_plugin.setString(hContact, "UserSessionsMarks", szValue);
		}
	}
}

bool LoadContactsFromMask(MCONTACT hContact, int mode, int count)
{
	CMStringA szValue;
	if (mode == 0)
		szValue = g_plugin.getMStringA(hContact, "LastSessionsMarks");
	else if (mode == 1)
		szValue = g_plugin.getMStringA(hContact, "UserSessionsMarks");

	if (szValue.IsEmpty())
		return false;

	return szValue[count] == '1';
}

int GetInSessionOrder(MCONTACT hContact, int mode, int count)
{
	char szTemp[3] = { 0, 0, 0 };
	count *= 2;

	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			memcpy(szTemp, szValue.c_str() + count, 2);
			return atoi(szTemp);
		}
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			memcpy(szTemp, szValue.c_str() + count, 2);
			return atoi(szTemp);
		}
	}
	return 0;
}

void SetInSessionOrder(MCONTACT hContact, int mode, int count, int ordernum)
{
	char szTemp[3];
	mir_snprintf(szTemp, "%02u", ordernum);
	count *= 2;

	if (mode == 0) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "LastSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			szValue.SetAt(count, szTemp[0]);
			szValue.SetAt(count + 1, szTemp[1]);
			g_plugin.setString(hContact, "LastSessionsOrder", szValue);
		}
	}
	else if (mode == 1) {
		CMStringA szValue(g_plugin.getMStringA(hContact, "UserSessionsOrder"));
		if (!szValue.IsEmpty() && count < szValue.GetLength()) {
			szValue.SetAt(count, szTemp[0]);
			szValue.SetAt(count + 1, szTemp[1]);
			g_plugin.setString(hContact, "UserSessionsOrder", szValue);
		}
	}
}

BOOL ResaveSettings(char *szName, int iFirst, int iLimit, wchar_t *szBuffer)
{
	for (int i = iFirst; i < iLimit; i++) {
		if (szBuffer == nullptr)
			break;

		char szNameBuf[256];
		mir_snprintf(szNameBuf, "%s_%u", szName, i);

		wchar_t *ptszTemp = g_plugin.getWStringA(szNameBuf);
		g_plugin.setWString(szNameBuf, szBuffer);
		mir_free(szBuffer);

		BYTE marked = IsMarkedUserDefSession(i);
		MarkUserDefSession(i, (BYTE)((i == iFirst) ? IsMarkedUserDefSession(iFirst - 1) : marked));

		if (ptszTemp == nullptr) // read failed
			return 0;

		szBuffer = ptszTemp;
	}

	mir_free(szBuffer);
	return 1;
}

int AddToCurSession(MCONTACT wparam, LPARAM)
{
	if (CheckForDuplicate(session_list, wparam) == -1) {
		for (int i = 0;; i++) {
			if (session_list[i] == 0) {
				session_list[i] = wparam;
				break;
			}
		}
	}
	return 0;
}

int DelFromCurSession(MCONTACT wparam, LPARAM)
{
	for (int i = 0; session_list[i] != 0; i++) {
		if (session_list[i] == wparam) {
			while (session_list[i + 1] != 0) {
				session_list[i] = session_list[i + 1];
				i++;
			}
			session_list[i] = 0;
		}
	}
	return 0;
}

int CheckForDuplicate(MCONTACT contact_list[], MCONTACT lparam)
{
	MCONTACT s_list[255] = { 0 };
	memcpy(s_list, contact_list, sizeof(s_list));
	for (int i = 0;; i++) {
		if (s_list[i] == lparam)
			return i;
		if (s_list[i] == 0)
			return -1;
	}
}

int LoadSessionToCombobox(HWND hdlg, BOOL mode, int iLimit, char* pszSetting, int iFirstNum)
{
	int ses_count = iFirstNum, index;
	char szBuffer[256] = { 0 };
	if (session_list_recovered[0] && !iFirstNum && !mode) {
		index = SendDlgItemMessage(hdlg, IDC_LIST, CB_ADDSTRING, 0, (LPARAM)TranslateT("Session Before Last Crash"));
		SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)index, 256);
	}

	for (int i = 0; i < iLimit; i++) {
		mir_snprintf(szBuffer, "%s_%u", pszSetting, i);
		wchar_t *pszBuffer = g_plugin.getWStringA(szBuffer);
		if (pszBuffer) {
			if (!IsMarkedUserDefSession(i + iFirstNum) || mode == 1) {
				index = SendDlgItemMessage(hdlg, IDC_LIST, CB_ADDSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)index, i + iFirstNum);
			}
			else {
				SendDlgItemMessage(hdlg, IDC_LIST, CB_INSERTSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, 0, i + iFirstNum);
			}
			mir_free(pszBuffer);
		}
		else {
			if (i == 0) ses_count = iLimit - iFirstNum;
			break;
		}
	}

	return ses_count;
}

int FillFavoritesMenu(HMENU hMenu, int iLimit)
{
	int iItems = 0;

	for (int i = 0; i < iLimit; i++) {
		if (IsMarkedUserDefSession(i)) {
			char szBuffer[256];
			mir_snprintf(szBuffer, "%s_%u", "UserSessionDsc", i);
			wchar_t *pszBuffer = g_plugin.getWStringA(szBuffer);
			if (pszBuffer) {
				AppendMenu(hMenu, MF_STRING, i + 1, pszBuffer);
				iItems++;
				mir_free(pszBuffer);
			}
		}
	}
	return iItems;
}

void OffsetWindow(HWND parent, HWND hwnd, int dx, int dy)
{
	POINT pt;
	RECT rc;
	GetWindowRect(hwnd, &rc);
	pt.x = rc.left;
	pt.y = rc.top;
	ScreenToClient(parent, &pt);
	rc.left = pt.x;
	rc.top = pt.y;

	OffsetRect(&rc, dx, dy);
	SetWindowPos(hwnd, nullptr, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

int CheckContactVisibility(MCONTACT hContact)
{
	return db_mc_isSub(hContact) || !Contact_IsHidden(hContact);
}

void RenameUserDefSession(int ses_count, wchar_t* ptszNewName)
{
	char szSession[256];
	mir_snprintf(szSession, "%s_%u", "UserSessionDsc", ses_count);
	g_plugin.setWString(szSession, ptszNewName);
}

int MarkUserDefSession(int ses_count, BYTE bCheck)
{
	char szSessionName[256];
	mir_snprintf(szSessionName, "%s_%u", "FavUserSession", ses_count);
	g_plugin.setByte(szSessionName, bCheck);
	return 0;
}

BYTE IsMarkedUserDefSession(int ses_count)
{
	char szSessionName[256];
	mir_snprintf(szSessionName, "%s_%u", "FavUserSession", ses_count);
	return g_plugin.getByte(szSessionName, 0);
}

void SavePosition(HWND hwnd, char *wndName)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	char buffer[512];
	mir_snprintf(buffer, "%sPosX", wndName);
	g_plugin.setDword(buffer, rc.left);
	mir_snprintf(buffer, "%sPosY", wndName);
	g_plugin.setDword(buffer, rc.top);
}

void LoadPosition(HWND hWnd, char *wndName)
{
	char buffer[512];
	mir_snprintf(buffer, "%sPosX", wndName);
	int x = g_plugin.getDword(buffer, ((GetSystemMetrics(SM_CXSCREEN)) / 2) - 130);
	mir_snprintf(buffer, "%sPosY", wndName);
	int y = g_plugin.getDword(buffer, ((GetSystemMetrics(SM_CYSCREEN)) / 2) - 80);
	SetWindowPos(hWnd, nullptr, x, y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
}
