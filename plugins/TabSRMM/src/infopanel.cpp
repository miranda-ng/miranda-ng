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
// the info area for both im and chat sessions

#include "stdafx.h"

wchar_t *xStatusDescr[] =
{
	L"Angry", L"Duck", L"Tired", L"Party", L"Beer", L"Thinking", L"Eating",
	L"TV", L"Friends", L"Coffee", L"Music", L"Business", L"Camera", L"Funny",
	L"Phone", L"Games", L"College", L"Shopping", L"Sick", L"Sleeping",
	L"Surfing", L"@Internet", L"Engineering", L"Typing", L"Eating... yummy",
	L"Having fun", L"Chit chatting", L"Crashing", L"Going to toilet", L"<undef>",
	L"<undef>", L"<undef>"
};

TInfoPanelConfig CInfoPanel::m_ipConfig = {};

/////////////////////////////////////////////////////////////////////////////////////////

CInfoPanel::CInfoPanel(CMsgDialog *dat)
{
	if (dat) {
		m_dat = dat;
		m_isChat = dat->isChat();
	}
	m_defaultHeight = PluginConfig.m_panelHeight;
	m_defaultMUCHeight = PluginConfig.m_MUCpanelHeight;
	m_hwndConfig = nullptr;
	m_hoverFlags = 0;
	m_tip = nullptr;
}

