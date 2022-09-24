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
// implements the message dialog window.

#include "stdafx.h"

bool IsStringValidLink(wchar_t *pszText);

LIST<void> g_arUnreadWindows(1, PtrKeySortT);

static int g_cLinesPerPage = 0;
static int g_iWheelCarryover = 0;

static const UINT formatControls[] = { IDC_SRMM_BOLD, IDC_SRMM_ITALICS, IDC_SRMM_UNDERLINE, IDC_FONTSTRIKEOUT };
static const UINT addControls[] = { IDC_ADD, IDC_CANCELADD };
static const UINT btnControls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER, IDC_ADD, IDC_CANCELADD };
static const UINT errorControls[] = { IDC_STATICERRORICON, IDC_STATICTEXT, IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };

struct
{
	int id;
	const wchar_t *text;
}
static tooltips[] =
{
	{ IDC_ADD, LPGENW("Add this contact permanently to your contact list") },
	{ IDC_CANCELADD, LPGENW("Do not add this contact permanently") },
	{ IDC_TOGGLESIDEBAR, LPGENW("Expand or collapse the side bar") }
};

struct
{
	int id;
	HICON *pIcon;
}
static buttonicons[] =
{
	{ IDC_ADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_ADD] },
	{ IDC_CANCELADD, &PluginConfig.g_buttonBarIcons[ICON_BUTTON_CANCEL] }
};

static void _clrMsgFilter(MSGFILTER *m)
{
	m->msg = 0;
	m->lParam = 0;
	m->wParam = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// sublassing procedure for the h++ based message log viewer

LRESULT CALLBACK HPPKFSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMsgDialog *mwdat = (CMsgDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	if (mwdat) {
		bool isCtrl, isShift, isAlt;
		mwdat->KbdState(isShift, isCtrl, isAlt);

		switch (msg) {
		case WM_NCCALCSIZE:
			return CSkin::NcCalcRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, HPPKFSubclassProc);
		case WM_NCPAINT:
			return CSkin::DrawRichEditFrame(hwnd, mwdat, ID_EXTBKHISTORY, msg, wParam, lParam, HPPKFSubclassProc);

		case WM_KEYDOWN:
			if (!isCtrl && !isAlt && !isShift) {
				if (wParam != VK_PRIOR && wParam != VK_NEXT && wParam != VK_DELETE &&
					wParam != VK_MENU  && wParam != VK_END  && wParam != VK_HOME &&
					wParam != VK_UP    && wParam != VK_DOWN && wParam != VK_LEFT &&
					wParam != VK_RIGHT && wParam != VK_TAB  && wParam != VK_SPACE) {
					SetFocus(GetDlgItem(mwdat->GetHwnd(), IDC_SRMM_MESSAGE));
					keybd_event((uint8_t)wParam, (uint8_t)MapVirtualKey(wParam, 0), KEYEVENTF_EXTENDEDKEY | 0, 0);
					return 0;
				}
				break;
			}
		}
	}
	return mir_callNextSubclass(hwnd, HPPKFSubclassProc, msg, wParam, lParam);
}

static void ShowMultipleControls(HWND hwndDlg, const UINT *controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		Utils::showDlgControl(hwndDlg, controls[i], state);
}

void CMsgDialog::SetDialogToType()
{
	if (!Contact::OnList(m_hContact)) {
		m_bNotOnList = true;
		ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_SHOW);
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_SHOW);
		SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
	}
	else {
		ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
		m_bNotOnList = false;
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
	}

	Utils::enableDlgControl(m_hwnd, IDC_TIME, true);

	m_message.Show();

	ShowMultipleControls(m_hwnd, errorControls, _countof(errorControls), m_bErrorState ? SW_SHOW : SW_HIDE);

	if (!m_SendFormat)
		ShowMultipleControls(m_hwnd, formatControls, _countof(formatControls), SW_HIDE);

	if (m_pContainer->m_hwndActive == m_hwnd)
		UpdateReadChars();

	SetDlgItemText(m_hwnd, IDC_STATICTEXT, TranslateT("A message failed to send successfully."));

	DM_RecalcPictureSize();
	GetAvatarVisibility();

	Utils::showDlgControl(m_hwnd, IDC_CONTACTPIC, m_bShowAvatar ? SW_SHOW : SW_HIDE);
	Utils::showDlgControl(m_hwnd, IDC_SPLITTERY, m_bIsAutosizingInput ? SW_HIDE : SW_SHOW);
	Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);

	EnableSendButton(GetWindowTextLength(m_message.GetHwnd()) != 0);
	UpdateTitle();
	Resize();

	Utils::enableDlgControl(m_hwnd, IDC_CONTACTPIC, false);

	m_pPanel.Configure();
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclasses the avatar display controls, needed for skinning and to prevent
// it from flickering during resize/move operations.

static LRESULT CALLBACK AvatarSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ERASEBKGND:
		return TRUE;

	case WM_UPDATEUISTATE:
		return TRUE;

	case WM_NOTIFY:
		ProcessAvatarChange(hwnd, lParam);
		break;
	}
	return mir_callNextSubclass(hwnd, AvatarSubclassProc, msg, wParam, lParam);
}

LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	HWND hwndParent = GetParent(hwnd);
	CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? PluginConfig.hCurSplitNS : PluginConfig.hCurSplitWE);
		return TRUE;

	case WM_LBUTTONDOWN:
		if (hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY)) {
			GetClientRect(hwnd, &rc);
			dat->m_savedSplitterY = dat->m_iSplitterY;
			dat->m_savedDynaSplit = dat->m_dynaSplitter;
		}
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			GetClientRect(hwnd, &rc);
			dat->SplitterMoved(rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, hwnd);
		}
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		GetClientRect(hwnd, &rc);
		{
			PAINTSTRUCT ps;
			HDC dc = BeginPaint(hwnd, &ps);

			if (dat && CSkin::m_skinEnabled)
				CSkin::SkinDrawBG(hwnd, dat->m_pContainer->m_hwnd, dat->m_pContainer, &rc, dc);
			else if (M.isAero() || M.isVSThemed()) {
				if (GetDlgCtrlID(hwnd) == IDC_PANELSPLITTER) {
					EndPaint(hwnd, &ps);
					return 0;
				}
				CSkin::FillBack(dc, &rc);
			}
			else CSkin::FillBack(dc, &rc);

			EndPaint(hwnd, &ps);
		}
		return 0;

	case WM_LBUTTONUP:
		HWND hwndCapture = GetCapture();

		ReleaseCapture();
		dat->DM_ScrollToBottom(0, 1);
		if (!dat->isChat() && hwnd == GetDlgItem(hwndParent, IDC_PANELSPLITTER)) {
			SendMessage(hwndParent, WM_SIZE, 0, 0);
			RedrawWindow(hwndParent, nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (hwnd == GetDlgItem(hwndParent, IDC_SPLITTERY)) {
			if (hwndCapture != hwnd)
				break;

			POINT pt;
			GetCursorPos(&pt);

			LONG messagePos = GetMessagePos();
			GetClientRect(hwnd, &rc);

			int selection;
			if (g_plugin.bUseSameSplitSize)
				selection = ID_SPLITTERCONTEXT_SAVEGLOBALFORALLSESSIONS;
			else if (dat->m_bIsAutosizingInput)
				selection = ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION;
			else
				selection = TrackPopupMenu(GetSubMenu(PluginConfig.g_hMenuContext, 6), TPM_RETURNCMD, pt.x, pt.y, 0, hwndParent, nullptr);

			switch (selection) {
			case ID_SPLITTERCONTEXT_SAVEFORTHISCONTACTONLY:
				dat->m_bSplitterOverride = true;
				db_set_b(dat->m_hContact, SRMSGMOD_T, "splitoverride", 1);
				if (!dat->isChat())
					dat->SaveSplitter();
				break;

			case ID_SPLITTERCONTEXT_SETPOSITIONFORTHISSESSION:
				if (dat->m_bIsAutosizingInput) {
					GetWindowRect(GetDlgItem(dat->GetHwnd(), IDC_SRMM_MESSAGE), &rc);
					dat->m_iInputAreaHeight = 0;
				}
				break;

			case ID_SPLITTERCONTEXT_SAVEGLOBALFORALLSESSIONS:
				{
					uint32_t dwOff_IM = 0, dwOff_CHAT = 0;

					dwOff_CHAT = -(2 + (PluginConfig.m_DPIscaleY > 1.0 ? 1 : 0));
					dwOff_IM = 2 + (PluginConfig.m_DPIscaleY > 1.0 ? 1 : 0);

					RECT rcWin;
					GetWindowRect(hwndParent, &rcWin);

					PluginConfig.lastSPlitterPos.pSrcDat = dat;
					PluginConfig.lastSPlitterPos.pSrcContainer = dat->m_pContainer;
					PluginConfig.lastSPlitterPos.lParam = rc.bottom;
					PluginConfig.lastSPlitterPos.pos = rcWin.bottom - HIWORD(messagePos);
					PluginConfig.lastSPlitterPos.pos_chat = rcWin.bottom - (short)HIWORD(messagePos) + rc.bottom / 2;
					PluginConfig.lastSPlitterPos.off_chat = dwOff_CHAT;
					PluginConfig.lastSPlitterPos.off_im = dwOff_IM;
					SendMessage(dat->GetHwnd(), DM_SPLITTERGLOBALEVENT, 0, 0);
					Srmm_Broadcast(DM_SPLITTERGLOBALEVENT, 0, 0);
				}
				break;

			default:
				dat->m_iSplitterY = dat->m_savedSplitterY;
				dat->m_dynaSplitter = dat->m_savedDynaSplit;
				dat->DM_RecalcPictureSize();
				dat->UpdateToolbarBG();
				SendMessage(hwndParent, WM_SIZE, 0, 0);
				dat->DM_ScrollToBottom(0, 1);
				break;
			}
		}
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

CMsgDialog::CMsgDialog(int iDlgId, MCONTACT hContact) :
	CSuper(g_plugin, iDlgId),
	m_pPanel(this),
	timerAwayMsg(this, 4),
	m_btnAdd(this, IDC_ADD),
	m_btnQuote(this, IDC_QUOTE),
	m_btnCancelAdd(this, IDC_CANCELADD)
{
	m_hContact = hContact;

	m_btnAdd.OnClick = Callback(this, &CMsgDialog::onClick_Add);
	m_btnQuote.OnClick = Callback(this, &CMsgDialog::onClick_Quote);
	m_btnCancelAdd.OnClick = Callback(this, &CMsgDialog::onClick_CancelAdd);

	Init();
}

CMsgDialog::CMsgDialog(SESSION_INFO *si) :
	CSuper(g_plugin, IDD_CHANNEL, si),
	m_pPanel(this),
	timerAwayMsg(this, 4),
	m_btnAdd(this, IDC_ADD),
	m_btnQuote(this, IDC_QUOTE),
	m_btnCancelAdd(this, IDC_CANCELADD)
{
	m_hContact = si->hContact;

	m_btnQuote.OnClick = Callback(this, &CMsgDialog::onClick_Quote);
	m_btnFilter.OnClick = Callback(this, &CMsgDialog::onClick_Filter);
	m_btnNickList.OnClick = Callback(this, &CMsgDialog::onClick_ShowNickList);

	m_nickList.OnDblClick = Callback(this, &CMsgDialog::onDblClick_List);

	Init();
}

void CMsgDialog::Init()
{
	m_szProto = Proto_GetBaseAccountName(m_hContact);
	m_autoClose = CLOSE_ON_CANCEL;
	m_forceResizable = true;
	m_bFilterEnabled = db_get_b(m_hContact, CHAT_MODULE, "FilterEnabled", m_bFilterEnabled) != 0;

	m_btnOk.OnClick = Callback(this, &CMsgDialog::onClick_Ok);

	m_message.OnChange = Callback(this, &CMsgDialog::onChange_Message);

	timerAwayMsg.OnEvent = Callback(this, &CMsgDialog::onAwayMsg);
	timerFlash.OnEvent = Callback(this, &CMsgDialog::onFlash);
	timerType.OnEvent = Callback(this, &CMsgDialog::onType);
}

CMsgDialog::~CMsgDialog()
{
	mir_free(m_sendBuffer);
	mir_free(m_hHistoryEvents);

	if (m_hClientIcon) DestroyIcon(m_hClientIcon);
	if (m_hSmileyIcon) DestroyIcon(m_hSmileyIcon);
	if (m_hXStatusIcon) DestroyIcon(m_hXStatusIcon);
	if (m_hTaskbarIcon) DestroyIcon(m_hTaskbarIcon);
}

bool CMsgDialog::OnInitDialog()
{
	CSuper::OnInitDialog();

	// m_hwnd is valid, pass it to the tab control
	TCITEM tci;
	tci.mask = TCIF_PARAM;
	tci.lParam = (LPARAM)m_hwnd;
	TabCtrl_SetItem(m_hwndParent, m_iTabID, &tci);

	// update another tab ids
	m_pContainer->UpdateTabs();

	// add this window to window list & proxy
	if (IsWinVer7Plus() && PluginConfig.m_useAeroPeek)
		m_pWnd = new CProxyWindow(this);
	else
		m_pWnd = nullptr;

	m_iLogMode = m_pLog->GetType();

	// set up Windows themes
	DM_ThemeChanged();

	// refresh cache data for this contact
	m_cache = CContactCache::getContactCache(m_hContact);
	m_cache->updateNick();
	m_cache->updateUIN();
	m_cache->setWindowData(this);

	m_bIsAutosizingInput = m_pContainer->cfg.flags.m_bAutoSplitter && !m_bSplitterOverride;
	m_szProto = const_cast<char *>(m_cache->getProto());
	m_bIsMeta = m_cache->isMeta();
	if (m_bIsMeta)
		m_cache->updateMeta();

	if (m_si) {
		m_si->pDlg = this;
		Chat_SetFilters(m_si);

		m_pPanel.getVisibility();
		m_pPanel.Configure();

		for (auto &it : btnControls)
			ShowWindow(GetDlgItem(m_hwnd, it), SW_HIDE);
	}
	else {
		ShowWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), SW_HIDE);
		m_nickList.Hide();

		for (auto &it : btnControls)
			CustomizeButton(GetDlgItem(m_hwnd, it));
	}

	m_hDbEventFirst = 0;

	if (m_szProto != nullptr) {
		m_wStatus = db_get_w(m_hContact, m_szProto, "Status", ID_STATUS_OFFLINE);
		wcsncpy_s(m_wszStatus, Clist_GetStatusModeDescription(m_wStatus, 0), _TRUNCATE);
	}
	else m_wStatus = ID_STATUS_OFFLINE;

	GetMYUIN();
	GetClientIcon();

	HWND hwndBtn = CreateWindowEx(0, L"MButtonClass", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 6, DPISCALEY_S(20), m_hwnd, (HMENU)IDC_TOGGLESIDEBAR, g_plugin.getInst(), nullptr);
	CustomizeButton(hwndBtn);
	SendMessage(hwndBtn, BUTTONSETASTHEMEDBTN, 1, 0);
	SendMessage(hwndBtn, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
	SendMessage(hwndBtn, BUTTONSETASFLATBTN, FALSE, 0);
	SendMessage(hwndBtn, BUTTONSETASTOOLBARBUTTON, TRUE, 0);
	SendMessage(hwndBtn, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Expand or collapse the side bar"), BATF_UNICODE);

	m_hwndPanelPicParent = CreateWindowEx(WS_EX_TOPMOST, L"Static", L"", SS_OWNERDRAW | WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, m_hwnd, (HMENU)6000, nullptr, nullptr);
	mir_subclassWindow(m_hwndPanelPicParent, CInfoPanel::avatarParentSubclass);

	m_bShowUIElements = (m_pContainer->cfg.flags.m_bHideToolbar) == 0;
	if (M.GetByte(m_hContact, "no_ack", 0))
		m_sendMode |= SMODE_NOACK;

	m_iCurrentQueueError = -1;

	// message history limit
	// hHistoryEvents holds up to n event handles
	m_maxHistory = M.GetDword(m_hContact, "maxhist", M.GetDword("maxhist", 0));
	m_curHistory = 0;
	if (m_maxHistory)
		m_hHistoryEvents = (MEVENT*)mir_alloc(m_maxHistory * sizeof(MEVENT));
	else
		m_hHistoryEvents = nullptr;

	if (!m_bIsMeta)
		UpdateWindowIcon();

	GetMyNick();

	m_iMultiSplit = g_plugin.getDword("multisplit", 150);
	if (AllowTyping()) {
		m_nTypeMode = PROTOTYPE_SELFTYPING_OFF;
		timerType.Start(1000);
	}
	m_iLastEventType = 0xffffffff;

	// load log option flags...
	m_dwFlags = m_pContainer->m_theme.dwFlags;

	// consider per-contact message setting overrides
	if (M.GetDword(m_hContact, "mwmask", 0))
		LoadLocalFlags();

	DM_InitTip();

	m_bSplitterOverride = M.GetByte(m_hContact, "splitoverride", 0) != 0;

	m_pPanel.getVisibility();
	m_pPanel.loadHeight();

	m_bShowAvatar = GetAvatarVisibility();

	Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, SW_HIDE);

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_SPLITTERY), &rc);

	POINT pt;
	pt.y = (rc.top + rc.bottom) / 2;
	pt.x = 0;
	ScreenToClient(m_hwnd, &pt);
	m_originalSplitterY = pt.y;
	if (m_iSplitterY == -1)
		m_iSplitterY = m_originalSplitterY + 60;

	GetWindowRect(m_message.GetHwnd(), &rc);
	m_minEditBoxSize.cx = rc.right - rc.left;
	m_minEditBoxSize.cy = rc.bottom - rc.top;

	BB_InitDlgButtons();
	SendMessage(m_hwnd, WM_CBD_LOADICONS, 0, 0);

	SendDlgItemMessage(m_hwnd, IDC_ADD, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(m_hwnd, IDC_CANCELADD, BUTTONSETASFLATBTN, TRUE, 0);

	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASFLATBTN, TRUE, 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASTHEMEDBTN, CSkin::IsThemed(), 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
	SendDlgItemMessage(m_hwnd, IDC_TOGGLESIDEBAR, BUTTONSETASTOOLBARBUTTON, TRUE, 0);

	NotifyEvent(MSG_WINDOW_EVT_OPENING);

	for (auto &it : tooltips)
		SendDlgItemMessage(m_hwnd, it.id, BUTTONADDTOOLTIP, (WPARAM)TranslateW(it.text), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, m_bNotOnList ? TranslateT("Contact not on list. You may add it...") :
		TranslateT("Auto scrolling is disabled (press F12 to enable it)"));

	SendDlgItemMessage(m_hwnd, IDC_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Close session"), BATF_UNICODE);
	SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Click for contact menu\nClick dropdown for window settings"), BATF_UNICODE);

	SetDlgItemText(m_hwnd, IDC_RETRY, TranslateT("Retry"));
	{
		UINT _ctrls[] = { IDC_RETRY, IDC_CANCELSEND, IDC_MSGSENDLATER };
		for (auto &it : _ctrls) {
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASPUSHBTN, TRUE, 0);
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASFLATBTN, FALSE, 0);
			SendDlgItemMessage(m_hwnd, it, BUTTONSETASTHEMEDBTN, TRUE, 0);
		}
	}

	SetDlgItemText(m_hwnd, IDC_CANCELSEND, TranslateT("Cancel"));
	SetDlgItemText(m_hwnd, IDC_MSGSENDLATER, TranslateT("Send later"));

	m_message.SendMsg(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_KEYEVENTS | ENM_CHANGE);
	m_message.SendMsg(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));

	m_bActualHistory = M.GetByte(m_hContact, "ActualHistory", 0) != 0;

	// subclassing stuff
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_CONTACTPIC), AvatarSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERX), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_SPLITTERY), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_MULTISPLITTER), SplitterSubclassProc);
	mir_subclassWindow(GetDlgItem(m_hwnd, IDC_PANELSPLITTER), SplitterSubclassProc);

	// load old messages from history (if wanted...)

	m_cache->getMaxMessageLength();
	m_cache->updateStats(TSessionStats::INIT_TIMER);

	LoadContactAvatar();
	LoadOwnAvatar();

	if (isChat()) {
		m_pLog->Clear();
		UpdateOptions();
		UpdateStatusBar();
		UpdateTitle();
		m_hTabIcon = m_hTabStatusIcon;

		UpdateNickList();
	}
	else {
		FindFirstEvent();

		DM_OptionsApplied();

		DB::ECPTR pCursor(DB::EventsRev(m_hContact));
		while (MEVENT hdbEvent = pCursor.FetchNext()) {
			DBEVENTINFO dbei = {};
			db_event_get(hdbEvent, &dbei);
			if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
				m_lastMessage = dbei.timestamp;
				DM_UpdateLastMessage();
				break;
			}
		}
	}

	// restore saved msg if any...
	ptrW wszSavedMsg(g_plugin.getWStringA(m_hContact, "SavedMsg"));
	if (wszSavedMsg != 0) {
		SETTEXTEX stx = { ST_DEFAULT, 1200 };
		m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, wszSavedMsg);
		UpdateSaveAndSendButton();
		if (m_pContainer->m_hwndActive == m_hwnd)
			UpdateReadChars();
	}

	if (wszInitialText) {
		m_message.SetText(wszInitialText);
		int len = GetWindowTextLength(m_message.GetHwnd());
		PostMessage(m_message.GetHwnd(), EM_SETSEL, len, len);
		if (len)
			EnableSendButton(true);
		mir_free(wszInitialText);
	}

	m_pContainer->QueryClientArea(rc);

	SetWindowPos(m_hwnd, nullptr, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), m_bActivate ? 0 : SWP_NOZORDER | SWP_NOACTIVATE);
	LoadSplitter();
	ShowPicture(true);

	if (m_pContainer->cfg.flags.m_bCreateMinimized || !m_bActivate || m_pContainer->cfg.flags.m_bDeferredTabSelect) {
		m_iFlashIcon = PluginConfig.g_IconMsgEvent;
		timerFlash.Start(TIMEOUT_FLASHWND);
		m_bCanFlashTab = true;

		DBEVENTINFO dbei = {};
		dbei.eventType = EVENTTYPE_MESSAGE;
		FlashOnClist(m_hDbEventFirst, &dbei);

		if (!isChat())
			m_pContainer->SetIcon(this, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
		m_pContainer->cfg.flags.m_bNeedsUpdateTitle = true;
		m_bWasBackgroundCreate = m_bNeedCheckSize = m_bDeferredScroll = true;
	}

	if (isChat()) {
		m_pContainer->m_hwndActive = m_hwnd;
		ShowWindow(m_hwnd, SW_SHOW);
	}
	else if (m_bActivate) {
		m_pContainer->m_hwndActive = m_hwnd;
		ShowWindow(m_hwnd, SW_SHOW);
		SetActiveWindow(m_hwnd);
		SetForegroundWindow(m_hwnd);
	}
	else if (m_pContainer->cfg.flags.m_bCreateMinimized) {
		m_bDeferredScroll = true;
		ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
		m_pContainer->m_hwndActive = m_hwnd;
		m_pContainer->cfg.flags.m_bDeferredConfigure = true;
	}
	m_pContainer->UpdateTitle(m_hContact);

	DM_RecalcPictureSize();
	m_dwLastActivity = GetTickCount() - 1000;
	m_pContainer->m_dwLastActivity = m_dwLastActivity;

	if (m_iLogMode == WANT_HPP_LOG)
		mir_subclassWindow(m_pLog->GetHwnd(), HPPKFSubclassProc);

	m_bInitMode = false;
	NotifyEvent(MSG_WINDOW_EVT_OPEN);

	if (m_pContainer->cfg.flags.m_bCreateMinimized) {
		m_pContainer->cfg.flags.m_bCreateMinimized = false;
		m_pContainer->m_hwndActive = m_hwnd;
	}
	return true;
}

