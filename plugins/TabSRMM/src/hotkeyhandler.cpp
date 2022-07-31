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

static UINT WM_TASKBARCREATED;
static HANDLE hSvcHotkeyProcessor = nullptr;

static HOTKEYDESC _hotkeydescs[] = {
	{ "tabsrmm_mostrecent", LPGEN("Most recent unread session"), TABSRMM_HK_SECTION_IM, MS_TABMSG_HOTKEYPROCESS, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'R'), 0, TABSRMM_HK_LASTUNREAD },
	{ "tabsrmm_paste_and_send", LPGEN("Paste and send"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'D'), 0, TABSRMM_HK_PASTEANDSEND },
	{ "tabsrmm_uprefs", LPGEN("Contact's messaging preferences"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'C'), 0, TABSRMM_HK_SETUSERPREFS },
	{ "tabsrmm_copts", LPGEN("Container options"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'O'), 0, TABSRMM_HK_CONTAINEROPTIONS },
	{ "tabsrmm_sendfile", LPGEN("Send a file"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT, 'F'), 0, TABSRMM_HK_SENDFILE },
	{ "tabsrmm_quote", LPGEN("Quote message"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT, 'Q'), 0, TABSRMM_HK_QUOTEMSG },
	{ "tabsrmm_clear", LPGEN("Clear message area"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'K'), 0, TABSRMM_HK_CLEARMSG },
	{ "tabsrmm_sendlater", LPGEN("Toggle send later"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'S'), 0, TABSRMM_HK_TOGGLESENDLATER },

	{ "tabsrmm_hist", LPGEN("Show message history"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_ALT, 'H'), 0, TABSRMM_HK_HISTORY },
	{ "tabsrmm_sendmenu", LPGEN("Show send menu"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'S'), 0, TABSRMM_HK_SENDMENU },
	{ "tabsrmm_protomenu", LPGEN("Show protocol menu"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'P'), 0, TABSRMM_HK_PROTOMENU },
	{ "tabsrmm_umenu", LPGEN("Show user menu"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT, 'D'), 0, TABSRMM_HK_USERMENU },
	{ "tabsrmm_udet", LPGEN("Show user details"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT, 'U'), 0, TABSRMM_HK_USERDETAILS },
	{ "tabsrmm_tbar", LPGEN("Toggle toolbar"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_SHIFT, 'T'), 0, TABSRMM_HK_TOGGLETOOLBAR },
	{ "tabsrmm_ipanel", LPGEN("Toggle info panel"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'I'), 0, TABSRMM_HK_TOGGLEINFOPANEL },
	{ "tabsrmm_rtl", LPGEN("Toggle text direction"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'B'), 0, TABSRMM_HK_TOGGLERTL },
	{ "tabsrmm_msend", LPGEN("Toggle multi send"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'M'), 0, TABSRMM_HK_TOGGLEMULTISEND },
	{ "tabsrmm_clearlog", LPGEN("Clear message log"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_CONTROL, 'L'), 0, TABSRMM_HK_CLEARLOG },
	{ "tabsrmm_notes", LPGEN("Edit user notes"), TABSRMM_HK_SECTION_IM, nullptr, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), 0, TABSRMM_HK_EDITNOTES },
	{ "tabsrmm_sbar", LPGEN("Collapse side bar"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(0, VK_F9), 0, TABSRMM_HK_TOGGLESIDEBAR },
	{ "tabsrmm_muc_cmgr", LPGEN("Channel manager"), TABSRMM_HK_SECTION_GC, nullptr, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'C'), 0, TABSRMM_HK_CHANNELMGR },
	{ "tabsrmm_muc_filter", LPGEN("Toggle filter"), TABSRMM_HK_SECTION_GC, nullptr, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'F'), 0, TABSRMM_HK_FILTERTOGGLE },
	{ "tabsrmm_muc_nick", LPGEN("Toggle nick list"), TABSRMM_HK_SECTION_GC, nullptr, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, 'N'), 0, TABSRMM_HK_LISTTOGGLE },
	{ "tabsrmm_muc_server_show", LPGEN("Show server window"), TABSRMM_HK_SECTION_GC, nullptr, HOTKEYCODE(HOTKEYF_SHIFT | HOTKEYF_CONTROL, '1'), 0, TABSRMM_HK_MUC_SHOWSERVER },
	{ "tabsrmm_close_other", LPGEN("Close other tabs"), TABSRMM_HK_SECTION_GENERIC, nullptr, HOTKEYCODE(HOTKEYF_ALT | HOTKEYF_CONTROL, 'W'), 0, TABSRMM_HK_CLOSE_OTHER },
};