CInfoPanel::~CInfoPanel()
{
	if (m_hwndConfig)
		::DestroyWindow(m_hwndConfig);
	saveHeight(true);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CInfoPanel::setActive(const int newActive)
{
	m_active = newActive ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load height. Private panel height is indicated by 0xffff for the high word

void CInfoPanel::loadHeight()
{
	uint8_t bSync = M.GetByte("syncAllPanels", 0);			// sync muc <> im panels

	m_height = M.GetDword(m_dat->m_hContact, "panelheight", -1);

	if (m_height == -1 || HIWORD(m_height) == 0) {
		if (m_dat->m_pContainer->cfg.fPrivate)
			m_height = m_dat->m_pContainer->cfg.panelheight;
		else
			m_height = bSync ? m_defaultHeight : (m_isChat ? m_defaultMUCHeight : m_defaultHeight);
		m_fPrivateHeight = false;
	}
	else {
		m_fPrivateHeight = true;
		m_height &= 0x0000ffff;
	}

	if (m_height <= 0 || m_height > 120)				// ensure, corrupted values don't stand a chance
		m_height = DEGRADE_THRESHOLD;					// standard height for 2 lines
}

/////////////////////////////////////////////////////////////////////////////////////////
// Save current panel height to the database
//
// @param fFlush bool: flush values to database (usually only requested by destructor)

void CInfoPanel::saveHeight(bool fFlush)
{
	uint8_t bSync = M.GetByte("syncAllPanels", 0);

	if (m_height < 110 && m_height >= MIN_PANELHEIGHT) {          // only save valid panel splitter positions
		if (!m_fPrivateHeight) {
			if (!m_isChat || bSync) {
				if (m_dat->m_pContainer->cfg.fPrivate)
					m_dat->m_pContainer->cfg.panelheight = m_height;
				else {
					PluginConfig.m_panelHeight = m_height;
					m_defaultHeight = m_height;
					if (fFlush)
						db_set_dw(0, SRMSGMOD_T, "panelheight", m_height);
				}
			}
			else if (m_isChat && !bSync) {
				if (m_dat->m_pContainer->cfg.fPrivate)
					m_dat->m_pContainer->cfg.panelheight = m_height;
				else {
					PluginConfig.m_MUCpanelHeight = m_height;
					m_defaultMUCHeight = m_height;
					if (fFlush)
						db_set_dw(0, CHAT_MODULE, "panelheight", m_height);
				}
			}
		}
		else db_set_dw(m_dat->m_hContact, SRMSGMOD_T, "panelheight", MAKELONG(m_height, 0xffff));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Sets the new height of the panel and broadcasts it to all
// open sessions
//
// @param newHeight  LONG: the new height.
// @param fBroadcast bool: broadcast the new height to all open sessions, respect
//        container's private setting flag.

void CInfoPanel::setHeight(LONG newHeight, bool fBroadcast)
{
	if (newHeight < MIN_PANELHEIGHT || newHeight > 100)
		return;

	m_height = newHeight;

	if (fBroadcast) {
		if (!m_fPrivateHeight) {
			if (!m_dat->m_pContainer->cfg.fPrivate)
				Srmm_Broadcast(DM_SETINFOPANEL, (WPARAM)m_dat, (LPARAM)newHeight);
			else
				m_dat->m_pContainer->BroadCastContainer(DM_SETINFOPANEL, (WPARAM)m_dat, (LPARAM)newHeight);
		}
		saveHeight();
	}
}

void CInfoPanel::Configure() const
{
	Utils::showDlgControl(m_dat->GetHwnd(), IDC_PANELSPLITTER, m_active ? SW_SHOW : SW_HIDE);
}

void CInfoPanel::showHide() const
{
	HBITMAP hbm = (m_active && m_dat->m_pContainer->cfg.avatarMode != 3) ? m_dat->m_hOwnPic : (m_dat->m_ace ? m_dat->m_ace->hbmPic : PluginConfig.g_hbmUnknown);
	HWND hwndDlg = m_dat->GetHwnd();

	if (!m_isChat) {
		::ShowWindow(m_dat->m_hwndPanelPicParent, (m_active) ? SW_SHOW : SW_HIDE);

		if (!m_active && m_dat->m_hwndPanelPic) {
			::DestroyWindow(m_dat->m_hwndPanelPic);
			m_dat->m_hwndPanelPic = nullptr;
		}

		m_dat->m_iRealAvatarHeight = 0;
		m_dat->AdjustBottomAvatarDisplay();

		BITMAP bm;
		::GetObject(hbm, sizeof(bm), &bm);
		m_dat->CalcDynamicAvatarSize(&bm);

		if (m_active) {
			if (m_dat->m_hwndContactPic) {
				::DestroyWindow(m_dat->m_hwndContactPic);
				m_dat->m_hwndContactPic = nullptr;
			}
			m_dat->GetAvatarVisibility();
			Configure();
			InvalidateRect(hwndDlg, nullptr, FALSE);
		}
		Utils::showDlgControl(hwndDlg, IDC_PANELSPLITTER, m_active ? SW_SHOW : SW_HIDE);
		::SendMessage(hwndDlg, WM_SIZE, 0, 0);
		::InvalidateRect(GetDlgItem(hwndDlg, IDC_CONTACTPIC), nullptr, TRUE);
	}
	else {
		Utils::showDlgControl(hwndDlg, IDC_PANELSPLITTER, m_active ? SW_SHOW : SW_HIDE);

		if (m_active) {
			Configure();
			::InvalidateRect(hwndDlg, nullptr, FALSE);
		}

		::SendMessage(hwndDlg, WM_SIZE, 0, 0);
	}
	m_dat->m_pContainer->SetAeroMargins();
	if (M.isAero())
		::InvalidateRect(GetParent(hwndDlg), nullptr, FALSE);
	m_dat->DM_ScrollToBottom(0, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Decide if info panel must be visible for this session. Uses container setting and,
// if applicable, local (per contact) override.
//
// @return bool: panel is visible for this session

bool CInfoPanel::getVisibility()
{
	uint8_t bDefault = (m_dat->m_pContainer->cfg.flags.m_bInfoPanel) ? 1 : 0;
	uint8_t bContact = M.GetByte(m_dat->m_hContact, "infopanel", 0);

	uint8_t visible = (bContact == 0 ? bDefault : (bContact == (uint8_t)-1 ? 0 : 1));
	setActive(visible);
	return m_active;
}

void CInfoPanel::mapRealRect(const RECT& rcSrc, RECT& rcDest, const SIZE& sz)
{
	rcDest.left = rcSrc.left;
	rcDest.right = rcDest.left + sz.cx;
	rcDest.top = rcSrc.top + (((rcSrc.bottom - rcSrc.top) - sz.cy) / 2);
	rcDest.bottom = rcDest.top + sz.cy;
}

void CInfoPanel::mapRealRectOnTop(const RECT& rcSrc, RECT& rcDest, const SIZE& sz)
{
	rcDest.left = rcSrc.left;
	rcDest.right = rcDest.left + sz.cx;
	rcDest.top = rcSrc.top;
	rcDest.bottom = rcDest.top + sz.cy;
}

/////////////////////////////////////////////////////////////////////////////////////////
// create an underlined version of the original font and select it
// in the given device context
//
// returns the previosuly selected font
//
// caller should not forget to delete the font!

HFONT CInfoPanel::setUnderlinedFont(const HDC hdc, HFONT hFontOrig)
{
	LOGFONT lf;
	::GetObject(hFontOrig, sizeof(lf), &lf);
	lf.lfUnderline = 1;

	HFONT hFontNew = ::CreateFontIndirect(&lf);
	return (HFONT)::SelectObject(hdc, hFontNew);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Render the info panel background.
//
// @param hdc	 HDC: target device context
// @param rc     RECT&: target rectangle
// @param item   CSkinItem *: The item to render in non-aero mode
// @param bAero  bool: aero active

void CInfoPanel::renderBG(const HDC hdc, RECT& rc, CSkinItem *item, bool bAero, bool fAutoCalc) const
{
	if (!m_active)
		return;

	if (fAutoCalc)
		rc.bottom = m_height + 1;
	if (bAero) {
		RECT rcBlack = rc;
		rc.bottom -= 2;
		::FillRect(hdc, &rc, CSkin::m_BrushBack);
		CSkin::ApplyAeroEffect(hdc, &rc, CSkin::AERO_EFFECT_AREA_INFOPANEL);
		rcBlack.top = rc.bottom;// + 1;
		rcBlack.bottom = rcBlack.top + 2;
		if (CSkin::m_pCurrentAeroEffect && CSkin::m_pCurrentAeroEffect->m_clrBack != 0)
			::DrawAlpha(hdc, &rcBlack, CSkin::m_pCurrentAeroEffect->m_clrBack, 90, CSkin::m_pCurrentAeroEffect->m_clrBack, 0, 0, 0, 1, nullptr);
		return;
	}

	if (CSkin::m_skinEnabled) {
		rc.bottom -= 2;
		CSkin::SkinDrawBG(m_dat->GetHwnd(), m_dat->m_pContainer->m_hwnd, m_dat->m_pContainer, &rc, hdc);
		item = &SkinItems[ID_EXTBKINFOPANELBG];

		// if new (= tabsrmm 3.x) skin item is not defined, use the old info panel
		// field background items. That should break less skins
		if (!item->IGNORED)
			CSkin::DrawItem(hdc, &rc, item);
		return;
	}

	rc.bottom -= 2;
	::DrawAlpha(hdc, &rc, PluginConfig.m_ipBackgroundGradient, 100, PluginConfig.m_ipBackgroundGradientHigh, 0, 17, 0, 0, nullptr);
	if (fAutoCalc) {
		rc.top = rc.bottom - 1;
		rc.left--; rc.right++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// render the content of the info panel. The target area is derived from the
// precalculated RECT structures in _MessageWindowData (calculated in the
// message window's WM_SIZE handler).
//
// @param hdc HDC: target device context

void CInfoPanel::renderContent(const HDC hdc)
{
	if (!m_active)
		return;

	if (!m_isChat) { // panel picture
		DRAWITEMSTRUCT dis = { 0 };
		dis.rcItem = m_dat->m_rcPic;
		dis.hDC = hdc;
		dis.hwndItem = m_dat->GetHwnd();
		if (m_dat->MsgWindowDrawHandler(&dis) == 0) {
			::PostMessage(m_dat->GetHwnd(), WM_SIZE, 0, 1);
			::PostMessage(m_dat->GetHwnd(), DM_FORCEREDRAW, 0, 0);
		}

		RECT rc = m_dat->m_rcNick;
		if (m_height >= DEGRADE_THRESHOLD)
			rc.top -= 2;

		RenderIPNickname(hdc, rc);
		if (m_height >= DEGRADE_THRESHOLD) {
			rc = m_dat->m_rcUIN;
			RenderIPUIN(hdc, rc);
		}
		rc = m_dat->m_rcStatus;
		RenderIPStatus(hdc, rc);
	}
	else {
		RECT rc = m_dat->m_rcNick;

		if (m_height >= DEGRADE_THRESHOLD) {
			rc.top -= 2;
			rc.bottom -= 2;
		}

		Chat_RenderIPNickname(hdc, rc);
		if (m_height >= DEGRADE_THRESHOLD) {
			rc = m_dat->m_rcUIN;
			Chat_RenderIPSecondLine(hdc, rc);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Render the nickname in the info panel.
// This will also show the status message (if one is available)
// The field will dynamically adjust itself to the available info panel space. If
// the info panel is too small to show both nick and UIN fields, this field will show
// the UIN _instead_ of the nickname (most people have the nickname in the title
// bar anyway).
//
// @param hdc    HDC: target DC for drawing
//
// @param rcItem RECT &: target rectangle

void CInfoPanel::RenderIPNickname(const HDC hdc, RECT &rcItem)
{
	const wchar_t *szStatusMsg = nullptr;
	const wchar_t *szTextToShow = nullptr;
	bool fShowUin = false;
	COLORREF clr = 0;

	if (m_height < DEGRADE_THRESHOLD) {
		szTextToShow = m_dat->m_cache->getUIN();
		fShowUin = true;
	}
	else szTextToShow = m_dat->m_cache->getNick();

	szStatusMsg = m_dat->m_cache->getStatusMsg();

	::SetBkMode(hdc, TRANSPARENT);

	rcItem.left += 2;
	if (szTextToShow[0]) {
		HICON xIcon = m_dat->GetXStatusIcon();
		if (xIcon) {
			::DrawIconEx(hdc, rcItem.left, (rcItem.bottom + rcItem.top - PluginConfig.m_smcyicon) / 2, xIcon, PluginConfig.m_smcxicon, PluginConfig.m_smcyicon, 0, nullptr, DI_NORMAL | DI_COMPAT);
			::DestroyIcon(xIcon);
			rcItem.left += 21;
		}

		HFONT hOldFont;
		if (fShowUin) {
			hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));
			clr = m_ipConfig.clrs[IPFONTID_UIN];
		}
		else {
			hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_NICK]));
			clr = m_ipConfig.clrs[IPFONTID_NICK];
		}

		m_szNick.cx = m_szNick.cy = 0;

		if (szStatusMsg) {
			SIZE sStatusMsg, sMask;
			::GetTextExtentPoint32(hdc, szTextToShow, (int)mir_wstrlen(szTextToShow), &m_szNick);
			::GetTextExtentPoint32(hdc, L"A", 1, &sMask);
			::GetTextExtentPoint32(hdc, szStatusMsg, (int)mir_wstrlen(szStatusMsg), &sStatusMsg);

			uint32_t dtFlagsNick = DT_SINGLELINE | DT_WORD_ELLIPSIS | DT_NOPREFIX;
			if ((m_szNick.cx + sStatusMsg.cx + 6) < (rcItem.right - rcItem.left) || (rcItem.bottom - rcItem.top) < (2 * sMask.cy)) {
				dtFlagsNick |= DT_VCENTER;
				mapRealRect(rcItem, m_rcNick, m_szNick);
			}
			else mapRealRectOnTop(rcItem, m_rcNick, m_szNick);

			if (m_hoverFlags & HOVER_NICK)
				setUnderlinedFont(hdc, fShowUin ? m_ipConfig.hFonts[IPFONTID_UIN] : m_ipConfig.hFonts[IPFONTID_NICK]);

			CSkin::RenderText(hdc, m_dat->m_hThemeIP, szTextToShow, &rcItem, dtFlagsNick, CSkin::m_glowSize, clr);

			HFONT hFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]));
			if (m_hoverFlags & HOVER_NICK)
				::DeleteObject(hFont);

			clr = m_ipConfig.clrs[IPFONTID_STATUS];

			rcItem.left += (m_szNick.cx + 10);

			uint32_t dtFlags;
			if (!(dtFlagsNick & DT_VCENTER))
				dtFlags = DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX;
			else
				dtFlags = DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER;

			rcItem.right -= 3;
			if (rcItem.left + 30 < rcItem.right)
				CSkin::RenderText(hdc, m_dat->m_hThemeIP, szStatusMsg, &rcItem, dtFlags, CSkin::m_glowSize, clr);
		}
		else {
			GetTextExtentPoint32(hdc, szTextToShow, (int)mir_wstrlen(szTextToShow), &m_szNick);
			mapRealRect(rcItem, m_rcNick, m_szNick);
			if (m_hoverFlags & HOVER_NICK)
				setUnderlinedFont(hdc, fShowUin ? m_ipConfig.hFonts[IPFONTID_UIN] : m_ipConfig.hFonts[IPFONTID_NICK]);
			CSkin::RenderText(hdc, m_dat->m_hThemeIP, szTextToShow, &rcItem, DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_NOPREFIX, CSkin::m_glowSize, clr);
			if (m_hoverFlags & HOVER_NICK)
				::DeleteObject(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));
		}
		if (hOldFont)
			::SelectObject(hdc, hOldFont);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Draws the UIN field for the info panel.
//
// @param hdc    HDC: device context for drawing.
// @param rcItem RECT &: target rectangle for drawing

void CInfoPanel::RenderIPUIN(const HDC hdc, RECT& rcItem)
{
	::SetBkMode(hdc, TRANSPARENT);

	rcItem.left += 2;

	COLORREF	clr = m_ipConfig.clrs[IPFONTID_UIN];
	HFONT hOldFont;
	if (m_hoverFlags & HOVER_UIN)
		hOldFont = setUnderlinedFont(hdc, m_ipConfig.hFonts[IPFONTID_UIN]);
	else
		hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));

	const wchar_t *tszUin = m_dat->m_cache->getUIN();
	if (tszUin[0]) {
		CMStringW str;

		if (m_dat->m_idle) {
			time_t diff = time(0) - m_dat->m_idle;
			int i_hrs = diff / 3600;
			int i_mins = (diff - i_hrs * 3600) / 60;
			str.Format(TranslateT("%s    Idle: %dh,%02dm"), tszUin, i_hrs, i_mins);
		}
		else str = tszUin;

		if (g_plugin.bShowDesc) {
			ptrW szVersion(db_get_wsa(m_dat->m_cache->getActiveContact(), m_dat->m_cache->getActiveProto(), "MirVer"));
			if (szVersion)
				str.AppendFormat(TranslateT("  Client: %s"), szVersion.get());
		}

		SIZE sUIN;
		::GetTextExtentPoint32(hdc, str, str.GetLength(), &sUIN);
		mapRealRect(rcItem, m_rcUIN, sUIN);
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, str, &rcItem, DT_SINGLELINE | DT_VCENTER, CSkin::m_glowSize, clr);
	}
	if (m_hoverFlags & HOVER_UIN)
		::DeleteObject(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));

	if (hOldFont)
		::SelectObject(hdc, hOldFont);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Render the info panel status field. Usually in the 2nd line, right aligned
// @param hdc    : target device context

void CInfoPanel::RenderIPStatus(const HDC hdc, RECT& rcItem)
{
	SIZE sProto = { 0 }, sStatus = { 0 }, sTime = { 0 };
	uint32_t oldPanelStatusCX = m_dat->m_panelStatusCX;

	if (m_dat->m_wszStatus[0])
		GetTextExtentPoint32(hdc, m_dat->m_wszStatus, (int)mir_wstrlen(m_dat->m_wszStatus), &sStatus);

	// figure out final account name
	const wchar_t *szFinalProto = m_dat->m_cache->getRealAccount();
	if (szFinalProto) {
		SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_PROTO]);
		GetTextExtentPoint32(hdc, szFinalProto, (int)mir_wstrlen(szFinalProto), &sProto);
	}

	wchar_t szResult[80]; szResult[0] = 0;
	if (m_dat->m_hTimeZone) {
		TimeZone_PrintDateTime(m_dat->m_hTimeZone, L"t", szResult, _countof(szResult), 0);
		GetTextExtentPoint32(hdc, szResult, (int)mir_wstrlen(szResult), &sTime);
	}

	m_dat->m_panelStatusCX = 3 + sStatus.cx + sProto.cx + 14 + (m_dat->m_hClientIcon ? 20 : 0) + sTime.cx + 13;

	if (m_dat->m_panelStatusCX != oldPanelStatusCX) {
		SendMessage(m_dat->GetHwnd(), WM_SIZE, 0, 0);
		rcItem = m_dat->m_rcStatus;
	}

	SetBkMode(hdc, TRANSPARENT);
	RECT rc = rcItem;
	rc.left += 2;
	rc.right -= 3;

	if (szResult[0]) {
		::DrawIconEx(hdc, rcItem.left, (rcItem.bottom - rcItem.top) / 2 - 8 + rcItem.top, PluginConfig.g_iconClock, 16, 16, 0, nullptr, DI_NORMAL);

		HFONT oldFont = (HFONT)SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_TIME]);

		rcItem.left += 16;
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, szResult, &rcItem, DT_SINGLELINE | DT_VCENTER, CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_TIME]);
		SelectObject(hdc, oldFont);
		rc.left += (sTime.cx + 20);
	}

	HFONT hOldFont = (HFONT)SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]);

	if (m_dat->m_wszStatus[0]) {
		SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]);
		mapRealRect(rc, m_rcStatus, sStatus);
		if (m_hoverFlags & HOVER_STATUS)
			setUnderlinedFont(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]);
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, m_dat->m_wszStatus, &rc, DT_SINGLELINE | DT_VCENTER, CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_STATUS]);
		if (m_hoverFlags & HOVER_STATUS)
			::DeleteObject(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]));
	}
	if (szFinalProto) {
		rc.left = rc.right - sProto.cx - 3 - (m_dat->m_hClientIcon ? 20 : 0);
		SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_PROTO]);
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, szFinalProto, &rc, DT_SINGLELINE | DT_VCENTER, CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_PROTO]);
	}

	if (m_dat->m_hClientIcon)
		DrawIconEx(hdc, rc.right - 19, (rc.bottom + rc.top - 16) / 2, m_dat->m_hClientIcon, 16, 16, 0, nullptr, DI_NORMAL);

	if (hOldFont)
		SelectObject(hdc, hOldFont);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Draws the Nickname field (first line) in a MUC window
