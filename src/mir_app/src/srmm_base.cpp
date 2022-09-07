/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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

CSrmmBaseDialog::CSrmmBaseDialog(CMPluginBase &pPlugin, int idDialog, SESSION_INFO *si) :
	CDlgBase(pPlugin, idDialog),
	timerFlash(this, 1),
	timerType(this, 2),

	m_message(this, IDC_SRMM_MESSAGE),
	m_nickList(this, IDC_SRMM_NICKLIST),

	m_btnOk(this, IDOK),
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

	m_nickList.OnDblClick = Callback(this, &CMsgDialog::onDblClick_List);

	if (si) {
		m_hContact = si->hContact;

		if (si->pMI->bColor) {
			m_iFG = 4;
			m_bFGSet = true;
		}
		if (si->pMI->bBkgColor) {
			m_iBG = 2;
			m_bBGSet = true;
		}
	}
}

void CSrmmBaseDialog::RunUserMenu(HWND hwndOwner, USERINFO *ui, const POINT &pt)
{
	HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_USERMENU));
	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	TranslateMenu(hSubMenu);

	USERINFO uinew;
	memcpy(&uinew, ui, sizeof(USERINFO));

	wchar_t szTemp[50];
	if (uinew.pszNick)
		mir_snwprintf(szTemp, TranslateT("&Message %s"), uinew.pszNick);
	else
		mir_wstrncpy(szTemp, TranslateT("&Message"), _countof(szTemp) - 1);

	if (mir_wstrlen(szTemp) > 40)
		mir_wstrncpy(szTemp + 40, L"...", 4);
	ModifyMenu(hMenu, 0, MF_STRING | MF_BYPOSITION, IDM_SENDMESSAGE, szTemp);

	UINT uID = Chat_CreateMenu(hwndOwner, hSubMenu, pt, m_si, uinew.pszUID);
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
	DestroyMenu(hMenu);
}

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK Srmm_ButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_RBUTTONUP:
		if (g_chatApi.bRightClickFilter) {
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

EXTERN_C MIR_APP_DLL(LRESULT) CALLBACK stubMessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CSrmmBaseDialog *pDlg = (CSrmmBaseDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (pDlg != nullptr)
		return pDlg->WndProc_Message(msg, wParam, lParam);

	return mir_callNextSubclass(hwnd, stubMessageProc, msg, wParam, lParam);
}

LRESULT CSrmmBaseDialog::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SETCURSOR:
		if (m_bInMenu) {
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return TRUE;
		}
		break;

	case WM_CHAR:
		switch (wParam) {
		case 0x02: // ctrl+B
			if (m_btnBold.Enabled())
				return 1;
			break;
		case 0x09: // ctrl+I
			if (m_btnItalic.Enabled())
				return 1;
			break;
		case 0x15: // ctrl+U
			if (m_btnUnderline.Enabled())
				return 1;
			break;
		}
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if (wParam == VK_BACK)
			if (m_message.GetRichTextLength() == 0)
				return 1;

		MSG tmp = { m_hwnd, msg, wParam, lParam };
		if (Hotkey_Check(&tmp, g_pszHotkeySection) == 100) {
			if (!(GetWindowLongPtr(m_message.GetHwnd(), GWL_STYLE) & ES_READONLY)) {
				PostMessage(m_hwnd, WM_COMMAND, IDOK, 0);
				return true;
			}
		}
	}

	LRESULT res = mir_callNextSubclass(m_message.GetHwnd(), stubMessageProc, msg, wParam, lParam);
	switch (msg) {
	case WM_GETDLGCODE:
		return res & ~DLGC_HASSETSEL;

	case WM_KEYUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		RefreshButtonStatus();
		break;

	case WM_KEYDOWN:
		if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'V' || (GetKeyState(VK_SHIFT) & 0x8000) && wParam == VK_INSERT) {
			if (IsClipboardFormatAvailable(CF_HDROP)) {
				m_message.SendMsg(WM_PASTE, 0, 0);
				return 0;
			}
		}

		__fallthrough;

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
			hwnd, nullptr, g_plugin.getInst(), nullptr);
	}

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	TOOLINFO ti = { sizeof(ti) };
	ti.uFlags = TTF_SUBCLASS;
	ti.hinst = g_plugin.getInst();
	ti.hwnd = hwnd;
	ti.uId = 1;
	ti.rect = clientRect;

	CMStringW wszBuf;

	USERINFO *ui1 = g_chatApi.SM_GetUserFromIndex(parentdat->ptszID, parentdat->pszModule, currentHovered);
	if (ui1) {
		if (ProtoServiceExists(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			wchar_t *p = (wchar_t*)CallProtoService(parentdat->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)parentdat->ptszID, (LPARAM)ui1->pszUID);
			if (p != nullptr) {
				wszBuf = p;
				mir_free(p);
			}
		}

		if (wszBuf.IsEmpty())
			wszBuf.Format(L"%s: %s\r\n%s: %s\r\n%s: %s",
				TranslateT("Nickname"), ui1->pszNick,
				TranslateT("Unique ID"), ui1->pszUID,
				TranslateT("Status"), g_chatApi.TM_WordToString(parentdat->pStatuses, ui1->Status));
		ti.lpszText = wszBuf.GetBuffer();
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

	uint32_t nItemUnderMouse = (uint32_t)SendMessage(hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
	if (HIWORD(nItemUnderMouse) == 1)
		nItemUnderMouse = (uint32_t)(-1);
	else
		nItemUnderMouse &= 0xFFFF;
	if (((int)nItemUnderMouse != si->currentHovered) || (nItemUnderMouse == -1)) {
		KillTimer(hwnd, idEvent);
		return;
	}

	USERINFO *ui1 = g_chatApi.SM_GetUserFromIndex(si->ptszID, si->pszModule, si->currentHovered);
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
				TranslateT("Status"), g_chatApi.TM_WordToString(si->pStatuses, ui1->Status));

		CLCINFOTIP ti = { sizeof(ti) };
		Tipper_ShowTip(wszBuf, &ti);
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
			bool bTooltipExists = ServiceExists(MS_TIPPER_HIDETIP);

			POINT pt = { LOWORD(lParam), HIWORD(lParam) };
			GetClientRect(m_nickList.GetHwnd(), &clientRect);
			if (PtInRect(&clientRect, pt)) {
				// hit test item under mouse
				uint32_t nItemUnderMouse = m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(nItemUnderMouse) == 1)
					nItemUnderMouse = (uint32_t)(-1);
				else
					nItemUnderMouse &= 0xFFFF;

				if (bTooltipExists) {
					if ((int)nItemUnderMouse == m_si->currentHovered)
						break;
					m_si->currentHovered = (int)nItemUnderMouse;

					KillTimer(m_nickList.GetHwnd(), 1);

					if (m_si->bHasToolTip) {
						Tipper_Hide();
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
						Tipper_Hide();
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

			int nUsers = m_si->getUserList().getCount();

			int index = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
			if (index == LB_ERR || nUsers <= 0)
				break;

			int height = m_nickList.SendMsg(LB_GETITEMHEIGHT, 0, 0);
			if (height == LB_ERR)
				break;

			GetClientRect(m_nickList.GetHwnd(), &rc);

			int items = nUsers - index;
			if (rc.bottom - rc.top > items * height) {
				rc.top = items * height;
				FillRect(dc, &rc, g_chatApi.hListBkgBrush);
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
			USERINFO *ui = g_chatApi.SM_GetUserFromIndex(m_si->ptszID, m_si->pszModule, item);
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

bool CSrmmBaseDialog::OnInitDialog()
{
	WindowList_Add(g_hWindowList, m_hwnd, m_hContact);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);

	m_pLog = Srmm_GetLogWindow((CMsgDialog*)this);
	if (m_pLog->GetType() != 0) { // custom log type
		HWND hwndLog = GetDlgItem(m_hwnd, IDC_SRMM_LOG);
		EnableWindow(hwndLog, FALSE);
		ShowWindow(hwndLog, SW_HIDE);
	}
	m_pLog->Attach();

	SetWindowLongPtr(m_message.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_message.GetHwnd(), stubMessageProc);
	m_message.SetReadOnly(false);
	::DragAcceptFiles(m_message.GetHwnd(), TRUE);

	if (isChat()) {
		SetWindowLongPtr(m_nickList.GetHwnd(), GWLP_USERDATA, LPARAM(this));
		mir_subclassWindow(m_nickList.GetHwnd(), stubNicklistProc);
	}

	// three buttons below are initiated inside this call, so button creation must precede subclassing
	Srmm_CreateToolbarIcons(m_hwnd, isChat() ? BBBF_ISCHATBUTTON : BBBF_ISIMBUTTON);

	mir_subclassWindow(m_btnFilter.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), Srmm_ButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), Srmm_ButtonSubclassProc);

	LoadSettings();
	return true;
}

void CSrmmBaseDialog::OnDestroy()
{
	m_pLog->Detach();
	delete m_pLog;

	WindowList_Remove(g_hWindowList, m_hwnd);

	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, 0);
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

	case WM_ACTIVATE:
		if (m_si && LOWORD(wParam) == WA_INACTIVE) {
			m_si->wState &= ~GC_EVENT_HIGHLIGHT;
			m_si->wState &= ~STATE_TALK;
		}
		break;

	case WM_CBD_RECREATE:
		Srmm_CreateToolbarIcons(m_hwnd, isChat() ? BBBF_ISCHATBUTTON : BBBF_ISIMBUTTON);
		break;

	case WM_NOTIFY:
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->hwndFrom == m_pLog->GetHwnd())
			m_pLog->Notify(wParam, lParam);
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CSrmmBaseDialog::AddLog()
{
	if (m_si->pLogEnd)
		m_pLog->LogEvents(m_si->pLog, false);
	else
		m_pLog->Clear();
}

