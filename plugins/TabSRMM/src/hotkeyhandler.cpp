/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * The hotkeyhandler is a small, invisible window which handles the following things:

    a) event notify stuff, messages posted from the popups to avoid threading
       issues.

    b) tray icon handling

    c) send later job management. Periodically process the queue of open
	   deferred send jobs.
 */

#include "commonheaders.h"

static UINT 	WM_TASKBARCREATED;
static HANDLE 	hSvcHotkeyProcessor = 0;

static HOTKEYDESC _hotkeydescs[] = {
	{ 0, "tabsrmm_mostrecent", LPGEN("Most recent unread session"), TABSRMM_HK_SECTION_IM, MS_TABMSG_HOTKEYPROCESS, HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'R'), TABSRMM_HK_LASTUNREAD },
	{ 0, "tabsrmm_paste_and_send", LPGEN("Paste and send"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'D'), TABSRMM_HK_PASTEANDSEND },
	{ 0, "tabsrmm_uprefs", LPGEN("Contact's messaging prefs"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'C'), TABSRMM_HK_SETUSERPREFS },
	{ 0, "tabsrmm_copts", LPGEN("Container options"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'O'), TABSRMM_HK_CONTAINEROPTIONS },
	{ 0, "tabsrmm_nudge", LPGEN("Send nudge"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'N'), TABSRMM_HK_NUDGE },
	{ 0, "tabsrmm_sendfile", LPGEN("Send a file"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'F'), TABSRMM_HK_SENDFILE },
	{ 0, "tabsrmm_quote", LPGEN("Quote message"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'Q'), TABSRMM_HK_QUOTEMSG },
	{ 0, "tabsrmm_sendlater", LPGEN("Toggle send later"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'S'), TABSRMM_HK_TOGGLESENDLATER },

	{ 0, "tabsrmm_send", LPGEN("Send message"), TABSRMM_HK_SECTION_GENERIC, 0, 0, TABSRMM_HK_SEND },
	{ 0, "tabsrmm_emot", LPGEN("Smiley selector"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT, 'E'), TABSRMM_HK_EMOTICONS },
	{ 0, "tabsrmm_hist", LPGEN("Show message history"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT, 'H'), TABSRMM_HK_HISTORY },
	{ 0, "tabsrmm_umenu", LPGEN("Show user menu"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'D'), TABSRMM_HK_USERMENU },
	{ 0, "tabsrmm_udet", LPGEN("Show user details"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT, 'U'), TABSRMM_HK_USERDETAILS },
	{ 0, "tabsrmm_tbar", LPGEN("Toggle toolbar"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT|HOTKEYF_SHIFT, 'T'), TABSRMM_HK_TOGGLETOOLBAR },
	{ 0, "tabsrmm_ipanel", LPGEN("Toggle info panel"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT|HOTKEYF_CONTROL, 'I'), TABSRMM_HK_TOGGLEINFOPANEL },
	{ 0, "tabsrmm_rtl", LPGEN("Toggle text direction"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT|HOTKEYF_CONTROL, 'B'), TABSRMM_HK_TOGGLERTL },
	{ 0, "tabsrmm_msend", LPGEN("Toggle multi send"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_ALT|HOTKEYF_CONTROL, 'M'), TABSRMM_HK_TOGGLEMULTISEND },
	{ 0, "tabsrmm_clearlog", LPGEN("Clear message log"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_CONTROL, 'L'), TABSRMM_HK_CLEARLOG },
	{ 0, "tabsrmm_notes", LPGEN("Edit user notes"), TABSRMM_HK_SECTION_IM, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), TABSRMM_HK_EDITNOTES },
	{ 0, "tabsrmm_sbar", LPGEN("Collapse side bar"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(0, VK_F9), TABSRMM_HK_TOGGLESIDEBAR },
	{ 0, "tabsrmm_muc_cmgr", LPGEN("Channel manager"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'C'), TABSRMM_HK_CHANNELMGR },
	{ 0, "tabsrmm_muc_filter", LPGEN("Toggle filter"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'F'), TABSRMM_HK_FILTERTOGGLE },
	{ 0, "tabsrmm_muc_nick", LPGEN("Toggle nick list"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), TABSRMM_HK_LISTTOGGLE },
	{ 0, "tabsrmm_muc_server_show", LPGEN("Show server window"), TABSRMM_HK_SECTION_GC, 0, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, '1'), TABSRMM_HK_MUC_SHOWSERVER },
   { 0, "tabsrmm_close_other", LPGEN("Close Other Tabs"), TABSRMM_HK_SECTION_GENERIC, 0, HOTKEYCODE(HOTKEYF_ALT|HOTKEYF_CONTROL, 'W'), TABSRMM_HK_CLOSE_OTHER },
};