//
// @param hdc    HDC: device context for drawing.
// @param rcItem RECT &: target rectangle for drawing

void CInfoPanel::Chat_RenderIPNickname(const HDC hdc, RECT& rcItem)
{
	SESSION_INFO *si = m_dat->m_si;
	if (si == nullptr)
		return;

	::SetBkMode(hdc, TRANSPARENT);
	m_szNick.cx = m_szNick.cy = 0;

	HFONT hOldFont;

	if (m_height < DEGRADE_THRESHOLD) {
		wchar_t	tszText[2048];
		mir_snwprintf(tszText, TranslateT("Topic is: %s"),
			si->ptszTopic ? si->ptszTopic : TranslateT("no topic set."));

		hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, tszText, &rcItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER,
			CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_UIN]);
	}
	else {
		const wchar_t *tszNick = m_dat->m_cache->getNick();

		hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_NICK]));
		::GetTextExtentPoint32(hdc, tszNick, (int)mir_wstrlen(tszNick), &m_szNick);
		mapRealRect(rcItem, m_rcNick, m_szNick);

		if (m_hoverFlags & HOVER_NICK)
			setUnderlinedFont(hdc, m_ipConfig.hFonts[IPFONTID_NICK]);

		CSkin::RenderText(hdc, m_dat->m_hThemeIP, tszNick, &rcItem, DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER,
			CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_NICK]);

		if (m_hoverFlags & HOVER_NICK)
			::DeleteObject(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_NICK]));

		rcItem.left += (m_szNick.cx + 4);

		::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_STATUS]);
		if (si->ptszStatusbarText) {
			wchar_t *pTmp = wcschr(si->ptszStatusbarText, ']');
			pTmp += 2;

			if (si->ptszStatusbarText[0] == '[' && pTmp > si->ptszStatusbarText) {
				CMStringW tszTemp(si->ptszStatusbarText, pTmp - si->ptszStatusbarText);
				CSkin::RenderText(hdc, m_dat->m_hThemeIP, tszTemp, &rcItem, DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER,
					CSkin::m_glowSize, m_ipConfig.clrs[IPFONTID_STATUS]);
			}
		}
	}
	if (hOldFont)
		::SelectObject(hdc, hOldFont);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Draw 2nd line of text in the info panel.
