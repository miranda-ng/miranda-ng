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

static HKL hkl = nullptr;
char szIndicators[] = { 0, '+', '%', '@', '!', '*' };

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
		enlink.nmhdr.idFrom = IDC_SRMM_LOG;
		enlink.nmhdr.code = EN_LINK;
		enlink.msg = uMsg;
		enlink.wParam = wParam;
		enlink.lParam = lParam;
		enlink.chrg.cpMin = cpMin;
		enlink.chrg.cpMax = cpMax;
		SendMessage(GetParent(hwndDlg), WM_NOTIFY, IDC_SRMM_LOG, (LPARAM)&enlink);
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
	if (m_si == nullptr)
		return;

	if (msg == WM_ACTIVATE) {
		if (m_pContainer->dwFlags & CNT_TRANSPARENCY) {
			DWORD trans = LOWORD(m_pContainer->settings->dwTransparency);
			SetLayeredWindowAttributes(m_pContainer->m_hwnd, CSkin::m_ContainerColorKey, (BYTE)trans, (m_pContainer->dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
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

	if (m_pContainer->hwndSaved == m_hwnd)
		return;

	m_pContainer->hwndSaved = m_hwnd;

	pci->SetActiveSession(m_si);
	m_hTabIcon = m_hTabStatusIcon;

	if (m_iTabID >= 0) {
		if (db_get_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
		if (pcli->pfnGetEvent(m_si->hContact, 0))
			pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);

		UpdateTitle();
		m_hTabIcon = m_hTabStatusIcon;
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
		SetFocus(m_message.GetHwnd());
		m_dwLastActivity = GetTickCount();
		m_pContainer->dwLastActivity = m_dwLastActivity;
		m_pContainer->MenuBar->configureMenu();
		UpdateTrayMenuState(this, FALSE);
		DM_SetDBButtonStates();

		if (m_dwFlagsEx & MWF_EX_DELAYEDSPLITTER) {
			m_dwFlagsEx &= ~MWF_EX_DELAYEDSPLITTER;
			ShowWindow(m_pContainer->m_hwnd, SW_RESTORE);
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
	bool bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;
	bool bInfoPanel = m_pPanel.isActive();
	int  panelHeight = m_pPanel.getHeight() + 1;
	int  iSplitterX = m_pContainer->settings->iSplitterX;

	RECT rcTabs;
	GetClientRect(m_hwnd, &rcTabs);

	if (m_bIsAutosizingInput)
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERY, SW_HIDE);

	if (m_si->iType != GCW_SERVER) {
		m_nickList.Show(m_bNicklistEnabled);
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERX, m_bNicklistEnabled ? SW_SHOW : SW_HIDE);

		m_btnNickList.Enable(true);
		m_btnFilter.Enable(true);
		if (m_si->iType == GCW_CHATROOM) {
			MODULEINFO *tmp = pci->MM_FindModule(m_si->pszModule);
			if (tmp)
				m_btnChannelMgr.Enable(tmp->bChanMgr);
		}
	}
	else {
		m_nickList.Hide();
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERX, SW_HIDE);
	}

	if (m_si->iType == GCW_SERVER) {
		m_btnNickList.Enable(false);
		m_btnFilter.Enable(false);
		m_btnChannelMgr.Enable(false);
	}

	switch (urc->wId) {
	case IDC_PANELSPLITTER:
		urc->rcItem.bottom = panelHeight;
		urc->rcItem.top = panelHeight - 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_SRMM_LOG:
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

	case IDC_SRMM_NICKLIST:
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

	case IDC_SRMM_MESSAGE:
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

bool CChatRoomDlg::TabAutoComplete()
{
	LRESULT lResult = m_message.SendMsg(EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	m_message.SendMsg(EM_SETSEL, end, end);

	GETTEXTEX gt = { 0 };
	gt.codepage = 1200;
	gt.flags = GTL_DEFAULT | GTL_PRECISE;
	int iLen = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gt, 0);
	if (iLen <= 0)
		return false;

	bool isTopic = false, isRoom = false;
	wchar_t *pszName = nullptr;
	wchar_t *pszText = (wchar_t*)mir_calloc((iLen + 10) * sizeof(wchar_t));

	gt.flags = GT_DEFAULT;
	gt.cb = (iLen + 9) * sizeof(wchar_t);
	m_message.SendMsg(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)pszText);

	if (start > 1 && pszText[start - 1] == ' ' && pszText[start - 2] == ':')
		start -= 2;

	if (m_wszSearchResult != nullptr) {
		int cbResult = (int)mir_wstrlen(m_wszSearchResult);
		if (start >= cbResult && !wcsnicmp(m_wszSearchResult, pszText + start - cbResult, cbResult)) {
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
	if (m_wszSearchQuery == nullptr) {
		m_wszSearchQuery = mir_wstrndup(pszText + start, end - start);
		m_wszSearchResult = mir_wstrdup(m_wszSearchQuery);
		m_pLastSession = nullptr;
	}
	if (isTopic)
		pszName = m_si->ptszTopic;
	else if (isRoom) {
		m_pLastSession = SM_FindSessionAutoComplete(m_si->pszModule, m_si, m_pLastSession, m_wszSearchQuery, m_wszSearchResult);
		if (m_pLastSession != nullptr)
			pszName = m_pLastSession->ptszName;
	}
	else pszName = pci->UM_FindUserAutoComplete(m_si->pUsers, m_wszSearchQuery, m_wszSearchResult);

	replaceStrW(m_wszSearchResult, nullptr);

	if (pszName != nullptr) {
		m_wszSearchResult = mir_wstrdup(pszName);
		if (end != start) {
			ptrW szReplace;
			if (!isRoom && !isTopic && g_Settings.bAddColonToAutoComplete && start == 0) {
				szReplace = (wchar_t*)mir_alloc((mir_wstrlen(pszName) + 4) * sizeof(wchar_t));
				mir_wstrcpy(szReplace, pszName);
				mir_wstrcat(szReplace, g_Settings.bUseCommaAsColon ? L", " : L": ");
				pszName = szReplace;
			}
			m_message.SendMsg(EM_SETSEL, start, end);
			m_message.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)pszName);
		}
		return true;
	}

	if (end != start) {
		m_message.SendMsg(EM_SETSEL, start, end);
		m_message.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)m_wszSearchQuery);
	}
	replaceStrW(m_wszSearchQuery, nullptr);
	return false;
}

static void __cdecl phase2(void * lParam)
{
	Thread_SetName("TabSRMM: phase2");

	SESSION_INFO *si = (SESSION_INFO*)lParam;
	Sleep(30);
	if (si && si->pDlg)
		si->pDlg->RedrawLog2();
}

/////////////////////////////////////////////////////////////////////////////////////////
// the actual group chat session window procedure.Handles the entire chat session window
// which is usually a (tabbed) child of a container class window.

CChatRoomDlg::CChatRoomDlg(SESSION_INFO *si)
	: CTabBaseDlg(IDD_CHANNEL, si),
	m_btnOk(this, IDOK)
{
	m_szProto = GetContactProto(m_hContact);
	m_bFilterEnabled = db_get_b(m_hContact, CHAT_MODULE, "FilterEnabled", m_bFilterEnabled) != 0;
	Chat_SetFilters(m_si);

	m_btnOk.OnClick = Callback(this, &CChatRoomDlg::onClick_OK);
	m_btnFilter.OnClick = Callback(this, &CChatRoomDlg::onClick_Filter);
	m_btnNickList.OnClick = Callback(this, &CChatRoomDlg::onClick_ShowNickList);
	
	m_message.OnChange = Callback(this, &CChatRoomDlg::onChange_Message);
}

CThumbBase* CChatRoomDlg::tabCreateThumb(CProxyWindow *pProxy) const
{
	return new CThumbMUC(pProxy, m_si);
}

void CChatRoomDlg::tabClearLog()
{
	SESSION_INFO *s = pci->SM_FindSession(m_si->ptszID, m_si->pszModule);
	if (s) {
		ClearLog();
		pci->LM_RemoveAll(&s->pLog, &s->pLogEnd);
		s->iEventCount = 0;
		s->LastTime = 0;
		m_si->iEventCount = 0;
		m_si->LastTime = 0;
		m_si->pLog = s->pLog;
		m_si->pLogEnd = s->pLogEnd;
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
	}
}

void CChatRoomDlg::OnInitDialog()
{
	CTabBaseDlg::OnInitDialog();

	m_si->pDlg = this;
	
	m_iSplitterY = m_pContainer->settings->iSplitterY;
	if (m_bIsAutosizingInput)
		m_iSplitterY = GetDefaultMinimumInputHeight();

	// Typing support for GCW_PRIVMESS sessions
	if (m_si->iType == GCW_PRIVMESS) {
		m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
		SetTimer(m_hwnd, TIMERID_TYPE, 1000, nullptr);
	}

	m_pPanel.getVisibility();
	m_pPanel.Configure();

	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_PANELSPLITTER), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
	NotifyEvent(MSG_WINDOW_EVT_OPENING);

	m_log.SendMsg(EM_AUTOURLDETECT, 1, 0);
	m_log.SendMsg(EM_SETEVENTMASK, 0, m_log.SendMsg(EM_GETEVENTMASK, 0, 0) | ENM_LINK | ENM_MOUSEEVENTS | ENM_KEYEVENTS);
	m_log.SendMsg(EM_LIMITTEXT, 0x7FFFFFFF, 0);
	m_log.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
	m_message.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	m_pPanel.loadHeight();

	if (PluginConfig.g_hMenuTrayUnread != 0 && m_hContact != 0 && m_szProto != nullptr)
		UpdateTrayMenu(0, m_wStatus, m_szProto, m_wszStatus, m_hContact, FALSE);

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

	UpdateOptions();
	UpdateStatusBar();
	UpdateTitle();
	m_hTabIcon = m_hTabStatusIcon;

	RECT rc;
	SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
	SetWindowPos(m_hwnd, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), 0);
	ShowWindow(m_hwnd, SW_SHOW);
	UpdateNickList();
	m_pContainer->m_hwndActive = m_hwnd;
	NotifyEvent(MSG_WINDOW_EVT_OPEN);
}

