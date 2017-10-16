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
// Helper functions for the group chat module.

#include "stdafx.h"

static void __stdcall ShowRoomFromPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	ShowRoom(nullptr, si);
}

static void __stdcall Chat_DismissPopup(void *pi)
{
	SESSION_INFO *si = (SESSION_INFO*)pi;
	if (si->hContact)
		if (pcli->pfnGetEvent(si->hContact, 0))
			pcli->pfnRemoveEvent(si->hContact, GC_FAKE_EVENT);

	if (si->pDlg && KillTimer(si->pDlg->GetHwnd(), TIMERID_FLASHWND))
		FlashWindow(si->pDlg->GetHwnd(), FALSE);
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SESSION_INFO *si = (SESSION_INFO*)PUGetPluginData(hWnd);

	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			if (si) {
				if (nen_options.maskActL & MASK_OPEN)
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
			if (nen_options.maskActR & MASK_OPEN)
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
	POPUPDATAT pd = { 0 };
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
	mir_snwprintf(pd.lptzContactName, L"%s - %s", (pa == nullptr) ? _A2T(pszProtoName) : pa->tszAccountName, pcli->pfnGetContactDisplayName(hContact, 0));
	wcsncpy_s(pd.lptzText, TranslateW(szBuf), _TRUNCATE);
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
	return PUAddPopupT(&pd);
}

BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	int iEvent = gce->iType;
	if (si == nullptr || !(iEvent & si->iLogPopupFlags))
		return true;

	CTabBaseDlg *dat = si->pDlg;
	TContainerData *pContainer = dat ? dat->m_pContainer : nullptr;

	wchar_t *bbStart, *bbEnd;
	if (g_Settings.bBBCodeInPopups) {
		bbStart = L"[b]";
		bbEnd = L"[/b]";
	}
	else bbStart = bbEnd = L"";

	if (nen_options.iMUCDisable)                          // no popups at all. Period
		return 0;
	/*
	* check the status mode against the status mask
	*/

	char *szProto = dat ? dat->m_szProto : si->pszModule;
	if (nen_options.dwStatusMask != -1) {
		DWORD dwStatus = 0;
		if (szProto != nullptr) {
			dwStatus = (DWORD)CallProtoService(szProto, PS_GETSTATUS, 0, 0);
			if (!(dwStatus == 0 || dwStatus <= ID_STATUS_OFFLINE || ((1 << (dwStatus - ID_STATUS_ONLINE)) & nen_options.dwStatusMask)))            // should never happen, but...
				return 0;
		}
	}
	if (dat && pContainer != 0) {                // message window is open, need to check the container config if we want to see a popup nonetheless
		if (nen_options.bWindowCheck) {                  // no popups at all for open windows... no exceptions
			if (!PluginConfig.m_bHideOnClose)
				return 0;
			if (pContainer->fHidden)
				goto passed;
			return 0;
		}
		if (pContainer->dwFlags & CNT_DONTREPORT && IsIconic(pContainer->m_hwnd))        // in tray counts as "minimised"
			goto passed;
		if (pContainer->dwFlags & CNT_DONTREPORTUNFOCUSED) {
			if (!IsIconic(pContainer->m_hwnd) && GetForegroundWindow() != pContainer->m_hwnd && GetActiveWindow() != pContainer->m_hwnd)
				goto passed;
		}
		if (pContainer->dwFlags & CNT_ALWAYSREPORTINACTIVE) {
			if (pContainer->dwFlags & CNT_DONTREPORTFOCUSED)
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
		clr = pci->aFonts[16].color;
		iNewEvent &= ~GC_EVENT_HIGHLIGHT;
	}

	if (iNewEvent == GC_EVENT_MESSAGE) {
		ShowPopup(si->hContact, si, pci->hIcons[ICON_MESSAGE], si->pszModule, si->ptszName, clr ? clr : pci->aFonts[9].color,
			TranslateT("%s%s says:%s %s"), bbStart, gce->ptszNick, bbEnd, pci->RemoveFormatting(gce->ptszText));
	}
	else oldDoPopup(si, gce);

	return TRUE;
}

void DoFlashAndSoundWorker(FLASH_PARAMS *p)
{
	SESSION_INFO *si = SM_FindSessionByHCONTACT(p->hContact);
	if (si == nullptr)
		return;

	CTabBaseDlg *dat = 0;
	if (si->pDlg) {
		dat = si->pDlg;
		if (dat) {
			p->bInactive = dat->m_pContainer->m_hwnd != GetForegroundWindow();
			p->bActiveTab = (dat->m_pContainer->m_hwndActive == si->pDlg->GetHwnd());
			if (p->sound && dat->MustPlaySound())
				Skin_PlaySound(p->sound);
		}
	}
	else if (p->sound)
		Skin_PlaySound(p->sound);

	if (dat) {
		HWND hwndTab = GetParent(si->pDlg->GetHwnd());
		BOOL bForcedIcon = (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT] || p->hNotifyIcon == pci->hIcons[ICON_MESSAGE]);

		if ((p->iEvent & si->iLogTrayFlags) || bForcedIcon) {
			if (!p->bActiveTab) {
				if (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT])
					dat->m_iFlashIcon = p->hNotifyIcon;
				else {
					if (dat->m_iFlashIcon != pci->hIcons[ICON_HIGHLIGHT] && dat->m_iFlashIcon != pci->hIcons[ICON_MESSAGE])
						dat->m_iFlashIcon = p->hNotifyIcon;
				}
				dat->m_bCanFlashTab = TRUE;
				SetTimer(si->pDlg->GetHwnd(), TIMERID_FLASHWND, TIMEOUT_FLASHWND, nullptr);
			}
		}
		if (dat->m_pWnd) {
			dat->m_pWnd->updateIcon(p->hNotifyIcon);
			dat->m_pWnd->setOverlayIcon(p->hNotifyIcon, true);
		}

		// autoswitch tab..
		if (p->bMustAutoswitch) {
			if ((IsIconic(dat->m_pContainer->m_hwnd)) && !IsZoomed(dat->m_pContainer->m_hwnd) && PluginConfig.haveAutoSwitch() && dat->m_pContainer->m_hwndActive != si->pDlg->GetHwnd()) {
				int iItem = GetTabIndexFromHWND(hwndTab, si->pDlg->GetHwnd());
				if (iItem >= 0) {
					TabCtrl_SetCurSel(hwndTab, iItem);
					ShowWindow(dat->m_pContainer->m_hwndActive, SW_HIDE);
					dat->m_pContainer->m_hwndActive = si->pDlg->GetHwnd();
					dat->m_pContainer->UpdateTitle(dat->m_hContact);
					dat->m_pContainer->dwFlags |= CNT_DEFERREDTABSELECT;
				}
			}
		}

		// flash window if it is not focused
		if (p->bMustFlash && p->bInactive)
			if (!(dat->m_pContainer->dwFlags & CNT_NOFLASH))
				FlashContainer(dat->m_pContainer, 1, 0);

		if (p->hNotifyIcon && p->bInactive && ((p->iEvent & si->iLogTrayFlags) || bForcedIcon)) {
			if (p->bMustFlash)
				dat->m_hTabIcon = p->hNotifyIcon;
			else if (dat->m_iFlashIcon) {
				dat->m_hTabIcon = dat->m_iFlashIcon;

				TCITEM item = {};
				item.mask = TCIF_IMAGE;
				item.iImage = 0;
				TabCtrl_SetItem(GetParent(si->pDlg->GetHwnd()), dat->m_iTabID, &item);
			}

			HICON hIcon = (HICON)SendMessage(dat->m_pContainer->m_hwnd, WM_GETICON, ICON_BIG, 0);
			if (p->hNotifyIcon == pci->hIcons[ICON_HIGHLIGHT] || (hIcon != pci->hIcons[ICON_MESSAGE] && hIcon != pci->hIcons[ICON_HIGHLIGHT])) {
				SendMessage(dat->m_pContainer->m_hwnd, DM_SETICON, (WPARAM)dat, (LPARAM)p->hNotifyIcon);
				dat->m_pContainer->dwFlags |= CNT_NEED_UPDATETITLE;
			}
		}

		if (p->bMustFlash && p->bInactive)
			UpdateTrayMenu(dat, si->wStatus, si->pszModule, dat->m_wszStatus, si->hContact, 1);
	}

	mir_free(p);
}

