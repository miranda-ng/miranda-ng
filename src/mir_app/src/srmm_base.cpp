/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team,
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
#include "skin.h"

CSrmmBaseDialog::CSrmmBaseDialog(CMPluginBase &pPlugin, int idDialog, MCONTACT hContact) :
	CDlgBase(pPlugin, idDialog),
	timerFlash(this, 1),
	timerType(this, 2),
	timerNickList(this, 3),
	timerRedraw(this, 4),

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
	m_btnStrikeout(this, IDC_SRMM_STRIKEOUT),

	m_Quote(this, IDC_SRMM_QUOTE),
	m_btnCloseQuote(this, IDC_SRMM_CLOSEQUOTE, SKINICON_OTHER_DELETE, LPGEN("Remove quoting")),

	m_si(0),
	m_hContact(hContact),
	m_clrInputBG(GetSysColor(COLOR_WINDOW))
{
	m_btnColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_Color);
	m_btnBkColor.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BkColor);
	m_btnBold.OnClick = m_btnItalic.OnClick = m_btnUnderline.OnClick = m_btnStrikeout.OnClick = Callback(this, &CSrmmBaseDialog::onClick_BIU);

	m_btnFilter.OnClick = Callback(this, &CSrmmBaseDialog::onClick_Filter);
	m_btnHistory.OnClick = Callback(this, &CSrmmBaseDialog::onClick_History);
	m_btnChannelMgr.OnClick = Callback(this, &CSrmmBaseDialog::onClick_ChanMgr);

	m_btnCloseQuote.OnClick = Callback(this, &CSrmmBaseDialog::onClick_CloseQuote);

	m_nickList.OnDblClick = Callback(this, &CSrmmBaseDialog::onDblClick_List);
	m_nickList.OnBuildMenu = Callback(this, &CSrmmBaseDialog::onContextMenu);

	timerRedraw.OnEvent = Callback(this, &CSrmmBaseDialog::OnRedrawTimer);

	if (Contact::IsGroupChat(hContact)) {
		m_si = Chat_Find(hContact);

		m_iFG = 4;
		m_iBG = 2;
		m_bFGSet = m_bBGSet = true;

		m_bFilterEnabled = db_get_b(m_hContact, CHAT_MODULE, "FilterEnabled", Chat::bFilterEnabled) != 0;
		m_iLogFilterFlags = Chat::iFilterFlags;
		m_bNicklistEnabled = Chat::bShowNicklist;

		timerNickList.OnEvent = Callback(this, &CSrmmBaseDialog::OnNickListTimer);
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

	Chat_CreateMenu(hSubMenu, m_si, uinew.pszUID);
	UINT uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwndOwner, nullptr);
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

