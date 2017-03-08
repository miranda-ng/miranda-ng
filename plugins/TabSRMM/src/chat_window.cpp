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
//
// This implements the group chat dialog window

#include "stdafx.h"

// externs...
extern LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern HRESULT(WINAPI *MyCloseThemeData)(HANDLE);

int g_cLinesPerPage = 0;
int g_iWheelCarryover = 0;

extern HMENU g_hMenu;

static HKL hkl = nullptr;
char szIndicators[] = { 0, '+', '%', '@', '!', '*' };

struct MESSAGESUBDATA
{
	time_t lastEnterTime;
	wchar_t *szSearchQuery;
	wchar_t *szSearchResult;
	BOOL   iSavedSpaces;
	SESSION_INFO *lastSession;
};

const CLSID IID_ITextDocument = { 0x8CC497C0, 0xA1DF, 0x11CE, { 0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D } };

/////////////////////////////////////////////////////////////////////////////////////////
// checking if theres's protected text at the point
// emulates EN_LINK WM_NOTIFY to parent to process links

static BOOL CheckCustomLink(HWND hwndDlg, POINT *ptClient, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL bUrlNeeded)
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
	ClientToScreen(hwndDlg, &pt);

	do {
		if (!SendMessage(hwndDlg, EM_GETOLEINTERFACE, 0, (LPARAM)&RichEditOle)) break;
		if (RichEditOle->QueryInterface(IID_ITextDocument, (void**)&TextDocument) != S_OK) break;
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
		ENLINK enlink = { 0 };
		enlink.nmhdr.hwndFrom = hwndDlg;
		enlink.nmhdr.idFrom = IDC_LOG;
		enlink.nmhdr.code = EN_LINK;
		enlink.msg = uMsg;
		enlink.wParam = wParam;
		enlink.lParam = lParam;
		enlink.chrg.cpMin = cpMin;
		enlink.chrg.cpMax = cpMax;
		SendMessage(GetParent(hwndDlg), WM_NOTIFY, IDC_LOG, (LPARAM)&enlink);
	}
	return bIsCustomLink;
}