void CMsgDialog::OnDestroy()
{
	NotifyEvent(MSG_WINDOW_EVT_CLOSING);

	g_arUnreadWindows.remove((HANDLE)m_hContact);

	m_cache->setWindowData();
	m_pContainer->ClearMargins();
	PostMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 1);
	if (m_pContainer->cfg.flags.m_bSideBar)
		m_pContainer->m_pSideBar->removeSession(this);

	if (g_plugin.bDeleteTemp)
		if (!Contact::OnList(m_hContact))
			db_delete_contact(m_hContact);

	if (m_hwndContactPic)
		DestroyWindow(m_hwndContactPic);

	if (m_hwndPanelPic)
		DestroyWindow(m_hwndPanelPic);

	if (m_hwndPanelPicParent)
		DestroyWindow(m_hwndPanelPicParent);

	if (m_si) {
		if (g_clistApi.pfnGetEvent(m_si->hContact, 0))
			g_clistApi.pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);
		m_si->wState &= ~STATE_TALK;
		m_si->pDlg = nullptr;
		m_si = nullptr;
	}

	if (m_cache->isValid()) { // not valid means the contact was deleted
		if (!m_bEditNotesActive) {
			char *msg = m_message.GetRichTextRtf(true);
			if (msg) {
				g_plugin.setUString(m_hContact, "SavedMsg", msg);
				mir_free(msg);
			}
			else g_plugin.delSetting(m_hContact, "SavedMsg");
		}
		else SendMessage(m_hwnd, WM_COMMAND, IDC_PIC, 0);
	}

	if (AllowTyping())
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

	DM_FreeTheme();

	// search the sendqueue for unfinished send jobs and mir_free them. Leave unsent
	// messages in the queue as they can be acked later
	{
		SendJob *jobs = sendQueue->getJobByIndex(0);

		for (int i = 0; i < SendQueue::NR_SENDJOBS; i++) {
			if (jobs[i].hContact == m_hContact) {
				if (jobs[i].iStatus > (unsigned)SendQueue::SQ_INPROGRESS)
					sendQueue->clearJob(i);

				// unfinished jobs which did not yet return anything are kept in the queue.
				// the hwndOwner is set to 0 because the window handle is now no longer valid.
				// Response for such a job is still silently handled by AckMessage() (sendqueue.c)
				if (jobs[i].iStatus == (unsigned)SendQueue::SQ_INPROGRESS)
					jobs[i].hOwnerWnd = nullptr;
			}
		}
	}

	if (m_pWnd) {
		delete m_pWnd;
		m_pWnd = nullptr;
	}

	if (m_hwndTip)
		DestroyWindow(m_hwndTip);

	if (m_cache->isValid())
		g_plugin.setDword("multisplit", m_iMultiSplit);

	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);
	if (i >= 0) {
		SendMessage(m_hwndParent, WM_USER + 100, 0, 0);                      // remove tooltip
		TabCtrl_DeleteItem(m_hwndParent, i);
		m_pContainer->UpdateTabs();
		m_iTabID = -1;
	}

	NotifyEvent(MSG_WINDOW_EVT_CLOSE);

	CSuper::OnDestroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Send button handler

void CMsgDialog::onClick_Ok(CCtrlButton *)
{
	if (m_bEditNotesActive) {
		ActivateTooltip(IDC_PIC, TranslateT("You are editing the user notes. Click the button again or use the hotkey (default: Alt+N) to save the notes and return to normal messaging mode"));
		return;
	}

	int final_sendformat;

	if (!isChat()) {
		// don't parse text formatting when the message contains curly braces - these are used by the rtf syntax
		// and the parser currently cannot handle them properly in the text - XXX needs to be fixed later.
		FINDTEXTEX fi = { 0 };
		fi.chrg.cpMin = 0;
		fi.chrg.cpMax = -1;
		fi.lpstrText = L"{";
		final_sendformat = m_message.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? m_SendFormat : 0;
		fi.lpstrText = L"}";
		final_sendformat = m_message.SendMsg(EM_FINDTEXTEX, FR_DOWN, (LPARAM)&fi) == -1 ? final_sendformat : 0;
	}
	else final_sendformat = true;

	if (GetSendButtonState() == PBS_DISABLED)
		return;

	ptrA streamOut(m_message.GetRichTextRtf(!final_sendformat));
	if (streamOut == nullptr)
		return;

	CMStringW decoded(ptrW(mir_utf8decodeW(streamOut)));
	if (decoded.IsEmpty())
		return;

	if (isChat()) {
		m_cache->saveHistory();
		DoRtfToTags(decoded);
		decoded.Trim();

		if (m_si->pMI->bAckMsg) {
			m_message.Enable(false);
			m_message.SendMsg(EM_SETREADONLY, TRUE, 0);
		}
		else m_message.SetText(L"");

		Utils::enableDlgControl(m_hwnd, IDOK, false);

		// Typing support for GCW_PRIVMESS sessions
		if (AllowTyping())
			if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
				DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

		bool fSound = true;
		if (decoded[0] == '/' || m_si->iType == GCW_SERVER)
			fSound = false;
		Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, decoded, 0);
		m_si->pMI->idleTimeStamp = time(0);
		UpdateStatusBar();
		if (m_pContainer)
			if (fSound && !NEN::bNoSounds && !m_pContainer->cfg.flags.m_bNoSound)
				Skin_PlaySound("SendMsg");
	}
	else {
		if (final_sendformat)
			DoRtfToTags(decoded);
		decoded.TrimRight();

		T2Utf utfResult(decoded);
		size_t memRequired = mir_strlen(utfResult) + 1;

		// try to detect RTL
		HWND hwndEdit = m_message.GetHwnd();
		SendMessage(hwndEdit, WM_SETREDRAW, FALSE, 0);

		PARAFORMAT2 pf2;
		memset(&pf2, 0, sizeof(PARAFORMAT2));
		pf2.cbSize = sizeof(pf2);
		pf2.dwMask = PFM_RTLPARA;
		SendMessage(hwndEdit, EM_SETSEL, 0, -1);
		SendMessage(hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);

		int flags = 0;
		if (pf2.wEffects & PFE_RTLPARA)
			if (Utils_IsRtl(decoded))
				flags |= PREF_RTL;

		SendMessage(hwndEdit, WM_SETREDRAW, TRUE, 0);
		SendMessage(hwndEdit, EM_SETSEL, -1, -1);
		InvalidateRect(hwndEdit, nullptr, FALSE);

		if (memRequired > m_iSendBufferSize) {
			m_sendBuffer = (char *)mir_realloc(m_sendBuffer, memRequired);
			m_iSendBufferSize = memRequired;
		}

		memcpy(m_sendBuffer, (char *)utfResult, memRequired);

		if (memRequired == 0 || m_sendBuffer[0] == 0)
			return;

		if (m_sendMode & SMODE_CONTAINER && m_pContainer->m_hwndActive == m_hwnd && GetForegroundWindow() == m_pContainer->m_hwnd) {
			int tabCount = TabCtrl_GetItemCount(m_hwndParent);
			ptrA szFromStream(m_message.GetRichTextRtf(!m_SendFormat));

			for (int i = 0; i < tabCount; i++) {
				// get the contact from the tabs lparam which hopefully is the tabs hwnd so we can get its userdata.... hopefully
				HWND contacthwnd = GetTabWindow(m_hwndParent, i);
				if (IsWindow(contacthwnd)) {
					// if the contact hwnd is the current contact then ignore it and let the normal code deal with the msg
					if (contacthwnd != m_hwnd) {
						SETTEXTEX stx = { ST_DEFAULT, CP_UTF8 };
						// send the buffer to the contacts msg typing area
						SendDlgItemMessage(contacthwnd, IDC_SRMM_MESSAGE, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szFromStream);
						SendMessage(contacthwnd, WM_COMMAND, IDOK, 0);
					}
				}
			}
		}
		// END /all /MOD
		if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
			DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);

		sendQueue->addTo(this, memRequired, flags);
	}

	SetFocus(m_message.GetHwnd());
}

void CMsgDialog::onClick_Add(CCtrlButton*)
{
	Contact::Add(m_hContact, m_hwnd);

	if (Contact::OnList(m_hContact)) {
		m_bNotOnList = false;
		ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
		if (!m_bScrollingDisabled)
			Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
		Resize();
	}
}

void CMsgDialog::onClick_Quote(CCtrlButton*)
{
	SETTEXTEX stx = { ST_SELECTION, 1200 };

	wchar_t *selected = m_pLog->GetSelection();
	if (selected != nullptr) {
		ptrW szQuoted(QuoteText(selected));
		m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)szQuoted);
		return;
	}

	MEVENT hDBEvent = db_event_last(m_hContact);
	if (hDBEvent == 0)
		return;

	bool bUseSelection = false;
	if (m_iLogMode == WANT_BUILTIN_LOG) {
		CHARRANGE sel;
		LOG()->WndProc(EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin != sel.cpMax) {
			ptrA szFromStream(LOG()->GetRichTextRtf(true, true));
			ptrW converted(mir_utf8decodeW(szFromStream));
			Utils::FilterEventMarkers(converted);
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, ptrW(QuoteText(converted)));
			bUseSelection = true;
		}
	}

	if (!bUseSelection) {
		DBEVENTINFO dbei = {};
		dbei.cbBlob = db_event_getBlobSize(hDBEvent);
		wchar_t *szText = (wchar_t*)mir_alloc((dbei.cbBlob + 1) * sizeof(wchar_t));   // URLs are made one char bigger for crlf
		dbei.pBlob = (uint8_t*)szText;
		db_event_get(hDBEvent, &dbei);
		int iSize = int(mir_strlen((char*)dbei.pBlob)) + 1;

		bool bNeedsFree = false;
		wchar_t *szConverted;
		if (dbei.flags & DBEF_UTF) {
			szConverted = mir_utf8decodeW((char*)szText);
			bNeedsFree = true;
		}
		else {
			if (iSize != (int)dbei.cbBlob)
				szConverted = (wchar_t*)&dbei.pBlob[iSize];
			else {
				szConverted = (wchar_t*)mir_alloc(sizeof(wchar_t) * iSize);
				bNeedsFree = true;
				MultiByteToWideChar(CP_ACP, 0, (char*)dbei.pBlob, -1, szConverted, iSize);
			}
		}

		if (dbei.eventType == EVENTTYPE_FILE) {
			size_t iDescr = mir_strlen((char *)(szText + sizeof(uint32_t)));
			memmove(szText, szText + sizeof(uint32_t), iDescr);
			memmove(szText + iDescr + 2, szText + sizeof(uint32_t) + iDescr, dbei.cbBlob - iDescr - sizeof(uint32_t) - 1);
			szText[iDescr] = '\r';
			szText[iDescr + 1] = '\n';
			szConverted = (wchar_t*)mir_alloc(sizeof(wchar_t)* (1 + mir_strlen((char *)szText)));
			MultiByteToWideChar(CP_ACP, 0, (char *)szText, -1, szConverted, 1 + (int)mir_strlen((char *)szText));
			bNeedsFree = true;
		}

		if (szConverted != nullptr)
			m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, ptrW(QuoteText(szConverted)));

		mir_free(szText);
		if (bNeedsFree)
			mir_free(szConverted);
	}

	SetFocus(m_message.GetHwnd());
}

void CMsgDialog::onClick_CancelAdd(CCtrlButton*)
{
	m_bNotOnList = false;
	ShowMultipleControls(m_hwnd, addControls, _countof(addControls), SW_HIDE);
	if (!m_bScrollingDisabled)
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, SW_HIDE);
	Resize();
}

void CMsgDialog::onClick_Filter(CCtrlButton *pButton)
{
	if (!pButton->Enabled())
		return;

	m_bFilterEnabled = !m_bFilterEnabled;
	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(m_bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);

	if (m_bFilterEnabled && !g_chatApi.bRightClickFilter) 
		ShowFilterMenu();
	else {
		RedrawLog();
		UpdateTitle();
		db_set_b(m_si->hContact, CHAT_MODULE, "FilterEnabled", m_bFilterEnabled);
	}
}