LRESULT CMsgDialog::ProcessHotkeysByMsgFilter(const CCtrlBase &pCtrl, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MSGFILTER mf;
	mf.nmhdr.code = EN_MSGFILTER;
	mf.nmhdr.hwndFrom = pCtrl.GetHwnd();
	mf.nmhdr.idFrom = pCtrl.GetCtrlId();
	mf.lParam = lParam;
	mf.wParam = wParam;
	mf.msg = msg;
	return OnFilter(&mf);
}

static void HandleMenuEntryFromhContact(MCONTACT hContact)
{
	if (hContact == 0)
		return;

	auto *pDlg = Srmm_FindDialog(hContact);
	if (pDlg && IsWindow(pDlg->GetHwnd())) {
		if (TContainerData *pContainer = pDlg->m_pContainer) {
			pDlg->ActivateTab();
			pContainer->m_hwndSaved = nullptr;
			SetForegroundWindow(pContainer->m_hwnd);
		}
		else CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		return;
	}

	Clist_ContactDoubleClicked(hContact);
}

static INT_PTR HotkeyProcessor(WPARAM, LPARAM lParam)
{
	if (lParam == TABSRMM_HK_LASTUNREAD) {
		if (g_arUnreadWindows.getCount()) {
			HANDLE hContact = g_arUnreadWindows[0];
			g_arUnreadWindows.remove(0);
			HandleMenuEntryFromhContact(UINT_PTR(hContact));
		}
		// restore last active container
		else if (pLastActiveContainer != nullptr) { 
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

	return 0;
}

void TSAPI DrawMenuItem(DRAWITEMSTRUCT *dis, HICON hIcon, uint32_t dwIdle)
{
	FillRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_MENU));
	if (dwIdle)
		CSkin::DrawDimmedIcon(dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - 16) / 2, 16, 16, hIcon, 180);
	else
		DrawIconEx(dis->hDC, 2, (dis->rcItem.bottom + dis->rcItem.top - 16) / 2, hIcon, 16, 16, 0, nullptr, DI_NORMAL | DI_COMPAT);
}

