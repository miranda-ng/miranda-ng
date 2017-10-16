/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project,
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

#include "chat.h"
#include "resource.h"
#include "skin.h"
#include <m_history.h>

CSrmmBaseDialog::CSrmmBaseDialog(HINSTANCE hInst, int idDialog, SESSION_INFO *si)
	: CDlgBase(hInst, idDialog),
	m_log(this, IDC_SRMM_LOG),
	m_message(this, IDC_SRMM_MESSAGE),
	m_nickList(this, IDC_SRMM_NICKLIST),

	m_btnFilter(this, IDC_SRMM_FILTER),
	m_btnHistory(this, IDC_SRMM_HISTORY),
	m_btnNickList(this, IDC_SRMM_SHOWNICKLIST),
	m_btnChannelMgr(this, IDC_SRMM_CHANMGR),

	m_btnColor(this, IDC_SRMM_COLOR),
	m_btnBkColor(this, IDC_SRMM_BKGCOLOR),
	m_btnBold(this, IDC_SRMM_BOLD),

	m_btnItalic(this, IDC_SRMM_ITALICS),
	m_btnUnderline(this, IDC_SRMM_UNDERLINE),

	m_si(si),
	m_hContact(0),
	m_clrInputBG(GetSysColor(COLOR_WINDOW))
{
	m_bFilterEnabled = db_get_b(0, CHAT_MODULE, "FilterEnabled", 0) != 0;
	m_bNicklistEnabled = db_get_b(0, CHAT_MODULE, "ShowNicklist", 1) != 0;
	m_iLogFilterFlags = db_get_dw(0, CHAT_MODULE, "FilterFlags", 0x03E0);

	m_btnColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BkColor);
	m_btnBold.OnClick = m_btnItalic.OnClick = m_btnUnderline.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BIU);

	m_btnHistory.OnClick = Callback(this, &CSrmmBaseDialog::onClick_History);
	m_btnChannelMgr.OnClick = Callback(this, &CSrmmBaseDialog::onClick_ChanMgr);

	m_nickList.OnDblClick = Callback(this, &CChatRoomDlg::onDblClick_List);

	if (si) {
		m_hContact = si->hContact;

		MODULEINFO *mi = chatApi.MM_FindModule(si->pszModule);
		if (mi == nullptr) {
			if (mi->bColor) {
				m_iFG = 4;
				m_bFGSet = true;
			}
			if (mi->bBkgColor) {
				m_iBG = 2;
				m_bBGSet = true;
			}
		}
	}
}

CSrmmBaseDialog::CSrmmBaseDialog(const CSrmmBaseDialog&) :
	CDlgBase(0, 0),
	m_btnColor(0, 0), m_btnBkColor(0, 0),
	m_btnBold(0, 0), m_btnItalic(0, 0), m_btnUnderline(0, 0),
	m_btnFilter(0, 0), m_btnChannelMgr(0, 0), m_btnHistory(0, 0), m_btnNickList(0, 0),
	m_nickList(0, 0), m_log(0, 0), m_message(0, 0)
{
}

CSrmmBaseDialog& CSrmmBaseDialog::operator=(const CSrmmBaseDialog&)
{
	return *this;
}

void CSrmmBaseDialog::RunUserMenu(HWND hwndOwner, USERINFO *ui, const POINT &pt)
{
	USERINFO uinew;
	memcpy(&uinew, ui, sizeof(USERINFO));
	HMENU hMenu = GetSubMenu(g_hMenu, 0);
	UINT uID = Chat_CreateGCMenu(hwndOwner, hMenu, pt, m_si, uinew.pszUID, uinew.pszNick);
	switch (uID) {
	case 0:
		break;

	case IDM_SENDMESSAGE:
		Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
		break;

	default:
		Chat_DoEventHook(m_si, GC_USER_NICKLISTMENU, ui, nullptr, uID);
		break;
	}
	Chat_DestroyGCMenu(hMenu, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK Srmm_ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) != 0) {
			CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
			if (pDlg == nullptr)
				break;

			switch (GetDlgCtrlID(hwnd)) {
			case IDC_SRMM_FILTER:
				pDlg->ShowFilterMenu();
				break;

			case IDC_SRMM_COLOR:
				pDlg->ShowColorChooser(IDC_SRMM_COLOR);
				break;

			case IDC_SRMM_BKGCOLOR:
				pDlg->ShowColorChooser(IDC_SRMM_BKGCOLOR);
				break;
			}
		}
		break;
	}

	return mir_callNextSubclass(hwnd, Srmm_ButtonSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubLogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (pDlg != nullptr)
		return pDlg->WndProc_Log(msg, wParam, lParam);

	return mir_callNextSubclass(hwnd, stubLogProc, msg, wParam, lParam);
}