void CMsgDialog::onClick_ShowNickList(CCtrlButton *pButton)
{
	if (!pButton->Enabled() || m_si->iType == GCW_SERVER)
		return;

	m_bNicklistEnabled = !m_bNicklistEnabled;

	Resize();
	if (CSkin::m_skinEnabled)
		InvalidateRect(m_hwnd, nullptr, TRUE);
	m_pLog->ScrollToBottom();
}

void CMsgDialog::onDblClick_List(CCtrlListBox *pList)
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
	if (Chat::bDoubleClick4Privat ? bShift : !bShift) {
		int selStart = LOWORD(m_message.SendMsg(EM_GETSEL, 0, 0));
		CMStringW tszName(ui->pszNick);
		if (selStart == 0 && mir_wstrlen(g_Settings.pwszAutoText))
			tszName.Append(g_Settings.pwszAutoText);
		tszName.AppendChar(' ');

		m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)tszName.GetString());
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
		SetFocus(m_message.GetHwnd());
	}
	else Chat_DoEventHook(m_si, GC_USER_PRIVMESS, ui, nullptr, 0);
}

void CMsgDialog::onChange_Message(CCtrlEdit*)
{
	if (m_pContainer->m_hwndActive == m_hwnd)
		UpdateReadChars();

	m_dwLastActivity = GetTickCount();
	m_pContainer->m_dwLastActivity = m_dwLastActivity;
	UpdateSaveAndSendButton();

	m_btnOk.SendMsg(BUTTONSETASNORMAL, m_message.GetRichTextLength() != 0, 0);
	m_btnOk.Enable(m_message.GetRichTextLength() != 0);

	// Typing support for GCW_PRIVMESS sessions
	if (AllowTyping()) {
		if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
			m_nLastTyping = GetTickCount();
			if (GetWindowTextLength(m_message.GetHwnd())) {
				if (m_nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
					if (!m_bInitMode)
						DM_NotifyTyping(PROTOTYPE_SELFTYPING_ON);
				}
			}
			else if (m_nTypeMode == PROTOTYPE_SELFTYPING_ON)
				DM_NotifyTyping(PROTOTYPE_SELFTYPING_OFF);
		}
	}
}

void CMsgDialog::onType(CTimer *)
{
	if (AllowTyping())
		DM_Typing(false);
}

void CMsgDialog::onFlash(CTimer *)
{
	if (m_bCanFlashTab)
		FlashTab(true);
}

// timer to control info panel hovering
void CMsgDialog::onAwayMsg(CTimer *pTimer)
{
	pTimer->Stop();

	POINT pt;
	GetCursorPos(&pt);

	if (m_pPanel.hitTest(pt) != CInfoPanel::HTNIRVANA)
		ActivateTooltip(0, 0);
	else
		m_bAwayMsgTimer = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// resizer proc for the "new" layout.

int CMsgDialog::Resizer(UTILRESIZECONTROL *urc)
{
	int panelHeight = m_pPanel.getHeight() + 1;

	bool bNick = false;
	bool bInfoPanel = m_pPanel.isActive();
	bool bShowToolbar = (m_pContainer->cfg.flags.m_bHideToolbar) == 0;
	bool bBottomToolbar = (m_pContainer->cfg.flags.m_bBottomToolbar) != 0;

	int  iSplitterX = m_pContainer->cfg.iSplitterX;

	RECT rc, rcButton;
	GetClientRect(m_pLog->GetHwnd(), &rc);
	GetClientRect(GetDlgItem(m_hwnd, IDC_PROTOCOL), &rcButton);

	if (m_bIsAutosizingInput)
		Utils::showDlgControl(m_hwnd, IDC_SPLITTERY, SW_HIDE);

	if (m_panelStatusCX == 0)
		m_panelStatusCX = 80;

	if (m_si) {
		if (m_si->iType != GCW_SERVER) {
			m_nickList.Show(m_bNicklistEnabled);
			Utils::showDlgControl(m_hwnd, IDC_SPLITTERX, m_bNicklistEnabled ? SW_SHOW : SW_HIDE);

			m_btnNickList.Enable(true);
			m_btnFilter.Enable(true);
			if (m_si->iType == GCW_CHATROOM)
				m_btnChannelMgr.Enable(m_si->pMI->bChanMgr);
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

		bNick = m_si->iType != GCW_SERVER && m_bNicklistEnabled;
	}

	switch (urc->wId) {
	case IDC_PANELSPLITTER:
		urc->rcItem.bottom = panelHeight;
		urc->rcItem.top = panelHeight - 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_TOP;

	case IDC_SRMM_LOG:
		if (isChat()) {
			urc->rcItem.top = 0;
			urc->rcItem.left = 0;
			urc->rcItem.right = bNick ? urc->dlgNewSize.cx - iSplitterX : urc->dlgNewSize.cx;
			urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
			if (!bShowToolbar || bBottomToolbar)
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
		}

		if (m_bErrorState)
			urc->rcItem.bottom -= ERRORPANEL_HEIGHT;
		if (m_bScrollingDisabled || m_bNotOnList)
			urc->rcItem.bottom -= 20;
		if (m_sendMode & SMODE_MULTIPLE)
			urc->rcItem.right -= (m_iMultiSplit + 3);
		urc->rcItem.bottom -= m_iSplitterY - m_originalSplitterY;
		if (!bShowToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		urc->rcItem.bottom += 3;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKHISTORY];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		rcLogBottom = urc->rcItem.bottom;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SRMM_NICKLIST:
		urc->rcItem.top = 0;
		urc->rcItem.right = urc->dlgNewSize.cx;
		urc->rcItem.left = urc->dlgNewSize.cx - iSplitterX + 2;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		if (!bShowToolbar || bBottomToolbar)
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

	case IDC_CONTACTPIC:
		GetClientRect(m_message.GetHwnd(), &rc);
		urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		urc->rcItem.left = urc->rcItem.right - (m_pic.cx + 2);
		if ((urc->rcItem.bottom - urc->rcItem.top) < (m_pic.cy/* + 2*/) && m_bShowAvatar) {
			urc->rcItem.top = urc->rcItem.bottom - m_pic.cy;
			m_bUseOffset = true;
		}
		else m_bUseOffset = false;

		if (bShowToolbar && bBottomToolbar && (PluginConfig.m_bAlwaysFullToolbarWidth || ((m_pic.cy - DPISCALEY_S(6)) < rc.bottom))) {
			urc->rcItem.bottom -= DPISCALEY_S(22);
			if (m_bIsAutosizingInput) {
				urc->rcItem.left--;
				urc->rcItem.top--;
			}
		}

		if (m_hwndContactPic) //if m_pPanel control was created?
			SetWindowPos(m_hwndContactPic, HWND_TOP, 1, ((urc->rcItem.bottom - urc->rcItem.top) - (m_pic.cy)) / 2 + 1,  //resizes it
				m_pic.cx - 2, m_pic.cy - 2, SWP_SHOWWINDOW);

		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SPLITTERX:
		urc->rcItem.right = urc->dlgNewSize.cx - iSplitterX + 2;
		urc->rcItem.left = urc->dlgNewSize.cx - iSplitterX;
		urc->rcItem.bottom = urc->dlgNewSize.cy - m_iSplitterY;
		if (!bShowToolbar || bBottomToolbar)
			urc->rcItem.bottom += DPISCALEY_S(21);
		urc->rcItem.top = 0;
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_SPLITTERY:
		urc->rcItem.right = urc->dlgNewSize.cx;
		if (isChat()) {
			urc->rcItem.top = urc->dlgNewSize.cy - m_iSplitterY + DPISCALEY_S(23);
			urc->rcItem.bottom = urc->rcItem.top + DPISCALEY_S(2);
			urc->rcItem.left = 0;
			urc->rcItem.bottom++;
			urc->rcItem.top++;
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;
		}
		urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		urc->rcItem.bottom = urc->rcItem.top + DPISCALEY_S(2);
		OffsetRect(&urc->rcItem, 0, 1);
		urc->rcItem.left = 0;

		if (m_bUseOffset)
			urc->rcItem.right -= (m_pic.cx); // + DPISCALEX(2));
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_SRMM_MESSAGE:
		urc->rcItem.right = urc->dlgNewSize.cx;
		if (m_bShowAvatar)
			urc->rcItem.right -= m_pic.cx + 2;
		if (isChat()) {
			urc->rcItem.bottom = urc->dlgNewSize.cy;
			urc->rcItem.top = urc->dlgNewSize.cy - m_iSplitterY + 3 + DPISCALEY_S(23);
		}
		else {
			urc->rcItem.top -= m_iSplitterY - m_originalSplitterY;
		}

		if (bBottomToolbar && bShowToolbar)
			urc->rcItem.bottom -= DPISCALEY_S(22);

		if (m_bIsAutosizingInput)
			urc->rcItem.top -= DPISCALEY_S(1);

		msgTop = urc->rcItem.top;
		if (CSkin::m_skinEnabled) {
			CSkinItem *item = &SkinItems[ID_EXTBKINPUTAREA];
			if (!item->IGNORED) {
				urc->rcItem.left += item->MARGIN_LEFT;
				urc->rcItem.right -= item->MARGIN_RIGHT;
				urc->rcItem.top += item->MARGIN_TOP;
				urc->rcItem.bottom -= item->MARGIN_BOTTOM;
			}
		}
		if (isChat())
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_MULTISPLITTER:
		if (bInfoPanel)
			urc->rcItem.top += panelHeight;
		urc->rcItem.left -= m_iMultiSplit;
		urc->rcItem.right -= m_iMultiSplit;
		urc->rcItem.bottom = rcLogBottom;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_HEIGHT;

	case IDC_LOGFROZENTEXT:
		urc->rcItem.right = urc->dlgNewSize.cx - 50;
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 16 - (bBottomToolbar ? 0 : 28);
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_ADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 28;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_CANCELADD:
		urc->rcItem.bottom = msgTop - (bBottomToolbar ? 0 : 28);
		urc->rcItem.top = msgTop - 18 - (bBottomToolbar ? 0 : 28);
		urc->rcItem.right = urc->dlgNewSize.cx - 4;
		urc->rcItem.left = urc->rcItem.right - 20;
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;

	case IDC_TOGGLESIDEBAR:
		return RD_ANCHORX_CUSTOM | RD_ANCHORY_CUSTOM;

	case IDC_RETRY:
	case IDC_CANCELSEND:
	case IDC_MSGSENDLATER:
		if (m_bErrorState) {
			urc->rcItem.bottom = msgTop - 5 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_bScrollingDisabled) ? 20 : 0);
			urc->rcItem.top = msgTop - 25 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_bScrollingDisabled) ? 20 : 0);
		}
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_STATICTEXT:
	case IDC_STATICERRORICON:
		if (m_bErrorState) {
			urc->rcItem.bottom = msgTop - 28 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_bScrollingDisabled) ? 20 : 0);
			urc->rcItem.top = msgTop - 45 - (bBottomToolbar ? 0 : 28) - ((m_bNotOnList || m_bScrollingDisabled) ? 20 : 0);
		}
		if (!bShowToolbar && !bBottomToolbar) {
			urc->rcItem.bottom += 21;
			urc->rcItem.top += 21;
		}
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

/////////////////////////////////////////////////////////////////////////////////////////
// message filter