bool IsStringValidLink(wchar_t *pszText)
{
	if (pszText == nullptr)
		return false;

	if (mir_wstrlen(pszText) < 5 || wcschr(pszText, '"'))
		return false;

	if (towlower(pszText[0]) == 'w' && towlower(pszText[1]) == 'w' && towlower(pszText[2]) == 'w' && pszText[3] == '.' && iswalnum(pszText[4]))
		return true;

	return wcsstr(pszText, L"://") != nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called whenever a group chat tab becomes active(either by switching tabs or activating a
// container window

void CChatRoomDlg::UpdateWindowState(UINT msg)
{
	if (si == nullptr)
		return;

	if (msg == WM_ACTIVATE) {
		if (m_pContainer->dwFlags & CNT_TRANSPARENCY) {
			DWORD trans = LOWORD(m_pContainer->settings->dwTransparency);
			SetLayeredWindowAttributes(m_pContainer->hwnd, CSkin::m_ContainerColorKey, (BYTE)trans, (m_pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
		}
	}

	if (m_hwndFilter) {
		POINT pt;
		GetCursorPos(&pt);

		RECT rcFilter;
		GetWindowRect(m_hwndFilter, &rcFilter);
		if (!PtInRect(&rcFilter, pt)) {
			SendMessage(m_hwndFilter, WM_CLOSE, 1, 1);
			m_hwndFilter = 0;
		}
	}

	if (m_bIsAutosizingInput && m_iInputAreaHeight == -1) {
		m_iInputAreaHeight = 0;
		m_message.SendMsg(EM_REQUESTRESIZE, 0, 0);
	}

	m_pPanel.dismissConfig();
	m_dwUnread = 0;
	if (m_pWnd) {
		m_pWnd->activateTab();
		m_pWnd->setOverlayIcon(0, true);
	}

	if (m_pContainer->hwndSaved == m_hwnd || m_bWasDeleted)
		return;

	m_pContainer->hwndSaved = m_hwnd;

	pci->SetActiveSession(si->ptszID, si->pszModule);
	m_hTabIcon = m_hTabStatusIcon;

	if (m_iTabID >= 0) {
		if (db_get_w(si->hContact, si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
		if (pcli->pfnGetEvent(si->hContact, 0))
			pcli->pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

		SendMessage(m_hwnd, GC_UPDATETITLE, 0, 1);
		m_dwTickLastEvent = 0;
		m_dwFlags &= ~MWF_DIVIDERSET;
		if (KillTimer(m_hwnd, TIMERID_FLASHWND) || m_iFlashIcon) {
			FlashTab(false);
			m_bCanFlashTab = FALSE;
			m_iFlashIcon = 0;
		}
		if (m_pContainer->dwFlashingStarted != 0) {
			FlashContainer(m_pContainer, 0, 0);
			m_pContainer->dwFlashingStarted = 0;
		}
		m_pContainer->dwFlags &= ~CNT_NEED_UPDATETITLE;

		if (m_dwFlags & MWF_NEEDCHECKSIZE)
			PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

		if (PluginConfig.m_bAutoLocaleSupport) {
			if (hkl == 0)
				DM_LoadLocale();
			else
				SendMessage(m_hwnd, DM_SETLOCALE, 0, 0);
		}
		SetFocus(GetDlgItem(m_hwnd, IDC_MESSAGE));
		m_dwLastActivity = GetTickCount();
		m_pContainer->dwLastActivity = m_dwLastActivity;
		m_pContainer->MenuBar->configureMenu();
		UpdateTrayMenuState(this, FALSE);
		DM_SetDBButtonStates();

		if (m_dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
			m_dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
			ShowWindow(m_pContainer->hwnd, SW_RESTORE);
			PostMessage(m_hwnd, DM_SPLITTERGLOBALEVENT, m_wParam, m_lParam);
			PostMessage(m_hwnd, WM_SIZE, 0, 0);
			m_wParam = m_lParam = 0;
		}
	}
	BB_SetButtonsPos();
	if (M.isAero())
		InvalidateRect(m_hwndParent, nullptr, FALSE);
	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->setActiveItem(this);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////
// resizer callback for the group chat session window.Called from Mirandas dialog
// resizing service

int CChatRoomDlg::Resizer(UTILRESIZECONTROL *urc)
{
	bool bToolbar = !(m_pContainer->dwFlags & CNT_HIDETOOLBAR);
	bool bBottomToolbar = (m_pContainer->dwFlags & CNT_BOTTOMTOOLBAR) != 0;
	bool bNick = si->iType != GCW_SERVER && si->bNicklistEnabled;
	bool bInfoPanel = m_pPanel.isActive();
	int  panelHeight = m_pPanel.getHeight() + 1;
	int  iSplitterX = m_pContainer->settings->iSplitterX;

	RECT rcTabs;
	GetClientRect(m_hwnd, &rcTabs);

	if (m_bIsAutosizingInput)
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERY, SW_HIDE);

	if (si->iType != GCW_SERVER) {
		Utils::showDlgControl(m_hwnd, IDC_LIST, si->bNicklistEnabled ? SW_SHOW : SW_HIDE);
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERX, si->bNicklistEnabled ? SW_SHOW : SW_HIDE);

		m_btnNickList.Enable(true);
		m_btnFilter.Enable(true);
		if (si->iType == GCW_CHATROOM) {
			MODULEINFO *tmp = pci->MM_FindModule(si->pszModule);
			if (tmp)
				m_btnChannelMgr.Enable(tmp->bChanMgr);
		}
	}
	else {
		Utils::showDlgControl(m_hwnd, IDC_LIST, SW_HIDE);
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERX, SW_HIDE);
	}

	if (si->iType == GCW_SERVER) {
		m_btnNickList.Enable(false);
		m_btnFilter.Enable(false);
		m_btnChannelMgr.Enable(false);
	}

	switch (urc->wId) {
	case IDC_PANELSPLITTER:
		urc->rcItem.bottom = panelHeight;
		urc->rcItem.top = panelHeight - 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_LOG:
		urc->rcItem.top = 0;
		urc->rcItem.left = 0;
		urc->rcItem.right = bNick ? urc->dlgNewSize.cx - iSplitterX : urc->dlgNewSize.cx;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		if (!bToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKHISTORY];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_LIST:
		urc->rcItem.top = 0;
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.left = urc->dlgNewSize.cx - iSplitterX + 2;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		if (!bToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKUSERLIST];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERX:
		urc->rcItem.right = urc->dlgNewSize.cx - iSplitterX + 2;
		urc->rcItem.left = urc->dlgNewSize.cx - iSplitterX;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		if (!bToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		urc->rcItem.top = 0;
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERY:
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top = urc->dlgNewSize.cy - m_iSplitterY + DPISCALEY_S(23);
		urc->rcItem.bottom = urc->rcItem.top + DPISCALEY_S(2);
		urc->rcItem.left = 0;
		urc->rcItem.bottom++;
		urc->rcItem.top++;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_MESSAGE:
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.top = urc->dlgNewSize.cy - m_iSplitterY + 3 + DPISCALEY_S(23);
		urc->rcItem.bottom = urc->dlgNewSize.cy;
		if (bBottomToolbar && bToolbar)
			urc->rcItem.bottom -= DPISCALEY_S(22);

		if (m_bIsAutosizingInput)
			urc->rcItem.top -= DPISCALEY_S(1);

		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKINPUTAREA];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the message input control(a richedit text control)

static bool TabAutoComplete(HWND hwnd, MESSAGESUBDATA *dat, SESSION_INFO *si)
{
	LRESULT lResult = (LRESULT)SendMessage(hwnd, EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	SendMessage(hwnd, EM_SETSEL, end, end);

	GETTEXTEX gt = { 0 };
	gt.codepage = 1200;
	gt.flags = GTL_DEFAULT | GTL_PRECISE;
	int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gt, 0);
	if (iLen <= 0)
		return false;

	bool isTopic = false, isRoom = false;
	wchar_t *pszName = nullptr;
	wchar_t *pszText = (wchar_t*)mir_calloc((iLen + 10) * sizeof(wchar_t));

	gt.flags = GT_DEFAULT;
	gt.cb = (iLen + 9) * sizeof(wchar_t);
	SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);

	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (dat->szSearchResult != nullptr) {
		int cbResult = (int)mir_wstrlen(dat->szSearchResult);
		if (start >= cbResult && !wcsnicmp(dat->szSearchResult, pszText + start - cbResult, cbResult)) {
			start -= cbResult;
			goto LBL_SkipEnd;
		}
	}

	while (start > 0 && pszText[start - 1] != ' ' && pszText[start - 1] != 13 && pszText[start - 1] != VK_TAB)
		start--;

LBL_SkipEnd:
	while (end < iLen && pszText[end] != ' ' && pszText[end] != 13 && pszText[end - 1] != VK_TAB)
		end++;

	if (pszText[start] == '#')
		isRoom = TRUE;
	else {
		int topicStart = start;
		while (topicStart > 0 && (pszText[topicStart - 1] == ' ' || pszText[topicStart - 1] == 13 || pszText[topicStart - 1] == VK_TAB))
			topicStart--;
		if (topicStart > 5 && wcsstr(&pszText[topicStart - 6], L"/topic") == &pszText[topicStart - 6])
			isTopic = TRUE;
	}
	if (dat->szSearchQuery == nullptr) {
		dat->szSearchQuery = mir_wstrndup(pszText + start, end - start);
		dat->szSearchResult = mir_wstrdup(dat->szSearchQuery);
		dat->lastSession = nullptr;
	}
	if (isTopic)
		pszName = si->ptszTopic;
	else if (isRoom) {
		dat->lastSession = SM_FindSessionAutoComplete(si->pszModule, si, dat->lastSession, dat->szSearchQuery, dat->szSearchResult);
		if (dat->lastSession != nullptr)
			pszName = dat->lastSession->ptszName;
	}
	else pszName = pci->UM_FindUserAutoComplete(si->pUsers, dat->szSearchQuery, dat->szSearchResult);

	replaceStrW(dat->szSearchResult, nullptr);

	if (pszName != nullptr) {
		dat->szSearchResult = mir_wstrdup(pszName);
		if (end != start) {
			ptrW szReplace;
			if (!isRoom && !isTopic && g_Settings.bAddColonToAutoComplete && start == 0) {
				szReplace = (wchar_t*)mir_alloc((mir_wstrlen(pszName) + 4) * sizeof(wchar_t));
				mir_wstrcpy(szReplace, pszName);
				mir_wstrcat(szReplace, g_Settings.bUseCommaAsColon ? L", " : L": ");
				pszName = szReplace;
			}
			SendMessage(hwnd, EM_SETSEL, start, end);
			SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)pszName);
		}
		return true;
	}

	if (end != start) {
		SendMessage(hwnd, EM_SETSEL, start, end);
		SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)dat->szSearchQuery);
	}
	replaceStrW(dat->szSearchQuery, nullptr);
	return false;
}

static LRESULT CALLBACK MessageSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	CChatRoomDlg *mwdat = (CChatRoomDlg*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
	MESSAGESUBDATA *dat = (MESSAGESUBDATA*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (mwdat != nullptr)
		if (mwdat->m_fkeyProcessed && (msg == WM_KEYUP)) {
			GetKeyboardState(mwdat->kstate);
			if (!(mwdat->kstate[VK_CONTROL] & 0x80) && !(mwdat->kstate[VK_SHIFT] & 0x80))
				mwdat->m_fkeyProcessed = false;
			return 0;
		}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKINPUTAREA, msg, wParam, lParam, MessageSubclassProc);

	case EM_SUBCLASSED:
		dat = (MESSAGESUBDATA*)mir_calloc(sizeof(MESSAGESUBDATA));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		return 0;

	case WM_CONTEXTMENU:
		POINT pt;
		GetCursorPos(&pt);
		{
			HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 2);
			RemoveMenu(hSubMenu, 9, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 4, MF_BYPOSITION);

			MODULEINFO *mi = pci->MM_FindModule(mwdat->si->pszModule);
			EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | ((mi && mi->bBold) ? MF_ENABLED : MF_GRAYED));
			TranslateMenu(hSubMenu);

			CHARRANGE sel, all = { 0, -1 };
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
			}

			MessageWindowPopupData mwpd = { sizeof(mwpd) };
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = mwdat->m_hContact;
			mwpd.hwnd = hwnd;
			mwpd.hMenu = hSubMenu;
			mwpd.pt = pt;
			NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);

			int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, GetParent(hwnd), nullptr);

			mwpd.selection = iSelection;
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(PluginConfig.m_event_MsgPopup, 0, (LPARAM)&mwpd);

			switch (iSelection) {
			case IDM_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;
			case IDM_CUT:
				SendMessage(hwnd, WM_CUT, 0, 0);
				break;
			case IDM_PASTE:
			case IDM_PASTEFORMATTED:
				SendMessage(hwnd, EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_UNICODETEXT : 0, 0);
				break;
			case IDM_COPYALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
				SendMessage(hwnd, WM_COPY, 0, 0);
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
				break;
			case IDM_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
				break;
			}
			DestroyMenu(hMenu);
		}
		return TRUE;

	case WM_MOUSEWHEEL:
		if (mwdat->DM_MouseWheelHandler(wParam, lParam) == 0)
			return 0;

		dat->lastEnterTime = 0;
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_MESSAGE);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		mwdat->m_fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_MESSAGE)) {
			mwdat->m_fkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (mwdat->m_fkeyProcessed) {
			mwdat->m_fkeyProcessed = false;						// preceeding key event has been processed by miranda hotkey service
			return 0;
		}

		if ((wParam >= '0' && wParam <= '9') && (GetKeyState(VK_MENU) & 0x8000)) {       // ALT-1 -> ALT-0 direct tab selection
			BYTE bChar = (BYTE)wParam;
			int iIndex = (bChar == '0') ? 10 : bChar - (BYTE)'0';
			SendMessage(mwdat->m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
			return 0;
		}
		break;

	case WM_CHAR:
		bool isShift, isAlt, isCtrl;
		mwdat->KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt &&!isCtrl&&!(mwdat->m_pContainer->dwFlags & CNT_NOSOUND) && wParam != VK_ESCAPE&&!(wParam == VK_TAB && PluginConfig.m_bAllowTab))
			SkinPlaySound("SoundOnTyping");

		if (isCtrl && !isAlt && !isShift) {
			MODULEINFO *mi = pci->MM_FindModule(mwdat->si->pszModule);
			if (mi == nullptr)
				return 0;

			switch (wParam) {
			case 0x09: 		// ctrl-i (italics)
				if (mi->bItalics) {
					CheckDlgButton(hwndParent, IDC_ITALICS, IsDlgButtonChecked(hwndParent, IDC_ITALICS) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				}
				return 0;
			case 0x02:		// ctrl-b (bold)
				if (mi->bBold) {
					CheckDlgButton(hwndParent, IDC_BOLD, IsDlgButtonChecked(hwndParent, IDC_BOLD) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
				}
				return 0;
			case 0x20:		// ctrl-space clear formatting
				if (mi->bBold && mi->bItalics && mi->bUnderline) {
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_UNCHECKED);
					CheckDlgButton(hwndParent, IDC_COLOR, BST_UNCHECKED);
					CheckDlgButton(hwndParent, IDC_BOLD, BST_UNCHECKED);
					CheckDlgButton(hwndParent, IDC_UNDERLINE, BST_UNCHECKED);
					CheckDlgButton(hwndParent, IDC_ITALICS, BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BOLD, 0), 0);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_ITALICS, 0), 0);
				}
				return 0;
			case 0x0c:		// ctrl-l background color
				if (mi->bBkgColor) {
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, IsDlgButtonChecked(hwndParent, IDC_BKGCOLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_BKGCOLOR, 0), 0);
				}
				return 0;
			case 0x15:		// ctrl-u underlined
				if (mi->bUnderline) {
					CheckDlgButton(hwndParent, IDC_UNDERLINE, IsDlgButtonChecked(hwndParent, IDC_UNDERLINE) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_UNDERLINE, 0), 0);
				}
				return 0;	// ctrl-k color
			case 0x0b:
				if (mi->bColor) {
					CheckDlgButton(hwndParent, IDC_COLOR, IsDlgButtonChecked(hwndParent, IDC_COLOR) == BST_UNCHECKED ? BST_CHECKED : BST_UNCHECKED);
					SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_COLOR, 0), 0);
				}
				return 0;
			case 0x17:
				PostMessage(hwndParent, WM_CLOSE, 0, 1);
				return 0;
			}
		}
		break;

	case WM_KEYDOWN:
		mwdat->KbdState(isShift, isCtrl, isAlt);

		// sound on typing..
		if (PluginConfig.m_bSoundOnTyping && !isAlt && wParam == VK_DELETE)
			SkinPlaySound("SoundOnTyping");

		if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
			mwdat->m_fInsertMode = !mwdat->m_fInsertMode;
			SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);
		}
		if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
			SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM)hwnd);

		if (isCtrl && isAlt && !isShift) {
			switch (wParam) {
			case VK_UP:
			case VK_DOWN:
			case VK_PRIOR:
			case VK_NEXT:
			case VK_HOME:
			case VK_END:
				WPARAM wp = 0;

				if (wParam == VK_UP)
					wp = MAKEWPARAM(SB_LINEUP, 0);
				else if (wParam == VK_PRIOR)
					wp = MAKEWPARAM(SB_PAGEUP, 0);
				else if (wParam == VK_NEXT)
					wp = MAKEWPARAM(SB_PAGEDOWN, 0);
				else if (wParam == VK_HOME)
					wp = MAKEWPARAM(SB_TOP, 0);
				else if (wParam == VK_END) {
					mwdat->DM_ScrollToBottom(0, 0);
					return 0;
				}
				else if (wParam == VK_DOWN)
					wp = MAKEWPARAM(SB_LINEDOWN, 0);

				SendDlgItemMessage(hwndParent, IDC_LOG, WM_VSCROLL, wp, 0);
				return 0;
			}
		}

		if (wParam == VK_RETURN) {
			if (isShift) {
				if (PluginConfig.m_bSendOnShiftEnter) {
					PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
					return 0;
				}
				break;
			}
			if ((isCtrl && !isShift) ^ (0 != PluginConfig.m_bSendOnEnter)) {
				PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (!PluginConfig.m_bSendOnEnter && !PluginConfig.m_bSendOnDblEnter)
				break;
			if (isCtrl)
				break;

			if (PluginConfig.m_bSendOnDblEnter) {
				if (dat->lastEnterTime + 2 < time(nullptr)) {
					dat->lastEnterTime = time(nullptr);
					break;
				}

				SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
				SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
				PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
				return 0;
			}
			PostMessage(hwndParent, WM_COMMAND, IDOK, 0);
			return 0;
		}
		else dat->lastEnterTime = 0;

		if ((wParam == VK_NEXT && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && !isShift)) { // CTRL-TAB (switch tab/window)
			SendMessage(mwdat->m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
			return TRUE;
		}

		if ((wParam == VK_PRIOR && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && isShift)) { // CTRL_SHIFT-TAB (switch tab/window)
			SendMessage(mwdat->m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
			return TRUE;
		}
		if (wParam == VK_TAB && !isCtrl && !isShift) {    //tab-autocomplete
			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
			bool fCompleted = TabAutoComplete(hwnd, dat, mwdat->si);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
			if (!fCompleted && !PluginConfig.m_bAllowTab) {
				if ((GetSendButtonState(mwdat->GetHwnd()) != PBS_DISABLED))
					SetFocus(GetDlgItem(mwdat->GetHwnd(), IDOK));
				else
					SetFocus(GetDlgItem(mwdat->GetHwnd(), IDC_LOG));
			}
			return 0;
		}
		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			mir_free(dat->szSearchQuery);
			dat->szSearchQuery = nullptr;
			mir_free(dat->szSearchResult);
			dat->szSearchResult = nullptr;
		}

		if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
			SendMessage(hwndParent, WM_COMMAND, MAKEWPARAM(IDC_CLOSE, BN_CLICKED), 0);
			return 0;
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			HWND htemp = hwndParent;
			SendDlgItemMessage(htemp, IDC_LOG, msg, wParam, lParam);
			dat->lastEnterTime = 0;
			return 0;
		}

		if (wParam == VK_UP && isCtrl && !isAlt) {
			char *lpPrevCmd = pci->SM_GetPrevCommand(mwdat->si->ptszID, mwdat->si->pszModule);

			if (!mwdat->si->lpCurrentCommand || !mwdat->si->lpCurrentCommand->last) {
				// Next command is not defined. It means currently entered text is not saved in the history and it
				// need to be saved in the window context.
				char *enteredText = Message_GetFromStream(hwndParent);
				if (mwdat->m_enteredText)
					mir_free(mwdat->m_enteredText);

				mwdat->m_enteredText = enteredText;
			}

			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

			LOGFONTA lf;
			LoadLogfont(MSGFONTID_MESSAGEAREA, &lf, nullptr, FONTMODULE);

			SETTEXTEX ste;
			ste.flags = ST_DEFAULT;
			ste.codepage = CP_ACP;
			if (lpPrevCmd)
				SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
			else
				SetWindowText(hwnd, L"");

			GETTEXTLENGTHEX gtl = { 0 };
			gtl.flags = GTL_PRECISE;
			gtl.codepage = CP_ACP;
			int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
			SendMessage(hwnd, EM_SETSEL, iLen, iLen);
			dat->lastEnterTime = 0;
			return 0;
		}

		if (wParam == VK_DOWN && isCtrl && !isAlt) {
			SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

			SETTEXTEX ste;
			ste.flags = ST_DEFAULT;
			ste.codepage = CP_ACP;

			char *lpPrevCmd = pci->SM_GetNextCommand(mwdat->si->ptszID, mwdat->si->pszModule);
			if (lpPrevCmd)
				SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
			else if (mwdat->m_enteredText) {
				// If we cannot load the message from history, load the last edited text.
				SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)mwdat->m_enteredText);
				mir_free(mwdat->m_enteredText);
				mwdat->m_enteredText = nullptr;
			}

			GETTEXTLENGTHEX gtl = { 0 };
			gtl.flags = GTL_PRECISE;
			gtl.codepage = CP_ACP;
			int iLen = SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			SendMessage(hwnd, EM_SCROLLCARET, 0, 0);
			SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
			RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE);
			SendMessage(hwnd, EM_SETSEL, iLen, iLen);
			dat->lastEnterTime = 0;
			return 0;
		}
		// fall through

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		COLORREF cr;
		LoadLogfont(MSGFONTID_MESSAGEAREA, nullptr, &cr, FONTMODULE);

		CHARFORMAT2 cf;
		cf.cbSize = sizeof(CHARFORMAT2);
		cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR | CFM_UNDERLINETYPE;
		cf.dwEffects = 0;
		SendMessage(hwnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		{
			MODULEINFO *mi = pci->MM_FindModule(mwdat->si->pszModule);
			if (mi == nullptr)
				break;

			if (mi->bColor) {
				int index = Chat_GetColorIndex(mwdat->si->pszModule, cf.crTextColor);
				UINT u = IsDlgButtonChecked(GetParent(hwnd), IDC_COLOR);

				if (index >= 0) {
					mwdat->si->bFGSet = true;
					mwdat->si->iFG = index;
				}

				if (u == BST_UNCHECKED && cf.crTextColor != cr)
					CheckDlgButton(hwndParent, IDC_COLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crTextColor == cr)
					CheckDlgButton(hwndParent, IDC_COLOR, BST_UNCHECKED);
			}

			if (mi->bBkgColor) {
				int index = Chat_GetColorIndex(mwdat->si->pszModule, cf.crBackColor);
				COLORREF crB = (COLORREF)M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);
				UINT u = IsDlgButtonChecked(hwndParent, IDC_BKGCOLOR);

				if (index >= 0) {
					mwdat->si->bBGSet = true;
					mwdat->si->iBG = index;
				}

				if (u == BST_UNCHECKED && cf.crBackColor != crB)
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_CHECKED);
				else if (u == BST_CHECKED && cf.crBackColor == crB)
					CheckDlgButton(hwndParent, IDC_BKGCOLOR, BST_UNCHECKED);
			}

			if (mi->bBold) {
				UINT u = IsDlgButtonChecked(hwndParent, IDC_BOLD);
				UINT u2 = cf.dwEffects;
				u2 &= CFE_BOLD;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndParent, IDC_BOLD, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndParent, IDC_BOLD, BST_UNCHECKED);
			}

			if (mi->bItalics) {
				UINT u = IsDlgButtonChecked(hwndParent, IDC_ITALICS);
				UINT u2 = cf.dwEffects;
				u2 &= CFE_ITALIC;
				if (u == BST_UNCHECKED && u2)
					CheckDlgButton(hwndParent, IDC_ITALICS, BST_CHECKED);
				else if (u == BST_CHECKED && u2 == 0)
					CheckDlgButton(hwndParent, IDC_ITALICS, BST_UNCHECKED);
			}

			if (mi->bUnderline) {
				UINT u = IsDlgButtonChecked(hwndParent, IDC_UNDERLINE);
				if (cf.dwEffects & CFE_UNDERLINE && (cf.bUnderlineType & CFU_UNDERLINE || cf.bUnderlineType & CFU_UNDERLINEWORD)) {
					if (u == BST_UNCHECKED)
						CheckDlgButton(hwndParent, IDC_UNDERLINE, BST_CHECKED);
				}
				else {
					if (u == BST_CHECKED)
						CheckDlgButton(hwndParent, IDC_UNDERLINE, BST_UNCHECKED);
				}
			}
		}
		break;

	case WM_INPUTLANGCHANGE:
		if (PluginConfig.m_bAutoLocaleSupport && GetFocus() == hwnd && mwdat->m_pContainer->hwndActive == hwndParent && GetForegroundWindow() == mwdat->m_pContainer->hwnd && GetActiveWindow() == mwdat->m_pContainer->hwnd) {
			mwdat->DM_SaveLocale(wParam, lParam);
			SendMessage(hwnd, EM_SETLANGOPTIONS, 0, (LPARAM)SendMessage(hwnd, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
			return 1;
		}
		break;

	case WM_ERASEBKGND:
		return !CSkin::m_skinEnabled;

	case WM_DESTROY:
		mir_free(dat);
	}

	return mir_callNextSubclass(hwnd, MessageSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the message filter dialog (set and configure event filters for the 
// current session

static UINT _eventorder[] =
{
	GC_EVENT_ACTION,
	GC_EVENT_MESSAGE,
	GC_EVENT_NICK,
	GC_EVENT_JOIN,
	GC_EVENT_PART,
	GC_EVENT_TOPIC,
	GC_EVENT_ADDSTATUS,
	GC_EVENT_INFORMATION,
	GC_EVENT_QUIT,
	GC_EVENT_KICK,
	GC_EVENT_NOTICE
};

static INT_PTR CALLBACK FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
	case WM_INITDIALOG:
		si = (SESSION_INFO*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)si);
		{
			DWORD dwMask = db_get_dw(si->hContact, CHAT_MODULE, "FilterMask", 0);
			DWORD dwFlags = db_get_dw(si->hContact, CHAT_MODULE, "FilterFlags", 0);

			DWORD dwPopupMask = db_get_dw(si->hContact, CHAT_MODULE, "PopupMask", 0);
			DWORD dwPopupFlags = db_get_dw(si->hContact, CHAT_MODULE, "PopupFlags", 0);

			DWORD dwTrayMask = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconMask", 0);
			DWORD dwTrayFlags = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", 0);

			for (int i = 0; i < _countof(_eventorder); i++) {
				CheckDlgButton(hwndDlg, IDC_1 + i, dwMask & _eventorder[i] ? (dwFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
				CheckDlgButton(hwndDlg, IDC_P1 + i, dwPopupMask & _eventorder[i] ? (dwPopupFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
				CheckDlgButton(hwndDlg, IDC_T1 + i, dwTrayMask & _eventorder[i] ? (dwTrayFlags & _eventorder[i] ? BST_CHECKED : BST_UNCHECKED) : BST_INDETERMINATE);
			}
		}
		return FALSE;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, RGB(60, 60, 150));
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_CLOSE:
		if (wParam == 1 && lParam == 1) {
			int iFlags = 0, i;
			DWORD dwMask = 0;

			for (i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}

			if (iFlags&GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (si) {
				if (dwMask == 0) {
					db_unset(si->hContact, CHAT_MODULE, "FilterFlags");
					db_unset(si->hContact, CHAT_MODULE, "FilterMask");
				}
				else {
					db_set_dw(si->hContact, CHAT_MODULE, "FilterFlags", iFlags);
					db_set_dw(si->hContact, CHAT_MODULE, "FilterMask", dwMask);
				}
			}

			dwMask = iFlags = 0;

			for (i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_P1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (si) {
				if (dwMask == 0) {
					db_unset(si->hContact, CHAT_MODULE, "PopupFlags");
					db_unset(si->hContact, CHAT_MODULE, "PopupMask");
				}
				else {
					db_set_dw(si->hContact, CHAT_MODULE, "PopupFlags", iFlags);
					db_set_dw(si->hContact, CHAT_MODULE, "PopupMask", dwMask);
				}
			}

			dwMask = iFlags = 0;

			for (i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_T1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}
			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (si) {
				if (dwMask == 0) {
					db_unset(si->hContact, CHAT_MODULE, "TrayIconFlags");
					db_unset(si->hContact, CHAT_MODULE, "TrayIconMask");
				}
				else {
					db_set_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", iFlags);
					db_set_dw(si->hContact, CHAT_MODULE, "TrayIconMask", dwMask);
				}
				Chat_SetFilters(si);
				SendMessage(si->hWnd, GC_CHANGEFILTERFLAG, 0, iFlags);
				if (si->bFilterEnabled)
					SendMessage(si->hWnd, GC_REDRAWLOG, 0, 0);
			}
		}
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclass for some tool bar buttons which must perform special actions
// on right click.

static LRESULT CALLBACK ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (M.GetByte(CHAT_MODULE, "RightClickFilter", 0) != 0) {
			HWND hwndParent = GetParent(hwnd);
			switch (GetDlgCtrlID(hwnd)) {
			case IDC_FILTER:
				SendMessage(hwndParent, GC_SHOWFILTERMENU, 0, 0);
				break;
			case IDC_COLOR:
				SendMessage(hwndParent, GC_SHOWCOLORCHOOSER, 0, IDC_COLOR);
				break;
			case IDC_BKGCOLOR:
				SendMessage(hwndParent, GC_SHOWCOLORCHOOSER, 0, IDC_BKGCOLOR);
			}
		}
		break;
	}

	return mir_callNextSubclass(hwnd, ButtonSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the message history display(rich edit control in which the chat history appears)

static LRESULT CALLBACK LogSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	CChatRoomDlg *mwdat = (CChatRoomDlg*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, LogSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, LogSubclassProc);

	case WM_COPY:
		return Utils::WMCopyHandler(hwnd, LogSubclassProc, msg, wParam, lParam);

	case WM_SETCURSOR:
		if (g_Settings.bClickableNicks && (LOWORD(lParam) == HTCLIENT)) {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);
			if (CheckCustomLink(hwnd, &pt, msg, wParam, lParam, FALSE)) return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		if (g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(hwnd, &pt, msg, wParam, lParam, TRUE);
		}
		break;

	case WM_LBUTTONUP:
		if (g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(hwnd, &pt, msg, wParam, lParam, TRUE);
		}
		if (M.GetByte("autocopy", 1)) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				SendMessage(hwnd, WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
				SetFocus(GetDlgItem(hwndParent, IDC_MESSAGE));
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(hwndParent, WM_CLOSE, 0, 1);
			return TRUE;
		}
		if (wParam == VK_INSERT && GetKeyState(VK_CONTROL) & 0x8000)
			return Utils::WMCopyHandler(hwnd, LogSubclassProc, msg, wParam, lParam);
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		mwdat->m_fkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(hwnd, msg, wParam, lParam, IDC_LOG)) {
			mwdat->m_fkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (mwdat->m_fkeyProcessed) {
			mwdat->m_fkeyProcessed = false;
			return 0;
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			CHARRANGE sel;
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax;
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&sel);
			}
		}
		break;

	case WM_CHAR:
		bool isCtrl, isShift, isAlt;
		mwdat->KbdState(isShift, isCtrl, isAlt);
		if (wParam == 0x03 && isCtrl) // Ctrl+C
			return Utils::WMCopyHandler(hwnd, LogSubclassProc, msg, wParam, lParam);

		SetFocus(GetDlgItem(hwndParent, IDC_MESSAGE));
		SendDlgItemMessage(hwndParent, IDC_MESSAGE, WM_CHAR, wParam, lParam);
		break;
	}

	return mir_callNextSubclass(hwnd, LogSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the nickname list control.It is an ownerdrawn listbox

static LRESULT CALLBACK NicklistSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hwnd);
	CChatRoomDlg *dat = (CChatRoomDlg*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
	case WM_NCCALCSIZE:
		if (CSkin::m_DisableScrollbars) {
			RECT lpRect;
			GetClientRect(hwnd, &lpRect);
			LONG itemHeight = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
			g_cLinesPerPage = (lpRect.bottom - lpRect.top) / itemHeight;
		}
		return CSkin::NcCalcRichEditFrame(hwnd, dat, ID_EXTBKUSERLIST, msg, wParam, lParam, NicklistSubclassProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(hwnd, dat, ID_EXTBKUSERLIST, msg, wParam, lParam, NicklistSubclassProc);

	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc) {
				int index = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				if (index == LB_ERR || dat->si->nUsersInNicklist <= 0)
					return 0;

				int items = dat->si->nUsersInNicklist - index;
				int height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);

				if (height != LB_ERR) {
					RECT rc = { 0 };
					GetClientRect(hwnd, &rc);

					if (rc.bottom - rc.top > items * height) {
						rc.top = items * height;
						FillRect(dc, &rc, pci->hListBkgBrush);
					}
				}
			}
		}
		return 1;

	case WM_MOUSEWHEEL:
		if (CSkin::m_DisableScrollbars)	{
			UINT uScroll;
			short zDelta = (short)HIWORD(wParam);
			if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScroll, 0))
				uScroll = 3;    /* default value */

			if (uScroll == WHEEL_PAGESCROLL)
				uScroll = g_cLinesPerPage;
			if (uScroll == 0)
				return 0;

			zDelta += g_iWheelCarryover;    /* Accumulate wheel motion */

			int dLines = zDelta * (int)uScroll / WHEEL_DELTA;

			//Record the unused portion as the next carryover.
			g_iWheelCarryover = zDelta - dLines * WHEEL_DELTA / (int)uScroll;

			// scrolling.
			while (abs(dLines)) {
				if (dLines > 0) {
					SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
					dLines--;
				}
				else {
					SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
					dLines++;
				}
			}
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(hwndParent, WM_CLOSE, 0, 1);
			return TRUE;
		}

		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT ||
			wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
			if (dat) {
				dat->m_wszSearch[0] = 0;
				dat->m_iSearchItem = -1;
			}
		}
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		if (dat) { // set/kill focus invalidates incremental search status
			dat->m_wszSearch[0] = 0;
			dat->m_iSearchItem = -1;
		}
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		// simple incremental search for the user (nick) - list control
		// typing esc or movement keys will clear the current search string
		if (dat) {
			if (wParam == 27 && dat->m_wszSearch[0]) { // escape - reset everything
				dat->m_wszSearch[0] = 0;
				dat->m_iSearchItem = -1;
				break;
			}
			if (wParam == '\b' && dat->m_wszSearch[0])					// backspace
				dat->m_wszSearch[mir_wstrlen(dat->m_wszSearch) - 1] = '\0';
			else if (wParam < ' ')
				break;
			else {
				if (mir_wstrlen(dat->m_wszSearch) >= _countof(dat->m_wszSearch) - 2) {
					MessageBeep(MB_OK);
					break;
				}
				wchar_t szNew[2];
				szNew[0] = (wchar_t)wParam;
				szNew[1] = '\0';
				mir_wstrcat(dat->m_wszSearch, szNew);
			}
			if (dat->m_wszSearch[0]) {
				// iterate over the (sorted) list of nicknames and search for the
				// string we have
				int i, iItems = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
				for (i = 0; i < iItems; i++) {
					USERINFO *ui = pci->UM_FindUserFromIndex(dat->si->pUsers, i);
					if (ui) {
						if (!wcsnicmp(ui->pszNick, dat->m_wszSearch, mir_wstrlen(dat->m_wszSearch))) {
							SendMessage(hwnd, LB_SETSEL, FALSE, -1);
							SendMessage(hwnd, LB_SETSEL, TRUE, i);
							dat->m_iSearchItem = i;
							InvalidateRect(hwnd, nullptr, FALSE);
							return 0;
						}
					}
				}
				if (i == iItems) {
					MessageBeep(MB_OK);
					dat->m_wszSearch[mir_wstrlen(dat->m_wszSearch) - 1] = '\0';
					return 0;
				}
			}
		}
		break;

	case WM_RBUTTONDOWN:
		{
			int iCounts = SendMessage(hwnd, LB_GETSELCOUNT, 0, 0);
			if (iCounts != LB_ERR && iCounts > 1)
				return 0;
			SendMessage(hwnd, WM_LBUTTONDOWN, wParam, lParam);
		}
		break;

	case WM_RBUTTONUP:
		SendMessage(hwnd, WM_LBUTTONUP, wParam, lParam);
		break;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU)
				return Menu_MeasureItem(lParam);
		}
		return FALSE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU)
				return Menu_DrawItem(lParam);
		}
		return FALSE;

	case WM_CONTEXTMENU:
		{
			SESSION_INFO *si = dat->si;
			if (si == nullptr)
				break;

			int height = 0;
			TVHITTESTINFO hti;
			hti.pt.x = GET_X_LPARAM(lParam);
			hti.pt.y = GET_Y_LPARAM(lParam);
			if (hti.pt.x == -1 && hti.pt.y == -1) {
				int index = SendMessage(hwnd, LB_GETCURSEL, 0, 0);
				int top = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
				height = SendMessage(hwnd, LB_GETITEMHEIGHT, 0, 0);
				hti.pt.x = 4;
				hti.pt.y = (index - top) * height + 1;
			}
			else ScreenToClient(hwnd, &hti.pt);

			int item = (DWORD)(SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
			if (HIWORD(item) == 1)
				item = (DWORD)(-1);
			else
				item &= 0xFFFF;

			USERINFO *ui = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, item);
			if (ui) {
				HMENU hMenu = 0;
				USERINFO uinew;
				memcpy(&uinew, ui, sizeof(USERINFO));
				if (hti.pt.x == -1 && hti.pt.y == -1)
					hti.pt.y += height - 4;
				ClientToScreen(hwnd, &hti.pt);

				UINT uID = CreateGCMenu(hwnd, &hMenu, 0, hti.pt, si, uinew.pszUID, uinew.pszNick);
				switch (uID) {
				case 0:
					break;

				case 20020: // add to highlight...
					{
						THighLightEdit the = { THighLightEdit::CMD_ADD, si, ui };
						HWND hwndDlg = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_ADDHIGHLIGHT), dat->m_pContainer->hwnd, CMUCHighlight::dlgProcAdd, (LPARAM)&the);
						TranslateDialogDefault(hwndDlg);

						RECT rc, rcWnd;
						GetClientRect(dat->m_pContainer->hwnd, &rcWnd);
						GetWindowRect(hwndDlg, &rc);
						SetWindowPos(hwndDlg, HWND_TOP, (rcWnd.right - (rc.right - rc.left)) / 2, (rcWnd.bottom - (rc.bottom - rc.top)) / 2, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
					}
					break;

				case ID_MESS:
					pci->DoEventHookAsync(GetParent(hwnd), si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
					break;

				default:
					int iCount = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
					if (iCount != LB_ERR) {
						int iSelectedItems = SendMessage(hwnd, LB_GETSELCOUNT, 0, 0);
						if (iSelectedItems != LB_ERR) {
							int *pItems = (int *)mir_alloc(sizeof(int) * (iSelectedItems + 1));
							if (pItems) {
								if (SendMessage(hwnd, LB_GETSELITEMS, iSelectedItems, (LPARAM)pItems) != LB_ERR) {
									for (int i = 0; i < iSelectedItems; i++) {
										USERINFO *ui1 = pci->SM_GetUserFromIndex(si->ptszID, si->pszModule, pItems[i]);
										if (ui1)
											pci->DoEventHookAsync(hwndParent, si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui1, nullptr, (LPARAM)uID);
									}
								}
								mir_free(pItems);
							}
						}
					}
					break;
				}
				DestroyGCMenu(&hMenu, 1);
				return TRUE;
			}
		}
		break;

	case WM_MOUSEMOVE:
		Chat_HoverMouse(dat->si, hwnd, lParam, M.GetByte("adv_TipperTooltip", 1) && ServiceExists("mToolTip/HideTip"));
		break;
	}
	return mir_callNextSubclass(hwnd, NicklistSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculate the required rectangle for a string using the given font. This is more
// precise than using GetTextExtentPoint...()

int GetTextPixelSize(wchar_t* pszText, HFONT hFont, bool bWidth)
{
	if (!pszText || !hFont)
		return 0;

	HDC hdc = GetDC(nullptr);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	RECT rc = { 0 };
	DrawText(hdc, pszText, -1, &rc, DT_CALCRECT);

	SelectObject(hdc, hOldFont);
	ReleaseDC(nullptr, hdc);
	return bWidth ? rc.right - rc.left : rc.bottom - rc.top;
}

static void __cdecl phase2(void * lParam)
{
	Thread_SetName("TabSRMM: phase2");

	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->hWnd)
		PostMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// the actual group chat session window procedure.Handles the entire chat session window
// which is usually a (tabbed) child of a container class window.

CChatRoomDlg::CChatRoomDlg(SESSION_INFO *_si)
	: CTabBaseDlg(IDD_CHANNEL),
	m_btnOk(this, IDOK),
	m_list(this, IDC_LIST),
	m_btnBold(this, IDC_BOLD),
	m_btnColor(this, IDC_COLOR),
	m_btnFilter(this, IDC_FILTER),
	m_btnItalic(this, IDC_ITALICS),
	m_btnHistory(this, IDC_HISTORY),
	m_btnBkColor(this, IDC_BKGCOLOR),
	m_btnChannelMgr(this, IDC_CHANMGR),
	m_btnUnderline(this, IDC_UNDERLINE),
	m_btnNickList(this, IDC_SHOWNICKLIST)
{
	m_bType = SESSIONTYPE_CHAT;

	si = _si;
	m_hContact = si->hContact;
	m_szProto = GetContactProto(si->hContact);

	m_btnOk.OnClick = Callback(this, &CChatRoomDlg::OnClick_OK);
	m_btnFilter.OnClick = Callback(this, &CChatRoomDlg::OnClick_Filter);
	m_btnHistory.OnClick = Callback(this, &CChatRoomDlg::OnClick_History);
	m_btnNickList.OnClick = Callback(this, &CChatRoomDlg::OnClick_ShowNickList);
	m_btnChannelMgr.OnClick = Callback(this, &CChatRoomDlg::OnClick_ChanMgr);
	
	m_btnColor.OnClick = Callback(this, &CChatRoomDlg::OnClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CChatRoomDlg::OnClick_BkColor);
	m_btnBold.OnClick = m_btnItalic.OnClick = m_btnUnderline.OnClick = Callback(this, &CChatRoomDlg::OnClick_BIU);

	m_message.OnChange = Callback(this, &CChatRoomDlg::OnChange_Message);

	m_list.OnDblClick = Callback(this, &CChatRoomDlg::OnDblClick_List);
}

CThumbBase* CChatRoomDlg::CreateThumb(CProxyWindow *pProxy) const
{
	return new CThumbMUC(pProxy, si);
}

void CChatRoomDlg::ClearLog()
{
	SESSION_INFO *s = pci->SM_FindSession(si->ptszID, si->pszModule);
	if (s) {
		m_log.SetText(L"");
		pci->LM_RemoveAll(&s->pLog, &s->pLogEnd);
		s->iEventCount = 0;
		s->LastTime = 0;
		si->iEventCount = 0;
		si->LastTime = 0;
		si->pLog = s->pLog;
		si->pLogEnd = s->pLogEnd;
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
	}
}

void CChatRoomDlg::OnInitDialog()
{
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	m_cache = CContactCache::getContactCache(m_hContact);
	m_cache->updateNick();
	m_cache->updateUIN();

	si->hWnd = m_hwnd;
	si->dat = this;
	m_bIsAutosizingInput = IsAutoSplitEnabled();
	m_fLimitedUpdate = false;
	m_iInputAreaHeight = -1;
	
	m_iSplitterY = m_pContainer->settings->iSplitterY;
	if (m_bIsAutosizingInput)
		m_iSplitterY = GetDefaultMinimumInputHeight();

	CProxyWindow::add(this);

	// Typing support for GCW_PRIVMESS sessions
	if (si->iType == GCW_PRIVMESS) {
		m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
		SetTimer(m_hwnd, TIMERID_TYPE, 1000, nullptr);
	}

	m_pPanel.setActive(false);
	m_pPanel.Configure();
	M.AddWindow(m_hwnd, m_hContact);
	BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);

	m_log.SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)&reOleCallback);
	m_log.SendMsg(EM_AUTOURLDETECT, 1, 0);
	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_PANELSPLITTER), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPENING, 0);

	m_log.SendMsg(EM_SETEVENTMASK, 0, m_log.SendMsg(EM_GETEVENTMASK, 0, 0) | ENM_LINK | ENM_MOUSEEVENTS | ENM_KEYEVENTS);
	m_log.SendMsg(EM_LIMITTEXT, 0x7FFFFFFF, 0);
	m_log.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
	m_message.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	m_pPanel.loadHeight();

	if (PluginConfig.g_hMenuTrayUnread != 0 && m_hContact != 0 && m_szProto != nullptr)
		UpdateTrayMenu(0, m_wStatus, m_szProto, m_wszStatus, m_hContact, FALSE);

	DM_ThemeChanged();
	m_log.SendMsg(EM_HIDESELECTION, TRUE, 0);

	GetMYUIN();
	GetMyNick();

	HWND hwndBtn = CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20), m_hwnd, (HMENU)IDC_TOGGLESIDEBAR, g_hInst, nullptr);
	CustomizeButton(hwndBtn);
	SendMessage(hwndBtn, BUTTONSETASTHEMEDBTN, 1, 0);
	SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
	SendMessage(hwndBtn, BUTTONSETASFLATBTN, FALSE, 0);
	SendMessage(hwndBtn, BUTTONSETASTOOLBARBUTTON, TRUE, 0);
	SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Expand or collapse the side bar"), BATF_UNICODE);

	DM_InitTip();
	BB_InitDlgButtons();
	SendMessage(m_hwnd, WM_CBD_LOADICONS, 0, 0);

	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SplitterSubclassProc);
	mir_subclassWindow(m_list.GetHwnd(), NicklistSubclassProc);
	mir_subclassWindow(m_log.GetHwnd(), LogSubclassProc);
	mir_subclassWindow(m_btnFilter.GetHwnd(), ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), ButtonSubclassProc);

	mir_subclassWindow(m_message.GetHwnd(), MessageSubclassProc);
	m_message.SendMsg(EM_SUBCLASSED, 0, 0);

	SendMessage(m_hwnd, GC_SETWNDPROPS, 0, 0);
	SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
	SendMessage(m_hwnd, GC_UPDATETITLE, 0, 1);

	RECT rc;
	SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
	SetWindowPos(m_hwnd, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), 0);
	ShowWindow(m_hwnd, SW_SHOW);
	PostMessage(m_hwnd, GC_UPDATENICKLIST, 0, 0);
	m_pContainer->hwndActive = m_hwnd;
	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_OPEN, 0);
}