LRESULT CSrmmBaseDialog::WndProc_Log(UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHARRANGE sel;

	switch (msg) {
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin != sel.cpMax) {
				sel.cpMin = sel.cpMax;
				m_log.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
			}
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(GetKeyState(VK_RMENU) & 0x8000)) {
			MSG message = { m_hwnd, msg, wParam, lParam };
			LRESULT iButtonFrom = Hotkey_Check(&message, BB_HK_SECTION);
			if (iButtonFrom) {
				Srmm_ProcessToolbarHotkey(m_hContact, iButtonFrom, m_hwnd);
				return TRUE;
			}
		}
		break;

	case WM_CHAR:
		SetFocus(m_message.GetHwnd());
		if (wParam != '\t')
			m_message.SendMsg(WM_CHAR, wParam, lParam);
		break;

	case WM_CONTEXTMENU:
		if (m_si == nullptr)
			break;

		POINT pt, ptl;
		m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
		if (lParam == 0xFFFFFFFF) {
			m_message.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
			ClientToScreen(m_log.GetHwnd(), &pt);
		}
		else {
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
		}
		ptl = pt;
		ScreenToClient(m_log.GetHwnd(), (LPPOINT)&ptl);
		{
			wchar_t *pszWord = (wchar_t*)_alloca(8192);
			pszWord[0] = '\0';

			int iCharIndex = m_log.SendMsg(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
			if (iCharIndex < 0)
				break;

			int start = m_log.SendMsg(EM_FINDWORDBREAK, WB_LEFT, iCharIndex);
			int end = m_log.SendMsg(EM_FINDWORDBREAK, WB_RIGHT, iCharIndex);

			if (end - start > 0) {
				static wchar_t szTrimString[] = L":;,.!?\'\"><()[]- \r\n";

				CHARRANGE cr;
				cr.cpMin = start;
				cr.cpMax = end;

				TEXTRANGE tr = { 0 };
				tr.chrg = cr;
				tr.lpstrText = (wchar_t*)pszWord;
				int iRes = m_log.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				if (iRes > 0) {
					size_t iLen = mir_wstrlen(pszWord) - 1;
					while (wcschr(szTrimString, pszWord[iLen])) {
						pszWord[iLen] = '\0';
						iLen--;
					}
				}
			}

			CHARRANGE all = { 0, -1 };
			HMENU hMenu = GetSubMenu(g_hMenu, 1);
			UINT uID = Chat_CreateGCMenu(m_log.GetHwnd(), hMenu, pt, m_si, nullptr, pszWord);
			switch (uID) {
			case 0:
				PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_COPYALL:
				m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&all);
				m_message.SendMsg(WM_COPY, 0, 0);
				m_message.SendMsg(EM_EXSETSEL, 0, (LPARAM)&sel);
				PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_CLEAR:
				m_log.SetText(L"");
				chatApi.LM_RemoveAll(&m_si->pLog, &m_si->pLogEnd);
				m_si->iEventCount = 0;
				m_si->LastTime = 0;
				PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			case IDM_SEARCH_GOOGLE:
			case IDM_SEARCH_BING:
			case IDM_SEARCH_YANDEX:
			case IDM_SEARCH_YAHOO:
			case IDM_SEARCH_WIKIPEDIA:
			case IDM_SEARCH_FOODNETWORK:
			case IDM_SEARCH_GOOGLE_MAPS:
			case IDM_SEARCH_GOOGLE_TRANSLATE:
				{
					CMStringW szURL;
					switch (uID) {
					case IDM_SEARCH_WIKIPEDIA:
						szURL.Format(L"http://en.wikipedia.org/wiki/%s", pszWord);
						break;
					case IDM_SEARCH_YAHOO:
						szURL.Format(L"http://search.yahoo.com/search?p=%s&ei=UTF-8", pszWord);
						break;
					case IDM_SEARCH_FOODNETWORK:
						szURL.Format(L"http://search.foodnetwork.com/search/delegate.do?fnSearchString=%s", pszWord);
						break;
					case IDM_SEARCH_BING:
						szURL.Format(L"http://www.bing.com/search?q=%s&form=OSDSRC", pszWord);
						break;
					case IDM_SEARCH_GOOGLE_MAPS:
						szURL.Format(L"http://maps.google.com/maps?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					case IDM_SEARCH_GOOGLE_TRANSLATE:
						szURL.Format(L"http://translate.google.com/?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					case IDM_SEARCH_YANDEX:
						szURL.Format(L"http://yandex.ru/yandsearch?text=%s", pszWord);
						break;
					case IDM_SEARCH_GOOGLE:
						szURL.Format(L"http://www.google.com/search?q=%s&ie=utf-8&oe=utf-8", pszWord);
						break;
					}
					Utils_OpenUrlW(szURL);
				}
				PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				break;

			default:
				PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
				Chat_DoEventHook(m_si, GC_USER_LOGMENU, nullptr, nullptr, uID);
				break;
			}
			Chat_DestroyGCMenu(hMenu, 5);
		}
		break;
	}

	LRESULT res = mir_callNextSubclass(m_log.GetHwnd(), stubLogProc, msg, wParam, lParam);
	if (msg == WM_GETDLGCODE)
		return res & ~DLGC_HASSETSEL;
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (pDlg != nullptr)
		return pDlg->WndProc_Message(msg, wParam, lParam);

	return mir_callNextSubclass(hwnd, stubMessageProc, msg, wParam, lParam);
}