// @param hdc	 : target device context
// @param rcItem : target rectangle

void CInfoPanel::Chat_RenderIPSecondLine(const HDC hdc, RECT& rcItem)
{
	SESSION_INFO *si = reinterpret_cast<SESSION_INFO*>(m_dat->m_si);
	if (si == nullptr)
		return;

	HFONT hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));
	COLORREF clr = m_ipConfig.clrs[IPFONTID_UIN];

	SIZE szTitle;
	wchar_t szPrefix[100];
	mir_snwprintf(szPrefix, TranslateT("Topic is: %s"), L"");
	::GetTextExtentPoint32(hdc, szPrefix, (int)mir_wstrlen(szPrefix), &szTitle);
	mapRealRect(rcItem, m_rcUIN, szTitle);
	if (m_hoverFlags & HOVER_UIN)
		setUnderlinedFont(hdc, m_ipConfig.hFonts[IPFONTID_UIN]);
	rcItem.right -= 3;
	CSkin::RenderText(hdc, m_dat->m_hThemeIP, szPrefix, &rcItem, DT_SINGLELINE | DT_NOPREFIX | DT_TOP, CSkin::m_glowSize, clr);
	rcItem.left += (szTitle.cx + 4);
	if (m_hoverFlags & HOVER_UIN)
		::DeleteObject(::SelectObject(hdc, m_ipConfig.hFonts[IPFONTID_UIN]));
	if (si->ptszTopic && mir_wstrlen(si->ptszTopic) > 1)
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, si->ptszTopic, &rcItem, DT_WORDBREAK | DT_END_ELLIPSIS | DT_NOPREFIX | DT_TOP, CSkin::m_glowSize, clr);
	else
		CSkin::RenderText(hdc, m_dat->m_hThemeIP, TranslateT("no topic set."), &rcItem, DT_TOP | DT_SINGLELINE | DT_NOPREFIX, CSkin::m_glowSize, clr);

	if (hOldFont)
		::SelectObject(hdc, hOldFont);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invalidate the info panel rectangle

