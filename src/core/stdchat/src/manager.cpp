/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "chat.h"

TABLIST *g_TabList = 0;

SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si)
{
	if (!si)
		return NULL;

	BOOL bFound = FALSE;
	SESSION_INFO *pTemp = pci->wndList;
	while (pTemp != NULL) {
		if (si == pTemp) {
			if (bFound)
				return NULL;
			else
				bFound = TRUE;
		}
		else if (bFound == TRUE && pTemp->hWnd)
			return pTemp;
		pTemp = pTemp->next;
		if (pTemp == NULL && bFound)
			pTemp = pci->wndList;
	}
	return NULL;
}

SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si)
{
	if (!si)
		return NULL;

	SESSION_INFO *pTemp = pci->wndList, *pLast = NULL;
	while (pTemp != NULL) {
		if (si == pTemp) {
			if (pLast) {
				if (pLast != pTemp)
					return pLast;
				else
					return NULL;
			}
		}
		if (pTemp->hWnd)
			pLast = pTemp;
		pTemp = pTemp->next;
		if (pTemp == NULL)
			pTemp = pci->wndList;
	}
	return NULL;
}

//---------------------------------------------------
//		Tab list manager functions
//
//		Necessary to keep track of what tabs should
//		be restored
//---------------------------------------------------

BOOL TabM_AddTab(const TCHAR *pszID, const char* pszModule)
{
	TABLIST *node = NULL;
	if (!pszID || !pszModule)
		return FALSE;

	node = (TABLIST*)mir_alloc(sizeof(TABLIST));
	memset(node, 0, sizeof(TABLIST));
	node->pszID = mir_tstrdup(pszID);
	node->pszModule = mir_strdup(pszModule);

	if (g_TabList == NULL) { // list is empty
		g_TabList = node;
		node->next = NULL;
	}
	else {
		node->next = g_TabList;
		g_TabList = node;
	}
	return TRUE;
}

BOOL TabM_RemoveAll(void)
{
	while (g_TabList != NULL) {
		TABLIST *pLast = g_TabList->next;
		mir_free(g_TabList->pszModule);
		mir_free(g_TabList->pszID);
		mir_free(g_TabList);
		g_TabList = pLast;
	}
	g_TabList = NULL;
	return TRUE;
}
