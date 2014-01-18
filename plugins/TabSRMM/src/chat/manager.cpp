/*
astyle --force-indent=tab=4 --brackets=linux --indent-switches
		--pad=oper --one-line=keep-blocks  --unpad=paren

Chat module plugin for Miranda NG

Copyright (C) 2003 Jörgen Persson

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

#include "..\commonheaders.h"

//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------

void SM_RemoveContainer(TContainerData *pContainer)
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		if (si->pContainer == pContainer)
			si->pContainer = NULL;
}

BOOL SM_ReconfigureFilters()
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		Chat_SetFilters(si);

	return TRUE;
}

SESSION_INFO* SM_FindSessionByHWND(HWND hWnd)
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		if (si->hWnd == hWnd)
			return si;

	return NULL;
}

SESSION_INFO* SM_FindSessionByHCONTACT(HANDLE h)
{
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		if (si->hContact == h)
			return si;

	return NULL;
}

SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const TCHAR* pszOriginal, const TCHAR* pszCurrent)
{
	if (prevSession == NULL && my_strstri(currSession->ptszName, pszOriginal) == currSession->ptszName)
		return currSession;

	TCHAR* pszName = NULL;
	if (currSession == prevSession)
		pszCurrent = pszOriginal;

	SESSION_INFO *pResult = NULL;
	for (SESSION_INFO *si = pci->wndList; si; si = si->next)
		if (si != currSession && !lstrcmpiA(pszModule, si->pszModule))
			if (my_strstri(si->ptszName, pszOriginal) == si->ptszName)
				if (prevSession != si && lstrcmpi(si->ptszName, pszCurrent) > 0 && (!pszName || lstrcmpi(si->ptszName, pszName) < 0)) {
					pResult = si;
					pszName = si->ptszName;
				}

	return pResult;
}