static LRESULT CALLBACK stubButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

	return mir_callNextSubclass(hwnd, stubButtonSubclassProc, msg, wParam, lParam);
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

	case WM_PASTE:
	case EM_PASTESPECIAL:
		if (OpenClipboard(m_message.GetHwnd())) {
			if (HANDLE hBitmap = GetClipboardData(CF_BITMAP))
				SendHBitmapAsFile((HBITMAP)hBitmap, m_hContact);
			else if (HANDLE hDrop = GetClipboardData(CF_HDROP))
				ProcessFileDrop((HDROP)hDrop, m_hContact);

			CloseClipboard();
		}
		break;

	case WM_DROPFILES:
		ProcessFileDrop((HDROP)wParam, m_hContact);
		return 0;

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
			if (IsClipboardFormatAvailable(CF_HDROP) || IsClipboardFormatAvailable(CF_BITMAP)) {
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
				ProcessToolbarHotkey(iButtonFrom);
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

static void ProcessNickListHovering(const CCtrlListBox &listBox, int hoveredItem, SESSION_INFO *si)
{
	static HWND hwndToolTip = nullptr;
	static HWND oldParent = nullptr;

	MWindow hwnd = listBox.GetHwnd();
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
	if (auto *ui = (USERINFO *)listBox.GetItemData(si->currentHovered)) {
		if (ProtoServiceExists(si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			wchar_t *p = (wchar_t*)CallProtoService(si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)si->ptszID, (LPARAM)ui->pszUID);
			if (p != nullptr) {
				wszBuf = p;
				mir_free(p);
			}
		}

		if (wszBuf.IsEmpty())
			wszBuf.Format(L"%s: %s\r\n%s: %s\r\n%s: %s",
				TranslateT("Nickname"), ui->pszNick,
				TranslateT("Unique ID"), ui->pszUID,
				TranslateT("Status"), TM_WordToString(si, ui->Status));
		ti.lpszText = wszBuf.GetBuffer();
	}

	SendMessage(hwndToolTip, bNewTip ? TTM_ADDTOOL : TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	SendMessage(hwndToolTip, TTM_ACTIVATE, (ti.lpszText != nullptr), 0);
	SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 400);
}

void CSrmmBaseDialog::OnNickListTimer(CTimer *pTimer)
{
	pTimer->Stop();

	if (auto *ui = (USERINFO *)m_nickList.GetItemData(m_si->currentHovered)) {
		CMStringW wszBuf;
		if (ProtoServiceExists(m_si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT)) {
			wchar_t *p = (wchar_t *)CallProtoService(m_si->pszModule, MS_GC_PROTO_GETTOOLTIPTEXT, (WPARAM)m_si->ptszID, (LPARAM)ui->pszUID);
			if (p) {
				wszBuf = p;
				mir_free(p);
			}
		}
		if (wszBuf.IsEmpty())
			wszBuf.Format(L"<b>%s:</b>\t%s\n<b>%s:</b>\t%s\n<b>%s:</b>\t%s",
				TranslateT("Nick"), ui->pszNick,
				TranslateT("Unique ID"), ui->pszUID,
				TranslateT("Status"), TM_WordToString(m_si, ui->Status));

		CLCINFOTIP ti = { sizeof(ti) };
		Tipper_ShowTip(wszBuf, &ti);
		m_si->bHasToolTip = true;
	}
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
				int nItemUnderMouse = m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(nItemUnderMouse) == 1)
					nItemUnderMouse = -1;
				else
					nItemUnderMouse &= 0xFFFF;

				if (nItemUnderMouse == m_si->currentHovered)
					break;
				m_si->currentHovered = nItemUnderMouse;

				if (bTooltipExists) {
					timerNickList.Stop();

					if (m_si->bHasToolTip) {
						Tipper_Hide();
						m_si->bHasToolTip = false;
					}

					if (nItemUnderMouse != -1)
						timerNickList.Start(450);
				}
				else ProcessNickListHovering(m_nickList, nItemUnderMouse, m_si);
			}
			else {
				if (bTooltipExists) {
					timerNickList.Stop();

					if (m_si->bHasToolTip) {
						Tipper_Hide();
						m_si->bHasToolTip = false;
					}
				}
				else ProcessNickListHovering(m_nickList, -1, nullptr);
			}
		}
		break;

	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)wParam;
			if (dc == nullptr)
				break;

			int nUsers = m_nickList.GetCount();

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

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			int index = m_nickList.GetCurSel();
			if (index != LB_ERR)
				if (auto *ui = (USERINFO *)m_nickList.GetItemData(index))
					Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
		}

		if (wParam == VK_ESCAPE || wParam == VK_UP || wParam == VK_DOWN || wParam == VK_NEXT || wParam == VK_PRIOR || wParam == VK_TAB || wParam == VK_HOME || wParam == VK_END) {
			m_wszNickSearch.Empty();
			m_iNickSearch = -1;
		}
		break;


	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		m_wszNickSearch.Empty();
		m_iNickSearch = -1;
		break;

	case WM_CHAR:
	case WM_UNICHAR:
		// simple incremental search for the user (nick) - list control
		// typing esc or movement keys will clear the current search string
		if (wParam == 27 && !m_wszNickSearch.IsEmpty()) { // escape - reset everything
			m_wszNickSearch.Empty();
			m_iNickSearch = -1;
			break;
		}
		if (wParam == '\b' && !m_wszNickSearch.IsEmpty())					// backspace
			m_wszNickSearch.Truncate(m_wszNickSearch.GetLength() - 1);
		else if (wParam < ' ')
			break;
		else
			m_wszNickSearch.AppendChar((wchar_t)wParam);

		if (!m_wszNickSearch.IsEmpty()) {
			m_iNickSearch = m_nickList.SendMsg(LB_FINDSTRING, -1, LPARAM(m_wszNickSearch.c_str()));
			if (m_iNickSearch == LB_ERR) {
				MessageBeep(MB_OK);
				m_wszNickSearch.Truncate(m_wszNickSearch.GetLength() - 1);
				return 0;
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

	m_btnCloseQuote.Hide();
	m_Quote.Hide();

	m_bReadOnly = Contact::IsReadonly(m_hContact);
	if (m_bReadOnly)
		m_message.Hide();

	auto *pDlg = (CMsgDialog *)this;
	if (auto *pLogWindowClass = Srmm_GetWindowClass(pDlg)) {
		m_pLog = pLogWindowClass->pfnBuilder(*pDlg);
		m_pLog->Attach();
		if (m_pLog->GetType() != 0) { // custom log type
			DestroyWindow(GetDlgItem(m_hwnd, IDC_SRMM_LOG));
			SetWindowLong(m_pLog->GetHwnd(), GWLP_ID, IDC_SRMM_LOG);
		}
	}

	SetWindowLongPtr(m_message.GetHwnd(), GWLP_USERDATA, LPARAM(this));
	mir_subclassWindow(m_message.GetHwnd(), stubMessageProc);
	m_message.SetReadOnly(false);
	::DragAcceptFiles(m_message.GetHwnd(), TRUE);

	if (isChat()) {
		m_si->pDlg = (CMsgDialog*)this;
		Chat_SetFilters(m_si);

		if (m_si->bHasNicklist) {
			SetWindowLongPtr(m_nickList.GetHwnd(), GWLP_USERDATA, LPARAM(this));
			mir_subclassWindow(m_nickList.GetHwnd(), stubNicklistProc);
		}
		else {
			m_bNicklistEnabled = false;
			m_btnNickList.Hide();
		}
	}

	// three buttons below are initiated inside this call, so button creation must precede subclassing
	Srmm_CreateToolbarIcons(this, isChat() ? BBBF_ISCHATBUTTON : BBBF_ISIMBUTTON);

	if (Chat::bShowFormatting)
		m_bSendFormat = true;
	else
		m_bSendFormat = ((CallContactService(m_hContact, PS_GETCAPS, PFLAGNUM_4) & PF4_SERVERFORMATTING) != 0);
	if (!m_bSendFormat) {
		m_btnBold.Disable();
		m_btnItalic.Disable();
		m_btnStrikeout.Disable();
		m_btnUnderline.Disable();
		m_btnColor.Disable();
		m_btnBkColor.Disable();
	}

	mir_subclassWindow(m_btnFilter.GetHwnd(), stubButtonSubclassProc);
	mir_subclassWindow(m_btnColor.GetHwnd(), stubButtonSubclassProc);
	mir_subclassWindow(m_btnBkColor.GetHwnd(), stubButtonSubclassProc);

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

static void doMarkEventRead(MCONTACT hContact, MEVENT hEvent)
{
	db_event_markRead(hContact, hEvent);
	Clist_RemoveEvent(-1, hEvent);
}

wchar_t *wszBbcodes[N_CUSTOM_BBCODES] = { L"[img]", L"[code]", L"[quote]" };

INT_PTR CSrmmBaseDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case DM_OPTIONSAPPLIED:
		OnOptionsApplied();
		return 0;

	case WM_SHOWWINDOW:
		if (wParam) {
			m_bActive = true;
			for (auto &it : m_arDisplayedEvents)
				doMarkEventRead(m_hContact, it);
			m_arDisplayedEvents.clear();
		}
		else m_bActive = false;
		break;

	case WM_COMMAND:
		if (!lParam && Clist_MenuProcessCommand(LOWORD(wParam), MPCF_CONTACTMENU, m_hContact))
			return 0;

		if (wParam >= MIN_CBUTTONID && wParam <= MAX_CBUTTONID) {
			Srmm_ClickToolbarIcon(m_hContact, wParam, m_hwnd, 0);
			return 0;
		}

		if (wParam == IDC_CODE) {
			if (lParam > 0 && lParam <= _countof(wszBbcodes))
				InsertBbcodeString(wszBbcodes[lParam-1]);
			else
				Srmm_ClickToolbarIcon(m_hContact, wParam, m_hwnd, 0);
		}
		break;

	case WM_ACTIVATE:
		if (m_si && LOWORD(wParam) == WA_INACTIVE) {
			m_si->wState &= ~GC_EVENT_HIGHLIGHT;
			m_si->wState &= ~STATE_TALK;
		}
		break;

	case WM_DROPFILES:
		ProcessFileDrop((HDROP)wParam, m_hContact);
		return 0;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			if (dis->CtlID == IDC_SRMM_NICKLIST && dis->itemID != LB_ERR) {
				UINT_PTR iData = m_nickList.GetItemData(dis->itemID);
				if (iData != LB_ERR) {
					if (auto *ui = (USERINFO *)iData) {
						if (!ui->isValid())
							DebugBreak();
						DrawNickList(ui, dis);
					}
					return TRUE;
				}
			}
		}
		break;

	case WM_CBD_RECREATE:
		Srmm_CreateToolbarIcons(this, isChat() ? BBBF_ISCHATBUTTON : BBBF_ISIMBUTTON);
		break;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_NOTIFY:
		LPNMHDR hdr = (LPNMHDR)lParam;
		if (hdr->hwndFrom == m_pLog->GetHwnd())
			m_pLog->Notify(wParam, lParam);
		
		if (hdr->code == EN_MSGFILTER) {
			auto *F = ((MSGFILTER *)lParam);
			if ((F->msg == WM_LBUTTONDOWN || F->msg == WM_KEYUP || F->msg == WM_LBUTTONUP) && F->nmhdr.idFrom == IDC_SRMM_MESSAGE) {
				int bBold = IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD);
				int bItalic = IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS);
				int bUnder = IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE);
				int bStrikeout = IsDlgButtonChecked(m_hwnd, IDC_SRMM_STRIKEOUT);

				CHARFORMAT2 cf2;
				cf2.cbSize = sizeof(CHARFORMAT2);
				cf2.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_UNDERLINETYPE | CFM_STRIKEOUT;
				cf2.dwEffects = 0;
				m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf2);
				if (cf2.dwEffects & CFE_BOLD) {
					if (bBold == BST_UNCHECKED)
						CheckDlgButton(m_hwnd, IDC_SRMM_BOLD, BST_CHECKED);
				}
				else if (bBold == BST_CHECKED)
					CheckDlgButton(m_hwnd, IDC_SRMM_BOLD, BST_UNCHECKED);

				if (cf2.dwEffects & CFE_ITALIC) {
					if (bItalic == BST_UNCHECKED)
						CheckDlgButton(m_hwnd, IDC_SRMM_ITALICS, BST_CHECKED);
				}
				else if (bItalic == BST_CHECKED)
					CheckDlgButton(m_hwnd, IDC_SRMM_ITALICS, BST_UNCHECKED);

				if (cf2.dwEffects & CFE_UNDERLINE && (cf2.bUnderlineType & CFU_UNDERLINE || cf2.bUnderlineType & CFU_UNDERLINEWORD)) {
					if (bUnder == BST_UNCHECKED)
						CheckDlgButton(m_hwnd, IDC_SRMM_UNDERLINE, BST_CHECKED);
				}
				else if (bUnder == BST_CHECKED)
					CheckDlgButton(m_hwnd, IDC_SRMM_UNDERLINE, BST_UNCHECKED);

				if (cf2.dwEffects & CFE_STRIKEOUT) {
					if (bStrikeout == BST_UNCHECKED)
						CheckDlgButton(m_hwnd, IDC_SRMM_STRIKEOUT, BST_CHECKED);
				}
				else if (bStrikeout == BST_CHECKED)
					CheckDlgButton(m_hwnd, IDC_SRMM_STRIKEOUT, BST_UNCHECKED);
			}

			if ((hdr->idFrom == IDC_SRMM_LOG || hdr->idFrom == IDC_SRMM_MESSAGE) && F->msg == WM_RBUTTONUP) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}

		}
		break;
	}

	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CSrmmBaseDialog::AddLog(const LOGINFO &lin)
{
	if (IsSuitableEvent(lin))
		m_pLog->LogChatEvents(&lin);
}