BOOL DoSoundsFlashPopupTrayStuff(SESSION_INFO *si, GCEVENT *gce, BOOL bHighlight, int bManyFix)
{
	if (gce == 0 || si == 0 || gce->bIsMe || si->iType == GCW_SERVER)
		return FALSE;

	CTabBaseDlg *dat = nullptr;
	FLASH_PARAMS *params = (FLASH_PARAMS*)mir_calloc(sizeof(FLASH_PARAMS));
	params->hContact = si->hContact;
	params->bInactive = TRUE;
	if (si->pDlg) {
		dat = si->pDlg;
		if ((si->pDlg->GetHwnd() == si->pDlg->m_pContainer->m_hwndActive) && GetForegroundWindow() == si->pDlg->m_pContainer->m_hwnd)
			params->bInactive = FALSE;
	}
	params->bActiveTab = params->bMustFlash = params->bMustAutoswitch = FALSE;
	params->iEvent = gce->iType;

	WPARAM wParamForHighLight = 0;
	bool bFlagUnread = false;
	if (bHighlight) {
		gce->iType |= GC_EVENT_HIGHLIGHT;
		params->sound = "ChatHighlight";
		if (db_get_b(si->hContact, "CList", "Hidden", 0) != 0)
			db_unset(si->hContact, "CList", "Hidden");
		if (params->bInactive) {
			bFlagUnread = true;
			DoTrayIcon(si, gce);
		}

		if (g_Settings.bCreateWindowOnHighlight && dat == nullptr)
			wParamForHighLight = 1;

		if (dat && g_Settings.bAnnoyingHighlight && params->bInactive && dat->m_pContainer->m_hwnd != GetForegroundWindow()) {
			wParamForHighLight = 2;
			params->hWnd = dat->GetHwnd();
		}

		if (dat || !nen_options.iMUCDisable)
			DoPopup(si, gce);
		if (g_Settings.bFlashWindowHighlight && params->bInactive)
			params->bMustFlash = TRUE;
		params->bMustAutoswitch = TRUE;
		params->hNotifyIcon = pci->hIcons[ICON_HIGHLIGHT];
	}
	else {
		// do blinking icons in tray
		if (params->bInactive || !g_Settings.bTrayIconInactiveOnly) {
			DoTrayIcon(si, gce);
			if (params->iEvent == GC_EVENT_MESSAGE)
				bFlagUnread = true;
		}
		// stupid thing to not create multiple popups for a QUIT event for instance
		if (bManyFix == 0) {
			// do popups
			if (dat || !nen_options.iMUCDisable)
				DoPopup(si, gce);

			// do sounds and flashing
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->sound = "ChatJoin";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->sound = "ChatPart";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->sound = "ChatQuit";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_QUIT];
				break;
			case GC_EVENT_ADDSTATUS:
			case GC_EVENT_REMOVESTATUS:
				params->sound = "ChatMode";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[params->iEvent == GC_EVENT_ADDSTATUS ? ICON_ADDSTATUS : ICON_REMSTATUS];
				break;
			case GC_EVENT_KICK:
				params->sound = "ChatKick";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_KICK];
				break;
			case GC_EVENT_MESSAGE:
				params->sound = "ChatMessage";
				if (params->bInactive && !(si->wState & STATE_TALK)) {
					si->wState |= STATE_TALK;
					db_set_w(si->hContact, si->pszModule, "ApparentMode", ID_STATUS_OFFLINE);
				}
				break;
			case GC_EVENT_ACTION:
				params->sound = "ChatAction";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->sound = "ChatNick";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->sound = "ChatNotice";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->sound = "ChatTopic";
				if (params->bInactive)
					params->hNotifyIcon = pci->hIcons[ICON_TOPIC];
				break;
			}
		}
		else {
			switch (params->iEvent) {
			case GC_EVENT_JOIN:
				params->hNotifyIcon = pci->hIcons[ICON_JOIN];
				break;
			case GC_EVENT_PART:
				params->hNotifyIcon = pci->hIcons[ICON_PART];
				break;
			case GC_EVENT_QUIT:
				params->hNotifyIcon = pci->hIcons[ICON_QUIT];
				break;
			case GC_EVENT_KICK:
				params->hNotifyIcon = pci->hIcons[ICON_KICK];
				break;
			case GC_EVENT_ACTION:
				params->hNotifyIcon = pci->hIcons[ICON_ACTION];
				break;
			case GC_EVENT_NICK:
				params->hNotifyIcon = pci->hIcons[ICON_NICK];
				break;
			case GC_EVENT_NOTICE:
				params->hNotifyIcon = pci->hIcons[ICON_NOTICE];
				break;
			case GC_EVENT_TOPIC:
				params->hNotifyIcon = pci->hIcons[ICON_TOPIC];
				break;
			case GC_EVENT_ADDSTATUS:
				params->hNotifyIcon = pci->hIcons[ICON_ADDSTATUS];
				break;
			case GC_EVENT_REMOVESTATUS:
				params->hNotifyIcon = pci->hIcons[ICON_REMSTATUS];
				break;
			}
		}

		if (params->iEvent == GC_EVENT_MESSAGE) {
			params->bMustAutoswitch = TRUE;
			if (g_Settings.bFlashWindow)
				params->bMustFlash = TRUE;
			params->hNotifyIcon = pci->hIcons[ICON_MESSAGE];
		}
	}
	if (dat && bFlagUnread) {
		dat->m_dwUnread++;
		if (dat->m_pWnd)
			dat->m_pWnd->Invalidate();
	}
	PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_MUCFLASHWORKER, wParamForHighLight, (LPARAM)params);
	return TRUE;
}