int CMsgDialog::OnFilter(MSGFILTER *pFilter)
{
	uint32_t msg = pFilter->msg;
	WPARAM wp = pFilter->wParam;
	LPARAM lp = pFilter->lParam;

	bool isCtrl, isShift, isAlt;
	KbdState(isShift, isCtrl, isAlt);

	if (msg == WM_SYSKEYUP) {
		if (wp == VK_MENU)
			if (!m_bkeyProcessed && !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000) && !(lp & (1 << 24)))
				m_pContainer->m_pMenuBar->autoShow();

		return _dlgReturn(m_hwnd, 0);
	}

	if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && !(GetKeyState(VK_RMENU) & 0x8000)) {
		MSG message = { m_hwnd, msg, wp, lp };
		LRESULT mim_hotkey_check = Hotkey_Check(&message, isChat() ? TABSRMM_HK_SECTION_GC : TABSRMM_HK_SECTION_IM);
		if (mim_hotkey_check)
			m_bkeyProcessed = true;

		switch (mim_hotkey_check) {
		case TABSRMM_HK_CHANNELMGR:
			m_btnChannelMgr.Click();
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_FILTERTOGGLE:
			m_btnFilter.Click();
			InvalidateRect(m_btnFilter.GetHwnd(), nullptr, TRUE);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_LISTTOGGLE:
			m_btnNickList.Click();
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_MUC_SHOWSERVER:
			if (m_si->iType != GCW_SERVER)
				Chat_DoEventHook(m_si, GC_USER_MESSAGE, nullptr, L"/servershow", 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_SETUSERPREFS:
			CallService(MS_TABMSG_SETUSERPREFS, m_hContact, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_SENDFILE:
			CallService(MS_FILE_SENDFILE, m_hContact, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_QUOTEMSG:
			SendMessage(m_hwnd, WM_COMMAND, IDC_QUOTE, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_CLEARMSG:
			m_message.SetText(L"");
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_USERMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_PROTOCOL, 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_SENDMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_SENDMENU, IDC_SENDMENU);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_PROTOMENU:
			SendMessage(m_hwnd, WM_COMMAND, IDC_PROTOMENU, IDC_PROTOMENU);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_USERDETAILS:
			SendMessage(m_hwnd, WM_COMMAND, MAKELONG(IDC_NAME, BN_CLICKED), 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_EDITNOTES:
			PostMessage(m_hwnd, WM_COMMAND, MAKELONG(IDC_PIC, BN_CLICKED), 0);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_TOGGLESENDLATER:
			if (SendLater::Avail) {
				m_sendMode ^= SMODE_SENDLATER;
				SetWindowPos(m_message.GetHwnd(), nullptr, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOZORDER |
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS);
				RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
			else CWarning::show(CWarning::WARN_NO_SENDLATER, MB_OK | MB_ICONINFORMATION);
			return _dlgReturn(m_hwnd, 1);
		case TABSRMM_HK_TOGGLERTL:
			m_dwFlags ^= MWF_LOG_RTL;
			{
				uint32_t	dwGlobal = M.GetDword("mwflags", MWF_LOG_DEFAULT);
				uint32_t	dwMask = M.GetDword(m_hContact, "mwmask", 0);
				uint32_t	dwFlags = M.GetDword(m_hContact, "mwflags", 0);

				if ((dwGlobal & MWF_LOG_RTL) != (dwFlags & MWF_LOG_RTL)) {
					dwMask |= MWF_LOG_RTL;
					dwFlags |= (dwFlags & MWF_LOG_RTL);
				}
				else {
					dwMask &= ~MWF_LOG_RTL;
					dwFlags &= ~MWF_LOG_RTL;
				}
				if (dwMask) {
					db_set_dw(m_hContact, SRMSGMOD_T, "mwmask", dwMask);
					db_set_dw(m_hContact, SRMSGMOD_T, "mwflags", dwFlags);
				}
				else {
					db_unset(m_hContact, SRMSGMOD_T, "mwmask");
					db_unset(m_hContact, SRMSGMOD_T, "mwflags");
				}
				DM_OptionsApplied();
				RemakeLog();
			}
			return _dlgReturn(m_hwnd, 1);

		case TABSRMM_HK_TOGGLEMULTISEND:
			m_sendMode ^= SMODE_MULTIPLE;
			if (m_sendMode & SMODE_MULTIPLE)
				DM_CreateClist();
			else if (IsWindow(GetDlgItem(m_hwnd, IDC_CLIST)))
				DestroyWindow(GetDlgItem(m_hwnd, IDC_CLIST));

			HWND hwndEdit = m_message.GetHwnd();
			SetWindowPos(hwndEdit, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);
			Resize();
			RedrawWindow(hwndEdit, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_UPDATENOW | RDW_ERASE);
			DM_ScrollToBottom(0, 0);
			Utils::showDlgControl(m_hwnd, IDC_MULTISPLITTER, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
			Utils::showDlgControl(m_hwnd, IDC_CLIST, (m_sendMode & SMODE_MULTIPLE) ? SW_SHOW : SW_HIDE);
			if (m_sendMode & SMODE_MULTIPLE)
				SetFocus(GetDlgItem(m_hwnd, IDC_CLIST));
			else
				SetFocus(m_message.GetHwnd());
			RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			return _dlgReturn(m_hwnd, 1);
		}
		
		if (DM_GenericHotkeysCheck(&message)) {
			m_bkeyProcessed = true;
			return _dlgReturn(m_hwnd, 1);
		}
	}
	if (wp == VK_BROWSER_BACK || wp == VK_BROWSER_FORWARD)
		return 1;

	if (msg == WM_CHAR) {
		if (isCtrl && !isShift && !isAlt) {
			switch (wp) {
			case 23:                // ctrl - w
				PostMessage(m_hwnd, WM_CLOSE, 1, 0);
				break;
			case 20:                // ctrl - t
				PostMessage(m_hwnd, WM_COMMAND, IDC_TOGGLETOOLBAR, 1);
				break;
			}
			return 1;
		}
	}
	if (msg == WM_KEYDOWN) {
		if ((wp == VK_INSERT && isShift && !isCtrl) || (wp == 'V' && isCtrl && !isShift && !isAlt)) {
			m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
			_clrMsgFilter(pFilter);
			return _dlgReturn(m_hwnd, 1);
		}
		if (isCtrl && isShift) {
			if (wp == 0x9) {            // ctrl-shift tab
				m_pContainer->SelectTab(DM_SELECT_PREV);
				_clrMsgFilter(pFilter);
				return _dlgReturn(m_hwnd, 1);
			}
		}
		if (isCtrl && !isShift && !isAlt) {
			if (wp == VK_TAB) {
				m_pContainer->SelectTab(DM_SELECT_NEXT);
				_clrMsgFilter(pFilter);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_F4) {
				PostMessage(m_hwnd, WM_CLOSE, 1, 0);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_PRIOR) {
				m_pContainer->SelectTab(DM_SELECT_PREV);
				return _dlgReturn(m_hwnd, 1);
			}
			if (wp == VK_NEXT) {
				m_pContainer->SelectTab(DM_SELECT_NEXT);
				return _dlgReturn(m_hwnd, 1);
			}
		}
	}
	if (msg == WM_SYSKEYDOWN && isAlt) {
		if (wp == 0x52) {
			m_pContainer->QueryPending();
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_MULTIPLY) {
			SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_DIVIDE) {
			SetFocus(m_pLog->GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_ADD) {
			m_pContainer->SelectTab(DM_SELECT_NEXT);
			return _dlgReturn(m_hwnd, 1);
		}
		if (wp == VK_SUBTRACT) {
			m_pContainer->SelectTab(DM_SELECT_PREV);
			return _dlgReturn(m_hwnd, 1);
		}
	}

	if (msg == WM_KEYDOWN && wp == VK_F12) {
		if (isShift || isCtrl || isAlt)
			return _dlgReturn(m_hwnd, 1);

		m_bScrollingDisabled = !m_bScrollingDisabled;
		Utils::showDlgControl(m_hwnd, IDC_LOGFROZENTEXT, (m_bNotOnList || m_bScrollingDisabled) ? SW_SHOW : SW_HIDE);
		if (!m_bScrollingDisabled)
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Contact not on list. You may add it..."));
		else
			SetDlgItemText(m_hwnd, IDC_LOGFROZENTEXT, TranslateT("Auto scrolling is disabled (press F12 to enable it)"));
		Resize();
		DM_ScrollToBottom(1, 1);
		return _dlgReturn(m_hwnd, 1);
	}

	// tabulation mod
	if (msg == WM_KEYDOWN && wp == VK_TAB) {
		if (g_plugin.bAllowTab) {
			if (pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE)
				m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)"\t");
			_clrMsgFilter(pFilter);
			if (pFilter->nmhdr.idFrom != IDC_SRMM_MESSAGE)
				SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		if (pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE) {
			if (GetSendButtonState() != PBS_DISABLED && !m_pContainer->cfg.flags.m_bHideToolbar)
				SetFocus(GetDlgItem(m_hwnd, IDOK));
			else
				SetFocus(m_pLog->GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		if (pFilter->nmhdr.idFrom == IDC_SRMM_LOG) {
			SetFocus(m_message.GetHwnd());
			return _dlgReturn(m_hwnd, 1);
		}

		return _dlgReturn(m_hwnd, 0);
	}

	if (msg == WM_MOUSEWHEEL && (pFilter->nmhdr.idFrom == IDC_SRMM_LOG || pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE)) {
		POINT pt;
		GetCursorPos(&pt);
		RECT rc;
		GetWindowRect(m_pLog->GetHwnd(), &rc);
		if (PtInRect(&rc, pt)) {
			short wDirection = (short)HIWORD(wp);
			if (LOWORD(wp) & MK_SHIFT) {
				if (wDirection < 0)
					LOG()->WndProc(WM_VSCROLL, MAKEWPARAM(SB_PAGEDOWN, 0), 0);
				else if (wDirection > 0)
					LOG()->WndProc(WM_VSCROLL, MAKEWPARAM(SB_PAGEUP, 0), 0);
				return 0;
			}
			return 0;
		}
		return 1;
	}

	if (msg == WM_CHAR && wp == 'c') {
		if (isCtrl) {
			SendDlgItemMessage(m_hwnd, pFilter->nmhdr.code, WM_COPY, 0, 0);
			return 0;
		}
	}

	if ((msg == WM_LBUTTONDOWN || msg == WM_KEYUP || msg == WM_LBUTTONUP) && pFilter->nmhdr.idFrom == IDC_SRMM_MESSAGE) {
		int bBold = IsDlgButtonChecked(m_hwnd, IDC_SRMM_BOLD);
		int bItalic = IsDlgButtonChecked(m_hwnd, IDC_SRMM_ITALICS);
		int bUnder = IsDlgButtonChecked(m_hwnd, IDC_SRMM_UNDERLINE);
		int bStrikeout = IsDlgButtonChecked(m_hwnd, IDC_FONTSTRIKEOUT);

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
				CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, BST_CHECKED);
		}
		else if (bStrikeout == BST_CHECKED)
			CheckDlgButton(m_hwnd, IDC_FONTSTRIKEOUT, BST_UNCHECKED);
	}
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			HCURSOR hCur = GetCursor();
			m_pContainer->m_pMenuBar->Cancel();
			if (hCur == PluginConfig.hCurSplitNS || hCur == PluginConfig.hCurSplitWE || hCur == PluginConfig.hCurSplitSW || hCur == PluginConfig.hCurSplitWSE) {
				SetWindowLongPtr(m_hwnd, DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
		}
		break;

	case WM_LBUTTONUP:
		// auto-select-and-copy handling...
		// if enabled, releasing the lmb with an active selection automatically copies the selection
		// to the clipboard.
		// holding ctrl while releasing the button pastes the selection to the input area, using plain text
		// holding ctrl-alt does the same, but pastes formatted text
		if (pFilter->nmhdr.idFrom == IDC_SRMM_LOG && g_plugin.bAutoCopy) {
			CHARRANGE cr;
			LOG()->WndProc(EM_EXGETSEL, 0, (LPARAM)&cr);
			if (cr.cpMax == cr.cpMin)
				break;
			cr.cpMin = cr.cpMax;
			if (isCtrl) {
				SETTEXTEX stx = { ST_KEEPUNDO | ST_SELECTION, CP_UTF8 };
				ptrA streamOut(LOG()->GetRichTextRtf(!isAlt, true));
				if (streamOut) {
					Utils::FilterEventMarkers(streamOut);
					m_message.SendMsg(EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)streamOut);
				}
				SetFocus(m_message.GetHwnd());
			}
			else if (!isShift) {
				LOG()->WndProc(WM_COPY, 0, 0);
				SetFocus(m_message.GetHwnd());
				if (m_pContainer->m_hwndStatus)
					SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)TranslateT("Selection copied to clipboard"));
			}
		}
		break;

	case WM_RBUTTONUP:
		return _dlgReturn(m_hwnd, 1);

	case WM_MOUSEMOVE:
		POINT pt;
		GetCursorPos(&pt);
		DM_DismissTip(pt);
		m_pPanel.trackMouse(pt);

		HCURSOR hCur = GetCursor();
		if (hCur == PluginConfig.hCurSplitNS || hCur == PluginConfig.hCurSplitWE || hCur == PluginConfig.hCurSplitSW || hCur == PluginConfig.hCurSplitWSE)
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMsgDialog::WndProc_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
	bool isCtrl, isShift, isAlt;

	// prevent the rich edit from switching text direction or keyboard layout when
	// using hotkeys with ctrl-shift or alt-shift modifiers
	if (m_bkeyProcessed && (msg == WM_KEYUP)) {
		GetKeyboardState(kstate);
		if (kstate[VK_CONTROL] & 0x80 || kstate[VK_SHIFT] & 0x80)
			return 0;

		m_bkeyProcessed = false;
		return 0;
	}

	switch (msg) {
	case WM_NCCALCSIZE:
		return CSkin::NcCalcRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_NCPAINT:
		return CSkin::DrawRichEditFrame(m_message.GetHwnd(), this, ID_EXTBKINPUTAREA, msg, wParam, lParam, stubMessageProc);

	case WM_DROPFILES:
		SendMessage(m_hwnd, WM_DROPFILES, wParam, lParam);
		return 0;

	case WM_CHAR:
		KbdState(isShift, isCtrl, isAlt);

		if (!isAlt && !isCtrl && !m_pContainer->cfg.flags.m_bNoSound && wParam != VK_ESCAPE && !(wParam == VK_TAB && g_plugin.bAllowTab))
			Skin_PlaySound("SoundOnTyping");
		break;

	case WM_MOUSEWHEEL:
		if (DM_MouseWheelHandler(wParam, lParam) == 0)
			return 0;
		break;

	case EM_PASTESPECIAL:
	case WM_PASTE:
		if (OpenClipboard(m_message.GetHwnd())) {
			HANDLE hClip = GetClipboardData(CF_TEXT);
			if (hClip) {
				size_t iMaxSize = m_cache->getMaxMessageLength();
				if (mir_strlen((char*)hClip) > iMaxSize) {
					wchar_t szBuffer[512];
					if (M.GetByte("autosplit", 0))
						mir_snwprintf(szBuffer, TranslateT("WARNING: The message you are trying to paste exceeds the message size limit for the active protocol. It will be sent in chunks of max %d characters"), iMaxSize - 10);
					else
						mir_snwprintf(szBuffer, TranslateT("The message you are trying to paste exceeds the message size limit for the active protocol. Only the first %d characters will be sent."), iMaxSize);
					ActivateTooltip(IDC_SRMM_MESSAGE, szBuffer);
				}
			}
			else if (hClip = GetClipboardData(CF_BITMAP))
				SendHBitmapAsFile((HBITMAP)hClip);
			else if (hClip = GetClipboardData(CF_HDROP))
				SendMessage(m_hwnd, WM_DROPFILES, WPARAM(hClip), 0);

			CloseClipboard();
		}
		break;

	case WM_KEYDOWN:
		KbdState(isShift, isCtrl, isAlt);

		if (!isAlt && !m_pContainer->cfg.flags.m_bNoSound && wParam == VK_DELETE)
			Skin_PlaySound("SoundOnTyping");

		if (wParam == VK_INSERT && !isShift && !isCtrl && !isAlt) {
			m_bInsertMode = !m_bInsertMode;
			m_message.OnChange(&m_message);
		}
		if (wParam == VK_CAPITAL || wParam == VK_NUMLOCK)
			m_message.OnChange(&m_message);

		if (isChat()) {
			// tab-autocomplete
			if (wParam == VK_TAB && !isCtrl && !isShift) {
				// if tab acts as a key pressing, simply do nothing
				if (g_plugin.bAllowTab)
					break;

				m_message.SetDraw(false);
				bool fCompleted = TabAutoComplete();
				m_message.SetDraw(true);
				RedrawWindow(m_message.GetHwnd(), nullptr, nullptr, RDW_INVALIDATE);
				if (!fCompleted) {
					if ((GetSendButtonState() != PBS_DISABLED))
						SetFocus(m_message.GetHwnd());
					else
						SetFocus(m_pLog->GetHwnd());
				}
				return 0;
			}

			if (wParam != VK_ESCAPE)
				if (ProcessHotkeys(wParam, isShift, isCtrl, isAlt))
					return 0;
		}

		if (wParam != VK_RIGHT && wParam != VK_LEFT) {
			replaceStrW(m_wszSearchQuery, nullptr);
			replaceStrW(m_wszSearchResult, nullptr);
		}

		if (wParam == VK_RETURN && m_bEditNotesActive)
			break;

		if (isCtrl && !isAlt && !isShift) {
			if (wParam == VK_UP || wParam == VK_DOWN) {          // input history scrolling (ctrl-up / down)
				m_cache->inputHistoryEvent(wParam);
				return 0;
			}
		}
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

				LOG()->WndProc(WM_VSCROLL, wp, 0);
				return 0;
			}
		}

	case WM_SYSKEYDOWN:
		m_bkeyProcessed = false;
		if (ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam)) {
			m_bkeyProcessed = true;
			return 0;
		}
		break;

	case WM_SYSKEYUP:
		if (wParam == VK_MENU) {
			ProcessHotkeysByMsgFilter(m_message, msg, wParam, lParam);
			return 0;
		}
		break;

	case WM_SYSCHAR:
		if (m_bkeyProcessed) {
			m_bkeyProcessed = false;
			return 0;
		}

		KbdState(isShift, isCtrl, isAlt);
		if ((wParam >= '0' && wParam <= '9') && isAlt) {      // ALT-1 -> ALT-0 direct tab selection
			uint8_t bChar = (uint8_t)wParam;

			int iIndex;
			if (bChar == '0')
				iIndex = 10;
			else
				iIndex = bChar - (uint8_t)'0';
			m_pContainer->SelectTab(DM_SELECT_BY_INDEX, iIndex);
			return 0;
		}
		break;

	case WM_ERASEBKGND:
		return !CSkin::m_skinEnabled;

		// sent by smileyadd when the smiley selection window dies
		// just grab the focus :)
	case WM_USER + 100:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_CONTEXTMENU:
		POINT pt;
		if (isChat()) {
			GetCursorPos(&pt);

			HMENU hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
			HMENU hSubMenu = GetSubMenu(hMenu, 2);
			RemoveMenu(hSubMenu, 9, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 8, MF_BYPOSITION);
			RemoveMenu(hSubMenu, 4, MF_BYPOSITION);

			EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, m_si->pMI->bBold ? MF_ENABLED : MF_GRAYED);
			TranslateMenu(hSubMenu);

			CHARRANGE sel, all = { 0, -1 };
			m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
			if (sel.cpMin == sel.cpMax) {
				EnableMenuItem(hSubMenu, IDM_COPY, MF_GRAYED);
				EnableMenuItem(hSubMenu, IDM_CUT, MF_GRAYED);
			}

			MessageWindowPopupData mwpd = { sizeof(mwpd) };
			mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
			mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
			mwpd.hContact = m_hContact;
			mwpd.hwnd = m_message.GetHwnd();
			mwpd.hMenu = hSubMenu;
			mwpd.pt = pt;
			NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)&mwpd);

			int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);

			mwpd.selection = iSelection;
			mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
			NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)&mwpd);

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
		else {
			if (lParam == 0xFFFFFFFF) {
				CHARRANGE sel;
				m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&sel);
				m_message.SendMsg(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)sel.cpMax);
				ClientToScreen(m_message.GetHwnd(), &pt);
			}
			else {
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
			}

			ShowPopupMenu(m_message, pt);
		}
		return TRUE;
	}

	return CSuper::WndProc_Message(msg, wParam, lParam);
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
// dialog procedure