void CChatRoomDlg::OnDestroy()
{
	// Typing support for GCW_PRIVMESS sessions
	if (m_si->iType == GCW_PRIVMESS)
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	if (pcli->pfnGetEvent(m_si->hContact, 0))
		pcli->pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
	m_si->wState &= ~STATE_TALK;
	m_si->pDlg = nullptr;
	m_si = nullptr;

	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	DM_FreeTheme();

	UpdateTrayMenuState(this, FALSE);               // remove me from the tray menu (if still there)
	if (PluginConfig.g_hMenuTrayUnread)
		DeleteMenu(PluginConfig.g_hMenuTrayUnread, (UINT_PTR)m_hContact, MF_BYCOMMAND);

	if (m_hwndTip)
		DestroyWindow(m_hwndTip);

	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
	if (i >= 0) {
		SendMessage(m_hwndParent, WM_USER + 100, 0, 0);              // clean up tooltip
		TabCtrl_DeleteItem(m_hwndParent, i);
		m_pContainer->UpdateTabs();
		m_iTabID = -1;
	}
	
	if (m_pWnd) {
		delete m_pWnd;
		m_pWnd = nullptr;
	}

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	m_pContainer->ClearMargins();
	PostMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 1);

	if (m_pContainer->dwFlags & CNT_SIDEBAR)
		m_pContainer->SideBar->removeSession(this);
	mir_free(m_enteredText);

	CSuper::OnDestroy();
}

void CChatRoomDlg::onClick_OK(CCtrlButton*)
{
	if (GetSendButtonState(m_hwnd) == PBS_DISABLED)
		return;

	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	if (mi == nullptr)
		return;

	ptrA pszRtf(m_message.GetRichTextRtf());
	pci->SM_AddCommand(m_si->ptszID, m_si->pszModule, pszRtf);

	CMStringW ptszText(ptrW(mir_utf8decodeW(pszRtf)));
	if (ptszText.IsEmpty())
		return;

	DoRtfToTags(ptszText);
	ptszText.Trim();
	ptszText.Replace(L"%", L"%%");

	if (mi->bAckMsg) {
		m_message.Enable(false);
		m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
	}
	else m_message.SetText(L"");

	Utils::enableDlgControl(m_hwnd, IDOK, false);

	// Typing support for GCW_PRIVMESS sessions
	if (m_si->iType == GCW_PRIVMESS)
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	bool fSound = true;
	if (ptszText[0] == '/' || m_si->iType == GCW_SERVER)
		fSound = false;
	Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, ptszText, 0);
	mi->idleTimeStamp = time(0);
	mi->lastIdleCheck = 0;
	UpdateStatusBar();
	if (m_pContainer)
		if (fSound && !nen_options.iNoSounds && !(m_pContainer->dwFlags & CNT_NOSOUND))
			Skin_PlaySound("ChatSent");

	SetFocus(m_message.GetHwnd());
}