void CChatRoomDlg::OnDestroy()
{
	// Typing support for GCW_PRIVMESS sessions
	if (si->iType == GCW_PRIVMESS)
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (pcli->pfnGetEvent(si->hContact, 0))
		pcli->pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);
	si->wState &= ~STATE_TALK;
	si->hWnd = nullptr;
	si->dat = nullptr;
	si = nullptr;

	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSING, 0);

	DM_FreeTheme();

	UpdateTrayMenuState(this, FALSE);               // remove me from the tray menu (if still there)
	if (PluginConfig.g_hMenuTrayUnread)
		DeleteMenu(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)m_hContact, MF_BYCOMMAND);

	if (m_hSmileyIcon)
		DestroyIcon(m_hSmileyIcon);

	if (m_hwndTip)
		DestroyWindow(m_hwndTip);

	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
	if (i >= 0) {
		SendMessage(m_hwndParent, WM_USER + 100, 0, 0);              // clean up tooltip
		TabCtrl_DeleteItem(m_hwndParent, i);
		BroadCastContainer(m_pContainer, DM_REFRESHTABINDEX, 0, 0);
		m_iTabID = -1;
	}
	if (m_pWnd) {
		delete m_pWnd;
		m_pWnd = 0;
	}
	if (m_sbCustom) {
		delete m_sbCustom;
		m_sbCustom = 0;
	}

	M.RemoveWindow(m_hwnd);

	TABSRMM_FireEvent(m_hContact, m_hwnd, MSG_WINDOW_EVT_CLOSE, 0);

	memset((void*)&m_pContainer->mOld, -1000, sizeof(MARGINS));
	PostMessage(m_pContainer->hwnd, WM_SIZE, 0, 1);

	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->removeSession(this);
	mir_free(m_enteredText);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
}