bool CSrmmBaseDialog::AllowTyping() const
{
	return isChat() ? m_si->iType != GCW_SERVER : true;
}

void CSrmmBaseDialog::ClearLog()
{
	m_pLog->Clear();
}

void CSrmmBaseDialog::UpdateOptions()
{
	MODULEINFO *mi = m_si->pMI;
	EnableWindow(m_btnBold.GetHwnd(), mi->bBold);
	EnableWindow(m_btnItalic.GetHwnd(), mi->bItalics);
	EnableWindow(m_btnUnderline.GetHwnd(), mi->bUnderline);
	EnableWindow(m_btnColor.GetHwnd(), mi->bColor);
	EnableWindow(m_btnBkColor.GetHwnd(), mi->bBkgColor);
	if (m_si->iType == GCW_CHATROOM)
		EnableWindow(m_btnChannelMgr.GetHwnd(), mi->bChanMgr);

	Resize();
	RedrawLog2(m_si);
}

/////////////////////////////////////////////////////////////////////////////////////////

void RedrawLog2(SESSION_INFO *si)
{
	si->LastTime = 0;
	if (si->pLog)
		si->pDlg->log()->LogEvents(si->pLogEnd, TRUE);
}

static void __cdecl phase2(SESSION_INFO *si)
{
	Sleep(30);
	if (si && si->pDlg)
		RedrawLog2(si);
}

