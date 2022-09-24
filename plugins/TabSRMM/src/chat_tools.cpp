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
// Helper functions for the group chat module.

#include "stdafx.h"

char szIndicators[] = { 0, '+', '%', '@', '!', '*' };

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	ShowRoom(nullptr, si);
}

static void __stdcall Chat_DismissPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	if (si->hContact)
		if (g_clistApi.pfnGetEvent(si->hContact, 0))
			g_clistApi.pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

	if (si->pDlg && si->pDlg->timerFlash.Stop())
		FlashWindow(si->pDlg->GetHwnd(), FALSE);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)PUGetPluginData(hWnd);

	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			if (si) {
				if (NEN::ActionLeft & MASK_OPEN)
					CallFunctionAsync(ShowRoomFromPopup, si);
				else
					CallFunctionAsync(Chat_DismissPopup, si);
				PUDeletePopup(hWnd);
			}
			return TRUE;
		}
		break;

	case WM_CONTEXTMENU:
		if (si && si->hContact) {
			if (NEN::ActionRight & MASK_OPEN)
				CallFunctionAsync(ShowRoomFromPopup, si);
			else
				CallFunctionAsync(Chat_DismissPopup, si);
			PUDeletePopup(hWnd);
		}
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->iType;
	if (si && (iEvent & si->iLogTrayFlags))
		return oldDoTrayIcon(si, gce);
	return TRUE;
}

int ShowPopup(MCONTACT hContact, SESSION_INFO *si, HICON hIcon, char* pszProtoName, wchar_t*, COLORREF crBkg, const wchar_t* fmt, ...)
{
	POPUPDATAW pd;
	va_list marker;
	static wchar_t szBuf[4 * 1024];

	if (!fmt || mir_wstrlen(fmt) == 0 || mir_wstrlen(fmt) > 2000)
		return 0;

	va_start(marker, fmt);
	mir_vsnwprintf(szBuf, _countof(szBuf), fmt, marker);
	va_end(marker);

	pd.lchContact = hContact;

	if (hIcon)
		pd.lchIcon = hIcon;
	else
		pd.lchIcon = LoadIconEx("window");

	PROTOACCOUNT *pa = Proto_GetAccount(pszProtoName);
	mir_snwprintf(pd.lpwzContactName, L"%s - %s", (pa == nullptr) ? _A2T(pszProtoName).get() : pa->tszAccountName, Clist_GetContactDisplayName(hContact));
	wcsncpy_s(pd.lpwzText, TranslateW(szBuf), _TRUNCATE);
	pd.iSeconds = g_Settings.iPopupTimeout;

	if (g_Settings.iPopupStyle == 2) {
		pd.colorBack = 0;
		pd.colorText = 0;
	}
	else if (g_Settings.iPopupStyle == 3) {
		pd.colorBack = g_Settings.crPUBkgColour;
		pd.colorText = g_Settings.crPUTextColour;
	}
	else {
		pd.colorBack = g_Settings.crLogBackground;
		pd.colorText = crBkg;
	}

	pd.PluginWindowProc = PopupDlgProc;
	pd.PluginData = si;
	return (INT_PTR)PUAddPopupW(&pd);
}

BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->iType;
	if (si == nullptr || !(iEvent & si->iLogPopupFlags))
		return true;

	CMsgDialog *dat = si->pDlg;
	TContainerData *pContainer = dat ? dat->m_pContainer : nullptr;

	wchar_t *bbStart, *bbEnd;
	if (g_plugin.bBBCodeInPopups) {
		bbStart = L"[b]";
		bbEnd = L"[/b]";
	}
	else bbStart = bbEnd = L"";

	if (!NEN::bMucPopups)                          // no popups at all. Period
		return 0;

	// check the status mode against the status mask
	char *szProto = dat ? dat->m_szProto : si->pszModule;
	if (szProto != nullptr) {
		int dwStatus = Proto_GetStatus(szProto);
		if (!(dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE))
			return 0;
	}

	if (dat && pContainer != nullptr) {                // message window is open, need to check the container config if we want to see a popup nonetheless
		if (NEN::bWindowCheck) {                  // no popups at all for open windows... no exceptions
			if (!g_plugin.bHideOnClose)
				return 0;
			if (pContainer->m_bHidden)
				goto passed;
			return 0;
		}
		if (pContainer->cfg.flags.m_bDontReport && IsIconic(pContainer->m_hwnd))        // in tray counts as "minimised"
			goto passed;
		if (pContainer->cfg.flags.m_bDontReportUnfocused) {
			if (!IsIconic(pContainer->m_hwnd) && !pContainer->IsActive())
				goto passed;
		}
		if (pContainer->cfg.flags.m_bAlwaysReportInactive) {
			if (pContainer->cfg.flags.m_bDontReportFocused)
				goto passed;

			if (pContainer->m_hwndActive == si->pDlg->GetHwnd())
				return 0;

			goto passed;
		}
		return 0;
	}