void CChatRoomDlg::OnClick_OK(CCtrlButton*)
{
	if (GetSendButtonState(m_hwnd) == PBS_DISABLED)
		return;

	MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
	if (mi == nullptr)
		return;

	ptrA pszRtf(Message_GetFromStream(m_message.GetHwnd()));
	pci->SM_AddCommand(si->ptszID, si->pszModule, pszRtf);

	CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
	if (ptszText.IsEmpty())
		return;

	DoRtfToTags(ptszText, mi->nColorCount, mi->crColors);
	ptszText.Trim();
	ptszText.Replace(L"%", L"%%");

	if (mi->bAckMsg) {
		Utils::enableDlgControl(m_hwnd, IDC_MESSAGE, false);
		m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
	}
	else SetDlgItemText(m_hwnd, IDC_MESSAGE, L"");

	Utils::enableDlgControl(m_hwnd, IDOK, false);

	// Typing support for GCW_PRIVMESS sessions
	if (si->iType == GCW_PRIVMESS)
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	bool fSound = true;
	if (ptszText[0] == '/' || si->iType == GCW_SERVER)
		fSound = false;
	pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_MESSAGE, nullptr, ptszText, 0);
	mi->idleTimeStamp = time(0);
	mi->lastIdleCheck = 0;
	pci->SM_BroadcastMessage(si->pszModule, GC_UPDATESTATUSBAR, 0, 1, TRUE);
	if (m_pContainer)
		if (fSound && !nen_options.iNoSounds && !(m_pContainer->dwFlags & CNT_NOSOUND))
			SkinPlaySound("ChatSent");

	SetFocus(m_message.GetHwnd());
}