bool CSrmmBaseDialog::AllowTyping() const
{
	return isChat() ? m_si->iType != GCW_SERVER : true;
}

void CSrmmBaseDialog::ClearLog()
{
	m_pLog->Clear();
}

void CSrmmBaseDialog::InsertBbcodeString(const wchar_t *pwszStr)
{
	CMStringW wszBbcode(pwszStr);
	wszBbcode.Insert(1, '/');
	
	LRESULT sel = m_message.SendMsg(EM_GETSEL, 0, 0);
	if (sel != 0) {
		int start = LOWORD(sel), end = HIWORD(sel);
		m_message.SendMsg(EM_SETSEL, end, end);
		m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)wszBbcode.c_str());
		
		m_message.SendMsg(EM_SETSEL, start, start);
		m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)pwszStr);
	}
	else {
		wszBbcode.Insert(0, pwszStr);
		SetMessageText(wszBbcode, true);
	}
}

bool CSrmmBaseDialog::IsSuitableEvent(const LOGINFO &lin) const
{
	return (m_si->iType == GCW_SERVER || (m_iLogFilterFlags & lin.iType));
}

void CSrmmBaseDialog::MarkEventRead(const DB::EventInfo &dbei)
{
	if (dbei.bRead)
		return;

	if (m_bActive)
		doMarkEventRead(m_hContact, dbei.getEvent());
	else {
		m_arDisplayedEvents.push_back(dbei.getEvent());
	}
}

