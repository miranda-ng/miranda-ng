/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors

#include "stdafx.h"

static int sttCompareNicknames(const wchar_t *s1, const wchar_t *s2)
{
	if (s2 == nullptr)
		return 1;

	// skip rubbish
	while (*s1 && !iswalpha(*s1)) ++s1;
	while (*s2 && !iswalpha(*s2)) ++s2;

	// are there ~0veRy^kEwL_n1kz?
	if (!*s1 && !*s2) return 0;
	if (!*s1 && *s2) return +1;
	if (*s1 && !*s2) return -1;

	// compare tails
	return mir_wstrcmpi(s1, s2);
}

int UM_CompareItem(USERINFO *u1, const wchar_t* pszNick, WORD wStatus)
{
	WORD dw1 = u1->Status;
	WORD dw2 = wStatus;

	for (int i = 0; i < 8; i++) {
		if ((dw1 & 1) && !(dw2 & 1))
			return -1;
		if ((dw2 & 1) && !(dw1 & 1))
			return 1;
		if ((dw1 & 1) && (dw2 & 1)) {
			if (g_Settings.bAlternativeSorting)
				return sttCompareNicknames(u1->pszNick, pszNick);
			else
				return mir_wstrcmp(u1->pszNick, pszNick);
		}
		dw1 = dw1 >> 1;
		dw2 = dw2 >> 1;
	}

	if (g_Settings.bAlternativeSorting)
		return sttCompareNicknames(u1->pszNick, pszNick);
	return mir_wstrcmp(u1->pszNick, pszNick);
}

//---------------------------------------------------
//		Session Manager functions
//
//		Keeps track of all sessions and its windows
//---------------------------------------------------

BOOL SM_ReconfigureFilters()
{
	for (int i = 0; i < pci->arSessions.getCount(); i++)
		Chat_SetFilters(pci->arSessions[i]);

	return TRUE;
}

SESSION_INFO* SM_FindSessionByHWND(HWND hWnd)
{
	for (int i = 0; i < pci->arSessions.getCount(); i++) {
		SESSION_INFO *si = pci->arSessions[i];
		if (si->pDlg && si->pDlg->GetHwnd() == hWnd)
			return si;
	}

	return nullptr;
}

SESSION_INFO* SM_FindSessionByHCONTACT(MCONTACT h)
{
	for (int i = 0; i < pci->arSessions.getCount(); i++) {
		SESSION_INFO *si = pci->arSessions[i];
		if (si->hContact == h)
			return si;
	}

	return nullptr;
}

SESSION_INFO* SM_FindSessionAutoComplete(const char* pszModule, SESSION_INFO* currSession, SESSION_INFO* prevSession, const wchar_t* pszOriginal, const wchar_t* pszCurrent)
{
	if (prevSession == nullptr && my_strstri(currSession->ptszName, pszOriginal) == currSession->ptszName)
		return currSession;

	wchar_t *pszName = nullptr;
	if (currSession == prevSession)
		pszCurrent = pszOriginal;

	SESSION_INFO *pResult = nullptr;
	for (int i = 0; i < pci->arSessions.getCount(); i++) {
		SESSION_INFO *si = pci->arSessions[i];
		if (si != currSession && !mir_strcmpi(pszModule, si->pszModule)) {
			if (my_strstri(si->ptszName, pszOriginal) == si->ptszName) {
				if (prevSession != si && mir_wstrcmpi(si->ptszName, pszCurrent) > 0 && (!pszName || mir_wstrcmpi(si->ptszName, pszName) < 0)) {
					pResult = si;
					pszName = si->ptszName;
				}
			}
		}
	}

	return pResult;
}