wchar_t* my_strstri(const wchar_t* s1, const wchar_t* s2)
{
	int i, j, k;

	_wsetlocale(LC_ALL, L"");
	for (i = 0; s1[i]; i++)
		for (j = i, k = 0; towlower(s1[j]) == towlower(s2[k]); j++, k++)
			if (!s2[k + 1])
				return (wchar_t*)(s1 + i);

	return nullptr;
}

/*
* log the event to the log file
* allows selective logging of wanted events
*/
BOOL LogToFile(SESSION_INFO *si, GCEVENT *gce)
{
	if (!si || !gce)
		return FALSE;

	/*
	* check whether we have to log this event
	*/
	if (!(gce->iType & si->iDiskLogFlags))
		return FALSE;

	return oldLogToFile(si, gce); // call kernel method
}

/*
 * set all filters and notification config for a session
 * uses per channel mask + filterbits, default config as backup
 */
void Chat_SetFilters(SESSION_INFO *si)
{
	DWORD dwFlags_default = M.GetDword(CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
	DWORD dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "FilterFlags", GC_EVENT_ALL);
	DWORD dwMask = db_get_dw(si->hContact, CHAT_MODULE, "FilterMask", 0);

	CChatRoomDlg *pDlg = si->pDlg;
	if (pDlg) {
		pDlg->m_iLogFilterFlags = dwFlags_default;
		for (int i = 0; i < 32; i++) {
			DWORD dwBit = 1 << i;
			if (dwMask & dwBit)
				pDlg->m_iLogFilterFlags = (dwFlags_local & dwBit) ? pDlg->m_iLogFilterFlags | dwBit : pDlg->m_iLogFilterFlags & ~dwBit;
		}
	}

	dwFlags_default = M.GetDword(CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "PopupFlags", GC_EVENT_HIGHLIGHT);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "PopupMask", 0);

	si->iLogPopupFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		DWORD dwBit = 1 << i;
		if (dwMask & dwBit)
			si->iLogPopupFlags = (dwFlags_local & dwBit) ? si->iLogPopupFlags | dwBit : si->iLogPopupFlags & ~dwBit;
	}

	dwFlags_default = M.GetDword(CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
	dwFlags_local = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconFlags", GC_EVENT_HIGHLIGHT);
	dwMask = db_get_dw(si->hContact, CHAT_MODULE, "TrayIconMask", 0);

	si->iDiskLogFlags = M.GetDword(CHAT_MODULE, "DiskLogFlags", GC_EVENT_ALL);

	si->iLogTrayFlags = dwFlags_default;
	for (int i = 0; i < 32; i++) {
		DWORD dwBit = 1 << i;
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

	for (USERINFO *ui = si->pUsers; ui; ui = ui->next) {
		if (!mir_wstrcmp(ui->pszNick, ptszNick)) {
			STATUSINFO *ti = pci->TM_FindStatus(si->pStatuses, pci->TM_WordToString(si->pStatuses, ui->Status));
			if (ti && (INT_PTR)ti->hIcon < si->iStatusCount) {
				if (iNickIndex)
					*iNickIndex = (INT_PTR)ti->hIcon; // color table's index is not zero-based
				return szIndicators[(INT_PTR)ti->hIcon];
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
	if (gce->ptszText != nullptr)
		dwMask |= CMUCHighlight::MATCH_TEXT;

	if (gce->ptszNick != nullptr) {
		dwMask |= CMUCHighlight::MATCH_NICKNAME;
		if (si && g_Settings.bLogClassicIndicators) {
			size_t len = mir_wstrlen(gce->ptszNick) + 1;
			wchar_t *tmp = (wchar_t*)_alloca(sizeof(wchar_t)*(len + 1));
			*tmp = GetIndicator(si, gce->ptszNick, 0);
			mir_wstrcpy(tmp + 1, gce->ptszNick);
			evTmp.ptszNick = tmp;
		}
	}
	return g_Settings.Highlight->match(&evTmp, si, dwMask);
}