void CChatRoomDlg::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	if (m_iLogFilterFlags == 0 && !m_bFilterEnabled) {
		MessageBox(0, TranslateT("The filter cannot be enabled, because there are no event types selected either global or for this chat room"), TranslateT("Event filter error"), MB_OK);
		m_bFilterEnabled = false;
	}
	else m_bFilterEnabled = !m_bFilterEnabled;

	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(m_bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);

	if (m_bFilterEnabled && M.GetByte(CHAT_MODULE, "RightClickFilter", 0) == 0) {
		ShowFilterMenu();
		return;
	}
	RedrawLog();
	UpdateTitle();
	db_set_b(m_si->hContact, CHAT_MODULE, "FilterEnabled", m_bFilterEnabled);
}

void CChatRoomDlg::onClick_ShowNickList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;

	Resize();
	if (CSkin::m_skinEnabled)
		InvalidateRect(m_hwnd, nullptr, TRUE);
	ScrollToBottom();
}

void CChatRoomDlg::onChange_Message(CCtrlEdit*)
{
	if (m_pContainer->m_hwndActive == m_hwnd)
		UpdateReadChars();
	m_dwLastActivity = GetTickCount();
	m_pContainer->dwLastActivity = m_dwLastActivity;
	m_btnOk.SendMsg(BUTTONSETASNORMAL, m_message.GetRichTextLength() != 0, 0);
	m_btnOk.Enable(m_message.GetRichTextLength() != 0);

	// Typing support for GCW_PRIVMESS sessions
	if (m_si->iType == GCW_PRIVMESS) {
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

/////////////////////////////////////////////////////////////////////////////////////////

void CChatRoomDlg::AddLog()
{
	if (PluginConfig.m_bUseDividers) {
		if (PluginConfig.m_bDividersUsePopupConfig) {
			if (!MessageWindowOpened(0, m_hwnd))
				DM_AddDivider();
		}
		else {
			bool bInactive = (GetForegroundWindow() != m_pContainer->m_hwnd || GetActiveWindow() != m_pContainer->m_hwnd);
			if (bInactive)
				DM_AddDivider();
			else if (m_pContainer->m_hwndActive != m_hwnd)
				DM_AddDivider();
		}
	}

	CSrmmBaseDialog::AddLog();
}

void CChatRoomDlg::CloseTab()
{
	int iTabs = TabCtrl_GetItemCount(m_hwndParent);
	if (iTabs == 1 && CMimAPI::m_shutDown == 0) {
		SendMessage(m_pContainer->m_hwnd, WM_CLOSE, 0, 1);
		return;
	}

	m_pContainer->iChilds--;
	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);

	// after closing a tab, we need to activate the tab to the left side of
	// the previously open tab.
	// normally, this tab has the same index after the deletion of the formerly active tab
	// unless, of course, we closed the last (rightmost) tab.
	if (!m_pContainer->bDontSmartClose && iTabs > 1) {
		if (i == iTabs - 1)
			i--;
		else
			i++;
		TabCtrl_SetCurSel(m_hwndParent, i);

		TCITEM item = {};
		item.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_hwndParent, i, &item); // retrieve dialog hwnd for the now active tab...
		m_pContainer->m_hwndActive = (HWND)item.lParam;

		RECT rc;
		SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rc);
		SetWindowPos(m_pContainer->m_hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
		ShowWindow((HWND)item.lParam, SW_SHOW);
		SetForegroundWindow(m_pContainer->m_hwndActive);
		SetFocus(m_pContainer->m_hwndActive);
		SendMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 0);
	}

	if (iTabs == 1)
		SendMessage(m_pContainer->m_hwnd, WM_CLOSE, 0, 1);
	else {
		PostMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 0);
		DestroyWindow(m_hwnd);
	}
}

void CChatRoomDlg::RedrawLog()
{
	m_si->LastTime = 0;
	if (m_si->pLog) {
		LOGINFO *pLog = m_si->pLog;
		if (m_si->iEventCount > 60) {
			int index = 0;
			while (index < 59) {
				if (pLog->next == nullptr)
					break;
				pLog = pLog->next;
				if ((m_si->iType != GCW_CHATROOM && m_si->iType != GCW_PRIVMESS) || !m_bFilterEnabled || (m_iLogFilterFlags & pLog->iType) != 0)
					index++;
			}
			StreamInEvents(pLog, TRUE);
			mir_forkthread(phase2, m_si);
		}
		else StreamInEvents(m_si->pLogEnd, TRUE);
	}
	else ClearLog();
}

void CChatRoomDlg::ScrollToBottom()
{
	DM_ScrollToBottom(0, 0);
}

