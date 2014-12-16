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

bool LoadMessageFont(LOGFONT *lf, COLORREF *colour)
{
	char str[32];
	int i = 8; // MSGFONTID_MESSAGEAREA

	if (colour) {
		mir_snprintf(str, SIZEOF(str), "SRMFont%dCol", i);
		*colour = db_get_dw(NULL, "SRMM", str, 0);
	}
	if (lf) {
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSize", i);
		lf->lfHeight = db_get_b(NULL, "SRMM", str, -12);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSty", i);
		int style = db_get_b(NULL, "SRMM", str, 0);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, SIZEOF(str), "SRMFont%d", i);

		DBVARIANT dbv;
		if (db_get_ts(NULL, "SRMM", str, &dbv))
			_tcscpy(lf->lfFaceName, _T("Arial"));
		else {
			mir_tstrncpy(lf->lfFaceName, dbv.ptszVal, SIZEOF(lf->lfFaceName));
			db_free(&dbv);
		}
		mir_snprintf(str, SIZEOF(str), "SRMFont%dSet", i);
		lf->lfCharSet = db_get_b(NULL, "SRMM", str, DEFAULT_CHARSET);
	}
	return true;
}

int GetRichTextLength(HWND hwnd)
{
	GETTEXTLENGTHEX gtl;
	gtl.flags = GTL_PRECISE;
	gtl.codepage = CP_ACP;
	return (int)SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

int GetColorIndex(const char* pszModule, COLORREF cr)
{
	MODULEINFO * pMod = pci->MM_FindModule(pszModule);
	int i = 0;

	if (!pMod || pMod->nColorCount == 0)
		return -1;

	for (i = 0; i < pMod->nColorCount; i++)
	if (pMod->crColors[i] == cr)
		return i;

	return -1;
}

// obscure function that is used to make sure that any of the colors
// passed by the protocol is used as fore- or background color
// in the messagebox. THis is to vvercome limitations in the richedit
// that I do not know currently how to fix

void CheckColorsInModule(const char* pszModule)
{
	MODULEINFO *pMod = pci->MM_FindModule(pszModule);
	int i = 0;
	COLORREF crBG = (COLORREF)db_get_dw(NULL, CHAT_MODULE, "ColorMessageBG", GetSysColor(COLOR_WINDOW));

	if (!pMod)
		return;

	for (i = 0; i < pMod->nColorCount; i++) {
		if (pMod->crColors[i] == g_Settings.MessageAreaColor || pMod->crColors[i] == crBG) {
			if (pMod->crColors[i] == RGB(255, 255, 255))
				pMod->crColors[i]--;
			else
				pMod->crColors[i]++;
		}
	}
}

UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, TCHAR* pszUID, TCHAR* pszWordText)
{
	HMENU hSubMenu = 0;
	*hMenu = GetSubMenu(g_hMenu, iIndex);
	TranslateMenu(*hMenu);

	GCMENUITEMS gcmi = { 0 };
	gcmi.pszID = si->ptszID;
	gcmi.pszModule = si->pszModule;
	gcmi.pszUID = pszUID;

	if (iIndex == 1) {
		int i = GetRichTextLength(GetDlgItem(hwndDlg, IDC_LOG));

		EnableMenuItem(*hMenu, ID_CLEARLOG, MF_ENABLED);
		EnableMenuItem(*hMenu, ID_COPYALL, MF_ENABLED);
		ModifyMenu(*hMenu, 4, MF_GRAYED | MF_BYPOSITION, 4, NULL);
		if (!i) {
			EnableMenuItem(*hMenu, ID_COPYALL, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(*hMenu, ID_CLEARLOG, MF_BYCOMMAND | MF_GRAYED);
			if (pszWordText && pszWordText[0])
				ModifyMenu(*hMenu, 4, MF_ENABLED | MF_BYPOSITION, 4, NULL);
		}

		if (pszWordText && pszWordText[0]) {
			TCHAR szMenuText[4096];
			mir_sntprintf(szMenuText, SIZEOF(szMenuText), TranslateT("Look up '%s':"), pszWordText);
			ModifyMenu(*hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
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
			mir_tstrcpy(szTemp + 40, _T("..."));
		ModifyMenu(*hMenu, ID_MESS, MF_STRING | MF_BYCOMMAND, ID_MESS, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(pci->hBuildMenuEvent, 0, (WPARAM)&gcmi);

	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);

	for (int i = 0; i < gcmi.nItems; i++) {
		TCHAR* ptszText = TranslateTS(gcmi.Item[i].pszDesc);
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
	return TrackPopupMenu(*hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
}

void DestroyGCMenu(HMENU *hMenu, int iIndex)
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(mi);
	mi.fMask = MIIM_SUBMENU;
	while (GetMenuItemInfo(*hMenu, iIndex, TRUE, &mi)) {
		if (mi.hSubMenu != NULL)
			DestroyMenu(mi.hSubMenu);
		RemoveMenu(*hMenu, iIndex, MF_BYPOSITION);
	}
}

void ValidateFilename(TCHAR *filename)
{
	TCHAR *p1 = filename;
	TCHAR szForbidden[] = _T("\\/:*?\"<>|");
	while (*p1 != '\0') {
		if (_tcschr(szForbidden, *p1))
			*p1 = '_';
		p1 += 1;
	}
}