void CSrmmBaseDialog::RedrawLog()
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
				if (m_si->iType != GCW_CHATROOM || !m_bFilterEnabled || (m_iLogFilterFlags & pLog->iType) != 0)
					index++;
			}
			m_pLog->LogEvents(pLog, true);
			mir_forkThread<SESSION_INFO>(phase2, m_si);
		}
		else m_pLog->LogEvents(m_si->pLogEnd, true);
	}
	else ClearLog();
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
		if (!g_chatApi.bRightClickFilter) {
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
		if (!g_chatApi.bRightClickFilter) {
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

	if (m_si != nullptr)
		ShellExecute(m_hwnd, nullptr, g_chatApi.GetChatLogsFilename(m_si, 0), nullptr, nullptr, SW_SHOW);
	else
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
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
	USERINFO *ui = g_chatApi.UM_FindUserFromIndex(m_si, item);
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
	mwe.hwndLog = m_pLog->GetHwnd();
	return ::NotifyEventHooks(hHookSrmmEvent, 0, (LPARAM)&mwe);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CSrmmBaseDialog::ProcessFileDrop(HDROP hDrop, MCONTACT hContact)
{
	if (PasteFilesAsURL(hDrop))
		return true;

	return ::ProcessFileDrop(hDrop, hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// If enabled pastes droped files as list of URL of file:/// type
// Can be enabled/disabled by Chat/ShiftDropFilePasteURL database parameter
// @param hDrop - Drop handle
// @return Returns true if processed here, returns false if should be processed elsewhere

bool CSrmmBaseDialog::PasteFilesAsURL(HDROP hDrop)
{
	bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	if (db_get_b(0, CHAT_MODULE, "ShiftDropFilePasteURL", 1) == 0 || !isShift) // hidden setting: Chat/ShiftDropFilePasteURL
		return false;

	int fileCount = DragQueryFileW(hDrop, -1, nullptr, 0);
	if (fileCount == 0)
		return true;

	CMStringW pasteString(L" ");
	for (int i = 0; i < fileCount; i++) {
		wchar_t szFilename[MAX_PATH];
		if (DragQueryFileW(hDrop, i, szFilename, _countof(szFilename))) {
			CMStringW fileString(L"file:///");
			fileString.Append(szFilename);
			fileString.Replace(L"%", L"%25");
			fileString.Replace(L" ", L"%20");
			fileString.Append((i != fileCount - 1) ? L"\r\n" : L" ");
			pasteString += fileString;
		}
	}

	m_message.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)pasteString.c_str());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
			m_btnBold.Push(false); m_btnBold.Click();
			m_btnItalic.Push(false); m_btnItalic.Click();
			m_btnUnderline.Push(false); m_btnUnderline.Click();

			m_btnColor.Push(false); m_btnColor.Click();
			m_btnBkColor.Push(false); m_btnBkColor.Click();
			return true;

		case 0x42: // ctrl-b (bold)
			m_btnBold.Push(!m_btnBold.IsPushed());
			m_btnBold.Click();
			return true;

		case 0x48: // ctrl-h (history)
			m_btnHistory.Click();
			return true;

		case 0x49: // ctrl-i (italics)
			m_btnItalic.Push(!m_btnItalic.IsPushed());
			m_btnItalic.Click();
			return true;

		case 0x4b: // ctrl-k (text color)
			m_btnColor.Push(!m_btnColor.IsPushed());
			m_btnColor.Click();
			return true;

		case 0x4c: // ctrl-l (back color)
			m_btnBkColor.Push(!m_btnBkColor.IsPushed());
			m_btnBkColor.Click();
			return true;

		case 0x55: // ctrl-u (underlining)
			m_btnUnderline.Push(!m_btnUnderline.IsPushed());
			m_btnUnderline.Click();
			return true;

		case VK_F4: // ctrl-F4 
			CloseTab();
			return true;
		}
	}

	return false;
}

