/*

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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

#include "stdafx.h"

void CChatRoomDlg::StreamInEvents(LOGINFO *lin, bool bRedraw)
{
	if (m_hwnd == nullptr || lin == nullptr || m_si == nullptr)
		return;

	if (!bRedraw && m_si->iType == GCW_CHATROOM && m_bFilterEnabled && (m_iLogFilterFlags & lin->iType) == 0)
		return;

	LOGSTREAMDATA streamData;
	memset(&streamData, 0, sizeof(streamData));
	streamData.hwnd = m_log.GetHwnd();
	streamData.si = m_si;
	streamData.lin = lin;
	streamData.bStripFormat = FALSE;

	BOOL bFlag = FALSE;

	EDITSTREAM stream = {};
	stream.pfnCallback = Srmm_LogStreamCallback;
	stream.dwCookie = (DWORD_PTR)& streamData;

	SCROLLINFO scroll;
	scroll.cbSize = sizeof(SCROLLINFO);
	scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	GetScrollInfo(m_log.GetHwnd(), SB_VERT, &scroll);

	POINT point = {};
	SendMessage(m_log.GetHwnd(), EM_GETSCROLLPOS, 0, (LPARAM)&point);

	// do not scroll to bottom if there is a selection
	CHARRANGE oldsel, sel;
	SendMessage(m_log.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&oldsel);
	if (oldsel.cpMax != oldsel.cpMin)
		SendMessage(m_log.GetHwnd(), WM_SETREDRAW, FALSE, 0);

	//set the insertion point at the bottom
	sel.cpMin = sel.cpMax = GetRichTextLength(m_log.GetHwnd());
	SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);

	// fix for the indent... must be a M$ bug
	if (sel.cpMax == 0)
		bRedraw = TRUE;

	// should the event(s) be appended to the current log
	WPARAM wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

	//get the number of pixels per logical inch
	if (bRedraw) {
		HDC hdc = GetDC(NULL);
		pci->logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
		pci->logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
		SendMessage(m_log.GetHwnd(), WM_SETREDRAW, FALSE, 0);
		bFlag = TRUE;
	}

	// stream in the event(s)
	streamData.lin = lin;
	streamData.bRedraw = bRedraw;
	SendMessage(m_log.GetHwnd(), EM_STREAMIN, wp, (LPARAM)&stream);

	// do smileys
	if (SmileyAddInstalled && (bRedraw || (lin->ptszText && lin->iType != GC_EVENT_JOIN && lin->iType != GC_EVENT_NICK && lin->iType != GC_EVENT_ADDSTATUS && lin->iType != GC_EVENT_REMOVESTATUS))) {
		CHARRANGE newsel;
		newsel.cpMax = -1;
		newsel.cpMin = sel.cpMin;
		if (newsel.cpMin < 0)
			newsel.cpMin = 0;

		SMADD_RICHEDIT3 sm = {};
		sm.cbSize = sizeof(sm);
		sm.hwndRichEditControl = m_log.GetHwnd();
		sm.Protocolname = m_si->pszModule;
		sm.rangeToReplace = bRedraw ? NULL : &newsel;
		sm.disableRedraw = TRUE;
		sm.hContact = m_si->hContact;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&sm);
	}

	// scroll log to bottom if the log was previously scrolled to bottom, else restore old position
	if (bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 || scroll.nMax - scroll.nMin - scroll.nPage < 50)
		ScrollToBottom();
	else
		SendMessage(m_log.GetHwnd(), EM_SETSCROLLPOS, 0, (LPARAM)&point);

	// do we need to restore the selection
	if (oldsel.cpMax != oldsel.cpMin) {
		SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&oldsel);
		SendMessage(m_log.GetHwnd(), WM_SETREDRAW, TRUE, 0);
		InvalidateRect(m_log.GetHwnd(), NULL, TRUE);
	}

	// need to invalidate the window
	if (bFlag) {
		sel.cpMin = sel.cpMax = GetRichTextLength(m_log.GetHwnd());
		SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
		SendMessage(m_log.GetHwnd(), WM_SETREDRAW, TRUE, 0);
		InvalidateRect(m_log.GetHwnd(), NULL, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si)
{
	if (hwndDlg == 0 || si == 0)
		return NULL;

	char* pszText = NULL;
	EDITSTREAM stream;
	memset(&stream, 0, sizeof(stream));
	stream.pfnCallback = Srmm_MessageStreamCallback;
	stream.dwCookie = (DWORD_PTR)&pszText; // pass pointer to pointer

	DWORD dwFlags = SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE | (CP_UTF8 << 16);
	SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_STREAMOUT, dwFlags, (LPARAM)&stream);
	return pszText; // pszText contains the text
}

/////////////////////////////////////////////////////////////////////////////////////////

bool LoadMessageFont(LOGFONT *lf, COLORREF *colour)
{
	char str[32];
	int i = 8; // MSGFONTID_MESSAGEAREA

	if (colour) {
		mir_snprintf(str, "SRMFont%dCol", i);
		*colour = db_get_dw(NULL, "SRMM", str, 0);
	}
	if (lf) {
		mir_snprintf(str, "SRMFont%dSize", i);
		lf->lfHeight = (char)db_get_b(NULL, "SRMM", str, -12);
		lf->lfWidth = 0;
		lf->lfEscapement = 0;
		lf->lfOrientation = 0;
		mir_snprintf(str, "SRMFont%dSty", i);
		int style = db_get_b(NULL, "SRMM", str, 0);
		lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
		lf->lfItalic = style & DBFONTF_ITALIC ? 1 : 0;
		lf->lfUnderline = 0;
		lf->lfStrikeOut = 0;
		lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf->lfQuality = DEFAULT_QUALITY;
		lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		mir_snprintf(str, "SRMFont%d", i);

		ptrW wszFontFace(db_get_wsa(NULL, "SRMM", str));
		if (wszFontFace == nullptr)
			mir_wstrcpy(lf->lfFaceName, L"Arial");
		else
			mir_wstrncpy(lf->lfFaceName, wszFontFace, _countof(lf->lfFaceName));

		mir_snprintf(str, "SRMFont%dSet", i);
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
	MODULEINFO *pMod = pci->MM_FindModule(pszModule);
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

UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, wchar_t* pszUID, wchar_t* pszWordText)
{
	HMENU hSubMenu = 0;
	*hMenu = GetSubMenu(g_hMenu, iIndex);
	TranslateMenu(*hMenu);

	GCMENUITEMS gcmi = {};
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
			wchar_t szMenuText[4096];
			mir_snwprintf(szMenuText, TranslateT("Look up '%s':"), pszWordText);
			ModifyMenu(*hMenu, 4, MF_STRING | MF_BYPOSITION, 4, szMenuText);
		}
		else ModifyMenu(*hMenu, 4, MF_STRING | MF_GRAYED | MF_BYPOSITION, 4, TranslateT("No word to look up"));
		gcmi.Type = MENU_ON_LOG;
	}
	else if (iIndex == 0) {
		wchar_t szTemp[50];
		if (pszWordText)
			mir_snwprintf(szTemp, TranslateT("&Message %s"), pszWordText);
		else
			mir_wstrncpy(szTemp, TranslateT("&Message"), _countof(szTemp) - 1);

		if (mir_wstrlen(szTemp) > 40)
			mir_wstrcpy(szTemp + 40, L"...");
		ModifyMenu(*hMenu, ID_MESS, MF_STRING | MF_BYCOMMAND, ID_MESS, szTemp);
		gcmi.Type = MENU_ON_NICKLIST;
	}

	NotifyEventHooks(pci->hBuildMenuEvent, 0, (WPARAM)&gcmi);

	if (gcmi.nItems > 0)
		AppendMenu(*hMenu, MF_SEPARATOR, 0, 0);

	for (int i = 0; i < gcmi.nItems; i++) {
		wchar_t* ptszText = TranslateW(gcmi.Item[i].pszDesc);
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
	MENUITEMINFO mii = {};
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	while (GetMenuItemInfo(*hMenu, iIndex, TRUE, &mii)) {
		if (mii.hSubMenu != NULL)
			DestroyMenu(mii.hSubMenu);
		RemoveMenu(*hMenu, iIndex, MF_BYPOSITION);
	}
}

void ValidateFilename(wchar_t *filename)
{
	wchar_t *p1 = filename;
	wchar_t szForbidden[] = L"\\/:*?\"<>|";
	while (*p1 != '\0') {
		if (wcschr(szForbidden, *p1))
			*p1 = '_';
		p1 += 1;
	}
}

int RestoreWindowPosition(HWND hwnd, MCONTACT hContact, bool bHide)
{
	int x = db_get_dw(hContact, CHAT_MODULE, "roomx", -1);
	if (x == -1)
		return 0;

	int y = (int)db_get_dw(hContact, CHAT_MODULE, "roomy", -1);
	int width = db_get_dw(hContact, CHAT_MODULE, "roomwidth", -1);
	int height = db_get_dw(hContact, CHAT_MODULE, "roomheight", -1);

	DWORD dwFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (bHide)
		dwFlags |= SWP_HIDEWINDOW;
	SetWindowPos(hwnd, NULL, x, y, width, height, dwFlags);
	return 1;
}
