/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

SESSION_INFO* SM_FindSessionAutoComplete(const char *pszModule, SESSION_INFO *currSession, SESSION_INFO *prevSession, const wchar_t *pszOriginal, const wchar_t *pszCurrent)
{
	SESSION_INFO *pResult = nullptr;
	if (prevSession == nullptr && mir_wstrstri(currSession->ptszName, pszOriginal) == currSession->ptszName)
		pResult = currSession;
	else {
		wchar_t *pszName = nullptr;
		if (currSession == prevSession)
			pszCurrent = pszOriginal;

		for (auto &si : g_chatApi.arSessions) {
			if (si != currSession && !mir_strcmpi(pszModule, si->pszModule)) {
				if (mir_wstrstri(si->ptszName, pszOriginal) == si->ptszName) {
					if (prevSession != si && mir_wstrcmpi(si->ptszName, pszCurrent) > 0 && (!pszName || mir_wstrcmpi(si->ptszName, pszName) < 0)) {
						pResult = si;
						pszName = si->ptszName;
					}
				}
			}
		}
	}
	return pResult;
}

char SM_GetStatusIndicator(SESSION_INFO *si, USERINFO *ui)
{
	if (!ui || !si)
		return '\0';

	STATUSINFO *ti = g_chatApi.TM_FindStatus(si->pStatuses, g_chatApi.TM_WordToString(si->pStatuses, ui->Status));
	if (ti) {
		switch (si->iStatusCount - ti->iIconIndex - 1) {
		case 0: return '\0';
		case 1: return '+';
		case 2: return '%';
		case 3: return '@';
		case 4: return '!';
		case 5: return '*';
		}
	}
	return '\0';
}