void CSrmmBaseDialog::UpdateChatOptions()
{
	UpdateFilterButton();

	MODULEINFO *mi = m_si->pMI;
	if (m_si->iType == GCW_CHATROOM)
		m_btnChannelMgr.Enable(mi->bChanMgr);

	Resize();
}

void CSrmmBaseDialog::SetMessageText(const wchar_t *pwszText, bool bAppend)
{
	if (!pwszText)
		pwszText = L"";

	SETTEXTEX stx;
	stx.codepage = 1200;
	stx.flags = (bAppend) ? ST_SELECTION : ST_DEFAULT;
	m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)pwszText);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmBaseDialog::RedrawLog()
{
	m_si->LastTime = 0;

	if (m_si->arEvents.getCount())
		m_pLog->LogChatEvents(nullptr);
	else
		ClearLog();
}

void CSrmmBaseDialog::OnRedrawTimer(CTimer *pTimer)
{
	pTimer->Stop();

	if (isChat())
		RedrawLog();
	else 
		RemakeLog();
}

void CSrmmBaseDialog::RemakeLog()
{
	m_pLog->LogEvents(m_hDbEventFirst, -1, false);
}

void CSrmmBaseDialog::ScheduleRedrawLog()
{
	timerRedraw.Start(20);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSrmmBaseDialog::UpdateChatLog()
{
	if (!m_si->pMI->bDatabase)
		return;

	m_pLog->Clear();
	GetFirstEvent();

	for (auto &it : m_si->arEvents.rev_iter())
		if (it->hEvent)
			m_si->arEvents.removeItem(&it);

	int iHistoryMode = Srmm::iHistoryMode;

	auto *szProto = Proto_GetBaseAccountName(m_hContact);
	for (MEVENT hDbEvent = m_hDbEventFirst; hDbEvent; hDbEvent = db_event_next(m_hContact, hDbEvent)) {
		DB::EventInfo dbei(hDbEvent);
		if (dbei && !mir_strcmp(szProto, dbei.szModule) && g_chatApi.DbEventIsShown(dbei)) {
			if (iHistoryMode == LOADHISTORY_UNREAD && dbei.bRead)
				continue;

			Utf2T wszUserId(dbei.szUserId);
			ptrW wszText(dbei.getText());

			GCEVENT gce = { m_si, GC_EVENT_MESSAGE };
			gce.dwFlags = GCEF_ADDTOLOG;
			gce.pszUserInfo.w = wszUserId;
			gce.pszText.w = wszText;
			gce.time = dbei.getUnixtime();
			gce.hEvent = hDbEvent;

			if (dbei.szUserId == nullptr)
				gce.bIsMe = true;
			else if (USERINFO *ui = g_chatApi.UM_FindUser(m_si, wszUserId))
				gce.pszNick.w = ui->pszNick;
			else 
				gce.pszNick.w = wszUserId;
			SM_AddEvent(m_si, &gce, false);
		}
	}

	m_pLog->LogChatEvents(nullptr);
}

void CSrmmBaseDialog::OnOptionsApplied()
{
	m_pLog->UpdateOptions();
}

void CSrmmBaseDialog::UpdateFilterButton()
{
	db_set_b(m_hContact, CHAT_MODULE, "FilterEnabled", m_bFilterEnabled);
	
	if (m_si)
		Chat_SetFilters(m_si);

	m_btnFilter.SendMsg(BUTTONADDTOOLTIP, (WPARAM)(m_bFilterEnabled 
		? TranslateT("Disable the event filter")
		: TranslateT("Enable the event filter")), BATF_UNICODE);
}

/////////////////////////////////////////////////////////////////////////////////////////

static int compareStub(const void *p1, const void *p2)
{
	return g_chatApi.UM_CompareItem(*(USERINFO **)p1, *(USERINFO **)p2);
}

void CSrmmBaseDialog::UpdateNickList()
{
	int idx = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);

	m_nickList.SetDraw(false);
	m_nickList.ResetContent();

	auto &fromList = m_si->getUserList();
	LIST<USERINFO> tmpList(fromList.getCount());
	List_Copy((SortedList *)&fromList, (SortedList *)&tmpList, sizeof(void *));
	qsort(tmpList.getArray(), tmpList.getCount(), sizeof(void *), compareStub);

	for (auto &ui : tmpList)
		m_nickList.AddString(ui->pszNick, LPARAM(ui));

	m_nickList.SendMsg(LB_SETTOPINDEX, idx, 0);
	m_nickList.SetDraw(true);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, FALSE);
	UpdateWindow(m_nickList.GetHwnd());

	UpdateTitle();
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
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT;
	cf.dwEffects = 0;

	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD))
		cf.dwEffects |= CFE_BOLD;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS))
		cf.dwEffects |= CFE_ITALIC;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE))
		cf.dwEffects |= CFE_UNDERLINE;
	if (IsDlgButtonChecked(m_hwnd, IDC_SRMM_STRIKEOUT))
		cf.dwEffects |= CFM_STRIKEOUT;
	m_message.SendMsg(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
}

