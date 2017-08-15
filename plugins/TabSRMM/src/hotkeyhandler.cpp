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
// The hotkeyhandler is a small, invisible window which handles the following things:
//
//    a) event notify stuff, messages posted from the popups to avoid threading
//       issues.
//
//    b) tray icon handling
//
//    c) send later job management. Periodically process the queue of open
//	   deferred send jobs.

#include "stdafx.h"

static UINT 	WM_TASKBARCREATED;
static HANDLE 	hSvcHotkeyProcessor = 0;

static HOTKEYDESC _hotkeydescs[] = {
	{ "tabsrmm_mostrecent", LPGEN("Most recent unread session"), TABSRMM_HK_SECTION_IM, MS_TABMSG_HOTKEYPROCESS, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'R'), TABSRMM_HK_LASTUNREAD },
	{ "tabsrmm_paste_and_send", LPGEN("Paste and send"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'D'), TABSRMM_HK_PASTEANDSEND },
	{ "tabsrmm_uprefs", LPGEN("Contact's messaging preferences"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'C'), TABSRMM_HK_SETUSERPREFS },
	{ "tabsrmm_copts", LPGEN("Container options"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'O'), TABSRMM_HK_CONTAINEROPTIONS },
	{ "tabsrmm_sendfile", LPGEN("Send a file"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'F'), TABSRMM_HK_SENDFILE },
	{ "tabsrmm_quote", LPGEN("Quote message"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'Q'), TABSRMM_HK_QUOTEMSG },
	{ "tabsrmm_sendlater", LPGEN("Toggle send later"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'S'), TABSRMM_HK_TOGGLESENDLATER },

	{ "tabsrmm_send", LPGEN("Send message"), TABSRMM_HK_SECTION_GENERIC, 0, 0, TABSRMM_HK_SEND },
	{ "tabsrmm_hist", LPGEN("Show message history"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT, 'H'), TABSRMM_HK_HISTORY },
	{ "tabsrmm_sendmenu", LPGEN("Show send menu"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'S'), TABSRMM_HK_SENDMENU },
	{ "tabsrmm_protomenu", LPGEN("Show protocol menu"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'P'), TABSRMM_HK_PROTOMENU },
	{ "tabsrmm_umenu", LPGEN("Show user menu"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'D'), TABSRMM_HK_USERMENU },
	{ "tabsrmm_udet", LPGEN("Show user details"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'U'), TABSRMM_HK_USERDETAILS },
	{ "tabsrmm_tbar", LPGEN("Toggle toolbar"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_SHIFT, 'T'), TABSRMM_HK_TOGGLETOOLBAR },
	{ "tabsrmm_ipanel", LPGEN("Toggle info panel"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'I'), TABSRMM_HK_TOGGLEINFOPANEL },
	{ "tabsrmm_rtl", LPGEN("Toggle text direction"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'B'), TABSRMM_HK_TOGGLERTL },
	{ "tabsrmm_msend", LPGEN("Toggle multi send"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'M'), TABSRMM_HK_TOGGLEMULTISEND },
	{ "tabsrmm_clearlog", LPGEN("Clear message log"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'L'), TABSRMM_HK_CLEARLOG },
	{ "tabsrmm_notes", LPGEN("Edit user notes"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), TABSRMM_HK_EDITNOTES },
	{ "tabsrmm_sbar", LPGEN("Collapse side bar"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(0, VK_F9), TABSRMM_HK_TOGGLESIDEBAR },
	{ "tabsrmm_muc_cmgr", LPGEN("Channel manager"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'C'), TABSRMM_HK_CHANNELMGR },
	{ "tabsrmm_muc_filter", LPGEN("Toggle filter"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'F'), TABSRMM_HK_FILTERTOGGLE },
	{ "tabsrmm_muc_nick", LPGEN("Toggle nick list"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), TABSRMM_HK_LISTTOGGLE },
	{ "tabsrmm_muc_server_show", LPGEN("Show server window"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, '1'), TABSRMM_HK_MUC_SHOWSERVER },
	{ "tabsrmm_close_other", LPGEN("Close other tabs"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'W'), TABSRMM_HK_CLOSE_OTHER },
};