LRESULT ProcessHotkeysByMsgFilter(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR ctrlId)
{
	MSGFILTER mf;
	mf.nmhdr.code = EN_MSGFILTER;
	mf.nmhdr.hwndFrom = hwnd;
	mf.nmhdr.idFrom = ctrlId;
	mf.lParam = lParam;
	mf.wParam = wParam;
	mf.msg = msg;
	return SendMessage(GetParent(hwnd), WM_NOTIFY, 0, (LPARAM)&mf);
}

static INT_PTR HotkeyProcessor(WPARAM wParam, LPARAM lParam)
{
	if (lParam == TABSRMM_HK_LASTUNREAD)
		PostMessage(PluginConfig.g_hwndHotkeyHandler, DM_TRAYICONNOTIFY, 101, WM_MBUTTONDOWN);

	return 0;
}

void TSAPI HandleMenuEntryFromhContact(MCONTACT hContact)
{
	if (hContact == 0)
		return;

	HWND hWnd = M.FindWindow(hContact);
	if (hWnd && IsWindow(hWnd)) {
		TContainerData *pContainer = 0;
		SendMessage(hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
		if (pContainer) {
			ActivateExistingTab(pContainer, hWnd);
			pContainer->hwndSaved = 0;
			SetForegroundWindow(pContainer->hwnd);
		}
		else CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		return;
	}

	SESSION_INFO *si = SM_FindSessionByHCONTACT(hContact);
	if (si != NULL) {
		// session does exist, but no window is open for it
		if (si->hWnd) {
			TContainerData *pContainer = 0;
			SendMessage(si->hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
			if (pContainer) {
				ActivateExistingTab(pContainer, si->hWnd);
				if (GetForegroundWindow() != pContainer->hwnd)
					SetForegroundWindow(pContainer->hwnd);
				SetFocus(GetDlgItem(pContainer->hwndActive, IDC_CHAT_MESSAGE));
				return;
			}
		}
	}

	CallService(MS_CLIST_CONTACTDOUBLECLICKED, hContact, 0);
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

	TContainerData *p;

	switch (msg) {
	case WM_CREATE:
		for (int i=0; i < SIZEOF(_hotkeydescs); i++) {
			_hotkeydescs[i].cbSize = sizeof(HOTKEYDESC);
			Hotkey_Register(&_hotkeydescs[i]);
		}

		WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");
		ShowWindow(hwndDlg, SW_HIDE);
		hSvcHotkeyProcessor = CreateServiceFunction(MS_TABMSG_HOTKEYPROCESS, HotkeyProcessor);
		SetTimer(hwndDlg, TIMERID_SENDLATER, TIMEOUT_SENDLATER, NULL);
		break;

	case WM_HOTKEY:
		{
			CLISTEVENT *cli = (CLISTEVENT *)CallService(MS_CLIST_GETEVENT, (WPARAM)INVALID_HANDLE_VALUE, 0);
			if (cli != NULL) {
				if (strncmp(cli->pszService, "SRMsg/TypingMessage", strlen(cli->pszService))) {
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
			LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT) lParam;
			lpmi->itemHeight = 0;
			lpmi->itemWidth = 6;
		}
		return TRUE;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
			TWindowData *dat = 0;
			if (dis->CtlType == ODT_MENU && (dis->hwndItem == (HWND)PluginConfig.g_hMenuFavorites || dis->hwndItem == (HWND)PluginConfig.g_hMenuRecent)) {
				HICON hIcon = (HICON)dis->itemData;

				DrawMenuItem(dis, hIcon, 0);
				return TRUE;
			}
			else if (dis->CtlType == ODT_MENU) {
				HWND hWnd = M.FindWindow((MCONTACT)dis->itemID);
				DWORD idle = 0;

				if (hWnd == NULL) {
					SESSION_INFO *si = SM_FindSessionByHCONTACT((MCONTACT)dis->itemID);
					hWnd = si ? si->hWnd : 0;
				}

				if (hWnd)
					dat = (TWindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

				if (dis->itemData >= 0) {
					HICON hIcon;

					if (dis->itemData > 0)
						hIcon = dis->itemData & 0x10000000 ? pci->hIcons[ICON_HIGHLIGHT] : PluginConfig.g_IconMsgEvent;
					else if (dat != NULL) {
						hIcon = MY_GetContactIcon(dat, 0);
						idle = dat->idle;
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
				{
					POINT pt;
					GetCursorPos(&pt);
					if (wParam == 100)
						SetForegroundWindow(hwndDlg);
					if (GetMenuItemCount(PluginConfig.g_hMenuTrayUnread) > 0) {
						BOOL iSelection = TrackPopupMenu(PluginConfig.g_hMenuTrayUnread, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
						HandleMenuEntryFromhContact((MCONTACT)iSelection);
					}
					else TrackPopupMenu(GetSubMenu(PluginConfig.g_hMenuContext, 8), TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);

					if (wParam == 100)
						PostMessage(hwndDlg, WM_NULL, 0, 0);
				}
				break;

			case WM_MBUTTONDOWN:
				{
					if (wParam == 100)
						SetForegroundWindow(hwndDlg);

					int iCount = GetMenuItemCount(PluginConfig.g_hMenuTrayUnread);
					if (iCount > 0) {
						UINT uid = 0;
						MENUITEMINFOA mii = {0};
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
						}
							while (--i >= 0);

						if (uid == 0 && pLastActiveContainer != NULL) {                // no session found, restore last active container
							if (IsIconic(pLastActiveContainer->hwnd) || !IsWindowVisible(pLastActiveContainer->hwnd)) {
								SendMessage(pLastActiveContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
								SetForegroundWindow(pLastActiveContainer->hwnd);
								SetFocus(GetDlgItem(pLastActiveContainer->hwndActive, IDC_MESSAGE));
							}
							else if (GetForegroundWindow() != pLastActiveContainer->hwnd) {
								SetForegroundWindow(pLastActiveContainer->hwnd);
								SetFocus(GetDlgItem(pLastActiveContainer->hwndActive, IDC_MESSAGE));
							}
							else {
								if (PluginConfig.m_HideOnClose)
									ShowWindow(pLastActiveContainer->hwnd, SW_HIDE);
								else
									SendMessage(pLastActiveContainer->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
							}
						}
					}
					if (wParam == 100)
						PostMessage(hwndDlg, WM_NULL, 0, 0);
				}
				break;

			case WM_RBUTTONUP:
				{
					HMENU submenu = PluginConfig.g_hMenuTrayContext;
					POINT pt;

					if (wParam == 100)
						SetForegroundWindow(hwndDlg);
					GetCursorPos(&pt);
					CheckMenuItem(submenu, ID_TRAYCONTEXT_DISABLEALLPOPUPS, MF_BYCOMMAND | (nen_options.iDisable ? MF_CHECKED : MF_UNCHECKED));
					CheckMenuItem(submenu, ID_TRAYCONTEXT_DON40223, MF_BYCOMMAND | (nen_options.iNoSounds ? MF_CHECKED : MF_UNCHECKED));
					CheckMenuItem(submenu, ID_TRAYCONTEXT_DON, MF_BYCOMMAND | (nen_options.iNoAutoPopup ? MF_CHECKED : MF_UNCHECKED));
					EnableMenuItem(submenu, ID_TRAYCONTEXT_HIDEALLMESSAGECONTAINERS, MF_BYCOMMAND | (nen_options.bTraySupport) ? MF_ENABLED : MF_GRAYED);
					CheckMenuItem(submenu, ID_TRAYCONTEXT_SHOWTHETRAYICON, MF_BYCOMMAND | (nen_options.bTraySupport ? MF_CHECKED : MF_UNCHECKED));
					BOOL iSelection = TrackPopupMenu(submenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
					if (iSelection) {
						MENUITEMINFO mii = {0};

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
								for (p = pFirstContainer; p; p = p->pNext)
									ShowWindow(p->hwnd, SW_HIDE);
								break;
							case ID_TRAYCONTEXT_RESTOREALLMESSAGECONTAINERS:
								for (p = pFirstContainer; p; p = p->pNext)
									ShowWindow(p->hwnd, SW_SHOW);
								break;
							case ID_TRAYCONTEXT_BE:
								nen_options.iDisable = 1;
								nen_options.iNoSounds = 1;
								nen_options.iNoAutoPopup = 1;

								for (p = pFirstContainer; p; p = p->pNext)
									SendMessage(p->hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 1);
								break;
							}
						}
					}
					if (wParam == 100)
						PostMessage(hwndDlg, WM_NULL, 0, 0);
					break;
				}
			}
		}
		break;

	// handle an event from the popup module (mostly window activation). Since popups may run in different threads, the message
	// is posted to our invisible hotkey handler which does always run within the main thread.
	// wParam is the hContact
	// lParam the event handle
	case DM_HANDLECLISTEVENT:
		// if lParam == NULL, don't consider clist events, just open the message tab
		if (lParam == 0)
			HandleMenuEntryFromhContact(wParam);
		else {
			CLISTEVENT *cle = (CLISTEVENT *)CallService(MS_CLIST_GETEVENT, wParam, 0);
			if (cle) {
				if (ServiceExists(cle->pszService)) {
					CallService(cle->pszService, 0, (LPARAM)cle);
					CallService(MS_CLIST_REMOVEEVENT, (WPARAM)cle->hContact, (LPARAM)cle->hDbEvent);
				}
			}
			// still, we got that message posted.. the event may be waiting in tabSRMMs tray...
			else HandleMenuEntryFromhContact(wParam);
		}
		break;

	case DM_DOCREATETAB:
		{
			HWND hWnd = M.FindWindow(lParam);
			if (hWnd && IsWindow(hWnd)) {
				TContainerData *pContainer = 0;
				SendMessage(hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
				if (pContainer) {
					int iTabs = TabCtrl_GetItemCount(GetDlgItem(pContainer->hwnd, IDC_MSGTABS));
					if (iTabs == 1)
						SendMessage(pContainer->hwnd, WM_CLOSE, 0, 1);
					else
						SendMessage(hWnd, WM_CLOSE, 0, 1);

					CreateNewTabForContact((TContainerData*)wParam, lParam, 0, NULL, TRUE, TRUE, FALSE, 0);
				}
			}
		}
		break;

	case DM_DOCREATETAB_CHAT:
		{
			SESSION_INFO *si = SM_FindSessionByHWND((HWND)lParam);
			if (si && IsWindow(si->hWnd)) {
				TContainerData *pContainer = 0;
				SendMessage(si->hWnd, DM_QUERYCONTAINER, 0, (LPARAM)&pContainer);
				if (pContainer) {
					int iTabs = TabCtrl_GetItemCount(GetDlgItem(pContainer->hwnd, 1159));
					if (iTabs == 1)
						SendMessage(pContainer->hwnd, WM_CLOSE, 0, 1);
					else
						SendMessage(si->hWnd, WM_CLOSE, 0, 1);

					si->hWnd = CreateNewRoom((TContainerData*)wParam, si, TRUE, 0, 0);
				}
			}
		}
		break;

	case DM_SENDMESSAGECOMMANDW:
		SendMessageCommand_W(wParam, lParam);
		if (lParam)
			mir_free((void*)lParam);
		return 0;

	case DM_SENDMESSAGECOMMAND:
		SendMessageCommand(wParam, lParam);
		if (lParam)
			mir_free((void*)lParam);
		return 0;

	// sent from the popup to "dismiss" the event. we should do this in the main thread
	case DM_REMOVECLISTEVENT:
		CallService(MS_CLIST_REMOVEEVENT, wParam, lParam);
		db_event_markRead(wParam, (HANDLE)lParam);
		return 0;

	case DM_SETLOCALE:
		{
			HKL hkl = (HKL)lParam;
			MCONTACT hContact = wParam;

			HWND	hWnd = M.FindWindow(hContact);
			if (hWnd) {
				TWindowData *dat = (TWindowData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
				if (dat) {
					if (hkl) {
						dat->hkl = hkl;
						PostMessage(dat->hwnd, DM_SETLOCALE, 0, 0);
					}

					DBVARIANT  dbv;
					if (0 == db_get_ts(hContact, SRMSGMOD_T, "locale", &dbv)) {
						GetLocaleID(dat, dbv.ptszVal);
						db_free(&dbv);
						UpdateReadChars(dat);
					}
				}
			}
		}
		return 0;

	// react to changes in the desktop composition state
	// (enable/disable DWM, change to a non-aero visual style
	// or classic Windows theme
	case WM_DWMCOMPOSITIONCHANGED:
		{
			bool fNewAero = M.getAeroState();					// refresh dwm state
			SendMessage(hwndDlg, WM_THEMECHANGED, 0, 0);

			for (p = pFirstContainer; p; p = p->pNext) {
				if (fNewAero)
					SetAeroMargins(p);
				else {
					MARGINS m = {0};
					if (M.m_pfnDwmExtendFrameIntoClientArea)
						M.m_pfnDwmExtendFrameIntoClientArea(p->hwnd, &m);
				}
				if (p->SideBar->isActive())
					RedrawWindow(GetDlgItem(p->hwnd, 5000), NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);			// the container for the sidebar buttons
				RedrawWindow(p->hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
			}
		}
		M.BroadcastMessage(WM_DWMCOMPOSITIONCHANGED, 0, 0);
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

		for (p = pFirstContainer; p; p = p->pNext) {
			SendMessage(GetDlgItem(p->hwnd, IDC_MSGTABS), EM_THEMECHANGED, 0, 0);
			BroadCastContainer(p, EM_THEMECHANGED, 0, 0);
		}
		break;

	case DM_SPLITSENDACK:
		{
			SendJob *job = sendQueue->getJobByIndex((int)wParam);

			ACKDATA ack = {0};
			ack.hContact = job->hContact;
			ack.hProcess = job->hSendId;
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
				CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)p->hContact, 1);
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
		for (p = pFirstContainer; p; p = p->pNext)
			if (CSkin::m_skinEnabled) {             // invalidate cached background DCs for skinned containers
				p->oldDCSize.cx = p->oldDCSize.cy = 0;
				SelectObject(p->cachedDC, p->oldHBM);
				DeleteObject(p->cachedHBM);
				DeleteDC(p->cachedDC);
				p->cachedDC = 0;
				RedrawWindow(p->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
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
			for (p = pFirstContainer; p; p = p->pNext)
				SendMessage(p->hwnd, WM_TIMER, TIMERID_HEARTBEAT, 0);

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
			if ( !sendLater->haveJobs()) {
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