void CInfoPanel::Invalidate(BOOL fErase) const
{
	if (m_active) {
		RECT rc;
		::GetClientRect(m_dat->GetHwnd(), &rc);
		rc.bottom = m_height;
		::InvalidateRect(m_dat->GetHwnd(), &rc, fErase);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// build the left click contextual menu for the info panel
// @return HMENU: menu handle for the fully prepared menu

HMENU CInfoPanel::constructContextualMenu() const
{
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_DATA | MIIM_ID | MIIM_BITMAP | MIIM_STRING;
	mii.hbmpItem = HBMMENU_CALLBACK;

	if (!(m_hoverFlags & HOVER_NICK))
		return nullptr;

	HMENU m = ::CreatePopupMenu();

	if (m_hoverFlags & HOVER_NICK) {
		Utils::addMenuItem(m, mii, ::Skin_LoadIcon(SKINICON_OTHER_USERDETAILS), TranslateT("Open user details..."), IDC_NAME, 0);
		Utils::addMenuItem(m, mii, ::Skin_LoadIcon(SKINICON_OTHER_HISTORY), TranslateT("Open history..."), IDC_SRMM_HISTORY, 0);
		if (!m_isChat)
			Utils::addMenuItem(m, mii, PluginConfig.g_iconContainer, TranslateT("Messaging settings..."), ID_MESSAGELOGSETTINGS_FORTHISCONTACT, 1);
		else {
			::AppendMenu(m, MF_STRING, IDC_SRMM_CHANMGR, TranslateT("Room settings..."));
			if (GCW_SERVER & m_dat->m_si->iType)
				::EnableMenuItem(m, IDC_SRMM_CHANMGR, MF_GRAYED);
		}
		::AppendMenu(m, MF_SEPARATOR, 1000, nullptr);
		Utils::addMenuItem(m, mii, PluginConfig.g_buttonBarIcons[6], TranslateT("Close session"), IDC_CLOSE, 4);
	}
	::AppendMenu(m, MF_SEPARATOR, 1000, nullptr);
	::AppendMenu(m, MF_STRING, CMD_IP_COPY, TranslateT("Copy to clipboard"));

	return m;
}

/////////////////////////////////////////////////////////////////////////////////////////
// process internal menu commands from info panel fields
// if this does not handle the selected command, Utils::CmdDispatcher() will be called
// to chain the command through message window command handlers.
// 
// @param cmd		command id
// @return			0 if command was processed, != 0 otherwise

LRESULT CInfoPanel::cmdHandler(UINT cmd)
{
	switch (cmd) {
	case CMD_IP_COPY:
		if (m_hoverFlags & HOVER_NICK) {
			Utils_ClipboardCopy(m_dat->m_cache->getNick());
			return(S_OK);
		}
		if (m_hoverFlags & HOVER_UIN) {
			Utils_ClipboardCopy(m_isChat ? m_dat->m_si->ptszTopic : m_dat->m_cache->getUIN());
			return(S_OK);
		}
		break;

	case IDC_SRMM_HISTORY:
	case IDC_SRMM_CHANMGR:
		if (m_isChat) {
			SendMessage(m_dat->GetHwnd(), WM_COMMAND, cmd, 0);
			return(S_OK);
		}
		break;
	}
	return S_FALSE;				// not handled
}

/////////////////////////////////////////////////////////////////////////////////////////
// handle mouse clicks on the info panel.
//
// @param pt: mouse cursor pos

void CInfoPanel::handleClick(const POINT& pt)
{
	if (!m_active || m_hoverFlags == 0)
		return;

	if (!m_isChat) {
		m_dat->timerAwayMsg.Stop();
		m_dat->m_bAwayMsgTimer = false;
	}
	HMENU m = constructContextualMenu();
	if (m) {
		LRESULT r = ::TrackPopupMenu(m, TPM_RETURNCMD, pt.x, pt.y, 0, m_dat->GetHwnd(), nullptr);

		::DestroyMenu(m);
		if (S_OK != cmdHandler(r))
			if (m_dat->MsgWindowMenuHandler(r, MENU_LOGMENU) == 0)
				m_dat->DM_MsgWindowCmdHandler(r, 0, 0);
	}
	m_hoverFlags = 0;
	Invalidate(TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// peforms a hit test on the given position. returns 0, if cursor is NOT
// inside any of the 3 relevant hovering areas.
//
// @param pt	POINT (in screen coordinates)
// @return		Hit test result or 0 if none applies.

int CInfoPanel::hitTest(POINT pt)
{
	::ScreenToClient(m_dat->GetHwnd(), &pt);

	if (!m_isChat && ::PtInRect(&m_rcStatus, pt))
		return(HTSTATUS);
	else if (::PtInRect(&m_rcNick, pt))
		return(HTNICK);
	else if (::PtInRect(&m_rcUIN, pt))
		return(HTUIN);

	return(HTNIRVANA);
}

/////////////////////////////////////////////////////////////////////////////////////////
// track mouse movements inside the panel. Needed for tooltip activation
// and to hover the info panel fields.
// 
// @param pt : mouse coordinates (screen)

void CInfoPanel::trackMouse(POINT &pt)
{
	if (!m_active)
		return;

	int result = hitTest(pt);

	uint32_t dwOldHovering = m_hoverFlags;
	m_hoverFlags = 0;

	switch (result) {
	case HTSTATUS:
		m_hoverFlags |= HOVER_STATUS;
		::SetCursor(LoadCursor(nullptr, IDC_HAND));
		break;

	case HTNICK:
		m_hoverFlags |= HOVER_NICK;
		::SetCursor(LoadCursor(nullptr, IDC_HAND));
		break;

	case HTUIN:
		::SetCursor(LoadCursor(nullptr, IDC_HAND));
		m_hoverFlags |= HOVER_UIN;
		break;
	}

	if (m_hoverFlags) {
		if (!m_dat->m_bAwayMsgTimer) {
			m_dat->timerAwayMsg.Start(1000);
			m_dat->m_bAwayMsgTimer = true;
		}
	}
	if (dwOldHovering != m_hoverFlags)
		Invalidate(TRUE);
	if (m_hoverFlags == 0)
		m_dat->m_bAwayMsgTimer = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// activate a tooltip
// @param ctrlId : control id
// @param lParam : typically a wchar_t * for the tooltip text

void CInfoPanel::showTip(UINT ctrlId, const wchar_t *pwszTip)
{
	if (!m_active || !m_dat->m_hwndTip)
		return;

	HWND hwndDlg = m_dat->GetHwnd();
	{
		RECT rc;
		::GetWindowRect(GetDlgItem(hwndDlg, ctrlId), &rc);
		::SendMessage(m_dat->m_hwndTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(rc.left, rc.bottom));
	}
	if (pwszTip)
		m_dat->ti.lpszText = (wchar_t*)pwszTip;
	else {
		if (m_hwndConfig)
			return;

		CMStringW str(FORMAT, RTF_DEFAULT_HEADER, 0, 0, 0, 30 * 15);

		str.AppendFormat(TranslateT("\\ul\\b Status message:\\ul0\\b0 \\par %s"),
			m_dat->m_cache->getStatusMsg() ? m_dat->m_cache->getStatusMsg() : TranslateT("No status message"));

		DBVARIANT dbv = { 0 };
		if (uint8_t xStatus = m_dat->m_cache->getXStatusId()) {
			wchar_t	*tszXStatusName = nullptr;
			if (0 == db_get_ws(m_dat->m_cache->getContact(), m_dat->m_cache->getProto(), "XStatusName", &dbv))
				tszXStatusName = dbv.pwszVal;
			else if (xStatus > 0 && xStatus <= 31)
				tszXStatusName = xStatusDescr[xStatus - 1];

			if (tszXStatusName) {
				str.Append(TranslateT("\\par\\par\\ul\\b Extended status information:\\ul0\\b0 \\par "));
				str.AppendFormat(L"%s%s%s", tszXStatusName, m_dat->m_cache->getXStatusMsg() ? L" / " : L"",
					m_dat->m_cache->getXStatusMsg() ? m_dat->m_cache->getXStatusMsg() : L"");

				if (dbv.pwszVal)
					mir_free(dbv.pwszVal);
			}
		}

		if (m_dat->m_cache->getListeningInfo())
			str.AppendFormat(TranslateT("\\par\\par\\ul\\b Listening to:\\ul0\\b0 \\par %s"), m_dat->m_cache->getListeningInfo());

		if (0 == db_get_ws(m_dat->m_cache->getActiveContact(), m_dat->m_cache->getActiveProto(), "MirVer", &dbv)) {
			str.AppendFormat(TranslateT("\\par\\par\\ul\\b Client:\\ul0\\b0  %s"), dbv.pwszVal);
			::db_free(&dbv);
		}
		str.AppendChar('}');

		// convert line breaks to rtf
		str.Replace(L"\n", L"\\line ");

		POINT pt;
		RECT rc = { 0, 0, 400, 600 };
		GetCursorPos(&pt);
		m_tip = new CTip(m_dat->GetHwnd(), m_dat->m_hContact, str, this);
		m_tip->show(rc, pt, m_dat->m_hTabIcon, m_dat->m_wszStatus);
		return;
	}

	::SendMessage(m_dat->m_hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&m_dat->ti);
	::SendMessage(m_dat->m_hwndTip, TTM_SETMAXTIPWIDTH, 0, 350);

	::SendMessage(m_dat->m_hwndTip, TTM_SETTITLE, 1, (LPARAM)TranslateT("TabSRMM information"));
	::SendMessage(m_dat->m_hwndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_dat->ti);
	::GetCursorPos(&m_dat->m_ptTipActivation);
}

/////////////////////////////////////////////////////////////////////////////////////////
// hide a tooltip (if it was created)
// this is only used from outside (i.e. container window dialog)
// 
// hwndNew = window to become active (as reported by WM_ACTIVATE).

void CInfoPanel::hideTip(const HWND hwndNew)
{
	if (m_tip) {
		if (hwndNew == m_tip->getHwnd())
			return;
		if (::IsWindow(m_tip->getHwnd()))
			::DestroyWindow(m_tip->getHwnd());
		m_tip = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// draw the background (and border) of the parent control that holds the avs-based avatar display
// (ACC window class). Only required when support for animated avatars is enabled because
// native avatar rendering does not support animated images.
// To avoid clipping issues, this is done during WM_ERASEBKGND.

LRESULT CALLBACK CInfoPanel::avatarParentSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NOTIFY:
		ProcessAvatarChange(hwnd, lParam);
		break;

	case WM_ERASEBKGND:
		// parent window of the infopanel ACC control
		RECT rc, rcItem;
		CMsgDialog *dat = (CMsgDialog*)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
		if (dat == nullptr)
			break;

		GetClientRect(hwnd, &rcItem);
		rc = rcItem;
		if (!IsWindowEnabled(hwnd) || !dat->m_pPanel.isActive() || !dat->m_bShowInfoAvatar)
			return TRUE;

		HDC dcWin = (HDC)wParam;

		if (M.isAero()) {
			LONG cx = rcItem.right - rcItem.left;
			LONG cy = rcItem.bottom - rcItem.top;

			rc.left -= 3; rc.right += 3;
			rc.bottom += 2;

			HDC hdc = CreateCompatibleDC(dcWin);
			HBITMAP hbm = CSkin::CreateAeroCompatibleBitmap(rc, dcWin);
			HBITMAP hbmOld = (HBITMAP)SelectObject(hdc, hbm);

			if (CSkin::m_pCurrentAeroEffect == nullptr)
				FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
			else {
				if (CSkin::m_pCurrentAeroEffect->m_finalAlpha == 0)
					CSkin::ApplyAeroEffect(hdc, &rc, CSkin::AERO_EFFECT_AREA_INFOPANEL);
				else {
					FillRect(hdc, &rc, CSkin::m_BrushBack);
					CSkin::ApplyAeroEffect(hdc, &rc, CSkin::AERO_EFFECT_AREA_INFOPANEL);
				}
			}
			BitBlt(dcWin, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
			SelectObject(hdc, hbmOld);
			DeleteObject(hbm);
			DeleteDC(hdc);
		}
		else {
			rc.bottom += 2;
			rc.left -= 3; rc.right += 3;
			dat->m_pPanel.renderBG(dcWin, rc, &SkinItems[ID_EXTBKINFOPANELBG], M.isAero(), false);
		}

		if (CSkin::m_bAvatarBorderType == 1) {
			RECT rcPic;
			GetClientRect(dat->m_hwndPanelPic, &rcPic);
			LONG ix = ((rcItem.right - rcItem.left) - rcPic.right) / 2 - 1;
			LONG iy = ((rcItem.bottom - rcItem.top) - rcPic.bottom) / 2 - 1;

			HRGN clipRgn = CreateRectRgn(ix, iy, ix + rcPic.right + 2, iy + rcPic.bottom + 2);

			HBRUSH hbr = CreateSolidBrush(CSkin::m_avatarBorderClr);
			FrameRgn(dcWin, clipRgn, hbr, 1, 1);
			DeleteObject(hbr);
			DeleteObject(clipRgn);
		}
		return TRUE;
	}

	return mir_callNextSubclass(hwnd, CInfoPanel::avatarParentSubclass, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Stub for the dialog procedure. Just handles INITDIALOG and sets
// our userdata. Real processing is done by ConfigDlgProc()
//
// @params Like a normal dialog procedure

INT_PTR CALLBACK CInfoPanel::ConfigDlgProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CInfoPanel *infoPanel = reinterpret_cast<CInfoPanel *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (infoPanel)
		return infoPanel->ConfigDlgProc(hwnd, msg, wParam, lParam);

	switch (msg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		infoPanel = reinterpret_cast<CInfoPanel *>(lParam);
		return infoPanel->ConfigDlgProc(hwnd, msg, wParam, lParam);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// dialog procedure for the info panel config popup

INT_PTR CALLBACK CInfoPanel::ConfigDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			wchar_t	tszTitle[100];
			mir_snwprintf(tszTitle, TranslateT("Set panel visibility for this %s"),
				m_isChat ? TranslateT("chat room") : TranslateT("contact"));
			::SetDlgItemText(hwnd, IDC_STATIC_VISIBILTY, tszTitle);

			mir_snwprintf(tszTitle, m_isChat ? TranslateT("Do not synchronize the panel height with IM windows") :
				TranslateT("Do not synchronize the panel height with group chat windows"));

			::SetDlgItemText(hwnd, IDC_NOSYNC, tszTitle);

			::SendDlgItemMessage(hwnd, IDC_PANELVISIBILITY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Inherit from container setting"));
			::SendDlgItemMessage(hwnd, IDC_PANELVISIBILITY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always off"));
			::SendDlgItemMessage(hwnd, IDC_PANELVISIBILITY, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Always on"));

			uint8_t v = M.GetByte(m_dat->m_hContact, "infopanel", 0);
			::SendDlgItemMessage(hwnd, IDC_PANELVISIBILITY, CB_SETCURSEL, (WPARAM)(v == 0 ? 0 : (v == (uint8_t)-1 ? 1 : 2)), 0);

			::SendDlgItemMessage(hwnd, IDC_PANELSIZE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use default size"));
			::SendDlgItemMessage(hwnd, IDC_PANELSIZE, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use private size"));

			::SendDlgItemMessage(hwnd, IDC_PANELSIZE, CB_SETCURSEL, (WPARAM)(m_fPrivateHeight ? 1 : 0), 0);

			::CheckDlgButton(hwnd, IDC_NOSYNC, M.GetByte("syncAllPanels", 0) ? BST_UNCHECKED : BST_CHECKED);

			Utils::showDlgControl(hwnd, IDC_IPCONFIG_PRIVATECONTAINER, m_dat->m_pContainer->cfg.fPrivate ? SW_SHOW : SW_HIDE);

			if (!m_isChat) {
				v = db_get_b(m_dat->m_hContact, SRMSGMOD_T, "hideavatar", -1);
				::SendDlgItemMessage(hwnd, IDC_PANELPICTUREVIS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Use global setting"));
				::SendDlgItemMessage(hwnd, IDC_PANELPICTUREVIS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Show always (if present)"));
				::SendDlgItemMessage(hwnd, IDC_PANELPICTUREVIS, CB_INSERTSTRING, -1, (LPARAM)TranslateT("Never show it at all"));
				::SendDlgItemMessage(hwnd, IDC_PANELPICTUREVIS, CB_SETCURSEL, (v == (uint8_t)-1 ? 0 : (v == 1 ? 1 : 2)), 0);
			}
			else Utils::enableDlgControl(hwnd, IDC_PANELPICTUREVIS, false);
		}
		return FALSE;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		{
			HWND hwndChild = (HWND)lParam;
			UINT id = ::GetDlgCtrlID(hwndChild);

			if (m_configDlgFont == nullptr) {
				HFONT hFont = (HFONT)::SendDlgItemMessage(hwnd, IDC_IPCONFIG_TITLE, WM_GETFONT, 0, 0);
				LOGFONT lf = { 0 };

				::GetObject(hFont, sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				m_configDlgBoldFont = ::CreateFontIndirect(&lf);

				lf.lfHeight = (int)(lf.lfHeight * 1.2);
				m_configDlgFont = ::CreateFontIndirect(&lf);
				::SendDlgItemMessage(hwnd, IDC_IPCONFIG_TITLE, WM_SETFONT, (WPARAM)m_configDlgFont, FALSE);
			}

			if (hwndChild == ::GetDlgItem(hwnd, IDC_IPCONFIG_TITLE)) {
				::SetTextColor((HDC)wParam, RGB(60, 60, 150));
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_configDlgFont, FALSE);
			}
			else if (id == IDC_IPCONFIG_FOOTER || id == IDC_SIZE_TIP || id == IDC_IPCONFIG_PRIVATECONTAINER)
				::SetTextColor((HDC)wParam, RGB(160, 50, 50));
			else if (id == IDC_GROUP_SIZE || id == IDC_GROUP_SCOPE || id == IDC_GROUP_OTHER)
				::SendMessage(hwndChild, WM_SETFONT, (WPARAM)m_configDlgBoldFont, FALSE);

			::SetBkColor((HDC)wParam, ::GetSysColor(COLOR_WINDOW));
		}
		return reinterpret_cast<INT_PTR>(::GetSysColorBrush(COLOR_WINDOW));

	case WM_COMMAND:
		{
			LONG	lOldHeight = m_height;
			switch (LOWORD(wParam)) {
			case IDC_PANELSIZE:
				{
					LRESULT iResult = ::SendDlgItemMessage(hwnd, IDC_PANELSIZE, CB_GETCURSEL, 0, 0);
					if (iResult == 0) {
						if (m_fPrivateHeight) {
							db_set_dw(m_dat->m_hContact, SRMSGMOD_T, "panelheight", m_height);
							loadHeight();
						}
					}
					else if (iResult == 1) {
						db_set_dw(m_dat->m_hContact, SRMSGMOD_T, "panelheight",
							MAKELONG(M.GetDword(m_dat->m_hContact, "panelheight", m_height), 0xffff));
						loadHeight();
					}
				}
				break;

			case IDC_PANELPICTUREVIS:
				{
					uint8_t	vOld = db_get_b(m_dat->m_hContact, SRMSGMOD_T, "hideavatar", -1);
					LRESULT iResult = ::SendDlgItemMessage(hwnd, IDC_PANELPICTUREVIS, CB_GETCURSEL, 0, 0);

					uint8_t vNew = (iResult == 0 ? (uint8_t)-1 : (iResult == 1 ? 1 : 0));
					if (vNew != vOld) {
						if (vNew == (uint8_t)-1)
							db_unset(m_dat->m_hContact, SRMSGMOD_T, "hideavatar");
						else
							db_set_b(m_dat->m_hContact, SRMSGMOD_T, "hideavatar", vNew);

						m_dat->ShowPicture(false);
						::SendMessage(m_dat->GetHwnd(), WM_SIZE, 0, 0);
						m_dat->DM_ScrollToBottom(0, 1);
					}
				}
				break;

			case IDC_PANELVISIBILITY:
				{
					uint8_t	vOld = db_get_b(m_dat->m_hContact, SRMSGMOD_T, "infopanel", 0);
					LRESULT iResult = ::SendDlgItemMessage(hwnd, IDC_PANELVISIBILITY, CB_GETCURSEL, 0, 0);

					uint8_t vNew = (iResult == 0 ? 0 : (iResult == 1 ? (uint8_t)-1 : 1));
					if (vNew != vOld) {
						db_set_b(m_dat->m_hContact, SRMSGMOD_T, "infopanel", vNew);
						getVisibility();
						showHide();
					}
				}
				break;

			case IDC_SIZECOMPACT:
				setHeight(MIN_PANELHEIGHT + 2, true);
				break;

			case IDC_SIZENORMAL:
				setHeight(DEGRADE_THRESHOLD, true);
				break;

			case IDC_SIZELARGE:
				setHeight(51, true);
				break;

			case IDC_NOSYNC:
				db_set_b(0, SRMSGMOD_T, "syncAllPanels", ::IsDlgButtonChecked(hwnd, IDC_NOSYNC) ? 0 : 1);
				if (BST_UNCHECKED == IsDlgButtonChecked(hwnd, IDC_NOSYNC)) {
					loadHeight();
					if (!m_dat->m_pContainer->cfg.fPrivate)
						Srmm_Broadcast(DM_SETINFOPANEL, (WPARAM)m_dat, (LPARAM)m_defaultHeight);
					else
						m_dat->m_pContainer->BroadCastContainer(DM_SETINFOPANEL, (WPARAM)m_dat, (LPARAM)m_defaultHeight);
				}
				else {
					if (!m_dat->m_pContainer->cfg.fPrivate)
						Srmm_Broadcast(DM_SETINFOPANEL, (WPARAM)m_dat, 0);
					else
						m_dat->m_pContainer->BroadCastContainer(DM_SETINFOPANEL, (WPARAM)m_dat, 0);
				}
				break;
			}

			if (m_height != lOldHeight) {
				::SendMessage(m_dat->GetHwnd(), WM_SIZE, 0, 0);
				m_dat->m_pContainer->SetAeroMargins();
				::RedrawWindow(m_dat->GetHwnd(), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
				::RedrawWindow(GetParent(m_dat->GetHwnd()), nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		break;

	case WM_CLOSE:
		if (wParam == 1 && lParam == 1)
			::DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		::DeleteObject(m_configDlgBoldFont);
		::DeleteObject(m_configDlgFont);

		m_configDlgBoldFont = m_configDlgFont = nullptr;
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// invoke info panel config popup dialog
// @param pt : mouse coordinates (screen)
// @return   : always 0

int CInfoPanel::invokeConfigDialog(const POINT &pt)
{
	if (!m_active)
		return 0;

	RECT rc;
	::GetWindowRect(m_dat->GetHwnd(), &rc);
	rc.bottom = rc.top + m_height;
	rc.right -= m_dat->m_iPanelAvatarX;

	POINT	ptTest = pt;
	if (!::PtInRect(&rc, ptTest))
		return 0;

	if (m_hwndConfig == nullptr) {
		m_configDlgBoldFont = m_configDlgFont = nullptr;
		m_hwndConfig = ::CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_INFOPANEL), nullptr /*m_dat->m_pContainer->m_hwnd */,
			ConfigDlgProcStub, (LPARAM)this);
		if (m_hwndConfig) {
			TranslateDialogDefault(m_hwndConfig);

			::GetClientRect(m_hwndConfig, &rc);

			RECT rcLog;
			::GetWindowRect(GetDlgItem(m_dat->GetHwnd(), IDC_SRMM_LOG), &rcLog);

			m_fDialogCreated = true;
			::SetWindowPos(m_hwndConfig, HWND_TOP, rcLog.left + 10, rcLog.top - (m_active ? 10 : 0), 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
			return 1;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// remove the info panel configuration dialog
// @param fForced: bool, if true, dismiss it under any circumstances, even
// with the pointer still inside the dialog.

void CInfoPanel::dismissConfig(bool fForced)
{
	if (m_hwndConfig == nullptr)
		return;

	if (!m_fDialogCreated) {
		POINT pt;
		::GetCursorPos(&pt);

		RECT rc;
		::GetWindowRect(m_hwndConfig, &rc);
		if (fForced || !PtInRect(&rc, pt)) {
			SendMessage(m_hwndConfig, WM_CLOSE, 1, 1);
			m_hwndConfig = nullptr;
		}
	}
	m_fDialogCreated = false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// construct a richedit tooltip object.
//
// @param hwndParent HWND owner (used only for position calculation)
// @param hContact HANDLE contact handle
// @param pszText wchar_t* the content of the rich edit control
// @param panel CInfoPanel* the panel which owns it

CTip::CTip(const HWND hwndParent, const MCONTACT hContact, const wchar_t *pszText, const CInfoPanel* panel)
{
	m_hwnd = ::CreateWindowEx(WS_EX_TOOLWINDOW, L"RichEditTipClass", L"", (M.isAero() ? WS_THICKFRAME : WS_BORDER) | WS_POPUPWINDOW | WS_TABSTOP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, 40, 40, nullptr, nullptr, g_plugin.getInst(), this);

	m_hRich = ::CreateWindowEx(0, L"RICHEDIT50W", L"", WS_CHILD | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_READONLY | WS_VSCROLL | WS_TABSTOP,
		0, 0, 40, 40, m_hwnd, reinterpret_cast<HMENU>(1000), g_plugin.getInst(), nullptr);

	::SendMessage(m_hRich, EM_AUTOURLDETECT, TRUE, 0);
	::SendMessage(m_hRich, EM_SETEVENTMASK, 0, ENM_LINK);
	::SendMessage(m_hRich, WM_SETFONT, (WPARAM)CInfoPanel::m_ipConfig.hFonts[IPFONTID_STATUS], 0);

	m_hContact = hContact;
	m_pszText = mir_utf8encodeW(pszText);
	m_panel = panel;
	m_hwndParent = hwndParent;
	mir_subclassWindow(m_hRich, RichEditProc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Show the tooltip at the given position (the position can be adjusted to keep it on screen and
// inside its parent window.
//
// it will auto-adjust the size (height only) of the richedit control to fit the m_pszText
//
// @param rc dimensions of the tip (left and top should be 0)
// @param pt point in screen coordinates
// @param hIcon optional icon to display in the tip header
// @param szTitle optional title to display in the tip header

void CTip::show(const RECT& rc, POINT& pt, const HICON hIcon, const wchar_t *szTitle)
{
	HDC hdc = ::GetDC(m_hwnd);

	m_leftWidth = (m_panel->getDat()->m_hClientIcon || m_panel->getDat()->m_hXStatusIcon ? LEFT_BAR_WIDTH : 0);

	int xBorder = M.isAero() ? GetSystemMetrics(SM_CXSIZEFRAME) : 1;
	int yBorder = M.isAero() ? GetSystemMetrics(SM_CYSIZEFRAME) : 1;

	m_hIcon = hIcon;
	m_szTitle = szTitle;

	::SendMessage(m_hRich, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(4, 4));

	SETTEXTEX stx = { ST_SELECTION, CP_UTF8 };
	::SendMessage(m_hRich, EM_SETTEXTEX, (WPARAM)&stx, (LPARAM)m_pszText);

	if (PluginConfig.g_SmileyAddAvail) {
		CContactCache *c = CContactCache::getContactCache(m_hContact);
		::SendMessage(m_hRich, EM_SETBKGNDCOLOR, 0, (LPARAM)PluginConfig.m_ipBackgroundGradientHigh);

		SMADD_RICHEDIT3 smadd = { sizeof(smadd) };
		smadd.hwndRichEditControl = m_hRich;
		smadd.Protocolname = const_cast<char *>(c->getActiveProto());
		smadd.hContact = c->getActiveContact();
		smadd.flags = 0;
		smadd.rangeToReplace = nullptr;
		smadd.disableRedraw = TRUE;
		CallService(MS_SMILEYADD_REPLACESMILEYS, 0, (LPARAM)&smadd);
	}

	RECT rcParent;
	::GetWindowRect(m_hwndParent, &rcParent);
	if (pt.x + rc.right > rcParent.right)
		pt.x = rcParent.right - rc.right - 5;

	m_rcRich = rc;

	m_rcRich.bottom = 800;
	m_rcRich.left = LEFT_BORDER + m_leftWidth; m_rcRich.top = TOP_BORDER;
	m_rcRich.right -= (LEFT_BORDER + RIGHT_BORDER + m_leftWidth);

	int twips = (int)(15.0f / PluginConfig.m_DPIscaleY);
	m_rcRich.right = m_rcRich.left + (twips * (m_rcRich.right - m_rcRich.left)) - 10 * twips;
	m_rcRich.bottom = m_rcRich.top + (twips * (m_rcRich.bottom - m_rcRich.top));

	FORMATRANGE fr = {};
	fr.hdc = hdc;
	fr.hdcTarget = hdc;
	fr.rc = m_rcRich;
	fr.chrg.cpMax = -1;
	::SendMessage(m_hRich, EM_FORMATRANGE, 0, (LPARAM)&fr);

	m_szRich.cx = ((fr.rc.right - fr.rc.left) / twips) + 8;
	m_szRich.cy = ((fr.rc.bottom - fr.rc.top) / twips) + 3;

	m_rcRich.right = m_rcRich.left + m_szRich.cx;
	m_rcRich.bottom = m_rcRich.top + m_szRich.cy;

	::SendMessage(m_hRich, EM_FORMATRANGE, 0, 0);			// required, clear cached painting data in the richedit

	::SetWindowPos(m_hwnd, HWND_TOP, pt.x - 5, pt.y - 5, m_szRich.cx + m_leftWidth + LEFT_BORDER + RIGHT_BORDER + 2 * xBorder,
		m_szRich.cy + TOP_BORDER + BOTTOM_BORDER + 2 * yBorder, SWP_NOACTIVATE | SWP_SHOWWINDOW);

	::SetWindowPos(m_hRich, nullptr, LEFT_BORDER + m_leftWidth, TOP_BORDER, m_szRich.cx, m_szRich.cy, SWP_SHOWWINDOW);

	::ReleaseDC(m_hwnd, hdc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// register richedit tooltip window class

void CTip::registerClass()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = L"RichEditTipClass";
	wc.lpfnWndProc = CTip::WndProcStub;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.cbWndExtra = sizeof(CTip *);
	wc.style = CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	RegisterClassEx(&wc);
}

/////////////////////////////////////////////////////////////////////////////////////////
// subclass the rich edit control inside the tip. Needed to hide the blinking
// caret and prevent all scrolling actions.

LRESULT CALLBACK CTip::RichEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_SETCURSOR:
		::HideCaret(hwnd);
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_NCCALCSIZE:
		SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~WS_VSCROLL);
		EnableScrollBar(hwnd, SB_VERT, ESB_DISABLE_BOTH);
		ShowScrollBar(hwnd, SB_VERT, FALSE);
		break;

	case WM_VSCROLL:
		return 0;
	}
	return ::mir_callNextSubclass(hwnd, CTip::RichEditProc, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// stub for the tip control window procedure. Just handle WM_CREATE and set the
// this pointer.

LRESULT CALLBACK CTip::WndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CTip *tip = reinterpret_cast<CTip *>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (tip)
		return tip->WndProc(hwnd, msg, wParam, lParam);

	switch (msg) {
	case WM_CREATE:
		CREATESTRUCT *cs = reinterpret_cast<CREATESTRUCT *>(lParam);
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// the window procedure for the tooltip window.

INT_PTR CALLBACK CTip::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	POINT	pt;
	RECT	rc;

	switch (msg) {
	case WM_ACTIVATE:
	case WM_SETCURSOR:
		::KillTimer(hwnd, 1000);
		::SetTimer(hwnd, 1000, 200, nullptr);

		if (msg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)
			::DestroyWindow(hwnd);
		break;

		/* prevent resizing */
	case WM_NCHITTEST:
		return(HTCLIENT);
		break;

	case WM_ERASEBKGND:
		wchar_t szTitle[128];
		{
			HDC hdc = (HDC)wParam;
			COLORREF	clr = CInfoPanel::m_ipConfig.clrs[IPFONTID_NICK];
			GetClientRect(hwnd, &rc);
			CContactCache* c = CContactCache::getContactCache(m_hContact);
			RECT rcText = { 0, 0, rc.right, TOP_BORDER };
			LONG cx = rc.right;
			LONG cy = rc.bottom;
			HANDLE hTheme = nullptr;

			mir_snwprintf(szTitle, m_szTitle ? L"%s (%s)" : L"%s%s", c->getNick(), m_szTitle ? m_szTitle : L"");

			if (m_panel) {
				HDC hdcMem = ::CreateCompatibleDC(hdc);
				HBITMAP hbm = ::CSkin::CreateAeroCompatibleBitmap(rc, hdc);
				HBITMAP hbmOld = reinterpret_cast<HBITMAP>(::SelectObject(hdcMem, hbm));
				HFONT hOldFont = reinterpret_cast<HFONT>(::SelectObject(hdcMem, CInfoPanel::m_ipConfig.hFonts[IPFONTID_NICK]));

				::SetBkMode(hdcMem, TRANSPARENT);
				rc.bottom += 2;
				rc.left -= 4; rc.right += 4;
				HBRUSH br = ::CreateSolidBrush(PluginConfig.m_ipBackgroundGradientHigh);
				if (M.isAero()) {
					::FillRect(hdcMem, &rc, reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH)));
					CSkin::ApplyAeroEffect(hdcMem, &rcText, CSkin::AERO_EFFECT_AREA_MENUBAR);
					::FillRect(hdcMem, &m_rcRich, br);

					hTheme = OpenThemeData(m_hwnd, L"BUTTON");
					MARGINS m;
					m.cxLeftWidth = LEFT_BORDER + m_leftWidth;
					m.cxRightWidth = RIGHT_BORDER;
					m.cyBottomHeight = BOTTOM_BORDER;
					m.cyTopHeight = TOP_BORDER;
					CMimAPI::m_pfnDwmExtendFrameIntoClientArea(m_hwnd, &m);
				}
				else {
					::FillRect(hdcMem, &rc, br);
					::DrawAlpha(hdcMem, &rcText, PluginConfig.m_ipBackgroundGradientHigh, 100, PluginConfig.m_ipBackgroundGradient,
						0, GRADIENT_TB + 1, 0, 2, nullptr);
				}
				::DeleteObject(br);
				rcText.left = 20;

				LONG dy = 4;

				if (m_hIcon) {
					::DrawIconEx(hdcMem, 2, dy, m_hIcon, 16, 16, 0, nullptr, DI_NORMAL);
					dy = TOP_BORDER + 4;
				}
				if (m_panel->getDat()->m_hXStatusIcon) {
					::DrawIconEx(hdcMem, 2, dy, m_panel->getDat()->m_hXStatusIcon, 16, 16, 0, nullptr, DI_NORMAL);
					dy += 18;
				}
				if (m_panel->getDat()->m_hClientIcon)
					::DrawIconEx(hdcMem, 2, dy, m_panel->getDat()->m_hClientIcon, 16, 16, 0, nullptr, DI_NORMAL);

				CSkin::RenderText(hdcMem, hTheme, szTitle, &rcText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_VCENTER, CSkin::m_glowSize, clr);
				if (hTheme)
					CloseThemeData(hTheme);
				::SelectObject(hdcMem, hOldFont);
				::BitBlt(hdc, 0, 0, cx, cy, hdcMem, 0, 0, SRCCOPY);
				::SelectObject(hdcMem, hbmOld);
				::DeleteObject(hbm);
				::DeleteDC(hdcMem);
			}
		}
		return TRUE;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code) {
		case EN_LINK:
			::SetFocus(m_hRich);
			switch (((ENLINK*)lParam)->msg) {
			case WM_LBUTTONUP:
				ENLINK *e = reinterpret_cast<ENLINK *>(lParam);
				ptrW tszUrl(Utils::extractURLFromRichEdit(e, m_hRich));
				if (tszUrl)
					Utils_OpenUrlW(tszUrl);

				::DestroyWindow(hwnd);
			}
		}
		break;

	case WM_COMMAND:
		if ((HWND)lParam == m_hRich && HIWORD(wParam) == EN_SETFOCUS)
			::HideCaret(m_hRich);
		break;

	case WM_TIMER:
		if (wParam == 1000) {
			::KillTimer(hwnd, 1000);
			::GetCursorPos(&pt);
			::GetWindowRect(hwnd, &rc);
			if (!PtInRect(&rc, pt))
				::DestroyWindow(hwnd);
			else
				break;
			if (::GetActiveWindow() != hwnd)
				::DestroyWindow(hwnd);
		}
		break;

	case WM_NCDESTROY:
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
		delete this;
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}
