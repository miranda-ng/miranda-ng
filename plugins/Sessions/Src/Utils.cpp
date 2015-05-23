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

#include "sessions.h"

void AddSessionMark(MCONTACT hContact, int mode, char bit)
{
	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsMarks"));
		if (szValue) {
			char temp_1 = szValue[0];
			for (int i = 0; i < g_ses_limit; i++) {
				char temp_2 = szValue[i + 1];
				szValue[i + 1] = temp_1;
				temp_1 = temp_2;
			}
			for (int i = g_ses_limit; i < 10; i++)
				szValue[i] = '0';
			szValue[0] = bit;
			db_set_s(hContact, MODNAME, "LastSessionsMarks", szValue);
		}
		else if (bit == '1')
			db_set_s(hContact, MODNAME, "LastSessionsMarks", "10000000000");
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsMarks"));
		if (szValue) {
			char *pszBuffer;
			if (mir_strlen(szValue) < g_ses_count) {
				pszBuffer = (char*)mir_alloc(g_ses_count + 1);
				memset(pszBuffer, 0, (g_ses_count + 1));
				mir_strcpy(pszBuffer, szValue);
			}
			else pszBuffer = szValue.detouch();

			char temp_1 = pszBuffer[0];
			for (size_t i = 0; i < g_ses_count; i++) {
				char temp_2 = pszBuffer[i + 1];
				pszBuffer[i + 1] = temp_1;
				temp_1 = temp_2;
			}
			pszBuffer[0] = bit;
			db_set_s(hContact, MODNAME, "UserSessionsMarks", pszBuffer);

			mir_free(pszBuffer);
		}
		else if (bit == '1')
			db_set_s(hContact, MODNAME, "UserSessionsMarks", "10000000000");
		else
			db_set_s(hContact, MODNAME, "UserSessionsMarks", "00000000000");
	}
}

void RemoveSessionMark(MCONTACT hContact, int mode, int marknum)
{
	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsMarks"));
		if (szValue) {
			for (int i = marknum; i < g_ses_limit; i++)
				szValue[i] = szValue[i + 1];

			for (int i = g_ses_limit; i < 10; i++)
				szValue[i] = '0';

			db_set_s(hContact, MODNAME, "LastSessionsMarks", szValue);
		}
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsMarks"));
		if (szValue) {
			for (int i = marknum; i < g_ses_limit; i++)
				szValue[i] = szValue[i + 1];

			db_set_s(hContact, MODNAME, "UserSessionsMarks", szValue);
		}
	}
}

void SetSessionMark(MCONTACT hContact, int mode, char bit, unsigned int marknum)
{
	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsMarks"));
		if (szValue) {
			szValue[marknum] = bit;
			db_set_s(hContact, MODNAME, "LastSessionsMarks", szValue);
		}
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsMarks"));
		if (szValue) {
			szValue[marknum] = bit;
			db_set_s(hContact, MODNAME, "UserSessionsMarks", szValue);
		}
	}
}

bool LoadContactsFromMask(MCONTACT hContact, int mode, int count)
{
	ptrA szValue;
	if (mode == 0)
		szValue = db_get_sa(hContact, MODNAME, "LastSessionsMarks");
	else if (mode == 1)
		szValue = db_get_sa(hContact, MODNAME, "UserSessionsMarks");

	if (szValue == NULL)
		return false;

	return szValue[count] == '1';
}

void AddInSessionOrder(MCONTACT hContact, int mode, int ordernum, int writemode)
{
	char szFormNumBuf[100];
	mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%02u", ordernum);

	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsMarks"));
		if (szValue) {
			int len = (int)mir_strlen(szValue);
			if (!len)
				len = 20;

			char *temp2 = (char*)_alloca(len - 1);
			strncpy(temp2, szValue, len - 2);
			temp2[len - 2] = '\0';

			char *temp = (char*)_alloca(len + 1);
			mir_snprintf(temp, len + 1, "%02u%s", ordernum, temp2);

			for (int i = (g_ses_limit * 2); i < 20; i++)
				temp[i] = '0';

			db_set_s(hContact, MODNAME, "LastSessionsOrder", temp);
		}
		else if (writemode == 1) {
			mir_snprintf(szFormNumBuf, SIZEOF(szFormNumBuf), "%02u%s", ordernum, "000000000000000000");
			db_set_s(hContact, MODNAME, "LastSessionsOrder", szFormNumBuf);
		}
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsOrder"));
		if (szValue) {
			char *pszBuffer;
			if (mir_strlen(szValue) < (g_ses_count * 2)) {
				pszBuffer = (char*)mir_alloc((g_ses_count * 2) + 1);
				memset(pszBuffer, 0, ((g_ses_count * 2) + 1));
				mir_strcpy(pszBuffer, szValue);
			}
			else pszBuffer = mir_strdup(szValue);

			int len = (int)mir_strlen(pszBuffer);
			len = (len == 0) ? 20 : len + 2;
			char *temp = (char*)_alloca(len + 1);
			mir_snprintf(temp, len + 1, "%02u%s", ordernum, szValue);

			db_set_s(hContact, MODNAME, "UserSessionsOrder", temp);
			mir_free(pszBuffer);
		}
		else if (writemode == 1)
			db_set_s(hContact, MODNAME, "UserSessionsOrder", szFormNumBuf);
		else
			db_set_s(hContact, MODNAME, "UserSessionsOrder", "00");
	}
}