LRESULT CSrmmBaseDialog::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = mir_callNextSubclass(m_message.GetHwnd(), stubMessageProc, msg, wParam, lParam);
	switch (msg) {
	case WM_GETDLGCODE:
		return res & ~DLGC_HASSETSEL;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!(GetKeyState(VK_RMENU) & 0x8000)) {
			MSG message = { m_hwnd, msg, wParam, lParam };
			LRESULT iButtonFrom = Hotkey_Check(&message, BB_HK_SECTION);
			if (iButtonFrom) {
				Srmm_ProcessToolbarHotkey(m_hContact, iButtonFrom, m_hwnd);
				return TRUE;
			}
		}
		break;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////
// process mouse - hovering for the nickname list.fires events so the protocol can
// show the userinfo - tooltip.

static void ProcessNickListHovering(HWND hwnd, int hoveredItem, SESSION_INFO *parentdat)
{
	static int currentHovered = -1;
	static HWND hwndToolTip = nullptr;
	static HWND oldParent = nullptr;

	if (hoveredItem == currentHovered)
		return;

	currentHovered = hoveredItem;

	if (oldParent != hwnd && hwndToolTip) {
		SendMessage(hwndToolTip, TTM_DELTOOL, 0, 0);
		DestroyWindow(hwndToolTip);
		hwndToolTip = nullptr;
	}

	if (hoveredItem == -1) {
		SendMessage(hwndToolTip, TTM_ACTIVATE, 0, 0);
		return;
	}

	bool bNewTip = false;
	if (!hwndToolTip) {
		bNewTip = true;
		hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwnd, nullptr, g_hInst, nullptr);
	}

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	TOOLINFO ti = { sizeof(ti) };
	ti.uFlags = TTF_SUBCLASS;
	ti.hinst = g_hInst;
	ti.hwnd = hwnd;
	ti.uId = 1;
	ti.rect = clientRect;

	wchar_t tszBuf[1024]; tszBuf[0] = 0;

	USERINFO *ui1 = chatApi.SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, currentHovered);
	if (ui1) {
		if (ProtoServiceExists(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			wchar_t *p = (wchar_t*)CallProtoService(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)parentdat->ptszID, (LPARAM)ui1->pszUID);
			if (p != nullptr) {
				wcsncpy_s(tszBuf, p, _TRUNCATE);
				mir_free(p);
			}
		}

		if (tszBuf[0] == 0)
			mir_snwprintf(tszBuf, L"%s: %s\r\n%s: %s\r\n%s: %s",
				TranslateT("Nickname"), ui1->pszNick,
				TranslateT("Unique ID"), ui1->pszUID,
				TranslateT("Status"), chatApi.TM_WordToString(parentdat->pStatuses, ui1->Status));
		ti.lpszText = tszBuf;
	}

	SendMessage(hwndToolTip, bNewTip ? TTM_ADDTOOL : TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hwndToolTip, TTM_ACTIVATE, (ti.lpszText != nullptr), 0);
	SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 400);
}