LRESULT ProcessHotkeysByMsgFilter(const CCtrlBase &pCtrl, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MSGFILTER mf;
	mf.nmhdr.code = EN_MSGFILTER;
	mf.nmhdr.hwndFrom = pCtrl.GetHwnd();
	mf.nmhdr.idFrom = pCtrl.GetCtrlId();
	mf.lParam = lParam;
	mf.wParam = wParam;
	mf.msg = msg;
	return SendMessage(pCtrl.GetParent()->GetHwnd(), WM_NOTIFY, 0, (LPARAM)&mf);
}

static INT_PTR HotkeyProcessor(WPARAM, LPARAM lParam)
{
	if (lParam == TABSRMM_HK_LASTUNREAD)
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_MBUTTONDOWN);

	return 0;
}

void TSAPI HandleMenuEntryFromhContact(MCONTACT hContact)
{
	if (hContact == 0)
		return;

	HWND hWnd = Srmm_FindWindow(hContact);
	if (hWnd && IsWindow(hWnd)) {
		TContainerData *pContainer = 0;
		SendMessage(hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
		if (pContainer) {
			ActivateExistingTab(pContainer, hWnd);
			pContainer->hwndSaved = 0;
			SetForegroundWindow(pContainer->m_hwnd);
		}
		else CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		return;
	}

	Clist_ContactDoubleClicked(hContact);
}

void TSAPI DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, DWORD dwIdle)
{
	FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_MENU));
	if (dwIdle)
		CSkin::DrawDimmedIcon(dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - 16) / 2, 16, 16, hIcon, 180);
	else
		DrawIconEx(dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - 16) / 2, hIcon, 16, 16, 0, 0, DI_NORMAL | DI_COMPAT);
}