passed:
	int iNewEvent = iEvent;
	COLORREF clr = 0;

	if ((iNewEvent & GC_EVENT_HIGHLIGHT)) {
		clr = g_chatApi.aFonts[16].color;
		iNewEvent &= ~GC_EVENT_HIGHLIGHT;
	}

	if (iNewEvent == GC_EVENT_MESSAGE) {
		ShowPopup(si->hContact, si, g_chatApi.getIcon(GC_EVENT_MESSAGE), si->pszModule, si->ptszName, clr ? clr : g_chatApi.aFonts[9].color,
			TranslateT("%s%s says:%s %s"), bbStart, gce->pszNick.w, bbEnd, g_chatApi.RemoveFormatting(gce->pszText.w));
	}
	else oldDoPopup(si, gce);

	return TRUE;
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (gce == nullptr || si == nullptr || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	CMsgDialog *dat = si->pDlg;
	int iMuteMode = db_get_b(si->hContact, "SRMM", "MuteMode", CHATMODE_NORMAL);

	int  iEvent = gce->iType;
	bool bInactive = (dat) ? !dat->IsActive() : true;
	bool bActiveTab = false;
	bool bMustFlash = false;
	bool bMustAutoswitch = false;
	bool bFlagUnread = false;

	if (bHighlight) {
		gce->iType |= GC_EVENT_HIGHLIGHT;
		if (Contact::IsHidden(si->hContact) != 0)
			Contact::Hide(si->hContact, false);
		
		if (bInactive) {
			bFlagUnread = true;
			DoTrayIcon(si, gce);
		}

		if (dat || NEN::bMucPopups)
			if (iMuteMode != CHATMODE_MUTE)
				DoPopup(si, gce);

		if (Chat::bFlashWindowHighlight && bInactive)
			bMustFlash = true;

		bMustAutoswitch = true;
		if (g_plugin.bCreateWindowOnHighlight && dat == nullptr) {
			Clist_ContactDoubleClicked(si->hContact);
			bActiveTab = true;
			bInactive = bMustAutoswitch = bMustFlash = false;
		}

		if (dat && g_plugin.bAnnoyingHighlight && bInactive && dat->m_pContainer->m_hwnd != GetForegroundWindow()) {
			bActiveTab = true;
			bInactive = bMustAutoswitch = bMustFlash = false;
			dat->ActivateTab();
		}
	}
	else {
		// do blinking icons in tray
		if (bInactive || !g_Settings.bTrayIconInactiveOnly) {
			DoTrayIcon(si, gce);
			if (iEvent == GC_EVENT_MESSAGE)
				bFlagUnread = true;
		}
		// stupid thing to not create multiple popups for a QUIT event for instance
		if (bManyFix == 0) {
			// do popups
			if (dat || NEN::bMucPopups)
				if (iMuteMode != CHATMODE_MUTE)
					DoPopup(si, gce);

			// do sounds and flashing
			if (iEvent == GC_EVENT_MESSAGE) {
				if (bInactive && !(si->wState & STATE_TALK)) {
					si->wState |= STATE_TALK;
					db_set_w(si->hContact, si->pszModule, "ApparentMode", ID_STATUS_OFFLINE);
				}
			}
		}

		if (iEvent == GC_EVENT_MESSAGE) {
			bMustAutoswitch = true;
			if (Chat::bFlashWindow)
				bMustFlash = true;
		}
	}

	if (dat && bFlagUnread) {
		dat->m_dwUnread++;
		if (dat->m_pWnd)
			dat->m_pWnd->Invalidate();
	}
	
	if (iMuteMode != CHATMODE_MUTE) {
		auto sound = si->getSoundName(gce->iType);
		if (dat) {
			bInactive = dat->m_pContainer->m_hwnd != GetForegroundWindow();
			bActiveTab = (dat->m_pContainer->m_hwndActive == dat->GetHwnd());
			if (sound && dat->m_pContainer->MustPlaySound(dat))
				Skin_PlaySound(sound);
		}
		else if (sound)
			Skin_PlaySound(sound);
	}

	// dialog event processing
	if (dat) {
		HICON hIconHighlight = g_chatApi.getIcon(GC_EVENT_HIGHLIGHT), hIconMessage = g_chatApi.getIcon(GC_EVENT_MESSAGE);
		HICON hNotifyIcon = (bManyFix && !bInactive) ? 0 : g_chatApi.getIcon(gce->iType);
		BOOL bForcedIcon = (hNotifyIcon == hIconHighlight || hNotifyIcon == hIconMessage);

		if ((gce->iType & si->iLogTrayFlags) || bForcedIcon) {
			if (!bActiveTab) {
				if (hNotifyIcon == hIconHighlight)
					dat->m_iFlashIcon = hNotifyIcon;
				else {
					if (dat->m_iFlashIcon != hIconHighlight && dat->m_iFlashIcon != hIconMessage)
						dat->m_iFlashIcon = hNotifyIcon;
				}
				dat->m_bCanFlashTab = TRUE;
				dat->timerFlash.Start(TIMEOUT_FLASHWND);
			}
		}
		if (dat->m_pWnd) {
			dat->m_pWnd->updateIcon(hNotifyIcon);
			dat->m_pWnd->setOverlayIcon(hNotifyIcon, true);
		}

		// autoswitch tab..
		if (bMustAutoswitch) {
			if ((IsIconic(dat->m_pContainer->m_hwnd)) && !IsZoomed(dat->m_pContainer->m_hwnd) && g_plugin.bAutoSwitchTabs && dat->m_pContainer->m_hwndActive != dat->GetHwnd()) {
				int iItem = GetTabIndexFromHWND(dat->m_pContainer->m_hwndTabs, dat->GetHwnd());
				if (iItem >= 0) {
					TabCtrl_SetCurSel(dat->m_pContainer->m_hwndTabs, iItem);
					ShowWindow(dat->m_pContainer->m_hwndActive, SW_HIDE);
					dat->m_pContainer->m_hwndActive = dat->GetHwnd();
					dat->m_pContainer->UpdateTitle(dat->m_hContact);
					dat->m_pContainer->cfg.flags.m_bDeferredTabSelect = true;
				}
			}
		}

		// flash window if it is not focused
		if (bMustFlash && bInactive)
			if (!dat->m_pContainer->cfg.flags.m_bNoFlash)
				dat->m_pContainer->FlashContainer(1, 0);

		if (hNotifyIcon && bInactive && ((gce->iType & si->iLogTrayFlags) || bForcedIcon)) {
			if (bMustFlash)
				dat->m_hTabIcon = hNotifyIcon;
			else if (dat->m_iFlashIcon) {
				dat->m_hTabIcon = dat->m_iFlashIcon;

				TCITEM item = {};
				item.mask = TCIF_IMAGE;
				item.iImage = 0;
				TabCtrl_SetItem(GetParent(dat->GetHwnd()), dat->m_iTabID, &item);
			}

			HICON hIcon = (HICON)SendMessage(dat->m_pContainer->m_hwnd, WM_GETICON, ICON_BIG, 0);
			if (hNotifyIcon == hIconHighlight || (hIcon != hIconMessage && hIcon != hIconHighlight)) {
				dat->m_pContainer->SetIcon(dat, hNotifyIcon);
				dat->m_pContainer->cfg.flags.m_bNeedsUpdateTitle = true;
			}
		}

		if (bMustFlash && bInactive)
			AddUnreadContact(dat->m_hContact);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// set all filters and notification config for a session
// uses per channel mask + filterbits, default config as backup

void Chat_SetFilters(SESSION_INFO *si)
{
	uint32_t dwFlags_default = db_get_dw(0, CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
	uint32_t dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
	uint32_t dwMask = db_get_dw(si->hContact, CHAT_MODULE, "FilterMask", 0);

	CMsgDialog *pDlg = si->pDlg;
	if (pDlg) {
		pDlg->m_iLogFilterFlags = dwFlags_default;
		for (int i = 0; i < 32; i++) {
			uint32_t dwBit = 1 << i;
			if (dwMask & dwBit)
				pDlg->m_iLogFilterFlags = (dwFlags_local & dwBit) ? pDlg->m_iLogFilterFlags | dwBit : pDlg->m_iLogFilterFlags & ~dwBit;
		}
	}

	dwFlags_default = db_get_dw(0, CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "PopupMask", 0);

	si->iLogPopupFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		uint32_t dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogPopupFlags = (dwFlags_local & dwBit) ? si->iLogPopupFlags | dwBit : si->iLogPopupFlags & ~dwBit;
	}

	dwFlags_default = db_get_dw(0, CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconMask", 0);

	si->iLogTrayFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		uint32_t dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogTrayFlags = (dwFlags_local & dwBit) ? si->iLogTrayFlags | dwBit : si->iLogTrayFlags & ~dwBit;
	}

	if (pDlg != nullptr && pDlg->m_iLogFilterFlags == 0)
		pDlg->m_bFilterEnabled = 0;
}

char GetIndicator(SESSION_INFO *si, LPCTSTR ptszNick, int *iNickIndex)
{
	if (iNickIndex)
		*iNickIndex = 0;

	for (auto &ui : si->getUserList()) {
		if (!mir_wstrcmp(ui->pszNick, ptszNick)) {
			STATUSINFO *ti = g_chatApi.TM_FindStatus(si->pStatuses, g_chatApi.TM_WordToString(si->pStatuses, ui->Status));
			if (ti) {
				if (iNickIndex)
					*iNickIndex = ti->iIconIndex; // color table's index is not zero-based
				return szIndicators[ti->iIconIndex];
			}
			break;
		}
	}
	return 0;
}

bool IsHighlighted(SESSION_INFO *si, GCEVENT *gce)
{
	if (!g_Settings.bHighlightEnabled || !gce || gce->bIsMe)
		return false;

	GCEVENT evTmp = *gce;

	int dwMask = 0;
	if (gce->pszText.w != nullptr)
		dwMask |= CMUCHighlight::MATCH_TEXT;

	if (gce->pszNick.w != nullptr) {
		dwMask |= CMUCHighlight::MATCH_NICKNAME;
		if (si && g_Settings.bLogClassicIndicators) {
			size_t len = mir_wstrlen(gce->pszNick.w) + 1;
			wchar_t *tmp = (wchar_t*)_alloca(sizeof(wchar_t)*(len + 1));
			*tmp = GetIndicator(si, gce->pszNick.w, nullptr);
			mir_wstrcpy(tmp + 1, gce->pszNick.w);
			evTmp.pszNick.w = tmp;
		}
	}
	return g_Settings.Highlight->match(&evTmp, si, dwMask);
}