void CChatRoomDlg::ShowFilterMenu()
{
	m_hwndFilter = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_FILTER), m_pContainer->m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(m_hwndFilter);

	RECT rcFilter, rcLog;
	GetClientRect(m_hwndFilter, &rcFilter);
	GetWindowRect(m_log.GetHwnd(), &rcLog);

	POINT pt;
	pt.x = rcLog.right; pt.y = rcLog.bottom;
	ScreenToClient(m_pContainer->m_hwnd, &pt);

	SetWindowPos(m_hwndFilter, HWND_TOP, pt.x - rcFilter.right, pt.y - rcFilter.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CChatRoomDlg::UpdateNickList()
{
	int i = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
	m_nickList.SendMsg(LB_SETCOUNT, m_si->nUsersInNicklist, 0);
	m_nickList.SendMsg(LB_SETTOPINDEX, i, 0);
	UpdateTitle();
	m_hTabIcon = m_hTabStatusIcon;
}

void CChatRoomDlg::UpdateOptions()
{
	MODULEINFO *pInfo = m_si ? pci->MM_FindModule(m_si->pszModule) : nullptr;
	if (pInfo) {
		m_btnBold.Enable(pInfo->bBold);
		m_btnItalic.Enable(pInfo->bItalics);
		m_btnUnderline.Enable(pInfo->bUnderline);
		m_btnColor.Enable(pInfo->bColor);
		m_btnBkColor.Enable(pInfo->bBkgColor);
		if (m_si->iType == GCW_CHATROOM)
			m_btnChannelMgr.Enable(pInfo->bChanMgr);
	}
	m_log.SendMsg(EM_SETBKGNDCOLOR, 0, M.GetDword(FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

	DM_InitRichEdit();
	m_btnOk.SendMsg(BUTTONSETASNORMAL, TRUE, 0);

	m_nickList.SetItemHeight(0, g_Settings.iNickListFontHeight);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(m_bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);
	Resize();
	RedrawLog2();
}

void CChatRoomDlg::UpdateStatusBar()
{
	if (m_pContainer->m_hwndActive != m_hwnd || m_pContainer->hwndStatus == 0 || CMimAPI::m_shutDown || m_wszStatusBar[0])
		return;

	if (m_si->pszModule == nullptr)
		return;

	//Mad: strange rare crash here...
	MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
	if (!mi)
		return;

	if (!mi->ptszModDispName)
		return;

	int x = 12;
	x += Chat_GetTextPixelSize(mi->ptszModDispName, (HFONT)SendMessage(m_pContainer->hwndStatus, WM_GETFONT, 0, 0), true);
	x += GetSystemMetrics(SM_CXSMICON);

	wchar_t szFinalStatusBarText[512];
	if (m_pPanel.isActive()) {
		time_t now = time(0);
		DWORD diff = (now - mi->idleTimeStamp) / 60;

		if ((diff >= 1 && diff != mi->lastIdleCheck)) {
			mi->lastIdleCheck = diff;
			if (diff > 59) {
				DWORD hours = diff / 60;
				DWORD minutes = diff % 60;
				mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s, %d %s idle"), 
					hours, hours > 1 ? TranslateT("hours") : TranslateT("hour"),
					minutes, minutes > 1 ? TranslateT("minutes") : TranslateT("minute"));
			}
			else mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s idle"), diff, diff > 1 ? TranslateT("minutes") : TranslateT("minute"));
		}
		mir_snwprintf(szFinalStatusBarText, TranslateT("%s on %s%s"), m_wszMyNickname, mi->ptszModDispName, mi->tszIdleMsg);
	}
	else {
		if (m_si->ptszStatusbarText)
			mir_snwprintf(szFinalStatusBarText, L"%s %s", mi->ptszModDispName, m_si->ptszStatusbarText);
		else
			wcsncpy_s(szFinalStatusBarText, mi->ptszModDispName, _TRUNCATE);
	}
	SendMessage(m_pContainer->hwndStatus, SB_SETTEXT, 0, (LPARAM)szFinalStatusBarText);
	tabUpdateStatusBar();
	m_pPanel.Invalidate();
	if (m_pWnd)
		m_pWnd->Invalidate();
}

void CChatRoomDlg::UpdateTitle()
{
	m_wStatus = m_si->wStatus;

	const wchar_t *szNick = m_cache->getNick();
	if (mir_wstrlen(szNick) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(szNick, m_wszTitle, _countof(m_wszTitle));
		else
			wcsncpy_s(m_wszTitle, szNick, _TRUNCATE);
	}

	wchar_t szTemp[100];
	HICON hIcon = 0;

	switch (m_si->iType) {
	case GCW_CHATROOM:
		hIcon = Skin_LoadProtoIcon(m_si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
		mir_snwprintf(szTemp,
			(m_si->nUsersInNicklist == 1) ? TranslateT("%s: chat room (%u user%s)") : TranslateT("%s: chat room (%u users%s)"),
			szNick, m_si->nUsersInNicklist, m_bFilterEnabled ? TranslateT(", event filter active") : L"");
		break;
	case GCW_PRIVMESS:
		hIcon = Skin_LoadProtoIcon(m_si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
		if (m_si->nUsersInNicklist == 1)
			mir_snwprintf(szTemp, TranslateT("%s: message session"), szNick);
		else
			mir_snwprintf(szTemp, TranslateT("%s: message session (%u users)"), szNick, m_si->nUsersInNicklist);
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

	if (m_cache->getStatus() != m_cache->getOldStatus()) {
		wcsncpy_s(m_wszStatus, pcli->pfnGetStatusModeDescription(m_wStatus, 0), _TRUNCATE);

		TCITEM item = {};
		item.mask = TCIF_TEXT;
		item.pszText = m_wszTitle;
		TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
	}
	SetWindowText(m_hwnd, szTemp);
	if (m_pContainer->m_hwndActive == m_hwnd) {
		m_pContainer->UpdateTitle(0, this);
		UpdateStatusBar();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the message history display(rich edit control in which the chat history appears)

LRESULT CChatRoomDlg::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_log.GetHwnd(), this, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_log.GetHwnd(), this, ID_EXTBKHISTORY, msg, wParam, lParam, stubLogProc);

	case WM_COPY:
		return WMCopyHandler(msg, wParam, lParam);

	case WM_SETCURSOR:
		if (g_Settings.bClickableNicks && (LOWORD(lParam) == HTCLIENT)) {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(m_log.GetHwnd(), &pt);
			if (CheckCustomLink(m_log.GetHwnd(), &pt, msg, wParam, lParam, FALSE)) return TRUE;
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		if (g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(m_log.GetHwnd(), &pt, msg, wParam, lParam, TRUE);
		}
		break;

	case WM_LBUTTONUP:
		if (g_Settings.bClickableNicks) {
			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			CheckCustomLink(m_log.GetHwnd(), &pt, msg, wParam, lParam, TRUE);
		}
		if (M.GetByte("autocopy", 1)) {
			CHARRANGE sel;
			SendMessage(m_log.GetHwnd(), EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				SendMessage(m_log.GetHwnd(), WM_COPY, 0, 0);
				sel.cpMin = sel.cpMax;
				SendMessage(m_log.GetHwnd(), EM_EXSETSEL, 0, (LPARAM)&sel);
				SetFocus(m_message.GetHwnd());
			}
		}
		break;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(m_hwnd, WM_CLOSE, 0, 1);
			return TRUE;
		}
		if (wParam == VK_INSERT && GetKeyState(VK_CONTROL) & 0x8000)
			return WMCopyHandler(msg, wParam, lParam);
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(m_log, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		m_bkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(m_log, msg, wParam, lParam)) {
			m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_bkeyProcessed) {
			m_bkeyProcessed = false;
			return 0;
		}
		break;

	case WM_CHAR:
		bool isCtrl, isShift, isAlt;
		KbdState(isShift, isCtrl, isAlt);
		if (wParam == 0x03 && isCtrl) // Ctrl+C
			return WMCopyHandler(msg, wParam, lParam);
		break;
	}

	return CSuper::WndProc_Log(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the message input control(a richedit text control)

LRESULT CChatRoomDlg::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_bkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(kstate);
		if (!(kstate[VK_CONTROL] & 0x80) && !(kstate[VK_SHIFT] & 0x80))
			m_bkeyProcessed = false;
		return 0;
	}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_CONTEXTMENU:
		POINT pt;
		GetCursorPos(&pt);
		{
			HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 2);
			RemoveMenu(hSubMenu, 9, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 4, MF_BYPOSITION);

			MODULEINFO *mi = pci->MM_FindModule(m_si->pszModule);
			EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, MF_BYCOMMAND | ((mi && mi->bBold) ? MF_ENABLED : MF_GRAYED));
			TranslateMenu(hSubMenu);

			CHARRANGE sel, all = { 0, -1 };
			m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hSubMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
			}

			MessageWindowPopupData mwpd = { sizeof(mwpd) };
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = m_hContact;
			mwpd.hwnd = m_message.GetHwnd();
			mwpd.hMenu = hSubMenu;
			mwpd.pt = pt;
			NotifyEventHooks(pci->hevWinPopup, 0, (LPARAM)&mwpd);

			int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);

			mwpd.selection = iSelection;
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(pci->hevWinPopup, 0, (LPARAM)&mwpd);

			switch (iSelection) {
			case IDM_COPY:
				m_message.SendMsg(WM_COPY, 0, 0);
				break;
			case IDM_CUT:
				m_message.SendMsg(WM_CUT, 0, 0);
				break;
			case IDM_PASTE:
			case IDM_PASTEFORMATTED:
				m_message.SendMsg(EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_UNICODETEXT : 0, 0);
				break;
			case IDM_COPYALL:
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				m_message.SendMsg(WM_COPY, 0, 0);
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
				break;
			case IDM_SELECTALL:
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				break;
			}
			DestroyMenu(hMenu);
		}
		return TRUE;

	case WM_MOUSEWHEEL:
		if (DM_MouseWheelHandler(wParam, lParam) == 0)
			return 0;

		m_iLastEnterTime = 0;
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSKEYDOWN:
		m_bkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam)) {
			m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_bkeyProcessed) {
			m_bkeyProcessed = false;						// preceeding key event has been processed by miranda hotkey service
			return 0;
		}

		if ((wParam >= '0' && wParam <= '9') && (GetKeyState(VK_MENU) & 0x8000)) {       // ALT-1 -> ALT-0 direct tab selection
			BYTE bChar = (BYTE)wParam;
			int iIndex = (bChar == '0') ? 10 : bChar - (BYTE)'0';
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_BY_INDEX, (LPARAM)iIndex);
			return 0;
		}
		break;

	case WM_CHAR:
		bool isShift, isAlt, isCtrl;
		KbdState(isShift, isCtrl, isAlt);

		if (PluginConfig.m_bSoundOnTyping && !isAlt && !isCtrl && !(m_pContainer->dwFlags & CNT_NOSOUND) && wParam != VK_ESCAPE && !(wParam == VK_TAB && PluginConfig.m_bAllowTab))
			Skin_PlaySound("SoundOnTyping");

		if (isCtrl && !isAlt && !isShift)
			switch (wParam) {
			case 0x17:
				PostMessage(m_hwnd, WM_CLOSE, 0, 1);
				return 0;
			}

		break;

	case WM_KEYDOWN:
		KbdState(isShift, isCtrl, isAlt);

		// sound on typing..
		if (PluginConfig.m_bSoundOnTyping && !isAlt && wParam == VK_DELETE)
			Skin_PlaySound("SoundOnTyping");

		if (wParam != VK_ESCAPE)
			if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
				return true;

		if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
			m_bInsertMode = !m_bInsertMode;
			m_message.OnChange(&m_message);
		}
		if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
			m_message.OnChange(&m_message);

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
					DM_ScrollToBottom(0, 0);
					return 0;
				}
				else if (wParam == VK_DOWN)
					wp = MAKEWPARAM(SB_LINEDOWN, 0);

				m_log.SendMsg(WM_VSCROLL, wp, 0);
				return 0;
			}
		}

		if (wParam == VK_RETURN) {
			if (isShift) {
				if (PluginConfig.m_bSendOnShiftEnter) {
					PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
					return 0;
				}
				break;
			}
			if ((isCtrl && !isShift) ^ (0 != PluginConfig.m_bSendOnEnter)) {
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (!PluginConfig.m_bSendOnEnter && !PluginConfig.m_bSendOnDblEnter)
				break;
			if (isCtrl)
				break;

			if (PluginConfig.m_bSendOnDblEnter) {
				if (m_iLastEnterTime + 2 < time(nullptr)) {
					m_iLastEnterTime = time(nullptr);
					break;
				}

				m_message.SendMsg(WM_KEYDOWN, VK_BACK, 0);
				m_message.SendMsg(WM_KEYUP, VK_BACK, 0);
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return 0;
			}
			PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
			return 0;
		}
		else m_iLastEnterTime = 0;

		if ((wParam == VK_NEXT && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && !isShift)) { // CTRL-TAB (switch tab/window)
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
			return TRUE;
		}

		if ((wParam == VK_PRIOR && isCtrl && !isShift) || (wParam == VK_TAB && isCtrl && isShift)) { // CTRL_SHIFT-TAB (switch tab/window)
			SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
			return TRUE;
		}
		if (wParam == VK_TAB && !isCtrl && !isShift) { // tab-autocomplete
			m_message.SendMsg(WM_SETREDRAW, FALSE, 0);
			bool fCompleted = TabAutoComplete();
			m_message.SendMsg(WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			if (!fCompleted && !PluginConfig.m_bAllowTab) {
				if ((GetSendButtonState(GetHwnd()) != PBS_DISABLED))
					SetFocus(m_btnOk.GetHwnd());
				else
					SetFocus(m_log.GetHwnd());
			}
			return 0;
		}

		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			replaceStrW(m_wszSearchQuery, nullptr);
			replaceStrW(m_wszSearchResult, nullptr);
		}

		if (wParam == VK_F4 && isCtrl && !isAlt) { // ctrl-F4 (close tab)
			CloseTab();
			return 0;
		}

		if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			m_log.SendMsg(msg, wParam, lParam);
			m_iLastEnterTime = 0;
			return 0;
		}

		if (wParam == VK_UP && isCtrl && !isAlt) {
			char *lpPrevCmd = pci->SM_GetPrevCommand(m_si->ptszID, m_si->pszModule);

			if (!m_si->lpCurrentCommand || !m_si->lpCurrentCommand->last) {
				// Next command is not defined. It means currently entered text is not saved in the history and it
				// need to be saved in the window context.
				char *enteredText = m_message.GetRichTextRtf();
				if (m_enteredText)
					mir_free(m_enteredText);

				m_enteredText = enteredText;
			}

			m_message.SendMsg(WM_SETREDRAW, FALSE, 0);

			LOGFONTA lf;
			LoadLogfont(FONTSECTION_IM, MSGFONTID_MESSAGEAREA, &lf, nullptr, FONTMODULE);

			SETTEXTEX ste;
			ste.flags = ST_DEFAULT;
			ste.codepage = CP_ACP;
			if (lpPrevCmd)
				m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
			else
				m_message.SetText(L"");

			GETTEXTLENGTHEX gtl = { 0 };
			gtl.flags = GTL_PRECISE;
			gtl.codepage = CP_ACP;
			int iLen = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			m_message.SendMsg(EM_SCROLLCARET, 0, 0);
			m_message.SendMsg(WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			m_message.SendMsg(EM_SETSEL, iLen, iLen);
			m_iLastEnterTime = 0;
			return 0;
		}

		if (wParam == VK_DOWN && isCtrl && !isAlt) {
			m_message.SendMsg(WM_SETREDRAW, FALSE, 0);

			SETTEXTEX ste;
			ste.flags = ST_DEFAULT;
			ste.codepage = CP_ACP;

			char *lpPrevCmd = pci->SM_GetNextCommand(m_si->ptszID, m_si->pszModule);
			if (lpPrevCmd)
				m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)lpPrevCmd);
			else if (m_enteredText) {
				// If we cannot load the message from history, load the last edited text.
				m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)m_enteredText);
				mir_free(m_enteredText);
				m_enteredText = nullptr;
			}

			GETTEXTLENGTHEX gtl = { 0 };
			gtl.flags = GTL_PRECISE;
			gtl.codepage = CP_ACP;
			int iLen = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			m_message.SendMsg(EM_SCROLLCARET, 0, 0);
			m_message.SendMsg(WM_SETREDRAW, TRUE, 0);
			RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
			m_message.SendMsg(EM_SETSEL, iLen, iLen);
			m_iLastEnterTime = 0;
			return 0;
		}
		// fall through

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		m_iLastEnterTime = 0;
		break;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		RefreshButtonStatus();
		break;

	case WM_INPUTLANGCHANGE:
		if (PluginConfig.m_bAutoLocaleSupport && GetFocus() == m_message.GetHwnd() && m_pContainer->m_hwndActive == m_hwnd && GetForegroundWindow() == m_pContainer->m_hwnd && GetActiveWindow() == m_pContainer->m_hwnd) {
			DM_SaveLocale(wParam, lParam);
			m_message.SendMsg(EM_SETLANGOPTIONS, 0, (LPARAM)m_message.SendMsg(EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
			return 1;
		}
		break;

	case WM_ERASEBKGND:
		return !CSkin::m_skinEnabled;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the nickname list control.It is an ownerdrawn listbox

LRESULT CChatRoomDlg::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCCALCSIZE:
		if (CSkin::m_DisableScrollbars) {
			RECT lpRect;
			GetClientRect(m_nickList.GetHwnd(), &lpRect);
			LONG itemHeight = m_nickList.SendMsg(LB_GETITEMHEIGHT, 0, 0);
			g_cLinesPerPage = (lpRect.bottom - lpRect.top) / itemHeight;
		}
		return CSkin::NcCalcRichEditFrame(m_nickList.GetHwnd(), this, ID_EXTBKUSERLIST, msg, wParam, lParam, stubNicklistProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_nickList.GetHwnd(), this, ID_EXTBKUSERLIST, msg, wParam, lParam, stubNicklistProc);

	case WM_MOUSEWHEEL:
		if (CSkin::m_DisableScrollbars) {
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
					m_nickList.SendMsg(WM_VSCROLL, SB_LINEUP, 0);
					dLines--;
				}
				else {
					m_nickList.SendMsg(WM_VSCROLL, SB_LINEDOWN, 0);
					dLines++;
				}
			}
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == 0x57 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w (close window)
			PostMessage(m_hwnd, WM_CLOSE, 0, 1);
			return TRUE;
		}

		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT || wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
			m_wszSearch[0] = 0;
			m_iSearchItem = -1;
		}
		break;

	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		m_wszSearch[0] = 0;
		m_iSearchItem = -1;
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		// simple incremental search for the user (nick) - list control
		// typing esc or movement keys will clear the current search string
		if (wParam == 27 && m_wszSearch[0]) { // escape - reset everything
			m_wszSearch[0] = 0;
			m_iSearchItem = -1;
			break;
		}
		if (wParam == '\b' && m_wszSearch[0])					// backspace
			m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
		else if (wParam < ' ')
			break;
		else {
			if (mir_wstrlen(m_wszSearch) >= _countof(m_wszSearch) - 2) {
				MessageBeep(MB_OK);
				break;
			}
			wchar_t szNew[2];
			szNew[0] = (wchar_t)wParam;
			szNew[1] = '\0';
			mir_wstrcat(m_wszSearch, szNew);
		}
		if (m_wszSearch[0]) {
			// iterate over the (sorted) list of nicknames and search for the
			// string we have
			int i, iItems = m_nickList.SendMsg(LB_GETCOUNT, 0, 0);
			for (i = 0; i < iItems; i++) {
				USERINFO *ui = pci->UM_FindUserFromIndex(m_si->pUsers, i);
				if (ui) {
					if (!wcsnicmp(ui->pszNick, m_wszSearch, mir_wstrlen(m_wszSearch))) {
						m_nickList.SendMsg(LB_SETSEL, FALSE, -1);
						m_nickList.SendMsg(LB_SETSEL, TRUE, i);
						m_iSearchItem = i;
						InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
						return 0;
					}
				}
			}
			if (i == iItems) {
				MessageBeep(MB_OK);
				m_wszSearch[mir_wstrlen(m_wszSearch) - 1] = '\0';
				return 0;
			}
		}
		break;
	}

	return CSuper::WndProc_Nicklist(msg, wParam, lParam);
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