void CSrmmBaseDialog::RefreshButtonStatus()
{
	if (m_si == nullptr)
		return;

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
	m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	if (m_si->pMI->bColor) {
		bool bState = m_btnColor.IsPushed();
		if (!bState && cf.crTextColor != m_clrInputFG)
			m_btnColor.Push(true);
		else if (bState && cf.crTextColor == m_clrInputFG)
			m_btnColor.Push(false);
	}

	if (m_si->pMI->bBkgColor) {
		bool bState = m_btnBkColor.IsPushed();
		if (!bState && cf.crBackColor != m_clrInputBG)
			m_btnBkColor.Push(true);
		else if (bState && cf.crBackColor == m_clrInputBG)
			m_btnBkColor.Push(false);
	}

	if (m_si->pMI->bBold) {
		bool bState = m_btnBold.IsPushed();
		UINT u2 = cf.dwEffects & CFE_BOLD;
		if (!bState && u2 != 0)
			m_btnBold.Push(true);
		else if (bState && u2 == 0)
			m_btnBold.Push(false);
	}

	if (m_si->pMI->bItalics) {
		bool bState = m_btnItalic.IsPushed();
		UINT u2 = cf.dwEffects & CFE_ITALIC;
		if (!bState && u2 != 0)
			m_btnItalic.Push(true);
		else if (bState && u2 == 0)
			m_btnItalic.Push(false);
	}

	if (m_si->pMI->bUnderline) {
		bool bState = m_btnUnderline.IsPushed();
		UINT u2 = cf.dwEffects & CFE_UNDERLINE;
		if (!bState && u2 != 0)
			m_btnUnderline.Push(true);
		else if (bState && u2 == 0)
			m_btnUnderline.Push(false);
	}
}