LONG_PTR CALLBACK HotkeyHandlerDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static POINT ptLast;
	static int iMousedown;

	switch (msg) {
	case WM_CREATE:
		for (auto &it : _hotkeydescs)
			g_plugin.addHotkey(&it);

		WM_TASKBARCREATED = RegisterWindowMessageA("TaskbarCreated");
		ShowWindow(hwndDlg, SW_HIDE);
		hSvcHotkeyProcessor = CreateServiceFunction(MS_TABMSG_HOTKEYPROCESS, HotkeyProcessor);
		SetTimer(hwndDlg, TIMERID_SENDLATER, TIMEOUT_SENDLATER, nullptr);
		break;

	case WM_HOTKEY:
		{
			CLISTEVENT *cli = g_clistApi.pfnGetEvent(-1, 0);
			if (cli != nullptr) {
				if (strncmp(cli->pszService, MS_MSG_TYPINGMESSAGE, mir_strlen(cli->pszService))) {
					CallService(cli->pszService, 0, (LPARAM)cli);
					break;
				}
			}
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
			if (dis->CtlType == ODT_MENU) {
				HWND hWnd = Srmm_FindWindow((MCONTACT)dis->itemID);
				uint32_t idle = 0;

				if (hWnd == nullptr) {
					SESSION_INFO *si = SM_FindSessionByHCONTACT((MCONTACT)dis->itemID);
					hWnd = si ? si->pDlg->GetHwnd() : nullptr;
				}

				CMsgDialog *dat = nullptr;
				if (hWnd)
					dat = (CMsgDialog*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

				{
					HICON hIcon;

					if (dis->itemData > 0)
						hIcon = (dis->itemData & 0x10000000) ? g_chatApi.getIcon(GC_EVENT_HIGHLIGHT) : PluginConfig.g_IconMsgEvent;
					else if (dat != nullptr) {
						hIcon = dat->GetMyContactIcon(nullptr);
						idle = dat->m_idle;
					}
					else hIcon = PluginConfig.g_iconContainer;

					DrawMenuItem(dis, hIcon, idle);
					return TRUE;
				}
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
			CLISTEVENT *cle = g_clistApi.pfnGetEvent(wParam, 0);
			if (cle) {
				if (ServiceExists(cle->pszService)) {
					CallService(cle->pszService, 0, (LPARAM)cle);
					g_clistApi.pfnRemoveEvent(cle->hContact, cle->hDbEvent);
				}
			}
			// still, we got that message posted.. the event may be waiting in tabSRMMs tray...
			else HandleMenuEntryFromhContact(wParam);
		}
		break;

	case DM_CREATECONTAINER:
		AutoCreateWindow(wParam, lParam);
		break;

	case DM_DOCREATETAB:
		{
			auto *pDlg = Srmm_FindDialog(lParam);
			if (pDlg) {
				TContainerData *pContainer = pDlg->m_pContainer;
				if (pContainer) {
					int iTabs = TabCtrl_GetItemCount(pContainer->m_hwndTabs);
					if (iTabs == 1)
						SendMessage(pContainer->m_hwnd, WM_CLOSE, 0, 1);
					else
						SendMessage(pDlg->GetHwnd(), WM_CLOSE, 0, 1);

					char *szProto = Proto_GetBaseAccountName(lParam);
					if (szProto != nullptr && Contact::IsGroupChat(lParam, szProto))
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
		g_clistApi.pfnRemoveEvent(wParam, lParam);
		db_event_markRead(wParam, lParam);
		return 0;

	case WM_DWMCOMPOSITIONCHANGED:
		// react to changes in the desktop composition state
		// (enable/disable DWM, change to a non-aero visual style
		// or classic Windows theme
		SendMessage(hwndDlg, WM_THEMECHANGED, 0, 0);
		{
			bool bNewAero = M.getAeroState(); // refresh dwm state

			for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext) {
				if (bNewAero)
					pCont->SetAeroMargins();
				else {
					MARGINS m = { 0 };
					if (M.m_pfnDwmExtendFrameIntoClientArea)
						M.m_pfnDwmExtendFrameIntoClientArea(pCont->m_hwnd, &m);
				}
				if (pCont->m_pSideBar)
					if (pCont->m_pSideBar->isActive()) // the container for the sidebar buttons
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
			SendMessage(pCont->m_hwndTabs, EM_THEMECHANGED, 0, 0);
			pCont->BroadCastContainer(EM_THEMECHANGED, 0, 0);
		}
		break;

	case DM_SPLITSENDACK:
		{
			SendJob *job = sendQueue->getJobByIndex((int)wParam);

			ACKDATA ack = {};
			ack.hContact = job->hContact;
			ack.hProcess = (HANDLE)job->iSendId;
			ack.type = ACKTYPE_MESSAGE;
			ack.result = ACKRESULT_SUCCESS;

			if (job->hContact && job->iAcksNeeded && job->iStatus == SendQueue::SQ_INPROGRESS) {
				if (IsWindow(job->hOwnerWnd))
					::SendMessage(job->hOwnerWnd, HM_EVENTSENT, (WPARAM)MAKELONG(wParam, 0), (LPARAM)&ack);
				else
					sendQueue->ackMessage(nullptr, (WPARAM)MAKELONG(wParam, 0), (LPARAM)&ack);
			}
		}
		return 0;

	case DM_LOGSTATUSCHANGE:
		CGlobals::logStatusChange(wParam, reinterpret_cast<CContactCache *>(lParam));
		return 0;

	case WM_POWERBROADCAST:
	case WM_DISPLAYCHANGE:
		for (TContainerData *pCont = pFirstContainer; pCont; pCont = pCont->pNext)
			if (CSkin::m_skinEnabled) {             // invalidate cached background DCs for skinned containers
				pCont->m_oldDCSize.cx = pCont->m_oldDCSize.cy = 0;
				SelectObject(pCont->m_cachedDC, pCont->m_oldHBM);
				DeleteObject(pCont->m_cachedHBM);
				DeleteDC(pCont->m_cachedDC);
				pCont->m_cachedDC = nullptr;
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
			if (SendLater::Avail && !SendLater::isInteractive() && (time(0) - SendLater::lastProcessed()) > SENDLATER_PROCESS_INTERVAL) {
				SendLater::setLastProcessed(time(0));

				// check the list of contacts that may have new send later jobs
				// (added on user's request)
				SendLater::processContacts();

				// start processing the job list
				if (!SendLater::isJobListEmpty()) {
					KillTimer(hwndDlg, wParam);
					SendLater::startJobListProcess();
					SetTimer(hwndDlg, TIMERID_SENDLATER_TICK, TIMEOUT_SENDLATER_TICK, nullptr);
				}
			}
		}

		// process one entry per tick (default: 200ms)
		// TODO better timings, possibly slow down when many jobs are in the
		// queue.
		else if (wParam == TIMERID_SENDLATER_TICK) {
			if (!SendLater::haveJobs()) {
				KillTimer(hwndDlg, wParam);
				SetTimer(hwndDlg, TIMERID_SENDLATER, TIMEOUT_SENDLATER, nullptr);
				SendLater::qMgrUpdate(true);
			}
			else SendLater::processCurrentJob();
		}
		break;

	case WM_DESTROY:
		DestroyServiceFunction(hSvcHotkeyProcessor);
		KillTimer(hwndDlg, TIMERID_SENDLATER_TICK);
		KillTimer(hwndDlg, TIMERID_SENDLATER);
		break;
	}
	return DefWindowProc(hwndDlg, msg, wParam, lParam);
}