static void CALLBACK ChatTimerProc(HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
{
	SESSION_INFO *si = (SESSION_INFO*)idEvent;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	DWORD nItemUnderMouse = (DWORD)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
	if (HIWORD(nItemUnderMouse) == 1)
		nItemUnderMouse = (DWORD)(-1);
	else
		nItemUnderMouse &= 0xFFFF;
	if (((int)nItemUnderMouse != si->currentHovered) || (nItemUnderMouse == -1)) {
		KillTimer(hwnd, idEvent);
		return;
	}

	USERINFO *ui1 = chatApi.SM_GetUserFromIndex(si->ptszID, si->pszModule, si->currentHovered);
	if (ui1) {
		CMStringW wszBuf;
		if (ProtoServiceExists(si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			wchar_t *p = (wchar_t*)CallProtoService(si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)si->ptszID, (LPARAM)ui1->pszUID);
			if (p) {
				wszBuf = p;
				mir_free(p);
			}
		}
		if (wszBuf.IsEmpty())
			wszBuf.Format(L"<b>%s:</b>\t%s\n<b>%s:</b>\t%s\n<b>%s:</b>\t%s",
				TranslateT("Nick"), ui1->pszNick,
				TranslateT("Unique ID"), ui1->pszUID,
				TranslateT("Status"), chatApi.TM_WordToString(si->pStatuses, ui1->Status));

		CLCINFOTIP ti = { sizeof(ti) };
		if (CallService("mToolTip/ShowTipW", (WPARAM)wszBuf.c_str(), (LPARAM)&ti))
			si->bHasToolTip = true;
	}
	KillTimer(hwnd, idEvent);
}

EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubNicklistProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (pDlg != nullptr)
		return pDlg->WndProc_Nicklist(msg, wParam, lParam);

	return mir_callNextSubclass(hwnd, stubNicklistProc, msg, wParam, lParam);
}