int GetInSessionOrder(MCONTACT hContact, int mode, int count)
{
	char szTemp[3];
	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsOrder"));
		if (szValue) {
			strncpy_s(szTemp, &szValue[count * 2], 2);
			return atoi(szTemp);
		}
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsOrder"));
		if (szValue) {
			strncpy_s(szTemp, &szValue[count * 2], 2);
			return atoi(szTemp);
		}
	}
	return 0;
}

void SetInSessionOrder(MCONTACT hContact, int mode, int count, unsigned int ordernum)
{
	char szTemp[3];
	mir_snprintf(szTemp, SIZEOF(szTemp), "%02u", ordernum);

	if (mode == 0) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "LastSessionsOrder"));
		if (szValue) {
			szValue[count * 2] = szTemp[0];
			szValue[count * 2 + 1] = szTemp[1];
			db_set_s(hContact, MODNAME, "LastSessionsOrder", szValue);
		}
	}
	else if (mode == 1) {
		ptrA szValue(db_get_sa(hContact, MODNAME, "UserSessionsOrder"));
		if (szValue) {
			szValue[count * 2] = szTemp[0];
			szValue[count * 2 + 1] = szTemp[1];
			db_set_s(hContact, MODNAME, "UserSessionsOrder", szValue);
		}
	}
}

BOOL ResaveSettings(char *szName, int iFirst, int iLimit, TCHAR *szBuffer)
{
	for (int i = iFirst; i < iLimit; i++) {
		if (szBuffer == NULL)
			break;

		char szNameBuf[256];
		mir_snprintf(szNameBuf, SIZEOF(szNameBuf), "%s_%u", szName, i);

		TCHAR *ptszTemp = db_get_tsa(NULL, MODNAME, szNameBuf);
		db_set_ts(NULL, MODNAME, szNameBuf, szBuffer);
		mir_free(szBuffer);

		BYTE marked = IsMarkedUserDefSession(i);
		MarkUserDefSession(i, (BYTE)((i == iFirst) ? IsMarkedUserDefSession(iFirst - 1) : marked));

		if (ptszTemp == NULL) // read failed
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
		mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s_%u", pszSetting, i);
		TCHAR *pszBuffer = db_get_tsa(NULL, MODNAME, szBuffer);
		if (pszBuffer) {
			if (!IsMarkedUserDefSession(i + iFirstNum) || mode == 1) {
				index = SendDlgItemMessage(hdlg, IDC_LIST, CB_ADDSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, (WPARAM)index, (LPARAM)(i + iFirstNum));
			}
			else {
				SendDlgItemMessage(hdlg, IDC_LIST, CB_INSERTSTRING, 0, (LPARAM)pszBuffer);
				SendDlgItemMessage(hdlg, IDC_LIST, CB_SETITEMDATA, 0, (LPARAM)(i + iFirstNum));
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
			mir_snprintf(szBuffer, SIZEOF(szBuffer), "%s_%u", "UserSessionDsc", i);
			TCHAR *pszBuffer = db_get_tsa(NULL, MODNAME, szBuffer);
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
	SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

int CheckContactVisibility(MCONTACT hContact)
{
	return !db_get_b(hContact, "CList", "Hidden", 0);
}

void RenameUserDefSession(int ses_count, TCHAR* ptszNewName)
{
	char szSession[256];
	mir_snprintf(szSession, SIZEOF(szSession), "%s_%u", "UserSessionDsc", ses_count);
	db_set_ts(NULL, MODNAME, szSession, ptszNewName);
}

int MarkUserDefSession(int ses_count, BYTE bCheck)
{
	char szSessionName[256];
	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "FavUserSession", ses_count);
	db_set_b(NULL, MODNAME, szSessionName, bCheck);
	return 0;
}

BYTE IsMarkedUserDefSession(int ses_count)
{
	char szSessionName[256];
	mir_snprintf(szSessionName, SIZEOF(szSessionName), "%s_%u", "FavUserSession", ses_count);
	return db_get_b(NULL, MODNAME, szSessionName, 0);
}

void SavePosition(HWND hwnd, char *wndName)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	char buffer[512];
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosX", wndName);
	db_set_dw(0, MODNAME, buffer, rc.left);
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosY", wndName);
	db_set_dw(0, MODNAME, buffer, rc.top);
}

void LoadPosition(HWND hWnd, char *wndName)
{
	char buffer[512];
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosX", wndName);
	int x = db_get_dw(0, MODNAME, buffer, ((GetSystemMetrics(SM_CXSCREEN)) / 2) - 130);
	mir_snprintf(buffer, SIZEOF(buffer), "%sPosY", wndName);
	int y = db_get_dw(0, MODNAME, buffer, ((GetSystemMetrics(SM_CYSCREEN)) / 2) - 80);
	SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);
}