void CSrmmBaseDialog::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	UpdateFilterButton();

	if (m_bFilterEnabled && !g_chatApi.bRightClickFilter)
		ShowFilterMenu();
	else {
		if (m_hwndFilter)
			SendMessage(m_hwndFilter, WM_CLOSE, 0, 0);

		RedrawLog();
	}
}

void CSrmmBaseDialog::onClick_History(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	if (m_si != nullptr && !m_si->pMI->bDatabase)
		ShellExecute(m_hwnd, nullptr, g_chatApi.GetChatLogsFilename(m_si, 0), nullptr, nullptr, SW_SHOW);
	else
		CallService(MS_HISTORY_SHOWCONTACTHISTORY, m_hContact, 0);
}

void CSrmmBaseDialog::onClick_ChanMgr(CCtrlButton *pButton)
{
	if (pButton->Enabled())
		Chat_DoEventHook(m_si, GC_USER_CHANMGR, nullptr, nullptr, 0);
}

void CSrmmBaseDialog::onClick_CloseQuote(CCtrlButton*)
{
	m_Quote.Hide();
	m_btnCloseQuote.Hide();
	m_hQuoteEvent = 0;
	Resize();
}

void CSrmmBaseDialog::onContextMenu(CContextMenuPos *pos)
{
	POINT pt = pos->pt;
	ScreenToClient(m_nickList.GetHwnd(), &pt);
	
	int item = m_nickList.SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
	if (HIWORD(item) == 0) // clicked inside the client area
		if (auto *ui = (USERINFO *)m_nickList.GetItemData(item))
			RunUserMenu(m_nickList.GetHwnd(), ui, pos->pt);
}

