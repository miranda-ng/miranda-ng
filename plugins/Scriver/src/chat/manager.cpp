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

#include "../commonheaders.h"

HWND SM_FindWindowByContact(MCONTACT hContact)
{
	SESSION_INFO *pTemp = pci->wndList;

	while (pTemp != NULL) {
		if (pTemp->hContact == hContact)
			return pTemp->hWnd;

		pTemp = pTemp->next;
	}
	return NULL;
}

SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	SESSION_INFO* pResult = NULL;
	if (prevSession == NULL && my_strstri(currSession->ptszName, pszOriginal) == currSession->ptszName)
		pResult = currSession;
	else {
		TCHAR* pszName = NULL;
		SESSION_INFO* pTemp = pci->wndList;
		if (currSession == prevSession)
			pszCurrent = pszOriginal;

		while (pTemp != NULL) {
			if (pTemp != currSession && !mir_strcmpi(pszModule, pTemp->pszModule)) {
				if (my_strstri(pTemp->ptszName, pszOriginal) == pTemp->ptszName) {
					if (prevSession != pTemp && mir_tstrcmpi(pTemp->ptszName, pszCurrent) > 0 && (!pszName || mir_tstrcmpi(pTemp->ptszName, pszName) < 0)) {
						pResult = pTemp;
						pszName = pTemp->ptszName;
					}
				}
			}
			pTemp = pTemp->next;
		}
	}
	return pResult;
}

char SM_GetStatusIndicator(SESSION_INFO *si, USERINFO *ui)
{
	STATUSINFO * ti;
	if (!ui || !si)
		return '\0';

	ti = pci->TM_FindStatus(si->pStatuses, pci->TM_WordToString(si->pStatuses, ui->Status));
	if (ti) {
		if ((INT_PTR)ti->hIcon < STATUSICONCOUNT) {
			INT_PTR id = si->iStatusCount - (INT_PTR)ti->hIcon - 1;
			if (id == 0)
				return '\0';
			if (id == 1)
				return '+';
			if (id == 2)
				return '%';
			if (id == 3)
				return '@';
			if (id == 4)
				return '!';
			if (id == 5)
				return '*';
		}
		else
			return '\0';
	}
	return '\0';
}