LRESULT CSrmmBaseDialog::WndProc_Nicklist(UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;

	switch (msg) {
	case WM_RBUTTONDOWN:
		m_nickList.SendMsg(WM_LBUTTONDOWN, wParam, lParam);
		break;

	case WM_RBUTTONUP:
		m_nickList.SendMsg(WM_LBUTTONUP, wParam, lParam);
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

	case WM_MOUSEMOVE:
		RECT clientRect;
		{
			bool bTooltipExists = ServiceExists("mToolTip/HideTip");

			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			GetClientRect(m_nickList.GetHwnd(), &clientRect);
			if (PtInRect(&clientRect, pt)) {
				// hit test item under mouse
				DWORD nItemUnderMouse = m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(nItemUnderMouse) == 1)
					nItemUnderMouse = (DWORD)(-1);
				else
					nItemUnderMouse &= 0xFFFF;

				if (bTooltipExists) {
					if ((int)nItemUnderMouse == m_si->currentHovered)
						break;
					m_si->currentHovered = (int)nItemUnderMouse;

					KillTimer(m_nickList.GetHwnd(), 1);

					if (m_si->bHasToolTip) {
						CallService("mToolTip/HideTip", 0, 0);
						m_si->bHasToolTip = false;
					}

					if (nItemUnderMouse != -1)
						SetTimer(m_nickList.GetHwnd(), (UINT_PTR)m_si, 450, ChatTimerProc);
				}
				else ProcessNickListHovering(m_nickList.GetHwnd(), (int)nItemUnderMouse, m_si);
			}
			else {
				if (bTooltipExists) {
					KillTimer(m_nickList.GetHwnd(), 1);
					if (m_si->bHasToolTip) {
						CallService("mToolTip/HideTip", 0, 0);
						m_si->bHasToolTip = false;
					}
				}
				else ProcessNickListHovering(m_nickList.GetHwnd(), -1, nullptr);
			}
		}
		break;

	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc == nullptr)
				break;

			int index = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
			if (index == LB_ERR || m_si->nUsersInNicklist <= 0)
				break;

			int height = m_nickList.SendMsg(LB_GETITEMHEIGHT, 0, 0);
			if (height == LB_ERR)
				break;

			GetClientRect(m_nickList.GetHwnd(), &rc);

			int items = m_si->nUsersInNicklist - index;
			if (rc.bottom - rc.top > items * height) {
				rc.top = items * height;
				FillRect(dc, &rc, chatApi.hListBkgBrush);
			}
		}
		return 1;

	case WM_CONTEXTMENU:
		POINT pt;
		{
			int height = 0;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			if (pt.x == -1 && pt.y == -1) {
				int index = m_nickList.GetCurSel();
				int top = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
				height = m_nickList.SendMsg(LB_GETITEMHEIGHT, 0, 0);
				pt.x = 4;
				pt.y = (index - top)*height + 1;
			}
			else ScreenToClient(m_nickList.GetHwnd(), &pt);

			int item = LOWORD(m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y)));
			USERINFO *ui = chatApi.SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, item);
			if (ui != nullptr) {
				if (pt.x == -1 && pt.y == -1)
					pt.y += height - 4;
				ClientToScreen(m_nickList.GetHwnd(), &pt);

				RunUserMenu(m_nickList.GetHwnd(), ui, pt);
				return TRUE;
			}
		}
		break;
	}

	return mir_callNextSubclass(m_nickList.GetHwnd(), stubNicklistProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmBaseDialog::OnInitDialog()
{
	WindowList_Add(g_hWindowList, m_hwnd, m_hContact);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	SetWindowLongPtr(m_log.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_log.GetHwnd(), stubLogProc);
	m_log.SetReadOnly(true);

	SetWindowLongPtr(m_message.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_message.GetHwnd(), stubMessageProc);

	SetWindowLongPtr(m_nickList.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_nickList.GetHwnd(), stubNicklistProc);

	// three buttons below are initiated inside this call, so button creation must precede subclassing
	Srmm_CreateToolbarIcons(m_hwnd, isChat() ? BBBF_ISCHATBUTTON : BBBF_ISIMBUTTON);

	mir_subclassWindow(m_btnFilter.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), Srmm_ButtonSubclassProc);

	LoadSettings();
}

void CSrmmBaseDialog::OnDestroy()
{
	WindowList_Remove(g_hWindowList, m_hwnd);

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
	mir_unsubclassWindow(m_log.GetHwnd(), stubLogProc);
	mir_unsubclassWindow(m_message.GetHwnd(), stubMessageProc);
	mir_unsubclassWindow(m_nickList.GetHwnd(), stubNicklistProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CSrmmBaseDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_COMMAND:
		if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
			return 0;

		if (wParam >= MIN_CBUTTONID && wParam <= MAX_CBUTTONID) {
			Srmm_ClickToolbarIcon(m_hContact, wParam, m_hwnd, 0);
			return 0;
		}
		break;

	case WM_NOTIFY:
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->hwndFrom == m_log.GetHwnd() && hdr->code == EN_LINK) {
			ENLINK *pLink = (ENLINK*)lParam;
			switch (pLink->msg) {
			case WM_SETCURSOR:
				SetCursor(g_hCurHyperlinkHand);
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;

			case WM_RBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
				CHARRANGE sel;
				m_log.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				if (sel.cpMin != sel.cpMax)
					break;

				CMStringW wszText(' ', pLink->chrg.cpMax - pLink->chrg.cpMin + 1);

				TEXTRANGE tr;
				tr.chrg = pLink->chrg;
				tr.lpstrText = wszText.GetBuffer();
				m_log.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				if (wcschr(tr.lpstrText, '@') != nullptr && wcschr(tr.lpstrText, ':') == nullptr && wcschr(tr.lpstrText, '/') == nullptr)
					wszText.Insert(0, L"mailto:");

				if (pLink->msg == WM_RBUTTONDOWN) {
					HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
					HMENU hSubMenu = GetSubMenu(hMenu, 6);
					TranslateMenu(hSubMenu);

					POINT pt = { GET_X_LPARAM(pLink->lParam), GET_Y_LPARAM(pLink->lParam) };
					ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr)) {
					case IDM_OPENLINK:
						Utils_OpenUrlW(wszText);
						break;

					case IDM_COPYLINK:
						if (OpenClipboard(m_hwnd)) {
							EmptyClipboard();
							HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (wszText.GetLength() + 1) * sizeof(wchar_t));
							mir_wstrcpy((wchar_t*)GlobalLock(hData), wszText);
							GlobalUnlock(hData);
							SetClipboardData(CF_UNICODETEXT, hData);
							CloseClipboard();
						}
						break;
					}

					DestroyMenu(hMenu);
					SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}

				Utils_OpenUrlW(wszText);
				SetFocus(m_message.GetHwnd());
			}
		}
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CSrmmBaseDialog::AddLog()
{
	if (m_si->pLogEnd)
		StreamInEvents(m_si->pLog, false);
	else
		ClearLog();
}