void CSrmmBaseDialog::onDblClick_List(CCtrlListBox *pList)
{
	TVHITTESTINFO hti;
	hti.pt.x = (short)LOWORD(GetMessagePos());
	hti.pt.y = (short)HIWORD(GetMessagePos());
	ScreenToClient(pList->GetHwnd(), &hti.pt);

	int item = LOWORD(pList->SendMsg(LB_ITEMFROMPOINT, 0, MAKELPARAM(hti.pt.x, hti.pt.y)));
	auto *ui = (USERINFO *)m_nickList.GetItemData(item);
	if (ui == nullptr)
		return;

	bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	if (Chat::bDoubleClick4Privat ? bShift : !bShift) {
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

	return ::NotifyEventHooks(hHookSrmmEvent, code, (LPARAM)this);
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
	bool isShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0, isCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	if (db_get_b(0, CHAT_MODULE, "ShiftDropFilePasteURL", 1) == 0 || !(isShift && !isCtrl)) // hidden setting: Chat/ShiftDropFilePasteURL
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

	if (isCtrl && !isAlt && key == VK_F4) { // ctrl-F4 
		CloseTab();
		return true;
	}

	return false;
}

void CSrmmBaseDialog::RefreshButtonStatus()
{
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_BACKCOLOR | CFM_COLOR;
	m_message.SendMsg(EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	bool bState = m_btnColor.IsPushed();
	if (!bState && cf.crTextColor != m_clrInputFG)
		m_btnColor.Push(true);
	else if (bState && cf.crTextColor == m_clrInputFG)
		m_btnColor.Push(false);

	bState = m_btnBkColor.IsPushed();
	if (!bState && cf.crBackColor != m_clrInputBG)
		m_btnBkColor.Push(true);
	else if (bState && cf.crBackColor == m_clrInputBG)
		m_btnBkColor.Push(false);

	bState = m_btnBold.IsPushed();
	UINT u2 = cf.dwEffects & CFE_BOLD;
	if (!bState && u2 != 0)
		m_btnBold.Push(true);
	else if (bState && u2 == 0)
		m_btnBold.Push(false);

	bState = m_btnItalic.IsPushed();
	u2 = cf.dwEffects & CFE_ITALIC;
	if (!bState && u2 != 0)
		m_btnItalic.Push(true);
	else if (bState && u2 == 0)
		m_btnItalic.Push(false);

	bState = m_btnUnderline.IsPushed();
	u2 = cf.dwEffects & CFE_UNDERLINE;
	if (!bState && u2 != 0)
		m_btnUnderline.Push(true);
	else if (bState && u2 == 0)
		m_btnUnderline.Push(false);
}

void CSrmmBaseDialog::SetQuoteEvent(MEVENT hEvent)
{
	DB::EventInfo dbei(hEvent);
	if (dbei) {
		CMStringW wszText(TranslateT("In reply to"));
		wszText += L": ";
		wszText += ptrW(dbei.getText()).get();
		m_Quote.SetText(wszText);

		m_hQuoteEvent = hEvent;
		m_Quote.Show();
		m_btnCloseQuote.Show();
		Resize();
	}
}