INT_PTR CMsgDialog::DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt, tmp, cur;
	RECT rc;

	switch (uMsg) {
	case WM_ERASEBKGND:
		RECT rcClient, rcWindow;
		{
			HDC hdc = (HDC)wParam;
			HDC hdcMem = nullptr;
			HBITMAP hbm, hbmOld;
			HANDLE hpb = nullptr;

			GetClientRect(m_hwnd, &rcClient);
			uint32_t cx = rcClient.right - rcClient.left;
			uint32_t cy = rcClient.bottom - rcClient.top;

			if (CMimAPI::m_haveBufferedPaint) {
				hpb = CMimAPI::m_pfnBeginBufferedPaint(hdc, &rcClient, BPBF_TOPDOWNDIB, nullptr, &hdcMem);
				hbm = hbmOld = nullptr;
			}
			else {
				hdcMem = CreateCompatibleDC(hdc);
				hbm = CSkin::CreateAeroCompatibleBitmap(rcClient, hdc);
				hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);
			}

			bool	bAero = M.isAero();

			if (CSkin::m_skinEnabled) {
				UINT item_ids[] = { ID_EXTBKUSERLIST, ID_EXTBKHISTORY, ID_EXTBKINPUTAREA };
				UINT ctl_ids[] = { IDC_SRMM_NICKLIST, IDC_SRMM_LOG, IDC_SRMM_MESSAGE };
				BOOL isEditNotesReason = m_bEditNotesActive;
				BOOL isSendLaterReason = (m_sendMode & SMODE_SENDLATER);
				BOOL isMultipleReason = (m_sendMode & SMODE_MULTIPLE || m_sendMode & SMODE_CONTAINER);

				CSkin::SkinDrawBG(m_hwnd, m_pContainer->m_hwnd, m_pContainer, &rcClient, hdcMem);

				for (int i = 0; i < _countof(item_ids); i++) {
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
						if (item_ids[i] == ID_EXTBKINPUTAREA && (isMultipleReason || isEditNotesReason || isSendLaterReason)) {
							HBRUSH br = CreateSolidBrush(isMultipleReason ? RGB(255, 130, 130) : (isEditNotesReason ? RGB(80, 255, 80) : RGB(80, 80, 255)));
							FillRect(hdcMem, &rc, br);
							DeleteObject(br);
						}
						else CSkin::DrawItem(hdcMem, &rc, item);
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

			// draw the (new) infopanel background. Use the gradient from the statusitem.
			GetClientRect(m_hwnd, &rc);
			m_pPanel.renderBG(hdcMem, rc, &SkinItems[ID_EXTBKINFOPANELBG], bAero);

			// draw aero related stuff
			if (!CSkin::m_skinEnabled)
				RenderToolbarBG(hdcMem, rcClient);

			// render info panel fields
			m_pPanel.renderContent(hdcMem);

			if (hpb)
				CSkin::FinalizeBufferedPaint(hpb, &rcClient);
			else {
				BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				SelectObject(hdcMem, hbmOld);
				DeleteObject(hbm);
				DeleteDC(hdcMem);
			}
			if (!m_fLimitedUpdate)
				m_pContainer->SetAeroMargins();
		}
		return 1;

	case WM_NCPAINT:
		if (CSkin::m_skinEnabled)
			return 0;
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(m_hwnd, &ps);
		EndPaint(m_hwnd, &ps);
		return 0;

	case WM_SIZE:
		if (wParam == SIZE_MAXIMIZED)
			m_pLog->ScrollToBottom();

		if (!IsIconic(m_hwnd)) {
			if (m_ipFieldHeight == 0)
				m_ipFieldHeight = CInfoPanel::m_ipConfig.height2;

			if (m_pContainer->m_uChildMinHeight > 0 && HIWORD(lParam) >= m_pContainer->m_uChildMinHeight) {
				if (m_iSplitterY > HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT)) {
					m_iSplitterY = HIWORD(lParam) - DPISCALEY_S(MINLOGHEIGHT);
					m_dynaSplitter = m_iSplitterY - DPISCALEY_S(34);
					DM_RecalcPictureSize();
				}
				if (m_iSplitterY < DPISCALEY_S(MINSPLITTERY))
					LoadSplitter();
			}

			if (m_si == nullptr) {
				HBITMAP hbm = ((m_pPanel.isActive()) && m_pContainer->cfg.avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
				if (hbm != nullptr) {
					BITMAP bminfo;
					GetObject(hbm, sizeof(bminfo), &bminfo);
					CalcDynamicAvatarSize(&bminfo);
				}
			}

			GetClientRect(m_hwnd, &rc);

			CSuper::DlgProc(uMsg, 0, 0); // call basic window resizer

			BB_SetButtonsPos();

			// size info panel fields
			if (m_pPanel.isActive()) {
				LONG cx = rc.right;
				LONG panelHeight = m_pPanel.getHeight();

				HBITMAP hbm = (m_pContainer->cfg.avatarMode == 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
				double dHeight = 0, dWidth = 0;
				Utils::scaleAvatarHeightLimited(hbm, dWidth, dHeight, panelHeight - 2);
				m_iPanelAvatarX = (int)dWidth;
				m_iPanelAvatarY = (int)dHeight;

				rc.top = 1;
				rc.left = cx - m_iPanelAvatarX;
				rc.bottom = panelHeight - (CSkin::m_bAvatarBorderType ? 2 : 0);
				rc.right = cx;
				m_rcPic = rc;

				if (m_bShowInfoAvatar) {
					SetWindowPos(m_hwndPanelPicParent, HWND_TOP, rc.left - 2, rc.top, rc.right - rc.left, rc.bottom - rc.top + 1, 0);
					ShowWindow(m_hwndPanelPicParent, (m_iPanelAvatarX == 0) || !m_pPanel.isActive() ? SW_HIDE : SW_SHOW);
				}
				else {
					ShowWindow(m_hwndPanelPicParent, SW_HIDE);
					m_iPanelAvatarX = m_iPanelAvatarY = 0;
				}

				rc.right = cx - m_iPanelAvatarX;
				rc.left = rc.right - m_panelStatusCX;
				rc.bottom = panelHeight - 3;
				rc.top = rc.bottom - m_ipFieldHeight;
				m_rcStatus = rc;

				rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
				rc.right = cx - m_iPanelAvatarX - (panelHeight < CInfoPanel::DEGRADE_THRESHOLD ? (m_rcStatus.right - m_rcStatus.left) + 3 : 0);
				rc.bottom = panelHeight - (panelHeight >= CInfoPanel::DEGRADE_THRESHOLD ? m_ipFieldHeight : 0) - 1;
				rc.top = 1;
				m_rcNick = rc;

				rc.left = CInfoPanel::LEFT_OFFSET_LOGO;
				rc.right = cx - (m_iPanelAvatarX + 2) - m_panelStatusCX;
				rc.bottom = panelHeight - 3;
				rc.top = rc.bottom - m_ipFieldHeight;
				m_rcUIN = rc;

				m_pPanel.Invalidate();
			}

			if (GetDlgItem(m_hwnd, IDC_CLIST) != nullptr) {
				RECT rcLog;
				GetClientRect(m_hwnd, &rcClient);
				GetClientRect(m_pLog->GetHwnd(), &rcLog);
				rc.top = 0;
				rc.right = rcClient.right;
				rc.left = rcClient.right - m_iMultiSplit;
				rc.bottom = rcLog.bottom;
				if (m_pPanel.isActive())
					rc.top += (m_pPanel.getHeight() + 1);
				MoveWindow(GetDlgItem(m_hwnd, IDC_CLIST), rc.left, rc.top, rc.right - rc.left, rcLog.bottom - rcLog.top, FALSE);
			}

			m_pLog->Resize();

			DetermineMinHeight();
		}
		return 0;

	case WM_TIMECHANGE:
		if (isChat())
			RedrawLog();
		else
			DM_OptionsApplied();
		break;

	case WM_NOTIFY:
		if (this != nullptr && ((NMHDR*)lParam)->hwndFrom == m_hwndTip) {
			if (((NMHDR*)lParam)->code == NM_CLICK)
				SendMessage(m_hwndTip, TTM_TRACKACTIVATE, FALSE, 0);
			break;
		}

		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_SRMM_LOG:
		case IDC_SRMM_MESSAGE:
			switch (((NMHDR*)lParam)->code) {
			case EN_MSGFILTER:
				if (OnFilter((MSGFILTER*)lParam))
					return TRUE;
				break;

			case EN_REQUESTRESIZE:
				DM_HandleAutoSizeRequest((REQRESIZE *)lParam);
				break;

			case EN_LINK:
				if (((LPNMHDR)lParam)->idFrom == IDC_SRMM_LOG) {
					switch (((ENLINK *)lParam)->msg) {
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_LBUTTONDBLCLK:
						CHARRANGE sel;
						SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)&sel);
						if (sel.cpMin == sel.cpMax) {
							UINT msg = ((ENLINK *)lParam)->msg;
							m_pContainer->m_pMenuBar->Cancel();

							TEXTRANGE tr;
							tr.lpstrText = nullptr;
							tr.chrg = ((ENLINK *)lParam)->chrg;
							tr.lpstrText = (wchar_t *)mir_alloc(sizeof(wchar_t) * (tr.chrg.cpMax - tr.chrg.cpMin + 2));
							SendMessage(((LPNMHDR)lParam)->hwndFrom, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

							BOOL isLink = IsStringValidLink(tr.lpstrText);
							if (isLink) // handled by core
								break;

							// clicked a nick name
							if (isChat() && g_Settings.bClickableNicks) {
								if (msg == WM_RBUTTONDOWN) {
									for (auto &ui : m_si->getUserList()) {
										if (mir_wstrcmp(ui->pszNick, tr.lpstrText))
											continue;

										pt.x = (short)LOWORD(((ENLINK *)lParam)->lParam);
										pt.y = (short)HIWORD(((ENLINK *)lParam)->lParam);
										ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);
										RunUserMenu(m_hwnd, ui, pt);
										break;
									}
									return TRUE;
								}

								if (msg == WM_LBUTTONUP) {
									CHARRANGE chr;
									m_message.SendMsg(EM_EXGETSEL, 0, (LPARAM)&chr);

									CMStringW buf(tr.lpstrText);

									wchar_t str[2] = { 0, 0 };
									TEXTRANGE tr2;
									tr2.lpstrText = str;

									if (chr.cpMin) {
										// prepend nick with space if needed
										tr2.chrg.cpMin = chr.cpMin - 1;
										tr2.chrg.cpMax = chr.cpMin;
										m_message.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr2);
										if (!iswspace(*tr2.lpstrText))
											buf.Insert(0, ' ');
									}
									else {// in the beginning of the message window
										if (mir_wstrlen(g_Settings.pwszAutoText))
											buf.Append(g_Settings.pwszAutoText);
									}

									if (chr.cpMax != -1) {
										tr2.chrg.cpMin = chr.cpMax;
										tr2.chrg.cpMax = chr.cpMax + 1;
										// if there is no space after selection,
										// or there is nothing after selection at all...
										if (!m_message.SendMsg(EM_GETTEXTRANGE, 0, (LPARAM)&tr2) || !iswspace(*tr2.lpstrText))
											buf.AppendChar(' ');
									}
									else buf.AppendChar(' ');

									m_message.SendMsg(EM_REPLACESEL, FALSE, (LPARAM)buf.c_str());
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
		}
		break;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wParam, g_Settings.crUserListBGColor);
		return (INT_PTR)g_chatApi.hListBkgBrush;

	case DM_UPDATEWINICON:
		UpdateWindowIcon();
		return 0;

	case DM_CONFIGURETOOLBAR:
		// configures the toolbar only... if lParam != 0, then it also calls
		// SetDialogToType() to reconfigure the message window
		m_bShowUIElements = m_pContainer->cfg.flags.m_bHideToolbar ? 0 : 1;

		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTERY), GWL_EXSTYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_SPLITTERY), GWL_EXSTYLE) & ~WS_EX_STATICEDGE);

		if (lParam == 1) {
			GetSendFormat();
			SetDialogToType();
			DM_RecalcPictureSize();
			Resize();
			DM_ScrollToBottom(0, 1);
		}
		return 0;

	case WM_CBD_LOADICONS:
		for (auto &it : buttonicons) {
			SendDlgItemMessage(m_hwnd, it.id, BM_SETIMAGE, IMAGE_ICON, (LPARAM)*it.pIcon);
			SendDlgItemMessage(m_hwnd, it.id, BUTTONSETCONTAINER, (LPARAM)m_pContainer, 0);
		}

		Srmm_UpdateToolbarIcons(m_hwnd);
		UpdateWindowIcon();
		return 0;

	case DM_OPTIONSAPPLIED:
		DM_OptionsApplied(lParam == 0);
		return 0;

	case DM_UPDATESTATUSMSG:
		m_pPanel.Invalidate();
		return 0;

	case DM_OWNNICKCHANGED:
		GetMyNick();
		return 0;

	case WM_KEYDOWN:
		SetFocus(m_message.GetHwnd());
		break;

	case WM_SETFOCUS:
		if (CMimAPI::m_shutDown)
			break;

		UpdateWindowState(WM_SETFOCUS);
		SetFocus(m_message.GetHwnd());
		return 1;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE) {
			m_pContainer->m_hwndSaved = nullptr;
			break;
		}
		__fallthrough;

	case WM_MOUSEACTIVATE:
		UpdateWindowState(WM_ACTIVATE);
		return 1;

	case DM_UPDATEPICLAYOUT:
		if (wParam == 0 || wParam == m_hContact) {
			LoadContactAvatar();
			Resize();
		}
		return 0;

	case DM_FORCEDREMAKELOG:
		m_dwFlags &= ~(MWF_LOG_ALL);
		m_dwFlags |= (lParam & MWF_LOG_ALL);
		m_bDeferredRemakeLog = true;
		return 0;

	case DM_APPENDMCEVENT:
		if (m_hContact == db_mc_getMeta(wParam) && m_hDbEventFirst == 0) {
			m_hDbEventFirst = lParam;
			RemakeLog();
		}
		else if (m_hContact == wParam && db_mc_isSub(wParam) && db_event_getContact(lParam) != wParam)
			StreamEvents(lParam, 1, 1);
		return 0;

	case HM_DBEVENTADDED:
		// this is called whenever a new event has been added to the database.
		// this CAN be posted (some sanity checks required).
		if (this)
			DM_EventAdded(m_hContact, lParam);
		return 0;

	case WM_TIMER:
		// timer id for message timeouts is composed like:
		// for single message sends: basevalue (TIMERID_MSGSEND) + send queue index
		if (wParam >= TIMERID_MSGSEND) {
			int iIndex = wParam - TIMERID_MSGSEND;
			if (iIndex < SendQueue::NR_SENDJOBS) { // single sendjob timer
				SendJob *job = sendQueue->getJobByIndex(iIndex);
				KillTimer(m_hwnd, wParam);

				replaceStrW(job->pwszErrorMsg, CMStringW(FORMAT, TranslateT("Delivery failure: %s"), TranslateT("The message send timed out")));
				job->iStatus = SendQueue::SQ_ERROR;
				if (!NEN::bNoSounds && !m_pContainer->cfg.flags.m_bNoSound)
					Skin_PlaySound("SendError");
				if (!m_bErrorState)
					sendQueue->handleError(this, iIndex);
				break;
			}
		}
		break;

	case DM_QUERYLASTUNREAD:
		// return timestamp (in ticks) of last recent message which has not been read yet.
		// 0 if there is none
		// lParam = pointer to a dword receiving the value.
		{
			uint32_t *pdw = (uint32_t *)lParam;
			if (pdw)
				*pdw = m_dwTickLastEvent;
		}
		return 0;

	case DM_UPDATELASTMESSAGE:
		DM_UpdateLastMessage();
		return 0;

	case DM_SAVESIZE:
		if (m_bNeedCheckSize)
			lParam = 0;

		m_bNeedCheckSize = false;
		if (m_bWasBackgroundCreate) {
			m_bInitMode = false;
			if (m_lastMessage)
				DM_UpdateLastMessage();
		}

		m_pContainer->QueryClientArea(rcClient);
		MoveWindow(m_hwnd, rcClient.left, rcClient.top, (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top), TRUE);
		if (m_bWasBackgroundCreate) {
			m_bWasBackgroundCreate = false;
			Resize();
			PostMessage(m_hwnd, DM_UPDATEPICLAYOUT, 0, 0);

			SetFocus(m_message.GetHwnd());
			if (m_pContainer->cfg.flags.m_bSideBar)
				m_pContainer->m_pSideBar->moveButtons();
		}
		else {
			Resize();
			if (lParam == 0)
				DM_ScrollToBottom(0, 1);
		}
		return 0;

	case WM_LBUTTONDOWN:
		GetCursorPos(&tmp);
		cur.x = (SHORT)tmp.x;
		cur.y = (SHORT)tmp.y;
		if (!m_pPanel.isHovered())
			SendMessage(m_pContainer->m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, *((LPARAM*)(&cur)));
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

	case WM_RBUTTONUP:
		{
			RECT rcPicture, rcPanelNick = { 0 };
			int menuID = 0;

			GetWindowRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), &rcPicture);
			rcPanelNick.left = rcPanelNick.right - 30;
			GetCursorPos(&pt);

			if (m_pPanel.invokeConfigDialog(pt))
				break;

			if (PtInRect(&rcPicture, pt))
				menuID = MENU_PICMENU;

			if ((menuID == MENU_PICMENU && ((m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown) || m_hOwnPic) && m_bShowAvatar != 0)) {
				HMENU submenu = GetSubMenu(PluginConfig.g_hMenuContext, 1);
				GetCursorPos(&pt);
				MsgWindowUpdateMenu(submenu, menuID);

				int iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
				MsgWindowMenuHandler(iSelection, menuID);
				break;
			}

			HMENU subMenu = GetSubMenu(PluginConfig.g_hMenuContext, 0);
			MsgWindowUpdateMenu(subMenu, MENU_TABCONTEXT);

			int iSelection = TrackPopupMenu(subMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);
			if (iSelection >= IDM_CONTAINERMENU) {
				char szIndex[10];
				char *szKey = "TAB_ContainersW";

				mir_snprintf(szIndex, "%d", iSelection - IDM_CONTAINERMENU);
				if (iSelection - IDM_CONTAINERMENU >= 0) {
					ptrW val(db_get_wsa(0, szKey, szIndex));
					if (val)
						SwitchToContainer(val);
				}
				break;
			}
			MsgWindowMenuHandler(iSelection, MENU_TABCONTEXT);
		}
		break;

	case WM_MOUSEMOVE:
		GetCursorPos(&pt);
		DM_DismissTip(pt);
		m_pPanel.trackMouse(pt);
		break;

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

				USERINFO *ui = g_chatApi.UM_FindUserFromIndex(m_si, index);
				if (ui == nullptr)
					return TRUE;

				int height = dis->rcItem.bottom - dis->rcItem.top;
				if (height & 1)
					height++;
				int offset = (height == 10) ? 0 : height / 2;

				HICON hIcon = g_chatApi.SM_GetStatusIcon(m_si, ui);
				HFONT hFont = g_Settings.UserListFonts[ui->iStatusEx];
				HFONT hOldFont = (HFONT)SelectObject(dis->hDC, hFont);
				SetBkMode(dis->hDC, TRANSPARENT);

				int nickIndex = 0;
				for (int i = 0; i < STATUSICONCOUNT; i++) {
					if (hIcon == g_chatApi.hStatusIcons[i]) {
						nickIndex = i;
						break;
					}
				}

				if (dis->itemState & ODS_SELECTED) {
					FillRect(dis->hDC, &dis->rcItem, g_Settings.SelectionBGBrush);
					SetTextColor(dis->hDC, g_Settings.nickColors[6]);
				}
				else {
					FillRect(dis->hDC, &dis->rcItem, g_chatApi.hListBkgBrush);
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
		return MsgWindowDrawHandler((DRAWITEMSTRUCT *)lParam);

	case WM_NCHITTEST:
		SendMessage(m_pContainer->m_hwnd, WM_NCHITTEST, wParam, lParam);
		break;

	case WM_APPCOMMAND:
		{
			uint32_t cmd = GET_APPCOMMAND_LPARAM(lParam);
			if (cmd == APPCOMMAND_BROWSER_BACKWARD || cmd == APPCOMMAND_BROWSER_FORWARD) {
				m_pContainer->SelectTab(cmd == APPCOMMAND_BROWSER_BACKWARD ? DM_SELECT_PREV : DM_SELECT_NEXT);
				return 1;
			}
		}
		break;

	case WM_COMMAND:
		DM_MsgWindowCmdHandler(LOWORD(wParam), wParam, lParam);
		break;

	case WM_CONTEXTMENU:
		{
			uint32_t idFrom = GetDlgCtrlID((HWND)wParam);
			if (idFrom >= MIN_CBUTTONID && idFrom <= MAX_CBUTTONID) {
				Srmm_ClickToolbarIcon(m_hContact, idFrom, m_hwnd, 1);
				break;
			}
		}
		break;

	case HM_EVENTSENT:
		// this is now *only* called from the global ME_PROTO_ACK handler (static int ProtoAck() in msgs.c)
		// it receives:
		// wParam = index of the sendjob in the queue in the low word, index of the found sendID in the high word
		// (normally 0, but if its a multisend job, then the sendjob may contain more than one hContact/iSendId pairs.)
		// lParam = the original ackdata
		//
		// the "per message window" ACK hook is gone, the global ack handler cares about all types of ack's (currently
		// *_MESSAGE and *_AVATAR and dispatches them to the owner windows).
		sendQueue->ackMessage(this, wParam, lParam);
		return 0;

	case WM_NEXTDLGCTL:
		if (m_bWasBackgroundCreate)
			return 1;
		break;

	case DM_IEVIEWOPTIONSCHANGED:
		RemakeLog();
		break;

	case DM_SMILEYOPTIONSCHANGED:
		if (isChat())
			RedrawLog();
		else
			RemakeLog();
		break;

	case DM_MYAVATARCHANGED:
		{
			const char *szProto = m_cache->getActiveProto();
			if (!mir_strcmp((char *)wParam, szProto) && mir_strlen(szProto) == mir_strlen((char *)wParam))
				LoadOwnAvatar();
		}
		break;

	case DM_CLIENTCHANGED:
		GetClientIcon();
		if (m_hClientIcon && m_pPanel.isActive())
			InvalidateRect(m_hwnd, nullptr, TRUE);
		return 0;

	case DM_UPDATEUIN:
		if (m_pPanel.isActive())
			m_pPanel.Invalidate();
		if (m_pContainer->cfg.flags.m_bUinStatusBar)
			tabUpdateStatusBar();
		return 0;

	case EM_THEMECHANGED:
		DM_FreeTheme();
		DM_ThemeChanged();
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
			CB_DestroyButton((uint32_t)wParam, (uint32_t)lParam);
		else
			CB_DestroyAllButtons();
		return 0;

	case WM_DROPFILES:
		if (!ProcessFileDrop((HDROP)wParam, m_cache->getActiveContact()))
			ActivateTooltip(IDC_SRMM_MESSAGE, TranslateT("Contact is offline and this protocol does not support sending files to offline users."));
		return 0;

	case DM_CHECKQUEUEFORCLOSE:
		{
			int *uOpen = (int*)lParam;
			if (uOpen)
				*uOpen += m_iOpenJobs;
		}
		return 0;

	case WM_CLOSE:
		// usual close, not forced
		if (wParam == 0 && lParam == 0) {
			// esc handles error controls if we are in error state (error controls visible)
			if (m_bErrorState) {
				DM_ErrorDetected(MSGERROR_CANCEL, 0);
				return TRUE;
			}

			switch (PluginConfig.m_EscapeCloses) {
			case 1: // minimize container
				SendMessage(m_pContainer->m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				return TRUE;

			case 2: // close or hide, optionally
				if (g_plugin.bHideOnClose) {
					ShowWindow(m_pContainer->m_hwnd, SW_HIDE);
					return TRUE;
				}
				break;

			case 3: // do nothing
				_dlgReturn(m_hwnd, FALSE);
				return TRUE;
			}
			_dlgReturn(m_hwnd, TRUE);
		}

		if (m_iOpenJobs > 0 && lParam != 2) {
			if (m_bErrorState)
				DM_ErrorDetected(MSGERROR_CANCEL, 1);
			else {
				if (m_bWarnClose)
					return TRUE;

				m_bWarnClose = true;
				LRESULT result = SendQueue::WarnPendingJobs(0);
				m_bWarnClose = false;
				if (result == IDNO)
					return TRUE;
			}
		}
		CloseTab();
		return 0;

	case WM_DWMCOMPOSITIONCHANGED:
		BB_RefreshTheme();
		m_pContainer->ClearMargins();
		VerifyProxy();
		break;

	case DM_FORCEREDRAW:
		RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		return 0;

	case DM_CHECKINFOTIP:
		m_pPanel.hideTip(reinterpret_cast<HWND>(lParam));
		return 0;

	case DM_SETINFOPANEL: // broadcasted when global info panel setting changes
		if (wParam == 0 && lParam == 0) {
			m_pPanel.getVisibility();
			m_pPanel.loadHeight();
			m_pPanel.showHide();
		}
		else {
			CMsgDialog *srcDat = (CMsgDialog *)wParam;
			if (lParam == 0)
				m_pPanel.loadHeight();
			else {
				if (srcDat && lParam && this != srcDat && !m_pPanel.isPrivateHeight()) {
					if (srcDat->isChat() != isChat() && M.GetByte("syncAllPanels", 0) == 0)
						return 0;

					if (m_pContainer->cfg.fPrivate && srcDat->m_pContainer != m_pContainer)
						return 0;

					m_pPanel.setHeight((LONG)lParam);
				}
			}
			Resize();
		}
		return 0;

	case DM_STATUSICONCHANGE:
		m_pContainer->InitRedraw();
		return 0;

	case DM_QUERYHCONTACT:
		if (lParam)
			*(MCONTACT *)lParam = m_hContact;
		return 0;

	case DM_CHECKSIZE:
		m_bNeedCheckSize = true;
		return 0;

	case DM_STATUSBARCHANGED:
		tabUpdateStatusBar();
		break;

	case DM_CHECKAUTOHIDE:
		// This is broadcasted by the container to all child windows to check if the
		// container can be autohidden or -closed.
		//
		// wParam is the autohide timeout (in seconds)
		// lParam points to a BOOL and a session which wants to prevent auto-hiding
		// the container must set it to FALSE.
		//
		// If no session in the container disagrees, the container will be hidden.

		if (lParam) {
			BOOL *fResult = (BOOL *)lParam;
			// text entered in the input area -> prevent autohide/cose
			if (GetWindowTextLength(m_message.GetHwnd()) > 0)
				*fResult = FALSE;
			// unread events, do not hide or close the container
			else if (m_dwUnread)
				*fResult = FALSE;
			// time since last activity did not yet reach the threshold.
			else if (((GetTickCount() - m_dwLastActivity) / 1000) <= wParam)
				*fResult = FALSE;
		}
		return 0;

	case DM_SPLITTERGLOBALEVENT:
		DM_SplitterGlobalEvent(wParam, lParam);
		return 0;
	}

	return CSuper::DlgProc(uMsg, wParam, lParam);
}