LONG_PTR CALLBACK HotkeyHandlerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static POINT ptLast;
	static int iMousedown;

	if (msg == WM_TASKBARCREATED) {
		CreateSystrayIcon(FALSE);
		if (nen_options.bTraySupport)
			CreateSystrayIcon(TRUE);
		return 0;
	}

	switch (msg) {
	case WM_CREATE:
		for (int i = 0; i < _countof(_hotkeydescs); i++)
			Hotkey_Register(&_hotkeydescs[i]);

		WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");
		ShowWindow(hwndDlg, SW_HIDE);
		hSvcHotkeyProcessor = CreateServiceFunction(MS_TABMSG_HOTKEYPROCESS, HotkeyProcessor);
		SetTimer(hwndDlg, TIMERID_SENDLATER, TIMEOUT_SENDLATER, nullptr);
		break;

	case WM_HOTKEY:
		{
			CLISTEVENT *cli = pcli->pfnGetEvent(-1, 0);
			if (cli != nullptr) {
				if (strncmp(cli->pszService, MS_MSG_TYPINGMESSAGE, mir_strlen(cli->pszService))) {
					CallService(cli->pszService, 0, (LPARAM)cli);
					break;
				}
			}
			if (wParam == 0xc001)
				SendMessage(hwndDlg, DM_TRAYICONNOTIFY, 101, WM_MBUTTONDOWN);
		}
		break;

	// handle the popup menus (session list, favorites, recents...
	// just draw some icons, nothing more :)
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lParam;
			lpmi->itemHeight = 0;
			lpmi->itemWidth = 6;
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU && (dis->hwndItem == (HWND)PluginConfig.g_hMenuFavorites || dis->hwndItem == (HWND)PluginConfig.g_hMenuRecent)) {
				HICON hIcon = (HICON)dis->itemData;

				DrawMenuItem(dis, hIcon, 0);
				return TRUE;
			}
			else if (dis->CtlType == ODT_MENU) {
				HWND hWnd = Srmm_FindWindow((MCONTACT)dis->itemID);
				DWORD idle = 0;

				if (hWnd == nullptr) {
					SESSION_INFO *si = SM_FindSessionByHCONTACT((MCONTACT)dis->itemID);
					hWnd = si ? si->pDlg->GetHwnd() : nullptr;
				}

				CSrmmWindow *dat = 0;
				if (hWnd)
					dat = (CSrmmWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

				{
					HICON hIcon;

					if (dis->itemData > 0)
						hIcon = (dis->itemData & 0x10000000) ? pci->hIcons[ICON_HIGHLIGHT] : PluginConfig.g_IconMsgEvent;
					else if (dat != nullptr) {
						hIcon = dat->GetMyContactIcon(0);
						idle = dat->m_idle;
					}
					else hIcon = PluginConfig.g_iconContainer;

					DrawMenuItem(dis, hIcon, idle);
					return TRUE;
				}
			}
		}
		break;

	case DM_TRAYICONNOTIFY:
		if (wParam == 100 || wParam == 101) {
			switch (lParam) {
			case WM_LBUTTONUP:
				POINT pt;
				{
					GetCursorPos(&pt);
					if (wParam == 100)
						SetForegroundWindow(hwndDlg);
					if (GetMenuItemCount(PluginConfig.g_hMenuTrayUnread) > 0) {
						BOOL iSelection = TrackPopupMenu(PluginConfig.g_hMenuTrayUnread, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);
						HandleMenuEntryFromhContact((MCONTACT)iSelection);
					}
					else TrackPopupMenu(GetSubMenu(PluginConfig.g_hMenuContext, 8), TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);

					if (wParam == 100)
						PostMessage(hwndDlg, WM_NULL, 0, 0);
				}
				break;

			case WM_MBUTTONDOWN:
				if (wParam == 100)
					SetForegroundWindow(hwndDlg);
				{
					int iCount = GetMenuItemCount(PluginConfig.g_hMenuTrayUnread);
					if (iCount > 0) {
						UINT uid = 0;
						MENUITEMINFOA mii = { 0 };
						mii.fMask = MIIM_DATA;
						mii.cbSize = sizeof(mii);
						int i = iCount - 1;
						do {
							GetMenuItemInfoA(PluginConfig.g_hMenuTrayUnread, i, TRUE, &mii);
							if (mii.dwItemData > 0) {
								uid = GetMenuItemID(PluginConfig.g_hMenuTrayUnread, i);
								HandleMenuEntryFromhContact((MCONTACT)uid);
								break;
							}
						} while (--i >= 0);

						if (uid == 0 && pLastActiveContainer != nullptr) {                // no session found, restore last active container
							if (IsIconic(pLastActiveContainer->m_hwnd) || !IsWindowVisible(pLastActiveContainer->m_hwnd)) {
								SendMessage(pLastActiveContainer->m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
								SetForegroundWindow(pLastActiveContainer->m_hwnd);
								SetFocus(GetDlgItem(pLastActiveContainer->m_hwndActive, IDC_SRMM_MESSAGE));
							}
							else if (GetForegroundWindow() != pLastActiveContainer->m_hwnd) {
								SetForegroundWindow(pLastActiveContainer->m_hwnd);
								SetFocus(GetDlgItem(pLastActiveContainer->m_hwndActive, IDC_SRMM_MESSAGE));
							}
							else {
								if (PluginConfig.m_bHideOnClose)
									ShowWindow(pLastActiveContainer->m_hwnd, SW_HIDE);
								else
									SendMessage(pLastActiveContainer->m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
							}
						}
					}
					if (wParam == 100)
						PostMessage(hwndDlg, WM_NULL, 0, 0);
				}
				break;

			case WM_RBUTTONUP:
				HMENU submenu = PluginConfig.g_hMenuTrayContext;

				if (wParam == 100)
					SetForegroundWindow(hwndDlg);
				GetCursorPos(&pt);
				CheckMenuItem(submenu, ID_TRAYCONTEXT_DISABLEALLPOPUPS, MF_BYCOMMAND | (nen_options.iDisable ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(submenu, ID_TRAYCONTEXT_DON40223, MF_BYCOMMAND | (nen_options.iNoSounds ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem(submenu, ID_TRAYCONTEXT_DON, MF_BYCOMMAND | (nen_options.iNoAutoPopup ? MF_CHECKED : MF_UNCHECKED));
				EnableMenuItem(submenu, ID_TRAYCONTEXT_HIDEALLMESSAGECONTAINERS, MF_BYCOMMAND | (nen_options.bTraySupport) ? MF_ENABLED : MF_GRAYED);
				CheckMenuItem(submenu, ID_TRAYCONTEXT_SHOWTHETRAYICON, MF_BYCOMMAND | (nen_options.bTraySupport ? MF_CHECKED : MF_UNCHECKED));
				BOOL iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, nullptr);
				if (iSelection) {
					MENUITEMINFO mii = { 0 };
					mii.cbSize = sizeof(mii);
					mii.fMask = MIIM_DATA | MIIM_ID;
					GetMenuItemInfo(submenu, (UINT_PTR)iSelection, FALSE, &mii);
					if (mii.dwItemData != 0)  // this must be an itm of the fav or recent menu
						HandleMenuEntryFromhContact((MCONTACT)iSelection);
					else {
						switch (iSelection) {
						case ID_TRAYCONTEXT_SHOWTHETRAYICON:
							nen_options.bTraySupport = !nen_options.bTraySupport;
							CreateSystrayIcon(nen_options.bTraySupport ? TRUE : FALSE);
							break;
						case ID_TRAYCONTEXT_DISABLEALLPOPUPS:
							nen_options.iDisable ^= 1;
							break;
						case ID_TRAYCONTEXT_DON40223:
							nen_options.iNoSounds ^= 1;
							break;
						case ID_TRAYCONTEXT_DON:
							nen_options.iNoAutoPopup ^= 1;
							break;
						case ID_TRAYCONTEXT_HIDEALLMESSAGECONTAINERS:
							for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
								ShowWindow(pCont->m_hwnd, SW_HIDE);
							break;
						case ID_TRAYCONTEXT_RESTOREALLMESSAGECONTAINERS:
							for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
								ShowWindow(pCont->m_hwnd, SW_SHOW);
							break;
						case ID_TRAYCONTEXT_BE:
							nen_options.iDisable = 1;
							nen_options.iNoSounds = 1;
							nen_options.iNoAutoPopup = 1;

							for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
								SendMessage(pCont->m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 1);
							break;
						}
					}
				}
				if (wParam == 100)
					PostMessage(hwndDlg, WM_NULL, 0, 0);
				break;
			}
		}
		break;

	// handle an event from the popup module (mostly window activation). Since popups may run in different threads, the message
	// is posted to our invisible hotkey handler which does always run within the main thread.
	// wParam is the hContact
	// lParam the event handle
	case DM_HANDLECLISTEVENT:
		// if lParam == nullptr, don't consider clist events, just open the message tab
		if (lParam == 0)
			HandleMenuEntryFromhContact(wParam);
		else {
			CLISTEVENT *cle = pcli->pfnGetEvent(wParam, 0);
			if (cle) {
				if (ServiceExists(cle->pszService)) {
					CallService(cle->pszService, 0, (LPARAM)cle);
					pcli->pfnRemoveEvent(cle->hContact, cle->hDbEvent);
				}
			}
			// still, we got that message posted.. the event may be waiting in tabSRMMs tray...
			else HandleMenuEntryFromhContact(wParam);
		}
		break;

	case DM_DOCREATETAB:
		{
			HWND hWnd = Srmm_FindWindow(lParam);
			if (hWnd && IsWindow(hWnd)) {
				TContainerData *pContainer = 0;
				SendMessage(hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
				if (pContainer) {
					int iTabs = TabCtrl_GetItemCount(GetDlgItem(pContainer->m_hwnd, IDC_MSGTABS));
					if (iTabs == 1)
						SendMessage(pContainer->m_hwnd, WM_CLOSE, 0, 1);
					else
						SendMessage(hWnd, WM_CLOSE, 0, 1);

					char *szProto = GetContactProto(lParam);
					if (szProto != nullptr && db_get_b(lParam, szProto, "ChatRoom", 0))
						ShowRoom((TContainerData*)wParam, SM_FindSessionByHCONTACT(lParam));
					else
						CreateNewTabForContact((TContainerData*)wParam, lParam, true, true, false);
				}
			}
		}
		break;

	case DM_SENDMESSAGECOMMANDW:
		SendMessageCommand_W(wParam, lParam);
		if (lParam)
			mir_free((void*)lParam);
		return 0;

	case DM_REMOVECLISTEVENT:
		// sent from the popup to "dismiss" the event. we should do this in the main thread
		pcli->pfnRemoveEvent(wParam, lParam);
		db_event_markRead(wParam, lParam);
		return 0;

	case DM_SETLOCALE:
		{
			HKL hkl = (HKL)lParam;
			MCONTACT hContact = wParam;

			HWND	hWnd = Srmm_FindWindow(hContact);
			if (hWnd) {
				CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if (dat) {
					if (hkl) {
						dat->m_hkl = hkl;
						PostMessage(dat->GetHwnd(), DM_SETLOCALE, 0, 0);
					}

					DBVARIANT  dbv;
					if (0 == db_get_ws(hContact, SRMSGMOD_T, "locale", &dbv)) {
						dat->GetLocaleID(dbv.ptszVal);
						db_free(&dbv);
						dat->UpdateReadChars();
					}
				}
			}
		}
		return 0;

	// react to changes in the desktop composition state
	// (enable/disable DWM, change to a non-aero visual style
	// or classic Windows theme
	case WM_DWMCOMPOSITIONCHANGED:
		SendMessage(hwndDlg, WM_THEMECHANGED, 0, 0);
		{
			bool bNewAero = M.getAeroState(); // refresh dwm state

			for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext) {
				if (bNewAero)
					SetAeroMargins(pCont);
				else {
					MARGINS m = { 0 };
					if (M.m_pfnDwmExtendFrameIntoClientArea)
						M.m_pfnDwmExtendFrameIntoClientArea(pCont->m_hwnd, &m);
				}
				if (pCont->SideBar)
					if (pCont->SideBar->isActive()) // the container for the sidebar buttons
						RedrawWindow(GetDlgItem(pCont->m_hwnd, 5000), nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(pCont->m_hwnd, nullptr, nullptr, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
		}
		Srmm_Broadcast(WM_DWMCOMPOSITIONCHANGED, 0, 0);
		break;

	// this message is fired when the user changes desktop color
	// settings (Desktop->personalize)
	// the handler reconfigures the aero-related skin images for
	// tabs and buttons to match the new desktop color theme.
	case WM_DWMCOLORIZATIONCOLORCHANGED:
		M.getAeroState();
		Skin->setupAeroSkins();
		CSkin::initAeroEffect();
		break;

		// user has changed the visual style or switched to/from
		// classic Windows theme
	case WM_THEMECHANGED:
		M.getAeroState();
		Skin->setupTabCloseBitmap();
		CSkin::initAeroEffect();
		PluginConfig.m_ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &PluginConfig.m_ncm, 0);
		FreeTabConfig();
		ReloadTabConfig();

		for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext) {
			SendDlgItemMessage(pCont->m_hwnd, IDC_MSGTABS, EM_THEMECHANGED, 0, 0);
			BroadCastContainer(pCont, EM_THEMECHANGED, 0, 0);
		}
		break;

	case DM_SPLITSENDACK:
		{
			SendJob *job = sendQueue->getJobByIndex((int)wParam);

			ACKDATA ack = { 0 };
			ack.hContact = job->hContact;
			ack.hProcess = (HANDLE)job->iSendId;
			ack.type = ACKTYPE_MESSAGE;
			ack.result = ACKRESULT_SUCCESS;

			if (job->hContact && job->iAcksNeeded && job->iStatus == SendQueue::SQ_INPROGRESS) {
				if (IsWindow(job->hOwnerWnd))
					::SendMessage(job->hOwnerWnd, HM_EVENTSENT, (WPARAM)MAKELONG(wParam, 0), (LPARAM)&ack);
				else
					sendQueue->ackMessage(0, (WPARAM)MAKELONG(wParam, 0), (LPARAM)&ack);
			}
		}
		return 0;

	case DM_LOGSTATUSCHANGE:
		CGlobals::logStatusChange(wParam, reinterpret_cast<CContactCache *>(lParam));
		return 0;

	case DM_MUCFLASHWORKER:
		{
			FLASH_PARAMS *p = reinterpret_cast<FLASH_PARAMS*>(lParam);
			if (1 == wParam) {
				Clist_ContactDoubleClicked(p->hContact);
				p->bActiveTab = TRUE;
				p->bInactive = FALSE;
				p->bMustAutoswitch = p->bMustFlash = FALSE;
			}

			if (2 == wParam) {
				p->bActiveTab = TRUE;
				p->bInactive = FALSE;
				p->bMustAutoswitch = p->bMustFlash = FALSE;
				SendMessage(p->hWnd, DM_ACTIVATEME, 0, 0);
			}
			DoFlashAndSoundWorker(p);
		}
		return 0;

	case WM_POWERBROADCAST:
	case WM_DISPLAYCHANGE:
		for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
			if (CSkin::m_skinEnabled) {             // invalidate cached background DCs for skinned containers
				pCont->oldDCSize.cx = pCont->oldDCSize.cy = 0;
				SelectObject(pCont->cachedDC, pCont->oldHBM);
				DeleteObject(pCont->cachedHBM);
				DeleteDC(pCont->cachedDC);
				pCont->cachedDC = 0;
				RedrawWindow(pCont->m_hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
			}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			SetWindowPos(hwndDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		return 0;

	case WM_CLOSE:
		return 0;

	case WM_TIMER:
		if (wParam == TIMERID_SENDLATER) {
			// send heartbeat to each container, they use this to update
			// dynamic content (i.e. local time in the info panel).
			for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
				SendMessage(pCont->m_hwnd, WM_TIMER, TIMERID_HEARTBEAT, 0);

			// process send later contacts and jobs, if enough time has elapsed
			if (sendLater->isAvail() && !sendLater->isInteractive() && (time(0) - sendLater->lastProcessed()) > CSendLater::SENDLATER_PROCESS_INTERVAL) {
				sendLater->setLastProcessed(time(0));

				// check the list of contacts that may have new send later jobs
				// (added on user's request)
				sendLater->processContacts();

				// start processing the job list
				if (!sendLater->isJobListEmpty()) {
					KillTimer(hwndDlg, wParam);
					sendLater->startJobListProcess();
					SetTimer(hwndDlg, TIMERID_SENDLATER_TICK, TIMEOUT_SENDLATER_TICK, 0);
				}
			}
		}

		// process one entry per tick (default: 200ms)
		// TODO better timings, possibly slow down when many jobs are in the
		// queue.
		else if (wParam == TIMERID_SENDLATER_TICK) {
			if (!sendLater->haveJobs()) {
				KillTimer(hwndDlg, wParam);
				SetTimer(hwndDlg, TIMERID_SENDLATER, TIMEOUT_SENDLATER, 0);
				sendLater->qMgrUpdate(true);
			}
			else sendLater->processCurrentJob();
		}
		break;

	case WM_DESTROY:
		KillTimer(hwndDlg, TIMERID_SENDLATER_TICK);
		KillTimer(hwndDlg, TIMERID_SENDLATER);
		DestroyServiceFunction(hSvcHotkeyProcessor);
		break;
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}