void CSrmmBaseDialog::ClearLog()
{
	m_log.SetText(L"");
}

void CSrmmBaseDialog::RedrawLog2()
{
	m_si->LastTime = 0;
	if (m_si->pLog)
		StreamInEvents(m_si->pLogEnd, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmBaseDialog::onClick_Color(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;
	cf.dwMask = CFM_COLOR;

	if (IsDlgButtonChecked(m_hwnd, pButton->GetCtrlId())) {
		if (db_get_b(0, CHAT_MODULE, "RightClickFilter", 0) == 0) {
			ShowColorChooser(pButton->GetCtrlId());
			return;
		}
		if (m_bFGSet)
			cf.crTextColor = m_iFG;
	}
	else cf.crTextColor = m_clrInputFG;

	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_BkColor(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwEffects = 0;
	cf.dwMask = CFM_BACKCOLOR;

	if (IsDlgButtonChecked(m_hwnd, pButton->GetCtrlId())) {
		if (!db_get_b(0, CHAT_MODULE, "RightClickFilter", 0)) {
			ShowColorChooser(pButton->GetCtrlId());
			return;
		}
		if (m_bBGSet)
			cf.crBackColor = m_iBG;
	}
	else cf.crBackColor = m_clrInputBG;

	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_BIU(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD))
		cf.dwEffects |= CFE_BOLD;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS))
		cf.dwEffects |= CFE_ITALIC;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE))
		cf.dwEffects |= CFE_UNDERLINE;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_History(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	if (m_si != nullptr) {
		MODULEINFO *pInfo = chatApi.MM_FindModule(m_si->pszModule);
		if (pInfo)
			ShellExecute(m_hwnd, nullptr, chatApi.GetChatLogsFilename(m_si, 0), nullptr, nullptr, SW_SHOW);
	}
	else CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
}

void CSrmmBaseDialog::onClick_ChanMgr(CCtrlButton *pButton)
{
	if (pButton->Enabled())
		Chat_DoEventHook(m_si, GC_USER_CHANMGR, nullptr, nullptr, 0);
}

void CSrmmBaseDialog::onDblClick_List(CCtrlListBox *pList)
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(pList->GetHwnd(), &hti.pt);

	int item = LOWORD(pList->SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
	USERINFO *ui = chatApi.UM_FindUserFromIndex(m_si->pUsers, item);
	if (ui == nullptr)
		return;

	bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	if (g_Settings->bDoubleClick4Privat ? bShift : !bShift) {
		int selStart = LOWORD(m_message.SendMsg(EM_GETSEL, 0, 0));
		CMStringW tszName(ui->pszNick);
		if (selStart == 0)
			tszName.AppendChar(':');
		tszName.AppendChar(' ');

		m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)tszName.GetString());
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		SetFocus(m_message.GetHwnd());
	}
	else Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern HANDLE hHookSrmmEvent;