INT_PTR CALLBACK CChatRoomDlg::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CChatRoomDlg *pDlg = (CChatRoomDlg*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CChatRoomDlg*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			DWORD dwMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", 0);
			DWORD dwFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", 0);

			DWORD dwPopupMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "PopupMask", 0);
			DWORD dwPopupFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "PopupFlags", 0);

			DWORD dwTrayMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask", 0);
			DWORD dwTrayFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags", 0);

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

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (pDlg) {
				if (dwMask == 0) {
					db_unset(pDlg->m_hContact, CHAT_MODULE, "FilterFlags");
					db_unset(pDlg->m_hContact, CHAT_MODULE, "FilterMask");
				}
				else {
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", iFlags);
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", dwMask);
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

			if (pDlg) {
				if (dwMask == 0) {
					db_unset(pDlg->m_hContact, CHAT_MODULE, "PopupFlags");
					db_unset(pDlg->m_hContact, CHAT_MODULE, "PopupMask");
				}
				else {
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "PopupFlags", iFlags);
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "PopupMask", dwMask);
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

			if (pDlg) {
				if (dwMask == 0) {
					db_unset(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags");
					db_unset(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask");
				}
				else {
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags", iFlags);
					db_set_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask", dwMask);
				}
				Chat_SetFilters(pDlg->m_si);
				if (pDlg->m_iLogFilterFlags == 0 && pDlg->m_bFilterEnabled)
					pDlg->onClick_Filter(&pDlg->m_btnFilter);
				if (pDlg->m_bFilterEnabled)
					pDlg->RedrawLog();
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

INT_PTR CChatRoomDlg::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt, tmp, cur;
	RECT rc;

	switch (uMsg) {
	case WM_SETFOCUS:
		if (CMimAPI::m_shutDown)
			break;

		UpdateWindowState(WM_SETFOCUS);
		SetFocus(m_message.GetHwnd());
		return 1;

	case WM_TIMECHANGE:
		RedrawLog();
		break;

	case WM_CBD_LOADICONS:
		Srmm_UpdateToolbarIcons(m_hwnd);
		return 0;

	case WM_SIZE:
		if (m_ipFieldHeight == 0)
			m_ipFieldHeight = CInfoPanel::m_ipConfig.height1;

		if (wParam == SIZE_MAXIMIZED)
			ScrollToBottom();

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

	case DM_TYPING:
		// Typing support for GCW_PRIVMESS sessions
		if (m_si->iType == GCW_PRIVMESS) {
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

			if (dis->CtlID == IDC_SRMM_NICKLIST) {
				int x_offset = 0;
				int index = dis->itemID;

				USERINFO *ui = pci->UM_FindUserFromIndex(m_si->pUsers, index);
				if (ui == nullptr)
					return TRUE;

				int height = dis->rcItem.bottom - dis->rcItem.top;
				if (height & 1)
					height++;
				int offset = (height == 10) ? 0 : height / 2;

				HICON hIcon = pci->SM_GetStatusIcon(m_si, ui);
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
					HICON icon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
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
					HICON icon = Skin_LoadProtoIcon(m_si->pszModule, ui->ContactStatus);
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
				Srmm_ClickToolbarIcon(m_hContact, idFrom, m_hwnd, 1);
		}
		break;

	case DM_SPLITTERMOVED:
		RECT rcLog;
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
			Resize();
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
			Resize();
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
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND)
			if (m_bCanFlashTab)
				FlashTab(true);

		// Typing support for GCW_PRIVMESS sessions
		if (m_si->iType == GCW_PRIVMESS && wParam == TIMERID_TYPE)
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
						if (!m_bkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
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
						m_bkeyProcessed = true;
						return _dlgReturn(m_hwnd, 1);
					}

					LRESULT mim_hotkey_check = Hotkey_Check(&message, TABSRMM_HK_SECTION_GC);
					if (mim_hotkey_check)
						m_bkeyProcessed = true;
					
					switch (mim_hotkey_check) {
					case TABSRMM_HK_CHANNELMGR:
						onClick_ChanMgr(&m_btnChannelMgr);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_FILTERTOGGLE:
						onClick_Filter(&m_btnFilter);
						InvalidateRect(m_btnFilter.GetHwnd(), nullptr, TRUE);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_LISTTOGGLE:
						onClick_ShowNickList(&m_btnNickList);
						return _dlgReturn(m_hwnd, 1);
					case TABSRMM_HK_MUC_SHOWSERVER:
						if (m_si->iType != GCW_SERVER)
							Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, L"/servershow", 0);
						return _dlgReturn(m_hwnd, 1);
					}
				}

				if (msg == WM_KEYDOWN && ((NMHDR*)lParam)->idFrom != IDC_SRMM_MESSAGE) {
					if ((wp == VK_NEXT && isCtrl && !isShift) || (wp == VK_TAB && isCtrl && !isShift)) // CTRL-TAB (switch tab/window)
						SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_NEXT, 0);
					else if ((wp == VK_PRIOR && isCtrl && !isShift) || (wp == VK_TAB && isCtrl && isShift)) // CTRL_SHIFT-TAB (switch tab/window)
						SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, DM_SELECT_PREV, 0);
				}					

				if (msg == WM_KEYDOWN && wp == VK_TAB) {
					if (((NMHDR*)lParam)->idFrom == IDC_SRMM_LOG) {
						SetFocus(m_message.GetHwnd());
						return _dlgReturn(m_hwnd, 1);
					}
				}

				if (((LPNMHDR)lParam)->idFrom == IDC_SRMM_LOG && ((MSGFILTER *)lParam)->msg == WM_RBUTTONUP)
					return _dlgReturn(m_hwnd, 1);
			}
			break;

		case EN_REQUESTRESIZE:
			if (((LPNMHDR)lParam)->idFrom == IDC_SRMM_MESSAGE)
				DM_HandleAutoSizeRequest((REQRESIZE *)lParam);
			break;

		case EN_LINK:
			if (((LPNMHDR)lParam)->idFrom == IDC_SRMM_LOG) {
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
								for (USERINFO *ui = m_si->pUsers; ui; ui = ui->next) {
									if (mir_wstrcmp(ui->pszNick, tr.lpstrText))
										continue;

									pt.x = (short)LOWORD(((ENLINK*)lParam)->lParam);
									pt.y = (short)HIWORD(((ENLINK*)lParam)->lParam);
									ClientToScreen(((NMHDR*)lParam)->hwndFrom, &pt);
									RunUserMenu(m_hwnd, ui, pt);
									break;
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
			SendMessage(m_pContainer->m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
		}
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&tmp);
		if (m_pPanel.isHovered())
			m_pPanel.handleClick(tmp);
		else {
			cur.x = (SHORT)tmp.x;
			cur.y = (SHORT)tmp.y;
			SendMessage(m_pContainer->m_hwnd, WM_NCLBUTTONUP, HTCAPTION, *((LPARAM*)(&cur)));
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
				SendMessage(m_pContainer->m_hwnd, DM_SELECTTAB, cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT, 0);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_TOGGLESIDEBAR:
			SendMessage(m_pContainer->m_hwnd, WM_COMMAND, IDC_TOGGLESIDEBAR, 0);
			break;

		case IDCANCEL:
			ShowWindow(m_pContainer->m_hwnd, SW_MINIMIZE);
			return FALSE;

		case IDC_CLOSE:
			CloseTab();
			break;

		case IDC_SELFTYPING:
			// Typing support for GCW_PRIVMESS sessions
			if (m_si->iType == GCW_PRIVMESS) {
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
			HDC hdc = (HDC)wParam;
			UINT item_ids[3] = { ID_EXTBKUSERLIST, ID_EXTBKHISTORY, ID_EXTBKINPUTAREA };
			UINT ctl_ids[3] = { IDC_SRMM_NICKLIST, IDC_SRMM_LOG, IDC_SRMM_MESSAGE };
			HANDLE hbp = nullptr;
			HDC hdcMem = nullptr;
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
				CSkin::SkinDrawBG(m_hwnd, m_pContainer->m_hwnd, m_pContainer, &rcClient, hdcMem);
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
			ScrollToBottom();
		break;

	case WM_CLOSE:
		if (wParam == 0 && lParam == 0) {
			if (GetCapture() != nullptr)
				return TRUE;

			if (PluginConfig.m_EscapeCloses == 1) {
				SendMessage(m_pContainer->m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;
			}
			if (PluginConfig.m_bHideOnClose && PluginConfig.m_EscapeCloses == 2) {
				ShowWindow(m_pContainer->m_hwnd, SW_HIDE);
				return TRUE;
			}
			_dlgReturn(m_hwnd, TRUE);
		}
		CloseTab();
		return 0;

	case DM_SAVESIZE:
		if (m_dwFlags & MWF_NEEDCHECKSIZE)
			lParam = 0;

		m_dwFlags &= ~MWF_NEEDCHECKSIZE;
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE)
			m_dwFlags &= ~MWF_INITMODE;

		SendMessage(m_pContainer->m_hwnd, DM_QUERYCLIENTAREA, 0, (LPARAM)&rcClient);
		MoveWindow(m_hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		
		if (m_dwFlags & MWF_WASBACKGROUNDCREATE) {
			m_dwFlags &= ~MWF_WASBACKGROUNDCREATE;
			Resize();

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
			Resize();
			if (lParam == 0)
				DM_ScrollToBottom(1, 1);
		}
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
		Resize();
		break;

	case DM_SMILEYOPTIONSCHANGED:
		RedrawLog();
		break;

	case EM_THEMECHANGED:
		DM_FreeTheme();
		DM_ThemeChanged();
		return 0;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme();
		m_pContainer->ClearMargins();
		VerifyProxy();
		break;
	}
	return CTabBaseDlg::DlgProc(uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat session creator

void ShowRoom(TContainerData *pContainer, SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	if (si->pDlg != nullptr) {
		ActivateExistingTab(si->pDlg->m_pContainer, si->pDlg->GetHwnd());
		return;
	}

	wchar_t szName[CONTAINER_NAMELEN + 2]; szName[0] = 0;

	if (pContainer == nullptr)
		if (si->pDlg != nullptr)
			pContainer = si->pDlg->m_pContainer;

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
	if (Srmm_FindWindow(hContact) != 0)
		return;

	if (hContact != 0 && M.GetByte("limittabs", 0) && !wcsncmp(pContainer->m_wszName, L"default", 6))
		if ((pContainer = FindMatchingContainer(L"default")) == nullptr)
			if ((pContainer = CreateContainer(L"default", CNT_CREATEFLAG_CLONED, hContact)) == nullptr)
				return;

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

	HWND hwndTab = GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS);

	// hide the active tab
	if (pContainer->m_hwndActive)
		ShowWindow(pContainer->m_hwndActive, SW_HIDE);

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
	item.mask = TCIF_TEXT | TCIF_IMAGE;
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

	SendMessage(pContainer->m_hwnd, WM_SIZE, 0, 0);
	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->m_hwnd)) {
		SendMessage(pContainer->m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetFocus(pContainer->m_hwndActive);
	}

	if (PluginConfig.m_bHideOnClose && !IsWindowVisible(pContainer->m_hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->m_hwnd, &wp);

		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);			// make sure all tabs will re-check layout on activation
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->m_hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(pContainer->m_hwnd, SW_SHOWNORMAL);
		}
		SendMessage(pContainer->m_hwndActive, WM_SIZE, 0, 0);
		SetFocus(hwndNew);
	}
	else {
		SetFocus(hwndNew);
		RedrawWindow(pContainer->m_hwnd, nullptr, nullptr, RDW_INVALIDATE);
		UpdateWindow(pContainer->m_hwnd);
		if (GetForegroundWindow() != pContainer->m_hwnd)
			SetForegroundWindow(pContainer->m_hwnd);
	}

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M.GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);
}
