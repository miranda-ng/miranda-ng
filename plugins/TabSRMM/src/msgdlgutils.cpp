/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
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
//
// Helper functions for the message dialog.

#include "stdafx.h"

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

#define EVENTTYPE_WAT_ANSWER            9602
#define EVENTTYPE_JABBER_CHATSTATES     2000
#define EVENTTYPE_JABBER_PRESENCE       2001

static int g_status_events[] = {
	EVENTTYPE_STATUSCHANGE,
	EVENTTYPE_WAT_ANSWER,
	EVENTTYPE_JABBER_CHATSTATES,
	EVENTTYPE_JABBER_PRESENCE
};

static int g_status_events_size = 0;

bool TSAPI IsStatusEvent(int eventType)
{
	if (g_status_events_size == 0)
		g_status_events_size = _countof(g_status_events);

	for (int i = 0; i < g_status_events_size; i++) {
		if (eventType == g_status_events[i])
			return true;
	}
	return false;
}

bool TSAPI IsCustomEvent(int eventType)
{
	if (eventType == EVENTTYPE_MESSAGE || eventType == EVENTTYPE_CONTACTS || eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST || eventType == EVENTTYPE_FILE)
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void TSAPI AddUnreadContact(MCONTACT hContact)
{
	if (!g_arUnreadWindows.find(HANDLE(hContact)))
		g_arUnreadWindows.insert(HANDLE(hContact));
}

/////////////////////////////////////////////////////////////////////////////////////////
// checking if theres's protected text at the point
// emulates EN_LINK WM_NOTIFY to parent to process links

const CLSID IID_ITextDocument = { 0x8CC497C0, 0xA1DF, 0x11CE, { 0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D } };

BOOL TSAPI CheckCustomLink(HWND hwndRich, POINT *ptClient, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bUrlNeeded)
{
	long res = 0, cnt = 0;
	long cpMin = 0, cpMax = 0;
	POINT ptEnd = { 0 };
	IRichEditOle *RichEditOle = nullptr;
	ITextDocument *TextDocument = nullptr;
	ITextRange *TextRange = nullptr;
	ITextFont *TextFont = nullptr;
	BOOL bIsCustomLink = FALSE;

	POINT pt = *ptClient;
	ClientToScreen(hwndRich, &pt);

	do {
		if (!SendMessage(hwndRich, EM_GETOLEINTERFACE, 0, (LPARAM)& RichEditOle)) break;
		if (RichEditOle->QueryInterface(IID_ITextDocument, (void **)& TextDocument) != S_OK) break;
		if (TextDocument->RangeFromPoint(pt.x, pt.y, &TextRange) != S_OK) break;

		TextRange->GetStart(&cpMin);
		cpMax = cpMin + 1;
		TextRange->SetEnd(cpMax);

		if (TextRange->GetFont(&TextFont) != S_OK)
			break;

		TextFont->GetProtected(&res);
		if (res != tomTrue)
			break;

		TextRange->GetPoint(tomEnd + TA_BOTTOM + TA_RIGHT, &ptEnd.x, &ptEnd.y);
		if (pt.x > ptEnd.x || pt.y > ptEnd.y)
			break;

		if (bUrlNeeded) {
			TextRange->GetStoryLength(&cnt);
			for (; cpMin > 0; cpMin--) {
				res = tomTrue;
				TextRange->SetIndex(tomCharacter, cpMin + 1, tomTrue);
				TextFont->GetProtected(&res);
				if (res != tomTrue) { cpMin++; break; }
			}
			for (cpMax--; cpMax < cnt; cpMax++) {
				res = tomTrue;
				TextRange->SetIndex(tomCharacter, cpMax + 1, tomTrue);
				TextFont->GetProtected(&res);
				if (res != tomTrue)
					break;
			}
		}

		bIsCustomLink = (cpMin < cpMax);
	} while (FALSE);

	if (TextFont) TextFont->Release();
	if (TextRange) TextRange->Release();
	if (TextDocument) TextDocument->Release();
	if (RichEditOle) RichEditOle->Release();

	if (bIsCustomLink) {
		ENLINK enlink = {};
		enlink.nmhdr.hwndFrom = hwndRich;
		enlink.nmhdr.idFrom = IDC_SRMM_LOG;
		enlink.nmhdr.code = EN_LINK;
		enlink.msg = uMsg;
		enlink.wParam = wParam;
		enlink.lParam = lParam;
		enlink.chrg.cpMin = cpMin;
		enlink.chrg.cpMax = cpMax;
		SendMessage(GetParent(hwndRich), WM_NOTIFY, IDC_SRMM_LOG, (LPARAM)& enlink);
	}
	return bIsCustomLink;
}

/////////////////////////////////////////////////////////////////////////////////////////
// reorder tabs within a container. bSavePos indicates whether the new position should
// be saved to the contacts db record (if so, the container will try to open the tab
// at the saved position later)

void TSAPI RearrangeTab(HWND hwndDlg, const CMsgDialog *dat, int iMode, BOOL bSavePos)
{
	if (dat == nullptr || !IsWindow(hwndDlg))
		return;

	HWND hwndTab = GetParent(hwndDlg);
	wchar_t oldText[512];

	TCITEM item = {};
	item.mask = TCIF_IMAGE | TCIF_TEXT | TCIF_PARAM;
	item.pszText = oldText;
	item.cchTextMax = _countof(oldText);
	TabCtrl_GetItem(hwndTab, dat->m_iTabID, &item);

	int newIndex = LOWORD(iMode);
	if (newIndex >= 0 && newIndex <= TabCtrl_GetItemCount(hwndTab)) {
		TabCtrl_DeleteItem(hwndTab, dat->m_iTabID);
		item.lParam = (LPARAM)hwndDlg;
		TabCtrl_InsertItem(hwndTab, newIndex, &item);
		dat->m_pContainer->UpdateTabs();
		ActivateTabFromHWND(hwndTab, hwndDlg);
		if (bSavePos)
			db_set_dw(dat->m_hContact, SRMSGMOD_T, "tabindex", newIndex * 100);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the save as file dialog (needed to set it to thumbnail view on Windows 2000
// or later

UINT_PTR CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
		break;

	case WM_NOTIFY:
		OPENFILENAMEA *ofn = (OPENFILENAMEA *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		HWND hwndParent = GetParent(hwnd);
		HWND hwndLv = FindWindowEx(hwndParent, nullptr, L"SHELLDLL_DefView", nullptr);

		if (hwndLv != nullptr && *((uint32_t *)(ofn->lCustData))) {
			SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
			*((uint32_t *)(ofn->lCustData)) = 0;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// provide user feedback via icons on tabs.Used to indicate "send in progress" or
// any error state.
//
// NOT used for typing notification feedback as this is handled directly from the
// MTN handler.

void TSAPI HandleIconFeedback(CMsgDialog *dat, HICON iIcon)
{
	TCITEM item = {};

	if (iIcon == (HICON)-1) { // restore status image
		if (dat->m_bErrorState)
			dat->m_hTabIcon = PluginConfig.g_iconErr;
		else
			dat->m_hTabIcon = dat->m_hTabStatusIcon;
	}
	else dat->m_hTabIcon = iIcon;

	item.iImage = 0;
	item.mask = TCIF_IMAGE;
	if (dat->m_pContainer->cfg.flags.m_bSideBar)
		dat->m_pContainer->m_pSideBar->updateSession(dat);
	else
		TabCtrl_SetItem(dat->m_pContainer->m_hwndTabs, dat->m_iTabID, &item);
}

/////////////////////////////////////////////////////////////////////////////////////////
// catches notifications from the AVS controls

void TSAPI ProcessAvatarChange(HWND hwnd, LPARAM lParam)
{
	if (((LPNMHDR)lParam)->code == NM_AVATAR_CHANGED) {
		HWND hwndDlg = GetParent(hwnd);
		CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
		if (!dat)
			return;

		dat->m_ace = Utils::loadAvatarFromAVS(dat->m_cache->getActiveContact());

		dat->GetAvatarVisibility();
		dat->ShowPicture(true);
		if (dat->m_pPanel.isActive())
			SendMessage(hwndDlg, WM_SIZE, 0, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// return value MUST be mir_free()'d by caller.

wchar_t* TSAPI QuoteText(const wchar_t *text)
{
	int outChar, lineChar;
	int iCharsPerLine = M.GetDword("quoteLineLength", 64);

	size_t bufSize = mir_wstrlen(text) + 23;
	wchar_t *strout = (wchar_t*)mir_alloc(bufSize * sizeof(wchar_t));
	int inChar = 0;
	int justDoneLineBreak = 1;
	for (outChar = 0, lineChar = 0; text[inChar];) {
		if (outChar >= (int)bufSize - 8) {
			bufSize += 20;
			strout = (wchar_t*)mir_realloc(strout, bufSize * sizeof(wchar_t));
		}
		if (justDoneLineBreak && text[inChar] != '\r' && text[inChar] != '\n') {
			strout[outChar++] = '>';
			strout[outChar++] = ' ';
			lineChar = 2;
		}
		if (lineChar == iCharsPerLine && text[inChar] != '\r' && text[inChar] != '\n') {
			int decreasedBy;
			for (decreasedBy = 0; lineChar > 10; lineChar--, inChar--, outChar--, decreasedBy++)
				if (strout[outChar] == ' ' || strout[outChar] == '\t' || strout[outChar] == '-') break;
			if (lineChar <= 10) {
				lineChar += decreasedBy;
				inChar += decreasedBy;
				outChar += decreasedBy;
			}
			else inChar++;
			strout[outChar++] = '\r';
			strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			continue;
		}
		strout[outChar++] = text[inChar];
		lineChar++;
		if (text[inChar] == '\n' || text[inChar] == '\r') {
			if (text[inChar] == '\r' && text[inChar + 1] != '\n')
				strout[outChar++] = '\n';
			justDoneLineBreak = 1;
			lineChar = 0;
		}
		else justDoneLineBreak = 0;
		inChar++;
	}
	strout[outChar++] = '\r';
	strout[outChar++] = '\n';
	strout[outChar] = 0;
	return strout;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool IsStringValidLink(wchar_t *pszText)
{
	if (pszText == nullptr)
		return false;

	if (pszText[0] == '\\' && pszText[1] == '\\')
		return true;

	if (mir_wstrlen(pszText) < 5 || wcschr(pszText, '"'))
		return false;

	if (towlower(pszText[0]) == 'w' && towlower(pszText[1]) == 'w' && towlower(pszText[2]) == 'w' && pszText[3] == '.' && iswalnum(pszText[4]))
		return true;

	return wcsstr(pszText, L"://") != nullptr;
}
