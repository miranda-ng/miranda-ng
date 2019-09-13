/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-19 Miranda NG team,
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

void CMsgDialog::UpdateWindowState(UINT msg)
{
	if (m_si == nullptr)
		return;

	if (msg == WM_ACTIVATE) {
		if (m_pContainer->m_dwFlags & CNT_TRANSPARENCY) {
			DWORD trans = LOWORD(m_pContainer->m_pSettings->dwTransparency);
			SetLayeredWindowAttributes(m_pContainer->m_hwnd, CSkin::m_ContainerColorKey, (BYTE)trans, (m_pContainer->m_dwFlags & CNT_TRANSPARENCY ? LWA_ALPHA : 0));
		}
	}

	if (m_hwndFilter) {
		POINT pt;
		GetCursorPos(&pt);

		RECT rcFilter;
		GetWindowRect(m_hwndFilter, &rcFilter);
		if (!PtInRect(&rcFilter, pt)) {
			SendMessage(m_hwndFilter, WM_CLOSE, 1, 1);
			m_hwndFilter = nullptr;
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
		m_pWnd->setOverlayIcon(nullptr, true);
	}

	if (m_pContainer->m_hwndSaved == m_hwnd)
		return;

	m_pContainer->m_hwndSaved = m_hwnd;

	g_chatApi.SetActiveSession(m_si);
	m_hTabIcon = m_hTabStatusIcon;

	if (m_iTabID >= 0) {
		if (db_get_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
		if (g_clistApi.pfnGetEvent(m_si->hContact, 0))
			g_clistApi.pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);

		UpdateTitle();
		m_hTabIcon = m_hTabStatusIcon;
		m_dwTickLastEvent = 0;
		m_dwFlags &= ~MWF_DIVIDERSET;
		if (KillTimer(m_hwnd, TIMERID_FLASHWND) || m_iFlashIcon) {
			FlashTab(false);
			m_bCanFlashTab = FALSE;
			m_iFlashIcon = nullptr;
		}
		if (m_pContainer->m_dwFlashingStarted != 0) {
			FlashContainer(m_pContainer, 0, 0);
			m_pContainer->m_dwFlashingStarted = 0;
		}
		m_pContainer->m_dwFlags &= ~CNT_NEED_UPDATETITLE;

		if (m_dwFlags & MWF_NEEDCHECKSIZE)
			PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

		SetFocus(m_message.GetHwnd());
		m_dwLastActivity = GetTickCount();
		m_pContainer->m_dwLastActivity = m_dwLastActivity;
		m_pContainer->m_pMenuBar->configureMenu();
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
	if (m_pContainer->m_dwFlags & CNT_SIDEBAR)
		m_pContainer->m_pSideBar->setActiveItem(this);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::ScrollToBottom()
{
	DM_ScrollToBottom(0, 0);
}

void CMsgDialog::ShowFilterMenu()
{
	m_hwndFilter = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER), m_pContainer->m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(m_hwndFilter);

	RECT rcFilter, rcLog;
	GetClientRect(m_hwndFilter, &rcFilter);
	GetWindowRect(m_log.GetHwnd(), &rcLog);

	POINT pt;
	pt.x = rcLog.right; pt.y = rcLog.bottom;
	ScreenToClient(m_pContainer->m_hwnd, &pt);

	SetWindowPos(m_hwndFilter, HWND_TOP, pt.x - rcFilter.right, pt.y - rcFilter.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CMsgDialog::UpdateNickList()
{
	int i = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
	m_nickList.SendMsg(LB_SETCOUNT, m_si->getUserList().getCount(), 0);
	m_nickList.SendMsg(LB_SETTOPINDEX, i, 0);
	UpdateTitle();
	m_hTabIcon = m_hTabStatusIcon;
}

void CMsgDialog::UpdateOptions()
{
	MODULEINFO *pInfo = m_si ? m_si->pMI : nullptr;
	if (pInfo) {
		m_btnBold.Enable(pInfo->bBold);
		m_btnItalic.Enable(pInfo->bItalics);
		m_btnUnderline.Enable(pInfo->bUnderline);
		m_btnColor.Enable(pInfo->bColor);
		m_btnBkColor.Enable(pInfo->bBkgColor);
		if (m_si->iType == GCW_CHATROOM)
			m_btnChannelMgr.Enable(pInfo->bChanMgr);
	}
	m_log.SendMsg(EM_SETBKGNDCOLOR, 0, db_get_dw(0, FONTMODULE, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR));

	DM_InitRichEdit();
	m_btnOk.SendMsg(BUTTONSETASNORMAL, TRUE, 0);

	m_nickList.SetItemHeight(0, g_Settings.iNickListFontHeight);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(m_bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);
	Resize();
	RedrawLog2();
}

void CMsgDialog::UpdateStatusBar()
{
	if (m_pContainer->m_hwndActive != m_hwnd || m_pContainer->m_hwndStatus == nullptr || CMimAPI::m_shutDown || m_wszStatusBar[0])
		return;

	if (m_si->pszModule == nullptr)
		return;

	//Mad: strange rare crash here...
	MODULEINFO *mi = m_si->pMI;
	if (!mi->ptszModDispName)
		return;

	int x = 12;
	x += Chat_GetTextPixelSize(mi->ptszModDispName, (HFONT)SendMessage(m_pContainer->m_hwndStatus, WM_GETFONT, 0, 0), true);
	x += GetSystemMetrics(SM_CXSMICON);

	wchar_t szFinalStatusBarText[512];
	if (m_pPanel.isActive()) {
		time_t now = time(0);
		DWORD diff = (now - mi->idleTimeStamp) / 60;
		if (diff >= 1) {																
			if (diff > 59) {
				DWORD hours = diff / 60;
				DWORD minutes = diff % 60;
				mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s, %d %s idle"), 
					hours, hours > 1 ? TranslateT("hours") : TranslateT("hour"),
					minutes, minutes > 1 ? TranslateT("minutes") : TranslateT("minute"));
			}
			else mir_snwprintf(mi->tszIdleMsg, TranslateT(", %d %s idle"), diff, diff > 1 ? TranslateT("minutes") : TranslateT("minute"));
		}
		else mi->tszIdleMsg[0] = 0;

		mir_snwprintf(szFinalStatusBarText, TranslateT("%s on %s%s"), m_wszMyNickname, mi->ptszModDispName, mi->tszIdleMsg);
	}
	else {
		if (m_si->ptszStatusbarText)
			mir_snwprintf(szFinalStatusBarText, L"%s %s", mi->ptszModDispName, m_si->ptszStatusbarText);
		else
			wcsncpy_s(szFinalStatusBarText, mi->ptszModDispName, _TRUNCATE);
	}
	SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)szFinalStatusBarText);
	tabUpdateStatusBar();
	m_pPanel.Invalidate();
	if (m_pWnd)
		m_pWnd->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclassing for the nickname list control.It is an ownerdrawn listbox

LRESULT CMsgDialog::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
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
				USERINFO *ui = g_chatApi.UM_FindUserFromIndex(m_si, i);
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

INT_PTR CALLBACK CMsgDialog::FilterWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMsgDialog *pDlg = (CMsgDialog*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CMsgDialog*)lParam;
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
			int iFlags = 0;
			DWORD dwMask = 0;

			for (int i = 0; i < _countof(_eventorder); i++) {
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

			for (int i = 0; i < _countof(_eventorder); i++) {
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

			for (int i = 0; i < _countof(_eventorder); i++) {
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

				if (pDlg->m_bFilterEnabled) {
					if (pDlg->m_iLogFilterFlags == 0)
						pDlg->onClick_Filter(&pDlg->m_btnFilter);
					pDlg->RedrawLog();
					db_set_b(pDlg->m_hContact, CHAT_MODULE, "FilterEnabled", pDlg->m_bFilterEnabled);
				}
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
	if (Srmm_FindWindow(hContact) != nullptr)
		return;

	if (hContact != 0 && M.GetByte("limittabs", 0) && !wcsncmp(pContainer->m_wszName, L"default", 6))
		if ((pContainer = FindMatchingContainer(L"default")) == nullptr)
			if ((pContainer = CreateContainer(L"default", CNT_CREATEFLAG_CLONED, hContact)) == nullptr)
				return;

	wchar_t *contactName = Clist_GetContactDisplayName(hContact);

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

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->m_iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);

	pContainer->m_iTabIndex = iCount;
	if (iCount > 0) {
		for (int i = iCount - 1; i >= 0; i--) {
			HWND hwnd = GetTabWindow(hwndTab, i);
			CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->m_hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->m_iTabIndex = i;
			}
		}
	}

	TCITEM item = {};
	item.pszText = newcontactname;
	item.mask = TCIF_TEXT | TCIF_IMAGE;
	int iTabId = TabCtrl_InsertItem(hwndTab, pContainer->m_iTabIndex, &item);

	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	TabCtrl_SetCurSel(hwndTab, iTabId);
	pContainer->m_iChilds++;

	CMsgDialog *pDlg = new CMsgDialog(si);
	pDlg->m_iTabID = iTabId;
	pDlg->m_pContainer = pContainer;
	pDlg->SetParent(hwndTab);
	pDlg->Create();

	HWND hwndNew = pDlg->GetHwnd();
	item.lParam = (LPARAM)hwndNew;
	TabCtrl_SetItem(hwndTab, iTabId, &item);

	if (pContainer->m_dwFlags & CNT_SIDEBAR)
		pContainer->m_pSideBar->addSession(pDlg, pContainer->m_iTabIndex);

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

	if (IsWinVer7Plus() && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M.GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);
}