int CSrmmBaseDialog::NotifyEvent(int code)
{
	if (m_hContact == 0 && m_hwnd == nullptr)
		return -1;

	MessageWindowEventData mwe = {};
	mwe.hContact = m_hContact;
	mwe.hwndWindow = m_hwnd;
	mwe.uType = code;
	mwe.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwe.hwndInput = m_message.GetHwnd();
	mwe.hwndLog = m_log.GetHwnd();
	return ::NotifyEventHooks(hHookSrmmEvent, 0, (LPARAM)&mwe);
}

bool CSrmmBaseDialog::ProcessHotkeys(int key, bool isShift, bool isCtrl, bool isAlt)
{
	// Esc (close tab)
	if (key == VK_ESCAPE && !isShift && !isCtrl && !isAlt) {
		CloseTab();
		return true;
	}

	if (isCtrl && !isAlt) {
		switch (key) {
		case VK_SPACE: // ctrl-space (paste clean text)
			m_btnBold.Push(false); onClick_BIU(&m_btnBold);
			m_btnItalic.Push(false); onClick_BIU(&m_btnItalic);
			m_btnUnderline.Push(false); onClick_BIU(&m_btnUnderline);

			m_btnColor.Push(false); onClick_Color(&m_btnColor);
			m_btnBkColor.Push(false); onClick_BkColor(&m_btnBkColor);
			return true;

		case 0x42: // ctrl-b (bold)
			m_btnBold.Push(!m_btnBold.IsPushed());
			onClick_BIU(&m_btnBold);
			return true;

		case 0x48: // ctrl-h (history)
			onClick_History(&m_btnHistory);
			return true;

		case 0x49: // ctrl-i (italics)
			m_btnItalic.Push(!m_btnItalic.IsPushed());
			onClick_BIU(&m_btnItalic);
			return true;

		case 0x4b: // ctrl-k (paste clean text)
			m_btnColor.Push(!m_btnColor.IsPushed());
			onClick_Color(&m_btnColor);
			return true;

		case 0x4c: // ctrl-l (back color)
			m_btnBkColor.Push(!m_btnBkColor.IsPushed());
			onClick_BkColor(&m_btnBkColor);
			return true;

		case 0x55: // ctrl-u (underlining)
			m_btnUnderline.Push(!m_btnUnderline.IsPushed());
			onClick_BIU(&m_btnUnderline);
			return true;

		case VK_F4: // ctrl-F4 (close tab)
			CloseTab();
			return true;
		}
	}

	return false;
}

void CSrmmBaseDialog::RefreshButtonStatus(void)
{
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
	m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	MODULEINFO *mi = chatApi.MM_FindModule(m_si->pszModule);
	if (mi == nullptr)
		return;

	if (mi->bColor) {
		bool bState = m_btnColor.IsPushed();
		if (!bState && cf.crTextColor != m_clrInputFG)
			m_btnColor.Push(true);
		else if (bState && cf.crTextColor == m_clrInputFG)
			m_btnColor.Push(false);
	}

	if (mi->bBkgColor) {
		bool bState = m_btnBkColor.IsPushed();
		if (!bState && cf.crBackColor != m_clrInputBG)
			m_btnBkColor.Push(true);
		else if (bState && cf.crBackColor == m_clrInputBG)
			m_btnBkColor.Push(false);
	}

	if (mi->bBold) {
		bool bState = m_btnBold.IsPushed();
		UINT u2 = cf.dwEffects & CFE_BOLD;
		if (!bState && u2 != 0)
			m_btnBold.Push(true);
		else if (bState && u2 == 0)
			m_btnBold.Push(false);
	}

	if (mi->bItalics) {
		bool bState = m_btnItalic.IsPushed();
		UINT u2 = cf.dwEffects & CFE_ITALIC;
		if (!bState && u2 != 0)
			m_btnItalic.Push(true);
		else if (bState && u2 == 0)
			m_btnItalic.Push(false);
	}

	if (mi->bUnderline) {
		bool bState = m_btnUnderline.IsPushed();
		UINT u2 = cf.dwEffects & CFE_UNDERLINE;
		if (!bState && u2 != 0)
			m_btnUnderline.Push(true);
		else if (bState && u2 == 0)
			m_btnUnderline.Push(false);
	}
}