void CChatRoomDlg::OnClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	if (si->iLogFilterFlags == 0 && !si->bFilterEnabled) {
		MessageBox(0, TranslateT("The filter cannot be enabled, because there are no event types selected either global or for this chat room"), TranslateT("Event filter error"), MB_OK);
		si->bFilterEnabled = false;
	}
	else si->bFilterEnabled = !si->bFilterEnabled;

	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON,
		(LPARAM)(si->bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);

	if (si->bFilterEnabled && M.GetByte(CHAT_MODULE, "RightClickFilter", 0) == 0) {
		SendMessage(m_hwnd, GC_SHOWFILTERMENU, 0, 0);
		return;
	}
	SendMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
	SendMessage(m_hwnd, GC_UPDATETITLE, 0, 0);
	db_set_b(si->hContact, CHAT_MODULE, "FilterEnabled", (BYTE)si->bFilterEnabled);
}

void CChatRoomDlg::OnClick_History(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	MODULEINFO *pInfo = pci->MM_FindModule(si->pszModule);
	if (ServiceExists("MSP/HTMLlog/ViewLog") && strstr(si->pszModule, "IRC")) {
		char szName[MAX_PATH];
		WideCharToMultiByte(CP_ACP, 0, si->ptszName, -1, szName, MAX_PATH, 0, 0);
		szName[MAX_PATH - 1] = 0;
		CallService("MSP/HTMLlog/ViewLog", (WPARAM)si->pszModule, (LPARAM)szName);
	}
	else if (pInfo)
		ShellExecute(m_hwnd, nullptr, pci->GetChatLogsFilename(si, 0), nullptr, nullptr, SW_SHOW);
}

void CChatRoomDlg::OnClick_ShowNickList(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;
	if (si->iType == GCW_SERVER)
		return;

	si->bNicklistEnabled = !si->bNicklistEnabled;

	SendMessage(m_hwnd, WM_SIZE, 0, 0);
	if (CSkin::m_skinEnabled)
		InvalidateRect(m_hwnd, nullptr, TRUE);
	PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
}

void CChatRoomDlg::OnClick_ChanMgr(CCtrlButton *pButton)
{
	if (pButton->Enabled())
		pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_CHANMGR, nullptr, nullptr, 0);
}

void CChatRoomDlg::OnClick_BIU(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_BOLD))
		cf.dwEffects |= CFE_BOLD;
	if (IsDlgButtonChecked(m_hwnd, IDC_ITALICS))
		cf.dwEffects |= CFE_ITALIC;
	if (IsDlgButtonChecked(m_hwnd, IDC_UNDERLINE))
		cf.dwEffects |= CFE_UNDERLINE;

	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CChatRoomDlg::OnClick_Color(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_COLOR)) {
		if (M.GetByte(CHAT_MODULE, "RightClickFilter", 0) == 0)
			SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_COLOR);
		else if (si->bFGSet) {
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = pci->MM_FindModule(si->pszModule)->crColors[si->iFG];
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		COLORREF cr;
		LoadLogfont(MSGFONTID_MESSAGEAREA, nullptr, &cr, FONTMODULE);
		cf.dwMask = CFM_COLOR;
		cf.crTextColor = cr;
		m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::OnClick_BkColor(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_BKGCOLOR)) {
		if (M.GetByte(CHAT_MODULE, "RightClickFilter", 0) == 0)
			SendMessage(m_hwnd, GC_SHOWCOLORCHOOSER, 0, IDC_BKGCOLOR);
		else if (si->bBGSet) {
			cf.dwMask = CFM_BACKCOLOR;
			cf.crBackColor = pci->MM_FindModule(si->pszModule)->crColors[si->iBG];
			m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	else {
		cf.dwMask = CFM_BACKCOLOR;
		cf.crBackColor = (COLORREF)M.GetDword(FONTMODULE, "inputbg", SRMSGDEFSET_BKGCOLOUR);
		m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
}

void CChatRoomDlg::OnChange_Message(CCtrlEdit*)
{
	if (m_pContainer->hwndActive == m_hwnd)
		UpdateReadChars();
	m_dwLastActivity = GetTickCount();
	m_pContainer->dwLastActivity = m_dwLastActivity;
	m_btnOk.SendMsg(BUTTONSETASNORMAL, GetRichTextLength(m_message.GetHwnd()) != 0, 0);
	m_btnOk.Enable(GetRichTextLength(m_message.GetHwnd()) != 0);

	// Typing support for GCW_PRIVMESS sessions
	if (si->iType == GCW_PRIVMESS) {
		if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
			m_nLastTyping = GetTickCount();
			if (GetWindowTextLength(m_message.GetHwnd())) {
				if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
					if (!(m_dwFlags & MWF_INITMODE))
						DM_NotifyTyping(PROTOTYPE_SELFTYPING_ON);
				}
			}
			else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
				DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
		}
	}
}

void CChatRoomDlg::OnDblClick_List(CCtrlListBox*)
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(m_list.GetHwnd(), &hti.pt);

	int item = LOWORD(m_list.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
	USERINFO *ui = pci->UM_FindUserFromIndex(si->pUsers, item);
	if (ui == nullptr)
		return;

	bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	if (g_Settings.bDoubleClick4Privat ? bShift : !bShift) {
		int selStart = LOWORD(m_message.SendMsg(EM_GETSEL, 0, 0));
		CMStringW tszName(ui->pszNick);
		if (selStart == 0)
			tszName.AppendChar(':');
		tszName.AppendChar(' ');

		m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)tszName.GetString());
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		SetFocus(m_message.GetHwnd());
	}
	else pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
}

