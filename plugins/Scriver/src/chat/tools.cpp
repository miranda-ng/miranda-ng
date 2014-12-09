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

TCHAR* my_strstri(const TCHAR* s1, const TCHAR* s2)
{
	for (int i = 0; s1[i]; i++)
		for (int j = i, k = 0; _totlower(s1[j]) == _totlower(s2[k]); j++, k++)
			if (!s2[k + 1])
				return (TCHAR*)(s1 + i);

	return NULL;
}

UINT CreateGCMenu(HWND hwnd, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText)
{
	GCMENUITEMS gcmi = { 0 };
	HMENU hSubMenu = 0;

	*hMenu = GetSubMenu(g_hMenu, iIndex);
	gcmi.pszID = si->ptszID;
	gcmi.pszModule = si->pszModule;
	gcmi.pszUID = pszUID;

	if (iIndex == 1) {
		int iLen = GetRichTextLength(hwnd, CP_ACP, FALSE);

		EnableMenuItem(*hMenu, IDM_CLEAR, MF_ENABLED);
		EnableMenuItem(*hMenu, ID_COPYALL, MF_ENABLED);
		if (!iLen) {
			EnableMenuItem(*hMenu, ID_COPYALL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(*hMenu, IDM_CLEAR, MF_BYCOMMAND | MF_GRAYED);
		}

		if (pszWordText && pszWordText[0]) {
			TCHAR szMenuText[4096];
			mir_sntprintf(szMenuText, SIZEOF(szMenuText), TranslateT("Look up '%s':"), pszWordText);
			ModifyMenu(*hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
			SetSearchEngineIcons(*hMenu, g_dat.hSearchEngineIconList);
		}
		else ModifyMenu(*hMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));
		gcmi.Type = MENU_ON_LOG;
	}
	else if (iIndex == 0) {
		TCHAR szTemp[50];
		if (pszWordText)
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("&Message %s"), pszWordText);
		else
			mir_tstrncpy(szTemp, TranslateT("&Message"), SIZEOF(szTemp) - 1);

		if (mir_tstrlen(szTemp) > 40)
			mir_tstrncpy(szTemp + 40, _T("..."), 4);
		ModifyMenu(*hMenu, ID_MESS, MF_STRING | MF_BYCOMMAND, ID_MESS, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(pci->hBuildMenuEvent, 0, (WPARAM)&gcmi);

	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);

	for (int i = 0; i < gcmi.nItems; i++) {
		TCHAR *ptszText = TranslateTS(gcmi.Item[i].pszDesc);
		DWORD dwState = gcmi.Item[i].bDisabled ? MF_GRAYED : 0;

		if (gcmi.Item[i].uType == MENU_NEWPOPUP) {
			hSubMenu = CreateMenu();
			AppendMenu(*hMenu, dwState | MF_POPUP, (UINT_PTR)hSubMenu, ptszText);
		}
		else if (gcmi.Item[i].uType == MENU_POPUPHMENU)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_POPUP, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPITEM)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPCHECK)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, dwState | MF_CHECKED | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_POPUPSEPARATOR)
			AppendMenu(hSubMenu == 0 ? *hMenu : hSubMenu, MF_SEPARATOR, 0, ptszText);
		else if (gcmi.Item[i].uType == MENU_SEPARATOR)
			AppendMenu(*hMenu, MF_SEPARATOR, 0, ptszText);
		else if (gcmi.Item[i].uType == MENU_HMENU)
			AppendMenu(*hMenu, dwState | MF_POPUP, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_ITEM)
			AppendMenu(*hMenu, dwState | MF_STRING, gcmi.Item[i].dwID, ptszText);
		else if (gcmi.Item[i].uType == MENU_CHECK)
			AppendMenu(*hMenu, dwState | MF_CHECKED | MF_STRING, gcmi.Item[i].dwID, ptszText);
	}
	return TrackPopupMenu(*hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
}

void DestroyGCMenu(HMENU *hMenu, int iIndex)
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_SUBMENU;
	while(GetMenuItemInfo(*hMenu, iIndex, TRUE, &mi))
	{
		if (mi.hSubMenu != NULL)
			DestroyMenu(mi.hSubMenu);
		RemoveMenu(*hMenu, iIndex, MF_BYPOSITION);
	}
}
