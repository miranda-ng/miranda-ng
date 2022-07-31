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

UINT_PTR CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// show the balloon tooltip control.

void CMsgDialog::ActivateTooltip(int iCtrlId, const wchar_t *pwszMessage)
{
	if (!IsIconic(m_pContainer->m_hwnd) && m_pContainer->m_hwndActive == m_hwnd)
		m_pPanel.showTip(iCtrlId, pwszMessage);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::AddLog()
{
	if (PluginConfig.m_bUseDividers) {
		if (PluginConfig.m_bDividersUsePopupConfig) {
			if (!MessageWindowOpened(0, this))
				DM_AddDivider();
		}
		else {
			if (!IsActive())
				DM_AddDivider();
			else if (m_pContainer->m_hwndActive != m_hwnd)
				DM_AddDivider();
		}
	}

	CSrmmBaseDialog::AddLog();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::AdjustBottomAvatarDisplay()
{
	GetAvatarVisibility();

	bool bInfoPanel = m_pPanel.isActive();
	HBITMAP hbm = (bInfoPanel && m_pContainer->cfg.avatarMode != 3) ? m_hOwnPic : (m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown);
	if (hbm) {
		if (m_dynaSplitter == 0 || m_iSplitterY == 0)
			LoadSplitter();
		m_dynaSplitter = m_iSplitterY - DPISCALEY_S(34);
		DM_RecalcPictureSize();
		Utils::showDlgControl(m_hwnd, IDC_CONTACTPIC, m_bShowAvatar ? SW_SHOW : SW_HIDE);
		InvalidateRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), nullptr, TRUE);
	}
	else {
		Utils::showDlgControl(m_hwnd, IDC_CONTACTPIC, m_bShowAvatar ? SW_SHOW : SW_HIDE);
		m_pic.cy = m_pic.cx = DPISCALEY_S(60);
		InvalidateRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), nullptr, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculates avatar layouting, based on splitter position to find the optimal size
// for the avatar w/o disturbing the toolbar too much.

void CMsgDialog::CalcDynamicAvatarSize(BITMAP *bminfo)
{
	if (m_bWasBackgroundCreate || m_pContainer->cfg.flags.m_bDeferredConfigure || m_pContainer->cfg.flags.m_bCreateMinimized || IsIconic(m_pContainer->m_hwnd))
		return;  // at this stage, the layout is not yet ready...

	RECT rc;
	GetClientRect(m_hwnd, &rc);

	BOOL bBottomToolBar = m_pContainer->cfg.flags.m_bBottomToolbar;
	BOOL bToolBar = m_pContainer->cfg.flags.m_bHideToolbar ? 0 : 1;
	int  iSplitOffset = m_bIsAutosizingInput ? 1 : 0;

	double picAspect = (bminfo->bmWidth == 0 || bminfo->bmHeight == 0) ? 1.0 : (double)(bminfo->bmWidth / (double)bminfo->bmHeight);
	double picProjectedWidth = (double)((m_dynaSplitter - ((bBottomToolBar && bToolBar) ? DPISCALEX_S(24) : 0) + ((m_bShowUIElements) ? DPISCALEX_S(28) : DPISCALEX_S(2)))) * picAspect;

	if ((rc.right - (int)picProjectedWidth) > (m_iButtonBarReallyNeeds) && !PluginConfig.m_bAlwaysFullToolbarWidth && bToolBar)
		m_iRealAvatarHeight = m_dynaSplitter + 3 + (m_bShowUIElements ? DPISCALEY_S(28) : DPISCALEY_S(2));
	else
		m_iRealAvatarHeight = m_dynaSplitter + DPISCALEY_S(6) + DPISCALEY_S(iSplitOffset);

	m_iRealAvatarHeight -= ((bBottomToolBar && bToolBar) ? DPISCALEY_S(22) : 0);

	if (PluginConfig.m_LimitStaticAvatarHeight > 0)
		m_iRealAvatarHeight = min(m_iRealAvatarHeight, PluginConfig.m_LimitStaticAvatarHeight);

	if (M.GetByte(m_hContact, "dontscaleavatars", M.GetByte("dontscaleavatars", 0)))
		m_iRealAvatarHeight = min(bminfo->bmHeight, m_iRealAvatarHeight);

	double aspect = (bminfo->bmHeight != 0) ? (double)m_iRealAvatarHeight / (double)bminfo->bmHeight : 1.0;
	double newWidth = (double)bminfo->bmWidth * aspect;
	if (newWidth > (double)(rc.right) * 0.8)
		newWidth = (double)(rc.right) * 0.8;
	m_pic.cy = m_iRealAvatarHeight + 2;
	m_pic.cx = (int)newWidth + 2;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::CloseTab()
{
	int iTabs = TabCtrl_GetItemCount(m_hwndParent);
	if (iTabs == 1) {
		SendMessage(m_pContainer->m_hwnd, WM_CLOSE, 0, 1);
		return;
	}

	m_pContainer->m_iChilds--;
	int i = GetTabIndexFromHWND(m_hwndParent, m_hwnd);

	// after closing a tab, we need to activate the tab to the left side of
	// the previously open tab.
	// normally, this tab has the same index after the deletion of the formerly active tab
	// unless, of course, we closed the last (rightmost) tab.
	if (!m_pContainer->m_bDontSmartClose && iTabs > 1) {
		if (i == iTabs - 1)
			i--;
		else
			i++;
		TabCtrl_SetCurSel(m_hwndParent, i);

		m_pContainer->m_hwndActive = GetTabWindow(m_hwndParent, i);

		RECT rc;
		m_pContainer->QueryClientArea(rc);
		SetWindowPos(m_pContainer->m_hwndActive, HWND_TOP, rc.left, rc.top, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
		ShowWindow(m_pContainer->m_hwndActive, SW_SHOW);
		SetForegroundWindow(m_pContainer->m_hwndActive);
		SetFocus(m_pContainer->m_hwndActive);
	}

	SendMessage(m_pContainer->m_hwnd, WM_SIZE, 0, 0);
	DestroyWindow(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// calculate the minimum required client height for the given message
// window layout
//
// the container will use this in its WM_GETMINMAXINFO handler to set
// minimum tracking height.

void CMsgDialog::DetermineMinHeight()
{
	RECT rc;
	LONG height = (m_pPanel.isActive() ? m_pPanel.getHeight() + 2 : 0);
	if (!m_pContainer->cfg.flags.m_bHideToolbar)
		height += DPISCALEY_S(24); // toolbar
	GetClientRect(m_message.GetHwnd(), &rc);
	height += rc.bottom; // input area
	height += 40; // min space for log area and some padding

	m_pContainer->m_uChildMinHeight = height;
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert rich edit code to bbcode (if wanted). Otherwise, strip all RTF formatting
// tags and return plain text

static wchar_t tszRtfBreaks[] = L" \\\n\r";

static void CreateColorMap(CMStringW &Text, int iCount, COLORREF *pSrc, int *pDst)
{
	const wchar_t *pszText = Text;
	int iIndex = 1;

	static const wchar_t *lpszFmt = L"\\red%[^ \x5b\\]\\green%[^ \x5b\\]\\blue%[^ \x5b;];";
	wchar_t szRed[10], szGreen[10], szBlue[10];

	const wchar_t *p1 = wcsstr(pszText, L"\\colortbl");
	if (!p1)
		return;

	const wchar_t *pEnd = wcschr(p1, '}');

	const wchar_t *p2 = wcsstr(p1, L"\\red");

	for (int i = 0; i < iCount; i++)
		pDst[i] = -1;

	while (p2 && p2 < pEnd) {
		if (swscanf(p2, lpszFmt, &szRed, &szGreen, &szBlue) > 0) {
			for (int i = 0; i < iCount; i++) {
				if (pSrc[i] == RGB(_wtoi(szRed), _wtoi(szGreen), _wtoi(szBlue)))
					pDst[i] = iIndex;
			}
		}
		iIndex++;
		p1 = p2;
		p1++;

		p2 = wcsstr(p1, L"\\red");
	}
}

static int RtfColorToIndex(int iNumColors, int *pIndex, int iCol)
{
	for (int i = 0; i < iNumColors; i++)
		if (pIndex[i] == iCol)
			return i;

	return -1;
}

BOOL CMsgDialog::DoRtfToTags(CMStringW &pszText) const
{
	if (pszText.IsEmpty())
		return FALSE;

	// used to filter out attributes which are already set for the default message input area font
	auto &lf = m_pContainer->m_theme.logFonts[MSGFONTID_MESSAGEAREA];

	// create an index of colors in the module and map them to
	// corresponding colors in the RTF color table
	int iNumColors = Utils::rtf_clrs.getCount();
	int *pIndex = (int *)_alloca(iNumColors * sizeof(int));
	COLORREF *pColors = (COLORREF *)_alloca(iNumColors * sizeof(COLORREF));
	for (int i = 0; i < iNumColors; i++)
		pColors[i] = Utils::rtf_clrs[i].clr;
	CreateColorMap(pszText, iNumColors, pColors, pIndex);

	// scan the file for rtf commands and remove or parse them
	int idx = pszText.Find(L"\\pard");
	if (idx == -1) {
		if ((idx = pszText.Find(L"\\ltrpar")) == -1)
			return FALSE;
		idx += 7;
	}
	else idx += 5;

	MODULEINFO *mi = (isChat()) ? m_si->pMI : nullptr;

	bool bInsideColor = false, bInsideUl = false;
	CMStringW res;

	// iterate through all characters, if rtf control character found then take action
	for (const wchar_t *p = pszText.GetString() + idx; *p;) {
		switch (*p) {
		case '\\':
			if (p[1] == '\\' || p[1] == '{' || p[1] == '}') { // escaped characters
				res.AppendChar(p[1]);
				p += 2; break;
			}
			if (p[1] == '~') { // non-breaking space
				res.AppendChar(0xA0);
				p += 2; break;
			}

			if (!wcsncmp(p, L"\\cf", 3)) { // foreground color
				int iCol = _wtoi(p + 3);
				int iInd = RtfColorToIndex(iNumColors, pIndex, iCol);

				if (iCol > 0) {
					if (isChat()) {
						if (mi && mi->bColor) {
							if (iInd >= 0) {
								if (!(res.IsEmpty() && m_pContainer->m_theme.fontColors[MSGFONTID_MESSAGEAREA] == pColors[iInd]))
									res.AppendFormat(L"%%c%u", iInd);
							}
							else if (!res.IsEmpty())
								res.Append(L"%%C");
						}
					}
					else res.AppendFormat((iInd >= 0) ? (bInsideColor ? L"[/color][color=%s]" : L"[color=%s]") : (bInsideColor ? L"[/color]" : L""), Utils::rtf_clrs[iInd].szName);
				}

				bInsideColor = iInd >= 0;
			}
			else if (!wcsncmp(p, L"\\highlight", 10)) { // background color
				if (isChat()) {
					if (mi && mi->bBkgColor) {
						int iInd = RtfColorToIndex(iNumColors, pIndex, _wtoi(p + 10));
						if (iInd >= 0) {
							// if the entry field is empty & the color passed is the back color, skip it
							if (!(res.IsEmpty() && m_pContainer->m_theme.inputbg == pColors[iInd]))
								res.AppendFormat(L"%%f%u", iInd);
						}
						else if (!res.IsEmpty())
							res.AppendFormat(L"%%F");
					}
				}
			}
			else if (!wcsncmp(p, L"\\line", 5)) { // soft line break;
				res.AppendChar('\n');
			}
			else if (!wcsncmp(p, L"\\endash", 7)) {
				res.AppendChar(0x2013);
			}
			else if (!wcsncmp(p, L"\\emdash", 7)) {
				res.AppendChar(0x2014);
			}
			else if (!wcsncmp(p, L"\\bullet", 7)) {
				res.AppendChar(0x2022);
			}
			else if (!wcsncmp(p, L"\\ldblquote", 10)) {
				res.AppendChar(0x201C);
			}
			else if (!wcsncmp(p, L"\\rdblquote", 10)) {
				res.AppendChar(0x201D);
			}
			else if (!wcsncmp(p, L"\\lquote", 7)) {
				res.AppendChar(0x2018);
			}
			else if (!wcsncmp(p, L"\\rquote", 7)) {
				res.AppendChar(0x2019);
			}
			else if (!wcsncmp(p, L"\\b", 2)) { //bold
				if (isChat()) {
					if (mi && mi->bBold)
						res.Append((p[2] != '0') ? L"%b" : L"%B");
				}
				else {
					if (!(lf.lfWeight == FW_BOLD)) // only allow bold if the font itself isn't a bold one, otherwise just strip it..
						if (m_SendFormat)
							res.Append((p[2] != '0') ? L"[b]" : L"[/b]");
				}
			}
			else if (!wcsncmp(p, L"\\i", 2)) { // italics
				if (isChat()) {
					if (mi && mi->bItalics)
						res.Append((p[2] != '0') ? L"%i" : L"%I");
				}
				else {
					if (!lf.lfItalic && m_SendFormat)
						res.Append((p[2] != '0') ? L"[i]" : L"[/i]");
				}
			}
			else if (!wcsncmp(p, L"\\strike", 7)) { // strike-out
				if (!lf.lfStrikeOut && m_SendFormat)
					res.Append((p[7] != '0') ? L"[s]" : L"[/s]");
			}
			else if (!wcsncmp(p, L"\\ul", 3)) { // underlined
				if (isChat()) {
					if (mi && mi->bUnderline)
						res.Append((p[3] != '0') ? L"%u" : L"%U");
				}
				else {
					if (!lf.lfUnderline && m_SendFormat) {
						if (p[3] == 0 || wcschr(tszRtfBreaks, p[3])) {
							res.Append(L"[u]");
							bInsideUl = true;
						}
						else if (!wcsncmp(p + 3, L"none", 4)) {
							if (bInsideUl)
								res.Append(L"[/u]");
							bInsideUl = false;
						}
					}
				}
			}
			else if (!wcsncmp(p, L"\\tab", 4)) { // tab
				res.AppendChar('\t');
			}
			else if (p[1] == '\'') { // special character
				if (p[2] != ' ' && p[2] != '\\') {
					wchar_t tmp[10];

					if (p[3] != ' ' && p[3] != '\\') {
						wcsncpy(tmp, p + 2, 3);
						tmp[3] = 0;
					}
					else {
						wcsncpy(tmp, p + 2, 2);
						tmp[2] = 0;
					}

					// convert string containing char in hex format to int.
					wchar_t *stoppedHere;
					res.AppendChar(wcstol(tmp, &stoppedHere, 16));
				}
			}

			p++; // skip initial slash
			p += wcscspn(p, tszRtfBreaks);
			if (*p == ' ')
				p++;
			break;

		case '{': // other RTF control characters
		case '}':
			p++;
			break;

		case '%': // double % for stupid chat engine
			if (isChat())
				res.Append(L"%%");
			else
				res.AppendChar(*p);
			p++;
			break;

		default: // other text that should not be touched
			res.AppendChar(*p++);
			break;
		}
	}

	if (bInsideColor && !isChat())
		res.Append(L"[/color]");
	if (bInsideUl)
		res.Append(L"[/u]");

	pszText = res;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::EnableSendButton(bool bMode) const
{
	SendDlgItemMessage(m_hwnd, IDOK, BUTTONSETASNORMAL, bMode, 0);
	SendDlgItemMessage(m_hwnd, IDC_PIC, BUTTONSETASNORMAL, m_bEditNotesActive ? TRUE : (!bMode && m_iOpenJobs == 0) ? TRUE : FALSE, 0);

	HWND hwndOK = GetDlgItem(GetParent(GetParent(m_hwnd)), IDOK);
	if (IsWindow(hwndOK))
		SendMessage(hwndOK, BUTTONSETASNORMAL, bMode, 0);
}

void CMsgDialog::EnableSending(bool bMode) const
{
	m_message.SendMsg(EM_SETREADONLY, !bMode, 0);
	Utils::enableDlgControl(m_hwnd, IDC_CLIST, bMode);
	EnableSendButton(bMode);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::FindFirstEvent()
{
	int historyMode = g_plugin.getByte(m_hContact, SRMSGSET_LOADHISTORY, -1);
	if (historyMode == -1)
		historyMode = (int)g_plugin.getByte(SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);

	m_hDbEventFirst = db_event_firstUnread(m_hContact);

	if (m_bActualHistory)
		historyMode = LOADHISTORY_COUNT;

	DBEVENTINFO dbei = {};
	DB::ECPTR pCursor(DB::EventsRev(m_hContact, m_hDbEventFirst));

	switch (historyMode) {
	case LOADHISTORY_COUNT:
		int i;

		// ability to load only current session's history
		if (m_bActualHistory)
			i = m_cache->getSessionMsgCount();
		else
			i = g_plugin.getWord(SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT);

		for (; i > 0; i--) {
			MEVENT hPrevEvent = pCursor.FetchNext();
			if (hPrevEvent == 0)
				break;

			dbei.cbBlob = 0;
			m_hDbEventFirst = hPrevEvent;
			db_event_get(m_hDbEventFirst, &dbei);
			if (!DbEventIsShown(&dbei))
				i++;
		}
		break;

	case LOADHISTORY_TIME:
		if (m_hDbEventFirst == 0)
			dbei.timestamp = time(0);
		else
			db_event_get(m_hDbEventFirst, &dbei);

		uint32_t firstTime = dbei.timestamp - 60 * g_plugin.getWord(SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);

		while (MEVENT hPrevEvent = pCursor.FetchNext()) {
			dbei.cbBlob = 0;
			db_event_get(hPrevEvent, &dbei);
			if (dbei.timestamp < firstTime)
				break;
			m_hDbEventFirst = hPrevEvent;
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::FlashOnClist(MEVENT hEvent, DBEVENTINFO *dbei)
{
	m_dwTickLastEvent = GetTickCount();

	if ((GetForegroundWindow() != m_pContainer->m_hwnd || m_pContainer->m_hwndActive != m_hwnd) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE) {
		m_dwUnread++;
		AddUnreadContact(m_hContact);
	}

	if (hEvent == 0)
		return;

	if (!PluginConfig.m_bFlashOnClist)
		return;

	if ((GetForegroundWindow() != m_pContainer->m_hwnd || m_pContainer->m_hwndActive != m_hwnd) && !(dbei->flags & DBEF_SENT) && dbei->eventType == EVENTTYPE_MESSAGE && !m_bFlashClist) {
		CLISTEVENT cle = {};
		cle.hContact = m_hContact;
		cle.hDbEvent = hEvent;
		cle.hIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = MS_MSG_READMESSAGE;
		g_clistApi.pfnAddEvent(&cle);

		m_bFlashClist = true;
		m_hFlashingEvent = hEvent;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// flash a tab icon if mode = true, otherwise restore default icon
// store flashing state into bState

void CMsgDialog::FlashTab(bool bInvertMode)
{
	if (bInvertMode)
		m_bTabFlash = !m_bTabFlash;

	TCITEM item = {};
	item.mask = TCIF_IMAGE;
	TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
	if (m_pContainer->cfg.flags.m_bSideBar)
		m_pContainer->m_pSideBar->updateSession(this);
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve the visiblity of the avatar window, depending on the global setting
// and local mode

bool CMsgDialog::GetAvatarVisibility()
{
	uint8_t bAvatarMode = m_pContainer->cfg.avatarMode;
	uint8_t bOwnAvatarMode = m_pContainer->cfg.ownAvatarMode;
	char hideOverride = (char)M.GetByte(m_hContact, "hideavatar", -1);

	// infopanel visible, consider own avatar display
	m_bShowAvatar = false;
	if (m_si)
		return false;

	if (m_pPanel.isActive() && bAvatarMode != 3) {
		if (!bOwnAvatarMode) {
			m_bShowAvatar = (m_hOwnPic && m_hOwnPic != PluginConfig.g_hbmUnknown);
			if (!m_hwndContactPic)
				m_hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, L"", WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(m_hwnd, IDC_CONTACTPIC), (HMENU)nullptr, nullptr, nullptr);
		}

		switch (bAvatarMode) {
		case 2:
			m_bShowInfoAvatar = false;
			break;
		case 0:
			m_bShowInfoAvatar = true;
		case 1:
			HBITMAP hbm = ((m_ace && !(m_ace->dwFlags & AVS_HIDEONCLIST)) ? m_ace->hbmPic : nullptr);
			if (hbm == nullptr && !bAvatarMode) {
				m_bShowInfoAvatar = false;
				break;
			}

			if (!m_hwndPanelPic) {
				m_hwndPanelPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, L"", WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, m_hwndPanelPicParent, (HMENU)7000, nullptr, nullptr);
				if (m_hwndPanelPic)
					SendMessage(m_hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, TRUE);
			}

			if (bAvatarMode != 0)
				m_bShowInfoAvatar = (hbm && hbm != PluginConfig.g_hbmUnknown);
			break;
		}

		if (m_bShowInfoAvatar)
			m_bShowInfoAvatar = hideOverride == 0 ? false : m_bShowInfoAvatar;
		else
			m_bShowInfoAvatar = hideOverride == 1 ? true : m_bShowInfoAvatar;

		Utils::setAvatarContact(m_hwndPanelPic, m_hContact);
		SendMessage(m_hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)m_cache->getActiveProto());
	}
	else {
		m_bShowInfoAvatar = false;

		switch (bAvatarMode) {
		case 0: // globally on
			m_bShowAvatar = true;
		LBL_Check:
			if (!m_hwndContactPic)
				m_hwndContactPic = CreateWindowEx(WS_EX_TOPMOST, AVATAR_CONTROL_CLASS, L"", WS_VISIBLE | WS_CHILD, 1, 1, 1, 1, GetDlgItem(m_hwnd, IDC_CONTACTPIC), (HMENU)nullptr, nullptr, nullptr);
			break;
		case 2: // globally OFF
			m_bShowAvatar = false;
			break;
		case 3: // on, if present
		case 1:
			HBITMAP hbm = (m_ace && !(m_ace->dwFlags & AVS_HIDEONCLIST)) ? m_ace->hbmPic : nullptr;
			m_bShowAvatar = (hbm && hbm != PluginConfig.g_hbmUnknown);
			goto LBL_Check;
		}

		if (m_bShowAvatar)
			m_bShowAvatar = hideOverride == 0 ? 0 : m_bShowAvatar;
		else
			m_bShowAvatar = hideOverride == 1 ? 1 : m_bShowAvatar;

		// reloads avatars
		if (m_hwndPanelPic) { // shows contact or user picture, depending on panel visibility
			SendMessage(m_hwndContactPic, AVATAR_SETPROTOCOL, 0, (LPARAM)m_cache->getActiveProto());
			Utils::setAvatarContact(m_hwndPanelPic, m_hContact);
		}
		else Utils::setAvatarContact(m_hwndContactPic, m_hContact);
	}
	return m_bShowAvatar;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::GetClientIcon()
{
	if (m_hClientIcon)
		DestroyIcon(m_hClientIcon);

	m_hClientIcon = nullptr;
	if (ServiceExists(MS_FP_GETCLIENTICONT)) {
		ptrW tszMirver(db_get_wsa(m_cache->getActiveContact(), m_cache->getActiveProto(), "MirVer"));
		if (tszMirver)
			m_hClientIcon = Finger_GetClientIcon(tszMirver, 1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

HICON CMsgDialog::GetMyContactIcon(LPCSTR szSetting)
{
	int bUseMeta = (szSetting == nullptr) ? false : M.GetByte(szSetting, mir_strcmp(szSetting, "MetaiconTab") == 0);
	if (bUseMeta)
		return Skin_LoadProtoIcon(m_cache->getProto(), m_cache->getStatus());
	return Skin_LoadProtoIcon(m_cache->getActiveProto(), m_cache->getActiveStatus());
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::GetMyNick()
{
	ptrW tszNick(Contact::GetInfo(CNF_CUSTOMNICK, 0, m_cache->getActiveProto()));
	if (tszNick == nullptr)
		tszNick = Contact::GetInfo(CNF_NICK, 0, m_cache->getActiveProto());
	if (tszNick != nullptr) {
		if (mir_wstrlen(tszNick) == 0 || !mir_wstrcmp(tszNick, TranslateT("'(Unknown contact)'")))
			wcsncpy_s(m_wszMyNickname, (m_myUin[0] ? m_myUin : TranslateT("'(Unknown contact)'")), _TRUNCATE);
		else
			wcsncpy_s(m_wszMyNickname, tszNick, _TRUNCATE);
	}
	else wcsncpy_s(m_wszMyNickname, L"<undef>", _TRUNCATE); // same here
}

/////////////////////////////////////////////////////////////////////////////////////////
// retrieve both buddys and my own UIN for a message session and store them in the message 
// window *dat respects metacontacts and uses the current protocol if the contact is a MC

void CMsgDialog::GetMYUIN()
{
	ptrW uid(Contact::GetInfo(CNF_DISPLAYUID, 0, m_cache->getActiveProto()));
	if (uid != nullptr)
		wcsncpy_s(m_myUin, uid, _TRUNCATE);
	else
		m_myUin[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the status of Send button

LRESULT CMsgDialog::GetSendButtonState()
{
	return m_btnOk.SendMsg(BUTTONGETSTATEID, TRUE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// reads send format and configures the toolbar buttons
// if mode == 0, int only configures the buttons and does not change send format

void CMsgDialog::GetSendFormat()
{
	m_SendFormat = M.GetDword(m_hContact, "sendformat", PluginConfig.m_SendFormat);
	if (m_SendFormat == -1)          // per contact override to disable it..
		m_SendFormat = 0;
	else if (m_SendFormat == 0)
		m_SendFormat = PluginConfig.m_SendFormat ? 1 : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

HICON CMsgDialog::GetXStatusIcon() const
{
	uint8_t xStatus = m_cache->getXStatusId();
	if (xStatus == 0)
		return nullptr;

	if (!ProtoServiceExists(m_cache->getActiveProto(), PS_GETCUSTOMSTATUSICON))
		return nullptr;

	return (HICON)(CallProtoService(m_cache->getActiveProto(), PS_GETCUSTOMSTATUSICON, xStatus, 0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// paste contents of the clipboard into the message input area and send it immediately

void CMsgDialog::HandlePasteAndSend()
{
	// is feature disabled?
	if (!PluginConfig.m_PasteAndSend) {
		ActivateTooltip(IDC_SRMM_MESSAGE, TranslateT("The 'paste and send' feature is disabled. You can enable it on the 'General' options page in the 'Sending messages' section"));
		return;
	}

	m_message.SendMsg(EM_PASTESPECIAL, CF_UNICODETEXT, 0);
	if (GetWindowTextLength(m_message.GetHwnd()) > 0)
		SendMessage(m_hwnd, WM_COMMAND, IDOK, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// is window active or not?

bool CMsgDialog::IsActive() const
{
	return m_pContainer->IsActive() && m_pContainer->m_hwndActive == m_hwnd;
}

/////////////////////////////////////////////////////////////////////////////////////////
// read keyboard state and return the state of the modifier keys

void CMsgDialog::KbdState(bool &isShift, bool &isControl, bool &isAlt)
{
	GetKeyboardState(kstate);
	isShift = (kstate[VK_SHIFT] & 0x80) != 0;
	isControl = (kstate[VK_CONTROL] & 0x80) != 0;
	isAlt = (kstate[VK_MENU] & 0x80) != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LimitMessageText(int iLen)
{
	if (this != nullptr)
		m_message.SendMsg(EM_EXLIMITTEXT, 0, iLen);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadContactAvatar()
{
	m_ace = Utils::loadAvatarFromAVS(m_bIsMeta ? db_mc_getSrmmSub(m_hContact) : m_hContact);

	BITMAP bm;
	if (m_ace && m_ace->hbmPic)
		GetObject(m_ace->hbmPic, sizeof(bm), &bm);
	else if (m_ace == nullptr)
		GetObject(PluginConfig.g_hbmUnknown, sizeof(bm), &bm);
	else
		return;

	AdjustBottomAvatarDisplay();
	CalcDynamicAvatarSize(&bm);

	if (!m_pPanel.isActive() || m_pContainer->cfg.avatarMode == 3) {
		m_iRealAvatarHeight = 0;
		PostMessage(m_hwnd, WM_SIZE, 0, 0);
	}
	else if (m_pPanel.isActive())
		GetAvatarVisibility();

	if (m_pWnd != nullptr)
		m_pWnd->verifyDwmState();
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadOwnAvatar()
{
	if (ServiceExists(MS_AV_GETMYAVATAR))
		m_ownAce = (AVATARCACHEENTRY *)CallService(MS_AV_GETMYAVATAR, 0, (LPARAM)(m_cache->getActiveProto()));
	else
		m_ownAce = nullptr;

	if (m_ownAce)
		m_hOwnPic = m_ownAce->hbmPic;
	else
		m_hOwnPic = PluginConfig.g_hbmUnknown;

	if (m_pPanel.isActive() && m_pContainer->cfg.avatarMode != 3) {
		BITMAP bm;

		m_iRealAvatarHeight = 0;
		AdjustBottomAvatarDisplay();
		GetObject(m_hOwnPic, sizeof(bm), &bm);
		CalcDynamicAvatarSize(&bm);
		Resize();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadSettings()
{
	m_clrInputBG = m_pContainer->m_theme.inputbg;
	LoadMsgDlgFont(FONTSECTION_IM, MSGFONTID_MESSAGEAREA, nullptr, &m_clrInputFG);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LoadSplitter()
{
	if (m_bIsAutosizingInput) {
		m_iSplitterY = (m_pContainer->cfg.flags.m_bBottomToolbar) ? DPISCALEY_S(46 + 22) : DPISCALEY_S(46);

		if (CSkin::m_skinEnabled && !SkinItems[ID_EXTBKINPUTAREA].IGNORED)
			m_iSplitterY += (SkinItems[ID_EXTBKINPUTAREA].MARGIN_BOTTOM + SkinItems[ID_EXTBKINPUTAREA].MARGIN_TOP - 2);
		return;
	}

	if (!m_bSplitterOverride) {
		if (!m_pContainer->cfg.fPrivate)
			m_iSplitterY = (int)M.GetDword("splitsplity", 60);
		else
			m_iSplitterY = m_pContainer->cfg.iSplitterY;
	}
	else m_iSplitterY = (int)M.GetDword(m_hContact, "splitsplity", M.GetDword("splitsplity", 60));

	if (m_iSplitterY < MINSPLITTERY)
		m_iSplitterY = 150;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::LogEvent(DBEVENTINFO &dbei)
{
	if (m_iLogMode != WANT_BUILTIN_LOG) {
		dbei.flags |= DBEF_TEMPORARY;

		MEVENT hDbEvent = db_event_add(m_hContact, &dbei);
		if (hDbEvent) {
			m_pLog->LogEvents(hDbEvent, 1, true);
			db_event_delete(hDbEvent);
		}
	}
	else LOG()->LogEvents(0, 1, true, &dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw various elements of the message window, like avatar(s), info panel fields
// and the color formatting menu

int CMsgDialog::MsgWindowDrawHandler(DRAWITEMSTRUCT *dis)
{
	if ((dis->hwndItem == GetDlgItem(m_hwnd, IDC_CONTACTPIC) && m_bShowAvatar) || (dis->hwndItem == m_hwnd && m_pPanel.isActive())) {
		HBITMAP hbmAvatar = m_ace ? m_ace->hbmPic : PluginConfig.g_hbmUnknown;
		if (hbmAvatar == nullptr)
			return TRUE;

		int top, cx, cy;
		RECT rcClient, rcFrame;
		bool bPanelPic = (dis->hwndItem == m_hwnd);
		if (bPanelPic && !m_bShowInfoAvatar)
			return TRUE;

		RECT rc;
		GetClientRect(m_hwnd, &rc);
		if (bPanelPic) {
			rcClient = dis->rcItem;
			cx = (rcClient.right - rcClient.left);
			cy = (rcClient.bottom - rcClient.top) + 1;
		}
		else {
			GetClientRect(dis->hwndItem, &rcClient);
			cx = rcClient.right;
			cy = rcClient.bottom;
		}

		if (cx < 5 || cy < 5)
			return TRUE;

		HDC hdcDraw = CreateCompatibleDC(dis->hDC);
		HBITMAP hbmDraw = CreateCompatibleBitmap(dis->hDC, cx, cy);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcDraw, hbmDraw);

		bool bAero = M.isAero();

		HRGN clipRgn = nullptr;
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcDraw, bAero ? (HBRUSH)GetStockObject(HOLLOW_BRUSH) : GetSysColorBrush(COLOR_3DFACE));
		rcFrame = rcClient;

		if (!bPanelPic) {
			top = (cy - m_pic.cy) / 2;
			RECT rcEdge = { 0, top, m_pic.cx, top + m_pic.cy };
			if (CSkin::m_skinEnabled)
				CSkin::SkinDrawBG(dis->hwndItem, m_pContainer->m_hwnd, m_pContainer, &dis->rcItem, hdcDraw);
			else if (PluginConfig.m_fillColor) {
				HBRUSH br = CreateSolidBrush(PluginConfig.m_fillColor);
				FillRect(hdcDraw, &rcFrame, br);
				DeleteObject(br);
			}
			else if (bAero && CSkin::m_pCurrentAeroEffect) {
				COLORREF clr = PluginConfig.m_tbBackgroundHigh ? PluginConfig.m_tbBackgroundHigh :
					(CSkin::m_pCurrentAeroEffect ? CSkin::m_pCurrentAeroEffect->m_clrToolbar : 0xf0f0f0);

				HBRUSH br = CreateSolidBrush(clr);
				FillRect(hdcDraw, &rcFrame, br);
				DeleteObject(br);
			}
			else FillRect(hdcDraw, &rcFrame, GetSysColorBrush(COLOR_3DFACE));

			HPEN hPenBorder = CreatePen(PS_SOLID, 1, CSkin::m_avatarBorderClr);
			HPEN hPenOld = (HPEN)SelectObject(hdcDraw, hPenBorder);

			if (CSkin::m_bAvatarBorderType == 1)
				Rectangle(hdcDraw, rcEdge.left, rcEdge.top, rcEdge.right, rcEdge.bottom);
			else if (CSkin::m_bAvatarBorderType == 2) {
				clipRgn = CreateRoundRectRgn(rcEdge.left, rcEdge.top, rcEdge.right + 1, rcEdge.bottom + 1, 6, 6);
				SelectClipRgn(hdcDraw, clipRgn);

				HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
				FrameRgn(hdcDraw, clipRgn, hbr, 1, 1);
				DeleteObject(hbr);
				DeleteObject(clipRgn);
			}

			SelectObject(hdcDraw, hPenOld);
			DeleteObject(hPenBorder);
		}

		if (bPanelPic) {
			bool bBorder = (CSkin::m_bAvatarBorderType ? true : false);

			int border_off = bBorder ? 1 : 0;
			int iMaxHeight = m_iPanelAvatarY - (bBorder ? 2 : 0);
			int iMaxWidth = m_iPanelAvatarX - (bBorder ? 2 : 0);

			rcFrame.left = rcFrame.top = 0;
			rcFrame.right = (rcClient.right - rcClient.left);
			rcFrame.bottom = (rcClient.bottom - rcClient.top);

			rcFrame.left = rcFrame.right - (LONG)m_iPanelAvatarX;
			rcFrame.bottom = (LONG)m_iPanelAvatarY;

			int height_off = (cy - iMaxHeight - (bBorder ? 2 : 0)) / 2;
			rcFrame.top += height_off;
			rcFrame.bottom += height_off;

			SendMessage(m_hwndPanelPic, AVATAR_SETAEROCOMPATDRAWING, 0, bAero ? TRUE : FALSE);
			SetWindowPos(m_hwndPanelPic, HWND_TOP, rcFrame.left + border_off, rcFrame.top + border_off,
				iMaxWidth, iMaxHeight, SWP_SHOWWINDOW | SWP_ASYNCWINDOWPOS | SWP_DEFERERASE | SWP_NOSENDCHANGING);
		}

		SelectObject(hdcDraw, hOldBrush);
		if (!bPanelPic)
			BitBlt(dis->hDC, 0, 0, cx, cy, hdcDraw, 0, 0, SRCCOPY);
		SelectObject(hdcDraw, hbmOld);
		DeleteObject(hbmDraw);
		DeleteDC(hdcDraw);
		return TRUE;
	}

	if (dis->hwndItem == GetDlgItem(m_hwnd, IDC_STATICTEXT) || dis->hwndItem == GetDlgItem(m_hwnd, IDC_LOGFROZENTEXT)) {
		wchar_t szWindowText[256];
		if (CSkin::m_skinEnabled) {
			SetTextColor(dis->hDC, CSkin::m_DefaultFontColor);
			CSkin::SkinDrawBG(dis->hwndItem, m_pContainer->m_hwnd, m_pContainer, &dis->rcItem, dis->hDC);
		}
		else {
			SetTextColor(dis->hDC, GetSysColor(COLOR_BTNTEXT));
			CSkin::FillBack(dis->hDC, &dis->rcItem);
		}
		GetWindowText(dis->hwndItem, szWindowText, _countof(szWindowText));
		szWindowText[255] = 0;
		SetBkMode(dis->hDC, TRANSPARENT);
		DrawText(dis->hDC, szWindowText, -1, &dis->rcItem, DT_SINGLELINE | DT_VCENTER | DT_NOCLIP | DT_END_ELLIPSIS);
		return TRUE;
	}

	if (dis->hwndItem == GetDlgItem(m_hwnd, IDC_STATICERRORICON)) {
		if (CSkin::m_skinEnabled)
			CSkin::SkinDrawBG(dis->hwndItem, m_pContainer->m_hwnd, m_pContainer, &dis->rcItem, dis->hDC);
		else
			CSkin::FillBack(dis->hDC, &dis->rcItem);
		DrawIconEx(dis->hDC, (dis->rcItem.right - dis->rcItem.left) / 2 - 8, (dis->rcItem.bottom - dis->rcItem.top) / 2 - 8,
			PluginConfig.g_iconErr, 16, 16, 0, nullptr, DI_NORMAL);
		return TRUE;
	}

	if (dis->CtlType == ODT_MENU && m_pPanel.isHovered()) {
		DrawMenuItem(dis, (HICON)dis->itemData, 0);
		return TRUE;
	}

	return Menu_DrawItem((LPARAM)dis);
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMsgDialog::MsgWindowUpdateMenu(HMENU submenu, int menuID)
{
	bool bInfoPanel = m_pPanel.isActive();

	if (menuID == MENU_TABCONTEXT) {
		EnableMenuItem(submenu, ID_TABMENU_LEAVECHATROOM, (isChat() && ProtoServiceExists(m_szProto, PS_LEAVECHAT)) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(submenu, ID_TABMENU_ATTACHTOCONTAINER, (M.GetByte("useclistgroups", 0) || M.GetByte("singlewinmode", 0)) ? MF_GRAYED : MF_ENABLED);
		EnableMenuItem(submenu, ID_TABMENU_CLEARSAVEDTABPOSITION, (M.GetDword(m_hContact, "tabindex", -1) != -1) ? MF_ENABLED : MF_GRAYED);
	}
	else if (menuID == MENU_PICMENU) {
		wchar_t *szText = nullptr;
		char  avOverride = (char)M.GetByte(m_hContact, "hideavatar", -1);
		HMENU visMenu = GetSubMenu(submenu, 0);
		BOOL picValid = bInfoPanel ? (m_hOwnPic != nullptr) : (m_ace && m_ace->hbmPic && m_ace->hbmPic != PluginConfig.g_hbmUnknown);

		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STRING;

		EnableMenuItem(submenu, ID_PICMENU_SAVETHISPICTUREAS, picValid ? MF_ENABLED : MF_GRAYED);

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, avOverride == -1 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, avOverride == 0 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, avOverride == 1 ? MF_CHECKED : MF_UNCHECKED);

		CheckMenuItem(submenu, ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH, PluginConfig.m_bAlwaysFullToolbarWidth ? MF_CHECKED : MF_UNCHECKED);
		if (!bInfoPanel) {
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED);
			szText = TranslateT("Contact picture settings...");
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_ENABLED);
		}
		else {
			EnableMenuItem(submenu, 0, MF_BYPOSITION | MF_GRAYED);
			EnableMenuItem(submenu, ID_PICMENU_SETTINGS, (ServiceExists(MS_AV_SETMYAVATARW) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(m_cache->getActiveProto()), 0)) ? MF_ENABLED : MF_GRAYED);
			szText = TranslateT("Set your avatar...");
		}
		mii.dwTypeData = szText;
		mii.cch = (int)mir_wstrlen(szText) + 1;
		SetMenuItemInfo(submenu, ID_PICMENU_SETTINGS, FALSE, &mii);
	}
	else if (menuID == MENU_PANELPICMENU) {
		HMENU visMenu = GetSubMenu(submenu, 0);
		char  avOverride = (char)M.GetByte(m_hContact, "hideavatar", -1);

		CheckMenuItem(visMenu, ID_VISIBILITY_DEFAULT, avOverride == -1 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(visMenu, ID_VISIBILITY_HIDDENFORTHISCONTACT, avOverride == 0 ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(visMenu, ID_VISIBILITY_VISIBLEFORTHISCONTACT, avOverride == 1 ? MF_CHECKED : MF_UNCHECKED);

		EnableMenuItem(submenu, ID_PICMENU_SETTINGS, ServiceExists(MS_AV_GETAVATARBITMAP) ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(submenu, ID_PANELPICMENU_SAVETHISPICTUREAS, (m_ace && m_ace->hbmPic && m_ace->hbmPic != PluginConfig.g_hbmUnknown) ? MF_ENABLED : MF_GRAYED);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// update state of the container - this is called whenever a tab becomes active, no matter how and
// deals with various things like updating the title bar, removing flashing icons, updating the
// session list, switching the keyboard layout (autolocale active)  and the general container status.
//
// it protects itself from being called more than once per session activation and is valid for
// normal IM sessions *only*. Group chat sessions have their own activation handler (see chat/window.c)


/////////////////////////////////////////////////////////////////////////////////////////

int CMsgDialog::MsgWindowMenuHandler(int selection, int menuId)
{
	if (menuId == MENU_PICMENU || menuId == MENU_PANELPICMENU || menuId == MENU_TABCONTEXT) {
		switch (selection) {
		case ID_TABMENU_ATTACHTOCONTAINER:
			SelectContainer();
			return 1;
		case ID_TABMENU_CONTAINEROPTIONS:
			m_pContainer->OptionsDialog();
			return 1;
		case ID_TABMENU_CLOSECONTAINER:
			SendMessage(m_pContainer->m_hwnd, WM_CLOSE, 0, 0);
			return 1;
		case ID_TABMENU_CLOSETAB:
			PostMessage(m_hwnd, WM_CLOSE, 1, 0);
			return 1;
		case ID_TABMENU_SAVETABPOSITION:
			db_set_dw(m_hContact, SRMSGMOD_T, "tabindex", m_iTabID * 100);
			break;
		case ID_TABMENU_CLEARSAVEDTABPOSITION:
			db_unset(m_hContact, SRMSGMOD_T, "tabindex");
			break;
		case ID_TABMENU_LEAVECHATROOM:
			if (isChat()) {
				char *szProto = Proto_GetBaseAccountName(m_hContact);
				if (szProto)
					CallProtoService(szProto, PS_LEAVECHAT, m_hContact, 0);
			}
			return 1;

		case ID_VISIBILITY_DEFAULT:
		case ID_VISIBILITY_HIDDENFORTHISCONTACT:
		case ID_VISIBILITY_VISIBLEFORTHISCONTACT:
			{
				uint8_t avOverrideMode;
				if (selection == ID_VISIBILITY_DEFAULT)
					avOverrideMode = -1;
				else if (selection == ID_VISIBILITY_VISIBLEFORTHISCONTACT)
					avOverrideMode = 1;
				else
					avOverrideMode = 0;
				db_set_b(m_hContact, SRMSGMOD_T, "hideavatar", avOverrideMode);
			}

			ShowPicture(false);
			Resize();
			DM_ScrollToBottom(0, 1);
			return 1;

		case ID_PICMENU_ALWAYSKEEPTHEBUTTONBARATFULLWIDTH:
			PluginConfig.m_bAlwaysFullToolbarWidth = !PluginConfig.m_bAlwaysFullToolbarWidth;
			db_set_b(0, SRMSGMOD_T, "alwaysfulltoolbar", (uint8_t)PluginConfig.m_bAlwaysFullToolbarWidth);
			Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 1);
			break;

		case ID_PICMENU_SAVETHISPICTUREAS:
			if (m_pPanel.isActive())
				SaveAvatarToFile(m_hOwnPic, 1);
			else if (m_ace)
				SaveAvatarToFile(m_ace->hbmPic, 0);
			break;

		case ID_PANELPICMENU_SAVETHISPICTUREAS:
			if (m_ace)
				SaveAvatarToFile(m_ace->hbmPic, 0);
			break;

		case ID_PICMENU_SETTINGS:
			if (menuId == MENU_PANELPICMENU)
				CallService(MS_AV_CONTACTOPTIONS, m_hContact, 0);
			else if (menuId == MENU_PICMENU) {
				if (m_pPanel.isActive()) {
					if (ServiceExists(MS_AV_SETMYAVATARW) && CallService(MS_AV_CANSETMYAVATAR, (WPARAM)(m_cache->getActiveProto()), 0))
						CallService(MS_AV_SETMYAVATARW, (WPARAM)(m_cache->getActiveProto()), 0);
				}
				else
					CallService(MS_AV_CONTACTOPTIONS, m_hContact, 0);
			}
			return 1;
		}
	}
	else if (menuId == MENU_LOGMENU) {
		switch (selection) {
		case ID_MESSAGELOGSETTINGS_GLOBAL:
			g_plugin.openOptions(nullptr, L"Message sessions", L"Message log");
			return 1;

		case ID_MESSAGELOGSETTINGS_FORTHISCONTACT:
			CallService(MS_TABMSG_SETUSERPREFS, m_hContact, 0);
			return 1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::NotifyDeliveryFailure() const
{
	if (M.GetByte("adv_noErrorPopups", 0))
		return;

	if (!Popup_Enabled())
		return;

	POPUPDATAW ppd = {};
	ppd.lchContact = m_hContact;
	ppd.PluginWindowProc = Utils::PopupDlgProcError;
	ppd.lchIcon = PluginConfig.g_iconErr;
	ppd.iSeconds = NEN::iDelayErr;
	if (!NEN::bColDefaultErr) {
		ppd.colorText = NEN::colTextErr;
		ppd.colorBack = NEN::colBackErr;
	}
	wcsncpy_s(ppd.lpwzContactName, m_cache->getNick(), _TRUNCATE);
	wcsncpy_s(ppd.lpwzText, TranslateT("A message delivery has failed.\nClick to open the message window."), _TRUNCATE);
	PUAddPopupW(&ppd);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::PlayIncomingSound() const
{
	int iPlay = MustPlaySound();
	if (iPlay) {
		if (GetForegroundWindow() == m_pContainer->m_hwnd && m_pContainer->m_hwndActive == m_hwnd)
			Skin_PlaySound("RecvMsgActive");
		else
			Skin_PlaySound("RecvMsgInactive");
	}
}

void CMsgDialog::RemakeLog()
{
	m_szMicroLf[0] = 0;
	m_lastEventTime = 0;
	m_iLastEventType = -1;
	StreamEvents(m_hDbEventFirst, -1, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// saves a contact picture to disk
// takes hbm (bitmap handle) and bool isOwnPic (1 == save the picture as your own avatar)
// requires AVS service (Miranda 0.7+)

void CMsgDialog::SaveAvatarToFile(HBITMAP hbm, int isOwnPic)
{
	wchar_t szFinalFilename[MAX_PATH];
	time_t t = time(0);
	struct tm *lt = localtime(&t);
	uint32_t setView = 1;

	wchar_t szTimestamp[100];
	mir_snwprintf(szTimestamp, L"%04u %02u %02u_%02u%02u", lt->tm_year + 1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min);

	wchar_t *szProto = mir_a2u(m_cache->getActiveProto());

	wchar_t szFinalPath[MAX_PATH];
	mir_snwprintf(szFinalPath, L"%s\\%s", M.getSavedAvatarPath(), szProto);
	mir_free(szProto);

	if (CreateDirectory(szFinalPath, nullptr) == 0) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			MessageBox(nullptr, TranslateT("Error creating destination directory"),
				TranslateT("Save contact picture"), MB_OK | MB_ICONSTOP);
			return;
		}
	}

	wchar_t szBaseName[MAX_PATH];
	if (isOwnPic)
		mir_snwprintf(szBaseName, L"My Avatar_%s", szTimestamp);
	else
		mir_snwprintf(szBaseName, L"%s_%s", m_cache->getNick(), szTimestamp);

	mir_snwprintf(szFinalFilename, L"%s.png", szBaseName);

	// do not allow / or \ or % in the filename
	Utils::sanitizeFilename(szFinalFilename);

	wchar_t filter[MAX_PATH];
	mir_snwprintf(filter, L"%s%c*.bmp;*.png;*.jpg;*.gif%c%c", TranslateT("Image files"), 0, 0, 0);

	OPENFILENAME ofn = { 0 };
	ofn.lpstrDefExt = L"png";
	ofn.lpstrFilter = filter;
	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
	ofn.lpfnHook = OpenFileSubclass;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFinalFilename;
	ofn.lpstrInitialDir = szFinalPath;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lCustData = (LPARAM)& setView;
	if (GetSaveFileName(&ofn)) {
		if (PathFileExists(szFinalFilename))
			if (MessageBox(nullptr, TranslateT("The file exists. Do you want to overwrite it?"), TranslateT("Save contact picture"), MB_YESNO | MB_ICONQUESTION) == IDNO)
				return;

		IMGSRVC_INFO ii;
		ii.cbSize = sizeof(ii);
		ii.pwszName = szFinalFilename;
		ii.hbm = hbm;
		ii.dwMask = IMGI_HBITMAP;
		ii.fif = FIF_UNKNOWN;			// get the format from the filename extension. png is default.
		Image_Save(&ii);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::SaveSplitter()
{
	if (m_bIsAutosizingInput)
		return;

	if (m_iSplitterY < DPISCALEY_S(MINSPLITTERY) || m_iSplitterY < 0)
		m_iSplitterY = DPISCALEY_S(MINSPLITTERY);

	if (m_bSplitterOverride)
		db_set_dw(m_hContact, SRMSGMOD_T, "splitsplity", m_iSplitterY);
	else {
		if (m_pContainer->cfg.fPrivate)
			m_pContainer->cfg.iSplitterY = m_iSplitterY;
		else
			db_set_dw(0, SRMSGMOD_T, "splitsplity", m_iSplitterY);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// send a pasted bitmap by file transfer.

static LIST<wchar_t> vTempFilenames(5);

void CMsgDialog::SendHBitmapAsFile(HBITMAP hbmp) const
{
	const wchar_t *mirandatempdir = L"Miranda";
	const wchar_t *filenametemplate = L"\\clp-%Y%m%d-%H%M%S0.jpg";
	wchar_t filename[MAX_PATH];
	size_t tempdirlen = GetTempPath(MAX_PATH, filename);
	bool fSend = true;

	const char *szProto = m_cache->getActiveProto();
	int wMyStatus = Proto_GetStatus(szProto);

	uint32_t protoCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	uint32_t typeCaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);

	// check protocol capabilities, status modes and visibility lists (privacy)
	// to determine whether the file can be sent. Throw a warning if any of
	// these checks fails.
	if (!(protoCaps & PF1_FILESEND))
		fSend = false;

	if ((ID_STATUS_OFFLINE == wMyStatus) || (ID_STATUS_OFFLINE == m_cache->getActiveStatus() && !(typeCaps & PF4_OFFLINEFILES)))
		fSend = false;

	if (protoCaps & PF1_VISLIST && db_get_w(m_cache->getActiveContact(), szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		fSend = false;

	if (protoCaps & PF1_INVISLIST && wMyStatus == ID_STATUS_INVISIBLE && db_get_w(m_cache->getActiveContact(), szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		fSend = false;

	if (!fSend) {
		CWarning::show(CWarning::WARN_SENDFILE, MB_OK | MB_ICONEXCLAMATION | CWarning::CWF_NOALLOWHIDE);
		return;
	}

	if (tempdirlen <= 0 || tempdirlen >= MAX_PATH - mir_wstrlen(mirandatempdir) - mir_wstrlen(filenametemplate) - 2) // -2 is because %Y takes 4 symbols
		filename[0] = 0;					// prompt for a new name
	else {
		mir_wstrcpy(filename + tempdirlen, mirandatempdir);
		if ((GetFileAttributes(filename) == INVALID_FILE_ATTRIBUTES || ((GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY) == 0)) && CreateDirectory(filename, nullptr) == 0)
			filename[0] = 0;
		else {
			tempdirlen = mir_wstrlen(filename);

			time_t rawtime;
			time(&rawtime);
			const tm *timeinfo;
			timeinfo = _localtime32((__time32_t *)& rawtime);
			wcsftime(filename + tempdirlen, MAX_PATH - tempdirlen, filenametemplate, timeinfo);
			size_t firstnumberpos = tempdirlen + 14;
			size_t lastnumberpos = tempdirlen + 20;
			while (GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES) {	// while it exists
				for (size_t pos = lastnumberpos; pos >= firstnumberpos; pos--)
					if (filename[pos]++ != '9')
						break;
					else
						if (pos == firstnumberpos)
							filename[0] = 0;	// all filenames exist => prompt for a new name
						else
							filename[pos] = '0';
			}
		}
	}

	if (filename[0] == 0) {	// prompting to save
		wchar_t filter[MAX_PATH];
		mir_snwprintf(filter, L"%s%c*.jpg%c%c", TranslateT("JPEG-compressed images"), 0, 0, 0);

		OPENFILENAME dlg;
		dlg.lStructSize = sizeof(dlg);
		dlg.lpstrFilter = filter;
		dlg.nFilterIndex = 1;
		dlg.lpstrFile = filename;
		dlg.nMaxFile = MAX_PATH;
		dlg.Flags = OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		dlg.lpstrDefExt = L"jpg";
		if (!GetSaveFileName(&dlg))
			return;
	}

	IMGSRVC_INFO ii;
	ii.cbSize = sizeof(ii);
	ii.hbm = hbmp;
	ii.pwszName = filename;
	ii.dwMask = IMGI_HBITMAP;
	ii.fif = FIF_JPEG;
	if (!Image_Save(&ii)) {
		CWarning::show(CWarning::WARN_SAVEFILE, MB_OK | MB_ICONEXCLAMATION | CWarning::CWF_NOALLOWHIDE);
		return;
	}

	vTempFilenames.insert(mir_wstrdup(filename));

	wchar_t *ppFiles[2] = { filename, nullptr };
	CallService(MS_FILE_SENDSPECIFICFILEST, m_cache->getActiveContact(), (LPARAM)&ppFiles);
}

// remove all temporary files created by the "send clipboard as file" feature.
void TSAPI CleanTempFiles()
{
	for (auto &it : vTempFilenames) {
		DeleteFileW(it);
		mir_free(it);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sets a status bar text for a contact

void CMsgDialog::SetStatusText(const wchar_t *wszText, HICON hIcon)
{
	if (wszText != nullptr) {
		m_bStatusSet = true;
		m_szStatusText = wszText;
		m_szStatusIcon = hIcon;
	}
	else {
		m_bStatusSet = false;
		m_szStatusText.Empty();
		m_szStatusIcon = nullptr;
	}

	tabUpdateStatusBar();
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
	CMsgDialog *pDlg = (CMsgDialog *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlg = (CMsgDialog *)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			uint32_t dwMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", 0);
			uint32_t dwFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", 0);

			uint32_t dwPopupMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "PopupMask", 0);
			uint32_t dwPopupFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "PopupFlags", 0);

			uint32_t dwTrayMask = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask", 0);
			uint32_t dwTrayFlags = db_get_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags", 0);

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
		if (wParam == 1 && lParam == 1 && pDlg) {
			int iFlags = 0;
			uint32_t dwMask = 0;

			for (int i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (dwMask == 0) {
				db_unset(pDlg->m_hContact, CHAT_MODULE, "FilterFlags");
				db_unset(pDlg->m_hContact, CHAT_MODULE, "FilterMask");
			}
			else {
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterFlags", iFlags);
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "FilterMask", dwMask);
			}

			dwMask = iFlags = 0;

			for (int i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_P1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}

			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (dwMask == 0) {
				db_unset(pDlg->m_hContact, CHAT_MODULE, "PopupFlags");
				db_unset(pDlg->m_hContact, CHAT_MODULE, "PopupMask");
			}
			else {
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "PopupFlags", iFlags);
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "PopupMask", dwMask);
			}

			dwMask = iFlags = 0;

			for (int i = 0; i < _countof(_eventorder); i++) {
				int result = IsDlgButtonChecked(hwndDlg, IDC_T1 + i);
				dwMask |= (result != BST_INDETERMINATE ? _eventorder[i] : 0);
				iFlags |= (result == BST_CHECKED ? _eventorder[i] : 0);
			}
			if (iFlags & GC_EVENT_ADDSTATUS)
				iFlags |= GC_EVENT_REMOVESTATUS;

			if (dwMask == 0) {
				db_unset(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags");
				db_unset(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask");
			}
			else {
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconFlags", iFlags);
				db_set_dw(pDlg->m_hContact, CHAT_MODULE, "TrayIconMask", dwMask);
			}
			Chat_SetFilters(pDlg->getChat());

			if (pDlg->m_bFilterEnabled) {
				if (pDlg->m_iLogFilterFlags == 0)
					pDlg->m_btnFilter.Click();
				pDlg->RedrawLog();
				db_set_b(pDlg->m_hContact, CHAT_MODULE, "FilterEnabled", pDlg->m_bFilterEnabled);
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

void CMsgDialog::ShowFilterMenu()
{
	m_hwndFilter = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_FILTER), m_pContainer->m_hwnd, FilterWndProc, (LPARAM)this);
	TranslateDialogDefault(m_hwndFilter);

	RECT rcFilter, rcLog;
	GetClientRect(m_hwndFilter, &rcFilter);
	GetWindowRect(m_pLog->GetHwnd(), &rcLog);

	POINT pt;
	pt.x = rcLog.right; pt.y = rcLog.bottom;
	ScreenToClient(m_pContainer->m_hwnd, &pt);

	SetWindowPos(m_hwndFilter, HWND_TOP, pt.x - rcFilter.right, pt.y - rcFilter.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::ShowPicture(bool showNewPic)
{
	if (!m_pPanel.isActive())
		m_pic.cy = m_pic.cx = DPISCALEY_S(60);

	if (showNewPic) {
		if (m_pPanel.isActive() && m_pContainer->cfg.avatarMode != 3) {
			if (!m_hwndPanelPic) {
				InvalidateRect(m_hwnd, nullptr, TRUE);
				UpdateWindow(m_hwnd);
				Resize();
			}
			return;
		}
		AdjustBottomAvatarDisplay();
	}
	else {
		m_bShowAvatar = !m_bShowAvatar;
		db_set_b(m_hContact, SRMSGMOD_T, "MOD_ShowPic", m_bShowAvatar);
	}

	RECT rc;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_CONTACTPIC), &rc);
	if (m_minEditBoxSize.cy + DPISCALEY_S(3) > m_iSplitterY)
		SplitterMoved(rc.bottom - m_minEditBoxSize.cy, GetDlgItem(m_hwnd, IDC_SPLITTERY));
	if (!showNewPic)
		SetDialogToType();
	else
		Resize();
}

/////////////////////////////////////////////////////////////////////////////////////////
// show a modified context menu for the richedit control(s)

void CMsgDialog::ShowPopupMenu(const CCtrlBase &pCtrl, POINT pt)
{
	CHARRANGE sel, all = { 0, -1 };

	HMENU hSubMenu, hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_CONTEXT));
	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG)
		hSubMenu = GetSubMenu(hMenu, 0);
	else {
		hSubMenu = GetSubMenu(hMenu, 2);
		EnableMenuItem(hSubMenu, IDM_PASTEFORMATTED, m_SendFormat != 0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hSubMenu, ID_EDITOR_PASTEANDSENDIMMEDIATELY, PluginConfig.m_PasteAndSend ? MF_ENABLED : MF_GRAYED);
		CheckMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, PluginConfig.m_visualMessageSizeIndicator ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem(hSubMenu, ID_EDITOR_SHOWMESSAGELENGTHINDICATOR, m_pContainer->m_hwndStatus ? MF_ENABLED : MF_GRAYED);
	}
	TranslateMenu(hSubMenu);
	pCtrl.SendMsg(EM_EXGETSEL, 0, (LPARAM)& sel);
	if (sel.cpMin == sel.cpMax) {
		EnableMenuItem(hSubMenu, IDM_COPY, MF_GRAYED);
		EnableMenuItem(hSubMenu, IDM_QUOTE, MF_GRAYED);
		if (pCtrl.GetCtrlId() == IDC_SRMM_MESSAGE)
			EnableMenuItem(hSubMenu, IDM_CUT, MF_GRAYED);
	}

	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG) {
		InsertMenuA(hSubMenu, 6, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
		CheckMenuItem(hSubMenu, ID_LOG_FREEZELOG, m_bScrollingDisabled ? MF_CHECKED : MF_UNCHECKED);
	}

	MessageWindowPopupData mwpd;
	// First notification
	mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
	mwpd.uFlags = (pCtrl.GetCtrlId() == IDC_SRMM_LOG ? MSG_WINDOWPOPUP_LOG : MSG_WINDOWPOPUP_INPUT);
	mwpd.hContact = m_hContact;
	mwpd.hwnd = pCtrl.GetHwnd();
	mwpd.hMenu = hSubMenu;
	mwpd.selection = 0;
	mwpd.pt = pt;
	NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)& mwpd);

	int iSelection = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hwnd, nullptr);

	// Second notification
	mwpd.selection = iSelection;
	mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
	NotifyEventHooks(g_chatApi.hevWinPopup, 0, (LPARAM)& mwpd);

	switch (iSelection) {
	case IDM_COPY:
		pCtrl.SendMsg(WM_COPY, 0, 0);
		break;
	case IDM_CUT:
		pCtrl.SendMsg(WM_CUT, 0, 0);
		break;
	case IDM_PASTE:
	case IDM_PASTEFORMATTED:
		if (pCtrl.GetCtrlId() == IDC_SRMM_MESSAGE)
			pCtrl.SendMsg(EM_PASTESPECIAL, (iSelection == IDM_PASTE) ? CF_UNICODETEXT : 0, 0);
		break;
	case IDM_COPYALL:
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)& all);
		pCtrl.SendMsg(WM_COPY, 0, 0);
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)& sel);
		break;
	case IDM_QUOTE:
		SendMessage(m_hwnd, WM_COMMAND, IDC_QUOTE, 0);
		break;
	case IDM_SELECTALL:
		pCtrl.SendMsg(EM_EXSETSEL, 0, (LPARAM)& all);
		break;
	case IDM_CLEAR:
		tabClearLog();
		break;
	case ID_LOG_FREEZELOG:
		SendDlgItemMessage(m_hwnd, IDC_SRMM_LOG, WM_KEYDOWN, VK_F12, 0);
		break;
	case ID_EDITOR_SHOWMESSAGELENGTHINDICATOR:
		PluginConfig.m_visualMessageSizeIndicator = !PluginConfig.m_visualMessageSizeIndicator;
		db_set_b(0, SRMSGMOD_T, "msgsizebar", (uint8_t)PluginConfig.m_visualMessageSizeIndicator);
		Srmm_Broadcast(DM_CONFIGURETOOLBAR, 0, 0);
		Resize();
		if (m_pContainer->m_hwndStatus)
			RedrawWindow(m_pContainer->m_hwndStatus, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	case ID_EDITOR_PASTEANDSENDIMMEDIATELY:
		HandlePasteAndSend();
		break;
	}

	if (pCtrl.GetCtrlId() == IDC_SRMM_LOG)
		RemoveMenu(hSubMenu, 7, MF_BYPOSITION);
	DestroyMenu(hMenu);
}

void CMsgDialog::SplitterMoved(int coord, HWND hwnd)
{
	POINT pt;
	RECT rc;

	switch (GetDlgCtrlID(hwnd)) {
	case IDC_MULTISPLITTER:
		GetClientRect(m_hwnd, &rc);
		pt.x = coord;
		pt.y = 0;
		ScreenToClient(m_hwnd, &pt);
		{
			int oldSplitterX = m_iMultiSplit;
			m_iMultiSplit = rc.right - pt.x;
			if (m_iMultiSplit < 25)
				m_iMultiSplit = 25;

			if (m_iMultiSplit > ((rc.right - rc.left) - 80))
				m_iMultiSplit = oldSplitterX;
		}
		Resize();
		break;

	case IDC_SPLITTERX:
		GetClientRect(m_hwnd, &rc);
		pt.x = coord, pt.y = 0;
		ScreenToClient(m_hwnd, &pt);
		{
			int iSplitterX = rc.right - pt.x + 1;
			if (iSplitterX < 35)
				iSplitterX = 35;
			if (iSplitterX > rc.right - rc.left - 35)
				iSplitterX = rc.right - rc.left - 35;
			m_pContainer->cfg.iSplitterX = iSplitterX;
		}
		Resize();
		break;

	case IDC_SPLITTERY:
		GetClientRect(m_hwnd, &rc);
		rc.top += (m_pPanel.isActive() ? m_pPanel.getHeight() + 40 : 30);
		pt.x = 0;
		pt.y = coord;
		ScreenToClient(m_hwnd, &pt);
		{
			int oldSplitterY = m_iSplitterY;
			int oldDynaSplitter = m_dynaSplitter;

			m_iSplitterY = rc.bottom - pt.y + DPISCALEY_S(23);

			// attempt to fix splitter troubles..
			// hardcoded limits... better solution is possible, but this works for now
			int bottomtoolbarH = 0;
			if (m_pContainer->cfg.flags.m_bBottomToolbar)
				bottomtoolbarH = 22;

			if (m_iSplitterY < (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH)) {	// min splitter size
				m_iSplitterY = (DPISCALEY_S(MINSPLITTERY) + 5 + bottomtoolbarH);
				m_dynaSplitter = m_iSplitterY - DPISCALEY_S(34);
				DM_RecalcPictureSize();
			}
			else if (m_iSplitterY > (rc.bottom - rc.top)) {
				m_iSplitterY = oldSplitterY;
				m_dynaSplitter = oldDynaSplitter;
				DM_RecalcPictureSize();
			}
			else {
				m_dynaSplitter = (rc.bottom - pt.y) - DPISCALEY_S(11);
				DM_RecalcPictureSize();
			}
		}
		UpdateToolbarBG();
		Resize();
		break;

	case IDC_PANELSPLITTER:
		GetClientRect(m_pLog->GetHwnd(), &rc);

		POINT	pnt = { 0, coord };
		ScreenToClient(m_hwnd, &pnt);
		if ((pnt.y + 2 >= MIN_PANELHEIGHT + 2) && (pnt.y + 2 < 100) && (pnt.y + 2 < rc.bottom - 30))
			m_pPanel.setHeight(pnt.y + 2, true);

		RedrawWindow(m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		if (M.isAero())
			InvalidateRect(GetParent(m_hwnd), nullptr, FALSE);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::StreamEvents(MEVENT hDbEventFirst, int count, bool bAppend)
{
	m_pLog->LogEvents(hDbEventFirst, count, bAppend);

	DM_ScrollToBottom(0, 0);
	if (bAppend && hDbEventFirst)
		m_hDbEventLast = hDbEventFirst;
	else
		m_hDbEventLast = db_event_last(m_hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// sent by the select container dialog box when a container was selected...

void CMsgDialog::SwitchToContainer(const wchar_t *szNewName)
{
	if (!mir_wstrcmp(szNewName, TranslateT("Default container")))
		szNewName = CGlobals::m_default_container_name;

	int iOldItems = TabCtrl_GetItemCount(m_hwndParent);
	if (!wcsncmp(m_pContainer->m_wszName, szNewName, CONTAINER_NAMELEN))
		return;

	TContainerData *pNewContainer = FindContainerByName(szNewName);
	if (pNewContainer == nullptr)
		if ((pNewContainer = CreateContainer(szNewName, FALSE, m_hContact)) == nullptr)
			return;

	db_set_ws(m_hContact, SRMSGMOD_T, "containerW", szNewName);
	PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_DOCREATETAB, (WPARAM)pNewContainer, m_hContact);
	if (iOldItems > 1)                // there were more than 1 tab, container is still valid
		SendMessage(m_pContainer->m_hwndActive, WM_SIZE, 0, 0);
	SetForegroundWindow(pNewContainer->m_hwnd);
	SetActiveWindow(pNewContainer->m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMsgDialog::TabAutoComplete()
{
	LRESULT lResult = m_message.SendMsg(EM_GETSEL, 0, 0);
	int start = LOWORD(lResult), end = HIWORD(lResult);
	int origStart = start, origEnd = end;
	m_message.SendMsg(EM_SETSEL, end, end);

	GETTEXTEX gt = { 0 };
	gt.codepage = 1200;
	gt.flags = GTL_DEFAULT | GTL_PRECISE;
	int iLen = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)& gt, 0);
	if (iLen <= 0)
		return false;

	bool isTopic = false, isRoom = false;
	wchar_t *pszText = (wchar_t *)mir_calloc((iLen + 10) * sizeof(wchar_t));

	gt.flags = GT_DEFAULT;
	gt.cb = (iLen + 9) * sizeof(wchar_t);
	m_message.SendMsg(EM_GETTEXTEX, (WPARAM)& gt, (LPARAM)pszText);

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
		isRoom = true;
	else {
		int topicStart = start;
		while (topicStart > 0 && (pszText[topicStart - 1] == ' ' || pszText[topicStart - 1] == 13 || pszText[topicStart - 1] == VK_TAB))
			topicStart--;
		if (topicStart > 5 && wcsstr(&pszText[topicStart - 6], L"/topic") == &pszText[topicStart - 6])
			isTopic = true;
	}
	
	if (m_wszSearchQuery == nullptr) {
		m_wszSearchQuery = mir_wstrndup(pszText + start, end - start);
		m_wszSearchResult = mir_wstrdup(m_wszSearchQuery);
		m_pLastSession = nullptr;
	}

	const wchar_t *pszName = nullptr;
	if (isTopic)
		pszName = m_si->ptszTopic;
	else if (isRoom) {
		m_pLastSession = SM_FindSessionAutoComplete(m_si->pszModule, m_si, m_pLastSession, m_wszSearchQuery, m_wszSearchResult);
		if (m_pLastSession != nullptr)
			pszName = m_pLastSession->ptszName;
	}
	else pszName = g_chatApi.UM_FindUserAutoComplete(m_si, m_wszSearchQuery, m_wszSearchResult);

	replaceStrW(m_wszSearchResult, nullptr);

	if (pszName != nullptr) {
		if (end != start) {
			CMStringW szReplace;
			if (!isRoom && !isTopic && start == 0) {
				szReplace = pszName;
				if (mir_wstrlen(g_Settings.pwszAutoText))
					szReplace.Append(g_Settings.pwszAutoText);
				szReplace.AppendChar(' ');
				m_wszSearchResult = szReplace.Detach();
				pszName = m_wszSearchResult;
			}
			else m_wszSearchResult = mir_wstrdup(pszName);

			m_message.SendMsg(EM_SETSEL, start, end);
			m_message.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)pszName);
		}
		else m_wszSearchResult = mir_wstrdup(pszName);

		return true;
	}

	if (end != start) {
		m_message.SendMsg(EM_SETSEL, start, end);
		m_message.SendMsg(EM_REPLACESEL, TRUE, (LPARAM)m_wszSearchQuery);
	}
	m_message.SendMsg(EM_SETSEL, origStart, origEnd);
	replaceStrW(m_wszSearchQuery, nullptr);
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::tabClearLog()
{
	if (isChat()) {
		g_chatApi.LM_RemoveAll(&m_si->pLog, &m_si->pLogEnd);
		m_si->iEventCount = 0;
		m_si->LastTime = 0;
		PostMessage(m_hwnd, WM_MOUSEACTIVATE, 0, 0);
	}

	m_pLog->Clear();
	m_hDbEventFirst = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

CThumbBase *CMsgDialog::tabCreateThumb(CProxyWindow *pProxy) const
{
	if (isChat())
		return new CThumbMUC(pProxy, m_si);

	return new CThumbIM(pProxy);
}

/////////////////////////////////////////////////////////////////////////////////////////
// update all status bar fields and force a redraw of the status bar.

void CMsgDialog::tabUpdateStatusBar() const
{
	if (m_pContainer->m_hwndStatus && m_pContainer->m_hwndActive == m_hwnd) {
		if (!isChat()) {
			if (m_wszStatusBar[0]) {
				SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING]);
				SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)m_wszStatusBar);
			}
			else if (m_bStatusSet) {
				SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, (LPARAM)m_szStatusIcon);
				SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)m_szStatusText.c_str());
			}
			else {
				SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, 0);
				DM_UpdateLastMessage();
			}
		}
		else {
			if (m_bStatusSet) {
				SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, (LPARAM)m_szStatusIcon);
				SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 0, (LPARAM)m_szStatusText.c_str());
			}
			else SendMessage(m_pContainer->m_hwndStatus, SB_SETICON, 0, 0);
		}
		UpdateReadChars();
		InvalidateRect(m_pContainer->m_hwndStatus, nullptr, TRUE);
		SendMessage(m_pContainer->m_hwndStatus, WM_USER + 101, 0, (LPARAM)this);
	}
}

int CMsgDialog::Typing(int secs)
{
	if (!AllowTyping())
		return 0;

	int preTyping = m_nTypeSecs != 0;

	setTyping(m_nTypeSecs = (secs > 0) ? secs : 0);
	if (m_nTypeSecs)
		m_bShowTyping = 0;

	return preTyping;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::UpdateNickList()
{
	int i = m_nickList.SendMsg(LB_GETTOPINDEX, 0, 0);
	m_nickList.SendMsg(LB_SETCOUNT, m_si->getUserList().getCount(), 0);
	m_nickList.SendMsg(LB_SETTOPINDEX, i, 0);
	UpdateTitle();
	m_hTabIcon = m_hTabStatusIcon;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::UpdateOptions()
{
	GetSendFormat();

	DM_InitRichEdit();
	m_btnOk.SendMsg(BUTTONSETASNORMAL, TRUE, 0);

	m_nickList.SetItemHeight(0, g_Settings.iNickListFontHeight);
	InvalidateRect(m_nickList.GetHwnd(), nullptr, TRUE);

	m_btnFilter.SendMsg(BUTTONSETOVERLAYICON, (LPARAM)(m_bFilterEnabled ? PluginConfig.g_iconOverlayEnabled : PluginConfig.g_iconOverlayDisabled), 0);

	CSuper::UpdateOptions();
}

/////////////////////////////////////////////////////////////////////////////////////////
// update the status bar field which displays the number of characters in the input area
// and various indicators (caps lock, num lock, insert mode).

void CMsgDialog::UpdateReadChars() const
{
	if (!m_pContainer->m_hwndStatus || m_pContainer->m_hwndActive != m_hwnd)
		return;

	int len;
	if (isChat())
		len = GetWindowTextLength(m_message.GetHwnd());
	else {
		// retrieve text length in UTF8 bytes, because this is the relevant length for most protocols
		GETTEXTLENGTHEX gtxl = { 0 };
		gtxl.codepage = CP_UTF8;
		gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMBYTES;

		len = m_message.SendMsg(EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
	}

	BOOL fCaps = (GetKeyState(VK_CAPITAL) & 1);
	BOOL fNum = (GetKeyState(VK_NUMLOCK) & 1);

	wchar_t szBuf[20]; szBuf[0] = 0;
	if (m_bInsertMode)
		mir_wstrcat(szBuf, L"O");
	if (fCaps)
		mir_wstrcat(szBuf, L"C");
	if (fNum)
		mir_wstrcat(szBuf, L"N");
	if (m_bInsertMode || fCaps || fNum)
		mir_wstrcat(szBuf, L" | ");

	wchar_t buf[128];
	mir_snwprintf(buf, L"%s%s %d/%d", szBuf, m_lcID, m_iOpenJobs, len);
	SendMessage(m_pContainer->m_hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
	if (PluginConfig.m_visualMessageSizeIndicator)
		InvalidateRect(m_pContainer->m_hwndStatus, nullptr, FALSE);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::UpdateSaveAndSendButton()
{
	GETTEXTLENGTHEX gtxl = { 0 };
	gtxl.codepage = CP_UTF8;
	gtxl.flags = GTL_DEFAULT | GTL_PRECISE | GTL_NUMBYTES;

	int len = SendDlgItemMessage(m_hwnd, IDC_SRMM_MESSAGE, EM_GETTEXTLENGTHEX, (WPARAM)&gtxl, 0);
	if (len && GetSendButtonState() == PBS_DISABLED)
		EnableSendButton(true);
	else if (len == 0 && GetSendButtonState() != PBS_DISABLED)
		EnableSendButton(false);

	if (len) {          // looks complex but avoids flickering on the button while typing.
		if (!m_bSaveBtn) {
			SendDlgItemMessage(m_hwnd, IDC_CLOSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_BUTTON_SAVE]);
			SendDlgItemMessage(m_hwnd, IDC_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Save and close session"), BATF_UNICODE);
			m_bSaveBtn = true;
		}
	}
	else {
		SendDlgItemMessage(m_hwnd, IDC_CLOSE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)PluginConfig.g_buttonBarIcons[ICON_BUTTON_CANCEL]);
		SendDlgItemMessage(m_hwnd, IDC_CLOSE, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Close session"), BATF_UNICODE);
		m_bSaveBtn = false;
	}
	m_textLen = len;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
		uint32_t diff = (now - mi->idleTimeStamp) / 60;
		if (diff >= 1) {
			if (diff > 59) {
				uint32_t hours = diff / 60;
				uint32_t minutes = diff % 60;
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

void CMsgDialog::UpdateTitle()
{
	if (isChat()) {
		m_wStatus = m_si->wStatus;

		const wchar_t *szNick = m_cache->getNick();
		if (mir_wstrlen(szNick) > 0) {
			if (M.GetByte("cuttitle", 0))
				CutContactName(szNick, m_wszTitle, _countof(m_wszTitle));
			else
				wcsncpy_s(m_wszTitle, szNick, _TRUNCATE);
		}

		CMStringW wszTitle;
		HICON hIcon = nullptr;
		int nUsers = m_si->getUserList().getCount();

		switch (m_si->iType) {
		case GCW_CHATROOM:
			hIcon = Skin_LoadProtoIcon(m_si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
			wszTitle.Format((nUsers == 1) ? TranslateT("%s: chat room (%u user%s)") : TranslateT("%s: chat room (%u users%s)"),
				szNick, nUsers, m_bFilterEnabled ? TranslateT(", event filter active") : L"");
			break;

		case GCW_PRIVMESS:
			hIcon = Skin_LoadProtoIcon(m_si->pszModule, (m_wStatus <= ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : m_wStatus);
			if (nUsers == 1)
				wszTitle.Format(TranslateT("%s: message session"), szNick);
			else
				wszTitle.Format(TranslateT("%s: message session (%u users)"), szNick, nUsers);
			break;

		case GCW_SERVER:
			wszTitle.Format(L"%s: Server", szNick);
			hIcon = LoadIconEx("window");
			break;

		default:
			return;
		}

		if (m_pWnd) {
			m_pWnd->updateTitle(m_wszTitle);
			m_pWnd->updateIcon(hIcon);
		}
		m_hTabStatusIcon = hIcon;

		if (m_cache->getStatus() != m_cache->getOldStatus()) {
			wcsncpy_s(m_wszStatus, Clist_GetStatusModeDescription(m_wStatus, 0), _TRUNCATE);

			TCITEM item = {};
			item.mask = TCIF_TEXT;
			item.pszText = m_wszTitle;
			TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
		}
		SetWindowText(m_hwnd, wszTitle);
		if (m_pContainer->m_hwndActive == m_hwnd) {
			m_pContainer->UpdateTitle(0, this);
			UpdateStatusBar();
		}
	}
	else {
		uint32_t dwOldIdle = m_idle;
		const char *szActProto = nullptr;

		m_wszStatus[0] = 0;

		if (m_iTabID == -1)
			return;

		TCITEM item = {};

		bool bChanged = false;
		wchar_t newtitle[128];
		if (m_szProto) {
			szActProto = m_cache->getProto();

			bool bHasName = (m_cache->getUIN()[0] != 0);
			m_idle = m_cache->getIdleTS();
			m_bIsIdle = m_idle != 0;

			m_wStatus = m_cache->getStatus();
			wcsncpy_s(m_wszStatus, Clist_GetStatusModeDescription(m_szProto == nullptr ? ID_STATUS_OFFLINE : m_wStatus, 0), _TRUNCATE);

			wchar_t newcontactname[128]; newcontactname[0] = 0;
			if (PluginConfig.m_bCutContactNameOnTabs)
				CutContactName(m_cache->getNick(), newcontactname, _countof(newcontactname));
			else
				wcsncpy_s(newcontactname, m_cache->getNick(), _TRUNCATE);

			Utils::DoubleAmpersands(newcontactname, _countof(newcontactname));

			if (newcontactname[0] != 0) {
				if (PluginConfig.m_bStatusOnTabs)
					mir_snwprintf(newtitle, L"%s (%s)", newcontactname, m_wszStatus);
				else
					wcsncpy_s(newtitle, newcontactname, _TRUNCATE);
			}
			else wcsncpy_s(newtitle, L"Forward", _TRUNCATE);

			if (mir_wstrcmp(newtitle, m_wszTitle))
				bChanged = true;
			else if (m_wStatus != m_wOldStatus)
				bChanged = true;

			UpdateWindowIcon();

			wchar_t fulluin[256];
			if (m_bIsMeta)
				mir_snwprintf(fulluin,
				TranslateT("UID: %s (Shift+click -> copy to clipboard)\nClick for user's details\nRight click for metacontact control\nClick dropdown to add or remove user from your favorites."),
				bHasName ? m_cache->getUIN() : TranslateT("No UID"));
			else
				mir_snwprintf(fulluin,
				TranslateT("UID: %s (Shift+click -> copy to clipboard)\nClick for user's details\nClick dropdown to change this contact's favorite status."),
				bHasName ? m_cache->getUIN() : TranslateT("No UID"));

			SendDlgItemMessage(m_hwnd, IDC_NAME, BUTTONADDTOOLTIP, (WPARAM)fulluin, BATF_UNICODE);
		}
		else wcsncpy_s(newtitle, L"Message Session", _TRUNCATE);

		m_wOldStatus = m_wStatus;
		if (m_idle != dwOldIdle || bChanged) {
			if (bChanged) {
				item.mask |= TCIF_TEXT;
				item.pszText = m_wszTitle;
				wcsncpy_s(m_wszTitle, newtitle, _TRUNCATE);
				if (m_pWnd)
					m_pWnd->updateTitle(m_cache->getNick());
			}
			if (m_iTabID >= 0) {
				TabCtrl_SetItem(m_hwndParent, m_iTabID, &item);
				if (m_pContainer->cfg.flags.m_bSideBar)
					m_pContainer->m_pSideBar->updateSession(this);
			}
			if (m_pContainer->m_hwndActive == m_hwnd && bChanged)
				m_pContainer->UpdateTitle(m_hContact);

			m_pPanel.Invalidate();
			if (m_pWnd)
				m_pWnd->Invalidate();
		}

		// care about MetaContacts and update the statusbar icon with the currently "most online" contact...
		if (m_bIsMeta) {
			PostMessage(m_hwnd, DM_OWNNICKCHANGED, 0, 0);
			if (m_pContainer->cfg.flags.m_bUinStatusBar)
				DM_UpdateLastMessage();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void CMsgDialog::UpdateWindowIcon()
{
	if (m_hXStatusIcon) {
		DestroyIcon(m_hXStatusIcon);
		m_hXStatusIcon = nullptr;
	}

	if (LPCSTR szProto = m_cache->getProto()) {
		m_hTabIcon = m_hTabStatusIcon = GetMyContactIcon("MetaiconTab");
		if (M.GetByte("use_xicons", 1))
			m_hXStatusIcon = GetXStatusIcon();

		SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BUTTONSETASDIMMED, m_bIsIdle, 0);
		SendDlgItemMessage(m_hwnd, IDC_PROTOCOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_hXStatusIcon ? m_hXStatusIcon : GetMyContactIcon("MetaiconBar")));

		if (m_pContainer->m_hwndActive == m_hwnd)
			m_pContainer->SetIcon(this, m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon);

		if (m_pWnd)
			m_pWnd->updateIcon(m_hXStatusIcon ? m_hXStatusIcon : m_hTabIcon);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// called whenever a group chat tab becomes active(either by switching tabs or activating a
// container window

void CMsgDialog::UpdateWindowState(UINT msg)
{
	if (m_iTabID < 0)
		return;

	if (msg == WM_ACTIVATE) {
		if (m_pContainer->cfg.flags.m_bTransparent) {
			uint32_t trans = LOWORD(m_pContainer->cfg.dwTransparency);
			SetLayeredWindowAttributes(m_pContainer->m_hwnd, CSkin::m_ContainerColorKey, (uint8_t)trans, (m_pContainer->cfg.flags.m_bTransparent ? LWA_ALPHA : 0));
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
	m_dwTickLastEvent = 0;
	m_bDividerSet = false;

	if (m_pContainer->m_dwFlashingStarted != 0) {
		m_pContainer->FlashContainer(0, 0);
		m_pContainer->m_dwFlashingStarted = 0;
	}

	if (m_si) {
		g_chatApi.SetActiveSession(m_si);
		m_hTabIcon = m_hTabStatusIcon;

		if (db_get_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0) != 0)
			db_set_w(m_si->hContact, m_si->pszModule, "ApparentMode", 0);
		if (g_clistApi.pfnGetEvent(m_si->hContact, 0))
			g_clistApi.pfnRemoveEvent(m_si->hContact, GC_FAKE_EVENT);

		UpdateTitle();
		m_hTabIcon = m_hTabStatusIcon;
		if (timerFlash.Stop() || m_iFlashIcon) {
			FlashTab(false);
			m_bCanFlashTab = FALSE;
			m_iFlashIcon = nullptr;
		}

		m_pContainer->cfg.flags.m_bNeedsUpdateTitle = false;

		if (m_bNeedCheckSize)
			PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

		SetFocus(m_message.GetHwnd());
		m_dwLastActivity = GetTickCount();
		m_pContainer->m_dwLastActivity = m_dwLastActivity;
		m_pContainer->m_pMenuBar->configureMenu();
	}
	else {
		if (timerFlash.Stop()) {
			FlashTab(false);
			m_bCanFlashTab = false;
		}

		if (m_bFlashClist) {
			m_bFlashClist = false;
			if (m_hFlashingEvent != 0)
				g_clistApi.pfnRemoveEvent(m_hContact, m_hFlashingEvent);
			m_hFlashingEvent = 0;
		}
		m_pContainer->cfg.flags.m_bNeedsUpdateTitle = false;

		if (m_bDeferredRemakeLog && !IsIconic(m_pContainer->m_hwnd)) {
			RemakeLog();
			m_bDeferredRemakeLog = false;
		}

		if (m_bNeedCheckSize)
			PostMessage(m_hwnd, DM_SAVESIZE, 0, 0);

		m_pContainer->m_hIconTaskbarOverlay = nullptr;
		m_pContainer->UpdateTitle(m_hContact);

		tabUpdateStatusBar();
		m_dwLastActivity = GetTickCount();
		m_pContainer->m_dwLastActivity = m_dwLastActivity;

		m_pContainer->m_pMenuBar->configureMenu();
		g_arUnreadWindows.remove(HANDLE(m_hContact));

		m_pPanel.Invalidate();

		if (m_bDeferredScroll) {
			m_bDeferredScroll = false;
			DM_ScrollToBottom(0, 1);
		}
	}

	DM_SetDBButtonStates();

	if (m_bDelayedSplitter) {
		m_bDelayedSplitter = false;
		ShowWindow(m_pContainer->m_hwnd, SW_RESTORE);
		PostMessage(m_hwnd, DM_SPLITTERGLOBALEVENT, m_wParam, m_lParam);
		PostMessage(m_hwnd, WM_SIZE, 0, 0);
		m_wParam = m_lParam = 0;
	}

	BB_SetButtonsPos();
	if (M.isAero())
		InvalidateRect(m_hwndParent, nullptr, FALSE);

	if (m_pContainer->cfg.flags.m_bSideBar)
		m_pContainer->m_pSideBar->setActiveItem(this, msg == WM_ACTIVATE);

	if (m_pWnd)
		m_pWnd->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////////////////
// generic handler for the WM_COPY message in message log/chat history richedit control(s).
// it filters out the invisible event boundary markers from the text copied to the clipboard.
// WINE Fix: overwrite clippboad data from original control data

LRESULT CMsgDialog::WMCopyHandler(UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = mir_callNextSubclass(m_pLog->GetHwnd(), stubLogProc, msg, wParam, lParam);

	ptrA szFromStream(LOG()->GetRichTextRtf(true, true));
	if (szFromStream != nullptr) {
		ptrW converted(mir_utf8decodeW(szFromStream));
		if (converted != nullptr) {
			Utils::FilterEventMarkers(converted);
			Utils::CopyToClipBoard(converted, m_pLog->GetHwnd());
		}
	}

	return result;
}