INT_PTR CChatRoomDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (si == nullptr && (uMsg == WM_ACTIVATE || uMsg == WM_SETFOCUS))
		return 0;

	POINT pt, tmp, cur;
	RECT rc;
	MODULEINFO *pInfo;

	switch (uMsg) {
	case WM_SETFOCUS:
		if (CMimAPI::m_shutDown)
			break;

		UpdateWindowState(WM_SETFOCUS);
		SetFocus(m_message.GetHwnd());
		return 1;

	case WM_TIMECHANGE:
		PostMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
		break;

	case WM_CBD_LOADICONS:
		Srmm_UpdateToolbarIcons(m_hwnd);
		return 0;

	case GC_SETWNDPROPS:
		pInfo = si ? pci->MM_FindModule(si->pszModule) : nullptr;
		if (pInfo) {
			m_btnBold.Enable(pInfo->bBold);
			m_btnItalic.Enable(pInfo->bItalics);
			m_btnUnderline.Enable(pInfo->bUnderline);
			m_btnColor.Enable(pInfo->bColor);
			m_btnBkColor.Enable(pInfo->bBkgColor);
			if (si->iType == GCW_CHATROOM)
				m_btnChannelMgr.Enable(pInfo->bChanMgr);
		}
		m_log.SendMsg(EM_SETBKGNDCOLOR, 0, M.GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

		DM_InitRichEdit();
		m_btnOk.SendMsg(BUTTONSETASNORMAL, TRUE, 0);

		m_list.SetItemHeight(0, g_Settings.iNickListFontHeight);
		InvalidateRect(m_list.GetHwnd(), nullptr, TRUE);

		m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(si->bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		SendMessage(m_hwnd, GC_REDRAWLOG2, 0, 0);
		break;

	case DM_UPDATETITLE:
	case GC_UPDATETITLE:
		if (!m_bWasDeleted) {
			m_wStatus = si->wStatus;

			const wchar_t *szNick = m_cache->getNick();
			if (mir_wstrlen(szNick) > 0) {
				if (M.GetByte("cuttitle", 0))
					CutContactName(szNick, m_wszTitle, _countof(m_wszTitle));
				else
					wcsncpy_s(m_wszTitle, szNick, _TRUNCATE);
			}

			wchar_t szTemp[100];
			HICON hIcon = 0;

			switch (si->iType) {
			case GCW_CHATROOM:
				hIcon = Skin_LoadProtoIcon(si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
				mir_snwprintf(szTemp,
					(si->nUsersInNicklist == 1) ? TranslateT("%s: chat room (%u user%s)") : TranslateT("%s: chat room (%u users%s)"),
					szNick, si->nUsersInNicklist, si->bFilterEnabled ? TranslateT(", event filter active") : L"");
				break;
			case GCW_PRIVMESS:
				hIcon = Skin_LoadProtoIcon(si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
				if (si->nUsersInNicklist == 1)
					mir_snwprintf(szTemp, TranslateT("%s: message session"), szNick);
				else
					mir_snwprintf(szTemp, TranslateT("%s: message session (%u users)"), szNick, si->nUsersInNicklist);
				break;
			case GCW_SERVER:
				mir_snwprintf(szTemp, L"%s: Server", szNick);
				hIcon = LoadIconEx("window");
				break;
			}

			if (m_pWnd) {
				m_pWnd->updateTitle(m_wszTitle);
				m_pWnd->updateIcon(hIcon);
			}
			m_hTabStatusIcon = hIcon;

			if (lParam)
				m_hTabIcon = m_hTabStatusIcon;

			if (m_cache->getStatus() != m_cache->getOldStatus()) {
				wcsncpy_s(m_wszStatus, pcli->pfnGetStatusModeDescription(m_wStatus, 0), _TRUNCATE);

				TCITEM item = {};
				item.mask = TCIF_TEXT;
				item.pszText = m_wszTitle;
				TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
			}
			SetWindowText(m_hwnd, szTemp);
			if (m_pContainer->hwndActive == m_hwnd) {
				SendMessage(m_pContainer->hwnd, DM_UPDATETITLE, (WPARAM)m_hwnd, 1);
				SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
			}
		}
		break;

	case GC_UPDATESTATUSBAR:
		if (m_bWasDeleted)
			return 0;

		if (m_pContainer->hwndActive != m_hwnd || m_pContainer->hwndStatus == 0 || CMimAPI::m_shutDown || m_wszStatusBar[0])
			break;

		if (si->pszModule != nullptr) {
			wchar_t  szFinalStatusBarText[512];

			//Mad: strange rare crash here...
			MODULEINFO *mi = pci->MM_FindModule(si->pszModule);
			if (!mi)
				break;

			if (!mi->ptszModDispName)
				break;

			int x = 12;
			x += GetTextPixelSize(mi->ptszModDispName, (HFONT)SendMessage(m_pContainer->hwndStatus, WM_GETFONT, 0, 0), TRUE);
			x += GetSystemMetrics(SM_CXSMICON);

			if (m_pPanel.isActive()) {
				time_t now = time(0);
				DWORD diff = (now - mi->idleTimeStamp) / 60;

				if ((diff >= 1 && diff != mi->lastIdleCheck) || lParam) {
					mi->lastIdleCheck = diff;
					if (diff == 0)
						mi->tszIdleMsg[0] = 0;
					else if (diff > 59) {
						DWORD hours = diff / 60;
						DWORD minutes = diff % 60;
						mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s, %d %s idle"), hours, hours > 1 ?
							TranslateT("hours") : TranslateT("hour"),
							minutes, minutes > 1 ? TranslateT("minutes") : TranslateT("minute"));
					}
					else mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s idle"), diff, diff > 1 ? TranslateT("minutes") : TranslateT("minute"));
				}
				mir_snwprintf(szFinalStatusBarText, TranslateT("%s on %s%s"), m_wszMyNickname, mi->ptszModDispName, mi->tszIdleMsg);
			}
			else {
				if (si->ptszStatusbarText)
					mir_snwprintf(szFinalStatusBarText, L"%s %s", mi->ptszModDispName, si->ptszStatusbarText);
				else
					wcsncpy_s(szFinalStatusBarText, mi->ptszModDispName, _TRUNCATE);
			}
			SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)szFinalStatusBarText);
			UpdateStatusBar();
			m_pPanel.Invalidate();
			if (m_pWnd)
				m_pWnd->Invalidate();
			return TRUE;
		}
		break;

	case WM_SIZE:
		if (m_ipFieldHeight == 0)
			m_ipFieldHeight = CInfoPanel::m_ipConfig.height1;

		if (wParam == SIZE_MAXIMIZED)
			PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);

		if (!IsIconic(m_hwnd)) {
			int panelHeight = m_pPanel.getHeight() + 1;

			GetClientRect(m_hwnd, &rc);
			int cx = rc.right;

			CTabBaseDlg::DlgProc(uMsg, 0, 0); // call basic window resizer

			BB_SetButtonsPos();

			rc.left = panelHeight <= CInfoPanel::LEFT_OFFSET_LOGO ? panelHeight : CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx;
			rc.top = 1;
			rc.bottom = (panelHeight > CInfoPanel::DEGRADE_THRESHOLD ? rc.top + m_ipFieldHeight - 2 : panelHeight - 1);
			m_rcNick = rc;

			rc.left = panelHeight <= CInfoPanel::LEFT_OFFSET_LOGO ? panelHeight : CInfoPanel::LEFT_OFFSET_LOGO;
			rc.right = cx;
			rc.bottom = panelHeight - 2;
			rc.top = m_rcNick.bottom + 1;
			m_rcUIN = rc;

			if (m_hwndIEView || m_hwndHPP)
				ResizeIeView();
			DetermineMinHeight();
		}
		return 0;

	case GC_REDRAWWINDOW:
		InvalidateRect(m_hwnd, nullptr, TRUE);
		break;

	case GC_REDRAWLOG:
		si->LastTime = 0;
		if (si->pLog) {
			LOGINFO * pLog = si->pLog;
			if (si->iEventCount > 60) {
				int index = 0;
				while (index < 59) {
					if (pLog->next == nullptr)
						break;
					pLog = pLog->next;
					if ((si->iType != GCW_CHATROOM && si->iType != GCW_PRIVMESS) || !si->bFilterEnabled || (si->iLogFilterFlags&pLog->iType) != 0)
						index++;
				}
				StreamInEvents(pLog, si, TRUE);
				mir_forkthread(phase2, si);
			}
			else StreamInEvents(si->pLogEnd, si, TRUE);
		}
		else SendMessage(m_hwnd, GC_CONTROL_MSG, WINDOW_CLEARLOG, 0);
		break;

	case GC_REDRAWLOG2:
		si->LastTime = 0;
		if (si->pLog)
			StreamInEvents(si->pLogEnd, si, TRUE);
		break;

	case GC_ADDLOG:
		if (PluginConfig.m_bUseDividers) {
			if (PluginConfig.m_bDividersUsePopupConfig) {
				if (!MessageWindowOpened(0, (LPARAM)m_hwnd))
					DM_AddDivider();
			}
			else {
				bool bInactive = (GetForegroundWindow() != m_pContainer->hwnd || GetActiveWindow() != m_pContainer->hwnd);
				if (bInactive)
					DM_AddDivider();
				else if (m_pContainer->hwndActive != m_hwnd)
					DM_AddDivider();
			}
		}

		if (si->pLogEnd)
			StreamInEvents(si->pLog, si, FALSE);
		else
			SendMessage(m_hwnd, GC_CONTROL_MSG, WINDOW_CLEARLOG, 0);
		break;

	case DM_TYPING:
		// Typing support for GCW_PRIVMESS sessions
		if (si->iType == GCW_PRIVMESS) {
			int preTyping = m_nTypeSecs != 0;
			m_nTypeSecs = (int)lParam > 0 ? (int)lParam : 0;

			if (m_nTypeSecs)
				m_bShowTyping = 0;

			SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, preTyping);
		}
		return TRUE;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)pci->hListBkgBrush;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *)lParam;
			if (mis->CtlType == ODT_MENU) {
				if (m_pPanel.isHovered()) {
					mis->itemHeight = 0;
					mis->itemWidth = 6;
					return TRUE;
				}
				return Menu_MeasureItem(lParam);
			}
			mis->itemHeight = g_Settings.iNickListFontHeight;
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlType == ODT_MENU) {
				if (m_pPanel.isHovered()) {
					DrawMenuItem(dis, (HICON)dis->itemData, 0);
					return TRUE;
				}
				return Menu_DrawItem(lParam);
			}

			if (dis->CtlID == IDC_LIST) {
				int x_offset = 0;
				int index = dis->itemID;

				USERINFO *ui = pci->UM_FindUserFromIndex(si->pUsers, index);
				if (ui == nullptr)
					return TRUE;

				int height = dis->rcItem.bottom - dis->rcItem.top;
				if (height & 1)
					height++;
				int offset = (height == 10) ? 0 : height / 2;

				HICON hIcon = pci->SM_GetStatusIcon(si, ui);
				HFONT hFont = g_Settings.UserListFonts[ui->iStatusEx];
				HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
				SetBkMode(dis->hDC, TRANSPARENT);

				int nickIndex = 0;
				for (int i = 0; i < STATUSICONCOUNT; i++) {
					if (hIcon == pci->hIcons[ICON_STATUS0 + i]) {
						nickIndex = i;
						break;
					}
				}

				if (dis->itemState & ODS_SELECTED) {
					FillRect(dis->hDC, &dis->rcItem, g_Settings.SelectionBGBrush);
					SetTextColor(dis->hDC, g_Settings.nickColors[6]);
				}
				else {
					FillRect(dis->hDC, &dis->rcItem, pci->hListBkgBrush);
					if (g_Settings.bColorizeNicks && nickIndex != 0)
						SetTextColor(dis->hDC, g_Settings.nickColors[nickIndex - 1]);
					else
						SetTextColor(dis->hDC, g_Settings.UserListColors[ui->iStatusEx]);
				}
				x_offset = 2;

				if (g_Settings.bShowContactStatus && g_Settings.bContactStatusFirst && ui->ContactStatus) {
					HICON icon = Skin_LoadProtoIcon(si->pszModule, ui->ContactStatus);
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 8, icon, 16, 16, 0, nullptr, DI_NORMAL);
					IcoLib_ReleaseIcon(icon);
					x_offset += 18;
				}

				if (g_Settings.bClassicIndicators) {
					char szTemp[3];
					szTemp[1] = 0;
					szTemp[0] = szIndicators[nickIndex];
					if (szTemp[0]) {
						SIZE szUmode;
						GetTextExtentPoint32A(dis->hDC, szTemp, 1, &szUmode);
						TextOutA(dis->hDC, x_offset, dis->rcItem.top, szTemp, 1);
						x_offset += szUmode.cx + 2;
					}
					else x_offset += 8;
				}
				else {
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 5, hIcon, 10, 10, 0, nullptr, DI_NORMAL);
					x_offset += 12;
				}

				if (g_Settings.bShowContactStatus && !g_Settings.bContactStatusFirst && ui->ContactStatus) {
					HICON icon = Skin_LoadProtoIcon(si->pszModule, ui->ContactStatus);
					DrawIconEx(dis->hDC, x_offset, dis->rcItem.top + offset - 8, icon, 16, 16, 0, nullptr, DI_NORMAL);
					IcoLib_ReleaseIcon(icon);
					x_offset += 18;
				}

				SIZE sz;
				if (m_iSearchItem != -1 && m_iSearchItem == index && m_wszSearch[0]) {
					COLORREF clr_orig = GetTextColor(dis->hDC);
					GetTextExtentPoint32(dis->hDC, ui->pszNick, (int)mir_wstrlen(m_wszSearch), &sz);
					SetTextColor(dis->hDC, RGB(250, 250, 0));
					TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick, (int)mir_wstrlen(m_wszSearch));
					SetTextColor(dis->hDC, clr_orig);
					x_offset += sz.cx;
					TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick + mir_wstrlen(m_wszSearch), int(mir_wstrlen(ui->pszNick) - mir_wstrlen(m_wszSearch)));
				}
				else {
					GetTextExtentPoint32(dis->hDC, ui->pszNick, (int)mir_wstrlen(ui->pszNick), &sz);
					TextOut(dis->hDC, x_offset, (dis->rcItem.top + dis->rcItem.bottom - sz.cy) / 2, ui->pszNick, (int)mir_wstrlen(ui->pszNick));
					SelectObject(dis->hDC, hOldFont);
				}
				return TRUE;
			}
		}
		break;

	case WM_CONTEXTMENU:
		{
			DWORD idFrom = GetDlgCtrlID((HWND)wParam);
			if (idFrom >= MIN_CBUTTONID && idFrom <= MAX_CBUTTONID)
				Srmm_ClickToolbarIcon(m_hContact, idFrom, (HWND)wParam, 1);
		}
		break;

	case GC_UPDATENICKLIST:
		{
			int i = m_list.SendMsg(LB_GETTOPINDEX, 0, 0);
			m_list.SendMsg(LB_SETCOUNT, si->nUsersInNicklist, 0);
			m_list.SendMsg(LB_SETTOPINDEX, i, 0);
			SendMessage(m_hwnd, GC_UPDATETITLE, 0, 0);
		}
		break;

	case GC_CONTROL_MSG:
		switch (wParam) {
		case SESSION_OFFLINE:
			SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
			SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
			return TRUE;

		case SESSION_ONLINE:
			SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
			return TRUE;

		case WINDOW_HIDDEN:
			SendMessage(m_hwnd, GC_CLOSEWINDOW, 0, 1);
			return TRUE;

		case WINDOW_CLEARLOG:
			SetDlgItemText(m_hwnd, IDC_LOG, L"");
			return TRUE;

		case SESSION_TERMINATE:
			if (pcli->pfnGetEvent(si->hContact, 0))
				pcli->pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

			si->wState &= ~STATE_TALK;
			m_bWasDeleted = true;
			db_set_w(si->hContact, si->pszModule, "ApparentMode", 0);
			SendMessage(m_hwnd, GC_CLOSEWINDOW, 0, lParam == 2 ? lParam : 1);
			return TRUE;

		case WINDOW_MINIMIZE:
			ShowWindow(m_hwnd, SW_MINIMIZE);
LABEL_SHOWWINDOW:
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			SendMessage(m_hwnd, GC_REDRAWLOG, 0, 0);
			SendMessage(m_hwnd, GC_UPDATENICKLIST, 0, 0);
			SendMessage(m_hwnd, GC_UPDATESTATUSBAR, 0, 0);
			ShowWindow(m_hwnd, SW_SHOW);
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			SetForegroundWindow(m_hwnd);
			return TRUE;

		case WINDOW_MAXIMIZE:
			ShowWindow(m_hwnd, SW_MAXIMIZE);
			goto LABEL_SHOWWINDOW;

		case SESSION_INITDONE:
			if (M.GetByte(CHAT_MODULE, "PopupOnJoin", 0) != 0)
				return TRUE;

			// fall through
		case WINDOW_VISIBLE:
			if (IsIconic(m_hwnd))
				ShowWindow(m_hwnd, SW_NORMAL);
			goto LABEL_SHOWWINDOW;
		}
		break;

	case DM_SPLITTERMOVED:
		RECT rcLog;
		{
			GetWindowRect(m_log.GetHwnd(), &rcLog);
			if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERX)) {
				GetClientRect(m_hwnd, &rc);
				pt.x = wParam, pt.y = 0;
				ScreenToClient(m_hwnd, &pt);

				int iSplitterX = rc.right - pt.x + 1;
				if (iSplitterX < 35)
					iSplitterX = 35;
				if (iSplitterX > rc.right - rc.left - 35)
					iSplitterX = rc.right - rc.left - 35;
				m_pContainer->settings->iSplitterX = iSplitterX;
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
			}
			else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_SPLITTERY) || lParam == -1) {
				GetClientRect(m_hwnd, &rc);
				rc.top += (m_pPanel.isActive() ? m_pPanel.getHeight() + 40 : 30);
				pt.x = 0, pt.y = wParam;
				ScreenToClient(m_hwnd, &pt);

				m_iSplitterY = rc.bottom - pt.y + DPISCALEY_S(1);
				if (m_iSplitterY < DPISCALEY_S(23))
					m_iSplitterY = DPISCALEY_S(23);
				if (m_iSplitterY > rc.bottom - rc.top - DPISCALEY_S(40))
					m_iSplitterY = rc.bottom - rc.top - DPISCALEY_S(40);
				m_pContainer->settings->iSplitterY = m_iSplitterY;
				UpdateToolbarBG();
				SendMessage(m_hwnd, WM_SIZE, 0, 0);
			}
			else if ((HWND)lParam == GetDlgItem(m_hwnd, IDC_PANELSPLITTER)) {
				pt.x = 0, pt.y = wParam;
				ScreenToClient(m_hwnd, &pt);
				GetClientRect(m_log.GetHwnd(), &rc);
				if ((pt.y + 2 >= MIN_PANELHEIGHT + 2) && (pt.y + 2 < 100) && (pt.y + 2 < rc.bottom - 30))
					m_pPanel.setHeight(pt.y + 2);
				RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
				if (M.isAero())
					InvalidateRect(m_hwndParent, nullptr, FALSE);
				SendMessage(m_hwnd, WM_SIZE, DM_SPLITTERMOVED, 0);
				break;
			}
		}
		break;

	case GC_FIREHOOK:
		if (lParam) {
			GCHOOK *gch = (GCHOOK*)lParam;
			NotifyEventHooks(pci->hSendEvent, 0, (WPARAM)gch);
			if (gch->pDest) {
				mir_free((void*)gch->pDest->ptszID);
				mir_free((void*)gch->pDest->pszModule);
				mir_free(gch->pDest);
			}
			mir_free(gch->ptszText);
			mir_free(gch->ptszUID);
			mir_free(gch);
		}
		break;

	case GC_CHANGEFILTERFLAG:
		if (si->iLogFilterFlags == 0 && si->bFilterEnabled)
			SendMessage(m_hwnd, WM_COMMAND, IDC_FILTER, 0);
		break;

	case GC_SHOWFILTERMENU:
		m_hwndFilter = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_pContainer->hwnd, FilterWndProc, (LPARAM)si);
		TranslateDialogDefault(m_hwndFilter);

		RECT rcFilter;
		GetClientRect(m_hwndFilter, &rcFilter);
		GetWindowRect(m_log.GetHwnd(), &rcLog);

		pt.x = rcLog.right; pt.y = rcLog.bottom;
		ScreenToClient(m_pContainer->hwnd, &pt);

		SetWindowPos(m_hwndFilter, HWND_TOP, pt.x - rcFilter.right, pt.y - rcFilter.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		break;

	case GC_SHOWCOLORCHOOSER:
		pci->ColorChooser(si, lParam == IDC_COLOR, m_hwnd, m_message.GetHwnd(), GetDlgItem(m_hwnd, lParam));
		break;

	case GC_SCROLLTOBOTTOM:
		DM_ScrollToBottom(wParam, lParam);
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND)
			if (m_bCanFlashTab)
				FlashTab(true);

		// Typing support for GCW_PRIVMESS sessions
		if (si->iType == GCW_PRIVMESS && wParam == TIMERID_TYPE)
			DM_Typing(false);
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE) {
			m_pContainer->hwndSaved = 0;
			break;
		}
		// fall through

	case WM_MOUSEACTIVATE:
		UpdateWindowState(WM_ACTIVATE);
		return 1;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case EN_MSGFILTER:
			bool isShift, isCtrl, isMenu;
			{
				UINT  msg = ((MSGFILTER *)lParam)->msg;
				WPARAM wp = ((MSGFILTER *)lParam)->wParam;
				LPARAM lp = ((MSGFILTER *)lParam)->lParam;

				KbdState(isShift, isCtrl, isMenu);

				MSG message;
				message.hwnd = m_hwnd;
				message.message = msg;
				message.lParam = lp;
				message.wParam = wp;

				if (msg == WM_SYSKEYUP) {
					if (wp == VK_MENU)
						if (!m_fkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
							m_pContainer->MenuBar->autoShow();

					return _dlgReturn(m_hwnd, 0);
				}

				if (msg == WM_MOUSEMOVE) {
					GetCursorPos(&pt);
					DM_DismissTip(pt);
					m_pPanel.trackMouse(pt);
					break;
				}
				if (msg == WM_KEYDOWN) {
					if ((wp == VK_INSERT && isShift && !isCtrl && !isMenu) || (wp == 'V' && !isShift && !isMenu && isCtrl)) {
						m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
						((MSGFILTER*)lParam)->msg = WM_NULL;
						((MSGFILTER*)lParam)->wParam = 0;
						((MSGFILTER*)lParam)->lParam = 0;
						return _dlgReturn(m_hwnd, 1);
					}
				}

				if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN)
					m_pContainer->MenuBar->Cancel();

				if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {
					if (DM_GenericHotkeysCheck(&message)) {
						m_fkeyProcessed = true;
						return _dlgReturn(m_hwnd, 1);
					}

					LRESULT mim_hotkey_check = CallService(MS_HOTKEY_CHECK, (WPARAM)&message, (LPARAM)(TABSRMM_HK_SECTION_GC));
					if (mim_hotkey_check)
						m_fkeyProcessed = true;
					
					switch (mim_hotkey_check) {
					case TABSRMM_HK_CHANNELMGR:
						OnClick_ChanMgr(&m_btnChannelMgr);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_FILTERTOGGLE:
						OnClick_Filter(&m_btnFilter);
						InvalidateRect(m_btnFilter.GetHwnd(), nullptr, TRUE);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_LISTTOGGLE:
						OnClick_ShowNickList(&m_btnNickList);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_MUC_SHOWSERVER:
						if (si->iType != GCW_SERVER)
							pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_MESSAGE, nullptr, L"/servershow", 0);
						return _dlgReturn(m_hwnd, 1);
					}
				}

				if (msg == WM_KEYDOWN && ((NMHDR*)lParam)->idFrom != IDC_MESSAGE) {
					if ((wp == VK_NEXT && isCtrl && !isShift) || (wp == VK_TAB && isCtrl && !isShift)) // CTRL-TAB (switch tab/window)
						SendMessage(m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
					else if ((wp == VK_PRIOR && isCtrl && !isShift) || (wp == VK_TAB && isCtrl && isShift)) // CTRL_SHIFT-TAB (switch tab/window)
						SendMessage(m_pContainer->hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
				}					

				if (msg == WM_KEYDOWN && wp == VK_TAB) {
					if (((NMHDR*)lParam)->idFrom == IDC_LOG) {
						SetFocus(m_message.GetHwnd());
						return _dlgReturn(m_hwnd, 1);
					}
				}

				if (((LPNMHDR)lParam)->idFrom == IDC_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP) {
					CHARRANGE sel, all = { 0, -1 };

					pt.x = LOWORD(((ENLINK*)lParam)->lParam), pt.y = HIWORD(((ENLINK*)lParam)->lParam);
					ClientToScreen(((LPNMHDR)lParam)->hwndFrom, &pt);

					// fixing stuff for searches
					wchar_t *pszWord = (wchar_t*)_alloca(8192);
					pszWord[0] = '\0';
					POINTL ptl = { pt.x, pt.y };
					ScreenToClient(m_log.GetHwnd(), (LPPOINT)&ptl);
					int iCharIndex = m_log.SendMsg(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
					if (iCharIndex < 0)
						break;

					int start = m_log.SendMsg(EM_FINDWORDBREAK, WB_LEFT, iCharIndex);
					int end = m_log.SendMsg(EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);

					if (end - start > 0) {
						static char szTrimString[] = ":;,.!?\'\"><()[]- \r\n";

						CHARRANGE cr;
						cr.cpMin = start;
						cr.cpMax = end;

						TEXTRANGE tr = { 0 };
						tr.chrg = cr;
						tr.lpstrText = (wchar_t*)pszWord;
						int iRes = m_log.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);

						if (iRes > 0) {
							size_t iLen = mir_wstrlen(pszWord) - 1;
							while (strchr(szTrimString, pszWord[iLen])) {
								pszWord[iLen] = '\0';
								iLen--;
							}
						}
					}

					HMENU hMenu = 0;
					UINT uID = CreateGCMenu(m_hwnd, &hMenu, 1, pt, si, nullptr, pszWord);
					switch (uID) {
					case 0:
						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						break;

					case ID_COPYALL:
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)&all);
						SendMessage(((LPNMHDR)lParam)->hwndFrom, WM_COPY, 0, 0);
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)&sel);
						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						break;

					case ID_CLEARLOG:
						ClearLog();
						break;

					case ID_SEARCH_GOOGLE:
						if (pszWord[0])
							Utils_OpenUrlW(CMStringW(FORMAT, L"http://www.google.com/search?q=%s", pszWord));

						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						break;

					case ID_SEARCH_WIKIPEDIA:
						if (pszWord[0])
							Utils_OpenUrlW(CMStringW(FORMAT, L"http://en.wikipedia.org/wiki/%s", pszWord));

						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						break;

					default:
						PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
						pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_LOGMENU, nullptr, nullptr, (LPARAM)uID);
						break;
					}

					DestroyGCMenu(&hMenu, 5);
				}
			}
			break;

		case EN_REQUESTRESIZE:
			if (((LPNMHDR)lParam)->idFrom == IDC_MESSAGE)
				DM_HandleAutoSizeRequest((REQRESIZE *)lParam);
			break;

		case EN_LINK:
			if (((LPNMHDR)lParam)->idFrom == IDC_LOG) {
				switch (((ENLINK*)lParam)->msg) {
				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
				case WM_LBUTTONDBLCLK:
					CHARRANGE sel;
					SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
					if (sel.cpMin == sel.cpMax) {
						UINT msg = ((ENLINK*)lParam)->msg;
						m_pContainer->MenuBar->Cancel();

						TEXTRANGE tr;
						tr.lpstrText = nullptr;
						tr.chrg = ((ENLINK*)lParam)->chrg;
						tr.lpstrText = (wchar_t*)mir_alloc(sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 2));
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

						BOOL isLink = IsStringValidLink(tr.lpstrText);
						if (isLink) // handled by core
							break;

						// clicked a nick name
						if (g_Settings.bClickableNicks) {
							if (msg == WM_RBUTTONDOWN) {
								HMENU hMenu = 0;
								USERINFO uiNew;
								for (USERINFO *ui = si->pUsers; ui; ui = ui->next) {
									if (mir_wstrcmp(ui->pszNick, tr.lpstrText))
										continue;

									pt.x = (short)LOWORD(((ENLINK*)lParam)->lParam);
									pt.y = (short)HIWORD(((ENLINK*)lParam)->lParam);
									ClientToScreen(((NMHDR*)lParam)->hwndFrom, &pt);
									memcpy(&uiNew, ui, sizeof(USERINFO));
									UINT uID = CreateGCMenu(m_hwnd, &hMenu, 0, pt, si, uiNew.pszUID, uiNew.pszNick);
									switch (uID) {
									case 0:
										break;

									case ID_MESS:
										pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_PRIVMESS, ui, nullptr, 0);
										break;

									default:
										pci->DoEventHookAsync(m_hwnd, si->ptszID, si->pszModule, GC_USER_NICKLISTMENU, ui, nullptr, (LPARAM)uID);
										break;
									}
									DestroyGCMenu(&hMenu, 1);
									return TRUE;
								}
								return TRUE;
							}
							
							if (msg == WM_LBUTTONUP) {
								CHARRANGE chr;
								m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&chr);

								wchar_t tszAplTmpl[] = L"%s:";
								size_t bufSize = mir_wstrlen(tr.lpstrText) + mir_wstrlen(tszAplTmpl) + 3;
								wchar_t *tszTmp = (wchar_t*)mir_alloc(bufSize * sizeof(wchar_t)), *tszAppeal = tszTmp;

								TEXTRANGE tr2;
								tr2.lpstrText = (LPTSTR)mir_alloc(sizeof(wchar_t) * 2);
								if (chr.cpMin) {
									// prepend nick with space if needed
									tr2.chrg.cpMin = chr.cpMin - 1;
									tr2.chrg.cpMax = chr.cpMin;
									m_message.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr2);
									if (!iswspace(*tr2.lpstrText))
										*tszTmp++ = ' ';
									mir_wstrcpy(tszTmp, tr.lpstrText);
								}
								else // in the beginning of the message window
									mir_snwprintf(tszAppeal, bufSize, tszAplTmpl, tr.lpstrText);

								size_t st = mir_wstrlen(tszAppeal);
								if (chr.cpMax != -1) {
									tr2.chrg.cpMin = chr.cpMax;
									tr2.chrg.cpMax = chr.cpMax + 1;
									// if there is no space after selection,
									// or there is nothing after selection at all...
									if (!m_message.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr2) || !iswspace(*tr2.lpstrText)) {
										tszAppeal[st++] = ' ';
										tszAppeal[st++] = '\0';
									}
								}
								else {
									tszAppeal[st++] = ' ';
									tszAppeal[st++] = '\0';
								}
								m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)tszAppeal);
								mir_free((void*)tr2.lpstrText);
								mir_free((void*)tszAppeal);
							}
						}
						SetFocus(m_message.GetHwnd());
						mir_free(tr.lpstrText);
						return TRUE;
					}
				}
				break;
			}
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		GetCursorPos(&tmp);
		if (!m_pPanel.isHovered()) {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(m_pContainer->hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&tmp);
		if (m_pPanel.isHovered())
			m_pPanel.handleClick(tmp);
		else {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(m_pContainer->hwnd, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		DM_DismissTip(pt);
		m_pPanel.trackMouse(pt);
		break;

	case WM_APPCOMMAND:
		{
			DWORD cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				SendMessage(m_pContainer->hwnd, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) >= MIN_CBUTTONID && LOWORD(wParam) <= MAX_CBUTTONID){
			Srmm_ClickToolbarIcon(m_hContact, LOWORD(wParam), GetDlgItem(m_hwnd, LOWORD(wParam)), 0);
			break;
		}

		switch (LOWORD(wParam)) {
		case IDC_TOGGLESIDEBAR:
			SendMessage(m_pContainer->hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
			break;

		case IDCANCEL:
			ShowWindow(m_pContainer->hwnd, SW_MINIMIZE);
			return FALSE;

		case IDC_CLOSE:
			SendMessage(m_hwnd, WM_CLOSE, 0, 1);
			break;

		case IDC_SELFTYPING:
			// Typing support for GCW_PRIVMESS sessions
			if (si->iType == GCW_PRIVMESS) {
				if (m_hContact) {
					int iCurrentTypingMode = db_get_b(m_hContact, SRMSGMOD, SRMSGSET_TYPING, M.GetByte(SRMSGMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW));

					if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON && iCurrentTypingMode) {
						DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
						m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
					}
					db_set_b(m_hContact, SRMSGMOD, SRMSGSET_TYPING, (BYTE)!iCurrentTypingMode);
				}
			}
			break;
		}
		break;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_ERASEBKGND:
		RECT rcClient, rcWindow;
		{
			HDC  hdc = (HDC)wParam;
			UINT item_ids[3] = { ID_EXTBKUSERLIST, ID_EXTBKHISTORY, ID_EXTBKINPUTAREA };
			UINT ctl_ids[3] = { IDC_LIST, IDC_LOG, IDC_MESSAGE };
			HANDLE hbp = 0;
			HDC hdcMem = 0;
			HBITMAP hbm, hbmOld;

			GetClientRect(m_hwnd, &rcClient);
			LONG cx = rcClient.right - rcClient.left;
			LONG cy = rcClient.bottom - rcClient.top;

			if (CMimAPI::m_haveBufferedPaint) {
				hbp = CSkin::InitiateBufferedPaint(hdc, rcClient, hdcMem);
				hbm = hbmOld = 0;
			}
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm = CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			if (CSkin::m_skinEnabled && !M.isAero()) {
				CSkin::SkinDrawBG(m_hwnd, m_pContainer->hwnd, m_pContainer, &rcClient, hdcMem);
				for (int i = 0; i < 3; i++) {
					CSkinItem *item = &SkinItems[item_ids[i]];
					if (!item->IGNORED) {
						GetWindowRect(GetDlgItem(m_hwnd, ctl_ids[i]), &rcWindow);
						pt.x = rcWindow.left;
						pt.y = rcWindow.top;
						ScreenToClient(m_hwnd, &pt);
						rc.left = pt.x - item->MARGIN_LEFT;
						rc.top = pt.y - item->MARGIN_TOP;
						rc.right = rc.left + item->MARGIN_RIGHT + (rcWindow.right - rcWindow.left) + item->MARGIN_LEFT;
						rc.bottom = rc.top + item->MARGIN_BOTTOM + (rcWindow.bottom - rcWindow.top) + item->MARGIN_TOP;
						CSkin::DrawItem(hdcMem, &rc, item);
					}
				}
			}
			else {
				CSkin::FillBack(hdcMem, &rcClient);

				if (M.isAero()) {
					LONG temp = rcClient.bottom;
					rcClient.bottom = m_pPanel.isActive() ? m_pPanel.getHeight() + 5 : 5;
					FillRect(hdcMem, &rcClient, (HBRUSH)GetStockObject(BLACK_BRUSH));
					rcClient.bottom = temp;
				}
			}

			GetClientRect(m_hwnd, &rc);
			m_pPanel.renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], M.isAero());
			m_pPanel.renderContent(hdcMem);

			if (!CSkin::m_skinEnabled)
				RenderToolbarBG(hdcMem, rcClient);

			if (hbp)
				CSkin::FinalizeBufferedPaint(hbp, &rcClient);
			else {
				BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbm);
				DeleteDC(hdcMem);
			}
			if (!m_fLimitedUpdate)
				SetAeroMargins(m_pContainer);
		}
		return TRUE;

	case WM_NCPAINT:
		if (CSkin::m_skinEnabled)
			return 0;
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);
		EndPaint(m_hwnd, &ps);
		return 0;

	case WM_RBUTTONUP:
		GetCursorPos(&pt);
		if (!m_pPanel.invokeConfigDialog(pt)) {
			HMENU subMenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);

			MsgWindowUpdateMenu(subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				_itoa_s(iSelection - IDM_CONTAINERMENU, szIndex, 10);
				ptrW wszContainer(db_get_wsa(0, "TAB_ContainersW", szIndex));
				if (wszContainer != nullptr)
					SendMessage(m_hwnd, DM_CONTAINERSELECTED, 0, wszContainer);
				break;
			}
			MsgWindowMenuHandler(iSelection, MENU_TABCONTEXT);
		}
		break;

	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam) < 30)
			PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 0, 0);
		break;

	case WM_CLOSE:
		if (wParam == 0 && lParam == 0) {
			if (PluginConfig.m_EscapeCloses == 1) {
				SendMessage(m_pContainer->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;
			}
			if (PluginConfig.m_bHideOnClose && PluginConfig.m_EscapeCloses == 2) {
				ShowWindow(m_pContainer->hwnd, SW_HIDE);
				return TRUE;
			}
			_dlgReturn(m_hwnd, TRUE);
		}
		SendMessage(m_hwnd, GC_CLOSEWINDOW, 0, 1);
		return 0;

	case GC_CLOSEWINDOW:
		{
			bool bForced = (lParam == 2);

			int iTabs = TabCtrl_GetItemCount(m_hwndParent);
			if (iTabs == 1 && CMimAPI::m_shutDown == 0) {
				SendMessage(m_pContainer->hwnd, WM_CLOSE, 0, 1);
				return 1;
			}

			m_pContainer->iChilds--;
			int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);

			// after closing a tab, we need to activate the tab to the left side of
			// the previously open tab.
			// normally, this tab has the same index after the deletion of the formerly active tab
			// unless, of course, we closed the last (rightmost) tab.
			if (!m_pContainer->bDontSmartClose && iTabs > 1 && !bForced) {
				if (i == iTabs - 1)
					i--;
				else
					i++;
				TabCtrl_SetCurSel(m_hwndParent, i);

				TCITEM item = {};
				item.mask = TCIF_PARAM;
				TabCtrl_GetItem(m_hwndParent, i, &item); // retrieve dialog hwnd for the now active tab...
				m_pContainer->hwndActive = (HWND)item.lParam;

				SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
				SetWindowPos(m_pContainer->hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
				ShowWindow((HWND)item.lParam, SW_SHOW);
				SetForegroundWindow(m_pContainer->hwndActive);
				SetFocus(m_pContainer->hwndActive);
				SendMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
			}

			if (iTabs == 1)
				SendMessage(m_pContainer->hwnd, WM_CLOSE, 0, 1);
			else {
				PostMessage(m_pContainer->hwnd, WM_SIZE, 0, 0);
				DestroyWindow(m_hwnd);
			}
		}
		return 0;

	case DM_SAVESIZE:
		if (m_dwFlags & MWF_NEEDCHECKSIZE)
			lParam = 0;

		m_dwFlags &= ~MWF_NEEDCHECKSIZE;
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			m_dwFlags &= ~MWF_INITMODE;

		SendMessage(m_pContainer->hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		MoveWindow(m_hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE) {
			m_dwFlags &= ~MWF_WASBACKGROUNDCREATE;
			SendMessage(m_hwnd, WM_SIZE, 0, 0);

			pt.x = pt.y = 0;
			m_log.SendMsg(EM_SETSCROLLPOS, 0, (LPARAM)&pt);
			if (PluginConfig.m_bAutoLocaleSupport) {
				if (hkl == 0)
					DM_LoadLocale();
				else
					PostMessage(m_hwnd, DM_SETLOCALE, 0, 0);
			}
		}
		else {
			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			if (lParam == 0)
				PostMessage(m_hwnd, GC_SCROLLTOBOTTOM, 1, 1);
		}
		return 0;

	case DM_REFRESHTABINDEX:
		m_iTabID = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
		return 0;

	case WM_CBD_UPDATED:
		if (lParam)
			CB_ChangeButton((CustomButtonData*)lParam);
		else
			BB_InitDlgButtons();

		BB_SetButtonsPos();
		return 0;

	case WM_CBD_REMOVED:
		if (lParam)
			CB_DestroyButton((DWORD)wParam, (DWORD)lParam);
		else
			CB_DestroyAllButtons();
		break;

	case DM_CONFIGURETOOLBAR:
		SendMessage(m_hwnd, WM_SIZE, 0, 0);
		break;

	case DM_SMILEYOPTIONSCHANGED:
		SendMessage(m_hwnd, GC_REDRAWLOG, 0, 1);
		break;

	case EM_THEMECHANGED:
		DM_FreeTheme();
		DM_ThemeChanged();
		return 0;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme();
		memset(&m_pContainer->mOld, -1000, sizeof(MARGINS));
		CProxyWindow::verify(this);
		break;
	}
	return CTabBaseDlg::DlgProc(uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat session creator

void ShowRoom(SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	if (si->hWnd != nullptr) {
		ActivateExistingTab(si->dat->m_pContainer, si->hWnd);
		return;
	}

	wchar_t szName[CONTAINER_NAMELEN + 2]; szName[0] = 0;
	TContainerData *pContainer = nullptr;
	if (si->dat != nullptr)
		pContainer = si->dat->m_pContainer;
	if (pContainer == nullptr) {
		GetContainerNameForContact(si->hContact, szName, CONTAINER_NAMELEN);
		if (!g_Settings.bOpenInDefault && !mir_wstrcmp(szName, L"default"))
			wcsncpy(szName, L"Chat Rooms", CONTAINER_NAMELEN);
		szName[CONTAINER_NAMELEN] = 0;
		pContainer = FindContainerByName(szName);
	}
	if (pContainer == nullptr)
		pContainer = CreateContainer(szName, FALSE, si->hContact);
	if (pContainer == nullptr)
		return; // smth went wrong, nothing to do here

	MCONTACT hContact = si->hContact;
	if (M.FindWindow(hContact) != 0)
		return;

	if (hContact != 0 && M.GetByte("limittabs", 0) && !wcsncmp(pContainer->szName, L"default", 6)) {
		if ((pContainer = FindMatchingContainer(L"default")) == nullptr) {
			wchar_t szName[CONTAINER_NAMELEN + 1];
			mir_snwprintf(szName, L"default");
			if ((pContainer = CreateContainer(szName, CNT_CREATEFLAG_CLONED, hContact)) == nullptr)
				return;
		}
	}

	wchar_t *contactName = pcli->pfnGetContactDisplayName(hContact, 0);

	// cut nickname if larger than x chars...
	wchar_t newcontactname[128];
	if (mir_wstrlen(contactName) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, _countof(newcontactname));
		else
			wcsncpy_s(newcontactname, contactName, _TRUNCATE);
	}
	else wcsncpy_s(newcontactname, L"_U_", _TRUNCATE);

	HWND hwndTab = GetDlgItem(pContainer->hwnd, IDC_MSGTABS);

	// hide the active tab
	if (pContainer->hwndActive)
		ShowWindow(pContainer->hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);

	pContainer->iTabIndex = iCount;
	if (iCount > 0) {
		TCITEM item = {};
		for (int i = iCount - 1; i >= 0; i--) {
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, i, &item);
			HWND hwnd = (HWND)item.lParam;
			CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->m_hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->iTabIndex = i;
			}
		}
	}

	TCITEM item = {};
	item.pszText = newcontactname;
	item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	int iTabId = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &item);

	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	TabCtrl_SetCurSel(hwndTab, iTabId);
	pContainer->iChilds++;

	CChatRoomDlg *pDlg = new CChatRoomDlg(si);
	pDlg->m_iTabID = iTabId;
	pDlg->m_pContainer = pContainer;
	pDlg->SetParent(hwndTab);
	pDlg->Create();

	HWND hwndNew = pDlg->GetHwnd();
	item.lParam = (LPARAM)hwndNew;
	TabCtrl_SetItem(hwndTab, iTabId, &item);

	if (pContainer->dwFlags & CNT_SIDEBAR)
		pContainer->SideBar->addSession(pDlg, pContainer->iTabIndex);

	SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);
	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->hwnd)) {
		SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetFocus(pContainer->hwndActive);
	}

	if (PluginConfig.m_bHideOnClose && !IsWindowVisible(pContainer->hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->hwnd, &wp);

		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);			// make sure all tabs will re-check layout on activation
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(pContainer->hwnd, SW_SHOWNORMAL);
		}
		SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
		SetFocus(hwndNew);
	}
	else {
		SetFocus(hwndNew);
		RedrawWindow(pContainer->hwnd, nullptr, nullptr, RDW_INVALIDATE);
		UpdateWindow(pContainer->hwnd);
		if (GetForegroundWindow() != pContainer->hwnd)
			SetForegroundWindow(pContainer->hwnd);
	}

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M.GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);
}
